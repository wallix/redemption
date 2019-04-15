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
    Copyright (C) Wallix 2019
    Author(s): Clément Moroldo
*/

#pragma once

#include <fstream>
#include <unistd.h>

#include "utils/sugar/cast.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/difftimeval.hpp"
#include "utils/texttime.hpp"

#include "mod/icap_files_service.hpp"


class ChannelFile {

private:
    std::string dir_path;
    std::string filename;
    std::string file_path;
    unique_fd fd = invalid_fd();
    size_t total_file_size = 0;
    size_t current_file_size = 0;

    uint32_t streamID = 0;

    bool valid = false;

    uint8_t direction = NONE;

    bool is_interupting_channel;
    bool is_saving_files;

    ICAPService * icap_service;

public:

    enum : uint8_t {
        NONE,
        FILE_FROM_SERVER,
        FILE_FROM_CLIENT
    };

    ChannelFile(std::string dir_path
    , bool is_interupting_channel
    , bool is_saving_files
    , ICAPService * icap_service) noexcept
        : streamID(0)
        , is_interupting_channel(is_interupting_channel)
        , is_saving_files(is_saving_files)
        , icap_service(icap_service)
    {
        if (this->is_saving_files) {
            if (!dir_path.empty()) {
                this->fd = unique_fd(std::move(dir_path), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
            }
        }
    }

//     void operator=(const ChannelFile & channel_file) {
//         this->dir_path = channel_file.dir_path;
//         this->total_file_size = channel_file.total_file_size;
//         this->current_file_size = 0;
//         this->valid = false;
//
//         std::string file_path = this->dir_path + this->filename;
//         std::ifstream file(file_path.c_str());
//         if (!file.good()) {
//             this->fd = unique_fd(file_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
//             if (!this->fd.is_open()) {
//                 LOG(LOG_WARNING,"File error, can't open %s", file_path);
//             }
//         } else {
//             LOG(LOG_WARNING,"Error,file %s already exist", file_path);
//         }
//     }
//
//     ChannelFile(const std::string & dir_path, const std::string & filename)
//     : dir_path(dir_path)
//     , filename(filename)
//     , fd(invalid_fd())
//     , total_file_size(0)
//     , current_file_size(0)
//     , valid(false)
//     {
//         std::string file_path = this->dir_path + filename;
//         std::ifstream file(file_path.c_str());
//         if (!file.good()) {
//             this->fd = unique_fd(file_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
//             if (!this->fd.is_open()) {
//                 LOG(LOG_WARNING,"File error, can't open %s", file_path);
//             }
//         } else {
//             LOG(LOG_WARNING,"Error,file %s already exist", file_path);
//         }
//     }

    void set_total_file_size(const size_t total_file_size) {
        this->total_file_size = total_file_size;
    }

    void new_file(const std::string & filename, const size_t total_file_size, const uint8_t direction, const timeval tv) {
        this->direction = direction;
        this->total_file_size = total_file_size;
        this->filename = filename;
        this->current_file_size = 0;
        this->fd.close();

        if (this->is_saving_files) {
            this->file_path = this->dir_path + get_full_text_sec_and_usec(tv) + "_" + this->filename;
            std::ifstream file(this->file_path.c_str());
            if (!file.good()) {
                this->fd = unique_fd(this->file_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
                if (!this->fd.is_open()) {
                    LOG(LOG_WARNING,"File error, can't open %s", file_path);
                }
            } else {
                LOG(LOG_WARNING,"Error,file %s already exist", file_path);
            }
        }

        this->streamID = icap_open_file(this->icap_service, filename.c_str());
    }

    void set_data(const uint8_t * data, const size_t data_size) {
        if (this->is_saving_files) {
            if (this->fd.is_open()) {
                size_t new_size = this->current_file_size + data_size;
                size_t over_data_len = 0;
                if (new_size > this->total_file_size) {
                    over_data_len = new_size - this->total_file_size;
                }
                int written_data_size = ::write(this->fd.fd(), data, (data_size - over_data_len));

                if (written_data_size == -1 && (int(data_size) == written_data_size)) {
                    LOG(LOG_WARNING,"File error, can't write into \"%s\" (received data size = %zu, written data size = %d)", this->dir_path, data_size, written_data_size);
                } else {
                    this->current_file_size += (data_size - over_data_len);
                    if ( this->current_file_size == this->total_file_size) {
                        this->fd.close();
                    }
                }
            }
        }

        icap_send_data(this->icap_service, this->streamID, char_ptr_cast(data), data_size);

//         if ( this->current_file_size >= this->total_file_size) {
//             icap_end_of_file(this->icap_service, this->streamID);
//         }
    }

    void set_end_of_file() {
        icap_end_of_file(this->icap_service, this->streamID);
    }

    void read_data(uint8_t * buffer, const size_t data_len) {
        this->fd = unique_fd(this->file_path.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
//         std::ifstream file(this->file_path.c_str());
        if (this->fd.is_open()) {

            size_t read_size = ::read(fd.fd(), buffer, data_len);

            if (read_size != data_len) {
                LOG(LOG_WARNING,"Read error, can't read data (%zu/%zu) in %s ", read_size, data_len, this->file_path);
            }
            this->fd.close();
        } else {
            LOG(LOG_WARNING,"File error, can't open %s", this->file_path);
        }
    }

    std::string get_result_content() {
        return this->icap_service->content;
    }

    void receive_result() {
        icap_receive_result(this->icap_service);
        LOG(LOG_INFO, "%s", this->icap_service->content);
    }

    uint8_t get_direction() {
        return this->direction;
    }

    size_t get_file_size() {
        return this->total_file_size;
    }

    uint32_t get_streamID() {
        return this->streamID;
    }

    bool is_complete() {
        return this->total_file_size == this->current_file_size;
    }

    bool is_valid() {
        this->valid = true;
        return this->is_complete() && this->valid;
    }

    ~ChannelFile() {
        this->fd.close();
    }

    bool is_interupting() {
        return this->is_interupting_channel;
    }

};
