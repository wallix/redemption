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
   Author(s): Christophe Grosjean, Raphael Zhou

   RDPSerializer is an implementation of GraphicApi that know how to serialize RDP Orders
   and send them on the wire or store them in a file (actual storage will be provided as a Transport class).
   Serialized RDP orders are put in a chunk and sent when flush is called (either explicit call or because
   the provided buffer is full).
*/


#pragma once

// MS-RDPECGI 2.2.2.2 Fast-Path Orders Update (TS_FP_UPDATE_ORDERS)
// ================================================================
// The TS_FP_UPDATE_ORDERS structure contains primary, secondary, and alternate
// secondary drawing orders aligned on byte boundaries. This structure conforms
// to the layout of a Fast-Path Update (see [MS-RDPBCGR] section 2.2.9.1.2.1)
// and is encapsulated within a Fast-Path Update PDU (see [MS-RDPBCGR] section
// 2.2.9.1.2.1.1).

// updateHeader (1 byte): An 8-bit, unsigned integer. The format of this field
//   is the same as the updateHeader byte field described in the Fast-Path
//   Update structure (see [MS-RDPBCGR] section 2.2.9.1.2.1). The updateCode
//   bitfield (4 bits in size) MUST be set to FASTPATH_UPDATETYPE_ORDERS (0x0).

// compressionFlags (1 byte): An 8-bit, unsigned integer. The format of this
//   optional field (as well as the possible values) is the same as the
//   compressionFlags field described in the Fast-Path Update structure
//   specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// size (2 bytes): A 16-bit, unsigned integer. The format of this field (as well
//   as the possible values) is the same as the size field described in the
//   Fast-Path Update structure specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//   Order (section 2.2.2.1.1) structures contained in the orderData field.

// orderData (variable): A variable-sized array of Drawing Order (section
//   2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//   primary, secondary, or alternate secondary drawing order. The controlFlags
//   field of the Drawing Order identifies the type of drawing order.


// MS-RDPECGI 2.2.2.1 Orders Update (TS_UPDATE_ORDERS_PDU_DATA)
// ============================================================
// The TS_UPDATE_ORDERS_PDU_DATA structure contains primary, secondary, and
// alternate secondary drawing orders aligned on byte boundaries. This structure
// conforms to the layout of a Slow Path Graphics Update (see [MS-RDPBCGR]
// section 2.2.9.1.1.3.1) and is encapsulated within a Graphics Update PDU (see
// [MS-RDPBCGR] section 2.2.9.1.1.3.1.1).

// shareDataHeader (18 bytes): Share Data Header (see [MS-RDPBCGR], section
//   2.2.8.1.1.1.2) containing information about the packet. The type subfield
//   of the pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
//   MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//   Header MUST be set to PDUTYPE2_UPDATE (2).

// updateType (2 bytes): A 16-bit, unsigned integer. The field contains the
//   graphics update type. This field MUST be set to UPDATETYPE_ORDERS (0x0000).

// pad2OctetsA (2 bytes): A 16-bit, unsigned integer used as a padding field.
//   Values in this field are arbitrary and MUST be ignored.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//   Order (section 2.2.2.1.1) structures contained in the orderData field.

// pad2OctetsB (2 bytes): A 16-bit, unsigned integer used as a padding field.
//   Values in this field are arbitrary and MUST be ignored.

// orderData (variable): A variable-sized array of Drawing Order (section
//   2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//   primary, secondary, or alternate secondary drawing order. The controlFlags
//   field of the Drawing Order identifies the type of drawing order.

#include "utils/sugar/finally.hpp"
#include "utils/verbose_flags.hpp"

#include "core/RDP/bitmapupdate.hpp"

#include "core/RDP/state_chunk.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"

#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/orders/RDPSurfaceCommands.hpp"

#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/caches/glyphcache.hpp"

#include "utils/stream.hpp"
#include "transport/transport.hpp"

#include "gdi/graphic_api.hpp"
#include "gdi/graphic_cmd_color.hpp"



class RDPSerializer
: public gdi::GraphicApi
{
public:
    static constexpr std::size_t SERIALIZER_HEADER_SIZE = 106;

    // Packet more than 16384 bytes can cause MSTSC to crash.
    enum { MAX_ORDERS_SIZE = 16384 };

protected:
    OutStream & stream_orders;
    OutStream & stream_bitmaps;

    const BitsPerPixel capture_bpp;

private:
          int bitmap_cache_version;
    const bool use_bitmap_comp;
    const bool use_compact_packets;
    const size_t max_data_block_size;

protected:

    // TODO: these 3 orders should be in next movie format and moved to StateChunk
    // until then it will lead to subtile recording bugs if these orders are enabled
    // (order state not propagated between movie parts).
    // Internal state of orders
    RDPPolygonSC polygonSC;
    RDPPolygonCB polygonCB;
    RDPEllipseCB ellipseCB = RDPEllipseCB(Rect(), 0, 0, RDPColor{}, RDPColor{}, RDPBrush());

    StateChunk ssc;

    // state variables for gathering batch of orders
    size_t order_count = 0;
    size_t bitmap_count = 0;

    // TODO: check how caches are managed by recording layer
    BmpCache      & bmp_cache;
    GlyphCache    & glyph_cache;
    PointerCache  & pointer_cache;

    // RDPSerializer serializer(, ...., RDPSerializer::Verbose)

public:
    REDEMPTION_VERBOSE_FLAGS(protected, verbose)
    {
        none,
        pointer             = 4,
        primary_orders      = 32,
        secondary_orders    = 64,
        bitmap_update       = 128,
        surface_commands    = 256,
        bmp_cache           = 512,
        internal_buffer     = 1024
    };

    RDPSerializer( OutStream & stream_orders
                 , OutStream & stream_bitmaps
                 , const BitsPerPixel bpp
                 , BmpCache & bmp_cache
                 , GlyphCache & glyph_cache
                 , PointerCache & pointer_cache
                 , const int bitmap_cache_version
                 , const bool use_bitmap_comp
                 , const bool use_compact_packets
                 , size_t max_data_block_size
                 , bool experimental_enable_serializer_data_block_size_limit
                 , Verbose verbose)
    : stream_orders(stream_orders)
    , stream_bitmaps(stream_bitmaps)
    , capture_bpp(bpp)
    , bitmap_cache_version(bitmap_cache_version)
    , use_bitmap_comp(use_bitmap_comp)
    , use_compact_packets(use_compact_packets)
    , max_data_block_size(std::min(max_data_block_size,
                                   (experimental_enable_serializer_data_block_size_limit ?
                                    static_cast<decltype(max_data_block_size)>(MAX_ORDERS_SIZE) :
                                    std::numeric_limits<decltype(max_data_block_size)>::max())))
    , bmp_cache(bmp_cache)
    , glyph_cache(glyph_cache)
    , pointer_cache(pointer_cache)
    , verbose{verbose}
    {
    }

    ~RDPSerializer() override = default;

    [[nodiscard]] size_t get_max_data_block_size() const {
        return (this->max_data_block_size - SERIALIZER_HEADER_SIZE);
    }

    void force_using_cache_bitmap_r2() {
        LOG(LOG_INFO, "RDPSerializer::force_using_cache_bitmap_r2");
        this->bitmap_cache_version = 2;
    }

protected:
    virtual void flush_orders() = 0;
    virtual void flush_bitmaps() = 0;

    virtual void send_pointer(int cache_idx, const Pointer & cursor) = 0;
    virtual void cached_pointer_update(int cache_idx) = 0;

public:
    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    void reserve_order(size_t asked_size)
    {
        //LOG(LOG_INFO, "RDPSerializer::reserve_order %u (avail=%u)", asked_size, this->stream_orders.size());
        // To support 64x64 32-bit bitmap.
        size_t const max_packet_size = std::min(this->stream_orders.get_capacity(), this->max_data_block_size);
        size_t const used_size = this->stream_orders.get_offset();
        LOG_IF(bool(this->verbose & Verbose::internal_buffer), LOG_INFO
          , "<Serializer %p> RDPSerializer::reserve_order[%zu](%zu) used=%zu free=%zu"
          , static_cast<void*>(this)
          , this->order_count
          , asked_size, used_size
          , max_packet_size - used_size - SERIALIZER_HEADER_SIZE);

        if (asked_size + SERIALIZER_HEADER_SIZE > max_packet_size) {
            LOG( LOG_ERR
               , "(asked size (%zu) + HEADER_SIZE (%zu) = %zu) > order batch capacity (%zu)"
               , asked_size
               , SERIALIZER_HEADER_SIZE
               , asked_size + SERIALIZER_HEADER_SIZE
               , max_packet_size);
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }
        if (this->bitmap_count) { this->flush_bitmaps(); }
        const size_t max_order_batch = 4096;
        if (   (this->order_count >= max_order_batch)
            || ((used_size + asked_size + SERIALIZER_HEADER_SIZE) > max_packet_size)) {
            this->flush_orders();
        }
        this->order_count++;
        //LOG(LOG_INFO, "RDPSerializer::reserve_order done");
    }

private:
    struct color_convertor
    {
        BitsPerPixel depth_encoding;
        gdi::ColorCtx color_ctx;

        RDPColor operator()(RDPColor c) const noexcept
        {
            auto const d = color_decode(c, color_ctx);
            auto const e = color_encode(d, depth_encoding);
            return e;
        }
    };

    template<class Cmd, class F>
    void reencode_cmd_color(Cmd const & cmd, gdi::ColorCtx color_ctx, F f) const
    {
        if (this->capture_bpp == color_ctx.depth().to_bpp()) {
            f(cmd);
        }
        else {
            auto new_cmd = cmd;
            gdi::GraphicCmdColor::encode_cmd_color(
                color_convertor{this->capture_bpp, color_ctx},
                new_cmd
            );
            f(new_cmd);
        }
    }

public:
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}

    void draw(RDPOpaqueRect const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        //LOG(LOG_INFO, "RDPSerializer::draw::RDPOpaqueRect");
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPOpaqueRect const & cmd){
            this->reserve_order(23);
            RDPOrderCommon newcommon(RDP::RECT, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.opaquerect);
            this->ssc.common = newcommon;
            this->ssc.opaquerect = cmd;

            if (bool(this->verbose & Verbose::primary_orders)) {
                cmd.log(LOG_INFO, this->ssc.common.clip);
            }
        });
        //LOG(LOG_INFO, "RDPSerializer::draw::RDPOpaqueRect done");
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override {
        this->reserve_order(25);
        RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.scrblt);
        this->ssc.common = newcommon;
        this->ssc.scrblt = cmd;
        if (bool(this->verbose & Verbose::primary_orders)) {
            cmd.log(LOG_INFO, this->ssc.common.clip);
        }
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override {
        this->reserve_order(21);
        RDPOrderCommon newcommon(RDP::DESTBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.destblt);
        this->ssc.common = newcommon;
        this->ssc.destblt = cmd;
        if (bool(this->verbose & Verbose::primary_orders)) {
            cmd.log(LOG_INFO, this->ssc.common.clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        this->reserve_order(395 * 2);
        RDPOrderCommon newcommon(RDP::MULTIDSTBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.multidstblt);
        this->ssc.common      = newcommon;
        this->ssc.multidstblt = cmd;
        if (bool(this->verbose & Verbose::primary_orders)) {
            cmd.log(LOG_INFO, this->ssc.common.clip);
        }
    }

    void draw(RDPMultiOpaqueRect const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPMultiOpaqueRect const & cmd){
            this->reserve_order(397 * 2);
            RDPOrderCommon newcommon(RDP::MULTIOPAQUERECT, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.multiopaquerect);
            this->ssc.common          = newcommon;
            this->ssc.multiopaquerect = cmd;
            if (bool(this->verbose & Verbose::primary_orders)) {
                cmd.log(LOG_INFO, this->ssc.common.clip);
            }
        });
    }

    void draw(RDP::RDPMultiPatBlt const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDP::RDPMultiPatBlt const & cmd){
            this->reserve_order(412 * 2);
            RDPOrderCommon newcommon(RDP::MULTIPATBLT, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.multipatblt);
            this->ssc.common      = newcommon;
            this->ssc.multipatblt = cmd;
            if (bool(this->verbose & Verbose::primary_orders)) {
                cmd.log(LOG_INFO, this->ssc.common.clip);
            }
        });
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        this->reserve_order(399 * 2);
        RDPOrderCommon newcommon(RDP::MULTISCRBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.multiscrblt);
        this->ssc.common      = newcommon;
        this->ssc.multiscrblt = cmd;
        if (bool(this->verbose & Verbose::primary_orders)) {
            cmd.log(LOG_INFO, this->ssc.common.clip);
        }
    }

    void draw(RDPPatBlt const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPPatBlt const & cmd){
            this->reserve_order(29);
            RDPOrderCommon newcommon(RDP::PATBLT, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.patblt);
            this->ssc.common = newcommon;
            this->ssc.patblt = cmd;
            if (bool(this->verbose & Verbose::primary_orders)) {
                cmd.log(LOG_INFO, this->ssc.common.clip);
            }
        });
    }

    // These should not be part of draw interface, they have their own pdu
    // same thing for bitmap update
    void draw(RDPSetSurfaceCommand const & cmd) override {}
    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override {}


protected:
    void emit_bmp_cache(uint8_t cache_id, uint16_t cache_idx, bool in_wait_list)
    {
        const Bitmap & bmp = (in_wait_list)
            ? this->bmp_cache.get(BmpCache::MAXIMUM_NUMBER_OF_CACHES, cache_idx)
            : this->bmp_cache.get(cache_id, cache_idx);

        if (!bmp.is_valid()) {
            //LOG(LOG_INFO, "skipping RDPSerializer::emit_bmp_cache for %u:%u (entry not used)",
            //    cache_id, cache_idx);
            return;
        }
        RDPBmpCache cmd_cache(bmp, cache_id, cache_idx,
            this->bmp_cache.is_cache_persistent(cache_id), in_wait_list,
            bool(this->verbose & Verbose::secondary_orders));
        bool really_use_bitmap_comp = this->use_bitmap_comp;
        if (really_use_bitmap_comp) {
            if (!cmd_cache.bmp.has_data_compressed()) {
                StaticOutStream<65535> bmp_stream;
                cmd_cache.bmp.compress(this->capture_bpp, bmp_stream);
            }
            if (cmd_cache.bmp.data_compressed().size() >= cmd_cache.bmp.bmp_size()) {
                really_use_bitmap_comp = false;
            }
        }
        this->reserve_order(cmd_cache.bmp.bmp_size() + 32);
        cmd_cache.emit( this->capture_bpp, this->stream_orders, this->bitmap_cache_version
                      , really_use_bitmap_comp, this->use_compact_packets);

        if (bool(this->verbose & Verbose::secondary_orders)) {
            cmd_cache.log(LOG_INFO);
        }
    }

    void emit_glyph_cache(uint8_t cacheId, uint8_t cacheIndex) {
        FontChar & fc = this->glyph_cache.glyphs[cacheId][cacheIndex].font_item;
        RDPGlyphCache cmd(
            cacheId, /*1, */cacheIndex, fc.offsetx, fc.offsety, fc.width, fc.height, std::move(fc.data));
        // always restored fc.data
        auto finally_ = finally([&]{ fc.data = std::move(cmd.aj); });

        this->reserve_order(cmd.total_order_size());
        cmd.emit(this->stream_orders);

        if (bool(this->verbose & Verbose::secondary_orders)) {
            cmd.log(LOG_INFO);
        }
    }

public:
    template<class MemBlt>
    void draw_memblt(const MemBlt & cmd, MemBlt & this_memblt, Rect clip, const Bitmap & oldbmp)
    {
        uint32_t res          = this->bmp_cache.cache_bitmap(oldbmp);
        bool     in_wait_list = (res >> 16) & BmpCache::IN_WAIT_LIST;
        uint8_t  cache_id     = (res >> 16) & 0x7;
        uint16_t cache_idx    = res;

        using is_RDPMemBlt = std::is_same<RDPMemBlt, MemBlt>;

        LOG_IF(bool(this->verbose & Verbose::bmp_cache), LOG_INFO
          , "RDPSerializer: draw %s, cache_id=%u cache_index=%u in_wait_list=%s"
          , is_RDPMemBlt() ? "MemBlt" : "Mem3Blt", cache_id, cache_idx
          , (in_wait_list ? "true" : "false"));

        if ((res >> 24) == BmpCache::ADDED_TO_CACHE) {
            this->emit_bmp_cache(cache_id, cache_idx, in_wait_list);
        }
        else if ((this->bmp_cache.owner == BmpCache::Recorder) && !this->bmp_cache.is_cached(cache_id, cache_idx)) {
            this->emit_bmp_cache(cache_id, cache_idx, in_wait_list);
            this->bmp_cache.set_cached(cache_id, cache_idx, true);
        }

        MemBlt newcmd = cmd;
        newcmd.cache_id = cache_id;
        newcmd.cache_idx = (in_wait_list ? uint16_t(RDPBmpCache::BITMAPCACHE_WAITING_LIST_INDEX) : cache_idx);

        this->reserve_order(is_RDPMemBlt() ? 30 : 60);
        RDPOrderCommon newcommon(is_RDPMemBlt() ? RDP::MEMBLT : RDP::MEM3BLT, clip);
        newcmd.emit(this->stream_orders, newcommon, this->ssc.common, this_memblt);
        this->ssc.common = newcommon;
        this_memblt = newcmd;
        if (bool(this->verbose & Verbose::primary_orders)) {
            newcmd.log(LOG_INFO, this->ssc.common.clip);
        }
    }

public:
    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & oldbmp) override {
        this->draw_memblt(cmd, this->ssc.memblt, clip, oldbmp);
    }

    void draw(RDPMem3Blt const & cmd_, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & oldbmp) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPMem3Blt const & cmd){
            this->draw_memblt(cmd, this->ssc.mem3blt, clip, oldbmp);
        });
    }

    void draw(RDPLineTo const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPLineTo const & cmd){
            this->reserve_order(32);
            RDPOrderCommon newcommon(RDP::LINE, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.lineto);
            this->ssc.common = newcommon;
            this->ssc.lineto = cmd;
            if (bool(this->verbose & Verbose::primary_orders)) {
                cmd.log(LOG_INFO, this->ssc.common.clip);
            }
        });
    }

    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        auto get_delta = [] (RDPGlyphIndex & cmd, uint8_t & i) -> uint16_t {
            uint16_t delta = cmd.data[i++];
            if (delta == 0x80) {
                Parse stream(cmd.data + i);
                i += sizeof(uint16_t);

                delta = stream.in_uint16_le();
            }
            return delta;
        };

        RDPGlyphIndex new_cmd = cmd;
        gdi::GraphicCmdColor::encode_cmd_color(
            color_convertor{this->capture_bpp, color_ctx},
            new_cmd
        );
        bool has_delta_byte = (!new_cmd.ui_charinc && !(new_cmd.fl_accel & SO_CHAR_INC_EQUAL_BM_BASE));
        for (uint8_t i = 0; i < new_cmd.data_len; ) {
            if (new_cmd.data[i] <= 0xFD) {
                //LOG(LOG_INFO, "Index in the fragment cache=%u", new_cmd.data[i]);
                FontChar const & fc = gly_cache.glyphs[new_cmd.cache_id][new_cmd.data[i]].font_item;
                assert(fc);

                int cacheIndex;
                if (this->glyph_cache.add_glyph(fc, new_cmd.cache_id, cacheIndex) ==
                    GlyphCache::GLYPH_ADDED_TO_CACHE) {
                    this->emit_glyph_cache(new_cmd.cache_id, cacheIndex);
                }
                else if ((this->bmp_cache.owner == BmpCache::Recorder) &&
                         !this->glyph_cache.is_cached(new_cmd.cache_id, cacheIndex)) {
                    this->emit_glyph_cache(new_cmd.cache_id, cacheIndex);
                    this->glyph_cache.set_cached(new_cmd.cache_id, cacheIndex, true);
                }

                assert(cacheIndex >= 0);
                new_cmd.data[i++] = static_cast<uint8_t>(cacheIndex);
                if (has_delta_byte) {
                    const uint16_t delta = get_delta(new_cmd, i);

                    LOG_IF(bool(this->verbose & Verbose::primary_orders), LOG_INFO,
                        "RDPSerializer::draw(RDPGlyphIndex, ...): "
                        "Experimental support of "
                        "the distance between two consecutive glyphs "
                        "indicated by delta bytes in "
                        "GlyphIndex Primary Drawing Order. "
                        "delta=%u",
                        delta);
                }
            }
            else if (new_cmd.data[i] == 0xFE) {
                i++;

                const uint8_t fragment_index = new_cmd.data[i++];

                if (has_delta_byte) {
                    const uint16_t delta = get_delta(new_cmd, i);

                    LOG_IF(bool(this->verbose & Verbose::primary_orders), LOG_INFO,
                        "RDPSerializer::draw(RDPGlyphIndex, ...): "
                            "Experimental support of "
                            "the distance between two consecutive fragments "
                            "indicated by delta bytes in "
                            "GlyphIndex Primary Drawing Order. "
                            "delta=%u",
                        delta);
                }

                LOG_IF(bool(this->verbose & Verbose::primary_orders), LOG_INFO,
                    "RDPSerializer::draw(RDPGlyphIndex, ...): "
                        "Experimental support of USE (0xFE) operation byte in "
                        "GlyphIndex Primary Drawing Order. "
                        "fragment_index=%u",
                    fragment_index);
            }
            else if (new_cmd.data[i] == 0xFF) {
                i++;

                const uint8_t fragment_index = new_cmd.data[i++];
                const uint8_t fragment_size  = new_cmd.data[i++];

                LOG_IF(bool(this->verbose & Verbose::primary_orders), LOG_INFO,
                    "RDPSerializer::draw(RDPGlyphIndex, ...): "
                        "Experimental support of ADD (0xFF) operation byte in "
                        "GlyphIndex Primary Drawing Order. "
                        "fragment_index=%u fragment_size=%u",
                    fragment_index, fragment_size);

                assert(i == new_cmd.data_len);
            }
        }

        this->reserve_order(297);
        RDPOrderCommon newcommon(RDP::GLYPHINDEX, clip);
        new_cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.glyphindex);
        this->ssc.common = newcommon;
        this->ssc.glyphindex = new_cmd;
        if (bool(this->verbose & Verbose::primary_orders)) {
            new_cmd.log(LOG_INFO, this->ssc.common.clip);
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        this->reserve_order(cmd.size + 12);
        cmd.emit(this->stream_orders);
    }

    void draw(const RDPColCache & cmd) override {
        this->reserve_order(2000);
        cmd.emit(this->stream_orders);
    }

    void draw(RDPPolygonSC const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPPolygonSC const & cmd){
            this->reserve_order(256);
            RDPOrderCommon newcommon(RDP::POLYGONSC, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->polygonSC);
            this->ssc.common    = newcommon;
            this->polygonSC = cmd;
        });
    }

    void draw(RDPPolygonCB const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPPolygonCB const & cmd){
            this->reserve_order(256);
            RDPOrderCommon newcommon(RDP::POLYGONCB, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->polygonCB);
            this->ssc.common    = newcommon;
            this->polygonCB = cmd;
        });
    }

    void draw(RDPPolyline const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPPolyline const & cmd){
            this->reserve_order(256);
            RDPOrderCommon newcommon(RDP::POLYLINE, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.polyline);
            this->ssc.common   = newcommon;
            this->ssc.polyline = cmd;
            if (bool(this->verbose & Verbose::primary_orders)) {
                cmd.log(LOG_INFO, this->ssc.common.clip);
            }
        });
    }

    void draw(RDPEllipseSC const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPEllipseSC const & cmd){
            this->reserve_order(26);
            RDPOrderCommon newcommon(RDP::ELLIPSESC, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ssc.ellipseSC);
            this->ssc.common = newcommon;
            this->ssc.ellipseSC = cmd;
        });
    }

    void draw(RDPEllipseCB const & cmd_, Rect clip, gdi::ColorCtx color_ctx) override {
        this->reencode_cmd_color(cmd_, color_ctx, [&](RDPEllipseCB const & cmd){
            this->reserve_order(54);
            RDPOrderCommon newcommon(RDP::ELLIPSECB, clip);
            cmd.emit(this->stream_orders, newcommon, this->ssc.common, this->ellipseCB);
            this->ssc.common = newcommon;
            this->ellipseCB = cmd;
        });
    }

    void draw(const RDP::FrameMarker & order) override {
        this->reserve_order(5);
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {
        this->reserve_order(order.size());
        order.emit(this->stream_orders);
        if (bool(this->verbose & Verbose::secondary_orders)) {
            order.log(LOG_INFO);
        }
    }

    // check if the next bitmap will fit in available packet size
    // if not send previous bitmaps we got and init a new packet
    void reserve_bitmap(size_t asked_size) {
        size_t const max_packet_size = std::min(this->stream_bitmaps.get_capacity(), this->max_data_block_size);
        // TODO QuickFix, should set a max packet size according to RDP compression version of client, proxy and server
        size_t const used_size       = this->stream_bitmaps.get_offset();
        LOG_IF(bool(this->verbose & Verbose::internal_buffer), LOG_INFO
          , "<Serializer %p> RDPSerializer::reserve_bitmap[%zu](%zu) used=%zu free=%zu"
          , static_cast<void*>(this)
          , this->bitmap_count
          , asked_size
          , used_size
          , max_packet_size - used_size - SERIALIZER_HEADER_SIZE);

        if (asked_size + SERIALIZER_HEADER_SIZE > max_packet_size) {
            LOG( LOG_ERR
               , "(asked size (%zu) + HEADER_SIZE (%zu) = %zu) > image batch capacity (%zu)"
               , asked_size
               , SERIALIZER_HEADER_SIZE
               , asked_size + SERIALIZER_HEADER_SIZE
               , max_packet_size);
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }
        if (this->order_count) { this->flush_orders(); }
        const size_t max_image_batch = 4096;
        if (   (this->bitmap_count >= max_image_batch)
            || ((used_size + asked_size + SERIALIZER_HEADER_SIZE) > max_packet_size)) {
            this->flush_bitmaps();
        }
        this->bitmap_count++;
    }

    void draw( const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (bmp.has_data_compressed() && (bitmap_data.flags & BITMAP_COMPRESSION)) {
            auto data_compressed = bmp.data_compressed();
            this->reserve_bitmap(bitmap_data.struct_size() + data_compressed.size());

            bitmap_data.emit(this->stream_bitmaps);
            this->stream_bitmaps.out_copy_bytes(data_compressed.data(), data_compressed.size());
        }
        else {
            if (bitmap_data.flags & BITMAP_COMPRESSION) {
                RDPBitmapData bitmap_data_new = bitmap_data;

                bitmap_data_new.flags         &= ~(BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR); /*NOLINT*/
                bitmap_data_new.bitmap_length  = bmp.bmp_size();

                this->reserve_bitmap(bitmap_data_new.struct_size() + bmp.bmp_size());

                bitmap_data_new.emit(this->stream_bitmaps);
            }
            else {
                this->reserve_bitmap(bitmap_data.struct_size() + bmp.bmp_size());

                bitmap_data.emit(this->stream_bitmaps);
            }

            this->stream_bitmaps.out_copy_bytes(bmp.data(), bmp.bmp_size());
        }
        if (bool(this->verbose & Verbose::bitmap_update)) {
            bitmap_data.log(LOG_INFO);
        }
    }

    void set_pointer(uint16_t /*cache_idx*/, Pointer const& cursor, SetPointerMode mode) override
    {
        int cache_idx = 0;
        switch (this->pointer_cache.add_pointer(cursor, cache_idx)) {
        case POINTER_TO_SEND:
            this->send_pointer(cache_idx, cursor);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            if (this->bmp_cache.owner == BmpCache::Recorder
             && !this->pointer_cache.is_cached(cache_idx)
            ) {
                this->send_pointer(cache_idx, cursor);
                this->pointer_cache.set_cached(cache_idx, true);
                break;
            }
        }

        switch (mode) {
        case SetPointerMode::Cached:
        case SetPointerMode::Insert:
            this->cached_pointer_update(cache_idx);
            break;
        case SetPointerMode::New:
            break;
        }
    }

    // TODO set_palette unimplemented
    void set_palette(const BGRPalette& /*unused*/) override {}
};
