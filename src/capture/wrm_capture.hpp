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

#include "gdi/image_frame_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/kbd_input_api.hpp"

#include "transport/out_meta_sequence_transport.hpp"

#include "utils/compression_transport_builder.hpp"
#include "utils/difftimeval.hpp"
#include "utils/genfstat.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/png.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <cstddef>


template<std::size_t SZ>
class OutChunkedBufferingTransport : public Transport
{
    Transport & trans;
    size_t max;
    uint8_t buf[SZ];
    OutStream stream;

    static_assert(SZ >= 8);

public:
    explicit OutChunkedBufferingTransport(Transport & trans)
    : trans(trans)
    , max(SZ-8)
    , stream(buf)
    {
    }

    void flush() override {
        if (this->stream.get_offset() > 0) {
            send_wrm_chunk(this->trans, WrmChunkType::LAST_IMAGE_CHUNK, this->stream.get_offset(), 1);
            this->trans.send(this->stream.get_bytes());
            this->stream = OutStream(buf);
        }
    }

private:
    void do_send(const uint8_t * const buffer, size_t len) override {
        size_t to_buffer_len = len;
        while (this->stream.get_offset() + to_buffer_len > this->max) {
            send_wrm_chunk(this->trans, WrmChunkType::PARTIAL_IMAGE_CHUNK, this->max, 1);
            this->trans.send(stream.get_bytes());
            size_t to_send = this->max - this->stream.get_offset();
            this->trans.send(buffer + len - to_buffer_len, to_send);
            to_buffer_len -= to_send;
            this->stream = OutStream(buf);
        }
        this->stream.out_copy_bytes(buffer + len - to_buffer_len, to_buffer_len);
    }
};


/**
 * To keep things easy all chunks have 8 bytes headers
 * starting with chunk_type, chunk_size and order_count
 *  (whatever it means, depending on chunks)
 */
class GraphicToFile
: public RDPSerializer
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
{
    enum {
        GTF_SIZE_KEYBUF_REC = 1024
    };

    CompressionOutTransportBuilder compression_bullder;
    Transport & trans_target;
    Transport & trans;
    StaticOutStream<65536> buffer_stream_orders;
    StaticOutStream<65536> buffer_stream_bitmaps;

    const std::chrono::microseconds delta_time = std::chrono::seconds(1);
    timeval timer;
    timeval last_sent_timer;
    uint16_t mouse_x;
    uint16_t mouse_y;
    const bool send_input;
    gdi::ImageFrameApi & image_frame_api;


    uint8_t keyboard_buffer_32_buf[GTF_SIZE_KEYBUF_REC * sizeof(uint32_t)];
    // Extractor
    OutStream keyboard_buffer_32;

    const uint8_t wrm_format_version;

    const bool remote_app;

public:
    enum class SendInput { NO, YES };

    GraphicToFile(const timeval & now
                , Transport & trans
                , const BitsPerPixel capture_bpp
                // TOSO strong type
                , const bool remote_app
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::ImageFrameApi & image_frame_api
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input = SendInput::NO
                , Verbose verbose = Verbose::none)
    : RDPSerializer( this->buffer_stream_orders, this->buffer_stream_bitmaps, capture_bpp
                   , bmp_cache, gly_cache, ptr_cache, 0, true, true, 32 * 1024, true, verbose)
    , compression_bullder(trans, wrm_compression_algorithm)
    , trans_target(trans)
    , trans(this->compression_bullder.get())
    , timer(now)
    , last_sent_timer{0, 0}
    , mouse_x(0)
    , mouse_y(0)
    , send_input(send_input == SendInput::YES)
    , image_frame_api(image_frame_api)
    , keyboard_buffer_32(keyboard_buffer_32_buf)
    , wrm_format_version(remote_app ? 5 : (bool(this->compression_bullder.get_algorithm()) ? 4 : 3))
    , remote_app(remote_app)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        this->order_count = 0;

        this->send_meta_chunk();
        this->send_image_chunk();
    }

    void dump_png24(Transport & trans, bool bgr) const
    {
        ::dump_png24(trans, this->image_frame_api, bgr);
    }

    /// \brief Update timestamp but send nothing, the timestamp will be sent later with the next effective event
    void timestamp(const timeval& now)
    {
        if (this->timer < now) {
            this->flush_orders();
            this->flush_bitmaps();
            this->timer = now;
            this->trans.timestamp(now);
        }
    }

    void mouse(uint16_t mouse_x, uint16_t mouse_y)
    {
        this->mouse_x = mouse_x;
        this->mouse_y = mouse_y;
    }

    bool kbd_input(const timeval & now, uint32_t uchar) override {
        (void)now;
        if (keyboard_buffer_32.has_room(sizeof(uint32_t))) {
            keyboard_buffer_32.out_uint32_le(uchar);
        }
        return true;
    }

    void enable_kbd_input_mask(bool /*enable*/) override {}

    void send_meta_chunk()
    {
        const BmpCache::cache_ & c0 = this->bmp_cache.get_cache(0);
        const BmpCache::cache_ & c1 = this->bmp_cache.get_cache(1);
        const BmpCache::cache_ & c2 = this->bmp_cache.get_cache(2);
        const BmpCache::cache_ & c3 = this->bmp_cache.get_cache(3);
        const BmpCache::cache_ & c4 = this->bmp_cache.get_cache(4);

        auto const image_view = image_frame_api.get_image_view();

        WrmMetaChunk{
            this->wrm_format_version

          , image_view.width()
          , image_view.height()
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

          , this->compression_bullder.get_algorithm()

          , this->remote_app
        }.send(this->trans_target);
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(bool bgr = false)
    {
        OutChunkedBufferingTransport<65536> png_trans(this->trans);
        ::dump_png24(png_trans, this->image_frame_api, bgr);
    }

    void send_reset_chunk()
    {
        send_wrm_chunk(this->trans, WrmChunkType::RESET_CHUNK, 0, 1);
    }

    void send_timestamp_chunk(bool ignore_time_interval = false)
    {
        StaticOutStream<12 + GTF_SIZE_KEYBUF_REC * sizeof(uint32_t) + 1> payload;
        payload.out_timeval_to_uint64le_usec(this->timer);
        if (this->send_input) {
            payload.out_uint16_le(this->mouse_x);
            payload.out_uint16_le(this->mouse_y);

            payload.out_uint8(ignore_time_interval ? 1 : 0);

            payload.out_copy_bytes(keyboard_buffer_32.get_bytes());
            keyboard_buffer_32 = OutStream(keyboard_buffer_32_buf);
        }

        send_wrm_chunk(this->trans, WrmChunkType::TIMESTAMP, payload.get_offset(), 1);
        this->trans.send(payload.get_bytes());

        this->last_sent_timer = this->timer;
    }

    void send_save_state_chunk()
    {
        StaticOutStream<4096> payload;
        SaveStateChunk ssc;
        ssc.send(payload, this->ssc);

        //------------------------------ missing variable length ---------------

        send_wrm_chunk(this->trans, WrmChunkType::SAVE_STATE, payload.get_offset(), 1);
        this->trans.send(payload.get_bytes());
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

    void save_ptr_cache() {
        for (int index = 0; index < MAX_POINTER_COUNT; ++index) {
            this->pointer_cache.set_cached(index, false);
        }
    }

    void send_caches_chunk()
    {
        this->save_bmp_caches();
        this->save_glyph_caches();
        this->save_ptr_cache();
        if (this->order_count > 0) {
            this->send_orders_chunk();
        }
    }

    void breakpoint()
    {
        this->flush_orders();
        this->flush_bitmaps();
        this->send_timestamp_chunk();
        if (bool(this->compression_bullder.get_algorithm())) {
            this->send_reset_chunk();
        }
        this->trans.next();
        this->send_meta_chunk();
        this->send_timestamp_chunk();
        this->send_save_state_chunk();
        this->send_image_chunk(true);
        this->send_caches_chunk();
    }

private:
    std::chrono::microseconds elapsed_time() const
    {
        return ustime(this->timer) - ustime(this->last_sent_timer);
    }

protected:
    void flush_orders() override {
        if (this->order_count > 0) {
            if (this->elapsed_time() >= delta_time) {
                this->send_timestamp_chunk();
            }
            this->send_orders_chunk();
        }
    }

public:
    void send_orders_chunk()
    {
        send_wrm_chunk(this->trans, WrmChunkType::RDP_UPDATE_ORDERS, this->stream_orders.get_offset(), this->order_count);
        this->trans.send(this->stream_orders.get_bytes());
        this->order_count = 0;
        this->stream_orders.rewind();
    }

protected:
    void flush_bitmaps() override {
        if (this->bitmap_count > 0) {
            if (this->elapsed_time() >= delta_time) {
                this->send_timestamp_chunk();
            }
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
        send_wrm_chunk(this->trans, WrmChunkType::RDP_UPDATE_BITMAP, this->stream_bitmaps.get_offset(), this->bitmap_count);
        this->trans.send(this->stream_bitmaps.get_bytes());
        this->bitmap_count = 0;
        this->stream_bitmaps.rewind();
    }

    void send_image_frame_rect_chunk(const Rect& max_image_frame_rect, const Dimension& min_image_frame_dim)
    {
        StaticOutStream<32> payload;
        payload.out_sint16_le(max_image_frame_rect.x);
        payload.out_sint16_le(max_image_frame_rect.y);
        payload.out_uint16_le(max_image_frame_rect.cx);
        payload.out_uint16_le(max_image_frame_rect.cy);

        payload.out_uint16_le(min_image_frame_dim.w);
        payload.out_uint16_le(min_image_frame_dim.h);

        send_wrm_chunk(this->trans, WrmChunkType::IMAGE_FRAME_RECT, payload.get_offset(), 0);
        this->trans.send(payload.get_bytes());
    }

protected:
    void send_pointer(int cache_idx, const Pointer & cursor) override {
        auto const dimensions = cursor.get_dimensions();
        StaticOutStream<32+108*96> payload;
        bool pointer32x32 = ((dimensions.width == 32) && (dimensions.height == 32));
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        if (pointer32x32) {
            cursor.emit_pointer32x32(payload);
        }
        else {
            cursor.emit_pointer2(payload);
        }
        send_wrm_chunk(this->trans, (pointer32x32)?WrmChunkType::POINTER:WrmChunkType::POINTER2, payload.get_offset(), 0);
        this->trans.send(payload.get_bytes());
    }

    void cached_pointer_update(int cache_idx) override {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index
                      ;
        send_wrm_chunk(this->trans, WrmChunkType::POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        this->trans.send(payload.get_bytes());
    }

public:
    void session_update(timeval const & now, array_view_const_char message) override {
        this->timer = now;
        this->last_sent_timer = this->timer;

        if (this->keyboard_buffer_32.get_offset()) {
            this->send_timestamp_chunk();
        }

        uint16_t message_length = message.size() + 1;       // Null-terminator is included.

        StaticOutStream<16> payload;
        payload.out_timeval_to_uint64le_usec(now);
        payload.out_uint16_le(message_length);

        send_wrm_chunk(this->trans, WrmChunkType::SESSION_UPDATE, payload.get_offset() + message_length, 1);
        this->trans.send(payload.get_bytes());
        this->trans.send(message.data(), message.size());
        this->trans.send("\0", 1);
    }

    void possible_active_window_change() override {
        if (this->keyboard_buffer_32.get_offset()) {
            this->send_timestamp_chunk();
        }

        send_wrm_chunk(this->trans, WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE, 0, 0);
    }
};  // struct GraphicToFile



class WrmCaptureImpl :
    public gdi::KbdInputApi,
    public gdi::CaptureApi,
    public gdi::GraphicApi,
    public gdi::CaptureProbeApi,
    public gdi::ExternalCaptureApi // from gdi/capture_api.hpp
{
    BmpCache     bmp_cache;
    GlyphCache   gly_cache;
    PointerCache ptr_cache;

    OutMetaSequenceTransport out;

    struct Serializer final : GraphicToFile {
        Serializer(const timeval & now
                , Transport & trans
                , const BitsPerPixel capture_bpp
                , const bool remote_app
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::ImageFrameApi & image_frame_api
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input
                , GraphicToFile::Verbose verbose)
            : GraphicToFile(now, trans, capture_bpp, remote_app,
                            bmp_cache, gly_cache, ptr_cache,
                            image_frame_api, wrm_compression_algorithm,
                            send_input, verbose)
        {}

        using GraphicToFile::draw;
        using GraphicToFile::capture_bpp;

        void draw(RDPNineGrid const & /*cmd*/, Rect /*rect*/, gdi::ColorCtx /*color_ctx*/, Bitmap const &  /*bmp*/) override {}

        void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
            auto compress_and_draw_bitmap_update = [&bitmap_data, this](const Bitmap & bmp) {
                StaticOutStream<65535> bmp_stream;
                bmp.compress(this->capture_bpp, bmp_stream);

                RDPBitmapData target_bitmap_data = bitmap_data;

                target_bitmap_data.bits_per_pixel = safe_int(bmp.bpp());
                target_bitmap_data.flags          = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;  /*NOLINT*/
                target_bitmap_data.bitmap_length  = bmp_stream.get_offset();

                GraphicToFile::draw(target_bitmap_data, bmp);
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
                GraphicToFile::draw(bitmap_data, bmp);
            }
        }

        void enable_kbd_input_mask(bool /*unused*/) override {}

        bool kbd_input(const timeval & now, uint32_t uchar) override {
            return this->GraphicToFile::kbd_input(now, uchar);
        }

        ~Serializer() = default;
    } graphic_to_file;

public:

    // EXTERNAL CAPTURE API
    void external_breakpoint() override {
        this->nc.external_breakpoint();
    }

    void external_time(timeval const & now) override {
        this->nc.external_time(now);
    }

    // CAPTURE PROBE API
    void session_update(timeval const & now, array_view_const_char message) override {
        this->graphic_to_file.session_update(now, message);
    }
    void possible_active_window_change() override { this->graphic_to_file.possible_active_window_change(); }

    // GRAPHIC API
    void draw(RDP::FrameMarker    const & cmd) override { this->graphic_to_file.draw(cmd);}
    void draw(RDPDestBlt          const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPScrBlt           const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, bmp);
    }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, clip, bmp);
    }
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx, bmp);
    }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx, gly_cache);
    }
    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(RDPColCache   const & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(RDPBrushCache const & cmd) override {
        this->graphic_to_file.draw(cmd);
    }

    void draw(RDPNineGrid const & /*cmd*/, Rect /*rect*/, gdi::ColorCtx /*color_ctx*/, Bitmap const & /*bmp*/) override {}


    // XXXXXXXXXXXXXX
    void set_pointer(Pointer const & ptr) override {
        this->graphic_to_file.set_pointer(ptr);
    }

    class NativeCaptureLocal : public gdi::CaptureApi, public gdi::ExternalCaptureApi
    {
        timeval start_native_capture;
        std::chrono::microseconds inter_frame_interval_native_capture;

        timeval start_break_capture;
        std::chrono::microseconds inter_frame_interval_start_break_capture;

        GraphicToFile & recorder;
        std::chrono::microseconds time_to_wait;

    public:
        NativeCaptureLocal(
            GraphicToFile & recorder,
            const timeval & now,
            std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
            std::chrono::seconds break_interval
        )
        : start_native_capture(now)
        , inter_frame_interval_native_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(frame_interval))
        , start_break_capture(now)
        , inter_frame_interval_start_break_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(break_interval))
        , recorder(recorder)
        , time_to_wait(std::chrono::microseconds::zero())
        {}

        ~NativeCaptureLocal() override {
            this->recorder.sync();
        }

        // toggles externally genareted breakpoint.
        void external_breakpoint() override {
            this->recorder.breakpoint();
        }

        void external_time(const timeval & now) override {
            this->recorder.sync();
            this->recorder.timestamp(now);
        }

        Microseconds periodic_snapshot(
            const timeval & now, int x, int y, bool ignore_frame_in_timeval
        ) override {
            (void)ignore_frame_in_timeval;
            if (difftimeval(now, this->start_native_capture)
                    >= this->inter_frame_interval_native_capture) {
                this->recorder.timestamp(now);
                this->time_to_wait = this->inter_frame_interval_native_capture;
                this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
                this->start_native_capture = now;
                if ((difftimeval(now, this->start_break_capture) >=
                     this->inter_frame_interval_start_break_capture)) {
                    this->recorder.breakpoint();
                    this->start_break_capture = now;
                }
            }
            else {
                this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
            }
            return std::chrono::microseconds{this->time_to_wait};
        }
    } nc;

    bool kbd_input_mask_enabled;

    Rect      max_image_frame_rect;
    Dimension min_image_frame_dim;

    WrmCaptureImpl(
        const CaptureParams & capture_params, const WrmParams & wrm_params,
        gdi::ImageFrameApi & image_frame_api, ConstImageDataView const & image_view)
    : bmp_cache(
        BmpCache::Recorder, wrm_params.capture_bpp, 3, false,
        BmpCache::CacheOption(600, 768, false),
        BmpCache::CacheOption(300, 3072, false),
        BmpCache::CacheOption(262, 12288, false))
    , ptr_cache(/*pointer_cache_entries=*/0x19)
    , out(
        wrm_params.cctx,
        wrm_params.rnd,
        wrm_params.fstat,
        capture_params.record_path,
        wrm_params.hash_path,
        capture_params.basename,
        capture_params.now,
        image_view.width(),
        image_view.height(),
        capture_params.groupid,
        capture_params.report_message)
    , graphic_to_file(
        capture_params.now, this->out, wrm_params.capture_bpp, wrm_params.remote_app,
        this->bmp_cache, this->gly_cache, this->ptr_cache, image_frame_api,
        wrm_params.wrm_compression_algorithm, GraphicToFile::SendInput::YES,
        GraphicToFile::Verbose(wrm_params.wrm_verbose))
    , nc(this->graphic_to_file, capture_params.now,
        wrm_params.frame_interval, wrm_params.break_interval)
    , kbd_input_mask_enabled{false}
    {}

public:
    WrmCaptureImpl(
        const CaptureParams & capture_params, const WrmParams & wrm_params,
        gdi::ImageFrameApi & image_frame_api)
    : WrmCaptureImpl(capture_params, wrm_params, image_frame_api, image_frame_api.get_image_view())
    {}

    ~WrmCaptureImpl() override
    {
        if (!this->max_image_frame_rect.isempty()) {
            this->graphic_to_file.send_image_frame_rect_chunk(this->max_image_frame_rect, this->min_image_frame_dim);
        }
    }

    // shadow text
    bool kbd_input(const timeval& now, uint32_t uchar) override {
        return this->graphic_to_file.kbd_input(now, this->kbd_input_mask_enabled?'*':uchar);
    }

    void enable_kbd_input_mask(bool enable) override {
        this->kbd_input_mask_enabled = enable;
    }

    void send_timestamp_chunk(timeval const & now, bool ignore_time_interval) {
        this->graphic_to_file.timestamp(now);
        this->graphic_to_file.send_timestamp_chunk(ignore_time_interval);
    }

    Microseconds periodic_snapshot(
        const timeval & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        return this->nc.periodic_snapshot(now, x, y, ignore_frame_in_timeval);
    }

    void visibility_rects_event(Rect rect) override {
        if (!rect.isempty()) {
            this->max_image_frame_rect = this->max_image_frame_rect.disjunct(rect);

            this->min_image_frame_dim.w = std::max(this->min_image_frame_dim.w, rect.cx);
            this->min_image_frame_dim.h = std::max(this->min_image_frame_dim.h, rect.cy);
        }
    }
};
