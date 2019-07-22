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

#include "utils/log.hpp"


enum class ICAPFileId : uint32_t;

constexpr ICAPFileId invalid_icap_file_id = ICAPFileId(-1);

namespace LocalICAPProtocol
{

inline void emit_file_id(OutStream& stream, ICAPFileId file_id) noexcept
{
    stream.out_uint32_be(safe_int(file_id));
}

inline ICAPFileId recv_file_id(InStream& stream) noexcept
{
    return safe_int(stream.in_uint32_be());
}

enum class MsgType : uint8_t
{
    // Create a new file to analyse
    NewFile = 0x00,
    // Send data from a file
    DataFile = 0x01,
    // Close the session
    CloseSession = 0x02,
    // Indicates data file end
    Eof = 0x03,
    // Abort file.
    AbortFile = 0x04,
    // Result received from Validator
    Result = 0x05,
    // File validator check server icap result
    Check = 0x07,

    Unknown,
};

enum class ValidationType : uint8_t
{
    IsAccepted,
    IsRejected,
    Error,
    Wait,
};

enum class [[nodiscard]] ReceiveStatus : bool
{
    WaitingData,
    Ok,
};

struct ICAPHeader
{
    // This header starts every message receive from or emit to the local service.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |   msg_type    |          msg_size         |
    // +---------------+----------------------------

    MsgType msg_type;
    uint32_t msg_len;


    ICAPHeader() = default;

    ICAPHeader(MsgType msg_type, uint32_t msg_len) noexcept
    : msg_type(msg_type)
    , msg_len(msg_len)
    {}

    void emit(OutStream& stream) const noexcept
    {
        stream.out_uint8(safe_int(this->msg_type));
        stream.out_uint32_be(this->msg_len);
    }

    ReceiveStatus recv(InStream& stream) noexcept
    {
        if (stream.in_remain() < 5) {
            return ReceiveStatus::WaitingData;
        }

        this->msg_type = safe_int(stream.in_uint8());
        this->msg_len = stream.in_uint32_be();

        return ReceiveStatus::Ok;
    }
};


struct ICAPResultHeader
{
    ValidationType result = ValidationType::Wait;
    ICAPFileId file_id;
    uint32_t content_size;

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

    ReceiveStatus recv(InStream& stream) noexcept
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

    void emit(OutStream& stream) noexcept
    {
        stream.out_uint8(safe_int(this->result));
        stream.out_uint32_be(safe_int(this->file_id));
        stream.out_uint32_be(safe_int(this->content_size));
    }
};

struct ICAPCheck
{
    enum class IsUp : bool { Down, Up };

    IsUp service_is_up;
    uint32_t max_connections_number;

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

    ReceiveStatus recv(InStream& stream) noexcept
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
    constexpr std::ptrdiff_t max_pkt_len = 1024*64;

    auto p = data.begin();
    const auto pend = data.end();

    while (p != pend) {
        const auto pkt_len = std::min(pend-p, max_pkt_len);
        send_header(trans, file_id, MsgType::DataFile, pkt_len);
        trans.send({p, std::size_t(pkt_len)});
        p += pkt_len;
    }
}

} // namespace LocalICAPProtocol

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

    enum class [[nodiscard]] ResponseType : uint8_t
    {
        WaitingData,
        Error,
        HasPacket,
        HasContent,
    };

    ResponseType receive_response()
    {
        if (this->state <= State::WaitingData) {
            this->buf.read_from(this->trans);
            // hexdump_av(this->buf.av());
        }

        this->state = this->_receive_response();
        return this->state == State::ContinuationData
            ? ResponseType::WaitingData
            : ResponseType(underlying_cast(this->state)-1);
        // switch (this->state) {
        //     case State::Error:      return ResponseType::Error;
        //     case State::HasContent: return ResponseType::HasContent;
        //     case State::HasPacket:  return ResponseType::HasPacket;
        //     default:                return ResponseType::WaitingData;
        // }
    }

private:
    enum class [[nodiscard]] State : uint8_t
    {
        ContinuationData,
        WaitingData,

        Error,
        HasPacket,
        HasContent,
    };

    State _receive_response()
    {
        InStream in_stream(this->buf.av());

        auto shift_data = [&](State r){
            this->buf.advance(in_stream.get_offset());
            return r;
        };

        auto read_content = [&]{
            std::size_t remaining_buf = this->result_header.content_size - this->content.size();
            std::size_t remaining_message = std::min(in_stream.in_remain(), remaining_buf);
            auto message = in_stream.in_skip_bytes(remaining_message);
            this->content.append(char_ptr_cast(message.data()), message.size());

            if (this->content.size() < this->result_header.content_size) {
                return shift_data(State::ContinuationData);
            }

            return shift_data(State::HasContent);
        };

        using namespace LocalICAPProtocol;

        if (this->state == State::ContinuationData) {
            return read_content();
        }

        ICAPHeader header;
        if (header.recv(in_stream) != ReceiveStatus::Ok) {
            return State::WaitingData;
        }

        switch (header.msg_type)
        {
            case MsgType::Check:
                if (this->check.recv(in_stream) != ReceiveStatus::Ok) {
                    return State::WaitingData;
                }

                return shift_data(State::HasPacket);

            case MsgType::Result:
                if (this->result_header.recv(in_stream) != ReceiveStatus::Ok) {
                    return State::WaitingData;
                }

                this->content.clear();
                return read_content();

            default:
                LOG(LOG_ERR, "ICAPService::receive_response: Unknown packet: msg_type=%d",
                    int(header.msg_type));
                in_stream.in_skip_bytes(std::min<std::size_t>(
                    header.msg_len, in_stream.in_remain()));
                return shift_data(State::Error);
        }
    }

    ICAPFileId generate_id() noexcept
    {
        ++this->current_id;
        return ICAPFileId(this->current_id);
    }

    Transport& trans;
    LocalICAPProtocol::ICAPResultHeader result_header;
    LocalICAPProtocol::ICAPCheck check {};
    std::string content;
    int32_t current_id = 0;
    State state = State::WaitingData;

    BasicStaticBuffer<1024*16, uint16_t> buf;
};
