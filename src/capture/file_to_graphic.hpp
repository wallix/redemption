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

#include "capture/file_to_graphic_verbose.hpp"
#include "capture/wrm_meta_chunk.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/state_chunk.hpp"
#include "utils/compression_transport_builder.hpp"
#include "utils/monotonic_clock.hpp"
#include "utils/real_clock.hpp"
#include "utils/verbose_flags.hpp"


class Transport;
class BmpCache;
class InStream;

namespace gdi
{
    class GraphicApi;
    class CaptureApi;
    class KbdInputApi;
    class CaptureProbeApi;
    class ExternalCaptureApi;
    class RelayoutApi;
    class ResizeApi;
}

namespace RDP
{
    class AltsecDrawingOrderHeader;
}


class FileToGraphic
{
    uint8_t stream_buf[65536];
    InStream stream {stream_buf};

    CompressionInTransportBuilder compression_builder;

    Transport * trans_source;
    Transport * trans;

public:
    Dimension max_screen_dim;

private:
    // Internal state of orders
    StateChunk ssc;

    std::unique_ptr<BmpCache> bmp_cache;
    GlyphCache     gly_cache;
    PointerCache   ptr_cache {PointerCache::MAX_POINTER_COUNT};

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size = 0;
    WrmChunkType chunk_type = WrmChunkType::INVALID_CHUNK;
    uint16_t chunk_count = 0;
    uint16_t remaining_order_count = 0;

    // total number of RDP orders read from the start of the movie
    // (non orders chunks are counted as 1 order)
    uint32_t total_orders_count = 0;

    MonotonicTimePoint record_now {};

    MonotonicTimePoint monotonic_real_time {};
    RealTimePoint last_real_time {};

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

        void clear() { this->last = this->arr; }

        [[nodiscard]] std::size_t size() const noexcept {
            return static_cast<std::size_t>(this->last - this->arr);
        }

        [[nodiscard]] bool empty() const noexcept { return this->last == this->arr; }

    private:
        T * arr[N];
        T * * last = arr;
    };

    fixed_ptr_array<gdi::GraphicApi, 8> graphic_consumers;
    fixed_ptr_array<gdi::CaptureApi, 8> capture_consumers;
    fixed_ptr_array<gdi::KbdInputApi, 8> kbd_input_consumers;
    fixed_ptr_array<gdi::CaptureProbeApi, 8> capture_probe_consumers;
    fixed_ptr_array<gdi::ExternalCaptureApi, 8> external_event_consumers;
    fixed_ptr_array<gdi::RelayoutApi, 8> relayout_consumers;
    fixed_ptr_array<gdi::ResizeApi, 8> resize_consumers;

    bool meta_ok = false;
    bool timestamp_ok = false;
    uint16_t mouse_x = 0;
    uint16_t mouse_y = 0;

    WrmMetaChunk info {};

public:
    Rect      rail_wrm_window_rect;
    Rect      max_image_frame_rect;
    Dimension min_image_frame_dim;

    struct Statistics
    {
        struct Order
        {
            uint32_t count;
            uint64_t total_len;
        };

        Order DstBlt;
        Order MultiDstBlt;
        Order PatBlt;
        Order MultiPatBlt;
        Order OpaqueRect;
        Order MultiOpaqueRect;
        Order ScrBlt;
        Order MultiScrBlt;
        Order MemBlt;
        Order Mem3Blt;
        Order LineTo;
        Order GlyphIndex;
        Order Polyline;
        Order EllipseSC;

        Order CacheBitmap;
        Order CacheColorTable;
        Order CacheGlyph;

        Order FrameMarker;

        Order BitmapUpdate;

        Order CachePointer;
        Order PointerIndex;

        uint32_t graphics_update_chunk;
        uint32_t bitmap_update_chunk;
        Order timestamp_chunk;

        uint64_t total_read_len;
        uint64_t internal_order_read_len;

        Order NewOrExistingWindow;
        Order DeletedWindow;
    };
    Statistics statistics {};

    using Verbose = FileToGraphicVerbose;

private:
    bool play_video_with_corrupted_bitmap;

    Verbose verbose;

public:
    FileToGraphic(
        Transport & trans,
        bool play_video_with_corrupted_bitmap,
        Verbose verbose);

    ~FileToGraphic();

    WrmMetaChunk const& get_wrm_info() const noexcept { return this->info; }
    MonotonicTimePoint get_monotonic_time() const noexcept { return this->record_now; }
    RealTimePoint get_real_time() const noexcept { return this->last_real_time + (this->record_now - this->monotonic_real_time); }

    PointerCache::SourcePointersView pointers_view() const noexcept
    {
        return this->ptr_cache.source_pointers_view();
    }

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr,
        gdi::RelayoutApi * relayout_ptr,
        gdi::ResizeApi * resize_ptr
    );

    void clear_consumer();

    /* order count set this->stream.p to the beginning of the next order.
     * Most of the times it means not changing it, except when it must read next chunk
     * when remaining order count is 0.
     * It update chunk headers (merely remaining orders count) and
     * reads the next chunk if necessary.
     */
    bool next_order();

    void interpret_order();


    void process_windowing(InStream & stream, const RDP::AltsecDrawingOrderHeader & header);

    void process_window_information( InStream & stream, const RDP::AltsecDrawingOrderHeader & /*unused*/
                                   , uint32_t FieldsPresentFlags);

    void process_notification_icon_information( InStream & stream
                                              , const RDP::AltsecDrawingOrderHeader & /*unused*/
                                              , uint32_t FieldsPresentFlags);

    void process_desktop_information( InStream & stream, const RDP::AltsecDrawingOrderHeader & /*unused*/
                                    , uint32_t FieldsPresentFlags);

    void play(bool const & requested_to_stop, MonotonicTimePoint begin_capture, MonotonicTimePoint end_capture);

    template<class CbUpdateProgress>
    void play(CbUpdateProgress update_progess, bool const & requested_to_stop, MonotonicTimePoint begin_capture, MonotonicTimePoint end_capture)
    {
        MonotonicTimePoint last_sent_record_now {};
        this->privplay([&](MonotonicTimePoint record_now) {
            if (last_sent_record_now != record_now) {
                update_progess(record_now);
                last_sent_record_now = record_now;
            }
        }, requested_to_stop, begin_capture, end_capture);
    }

private:
    void snapshot_play();
    void log_play() const;

    template<class CbUpdateProgress>
    void privplay(
        CbUpdateProgress update_progess, bool const & requested_to_stop,
        MonotonicTimePoint begin_capture, MonotonicTimePoint end_capture)
    {
        while (!requested_to_stop && this->next_order()) {
            this->log_play();

            this->interpret_order();

            if (begin_capture <= this->record_now) {
                this->snapshot_play();
                update_progess(this->record_now);
            }

            if (end_capture < this->record_now) {
                break;
            }
        }
    }

    friend class ReceiveOrder;
};
