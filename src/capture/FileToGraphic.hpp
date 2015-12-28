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
#include "RDP/caches/pointercache.hpp"
#include "RDP/caches/glyphcache.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "RDP/share.hpp"
#include "RDP/bitmapupdate.hpp"
#include "difftimeval.hpp"
#include "compression_transport_wrapper.hpp"
#include "chunked_image_transport.hpp"
#include "CaptureDevice.hpp"
#include "wrm_label.hpp"
#include "cast.hpp"
#include "png.hpp"

struct FileToGraphic
{
    enum {
        HEADER_SIZE = 8
    };

private:
    uint8_t stream_buf[65536];
    InStream stream;

    CompressionInTransportWrapper compression_wrapper;

public:
    Transport * trans_source;
    Transport * trans;

    Rect screen_rect;

    // Internal state of orders
    RDPOrderCommon     common;
    RDPDestBlt         destblt;
    RDPMultiDstBlt     multidstblt;
    RDPMultiOpaqueRect multiopaquerect;
    RDP::RDPMultiPatBlt     multipatblt;
    RDP::RDPMultiScrBlt     multiscrblt;
    RDPPatBlt          patblt;
    RDPScrBlt          scrblt;
    RDPOpaqueRect      opaquerect;
    RDPMemBlt          memblt;
    RDPMem3Blt         mem3blt;
    RDPLineTo          lineto;
    RDPGlyphIndex      glyphindex;
    RDPPolyline        polyline;
    RDPEllipseSC       ellipseSC;

    BmpCache     * bmp_cache;
    PointerCache   ptr_cache;
    GlyphCache     gly_cache;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;
private:
    uint16_t remaining_order_count;

public:
    // total number of RDP orders read from the start of the movie
    // (non orders chunks are counted as 1 order)
    uint32_t total_orders_count;

    timeval record_now;

private:
    timeval start_record_now;
    timeval start_synctime_now;

public:
    uint16_t nbconsumers;

    struct Consumer {
        RDPGraphicDevice * graphic_device;
        RDPCaptureDevice * capture_device;
    } consumers[10];

    bool meta_ok;
    bool timestamp_ok;
    uint16_t mouse_x;
    uint16_t mouse_y;
    uint16_t input_len;
    uint8_t  input[8192];
    bool real_time;

    const BGRPalette & palette = BGRPalette::classic_332(); // We don't really care movies are always 24 bits for now

    const timeval begin_capture;
    const timeval end_capture;
    uint32_t max_order_count;
    uint32_t verbose;

    bool mem3blt_support;
    bool polyline_support;
    bool multidstblt_support;
    bool multiopaquerect_support;
    bool multipatblt_support;
    bool multiscrblt_support;

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
    uint8_t  info_compression_algorithm;

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

    FileToGraphic(Transport * trans, const timeval begin_capture, const timeval end_capture, bool real_time, uint32_t verbose)
        : stream(stream_buf)
        , compression_wrapper(*trans, CompressionTransportBase::Algorithm::None)
        , trans_source(trans)
        , trans(trans)
        , common(RDP::PATBLT, Rect(0, 0, 1, 1))
        , destblt(Rect(), 0)
        , multidstblt()
        , multiopaquerect()
        , multipatblt()
        , multiscrblt()
        , patblt(Rect(), 0, 0, 0, RDPBrush())
        , scrblt(Rect(), 0, 0, 0)
        , opaquerect(Rect(), 0)
        , memblt(0, Rect(), 0, 0, 0, 0)
        , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
        , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
        , glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0
                    , reinterpret_cast<const uint8_t *>(""))
        , polyline()
        , ellipseSC()
        , bmp_cache(nullptr)
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
        , polyline_support(false)
        , multidstblt_support(false)
        , multiopaquerect_support(false)
        , multipatblt_support(false)
        , multiscrblt_support(false)
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
        , info_compression_algorithm(0)
        , ignore_frame_in_timeval(false)
        , statistics()
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

    void add_consumer(RDPGraphicDevice * graphic_device, RDPCaptureDevice * capture_device) {
        this->consumers[this->nbconsumers  ].graphic_device = graphic_device;
        this->consumers[this->nbconsumers++].capture_device = capture_device;
    }

    bool next_order()
    REDOC("order count set this->stream.p to the beginning of the next order."
          "Most of the times it means not changing it, except when it must read next chunk"
          "when remaining order count is 0."
          "It update chunk headers (merely remaining orders count) and"
          " reads the next chunk if necessary.")
    {
        if (this->chunk_type != LAST_IMAGE_CHUNK
         && this->chunk_type != PARTIAL_IMAGE_CHUNK) {
            if (this->stream.get_current() == this->stream.get_data_end()
             && this->remaining_order_count) {
                LOG(LOG_ERR, "Incomplete order batch at chunk %" PRIu16 " "
                             "order [%u/%" PRIu16 "] "
                             "remaining [%zu/%" PRIu32 "]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             this->stream.in_remain(), this->chunk_size);
                return false;
            }
        }

        if (!this->remaining_order_count){
            for (size_t i = 0; i < this->nbconsumers; i++){
                this->consumers[i].graphic_device->flush();
            }

            try {
                uint8_t buf[HEADER_SIZE];
                {
                    auto end = buf;
                    this->trans->recv(&end, HEADER_SIZE);
                }
                InStream header(buf);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->remaining_order_count = this->chunk_count = header.in_uint16_le();

                if (this->chunk_type != LAST_IMAGE_CHUNK && this->chunk_type != PARTIAL_IMAGE_CHUNK) {
                    switch (this->chunk_type) {
                        case RDP_UPDATE_ORDERS:
                            this->statistics.graphics_update_chunk++; break;
                        case RDP_UPDATE_BITMAP:
                            this->statistics.bitmap_update_chunk++;   break;
                        case TIMESTAMP:
                            this->statistics.timestamp_chunk++;       break;
                    }
                    if (this->chunk_size > 65536){
                        LOG(LOG_INFO,"chunk_size (%d) > 65536", this->chunk_size);
                        return false;
                    }
                    this->stream = InStream(this->stream_buf);
                    if (this->chunk_size - HEADER_SIZE > 0) {
                        this->stream = InStream(this->stream_buf, this->chunk_size - HEADER_SIZE);
                        auto end = this->stream_buf;
                        this->trans->recv(&end, this->chunk_size - HEADER_SIZE);
                    }
                }
            }
            catch (Error & e){
                if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                    throw;
                }

                if (this->verbose) {
                    LOG(LOG_INFO,"receive error %u : end of transport", e.id);
                }
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
                        if (this->verbose > 32){
                            order.log(LOG_INFO);
                        }
                        for (size_t i = 0; i < this->nbconsumers; i++){
                            this->consumers[i].graphic_device->draw(order);
                        }
                    }
                    break;
                    default:
                        LOG(LOG_ERR, "unsupported Alternate Secondary Drawing Order (%d)", header.orderType);
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
                    if (this->verbose > 32){
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
                    if (this->verbose > 32){
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
                RDPPrimaryOrderHeader header = this->common.receive(this->stream, control);
                const Rect & clip = (control & RDP::BOUNDS)?this->common.clip:this->screen_rect;
                switch (this->common.order) {
                case RDP::GLYPHINDEX:
                    this->statistics.GlyphIndex++;
                    this->glyphindex.receive(this->stream, header);
                    for (size_t i = 0; i < this->nbconsumers; i++){
                        this->consumers[i].graphic_device->draw(this->glyphindex, clip, &this->gly_cache);
                    }
                    break;
                case RDP::DESTBLT:
                    this->statistics.DstBlt++;
                    this->destblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->destblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++){
                        this->consumers[i].graphic_device->draw(this->destblt, clip);
                    }
                    break;
                case RDP::MULTIDSTBLT:
                    this->statistics.MultiDstBlt++;
                    this->multidstblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->multidstblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->multidstblt, clip);
                    }
                    break;
                case RDP::MULTIOPAQUERECT:
                    this->statistics.MultiOpaqueRect++;
                    this->multiopaquerect.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->multiopaquerect.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->multiopaquerect, clip);
                    }
                    break;
                case RDP::MULTIPATBLT:
                    this->statistics.MultiPatBlt++;
                    this->multipatblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->multipatblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->multipatblt, clip);
                    }
                    break;
                case RDP::MULTISCRBLT:
                    this->statistics.MultiScrBlt++;
                    this->multiscrblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->multiscrblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->multiscrblt, clip);
                    }
                    break;
                case RDP::PATBLT:
                    this->statistics.PatBlt++;
                    this->patblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->patblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++){
                        this->consumers[i].graphic_device->draw(this->patblt, clip);
                    }
                    break;
                case RDP::SCREENBLT:
                    this->statistics.ScrBlt++;
                    this->scrblt.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->scrblt.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++){
                        this->consumers[i].graphic_device->draw(this->scrblt, clip);
                    }
                    break;
                case RDP::LINE:
                    this->statistics.LineTo++;
                    this->lineto.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->lineto.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->lineto, clip);
                    }
                    break;
                case RDP::RECT:
                    this->statistics.OpaqueRect++;
                    this->opaquerect.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->opaquerect.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++){
                        this->consumers[i].graphic_device->draw(this->opaquerect, clip);
                    }
                    break;
                case RDP::MEMBLT:
                    {
                        this->statistics.MemBlt++;
                        this->memblt.receive(this->stream, header);
                        if (this->verbose > 32){
                            this->memblt.log(LOG_INFO, clip);
                        }
                        const Bitmap & bmp = this->bmp_cache->get(this->memblt.cache_id, this->memblt.cache_idx);
                        if (!bmp.is_valid()){
                            LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)", this->memblt.cache_id, this->memblt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (size_t i = 0; i < this->nbconsumers; i++){
                                this->consumers[i].graphic_device->draw(this->memblt, clip, bmp);
                            }
                        }
                    }
                    break;
                case RDP::MEM3BLT:
                    {
                        this->statistics.Mem3Blt++;
                        this->mem3blt.receive(this->stream, header);
                        if (this->verbose > 32){
                            this->mem3blt.log(LOG_INFO, clip);
                        }
                        const Bitmap & bmp = this->bmp_cache->get(this->mem3blt.cache_id, this->mem3blt.cache_idx);
                        if (!bmp.is_valid()){
                            LOG(LOG_ERR, "Mem3blt bitmap not found in cache at (%u, %u)", this->mem3blt.cache_id, this->mem3blt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (size_t i = 0; i < this->nbconsumers; i++){
                                this->consumers[i].graphic_device->draw(this->mem3blt, clip, bmp);
                            }
                        }
                    }
                    break;
                case RDP::POLYLINE:
                    this->statistics.Polyline++;
                    this->polyline.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->polyline.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->polyline, clip);
                    }
                    break;
                case RDP::ELLIPSESC:
                    this->statistics.EllipseSC++;
                    this->ellipseSC.receive(this->stream, header);
                    if (this->verbose > 32){
                        this->ellipseSC.log(LOG_INFO, clip);
                    }
                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->draw(this->ellipseSC, clip);
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
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
            case TIMESTAMP:
            {
                this->stream.in_timeval_from_uint64le_usec(this->record_now);

                for (size_t i = 0; i < this->nbconsumers; i++){
                    if (this->consumers[i].capture_device) {
                        this->consumers[i].capture_device->external_time(this->record_now);
                    }
                }

                REDOC("If some data remains, it is input data : mouse_x, mouse_y and decoded keyboard keys (utf8)")
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

                    if (this->verbose > 16) {
                        LOG( LOG_INFO, "TIMESTAMP %lu.%lu mouse (x=%" PRIu16 ", y=%" PRIu16 ")\n"
                           , static_cast<unsigned long>(this->record_now.tv_sec)
                           , static_cast<unsigned long>(this->record_now.tv_usec)
                           , this->mouse_x
                           , this->mouse_y);
                    }

                    TODO("this->input contains UTF32 unicode points, it should not be a byte buffer."
                         "This leads to back and forth conversion between 32 bits and 8 bits")
                    this->input_len = std::min( static_cast<uint16_t>(stream.in_remain())
                                              , static_cast<uint16_t>(sizeof(this->input) - 1));
                    if (this->input_len){
                        this->stream.in_copy_bytes(this->input, this->input_len);
                        this->input[this->input_len] = 0;
                        this->stream.in_skip_bytes(this->stream.in_remain());

                        for (size_t i = 0; i < this->nbconsumers; i++){
                            if (this->consumers[i].capture_device) {
                                this->consumers[i].capture_device->input(this->record_now, this->input, this->input_len);
                            }
                        }

                        if (this->verbose > 16) {
                            while (this->input_len >= 4) {
                                uint8_t         key8[6];
                                const size_t    len = UTF32toUTF8(this->input, 4, key8, sizeof(key8));
                                key8[len] = 0;

                                LOG( LOG_INFO, "TIMESTAMP %lu.%lu keyboard '%s'"
                                   , static_cast<unsigned long>(this->record_now.tv_sec)
                                   , static_cast<unsigned long>(this->record_now.tv_usec)
                                   , key8);
                            }
                        }
                    }
                }

                if (!this->timestamp_ok) {
                   if (this->real_time) {
                        this->start_record_now   = this->record_now;
                        this->start_synctime_now = tvtime();
                    }
                    this->timestamp_ok = true;
                }
                else {
                   if (this->real_time) {
                        for (size_t i = 0; i < this->nbconsumers; i++) {
                            this->consumers[i].graphic_device->flush();
                        }

                        struct timeval now     = tvtime();
                        uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

                        uint64_t movie_elapsed = difftimeval(this->record_now, this->start_record_now);

                        if (elapsed < movie_elapsed) {
                            struct timespec wtime     = {
                                  static_cast<time_t>( (movie_elapsed - elapsed) / 1000000LL)
                                , static_cast<time_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                                };
                            struct timespec wtime_rem = { 0, 0 };

                            while ((nanosleep(&wtime, &wtime_rem) == -1) && (errno == EINTR)) {
                                wtime = wtime_rem;
                            }
                        }
                    }
                }
            }
            break;
            case META_FILE:
            TODO("Cache meta_data (sizes, number of entries) should be put in META chunk");
            {
                this->info_version                   = this->stream.in_uint16_le();
                this->mem3blt_support                = (this->info_version > 1);
                this->polyline_support               = (this->info_version > 2);
                this->multidstblt_support            = (this->info_version > 3);
                this->multiopaquerect_support        = (this->info_version > 3);
                this->multipatblt_support            = (this->info_version > 3);
                this->multiscrblt_support            = (this->info_version > 3);
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

                    this->info_compression_algorithm = this->stream.in_uint8();
                    REDASSERT(this->info_compression_algorithm < CompressionTransportBase::max_algorithm);

                    // re-init
                    new (&this->compression_wrapper) CompressionInTransportWrapper(
                        *this->trans_source, this->info_compression_algorithm);
                    this->trans = &this->compression_wrapper.get();
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

                for (size_t i = 0; i < this->nbconsumers; i++) {
                    if (this->consumers[i].capture_device) {
                        this->consumers[i].capture_device->external_breakpoint();
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

                // RDPPolyine polyline;
                if (this->polyline_support) {
                    this->polyline.xStart          = this->stream.in_sint16_le();
                    this->polyline.yStart          = this->stream.in_sint16_le();
                    this->polyline.bRop2           = this->stream.in_uint8();
                    this->polyline.BrushCacheEntry = this->stream.in_uint16_le();
                    this->polyline.PenColor        = this->stream.in_uint32_le();
                    this->polyline.NumDeltaEntries = this->stream.in_uint8();
                    for (uint8_t i = 0; i < this->polyline.NumDeltaEntries; i++) {
                        this->polyline.deltaEncodedPoints[i].xDelta = this->stream.in_sint16_le();
                        this->polyline.deltaEncodedPoints[i].yDelta = this->stream.in_sint16_le();
                    }
                }

                // RDPMultiDstBlt multidstblt;
                if (this->multidstblt_support) {
                    this->multidstblt.nLeftRect     = this->stream.in_sint16_le();
                    this->multidstblt.nTopRect      = this->stream.in_sint16_le();
                    this->multidstblt.nWidth        = this->stream.in_sint16_le();
                    this->multidstblt.nHeight       = this->stream.in_sint16_le();
                    this->multidstblt.bRop          = this->stream.in_uint8();
                    this->multidstblt.nDeltaEntries = this->stream.in_uint8();
                    for (uint8_t i = 0; i < this->multidstblt.nDeltaEntries; i++) {
                        this->multidstblt.deltaEncodedRectangles[i].leftDelta = this->stream.in_sint16_le();
                        this->multidstblt.deltaEncodedRectangles[i].topDelta  = this->stream.in_sint16_le();
                        this->multidstblt.deltaEncodedRectangles[i].width     = this->stream.in_sint16_le();
                        this->multidstblt.deltaEncodedRectangles[i].height    = this->stream.in_sint16_le();
                    }
                }

                // RDPMultiOpaqueRect multiopaquerect;
                if (this->multiopaquerect_support) {
                    this->multiopaquerect.nLeftRect         = this->stream.in_sint16_le();
                    this->multiopaquerect.nTopRect          = this->stream.in_sint16_le();
                    this->multiopaquerect.nWidth            = this->stream.in_sint16_le();
                    this->multiopaquerect.nHeight           = this->stream.in_sint16_le();
                    {
                        uint8_t red                         = this->stream.in_uint8();
                        uint8_t green                       = this->stream.in_uint8();
                        uint8_t blue                        = this->stream.in_uint8();
                        this->multiopaquerect._Color        = red | green << 8 | blue << 16;
                    }
                    this->multiopaquerect.nDeltaEntries     = this->stream.in_uint8();
                    for (uint8_t i = 0; i < this->multiopaquerect.nDeltaEntries; i++) {
                        this->multiopaquerect.deltaEncodedRectangles[i].leftDelta = this->stream.in_sint16_le();
                        this->multiopaquerect.deltaEncodedRectangles[i].topDelta  = this->stream.in_sint16_le();
                        this->multiopaquerect.deltaEncodedRectangles[i].width     = this->stream.in_sint16_le();
                        this->multiopaquerect.deltaEncodedRectangles[i].height    = this->stream.in_sint16_le();
                    }
                }

                // RDPMultiPatBlt multipatblt;
                if (this->multipatblt_support) {
                    this->multipatblt.nLeftRect  = this->stream.in_sint16_le();
                    this->multipatblt.nTopRect   = this->stream.in_sint16_le();
                    this->multipatblt.nWidth     = this->stream.in_uint16_le();
                    this->multipatblt.nHeight    = this->stream.in_uint16_le();
                    this->multipatblt.bRop       = this->stream.in_uint8();
                    this->multipatblt.BackColor  = this->stream.in_uint32_le();
                    this->multipatblt.ForeColor  = this->stream.in_uint32_le();
                    this->multipatblt.BrushOrgX  = this->stream.in_uint8();
                    this->multipatblt.BrushOrgY  = this->stream.in_uint8();
                    this->multipatblt.BrushStyle = this->stream.in_uint8();
                    this->multipatblt.BrushHatch = this->stream.in_uint8();
                    this->stream.in_copy_bytes(this->multipatblt.BrushExtra, 7);
                    this->multipatblt.nDeltaEntries = this->stream.in_uint8();
                    for (uint8_t i = 0; i < this->multipatblt.nDeltaEntries; i++) {
                        this->multipatblt.deltaEncodedRectangles[i].leftDelta = this->stream.in_sint16_le();
                        this->multipatblt.deltaEncodedRectangles[i].topDelta  = this->stream.in_sint16_le();
                        this->multipatblt.deltaEncodedRectangles[i].width     = this->stream.in_sint16_le();
                        this->multipatblt.deltaEncodedRectangles[i].height    = this->stream.in_sint16_le();
                    }
                }

                // RDPMultiScrBlt multiscrblt;
                if (this->multiscrblt_support) {
                    this->multiscrblt.nLeftRect  = this->stream.in_sint16_le();
                    this->multiscrblt.nTopRect   = this->stream.in_sint16_le();
                    this->multiscrblt.nWidth     = this->stream.in_uint16_le();
                    this->multiscrblt.nHeight    = this->stream.in_uint16_le();
                    this->multiscrblt.bRop       = this->stream.in_uint8();
                    this->multiscrblt.nXSrc      = this->stream.in_sint16_le();
                    this->multiscrblt.nYSrc      = this->stream.in_sint16_le();
                    this->multiscrblt.nDeltaEntries = this->stream.in_uint8();
                    for (uint8_t i = 0; i < this->multiscrblt.nDeltaEntries; i++) {
                        this->multiscrblt.deltaEncodedRectangles[i].leftDelta = this->stream.in_sint16_le();
                        this->multiscrblt.deltaEncodedRectangles[i].topDelta  = this->stream.in_sint16_le();
                        this->multiscrblt.deltaEncodedRectangles[i].width     = this->stream.in_sint16_le();
                        this->multiscrblt.deltaEncodedRectangles[i].height    = this->stream.in_sint16_le();
                    }
                }
            break;

            case LAST_IMAGE_CHUNK:
            case PARTIAL_IMAGE_CHUNK:
            {
                if (this->nbconsumers){
                    InChunkedImageTransport chunk_trans(this->chunk_type, this->chunk_size, this->trans);

                    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
                    png_set_read_fn(ppng, &chunk_trans, &png_read_data_fn);
                    png_info * pinfo = png_create_info_struct(ppng);
                    png_read_info(ppng, pinfo);

                    size_t height = png_get_image_height(ppng, pinfo);
                    const size_t width = screen_rect.cx;
                    TODO("check png row_size is identical to drawable rowsize");

                    uint32_t tmp[8192];
                    for (size_t k = 0; k < height; ++k) {
                        png_read_row(ppng, reinterpret_cast<uint8_t*>(tmp), nullptr);

                        uint32_t bgrtmp[8192];
                        const uint32_t * s = reinterpret_cast<const uint32_t*>(tmp);
                        uint32_t * t = bgrtmp;
                        for (size_t n = 0; n < (width / 4); n++){
                            unsigned bRGB = *s++;
                            unsigned GBrg = *s++;
                            unsigned rgbR = *s++;
                            *t++ = ((GBrg << 16) & 0xFF000000)
                               | ((bRGB << 16) & 0x00FF0000)
                               | (bRGB         & 0x0000FF00)
                               | ((bRGB >> 16) & 0x000000FF);
                            *t++ = (GBrg         & 0xFF000000)
                               | ((rgbR << 16) & 0x00FF0000)
                               | ((bRGB >> 16) & 0x0000FF00)
                               | ( GBrg        & 0x000000FF);
                            *t++ = ((rgbR << 16) & 0xFF000000)
                               | (rgbR         & 0x00FF0000)
                               | ((rgbR >> 16) & 0x0000FF00)
                               | ((GBrg >> 16) & 0x000000FF);
                        }

                        for (size_t cu = 0; cu < this->nbconsumers; cu++){
                            if (this->consumers[cu].capture_device) {
                                this->consumers[cu].capture_device->set_row(k, reinterpret_cast<uint8_t*>(bgrtmp));
                            }
                        }
                    }
                    png_read_end(ppng, pinfo);
                    TODO("is there a memory leak ? is info structure destroyed of not ?");
                    png_destroy_read_struct(&ppng, &pinfo, nullptr);
                }
                else {
                    REDOC("If no drawable is available ignore images chunks");
                    this->stream.rewind();
                    std::size_t sz = this->chunk_size - HEADER_SIZE;
                    auto end = this->stream_buf;
                    this->trans->recv(&end, sz);
                    this->stream = InStream(this->stream_buf, sz, sz);
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

                if (this->verbose > 32){
                    bitmap_data.log(LOG_INFO, "         ");
                }

                for (size_t i = 0; i < this->nbconsumers; i++) {
                    this->consumers[i].graphic_device->draw( bitmap_data
                                            , data
                                            , bitmap_data.bitmap_size()
                                            , bitmap);
                }
            }
            break;
            case POINTER:
            {
                uint8_t          cache_idx;

                this->mouse_x = this->stream.in_uint16_le();
                this->mouse_y = this->stream.in_uint16_le();
                cache_idx     = this->stream.in_uint8();

                if (  chunk_size - 8 /*header(8)*/
                    > 5 /*mouse_x(2) + mouse_y(2) + cache_idx(1)*/) {
                    this->statistics.CachePointer++;
                    struct Pointer cursor(Pointer::POINTER_NULL);
                    cursor.width = 32;
                    cursor.height = 32;
                    cursor.bpp = 24;
                    cursor.x = this->stream.in_uint8();
                    cursor.y = this->stream.in_uint8();
                    stream.in_copy_bytes(cursor.data, 32 * 32 * 3);
                    stream.in_copy_bytes(cursor.mask, 128);

                    this->ptr_cache.add_pointer_static(cursor, cache_idx);

                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->server_set_pointer(cursor);
                    }
                }
                else {
                    this->statistics.PointerIndex++;
                    Pointer & pi = this->ptr_cache.Pointers[cache_idx];
                    Pointer cursor(Pointer::POINTER_NULL);
                    cursor.width = 32;
                    cursor.height = 32;
                    cursor.bpp = 24;
                    cursor.x = pi.x;
                    cursor.y = pi.y;
                    memcpy(cursor.data, pi.data, sizeof(pi.data));
                    memcpy(cursor.mask, pi.mask, sizeof(pi.mask));

                    for (size_t i = 0; i < this->nbconsumers; i++) {
                        this->consumers[i].graphic_device->server_set_pointer(cursor);
                    }
                }
            }
            break;
            case RESET_CHUNK:
                this->info_compression_algorithm = 0;

                this->trans = this->trans_source;
            break;
            case SESSION_UPDATE:
                this->stream.in_timeval_from_uint64le_usec(this->record_now);

                for (size_t i = 0; i < this->nbconsumers; i++){
                    if (this->consumers[i].capture_device) {
                        this->consumers[i].capture_device->external_time(this->record_now);
                    }
                }

                {
                    uint16_t message_length = this->stream.in_uint16_le();

                    const char * message =  ::char_ptr_cast(this->stream.get_current()); // Null-terminator is included.

                    this->stream.in_skip_bytes(message_length);

                    for (size_t i = 0; i < this->nbconsumers; i++){
                        if (this->consumers[i].capture_device) {
                            this->consumers[i].capture_device->session_update(
                                this->record_now, message);
                        }
                    }
                }

                if (!this->timestamp_ok) {
                   if (this->real_time) {
                        this->start_record_now   = this->record_now;
                        this->start_synctime_now = tvtime();
                    }
                    this->timestamp_ok = true;
                }
                else {
                   if (this->real_time) {
                        for (size_t i = 0; i < this->nbconsumers; i++) {
                            this->consumers[i].graphic_device->flush();
                        }

                        struct timeval now     = tvtime();
                        uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

                        uint64_t movie_elapsed = difftimeval(this->record_now, this->start_record_now);

                        if (elapsed < movie_elapsed) {
                            struct timespec wtime     = {
                                  static_cast<time_t>( (movie_elapsed - elapsed) / 1000000LL)
                                , static_cast<time_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                                };
                            struct timespec wtime_rem = { 0, 0 };

                            while ((nanosleep(&wtime, &wtime_rem) == -1) && (errno == EINTR)) {
                                wtime = wtime_rem;
                            }
                        }
                    }
                }
            break;
            default:
                LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
                throw Error(ERR_WRM);
        }
    }

    void play(bool const & requested_to_stop) {
        this->privplay([](time_t){}, requested_to_stop);
    }

    template<class CbUpdateProgress>
    void play(CbUpdateProgress update_progess, bool const & requested_to_stop) {
        time_t last_sent_record_now = 0;
        this->privplay([&](time_t record_now) {
                if (last_sent_record_now != this->record_now.tv_sec) {
                    update_progess(this->record_now.tv_sec);
                    last_sent_record_now = this->record_now.tv_sec;
                }
            }, requested_to_stop);
    }

private:
    template<class CbUpdateProgress>
    void privplay(CbUpdateProgress update_progess, bool const & requested_to_stop) {
        while (!requested_to_stop && this->next_order()) {
            if (this->verbose > 8) {
                LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                   , unsigned(this->record_now.tv_sec), unsigned(this->total_orders_count));
            }
            this->interpret_order();
            if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                for (size_t i = 0; i < this->nbconsumers; i++) {
                    if (this->consumers[i].capture_device) {
                        this->consumers[i].capture_device->snapshot( this->record_now, this->mouse_x, this->mouse_y
                                                                   , this->ignore_frame_in_timeval
                                                                   , requested_to_stop);
                    }
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
};

#endif
