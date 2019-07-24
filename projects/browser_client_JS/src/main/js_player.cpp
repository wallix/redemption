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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "capture/save_state_chunk.hpp"
#include "capture/wrm_meta_chunk.hpp"
#include "capture/wrm_chunk_type.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/state_chunk.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "transport/mwrm_reader.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/difftimeval.hpp"
#include "utils/timeval_ops.hpp"
#include "utils/log.hpp"

#include "red_emscripten/bind.hpp"
#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/val.hpp"
#include "redjs/browser_graphic.hpp"

#include <chrono>


namespace
{
    namespace jsnames
    {
        constexpr char const* update_mouse_position = "setPointerPosition";
        constexpr char const* set_time = "setTime";
    }

    using SetPointerMode = gdi::GraphicApi::SetPointerMode;

    const Pointer dummy_cursor; // for gdi::GraphicApi::SetPointerMode::Cached
}

struct WrmPlayer
{
    WrmPlayer(emscripten::val callbacks) noexcept
    : callbacks(std::move(callbacks))
    , gd(this->callbacks, 0, 0)
    {}

    std::size_t remaining_data_size() const noexcept
    {
        return this->offset;
    }

    void next_data(std::string data)
    {
        this->data.erase(0, this->offset);
        this->offset = 0;
        this->chunk.count = 0;
        this->in_stream = InStream();

        if (this->data.empty())
        {
            this->data = std::move(data);
        }
        else
        {
            this->data += data;
        }
    }

    bool next_timestamp_order()
    {
        while (this->next_order())
        {
            this->interpret_order();

            if (WrmChunkType::TIMESTAMP == this->chunk.type
             || WrmChunkType::SESSION_UPDATE == this->chunk.type)
            {
                return true;
            }
        }

        return false;
    }

    bool next_order() noexcept
    {
        if (!this->chunk.count)
        {
            this->offset += this->in_stream.get_capacity();

            InStream header(const_bytes_view(this->data).from_at(this->offset));
            if (header.in_remain() < WRM_HEADER_SIZE)
            {
                this->in_stream = InStream();
                return false;
            }

            this->chunk.type = safe_cast<WrmChunkType>(header.in_uint16_le());
            auto chunk_size = header.in_uint32_le();
            this->chunk.count = header.in_uint16_le();

            if (header.in_remain() + WRM_HEADER_SIZE < chunk_size)
            {
                this->in_stream = InStream();
                return false;
            }

            this->in_stream = InStream({header.get_data(), chunk_size});
            this->in_stream.in_skip_bytes(WRM_HEADER_SIZE);
        }

        if (this->chunk.count > 0)
        {
            --this->chunk.count;
        }

        return true;
    }

    void interpret_order()
    {
        switch (this->chunk.type)
        {
            case WrmChunkType::RDP_UPDATE_ORDERS:
            {
                uint8_t control = this->in_stream.in_uint8();

                switch (uint8_t(control & (RDP::STANDARD | RDP::SECONDARY)))
                {
                    case RDP::SECONDARY:
                        this->_interpret_secondary_order(control);
                        break;
                    case RDP::STANDARD:
                        this->_interpret_standard_order(control);
                        break;
                    case RDP::STANDARD | RDP::SECONDARY:
                        this->_interpret_cache_order();
                        break;
                    default:
                        LOG(LOG_ERR, "Unsupported drawing order detected : protocol error");
                        throw Error(ERR_WRM);
                }

                break;
            }

            case WrmChunkType::TIMESTAMP:
            {
                timeval now;
                this->in_stream.in_timeval_from_uint64le_usec(now);

                if (this->in_stream.in_remain() >= 4)
                {
                    this->_interpret_mouse_position();
                }

                this->_skip_chunk();
                this->_update_time(now);
                break;
            }

            case WrmChunkType::META_FILE:
            {
                this->wrm_info.receive(this->in_stream);
                this->_skip_chunk();
                this->gd.resize_canvas(this->wrm_info.width, this->wrm_info.height);
                const uint16_t reserved_by_watning_list = this->wrm_info.use_waiting_list;
                this->gd.set_bmp_cache_entries({
                    uint16_t(this->wrm_info.cache_0_size + reserved_by_watning_list),
                    uint16_t(this->wrm_info.cache_1_size + reserved_by_watning_list),
                    uint16_t(this->wrm_info.cache_2_size + reserved_by_watning_list)
                });
                // TODO this->wrm_info.compression_algorithm
                break;
            }

            case WrmChunkType::SAVE_STATE:
                SaveStateChunk().recv(this->in_stream, this->ssc, this->wrm_info.version);
                break;

            case WrmChunkType::LAST_IMAGE_CHUNK:
            case WrmChunkType::PARTIAL_IMAGE_CHUNK:
            {
                // TODO
                /*if (this->graphic_consumers.size())
                {
                    set_rows_from_image_chunk(
                        *this->trans,
                        this->chunk_type,
                        this->chunk_size,
                        this->screen_rect.cx,
                        this->graphic_consumers
                    );
                }
                else*/
                {
                    this->_skip_chunk();
                }

                break;
            }

            case WrmChunkType::RDP_UPDATE_BITMAP:
            {
                RDPBitmapData bitmap_data;
                bitmap_data.receive(this->in_stream);

                // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
                if (!(bitmap_data.flags & BITMAP_COMPRESSION)) {
                    assert(this->in_stream.in_remain() < bitmap_data.bitmap_size());
                    const uint8_t * RM18446_test_data = this->in_stream.get_current();

                    size_t RM18446_adjusted_size = 0;

                    Bitmap RM18446_test_bitmap( this->wrm_info.bpp
                                    , checked_int(bitmap_data.bits_per_pixel)
                                    , /*0*/&BGRPalette::classic_332()
                                    , bitmap_data.width
                                    , bitmap_data.height
                                    , RM18446_test_data
                                    , this->in_stream.in_remain()
                                    , true
                                    , &RM18446_adjusted_size
                                    );

                    if (RM18446_adjusted_size) {
                        RDPBitmapData RM18446_test_bitmap_data = bitmap_data;

                        RM18446_test_bitmap_data.flags         = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR; /*NOLINT*/
                        RM18446_test_bitmap_data.bitmap_length = RM18446_adjusted_size;

                        this->in_stream.in_skip_bytes(RM18446_adjusted_size);

                        this->gd.draw(RM18446_test_bitmap_data, RM18446_test_bitmap);

                        break;
                    }
                }

                const uint8_t * data = this->in_stream.in_uint8p(bitmap_data.bitmap_size());

                Bitmap bitmap( this->wrm_info.bpp
                            , checked_int(bitmap_data.bits_per_pixel)
                            , /*0*/&BGRPalette::classic_332()
                            , bitmap_data.width
                            , bitmap_data.height
                            , data
                            , bitmap_data.bitmap_size()
                            , (bitmap_data.flags & BITMAP_COMPRESSION)
                            );

                this->gd.draw(bitmap_data, bitmap);

                break;
            }

            case WrmChunkType::RDP_UPDATE_BITMAP2:
            {
                RDPBitmapData bitmap_data;
                bitmap_data.receive(this->in_stream);

                const uint8_t * data = this->in_stream.in_uint8p(bitmap_data.bitmap_size());

                Bitmap bitmap( this->wrm_info.bpp
                            , checked_int(bitmap_data.bits_per_pixel)
                            , /*0*/&BGRPalette::classic_332()
                            , bitmap_data.width
                            , bitmap_data.height
                            , data
                            , bitmap_data.bitmap_size()
                            , (bitmap_data.flags & BITMAP_COMPRESSION)
                            );

                this->gd.draw(bitmap_data, bitmap);

                break;
            }

            case WrmChunkType::POINTER:
            {
                this->_interpret_mouse_position();

                uint8_t const cache_idx = this->in_stream.in_uint8();

                if (this->in_stream.in_remain()) {
                    const Pointer cursor = pointer_loader_32x32(this->in_stream);
                    this->gd.set_pointer(cache_idx, cursor, SetPointerMode::New);
                }
                this->gd.set_pointer(cache_idx, dummy_cursor, SetPointerMode::Cached);
                break;
            }

            case WrmChunkType::POINTER2:
            {
                this->_interpret_mouse_position();

                uint8_t cache_idx = this->in_stream.in_uint8();
                const Pointer cursor = pointer_loader_2(this->in_stream);
                this->gd.set_pointer(cache_idx, cursor, SetPointerMode::New);
                break;
            }

            case WrmChunkType::RESET_CHUNK:
                this->wrm_info.compression_algorithm = WrmCompressionAlgorithm::no_compression;
                // TODO
                break;

            case WrmChunkType::SESSION_UPDATE:
            {
                timeval now;
                this->in_stream.in_timeval_from_uint64le_usec(now);
                this->in_stream.in_skip_bytes(this->in_stream.in_uint16_le());
                this->_update_time(now);
                break;
            }

            case WrmChunkType::POSSIBLE_ACTIVE_WINDOW_CHANGE:
                // nothing
                break;

            case WrmChunkType::IMAGE_FRAME_RECT:
                this->_skip_chunk();
                break;

            case WrmChunkType::KBD_INPUT_MASK:
                this->in_stream.in_skip_bytes(1);
                break;

            default:
                LOG(LOG_ERR, "unknown chunk type %d", this->chunk.type);
                throw Error(ERR_WRM);
        }
    }

private:
    void _skip_chunk() noexcept
    {
        this->chunk.count = 0;
        this->in_stream.in_skip_bytes(this->in_stream.in_remain());
    }

    void _interpret_secondary_order(uint8_t control);
    void _interpret_standard_order(uint8_t control);
    void _interpret_cache_order();

    void _interpret_mouse_position()
    {
        uint16_t mouse_x = this->in_stream.in_uint16_le();
        uint16_t mouse_y = this->in_stream.in_uint16_le();

        redjs::emval_call(this->callbacks, jsnames::update_mouse_position,
            mouse_x, mouse_y);
    }

    void _update_time(timeval const& now)
    {
        auto ms = to_ms(now);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
        redjs::emval_call(this->callbacks, jsnames::set_time,
            uint32_t(seconds.count()), uint32_t((ms - seconds).count()));
    }

    struct Chunk
    {
        uint16_t count = 0;
        WrmChunkType type;
    };

    Chunk chunk;
    emscripten::val callbacks;
    std::string data;
    std::size_t offset = 0;
    InStream in_stream;
    StateChunk ssc;
    WrmMetaChunk wrm_info;
    GlyphCache gly_cache;
    redjs::BrowserGraphic gd;
};


EMSCRIPTEN_BINDINGS(player)
{
    redjs::class_<WrmPlayer>("WrmPlayer")
        .constructor<emscripten::val>()
        .function("nextData", &WrmPlayer::next_data)
        .function("nextOrder", &WrmPlayer::next_order)
        .function("interpretOrder", &WrmPlayer::interpret_order)
        .function("remainingDataSize", &WrmPlayer::remaining_data_size)
        .function("nextTimestampOrder", &WrmPlayer::next_timestamp_order)
    ;
}


void WrmPlayer::_interpret_secondary_order(uint8_t control)
{
    RDP::AltsecDrawingOrderHeader header(control);
    switch (header.orderType)
    {
        case RDP::AltsecDrawingOrderType::FrameMarker:
        {
            RDP::FrameMarker cmd;
            cmd.receive(this->in_stream, header);
            this->gd.draw(cmd);
            break;
        }

        case RDP::AltsecDrawingOrderType::Window:
        {
            std::size_t len = this->in_stream.clone().in_uint16_le();
            this->in_stream.in_skip_bytes(std::min(len, this->in_stream.in_remain()));
            break;
        }

        default:
            LOG(LOG_WARNING, "unsupported Alternate Secondary Drawing Order (%d)", header.orderType);
            throw Error(ERR_WRM);
    }
}

void WrmPlayer::_interpret_cache_order()
{
    RDPSecondaryOrderHeader header(this->in_stream);
    uint8_t const *next_order = this->in_stream.get_current() + header.order_data_length();

    switch (header.type)
    {
        case RDP::TS_CACHE_BITMAP_COMPRESSED:
        case RDP::TS_CACHE_BITMAP_UNCOMPRESSED:
        case RDP::TS_CACHE_BITMAP_COMPRESSED_REV2:
        case RDP::TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
        case RDP::TS_CACHE_BITMAP_COMPRESSED_REV3:
        {
            RDPBmpCache cmd;
            cmd.receive(this->in_stream, header, BGRPalette::classic_332(), this->wrm_info.bpp);
            this->gd.draw(cmd);
            break;
        }

        case RDP::TS_CACHE_GLYPH:
        {
            RDPGlyphCache cmd;
            cmd.receive(this->in_stream, header);
            this->gly_cache.set_glyph(
                FontChar(std::move(cmd.aj), cmd.x, cmd.y, cmd.cx, cmd.cy, -1),
                cmd.cacheId, cmd.cacheIndex
            );
            break;
        }

        default:
            LOG(LOG_ERR, "unsupported SECONDARY ORDER (%u)", header.type);
            throw Error(ERR_WRM);
    }

    this->in_stream.in_skip_bytes(next_order - this->in_stream.get_current());
}

void WrmPlayer::_interpret_standard_order(uint8_t control)
{
    RDPPrimaryOrderHeader header = this->ssc.common.receive(this->in_stream, control);
    const Rect clip = (control & RDP::BOUNDS)
      ? this->ssc.common.clip
      : Rect(0, 0, this->wrm_info.width, this->wrm_info.height);

    auto read_and_draw = [&](auto& cmd, auto const&... draw_args)
    {
        cmd.receive(this->in_stream, header);
        this->gd.draw(cmd, clip, draw_args...);
    };

    auto color_ctx = [this]()
    {
        return gdi::ColorCtx::from_bpp(this->wrm_info.bpp, BGRPalette::classic_332());
    };

    switch (this->ssc.common.order)
    {
        case RDP::GLYPHINDEX: read_and_draw(this->ssc.glyphindex, color_ctx(), this->gly_cache); break;
        case RDP::DESTBLT: read_and_draw(this->ssc.destblt); break;
        case RDP::MULTIDSTBLT: read_and_draw(this->ssc.multidstblt); break;
        case RDP::MULTIOPAQUERECT: read_and_draw(this->ssc.multiopaquerect, color_ctx()); break;
        case RDP::MULTIPATBLT: read_and_draw(this->ssc.multipatblt, color_ctx()); break;
        case RDP::MULTISCRBLT: read_and_draw(this->ssc.multiscrblt); break;
        case RDP::PATBLT: read_and_draw(this->ssc.patblt, color_ctx()); break;
        case RDP::SCREENBLT: read_and_draw(this->ssc.scrblt); break;
        case RDP::LINE: read_and_draw(this->ssc.lineto, color_ctx()); break;
        case RDP::RECT: read_and_draw(this->ssc.opaquerect, color_ctx()); break;
        case RDP::MEMBLT: read_and_draw(this->ssc.memblt); break;
        case RDP::MEM3BLT: read_and_draw(this->ssc.mem3blt, color_ctx()); break;
        case RDP::POLYLINE: read_and_draw(this->ssc.polyline, color_ctx()); break;
        case RDP::ELLIPSESC: read_and_draw(this->ssc.ellipseSC, color_ctx()); break;
        default:
            /* error unknown order */
            LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->ssc.common.order);
            throw Error(ERR_WRM);
    }
}
