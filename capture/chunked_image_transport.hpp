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

#ifndef _REDEMPTION_CAPTURE_CHUNKED_IMAGE_TRANSPORT_HPP_
#define _REDEMPTION_CAPTURE_CHUNKED_IMAGE_TRANSPORT_HPP_

#include "transport.hpp"
#include "RDP/RDPDrawable.hpp"
#include "png.hpp"


class InChunkedImageTransport : public Transport {
    uint16_t chunk_type; 
    uint32_t chunk_size;
    uint16_t chunk_count;
    Transport * trans;
    BStream stream;
public:
    InChunkedImageTransport(uint16_t chunk_type, uint32_t chunk_size, Transport * trans) 
        : chunk_type(chunk_type)
        , chunk_size(chunk_size)
        , chunk_count(1)
        , trans(trans) 
    {
        this->stream.init(this->chunk_size - 8);
        this->trans->recv(&stream.end, this->chunk_size - 8);
    }
    
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t total_len = 0;
        while (total_len < len){
            if (static_cast<size_t>(stream.end - stream.p) >= static_cast<size_t>(len - total_len)){
                stream.in_copy_bytes(*pbuffer + total_len, len - total_len);
                total_len += len - total_len;
                *pbuffer += len;
                return;
            }
            uint16_t remaining = stream.end - stream.p;
            stream.in_copy_bytes(*pbuffer + total_len, remaining);
            total_len += remaining;
            switch (this->chunk_type){
            case PARTIAL_IMAGE_CHUNK:
            {
                BStream header(8);
                this->trans->recv(&header.end, 8);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
                this->stream.init(this->chunk_size - 8);
                this->trans->recv(&this->stream.end, this->chunk_size - 8);
            }
            break;
            case LAST_IMAGE_CHUNK:
                LOG(LOG_ERR, "Failed to read embedded image from WRM (transport closed)");
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            break;
            default:
                LOG(LOG_ERR, "Failed to read embedded image from WRM");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            break;
            }
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }
    
    virtual void seek(int64_t offset, int whence) throw (Error) { throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE); }
    
};

#endif
