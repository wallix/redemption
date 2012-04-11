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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 */

#if !defined(__META_WRM_HPP__)
#define __META_WRM_HPP__

#include "RDP/RDPGraphicDevice.hpp"

struct MetaWRM {
    uint16_t version;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;

    MetaWRM()
    : version(0)
    , width(0)
    , height(0)
    , bpp(0)
    {}

    MetaWRM(RDPUnserializer& unserializer)
    {
        this->recv(unserializer);
    }

    MetaWRM(uint16_t width, uint16_t height, uint8_t bpp)
    : version(0)
    , width(width)
    , height(height)
    , bpp(bpp)
    {}

    void in(Stream& stream)
    {
        stream.in_copy_bytes((uint8_t *)this, sizeof(*this));/*
        this->version = stream.in_uint16_le();
        this->width = stream.in_uint16_le();
        this->height = stream.in_uint16_le();
        this->bpp = stream.in_uint8();*/
    }

    void recv(RDPUnserializer& unserializer)
    {
        unserializer.selected_next_order();
        if (unserializer.chunk_type == WRMChunk::META_INFO) {
            this->in(unserializer.stream);
            --unserializer.remaining_order_count;
        }
        else {
            this->version = 0;
            this->width = 800;
            this->height = 600;
            this->bpp = 24;
        }
    }

    void send(RDPSerializer& serializer) const
    {
        this->send(serializer.stream, *serializer.trans);
    }

    void send(Stream& stream, Transport& transport) const
    {
        out(stream);
        transport.send(stream.data, sizeof(*this)+8/*stream.p - stream.data*/);
    }

    void out(Stream& stream) const
    {
        stream.set_out_uint16_le(WRMChunk::META_INFO, 0);
        stream.set_out_uint16_le(sizeof(*this) + 8, 2);
        stream.set_out_uint16_le(1, 4);
        stream.set_out_uint16_le(0, 6);
        stream.set_out_copy_bytes((const uint8_t *)this, sizeof(*this), 8);
    }
};

inline bool operator==(const MetaWRM& meta1, const MetaWRM& meta2)
{
    return meta1.width == meta2.width
        && meta1.height == meta2.height
        && meta1.bpp == meta2.bpp;
}

inline bool operator!=(const MetaWRM& meta1, const MetaWRM& meta2)
{
    return !(meta1 == meta2);
}

#endif