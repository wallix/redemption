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
    Copyright (C) Wallix 2015
    Author(s): Clément Moroldo
*/

#pragma once

#include "utils/sugar/unique_fd.hpp"
#include <fstream>


class ChannelFile {

private:
    const std::string dir_path;
    unique_fd fd;
    size_t total_file_size;
    size_t current_file_size ;

    bool validated;

public:
    ChannelFile(const std::string & dir_path)
    : dir_path(dir_path)
    , fd(invalid_fd())
    , total_file_size(0)
    ,current_file_size(0)
    , validated(false)
    {}

//     void set_total_file_size() {
//         this->total_file_size = total_file_size;
//     }

    void new_file(const std::string & filename, const size_t total_file_size) {
        this->total_file_size = total_file_size;
        this->current_file_size = 0;
        this->fd.close();
        std::string file_path = this->dir_path + filename;
        std::ifstream file(file_path.c_str());
        if (!file.good()) {
            this->fd = unique_fd(file_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
            if (!this->fd.is_open()) {
                LOG(LOG_WARNING,"File error, can't open %s", file_path);
            }
        } else {
            LOG(LOG_WARNING,"Error,file %s already exist", file_path);
        }
    }

    void set_data(const uint8_t * data, const size_t data_size) {
        if (this->fd.is_open()) {
            if ((this->current_file_size + data_size) <= this->total_file_size) {

                int written_data_size = ::write(this->fd.fd(), data, data_size);

                if (written_data_size == -1 && int(data_size) == written_data_size) {
                    LOG(LOG_WARNING,"File error, can't write into \"%s\" (received data size = %zu, written data size = %d)", this->dir_path, data_size, written_data_size);
                } else {
                    this->current_file_size += data_size;
                    if ( this->current_file_size == this->total_file_size) {
                        this->fd.close();
                    }
                }
            }
        }
    }

    bool is_complete() {
        return this->total_file_size == this->current_file_size;
    }

    ~ChannelFile() {
        this->fd.close();
    }

};
