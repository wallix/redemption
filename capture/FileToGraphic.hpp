/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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

#ifndef _REDEMPTION_CAPTURE_FILETOGRAPHIC_HPP_
#define _REDEMPTION_CAPTURE_FILETOGRAPHIC_HPP_

#include "transport.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/RDPDrawable.hpp"
#include "RDP/RDPSerializer.hpp"
#include "difftimeval.hpp"

#include "chunked_image_transport.hpp"

struct FileToGraphic
{
    enum {
        HEADER_SIZE = 8,
    };
    BStream stream;

    Transport * trans;

    Rect screen_rect;

    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPMem3Blt mem3blt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;

    BmpCache * bmp_cache;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;
    uint16_t remaining_order_count;

    // total number of RDP orders read from the start of the movie
    // (non orders chunks are counted as 1 order)
    uint32_t total_orders_count;

    timeval synctime_now;
    timeval record_now;

    uint16_t nbconsumers;
    RDPGraphicDevice * consumers[10];

    bool meta_ok;
    bool timestamp_ok;
    uint16_t mouse_x;
    uint16_t mouse_y;
    uint16_t input_len;
    uint8_t  input[8192];
    bool real_time;

    BGRPalette palette;

    timeval begin_capture;
    timeval end_capture;
    uint32_t max_order_count;
    uint32_t verbose;

    bool mem3blt_support;

    uint16_t info_version;
    uint16_t info_width;
    uint16_t info_height;
    uint16_t info_bpp;
    uint16_t info_small_entries;
    uint16_t info_small_size;
    uint16_t info_medium_entries;
    uint16_t info_medium_size;
    uint16_t info_big_entries;
    uint16_t info_big_size;

    FileToGraphic(Transport * trans, const timeval begin_capture, const timeval end_capture, bool real_time, uint32_t verbose)
        : stream(65536)
        , trans(trans)
        , common(RDP::PATBLT, Rect(0, 0, 1, 1))
        , destblt(Rect(), 0)
        , patblt(Rect(), 0, 0, 0, RDPBrush())
        , scrblt(Rect(), 0, 0, 0)
        , opaquerect(Rect(), 0)
        , memblt(0, Rect(), 0, 0, 0, 0)
        , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
        , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
        , glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"")
        , bmp_cache(NULL)
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(0)
        , chunk_count(0)
        , remaining_order_count(0)
        , total_orders_count(0)
        , nbconsumers(0)
        , meta_ok(false)
        , timestamp_ok(false)
        , mouse_x(0)
        , mouse_y(0)
        , input_len(0)
        , real_time(real_time)
        , begin_capture(begin_capture)
        , end_capture(end_capture)
        , max_order_count(0)
        , verbose(verbose)
        , mem3blt_support(false)
        , info_version(0)
        , info_width(0)
        , info_height(0)
        , info_bpp(0)
        , info_small_entries(0)
        , info_small_size(0)
        , info_medium_entries(0)
        , info_medium_size(0)
        , info_big_entries(0)
        , info_big_size(0)
    {
        init_palette332(this->palette); // We don't really care movies are always 24 bits for now

        while (this->next_order()){
            this->interpret_order();
            if (this->meta_ok && this->timestamp_ok){
                break;
            }
        }
    }

    ~FileToGraphic()
    {
        delete this->bmp_cache;
    }

    void add_consumer(RDPGraphicDevice * consumer)
    {
        this->consumers[this->nbconsumers++] = consumer;
    }

    bool next_order()
    REDOC("order count set this->stream.p to the beginning of the next order."
          "Most of the times it means not changing it, except when it must read next chunk"
          "when remaining order count is 0."
          "It update chunk headers (merely remaining orders count) and"
          " reads the next chunk if necessary.")
    {
        if (this->chunk_type != LAST_IMAGE_CHUNK
        && this->chunk_type != PARTIAL_IMAGE_CHUNK){
            if ((this->stream.p == this->stream.end)
            && (this->remaining_order_count)){
                LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                             "order [%u/%u] "
                             "remaining [%u/%u]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             (this->stream.end - this->stream.p), this->chunk_size);
                return false;
            }
            if ((this->stream.p != this->stream.end)
            && (this->remaining_order_count == -1)){
                LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                             "order [%u/%u] "
                             "remaining [%u/%u]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             (this->stream.end - this->stream.p), this->chunk_size);
                return false;
            }
        }

        if (!this->remaining_order_count){
            try {
                BStream header(HEADER_SIZE);
                this->trans->recv(&header.end, HEADER_SIZE);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->remaining_order_count = this->chunk_count = header.in_uint16_le();

                if (this->chunk_type != LAST_IMAGE_CHUNK && this->chunk_type != PARTIAL_IMAGE_CHUNK){
                    if (this->chunk_size > 65536){
                        LOG(LOG_INFO,"chunk_size (%d) > 65536", this->chunk_size);
                        return false;
                    }
                    this->stream.reset();
                    this->trans->recv(&this->stream.end, this->chunk_size - HEADER_SIZE);
                }
            }
            catch (Error & e){
                if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                    throw;
                }

                LOG(LOG_INFO,"receive error %u : end of transport", e.id);
                // receive error, end of transport
                return false;
            }
        }
        if (this->remaining_order_count > 0){this->remaining_order_count--;}
        return true;
    }

    void interpret_order()
    {
        this->total_orders_count++;
        switch (this->chunk_type){
        case RDP_UPDATE_ORDERS:
        {
            if (!this->meta_ok){
                LOG(LOG_ERR,"Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                throw Error(ERR_WRM);
            }
            if (!this->timestamp_ok){
                LOG(LOG_ERR,"Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing\n");
                throw Error(ERR_WRM);
            }
            uint8_t control = this->stream.in_uint8();
            if (!control & RDP::STANDARD){
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                throw Error(ERR_WRM);
            }
            else if (control & RDP::SECONDARY) {
                using namespace RDP;
                RDPSecondaryOrderHeader header(this->stream);
                uint8_t *next_order = this->stream.p + header.length + 7;
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                {
                    RDPBmpCache cmd;
                    cmd.receive(this->stream, control, header, this->palette);
                    if (this->verbose > 32){
                        cmd.log(LOG_INFO);
                    }
                    this->bmp_cache->put(cmd.id, cmd.idx, cmd.bmp);
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
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->glyphindex, clip);
                    }
                    break;
                case RDP::DESTBLT:
                    this->destblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->destblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->destblt, clip);
                    }
                    break;
                case RDP::PATBLT:
                    this->patblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->patblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->patblt, clip);
                    }
                    break;
                case RDP::SCREENBLT:
                    this->scrblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->scrblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->scrblt, clip);
                    }
                    break;
                case RDP::LINE:
                    this->lineto.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->lineto.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->lineto, clip);
                    }
                    break;
                case RDP::RECT:
                    this->opaquerect.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->opaquerect.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers ; i++){
                        this->consumers[i]->draw(this->opaquerect, clip);
                    }
                    break;
                case RDP::MEMBLT:
                    {
                        this->memblt.receive(this->stream, header);
                        if (this->verbose > 32){
                            this->memblt.log(LOG_INFO, clip);
                        }
                        const Bitmap * bmp = this->bmp_cache->get(this->memblt.cache_id, this->memblt.cache_idx);
                        if (!bmp){
                            LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)", this->memblt.cache_id, this->memblt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (size_t i = 0; i < this->nbconsumers ; i++){
                                this->consumers[i]->draw(this->memblt, clip, *bmp);
                            }
                        }
                    }
                    break;
                case RDP::MEM3BLT:
                    {
                        this->mem3blt.receive(this->stream, header);
                        if (this->verbose > 32){
                            this->mem3blt.log(LOG_INFO, clip);
                        }
                        const Bitmap * bmp = this->bmp_cache->get(this->mem3blt.cache_id, this->mem3blt.cache_idx);
                        if (!bmp){
                            LOG(LOG_ERR, "Mem3blt bitmap not found in cache at (%u, %u)", this->mem3blt.cache_id, this->mem3blt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (size_t i = 0; i < this->nbconsumers ; i++){
                                this->consumers[i]->draw(this->mem3blt, clip, *bmp);
                            }
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    throw Error(ERR_WRM);
                }
            }
            }
            break;
            case TIMESTAMP:
            {
                timeval last_movie_time = this->record_now;
                this->stream.in_timeval_from_uint64le_usec(this->record_now);

                REDOC("If some data remains, it is input data : mouse_x, mouse_y and decoded keyboard keys (utf8)")
                if (this->stream.end - this->stream.p > 0){
                    if (this->stream.end - this->stream.p < 4){
                        LOG(LOG_WARNING, "Input data truncated");
                        hexdump_d(stream.p, stream.end - stream.p);
                    }

                    this->mouse_x = this->stream.in_uint16_le();
                    this->mouse_y = this->stream.in_uint16_le();

                    if (this->verbose > 16){
                        LOG(LOG_INFO, "TIMESTAMP %u.%u mouse (x=%u, y=%u)\n"
                            , this->record_now.tv_sec
                            , this->record_now.tv_usec
                            , this->mouse_x
                            , this->mouse_y);
                    }

                    this->input_len = std::min(static_cast<uint16_t>(stream.end - stream.p), static_cast<uint16_t>(sizeof(this->input)-1));
                    if (this->input_len){
                        this->stream.in_copy_bytes(this->input, this->input_len);
                        this->input[this->input_len] = 0;
                        this->stream.p = this->stream.end;

                        StaticStream ss(this->input, this->input_len);

                        for (size_t i = 0; i < this->nbconsumers ; i++){
                            this->consumers[i]->input(this->record_now, ss);
                        }

                        if (this->verbose > 16) {
                            uint32_t key;

                            while (ss.in_check_rem(sizeof(uint32_t))) {
                                key = ss.in_uint32_le();

                                LOG(LOG_INFO, "TIMESTAMP %u.%u keyboard '%c'(0x%X)"
                                    , this->record_now.tv_sec
                                    , this->record_now.tv_usec
                                    , key
                                    , key);
                            }
                        }
                    }
                }

                if (!this->timestamp_ok){
                   if (this->real_time) {
                        this->synctime_now = tvtime();
                    }
                    else {
                        this->synctime_now = this->record_now;
                    }
                    this->timestamp_ok = true;
                }
                else {
                   if (this->real_time){
                        struct timeval now = tvtime();
                        uint64_t elapsed = difftimeval(now, this->synctime_now);
                        this->synctime_now = now;
                        uint64_t movie_elapsed = difftimeval(this->record_now, last_movie_time);

                        if (elapsed <= movie_elapsed){
                            struct timespec wtime =
                                { static_cast<uint32_t>((movie_elapsed - elapsed) / 1000000LL)
                                , static_cast<uint32_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                                };
                            nanosleep(&wtime, NULL);
                        }
                    }
                    else {
                        this->synctime_now = this->record_now;
                    }
                }
            }
            break;
            case META_FILE:
            TODO("meta should contain some WRM version identifier")
            TODO("Cache meta_data (sizes, number of entries) should be put in META chunk")
            {
                this->info_version        = this->stream.in_uint16_le();
                (void)this->info_version; // for now there is only one, we do not yet have problems
                this->mem3blt_support     = (this->info_version > 1);
                this->info_width          = this->stream.in_uint16_le();
                this->info_height         = this->stream.in_uint16_le();
                this->info_bpp            = this->stream.in_uint16_le();
                this->info_small_entries  = this->stream.in_uint16_le();
                this->info_small_size     = this->stream.in_uint16_le();
                this->info_medium_entries = this->stream.in_uint16_le();
                this->info_medium_size    = this->stream.in_uint16_le();
                this->info_big_entries    = this->stream.in_uint16_le();
                this->info_big_size       = this->stream.in_uint16_le();

                this->stream.p = this->stream.end;

                if (!this->meta_ok){
                    this->bmp_cache = new BmpCache(this->info_bpp, this->info_small_entries,
                        this->info_small_size, this->info_medium_entries, this->info_medium_size,
                         this->info_big_entries, this->info_big_size);
                    this->screen_rect = Rect(0, 0, this->info_width, this->info_height);
                    this->meta_ok = true;
                }
                else {
                    if (this->screen_rect.cx != this->info_width ||
                        this->screen_rect.cy != this->info_height){
                        LOG(LOG_ERR,"Inconsistant redundant meta chunk");
                        throw Error(ERR_WRM);
                    }
                }
            }
            break;
            case SAVE_STATE:
                // RDPOrderCommon common;
                this->common.order = this->stream.in_uint8();
                this->common.clip.x = this->stream.in_uint16_le();
                this->common.clip.y = this->stream.in_uint16_le();
                this->common.clip.cx = this->stream.in_uint16_le();
                this->common.clip.cy = this->stream.in_uint16_le();

                // RDPDestBlt destblt;
                this->destblt.rect.x = this->stream.in_uint16_le();
                this->destblt.rect.y = this->stream.in_uint16_le();
                this->destblt.rect.cx = this->stream.in_uint16_le();
                this->destblt.rect.cy = this->stream.in_uint16_le();
                this->destblt.rop = this->stream.in_uint8();

                // RDPPatBlt patblt;
                this->patblt.rect.x = this->stream.in_uint16_le();
                this->patblt.rect.y = this->stream.in_uint16_le();
                this->patblt.rect.cx = this->stream.in_uint16_le();
                this->patblt.rect.cy = this->stream.in_uint16_le();
                this->patblt.rop = this->stream.in_uint8();
                this->patblt.back_color = this->stream.in_uint32_le();
                this->patblt.fore_color = this->stream.in_uint32_le();
                this->patblt.brush.org_x = this->stream.in_uint8();
                this->patblt.brush.org_y = this->stream.in_uint8();
                this->patblt.brush.style = this->stream.in_uint8();
                this->patblt.brush.hatch = this->stream.in_uint8();
                this->stream.in_copy_bytes(this->patblt.brush.extra, 7);

                // RDPScrBlt scrblt;
                this->scrblt.rect.x = this->stream.in_uint16_le();
                this->scrblt.rect.y = this->stream.in_uint16_le();
                this->scrblt.rect.cx = this->stream.in_uint16_le();
                this->scrblt.rect.cy = this->stream.in_uint16_le();
                this->scrblt.rop = this->stream.in_uint8();
                this->scrblt.srcx = this->stream.in_uint16_le();
                this->scrblt.srcy = this->stream.in_uint16_le();

                // RDPOpaqueRect opaquerect;
                this->opaquerect.rect.x  = this->stream.in_uint16_le();
                this->opaquerect.rect.y  = this->stream.in_uint16_le();
                this->opaquerect.rect.cx = this->stream.in_uint16_le();
                this->opaquerect.rect.cy = this->stream.in_uint16_le();
                {
                    uint8_t red              = this->stream.in_uint8();
                    uint8_t green            = this->stream.in_uint8();
                    uint8_t blue             = this->stream.in_uint8();
                    this->opaquerect.color = red | green << 8 | blue << 16;
                }

                // RDPMemBlt memblt;
                this->memblt.cache_id = this->stream.in_uint16_le();
                this->memblt.rect.x  = this->stream.in_uint16_le();
                this->memblt.rect.y  = this->stream.in_uint16_le();
                this->memblt.rect.cx = this->stream.in_uint16_le();
                this->memblt.rect.cy = this->stream.in_uint16_le();
                this->memblt.rop = this->stream.in_uint8();
                this->memblt.srcx    = this->stream.in_uint8();
                this->memblt.srcy    = this->stream.in_uint8();
                this->memblt.cache_idx = this->stream.in_uint16_le();

                // RDPMem3Blt memblt;
                if (this->mem3blt_support) {
                    this->mem3blt.cache_id    = this->stream.in_uint16_le();
                    this->mem3blt.rect.x      = this->stream.in_uint16_le();
                    this->mem3blt.rect.y      = this->stream.in_uint16_le();
                    this->mem3blt.rect.cx     = this->stream.in_uint16_le();
                    this->mem3blt.rect.cy     = this->stream.in_uint16_le();
                    this->mem3blt.rop         = this->stream.in_uint8();
                    this->mem3blt.srcx        = this->stream.in_uint8();
                    this->mem3blt.srcy        = this->stream.in_uint8();
                    this->mem3blt.back_color  = this->stream.in_uint32_le();
                    this->mem3blt.fore_color  = this->stream.in_uint32_le();
                    this->mem3blt.brush.org_x = this->stream.in_uint8();
                    this->mem3blt.brush.org_y = this->stream.in_uint8();
                    this->mem3blt.brush.style = this->stream.in_uint8();
                    this->mem3blt.brush.hatch = this->stream.in_uint8();
                    this->stream.in_copy_bytes(this->mem3blt.brush.extra, 7);
                    this->mem3blt.cache_idx   = this->stream.in_uint16_le();
                }

                // RDPLineTo lineto;
                this->lineto.back_mode = this->stream.in_uint8();
                this->lineto.startx = this->stream.in_uint16_le();
                this->lineto.starty = this->stream.in_uint16_le();
                this->lineto.endx = this->stream.in_uint16_le();
                this->lineto.endy = this->stream.in_uint16_le();
                this->lineto.back_color = this->stream.in_uint32_le();
                this->lineto.rop2 = this->stream.in_uint8();
                this->lineto.pen.style = this->stream.in_uint8();
                this->lineto.pen.width = this->stream.in_sint8();
                this->lineto.pen.color = this->stream.in_uint32_le();

                // RDPGlyphIndex glyphindex;
                this->glyphindex.cache_id  = this->stream.in_uint8();
                this->glyphindex.fl_accel  = this->stream.in_sint16_le();
                this->glyphindex.ui_charinc  = this->stream.in_sint16_le();
                this->glyphindex.f_op_redundant = this->stream.in_sint16_le();
                this->glyphindex.back_color = this->stream.in_uint32_le();
                this->glyphindex.fore_color = this->stream.in_uint32_le();
                this->glyphindex.bk.x  = this->stream.in_uint16_le();
                this->glyphindex.bk.y  = this->stream.in_uint16_le();
                this->glyphindex.bk.cx = this->stream.in_uint16_le();
                this->glyphindex.bk.cy = this->stream.in_uint16_le();
                this->glyphindex.op.x  = this->stream.in_uint16_le();
                this->glyphindex.op.y  = this->stream.in_uint16_le();
                this->glyphindex.op.cx = this->stream.in_uint16_le();
                this->glyphindex.op.cy = this->stream.in_uint16_le();
                this->glyphindex.brush.org_x = this->stream.in_uint8();
                this->glyphindex.brush.org_y = this->stream.in_uint8();
                this->glyphindex.brush.style = this->stream.in_uint8();
                this->glyphindex.brush.hatch = this->stream.in_uint8();
                this->stream.in_copy_bytes(this->glyphindex.brush.extra, 7);
                this->glyphindex.glyph_x = this->stream.in_sint16_le();
                this->glyphindex.glyph_y = this->stream.in_sint16_le();
                this->glyphindex.data_len = this->stream.in_uint8();
                this->stream.in_copy_bytes(this->glyphindex.data, 256);
            break;

            case LAST_IMAGE_CHUNK:
            case PARTIAL_IMAGE_CHUNK:
            {
                if (this->nbconsumers){
                    InChunkedImageTransport chunk_trans(this->chunk_type, this->chunk_size, this->trans);

                    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                    png_set_read_fn(ppng, &chunk_trans, &png_read_data_fn);
                    png_info * pinfo = png_create_info_struct(ppng);
                    png_read_info(ppng, pinfo);

                    size_t height = png_get_image_height(ppng, pinfo);

                    TODO("check png row_size is identical to drawable rowsize");

                    uint32_t tmp[8192];
                    for (size_t k = 0 ; k < height ; ++k) {
                        png_read_row(ppng, reinterpret_cast<uint8_t*>(tmp), NULL);

                        for (size_t cu = 0 ; cu < this->nbconsumers ; cu++){
                            this->consumers[cu]->set_row(k, reinterpret_cast<uint8_t*>(tmp));
                        }
                    }
                    png_read_end(ppng, pinfo);
                    TODO("is there a memory leak ? is info structure destroyed of not ?");
                    png_destroy_read_struct(&ppng, &pinfo, NULL);
                }
                else {
                    REDOC("If no drawable is available ignore images chunks");
                    this->stream.reset();
                    this->trans->recv(&this->stream.end, this->chunk_size - HEADER_SIZE);
                    this->stream.p = this->stream.end;
                }
                this->remaining_order_count = 0;
            }
            break;
            case RDP_UPDATE_BITMAP:
            {
                if (!this->meta_ok) {
                    LOG(LOG_ERR, "Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                    throw Error(ERR_WRM);
                }
                if (!this->timestamp_ok) {
                    LOG(LOG_ERR, "Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing");
                    throw Error(ERR_WRM);
                }

                RDPBitmapData bitmap_data;
                bitmap_data.receive(this->stream);

                const uint8_t * data = this->stream.in_uint8p(bitmap_data.bitmap_size());

                Bitmap bitmap( bitmap_data.bits_per_pixel
                             , 0
                             , bitmap_data.width
                             , bitmap_data.height
                             , data
                             , bitmap_data.bitmap_size()
                             , (bitmap_data.flags & BITMAP_COMPRESSION)
                             );

                for (size_t i = 0; i < this->nbconsumers ; i++){
                    this->consumers[i]->draw( bitmap_data
                                            , data
                                            , bitmap_data.bitmap_size()
                                            , bitmap);
                }
            }
            break;
            default:
                LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
                throw Error(ERR_WRM);
            break;
        }
    }

    void play()
    {
        bool send_initial_image = true;
        if ((this->begin_capture.tv_sec == 0)
            ||(this->begin_capture.tv_sec > this->record_now.tv_sec)
            || (this->begin_capture.tv_sec == this->record_now.tv_sec && this->begin_capture.tv_usec > this->record_now.tv_usec)){
                send_initial_image = false;
        }

        while (this->next_order()){
            if (this->verbose > 8){
                LOG(LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n",
                    (unsigned)this->record_now.tv_sec, (unsigned)this->total_orders_count);
            }
            this->interpret_order();
            if ((this->begin_capture.tv_sec == 0)
            || (this->begin_capture.tv_sec < this->record_now.tv_sec)
            || (this->begin_capture.tv_sec == this->record_now.tv_sec
            && this->begin_capture.tv_usec <= this->record_now.tv_usec)){
                if (send_initial_image){
                    send_initial_image = false;
                }
                for (size_t i = 0; i < this->nbconsumers ; i++){
                    this->consumers[i]->snapshot(this->record_now, this->mouse_x, this->mouse_y, false, false);
                }
            }
            if (this->max_order_count && this->max_order_count <= this->total_orders_count){
                break;
            }
            if (this->end_capture.tv_sec
            && ((this->end_capture.tv_sec < this->record_now.tv_sec)
               || ((this->end_capture.tv_sec == this->record_now.tv_sec) && (this->end_capture.tv_usec < this->record_now.tv_usec)))){
                break;
            }
        }
    }
};

#endif
