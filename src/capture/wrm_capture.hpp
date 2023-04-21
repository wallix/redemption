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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once

#include "capture/save_state_chunk.hpp"
#include "capture/wrm_meta_chunk.hpp"
#include "capture/wrm_params.hpp"

#include "capture/capture_params.hpp"

#include "core/RDP/RDPSerializer.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"

#include "gdi/capture_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/graphic_api_forwarder.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/relayout_api.hpp"

#include "transport/out_meta_sequence_transport.hpp"

#include "utils/compression_transport_builder.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/png.hpp"
#include "utils/ref.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/monotonic_time_to_real_time.hpp"
#include "utils/drawable.hpp"

#include <cstddef>


template<std::size_t SZ>
class OutChunkedBufferingTransport : public Transport
{
    static_assert(SZ >= 8);
    static size_t const max = SZ - 8;

    Transport & trans;
    StaticOutStream<SZ> stream;

public:
    explicit OutChunkedBufferingTransport(Transport & trans)
    : trans(trans)
    {
    }

    void flush() override {
        if (this->stream.get_offset() > 0) {
            send_wrm_chunk(this->trans, WrmChunkType::LAST_IMAGE_CHUNK, this->stream.get_offset(), 1);
            this->trans.send(this->stream.get_produced_bytes());
            this->stream.rewind();
        }
    }

private:
    void do_send(const uint8_t * const buffer, size_t len) override {
        size_t to_buffer_len = len;
        while (this->stream.get_offset() + to_buffer_len > this->max) {
            send_wrm_chunk(this->trans, WrmChunkType::PARTIAL_IMAGE_CHUNK, this->max, 1);
            this->trans.send(stream.get_produced_bytes());
            size_t to_send = this->max - this->stream.get_offset();
            this->trans.send(buffer + len - to_buffer_len, to_send);
            to_buffer_len -= to_send;
            this->stream.rewind();
        }
        this->stream.out_copy_bytes(buffer + len - to_buffer_len, to_buffer_len);
    }
};


/**
 * To keep things easy all chunks have 8 bytes headers
 * starting with chunk_type, chunk_size and order_count
 *  (whatever it means, depending on chunks)
 */
class GraphicToFile final : public RDPSerializer
{
    enum {
        GTF_SIZE_KEYBUF_REC = 1024
    };

    CompressionOutTransportBuilder compression_builder;
    Transport & trans_target;
    SequencedTransport & trans;
    StaticOutStream<65536> buffer_stream_orders;
    StaticOutStream<65536> buffer_stream_bitmaps;

    MonotonicTimePoint timer;
    MonotonicTimePoint last_sent_timer {};
    const MonotonicTimePoint start_timer {};
    MonotonicTimePoint monotonic_real_time {};
    RealTimePoint last_real_time {};
    // for a monotic real time
    uint16_t mouse_x = 0;
    uint16_t mouse_y = 0;
    Drawable const& drawable;


    uint8_t keyboard_buffer_32_buf[GTF_SIZE_KEYBUF_REC * sizeof(uint32_t)];
    // Extractor
    OutStream keyboard_buffer_32;

    const uint8_t wrm_format_version;

    const bool remote_app;
    Rect rail_window_rect;

    PointerCache::SourcePointersView ptr_cache;
    std::array<bool, PointerCache::Mapping::original_max_entries> ptr_cached {};

public:
    GraphicToFile(MonotonicTimePoint now
                , RealTimePoint real_now
                , SequencedTransport & trans
                , const BitsPerPixel capture_bpp
                // TODO strong type
                , const bool remote_app
                , Rect rail_window_rect
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache::SourcePointersView ptr_cache
                , CRef<Drawable> drawable_ref
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , RDPSerializerVerbose verbose)
    : RDPSerializer( this->buffer_stream_orders, this->buffer_stream_bitmaps, capture_bpp
                   , bmp_cache, gly_cache, 0, true, true, 32 * 1024, true, verbose)
    , compression_builder(trans, wrm_compression_algorithm)
    , trans_target(trans)
    , trans(this->compression_builder.get())
    , timer(now)
    , start_timer(now)
    , monotonic_real_time(now)
    , last_real_time(real_now)
    , drawable(drawable_ref)
    , keyboard_buffer_32(keyboard_buffer_32_buf)
    , wrm_format_version(remote_app ? 5 : (bool(this->compression_builder.get_algorithm()) ? 4 : 3))
    , remote_app(remote_app)
    , rail_window_rect(rail_window_rect)
    , ptr_cache(ptr_cache)
    {
        if (wrm_compression_algorithm != this->compression_builder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        this->send_meta_chunk();
        this->send_image_chunk();
        this->send_time_points();
    }

    MonotonicTimePoint current_timer() const
    {
        return this->timer;
    }

    void dump_png24(Transport & trans, bool bgr) const
    {
        ::dump_png24(trans, this->drawable, bgr);
    }

    /// \brief Update timestamp but send nothing, the timestamp will be sent later with the next effective event
    void timestamp(MonotonicTimePoint now)
    {
        assert(now >= this->timer);

        this->flush_orders();
        this->flush_bitmaps();
        this->timer = now;
    }

    void update_times(MonotonicTimePoint::duration monotonic_delay, RealTimePoint real_time)
    {
        this->update_times(this->start_timer + monotonic_delay, real_time);
    }

    void update_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
    {
        this->sync();
        this->timestamp(monotonic_time);
        this->monotonic_real_time = monotonic_time;
        this->last_real_time = real_time;
        this->send_time_points();
    }

private:
    static void write_us_time(OutStream& out, MonotonicTimePoint::duration duration)
    {
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        out.out_uint64_le(checked_int{us.count()});
    }

    void send_time_points()
    {
        StaticOutStream<8*2> payload;
        write_us_time(payload, this->timer - this->start_timer);
        write_us_time(payload, this->last_real_time.time_since_epoch());
        send_wrm_chunk(this->trans, WrmChunkType::TIMES, 16, 1);
        this->trans.send(payload.get_produced_bytes());
    }

public:
    void mouse(uint16_t mouse_x, uint16_t mouse_y)
    {
        this->mouse_x = mouse_x;
        this->mouse_y = mouse_y;
    }

    bool kbd_input(uint32_t uchar) {
        if (keyboard_buffer_32.has_room(sizeof(uint32_t))) {
            keyboard_buffer_32.out_uint32_le(uchar);
        }
        return true;
    }

    void enable_kbd_input_mask(bool enable) {
        send_wrm_chunk(this->trans, WrmChunkType::KBD_INPUT_MASK, sizeof(uint8_t), 1);
        this->trans.send((enable ? "\1" : "\0"), 1);
    }

    void send_meta_chunk()
    {
        const BmpCache::cache_ & c0 = this->bmp_cache.get_cache(0);
        const BmpCache::cache_ & c1 = this->bmp_cache.get_cache(1);
        const BmpCache::cache_ & c2 = this->bmp_cache.get_cache(2);
        const BmpCache::cache_ & c3 = this->bmp_cache.get_cache(3);
        const BmpCache::cache_ & c4 = this->bmp_cache.get_cache(4);

        WrmMetaChunk{
            this->wrm_format_version

          , this->drawable.width()
          , this->drawable.height()
          , this->capture_bpp

          , static_cast<uint16_t>(c0.entries())
          , c0.bmp_size()
          , static_cast<uint16_t>(c1.entries())
          , c1.bmp_size()
          , static_cast<uint16_t>(c2.entries())
          , c2.bmp_size()

          , static_cast<uint16_t>(c3.entries())
          , c3.bmp_size()
          , static_cast<uint16_t>(c4.entries())
          , c4.bmp_size()

          , this->bmp_cache.number_of_cache
          , this->bmp_cache.use_waiting_list

          , c0.persistent()
          , c1.persistent()
          , c2.persistent()

          , c3.persistent()
          , c4.persistent()

          , this->compression_builder.get_algorithm()

          , this->remote_app
        }.send(this->trans_target);

        if (this->remote_app && !this->rail_window_rect.isempty()) {
            StaticOutStream<32> payload;
            payload.out_sint16_le(rail_window_rect.x);
            payload.out_sint16_le(rail_window_rect.y);
            payload.out_uint16_le(rail_window_rect.cx);
            payload.out_uint16_le(rail_window_rect.cy);
            send_wrm_chunk(this->trans, WrmChunkType::RAIL_WINDOW_RECT,
                payload.get_offset(), 0);
            this->trans.send(payload.get_produced_bytes());
        }
    }

    void visibility_rects_event(Rect rect)
    {
        this->rail_window_rect = rect;
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(bool bgr = false) /*NOLINT*/
    {
        OutChunkedBufferingTransport<65536> png_trans(this->trans);
        ::dump_png24(png_trans, this->drawable, bgr);
    }

    void send_reset_chunk()
    {
        send_wrm_chunk(this->trans, WrmChunkType::RESET_CHUNK, 0, 1);
    }

    void send_timestamp_chunk()
    {
        StaticOutStream<12 + GTF_SIZE_KEYBUF_REC * sizeof(uint32_t) + 1> payload;

        write_us_time(payload, this->timer - this->start_timer);

        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);

        payload.out_uint8(/*ignore_time_interval*/ 0);

        payload.out_copy_bytes(keyboard_buffer_32.get_produced_bytes());
        keyboard_buffer_32 = OutStream(keyboard_buffer_32_buf);

        send_wrm_chunk(this->trans, WrmChunkType::TIMESTAMP_OR_RECORD_DELAY, payload.get_offset(), 1);
        this->trans.send(payload.get_produced_bytes());

        this->last_sent_timer = this->timer;
    }

    void send_save_state_chunk()
    {
        StaticOutStream<4096> payload;
        SaveStateChunk ssc;
        ssc.send(payload, this->ssc);

        //------------------------------ missing variable length ---------------

        send_wrm_chunk(this->trans, WrmChunkType::SAVE_STATE, payload.get_offset(), 1);
        this->trans.send(payload.get_produced_bytes());
    }

    void save_bmp_caches()
    {
        for (uint8_t cache_id = 0
        ; cache_id < this->bmp_cache.number_of_cache
        ; ++cache_id) {
            const size_t entries = this->bmp_cache.get_cache(cache_id).entries();
            for (size_t i = 0; i < entries; i++) {
                this->bmp_cache.set_cached(cache_id, i, false);
            }
        }
    }

    void save_glyph_caches()
    {
        for (uint8_t cacheId = 0; cacheId < NUMBER_OF_GLYPH_CACHES; ++cacheId) {
            for (uint8_t cacheIndex = 0; cacheIndex < NUMBER_OF_GLYPH_CACHE_ENTRIES; ++cacheIndex) {
                this->glyph_cache.set_cached(cacheId, cacheIndex, false);
            }
        }
    }

    void breakpoint()
    {
        this->flush_orders();
        this->flush_bitmaps();
        this->send_timestamp_chunk();
        if (bool(this->compression_builder.get_algorithm())) {
            this->send_reset_chunk();
        }
        this->trans.next();
        this->last_real_time += this->timer - this->monotonic_real_time;
        this->monotonic_real_time = this->timer;
        this->send_meta_chunk();
        this->send_time_points();
        this->send_timestamp_chunk();
        this->send_save_state_chunk();
        this->send_image_chunk(true);
        this->save_bmp_caches();
        this->save_glyph_caches();
        this->ptr_cached.fill(false);
        if (this->order_count > 0) {
            this->send_orders_chunk();
        }
    }

private:
    void send_elapsed_time()
    {
        if (this->timer >= this->last_sent_timer + std::chrono::seconds(1)) {
            this->send_timestamp_chunk();
        }
    }

protected:
    void flush_orders() override {
        if (this->order_count > 0) {
            this->send_elapsed_time();
            this->send_orders_chunk();
        }
    }

public:
    void send_orders_chunk()
    {
        send_wrm_chunk(this->trans, WrmChunkType::RDP_UPDATE_ORDERS, this->stream_orders.get_offset(), this->order_count);
        this->trans.send(this->stream_orders.get_produced_bytes());
        this->order_count = 0;
        this->stream_orders.rewind();
    }

protected:
    void flush_bitmaps() override {
        if (this->bitmap_count > 0) {
            this->send_elapsed_time();
            this->send_bitmaps_chunk();
        }
    }

public:
    void sync() override {
        this->flush_bitmaps();
        this->flush_orders();
    }

    void send_bitmaps_chunk()
    {
        send_wrm_chunk(this->trans, WrmChunkType::RDP_UPDATE_BITMAP2, this->stream_bitmaps.get_offset(), this->bitmap_count);
        this->trans.send(this->stream_bitmaps.get_produced_bytes());
        this->bitmap_count = 0;
        this->stream_bitmaps.rewind();
    }

private:
    void send_pointer(uint16_t cache_idx, RdpPointerView const& cursor)
    {
        assert(cursor.xor_bits_per_pixel() != BitsPerPixel{0});

        StaticOutStream<32+RdpPointer::MAX_WIDTH*RdpPointer::MAX_HEIGHT*::nbbytes(RdpPointer::MAX_BPP)> payload;

        payload.out_uint16_le(safe_int(cursor.xor_bits_per_pixel()));

        payload.out_uint16_le(cache_idx);

        const auto hotspot = cursor.hotspot();

        payload.out_uint16_le(hotspot.x);
        payload.out_uint16_le(hotspot.y);

        auto const dimensions = cursor.dimensions();

        payload.out_uint16_le(dimensions.width);
        payload.out_uint16_le(dimensions.height);

        auto av_and_mask = cursor.and_mask();
        auto av_xor_mask = cursor.xor_mask();

        payload.out_uint16_le(av_and_mask.size());
        payload.out_uint16_le(av_xor_mask.size());

        payload.out_copy_bytes(av_xor_mask);
        payload.out_copy_bytes(av_and_mask); /* mask */

        if (payload.get_offset() % 2 != 0) {
            payload.out_clear_bytes(1);
        }

        send_wrm_chunk(this->trans, WrmChunkType::POINTER_NATIVE, payload.get_offset(), 0);
        this->trans.send(payload.get_produced_bytes());
    }

    void cached_pointer_update(uint16_t cache_idx)
    {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index
                      ;
        send_wrm_chunk(this->trans, WrmChunkType::POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        this->trans.send(payload.get_produced_bytes());
    }

public:
    void cached_pointer(gdi::CachePointerIndex cache_idx) override
    {
        if (!cache_idx.is_predefined_pointer()) {
            const auto idx = cache_idx.cache_index();
            if (!ptr_cached[idx]) {
                this->send_pointer(idx, this->ptr_cache.pointer(cache_idx));
                ptr_cached[idx] = true;
            }

            size_t size = 2  // mouse x
                        + 2  // mouse y
                        + 1  // cache index
                        ;
            send_wrm_chunk(this->trans, WrmChunkType::POINTER, size, 0);

            StaticOutStream<16> payload;
            payload.out_uint16_le(this->mouse_x);
            payload.out_uint16_le(this->mouse_y);
            payload.out_uint8(idx);
            this->trans.send(payload.get_produced_bytes());
        }
        else {
            size_t size = 2  // mouse x
                        + 2  // mouse y
                        + 1  // pointer type
                        ;
            send_wrm_chunk(this->trans, WrmChunkType::INTERNAL_POINTER, size, 0);

            StaticOutStream<16> payload;
            payload.out_uint16_le(this->mouse_x);
            payload.out_uint16_le(this->mouse_y);
            payload.out_uint8(safe_int(cache_idx.as_predefined_pointer()));
            this->trans.send(payload.get_produced_bytes());
        }
    }

    void new_pointer(gdi::CachePointerIndex cache_idx, RdpPointerView const& cursor) override
    {
        // assume that ptr_cache is updated from the outside
        (void)cursor;

        if (!cache_idx.is_predefined_pointer()) {
            const auto idx = cache_idx.cache_index();
            ptr_cached[idx] = false;
        }
    }

    void session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list)
    {
        if (this->timer < now) {
            this->timer = now;
            this->last_sent_timer = this->timer;
        }

        if (this->keyboard_buffer_32.get_offset()) {
            this->send_timestamp_chunk();
        }

        StaticOutStream<1024*16> out_stream;
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - this->start_timer);
        out_stream.out_sint64_le(checked_int{us.count()});
        OutStream kvheader(out_stream.out_skip_bytes(2 + 4 + 1));

        uint8_t kv_len = checked_int(kv_list.size());

        for (auto& kv : kv_list.first(kv_len)) {
            if (not out_stream.has_room(1 + 2 + kv.key.size() + kv.value.size())) {
                LOG(LOG_ERR, "WrmCapture::session_update(): message truncated");
                kv_len = uint8_t(&kv - kv_list.begin());
                break;
            }
            uint8_t const key_len = checked_int(kv.key.size());
            uint16_t const value_len = checked_int(kv.value.size());
            out_stream.out_uint8(key_len);
            out_stream.out_uint16_le(value_len);
            out_stream.out_copy_bytes(kv.key.first(key_len));
            out_stream.out_copy_bytes(kv.value.first(value_len));
        }

        kvheader.out_uint16_le(out_stream.get_offset() - 8 - 2);
        kvheader.out_uint32_le(safe_int(id));
        kvheader.out_uint8(kv_len);

        send_wrm_chunk(this->trans, WrmChunkType::SESSION_UPDATE, out_stream.get_offset(), 1);
        this->trans.send(out_stream.get_produced_bytes());
    }

    void possible_active_window_change() {
        if (this->keyboard_buffer_32.get_offset()) {
            this->send_timestamp_chunk();
        }

        send_wrm_chunk(this->trans, WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE, 0, 0);
    }

    void relayout(MonitorLayoutPDU const & monitor_layout_pdu) {
        send_wrm_chunk(this->trans, WrmChunkType::MONITOR_LAYOUT, monitor_layout_pdu.size(), 1);

        StaticOutStream<1024> payload;
        monitor_layout_pdu.emit(payload);

        this->trans.send(payload.get_produced_bytes());
    }

    using RDPSerializer::draw;

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override
    {
        auto compress_and_draw_bitmap_update = [&bitmap_data, this](const Bitmap & bmp) {
            size_t linesPerPacket = (16384 / bmp.line_size());

            // TODO same to front.hpp ?
            for (uint16_t yoff = 0; yoff < bitmap_data.height; yoff += linesPerPacket) {
                uint16_t currentHeight = linesPerPacket;
                if (yoff + linesPerPacket > bitmap_data.height) {
                    currentHeight = bitmap_data.height - yoff;
                }
                Rect subRect(0, yoff, bitmap_data.width, currentHeight);
                // LOG(LOG_ERR, "subRect: (%d,%d) - %dx%d", subRect.x, subRect.y, subRect.cx, subRect.cy);
                StaticOutStream<65535> bmp_stream;
                Bitmap subBmp(bmp, subRect);

                subBmp.compress(this->capture_bpp, bmp_stream);

                RDPBitmapData target_bitmap_data = bitmap_data;
                target_bitmap_data.dest_top = bitmap_data.dest_top + yoff;
                target_bitmap_data.dest_bottom = target_bitmap_data.dest_top + currentHeight - 1;
                target_bitmap_data.height = currentHeight;
                target_bitmap_data.bits_per_pixel = safe_int(bmp.bpp());
                target_bitmap_data.flags          = uint16_t(BITMAP_COMPRESSION)
                                                | uint16_t(NO_BITMAP_COMPRESSION_HDR);
                target_bitmap_data.bitmap_length  = bmp_stream.get_offset();

                RDPSerializer::draw(target_bitmap_data, subBmp);
            }
        };

        if (bmp.bpp() > this->capture_bpp) {
            // reducing the color depth of image.
            Bitmap capture_bmp(this->capture_bpp, bmp);
            compress_and_draw_bitmap_update(capture_bmp);
        }
        else if (!bmp.has_data_compressed()) {
            compress_and_draw_bitmap_update(bmp);
        }
        else {
            RDPSerializer::draw(bitmap_data, bmp);
        }
    }

    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const &content) override
    {
        /* no remoteFx support in recording, transcode to bitmapUpdates */
        for (const Rect & rect : content.region.rects) {
            // LOG(LOG_INFO, "draw(RDPSetSurfaceCommand cmd, RDPSurfaceContent const &content) stride=%u, rect=%s",
            //     content.stride, rect);
            Bitmap bitmap(content.data, content.stride, rect.get_dimension());
            RDPBitmapData bitmap_data;
            bitmap_data.dest_left = cmd.destRect.x + rect.ileft();
            bitmap_data.dest_right = cmd.destRect.x + rect.eright()-1;
            bitmap_data.dest_top = cmd.destRect.y + rect.itop();
            bitmap_data.dest_bottom = cmd.destRect.y + rect.ebottom()-1;

            bitmap_data.width = bitmap.cx();
            bitmap_data.height = bitmap.cy();
            bitmap_data.bits_per_pixel = 32;
            bitmap_data.flags = /*NO_BITMAP_COMPRESSION_HDR*/ 0;
            bitmap_data.bitmap_length = bitmap.bmp_size();

            this->draw(bitmap_data, bitmap);
        }
    }
};  // struct GraphicToFile

struct WrmCaptureDataImpl
{
    MonotonicTimePoint next_break;
    const std::chrono::seconds break_interval;
    const MonotonicTimeToRealTime original_monotonic_to_real;

    bool kbd_input_mask_enabled;

    BmpCache     bmp_cache;
    GlyphCache   gly_cache;

    OutMetaSequenceTransport out;
};

class WrmCaptureImpl final : WrmCaptureDataImpl,
    public gdi::KbdInputApi,
    public gdi::CaptureApi,
    public gdi::GraphicApiForwarder<GraphicToFile>,
    public gdi::CaptureProbeApi,
    public gdi::ExternalCaptureApi, // from gdi/capture_api.hpp
    public gdi::RelayoutApi
{
    void update_timestamp(MonotonicTimePoint now)
    {
        this->sink.timestamp(now);
        const auto tp = this->original_monotonic_to_real.to_real_time_point(now);
        this->out.timestamp(tp);
    }

public:
    // EXTERNAL CAPTURE API
    void external_breakpoint() override
    {
        this->sink.breakpoint();
    }

    void external_monotonic_time_point(MonotonicTimePoint now) override
    {
        this->sink.sync();
        this->update_timestamp(now);
    }

    void external_times(MonotonicTimePoint::duration monotonic_delay, RealTimePoint real_time) override
    {
        this->sink.update_times(monotonic_delay, real_time);
    }

    // CAPTURE PROBE API
    void session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list) override
    {
        this->sink.session_update(now, id, kv_list);
    }

    void possible_active_window_change() override
    {
        this->sink.possible_active_window_change();
    }

    WrmCaptureImpl(
        const CaptureParams & capture_params, const WrmParams & wrm_params,
        CRef<Drawable> drawable_ref, Rect rail_window_rect,
        PointerCache::SourcePointersView ptr_cache)
    : WrmCaptureDataImpl{
        .next_break = capture_params.now + wrm_params.break_interval,
        .break_interval = wrm_params.break_interval,
        .original_monotonic_to_real = MonotonicTimeToRealTime(capture_params.now, capture_params.real_now),
        .kbd_input_mask_enabled = false,
        .bmp_cache = BmpCache(
            BmpCache::Recorder, wrm_params.capture_bpp, 3, false,
            BmpCache::CacheOption(600, 768, false),
            BmpCache::CacheOption(300, 3072, false),
            BmpCache::CacheOption(262, 12288, false),
            BmpCache::CacheOption(),
            BmpCache::CacheOption(),
            BmpCache::Verbose::none
        ),
        .gly_cache = GlyphCache(),
        .out = OutMetaSequenceTransport(
            wrm_params.cctx,
            wrm_params.rnd,
            capture_params.record_path,
            wrm_params.hash_path,
            capture_params.basename,
            capture_params.real_now,
            drawable_ref.get().width(),
            drawable_ref.get().height(),
            capture_params.session_log,
            wrm_params.file_permissions
        )
    }
    , gdi::GraphicApiForwarder<GraphicToFile>(
        capture_params.now, capture_params.real_now,
        this->out, wrm_params.capture_bpp, wrm_params.remote_app,
        rail_window_rect, this->bmp_cache, this->gly_cache,
        ptr_cache, drawable_ref, wrm_params.wrm_compression_algorithm,
        wrm_params.wrm_verbose)
    {}

    void visibility_rects_event(Rect rect)
    {
        this->sink.visibility_rects_event(rect);
    }

    ~WrmCaptureImpl()
    {
        try {
            this->sink.sync();
        }
        catch (Error const& error) {
            LOG(LOG_ERR, "WrmCaptureImpl: error on destructor: %s", error.errmsg());
        }
    }

    // shadow text
    bool kbd_input(MonotonicTimePoint /*now*/, uint32_t uchar) override
    {
        return this->sink.kbd_input(this->kbd_input_mask_enabled ? '*' : uchar);
    }

    void enable_kbd_input_mask(bool enable) override
    {
        this->kbd_input_mask_enabled = enable;

        this->sink.enable_kbd_input_mask(enable);
    }

    void send_timestamp_chunk(MonotonicTimePoint now)
    {
        this->update_timestamp(now);
        this->sink.send_timestamp_chunk();
    }

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
    {
        this->sink.update_times(monotonic_time, real_time);
    }

    void update_mouse_position(uint16_t x, uint16_t y)
    {
        this->sink.mouse(x, y);
    }

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t x, uint16_t y
    ) override
    {
        this->sink.mouse(x, y);

        if (now >= this->next_break) {
            this->sink.breakpoint();
            this->next_break = now + this->break_interval;
        }

        return WaitingTimeBeforeNextSnapshot(this->next_break - now);
    }

    void relayout(MonitorLayoutPDU const & monitor_layout_pdu) override
    {
        this->sink.relayout(monitor_layout_pdu);
    }
};
