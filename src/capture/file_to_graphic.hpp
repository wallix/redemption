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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean, Jonatan Poelen, Raphael Zhou,
*              Meng Tan, Clément Moroldo
*/

#pragma once

#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"

#include "capture/wrm_chunk_type.hpp"
#include "capture/save_state_chunk.hpp"
#include "core/RDP/state_chunk.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/compression_transport_builder.hpp"
#include "utils/difftimeval.hpp"
#include "utils/png.hpp"

#include <chrono>


struct FileToGraphic
{
    enum {
        HEADER_SIZE = 8
    };


private:
    uint8_t stream_buf[65536];
    InStream stream;

    CompressionInTransportBuilder compression_builder;

    Transport * trans_source;
    Transport * trans;

public:
    Rect screen_rect;

    // Internal state of orders
    StateChunk ssc;

    BmpCache     * bmp_cache;
    PointerCache   ptr_cache;
    GlyphCache     gly_cache;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    WrmChunkType chunk_type;
    uint16_t chunk_count;
private:
    uint16_t remaining_order_count;

public:
    // total number of RDP orders read from the start of the movie
    // (non orders chunks are counted as 1 order)
    uint32_t total_orders_count;

    timeval record_now;

// private:
    timeval start_record_now;
    timeval start_synctime_now;

public:
    template<class T, std::size_t N>
    struct fixed_ptr_array
    {
        fixed_ptr_array() : last(arr) {}

        void push_back(T * p) {
            if (p) {
                assert(this->size() < N);
                *this->last = p;
                ++this->last;
            }
        }

        T * * begin() { return this->arr; }
        T * * end() { return this->last; }

        T * * data() noexcept { return this->arr; }

        std::size_t size() const noexcept {
            return static_cast<std::size_t>(this->last - this->arr);
        }

    private:
        T * arr[N];
        T * * last = arr;
    };

    fixed_ptr_array<gdi::GraphicApi, 10> graphic_consumers;
    fixed_ptr_array<gdi::CaptureApi, 10> capture_consumers;
    fixed_ptr_array<gdi::KbdInputApi, 10> kbd_input_consumers;
    fixed_ptr_array<gdi::CaptureProbeApi, 10> capture_probe_consumers;
    fixed_ptr_array<gdi::ExternalCaptureApi, 10> external_event_consumers;

    bool meta_ok;
    bool timestamp_ok;
    uint16_t mouse_x;
    uint16_t mouse_y;
    bool real_time;

    const BGRPalette & palette = BGRPalette::classic_332(); // We don't really care movies are always 24 bits for now

    timeval begin_capture;
    timeval end_capture;
    uint32_t max_order_count;

    uint16_t info_version;
    uint16_t info_width;
    uint16_t info_height;
    uint16_t info_bpp;
    uint16_t info_number_of_cache;
    bool     info_use_waiting_list;
    uint16_t info_cache_0_entries;
    uint16_t info_cache_0_size;
    bool     info_cache_0_persistent;
    uint16_t info_cache_1_entries;
    uint16_t info_cache_1_size;
    bool     info_cache_1_persistent;
    uint16_t info_cache_2_entries;
    uint16_t info_cache_2_size;
    bool     info_cache_2_persistent;
    uint16_t info_cache_3_entries;
    uint16_t info_cache_3_size;
    bool     info_cache_3_persistent;
    uint16_t info_cache_4_entries;
    uint16_t info_cache_4_size;
    bool     info_cache_4_persistent;
    WrmCompressionAlgorithm info_compression_algorithm;

    bool ignore_frame_in_timeval;

    struct Statistics {
        uint32_t DstBlt;
        uint32_t MultiDstBlt;
        uint32_t PatBlt;
        uint32_t MultiPatBlt;
        uint32_t OpaqueRect;
        uint32_t MultiOpaqueRect;
        uint32_t ScrBlt;
        uint32_t MultiScrBlt;
        uint32_t MemBlt;
        uint32_t Mem3Blt;
        uint32_t LineTo;
        uint32_t GlyphIndex;
        uint32_t Polyline;
        uint32_t EllipseSC;

        uint32_t CacheBitmap;
        uint32_t CacheColorTable;
        uint32_t CacheGlyph;

        uint32_t FrameMarker;

        uint32_t BitmapUpdate;

        uint32_t CachePointer;
        uint32_t PointerIndex;

        uint32_t graphics_update_chunk;
        uint32_t bitmap_update_chunk;
        uint32_t timestamp_chunk;
    } statistics;

    bool break_privplay_client;
    std::chrono::microseconds movie_elapsed_client;
    std::chrono::microseconds begin_to_elapse;



    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        play            = 8,
        timestamp       = 16,
        rdp_orders      = 32,
        probe           = 64,
        frame_marker    = 128
    };

    FileToGraphic(Transport & trans, const timeval begin_capture, const timeval end_capture, bool real_time, Verbose verbose)
        : stream(stream_buf)
        , compression_builder(trans, WrmCompressionAlgorithm::no_compression)
        , trans_source(&trans)
        , trans(&trans)
        , bmp_cache(nullptr)
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(WrmChunkType::INVALID_CHUNK)
        , chunk_count(0)
        , remaining_order_count(0)
        , total_orders_count(0)
        , meta_ok(false)
        , timestamp_ok(false)
        , mouse_x(0)
        , mouse_y(0)
        , real_time(real_time)
        , begin_capture(begin_capture)
        , end_capture(end_capture)
        , max_order_count(0)
        , info_version(0)
        , info_width(0)
        , info_height(0)
        , info_bpp(0)
        , info_number_of_cache(0)
        , info_use_waiting_list(true)
        , info_cache_0_entries(0)
        , info_cache_0_size(0)
        , info_cache_0_persistent(false)
        , info_cache_1_entries(0)
        , info_cache_1_size(0)
        , info_cache_1_persistent(false)
        , info_cache_2_entries(0)
        , info_cache_2_size(0)
        , info_cache_2_persistent(false)
        , info_cache_3_entries(0)
        , info_cache_3_size(0)
        , info_cache_3_persistent(false)
        , info_cache_4_entries(0)
        , info_cache_4_size(0)
        , info_cache_4_persistent(false)
        , info_compression_algorithm(WrmCompressionAlgorithm::no_compression)
        , ignore_frame_in_timeval(false)
        , statistics()
        , break_privplay_client(false)
        , movie_elapsed_client{}
        , begin_to_elapse(std::chrono::seconds(this->begin_capture.tv_sec))
        , verbose(verbose)
    {
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

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr
    ) {
        this->graphic_consumers.push_back(graphic_ptr);
        this->capture_consumers.push_back(capture_ptr);
        this->kbd_input_consumers.push_back(kbd_input_ptr);
        this->capture_probe_consumers.push_back(capture_probe_ptr);
        this->external_event_consumers.push_back(external_event_ptr);
    }

    void set_pause_client(timeval & time) {
        this->start_synctime_now = {this->start_synctime_now.tv_sec + time.tv_sec, this->start_synctime_now.tv_usec + time.tv_usec};
    }

    void set_wait_after_load_client(timeval & time) {
        this->start_synctime_now = {time.tv_sec, time.tv_usec};
    }

    void set_sync() {
        timeval now = tvtime();
        this->start_synctime_now = {now.tv_sec + this->begin_capture.tv_sec, now.tv_usec};
    }

    void reinit() {
        this->remaining_order_count = 0;
        this->total_orders_count = 0;
    }

    /* order count set this->stream.p to the beginning of the next order.
     * Most of the times it means not changing it, except when it must read next chunk
     * when remaining order count is 0.
     * It update chunk headers (merely remaining orders count) and
     * reads the next chunk if necessary.
     */
    bool next_order()
    {
        if (this->chunk_type != WrmChunkType::LAST_IMAGE_CHUNK
         && this->chunk_type != WrmChunkType::PARTIAL_IMAGE_CHUNK) {
            if (this->stream.get_current() == this->stream.get_data_end()
             && this->remaining_order_count) {
                LOG(LOG_ERR, "Incomplete order batch at chunk %" PRIu16 " "
                             "order [%u/%" PRIu16 "] "
                             "remaining [%zu/%" PRIu32 "]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             this->stream.in_remain(), this->chunk_size);
                throw Error(ERR_WRM);
            }
        }
        if (!this->remaining_order_count){
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->sync();
            }

            uint8_t buf[HEADER_SIZE];
            if (Transport::Read::Eof == this->trans->atomic_read(buf, HEADER_SIZE)){
                return false;
            }
            InStream header(buf);
            this->chunk_type = safe_cast<WrmChunkType>(header.in_uint16_le());
            this->chunk_size = header.in_uint32_le();
            this->remaining_order_count = this->chunk_count = header.in_uint16_le();

            if (this->chunk_type != WrmChunkType::LAST_IMAGE_CHUNK
            && this->chunk_type != WrmChunkType::PARTIAL_IMAGE_CHUNK) {
                switch (this->chunk_type) {
                    case WrmChunkType::RDP_UPDATE_ORDERS:
                        this->statistics.graphics_update_chunk++; break;
                    case WrmChunkType::RDP_UPDATE_BITMAP:
                        this->statistics.bitmap_update_chunk++;   break;
                    case WrmChunkType::TIMESTAMP:
                        this->statistics.timestamp_chunk++;       break;
                    default: ;
                }
                if (this->chunk_size > 65536){
                    LOG(LOG_ERR,"chunk_size (%d) > 65536", this->chunk_size);
                    throw Error(ERR_WRM);
                }
                this->stream = InStream(this->stream_buf);
                if (this->chunk_size - HEADER_SIZE > 0) {
                    this->stream = InStream(this->stream_buf, this->chunk_size - HEADER_SIZE);
                    this->trans->recv_boom(this->stream_buf, this->chunk_size - HEADER_SIZE);
                }
            }
        }
        if (this->remaining_order_count > 0){this->remaining_order_count--;}
        return true;
    }

    void interpret_order()
    {
        this->total_orders_count++;

//         if (this->begin_to_elapse <= this->movie_elapsed_client) {
//             if (this->chunk_type != WrmChunkType::SESSION_UPDATE && this->chunk_type != WrmChunkType::TIMESTAMP) {
//                 return;
//             }
//         }

        switch (this->chunk_type)
        {
        case WrmChunkType::RDP_UPDATE_ORDERS:
        {
            if (!this->meta_ok){
                LOG(LOG_ERR, "Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                throw Error(ERR_WRM);
            }
            if (!this->timestamp_ok){
                LOG(LOG_ERR, "Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing\n");
                throw Error(ERR_WRM);
            }
            uint8_t control = this->stream.in_uint8();
            uint8_t class_ = (control & (RDP::STANDARD | RDP::SECONDARY));
            if (class_ == RDP::SECONDARY) {
                RDP::AltsecDrawingOrderHeader header(control);
                switch (header.orderType) {
                    case RDP::AltsecDrawingOrderHeader::FrameMarker:
                    {
                        this->statistics.FrameMarker++;
                        RDP::FrameMarker order;

                        order.receive(stream, header);
                        if (bool(this->verbose & Verbose::frame_marker)){
                            order.log(LOG_INFO);
                        }
                        if (this->begin_to_elapse <= this->movie_elapsed_client) {
                            for (gdi::GraphicApi * gd : this->graphic_consumers){
                                gd->draw(order);
                            }
                        }
                    }
                    break;
                    case RDP::AltsecDrawingOrderHeader::Window:
                        this->process_windowing(stream, header);
                    break;
                    default:
                        LOG(LOG_WARNING, "unsupported Alternate Secondary Drawing Order (%d)", header.orderType);
                        /* error, unknown order */
                    break;
                }
            }
            else if (class_ == (RDP::STANDARD | RDP::SECONDARY)) {
                RDPSecondaryOrderHeader header(this->stream);
                uint8_t const *next_order = this->stream.get_current() + header.order_data_length();
                switch (header.type) {
                case RDP::TS_CACHE_BITMAP_COMPRESSED:
                case RDP::TS_CACHE_BITMAP_UNCOMPRESSED:
                {
                    this->statistics.CacheBitmap++;
                    RDPBmpCache cmd;
                    cmd.receive(this->stream, header, this->palette, this->info_bpp);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        cmd.log(LOG_INFO);
                    }
                    this->bmp_cache->put(cmd.id, cmd.idx, cmd.bmp, cmd.key1, cmd.key2);
                }
                break;
                case RDP::TS_CACHE_COLOR_TABLE:
                    this->statistics.CacheColorTable++;
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_COLOR_TABLE (%d)", header.type);
                    break;
                case RDP::TS_CACHE_GLYPH:
                {
                    this->statistics.CacheGlyph++;
                    RDPGlyphCache cmd;
                    cmd.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        cmd.log(LOG_INFO);
                    }
                    this->gly_cache.set_glyph(
                        FontChar(std::move(cmd.aj), cmd.x, cmd.y, cmd.cx, cmd.cy, -1),
                        cmd.cacheId, cmd.cacheIndex
                    );
                }
                break;
                case RDP::TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)", header.type);
                  break;
                case RDP::TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)", header.type);
                  break;
                case RDP::TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)", header.type);
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                this->stream.in_skip_bytes(next_order - this->stream.get_current());
            }
            else if (class_ == RDP::STANDARD) {
                RDPPrimaryOrderHeader header = this->ssc.common.receive(this->stream, control);
                const Rect clip = (control & RDP::BOUNDS) ? this->ssc.common.clip : this->screen_rect;
                switch (this->ssc.common.order) {
                case RDP::GLYPHINDEX:
                    this->statistics.GlyphIndex++;
                    this->ssc.glyphindex.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.glyphindex.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.glyphindex, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette), this->gly_cache);
                        }
                    }
                    break;
                case RDP::DESTBLT:
                    this->statistics.DstBlt++;
                    this->ssc.destblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.destblt.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.destblt, clip);
                        }
                    }
                    break;
                case RDP::MULTIDSTBLT:
                    this->statistics.MultiDstBlt++;
                    this->ssc.multidstblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multidstblt.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.multidstblt, clip);
                        }
                    }
                    break;
                case RDP::MULTIOPAQUERECT:
                    this->statistics.MultiOpaqueRect++;
                    this->ssc.multiopaquerect.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multiopaquerect.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.multiopaquerect, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                case RDP::MULTIPATBLT:
                    this->statistics.MultiPatBlt++;
                    this->ssc.multipatblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multipatblt.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.multipatblt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                case RDP::MULTISCRBLT:
                    this->statistics.MultiScrBlt++;
                    this->ssc.multiscrblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multiscrblt.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        if (this->begin_to_elapse <= this->movie_elapsed_client)
                            gd->draw(this->ssc.multiscrblt, clip);
                    }
                    }
                    break;
                case RDP::PATBLT:
                    this->statistics.PatBlt++;
                    this->ssc.patblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.patblt.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.patblt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                case RDP::SCREENBLT:
                    this->statistics.ScrBlt++;
                    this->ssc.scrblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.scrblt.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.scrblt, clip);
                        }
                    }
                    break;
                case RDP::LINE:
                    this->statistics.LineTo++;
                    this->ssc.lineto.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.lineto.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.lineto, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                case RDP::RECT:
                    this->statistics.OpaqueRect++;
                    this->ssc.opaquerect.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.opaquerect.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.opaquerect, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                case RDP::MEMBLT:
                    {
                        this->statistics.MemBlt++;
                        this->ssc.memblt.receive(this->stream, header);
                        if (bool(this->verbose & Verbose::rdp_orders)){
                            this->ssc.memblt.log(LOG_INFO, clip);
                        }
                        const Bitmap & bmp = this->bmp_cache->get(this->ssc.memblt.cache_id, this->ssc.memblt.cache_idx);
                        if (!bmp.is_valid()){
                            LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)", this->ssc.memblt.cache_id, this->ssc.memblt.cache_idx);
                            throw Error(ERR_WRM);
                        } else {
                            if (this->begin_to_elapse <= this->movie_elapsed_client) {
                                for (gdi::GraphicApi * gd : this->graphic_consumers){
                                    gd->draw(this->ssc.memblt, clip, bmp);
                                }
                            }
                        }
                    }
                    break;
                case RDP::MEM3BLT:
                    {
                        this->statistics.Mem3Blt++;
                        this->ssc.mem3blt.receive(this->stream, header);
                        if (bool(this->verbose & Verbose::rdp_orders)){
                            this->ssc.mem3blt.log(LOG_INFO, clip);
                        }
                        const Bitmap & bmp = this->bmp_cache->get(this->ssc.mem3blt.cache_id, this->ssc.mem3blt.cache_idx);
                        if (!bmp.is_valid()){
                            LOG(LOG_ERR, "Mem3blt bitmap not found in cache at (%u, %u)", this->ssc.mem3blt.cache_id, this->ssc.mem3blt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            if (this->begin_to_elapse <= this->movie_elapsed_client) {
                                for (gdi::GraphicApi * gd : this->graphic_consumers){
                                    gd->draw(this->ssc.mem3blt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette), bmp);
                                }
                            }
                        }
                    }
                    break;
                case RDP::POLYLINE:
                    this->statistics.Polyline++;
                    this->ssc.polyline.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.polyline.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(this->ssc.polyline, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                case RDP::ELLIPSESC:
                    this->statistics.EllipseSC++;
                    this->ssc.ellipseSC.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.ellipseSC.log(LOG_INFO, clip);
                    }
                    if (this->begin_to_elapse <= this->movie_elapsed_client) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                                gd->draw(this->ssc.ellipseSC, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->ssc.common.order);
                    throw Error(ERR_WRM);
                }
            }
            else {
                /* error, this should always be set */
                LOG(LOG_ERR, "Unsupported drawing order detected : protocol error");
                throw Error(ERR_WRM);
            }
        }
        break;
        case WrmChunkType::TIMESTAMP:
        {
            this->stream.in_timeval_from_uint64le_usec(this->record_now);

            for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                obj->external_time(this->record_now);
            }

            // If some data remains, it is input data : mouse_x, mouse_y and decoded keyboard keys (utf8)
            if (this->stream.in_remain() > 0){
                if (this->stream.in_remain() < 4){
                    LOG(LOG_WARNING, "Input data truncated");
                    hexdump_d(stream.get_data(), stream.in_remain());
                }

                this->mouse_x = this->stream.in_uint16_le();
                this->mouse_y = this->stream.in_uint16_le();

                if (  (this->info_version > 1)
                    && this->stream.in_uint8()) {
                    this->ignore_frame_in_timeval = true;
                }

                if (bool(this->verbose & Verbose::timestamp)) {
                    LOG( LOG_INFO, "TIMESTAMP %lu.%lu mouse (x=%" PRIu16 ", y=%" PRIu16 ")\n"
                        , static_cast<unsigned long>(this->record_now.tv_sec)
                        , static_cast<unsigned long>(this->record_now.tv_usec)
                        , this->mouse_x
                        , this->mouse_y);
                }


                auto const input_data = this->stream.get_current();
                auto const input_len = this->stream.in_remain();
                this->stream.in_skip_bytes(input_len);
                for (gdi::KbdInputApi * kbd : this->kbd_input_consumers){
                    InStream input(input_data, input_len);
                    while (input.in_remain()) {
                        kbd->kbd_input(this->record_now, input.in_uint32_le());
                    }
                }

                if (bool(this->verbose & Verbose::timestamp)) {
                    for (auto data = input_data, end = data + input_len/4*4; data != end; data += 4) {
                        uint8_t         key8[6];
                        const size_t    len = UTF32toUTF8(data, 4/4, key8, sizeof(key8)-1);
                        key8[len] = 0;

                        LOG(LOG_INFO, "TIMESTAMP keyboard '%s'", key8);
                    }
                }
            }

            if (!this->timestamp_ok) {
                if (this->real_time) {
                    this->start_record_now   = this->record_now;
                    this->start_synctime_now = tvtime();
                    this->start_synctime_now.tv_sec -= this->begin_capture.tv_sec;
                }
                this->timestamp_ok = true;
            }
            else {
                if (this->real_time) {
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->sync();
                    }

                    this->movie_elapsed_client = difftimeval(this->record_now, this->start_record_now);

                    /*struct timeval now     = tvtime();
                    uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

                    uint64_t movie_elapsed = difftimeval(this->record_now, this->start_record_now);
                    this->movie_elapsed_client = movie_elapsed;

                    if (elapsed < movie_elapsed) {
                        struct timespec wtime     = {
                                static_cast<time_t>( (movie_elapsed - elapsed) / 1000000LL)
                            , static_cast<time_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                            };
                        struct timespec wtime_rem = { 0, 0 };*/

                        /*while ((nanosleep(&wtime, &wtime_rem) == -1) && (errno == EINTR)) {
                            wtime = wtime_rem;
                        }
                    } */
                }
            }
        }
        break;
        case WrmChunkType::META_FILE:
        // TODO Cache meta_data (sizes, number of entries) should be put in META chunk
        {
            this->info_version                   = this->stream.in_uint16_le();
            this->info_width                     = this->stream.in_uint16_le();
            this->info_height                    = this->stream.in_uint16_le();
            this->info_bpp                       = this->stream.in_uint16_le();
            this->info_cache_0_entries           = this->stream.in_uint16_le();
            this->info_cache_0_size              = this->stream.in_uint16_le();
            this->info_cache_1_entries           = this->stream.in_uint16_le();
            this->info_cache_1_size              = this->stream.in_uint16_le();
            this->info_cache_2_entries           = this->stream.in_uint16_le();
            this->info_cache_2_size              = this->stream.in_uint16_le();

            if (this->info_version <= 3) {
                this->info_number_of_cache       = 3;
                this->info_use_waiting_list      = false;

                this->info_cache_0_persistent    = false;
                this->info_cache_1_persistent    = false;
                this->info_cache_2_persistent    = false;
            }
            else {
                this->info_number_of_cache       = this->stream.in_uint8();
                this->info_use_waiting_list      = (this->stream.in_uint8() ? true : false);

                this->info_cache_0_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_1_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_2_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_3_entries       = this->stream.in_uint16_le();
                this->info_cache_3_size          = this->stream.in_uint16_le();
                this->info_cache_3_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_4_entries       = this->stream.in_uint16_le();
                this->info_cache_4_size          = this->stream.in_uint16_le();
                this->info_cache_4_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_compression_algorithm = static_cast<WrmCompressionAlgorithm>(this->stream.in_uint8());
                REDASSERT(is_valid_enum_value(this->info_compression_algorithm));
                if (!is_valid_enum_value(this->info_compression_algorithm)) {
                    this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;
                }

                this->trans = &this->compression_builder.reset(
                    *this->trans_source, this->info_compression_algorithm
                );
            }

            this->stream.in_skip_bytes(this->stream.in_remain());

            if (!this->meta_ok) {
                this->bmp_cache = new BmpCache(BmpCache::Recorder, this->info_bpp, this->info_number_of_cache,
                    this->info_use_waiting_list,
                    BmpCache::CacheOption(
                        this->info_cache_0_entries, this->info_cache_0_size, this->info_cache_0_persistent),
                    BmpCache::CacheOption(
                        this->info_cache_1_entries, this->info_cache_1_size, this->info_cache_1_persistent),
                    BmpCache::CacheOption(
                        this->info_cache_2_entries, this->info_cache_2_size, this->info_cache_2_persistent),
                    BmpCache::CacheOption(
                        this->info_cache_3_entries, this->info_cache_3_size, this->info_cache_3_persistent),
                    BmpCache::CacheOption(
                        this->info_cache_4_entries, this->info_cache_4_size, this->info_cache_4_persistent));
                this->screen_rect = Rect(0, 0, this->info_width, this->info_height);
                this->meta_ok = true;
            }
            else {
                if (this->screen_rect.cx != this->info_width ||
                    this->screen_rect.cy != this->info_height) {
                    LOG( LOG_ERR,"Inconsistant redundant meta chunk: (%u x %u) -> (%u x %u)"
                        , this->screen_rect.cx, this->screen_rect.cy, this->info_width, this->info_height);
                    throw Error(ERR_WRM);
                }
            }

            for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                obj->external_breakpoint();
            }
        }
        break;
        case WrmChunkType::SAVE_STATE:
        {
            SaveStateChunk ssc;
            ssc.recv(this->stream, this->ssc, this->info_version);
        }
        break;
        case WrmChunkType::LAST_IMAGE_CHUNK:
        case WrmChunkType::PARTIAL_IMAGE_CHUNK:
        {
            if (this->graphic_consumers.size()) {
                set_rows_from_image_chunk(
                    *this->trans,
                    this->chunk_type,
                    this->chunk_size,
                    this->screen_rect.cx,
                    this->graphic_consumers
                );
            }
            else {
                // If no drawable is available ignore images chunks
                this->stream.rewind();
                std::size_t sz = this->chunk_size - HEADER_SIZE;
                this->trans->recv_boom(this->stream_buf, sz);
                this->stream = InStream(this->stream_buf, sz, sz);
            }
            this->remaining_order_count = 0;
        }
        break;
        case WrmChunkType::RDP_UPDATE_BITMAP:
        {
            if (!this->meta_ok) {
                LOG(LOG_ERR, "Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                throw Error(ERR_WRM);
            }
            if (!this->timestamp_ok) {
                LOG(LOG_ERR, "Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing");
                throw Error(ERR_WRM);
            }

            this->statistics.BitmapUpdate++;
            RDPBitmapData bitmap_data;
            bitmap_data.receive(this->stream);

            const uint8_t * data = this->stream.in_uint8p(bitmap_data.bitmap_size());

            Bitmap bitmap( this->info_bpp
                            , bitmap_data.bits_per_pixel
                            , /*0*/&this->palette
                            , bitmap_data.width
                            , bitmap_data.height
                            , data
                            , bitmap_data.bitmap_size()
                            , (bitmap_data.flags & BITMAP_COMPRESSION)
                            );

            if (bool(this->verbose & Verbose::rdp_orders)){
                bitmap_data.log(LOG_INFO);
            }

            if (this->begin_to_elapse <= this->movie_elapsed_client) {
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(bitmap_data, bitmap);
                }
            }

        }
        break;
        case WrmChunkType::POINTER:
        {
            if (bool(this->verbose & Verbose::rdp_orders)){
                LOG(LOG_INFO, "POINTER2");
            }

            uint8_t cache_idx;

            this->mouse_x = this->stream.in_uint16_le();
            this->mouse_y = this->stream.in_uint16_le();
            cache_idx     = this->stream.in_uint8();

            if (  chunk_size - 8 /*header(8)*/
                > 5 /*mouse_x(2) + mouse_y(2) + cache_idx(1)*/) {
                this->statistics.CachePointer++;
                struct Pointer cursor(Pointer::POINTER_NULL);
                cursor.width = 32;
                cursor.height = 32;
                cursor.x = this->stream.in_uint8();
                cursor.y = this->stream.in_uint8();
                stream.in_copy_bytes(cursor.data, 32 * 32 * 3);
                stream.in_copy_bytes(cursor.mask, 128);

                this->ptr_cache.add_pointer_static(cursor, cache_idx);

                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->set_pointer(cursor);
                }
            }
            else {
                this->statistics.PointerIndex++;
                Pointer & pi = this->ptr_cache.Pointers[cache_idx];
                Pointer cursor(Pointer::POINTER_NULL);
                cursor.width = pi.width;
                cursor.height = pi.height;
                cursor.x = pi.x;
                cursor.y = pi.y;
                memcpy(cursor.data, pi.data, sizeof(pi.data));
                memcpy(cursor.mask, pi.mask, sizeof(pi.mask));

                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->set_pointer(cursor);
                }
            }
        }
        break;
        case WrmChunkType::POINTER2:
        {
            if (bool(this->verbose & Verbose::rdp_orders)){
                LOG(LOG_INFO, "POINTER2");
            }

            uint8_t cache_idx;

            this->mouse_x = this->stream.in_uint16_le();
            this->mouse_y = this->stream.in_uint16_le();
            cache_idx     = this->stream.in_uint8();

            this->statistics.CachePointer++;
            struct Pointer cursor(Pointer::POINTER_NULL);

            cursor.width    = this->stream.in_uint8();
            cursor.height   = this->stream.in_uint8();
            /* cursor.bpp   = this->stream.in_uint8();*/
            this->stream.in_skip_bytes(1);

            cursor.x = this->stream.in_uint8();
            cursor.y = this->stream.in_uint8();

            uint16_t data_size = this->stream.in_uint16_le();
            REDASSERT(data_size <= Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT * 3);

            uint16_t mask_size = this->stream.in_uint16_le();
            REDASSERT(mask_size <= Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT * 1 / 8);

            stream.in_copy_bytes(cursor.data, std::min<size_t>(sizeof(cursor.data), data_size));
            stream.in_copy_bytes(cursor.mask, std::min<size_t>(sizeof(cursor.mask), mask_size));

            this->ptr_cache.add_pointer_static(cursor, cache_idx);

            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->set_pointer(cursor);
            }
        }
        break;
        case WrmChunkType::RESET_CHUNK:
            this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;

            this->trans = this->trans_source;
        break;
        case WrmChunkType::SESSION_UPDATE:
            this->stream.in_timeval_from_uint64le_usec(this->record_now);

            for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                obj->external_time(this->record_now);
            }

            {
                uint16_t message_length = this->stream.in_uint16_le();

                const char * message =  ::char_ptr_cast(this->stream.get_current()); // Null-terminator is included.

                this->stream.in_skip_bytes(message_length);

                for (gdi::CaptureProbeApi * cap_probe : this->capture_probe_consumers){
                    cap_probe->session_update(this->record_now, {message, message_length});
                }
            }

            if (!this->timestamp_ok) {
                if (this->real_time) {
                    this->start_record_now   = this->record_now;
                    this->start_synctime_now = tvtime();
                    this->start_synctime_now.tv_sec -= this->begin_capture.tv_sec;
                }
                this->timestamp_ok = true;
            }
            else {
                if (this->real_time) {
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->sync();
                    }

                    this->movie_elapsed_client = difftimeval(this->record_now, this->start_record_now);

                    /*struct timeval now     = tvtime();
                    uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

                    uint64_t movie_elapsed = difftimeval(this->record_now, this->start_record_now);


                    if (elapsed < movie_elapsed) {
                        struct timespec wtime     = {
                                static_cast<time_t>( (movie_elapsed - elapsed) / 1000000LL)
                            , static_cast<time_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                            };
                        struct timespec wtime_rem = { 0, 0 };*/

                        /*while ((nanosleep(&wtime, &wtime_rem) == -1) && (errno == EINTR)) {
                            wtime = wtime_rem;
                        }
                    }*/
                }
            }
        break;
        case WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE:
            for (gdi::CaptureProbeApi * cap_probe : this->capture_probe_consumers){
                cap_probe->possible_active_window_change();
            }
        break;
        default:
            LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
            throw Error(ERR_WRM);
        }
    }


    void process_windowing( InStream & stream, const RDP::AltsecDrawingOrderHeader & header) {
        if (bool(this->verbose & Verbose::probe)) {
            LOG(LOG_INFO, "rdp_orders::process_windowing");
        }

        const uint32_t FieldsPresentFlags = [&]{
            InStream stream2(stream.get_current(), stream.in_remain());
            stream2.in_skip_bytes(2);    // OrderSize(2)
            return stream2.in_uint32_le();
        }();

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                                      | RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY
                                      | RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP)) {
            case RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW:
                this->process_window_information(stream, header, FieldsPresentFlags);
                break;

            case RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY:
                this->process_notification_icon_information(stream, header, FieldsPresentFlags);
                break;

            case RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP:
                this->process_desktop_information(stream, header, FieldsPresentFlags);
                break;

            default:
                LOG(LOG_INFO,
                    "rdp_orders::process_windowing: "
                        "unsupported Windowing Alternate Secondary Drawing Orders! "
                        "FieldsPresentFlags=0x%08X",
                    FieldsPresentFlags);
                break;
        }
    }

    void process_window_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                   , uint32_t FieldsPresentFlags) {
        if (bool(this->verbose & Verbose::probe)) {
            LOG(LOG_INFO, "rdp_orders::process_window_information");
        }

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                      | RDP::RAIL::WINDOW_ORDER_ICON
                                      | RDP::RAIL::WINDOW_ORDER_CACHEDICON
                                      | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
        {
            case RDP::RAIL::WINDOW_ORDER_ICON: {
                    RDP::RAIL::WindowIcon order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case RDP::RAIL::WINDOW_ORDER_CACHEDICON: {
                    RDP::RAIL::CachedIcon order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                    RDP::RAIL::DeletedWindow order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case 0:
            case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                    RDP::RAIL::NewOrExistingWindow order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;
        }
    }

    void process_notification_icon_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                              , uint32_t FieldsPresentFlags) {
        if (bool(this->verbose & Verbose::probe)) {
            LOG(LOG_INFO, "rdp_orders::process_notification_icon_information");
        }

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                      | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
        {
            case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                    RDP::RAIL::DeletedNotificationIcons order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case 0:
            case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                    RDP::RAIL::NewOrExistingNotificationIcons order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;
        }
    }

    void process_desktop_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                    , uint32_t FieldsPresentFlags) {
        if (bool(this->verbose & Verbose::probe)) {
            LOG(LOG_INFO, "rdp_orders::process_desktop_information");
        }

        if (FieldsPresentFlags & RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_NONE) {
            RDP::RAIL::NonMonitoredDesktop order;
            order.receive(stream);
            order.log(LOG_INFO);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->draw(order);
            }
        }
        else {
            RDP::RAIL::ActivelyMonitoredDesktop order;
            order.receive(stream);
            order.log(LOG_INFO);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->draw(order);
            }
        }
    }

    void play(bool const & requested_to_stop) {
        this->privplay([](time_t){}, requested_to_stop);
    }

    bool play_client() {
        return this->privplay_client([](time_t){});
    }

    template<class CbUpdateProgress>
    void play(CbUpdateProgress update_progess, bool const & requested_to_stop) {
        time_t last_sent_record_now = 0;
        this->privplay([&](time_t record_now) {
            if (last_sent_record_now != record_now) {
                update_progess(record_now);
                last_sent_record_now = record_now;
            }
        }, requested_to_stop);
    }

private:
    template<class CbUpdateProgress>
    void privplay(CbUpdateProgress update_progess, bool const & requested_to_stop) {
        while (!requested_to_stop && this->next_order()) {
            if (bool(this->verbose & Verbose::play)) {
                LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                   , unsigned(this->record_now.tv_sec), this->total_orders_count);
            }
            this->interpret_order();
            if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                for (gdi::CaptureApi * cap : this->capture_consumers){
                    cap->periodic_snapshot(
                        this->record_now, this->mouse_x, this->mouse_y
                      , this->ignore_frame_in_timeval
                    );
                }

                this->ignore_frame_in_timeval = false;

                update_progess(this->record_now.tv_sec);
            }
            if (this->max_order_count && this->max_order_count <= this->total_orders_count) {
                break;
            }
            if (this->end_capture.tv_sec && this->end_capture < this->record_now) {
                break;
            }
        }
    }

public:
    void instant_play_client(std::chrono::microseconds endin_frame) {

        while (endin_frame >= this->movie_elapsed_client) {

            if (this->next_order()) {

                if (bool(this->verbose & Verbose::play)) {
                    LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                    , unsigned(this->record_now.tv_sec), this->total_orders_count);
                }

                this->interpret_order();

                if (this->max_order_count && this->max_order_count <= this->total_orders_count) {
                    break_privplay_client = true;
                }
                if (this->end_capture.tv_sec && this->end_capture < this->record_now) {
                    break_privplay_client = true;
                }
            } else {
                break_privplay_client = true;
            }
        }
    }


private:
    template<class CbUpdateProgress>
    bool privplay_client(CbUpdateProgress update_progess) {

        struct timeval now                = tvtime();
        std::chrono::microseconds elapsed = difftimeval(now, this->start_synctime_now) ;

        bool res(false);

        //LOG(LOG_INFO, "begin = %u movie_elapsed_client = %u elapsed = %u", unsigned(this->begin_to_elapse), unsigned(this->movie_elapsed_client), elapsed);
        if (elapsed >= this->movie_elapsed_client) {
            if (this->next_order()) {
                if (bool(this->verbose & Verbose::play)) {
                    LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                    , unsigned(this->record_now.tv_sec), this->total_orders_count);
                }

                if (this->remaining_order_count > 0) {
                    res = true;
                }

                this->interpret_order();
                if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                    for (gdi::CaptureApi * cap : this->capture_consumers){
                        cap->periodic_snapshot(
                            this->record_now, this->mouse_x, this->mouse_y
                        , this->ignore_frame_in_timeval
                        );
                    }

                    this->ignore_frame_in_timeval = false;

                    update_progess(this->record_now.tv_sec);
                }

                if (this->max_order_count && this->max_order_count <= this->total_orders_count) {
                    break_privplay_client = true;
                }
                if (this->end_capture.tv_sec && this->end_capture < this->record_now) {
                    break_privplay_client = true;
                }
            } else {
                break_privplay_client = true;
            }
        }

        return res;
    }
};
