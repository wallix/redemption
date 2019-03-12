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
#include <unistd.h>

#include "core/error.hpp"
#include "utils/netutils.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"

#include "utils/log.hpp"



struct ICAPService
{

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

namespace LocalICAPServiceProtocol {

enum {
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
        NEW_FILE_FLAG      = 0x00,
        DATA_FILE_FLAG     = 0x01,
        CLOSE_SESSION_FLAG = 0x02
    };

    enum {
    // +--------------------+----------------------------------------------+
    // | Value              | Meaning                                      |
    // +--------------------+----------------------------------------------+
    // | ACCEPTED_FLAG      | File is valid                                |
    // | 0x00               |                                              |
    // +--------------------+----------------------------------------------+
    // | REJECTED_FLAG      | File is NOT valid                            |
    // | 0x01               |                                              |
    // +--------------------+----------------------------------------------+
    // | ERROR_FLAG         | File analysis error                          |
    // | 0x02               |                                              |
    // +--------------------+----------------------------------------------+
        ACCEPTED_FLAG = 0x00,
        REJECTED_FLAG = 0x01,
        ERROR_FLAG    = 0x02
    };


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

    // NewFileMessage

    // This message is sent to create a new file to request icap analyse
    // for. It begins with an ICAPHeader, its msg_type must be NEW_FILE_FLAG.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         File_id_size                          |
    // +---------------------------------------------------------------+
    // |                            File_id                            |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+
    // |                        File_name_size                         |
    // +---------------------------------------------------------------+
    // |                           File_Name                           |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+
    // |                           File_size                           |
    // +---------------------------------------------------------------+

    // File_id_size: An unsigned, 32-bit integer that indicate file id
    //               length.

    // File_id: A variable length, ascii string that contains new file id.

    // File_name_size: An unsigned, 32-bit integer that indicate file name
    //                 length.

    // File_Name: A variable length, ascii string that contains new file
    //            name.

    // File_size: An unsigned, 32-bit integer that indicate file length
    //            in bytes.

    ICAPNewFile(const std::string & file_id, const std::string & file_name, const size_t file_size)
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



struct ICAPFileDataHeader
{
    const std::string file_id;

    // FileDataMessage

    // This message contains data from a file. It begins with an ICAPHeader
    // its msg_type must be DATA_FILE_FLAG.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         File_id_size                          |
    // +---------------------------------------------------------------+
    // |                            File_id                            |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+
    // |                             DATA                              |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+

    // File_id_size: An unsigned, 32-bit integer that indicate file id
    //               length.

    // File_id: A variable length, ascii string that contains file id data
    // are coming from.

    // DATA: A binary, variable length, pay load data from a file .

    ICAPFileDataHeader(const std::string & file_id)
    : file_id(file_id) {}

    void emit(OutStream & stream) {

        stream.out_uint32_be(this->file_id.length());
        stream.out_string(this->file_id.c_str());
    }
};



struct ICAPResult {

    uint8_t result;
    std::string id;

    // ResultMessage

    // This message is send from the local service to sessions, it does NOT
    // need an ICAPHeader so.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |    Result     |                 File_id_size                  |
    // +---------------+-----------------------------------------------+
    // |               |                   File_id                     |
    // +---------------+-----------------------------------------------+
    // |                             ...                               |
    // +---------------------------------------------------------------+

    // Result: An unsigned, 8-bit integer that indicate result analysis. Value
    //         must be ACCEPTED_FLAG, REJECTED_FLAG or ERROR_FLAG.

    // +--------------------+----------------------------------------------+
    // | Value              | Meaning                                      |
    // +--------------------+----------------------------------------------+
    // | ACCEPTED_FLAG      | File is valid                                |
    // | 0x00               |                                              |
    // +--------------------+----------------------------------------------+
    // | REJECTED_FLAG      | File is NOT valid                            |
    // | 0x01               |                                              |
    // +--------------------+----------------------------------------------+
    // | ERROR_FLAG         | File analysis error                          |
    // | 0x02               |                                              |
    // +--------------------+----------------------------------------------+

    // File_id_size: An unsigned, 32-bit integer that indicate file id field
    //               length.

    // File_id: A variable length, ascii string that contains file id
    //          corresponding analysis result.

    ICAPResult()
    : result(LocalICAPServiceProtocol::ERROR_FLAG) {}

    void receive(InStream & stream) {
        const unsigned expected = 5;    /* Result(1) + File_id_size(4) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "ICAPResult truncated, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }
        this->result = stream.in_uint8();
        const unsigned id_len = stream.in_uint32_be();
        if (!stream.in_check_rem(id_len)) {
            LOG( LOG_INFO, "ICAPResult truncated, need=%u remains=%zu"
               , id_len, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->id = std::string(char_ptr_cast(stream.get_current()));
    }

};

}

ICAPService * icap_open_session(std::string & socket_path, std::string & session_id);
std::string icap_open_file(ICAPService * service, std::string & file_name, size_t file_size);
int icap_send_data(const ICAPService * service, const std::string & file_id, const char * data, const size_t size);
LocalICAPServiceProtocol::ICAPResult icap_get_result(const ICAPService * service);
int icap_close_session(const ICAPService * service);



ICAPService * icap_open_session(std::string & socket_path, std::string & session_id) {

    return new ICAPService(socket_path, session_id);
}

std::string icap_open_file(ICAPService * service, std::string & file_name, size_t file_size) {

    std::string file_id = "error";

    if (service->fd.is_open()) {
        file_id = service->generate_id();

        if (file_name.length() > 512) {
            file_name = file_name.substr(0, 512);
        }

        StaticOutStream<1024> message;

        LocalICAPServiceProtocol::ICAPHeader header(LocalICAPServiceProtocol::NEW_FILE_FLAG, 12+file_name.length()+file_id.length());
        header.emit(message);

        LocalICAPServiceProtocol::ICAPNewFile icap_new_file(file_id, file_name, file_size);
        icap_new_file.emit(message);

        int n = write(service->fd.fd(), message.get_data(), message.get_offset());

        if (size_t(n) != message.get_offset()) {
            return "error";
        }
    }

    return file_id;
}

int icap_send_data(const ICAPService * service, const std::string & file_id, const char * data, const size_t size) {

    int total_n = -1;

    if (service->fd.is_open()) {

        size_t data_len_to_send = size;
        InStream stream_data(data, size);

        while (data_len_to_send > 0) {

            size_t partial_data_size = data_len_to_send;

            if (data_len_to_send > 2048) {
                data_len_to_send -= 2048;
                partial_data_size = 2048;
            } else {
                data_len_to_send = 0;
            }

            StaticOutStream<2600> message;

            LocalICAPServiceProtocol::ICAPHeader header(LocalICAPServiceProtocol::DATA_FILE_FLAG, 4+file_id.length()+partial_data_size);
            header.emit(message);

            LocalICAPServiceProtocol::ICAPFileDataHeader file_data(file_id);
            file_data.emit(message);

            message.out_copy_bytes(cbytes_view(stream_data.get_current(), partial_data_size));
            stream_data.in_skip_bytes(partial_data_size);

            if (service->fd.is_open()) {
                int sent_data = write(service->fd.fd(), message.get_data(), message.get_offset());
                total_n += sent_data;

                usleep(1);
            }
        }
    }

    return total_n;
}

LocalICAPServiceProtocol::ICAPResult icap_get_result(const ICAPService * service) {

    LocalICAPServiceProtocol::ICAPResult result;
    int read_data_len = -1;

    if (service->fd.is_open()) {
        char buff[512] = {0};

        while (read_data_len < 0) {
            read_data_len = read(service->fd.fd(), buff, 512);
        }

        InStream stream_data(buff, read_data_len);

        result.receive(stream_data);
    }

    return result;
}

int icap_close_session(const ICAPService * service) {

    int n = -1;

    if (service->fd.is_open()) {
        StaticOutStream<8> message;

        LocalICAPServiceProtocol::ICAPHeader header(LocalICAPServiceProtocol::CLOSE_SESSION_FLAG, 1);
        header.emit(message);

        n = write(service->fd.fd(), message.get_data(), message.get_offset());
    }
    delete service;

    return n;
}


