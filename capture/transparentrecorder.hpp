/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CAPTURE_TRANSPARENTRECORDER_HPP_
#define _REDEMPTION_CAPTURE_TRANSPARENTRECORDER_HPP_

#include "transport.hpp"
#include "transparentchunk.hpp"

class TransparentRecorder {
private:
    Transport * t;

public:
    TransparentRecorder(Transport * t) : t(t) {
        this->send_meta_chunk();
    }

    void send_data_indication_ex(uint16_t channelId, HStream & stream) {
        BStream header(TRANSPARENT_CHUNT_HEADER_SIZE);
        BStream payload(8);

        payload.out_uint16_le(channelId);
        payload.mark_end();

        this->make_chunk_header(header, CHUNK_TYPE_SLOWPATH, payload.size() + stream.size());

        this->t->send(header);
        this->t->send(payload);
        this->t->send(stream);
    }

    void send_fastpath_data(Stream & data) {
        BStream header(TRANSPARENT_CHUNT_HEADER_SIZE);
        this->make_chunk_header(header, CHUNK_TYPE_FASTPATH, data.size());

        this->t->send(header);
        this->t->send(data);
    }

    void send_to_front_channel( const char * const mod_channel_name
                              , uint8_t * data, size_t length
                              , size_t chunk_size, int flags) {
        BStream header(TRANSPARENT_CHUNT_HEADER_SIZE);
        BStream payload(65535);

        uint8_t mod_channel_name_length = strlen(mod_channel_name);
        payload.out_uint8(mod_channel_name_length);
        payload.out_uint16_le(length);
        payload.out_uint16_le(chunk_size);
        payload.out_uint32_le(flags);
        payload.mark_end();

        this->make_chunk_header(header, CHUNK_TYPE_FRONTCHANNEL, payload.size() + mod_channel_name_length + length);

        this->t->send(header);
        this->t->send(payload);
        this->t->send(reinterpret_cast<const uint8_t *>(mod_channel_name), mod_channel_name_length);
        this->t->send(reinterpret_cast<const uint8_t *>(data), length);
    }

    void server_resize(uint16_t width, uint16_t height, uint8_t bpp) {
        BStream header(TRANSPARENT_CHUNT_HEADER_SIZE);
        BStream payload(8);

        payload.out_uint16_le(width);
        payload.out_uint16_le(height);
        payload.out_uint8(bpp);
        payload.mark_end();

        this->make_chunk_header(header, CHUNK_TYPE_RESIZE, payload.size());

        this->t->send(header);
        this->t->send(payload);
    }

private:
    void make_chunk_header(Stream & stream, uint8_t chunk_type, uint16_t data_size) {
        stream.out_uint8(chunk_type);
        stream.out_uint16_le(data_size);
        struct timeval now = tvtime();
        stream.out_timeval_to_uint64le_usec(now);
        stream.mark_end();
    }

    void send_meta_chunk() {
        const uint8_t trm_format_version = 0;

        BStream header(TRANSPARENT_CHUNT_HEADER_SIZE);
        BStream payload(8);

        payload.out_uint8(trm_format_version);
        payload.mark_end();

        this->make_chunk_header(header, CHUNK_TYPE_META, payload.size());

        this->t->send(header);
        this->t->send(payload);
    }
};

#endif  // #ifndef _REDEMPTION_CAPTURE_TRANSPARENTRECORDER_HPP_
