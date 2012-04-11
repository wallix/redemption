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

#if !defined(__NATIVECAPTURE_HPP__)
#define __NATIVECAPTURE_HPP__

#include <iostream>
#include <stdio.h>
#include "rdtsc.hpp"
#include <sstream>
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

class NativeCapture : public RDPGraphicDevice
{
    public:
    struct timeval start;
    uint64_t inter_frame_interval;
    int width;
    int height;
    int bpp;
    BGRPalette palette;
    OutFileTransport trans;
    GraphicsToFile recorder;
    char basepath[1024];
    uint16_t basepath_len;
    uint32_t nb_file;

private:
    void open_file()
    {
        sprintf(this->basepath + this->basepath_len, "%u.wrm", this->nb_file++);
        LOG(LOG_INFO, "Recording to file : %s", this->basepath);
        this->trans.fd = open(this->basepath, O_WRONLY|O_CREAT, 0666);
        if (this->trans.fd < 0){
            LOG(LOG_ERR, "Error opening native capture file : %s", strerror(errno));
            throw Error(ERR_RECORDER_NATIVE_CAPTURE_OPEN_FAILED);
        }
    }

public:
    NativeCapture(int width, int height, int bpp, const BGRPalette & palette, const char * path)
    : inter_frame_interval(40000) // 1 000 000 us is 1 sec (default)
    , width(width)
    , height(height)
    , bpp(bpp)
    , trans(-1)
    , recorder(&this->trans, NULL, bpp, 8192, 768, 8192, 3072, 8192, 12288)
    , nb_file(0)
    {
        this->basepath_len = sprintf(this->basepath, "%s-%u-", path, getpid());
        this->open_file();
        this->basepath[this->basepath_len] = 0;
    }

    ~NativeCapture(){
        close(this->trans.fd);
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(now, this->start) < this->inter_frame_interval){
            return;
        }
        this->recorder.timestamp(now);
        this->start = now;
    }

    virtual void flush() {}

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

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {}

private:
    template<typename _T>
    void out_copy_bytes(const _T& v)
    {
        this->recorder.stream.out_copy_bytes((const uint8_t*)(&v), sizeof(_T));
    }

public:
// protected:
    void breakpoint()
    {
        this->recorder.flush();
        this->recorder.chunk_type = WRMChunk::NEXT_FILE;
        this->recorder.order_count = 1;
        {
            size_t len = strlen(this->basepath);
            this->out_copy_bytes(len);
            this->recorder.stream.out_copy_bytes(this->basepath, len);
        }
        this->recorder.send_order();
        this->basepath[this->basepath_len] = 0;

        close(this->trans.fd);
        this->open_file();

        {
            size_t size_alloc_stream = 8
            + sizeof(MetaWRM)

            + sizeof(this->recorder.common)
            + sizeof(this->recorder.destblt)
            + sizeof(this->recorder.patblt)
            + sizeof(this->recorder.scrblt)
            + sizeof(this->recorder.opaquerect)
            + sizeof(this->recorder.memblt)
            + sizeof(this->recorder.lineto)
            + sizeof(this->recorder.glyphindex)
            + sizeof(this->recorder.order_count)

            + sizeof(this->recorder.bmp_cache.small_entries)
            + sizeof(this->recorder.bmp_cache.small_size)
            + sizeof(this->recorder.bmp_cache.medium_entries)
            + sizeof(this->recorder.bmp_cache.medium_size)
            + sizeof(this->recorder.bmp_cache.big_entries)
            + sizeof(this->recorder.bmp_cache.big_size)
            + sizeof(this->recorder.bmp_cache.stamps)
            + sizeof(this->recorder.bmp_cache.stamp)

            + 3 * 8192 * sizeof(bool)
            + this->recorder.glyphindex.data_len;

            for (size_t cid = 0; cid < 3 ; cid++){
                for (size_t cidx = 0; cidx < 8192 ; cidx++){
                    const Bitmap* bmp = this->recorder.bmp_cache.cache[cid][cidx];
                    if (bmp){
                        size_alloc_stream += sizeof(bmp->original_bpp)
                        + sizeof(bmp->cx)
                        + sizeof(bmp->cy)
                        + sizeof(bmp->line_size)
                        + sizeof(bmp->bmp_size)
                        + bmp->bmp_size;
                    }
                }
            }
            this->recorder.init(size_alloc_stream);
        }

        this->recorder.chunk_type = WRMChunk::BREAKPOINT;
        this->recorder.order_count = 1;

        {
            MetaWRM meta(this->width, this->height, this->bpp);
            meta.send(this->recorder);
            this->out_copy_bytes(meta);
        }

        this->out_copy_bytes(this->recorder.common);
        this->out_copy_bytes(this->recorder.destblt);
        this->out_copy_bytes(this->recorder.patblt);
        this->out_copy_bytes(this->recorder.scrblt);
        this->out_copy_bytes(this->recorder.opaquerect);
        this->out_copy_bytes(this->recorder.memblt);
        this->out_copy_bytes(this->recorder.lineto);
        this->out_copy_bytes(this->recorder.glyphindex);
        this->recorder.stream.out_copy_bytes(this->recorder.glyphindex.data,
                                             this->recorder.glyphindex.data_len);
        this->out_copy_bytes(this->recorder.order_count);

        this->out_copy_bytes(this->recorder.bmp_cache.small_entries);
        this->out_copy_bytes(this->recorder.bmp_cache.small_size);
        this->out_copy_bytes(this->recorder.bmp_cache.medium_entries);
        this->out_copy_bytes(this->recorder.bmp_cache.medium_size);
        this->out_copy_bytes(this->recorder.bmp_cache.big_entries);
        this->out_copy_bytes(this->recorder.bmp_cache.big_size);
        this->out_copy_bytes(this->recorder.bmp_cache.stamps);
        this->out_copy_bytes(this->recorder.bmp_cache.stamp);

        for (size_t cid = 0; cid < 3 ; cid++){
            for (size_t cidx = 0; cidx < 8192 ; cidx++){
                const Bitmap* bmp = this->recorder.bmp_cache.cache[cid][cidx];
                if (bmp){
                    this->recorder.stream.out_uint8(1);
                    this->out_copy_bytes(bmp->original_bpp);
                    this->out_copy_bytes(bmp->cx);
                    this->out_copy_bytes(bmp->cy);
                    this->out_copy_bytes(bmp->line_size);
                    this->out_copy_bytes(bmp->bmp_size);
                    this->recorder.stream.out_copy_bytes(bmp->data(), bmp->bmp_size);
                }
                else{
                    this->recorder.stream.out_uint8(0);
                }
            }
        }

        this->recorder.send_order();
        this->recorder.chunk_type = RDP_UPDATE_ORDERS;
        this->recorder.init();
    }
};

#endif
