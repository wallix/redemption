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

public:
    RDPUnserializer reader;

public:
    MetaWRM meta;


private:
    static int open(const char * filename)
    {
        LOG(LOG_INFO, "Recording to file : %s", filename);
        int fd = ::open(filename, O_RDONLY);
        if (-1 == fd){
            LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
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
    void recv_rect(Rect& rect)
    {
        rect.x = this->reader.stream.in_uint16_le();
        rect.y = this->reader.stream.in_uint16_le();
        rect.cx = this->reader.stream.in_uint16_le();
        rect.cy = this->reader.stream.in_uint16_le();
    }

    void recv_brush(RDPBrush& brush)
    {
        brush.org_x = this->reader.stream.in_uint8();
        brush.org_y = this->reader.stream.in_uint8();
        brush.style = this->reader.stream.in_uint8();
        brush.hatch = this->reader.stream.in_uint8();
        brush.extra[0] = this->reader.stream.in_uint8();
        brush.extra[1] = this->reader.stream.in_uint8();
        brush.extra[2] = this->reader.stream.in_uint8();
        brush.extra[3] = this->reader.stream.in_uint8();
        brush.extra[4] = this->reader.stream.in_uint8();
        brush.extra[5] = this->reader.stream.in_uint8();
        brush.extra[6] = this->reader.stream.in_uint8();
    }

    void recv_pen(RDPPen& pen)
    {
        pen.color = this->reader.stream.in_uint32_le();
        pen.style = this->reader.stream.in_uint8();
        pen.width = this->reader.stream.in_uint8();
    }

public:
    void interpret_order()
    {
        switch (this->reader.chunk_type) {
            case WRMChunk::NEXT_FILE:
            {
                char filename[1024];
                size_t len = this->reader.stream.in_uint32_le();
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
                    this->meta.in(this->reader.stream);
                    //MetaWRM meta;
                    //meta.in(this->reader.stream);
                    //if (this->meta != meta){
                    //            this->reader.consumer->resize(meta.width, meta.height, meta.bpp);
                    //            this->meta = meta;
                    //}
                    //std::cout << "interpret_order: meta" << meta.width << ',' <<  meta.height << ',' <<  short(meta.bpp) << '\n';
                }

                //char texttest[10000];

                this->reader.common.order = this->reader.stream.in_uint8();
                this->recv_rect(this->reader.common.clip);
                //this->reader.common.str(texttest, 10000);
                //std::cout << "interpret_order: " << texttest << '\n';

                this->reader.opaquerect.color = this->reader.stream.in_uint32_le();
                this->recv_rect(this->reader.opaquerect.rect);
                //std::cout << "interpret_order: ";
                //this->reader.opaquerect.print(Rect(0,0,0,0));

                this->reader.destblt.rop = this->reader.stream.in_uint8();
                this->recv_rect(this->reader.destblt.rect);
                //std::cout << "interpret_order: ";
                //this->reader.destblt.print(Rect(0,0,0,0));

                this->reader.patblt.rop = this->reader.stream.in_uint8();
                this->reader.patblt.back_color = this->reader.stream.in_uint32_le();
                this->reader.patblt.fore_color = this->reader.stream.in_uint32_le();
                this->recv_brush(this->reader.patblt.brush);
                this->recv_rect(this->reader.patblt.rect);
                //std::cout << "interpret_order: ";
                //this->reader.patblt.print(Rect(0,0,0,0));

                this->reader.scrblt.rop = this->reader.stream.in_uint8();
                this->reader.scrblt.srcx = this->reader.stream.in_uint16_le();
                this->reader.scrblt.srcy = this->reader.stream.in_uint16_le();
                this->recv_rect(this->reader.scrblt.rect);
                //std::cout << "interpret_order: ";
                //this->reader.scrblt.print(Rect(0,0,0,0));

                this->reader.memblt.rop = this->reader.stream.in_uint8();
                this->reader.memblt.srcx = this->reader.stream.in_uint16_le();
                this->reader.memblt.srcy = this->reader.stream.in_uint16_le();
                this->reader.memblt.cache_id = this->reader.stream.in_uint16_le();
                this->reader.memblt.cache_idx = this->reader.stream.in_uint16_le();
                this->recv_rect(this->reader.memblt.rect);
                //std::cout << "interpret_order: ";
                //this->reader.memblt.print(Rect(0,0,0,0));

                this->reader.lineto.rop2 = this->reader.stream.in_uint8();
                this->reader.lineto.startx = this->reader.stream.in_uint16_le();
                this->reader.lineto.starty = this->reader.stream.in_uint16_le();
                this->reader.lineto.endx = this->reader.stream.in_uint16_le();
                this->reader.lineto.endy = this->reader.stream.in_uint16_le();
                this->reader.lineto.back_mode = this->reader.stream.in_uint8();
                this->reader.lineto.back_color = this->reader.stream.in_uint32_le();
                this->recv_pen(this->reader.lineto.pen);
                //std::cout << "interpret_order: ";
                //this->reader.lineto.print(Rect(0,0,0,0));

                this->reader.glyphindex.back_color = this->reader.stream.in_uint32_le();
                this->reader.glyphindex.fore_color = this->reader.stream.in_uint32_le();
                this->reader.glyphindex.f_op_redundant = this->reader.stream.in_uint16_le();
                this->reader.glyphindex.fl_accel = this->reader.stream.in_uint16_le();
                this->reader.glyphindex.glyph_x = this->reader.stream.in_uint16_le();
                this->reader.glyphindex.glyph_y = this->reader.stream.in_uint16_le();
                this->reader.glyphindex.ui_charinc = this->reader.stream.in_uint16_le();
                this->reader.glyphindex.cache_id = this->reader.stream.in_uint8();
                this->reader.glyphindex.data_len = this->reader.stream.in_uint8();
                this->recv_rect(this->reader.glyphindex.bk);
                this->recv_rect(this->reader.glyphindex.op);
                this->recv_brush(this->reader.glyphindex.brush);
                this->reader.glyphindex.data = (uint8_t*)malloc(this->reader.glyphindex.data_len);
                this->reader.stream.in_copy_bytes(this->reader.glyphindex.data, this->reader.glyphindex.data_len);
                //std::cout << "interpret_order: ";
                //this->reader.glyphindex.print(Rect(0,0,0,0));

                this->reader.order_count = this->reader.stream.in_uint16_le();
                //std::cout << "\ninterpret_order: "  << this->reader.order_count << '\n';

                this->reader.bmp_cache.small_entries = this->reader.stream.in_uint16_le();
                this->reader.bmp_cache.small_size = this->reader.stream.in_uint16_le();
                this->reader.bmp_cache.medium_entries = this->reader.stream.in_uint16_le();
                this->reader.bmp_cache.medium_size = this->reader.stream.in_uint16_le();
                this->reader.bmp_cache.big_entries = this->reader.stream.in_uint16_le();
                this->reader.bmp_cache.big_size = this->reader.stream.in_uint16_le();
                this->reader.bmp_cache.stamp = this->reader.stream.in_uint32_le();
                //std::cout << "interpret_order: "  << this->reader.bmp_cache.small_entries << ',' << this->reader.bmp_cache.small_size << ',' << this->reader.bmp_cache.medium_entries << ',' << this->reader.bmp_cache.medium_size << ',' << this->reader.bmp_cache.big_entries << ',' << this->reader.bmp_cache.big_size << '\n';

                for (size_t cid = 0; cid < 3 ; ++cid){
                    uint32_t* stamps = this->reader.bmp_cache.stamps[cid];
                    //std::cout << "\ninterpret_order: cid=" << cid << ": ";
                    for (uint8_t n = 0; n < 8; ++n, stamps += 1024){
                        this->reader.stream.init(4096);
                        this->trans.recv(&this->reader.stream.end, 4096);
                        for (size_t cidx = 0; cidx < 1024; ++cidx){
                            stamps[cidx] = this->reader.stream.in_uint32_le();
                            //if (stamps[cidx])
                            //    std::cout << cidx << ':' << stamps[cidx] << ',';
                        }
                    }
                }
                //std::cout << "cid end\n";

                {
                    uint8_t bpp;
                    uint16_t cx;
                    uint16_t cy;
                    size_t bmp_size;

                    BGRPalette palette;

                    char bmp_data[this->reader.bmp_cache.big_size];

                    uint16_t used;
                    uint8_t count;
                    uint16_t cidx;
                    uint16_t fcidx;

                    for (size_t cid = 0; cid < 3 ; ++cid){
                        const Bitmap * (&cache)[8192] = this->reader.bmp_cache.cache[cid];
                        uint8_t (&sha1)[8192][20] = this->reader.bmp_cache.sha1[cid];
                        bzero((void*)cache, 8192 * sizeof(Bitmap *));
                        this->reader.stream.init(4096);
                        this->trans.recv(&this->reader.stream.end, 2);
                        used = this->reader.stream.in_uint16_le();
                        //std::cout << "interpret_order bmp used: " << used << '\n';
                        for (; used; --used){
                            this->reader.stream.init(4096);
                            this->trans.recv(&this->reader.stream.end, 10);
                            bpp = this->reader.stream.in_uint8();
                            cx = this->reader.stream.in_uint16_le();
                            cy = this->reader.stream.in_uint16_le();
                            bmp_size = this->reader.stream.in_uint32_le();
                            count = this->reader.stream.in_uint8();
                            //std::cout << "interpret_order bmp: " << short(bpp) << ',' << cx << ',' << cy << ',' << bmp_size << ',' << short(count) << '\n';
                            char * p = (char*)bmp_data;
                            this->trans.recv(&p, bmp_size);

                            this->reader.stream.init(4096);
                            this->trans.recv(&this->reader.stream.end, sizeof(uint16_t) * count);
                            const Bitmap* bmp = new Bitmap(bpp, &palette, cx, cy, (uint8_t*)bmp_data, bmp_size);
                            cidx = this->reader.stream.in_uint16_le();
                            fcidx = cidx;
                            cache[cidx] = bmp;
                            bmp->compute_sha1(sha1[cidx]);
                            while (--count){
                                cidx = this->reader.stream.in_uint16_le();
                                cache[cidx] = new Bitmap(bpp, *bmp);
                                memcpy(sha1[cidx], sha1[fcidx], 20);
                            }
                        }
                    }
                }

                this->reader.stream.init(4096);
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
