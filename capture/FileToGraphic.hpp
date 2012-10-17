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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Jonathan Poelen

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#if !defined(__FILE_TO_GRAPHIC_HPP__)
#define __FILE_TO_GRAPHIC_HPP__

#include <sys/time.h>
#include <ctime>

#include "transport.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/RDPSerializer.hpp"
#include "difftimeval.hpp"
#include "meta_file.hpp"

struct RDPUnserializer
{
    enum {
        HEADER_SIZE = 8,
    };
    BStream stream;

//    uint8_t padding[65536];

    RDPGraphicDevice * consumer;
    Transport * trans;

    TODO("This should be extracted from serialized data. Serializer should have some API function to set geometry width x height x bpp saved in native movie.")
    Rect screen_rect;

    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;

    BmpCache bmp_cache;

    // variables used to read batch of orders "chunks"
    uint16_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;
    uint16_t chunk_flags;
    uint16_t remaining_order_count;

    timeval timer_cap;
    uint64_t movie_usec;

    DataMetaFile data_meta;

    RDPUnserializer(Transport * trans, const timeval & now, RDPGraphicDevice * consumer, const Rect screen_rect)
     : stream(4096), consumer(consumer), trans(trans), screen_rect(screen_rect),
     // Internal state of orders
    common(RDP::PATBLT, Rect(0, 0, 1, 1)),
    destblt(Rect(), 0),
    patblt(Rect(), 0, 0, 0, RDPBrush()),
    scrblt(Rect(), 0, 0, 0),
    opaquerect(Rect(), 0),
    memblt(0, Rect(), 0, 0, 0, 0),
    lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
    glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),

    bmp_cache(24),

    // variables used to read batch of orders "chunks"
    chunk_size(0),
    chunk_type(0),
    chunk_count(0),
    chunk_flags(0),
    remaining_order_count(0),
    timer_cap(now),
    movie_usec(0),

    data_meta()
    {
    }

    bool next_order()
    REDOC("order count set this->stream.p to the beginning of the next order."
          "Most of the times it means not changing it, except when it must read next chunk"
          "when remaining order count is 0."
          "It update chunk headers (merely remaining orders count) and"
          " reads the next chunk if necessary.") 
    {
            this->remaining_order_count, (unsigned)(stream.p - stream.data), (unsigned)(stream.end-stream.p));
        if ((this->stream.p == this->stream.end) && (this->remaining_order_count)){
            LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                         "order [%u/%u] "
                         "remaining [%u/%u]",
                         this->chunk_type,
                         (this->chunk_count-this->remaining_order_count), this->chunk_count,
                         (this->stream.end - this->stream.p), this->chunk_size);
            return false;
        }
        if ((this->stream.p != this->stream.end) && (this->remaining_order_count == 0)){
            LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                         "order [%u/%u] "
                         "remaining [%u/%u]",
                         this->chunk_type,
                         (this->chunk_count-this->remaining_order_count), this->chunk_count,
                         (this->stream.end - this->stream.p), this->chunk_size);
            return false;
        }


        if (!this->remaining_order_count){
            try {
                BStream header(HEADER_SIZE);
                this->trans->recv(&header.end, HEADER_SIZE);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint16_le();
                this->remaining_order_count = this->chunk_count = header.in_uint16_le();
                this->chunk_flags = header.in_uint16_le();
                LOG(LOG_INFO, "reading chunk: type=%u size=%u count=%u flags=%u\n", 
                    this->chunk_type, this->chunk_size, this->chunk_count, this->chunk_flags);
                this->stream.init(this->chunk_size - HEADER_SIZE);
                this->trans->recv(&this->stream.end, this->chunk_size - HEADER_SIZE);
            }
            catch (Error & e){
                // receive error, end of transport
                return false;
            }
        }
        if (this->remaining_order_count > 0){this->remaining_order_count--;}
        return true;
    }

    void interpret_order()
    {
        switch (this->chunk_type){
        case RDP_UPDATE_ORDERS:
        {
            uint8_t control = this->stream.in_uint8();
            if (!control & RDP::STANDARD){
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                TODO(" throw some error")
            }
            else if (control & RDP::SECONDARY) {
                using namespace RDP;
                RDPSecondaryOrderHeader header(this->stream);
                uint8_t *next_order = this->stream.p + header.length + 7;
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                {
                    // we need color depth and palette
                    RDPBmpCache cmd;
                    BGRPalette palette;
                    init_palette332(palette);
                    cmd.receive(this->stream, control, header, palette);
                    this->bmp_cache.put(cmd.id, cmd.idx, cmd.bmp);
                }
                break;
                case TS_CACHE_COLOR_TABLE:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_COLOR_TABLE (%d)", header.type);
//                    this->process_colormap(this->stream, control, header, mod);
                    break;
                case TS_CACHE_GLYPH:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_GLYPH (%d)", header.type);
//                    this->rdp_orders_process_fontcache(this->stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)", header.type);
                  break;
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)", header.type);
                  break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)", header.type);
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(this->stream, control);
                const Rect & clip = (control & RDP::BOUNDS)?this->common.clip:this->screen_rect;
                switch (this->common.order) {
                case RDP::GLYPHINDEX:
                    this->glyphindex.receive(this->stream, header);
                    consumer->draw(this->glyphindex, clip);
                    break;
                case RDP::DESTBLT:
                    this->destblt.receive(this->stream, header);
                    consumer->draw(this->destblt, clip);
                    break;
                case RDP::PATBLT:
                    this->patblt.receive(this->stream, header);
                    consumer->draw(this->patblt, clip);
                    break;
                case RDP::SCREENBLT:
                    this->scrblt.receive(this->stream, header);
                    consumer->draw(this->scrblt, clip);
                    break;
                case RDP::LINE:
                    this->lineto.receive(this->stream, header);
                    consumer->draw(this->lineto, clip);
                    break;
                case RDP::RECT:
                    this->opaquerect.receive(this->stream, header);
                    consumer->draw(this->opaquerect, clip);
                    break;
                case RDP::MEMBLT:
                    {
                        this->memblt.receive(this->stream, header);
                        const Bitmap * bmp = this->bmp_cache.get(this->memblt.cache_id, this->memblt.cache_idx);
                        if (!bmp){
                            LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)", this->memblt.cache_id, this->memblt.cache_idx);
                        }
                        else {
                            this->consumer->draw(this->memblt, clip, *bmp);
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
            }
            }
            break;
            case WRMChunk::TIMESTAMP:
            {
                if (!this->movie_usec){
                    LOG(LOG_INFO, "chunk timestamp reading first timestamp");
                    this->movie_usec = this->stream.in_uint64_le();
                }
                else {
                    LOG(LOG_INFO, "chunk timestamp reading other timestamps");
                    uint64_t last_movie_usec = this->movie_usec;
                    this->movie_usec = this->stream.in_uint64_le();
                    uint64_t movie_elapsed = movie_usec - last_movie_usec;
                    struct timeval now;
                    gettimeofday(&now, 0);
                    uint64_t elapsed = difftimeval(now, this->timer_cap);
                    this->timer_cap = now;
                    
                    LOG(LOG_INFO, "elapsed=%lu movie_elapsed=%lu\n", elapsed, movie_elapsed);
                    
                    if (elapsed <= movie_elapsed){
                        struct timespec wtime =
                            { static_cast<uint32_t>((movie_elapsed - elapsed) / 1000000)
                            , static_cast<uint32_t>((movie_elapsed - elapsed) % 1000000 * 1000)
                            };
                        nanosleep(&wtime, NULL);
                    }
                }
            }
            break;
            case WRMChunk::META_FILE:
            {
                // DATA in metafile:
                // 4 bytes   : len
                // len bytes : reference filename
                // 2 bytes width
                // 2 bytes height
                // 1 bytes BPP (bytes per plane)
                LOG(LOG_INFO, "META");
                if (this->data_meta.loaded)
                {
                    LOG(LOG_INFO, "ignore chunk type META_FILE");
                }
                else
                {
                    uint32_t len = this->stream.in_uint32_le();
                    this->stream.p[len] = 0;
                    if (!read_meta_file(this->data_meta, reinterpret_cast<const char*>(this->stream.p)))
                    {
                        LOG(LOG_ERR, "meta %s: %s", reinterpret_cast<const char*>(this->stream.p), strerror(errno));
                        throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errno);
                    }
                    this->screen_rect.cx = this->data_meta.width;
                    this->screen_rect.cy = this->data_meta.height;
                }
                this->stream.p = this->stream.end;
            }
            break;
            default:
                LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
            break;
        }
    }
};

#endif
