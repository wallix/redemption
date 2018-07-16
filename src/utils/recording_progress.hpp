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

#include <cassert>
#include <ctime>
#include <iostream>
#include <cerrno>
#include <cstring>

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
class UpdateProgressDataJSONFormat : noncopyable
{
    unique_fd fd;

    const time_t start_record;
    const time_t stop_record;

    const time_t processing_start_time;


    mutable bool error_raised = false;

    unsigned time_percentage = 0;
    unsigned int last_written_time_percentage = 0;
    int time_remaining = -1; // -1 = unknown
    unsigned nb_videos = 0;

public:
    UpdateProgressDataJSONFormat() = delete;
    UpdateProgressDataJSONFormat(
        unique_fd fd, const time_t start_record, const time_t stop_record) noexcept
    : fd(std::move(fd))
    , start_record(start_record)
    , stop_record(stop_record)
    , processing_start_time(::time(nullptr))
    {
        this->write_datas();
    }

    ~UpdateProgressDataJSONFormat()
    {
        if (!this->error_raised) {
            this->time_percentage = 100;
            this->time_remaining = 0;
            ++this->nb_videos;
            this->write_datas();
        }
    }

    void next_video(time_t record_now)
    {
        ++this->nb_videos;
        (*this)(record_now);
    }

    bool is_valid() const
    {
        return this->fd.is_open();
    }

    void operator()(time_t record_now)
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
            unsigned int const elapsed_time = ::time(nullptr) - this->processing_start_time;
            this->time_remaining = elapsed_time * 100 / this->time_percentage - elapsed_time;
            this->write_datas();
            this->last_written_time_percentage = this->time_percentage;
        }
    }

    void raise_error(int code, const char * message)
    {
        char str_error_message[1024];

        int const len = std::snprintf(
            str_error_message, sizeof(str_error_message),
            R"({"percentage":%u,"eta":%d,"videos":%u,"error":{"code":%d,"message":"%s"}})" ,
            this->time_percentage, this->time_remaining, this->nb_videos, code, (message ? message : "")
        );

        this->error_raised = true;
        this->write_buf(str_error_message, len);
    }

private:
    void write_datas()
    {
        char str_json[64];

        int const len = std::snprintf(
            str_json, sizeof(str_json),
            R"({"percentage":%u,"eta":%d,"videos":%u})",
            this->time_percentage, this->time_remaining, this->nb_videos
        );

        this->write_buf(str_json, len);
    }

    void write_buf(char const * buf, std::size_t len)
    {
        bool has_error = true;

        off_t const seek_result = ::lseek(this->fd.fd(), 0, SEEK_SET);
        if (seek_result != -1) {
            ssize_t const write_result = ::write(this->fd.fd(), buf, len);
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
};


// Old format

// format:
// start: 0 -1
// normal: $time_percentage $time_remaining
// done: 100 0
// error: -1 $error ($error_code)
class UpdateProgressDataOldFormat : noncopyable
{
    unique_fd fd;

    const time_t start_record;
    const time_t stop_record;

    const time_t processing_start_time;

    unsigned int last_written_time_percentage;

    mutable bool error_raised = false;

public:
    UpdateProgressDataOldFormat() = delete;
    UpdateProgressDataOldFormat(
        unique_fd fd, const time_t start_record, const time_t stop_record) noexcept
    : fd(std::move(fd))
    , start_record(start_record)
    , stop_record(stop_record)
    , processing_start_time(::time(nullptr))
    , last_written_time_percentage(0)
    {
        this->write("0 -1", 4);
    }

    ~UpdateProgressDataOldFormat()
    {
        if (!this->error_raised) {
            this->write("100 0", 5);
        }
    }

    bool is_valid() const
    {
        return this->fd.is_open();
    }

    void operator()(time_t record_now)
    {
        unsigned int time_percentage;

        if (record_now <= this->start_record) {
            time_percentage = 0;
        }
        else if (record_now >= this->stop_record) {
            time_percentage = 99;
        }
        else {
            time_percentage = (record_now - this->start_record) * 100 /
                (this->stop_record - this->start_record);
        }

        assert(time_percentage < 100);

        if (time_percentage != this->last_written_time_percentage) {
            unsigned int elapsed_time = ::time(nullptr) - this->processing_start_time;

            char str_time_percentage[64];

            std::size_t len = ::snprintf( str_time_percentage, sizeof(str_time_percentage), "%u %u"
                                        , time_percentage
                                        , elapsed_time * 100 / time_percentage - elapsed_time);

            this->write(str_time_percentage, len);
            this->last_written_time_percentage = time_percentage;
        }
    }

    void raise_error(int code, const char * message)
    {
        char str_error_message[1024];

        std::size_t len = ::snprintf( str_error_message, sizeof(str_error_message), "-1 %s (%d)"
                                    , (message ? message : "")
                                    , code
                                    );

        this->write(str_error_message, len);
        this->error_raised = true;
    }

private:
    void write(void const * data, size_t len)
    {
        bool has_error = true;

        off_t const seek_result = ::lseek(this->fd.fd(), 0, SEEK_SET);
        if (seek_result != -1) {
            ssize_t const write_result = ::write(this->fd.fd(), data, len);
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
};


class UpdateProgressData : noncopyable
{
public:
    enum Format { OLD_FORMAT, JSON_FORMAT };

    UpdateProgressData() = delete;
    UpdateProgressData(
        Format format, const char * progress_filename,
        const time_t begin_record, const time_t end_record,
        const time_t begin_capture, const time_t end_capture
    ) noexcept
    : format(format)
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

        auto const start_record = (begin_capture ? begin_capture : begin_record);
        auto const stop_record = (end_capture ? end_capture : end_record);

        if (format == OLD_FORMAT) {
            new(&u.old_format) UpdateProgressDataOldFormat(
                unique_fd{fd}, start_record, stop_record
            );
        }
        else {
            new(&u.json_format) UpdateProgressDataJSONFormat(
                unique_fd{fd}, start_record, stop_record
            );
        }
    }

    void next_video(time_t record_now)
    {
        if (format == OLD_FORMAT) {
            u.old_format(record_now);
        }
        else {
            u.json_format.next_video(record_now);
        }
    }

    ~UpdateProgressData()
    {
        if (format == OLD_FORMAT) {
            u.old_format.~UpdateProgressDataOldFormat();
        }
        else {
            u.json_format.~UpdateProgressDataJSONFormat();
        }
    }


    bool is_valid() const
    {
        return (format == OLD_FORMAT)
            ? u.old_format.is_valid()
            : u.json_format.is_valid();
    }


    void operator()(time_t record_now)
    {
        if (format == OLD_FORMAT) {
            u.old_format(record_now);
        }
        else {
            u.json_format(record_now);
        }
    }

    void raise_error(int code, const char * message)
    {
        if (format == OLD_FORMAT) {
            u.old_format.raise_error(code, message);
        }
        else {
            u.json_format.raise_error(code, message);
        }
    }

private:
    union U {
        char dummy;
        UpdateProgressDataOldFormat old_format;
        UpdateProgressDataJSONFormat json_format;
        U() : dummy() {}
        ~U() {} /*NOLINT*/
    } u;
    Format format;
};
