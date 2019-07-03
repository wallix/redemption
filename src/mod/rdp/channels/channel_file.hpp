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

#include "utils/sugar/cast.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/difftimeval.hpp"
#include "utils/texttime.hpp"
#include "utils/log.hpp"

#include "mod/icap_files_service.hpp"
#include "mod/rdp/channels/validator_params.hpp"

#include <unistd.h>


class ChannelFile
{
public:
    enum class Direction : uint8_t
    {
        None,
        FromServer,
        FromClient,
    };

private:
    std::string dir_path;
    std::string filename;
    std::string file_path;
    unique_fd fd = invalid_fd();
    size_t total_file_size = 0;
    size_t current_file_size = 0;

    ICAPFileId streamID {};

    uint8_t direction = NONE;

    bool is_interupting_channel;
    bool is_saving_files;

    ICAPService * icap_service;

    bool current_analysis_done = false;

    const std::string target_name;

public:

    enum : uint8_t {
        NONE,
        FILE_FROM_SERVER,
        FILE_FROM_CLIENT
    };

    ChannelFile(ICAPService * icap_service, ValidatorParams const& validator_params) noexcept
    : dir_path(validator_params.save_files_directory)
    , is_interupting_channel(validator_params.enable_interupting)
    , is_saving_files(validator_params.enable_save_files)
    , icap_service(icap_service)
    , target_name(validator_params.target_name)
    {}

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

            this->file_path = str_concat(this->dir_path, get_full_text_sec_and_usec(tv), '_', this->filename);

            LOG(LOG_INFO, "this->file_path = %s", this->file_path);

            this->fd = unique_fd(this->file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
            if (!this->fd.is_open()) {
                LOG(LOG_WARNING,"File error, can't open %s", file_path);
            }
        }

        if (this->icap_service) {
            if (bool(this->streamID) && !this->current_analysis_done) {
                this->icap_service->send_abort(this->streamID);
            }
            this->current_analysis_done = false;
            this->streamID = this->icap_service->open_file(filename, this->target_name);
        }
    }

    void set_data(const_bytes_view data) {

        size_t new_size = this->current_file_size + data.size();
        size_t over_data_len = 0;
        if (new_size > this->total_file_size) {
            over_data_len = new_size - this->total_file_size;
            LOG(LOG_INFO, "ChanneFile::set_data over_data_len=%zu data_size=%zu ", over_data_len, data.size());
        } else {
            this->current_file_size = new_size;
        }

        if (this->is_saving_files) {
            if (this->fd.is_open()) {

                int written_data_size = ::write(this->fd.fd(), data.data(), (data.size() - over_data_len));

                if (written_data_size == -1 && (int(data.size() - over_data_len) == written_data_size)) {
                    LOG(LOG_WARNING,"File error, can't write into \"%s\" (received data size = %zu, written data size = %d)", this->dir_path, data.size(), written_data_size);
                } else {
                    //this->current_file_size += data_size;
                    if ( this->current_file_size == this->total_file_size) {
                        this->fd.close();
                    }
                }

            }
        }

        if (this->icap_service) {
            this->icap_service->send_data(this->streamID, data.first(data.size() - over_data_len));
        }
    }

    void set_end_of_file() {
        if (this->icap_service) {
            this->icap_service->send_eof(this->streamID);
        }
    }

    void read_data(uint8_t * buffer, const size_t data_len) {
        if (this->is_saving_files) {
            this->fd = unique_fd(this->file_path.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

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
    }

    // TODO string const&
    std::string get_result_content() const noexcept
    {
        if (this->icap_service) {
            return this->icap_service->get_content();
        }
        return std::string();
    }

    bool receive_response()
    {
        if (this->icap_service) {
            // TODO loop ?
            auto r = this->icap_service->receive_response();
            this->current_analysis_done = true;
            // LOG(LOG_INFO, "%d %s", this->icap_service->result, this->icap_service->content);
            return r == ICAPService::ResponseType::Content;
        }
        return true;
    }

    uint8_t get_direction() const noexcept
    {
        return this->direction;
    }

    size_t get_file_size() const noexcept
    {
        return this->total_file_size;
    }

    ICAPFileId get_streamID() const noexcept
    {
        return this->streamID;
    }

    bool is_complete() const noexcept
    {
        return this->total_file_size == this->current_file_size;
    }

    bool is_valid() const noexcept
    {
        if (this->icap_service == nullptr) {
            return !this->icap_service;
        }
        return (this->icap_service->last_result_flag() == LocalICAPProtocol::ValidationType::IsAccepted);
    }

    bool is_enable_interuption() const noexcept
    {
        return this->is_interupting_channel;
    }

    std::string const& get_file_name() const noexcept
    {
        return this->filename;
    }

    bool is_save_files() const noexcept
    {
        return this->is_saving_files;
    }

    bool is_enable_validation() const noexcept
    {
        return this->icap_service;
    }
};
