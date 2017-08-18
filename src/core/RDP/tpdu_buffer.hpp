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

#include "core/RDP/x224.hpp"
#include "core/buf64k.hpp"
#include "transport/transport.hpp"
#include "utils/parse.hpp"

#include <cstring>

namespace Extractors
{
    struct HeaderResult
    {
        static HeaderResult fail() noexcept
        {
            return HeaderResult{};
        }

        static HeaderResult ok(uint16_t n) noexcept
        {
            return HeaderResult{n};
        }

        explicit operator bool () const noexcept
        {
            return this->is_extracted;
        }

        uint16_t data_size() const noexcept
        {
            return this->len;
        }

    private:
        HeaderResult() noexcept
          : is_extracted(false)
        {}

        HeaderResult(uint16_t len) noexcept
          : is_extracted(true)
          , len(len)
        {}

        bool is_extracted;
        uint16_t len;
    };

    struct X224Extractor
    {
        HeaderResult read_header(Buf64k & buf)
        {
            // fast path header occupies 2 or 3 octets, but assume then data len at least 2 octets.
            if (buf.remaining() < 4)
            {
                return HeaderResult::fail();
            }

            array_view_u8 av = buf.av(4);
            uint16_t len;

            switch (FastPath::FASTPATH_OUTPUT(av[0] & 0x03))
            {
                case FastPath::FASTPATH_OUTPUT_ACTION_FASTPATH:
                {
                    len = av[1];
                    if (len & 0x80){
                        len = (len & 0x7F) << 8 | av[2];
                        // len -= 1;
                        // buf.advance(1);
                    }
                    // len -= 1;
                    // buf.advance(2);
                    this->has_fast_path = true;
                }
                break;

                case FastPath::FASTPATH_OUTPUT_ACTION_X224:
                {
                    len = Parse(av.subarray(2, 2).data()).in_uint16_be();
                    if (len < 6) {
                        LOG(LOG_ERR, "Bad X224 header, length too short (length = %u)", len);
                        throw Error(ERR_X224);
                    }
                    // len -= 4;
                    // buf.advance(4);
                    this->has_fast_path = false;
                }
                break;

                default:
                    LOG(LOG_ERR, "Bad X224 header, unknown TPKT version (%.2x)", av[0]);
                    throw Error(ERR_X224);
            }

            return HeaderResult::ok(len);
        }

        void prepare_data(Buf64k const & buf)
        {
            if (!this->has_fast_path) {
                uint8_t tpdu_type = Parse(buf.sub(5, 1).data()).in_uint8();
                switch (uint8_t(tpdu_type & 0xF0)) {
                    case X224::CR_TPDU: // Connection Request 1110 xxxx
                    case X224::CC_TPDU: // Connection Confirm 1101 xxxx
                    case X224::DR_TPDU: // Disconnect Request 1000 0000
                    case X224::DT_TPDU: // Data               1111 0000 (no ROA = No Ack)
                    case X224::ER_TPDU: // TPDU Error         0111 0000
                        this->type = tpdu_type & 0xF0;
                        break;
                    default:
                        this->type = 0;
                        LOG(LOG_ERR, "Bad X224 header, unknown TPDU type (code = %u)", tpdu_type);
                        throw Error(ERR_X224);
                }
            }
        }

        bool is_fast_path() const noexcept
        {
            return this->has_fast_path;
        }

        uint8_t get_type() const noexcept
        {
            assert(this->is_fast_path());
            return this->type;
        }

    private:
        bool has_fast_path;
        uint8_t type;
    };


    struct CreedsppExtractor
    {
        HeaderResult read_header(Buf64k & buf)
        {
            if (buf.remaining() < 4)
            {
                return HeaderResult::fail();
            }

            array_view_u8 av = buf.av(4);

            /**/ if (av[1] <= 0x7F) { return HeaderResult::ok(av[1] + 2); }
            else if (av[1] == 0x81) { return HeaderResult::ok(av[2] + 3); }
            else if (av[1] == 0x82) { return HeaderResult::ok(((av[2] << 8) | av[3]) + 4); }
            else                    { throw Error(ERR_NEGO_INCONSISTENT_FLAGS); }
        }

        void prepare_data(Buf64k const &) const
        {}
    };
}


struct TpduBuffer
{
    TpduBuffer() = default;

    void load_data(InTransport trans)
    {
        this->buf.read_from(trans);
    }

    uint16_t remaining() const noexcept
    {
        return this->buf.remaining();
    }

    bool next_pdu()
    {
        return this->extract(this->extractors.x224);
    }

    bool next_credssp()
    {
        return this->extract(this->extractors.credssp);
    }

    array_view_u8 current_pdu_buffer() noexcept
    {
        assert(this->pdu_len);
        return this->buf.av(this->pdu_len);
    }

    bool current_pdu_is_fast_path() const noexcept
    {
        assert(this->pdu_len);
        return this->extractors.x224.is_fast_path();
    }

    uint8_t current_pdu_get_type() const noexcept
    {
        assert(this->pdu_len);
        return this->extractors.x224.get_type();
    }

private:
    enum class StateRead : bool
    {
        Header,
        Data,
    };

    struct Extractor // Extractor concept
    {
        Extractors::HeaderResult read_header(Buf64k& buf);
        void check_data(Buf64k const &) const;
    };

    template<class Extractor>
    bool extract(Extractor & extractor)
    {
        switch (this->state)
        {
            case StateRead::Header:
                this->buf.advance(this->pdu_len);
                this->pdu_len = 0;
                if (auto r = extractor.read_header(this->buf))
                {
                    this->pdu_len = r.data_size();
                    if (this->pdu_len <= this->buf.remaining())
                    {
                        extractor.prepare_data(this->buf);
                        return true;
                    }
                    this->state = StateRead::Data;
                }
                return false;

            case StateRead::Data:
                if (this->pdu_len <= this->buf.remaining())
                {
                    this->state = StateRead::Header;
                    extractor.prepare_data(this->buf);
                    return true;
                }
                return false;
        }
    }

    StateRead state = StateRead::Header;
    union U
    {
        Extractors::X224Extractor x224;
        Extractors::CreedsppExtractor credssp;

        char dummy;
        U():dummy() {}
    } extractors;
    uint16_t pdu_len = 0;
    Buf64k buf;
};
