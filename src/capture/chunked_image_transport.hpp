/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

    Chunked Image Transport is used to interleave a png file inside a wrm file
    using pnglib io API.

*/


#pragma once

#include "transport/transport.hpp"
#include "utils/stream.hpp"
#include "wrm_label.hpp"

#include <memory>

class InChunkedImageTransport : public Transport
{
    uint16_t chunk_type;
    uint32_t chunk_size;
    uint16_t chunk_count;
    Transport * trans;
    char buf[65536];
    InStream in_stream;

public:
    InChunkedImageTransport(uint16_t chunk_type, uint32_t chunk_size, Transport * trans)
        : chunk_type(chunk_type)
        , chunk_size(chunk_size)
        , chunk_count(1)
        , trans(trans)
        , in_stream(this->buf, this->chunk_size - 8)
    {
        auto * p = this->buf;
        this->trans->recv(&p, this->in_stream.get_capacity());
    }

private:
    void do_recv(uint8_t ** pbuffer, size_t len) override {
        size_t total_len = 0;
        while (total_len < len){
            size_t remaining = in_stream.in_remain();
            if (remaining >= (len - total_len)){
                in_stream.in_copy_bytes(*pbuffer + total_len, len - total_len);
                *pbuffer += len;
                return;
            }
            in_stream.in_copy_bytes(*pbuffer + total_len, remaining);
            total_len += remaining;
            switch (this->chunk_type){
            case PARTIAL_IMAGE_CHUNK:
            {
                const size_t header_sz = 8;
                char header_buf[header_sz];
                InStream header(header_buf);
                auto * p = header_buf;
                this->trans->recv(&p, header_sz);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
                this->in_stream = InStream(this->buf, this->chunk_size - 8);
                p = this->buf;
                this->trans->recv(&p, this->chunk_size - 8);
            }
            break;
            case LAST_IMAGE_CHUNK:
                LOG(LOG_ERR, "Failed to read embedded image from WRM (transport closed)");
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            default:
                LOG(LOG_ERR, "Failed to read embedded image from WRM");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
        }
    }
};

