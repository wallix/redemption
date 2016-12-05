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
   Author(s): Christophe Grosjean, Jonathan Poelen, Raphael Zhou
*/


#pragma once

#include "utils/colors.hpp"
#include "utils/compression_transport_wrapper.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/RDPSerializer.hpp"
#include "core/RDP/share.hpp"
#include "wrm_label.hpp"

#include "gdi/dump_png24.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"

#include "RDPChunkedDevice.hpp"
#include "utils/compression_transport_wrapper.hpp"

#include "cxx/attributes.hpp"
#include "capture/utils/save_state_chunk.hpp"

#include "utils/stream.hpp"
#include "transport/transport.hpp"
#include "wrm_label.hpp"


inline void send_wrm_chunk(Transport & t, uint16_t chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<8> header;
    header.out_uint16_le(chunktype);
    header.out_uint32_le(8 + data_size);
    header.out_uint16_le(count);
    t.send(header.get_data(), header.get_offset());
}


inline void send_meta_chunk(
    Transport & t
  , uint8_t wrm_format_version

  , uint16_t info_width
  , uint16_t info_height
  , uint16_t info_bpp

  , uint16_t info_cache_0_entries
  , uint16_t info_cache_0_size
  , uint16_t info_cache_1_entries
  , uint16_t info_cache_1_size
  , uint16_t info_cache_2_entries
  , uint16_t info_cache_2_size

  , uint16_t info_number_of_cache
  , bool     info_use_waiting_list

  , bool     info_cache_0_persistent
  , bool     info_cache_1_persistent
  , bool     info_cache_2_persistent

  , uint16_t info_cache_3_entries
  , uint16_t info_cache_3_size
  , bool     info_cache_3_persistent
  , uint16_t info_cache_4_entries
  , uint16_t info_cache_4_size
  , bool     info_cache_4_persistent

  , uint8_t  index_algorithm
) {
    StaticOutStream<36> payload;
    payload.out_uint16_le(wrm_format_version);
    payload.out_uint16_le(info_width);
    payload.out_uint16_le(info_height);
    payload.out_uint16_le(info_bpp);

    payload.out_uint16_le(info_cache_0_entries);
    payload.out_uint16_le(info_cache_0_size);
    payload.out_uint16_le(info_cache_1_entries);
    payload.out_uint16_le(info_cache_1_size);
    payload.out_uint16_le(info_cache_2_entries);
    payload.out_uint16_le(info_cache_2_size);

    if (wrm_format_version > 3) {
        payload.out_uint8(info_number_of_cache);
        payload.out_uint8(info_use_waiting_list);

        payload.out_uint8(info_cache_0_persistent);
        payload.out_uint8(info_cache_1_persistent);
        payload.out_uint8(info_cache_2_persistent);

        payload.out_uint16_le(info_cache_3_entries);
        payload.out_uint16_le(info_cache_3_size);
        payload.out_uint8(info_cache_3_persistent);
        payload.out_uint16_le(info_cache_4_entries);
        payload.out_uint16_le(info_cache_4_size);
        payload.out_uint8(info_cache_4_persistent);

        payload.out_uint8(index_algorithm);
    }

    send_wrm_chunk(t, META_FILE, payload.get_offset(), 1);
    t.send(payload.get_data(), payload.get_offset());
}


struct ChunkToFile : public RDPChunkedDevice {
private:
    CompressionOutTransportWrapper compression_wrapper;
    Transport & trans_target;
    Transport & trans;

    const uint8_t wrm_format_version;

    uint16_t info_version = 0;

public:
    ChunkToFile(Transport * trans

               , uint16_t info_width
               , uint16_t info_height
               , uint16_t info_bpp
               , uint16_t info_cache_0_entries
               , uint16_t info_cache_0_size
               , uint16_t info_cache_1_entries
               , uint16_t info_cache_1_size
               , uint16_t info_cache_2_entries
               , uint16_t info_cache_2_size

               , uint16_t info_number_of_cache
               , bool     info_use_waiting_list

               , bool     info_cache_0_persistent
               , bool     info_cache_1_persistent
               , bool     info_cache_2_persistent

               , uint16_t info_cache_3_entries
               , uint16_t info_cache_3_size
               , bool     info_cache_3_persistent
               , uint16_t info_cache_4_entries
               , uint16_t info_cache_4_size
               , bool     info_cache_4_persistent

               , WrmCompressionAlgorithm wrm_compression_algorithm)
    : RDPChunkedDevice()
    , compression_wrapper(*trans, wrm_compression_algorithm)
    , trans_target(*trans)
    , trans(this->compression_wrapper.get())
    , wrm_format_version(bool(this->compression_wrapper.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_wrapper.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , info_width
          , info_height
          , info_bpp
          , info_cache_0_entries
          , info_cache_0_size
          , info_cache_1_entries
          , info_cache_1_size
          , info_cache_2_entries
          , info_cache_2_size

          , info_number_of_cache
          , info_use_waiting_list

          , info_cache_0_persistent
          , info_cache_1_persistent
          , info_cache_2_persistent

          , info_cache_3_entries
          , info_cache_3_size
          , info_cache_3_persistent
          , info_cache_4_entries
          , info_cache_4_size
          , info_cache_4_persistent

          , static_cast<unsigned>(this->compression_wrapper.get_algorithm())
        );
    }

public:
    void chunk(uint16_t chunk_type, uint16_t chunk_count, InStream stream) override {
        switch (chunk_type) {
        case META_FILE:
            {
                this->info_version                  = stream.in_uint16_le();
                uint16_t info_width                 = stream.in_uint16_le();
                uint16_t info_height                = stream.in_uint16_le();
                uint16_t info_bpp                   = stream.in_uint16_le();
                uint16_t info_cache_0_entries       = stream.in_uint16_le();
                uint16_t info_cache_0_size          = stream.in_uint16_le();
                uint16_t info_cache_1_entries       = stream.in_uint16_le();
                uint16_t info_cache_1_size          = stream.in_uint16_le();
                uint16_t info_cache_2_entries       = stream.in_uint16_le();
                uint16_t info_cache_2_size          = stream.in_uint16_le();

                uint16_t info_number_of_cache       = 3;
                bool     info_use_waiting_list      = false;

                bool     info_cache_0_persistent    = false;
                bool     info_cache_1_persistent    = false;
                bool     info_cache_2_persistent    = false;

                uint16_t info_cache_3_entries       = 0;
                uint16_t info_cache_3_size          = 0;
                bool     info_cache_3_persistent    = false;
                uint16_t info_cache_4_entries       = 0;
                uint16_t info_cache_4_size          = 0;
                bool     info_cache_4_persistent    = false;

                if (this->info_version > 3) {
                    info_number_of_cache            = stream.in_uint8();
                    info_use_waiting_list           = (stream.in_uint8() ? true : false);

                    info_cache_0_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_1_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_2_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_3_entries            = stream.in_uint16_le();
                    info_cache_3_size               = stream.in_uint16_le();
                    info_cache_3_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_4_entries            = stream.in_uint16_le();
                    info_cache_4_size               = stream.in_uint16_le();
                    info_cache_4_persistent         = (stream.in_uint8() ? true : false);

                    //uint8_t info_compression_algorithm = stream.in_uint8();
                    //REDASSERT(info_compression_algorithm < 3);
                }


                send_meta_chunk(
                    this->trans_target
                  , this->wrm_format_version

                  , info_width
                  , info_height
                  , info_bpp
                  , info_cache_0_entries
                  , info_cache_0_size
                  , info_cache_1_entries
                  , info_cache_1_size
                  , info_cache_2_entries
                  , info_cache_2_size

                  , info_number_of_cache
                  , info_use_waiting_list

                  , info_cache_0_persistent
                  , info_cache_1_persistent
                  , info_cache_2_persistent

                  , info_cache_3_entries
                  , info_cache_3_size
                  , info_cache_3_persistent
                  , info_cache_4_entries
                  , info_cache_4_size
                  , info_cache_4_persistent

                  , static_cast<unsigned>(this->compression_wrapper.get_algorithm())
                );
            }
            break;

        case SAVE_STATE:
            {
                SaveStateChunk ssc;

                ssc.recv(stream, this->info_version);

                StaticOutStream<65536> payload;

                ssc.send(payload);

                send_wrm_chunk(this->trans, SAVE_STATE, payload.get_offset(), chunk_count);
                this->trans.send(payload.get_data(), payload.get_offset());
            }
            break;

        case RESET_CHUNK:
            {
                send_wrm_chunk(this->trans, RESET_CHUNK, 0, 1);
                this->trans.next();
            }
            break;

        case TIMESTAMP:
            {
                timeval record_now;
                stream.in_timeval_from_uint64le_usec(record_now);
                this->trans_target.timestamp(record_now);
            }
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            {
                send_wrm_chunk(this->trans, chunk_type, stream.get_capacity(), chunk_count);
                this->trans.send(stream.get_data(), stream.get_capacity());
            }
            break;
        }
    }
};



template <size_t SZ>
class OutChunkedBufferingTransport : public Transport
{
    Transport & trans;
    size_t max;
    uint8_t buf[SZ];
    OutStream stream;

    static_assert(SZ >= 8, "");

public:
    explicit OutChunkedBufferingTransport(Transport & trans)
        : trans(trans)
        , max(SZ-8)
        , stream(buf)
    {
    }

    void flush() override {
        if (this->stream.get_offset() > 0) {
            send_wrm_chunk(this->trans, LAST_IMAGE_CHUNK, this->stream.get_offset(), 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            this->stream = OutStream(buf);
        }
    }

private:
    void do_send(const uint8_t * const buffer, size_t len) override {
        size_t to_buffer_len = len;
        while (this->stream.get_offset() + to_buffer_len > this->max) {
            send_wrm_chunk(this->trans, PARTIAL_IMAGE_CHUNK, this->max, 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
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

    CompressionOutTransportWrapper compression_wrapper;
    Transport & trans_target;
    Transport & trans;
    StaticOutStream<65536> buffer_stream_orders;
    StaticOutStream<65536> buffer_stream_bitmaps;

    const int dela_time;
    timeval last_sent_timer;
    timeval timer;
    const uint16_t width;
    const uint16_t height;
    uint16_t mouse_x;
    uint16_t mouse_y;
    const bool send_input;
    gdi::DumpPng24Api & dump_png24_api;


    uint8_t keyboard_buffer_32_buf[GTF_SIZE_KEYBUF_REC * sizeof(uint32_t)];
    // Extractor
    OutStream keyboard_buffer_32;

    const uint8_t wrm_format_version;

public:
    enum class SendInput { NO, YES };

    GraphicToFile(const timeval & now
                , Transport & trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  capture_bpp
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::DumpPng24Api & dump_png24
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , const int dela_time
                , SendInput send_input = SendInput::NO
                , Verbose verbose = Verbose::none)
    : RDPSerializer( this->buffer_stream_orders, this->buffer_stream_bitmaps, capture_bpp
                   , bmp_cache, gly_cache, ptr_cache, 0, 1, 1, 32 * 1024, verbose)
    , compression_wrapper(trans, wrm_compression_algorithm)
    , trans_target(trans)
    , trans(this->compression_wrapper.get())
    , dela_time(dela_time)
    , last_sent_timer()
    , timer(now)
    , width(width)
    , height(height)
    , mouse_x(0)
    , mouse_y(0)
    , send_input(send_input == SendInput::YES)
    , dump_png24_api(dump_png24)
    , keyboard_buffer_32(keyboard_buffer_32_buf)
    , wrm_format_version(bool(this->compression_wrapper.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_wrapper.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        last_sent_timer.tv_sec = 0;
        last_sent_timer.tv_usec = 0;
        this->order_count = 0;

        this->send_meta_chunk();
        this->send_image_chunk();
    }

    void dump_png24(Transport & trans, bool bgr) const {
        this->dump_png24_api.dump_png24(trans, bgr);
    }

    /// \brief Update timestamp but send nothing, the timestamp will be sent later with the next effective event
    void timestamp(const timeval& now)
    {
        uint64_t old_timer = this->timer.tv_sec * 1000000ULL + this->timer.tv_usec;
        uint64_t current_timer = now.tv_sec * 1000000ULL + now.tv_usec;
        if (old_timer < current_timer) {
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

    void enable_kbd_input_mask(bool) override {
    }

    void send_meta_chunk()
    {
        const BmpCache::cache_ & c0 = this->bmp_cache.get_cache(0);
        const BmpCache::cache_ & c1 = this->bmp_cache.get_cache(1);
        const BmpCache::cache_ & c2 = this->bmp_cache.get_cache(2);
        const BmpCache::cache_ & c3 = this->bmp_cache.get_cache(3);
        const BmpCache::cache_ & c4 = this->bmp_cache.get_cache(4);

        ::send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , this->width
          , this->height
          , this->capture_bpp

          , c0.entries()
          , c0.bmp_size()
          , c1.entries()
          , c1.bmp_size()
          , c2.entries()
          , c2.bmp_size()

          , this->bmp_cache.number_of_cache
          , this->bmp_cache.use_waiting_list

          , c0.persistent()
          , c1.persistent()
          , c2.persistent()

          , c3.entries()
          , c3.bmp_size()
          , c3.persistent()
          , c4.entries()
          , c4.bmp_size()
          , c4.persistent()

          , static_cast<unsigned>(this->compression_wrapper.get_algorithm())
        );
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(void)
    {
        OutChunkedBufferingTransport<65536> png_trans(this->trans);
        this->dump_png24_api.dump_png24(png_trans, false);
    }

    void send_reset_chunk()
    {
        send_wrm_chunk(this->trans, RESET_CHUNK, 0, 1);
    }

    void send_timestamp_chunk(bool ignore_time_interval = false)
    {
        StaticOutStream<12 + GTF_SIZE_KEYBUF_REC * sizeof(uint32_t) + 1> payload;
        payload.out_timeval_to_uint64le_usec(this->timer);
        if (this->send_input) {
            payload.out_uint16_le(this->mouse_x);
            payload.out_uint16_le(this->mouse_y);

            payload.out_uint8(ignore_time_interval ? 1 : 0);

/*
            for (uint32_t i = 0, c = keyboard_buffer_32.size() / sizeof(uint32_t);
                 i < c; i++) {
                LOG(LOG_INFO, "send_timestamp_chunk: '%c'(0x%X)",
                    (keyboard_buffer_32.data[i]<128)?(char)keyboard_buffer_32.data[i]:'?',
                    keyboard_buffer_32.data[i]);
            }
*/

            payload.out_copy_bytes(keyboard_buffer_32.get_data(), keyboard_buffer_32.get_offset());
            keyboard_buffer_32 = OutStream(keyboard_buffer_32_buf);
        }

        send_wrm_chunk(this->trans, TIMESTAMP, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->last_sent_timer = this->timer;
    }

    void send_save_state_chunk()
    {
        StaticOutStream<4096> payload;
        this->ssc.send(payload);

        //------------------------------ missing variable length ---------------

        send_wrm_chunk(this->trans, SAVE_STATE, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());
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
        if (bool(this->compression_wrapper.get_algorithm())) {
            this->send_reset_chunk();
        }
        this->trans.next();
        this->send_meta_chunk();
        this->send_timestamp_chunk();
        this->send_save_state_chunk();

        OutChunkedBufferingTransport<65536> png_trans(this->trans);

        this->dump_png24_api.dump_png24(png_trans, true);

        this->send_caches_chunk();
    }

protected:

    bool timeval_comp(timeval a, timeval b) {

        int sec = a.tv_sec - b.tv_sec;
        int usec = a.tv_usec - b.tv_usec;
        if (sec > 0) {
            return true;
        }
        if ( (sec == 0) && (usec > this->dela_time) ) {
            return true;
        }

        return false;
    }

    void flush_orders() override {
        if (this->order_count > 0) {
            if (timeval_comp(this->timer, this->last_sent_timer)) {
                this->send_timestamp_chunk();
            }
            this->send_orders_chunk();
        }
    }

public:
    void send_orders_chunk()
    {
        send_wrm_chunk(this->trans, RDP_UPDATE_ORDERS, this->stream_orders.get_offset(), this->order_count);
        this->trans.send(this->stream_orders.get_data(), this->stream_orders.get_offset());
        this->order_count = 0;
        this->stream_orders.rewind();
    }

protected:
    void flush_bitmaps() override {
        if (this->bitmap_count > 0) {
            if (timeval_comp(this->timer, this->last_sent_timer)) {
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
        send_wrm_chunk(this->trans, RDP_UPDATE_BITMAP, this->stream_bitmaps.get_offset(), this->bitmap_count);
        this->trans.send(this->stream_bitmaps.get_data(), this->stream_bitmaps.get_offset());
        this->bitmap_count = 0;
        this->stream_bitmaps.rewind();
    }

protected:
    void send_pointer(int cache_idx, const Pointer & cursor) override {
        size_t size =   2           // mouse x
                      + 2           // mouse y
                      + 1           // cache index
                      + 1           // hotspot x
                      + 1           // hotspot y
                      + 32 * 32 * 3 // data
                      + 128         // mask
                      ;
        send_wrm_chunk(this->trans, POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        payload.out_uint8(cursor.x);
        payload.out_uint8(cursor.y);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->trans.send(cursor.data, cursor.data_size());
        this->trans.send(cursor.mask, cursor.mask_size());
    }

    void set_pointer(int cache_idx) override {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index
                      ;
        send_wrm_chunk(this->trans, POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        this->trans.send(payload.get_data(), payload.get_offset());
    }

public:
    void session_update(timeval const & now, array_view_const_char message) override {
        uint16_t message_length = message.size() + 1;       // Null-terminator is included.

        StaticOutStream<16> payload;
        payload.out_timeval_to_uint64le_usec(now);
        payload.out_uint16_le(message_length);

        send_wrm_chunk(this->trans, SESSION_UPDATE, payload.get_offset() + message_length, 1);
        this->trans.send(payload.get_data(), payload.get_offset());
        this->trans.send(message.data(), message.size());
        this->trans.send("\0", 1);

        this->last_sent_timer = this->timer;
    }

    void possible_active_window_change() override {}

    using RDPSerializer::set_pointer;
};  // struct GraphicToFile

