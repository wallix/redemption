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

#include "core/buf64k.hpp"
#include "core/error.hpp"

#include "transport/transport.hpp"

#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <string>
#include <string_view>


enum class ICAPFileId : uint32_t;

constexpr ICAPFileId invalid_icap_file_id = ICAPFileId(-1);

namespace LocalICAPProtocol
{

inline void emit_file_id(OutStream& stream, ICAPFileId file_id) noexcept
{
    stream.out_uint32_be(safe_int(file_id));
}

enum class MsgType : uint8_t
{
    // Create a new file to analyse
    NewFile,
    // Send data from a file
    DataFile,
    // Close the session
    CloseSession,
    // Indicates data file end
    Eof,
    // Abort file.
    AbortFile,
    // Result received from Validator
    Result,
    // File validator check server icap result
    Check,

    Error = 42
};

enum class ValidationType
{
    IsAccepted,
    IsRejected,
    Error,
};

struct ICAPHeader
{
    // This header starts every message receive from or emit to the local service.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |   msg_type    |                  msg_size                     |
    // +---------------+-----------------------------------------------+
    // |               |
    // +---------------+

    MsgType msg_type = MsgType::Error;
    uint32_t msg_len = 0;


    ICAPHeader() = default;

    ICAPHeader(MsgType msg_type, uint32_t msg_len) noexcept
    : msg_type(msg_type)
    , msg_len(msg_len)
    {}

    void emit(OutStream& stream) noexcept
    {
        stream.out_uint8(safe_int(this->msg_type));
        stream.out_uint32_be(this->msg_len);
    }
};

enum class [[nodiscard]] ReceiveStatus : bool
{
    WaitingData,
    Ok,
};

struct ICAPResultHeader
{
    ValidationType result = ValidationType::Error;
    ICAPFileId file_id;
    size_t content_size;

    // ResultMessage

    // This message is send from the local service to sessions, it does NOT
    // need an ICAPHeader so.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |    Result     |                local_File_id                  |
    // +---------------+-----------------------------------------------+
    // |               |               content_length                  |
    // +---------------+-----------------------------------------------+
    // |               |                  content                      |
    // +---------------+-----------------------------------------------+
    // |                             ...                               |
    // +---------------------------------------------------------------+

    // Result: An unsigned, 8-bit integer that indicate result analysis. Value
    //         must be ACCEPTED_FLAG, REJECTED_FLAG or Error_FLAG.

    //   +--------------------+----------------------------------------------+
    //   | Value              | Meaning                                      |
    //   +--------------------+----------------------------------------------+
    //   | ACCEPTED_FLAG      | File is valid                                |
    //   | 0x00               |                                              |
    //   +--------------------+----------------------------------------------+
    //   | REJECTED_FLAG      | File is NOT valid                            |
    //   | 0x01               |                                              |
    //   +--------------------+----------------------------------------------+
    //   | Error_FLAG         | File analysis error                          |
    //   | 0x02               |                                              |
    //   +--------------------+----------------------------------------------+

    // local_File_id: An unsigned, 32-bit integer that contains client local file
    //                id sent to the validator.

    // content_length: An unsigned, 32-bit integer that contains result content
    //                 length.

    // content: A variable length, ascii string that contains analysis result.

    ReceiveStatus receive(InStream& stream) noexcept
    {
        /* Result(1) + File_id_size(4) + content_size(4)  */
        if (stream.in_remain() < 9) {
            return ReceiveStatus::WaitingData;
        }

        this->result = safe_int(stream.in_uint8());
        this->file_id = safe_int(stream.in_uint32_be());
        this->content_size = stream.in_uint32_be();

        return ReceiveStatus::Ok;
    }
};

struct ICAPCheck
{
    enum class IsUp : bool { Down, Up };

    IsUp service_is_up;
    int max_connections_number;

    //     Check message

    //     This message is send from the local service to sessions. It begins with an
    //     ICAPHeader, its msg_type must be Result_FLAG.

    //     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //     | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    //     |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    //     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //     |    up_flag    |            max_connections_number             |
    //     +---------------+-----------------------------------------------+
    //     |               |
    //     +---------------+

    //     up_flag: An unsigned, 8-bit integer that contains flag SERVICE_UP_FLAG
    //              or SERVICE_DOWN_FLAG

    //     max_connections_number: An unsigned, 32-bit integer that contains the
    //                             maximum number of connection icap service support

    ReceiveStatus receive(InStream& stream) noexcept
    {
        /* up_flag(1) + max_connections_number(4) */
        if (stream.in_remain() < 5) {
            return ReceiveStatus::WaitingData;
        }

        this->service_is_up = IsUp(bool(stream.in_uint8()));
        this->max_connections_number = stream.in_uint32_be();

        return ReceiveStatus::Ok;
    }
};

inline ICAPFileId send_open_file(
    OutTransport trans, ICAPFileId file_id, std::string_view file_name, std::string_view target_name)
{
    file_name = std::string_view(file_name.data(), std::min(std::size_t(512), file_name.size()));

    StaticOutStream<1024> message;

    ICAPHeader(MsgType::NewFile, 4u + 8u + file_name.size() + target_name.size())
    .emit(message);

    emit_file_id(message, file_id);

    message.out_uint32_be(file_name.size());
    message.out_copy_bytes(file_name);

    message.out_uint32_be(target_name.size());
    message.out_copy_bytes(target_name);

    trans.send(message.get_bytes());

    return file_id;
}

inline void send_header(
    OutTransport trans, ICAPFileId file_id, MsgType msg_type, uint32_t pkt_len = 0)
{
    StaticOutStream<16> message;

    ICAPHeader(msg_type, 4u + pkt_len)
    .emit(message);

    emit_file_id(message, file_id);

    trans.send(message.get_bytes());
}

inline void send_data_file(OutTransport trans, ICAPFileId file_id, const_bytes_view data)
{
    // TODO 1024 ????
    constexpr std::ptrdiff_t max_pkt_len = 1024;

    auto p = data.begin();
    const auto pend = data.end();

    while (p != pend)
    {
        const auto pkt_len = std::min(pend-p, max_pkt_len);
        send_header(trans, file_id, MsgType::DataFile, pkt_len);
        trans.send(p, pkt_len);
        p += pkt_len;
    }
}

}

struct ICAPService
{
    ICAPService(Transport& trans) noexcept
    : trans(trans)
    {}

    bool service_is_up() const noexcept
    {
        return this->check.service_is_up == LocalICAPProtocol::ICAPCheck::IsUp::Up;
    }

    ICAPFileId open_file(std::string_view file_name, std::string_view target_name)
    {
        return LocalICAPProtocol::send_open_file(this->trans, this->generate_id(), file_name, target_name);
    }

    void send_data(ICAPFileId file_id, const_bytes_view data) const
    {
        LocalICAPProtocol::send_data_file(this->trans, file_id, data);
    }

    void send_eof(ICAPFileId file_id) const
    {
        using namespace LocalICAPProtocol;
        send_header(this->trans, file_id, MsgType::Eof);
    }

    void send_abort(ICAPFileId file_id) const
    {
        using namespace LocalICAPProtocol;
        send_header(this->trans, file_id, MsgType::AbortFile);
    }

    void send_close_session() const
    {
        using namespace LocalICAPProtocol;
        send_header(this->trans, ICAPFileId(), MsgType::CloseSession);
    }

    enum class [[nodiscard]] ResponseType : uint8_t
    {
        Error,
        WaitingData,
        Initialized,
        Content,
    };

    ResponseType receive_response()
    {
        if (this->response_type == ResponseType::WaitingData)
        {
            this->buf.read_from(this->trans);
        }
        this->response_type = this->_receive_response();
        return this->response_type;
    }

private:
    ResponseType _receive_response()
    {
        InStream in_stream(this->buf.av());

        auto shift_data = [&](ResponseType r){
            this->buf.advance(in_stream.get_offset());
            return r;
        };

        switch (this->state)
        {
            case State::InitHeader: {
                switch (check.receive(in_stream))
                {
                    case LocalICAPProtocol::ReceiveStatus::WaitingData:
                        return ResponseType::WaitingData;
                    case LocalICAPProtocol::ReceiveStatus::Ok:
                        this->state = State::StartMessage;
                        return shift_data(ResponseType::Initialized);
                }
            }

            case State::StartMessage: {
                this->content.clear();
                switch (this->result_header.receive(in_stream))
                {
                    case LocalICAPProtocol::ReceiveStatus::WaitingData:
                        return ResponseType::WaitingData;
                    case LocalICAPProtocol::ReceiveStatus::Ok:
                        break;
                }

                switch (this->result_header.result)
                {
                    case LocalICAPProtocol::ValidationType::Error:
                        return ResponseType::Error;
                    case LocalICAPProtocol::ValidationType::IsRejected:
                    case LocalICAPProtocol::ValidationType::IsAccepted:
                        this->state = State::FragmentMessage;
                        break;
                }
                [[fallthrough]];
            }

            case State::FragmentMessage: {
                std::size_t remaining_buf = this->result_header.content_size - this->content.size();
                std::size_t remaining_message = std::min(in_stream.in_remain(), remaining_buf);
                auto message = in_stream.in_skip_bytes(remaining_message);
                this->content.append(char_ptr_cast(message.data()), message.size());

                if (this->content.size() == this->result_header.content_size)
                {
                    this->state = State::StartMessage;
                    return shift_data(ResponseType::Content);
                }

                return shift_data(ResponseType::WaitingData);
            }

            default: REDEMPTION_UNREACHABLE();
        }
    }

public:
    std::string const& get_content() const noexcept
    {
        return this->content;
    }

    ICAPFileId last_file_id() const noexcept
    {
        return this->result_header.file_id;
    }

    LocalICAPProtocol::ValidationType last_result_flag() const noexcept
    {
        return this->result_header.result;
    }

    ResponseType last_response_type() const noexcept
    {
        return this->response_type;
    }

private:
    ICAPFileId generate_id() noexcept
    {
        ++this->current_id;
        return ICAPFileId(this->current_id);
    }


    enum class State : uint8_t
    {
        InitHeader,
        StartMessage,
        FragmentMessage,
    };

    Transport& trans;
    LocalICAPProtocol::ICAPResultHeader result_header;
    LocalICAPProtocol::ICAPCheck check {};
    std::string content;
    int32_t current_id = 0;
    State state = State::InitHeader;
    ResponseType response_type = ResponseType::WaitingData;

    BasicStaticBuffer<512, uint16_t> buf;
};
