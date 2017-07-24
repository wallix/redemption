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
*   Copyright (C) Wallix 2013-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "transport/transport.hpp"
#include "utils/sugar/byte.hpp"
#include "utils/parse.hpp"
#include "cxx/cxx.hpp"
#include "x224.hpp"

#include <cstring>

class TpduBuffer
{
    enum class StateRead : bool
    {
        Header,
        Data,
    };

public:
    TpduBuffer() = default;

    void load_data(Transport & trans)
    {
        this->buf.read_data(trans);
    }

    bool next_pdu()
    {
        switch (this->state)
        {
            case StateRead::Header:
                this->buf.advance(this->pdu_len);
                this->pdu_len = 0;
                if (!this->read_header()) {
                    return false;
                }
                if (this->pdu_len <= this->buf.remaining()) {
                    this->check_pdu();
                    return true;
                }
                this->state = StateRead::Data;
                return false;

            case StateRead::Data:
                if (this->pdu_len <= this->buf.remaining()) {
                    this->check_pdu();
                    this->state = StateRead::Header;
                    return true;
                }
                return false;
        }
    }

    bool has_pdu() const noexcept
    {
        return bool(this->pdu_len);
    }

    array_view_u8 current_pdu_buffer() noexcept
    {
        return {this->buf.buffer(), this->pdu_len};
    }

    bool current_pdu_is_fast_path() const noexcept
    {
        assert(this->pdu_len);
        return this->fast_path;
    }

private:
    bool read_header()
    {
        // fast path header occupies 2 or 3 octets, but assume then data len at least 2 octets.
        if (this->buf.remaining() < 4) {
            return false;
        }

        array_view_u8 buffer(this->buf.buffer(), 4);

        switch (FastPath::FASTPATH_OUTPUT(buffer[0] & 0x03))
        {
            case FastPath::FASTPATH_OUTPUT_ACTION_FASTPATH:
            {
                this->pdu_len = buffer[1];
                if (this->pdu_len & 0x80){
                    this->pdu_len = (this->pdu_len & 0x7F) << 8 | buffer[2];
                    this->pdu_len -= 1;
                    this->buf.advance(1);
                }
                this->pdu_len -= 1;
                this->fast_path = true;
                this->buf.advance(2);
            }
            break;

            case FastPath::FASTPATH_OUTPUT_ACTION_X224:
            {
                this->pdu_len = Parse(this->buf.buffer()+2).in_uint16_be();
                if (this->pdu_len < 6) {
                    LOG(LOG_ERR, "Bad X224 header, length too short (length = %u)", this->pdu_len);
                    throw Error(ERR_X224);
                }
                this->pdu_len -= 4;
                this->fast_path = false;
                this->buf.advance(4);
            }
            break;

            default:
                LOG(LOG_ERR, "Bad X224 header, unknown TPKT version (%.2x)", buffer[0]);
                throw Error(ERR_X224);
        }

        return true;
    }

    void check_pdu()
    {
        if (!this->fast_path) {
            Parse data(this->buf.buffer());
            data.in_skip_bytes(1);
            uint8_t tpdu_type = data.in_uint8();
            switch (uint8_t(tpdu_type & 0xF0)) {
                case X224::CR_TPDU: // Connection Request 1110 xxxx
                case X224::CC_TPDU: // Connection Confirm 1101 xxxx
                case X224::DR_TPDU: // Disconnect Request 1000 0000
                case X224::DT_TPDU: // Data               1111 0000 (no ROA = No Ack)
                case X224::ER_TPDU: // TPDU Error         0111 0000
                    //this->type = tpdu_type & 0xF0;
                    break;
                default:
                    //this->type = 0;
                    LOG(LOG_ERR, "Bad X224 header, unknown TPDU type (code = %u)", tpdu_type);
                    throw Error(ERR_X224);
            }
        }
    }

    struct Buffer
    {
        static constexpr std::size_t max_len = uint16_t(~uint16_t{});
        uint8_t buf[max_len];
        uint16_t len = 0;
        uint16_t idx = 0;

        std::size_t remaining() const noexcept
        {
            return std::size_t(this->len - this->idx);
        }

        uint8_t* buffer() noexcept
        {
            return this->buf + this->idx;
        }

        void advance(std::size_t n) noexcept
        {
            assert(this->idx + n <= this->len);
            this->idx += n;
        }

        void read_data(Transport & trans)
        {
            if (this->idx == this->len) {
                this->len = trans.partial_read(this->buf, max_len);
                this->idx = 0;
            }
            else {
                if (this->idx) {
                    std::memmove(this->buf, this->buf + this->idx, this->remaining());
                    this->len -= this->idx;
                    this->idx = 0;
                }
                this->len += trans.partial_read(this->buf + this->len, max_len - this->len);
            }
        }
    };


    StateRead state = StateRead::Header;
    bool fast_path = false;
    uint16_t pdu_len = 0;
    Buffer buf;
};
