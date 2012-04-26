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
#include <iosfwd>

struct MetaWRM {
    uint16_t version;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;

    static const size_t size_for_stream = 7;

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
        this->version = stream.in_uint16_le();
        this->width = stream.in_uint16_le();
        this->height = stream.in_uint16_le();
        this->bpp = stream.in_uint8();
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
        if (stream.p == stream.data)
        {
            stream.out_uint16_le(WRMChunk::META_INFO);
            stream.out_uint16_le(7 + 8);
            stream.out_uint16_le(1);
            stream.out_uint16_le(0);
        }
        else
        {
            stream.set_out_uint16_le(WRMChunk::META_INFO, 0);
            stream.set_out_uint16_le(7 + 8, 2);
            stream.set_out_uint16_le(1, 4);
        }
        this->out(stream);
        transport.send(stream.data, stream.p - stream.data);
        stream.p = stream.data + 8;
        stream.end = stream.p;
    }

    void out(Stream& stream) const
    {
        stream.out_uint16_le(this->version);
        stream.out_uint16_le(this->width);
        stream.out_uint16_le(this->height);
        stream.out_uint8(this->bpp);
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

template<typename _Char, typename _CharTraits>
std::basic_ostream<_Char, _CharTraits>&
operator<<(std::basic_ostream<_Char, _CharTraits>& os, const MetaWRM& meta)
{
    return os << "{version:" << meta.version
    << ", width: " << meta.width
    << ", height: " << meta.height
    << ", bpp: " << short(meta.bpp) << '}';
}

#endif