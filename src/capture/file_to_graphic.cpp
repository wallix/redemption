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

#include "core/log_id.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/relayout_api.hpp"
#include "gdi/resize_api.hpp"

#include "capture/file_to_graphic.hpp"
#include "capture/agent_data_extractor.hpp"
#include "capture/save_state_chunk.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/hexdump.hpp"
#include "utils/utf.hpp"
#include "utils/png.hpp"
#include <boost/hana/count.hpp>

namespace
{
    inline MonotonicTimePoint monotomic_time_point_from_stream(InStream& in_stream)
    {
        return MonotonicTimePoint(std::chrono::microseconds(in_stream.in_uint64_le()));
    }

    inline RealTimePoint real_time_point_from_stream(InStream& in_stream)
    {
        return RealTimePoint(std::chrono::microseconds(in_stream.in_uint64_le()));
    }
}

FileToGraphic::FileToGraphic(
    Transport & trans,
    MonotonicTimePoint begin_capture,
    MonotonicTimePoint end_capture,
    bool play_video_with_corrupted_bitmap,
    Verbose verbose
)
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
    , begin_capture(begin_capture)
    , end_capture(end_capture == MonotonicTimePoint{} ? MonotonicTimePoint::max() : end_capture)
    , max_order_count(0)
    , statistics()
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
    gdi::RelayoutApi * relayout_ptr,
    gdi::ResizeApi * resize_ptr
)
{
    this->graphic_consumers.push_back(graphic_ptr);
    this->capture_consumers.push_back(capture_ptr);
    this->kbd_input_consumers.push_back(kbd_input_ptr);
    this->capture_probe_consumers.push_back(capture_probe_ptr);
    this->external_event_consumers.push_back(external_event_ptr);
    this->relayout_consumers.push_back(relayout_ptr);
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

/* order count set this->stream.p to the beginning of the next order.
    * Most of the times it means not changing it, except when it must read next chunk
    * when remaining order count is 0.
    * It update chunk headers (merely remaining orders count) and
    * reads the next chunk if necessary.
    */
bool FileToGraphic::next_order()
{
    if (this->chunk_type != WrmChunkType::LAST_IMAGE_CHUNK
     && this->chunk_type != WrmChunkType::PARTIAL_IMAGE_CHUNK
    ) {
        if (this->stream.get_current() == this->stream.get_data_end()
         && this->remaining_order_count
        ) {
            LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                         "order [%d/%u] remaining [%zu/%u]",
                         this->chunk_type,
                         this->chunk_count - this->remaining_order_count,
                         this->chunk_count,
                         this->stream.in_remain(),
                         this->chunk_size);
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
         && this->chunk_type != WrmChunkType::PARTIAL_IMAGE_CHUNK
        ) {
            REDEMPTION_DIAGNOSTIC_PUSH()
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
            switch (this->chunk_type) {
                case WrmChunkType::RDP_UPDATE_ORDERS:
                    this->statistics.graphics_update_chunk++; break;
                case WrmChunkType::RDP_UPDATE_BITMAP:
                case WrmChunkType::RDP_UPDATE_BITMAP2:
                    this->statistics.bitmap_update_chunk++;   break;
                case WrmChunkType::TIMESTAMP_OR_RECORD_DELAY:
                    this->statistics.timestamp_chunk.count++;
                    REDEMPTION_CXX_FALLTHROUGH;
                case WrmChunkType::TIMES:
                case WrmChunkType::META_FILE:
                case WrmChunkType::SAVE_STATE:
                case WrmChunkType::RESET_CHUNK:
                case WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE:
                case WrmChunkType::KBD_INPUT_MASK:
                    this->statistics.internal_order_read_len += this->chunk_size; break;
                default: ;
            }
            REDEMPTION_DIAGNOSTIC_POP()
            if (this->chunk_size > 65536){
                LOG(LOG_ERR,"chunk_size (%u) > 65536", this->chunk_size);
                throw Error(ERR_WRM);
            }
            this->stream = InStream(this->stream_buf);
            if (this->chunk_size - WRM_HEADER_SIZE > 0) {
                auto av = this->trans->recv_boom(
                    this->stream_buf, this->chunk_size - WRM_HEADER_SIZE);
                this->stream = InStream(av);
                this->statistics.internal_order_read_len += WRM_HEADER_SIZE;
            }
        }
        else {
            this->statistics.internal_order_read_len += this->chunk_size;
        }
    }

    if (this->remaining_order_count > 0) {
        this->remaining_order_count--;
    }

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

    [[nodiscard]] ColorCtxFromBppConverter color_ctx(BGRPalette const & palette) const
    {
        return ColorCtxFromBppConverter{ftg.info.bpp, palette};
    }
};

void FileToGraphic::interpret_order()
{
    this->total_orders_count++;
    auto const & palette = BGRPalette::classic_332();

    ReceiveOrder receive_order{*this};

    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wcovered-switch-default")
    switch (this->chunk_type)
    {
    case WrmChunkType::RDP_UPDATE_ORDERS:
    {
        if (!this->meta_ok){
            LOG(LOG_ERR, "Drawing orders chunk must be preceded by a META chunk to get drawing device size");
            throw Error(ERR_WRM);
        }
        if (!this->timestamp_ok){
            LOG(LOG_ERR, "Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing");
            throw Error(ERR_WRM);
        }
        uint8_t control = this->stream.in_uint8();
        uint8_t class_ = (control & (RDP::STANDARD | RDP::SECONDARY));
        if (class_ == RDP::SECONDARY) {
            RDP::AltsecDrawingOrderHeader header(control);
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
            switch (header.orderType) {
                case RDP::AltsecDrawingOrderType::FrameMarker:
                {
                    receive_order.read_and_draw<RDP::FrameMarker>(
                        this->statistics.FrameMarker, Verbose::frame_marker, header);
                }
                break;
                case RDP::AltsecDrawingOrderType::Window:
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
            const Rect clip = (control & RDP::BOUNDS)
                ? this->ssc.common.clip
                : Rect(0, 0, this->info.width, this->info.height);
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

    case WrmChunkType::TIMES:
        this->record_now = monotomic_time_point_from_stream(this->stream);
        this->last_real_time = real_time_point_from_stream(this->stream);
        this->monotonic_real_time = this->record_now;
        this->timestamp_ok = true;
        for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
            obj->external_times(this->record_now.time_since_epoch(), this->last_real_time);
        }
        break;

    case WrmChunkType::TIMESTAMP_OR_RECORD_DELAY:
    {
        auto * const p = this->stream.get_current();

        this->record_now = monotomic_time_point_from_stream(this->stream);

        for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
            obj->external_monotonic_time_point(this->record_now);
        }

        // If some data remains, it is input data : mouse_x, mouse_y and decoded keyboard keys (utf8)
        if (this->stream.in_remain() > 0){
            if (this->stream.in_remain() < 4){
                LOG(LOG_WARNING, "Input data truncated");
                hexdump_d(stream.get_data(), stream.in_remain());
            }

            this->mouse_x = this->stream.in_uint16_le();
            this->mouse_y = this->stream.in_uint16_le();

            if (this->info.version > 1) {
                // ignore_frame_in_timeval
                this->stream.in_skip_bytes(1);
            }

            if (REDEMPTION_UNLIKELY(bool(this->verbose & Verbose::timestamp))) {
                using std::chrono::duration_cast;
                const auto duration = this->record_now.time_since_epoch();
                const auto milliseconds = duration_cast<std::chrono::milliseconds>(duration);
                const auto seconds = duration_cast<std::chrono::seconds>(milliseconds);
                const long long i_seconds = seconds.count();
                const long long i_milliseconds = (milliseconds - seconds).count();
                LOG(LOG_INFO, "TIMESTAMP %lld.%lld mouse (x=%" PRIu16 ", y=%" PRIu16 ")",
                    i_seconds, i_milliseconds, this->mouse_x, this->mouse_y);
            }

            auto input = this->stream.in_skip_bytes(this->stream.in_remain());
            for (gdi::KbdInputApi * kbd : this->kbd_input_consumers){
                InStream in_stream(input);
                while (in_stream.in_remain()) {
                    kbd->kbd_input(this->record_now, in_stream.in_uint32_le());
                }
            }

            if (bool(this->verbose & Verbose::timestamp)) {
                for (auto data = input.data(), end = data + input.size()/4*4; data != end; data += 4) {
                    uint8_t      key8[6];
                    const size_t len = UTF32toUTF8(data, 4/4, key8, sizeof(key8)-1);
                    key8[len] = 0;

                    LOG(LOG_INFO, "TIMESTAMP keyboard '%s'", key8);
                }
            }
        }

        this->timestamp_ok = true;
        this->statistics.timestamp_chunk.total_len += this->stream.get_current() - p;
    }
    break;
    case WrmChunkType::META_FILE:
    {
        auto const old_width = this->info.width;
        auto const old_height = this->info.height;

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
                    this->info.cache_4_entries, this->info.cache_4_size, this->info.cache_4_persistent),
                BmpCache::Verbose::none);
            this->meta_ok = true;
        }

        this->max_screen_dim.w = std::max(this->max_screen_dim.w, this->info.width);
        this->max_screen_dim.h = std::max(this->max_screen_dim.h, this->info.height);

        if (old_width != this->info.width || old_height != this->info.height) {
            for (gdi::ResizeApi * obj : this->resize_consumers){
                // Capture::resize use external_breakpoint()
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
        if (not this->graphic_consumers.empty()) {
            set_rows_from_image_chunk(
                *this->trans,
                this->chunk_type,
                this->chunk_size,
                this->info.width,
                this->graphic_consumers
            );
        }
        else {
            // If no drawable is available ignore images chunks
            this->stream.rewind();
            std::size_t sz = this->chunk_size - WRM_HEADER_SIZE;
            auto av = this->trans->recv_boom(this->stream_buf, sz);
            this->stream = InStream(av);
            this->stream.in_skip_bytes(av.size());
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
            assert(this->stream.in_remain() < bitmap_data.bitmap_size());
            const uint8_t * RM18446_test_data = this->stream.get_current();

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

                RM18446_test_bitmap_data.flags         = uint16_t(BITMAP_COMPRESSION)
                                                       | uint16_t(NO_BITMAP_COMPRESSION_HDR);
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
    case WrmChunkType::RDP_UPDATE_BITMAP2:
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
        LOG_IF(bool(this->verbose & Verbose::rdp_orders), LOG_INFO, "POINTER");

        this->mouse_x         = this->stream.in_uint16_le();
        this->mouse_y         = this->stream.in_uint16_le();
        uint8_t cache_idx     = this->stream.in_uint8();

        // this->stream.in_remain() ?
        if (  chunk_size - WRM_HEADER_SIZE > 5 /*mouse_x(2) + mouse_y(2) + cache_idx(1)*/) {
            size_t start_offset = this->stream.get_offset();
            const Pointer cursor = pointer_loader_32x32(this->stream);

            this->ptr_cache.add_pointer_static(cursor, cache_idx);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->set_pointer(cache_idx, cursor, gdi::GraphicApi::SetPointerMode::New);
                gd->set_pointer(cache_idx, cursor, gdi::GraphicApi::SetPointerMode::Cached);
            }

            this->statistics.CachePointer.total_len += this->stream.get_offset()-start_offset;
            this->statistics.CachePointer.count++;
        }
        else {
            Pointer const& cursor = this->ptr_cache.Pointers[cache_idx];
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->set_pointer(cache_idx, cursor, gdi::GraphicApi::SetPointerMode::Cached);
            }
            this->statistics.PointerIndex.count++;
        }
    }
    break;
    case WrmChunkType::POINTER2:
    {
        LOG_IF(bool(this->verbose & Verbose::rdp_orders), LOG_INFO, "POINTER2");

        size_t start_offset = this->stream.get_offset();
        this->mouse_x = this->stream.in_uint16_le();
        this->mouse_y = this->stream.in_uint16_le();
        uint8_t cache_idx     = this->stream.in_uint8();
        const Pointer cursor = pointer_loader_2(this->stream);

        this->ptr_cache.add_pointer_static(cursor, cache_idx);
        for (gdi::GraphicApi * gd : this->graphic_consumers){
            gd->set_pointer(cache_idx, cursor, gdi::GraphicApi::SetPointerMode::New);
        }
        this->statistics.CachePointer.total_len += this->stream.get_offset()-start_offset;
        this->statistics.CachePointer.count++;
    }
    break;

    case WrmChunkType::POINTER_NATIVE:
    {
        LOG_IF(bool(this->verbose & Verbose::rdp_orders), LOG_INFO, "POINTER_NATIVE");

        const size_t start_offset = this->stream.get_offset();

        const BitsPerPixel data_bpp = checked_int{this->stream.in_uint16_le()}; /* data bpp */

        const uint16_t cache_idx = this->stream.in_uint16_le();

        const Pointer cursor = pointer_loader_new(data_bpp, this->stream, palette, false, false);

/*
        this->mouse_x = this->stream.in_uint16_le();
        this->mouse_y = this->stream.in_uint16_le();
        uint8_t cache_idx     = this->stream.in_uint8();
        const Pointer cursor = pointer_loader_2(this->stream);
*/

        this->ptr_cache.add_pointer_static(cursor, cache_idx);
        for (gdi::GraphicApi * gd : this->graphic_consumers){
            gd->set_pointer(cache_idx, cursor, gdi::GraphicApi::SetPointerMode::New);
        }
        this->statistics.CachePointer.total_len += this->stream.get_offset()-start_offset;
        this->statistics.CachePointer.count++;
    }
    break;

    case WrmChunkType::RESET_CHUNK:
        this->info.compression_algorithm = WrmCompressionAlgorithm::no_compression;

        this->trans = this->trans_source;
    break;
    case WrmChunkType::OLD_SESSION_UPDATE:
    case WrmChunkType::SESSION_UPDATE: {
        auto event_time = MonotonicTimePoint(
            std::chrono::microseconds(this->stream.in_sint64_le()));

        if (event_time > this->record_now) {
            this->record_now = event_time;
            for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                obj->external_monotonic_time_point(this->record_now);
            }
        }

        uint16_t message_length = this->stream.in_uint16_le();
        bytes_view message = this->stream.in_skip_bytes(message_length);

        if (this->capture_probe_consumers.empty()) {
            // nothing
        }
        else if (this->chunk_type == WrmChunkType::OLD_SESSION_UPDATE) {
            AgentDataExtractor extractor;
            if (extractor.extract_old_format_list(message.as_chars())) {
                for (gdi::CaptureProbeApi * cap_probe : this->capture_probe_consumers){
                    cap_probe->session_update(event_time,
                        extractor.log_id(), extractor.kvlist());
                }
            }
        }
        else {
            InStream in(message);

            auto log_id = in.in_uint32_le();

            if (REDEMPTION_UNLIKELY(!is_valid_log_id(safe_int(log_id)))) {
                LOG(LOG_WARNING, "FileToGraphic::interpret_order(): "
                    "Invalid LogId %" PRIu32, log_id);
            }
            else if (AgentDataExtractor::is_relevant_log_id(LogId(log_id))) {
                KVLog kvlogs[255];
                auto* pkv = kvlogs;

                int nbkv {in.in_uint8()};

                for (int i = 0; i < nbkv; ++i) {
                    auto klen = in.in_uint8();
                    auto vlen = in.in_uint16_le();
                    auto key = in.in_skip_bytes(klen);
                    auto value = in.in_skip_bytes(vlen);
                    *pkv++ = KVLog(key.as_chars(), value.as_chars());
                }

                for (gdi::CaptureProbeApi * cap_probe : this->capture_probe_consumers){
                    cap_probe->session_update(event_time, LogId(log_id), {{kvlogs, pkv}});
                }
            }
        }

        this->timestamp_ok = true;
    }
    break;
    case WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE:
        for (gdi::CaptureProbeApi * cap_probe : this->capture_probe_consumers){
            cap_probe->possible_active_window_change();
        }
    break;

    case WrmChunkType::RAIL_WINDOW_RECT_START:
        this->rail_window_rect_start.x  = this->stream.in_sint16_le();
        this->rail_window_rect_start.y  = this->stream.in_sint16_le();
        this->rail_window_rect_start.cx = this->stream.in_uint16_le();
        this->rail_window_rect_start.cy = this->stream.in_uint16_le();
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

    case WrmChunkType::KBD_INPUT_MASK:
        {
            const bool enable = this->stream.in_uint8();

            for (gdi::KbdInputApi * kbd : this->kbd_input_consumers){
                kbd->enable_kbd_input_mask(enable);
            }
        }
    break;

    case WrmChunkType::MONITOR_LAYOUT:
        {
            MonitorLayoutPDU monitor_layout_pdu;

            monitor_layout_pdu.recv(this->stream);

            for (gdi::RelayoutApi * relayout : this->relayout_consumers){
                relayout->relayout(monitor_layout_pdu);
            }
        }
    break;

    case WrmChunkType::INVALID_CHUNK:
    default:
        LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
        throw Error(ERR_WRM);
    }
    REDEMPTION_DIAGNOSTIC_POP()
}


void FileToGraphic::process_windowing( InStream & stream, const RDP::AltsecDrawingOrderHeader & header)
{
    LOG_IF(bool(this->verbose & Verbose::probe), LOG_INFO, "rdp_orders::process_windowing");

    const uint32_t FieldsPresentFlags = [&]{
        InStream stream2(stream.remaining_bytes());
        stream2.in_skip_bytes(2);    // OrderSize(2)
        return stream2.in_uint32_le();
    }();

    switch (FieldsPresentFlags & ( uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW)
                                 | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY)
                                 | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP)))
    {
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
    LOG_IF(bool(this->verbose & Verbose::probe), LOG_INFO,
        "rdp_orders::process_window_information");

    switch (FieldsPresentFlags & ( uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_NEW)
                                 | uint32_t(RDP::RAIL::WINDOW_ORDER_ICON)
                                 | uint32_t(RDP::RAIL::WINDOW_ORDER_CACHEDICON)
                                 | uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)))
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
    LOG_IF(bool(this->verbose & Verbose::probe), LOG_INFO,
        "rdp_orders::process_notification_icon_information");

    switch (FieldsPresentFlags & ( uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_NEW)
                                 | uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)))
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
    LOG_IF(bool(this->verbose & Verbose::probe), LOG_INFO,
        "rdp_orders::process_desktop_information");

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
    this->privplay([](MonotonicTimePoint /*t*/){}, requested_to_stop);
}

void FileToGraphic::snapshot_play()
{
    for (gdi::CaptureApi * cap : this->capture_consumers){
        cap->periodic_snapshot(this->record_now, this->mouse_x, this->mouse_y);
    }
}

void FileToGraphic::log_play() const
{
    if (REDEMPTION_UNLIKELY(bool(this->verbose & Verbose::play))) {
        const auto duration = this->record_now.time_since_epoch();
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        long long i_seconds = seconds.count();
        LOG(LOG_INFO, "replay TIMESTAMP (first timestamp) = %lld order=%u",
            i_seconds, this->total_orders_count);
    }
}
