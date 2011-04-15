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
    struct Bitmap * cache_bitmap[3][600];

    #warning it looks strange that rdp_orders object should be depending on bpp parameter, it looks more like a cache implementation detail that should be abstracted here.
    rdp_orders(int bpp) :
        common(0, Rect()),
        memblt(0, Rect(), 0, 0, 0, 0),
        opaquerect(Rect(), 0),
        scrblt(Rect(), 0, 0, 0),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyph_index(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        cache_colormap(bpp)
    {
        memset(this->cache_bitmap, 0, sizeof(this->cache_bitmap));
    }


    ~rdp_orders(){
    }

    #warning smells like code duplication, it would probably be better to destroy rdp_orders object and recreate it instead of calling that reset_state.
    void rdp_orders_reset_state()
    {
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


    void rdp_orders_process_bmpcache(Stream & stream, int flags)
    {
        uint8_t* data;
        int size = 0;
        int pad2 = 0;
        int row_size = 0;
        int final_size = 0;

        int cache_id = stream.in_uint8();
        int pad1 = stream.in_uint8();
        pad1 = pad1; // just to remove warning, will be optimized away
        int width = stream.in_uint8();
        int height = stream.in_uint8();
        int bpp = stream.in_uint8();
        int bufsize = stream.in_uint16_le();
        int cache_idx = stream.in_uint16_le();
        if (flags & 0x400) {
            size = bufsize;
        } else {
            pad2 = stream.in_uint16_le();
            size = stream.in_uint16_le();
            row_size = stream.in_uint16_le();
            final_size = stream.in_uint16_le();
        }

//        printf("\n----------------------------------------------\n");
//        printf("Received bitmap from server:"
//               " bpp=%d width=%d height=%d bufsize=%d"
//               " cache_idx=%d flags=%x pad1=%d"
//               " pad2=%d size=%d row_size=%d final_size=%d\n",
//            bpp, width, height, bufsize,
//            cache_idx, flags, pad1,
//            pad2, size, row_size, final_size);

        data = stream.in_uint8p(size);

//        printf("------- Compressed V4---------\n");
//        for (int i = 0; i < size; i++){
//            if (0==(i % 16)){
//                printf("\n");
//            }
//            printf("0x%.2x, ", data[i]);
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");

        struct Bitmap * bitmap = new Bitmap(bpp, width, height);
        bitmap->decompress(data, size);

//        printf("------- DeCompressed V4---------\n");
//        for (int y = 0; y < height; y++){
//            printf("\n\n/*- %d -*/\n", y);
//            for (int x = 0; x < row_size; x++){
//                if (0==((y*row_size+x) % 16)){
//                    printf("\n      ");
//                }
//                printf("0x%.2x, ", bitmap->data_co[y*row_size+x]);
//            }
//        }
//        printf("\n");
//        printf("\n----------------------------\n");
//        printf("\n");


        if (this->cache_bitmap[cache_id][cache_idx]) {
            delete this->cache_bitmap[cache_id][cache_idx];
        }
        this->cache_bitmap[cache_id][cache_idx] = bitmap;
    }

    void rdp_orders_process_fontcache(Stream & stream, int flags, client_mod * mod)
    {
        int font = stream.in_uint8();
        int nglyphs = stream.in_uint8();
        for (int i = 0; i < nglyphs; i++) {
            int character = stream.in_uint16_le();
            int offset = stream.in_uint16_le();
            int baseline = stream.in_uint16_le();
            int width = stream.in_uint16_le();
            int height = stream.in_uint16_le();
            int datasize = (height * nbbytes(width) + 3) & ~3;
            uint8_t *data = stream.in_uint8p(datasize);

            mod->server_add_char(font, character, offset, baseline, width, height, data);
        }
    }

    static void rdp_orders_parse_brush(Stream & stream, struct RDPBrush* brush, int present)
    {
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
        /* not used */
    }

    /*****************************************************************************/
    /* Process a polyline order */
    void rdp_orders_process_polyline(Stream & stream, int present, int delta, client_mod * mod)
    {
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
            this->state.polyline_fgcolor = r | (g << 8) | (b << 16);
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
//            LOG(LOG_INFO, "Num order = %d on %d (%p -> %p)", processed, num_orders, stream.p, stream.data);
            uint8_t control = stream.in_uint8();

            if (!control & STANDARD){
                /* error, this should always be set */
                LOG(LOG_INFO, "Non standard order detected : protocol error");
                break;
            }
            if (control & SECONDARY) {
                using namespace RDP;

                RDPSecondaryOrderHeader header(stream);
                uint8_t *next_order = stream.p + header.length + 7;
                switch (header.type) {
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                    LOG(LOG_WARNING, "RAW_BMPCACHE");
                    {
                        RDPBmpCache bmp;
                        bmp.receive(stream, control, header);

                        #warning init of Bitmap is done here
                        struct Bitmap* bitmap = new Bitmap(bmp.bpp, bmp.width, bmp.height);
                        bitmap->copy(bmp.data);
                        if (this->cache_bitmap[bmp.cache_id][bmp.cache_idx]) {
                            delete(this->cache_bitmap[bmp.cache_id][bmp.cache_idx]);
                        }
                        this->cache_bitmap[bmp.cache_id][bmp.cache_idx] = bitmap;
                    }
                    break;
                case COLCACHE:
//                    LOG(LOG_WARNING, "COLCACHE");
                    this->cache_colormap.receive(stream, control, header);

                    break;
                case TS_CACHE_BITMAP_COMPRESSED:
//                    LOG(LOG_WARNING, "TS_CACHE_BITMAP_COMPRESSED");
                    this->rdp_orders_process_bmpcache(stream, header.flags);
                    break;
                case FONTCACHE:
//                    LOG(LOG_WARNING, "FONTCACHE");
                    this->rdp_orders_process_fontcache(stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG(LOG_WARNING, "TS_CACHE_BITMAP_COMPRESSED V2 not supported (1)");
                  break;
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG(LOG_WARNING, "RAW BMPCACHE V2 not supported (0)");
                  break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG(LOG_WARNING, "BMPCACHE V3 not supported (0)");
                  break;

                default:
                    LOG(LOG_WARNING, "unsupported SECONDARY ORDER (%d)", header.type);
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
                switch (this->common.order) {
                case TEXT2:
//                    LOG(LOG_WARNING, "TEXT2");
                    this->rdp_orders_process_text2(stream, mod, header);
                    break;
                case DESTBLT:
//                    LOG(LOG_WARNING, "DESTBLT");
                    this->rdp_orders_process_destblt(stream, mod, header);
                    break;
                case PATBLT:
//                    LOG(LOG_WARNING, "PATBLT");
                    this->rdp_orders_process_patblt(stream, mod, header);
                    break;
                case SCREENBLT:
//                    LOG(LOG_WARNING, "SCREENBLT");
                    this->rdp_orders_process_screenblt(stream, mod, header);
                    break;
                case LINE:
//                    LOG(LOG_WARNING, "LINE");
                    this->rdp_orders_process_line(stream, mod, header);
                    break;
                case RECT:
//                    LOG(LOG_WARNING, "RECT");
                    this->rdp_orders_process_rect(stream, mod, header);
                    break;
                case DESKSAVE:
//                    LOG(LOG_WARNING, "DESKSAVE");
                    this->rdp_orders_process_desksave(stream, header.fields, header.control & DELTA, mod);
                    break;
                case MEMBLT:
//                    LOG(LOG_WARNING, "MEMBLT");
                    this->rdp_orders_process_memblt(stream, mod, header);
                    break;
                case TRIBLT:
//                    LOG(LOG_WARNING, "TRIBLT");
                    rdp_orders_process_triblt(this, stream, header.fields, header.control & DELTA, mod);
                    break;
                case POLYLINE:
//                    LOG(LOG_WARNING, "POLYLINE");
                    this->rdp_orders_process_polyline(stream, header.fields, header.control & DELTA, mod);
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_WARNING, "unsupported PRIMARY ORDER (%d)", this->common.order);
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

    /*****************************************************************************/
    /* returns pointer, it might return bmpdata if the data dosen't need to
       be converted, else it mallocs it.  The calling function must free
       it if needed */
    #warning we should perform memory allocation outside, before calling this function
    uint8_t* rdp_orders_convert_bitmap(int in_bpp, int out_bpp, const uint8_t* src,
                              int width, int height, const uint32_t (& palette)[256])
    {
        uint8_t* dst = 0;
        uint8_t* out = 0;
        int i;
        int j;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        int pixel;

    #warning rewrite the code below in a sensible way, I don't want to be biten by unexpected cases (like in_bpp == 15 and out_bpp == 15) again.
    #warning also remove all returns in the middle of the function, use malloc instead of new (and change the matchind deletes by free) as it is a bunch of bytes
    #warning this code looks very alike some other code (written differently, but alike anyway) in bitmap.hpp check if they are not really the same.

        if (((in_bpp == 16) && (out_bpp == 16))
        || ((in_bpp == 15) && (out_bpp == 15))) {
            #warning casting below is evil and dangerous, see to that
            out = dst = (uint8_t*)src;
        }
        else {
            int BPP = 4;
            switch (out_bpp){
                case 8:
                    BPP = 1;
                break;
                case 15: case 16:
                    BPP = 2;
                break;
                case 24:
                    BPP = 3;
                break;
                case 32:
                    BPP = 4;
                break;
                default:
                    // should not happen
                break;
            }
            out = dst = (uint8_t*)malloc(width * height * BPP);

            if ((in_bpp == 8) && (out_bpp == 8)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        pixel = *((uint8_t*)src);
                        pixel = palette[pixel];
                        splitcolor32(red, green, blue, pixel);
                        pixel = color8(red, green, blue);
                        *dst = pixel;
                        src++;
                        dst++;
                    }
                }
            }
            else if ((in_bpp == 8) && (out_bpp == 16)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        pixel = *((uint8_t*)src);
                        pixel = palette[pixel];
                        splitcolor32(red, green, blue, pixel);
                        pixel = color16(red, green, blue);
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        src++;
                    }
                }
            }
            else if ((in_bpp == 8) && (out_bpp == 24)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        pixel = *((uint8_t*)src);
                        pixel = palette[pixel];
                        splitcolor32(red, green, blue, pixel);
                        pixel = color24RGB(red, green, blue);
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel >> 16;
                        src++;;
                    }
                }
            }
            else if ((in_bpp == 15) && (out_bpp == 16)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        pixel = *((uint16_t*)src);
                        splitcolor15(red, green, blue, pixel);
                        pixel = color16(red, green, blue);
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        src += 2;
                    }
                }
            }
            else if ((in_bpp == 15) && (out_bpp == 24)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        pixel = *((uint16_t*)src);
                        splitcolor15(red, green, blue, pixel);
                        pixel = color24RGB(red, green, blue);
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel >> 16;
                        src += 2;
                    }
                }
            }
            else if ((in_bpp == 16) && (out_bpp == 24)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        pixel = *((uint16_t*)src);
                        splitcolor16(red, green, blue, pixel);
                        pixel = color24RGB(red, green, blue);
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel >> 16;
                        src += 2;
                    }
                }
            }
            else if ((in_bpp == 24) && (out_bpp == 24)) {
                for (i = 0; i < height; i++) {
                    for (j = 0; j < width; j++) {
                        blue = *((uint8_t*)src);
                        src++;
                        green = *((uint8_t*)src);
                        src++;
                        red = *((uint8_t*)src);
                        src++;
                        pixel = color24RGB(red, green, blue);
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel >> 16;
                    }
                }
            }
        }
        return out;
    }

    /*****************************************************************************/
    /* returns color or 0 */
    int rdp_orders_convert_color(int in_bpp, int out_bpp, int in_color, const uint32_t (& palette)[256])
    {
        int pixel;
        uint8_t red;
        uint8_t green;
        uint8_t blue;

        if ((in_bpp == 8) && (out_bpp == 8)) {
            pixel = palette[in_color];
            splitcolor32(red, green, blue, pixel);
            pixel = color8(red, green, blue);
            return pixel;
        }
        if ((in_bpp == 8) && (out_bpp == 16)) {
            pixel = palette[in_color];
            splitcolor32(red, green, blue, pixel);
            pixel = color16(red, green, blue);
            return pixel;
        }
        if ((in_bpp == 8) && (out_bpp == 24)) {
            pixel = palette[in_color];
            splitcolor32(red, green, blue, pixel);
            pixel = color24BGR(red, green, blue);
            return pixel;
        }
        if ((in_bpp == 15) && (out_bpp == 16)) {
            pixel = in_color;
            splitcolor15(red, green, blue, pixel);
            pixel = color16(red, green, blue);
            return pixel;
        }
        if ((in_bpp == 15) && (out_bpp == 24)) {
            pixel = in_color;
            splitcolor15(red, green, blue, pixel);
            pixel = color24BGR(red, green, blue);
            return pixel;
        }
        if ((in_bpp == 16) && (out_bpp == 16)) {
            return in_color;
        }
        if ((in_bpp == 16) && (out_bpp == 24)) {
            pixel = in_color;
            splitcolor16(red, green, blue, pixel);
            pixel = color24BGR(red, green, blue);
            return pixel;
        }
        if ((in_bpp == 24) && (out_bpp == 24)) {
            return in_color;
        }
        return 0;
    }
    void rdp_orders_process_destblt(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->destblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->destblt.receive(stream, header);

//        this->destblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);

        mod->server_fill_rect(this->destblt.rop, this->destblt.rect);
    }

    void rdp_orders_process_rect(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->opaquerect.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->opaquerect.receive(stream, header);

//        this->opaquerect.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);


        int fgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->opaquerect.color,
                                           this->cache_colormap.palette[0]);
        mod->server_set_fgcolor(fgcolor);
        mod->server_fill_rect(0xCC, this->opaquerect.rect);
    }


    void rdp_orders_process_memblt(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->memblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->memblt.receive(stream, header);

//        this->memblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);

        struct Bitmap* bitmap = this->cache_bitmap[this->memblt.cache_id & 0xFF][this->memblt.cache_idx];
        uint8_t* bmpdata;
        if (bitmap != 0 && (bitmap->cx > 0) && (bitmap->cy > 0)) {
            assert(bitmap->data_co);
            try {
                bmpdata = rdp_orders_convert_bitmap(
                    this->cache_colormap.bpp,
                    mod->screen.colors->bpp,
                    bitmap->data_co,
                    bitmap->cx,
                    bitmap->cy,
                    this->cache_colormap.palette[(this->memblt.cache_id >> 8) & 0xFF]);
                mod->server_paint_rect(this->memblt.rop, this->memblt.rect,
                          bmpdata, bitmap->cx, bitmap->cy,
                          this->memblt.srcx, this->memblt.srcy);
            } catch (...){
                if (bmpdata != bitmap->data_co) {
                    free(bmpdata);
                }
                #warning see which exception is raised here, was source of memory leak
                throw;
            };
            if (bmpdata != bitmap->data_co) {
                free(bmpdata);
            }
        }
    }

    void rdp_orders_process_screenblt(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->scrblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->scrblt.receive(stream, header);

//        this->scrblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);

        mod->server_screen_blt(this->scrblt.rop,
                               this->scrblt.rect,
                               this->scrblt.srcx,
                               this->scrblt.srcy);
    }

    void rdp_orders_process_patblt(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->patblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->patblt.receive(stream, header);

//        this->patblt.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);

        int fgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->patblt.fore_color,
                                           this->cache_colormap.palette[0]);
        mod->server_set_fgcolor(fgcolor);
        int bgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->patblt.back_color,
                                           this->cache_colormap.palette[0]);
        mod->server_set_bgcolor(bgcolor);

        mod->server_set_brush(this->patblt.brush);

        mod->server_basic_fill_rect(this->patblt.rop, this->patblt.rect);
    }

    void rdp_orders_process_line(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->lineto.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->lineto.receive(stream, header);

//        this->lineto.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);


        int bgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->lineto.back_color,
                                           this->cache_colormap.palette[0]);
        int fgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->lineto.pen.color,
                                           this->cache_colormap.palette[0]);
        mod->server_set_fgcolor(fgcolor);
        mod->server_set_bgcolor(bgcolor);
        mod->server_set_pen(this->lineto.pen.style, this->lineto.pen.width);
        mod->server_draw_line(this->lineto.rop2,
                              this->lineto.startx,
                              this->lineto.starty,
                              this->lineto.endx,
                              this->lineto.endy);

    }

    void rdp_orders_process_text2(Stream & stream, client_mod * mod, const RDPPrimaryOrderHeader & header)
    {
//        char buffer[1000];
//        this->glyph_index.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "OLD RECV: %s", buffer);

        this->glyph_index.receive(stream, header);

//        this->glyph_index.str(buffer, 1000, this->common);
//        LOG(LOG_INFO, "NEW RECV: %s", buffer);

        int fgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->glyph_index.fore_color,
                                           this->cache_colormap.palette[0]);
        mod->server_set_fgcolor(fgcolor);
        int bgcolor = rdp_orders_convert_color(this->cache_colormap.bpp,
                                           mod->screen.colors->bpp,
                                           this->glyph_index.back_color,
                                           this->cache_colormap.palette[0]);
        mod->server_set_bgcolor(bgcolor);
        mod->server_draw_text2(
            this->glyph_index.cache_id,
            this->glyph_index.fl_accel,
            this->glyph_index.f_op_redundant,
            this->glyph_index.bk.x,
            this->glyph_index.bk.y,
            this->glyph_index.bk.x + this->glyph_index.bk.cx - 1,
            this->glyph_index.bk.y + this->glyph_index.bk.cy - 1,
            this->glyph_index.op.x - 1,
            this->glyph_index.op.y,
            this->glyph_index.op.x + this->glyph_index.op.cx - 1,
            this->glyph_index.op.y + this->glyph_index.op.cy - 1,
            this->glyph_index.glyph_x,
            this->glyph_index.glyph_y,
            this->glyph_index.data,
            this->glyph_index.data_len);
    }

};

#endif
