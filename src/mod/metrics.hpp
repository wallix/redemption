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
#include "utils/sugar/array_view.hpp"

#include <chrono>
#include <string>
#include <vector>
#include <ctime>


bool create_metrics_directory(const std::string & path);

class Metrics
{
public:
    Metrics( std::string const& path
           , std::string session_id
           , array_view_const_char primary_user_sig     // hashed primary user account
           , array_view_const_char account_sig          // hashed secondary account
           , array_view_const_char target_service_sig   // hashed (target service name + device name)
           , array_view_const_char session_info_sig     // hashed (source_host + client info)
           , timeval now                                // time at beginning of metrics
           , std::chrono::hours file_interval           // daily rotation of filename
           , std::chrono::seconds log_delay             // delay between 2 logs flush
           );

    void set_protocol(std::string fields_version, std::string protocol_name, size_t nb_metric_item);

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

    void log(timeval now);

    void disconnect();

    void rotate(timeval now);

private:
    std::vector<uint64_t> current_data;

    std::string version;
    std::string protocol_name;

    // output file info
    const std::chrono::hours file_interval;
    timeval current_file_date;
    const std::string path;
    unique_fd fd = invalid_fd();

    // LOG info
    struct Header
    {
        char buffer[1024];
        size_t len;
    };
    Header header;
    const std::string session_id;
    const timeval connection_time;

    const std::chrono::seconds log_delay;
    timeval next_log_time;
    std::string complete_metrics_file_path;
    std::string complete_index_file_path;

    class Impl;
};
