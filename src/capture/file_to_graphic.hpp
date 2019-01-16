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

#include "capture/wrm_meta_chunk.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/state_chunk.hpp"
#include "utils/compression_transport_builder.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/difftimeval.hpp"
#include "utils/timeval_ops.hpp"
#include "utils/stream.hpp"

#include <chrono>

#include <sys/time.h> // timeval


class Transport;
class BmpCache;
namespace gdi
{
    class GraphicApi;
    class CaptureApi;
    class KbdInputApi;
    class CaptureProbeApi;
    class ExternalCaptureApi;
    class ResizeApi;
}
namespace RDP
{
    class AltsecDrawingOrderHeader;
}


class FileToGraphic
{
    uint8_t stream_buf[65536];
    InStream stream;

    CompressionInTransportBuilder compression_builder;

    Transport * trans_source;
    Transport * trans;

public:
    Rect screen_rect;

    Dimension max_screen_dim;

    // Internal state of orders
    StateChunk ssc;

    std::unique_ptr<BmpCache> bmp_cache;
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

private:
    timeval start_record_now;
public:
    timeval start_synctime_now;

private:
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
    fixed_ptr_array<gdi::ResizeApi, 10> resize_consumers;

    bool meta_ok;
    bool timestamp_ok;
    uint16_t mouse_x;
    uint16_t mouse_y;
    const bool real_time;

    const timeval begin_capture;
    const timeval end_capture;
public:
    uint32_t max_order_count;

    WrmMetaChunk info {};

private:
    bool ignore_frame_in_timeval;

public:
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
    } statistics;

    bool break_privplay_client;

    std::chrono::microseconds movie_elapsed_client;

    bool play_video_with_corrupted_bitmap;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        play            = 8,
        timestamp       = 16,
        rdp_orders      = 32,
        probe           = 64,
        frame_marker    = 128
    };

    FileToGraphic(Transport & trans, const timeval begin_capture, const timeval end_capture, bool real_time, bool play_video_with_corrupted_bitmap, Verbose verbose);

    ~FileToGraphic();

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr,
        gdi::ResizeApi * resize_ptr
    );

    void clear_consumer();

    void set_pause_client(timeval & time);

    void set_wait_after_load_client(timeval & time);

    void set_sync();

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

    void play(bool const & requested_to_stop);

    bool play_client();

    template<class CbUpdateProgress>
    void play(CbUpdateProgress update_progess, bool const & requested_to_stop)
    {
        time_t last_sent_record_now = 0;
        this->privplay([&](time_t record_now) {
            if (last_sent_record_now != record_now) {
                update_progess(record_now);
                last_sent_record_now = record_now;
            }
        }, requested_to_stop);
    }

    void instant_play_client(std::chrono::microseconds endin_frame);

private:
    void snapshot_play();
    void log_play() const;

    template<class CbUpdateProgress>
    void privplay(CbUpdateProgress update_progess, bool const & requested_to_stop)
    {
        while (!requested_to_stop && this->next_order()) {
            this->log_play();

            this->interpret_order();

            if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                this->snapshot_play();
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

    template<class CbUpdateProgress>
    bool privplay_client(CbUpdateProgress update_progess)
    {
        timeval const now                       = tvtime();
        std::chrono::microseconds const elapsed = difftimeval(now, this->start_synctime_now) ;

        bool res(false);

        //LOG(LOG_INFO, "begin = %u movie_elapsed_client = %u elapsed = %u", unsigned(this->begin_to_elapse), unsigned(this->movie_elapsed_client), elapsed);
        if (elapsed >= this->movie_elapsed_client) {
            if (this->next_order()) {
                this->log_play();

                if (this->remaining_order_count > 0) {
                    res = true;
                }

                this->interpret_order();

                if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                    this->snapshot_play();
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

    friend class ReceiveOrder;
};
