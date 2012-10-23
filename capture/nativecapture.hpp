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

#include <errno.h>
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
#include "RDP/caches/bmpcache.hpp"
#include "colors.hpp"
#include "stream.hpp"

#include "GraphicToFile.hpp"
#include "png.hpp"
#include "auto_buffer.hpp"
#include "cipher_transport.hpp"

#include "zlib.hpp"

class NativeCapture : public RDPGraphicDevice
{
public:
    int width;
    int height;
    int bpp;

    uint64_t frame_interval;
    struct timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    struct timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    BStream stream;
    GraphicToFile recorder;
    uint32_t nb_file;

    NativeCapture(const timeval & now, Transport & trans, FileSequence & sequence, int width, int height)
    : width(width)
    , height(height)
    , bpp(24)
    , stream(65536)
    , recorder(&trans, &this->stream, NULL, width, height, 24, 8192, 768, 8192, 3072, 8192, 12288, now)
    , nb_file(0)
    {
        // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
        this->start_native_capture = now;
        this->frame_interval = 40;
        this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec

        this->start_break_capture = now;
        this->break_interval = 60 * 10; // break interval is in s, default value 1 break every 10 minutes
        this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec

        LOG(LOG_INFO, "update configuration frame_interval=%u break_interval=%u",
            this->frame_interval, this->break_interval);
    }

    ~NativeCapture(){
        this->recorder.flush();
    }

    void update_config(const Inifile & ini)
    {
        if (ini.globals.frame_interval != this->frame_interval){
            // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
            this->frame_interval = ini.globals.frame_interval;
            this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec
        }

        if (ini.globals.break_interval != this->break_interval){
            this->break_interval = ini.globals.break_interval; // break interval is in s, default value 1 break every 10 minutes
            this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec
        }
        LOG(LOG_INFO, "update configuration frame_interval=%u break_interval=%u",
            this->frame_interval, this->break_interval);
    }
        
    void snapshot(const timeval & now, int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        if (difftimeval(now, this->start_native_capture) >= this->inter_frame_interval_native_capture){
            LOG(LOG_INFO, "recorder timestamp");
            this->recorder.timestamp(now);
            this->start_native_capture = now;
            if (difftimeval(now, this->start_break_capture) >= this->inter_frame_interval_start_break_capture){
//                this->breakpoint(now);
                this->start_break_capture = now;
            }
        }
        this->recorder.flush();

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

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

private:
    void send_rect(const Rect& rect)
    {
        this->stream.out_uint16_le(rect.x);
        this->stream.out_uint16_le(rect.y);
        this->stream.out_uint16_le(rect.cx);
        this->stream.out_uint16_le(rect.cy);
    }

    void send_brush(const RDPBrush& brush)
    {
        this->stream.out_uint8(brush.org_x);
        this->stream.out_uint8(brush.org_y);
        this->stream.out_uint8(brush.style);
        this->stream.out_uint8(brush.hatch);
        this->stream.out_uint8(brush.extra[0]);
        this->stream.out_uint8(brush.extra[1]);
        this->stream.out_uint8(brush.extra[2]);
        this->stream.out_uint8(brush.extra[3]);
        this->stream.out_uint8(brush.extra[4]);
        this->stream.out_uint8(brush.extra[5]);
        this->stream.out_uint8(brush.extra[6]);
    }

    void send_pen(const RDPPen pen)
    {
        this->stream.out_uint32_le(pen.color);
        this->stream.out_uint8(pen.style);
        this->stream.out_uint8(pen.width);
    }

public:
    void breakpoint(const uint8_t* data_drawable, uint8_t bpp,
                    uint16_t width, uint16_t height, size_t rowsize,
                    const timeval& now)
    {
//        this->recorder.timestamp(now);
//        this->breakpoint(this->psc->drawable.data,
//                            24,
//                            this->psc->drawable.width,
//                            this->psc->drawable.height,
//                            this->psc->drawable.rowsize,
//                            now);

//        this->recorder.flush();

//        this->recorder.chunk_type = WRMChunk::NEXT_FILE_ID;
//        this->recorder.chunk_count = 1;
//        this->stream.out_uint32_le(this->nb_file);
//        this->filename_len = this->basepath_len + sprintf(this->filename + this->basepath_len, "%u.wrm", this->nb_file++);
//        this->recorder.flush();
//        close(this->trans.fd);
//        LOG(LOG_INFO, "Open to file : %s", this->filename);
//        this->trans.fd = open(this->filename, O_WRONLY|O_CREAT, 0666);
//        if (this->trans.fd < 0){
//            LOG(LOG_ERR, "Error opening native capture file : %s", strerror(errno));
//            throw Error(ERR_NATIVE_CAPTURE_OPEN_FAILED);
//        }


//        this->recorder.chunk_type = WRMChunk::META_FILE;
//        this->recorder.chunk_count = 1;
//        {
//            this->stream.out_uint32_le(this->meta_name_len);
//            this->stream.out_copy_bytes(this->meta_name, this->meta_name_len);
//        }
//        this->recorder.flush();

//        this->recorder.chunk_type = WRMChunk::BREAKPOINT;
//        this->recorder.chunk_count = 1;
//        this->stream.out_uint16_le(this->width);
//        this->stream.out_uint16_le(this->height);
//        this->stream.out_uint8(this->bpp);
//        this->stream.out_uint64_le(this->recorder.timer.tv_sec);
//        this->stream.out_uint64_le(this->recorder.timer.tv_usec);
//        this->recorder.send_order();

//        // write screen
//        {
//            this->filename[this->filename_len] = '.';
//            this->filename[this->filename_len+1] = 'p';
//            this->filename[this->filename_len+2] = 'n';
//            this->filename[this->filename_len+3] = 'g';
//            this->filename[this->filename_len+4] = 0;
//            if (std::FILE* fd = std::fopen(this->filename, "w+"))
//            {
//                dump_png24(fd, data_drawable, width, height, rowsize);
//                fclose(fd);
//            }
//            else
//            {
//                LOG(LOG_ERR, "Error opening context file : %s", strerror(errno));
//                throw Error(ERR_NATIVE_CAPTURE_OPEN_FAILED);
//            }
//            this->filename[this->filename_len] = 0;
//        }

//        this->recorder.init();
//        this->recorder.chunk_count = 1;
//        this->recorder.chunk_type = WRMChunk::BREAKPOINT;

//        this->stream.out_uint8(this->recorder.common.order);
//        this->send_rect(this->recorder.common.clip);

//        this->stream.out_uint32_le(this->recorder.opaquerect.color);
//        this->send_rect(this->recorder.opaquerect.rect);

//        this->stream.out_uint8(this->recorder.destblt.rop);
//        this->send_rect(this->recorder.destblt.rect);

//        this->stream.out_uint8(this->recorder.patblt.rop);
//        this->stream.out_uint32_le(this->recorder.patblt.back_color);
//        this->stream.out_uint32_le(this->recorder.patblt.fore_color);
//        this->send_brush(this->recorder.patblt.brush);
//        this->send_rect(this->recorder.patblt.rect);

//        this->stream.out_uint8(this->recorder.scrblt.rop);
//        this->stream.out_uint16_le(this->recorder.scrblt.srcx);
//        this->stream.out_uint16_le(this->recorder.scrblt.srcy);
//        this->send_rect(this->recorder.scrblt.rect);

//        this->stream.out_uint8(this->recorder.memblt.rop);
//        this->stream.out_uint16_le(this->recorder.memblt.srcx);
//        this->stream.out_uint16_le(this->recorder.memblt.srcy);
//        this->stream.out_uint16_le(this->recorder.memblt.cache_id);
//        this->stream.out_uint16_le(this->recorder.memblt.cache_idx);
//        this->send_rect(this->recorder.memblt.rect);

//        this->stream.out_uint8(this->recorder.lineto.rop2);
//        this->stream.out_uint16_le(this->recorder.lineto.startx);
//        this->stream.out_uint16_le(this->recorder.lineto.starty);
//        this->stream.out_uint16_le(this->recorder.lineto.endx);
//        this->stream.out_uint16_le(this->recorder.lineto.endy);
//        this->stream.out_uint8(this->recorder.lineto.back_mode);
//        this->stream.out_uint32_le(this->recorder.lineto.back_color);
//        this->send_pen(this->recorder.lineto.pen);

//        this->stream.out_uint32_le(this->recorder.glyphindex.back_color);
//        this->stream.out_uint32_le(this->recorder.glyphindex.fore_color);
//        this->stream.out_uint16_le(this->recorder.glyphindex.f_op_redundant);
//        this->stream.out_uint16_le(this->recorder.glyphindex.fl_accel);
//        this->stream.out_uint16_le(this->recorder.glyphindex.glyph_x);
//        this->stream.out_uint16_le(this->recorder.glyphindex.glyph_y);
//        this->stream.out_uint16_le(this->recorder.glyphindex.ui_charinc);
//        this->stream.out_uint8(this->recorder.glyphindex.cache_id);
//        this->stream.out_uint8(this->recorder.glyphindex.data_len);
//        this->send_rect(this->recorder.glyphindex.bk);
//        this->send_rect(this->recorder.glyphindex.op);
//        this->send_brush(this->recorder.glyphindex.brush);
//        this->stream.out_copy_bytes(this->recorder.glyphindex.data, this->recorder.glyphindex.data_len);

//        this->stream.out_uint16_le(this->recorder.chunk_count);

//        this->stream.out_uint16_le(this->recorder.bmp_cache.small_entries);
//        this->stream.out_uint16_le(this->recorder.bmp_cache.small_size);
//        this->stream.out_uint16_le(this->recorder.bmp_cache.medium_entries);
//        this->stream.out_uint16_le(this->recorder.bmp_cache.medium_size);
//        this->stream.out_uint16_le(this->recorder.bmp_cache.big_entries);
//        this->stream.out_uint16_le(this->recorder.bmp_cache.big_size);
//        this->stream.out_uint32_le(this->recorder.bmp_cache.stamp);

//        this->recorder.send_order();

//        this->stream.init(14);
//        this->recorder.chunk_type = WRMChunk::BREAKPOINT;
//        AutoBuffer buffer;
//        z_stream zstrm;
//        zstrm.zalloc = 0;
//        zstrm.zfree = 0;
//        zstrm.opaque = 0;
//        int ret;
//        const int Bpp = 3;
//        for (size_t cid = 0; cid != 3 ; ++cid){
//            const Bitmap* (&bitmaps)[8192] = this->recorder.bmp_cache.cache[cid];
//            const uint32_t (&stamps)[8192] = this->recorder.bmp_cache.stamps[cid];

//            for (uint16_t cidx = 0; cidx < 8192 ; ++cidx){
//                if (bitmaps[cidx]){
//                    this->stream.out_uint16_le(8192 * cid + cidx);
//                    this->stream.out_uint32_le(stamps[cidx]);
//                    this->stream.out_uint16_le(bitmaps[cidx]->cx);
//                    this->stream.out_uint16_le(bitmaps[cidx]->cy);

//                    if ((ret = deflateInit(&zstrm, Z_DEFAULT_COMPRESSION)) != Z_OK)
//                    {
//                        LOG(LOG_ERR, "zlib: deflateInit: %d", ret);
//                        throw Error(ERR_NATIVE_CAPTURE_ZIP_COMPRESS);
//                    }
//                    uint16_t y = 1;
//                    int flush;
//                    /*const*/ uint8_t *src = bitmaps[cidx]->data_bitmap.get();
//                    uint size_x = bitmaps[cidx]->cx * Bpp;
//                    uLong destlen = compressBound(size_x * bitmaps[cidx]->cy);
//                    buffer.alloc(destlen);

//                    do {
//                        zstrm.next_in = src;
//                        zstrm.avail_in = size_x;
//                        zstrm.next_out = buffer.get() + zstrm.total_out;
//                        zstrm.avail_out = destlen - zstrm.total_out;
//                        flush = y == bitmaps[cidx]->cy ? Z_FINISH : Z_NO_FLUSH;
//                        if ((ret = deflate(&zstrm, flush)) == Z_STREAM_ERROR)
//                        {
//                            deflateEnd(&zstrm);
//                            LOG(LOG_ERR, "zlib: deflate: %d", ret);
//                            throw Error(ERR_NATIVE_CAPTURE_ZIP_COMPRESS);
//                        }
//                        ++y;
//                        src += bitmaps[cidx]->line_size;
//                    } while (flush != Z_FINISH);
//                    deflateEnd(&zstrm);
//                    this->stream.out_uint32_le(zstrm.total_out);
//                    this->recorder.trans->send(this->stream.data, 14);
//                    this->recorder.trans->send(buffer.get(), zstrm.total_out);
//                    this->stream.p = this->stream.data;
//                }
//            }
//        }
//        this->stream.out_uint16_le(8192 * 3 + 1);
//        this->stream.out_uint32_le(0);
//        this->stream.out_uint16_le(0);
//        this->stream.out_uint16_le(0);
//        this->stream.out_uint32_le(0);
//        this->recorder.trans->send(this->stream.data, 14);

//        this->recorder.init();
//        this->recorder.timer = now;
//        this->recorder.chunk_type = RDP_UPDATE_ORDERS;
    }
};

#endif
