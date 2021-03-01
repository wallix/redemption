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
*   Copyright (C) Wallix 2013-2019
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "core/buf64k.hpp"
#include "utils/hexdump.hpp"
#include "transport/transport.hpp"

#include "core/RDP/x224.hpp"
#include "utils/parse.hpp"

namespace Extractors
{
    enum {
        FASTPATH = 1,
        CR_TPDU  = X224::CR_TPDU, // Connection Request 1110 xxxx
        CC_TPDU  = X224::CC_TPDU, // Connection Confirm 1101 xxxx
        DR_TPDU  = X224::DR_TPDU, // Disconnect Request 1000 0000
        DT_TPDU  = X224::DT_TPDU, // Data               1111 0000 (no ROA = No Ack)
        ER_TPDU  = X224::ER_TPDU, // TPDU Error         0111 0000
    };

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

        [[nodiscard]] uint16_t data_size() const noexcept
        {
            return this->len;
        }

    private:
        explicit HeaderResult() noexcept = default;

        explicit HeaderResult(uint16_t len) noexcept
          : is_extracted(true)
          , len(len)
        {}

        bool is_extracted{false};
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

            auto av = buf.av(4);
            uint16_t len;

            REDEMPTION_DIAGNOSTIC_PUSH()
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wcovered-switch-default")
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
                    this->type = Extractors::FASTPATH;
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
            REDEMPTION_DIAGNOSTIC_POP()

            return HeaderResult::ok(len);
        }

        void prepare_data(Buf64k const & buf)
        {
            if (this->has_fast_path) {
                this->type = FASTPATH;
                return;
            }
            uint8_t tpdu_type = Parse(buf.sub(5, 1).data()).in_uint8();
            switch (uint8_t(tpdu_type & 0xF0)) {
                case X224::CR_TPDU: // Connection Request 1110 xxxx
                    this->type = Extractors::CR_TPDU;
                break;
                case X224::CC_TPDU: // Connection Confirm 1101 xxxx
                    this->type = Extractors::CC_TPDU;
                break;
                case X224::DR_TPDU: // Disconnect Request 1000 0000
                    this->type = Extractors::DR_TPDU;
                break;
                case X224::DT_TPDU: // Data               1111 0000 (no ROA = No Ack)
                    this->type = Extractors::DT_TPDU;
                break;
                case X224::ER_TPDU: // TPDU Error         0111 0000
                    this->type = Extractors::ER_TPDU;
                break;
                default:
                    this->type = 0;
                    LOG(LOG_ERR, "Bad X224 header, unknown TPDU type (code = %u)", tpdu_type);
                    throw Error(ERR_X224);
            }
        }

        [[nodiscard]] uint8_t get_type() const noexcept
        {
            if (this->has_fast_path){
                return Extractors::FASTPATH;
            }
            return this->type;
        }

    private:
        bool has_fast_path;
        uint8_t type;
    };


    struct CreedsppExtractor
    {
        static HeaderResult read_header(Buf64k & buf)
        {
            if (buf.remaining() < 4)
            {
                return HeaderResult::fail();
            }

            auto av = buf.av(4);

            if (av[1] <= 0x7F) { return HeaderResult::ok(av[1] + 2); }
            if (av[1] == 0x81) { return HeaderResult::ok(av[2] + 3); }
            if (av[1] == 0x82) { return HeaderResult::ok(((av[2] << 8) | av[3]) + 4); }
            throw Error(ERR_NEGO_INCONSISTENT_FLAGS);
        }

        static void prepare_data(Buf64k const & /*unused*/)
        {}
    };
} // namespace Extractors

struct TpduBuffer
{
    enum TpduType
    {
            PDU = 0,
        CREDSSP = 1,
    };

    TpduBuffer() = default;

    void load_data(InTransport trans)
    {
        this->buf.read_from(trans);
    }

    [[nodiscard]] uint16_t remaining() const noexcept
    {
        return this->buf.remaining();
    }

    u8_array_view remaining_data() noexcept
    {
        return this->buf.av();
    }


// Having two 'next' functions is a bit awkward.
// Also it will work only if the buffer
// knows how to consume the previous packet *and* the reader code
// knows what the current packet type is (TPDU or CREDSSP)
// Maybe something like below is possible:

//    bool next(TpduType packet)
//    {
//        switch (packet){
//        case PACKET_TPDU:
//            return this->extract(this->extractors.x224);
//        case PACKET_CREDSSP:
//            return this->extract(this->extractors.credssp);
//        }
//    }

// or fully extracting the knowledge of the Tpdu structure.
// Write tests with both CredSSP and Tpdu and see how it looks.

    bool next(TpduType packet)
    {
        switch (packet){
        case PDU:
            return this->extract(this->extractors.x224);
        case CREDSSP:
            return this->extract(this->extractors.credssp);
        }
        REDEMPTION_UNREACHABLE();
    }


    // Works the same way for CREDSSP or PDU
    // We can use it to trace CREDSSP buffer
    writable_u8_array_view current_pdu_buffer() noexcept
    {
        assert(this->pdu_len);
        auto av = this->buf.av(this->pdu_len);
        if (this->trace_pdu){
            ::hexdump_d(av);
        }
        return av;
    }

    [[nodiscard]] uint8_t current_pdu_get_type() const noexcept
    {
        assert(this->pdu_len);
        return this->extractors.x224.get_type();
    }

    void consume_current_packet() noexcept
    {
        this->buf.advance(this->pdu_len);
        this->pdu_len = 0;
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
        if (this->data_ready){
            this->data_ready = false;
            return true;
        }

        switch (this->state)
        {
            case StateRead::Header:
                this->consume_current_packet();
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
    bool data_ready = false;
public:
    bool trace_pdu = false;
};
