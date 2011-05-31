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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rdp orders header at rdp module

*/


#if !defined(__RDP_ORDERS_HPP__)
#define __RDP_ORDERS_HPP__

#include <string.h>
#include <assert.h>
#include "constants.hpp"

#include "stream.hpp"
#include "client_mod.hpp"
#include "NewRDPOrders.hpp"
#include "log.hpp"

struct rdp_cursor {
    int x;
    int y;
    int width;
    int height;
    uint8_t mask[(32 * 32) / 8];
    uint8_t data[(32 * 32) * 3];
    rdp_cursor() {
        this->x = 0;
        this->y = 0;
        this->width = 0;
        this->height = 0;
        memset(this->mask, 0, (32 * 32) / 8);
        memset(this->data, 0, (32 * 32) * 3);
    }
};

struct rdp_orders_state {
    /* desksave order state */
    int desksave_offset;
    int desksave_left;
    int desksave_top;
    int desksave_right;
    int desksave_bottom;
    int desksave_action;
    /* polyline order state */
    int polyline_x;
    int polyline_y;
    int polyline_opcode;
    int polyline_fgcolor;
    int polyline_lines;
    int polyline_datasize;
    char polyline_data[256];

    rdp_orders_state()

    {
        /* desksave order state */
        this->desksave_offset = 0;
        this->desksave_left = 0;
        this->desksave_top = 0;
        this->desksave_right = 0;
        this->desksave_bottom = 0;
        this->desksave_action = 0;
        /* polyline order state */
        this->polyline_x = 0;
        this->polyline_y = 0;
        this->polyline_opcode = 0;
        this->polyline_fgcolor = 0;
        this->polyline_lines = 0;
        this->polyline_datasize = 0;
        memset(this->polyline_data, 0, 256);
    }
};


/* orders */
struct rdp_orders {
    // State
    RDPOrderCommon common;
    RDPMemBlt memblt;
    RDPOpaqueRect opaquerect;
    RDPScrBlt scrblt;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyph_index;

    /* order state */
    struct rdp_orders_state state;

    #warning look again details for cache_colormap, see comment.
    // currently when use palette 0 of cache_colormap to store the global palette
    // used for 8 bits colors in rdp drawing orders other than memblt and mem3blt
    // like OpaqueRect, or PATBlt it is not clear we should do that or use a
    // separate palette as was done before. RDP Documentation is not clear on
    // this subject (or probably I haven't found the relevant part).
    // If necessary this is easy to change, just track down accesses to
    // cache_colormap.palette[0]
    // also cache_colormap currently stores bpp, it may not be such a good idea.
    // the only real relationship is that palette is relevant only in 8bpp mode
    // mode it's probably not enough to store both in the same structure.
    struct RDPColCache cache_colormap;
    #warning this cache_bitmap here looks strange. At least it's size should ne negotiated. And why is it not managed by the other cache management code ? This probably hide some kind of problem. See when working on cache secondary order primitives.
    struct Bitmap * cache_bitmap[3][10000];

    #warning it looks strange that rdp_orders object should be depending on bpp parameter, it looks more like a cache implementation detail that should be abstracted here.
    rdp_orders() :
        common(0, Rect()),
        memblt(0, Rect(), 0, 0, 0, 0),
        opaquerect(Rect(), 0),
        scrblt(Rect(), 0, 0, 0),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyph_index(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        cache_colormap()
    {
        memset(this->cache_bitmap, 0, sizeof(this->cache_bitmap));
    }


    ~rdp_orders(){
    }

    #warning smells like code duplication, it would probably be better to destroy rdp_orders object and recreate it instead of calling that reset_state.
    void rdp_orders_reset_state()
    {
        LOG(LOG_INFO, "rdp orders: reset state");
        using namespace RDP;

        memset(&this->state, 0, sizeof(this->state));
        common = RDPOrderCommon(0, Rect());
        memblt = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        opaquerect = RDPOpaqueRect(Rect(), 0);
        scrblt = RDPScrBlt(Rect(), 0, 0, 0);
        destblt = RDPDestBlt(Rect(), 0);
        patblt = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        lineto = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        glyph_index = RDPGlyphIndex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"");

        common.order = PATBLT;
    }


    void rdp_orders_process_bmpcache(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
//        LOG(LOG_INFO, "rdp_orders_process_bmpcache");
        struct Bitmap* bitmap = NULL;
        uint8_t cache_id = 0;
        uint16_t cache_idx = 0;
        switch (header.type){
        case RDP::TS_CACHE_BITMAP_UNCOMPRESSED:
            {
                #warning RDPBmpCache is used to create bitmap
                RDPBmpCache bmp;
                bmp.receive(stream, control, header);
                cache_id = bmp.cache_id;
                cache_idx = bmp.cache_idx;
                bitmap = bmp.bmp;
            }
        break;
        case RDP::TS_CACHE_BITMAP_COMPRESSED:
            {
                #warning move that to RDPBmpCache -> receive
                int flags = header.flags;
                int size = 0;
                int pad2 = 0;
                size_t row_size = 0;
                int final_size = 0;

                cache_id = stream.in_uint8();
                int pad1 = stream.in_uint8();
                pad1 = pad1; // just to remove warning, will be optimized away
                uint8_t width = stream.in_uint8();
                uint8_t height = stream.in_uint8();
                uint8_t bpp = stream.in_uint8();
                int bufsize = stream.in_uint16_le();
                cache_idx = stream.in_uint16_le();
                if (flags & 0x400) {
                    size = bufsize;
                } else {
                    pad2 = stream.in_uint16_le();
                    size = stream.in_uint16_le();
                    row_size = stream.in_uint16_le();
                    final_size = stream.in_uint16_le();
                }

                const uint8_t* data = stream.in_uint8p(size);

                #warning valgrind say there is a memory leak here
                bitmap = new Bitmap(bpp, width, height);

                assert(row_size == bitmap->line_size);

                bitmap->decompress(data, size);
            }
        break;
        default:
            assert(false);
        }

        assert(bitmap);

        if (this->cache_bitmap[cache_id][cache_idx]) {
            delete this->cache_bitmap[cache_id][cache_idx];
        }
        this->cache_bitmap[cache_id][cache_idx] = bitmap;
    }

    void rdp_orders_process_fontcache(Stream & stream, int flags, client_mod * mod)
    {
//        LOG(LOG_INFO, "rdp_orders_process_fontcache");
        int font = stream.in_uint8();
        int nglyphs = stream.in_uint8();
        for (int i = 0; i < nglyphs; i++) {
            int character = stream.in_uint16_le();
            int offset = stream.in_uint16_le();
            int baseline = stream.in_uint16_le();
            int width = stream.in_uint16_le();
            int height = stream.in_uint16_le();
            int datasize = (height * nbbytes(width) + 3) & ~3;
            const uint8_t *data = stream.in_uint8p(datasize);

            mod->server_add_char(font, character, offset, baseline, width, height, data);
        }
    }

    static void rdp_orders_parse_brush(Stream & stream, struct RDPBrush* brush, int present)
    {
//        LOG(LOG_INFO, "rdp_orders_parse_brush");
        if (present & 1) {
            brush->org_x = stream.in_uint8();
        }
        if (present & 2) {
            brush->org_y = stream.in_uint8();
        }
        if (present & 4) {
            brush->style = stream.in_uint8();
        }
        if ((brush->style == 3) && (present & 8)) {
            memcpy(brush->extra, stream.in_uint8p( 7),  7);
        }
    }


    void rdp_orders_process_desksave(Stream & stream, int present, int delta, client_mod * mod)
    {
//        LOG(LOG_INFO, "rdp_orders_process_desksave");
        int width;
        int height;

        if (present & 0x01) {
            this->state.desksave_offset = stream.in_uint32_le();
        }
        if (present & 0x02) {
            if (delta){
                this->state.desksave_left += stream.in_sint8();
            }
            else {
                this->state.desksave_left = stream.in_sint16_le();
            }
        }
        if (present & 0x04) {
            if (delta){
                this->state.desksave_top += stream.in_sint8();
            }
            else {
                this->state.desksave_top = stream.in_sint16_le();
            }
        }
        if (present & 0x08) {
            if (delta){
                this->state.desksave_right += stream.in_sint8();
            }
            else {
                this->state.desksave_right = stream.in_sint16_le();
            }
        }
        if (present & 0x10) {
            if (delta){
                this->state.desksave_bottom += stream.in_sint8();
            }
            else {
                this->state.desksave_bottom = stream.in_sint16_le();
            }
        }
        if (present & 0x20) {
            this->state.desksave_action = stream.in_uint8();
        }
        width = (this->state.desksave_right - this->state.desksave_left) + 1;
        height = (this->state.desksave_bottom - this->state.desksave_top) + 1;
        if (this->state.desksave_action == 0) {
    //		ui_desktop_save(ostream.offset, ostream.left, ostream.top, width, height);
        } else {
    //		ui_desktop_restore(ostream.offset, ostream.left, ostream.top, width, height);
        }
    }

    /*****************************************************************************/
    /* Process a 3-way blt order */
    static void rdp_orders_process_triblt(struct rdp_orders* self, Stream & stream, int present, int delta, client_mod * mod)
    {
//        LOG(LOG_INFO, "rdp_orders_process_triblt");
        /* not used */
    }

    /*****************************************************************************/
    /* Process a polyline order */
    void rdp_orders_process_polyline(Stream & stream, int present, int delta, client_mod * mod)
    {
//        LOG(LOG_INFO, "rdp_orders_process_polyline");
        if (present & 0x01) {
            if (delta){
                this->state.polyline_x += stream.in_sint8();
            }
            else {
                this->state.polyline_x = stream.in_sint16_le();
            }
        }
        if (present & 0x02) {
            if (delta){
                this->state.polyline_y += stream.in_sint8();
            }
            else {
                this->state.polyline_y = stream.in_sint16_le();
            }
        }
        if (present & 0x04) {
            this->state.polyline_opcode = stream.in_uint8();
        }
        if (present & 0x10) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->state.polyline_fgcolor = (r << 16) | (g << 8) | b;
        }
        if (present & 0x20) {
            this->state.polyline_lines = stream.in_uint8();
        }
        if (present & 0x40) {
            this->state.polyline_datasize = stream.in_uint8();
            memcpy( this->state.polyline_data, stream.in_uint8p( this->state.polyline_datasize), this->state.polyline_datasize);
        }
        /* todo */
    }

    /*****************************************************************************/
    int rdp_orders_process_orders(Stream & stream, int num_orders, client_mod * mod)
    {
        using namespace RDP;
        int processed = 0;
        while (processed < num_orders) {
            uint8_t control = stream.in_uint8();

            if (!control & STANDARD){
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                break;
            }
            if (control & SECONDARY) {
                using namespace RDP;

                RDPSecondaryOrderHeader header(stream);
//                LOG(LOG_INFO, "secondary order=%d\n", header.type);
                uint8_t *next_order = stream.p + header.length + 7;
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                    this->rdp_orders_process_bmpcache(stream, control, header);
                    break;
                case TS_CACHE_COLOR_TABLE:
                    this->cache_colormap.receive(stream, control, header);
                    break;
                case FONTCACHE:
                    this->rdp_orders_process_fontcache(stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                  break;
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                  break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(stream, control);
                if (control & BOUNDS) {
                    mod->server_set_clip(this->common.clip);
                }
                else {
                    mod->server_reset_clip();
                }
                LOG(LOG_INFO, "/* order=%d ordername=%s */\n", this->common.order, ordernames[this->common.order]);
                switch (this->common.order) {
                case TEXT2:
                    this->glyph_index.receive(stream, header);
                    mod->server_glyph_index(this->glyph_index);
                    break;
                case DESTBLT:
                    this->destblt.receive(stream, header);
                    mod->dest_blt(this->destblt);
                    break;
                case PATBLT:
                    this->patblt.receive(stream, header);
                    mod->pat_blt(this->patblt);
                    break;
                case SCREENBLT:
                    this->scrblt.receive(stream, header);
                    mod->scr_blt(this->scrblt);
                    break;
                case LINE:
                    this->lineto.receive(stream, header);
                    mod->line_to(this->lineto);
                    break;
                case RECT:
                    this->opaquerect.receive(stream, header);
                    mod->opaque_rect(this->opaquerect);
                    break;
                case DESKSAVE:
                    this->rdp_orders_process_desksave(stream, header.fields, header.control & DELTA, mod);
                    break;
                case MEMBLT:
                    this->memblt.receive(stream, header);
                    {
                        struct Bitmap* bitmap = this->cache_bitmap[this->memblt.cache_id & 0xFF][this->memblt.cache_idx];
                        if (bitmap) {
                            mod->mem_blt(
                                this->memblt,
                                *bitmap,
                                this->cache_colormap.palette[this->memblt.cache_id >> 8]);
                        }
                    }
                    break;
                case TRIBLT:
                    rdp_orders_process_triblt(this, stream, header.fields, header.control & DELTA, mod);
                    break;
                case POLYLINE:
                    this->rdp_orders_process_polyline(stream, header.fields, header.control & DELTA, mod);
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
                if (header.control & BOUNDS) {
                    mod->server_reset_clip();
                }
            }
            processed++;
        }
        return 0;
    }
};

#endif
