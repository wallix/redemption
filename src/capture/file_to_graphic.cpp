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

#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/resize_api.hpp"

#include "capture/file_to_graphic.hpp"
#include "capture/save_state_chunk.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/difftimeval.hpp"
#include "utils/hexdump.hpp"
#include "utils/png.hpp"


FileToGraphic::FileToGraphic(Transport & trans, const timeval begin_capture, const timeval end_capture, bool real_time, bool play_video_with_corrupted_bitmap, Verbose verbose)
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
    , ignore_frame_in_timeval(false)
    , statistics()
    , break_privplay_client(false)
    , movie_elapsed_client{}
    , play_video_with_corrupted_bitmap(play_video_with_corrupted_bitmap)
    , verbose(verbose)
{
    while (this->next_order()){
        this->interpret_order();
        if (this->meta_ok && this->timestamp_ok){
            break;
        }
    }
}

FileToGraphic::~FileToGraphic() = default;

void FileToGraphic::add_consumer(
    gdi::GraphicApi * graphic_ptr,
    gdi::CaptureApi * capture_ptr,
    gdi::KbdInputApi * kbd_input_ptr,
    gdi::CaptureProbeApi * capture_probe_ptr,
    gdi::ExternalCaptureApi * external_event_ptr,
    gdi::ResizeApi * resize_ptr
)
{
    this->graphic_consumers.push_back(graphic_ptr);
    this->capture_consumers.push_back(capture_ptr);
    this->kbd_input_consumers.push_back(kbd_input_ptr);
    this->capture_probe_consumers.push_back(capture_probe_ptr);
    this->external_event_consumers.push_back(external_event_ptr);
    this->resize_consumers.push_back(resize_ptr);
}

void FileToGraphic::clear_consumer()
{
    this->graphic_consumers.clear();
    this->capture_consumers.clear();
    this->kbd_input_consumers.clear();
    this->capture_probe_consumers.clear();
    this->external_event_consumers.clear();
    this->resize_consumers.clear();
}

void FileToGraphic::set_pause_client(timeval & time)
{
    this->start_synctime_now = {this->start_synctime_now.tv_sec + time.tv_sec, this->start_synctime_now.tv_usec + time.tv_usec};
}

void FileToGraphic::set_wait_after_load_client(timeval & time)
{
    this->start_synctime_now = {time.tv_sec, time.tv_usec};
}

void FileToGraphic::set_sync()
{
    timeval now = tvtime();
    this->start_synctime_now = {now.tv_sec + this->begin_capture.tv_sec, now.tv_usec};
}

/* order count set this->stream.p to the beginning of the next order.
    * Most of the times it means not changing it, except when it must read next chunk
    * when remaining order count is 0.
    * It update chunk headers (merely remaining orders count) and
    * reads the next chunk if necessary.
    */
bool FileToGraphic::next_order()
{
    if (this->chunk_type != WrmChunkType::LAST_IMAGE_CHUNK
        && this->chunk_type != WrmChunkType::PARTIAL_IMAGE_CHUNK) {
        if (this->stream.get_current() == this->stream.get_data_end()
            && this->remaining_order_count) {
            LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                            "order [%d/%u] "
                            "remaining [%zu/%u]",
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

        uint8_t buf[WRM_HEADER_SIZE];
        if (Transport::Read::Eof == this->trans->atomic_read(buf, WRM_HEADER_SIZE)){
            return false;
        }
        InStream header(buf);
        this->chunk_type = safe_cast<WrmChunkType>(header.in_uint16_le());
        this->chunk_size = header.in_uint32_le();
        this->remaining_order_count = this->chunk_count = header.in_uint16_le();
        this->statistics.total_read_len += this->chunk_size;

        if (this->chunk_type != WrmChunkType::LAST_IMAGE_CHUNK
        && this->chunk_type != WrmChunkType::PARTIAL_IMAGE_CHUNK) {
            switch (this->chunk_type) {
                case WrmChunkType::RDP_UPDATE_ORDERS:
                    this->statistics.graphics_update_chunk++; break;
                case WrmChunkType::RDP_UPDATE_BITMAP:
                    this->statistics.bitmap_update_chunk++;   break;
                case WrmChunkType::TIMESTAMP:
                    this->statistics.timestamp_chunk.count++;
                    REDEMPTION_CXX_FALLTHROUGH;
                case WrmChunkType::META_FILE:
                case WrmChunkType::SAVE_STATE:
                case WrmChunkType::RESET_CHUNK:
                case WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE:
                    this->statistics.internal_order_read_len += this->chunk_size; break;
                default: ;
            }
            if (this->chunk_size > 65536){
                LOG(LOG_ERR,"chunk_size (%u) > 65536", this->chunk_size);
                throw Error(ERR_WRM);
            }
            this->stream = InStream(this->stream_buf);
            if (this->chunk_size - WRM_HEADER_SIZE > 0) {
                this->stream = InStream(this->stream_buf, this->chunk_size - WRM_HEADER_SIZE);
                this->trans->recv_boom(this->stream_buf, this->chunk_size - WRM_HEADER_SIZE);
                this->statistics.internal_order_read_len += WRM_HEADER_SIZE;
            }
        }
        else {
            this->statistics.internal_order_read_len += this->chunk_size;
        }
    }
    if (this->remaining_order_count > 0){this->remaining_order_count--;}
    return true;
}

struct ReceiveOrder
{
    FileToGraphic & ftg;

    template<class Order, class... Args>
    void read(
        Order & order, FileToGraphic::Statistics::Order & stat,
        Rect clip, Args const& ...args) const
    {
        stat.count++;
        auto * p = ftg.stream.get_current();
        order.receive(ftg.stream, args...);
        stat.total_len += ftg.stream.get_current() - p;
        if (bool(ftg.verbose & FileToGraphic::Verbose::rdp_orders)) {
            order.log(LOG_INFO, clip);
        }
    }

    template<class Order, class... Args>
    Order read(
        FileToGraphic::Statistics::Order & stat,
        FileToGraphic::Verbose verbose_flag, Args const& ...args) const
    {
        Order order;
        stat.count++;
        auto * p = ftg.stream.get_current();
        order.receive(ftg.stream, args...);
        stat.total_len += ftg.stream.get_current() - p;
        if (bool(ftg.verbose & verbose_flag)) {
            order.log(LOG_INFO);
        }
        return order;
    }

    template<class Order, class Header, class... Args>
    void read_and_draw(
        Order & order, FileToGraphic::Statistics::Order & stat,
        Rect clip, Header & header, Args const& ...draw_args) const
    {
        read(order, stat, clip, header);
        for (gdi::GraphicApi * gd : ftg.graphic_consumers){
            gd->draw(order, clip, draw_args...);
        }
    }

    template<class Order, class Header>
    void read_and_draw(
        FileToGraphic::Statistics::Order & stat,
        FileToGraphic::Verbose verbose_flag, Header & header) const
    {
        auto order = read<Order>(stat, verbose_flag, header);
        for (gdi::GraphicApi * gd : ftg.graphic_consumers){
            gd->draw(order);
        }
    }

    struct ColorCtxFromBppConverter
    {
        BitsPerPixel info_bpp;
        BGRPalette const & palette;

        operator gdi::ColorCtx () const
        {
            return gdi::ColorCtx::from_bpp(this->info_bpp, this->palette);
        }
    };

    ColorCtxFromBppConverter color_ctx(BGRPalette const & palette) const
    {
        return ColorCtxFromBppConverter{ftg.info.bpp, palette};
    }
};

void FileToGraphic::interpret_order()
{
    this->total_orders_count++;
    auto const & palette = BGRPalette::classic_332();

    // if (this->chunk_type != WrmChunkType::SESSION_UPDATE && this->chunk_type != WrmChunkType::TIMESTAMP) {
    //     return;
    // }

    ReceiveOrder receive_order{*this};

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
                    receive_order.read_and_draw<RDP::FrameMarker>(
                        this->statistics.FrameMarker, Verbose::frame_marker, header);
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
                auto cmd = receive_order.read<RDPBmpCache>(
                    this->statistics.CacheBitmap, Verbose::rdp_orders,
                    header, palette, this->info.bpp);
                this->bmp_cache->put(cmd.id, cmd.idx, cmd.bmp, cmd.key1, cmd.key2);
            }
            break;
            case RDP::TS_CACHE_COLOR_TABLE:
                this->statistics.CacheColorTable.count++;
                LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_COLOR_TABLE (%u)", header.type);
                break;
            case RDP::TS_CACHE_GLYPH:
            {
                auto cmd = receive_order.read<RDPGlyphCache>(
                    this->statistics.CacheGlyph, Verbose::rdp_orders, header);
                this->gly_cache.set_glyph(
                    FontChar(std::move(cmd.aj), cmd.x, cmd.y, cmd.cx, cmd.cy, -1),
                    cmd.cacheId, cmd.cacheIndex
                );
            }
            break;
            case RDP::TS_CACHE_BITMAP_COMPRESSED_REV2:
                LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%u)", header.type);
                break;
            case RDP::TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%u)", header.type);
                break;
            case RDP::TS_CACHE_BITMAP_COMPRESSED_REV3:
                LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%u)", header.type);
                break;
            default:
                LOG(LOG_ERR, "unsupported SECONDARY ORDER (%u)", header.type);
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
                receive_order.read_and_draw(
                    this->ssc.glyphindex, this->statistics.GlyphIndex,
                    clip, header, receive_order.color_ctx(palette), this->gly_cache);
                break;
            case RDP::DESTBLT:
                receive_order.read_and_draw(
                    this->ssc.destblt, this->statistics.DstBlt, clip, header);
                break;
            case RDP::MULTIDSTBLT:
                receive_order.read_and_draw(
                    this->ssc.multidstblt, this->statistics.MultiDstBlt, clip, header);
                break;
            case RDP::MULTIOPAQUERECT:
                receive_order.read_and_draw(
                    this->ssc.multiopaquerect, this->statistics.MultiOpaqueRect,
                    clip, header, receive_order.color_ctx(palette));
                break;
            case RDP::MULTIPATBLT:
                receive_order.read_and_draw(
                    this->ssc.multipatblt, this->statistics.MultiPatBlt,
                    clip, header, receive_order.color_ctx(palette));
                break;
            case RDP::MULTISCRBLT:
                receive_order.read_and_draw(
                    this->ssc.multiscrblt, this->statistics.MultiScrBlt, clip, header);
                break;
            case RDP::PATBLT:
                receive_order.read_and_draw(
                    this->ssc.patblt, this->statistics.PatBlt,
                    clip, header, receive_order.color_ctx(palette));
                break;
            case RDP::SCREENBLT:
                receive_order.read_and_draw(
                    this->ssc.scrblt, this->statistics.ScrBlt, clip, header);
                break;
            case RDP::LINE:
                receive_order.read_and_draw(
                    this->ssc.lineto, this->statistics.LineTo,
                    clip, header, receive_order.color_ctx(palette));
                break;
            case RDP::RECT:
                receive_order.read_and_draw(
                    this->ssc.opaquerect, this->statistics.OpaqueRect,
                    clip, header, receive_order.color_ctx(palette));
                break;
            case RDP::MEMBLT:
                {
                    receive_order.read(
                        this->ssc.memblt, this->statistics.MemBlt, clip, header);
                    const Bitmap & bmp = this->bmp_cache->get(
                        this->ssc.memblt.cache_id, this->ssc.memblt.cache_idx);
                    if (!bmp.is_valid()){
                        LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)",
                            this->ssc.memblt.cache_id, this->ssc.memblt.cache_idx);
                        throw Error(ERR_WRM);
                    }

                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.memblt, clip, bmp);
                    }
                }
                break;
            case RDP::MEM3BLT:
                {
                    receive_order.read(
                        this->ssc.mem3blt, this->statistics.Mem3Blt, clip, header);
                    const Bitmap & bmp = this->bmp_cache->get(
                        this->ssc.mem3blt.cache_id, this->ssc.mem3blt.cache_idx);
                    if (!bmp.is_valid()){
                        LOG(LOG_ERR, "Mem3blt bitmap not found in cache at (%u, %u)",
                            this->ssc.mem3blt.cache_id, this->ssc.mem3blt.cache_idx);
                        throw Error(ERR_WRM);
                    }

                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.mem3blt, clip, receive_order.color_ctx(palette), bmp);
                    }
                }
                break;
            case RDP::POLYLINE:
                receive_order.read_and_draw(
                    this->ssc.polyline, this->statistics.Polyline,
                    clip, header, receive_order.color_ctx(palette));
                break;
            case RDP::ELLIPSESC:
                receive_order.read_and_draw(
                    this->ssc.ellipseSC, this->statistics.EllipseSC,
                    clip, header, receive_order.color_ctx(palette));
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
        auto * const p = this->stream.get_current();

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

            if (  (this->info.version > 1)
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

        this->statistics.timestamp_chunk.total_len += this->stream.get_current() - p;
    }
    break;
    case WrmChunkType::META_FILE:
    {
        this->info.receive(this->stream);
        this->trans = &this->compression_builder.reset(
            *this->trans_source, this->info.compression_algorithm
        );

        this->stream.in_skip_bytes(this->stream.in_remain());

        if (!this->meta_ok) {
            this->bmp_cache = std::make_unique<BmpCache>(
                BmpCache::Recorder, this->info.bpp,
                this->info.number_of_cache,
                this->info.use_waiting_list,
                BmpCache::CacheOption(
                    this->info.cache_0_entries, this->info.cache_0_size, this->info.cache_0_persistent),
                BmpCache::CacheOption(
                    this->info.cache_1_entries, this->info.cache_1_size, this->info.cache_1_persistent),
                BmpCache::CacheOption(
                    this->info.cache_2_entries, this->info.cache_2_size, this->info.cache_2_persistent),
                BmpCache::CacheOption(
                    this->info.cache_3_entries, this->info.cache_3_size, this->info.cache_3_persistent),
                BmpCache::CacheOption(
                    this->info.cache_4_entries, this->info.cache_4_size, this->info.cache_4_persistent));
//            this->screen_rect = Rect(0, 0, this->info_width, this->info_height);
            this->meta_ok = true;
        }
        this->screen_rect = Rect(0, 0, this->info.width, this->info.height);

        if ((this->max_screen_dim.w != this->info.width) || (this->max_screen_dim.h != this->info.height)) {
            this->max_screen_dim.w = std::max(this->max_screen_dim.w, this->info.width);
            this->max_screen_dim.h = std::max(this->max_screen_dim.h, this->info.height);

            for (gdi::ResizeApi * obj : this->resize_consumers){
                obj->resize(this->info.width, this->info.height);
            }
        }
        else {
            for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                obj->external_breakpoint();
            }
        }
    }
    break;
    case WrmChunkType::SAVE_STATE:
    {
        SaveStateChunk ssc;
        ssc.recv(this->stream, this->ssc, this->info.version);
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
            std::size_t sz = this->chunk_size - WRM_HEADER_SIZE;
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

        auto bitmap_data = receive_order.read<RDPBitmapData>(
            this->statistics.BitmapUpdate, Verbose::rdp_orders);

        // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
        if (this->play_video_with_corrupted_bitmap && !(bitmap_data.flags & BITMAP_COMPRESSION)) {
            InStream RM18446_stream2(this->stream.get_current(), this->stream.in_remain());
            const uint8_t * RM18446_test_data = RM18446_stream2.in_uint8p(std::min<size_t>(RM18446_stream2.in_remain(), bitmap_data.bitmap_size()));

            size_t RM18446_adjusted_size = 0;

            Bitmap RM18446_test_bitmap( this->info.bpp
                              , checked_int(bitmap_data.bits_per_pixel)
                              , /*0*/&palette
                              , bitmap_data.width
                              , bitmap_data.height
                              , RM18446_test_data
                              , this->stream.in_remain()
                              , true
                              , &RM18446_adjusted_size
                              );

            if (RM18446_adjusted_size) {
                RDPBitmapData RM18446_test_bitmap_data = bitmap_data;

                RM18446_test_bitmap_data.flags         = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;
                RM18446_test_bitmap_data.bitmap_length = RM18446_adjusted_size;

                this->stream.in_skip_bytes(RM18446_adjusted_size);

                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(RM18446_test_bitmap_data, RM18446_test_bitmap);
                }

                break;
            }
        }

        const uint8_t * data = this->stream.in_uint8p(bitmap_data.bitmap_size());

        Bitmap bitmap( this->info.bpp
                     , checked_int(bitmap_data.bits_per_pixel)
                     , /*0*/&palette
                     , bitmap_data.width
                     , bitmap_data.height
                     , data
                     , bitmap_data.bitmap_size()
                     , (bitmap_data.flags & BITMAP_COMPRESSION)
                     );

        for (gdi::GraphicApi * gd : this->graphic_consumers){
            gd->draw(bitmap_data, bitmap);
        }

    }
    break;
    case WrmChunkType::POINTER:
    {
        if (bool(this->verbose & Verbose::rdp_orders)){
            LOG(LOG_INFO, "POINTER");
        }

        this->mouse_x         = this->stream.in_uint16_le();
        this->mouse_y         = this->stream.in_uint16_le();
        uint8_t cache_idx     = this->stream.in_uint8();

        if (  chunk_size - 8 /*header(8)*/ > 5 /*mouse_x(2) + mouse_y(2) + cache_idx(1)*/) {
            size_t start_offset = this->stream.get_offset();
            const Pointer cursor = pointer_loader_32x32(this->stream);

            this->ptr_cache.add_pointer_static(cursor, cache_idx);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->set_pointer(cursor);
            }

            this->ptr_cache.add_pointer_static(cursor, cache_idx);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->set_pointer(cursor);
            }

            this->statistics.CachePointer.total_len += this->stream.get_offset()-start_offset;
            this->statistics.CachePointer.count++;
        }
        else {
            Pointer cursor(this->ptr_cache.Pointers[cache_idx]);
            this->ptr_cache.Pointers[cache_idx] = cursor;
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->set_pointer(cursor);
            }
            this->statistics.PointerIndex.count++;
        }
    }
    break;
    case WrmChunkType::POINTER2:
    {
        if (bool(this->verbose & Verbose::rdp_orders)){
            LOG(LOG_INFO, "POINTER2");
        }

        size_t start_offset = this->stream.get_offset();
        this->mouse_x = this->stream.in_uint16_le();
        this->mouse_y = this->stream.in_uint16_le();
        uint8_t cache_idx     = this->stream.in_uint8();
        const Pointer cursor = pointer_loader_2(this->stream);

        this->ptr_cache.add_pointer_static(cursor, cache_idx);
        for (gdi::GraphicApi * gd : this->graphic_consumers){
            gd->set_pointer(cursor);
        }
        this->statistics.CachePointer.total_len += this->stream.get_offset()-start_offset;
        this->statistics.CachePointer.count++;
    }
    break;
    case WrmChunkType::RESET_CHUNK:
        this->info.compression_algorithm = WrmCompressionAlgorithm::no_compression;

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

    case WrmChunkType::IMAGE_FRAME_RECT:
        this->max_image_frame_rect.x  = this->stream.in_sint16_le();
        this->max_image_frame_rect.y  = this->stream.in_sint16_le();
        this->max_image_frame_rect.cx = this->stream.in_uint16_le();
        this->max_image_frame_rect.cy = this->stream.in_uint16_le();

        if (this->stream.in_remain() >= 4) {
            this->min_image_frame_dim.w = this->stream.in_uint16_le();
            this->min_image_frame_dim.h = this->stream.in_uint16_le();
        }

        this->stream.in_skip_bytes(this->stream.in_remain());
    break;
    default:
        LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
        throw Error(ERR_WRM);
    }
}


void FileToGraphic::process_windowing( InStream & stream, const RDP::AltsecDrawingOrderHeader & header)
{
    if (bool(this->verbose & Verbose::probe)) {
        LOG(LOG_INFO, "rdp_orders::process_windowing");
    }

    const uint32_t FieldsPresentFlags = [&]{
        InStream stream2(stream.get_current(), stream.in_remain());
        stream2.in_skip_bytes(2);    // OrderSize(2)
        return stream2.in_uint32_le();
    }();

    switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                                 | RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY /*NOLINT*/
                                 | RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP  /*NOLINT*/)) {
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

void FileToGraphic::process_window_information(
    InStream & stream, const RDP::AltsecDrawingOrderHeader & /*unused*/, uint32_t FieldsPresentFlags)
{
    if (bool(this->verbose & Verbose::probe)) {
        LOG(LOG_INFO, "rdp_orders::process_window_information");
    }

    switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                 | RDP::RAIL::WINDOW_ORDER_ICON /*NOLINT*/
                                 | RDP::RAIL::WINDOW_ORDER_CACHEDICON /*NOLINT*/
                                 | RDP::RAIL::WINDOW_ORDER_STATE_DELETED /*NOLINT*/))
    {
        case RDP::RAIL::WINDOW_ORDER_ICON: {
                RDP::RAIL::WindowIcon order;
                order.receive(stream);
                //order.log(LOG_INFO);
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(order);
                }
            }
            break;

        case RDP::RAIL::WINDOW_ORDER_CACHEDICON: {
                RDP::RAIL::CachedIcon order;
                order.receive(stream);
                //order.log(LOG_INFO);
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(order);
                }
            }
            break;

        case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                this->statistics.DeletedWindow.count++;
                auto * p = stream.get_current();
                RDP::RAIL::DeletedWindow order;
                order.receive(stream);
                this->statistics.DeletedWindow.total_len += stream.get_current() - p;
                //order.log(LOG_INFO);
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(order);
                }
            }
            break;

        case 0:
        case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                this->statistics.NewOrExistingWindow.count++;
                auto * p = stream.get_current();
                RDP::RAIL::NewOrExistingWindow order;
                order.receive(stream);
                this->statistics.NewOrExistingWindow.total_len += stream.get_current() - p;
                //order.log(LOG_INFO);
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(order);
                }
            }
            break;
    }
}

void FileToGraphic::process_notification_icon_information(
    InStream & stream, const RDP::AltsecDrawingOrderHeader & /*unused*/, uint32_t FieldsPresentFlags)
{
    if (bool(this->verbose & Verbose::probe)) {
        LOG(LOG_INFO, "rdp_orders::process_notification_icon_information");
    }

    switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                 | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
    {
        case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                RDP::RAIL::DeletedNotificationIcons order;
                order.receive(stream);
                //order.log(LOG_INFO);
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(order);
                }
            }
            break;

        case 0:
        case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                RDP::RAIL::NewOrExistingNotificationIcons order;
                order.receive(stream);
                //order.log(LOG_INFO);
                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(order);
                }
            }
            break;
    }
}

void FileToGraphic::process_desktop_information(
    InStream & stream, const RDP::AltsecDrawingOrderHeader & /*unused*/, uint32_t FieldsPresentFlags)
{
    if (bool(this->verbose & Verbose::probe)) {
        LOG(LOG_INFO, "rdp_orders::process_desktop_information");
    }

    if (FieldsPresentFlags & RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_NONE) {
        RDP::RAIL::NonMonitoredDesktop order;
        order.receive(stream);
        //order.log(LOG_INFO);
        for (gdi::GraphicApi * gd : this->graphic_consumers){
            gd->draw(order);
        }
    }
    else {
        RDP::RAIL::ActivelyMonitoredDesktop order;
        order.receive(stream);
        //order.log(LOG_INFO);
        for (gdi::GraphicApi * gd : this->graphic_consumers){
            gd->draw(order);
        }
    }
}

void FileToGraphic::play(bool const & requested_to_stop)
{
    this->privplay([](time_t){}, requested_to_stop);
}

bool FileToGraphic::play_client()
{
    return this->privplay_client([](time_t){});
}

void FileToGraphic::instant_play_client(std::chrono::microseconds endin_frame)
{

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

void FileToGraphic::snapshot_play()
{
    for (gdi::CaptureApi * cap : this->capture_consumers){
        cap->periodic_snapshot(
            this->record_now, this->mouse_x, this->mouse_y
            , this->ignore_frame_in_timeval
        );
    }
}

void FileToGraphic::log_play() const
{
    if (bool(this->verbose & Verbose::play)) {
        LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
            , unsigned(this->record_now.tv_sec), this->total_orders_count);
    }
}
