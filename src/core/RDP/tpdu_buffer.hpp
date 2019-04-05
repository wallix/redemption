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
#include "core/RDP/packet_extractors.hpp"

#include "transport/transport.hpp"

struct TpduBuffer
{
    enum TpduType {
            PDU = 0,
        CREDSSP = 1
    };

    TpduBuffer() = default;

    void load_data(InTransport trans)
    {
        this->buf.read_from(trans);
    }

    uint16_t remaining() const noexcept
    {
        return this->buf.remaining();
    }

    array_view_u8 remaining_data() noexcept
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

    bool check(TpduType packet)
    {
        switch (packet){
        default:
        case PDU:
            return this->check_extract(this->extractors.x224);
        case CREDSSP:
            return this->check_extract(this->extractors.x224);
        }
    }


    bool next(TpduType packet)
    {
        switch (packet){
        default:
        case PDU:
            return this->extract(this->extractors.x224);
        case CREDSSP:
            return this->extract(this->extractors.credssp);
        }
    }


    // Works the same way for CREDSSP or PDU
    // We can use it to trace CREDSSP buffer
    array_view_u8 current_pdu_buffer() noexcept
    {
        assert(this->pdu_len);
        auto av = this->buf.av(this->pdu_len);
//        if (this->trace_pdu){
            ::hexdump_av_d(av);
//        }
        return av;
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

    template<class Extractor>
    bool check_extract(Extractor & extractor)
    {
        if (!this->data_ready){
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
                            this->data_ready = true;
                            return true;
                        }
                        this->state = StateRead::Data;
                    }
                    this->data_ready = false;
                    return false;

                case StateRead::Data:
                    if (this->pdu_len <= this->buf.remaining())
                    {
                        this->state = StateRead::Header;
                        extractor.prepare_data(this->buf);
                        this->data_ready = true;
                        return true;
                    }
                    this->data_ready = false;
                    return false;
            }
        }
        return this->data_ready;
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
