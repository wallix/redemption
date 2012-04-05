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

#if !defined(__META_CAPTURE_HPP__)
#define __META_CAPTURE_HPP__

#include <stdint.h>
#include <unistd.h>

struct MetaWRM {
    uint16_t version;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;

    MetaWRM()
    : version(0)
    {}

    MetaWRM(uint16_t width, uint16_t height, uint8_t bpp)
    : version(0)
    , width(width)
    , height(height)
    , bpp(bpp)
    {}

    MetaWRM(uint8_t bpp)
    : version(0)
    , bpp(bpp)
    {}

    ssize_t read(int fd)
    {
        return ::read(fd, this, sizeof(MetaWRM));
    }

    ssize_t write(int fd) const
    {
        return ::write(fd, this, sizeof(MetaWRM));
    }
};

MetaWRM make_meta_wrm(int fd)
{
    MetaWRM meta;
    meta.read(fd);
    return meta;
}

#endif