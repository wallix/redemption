/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#if !defined(__RDP_GRAPHIC_DEVICE_HPP__)
#define __RDP_GRAPHIC_DEVICE_HPP__

#include "config.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"

struct RDPGraphicDevice
{
    virtual void flush() = 0;
    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPScrBlt & cmd, const Rect &clip) = 0;
    virtual void draw(const RDPDestBlt & cmd, const Rect &clip) = 0;
    virtual void draw(const RDPPatBlt & cmd, const Rect &clip) = 0;
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPLineTo& cmd, const Rect & clip) = 0;
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) = 0;

    virtual void draw(const RDPBrushCache & cmd) = 0;
    virtual void draw(const RDPColCache & cmd) = 0;
    virtual void draw(const RDPBmpCache & cmd) = 0;
    virtual void draw(const RDPGlyphCache & cmd) = 0;

protected:
    // this to avoid calling constructor or destructor of base abstract class
    RDPGraphicDevice() {}
    // if necessary (need to destroy object through pointer of base class)
    // we may also chose to make destructor virtual
    ~RDPGraphicDevice() {}
};

struct RDPUnserializer
{
    Stream stream;
    RDPGraphicDevice * consumer;
    Transport * trans;
    Rect screen_rect;

    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;

    // variables used to read batch of orders "chunks"
    uint16_t chunk_num;
    uint16_t chunk_size;
    uint16_t chunk_type;
    uint16_t remaining_order_count;
    uint16_t order_count;

    RDPUnserializer(Transport * trans, RDPGraphicDevice * consumer, const Rect screen_rect)
     : stream(4096), consumer(consumer), trans(trans), screen_rect(screen_rect),
     // Internal state of orders
    common(RDP::PATBLT, Rect(0, 0, 1, 1)),
    destblt(Rect(), 0),
    patblt(Rect(), 0, 0, 0, RDPBrush()),
    scrblt(Rect(), 0, 0, 0),
    opaquerect(Rect(), 0),
    memblt(0, Rect(), 0, 0, 0, 0),
    lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
    glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),

    // variables used to read batch of orders "chunks"
    chunk_num(0),
    chunk_size(0),
    chunk_type(0),
    remaining_order_count(0),
    order_count(0)
    {
    }

    uint8_t next(){
        if (((this->stream.p == this->stream.end) && (this->remaining_order_count))
        ||  ((this->stream.p != this->stream.end) && (this->remaining_order_count == 0))){
            LOG(LOG_ERR, "Incomplete order batch at chunk %u "
                         "order [%u/%u] "
                         "remaining [%u/%u]",
                         this->chunk_num,
                         (this->order_count-this->remaining_order_count), this->order_count,
                         (this->stream.end - this->stream.p), this->chunk_size);
        }
        if (!this->remaining_order_count){
            try {
                this->stream.init(4096);
                this->trans->recv(&this->stream.end, 8);
                this->chunk_type = this->stream.in_uint16_le();
                this->chunk_size = this->stream.in_uint16_le();
                this->remaining_order_count = this->order_count = this->stream.in_uint16_le();
                uint16_t pad = this->stream.in_uint16_le(); (void)pad;
            }
            catch (Error & e){
                TODO(" check specific error and return 0 only if actual EOF is reached or rethrow the error")
                return (e.id == ERR_TRANSPORT_READ_FAILED)?0:1;
            }
            this->trans->recv(&this->stream.end, this->chunk_size - 8);
        }
        switch (this->chunk_type){
        case RDP_UPDATE_ORDERS:
        {
            uint8_t control = this->stream.in_uint8();
            this->remaining_order_count--;
            if (!control & RDP::STANDARD){
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                TODO(" throw some error")
            }
            else if (control & RDP::SECONDARY) {
                using namespace RDP;
                RDPSecondaryOrderHeader header(this->stream);
                    uint8_t *next_order = this->stream.p + header.length + 7;
                    switch (header.type) {
                    case TS_CACHE_BITMAP_COMPRESSED:
                    case TS_CACHE_BITMAP_UNCOMPRESSED:
                    {
                        // we need color depth and palette
                        RDPBmpCache cmd(24);
                        BGRPalette palette;
                        init_palette332(palette);
                        cmd.receive(this->stream, control, header, palette);
                        consumer->draw(cmd);
                    }
                    break;
                    case TS_CACHE_COLOR_TABLE:
                        LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_COLOR_TABLE (%d)", header.type);
    //                    this->process_colormap(this->stream, control, header, mod);
                        break;
                    case TS_CACHE_GLYPH:
                        LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_GLYPH (%d)", header.type);
    //                    this->rdp_orders_process_fontcache(this->stream, header.flags, mod);
                        break;
                    case TS_CACHE_BITMAP_COMPRESSED_REV2:
                        LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)", header.type);
                      break;
                    case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                        LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)", header.type);
                      break;
                    case TS_CACHE_BITMAP_COMPRESSED_REV3:
                        LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)", header.type);
                      break;
                    default:
                        LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                        /* error, unknown order */
                        break;
                    }
                    stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(this->stream, control);
                const Rect & clip = (control & RDP::BOUNDS)?this->common.clip:this->screen_rect;
                switch (this->common.order) {
                case RDP::GLYPHINDEX:
                    this->glyphindex.receive(this->stream, header);
                    consumer->draw(this->glyphindex, clip);
                    break;
                case RDP::DESTBLT:
                    this->destblt.receive(this->stream, header);
                    consumer->draw(this->destblt, clip);
                    break;
                case RDP::PATBLT:
                    this->patblt.receive(this->stream, header);
                    consumer->draw(this->patblt, clip);
                    break;
                case RDP::SCREENBLT:
                    this->scrblt.receive(this->stream, header);
                    consumer->draw(this->scrblt, clip);
                    break;
                case RDP::LINE:
                    this->lineto.receive(this->stream, header);
                    consumer->draw(this->lineto, clip);
                    break;
                case RDP::RECT:
                    this->opaquerect.receive(this->stream, header);
                    consumer->draw(this->opaquerect, clip);
                    break;
                case RDP::MEMBLT:
                    this->memblt.receive(this->stream, header);
                    this->consumer->draw(this->memblt, clip);
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
            }
            }
            break;
            default:
                this->remaining_order_count = 0;
            break;
        }
        return 1;
    }
};

struct RDPSerializer : public RDPGraphicDevice
{
    Stream stream;
    Transport * trans;
    const Inifile * ini;
    const int bitmap_cache_version;
    const int use_bitmap_comp;
    const int op2;

    // Internal state of orders
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyphindex;
    // state variables for gathering batch of orders
    size_t order_count;
    uint32_t offset_order_count;


    RDPSerializer(Transport * trans, const Inifile * ini,
          const int bitmap_cache_version,
          const int use_bitmap_comp,
          const int op2)
        : stream(32768),
        trans(trans),
        ini(ini),
        bitmap_cache_version(bitmap_cache_version),
        use_bitmap_comp(use_bitmap_comp),
        op2(op2),
        // Internal state of orders
        common(RDP::PATBLT, Rect(0, 0, 1, 1)),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        scrblt(Rect(), 0, 0, 0),
        opaquerect(Rect(), 0),
        memblt(0, Rect(), 0, 0, 0, 0),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        // state variables for a batch of orders
        order_count(0),
        offset_order_count(0)
     {}
    ~RDPSerializer() {}
    virtual void flush() = 0;

    /*****************************************************************************/
    // check if the next order will fit in available packet size
    // if not send previous orders we got and init a new packet
    void reserve_order(size_t asked_size)
    {
        if (this->ini && this->ini->globals.debug.primary_orders){
            LOG(LOG_INFO, "GraphicsUpdatePDU::reserve_order[%u](%u) remains=%u", this->order_count, asked_size, std::min(this->stream.capacity, (size_t)4096) - this->stream.get_offset(0));
        }
        size_t max_packet_size = std::min(this->stream.capacity, (size_t)4096);
        size_t used_size = this->stream.get_offset(0);
        const size_t max_order_batch = 4096;
        if ((this->order_count >= max_order_batch)
        || (used_size + asked_size + 100) > max_packet_size) {
            this->flush();
        }
        this->order_count++;
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->reserve_order(23);
        RDPOrderCommon newcommon(RDP::RECT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->reserve_order(25);
        RDPOrderCommon newcommon(RDP::SCREENBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->reserve_order(21);
        RDPOrderCommon newcommon(RDP::DESTBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->reserve_order(29);
        using namespace RDP;
        RDPOrderCommon newcommon(RDP::PATBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }


    virtual void draw(const RDPMemBlt & cmd, const Rect & clip)
    {
        this->reserve_order(30);
        RDPOrderCommon newcommon(RDP::MEMBLT, clip);
        cmd.emit(this->stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->reserve_order(32);
        RDPOrderCommon newcommon(RDP::LINE, clip);
        cmd.emit(this->stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->reserve_order(297);
        RDPOrderCommon newcommon(RDP::GLYPHINDEX, clip);
        cmd.emit(this->stream, newcommon, this->common, this->glyphindex);
        this->common = newcommon;
        this->glyphindex = cmd;
        if (this->ini && this->ini->globals.debug.primary_orders){
            LOG(LOG_INFO, "glyphIndex");
//            cmd.log(LOG_INFO, common.clip);
        }
    }

    virtual void draw(const RDPBrushCache & cmd)
    {
        this->reserve_order(cmd.size + 12);
        cmd.emit(this->stream);
        if (this->ini && this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

    virtual void draw(const RDPColCache & cmd)
    {
        this->reserve_order(2000);
        cmd.emit(this->stream);
        if (this->ini && this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

    virtual void draw(const RDPBmpCache & cmd)
    {
        this->reserve_order(cmd.bmp->bmp_size(cmd.bpp) + 16);
        cmd.emit(this->stream, this->bitmap_cache_version, this->use_bitmap_comp, this->op2);
        if (this->ini && this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

    virtual void draw(const RDPGlyphCache & cmd)
    {
        TODO(" compute actual size  instead of a majoration as below")
        this->reserve_order(1000);
        cmd.emit(this->stream);
        if (this->ini && this->ini->globals.debug.secondary_orders){
            cmd.log(LOG_INFO);
        }
    }

};


#endif
