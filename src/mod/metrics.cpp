/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#include "mod/metrics.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/log.hpp"
#include "utils/fileutils.hpp"

#include <cerrno>
#include <cinttypes>
#include <cstdio>

#include <fcntl.h>
#include <sys/uio.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>


bool create_metrics_directory(const std::string & path)
{
    if (0 != access(path.c_str(), F_OK)) {
        LOG(LOG_INFO, "Creation of %s directory to store metrics", path);
        recursive_create_directory(path.c_str(), ACCESSPERMS, -1);
    }

    if (0 != access(path.c_str(), F_OK)) {
        LOG(LOG_WARNING, "Creation of %s directory to store metrics failed, metrics will be disabled", path);
        return false;
    }
    return true;
}

namespace
{
    // Returns the beginning of the timeslice of width seconds containing timepoint
    // origin of intervals is midnight 1 jan 1970
    MonotonicTimePoint timeslice(MonotonicTimePoint tp, MonotonicTimePoint::duration seconds)
    {
        return tp - tp.time_since_epoch() % seconds;
    }

    bool is_midnight(MonotonicTimePoint tp)
    {
        return (tp.time_since_epoch() % MonotonicTimePoint::duration(24h)).count() == 0;
    }

    tm to_tm(MonotonicTimePoint tp, DurationFromMonotonicTimeToRealTime monotonic_to_real)
    {
        struct tm tm;
        auto now = tp.time_since_epoch() + monotonic_to_real.duration;
        time_t time = std::chrono::duration_cast<std::chrono::seconds>(now).count();
        gmtime_r(&time, &tm);
        return tm;
    }

    // yyyy-MM-dd
    chars_view strftime_date(MonotonicTimePoint tp, DurationFromMonotonicTimeToRealTime monotonic_to_real, char (&buf)[35])
    {
        const auto tm = to_tm(tp, monotonic_to_real);
        return {buf, buf + snprintf(buf, std::size(buf), "%04d-%02d-%02d",
            1900+tm.tm_year, 1+tm.tm_mon, tm.tm_mday)
        };
    }

    // yyyy-MM-dd_hh-mm-ss
    chars_view strftime_filename(MonotonicTimePoint tp, DurationFromMonotonicTimeToRealTime monotonic_to_real, char (&buf)[35])
    {
        const auto tm = to_tm(tp, monotonic_to_real);
        return {buf, buf + snprintf(buf, std::size(buf),
            "%04d-%02d-%02d_%02d-%02d-%02d",
            1900+tm.tm_year, 1+tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec)
        };
    }

    // yyyy-MM-dd hh:mm:ss
    chars_view strftime_event(MonotonicTimePoint tp, DurationFromMonotonicTimeToRealTime monotonic_to_real, char (&buf)[35])
    {
        const auto tm = to_tm(tp, monotonic_to_real);
        return {buf, buf + snprintf(buf, std::size(buf),
            "%04d-%02d-%02d %02d:%02d:%02d",
            1900+tm.tm_year, 1+tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec)
        };
    }

    iovec to_iov(chars_view av) noexcept
    {
        return {const_cast<char*>(av.data()), av.size()}; /*NOLINT*/
    }
} // anonymous namespace

struct Metrics::Impl
{
    static void new_file(Metrics& m, MonotonicTimePoint timeslice, DurationFromMonotonicTimeToRealTime monotonic_to_real)
    {
        char buf[35];
        auto text_date = is_midnight(timeslice)
            ? strftime_date(timeslice, monotonic_to_real, buf)
            : strftime_filename(timeslice, monotonic_to_real, buf);

        auto metrics_path_suffix = ".logmetrics"_av;
        str_assign(m.complete_metrics_file_path,
            m.path, '/', m.protocol_name,
            "_metrics-", m.version, '-', text_date,
            metrics_path_suffix);
        m.complete_index_file_path = m.complete_metrics_file_path;
        m.complete_index_file_path.erase(m.complete_index_file_path.size() - metrics_path_suffix.size());
        m.complete_index_file_path += ".logindex";

        m.fd = unique_fd(m.complete_metrics_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!m.fd.is_open()) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\": %s", errnum,
                m.complete_metrics_file_path, strerror(errnum));
        }

        write_event_to_logindex(m, m.connection_time, monotonic_to_real, " connection "_av);
    }

    static void write_event_to_logmetrics(Metrics& m, MonotonicTimePoint now, DurationFromMonotonicTimeToRealTime monotonic_to_real)
    {
        char buf[35];
        auto text_datetime = strftime_event(now, monotonic_to_real, buf);

        char sentence[4096];
        char * ptr = sentence;
        for (auto x : m.current_data){
            ptr += ::snprintf(ptr, sizeof(sentence) - (ptr - sentence), " %" PRIu64, x);
        }
        ptr += ::snprintf(ptr, sizeof(sentence) - (ptr - sentence), "\n");

        iovec iov[] = {
            to_iov(text_datetime),
            to_iov(m.session_id),
            {sentence, size_t(ptr-sentence)}
        };

        ssize_t nwritten = ::writev(m.fd.fd(), iov, std::size(iov));

        if (nwritten == -1) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\": %s",
                errnum, m.complete_metrics_file_path, strerror(errnum));
        }
    }

    static void write_event_to_logindex(Metrics& m, MonotonicTimePoint tp, DurationFromMonotonicTimeToRealTime monotonic_to_real, chars_view event_name)
    {
        unique_fd fd_header(m.complete_index_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!fd_header.is_open()) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics Index error(%d): can't open \"%s\": %s", errnum, m.complete_index_file_path, strerror(errnum));
        }

        char buf[35];
        auto disconnect_time_str = strftime_event(tp, monotonic_to_real, buf);

        iovec iov[] = {
            to_iov(disconnect_time_str),
            to_iov(event_name),
            {const_cast<char*>(m.header.buffer), m.header.len} /*NOLINT*/
        };

        ssize_t const nwritten = ::writev(fd_header.fd(), iov, std::size(iov));

        if (nwritten == -1) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics Index error(%d): can't write \"%s\": %s", errnum, m.complete_index_file_path, strerror(errnum));
        }
    }
};


void Metrics::set_protocol(std::string fields_version, std::string protocol_name, size_t nb_metric_item)
{
    this->version = std::move(fields_version);
    this->protocol_name = std::move(protocol_name);
    this->current_data.resize(nb_metric_item, 0);

    LOG(LOG_INFO, "Metrics recording is enabled (%s) log_delay=%" PRIu64 " sec rotation=%" PRIu64 " hours",
        this->path.c_str(), static_cast<uint64_t>(this->log_delay.count()), static_cast<uint64_t>(this->file_interval.count()));

    Impl::new_file(*this, this->current_file_date, this->monotonic_to_real);
}

Metrics::Metrics(
    std::string const& path,
    std::string session_id,
    chars_view primary_user_sig,   // hashed primary user account
    chars_view account_sig,        // hashed secondary account
    chars_view target_service_sig, // hashed (target service name + device name)
    chars_view session_info_sig,   // hashed (source_host + client info)
    MonotonicTimePoint now,                   // time at beginning of metrics
    DurationFromMonotonicTimeToRealTime monotonic_to_real,
    std::chrono::hours file_interval,         // daily rotation of filename
    std::chrono::seconds log_delay            // delay between 2 logs flush
)
: current_data(0)
, version("0.0")
, protocol_name("none")
, file_interval{file_interval}
, current_file_date(timeslice(now, this->file_interval))
, monotonic_to_real(monotonic_to_real)
, path((path.back() == '/') ? path.substr(0,path.size()-1) : path)
, session_id(std::move(session_id.insert(0, 1, ' ')))
, connection_time(now)
, log_delay(log_delay)
, next_log_time{now+this->log_delay}
{
    this->header.len = size_t(snprintf(this->header.buffer, sizeof(this->header.buffer),
        "%.*s user=%.*s account=%.*s target_service_device=%.*s client_info=%.*s\n",
        int(this->session_id.size()-1u), this->session_id.data()+1,
        int(primary_user_sig.size()), primary_user_sig.data(),
        int(account_sig.size()), account_sig.data(),
        int(target_service_sig.size()), target_service_sig.data(),
        int(session_info_sig.size()), session_info_sig.data()));
}

void Metrics::log(MonotonicTimePoint now)
{
    if (this->next_log_time > now) {
        return ;
    }

    this->next_log_time += this->log_delay;

    this->rotate(now);
    Impl::write_event_to_logmetrics(*this, now, this->monotonic_to_real);
}

void Metrics::disconnect()
{
    this->rotate(this->next_log_time);
    Impl::write_event_to_logmetrics(*this, this->next_log_time, this->monotonic_to_real);
    Impl::write_event_to_logindex(*this, this->next_log_time, this->monotonic_to_real, " disconnection "_av);
}

void Metrics::rotate(MonotonicTimePoint now)
{
    if (this->current_file_date + this->file_interval <= now) {
        this->current_file_date = timeslice(now, this->file_interval);
        Impl::new_file(*this, this->current_file_date, this->monotonic_to_real);
    }
}
