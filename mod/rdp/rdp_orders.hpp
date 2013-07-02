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
   Author(s): Christophe Grosjean

   rdp module process orders
*/

#ifndef _REDEMPTION_MOD_RDP_RDP_ORDERS_HPP_
#define _REDEMPTION_MOD_RDP_RDP_ORDERS_HPP__

#include <string.h>

#include "log.hpp"
#include "stream.hpp"
#include "mod_api.hpp"

#include "RDP/protocol.hpp"

//#include "RDP/orders/RDPOrdersNames.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

/* orders */
struct rdp_orders {
    // State
    RDPOrderCommon common;
    RDPMemBlt      memblt;
    RDPMem3Blt     mem3blt;
    RDPOpaqueRect  opaquerect;
    RDPScrBlt      scrblt;
    RDPDestBlt     destblt;
    RDPPatBlt      patblt;
    RDPLineTo      lineto;
    RDPGlyphIndex  glyph_index;

    BGRPalette cache_colormap[6];
    BGRPalette global_palette;
    BGRPalette memblt_palette;

    TODO("CGR: this cache_bitmap here looks strange. At least it's size should be negotiated. And why is it not managed by the other cache management code ? This probably hide some kind of problem. See when working on cache secondary order primitives.")
    const Bitmap * cache_bitmap[3][10000];

    uint32_t verbose;

    rdp_orders(uint32_t verbose)
            : common(RDP::PATBLT, Rect(0, 0, 1, 1))
            , memblt(0, Rect(), 0, 0, 0, 0)
            , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
            , opaquerect(Rect(), 0)
            , scrblt(Rect(), 0, 0, 0)
            , destblt(Rect(), 0)
            , patblt(Rect(), 0, 0, 0, RDPBrush())
            , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
            , glyph_index( 0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0
                         , (uint8_t *)"")
            , verbose(verbose) {
        memset(this->cache_bitmap,   0, sizeof(this->cache_bitmap));
        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
        memset(this->memblt_palette, 0, sizeof(this->memblt_palette));
    }

    void reset()
    {
        this->common      = RDPOrderCommon(RDP::PATBLT, Rect(0, 0, 1, 1));
        this->memblt      = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        this->mem3blt     = RDPMem3Blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0);
        this->opaquerect  = RDPOpaqueRect(Rect(), 0);
        this->scrblt      = RDPScrBlt(Rect(), 0, 0, 0);
        this->destblt     = RDPDestBlt(Rect(), 0);
        this->patblt      = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        this->lineto      = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        this->glyph_index = RDPGlyphIndex( 0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1)
                                         , RDPBrush(), 0, 0, 0, (uint8_t *)"");

        this->destory_cache_bitmap();

        memset(this->cache_bitmap, 0, sizeof(this->cache_bitmap));
        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
        memset(this->memblt_palette, 0, sizeof(this->memblt_palette));
    }

    ~rdp_orders() {
        this->destory_cache_bitmap();
    }

private:
    void destory_cache_bitmap() {
        for (uint8_t cid = 0; cid < 3; cid++) {
            for (uint16_t cidx = 0 ; cidx < 10000; cidx++) {
                if (this->cache_bitmap[cid][cidx]) {
                    delete this->cache_bitmap[cid][cidx];
                }
            }
        }
    }

public:
    void process_bmpcache( uint8_t bpp, Stream & stream, const uint8_t control
                         , const RDPSecondaryOrderHeader & header)
    {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "rdp_orders_process_bmpcache bpp=%u", bpp);
        }
        RDPBmpCache bmp;
        bmp.receive(stream, control, header, this->global_palette);

        TODO("CGR: add cache_id, cache_idx range check, and also size check based on cache size by type and uncompressed bitmap size")
        if (this->cache_bitmap[bmp.id][bmp.idx]) {
            delete this->cache_bitmap[bmp.id][bmp.idx];
        }
        this->cache_bitmap[bmp.id][bmp.idx] = bmp.bmp;
        if (this->verbose & 64) {
            LOG( LOG_ERR
               , "rdp_orders_process_bmpcache bitmap id=%u idx=%u cx=%u cy=%u bmp_size=%u original_bpp=%u bpp=%u"
               , bmp.id, bmp.idx, bmp.bmp->cx, bmp.bmp->cy, bmp.bmp->bmp_size, bmp.bmp->original_bpp, bpp);
        }
    }

    void server_add_char( int font, int character
                        , int offset, int baseline
                        , int width, int height, const uint8_t * data
                        , mod_api * mod) {
        struct FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data, data, fi.datasize());

        RDPGlyphCache cmd(font, 1, character, fi.offset, fi.baseline, fi.width, fi.height, fi.data);
        mod->draw(cmd);
    }

    void process_fontcache(Stream & stream, int flags, mod_api * mod) {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "rdp_orders_process_fontcache");
        }
        int font    = stream.in_uint8();
        int nglyphs = stream.in_uint8();
        for (int i = 0; i < nglyphs; i++) {
            int character = stream.in_uint16_le();
            int offset    = stream.in_uint16_le();
            int baseline  = stream.in_uint16_le();
            int width     = stream.in_uint16_le();
            int height    = stream.in_uint16_le();

            int             datasize = (height * nbbytes(width) + 3) & ~3;
            const uint8_t * data     = stream.in_uint8p(datasize);

            this->server_add_char(font, character, offset, baseline, width, height, data, mod);
        }
        if (this->verbose & 64) {
            LOG(LOG_INFO, "rdp_orders_process_fontcache done");
        }
    }

    void process_colormap( Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header
                         , mod_api * mod) {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "process_colormap");
        }
        RDPColCache colormap;
        colormap.receive(stream, control, header);
        memcpy(this->cache_colormap[colormap.cacheIndex], &colormap.palette, sizeof(BGRPalette));
        RDPColCache cmd(colormap.cacheIndex, colormap.palette);
        mod->draw(cmd);

        if (this->verbose & 64) {
            LOG(LOG_INFO, "process_colormap done");
        }
    }

    /*****************************************************************************/
    int process_orders(uint8_t bpp, Stream & stream, bool fast_path, mod_api * mod) {
        if (this->verbose & 64) {
            LOG(LOG_INFO, "process_orders bpp=%u", bpp);
        }

        using namespace RDP;

        OrdersUpdate_Recv orders_update(stream, fast_path);

        int processed = 0;
        while (processed < orders_update.number_orders) {
            DrawingOrder_RecvFactory drawing_order(stream);

            if (!drawing_order.control_flags & STANDARD) {
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                break;
            }
            if (drawing_order.control_flags & SECONDARY) {
                RDPSecondaryOrderHeader header(stream);
//                LOG(LOG_INFO, "secondary order=%d", header.type);
                uint8_t * next_order = stream.p + header.order_data_length();
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                    this->process_bmpcache(bpp, stream, drawing_order.control_flags, header);
                    break;
                case TS_CACHE_COLOR_TABLE:
                    this->process_colormap(stream, drawing_order.control_flags, header, mod);
                    break;
                case TS_CACHE_GLYPH:
                    this->process_fontcache(stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG( LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)"
                       , header.type);
                  break;
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG( LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)"
                       , header.type);
                  break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG( LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)"
                       , header.type);
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(stream, drawing_order.control_flags);
                const Rect & cmd_clip = ( (drawing_order.control_flags & BOUNDS)
                                        ? this->common.clip
                                        : Rect(0, 0, mod->front_width, mod->front_height)
                                        );
//                LOG(LOG_INFO, "/* order=%d ordername=%s */", this->common.order, ordernames[this->common.order]);
                switch (this->common.order) {
                case GLYPHINDEX:
                    this->glyph_index.receive(stream, header);
                    mod->draw(this->glyph_index, cmd_clip);
                    break;
                case DESTBLT:
                    this->destblt.receive(stream, header);
                    mod->draw(this->destblt, cmd_clip);
                    break;
                case PATBLT:
                    this->patblt.receive(stream, header);
                    mod->draw(this->patblt, cmd_clip);
                    break;
                case SCREENBLT:
                    this->scrblt.receive(stream, header);
                    mod->draw(this->scrblt, cmd_clip);
                    break;
                case LINE:
                    this->lineto.receive(stream, header);
                    mod->draw(this->lineto, cmd_clip);
                    break;
                case RECT:
                    this->opaquerect.receive(stream, header);
                    mod->draw(this->opaquerect, cmd_clip);
                    break;
                case MEMBLT:
                    this->memblt.receive(stream, header);
                    {
                        if ((this->memblt.cache_id >> 8) >= 6) {
                            LOG( LOG_INFO, "colormap out of range in memblt:%x"
                               , (this->memblt.cache_id >> 8));
                            this->memblt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap * bitmap =
                            this->cache_bitmap[this->memblt.cache_id & 0x3][this->memblt.cache_idx];
                        TODO("CGR: check if bitmap has the right palette...")
                        TODO("CGR: 8 bits palettes should probabily be transmitted to front, not stored in bitmaps")
                        if (bitmap) {
                            mod->draw(this->memblt, cmd_clip, *bitmap);
                        }
                    }
                    break;
                case MEM3BLT:
                    this->mem3blt.receive(stream, header);
                    {
                        if ((this->mem3blt.cache_id >> 8) >= 6){
                            LOG( LOG_INFO, "colormap out of range in mem3blt: %x"
                               , (this->mem3blt.cache_id >> 8));
                            this->mem3blt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap * bitmap =
                            this->cache_bitmap[this->mem3blt.cache_id & 0x3][this->mem3blt.cache_idx];
                        TODO("CGR: check if bitmap has the right palette...")
                        TODO("CGR: 8 bits palettes should probabily be transmitted to front, not stored in bitmaps")
                        if (bitmap) {
                            mod->draw(this->mem3blt, cmd_clip, *bitmap);
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
            }
            processed++;
        }
        if (this->verbose & 64){
            LOG(LOG_INFO, "process_orders done");
        }
        return 0;
    }   // int process_orders(uint8_t bpp, Stream & stream, bool fast_path, mod_api * mod)
};

#endif
