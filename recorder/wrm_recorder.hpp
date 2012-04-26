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
#include "RDP/RDPDrawable.hpp"
#include "bitmap.hpp"
#include "stream.hpp"

#include <iostream>

class WRMRecorder
{
    InFileTransport trans;

public:
    RDPUnserializer reader;

public:
    MetaWRM meta;

private:
    Drawable * drawable;


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
    , drawable(0)
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

    void drawable_consumer(Drawable* consumer)
    {
        this->drawable = consumer;
    }

    RDPGraphicDevice * consumer()
    {
        return this->reader.consumer;
    }

    Drawable * drawable_consumer()
    {
        return this->drawable;
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
                //std::cout << "size stream " << short(this->reader.stream.end - this->reader.stream.data) << ' ' << short(this->reader.stream.end - this->reader.stream.p) << '\n';
                {
                    this->meta.in(this->reader.stream);
                    //MetaWRM meta;
                    //meta.in(this->reader.stream);
                    //if (this->meta != meta){
                    //            this->reader.consumer->resize(meta.width, meta.height, meta.bpp);
                    //            this->meta = meta;
                    //}
                    //std::cout << "read meta " << meta << std::endl;
                }

                //char texttest[10000];

                {
                    uint16_t width = this->reader.stream.in_uint16_le();
                    uint16_t height = this->reader.stream.in_uint16_le();
                    --this->reader.remaining_order_count;

                    if (this->drawable) {
                        uint8_t * pdata = this->drawable->data;
                        for (size_t n = 0, len = width * height * 3 / (4096 - 8); len--;){
                            this->reader.selected_next_order();
                            for (size_t last = n + (4096 - 8); n != last; ++n, ++pdata){
                                *pdata = this->reader.stream.in_uint8();
                            }
                            --this->reader.remaining_order_count;
                        }
                        if (width * height * 3 % (4096 - 8)){
                            this->reader.selected_next_order();
                            for (size_t last = width * height * 3 % (4096 - 8); last--; ++pdata){
                                *pdata = this->reader.stream.in_uint8();
                            }
                            --this->reader.remaining_order_count;
                        }
                    }
                    else{
                        for (size_t n = 0, len = width * height * 3 / (4096 - 8); n != len; ++n){
                            this->reader.selected_next_order();
                            --this->reader.remaining_order_count;
                        }
                        if (width * height * 3 % (4096 - 8)){
                            this->reader.selected_next_order();
                            --this->reader.remaining_order_count;
                        }
                    }


                    /*uint16_t nx = this->reader.stream.in_uint16_le();
                    //std::cout << "read nb_axis " << nx << ' ';
                    uint16_t ny = this->reader.stream.in_uint16_le();
                    //std::cout << ny << '\n';

                    //std::cout << "read data size " << (this->reader.stream.p - this->reader.stream.data) << '\n';


                    --this->reader.remaining_order_count;
                    //uint nn = 0;

                    if (this->redrawable) {
                        {
                            ///NOTE set zero test
                            Drawable& d = dynamic_cast<RDPDrawable*>(this->redrawable)->drawable;
                            for (int i = 0; i != d.height; ++i){
                                bzero(&d.data[i*d.rowsize], d.rowsize);
                            }
                        }

                        Rect clip(0,0, this->meta.width, this->meta.height);
                        RDPMemBlt memblt(0, Rect(0,0,32,32), 0xCC, 0, 0, 0);
                        RDPBmpCache cmdcache;
                        BGRPalette palette;

                        for (uint16_t y = 0 ; y != ny; ++y, memblt.rect.y += 32) {
                            memblt.rect.x = 0;
                            for (uint16_t x = 0 ; x != nx;
                                 ++x, memblt.rect.x += 32) {
                                this->reader.selected_next_order();
                                //std::cout << "read bmp chunk (type: " << this->reader.chunk_type << ", n:" << this->reader.remaining_order_count << "): " << (RDP_UPDATE_ORDERS == this->reader.chunk_type) <<  ' ';
                                uint8_t control = this->reader.stream.in_uint8();
                                --this->reader.remaining_order_count;
                                RDPSecondaryOrderHeader header(this->reader.stream);
                                uint8_t *next_order = this->reader.stream.p + header.length + 7;
                                //std::cout  << (header.type == RDP::TS_CACHE_BITMAP_COMPRESSED) << ' ';
                                cmdcache.receive(this->reader.stream, control, header, palette);
                                this->reader.stream.p = next_order;

                                {
                                    ///NOTE manuel draw()…
                                    Drawable& d = dynamic_cast<RDPDrawable*>(this->redrawable)->drawable;

                                    const Rect & trect = Rect(memblt.rect.x, memblt.rect.y,
                                                              std::min<int16_t>(d.width - memblt.rect.x, memblt.rect.cx),
                                                              std::min<int16_t>(d.height - memblt.rect.y, memblt.rect.cy));
                                    uint8_t * target = d.first_pixel(trect);
                                    const uint8_t * source = cmdcache.bmp->data() + (cmdcache.bmp->cy - 1) * (cmdcache.bmp->line_size);
                                    int steptarget = (d.width - trect.cx) * 3;
                                    int stepsource = (cmdcache.bmp->line_size) + trect.cx * 3;

                                    for (int yy = 0; yy < trect.cy ; yy++, target += steptarget, source -= stepsource){
                                        for (int xx = 0; xx < trect.cx ; xx++, target += 3, source += 3){
                                            uint32_t px = source[3-1];
                                            for (int b = 1 ; b < 3 ; b++){
                                                px = (px << 8) + source[3-1-b];
                                            }
                                            uint32_t color = color_decode(px, cmdcache.bmp->original_bpp, cmdcache.bmp->original_palette);
                                            //if (bgr){
                                            //    color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
                                            //}
                                            target[0] = color;
                                            target[1] = color >> 8;
                                            target[2] = color >> 16;
                                        }
                                    }
                                }
                                //this->redrawable->draw(memblt, clip, *cmdcache.bmp);

                                //std::cout << "reader bmp (size:" << cmdcache.bmp->bmp_size << "): " << (this->reader.stream.p - this->reader.stream.data) << '\n';
                                delete cmdcache.bmp;
                                //++nn;
                            }
                        }
                    }
                    else {
                        std::cout << "read ignore cache\n";
                        uint n = nx * ny;
                        while (n)
                        {
                            this->reader.selected_next_order();
                            n -= this->reader.remaining_order_count;
                            this->reader.remaining_order_count = 0;
                        }
                    }*/
                    //std::cout << "read number img  " << nn << '\n';
                }

                this->reader.selected_next_order();

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
                uint32_t stamp = this->reader.stream.in_uint32_le();
                //std::cout << "interpret_order: "  << this->reader.bmp_cache.small_entries << ',' << this->reader.bmp_cache.small_size << ',' << this->reader.bmp_cache.medium_entries << ',' << this->reader.bmp_cache.medium_size << ',' << this->reader.bmp_cache.big_entries << ',' << this->reader.bmp_cache.big_size << '\n';

                this->reader.bmp_cache.reset();
                uint16_t nb_img = this->reader.stream.in_uint16_le();                //std::cout << "read nb_img " << nb_img << '\n';
                this->reader.remaining_order_count = 0;
                for (; nb_img; --nb_img){
                    this->reader.next();
                }
                this->reader.bmp_cache.stamp = stamp;

                for (size_t cid = 0; cid != 3 ; ++cid){
                    uint32_t (&stamps)[8192] = this->reader.bmp_cache.stamps[cid];
                    uint16_t cidx = 0;
                    for (uint16_t n = 0; n != 9 ; ++n){
                        this->reader.selected_next_order();
                        for (uint16_t last = std::min<>(cidx + 8192 / 9, 8192); cidx != last; ++cidx){
                            stamps[cidx] = this->reader.stream.in_uint32_le();
                        }
                        this->reader.remaining_order_count = 0;
                    }
                }

                //this->reader.stream.init(this->reader.stream.capacity);
                //this->reader.remaining_order_count = 0;
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
