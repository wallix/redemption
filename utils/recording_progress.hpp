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

class UpdateProgressData : noncopyable {
    int fd;

    const time_t start_record;
    const time_t stop_record;

    const time_t processing_start_time;

    unsigned int last_written_time_percentage;

public:
    UpdateProgressData( const char * progress_filename
                      , const time_t begin_record, const time_t end_record
                      , const time_t begin_capture, const time_t end_capture) noexcept
    : fd(-1)
    , start_record(begin_capture ? begin_capture : begin_record)
    , stop_record(end_capture ? end_capture : end_record)
    , processing_start_time(::time(NULL))
    , last_written_time_percentage(0) {
        this->fd = ::open(progress_filename, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP);
        if (this->fd != -1) {
            int write_result = ::write(this->fd, "0 -1", 4);
(void)write_result;
        }
        else {
            cerr << "Failed to create file: \"" << progress_filename << "\"" << endl;
        }
    }

    ~UpdateProgressData() {
        if (this->fd != -1) {
            ::lseek(this->fd, 0, SEEK_SET);
            int write_result = ::write(this->fd, "100 0", 5);
(void)write_result;
            ::close(this->fd);
        }
    }

    bool is_valid() const {
        return (this->fd != -1);
    }

    void operator()(time_t record_now) {
        if (this->fd == -1) {
            return;
        }

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

        REDASSERT(time_percentage < 100);

        if (time_percentage != this->last_written_time_percentage) {
            unsigned int elapsed_time = ::time(NULL) - this->processing_start_time;

            char str_time_percentage[64];

            ::snprintf( str_time_percentage, sizeof(str_time_percentage), "%u %u"
                      , time_percentage
                      , elapsed_time * 100 / time_percentage - elapsed_time);

            ::lseek(this->fd, 0, SEEK_SET);
            int write_result = ::write(this->fd, str_time_percentage, strlen(str_time_percentage));
            if (write_result != -1) {
                int truncate_result = ::ftruncate(this->fd, write_result);
(void)truncate_result;
            }

            this->last_written_time_percentage = time_percentage;
        }
    }
};
