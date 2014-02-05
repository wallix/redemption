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

   RDPSerializer is an implementation of RDPGraphicDevice that know how to serialize RDP Orders
   and send them on the wire or store them in a file (actual storage will be provided as a Transport class).
   Serialized RDP orders are put in a chunk and sent when flush is called (either explicit call or because
   the provided buffer is full).
*/

#ifndef _REDEMPTION_CORE_RDP_RDPSERIALIZER_HPP_
#define _REDEMPTION_CORE_RDP_RDPSERIALIZER_HPP_

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

#include "RDP/RDPGraphicDevice.hpp"

#include "transport.hpp"

#include "RDP/caches/bmpcache.hpp"
#include "difftimeval.hpp"
#include "stream.hpp"
#include "rect.hpp"
#include "colors.hpp"

enum {
    BREAKPOINT          = 1005,
    META_FILE           = 1006,
    NEXT_FILE_ID        = 1007,
    TIMESTAMP           = 1008,
    POINTER             = 1009,
    LAST_IMAGE_CHUNK    = 0x1000,   // 4096
    PARTIAL_IMAGE_CHUNK = 0x1001,   // 4097
    SAVE_STATE          = 0x1002,   // 4098
};

struct RDPSerializer : public RDPGraphicDevice
{
    using RDPGraphicDevice::draw;

    Stream & stream_orders;
    Stream & stream_bitmaps;

    uint8_t bpp;

    Transport * trans;
    const Inifile & ini;
    const int bitmap_cache_version;
    const int use_bitmap_comp;
    const int op2;

    // Internal state of orders
    RDPOrderCommon     common;
    RDPDestBlt         destblt;
    RDPMultiDstBlt     multidstblt;
    RDPMultiOpaqueRect multiopaquerect;
    RDPPatBlt          patblt;
    RDPScrBlt          scrblt;
    RDPOpaqueRect      opaquerect;
    RDPMemBlt          memblt;
    RDPMem3Blt         mem3blt;
    RDPLineTo          lineto;
    RDPGlyphIndex      glyphindex;
    RDPPolygonSC       polygonSC;
    RDPPolygonCB       polygonCB;
    RDPPolyline        polyline;
    RDPEllipseSC       ellipseSC;
    RDPEllipseCB       ellipseCB;

    // state variables for gathering batch of orders
    size_t order_count;
    size_t chunk_flags;
    BmpCache & bmp_cache;

    size_t bitmap_count;

    uint32_t verbose;

    RDPSerializer( Transport * trans
                 , Stream & stream_orders
                 , Stream & stream_bitmaps
                 , const uint8_t bpp
                 , BmpCache & bmp_cache
                 , const int bitmap_cache_version
                 , const int use_bitmap_comp
                 , const int op2
                 , const Inifile & ini
                 , uint32_t verbose = 0)
    : RDPGraphicDevice()
    , stream_orders(stream_orders)
    , stream_bitmaps(stream_bitmaps)
    , bpp(bpp)
    , trans(trans)
    , ini(ini)
    , bitmap_cache_version(bitmap_cache_version)
    , use_bitmap_comp(use_bitmap_comp)
    , op2(op2)
    // Internal state of orders
    , common(RDP::PATBLT, Rect(0, 0, 1, 1))
    , destblt(Rect(), 0)
    , multidstblt()
    , patblt(Rect(), 0, 0, 0, RDPBrush())
    , scrblt(Rect(), 0, 0, 0)
    , opaquerect(Rect(), 0)
    , memblt(0, Rect(), 0, 0, 0, 0)
    , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
    , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
    , glyphindex( 0, 0, 0, 0, 0, 0
                , Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t *)"")
    , polygonSC()
    , polygonCB()
    , polyline()
    , ellipseSC()
    , ellipseCB(Rect(), 0, 0, 0, 0, RDPBrush())
    // state variables for a batch of orders
    , order_count(0)
    , bmp_cache(bmp_cache)
    , bitmap_count(0)
    , verbose(verbose) {}

    ~RDPSerializer() {}

protected:
    virtual void flush_orders() = 0;
    virtual void flush_bitmaps() = 0;
public:

    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    void reserve_order(size_t asked_size)
    {
        //LOG(LOG_INFO, "RDPSerializer::reserve_order %u (avail=%u)", asked_size, this->stream_orders.size());
        // To support 64x64 32-bit bitmap.
        size_t max_packet_size = std::min(this->stream_orders.get_capacity(), static_cast<size_t>(16384 + 256));
        size_t used_size = this->stream_orders.get_offset();
        if (this->ini.debug.primary_orders > 3){
            LOG( LOG_INFO
               , "<Serializer %p> RDPSerializer::reserve_order[%u](%u) used=%u free=%u"
               , this
               , this->order_count
               , asked_size, used_size
               , max_packet_size - used_size - 106
               );
        }
        if (asked_size + 106 > max_packet_size){
            LOG( LOG_ERR
               , "(asked size (%u) + 106 = %d) > order batch capacity (%u)"
               , asked_size
               , asked_size + 106
               , max_packet_size);
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }
        const size_t max_order_batch = 4096;
        if (   (this->order_count >= max_order_batch)
            || ((used_size + asked_size + 106) > max_packet_size)) {
            this->flush_orders();
        }
        this->order_count++;
        //LOG(LOG_INFO, "RDPSerializer::reserve_order done");
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        //LOG(LOG_INFO, "RDPSerializer::draw::RDPOpaqueRect");
        this->reserve_order(23);
        RDPOrderCommon newcommon(RDP::RECT, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;

        if (this->ini.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
        //LOG(LOG_INFO, "RDPSerializer::draw::RDPOpaqueRect done");
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->reserve_order(25);
        RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
        if (this->ini.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->reserve_order(21);
        RDPOrderCommon newcommon(RDP::DESTBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
        if (this->ini.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
        this->reserve_order(395 * 2);
        RDPOrderCommon newcommon(RDP::MULTIDSTBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->multidstblt);
        this->common      = newcommon;
        this->multidstblt = cmd;
        if (this->ini.debug.primary_orders) {
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
        this->reserve_order(397 * 2);
        RDPOrderCommon newcommon(RDP::MULTIOPAQUERECT, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->multiopaquerect);
        this->common          = newcommon;
        this->multiopaquerect = cmd;
        if (this->ini.debug.primary_orders) {
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->reserve_order(29);
        using namespace RDP;
        RDPOrderCommon newcommon(RDP::PATBLT, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
        if (this->ini.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    void emit_bmp_cache(uint8_t cache_id, uint16_t cache_idx)
    {
        const Bitmap * bmp = this->bmp_cache.get(cache_id, cache_idx);
        if (!bmp) {
//            LOG(LOG_INFO, "skipping RDPSerializer::emit_bmp_cache for %u:%u (entry not used)",
//                cache_id, cache_idx);
            return;
        }
        RDPBmpCache cmd_cache(bmp, cache_id, cache_idx, this->ini.debug.primary_orders);
        this->reserve_order(cmd_cache.bmp->bmp_size + 16);
        cmd_cache.emit( this->stream_orders, this->bitmap_cache_version, this->use_bitmap_comp
                      , this->op2);

        if (this->ini.debug.secondary_orders){
            cmd_cache.log(LOG_INFO);
        }
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & oldbmp)
    {
        uint32_t res = this->bmp_cache.cache_bitmap(oldbmp);
        uint8_t cache_id = (res >> 16) & 0x3;
        uint16_t cache_idx = res;

        if ((res >> 24) == BITMAP_ADDED_TO_CACHE){
            this->emit_bmp_cache(cache_id, cache_idx);
        }

        RDPMemBlt newcmd = cmd;
        newcmd.cache_id = cache_id;
        newcmd.cache_idx = cache_idx;

        this->reserve_order(30);
        RDPOrderCommon newcommon(RDP::MEMBLT, clip);
        newcmd.emit(this->stream_orders, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = newcmd;
        if (this->ini.debug.primary_orders){
            newcmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & oldbmp) {
        uint32_t res = this->bmp_cache.cache_bitmap(oldbmp);
        uint8_t cache_id = (res >> 16) & 0x3;
        uint16_t cache_idx = res;

        if ((res >> 24) == BITMAP_ADDED_TO_CACHE){
            this->emit_bmp_cache(cache_id, cache_idx);
        }

        RDPMem3Blt newcmd = cmd;
        newcmd.cache_id = cache_id;
        newcmd.cache_idx = cache_idx;

        this->reserve_order(60);
        RDPOrderCommon newcommon(RDP::MEM3BLT, clip);
        newcmd.emit(this->stream_orders, newcommon, this->common, this->mem3blt);
        this->common = newcommon;
        this->mem3blt = newcmd;
        if (this->ini.debug.primary_orders){
            newcmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->reserve_order(32);
        RDPOrderCommon newcommon(RDP::LINE, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip,
        const GlyphCache * gly_cache)
    {
        this->reserve_order(297);
        RDPOrderCommon newcommon(RDP::GLYPHINDEX, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->glyphindex);
        this->common = newcommon;
        this->glyphindex = cmd;
    }

    virtual void draw(const RDPBrushCache & cmd)
    {
        this->reserve_order(cmd.size + 12);
        cmd.emit(this->stream_orders);
    }

    virtual void draw(const RDPColCache & cmd)
    {
        this->reserve_order(2000);
        cmd.emit(this->stream_orders);
    }

    virtual void draw(const RDPGlyphCache & cmd)
    {
        this->reserve_order(cmd.total_order_size() + 16 /* majoration */);
        cmd.emit(this->stream_orders);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip) {
        this->reserve_order(256);
        RDPOrderCommon newcommon(RDP::POLYGONSC, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->polygonSC);
        this->common    = newcommon;
        this->polygonSC = cmd;
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        this->reserve_order(256);
        RDPOrderCommon newcommon(RDP::POLYGONCB, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->polygonCB);
        this->common    = newcommon;
        this->polygonCB = cmd;
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip) {
        this->reserve_order(256);
        RDPOrderCommon newcommon(RDP::POLYLINE, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->polyline);
        this->common   = newcommon;
        this->polyline = cmd;
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip)
    {
        this->reserve_order(26);
        RDPOrderCommon newcommon(RDP::ELLIPSESC, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->ellipseSC);
        this->common = newcommon;
        this->ellipseSC = cmd;
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip)
    {
        this->reserve_order(54);
        RDPOrderCommon newcommon(RDP::ELLIPSECB, clip);
        cmd.emit(this->stream_orders, newcommon, this->common, this->ellipseCB);
        this->common = newcommon;
        this->ellipseCB = cmd;
    }


    // check if the next bitmap will fit in available packet size
    // if not send previous bitmaps we got and init a new packet
    void reserve_bitmap(size_t asked_size) {
        size_t max_packet_size = std::min(this->stream_bitmaps.get_capacity(), static_cast<size_t>(8192));
        size_t used_size       = this->stream_bitmaps.get_offset();
        if (this->ini.debug.primary_orders > 3) {
            LOG( LOG_INFO
               , "<Serializer %p> RDPSerializer::reserve_bitmap[%u](%u) used=%u free=%u"
               , this
               , this->bitmap_count
               , asked_size
               , used_size
               , max_packet_size - used_size - 106
               );
        }
        if (asked_size + 106 > max_packet_size) {
            LOG( LOG_ERR
               , "asked size (%u) > image batch capacity (%u)"
               , asked_size + 106
               , max_packet_size
               );
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }
        const size_t max_image_batch = 4096;
        if (   (this->bitmap_count >= max_image_batch)
            || ((used_size + asked_size + 106) > max_packet_size)) {
            this->flush_bitmaps();
        }
        this->bitmap_count++;
    }

    virtual void draw( const RDPBitmapData & bitmap_data, const uint8_t * data
                     , size_t size, const Bitmap & bmp) {
        this->reserve_bitmap(bitmap_data.struct_size() + size);

        bitmap_data.emit(this->stream_bitmaps);
        this->stream_bitmaps.out_copy_bytes(data, size);
    }
};

#endif
