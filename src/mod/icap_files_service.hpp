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
*   Copyright (C) Wallix 2019
*   Author(s): Clément Moroldo
*/

#pragma once



#include <string>
#include <stdio.h>
#include <stdlib.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netdb.h>
#include "utils/netutils.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"

#include <unistd.h>
#include "utils/log.hpp"


struct ICAPService
{
    enum {
        NEW_FILE_FLAG      = 0x00,
        DATA_FILE_FLAG     = 0x01,
        CLOSE_SESSION_FLAG = 0x02
    };

    unique_fd fd;
    std::string session_id;

public:
    int file_id_int;

    ICAPService(std::string & socket_path,
                std::string & session_id)
    : fd( local_connect(socket_path.c_str(), 3, 1000))
    , session_id(session_id)
    , file_id_int(0)
    {}

    std::string generate_id() {
        this->file_id_int++;
        return this->session_id+"-"+std::to_string(this->file_id_int);
    }
};

ICAPService * icap_open_session(std::string & socket_path, std::string & session_id);
std::string icap_open_file(ICAPService * service, std::string & file_name, size_t file_size);
int icap_send_data(const ICAPService * service, const std::string & file_id, const char * data, const size_t size);

struct ICAPResult {
    int res= 0;
    std::string id;
};
ICAPResult icap_get_result(const ICAPService * service);
int icap_close_session(const ICAPService * service);



struct ICAPHeader {
    const uint8_t msg_type;
    const uint32_t msg_len;

    // HeaderMessage

    // This header starts every message receive from sessions to the local service.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |   msg_type    |                  msg_size                     |
    // +---------------+-----------------------------------------------+
    // |               |
    // +---------------+

    // msg_type: An unsigned, 8-bit integer that indicate message type. Value must be
    //           NEW_FILE_FLAG, DATA_FILE_FLAG or CLOSE_SESSION_FLAG.

    // +--------------------+-------------------------------------------------+
    // | Value              | Meaning                                         |
    // +--------------------+-------------------------------------------------+
    // | NEW_FILE_FLAG      | Create a new file to analyse.                   |
    // | 0x00               |                                                 |
    // +--------------------+-------------------------------------------------+
    // | DATA_FILE_FLAG     | Send data from a file.                          |
    // | 0x01               |                                                 |
    // +--------------------+-------------------------------------------------+
    // | CLOSE_SESSION_FLAG | Close the session.                              |
    // | 0x02               |                                                 |
    // +--------------------+-------------------------------------------------+

    // msg_size: An unsigned, 32-bit integer that indicate length of following message
    //           data.

    ICAPHeader(const uint8_t msg_type, const uint32_t msg_len)
    : msg_type(msg_type)
    , msg_len(msg_len) {}

    void emit(OutStream & stream) {
        stream.out_uint8(msg_type);
        stream.out_uint32_be(msg_len);
    }
};

struct ICAPNewFile {

    const std::string file_id;
    const std::string file_name;
    const size_t file_size;

    ICAPNewFile(std::string & file_id, std::string & file_name, const size_t file_size)
    : file_id(file_id)
    , file_name(file_name)
    , file_size(file_size) {}

    void emit(OutStream & stream) {

        stream.out_uint32_be(this->file_id.length());
        stream.out_string(this->file_id.c_str());

        stream.out_uint32_be(this->file_name.length());
        stream.out_string(this->file_name.c_str());

        stream.out_uint32_be(this->file_size);
    }

};


ICAPService * icap_open_session(std::string & socket_path, std::string & session_id) {

    return new ICAPService(socket_path, session_id);
}


std::string icap_open_file(ICAPService * service, std::string & file_name, size_t file_size) {

    std::string file_id = service->generate_id();

    if (file_name.length() > 512) {
        file_name = file_name.substr(0, 512);
    }

    StaticOutStream<1024> message;

    ICAPHeader header(ICAPService::NEW_FILE_FLAG, 12+file_name.length()+file_id.length());
    header.emit(message);

    ICAPNewFile icap_new_file(file_id, file_name, file_size);
    icap_new_file.emit(message);

    int n = write(service->fd.fd(), message.get_data(), message.get_offset());

    if (size_t(n) != message.get_offset()) {
        return "error";
    }

    return file_id;
}

int icap_send_data(const ICAPService * service, const std::string & file_id, const char * data, const size_t size) {

    int total_n = 0;

    size_t data_len_to_send = size;
    InStream stream_data(data, size);
//     LOG(LOG_INFO, "Total data to send %zu", size);
    while (data_len_to_send > 0) {

        size_t partial_data_size = data_len_to_send;

        if (data_len_to_send > 2048) {
            data_len_to_send -= 2048;
            partial_data_size = 2048;
        } else {
            data_len_to_send = 0;
        }

        StaticOutStream<2600> message;

        ICAPHeader header(ICAPService::DATA_FILE_FLAG, 4+file_id.length()+partial_data_size);
        header.emit(message);

        message.out_uint32_be(file_id.length());
        message.out_string(file_id.c_str());

        message.out_copy_bytes(cbytes_view(stream_data.get_current(), partial_data_size));

        stream_data.in_skip_bytes(partial_data_size);

        if (service->fd.is_open()) {
            int sent_data = write(service->fd.fd(), message.get_data(), message.get_offset());
            total_n += sent_data;

            usleep(1);
        }
    }

    return total_n;
}



ICAPResult icap_get_result(const ICAPService * service) {

    int read_data_len = -1;

    char buff[512] = {0};

    while (read_data_len < 0) {
        read_data_len = read(service->fd.fd(), buff, 512);
    }

    InStream stream_data(buff, read_data_len);

    int res = stream_data.in_uint8();
    int id_len = stream_data.in_uint32_be();

    std::string id(char_ptr_cast(stream_data.get_current()));

    LOG(LOG_INFO, "read_data_len=%d res=%d id_len=%d id=\"%s\"", read_data_len, res, id_len, id);

    ICAPResult result;
    result.res = res;
    result.id = id;

    return result;
}

int icap_close_session(const ICAPService * service) {
    //std::string message("20\r\n");
    StaticOutStream<8> message;
    message.out_uint8(ICAPService::CLOSE_SESSION_FLAG);
    message.out_uint8(0);
    message.out_uint8(0);
    message.out_uint8(0);
    message.out_uint8(1);
    message.out_uint8(0);
    int n = write(service->fd.fd(), message.get_data(), message.get_offset());

    delete service;

    return n;
}


