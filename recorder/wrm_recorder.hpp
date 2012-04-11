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

#if !defined(__WRM_RECORDER_HPP__)
#define __WRM_RECORDER_HPP__

#include "transport.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "meta_wrm.hpp"
#include "bitmap.hpp"

class WRMRecorder
{
    InFileTransport trans;
    RDPUnserializer reader;

public:
    MetaWRM meta;


private:
    static int open(const char * filename)
    {
        int fd = ::open(filename, O_RDONLY);
        if (-1 == fd){
           throw Error(0/*ERR_WRM_RECORDER_OPEN_FAILED*/);
        }
        return fd;
    }

public:
    WRMRecorder(const char *filename)
    : trans(open(filename))
    , reader(&trans, 0, Rect())
    , meta(reader)
    {
        this->reader.screen_rect.cx = this->meta.width;
        this->reader.screen_rect.cy = this->meta.height;
    }

    ~WRMRecorder()
    {
        ::close(this->trans.fd);
    }

    void consumer(RDPGraphicDevice * consumer)
    {
        this->reader.consumer = consumer;
    }

    RDPGraphicDevice * consumer()
    {
        return this->reader.consumer;
    }

    bool selected_next_order()
    {
        return this->reader.selected_next_order();
    }

    uint16_t& chunk_type()
    {
        return this->reader.chunk_type;
    }

    uint16_t& remaining_order_count()
    {
        return this->reader.remaining_order_count;
    }

private:
    template<typename _T>
    void in_copy_bytes(_T& v)
    {
        this->reader.stream.in_copy_bytes((uint8_t*)&v, sizeof(_T));
    }

public:
    void interpret_order()
    {
        switch (this->reader.chunk_type) {
            case WRMChunk::NEXT_FILE:
            {
                char filename[1024];
                size_t len;
                this->reader.stream.in_copy_bytes((uint8_t*)&len, sizeof(len));
                this->reader.stream.in_copy_bytes((uint8_t*)filename, len);

                filename[len] = 0;
                ::close(this->trans.fd);
                this->trans.fd = -1;
                this->trans.fd = open(filename); //can throw exception
                this->trans.total_received = 0;
                this->trans.last_quantum_received = 0;
                this->trans.total_sent = 0;
                this->trans.last_quantum_sent = 0;
                this->trans.quantum_count = 0;
                this->reader.remaining_order_count = 0;
            }
            break;
            case WRMChunk::BREAKPOINT:
            {
                {
                    MetaWRM meta;
                    meta.in(this->reader.stream);
                    /*if (this->meta != meta){
                     *            this->reader.consumer->resize(meta.width, meta.height, meta.bpp);
                     *            this->meta = meta;
                    }*/
                }

                this->in_copy_bytes(this->reader.common);
                this->in_copy_bytes(this->reader.destblt);
                this->in_copy_bytes(this->reader.patblt);
                this->in_copy_bytes(this->reader.scrblt);
                this->in_copy_bytes(this->reader.opaquerect);
                this->in_copy_bytes(this->reader.memblt);
                this->in_copy_bytes(this->reader.lineto);
                this->in_copy_bytes(this->reader.glyphindex);
                this->reader.glyphindex.data = (uint8_t*)"";
                //this->reader.stream.in_copy_bytes(this->reader.glyphindex.data, this->reader.glyphindex.data_len);
                this->in_copy_bytes(this->reader.order_count);

                this->in_copy_bytes(this->reader.bmp_cache.small_entries);
                this->in_copy_bytes(this->reader.bmp_cache.small_size);
                this->in_copy_bytes(this->reader.bmp_cache.medium_entries);
                this->in_copy_bytes(this->reader.bmp_cache.medium_size);
                this->in_copy_bytes(this->reader.bmp_cache.big_entries);
                this->in_copy_bytes(this->reader.bmp_cache.big_size);
                this->in_copy_bytes(this->reader.bmp_cache.stamps);
                this->in_copy_bytes(this->reader.bmp_cache.stamp);

                {
                    uint8_t bpp;
                    uint16_t cx;
                    uint16_t cy;
                    size_t bmp_size;

                    BGRPalette palette;
                    BGRPalette palette332;
                    init_palette332(palette332);

                    bool use_bmp;

                    for (size_t cid = 0; cid < 3 ; cid++){
                        for (size_t cidx = 0; cidx < 8192 ; cidx++){
                            this->in_copy_bytes(use_bmp);
                            if (use_bmp){
                                this->in_copy_bytes(bpp);
                                this->in_copy_bytes(cx);
                                this->in_copy_bytes(cy);
                                this->in_copy_bytes(bmp_size);
                                if (8 == bpp)
                                    this->in_copy_bytes(palette);

                                this->reader.bmp_cache.cache[cid][cidx] = new Bitmap(bpp, 8 == bpp ? &palette : &palette332, cx, cy, this->reader.stream.p, bmp_size);
                                this->reader.stream.p += bmp_size;
                            }
                            else{
                                this->reader.bmp_cache.cache[cid][cidx] = 0;
                            }
                        }
                    }
                }

                this->reader.remaining_order_count = 0;
            }
            break;
            default:
                this->reader.interpret_order();
                break;
        }
    }

    bool next_order()
    {
        if (this->selected_next_order())
        {
            this->interpret_order();
            return true;
        }
        return false;
    }
};

#endif
