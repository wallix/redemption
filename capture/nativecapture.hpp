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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__CAPTURE_NATIVECAPTURE_HPP__)
#define __CAPTURE_NATIVECAPTURE_HPP__

#include <list>
#include <stdio.h>
#include "rdtsc.hpp"
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <time.h>
#include "difftimeval.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "error.hpp"
#include "config.hpp"
#include "bmpcache.hpp"
#include "colors.hpp"

#include "GraphicToFile.hpp"

#include <iostream>

class NativeCapture : public RDPGraphicDevice
{
    public:
    int width;
    int height;
    int bpp;
    OutFileTransport trans;
    GraphicsToFile recorder;
    char basepath[1024];
    uint16_t basepath_len;
    uint32_t nb_file;

private:
    int next_filename()
    {
        return sprintf(this->basepath + this->basepath_len, "%u.wrm", this->nb_file++);
    }

    void open_file()
    {
        LOG(LOG_INFO, "Recording to file : %s", this->basepath);
        this->trans.fd = open(this->basepath, O_WRONLY|O_CREAT, 0666);
        if (this->trans.fd < 0){
            LOG(LOG_ERR, "Error opening native capture file : %s", strerror(errno));
            throw Error(ERR_RECORDER_NATIVE_CAPTURE_OPEN_FAILED);
        }
    }

public:
    NativeCapture(int width, int height, const char * path)
    : width(width)
    , height(height)
    , bpp(24)
    , trans(-1)
    , recorder(&this->trans, NULL, 24, 8192, 768, 8192, 3072, 8192, 12288)
    , nb_file(0)
    {
        this->basepath_len = sprintf(this->basepath, "%s-%u-", path, getpid());
        this->next_filename();
        this->open_file();
    }

    ~NativeCapture(){
        close(this->trans.fd);
    }

    virtual void flush()
    {}

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->recorder.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void glyph_index(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {}

private:
    struct BitmapsSender {

        uint16_t used;
        struct Data {
            uint16_t cidx;
            const Bitmap* bmp;
        };
        Data datas[8192];

        struct Compare
        {
            bool operator()(const Data a, const Data b)
            {
                return a.bmp->data() < b.bmp->data();
            }
        };

        BitmapsSender(const Bitmap* bitmaps[])
        : used(0)
        {
            for (uint16_t cidx = 0; cidx < 8192 ; ++cidx){
                const Bitmap* bmp = bitmaps[cidx];
                if (bmp){
                    this->datas[this->used].cidx = cidx;
                    this->datas[this->used].bmp = bmp;
                    ++this->used;
                }
            }
            std::sort<>(&this->datas[0], &this->datas[this->used], Compare());
        }

        size_t size() const
        {
            size_t ret = 2 + 8 * this->used;
            const uint8_t *prev_data = 0;
            for (uint16_t cidx = 0; cidx < this->used; ++cidx){
                const Bitmap* bmp = this->datas[cidx].bmp;
                if (prev_data != bmp->data()){
                    ret += 1 + bmp->bmp_size;
                }
                prev_data = bmp->data();
            }
            return ret;
        }

        void send_in(Stream& stream)
        {
            stream.out_uint16_le(this->used);
            const uint8_t *prev_data = 0;
            for (uint16_t n = 0; n < this->used; ++n){
                Data &data = this->datas[n];
                const Bitmap* bmp = data.bmp;
                if (prev_data == bmp->data())
                {
                    stream.out_uint16_le(~uint16_t(0));
                }
                else
                {
                    stream.out_uint16_le(data.cidx);
                    stream.out_uint32_le(bmp->bmp_size);
                    stream.out_copy_bytes(bmp->data(), bmp->bmp_size);
                }
                prev_data = bmp->data();
                stream.out_uint8(bmp->original_bpp);
                stream.out_uint16_le(bmp->cx);
                stream.out_uint16_le(bmp->cy);
            }
        }
    };

    /*struct BitmapsSender {
        typedef std::list<uint16_t> list_type;

        struct Data{
            uint8_t count;
            const Bitmap* bmp;
            list_type list;

            Data()
            : count(0)
            , bmp(0)
            , list()
            {}
        };

        uint16_t used;
        Data datas[8192];

        BitmapsSender(const Bitmap* bitmaps[])
        : used(0)
        {
            for (uint16_t cidx = 0; cidx < 8192 ; ++cidx){
                const Bitmap* bmp = bitmaps[cidx];
                if (bmp){
                    Data& data = this->find(bmp->data());
                    if (!data.bmp){
                        data.bmp = bmp;
                    }
                    data.list.push_back(cidx);
                    ++data.count;
                }
            }
        }

        Data& find(const uint8_t *p)
        {
            for (uint16_t i = 0; i < this->used; ++i){
                if (this->datas[i].bmp->data() == p){
                    return this->datas[i];
                }
            }
            return this->datas[this->used++];
        }

        size_t size() const
        {
            size_t ret = 2;
            for (size_t i = 0; i < this->used; ++i){
                Data &data = this->datas[i];
                ret += 1 + 2 + 2 + 4 + 1 + data.bmp->bmp_size
                + std::distance<>(data.list.begin(), data.list.end()) * 2;
            };
            return ret;
        }

        void send_in(OutFileTransport& trans, Stream& stream)
        {
            stream.init(4096);
            stream.out_uint16_le(this->used);
            trans.send(stream.data, 2);
            for (size_t i = 0; i < this->used; ++i){
                Data &data = this->datas[i];
                {
                    const Bitmap* bmp = data.bmp;
                    stream.init(4096);
                    stream.out_uint8(bmp->original_bpp);
                    stream.out_uint16_le(bmp->cx);
                    stream.out_uint16_le(bmp->cy);
                    stream.out_uint32_le(bmp->bmp_size);
                    stream.out_uint8(data.count);
                    trans.send(stream.data, 10);
                    trans.send((const char*)bmp->data(), bmp->bmp_size);
                }
                stream.init(4096);
                typedef typename list_type::iterator iterator;
                for (iterator it = data.list.begin(),
                    last = data.list.end(); it != last; ++it)
                {
                    stream.out_uint16_le(*it);
                }
                trans.send(stream.data, sizeof(uint16_t) * data.count);
            }
        }
    };*/

    void send_rect(const Rect& rect)
    {
        this->recorder.stream.out_uint16_le(rect.x);
        this->recorder.stream.out_uint16_le(rect.y);
        this->recorder.stream.out_uint16_le(rect.cx);
        this->recorder.stream.out_uint16_le(rect.cy);
    }

    void send_brush(const RDPBrush& brush)
    {
        this->recorder.stream.out_uint8(brush.org_x);
        this->recorder.stream.out_uint8(brush.org_y);
        this->recorder.stream.out_uint8(brush.style);
        this->recorder.stream.out_uint8(brush.hatch);
        this->recorder.stream.out_uint8(brush.extra[0]);
        this->recorder.stream.out_uint8(brush.extra[1]);
        this->recorder.stream.out_uint8(brush.extra[2]);
        this->recorder.stream.out_uint8(brush.extra[3]);
        this->recorder.stream.out_uint8(brush.extra[4]);
        this->recorder.stream.out_uint8(brush.extra[5]);
        this->recorder.stream.out_uint8(brush.extra[6]);
    }

    void send_pen(const RDPPen pen)
    {
        this->recorder.stream.out_uint32_le(pen.color);
        this->recorder.stream.out_uint8(pen.style);
        this->recorder.stream.out_uint8(pen.width);
    }

public:
    void breakpoint(const uint8_t* data_drawable, uint32_t pix_len)
    {
        this->recorder.flush();
        this->recorder.chunk_type = WRMChunk::NEXT_FILE;
        this->recorder.order_count = 1;
        {
            uint32_t len = this->basepath_len + this->next_filename();
            this->recorder.stream.out_uint32_le(len);
            this->recorder.stream.out_copy_bytes(this->basepath, len);
        }
        this->recorder.send_order();

        close(this->trans.fd);
        this->open_file();

        BitmapsSender sender_cache1(this->recorder.bmp_cache.cache[0]);
        BitmapsSender sender_cache2(this->recorder.bmp_cache.cache[1]);
        BitmapsSender sender_cache3(this->recorder.bmp_cache.cache[2]);
        {
            size_t allocate = 8 + MetaWRM::size_for_stream + 9 + 12 + 9 + 28
            + 13 + 17 + 20 + 47 + this->recorder.glyphindex.data_len + 2 + 16
            + 4 + pix_len + sizeof(this->recorder.bmp_cache.stamps) + sender_cache1.size() + sender_cache2.size() + sender_cache3.size();
            this->recorder.init(allocate);
        }

        this->recorder.chunk_type = WRMChunk::BREAKPOINT;
        this->recorder.order_count = 1;

        {
            MetaWRM meta(this->width, this->height, this->bpp);
            meta.out(this->recorder.stream);
            //std::cout << "breakpoint: meta" << this->width << ',' <<  this->height << ',' <<  this->bpp << '\n';
        }

        TODO("use absolute order chunk");

        this->recorder.stream.out_uint8(this->recorder.common.order);
        this->send_rect(this->recorder.common.clip);

        this->recorder.stream.out_uint32_le(this->recorder.opaquerect.color);
        this->send_rect(this->recorder.opaquerect.rect);

        this->recorder.stream.out_uint8(this->recorder.destblt.rop);
        this->send_rect(this->recorder.destblt.rect);

        this->recorder.stream.out_uint8(this->recorder.patblt.rop);
        this->recorder.stream.out_uint32_le(this->recorder.patblt.back_color);
        this->recorder.stream.out_uint32_le(this->recorder.patblt.fore_color);
        this->send_brush(this->recorder.patblt.brush);
        this->send_rect(this->recorder.patblt.rect);

        this->recorder.stream.out_uint8(this->recorder.scrblt.rop);
        this->recorder.stream.out_uint16_le(this->recorder.scrblt.srcx);
        this->recorder.stream.out_uint16_le(this->recorder.scrblt.srcy);
        this->send_rect(this->recorder.scrblt.rect);

        this->recorder.stream.out_uint8(this->recorder.memblt.rop);
        this->recorder.stream.out_uint16_le(this->recorder.memblt.srcx);
        this->recorder.stream.out_uint16_le(this->recorder.memblt.srcy);
        this->recorder.stream.out_uint16_le(this->recorder.memblt.cache_id);
        this->recorder.stream.out_uint16_le(this->recorder.memblt.cache_idx);
        this->send_rect(this->recorder.memblt.rect);

        this->recorder.stream.out_uint8(this->recorder.lineto.rop2);
        this->recorder.stream.out_uint16_le(this->recorder.lineto.startx);
        this->recorder.stream.out_uint16_le(this->recorder.lineto.starty);
        this->recorder.stream.out_uint16_le(this->recorder.lineto.endx);
        this->recorder.stream.out_uint16_le(this->recorder.lineto.endy);
        this->recorder.stream.out_uint8(this->recorder.lineto.back_mode);
        this->recorder.stream.out_uint32_le(this->recorder.lineto.back_color);
        this->send_pen(this->recorder.lineto.pen);

        this->recorder.stream.out_uint32_le(this->recorder.glyphindex.back_color);
        this->recorder.stream.out_uint32_le(this->recorder.glyphindex.fore_color);
        this->recorder.stream.out_uint16_le(this->recorder.glyphindex.f_op_redundant);
        this->recorder.stream.out_uint16_le(this->recorder.glyphindex.fl_accel);
        this->recorder.stream.out_uint16_le(this->recorder.glyphindex.glyph_x);
        this->recorder.stream.out_uint16_le(this->recorder.glyphindex.glyph_y);
        this->recorder.stream.out_uint16_le(this->recorder.glyphindex.ui_charinc);
        this->recorder.stream.out_uint8(this->recorder.glyphindex.cache_id);
        this->recorder.stream.out_uint8(this->recorder.glyphindex.data_len);
        this->send_rect(this->recorder.glyphindex.bk);
        this->send_rect(this->recorder.glyphindex.op);
        this->send_brush(this->recorder.glyphindex.brush);
        this->recorder.stream.out_copy_bytes(this->recorder.glyphindex.data, this->recorder.glyphindex.data_len);

        this->recorder.stream.out_uint16_le(this->recorder.order_count);

        this->recorder.stream.out_uint16_le(this->recorder.bmp_cache.small_entries);
        this->recorder.stream.out_uint16_le(this->recorder.bmp_cache.small_size);
        this->recorder.stream.out_uint16_le(this->recorder.bmp_cache.medium_entries);
        this->recorder.stream.out_uint16_le(this->recorder.bmp_cache.medium_size);
        this->recorder.stream.out_uint16_le(this->recorder.bmp_cache.big_entries);
        this->recorder.stream.out_uint16_le(this->recorder.bmp_cache.big_size);
        this->recorder.stream.out_uint32_le(this->recorder.bmp_cache.stamp);

        this->recorder.stream.out_uint32_le(pix_len);

        this->recorder.stream.out_copy_bytes(data_drawable, pix_len);

        for (size_t cid = 0; cid != 3 ; ++cid){
            const uint32_t (&stamps)[8192] = this->recorder.bmp_cache.stamps[cid];
            for (uint16_t cidx = 0; cidx != 8192; ++cidx){
                this->recorder.stream.out_uint32_le(stamps[cidx]);
            }
        }

        sender_cache1.send_in(this->recorder.stream);
        sender_cache2.send_in(this->recorder.stream);
        sender_cache3.send_in(this->recorder.stream);

        this->recorder.send_order();
        this->recorder.init();
        this->recorder.chunk_type = RDP_UPDATE_ORDERS;
    }
};

#endif
