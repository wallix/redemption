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

#pragma once

#include "utils/sugar/unique_fd.hpp"
#include "utils/log.hpp"
#include "utils/difftimeval.hpp"
#include "utils/texttime.hpp"
#include "system/linux/system/ssl_sha256.hpp"

#include <vector>

#include <cerrno>
#include <cstdio>
#include <ctime>

#include <cstring>
#include <fcntl.h>
#include <sys/uio.h>


class MetricsHmacSha256Encrypt
{
    std::array<char, SslSha256::DIGEST_LENGTH*2+1> dest;

public:
    MetricsHmacSha256Encrypt(const_byte_ptr src, const_byte_ptr key_crypt)
    {
        SslHMAC_Sha256 sha256(key_crypt, 32);
        sha256.update(src, sizeof(src));
        uint8_t sig[SslSha256::DIGEST_LENGTH];
        sha256.final(sig);

        static_assert(sizeof(sig) * 2 + 1 == sizeof(dest));

        const char * hex = "0123456789ABCDEF";
        auto p = std::begin(dest);
        for (uint8_t c : sig) {
            *p = hex[(c>>4) & 0xF];
            ++p;
            *p = hex[c & 0xF];
            ++p;
        }
        *p = 0;
    }

    operator array_view_const_char() const noexcept
    {
        return this->av();
    }

    array_view_const_char av() const noexcept
    {
        return {dest.data(), dest.size()-1u};
    }
};


class Metrics
{
public:
    std::vector<uint64_t> current_data;

    const std::string version;
    const std::string protocol_name;

    // output file info
    const std::chrono::hours file_interval;
    std::chrono::seconds current_file_date;
    const std::string path;
    unique_fd fd = invalid_fd();

    // LOG info
    struct Header
    {
        char buffer[1024];
        size_t len;

        iovec to_iovec() const
        {
            return {const_cast<char*>(buffer), len}; /*NOLINT*/
        }
    };
    Header header;
    const std::string session_id;
    const bool active_ = false;

    const std::chrono::seconds connection_time;

    const std::chrono::seconds log_delay;
    timeval next_log_time;
    char complete_file_path[4096] = {'\0'};

public:
    Metrics( std::string fields_version
           , std::string protocol_name
           , const bool activate                            // do nothing if false
           , size_t     nb_metric_item
           , std::string path
           , std::string session_id
           , array_view_const_char primary_user_sig         // hashed primary user account
           , array_view_const_char account_sig              // hashed secondary account
           , array_view_const_char target_service_sig       // hashed (target service name + device name)
           , array_view_const_char session_info_sig         // hashed (source_host + client info)
           , const std::chrono::seconds now                 // time at beginning of metrics
           , const std::chrono::hours file_interval         // daily rotation of filename
           , const std::chrono::seconds log_delay           // delay between 2 logs flush
           )
    : current_data(nb_metric_item, 0)
    , version(std::move(fields_version))
    , protocol_name(std::move(protocol_name))
    , file_interval{file_interval}
    , current_file_date(now - (now % this->file_interval))
    , path(std::move(path))
    , session_id(std::move(session_id.insert(0, 1, ' ')))
    , active_(activate)
    , connection_time(now)
    , log_delay(log_delay)
    , next_log_time{to_timeval(this->log_delay+now)}
    {
        if (activate) {
            this->header.len = size_t(snprintf(this->header.buffer, sizeof(this->header.buffer),
                "%.*s user=%.*s account=%.*s target_service_device=%.*s client_info=%.*s\n",
                int(this->session_id.size()-1u), this->session_id.data()+1,
                int(primary_user_sig.size()), primary_user_sig.data(),
                int(account_sig.size()), account_sig.data(),
                int(target_service_sig.size()), target_service_sig.data(),
                int(session_info_sig.size()), session_info_sig.data()));

            this->new_file(this->current_file_date);
        }
    }

    std::size_t count_data() noexcept
    {
        return this->current_data.size();
    }

    void add_to_current_data(size_t index, uint64_t value) noexcept
    {
        this->current_data[index] += value;
    }

    ~Metrics()
    {
         this->disconnect();
    }

    void log(timeval const& now)
    {
        if (!this->active_) {
            return ;
        }

        if (this->next_log_time > now) {
            return ;
        }

        this->next_log_time.tv_sec += to_timeval(this->log_delay).tv_sec;
        this->next_log_time.tv_usec = now.tv_usec;

        this->rotate(std::chrono::seconds(now.tv_sec));

        auto text_datetime = text_gmdatetime(std::chrono::seconds(now.tv_sec));

        char sentence[4096];
        char * ptr = sentence;
        for (auto x : this->current_data){
            ptr += ::snprintf(ptr, sizeof(sentence) - (ptr - sentence), " %lu", x);
        }
        ptr += ::snprintf(ptr, sizeof(sentence) - (ptr - sentence), "\n");

        iovec iov[] = {
            to_iov(text_datetime),
            to_iov(this->session_id),
            {sentence, size_t(ptr-sentence)}
        };

        ssize_t nwritten = ::writev(this->fd.fd(), iov, std::size(iov));

        if (nwritten == -1) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\": %s",
                errnum, this->complete_file_path, strerror(errnum));
        }
    }

    void disconnect()
    {
        if (!this->active_) {
            return ;
        }

        using namespace std::literals::chrono_literals;

        this->rotate(std::chrono::seconds(this->next_log_time.tv_sec));

        auto text_date = (this->current_file_date % 24h == 0s)
          ? text_gmdate(this->current_file_date)
          : filename_gmdatetime(this->current_file_date);

        this->write_event_to_logindex(
            text_date,
            std::chrono::seconds(this->next_log_time.tv_sec),
            " disconnection ");
    }

    void new_file(std::chrono::seconds now)
    {
        using namespace std::literals::chrono_literals;

        auto text_date = (now % 24h == 0s) ? text_gmdate(now) : filename_gmdatetime(now);

        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path),
            "%s%s_metrics-%s-%s.logmetrics",
            this->path.c_str(), this->protocol_name.c_str(), this->version.c_str(), text_date.c_str());

        this->fd = unique_fd(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!this->fd.is_open()) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\": %s", errnum, this->complete_file_path, strerror(errnum));
        }

        this->write_event_to_logindex(text_date, this->connection_time, " connection ");
    }

    void rotate(std::chrono::seconds now)
    {
        auto next_file_date = now - now % this->file_interval;
        if (this->current_file_date != next_file_date) {
            this->current_file_date = next_file_date;
            this->new_file(next_file_date);
        }
    }

private:
    void write_event_to_logindex(
        array_view_const_char date_for_file,
        std::chrono::seconds event_time,
        array_view_const_char event_name)
    {
        char index_file_path[4096];
        ::snprintf(index_file_path, sizeof(index_file_path),
            "%s%s_metrics-%s-%s.logindex",
            this->path.c_str(),
            this->protocol_name.c_str(),
            this->version.c_str(),
            date_for_file.data());

        unique_fd fd_header(index_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

        auto disconnect_time_str = text_gmdatetime(event_time);

        iovec iov[] = {
            to_iov(disconnect_time_str),
            to_iov(event_name),
            this->header.to_iovec(),
        };

        ssize_t const nwritten = ::writev(fd_header.fd(), iov, std::size(iov));

        if (nwritten == -1) {
            int const errnum = errno;
            LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\": %s", errnum, index_file_path, strerror(errnum));
        }
    }

    static iovec to_iov(array_view_const_char av) noexcept
    {
        return {const_cast<char*>(av.data()), av.size()}; /*NOLINT*/
    }
};
