/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once

#include "utils/log.hpp"

#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/bounded_array_view.hpp"
#include "utils/monotonic_clock.hpp"

#include <cassert>
#include <cerrno>
#include <cstdio>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * Format (JSON):
 * \code{.json}
   {
    "percentage": [0..100],
    "eta": [-1..],
    "videos": [0..],
    "error": {
        "code": %d,
        "message": "Failed to generate video"
    }
   }
 * \endcode
 *
 * - percentage is the remaining percentage of the video
 * - eta is the estimated time of arrival, the approximate end of generation of total videos. Start with -1 (unknown)
 * - videos is the number of already genereated videos
 * - error exists if the last videos fails (here the number 3), if it fails it sends back
 *   - an error code
 *   - a message
 */
class UpdateProgressData : noncopyable
{
    unique_fd fd;

    const MonotonicTimePoint start_record;
    const MonotonicTimePoint stop_record;

    const MonotonicTimePoint processing_start_time;

    mutable bool error_raised = false;

    unsigned time_percentage = 0;
    unsigned int last_written_time_percentage = 0;
    int time_remaining = -1; // -1 = unknown
    unsigned nb_videos = 0;

public:
    UpdateProgressData(
        const char * progress_filename,
        MonotonicTimePoint start_record, MonotonicTimePoint stop_record)
    : fd(create_progress_file(progress_filename))
    , start_record(start_record)
    , stop_record(stop_record)
    , processing_start_time(MonotonicTimePoint::clock::now())
    {
        this->write_datas();
    }

    ~UpdateProgressData()
    {
        if (!this->error_raised) {
            this->time_percentage = 100;
            this->time_remaining = 0;
            ++this->nb_videos;
            this->write_datas();
        }
    }

    void next_video(MonotonicTimePoint record_now)
    {
        ++this->nb_videos;
        (*this)(record_now);
    }

    bool is_valid() const
    {
        return this->fd.is_open();
    }

    void operator()(MonotonicTimePoint record_now)
    {
        if (record_now <= this->start_record) {
            this->time_percentage = 0;
        }
        else if (record_now >= this->stop_record) {
            this->time_percentage = 99;
        }
        else {
            this->time_percentage = (record_now - this->start_record) * 100 /
                (this->stop_record - this->start_record);
        }

        assert(this->time_percentage < 100);

        if (this->time_percentage != this->last_written_time_percentage) {
            auto const elapsed_time = MonotonicTimePoint::clock::now() - this->processing_start_time;
            this->time_remaining = std::chrono::duration_cast<std::chrono::seconds>(
                elapsed_time * 100 / this->time_percentage - elapsed_time).count();
            this->write_datas();
            this->last_written_time_percentage = this->time_percentage;
        }
    }

    void raise_error(int code, bounded_chars_view<0, 1024> message)
    {
        this->error_raised = true;

        char json[1024+256];
        int len = std::snprintf(json, sizeof(json),
            R"({"percentage":%u,"eta":%d,"videos":%u,"error":{"code":%d,"message":"%.*s"}})",
            this->time_percentage, this->time_remaining, this->nb_videos, code,
            int(message.size()), message.data());
        this->write_json({json, size_t(len)});

        // auto json =
        //     R"({"percentage":%u,"eta":%d,"videos":%u,"error":{"code":%d,"message":"%s"}})"
        //     ""_static_fmt(this->time_percentage, this->time_remaining, this->nb_videos, code, message);
        // this->write_json(json);
    }

private:
    void write_datas()
    {
        char json[256];
        int len = std::snprintf(json, sizeof(json),
            R"({"percentage":%u,"eta":%d,"videos":%u})",
            this->time_percentage, this->time_remaining, this->nb_videos);
        this->write_json({json, size_t(len)});

        // auto json =
        //     R"({"percentage":%u,"eta":%d,"videos":%u})"
        //     ""_static_fmt(this->time_percentage, this->time_remaining, this->nb_videos);
        // this->write_json(json);
    }

    void write_json(chars_view json)
    {
        bool has_error = true;

        off_t const seek_result = ::lseek(this->fd.fd(), 0, SEEK_SET);
        if (seek_result != -1) {
            ssize_t const write_result = ::write(this->fd.fd(), json.data(), json.size());
            if (write_result != -1) {
                int const truncate_result = ::ftruncate(this->fd.fd(), write_result);
                if (truncate_result !=  1) {
                    has_error = false;
                }
            }
        }

        if (has_error && this->fd.is_open()) {
            LOG(LOG_ERR, "Failed to write progress information file! %s", strerror(errno));
            this->fd.close();
        }
    }

    static unique_fd create_progress_file(const char * progress_filename)
    {
        int const file_mode = S_IRUSR | S_IRGRP;
        int const fd = ::open(progress_filename, O_CREAT | O_TRUNC | O_WRONLY, file_mode);
        // umask (man umask) can change effective mode of created file
        if ((fd < 0) || (chmod(progress_filename, file_mode) < 0)) {
            int const errnum = errno;
            LOG(LOG_ERR, "%s progress information file (%s): %s [%d]",
                (fd < 0) ? "Failed to create" : "Can't change mod of",
                progress_filename, strerror(errno), errnum);
        }
        return unique_fd{fd};
    }
};
