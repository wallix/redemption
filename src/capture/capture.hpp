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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan,
              Clément Moroldo
*/

#pragma once

#include "utils/sugar/noncopyable.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "capture/notify_next_video.hpp"
#include "capture/wrm_params.hpp"
#include "capture/png_params.hpp"
#include "capture/flv_params.hpp"
#include "capture/pattern_checker_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/sequenced_video_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/kbdlog_params.hpp"
#include "capture/wrm_chunk_type.hpp"
#include "RDPChunkedDevice.hpp"
#include "core/wait_obj.hpp"
#include "core/RDP/RDPSerializer.hpp"
#include "utils/fdbuf.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/compression_transport_builder.hpp"
#include "utils/png.hpp"

#include <vector>
#include <memory>


class SaveStateChunk
{
public:
    SaveStateChunk() = default;

    void recv(InStream & stream, StateChunk & sc, uint8_t info_version) {
        this->send_recv(stream, sc, info_version);
    }

    void send(OutStream & stream, StateChunk & sc) {
        this->send_recv(stream, sc, ~0);
    }

private:
    static void io_uint8(InStream & stream, uint8_t & value) { value = stream.in_uint8(); }
    static void io_uint8(OutStream & stream, uint8_t value) { stream.out_uint8(value); }

    static void io_sint8(InStream & stream, int8_t & value) { value = stream.in_sint8(); }
    static void io_sint8(OutStream & stream, int8_t value) { stream.out_sint8(value); }

    // TODO BUG this is an error
    static void io_uint8_unsafe(InStream & stream, uint16_t & value) { value = stream.in_uint8(); }
    static void io_uint8_unsafe(OutStream & stream, uint16_t value) { stream.out_uint8(value); }

    static void io_uint16_le(InStream & stream, uint16_t & value) { value = stream.in_uint16_le(); }
    static void io_uint16_le(OutStream & stream, uint16_t value) { stream.out_uint16_le(value); }

    static void io_sint16_le(InStream & stream, int16_t & value) { value = stream.in_sint16_le(); }
    static void io_sint16_le(OutStream & stream, int16_t value) { stream.out_sint16_le(value); }

    static void io_uint32_le(InStream & stream, uint32_t & value) { value = stream.in_uint32_le(); }
    static void io_uint32_le(OutStream & stream, uint32_t value) { stream.out_uint32_le(value); }

    static void io_uint32_le(InStream & stream, RDPColor & value) { value = RDPColor(stream.in_uint32_le()); }
    static void io_uint32_le(OutStream & stream, RDPColor value) { stream.out_uint32_le(value.as_bgr().to_u32()); }

    static void io_color(InStream & stream, uint32_t & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = red | green << 8 | blue << 16;
    }
    static void io_color(OutStream & stream, uint32_t color) {
        stream.out_uint8(color);
        stream.out_uint8(color >> 8);
        stream.out_uint8(color >> 16);
    }

    static void io_color(InStream & stream, RDPColor & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = RDPColor(blue << 16 | green << 8 | red);
    }
    static void io_color(OutStream & stream, RDPColor color) {
        stream.out_uint8(color.as_bgr().red());
        stream.out_uint8(color.as_bgr().green());
        stream.out_uint8(color.as_bgr().blue());
    }

    static void io_copy_bytes(InStream & stream, uint8_t * buf, unsigned n) { stream.in_copy_bytes(buf, n); }
    static void io_copy_bytes(OutStream & stream, uint8_t * buf, unsigned n) { stream.out_copy_bytes(buf, n); }

    template<class Stream>
    static void io_delta_encoded_rects(Stream & stream, array_view<RDP::DeltaEncodedRectangle> delta_rectangles) {
        // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
        for (RDP::DeltaEncodedRectangle & delta_rectangle : delta_rectangles) {
            io_sint16_le(stream, delta_rectangle.leftDelta);
            io_sint16_le(stream, delta_rectangle.topDelta);
            io_sint16_le(stream, delta_rectangle.width);
            io_sint16_le(stream, delta_rectangle.height);
        }
    }

    template<class Stream>
    static void io_brush(Stream & stream, RDPBrush & brush) {
        io_sint8(stream, brush.org_x);
        io_sint8(stream, brush.org_y);
        io_uint8(stream, brush.style);
        io_uint8(stream, brush.hatch);
        io_copy_bytes(stream, brush.extra, 7);
    }

    template<class Stream>
    static void io_rect(Stream & stream, Rect & rect) {
        io_sint16_le(stream, rect.x);
        io_sint16_le(stream, rect.y);
        io_uint16_le(stream, rect.cx);
        io_uint16_le(stream, rect.cy);
    }

    template<class Stream>
    void send_recv(Stream & stream, StateChunk & sc, uint8_t info_version) {
        const bool mem3blt_support         = (info_version > 1);
        const bool polyline_support        = (info_version > 2);
        const bool multidstblt_support     = (info_version > 3);
        const bool multiopaquerect_support = (info_version > 3);
        const bool multipatblt_support     = (info_version > 3);
        const bool multiscrblt_support     = (info_version > 3);

        // RDPOrderCommon common;
        io_uint8(stream, sc.common.order);
        io_rect(stream, sc.common.clip);

        // RDPDestBlt destblt;
        io_rect(stream, sc.destblt.rect);
        io_uint8(stream, sc.destblt.rop);

        // RDPPatBlt patblt;
        io_rect(stream, sc.patblt.rect);
        io_uint8(stream, sc.patblt.rop);
        io_uint32_le(stream, sc.patblt.back_color);
        io_uint32_le(stream, sc.patblt.fore_color);
        io_brush(stream, sc.patblt.brush);

        // RDPScrBlt scrblt;
        io_rect(stream, sc.scrblt.rect);
        io_uint8(stream, sc.scrblt.rop);
        io_uint16_le(stream, sc.scrblt.srcx);
        io_uint16_le(stream, sc.scrblt.srcy);

        // RDPOpaqueRect opaquerect;
        io_rect(stream, sc.opaquerect.rect);
        io_color(stream, sc.opaquerect.color);

        // RDPMemBlt memblt;
        io_uint16_le(stream, sc.memblt.cache_id);
        io_rect(stream, sc.memblt.rect);
        io_uint8(stream, sc.memblt.rop);
        // TODO bad length
        io_uint8_unsafe(stream, sc.memblt.srcx);
        io_uint8_unsafe(stream, sc.memblt.srcy);
        io_uint16_le(stream, sc.memblt.cache_idx);

        // RDPMem3Blt memblt;
        if (mem3blt_support) {
            io_uint16_le(stream, sc.mem3blt.cache_id);
            io_rect(stream, sc.mem3blt.rect);
            io_uint8(stream, sc.mem3blt.rop);
            io_uint8_unsafe(stream, sc.mem3blt.srcx);
            io_uint8_unsafe(stream, sc.mem3blt.srcy);
            io_uint32_le(stream, sc.mem3blt.back_color);
            io_uint32_le(stream, sc.mem3blt.fore_color);
            io_brush(stream, sc.mem3blt.brush);
            io_uint16_le(stream, sc.mem3blt.cache_idx);
        }

        // RDPLineTo lineto;
        io_uint8(stream, sc.lineto.back_mode);
        io_sint16_le(stream, sc.lineto.startx);
        io_sint16_le(stream, sc.lineto.starty);
        io_sint16_le(stream, sc.lineto.endx);
        io_sint16_le(stream, sc.lineto.endy);
        io_uint32_le(stream, sc.lineto.back_color);
        io_uint8(stream, sc.lineto.rop2);
        io_uint8(stream, sc.lineto.pen.style);
        io_uint8(stream, sc.lineto.pen.width);
        io_uint32_le(stream, sc.lineto.pen.color);

        // RDPGlyphIndex glyphindex;
        io_uint8(stream, sc.glyphindex.cache_id);
        io_sint16_le(stream, sc.glyphindex.fl_accel);
        io_sint16_le(stream, sc.glyphindex.ui_charinc);
        io_sint16_le(stream, sc.glyphindex.f_op_redundant);
        io_uint32_le(stream, sc.glyphindex.back_color);
        io_uint32_le(stream, sc.glyphindex.fore_color);
        io_rect(stream, sc.glyphindex.bk);
        io_rect(stream, sc.glyphindex.op);
        io_brush(stream, sc.glyphindex.brush);
        io_sint16_le(stream, sc.glyphindex.glyph_x);
        io_sint16_le(stream, sc.glyphindex.glyph_y);
        io_uint8(stream, sc.glyphindex.data_len);
        io_copy_bytes(stream, sc.glyphindex.data, 256);

        // RDPPolyine polyline;
        if (polyline_support) {
            io_sint16_le(stream, sc.polyline.xStart);
            io_sint16_le(stream, sc.polyline.yStart);
            io_uint8(stream, sc.polyline.bRop2);
            io_uint16_le(stream, sc.polyline.BrushCacheEntry);
            io_uint32_le(stream, sc.polyline.PenColor);
            io_uint8(stream, sc.polyline.NumDeltaEntries);
            // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
            for (uint8_t i = 0; i < sc.polyline.NumDeltaEntries; i++) {
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].xDelta);
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].yDelta);
            }
        }

        // RDPMultiDstBlt multidstblt;
        if (multidstblt_support) {
            io_sint16_le(stream, sc.multidstblt.nLeftRect);
            io_sint16_le(stream, sc.multidstblt.nTopRect);
            io_sint16_le(stream, sc.multidstblt.nWidth);
            io_sint16_le(stream, sc.multidstblt.nHeight);
            io_uint8(stream, sc.multidstblt.bRop);
            io_uint8(stream, sc.multidstblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multidstblt.deltaEncodedRectangles,
                sc.multidstblt.nDeltaEntries
            });
        }

        // RDPMultiOpaqueRect multiopaquerect;
        if (multiopaquerect_support) {
            io_sint16_le(stream, sc.multiopaquerect.nLeftRect);
            io_sint16_le(stream, sc.multiopaquerect.nTopRect);
            io_sint16_le(stream, sc.multiopaquerect.nWidth);
            io_sint16_le(stream, sc.multiopaquerect.nHeight);
            io_color(stream, sc.multiopaquerect._Color);
            io_uint8(stream, sc.multiopaquerect.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiopaquerect.deltaEncodedRectangles,
                sc.multiopaquerect.nDeltaEntries
            });
        }

        // RDPMultiPatBlt multipatblt;
        if (multipatblt_support) {
            io_rect(stream, sc.multipatblt.rect);
            io_uint8(stream, sc.multipatblt.bRop);
            io_uint32_le(stream, sc.multipatblt.BackColor);
            io_uint32_le(stream, sc.multipatblt.ForeColor);
            io_brush(stream, sc.multipatblt.brush);
            io_uint8(stream, sc.multipatblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multipatblt.deltaEncodedRectangles,
                sc.multipatblt.nDeltaEntries
            });
        }

        // RDPMultiScrBlt multiscrblt;
        if (multiscrblt_support) {
            io_rect(stream, sc.multiscrblt.rect);
            io_uint8(stream, sc.multiscrblt.bRop);
            io_sint16_le(stream, sc.multiscrblt.nXSrc);
            io_sint16_le(stream, sc.multiscrblt.nYSrc);
            io_uint8(stream, sc.multiscrblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiscrblt.deltaEncodedRectangles,
                sc.multiscrblt.nDeltaEntries
            });
        }
    }
};


inline void send_wrm_chunk(Transport & t, WrmChunkType chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<8> header;
    header.out_uint16_le(safe_cast<uint16_t>(chunktype));
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

    send_wrm_chunk(t, WrmChunkType::META_FILE, payload.get_offset(), 1);
    t.send(payload.get_data(), payload.get_offset());
}


struct ChunkToFile : public RDPChunkedDevice {
private:
    CompressionOutTransportBuilder compression_bullder;
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
    , compression_bullder(*trans, wrm_compression_algorithm)
    , trans_target(*trans)
    , trans(this->compression_bullder.get())
    , wrm_format_version(bool(this->compression_bullder.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
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

          , static_cast<unsigned>(this->compression_bullder.get_algorithm())
        );
    }

public:
    void chunk(uint16_t chunk_type, uint16_t chunk_count, InStream stream) override
    {
        auto wrm_chunk_type = safe_cast<WrmChunkType>(chunk_type);
        switch (wrm_chunk_type)
        {
        case WrmChunkType::META_FILE:
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

                  , static_cast<unsigned>(this->compression_bullder.get_algorithm())
                );
            }
            break;

        case WrmChunkType::SAVE_STATE:
            {
                StateChunk sc;
                SaveStateChunk ssc;

                ssc.recv(stream, sc, this->info_version);

                StaticOutStream<65536> payload;

                ssc.send(payload, sc);

                send_wrm_chunk(this->trans, WrmChunkType::SAVE_STATE, payload.get_offset(), chunk_count);
                this->trans.send(payload.get_data(), payload.get_offset());
            }
            break;

        case WrmChunkType::RESET_CHUNK:
            {
                send_wrm_chunk(this->trans, WrmChunkType::RESET_CHUNK, 0, 1);
                this->trans.next();
            }
            break;

        case WrmChunkType::TIMESTAMP:
            {
                timeval record_now;
                stream.in_timeval_from_uint64le_usec(record_now);
                this->trans_target.timestamp(record_now);
            }
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            {
                send_wrm_chunk(this->trans, wrm_chunk_type, stream.get_capacity(), chunk_count);
                this->trans.send(stream.get_data(), stream.get_capacity());
            }
            break;
        }
    }
};

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

private:
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
    uint64_t movie_elapsed_client;
    uint64_t begin_to_elapse;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        play        = 8,
        timestamp   = 16,
        rdp_orders  = 32,
        probe       = 64,
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
        , movie_elapsed_client(0)
        , begin_to_elapse(this->begin_capture.tv_sec * 1000000)
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
            if (!this->trans->atomic_read(buf, HEADER_SIZE)){
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
                    if (!this->trans->atomic_read(this->stream_buf, this->chunk_size - HEADER_SIZE)){
                        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
                    }
                }
            }
        }
        if (this->remaining_order_count > 0){this->remaining_order_count--;}
        return true;
    }

    void interpret_order()
    {
        this->total_orders_count++;
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
                        if (bool(this->verbose & Verbose::rdp_orders)){
                            order.log(LOG_INFO);
                        }
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(order);
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
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.glyphindex, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette), this->gly_cache);
                    }
                    break;
                case RDP::DESTBLT:
                    this->statistics.DstBlt++;
                    this->ssc.destblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.destblt.log(LOG_INFO, clip);
                    }

                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.destblt, clip);
                    }
                    break;
                case RDP::MULTIDSTBLT:
                    this->statistics.MultiDstBlt++;
                    this->ssc.multidstblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multidstblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multidstblt, clip);
                    }
                    break;
                case RDP::MULTIOPAQUERECT:
                    this->statistics.MultiOpaqueRect++;
                    this->ssc.multiopaquerect.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multiopaquerect.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multiopaquerect, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::MULTIPATBLT:
                    this->statistics.MultiPatBlt++;
                    this->ssc.multipatblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multipatblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multipatblt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::MULTISCRBLT:
                    this->statistics.MultiScrBlt++;
                    this->ssc.multiscrblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.multiscrblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multiscrblt, clip);
                    }
                    break;
                case RDP::PATBLT:
                    this->statistics.PatBlt++;
                    this->ssc.patblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.patblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.patblt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::SCREENBLT:
                    this->statistics.ScrBlt++;
                    this->ssc.scrblt.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.scrblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.scrblt, clip);
                    }
                    break;
                case RDP::LINE:
                    this->statistics.LineTo++;
                    this->ssc.lineto.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.lineto.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.lineto, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::RECT:
                    this->statistics.OpaqueRect++;
                    this->ssc.opaquerect.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.opaquerect.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.opaquerect, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
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
                        }
                        else {
                            for (gdi::GraphicApi * gd : this->graphic_consumers){
                                gd->draw(this->ssc.memblt, clip, bmp);
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
                            for (gdi::GraphicApi * gd : this->graphic_consumers){
                                gd->draw(this->ssc.mem3blt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette), bmp);
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
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.polyline, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::ELLIPSESC:
                    this->statistics.EllipseSC++;
                    this->ssc.ellipseSC.receive(this->stream, header);
                    if (bool(this->verbose & Verbose::rdp_orders)){
                        this->ssc.ellipseSC.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.ellipseSC, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
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
                    for (auto data = input_data, end = data + input_len/4; data != end; data += 4) {
                        uint8_t         key8[6];
                        const size_t    len = UTF32toUTF8(data, 4, key8, sizeof(key8)-1);
                        key8[len] = 0;

                        LOG( LOG_INFO, "TIMESTAMP %lu.%lu keyboard '%s'"
                            , static_cast<unsigned long>(this->record_now.tv_sec)
                            , static_cast<unsigned long>(this->record_now.tv_usec)
                            , key8);
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
                if (!this->trans->atomic_read(this->stream_buf, sz)){
                    throw Error(ERR_TRANSPORT_NO_MORE_DATA);
                }
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

            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->draw(bitmap_data, bitmap);
            }

        }
        break;
        case WrmChunkType::POINTER:
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
                   , unsigned(this->record_now.tv_sec), unsigned(this->total_orders_count));
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

    template<class CbUpdateProgress>
    bool privplay_client(CbUpdateProgress update_progess) {

        struct timeval now     = tvtime();
        uint64_t       elapsed = difftimeval(now, this->start_synctime_now) ;

        bool res(false);

        while (this->begin_to_elapse >= this->movie_elapsed_client) {
            if (this->next_order()) {
                if (bool(this->verbose & Verbose::play)) {
                    LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                    , unsigned(this->record_now.tv_sec), unsigned(this->total_orders_count));
                }

                if (this->remaining_order_count > 0) {
                    res = true;
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

        if (elapsed >= this->movie_elapsed_client) {
            if (this->next_order()) {
                if (bool(this->verbose & Verbose::play)) {
                    LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                    , unsigned(this->record_now.tv_sec), unsigned(this->total_orders_count));
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


struct FilenameGenerator
{
    enum Format {
        PATH_FILE_PID_COUNT_EXTENSION,
        PATH_FILE_COUNT_EXTENSION,
        PATH_FILE_PID_EXTENSION,
        PATH_FILE_EXTENSION
    };

private:
    char         path[1024];
    char         filename[1012];
    char         extension[12];
    Format       format;
    unsigned     pid;
    mutable char filename_gen[1024];

    const char * last_filename;
    unsigned     last_num;

public:
    FilenameGenerator(
        Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension)
    : format(format)
    , pid(getpid())
    , last_filename(nullptr)
    , last_num(-1u)
    {
        if (strlen(prefix) > sizeof(this->path) - 1
         || strlen(filename) > sizeof(this->filename) - 1
         || strlen(extension) > sizeof(this->extension) - 1) {
            throw Error(ERR_TRANSPORT);
        }

        strcpy(this->path, prefix);
        strcpy(this->filename, filename);
        strcpy(this->extension, extension);

        this->filename_gen[0] = 0;
    }

    const char * get(unsigned count) const
    {
        if (count == this->last_num && this->last_filename) {
            return this->last_filename;
        }

        using std::snprintf;
        switch (this->format) {
            default:
            case PATH_FILE_PID_COUNT_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u-%06u%s", this->path
                        , this->filename, this->pid, count, this->extension);
                break;
            case PATH_FILE_COUNT_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s", this->path
                        , this->filename, count, this->extension);
                break;
            case PATH_FILE_PID_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s", this->path
                        , this->filename, this->pid, this->extension);
                break;
            case PATH_FILE_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s%s", this->path
                        , this->filename, this->extension);
                break;
        }
        return this->filename_gen;
    }

    void set_last_filename(unsigned num, const char * name)
    {
        this->last_num = num;
        this->last_filename = name;
    }

private:
    FilenameGenerator(FilenameGenerator const &) = delete;
    FilenameGenerator& operator=(FilenameGenerator const &) = delete;
};

typedef FilenameGenerator::Format FilenameFormat;


// TODO in PngCapture
struct OutFilenameSequenceTransport : public Transport
{
    struct capture_out_sequence_filename_buf_param
    {
        FilenameGenerator::Format format;
        const char * const prefix;
        const char * const filename;
        const char * const extension;
        const int groupid;

        capture_out_sequence_filename_buf_param(
            FilenameGenerator::Format format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid)
        : format(format)
        , prefix(prefix)
        , filename(filename)
        , extension(extension)
        , groupid(groupid)
        {}
    };

    class pngcapture_out_sequence_filename_buf_impl
    {
        char current_filename_[1024];
        FilenameGenerator filegen_;
        io::posix::fdbuf buf_;
        unsigned num_file_;
        int groupid_;

    public:
        explicit pngcapture_out_sequence_filename_buf_impl(capture_out_sequence_filename_buf_param const & params)
        : filegen_(params.format, params.prefix, params.filename, params.extension)
        , buf_()
        , num_file_(0)
        , groupid_(params.groupid)
        {
            this->current_filename_[0] = 0;
        }

        int close()
        { return this->next(); }

        ssize_t write(const void * data, size_t len)
        {
            if (!this->buf_.is_open()) {
                const int res = this->open_filename(this->filegen_.get(this->num_file_));
                if (res < 0) {
                    return res;
                }
            }
            return this->buf_.write(data, len);
        }

        /// \return 0 if success
        int next()
        {
            if (this->buf_.is_open()) {
                this->buf_.close();
//                LOG(LOG_INFO, "pngcapture: \"%s\" -> \"%s\".", this->current_filename_, this->rename_to);
                return this->rename_filename() ? 0 : 1;
            }
            return 1;
        }

        off64_t seek(int64_t offset, int whence)
        { return this->buf_.seek(offset, whence); }

        const FilenameGenerator & seqgen() const noexcept
        { return this->filegen_; }

        io::posix::fdbuf & buf() noexcept
        { return this->buf_; }

        const char * current_path() const
        {
            if (!this->current_filename_[0] && !this->num_file_) {
                return nullptr;
            }
            return this->filegen_.get(this->num_file_ - 1);
        }

    protected:
        ssize_t open_filename(const char * filename)
        {
            snprintf(this->current_filename_, sizeof(this->current_filename_),
                        "%sred-XXXXXX.tmp", filename);
            const int fd = ::mkostemps(this->current_filename_, 4, O_WRONLY | O_CREAT);
            if (fd < 0) {
                return fd;
            }
            LOG(LOG_INFO, "pngcapture=%s\n", this->current_filename_);
            // TODO PERF used fchmod
            if (chmod(this->current_filename_, this->groupid_ ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->current_filename_
                   , this->groupid_ ? "u+r, g+r" : "u+r"
                   , strerror(errno), errno);
            }
            this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
            return this->buf_.open(fd);
        }

        const char * rename_filename()
        {
            const char * filename = this->get_filename_generate();
            const int res = ::rename(this->current_filename_, filename);
            LOG( LOG_ERR, "renaming file \"%s\" to \"%s\"\n"
                   , this->current_filename_, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , this->current_filename_, filename, errno, strerror(errno));
                return nullptr;
            }

            this->current_filename_[0] = 0;
            ++this->num_file_;
            this->filegen_.set_last_filename(-1u, "");

            return filename;
        }

        const char * get_filename_generate()
        {
            this->filegen_.set_last_filename(-1u, "");
            const char * filename = this->filegen_.get(this->num_file_);
            this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
            return filename;
        }
    };

    using Buf = pngcapture_out_sequence_filename_buf_impl;

    OutFilenameSequenceTransport(
        FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier)
    : buf(capture_out_sequence_filename_buf_param(format, prefix, filename, extension, groupid))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    const FilenameGenerator * seqgen() const noexcept
    { return &(this->buffer().seqgen()); }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    ~OutFilenameSequenceTransport() {
        this->buf.close();
    }

private:

    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            auto eid = ERR_TRANSPORT_WRITE_FAILED;
            if (errno == ENOSPC) {
                this->authentifier->report("FILESYSTEM_FULL", "100|unknown");
                errno = ENOSPC;
                eid = ERR_TRANSPORT_WRITE_NO_ROOM;
            }
            throw Error(eid, errno);
        }
        this->last_quantum_sent += res;
    }

    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    Buf buf;
};

struct NotifyTitleChanged : private noncopyable
{
    virtual void notify_title_changed(const timeval & now, array_view_const_char title) = 0;
    virtual ~NotifyTitleChanged() = default;
};

class SessionMeta;
class WrmCaptureImpl;
class PngCapture;
class PngCaptureRT;
class SyslogKbd;
class SessionLogKbd;
class PatternKbd;
class MetaCaptureImpl;
class TitleCaptureImpl;
class PatternsChecker;
class UpdateProgressData;
class RDPDrawable;
class SequencedVideoCaptureImpl;
class FullVideoCaptureImpl;

struct MouseTrace
{
    timeval last_now;
    int     last_x;
    int     last_y;
};

class Capture final
: public gdi::GraphicApi
, public gdi::CaptureApi
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
, public gdi::ExternalCaptureApi
{
    const bool is_replay_mod;

    using string_view = array_view_const_char;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl( cmd); }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

private:
    // Title changed
    //@{
    struct TitleChangedFunctions final : NotifyTitleChanged
    {
        Capture & capture;

        TitleChangedFunctions(Capture & capture) : capture(capture) {}

        void notify_title_changed(timeval const & now, string_view title) override;
    } notifier_title_changed{*this};
    //@}

    // Next video
    //@{
    struct NotifyMetaIfNextVideo final : NotifyNextVideo
    {
        SessionMeta * session_meta = nullptr;

        void notify_next_video(const timeval& now, NotifyNextVideo::reason reason) override;
    } notifier_next_video;
    struct NullNotifyNextVideo final : NotifyNextVideo
    {
        void notify_next_video(const timeval&, NotifyNextVideo::reason) override {}
    } null_notifier_next_video;
    //@}

public:

    std::unique_ptr<RDPDrawable> gd_drawable;

private:
    class Graphic final : public gdi::GraphicApi
    {
    public:
        void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
        void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
        void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
        void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

        void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

        void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

        void set_pointer(Pointer    const & pointer) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_pointer(pointer);
            }
        }

        void set_palette(BGRPalette const & palette) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_palette(palette);
            }
        }

        void sync() override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.sync();
            }
        }

        void set_row(std::size_t rownum, const uint8_t * data) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_row(rownum, data);
            }
        }

        void begin_update() override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.begin_update();
            }
        }

        void end_update() override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.end_update();
            }
        }

    private:
        template<class... Ts>
        void draw_impl(Ts const & ... args) {
            for (gdi::GraphicApi & gd : this->gds){
                gd.draw(args...);
            }
        }

        void draw_impl(RDP::FrameMarker const & cmd) {
            for (gdi::GraphicApi & gd : this->gds) {
                gd.draw(cmd);
            }

            if (cmd.action == RDP::FrameMarker::FrameEnd) {
                for (gdi::CaptureApi & cap : this->caps) {
                    cap.frame_marker_event(this->mouse.last_now, this->mouse.last_x, this->mouse.last_y, false);
                }
            }
        }

    public:
        MouseTrace const & mouse;
        const std::vector<std::reference_wrapper<gdi::GraphicApi>> & gds;
        const std::vector<std::reference_wrapper<gdi::CaptureApi>> & caps;

        Graphic(MouseTrace const & mouse,
                const std::vector<std::reference_wrapper<gdi::GraphicApi>> & gds,
                const std::vector<std::reference_wrapper<gdi::CaptureApi>> & caps)
        : mouse(mouse)
        , gds(gds)
        , caps(caps)
        {}
    };

    std::unique_ptr<Graphic> graphic_api;

    std::unique_ptr<WrmCaptureImpl> wrm_capture_obj;
    std::unique_ptr<PngCapture> png_capture_obj;
    std::unique_ptr<PngCaptureRT> png_capture_real_time_obj;

    std::unique_ptr<SyslogKbd> syslog_kbd_capture_obj;
    std::unique_ptr<SessionLogKbd> session_log_kbd_capture_obj;
    std::unique_ptr<PatternKbd> pattern_kbd_capture_obj;

    std::unique_ptr<SequencedVideoCaptureImpl> sequenced_video_capture_obj;
    std::unique_ptr<FullVideoCaptureImpl> full_video_capture_obj;
    std::unique_ptr<MetaCaptureImpl> meta_capture_obj;
    std::unique_ptr<TitleCaptureImpl> title_capture_obj;
    std::unique_ptr<PatternsChecker> patterns_checker;

    UpdateProgressData * update_progress_data;

    MouseTrace mouse_info;
    wait_obj capture_event;

    std::vector<std::reference_wrapper<gdi::GraphicApi>> gds;
    // Objects willing to be warned of FrameMarker Events
    std::vector<std::reference_wrapper<gdi::CaptureApi>> caps;
    std::vector<std::reference_wrapper<gdi::KbdInputApi>> kbds;
    std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> probes;
    std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> objs;

    bool capture_drawable = false;


public:
    Capture(
        bool capture_wrm, const WrmParams wrm_params,
        bool capture_png, const PngParams png_params,
        bool capture_pattern_checker, const PatternCheckerParams /* pattern_checker_params */,
        bool capture_ocr, const OcrParams ocr_params,
        bool capture_flv, const SequencedVideoParams /*sequenced_video_params*/,
        bool capture_flv_full, const FullVideoParams /*full_video_params*/,
        bool capture_meta, const MetaParams /*meta_params*/,
        bool capture_kbd, const KbdLogParams /*kbd_log_params*/,
        const char * basename,
        const timeval & now,
        int width,
        int height,
        int /*order_bpp*/,
        int /*capture_bpp*/,
        const char * record_tmp_path,
        const char * record_path,
        const int groupid,
        const FlvParams flv_params,
        bool no_timestamp,
        auth_api * authentifier,
        UpdateProgressData * update_progress_data,
        const char * pattern_kill,
        const char * pattern_notify,
        int debug_capture,
        bool flv_capture_chunk,
        bool meta_enable_session_log,
        const std::chrono::duration<long int> flv_break_interval,
        bool syslog_keyboard_log,
        bool rt_display,
        bool disable_keyboard_log,
        bool session_log_enabled,
        bool keyboard_fully_masked,
        bool meta_keyboard_log
    );

    ~Capture();

    wait_obj & get_capture_event() {
        return this->capture_event;
    }

    public:
    void update_config(bool enable_rt_display);

    void set_row(size_t rownum, const uint8_t * data) override;

    void sync() override
    {
        if (this->capture_drawable) {
            this->graphic_api->sync();
        }
    }

    bool kbd_input(timeval const & now, uint32_t uchar) override {
        bool ret = true;
        for (gdi::KbdInputApi & kbd : this->kbds) {
            ret &= kbd.kbd_input(now, uchar);
        }
        return ret;
    }

    void enable_kbd_input_mask(bool enable) override {
        for (gdi::KbdInputApi & kbd : this->kbds) {
            kbd.enable_kbd_input_mask(enable);
        }
    }

    gdi::GraphicApi * get_graphic_api() const {
        return this->graphic_api.get();
    }

    void add_graphic(gdi::GraphicApi & gd) {
        if (this->capture_drawable) {
            this->gds.push_back(gd);
        }
    }

protected:
    std::chrono::microseconds do_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override;

    template<class... Ts>
    void draw_impl(const Ts & ... args) {
        if (this->capture_drawable) {
            this->graphic_api->draw(args...);
        }
    }

public:
    void set_pointer(const Pointer & cursor) override {
        if (this->capture_drawable) {
            this->graphic_api->set_pointer(cursor);
        }
    }

    void set_palette(const BGRPalette & palette) override {
        if (this->capture_drawable) {
            this->graphic_api->set_palette(palette);
        }
    }

    void set_pointer_display();

    void external_breakpoint() override {
        for (gdi::ExternalCaptureApi & obj : this->objs) {
            obj.external_breakpoint();
        }
    }

    void external_time(timeval const & now) override {
        for (gdi::ExternalCaptureApi & obj : this->objs) {
            obj.external_time(now);
        }
    }

    void session_update(const timeval & now, array_view_const_char message) override {
        for (gdi::CaptureProbeApi & cap_prob : this->probes) {
            cap_prob.session_update(now, message);
        }
    }

    void possible_active_window_change() override {
        for (gdi::CaptureProbeApi & cap_prob : this->probes) {
            cap_prob.possible_active_window_change();
        }
    }
};
