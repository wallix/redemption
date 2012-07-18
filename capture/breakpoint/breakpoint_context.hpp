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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__CAPTURE_BREAKPOINT_BREAKPOINT_CONTEXT_HPP__)
#define __CAPTURE_BREAKPOINT_BREAKPOINT_CONTEXT_HPP__

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "filename_generator.hpp"
#include "png.hpp"
#include "stream.hpp"
#include <transport.hpp>

struct BreakpointContext {
    virtual int pack(Stream& stream, Transport& transport,
                     const uint8_t* data_drawable,
                     uint16_t width, uint16_t height,
                     std::size_t rowsize) = 0;

    virtual int unpack(Stream& stream, Transport& transport,
                       uint8_t* data_drawable,
                       uint16_t width, uint16_t height,
                       std::size_t rowsize) = 0;

    virtual int ignore_unpack(Stream& stream, Transport& transport,
                              uint16_t width, uint16_t height,
                              std::size_t rowsize) = 0;

    virtual const char * get_cerror(int err)
    {
        return strerror(err);
    }

    /*void set_type(Stream& stream, uint8_t type)
    {
        stream.out_uint8(type);
    }*/

    void send(Stream& stream, Transport& transport)
    {
        transport.send(stream.data, (uint16_t)(stream.p - stream.data));
    }
};

/*struct BreakpointContextType {
    static const uint8_t TO_PNG = 1;
};*/

class BreakpointContextToPng
: BreakpointContext
{
    FilenameGenerator *_gfilename;

public:
    BreakpointContextToPng(FilenameGenerator *gfilename = 0)
    : _gfilename(gfilename)
    {}

    virtual int pack(Stream& stream, Transport& transport,
                     const uint8_t* data_drawable,
                     uint16_t width, uint16_t height,
                     std::size_t rowsize)
    {
        const std::string& filename = this->_gfilename->get_filename();
        const char * cstr = filename.c_str();
        if (std::FILE* fd = std::fopen(cstr, "w+"))
        {
            stream.init(4096);
            //this->set_type(stream, BreakpointContextType::TO_PNG);
            stream.out_uint32_le(filename.size());
            stream.out_copy_bytes(cstr, filename.size());
            this->send(stream, transport);
            dump_png24(fd, data_drawable, width, height, rowsize);
            return fclose(fd);
        }
        return errno;
    }

    virtual int unpack(Stream& stream, Transport& transport,
                       uint8_t* data_drawable,
                       uint16_t width, uint16_t height,
                       std::size_t rowsize)
    {
        char filename[1024];
        this->set_filename(stream, transport, filename);
        if (std::FILE* fd = std::fopen(filename, "w+"))
        {
            read_png24(fd, data_drawable, width, height, rowsize);
            return fclose(fd);
        }
        return errno;
    }

    virtual int ignore_unpack(Stream& stream, Transport& transport,
                              uint16_t /*width*/, uint16_t /*height*/,
                              std::size_t /*rowsize*/)
    {
        char filename[1024];
        this->set_filename(stream, transport, filename);
        return 0;
    }

private:
    void set_filename(Stream& stream, Transport& transport, char * filename)
    {
        stream.init(4096);
        transport.recv(&stream.end, 4);
        uint32_t len = stream.in_uint32_le();
        transport.recv(&filename, len);
        filename[len] = 0;
    }
};

#endif