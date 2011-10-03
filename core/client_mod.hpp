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

   module object. Some kind of interface between core and modules

*/
#if !defined(__MOD_HPP__)
#define __MOD_HPP__

#include <string.h>
#include <stdio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <locale.h>

#include "client_info.hpp"
#include "font.hpp"
#include "cache.hpp"
#include "front.hpp"
#include "mainloop.hpp"
#include "bitmap_cache.hpp"
#include "wait_obj.hpp"
#include "keymap.hpp"
#include "callback.hpp"
#include "graphic_device.hpp"

struct GraphicDeviceMod : public GraphicDevice
{
    Front & front;
    Capture * capture;
    Rect clip;
    RDPPen pen;
    bool palette_sent;
    bool palette_memblt_sent[6];
    BGRPalette palette332;
    BGRPalette mod_palette;
    int current_pointer;
    uint8_t mod_bpp;
    BGRPalette memblt_mod_palette;
    bool mod_palette_setted;


    GraphicDeviceMod(Front & front)
    : front(front),
      capture(NULL),
      clip(clip),
      palette_sent(false),
      mod_palette_setted(0),
      mod_bpp(24)

    {
        for (size_t i = 0; i < 6 ; i++){
            this->palette_memblt_sent[i] = false;
        }
        this->current_pointer = 0;
        init_palette332(this->palette332);
        this->clip = Rect(0,0,4096,2048);
    }


    virtual void server_set_clip(const Rect & rect)
    {
        this->clip = rect;
    }

    virtual void server_reset_clip()
    {
        this->clip = this->get_front_rect();
    }


    virtual int text_width(const char * text){
        int rv = 0;
        if (text) {
            size_t len = mbstowcs(0, text, 0);
            wchar_t wstr[len + 2];
            mbstowcs(wstr, text, len + 1);
            for (size_t index = 0; index < len; index++) {
                FontChar *font_item = this->front.font.font_items[wstr[index]];
                rv = rv + font_item->incby;
            }
        }
        return rv;
    }

    virtual int text_height(const char * text){
        int rv = 0;
        if (text) {
            int len = mbstowcs(0, text, 0);
            wchar_t *wstr = new wchar_t[len + 2];
            mbstowcs(wstr, text, len + 1);
            for (int index = 0; index < len; index++) {
                FontChar *font_item = this->front.font.font_items[wstr[index]];
                rv = std::max(rv, font_item->height);
            }
            delete [] wstr;
        }
        return rv;
    }

    virtual int server_begin_update() {
        this->front.begin_update();
        return 0;
    }

    virtual int server_end_update(){
        this->front.end_update();
        return 0;
    }

    virtual const ClientInfo & get_client_info() const {
        return this->front.get_client_info();
    }

    virtual int get_front_bpp() const {
        return this->get_client_info().bpp;
    }

    virtual int get_front_width() const {
        return this->get_client_info().width;
    }

    virtual int get_front_height() const {
        return this->get_client_info().height;
    }

    virtual const Rect get_front_rect(){
        return Rect(0, 0, this->get_front_width(), get_front_height());
    }

    virtual void draw_window(const Rect & r, uint32_t bgcolor, const char * caption, bool has_focus){

        // Window surface and border
        this->opaque_rect(
            RDPOpaqueRect(r, bgcolor));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), WHITE));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), WHITE));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + 1, r.y + r.cy - 2, r.cx - 2, 1), DARK_GREY));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x+r.cx-2, r.y + 1, 1, r.cy), DARK_GREY));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), BLACK));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), BLACK));

        // Title bar
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + 3, r.y + 3, r.cx - 5, 18),
                          has_focus?WABGREEN:DARK_GREY));

        this->server_draw_text(r.x + 4, r.y + 4, caption,
                has_focus?WABGREEN:DARK_GREY,
                has_focus?WHITE:BLACK);
    }

    virtual void draw_combo(const Rect & r, const char * caption, int state, bool has_focus)
    {
        this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, r.cx, r.cy), GREY));
        this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 3, r.cy - 3), WHITE));
        if (has_focus) {
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + 3, r.y + 3, (r.cx - 6) - 18, r.cy - 5), DARK_WABGREEN));
        }
        this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), DARK_GREY));
        this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), DARK_GREY));
        this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y + r.cy- 1, r.cx, 1), WHITE));
        this->opaque_rect(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE));
        this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), BLACK));
        this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), BLACK));
        this->server_draw_text(r.x + 4, r.y + 3, caption, has_focus?DARK_WABGREEN:WHITE, has_focus?WHITE:BLACK);
        this->draw_button(Rect(r.x + r.cx - 20, r.y + 2, 18, r.cy - 4), "", state, false);
    }

    virtual void draw_button(const Rect & r, const char * caption, int state, bool has_focus){

        int bevel = (state == BUTTON_STATE_DOWN)?1:0;

        this->opaque_rect(RDPOpaqueRect(r, GREY));
        if (state == BUTTON_STATE_DOWN) {
            this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK));
        } else {
            this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), WHITE));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), WHITE));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY));
            this->opaque_rect(RDPOpaqueRect(Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK));
            this->opaque_rect(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK));
        }
        int w = this->text_width(caption);
        int h = this->text_height(caption);
        this->server_draw_text(
            r.x + r.cx / 2 - w / 2 + bevel,
            r.y + r.cy / 2 - h / 2 + bevel,
            caption, GREY, BLACK);
        // focus rect
        if (has_focus) {
            this->pat_blt(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")));
            this->pat_blt(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")));
            this->pat_blt(
                RDPPatBlt(Rect(r.x + r.cx - 6 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")));
            this->pat_blt(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + r.cy - 6 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")));
        }
    }

    virtual void draw_edit(const Rect & r, char password_char, char * buffer, size_t edit_pos, bool has_focus){
        this->opaque_rect(
            RDPOpaqueRect(r, GREY));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), WHITE));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), DARK_GREY));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), DARK_GREY));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), WHITE));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), BLACK));
        this->opaque_rect(
            RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), BLACK));

        /* draw text */
        char text[255];
        wchar_t wtext[255];

        if (password_char != 0) {
            int i = mbstowcs(0, buffer, 0);
            memset(text, password_char, i);
            text[i] = 0;
            this->server_draw_text(r.x + 4, r.y + 2, text, WHITE, BLACK);
        }
        else {
            this->server_draw_text(r.x + 4, r.y + 2, buffer, WHITE, BLACK);
        }
        /* draw xor box(cursor) */
        if (has_focus) {
            if (password_char != 0) {
                for (int index = 0; index < edit_pos; index++) {
                    if (index >= 255) {
                        break;
                    }
                    wtext[index] = password_char;
                }
                wtext[edit_pos] = 0;
                wcstombs(text, wtext, 255);
            } else {
                mbstowcs(wtext, buffer, 255);
                wtext[edit_pos] = 0;
                wcstombs(text, wtext, 255);
            }

            this->opaque_rect(
                RDPOpaqueRect(
                    Rect(r.x + 4 + this->text_width(text), r.y + 3, 2, r.cy - 6),
                    DARK_GREY));
        }
    }

    #warning implementation of the server_draw_text function below is quite broken (a small subset of possibilities is implemented, especially for data). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)
    virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor)
    {
        setlocale(LC_CTYPE, "fr_FR.UTF-8");
        this->send_global_palette();

        // add text to glyph cache
        #warning use mbsrtowcs instead
        int len = mbstowcs(0, text, 0);
        wchar_t* wstr = new wchar_t[len + 2];
        mbstowcs(wstr, text, len + 1);
        int total_width = 0;
        int total_height = 0;
        uint8_t *data = new uint8_t[len * 4];
        memset(data, 0, len * 4);
        int f = 0;
        int c = 0;
        int distance_from_previous_fragment = 0;
        for (int index = 0; index < len; index++) {
            FontChar* font_item = this->front.font.font_items[wstr[index]];
            #warning avoid passing parameters by reference to get results
            switch (this->front.cache.add_glyph(font_item, f, c))
            {
                case Cache::GLYPH_ADDED_TO_CACHE:
                    this->glyph_cache(*font_item, f, c);
                break;
                default:
                break;
            }
            data[index * 2] = c;
            data[index * 2 + 1] = distance_from_previous_fragment;
            distance_from_previous_fragment = font_item->incby;
            total_width += font_item->incby;
            total_height = std::max(total_height, font_item->height);
        }

        const Rect bk(x, y, total_width + 1, total_height);

         RDPGlyphIndex glyphindex(
            f, // cache_id
            0x03, // fl_accel
            0x0, // ui_charinc
            1, // f_op_redundant,
            bgcolor, // bgcolor
            fgcolor, // fgcolor
            bk, // bk
            Rect(), // op
            // brush
            RDPBrush(0, 0, 3, 0xaa,
                (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55"),
//            this->brush,
            x,  // glyph_x
            y + total_height, // glyph_y
            len * 2, // data_len in bytes
            data // data
        );

        this->glyph_index(glyphindex);

        delete [] wstr;
        delete [] data;
    }

    virtual void opaque_rect(const RDPOpaqueRect & cmd)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPOpaqueRect new_cmd = cmd;
            new_cmd.color = this->convert_opaque(cmd.color);
            this->front.orders->send(new_cmd, this->clip);

            if (this->capture){
                RDPOpaqueRect new_cmd24 = cmd;
                new_cmd24.color = this->convert24_opaque(cmd.color);
                this->capture->opaque_rect(new_cmd24, this->clip);
            }
        }
    }

    virtual void scr_blt(const RDPScrBlt & cmd)
    {
        if (!this->clip.isempty()
        && !this->clip.intersect(cmd.rect).isempty()){
            this->front.orders->send(cmd, clip);

            if (this->capture){
                this->capture->scr_blt(cmd, clip);
            }
        }
    }

    virtual void dest_blt(const RDPDestBlt & cmd)
    {
        if (!this->clip.isempty()
        && !this->clip.intersect(cmd.rect).isempty()){
            this->front.orders->send(cmd, this->clip);
            if (this->capture){
                this->capture->dest_blt(cmd, this->clip);
            }
        }
    }

    virtual void pat_blt(const RDPPatBlt & cmd)
    {
        if (!this->clip.isempty()
        && !this->clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPPatBlt new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.fore_color = this->convert(cmd.fore_color);

            if (new_cmd.brush.style == 3){
                if (this->get_client_info().brush_cache_code == 1) {
                    uint8_t pattern[8];
                    pattern[0] = new_cmd.brush.hatch;
                    memcpy(pattern+1, new_cmd.brush.extra, 7);
                    int cache_idx = 0;
                    if (BRUSH_TO_SEND == this->front.cache.add_brush(pattern, cache_idx)){
                        this->brush_cache(cache_idx);
                    }
                    new_cmd.brush.hatch = cache_idx;
                    new_cmd.brush.style = 0x81;
                }
            }
            this->front.orders->send(new_cmd, this->clip);

            if (this->capture){
                RDPPatBlt new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.fore_color = this->convert24(cmd.fore_color);

                this->capture->pat_blt(new_cmd24, this->clip);
            }
        }
    }

    virtual void mem_blt(const RDPMemBlt & memblt, Bitmap & bitmap, const BGRPalette & palette)
    {
        uint8_t palette_id = ((memblt.cache_id >> 4) >= 6)?0:(memblt.cache_id >> 4);

        if (this->get_front_bpp() == 8){
            #warning there may be some performance issue when client is 8 bits, palette is sent way too often
            this->palette_sent = false;
            this->send_global_palette();
            if (!this->palette_memblt_sent[palette_id]) {
                this->color_cache(palette, palette_id);
                this->palette_memblt_sent[palette_id] = true;
            }
            this->palette_sent = false;
        }

        const Rect & dst = memblt.rect;
        const int srcx = memblt.srcx;
        const int srcy = memblt.srcy;
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const Rect src_r(srcx, srcy, width, height);
        const uint8_t * src_data = bitmap.data_co(this->get_front_bpp());
        const uint8_t rop = memblt.rop;

        for (int y = 0; y < dst.cy ; y += 32) {
            int cy = std::min(32, dst.cy - y);
            for (int x = 0; x < dst.cx ; x += 32) {
                int cx = std::min(32, dst.cx - x);
                const Rect tile(x, y, cx, cy);
                if (!this->clip.intersect(tile.offset(dst.x, dst.y)).isempty()){
                    #warning transmit a bitmap to add_bitmap instead of individual components
                     uint32_t cache_ref = this->front.bmp_cache->add_bitmap(
                                                src_r.cx, src_r.cy,
                                                src_data,
                                                tile.offset(src_r.x, src_r.y),
                                                this->get_front_bpp(),
                                                bitmap.original_palette);

                    uint8_t send_type = (cache_ref >> 24);
                    uint8_t cache_id  = (cache_ref >> 16);
                    uint16_t cache_idx = (cache_ref & 0xFFFF);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->bitmap_cache(cache_id, cache_idx);
                    }

                    const RDPMemBlt cmd(cache_id + palette_id*16, tile.offset(dst.x, dst.y), rop, 0, 0, cache_idx);
//                    cmd.log(LOG_INFO, this->clip);

                    if (!this->clip.isempty()
                    && !this->clip.intersect(cmd.rect).isempty()){
                        this->front.orders->send(cmd, this->clip);
                        #warning capture should have it's own reference to bmp_cache
                        if (this->capture){
                            this->capture->mem_blt(cmd, *this->front.bmp_cache, this->clip);
                        }
                    }
                }
            }
        }
    }

    virtual void server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
    }

    virtual void line_to(const RDPLineTo & cmd)
    {
        const uint16_t minx = std::min(cmd.startx, cmd.endx);
        const uint16_t miny = std::min(cmd.starty, cmd.endy);
        const Rect rect(minx, miny,
                        std::max(cmd.startx, cmd.endx)-minx+1,
                        std::max(cmd.starty, cmd.endy)-miny+1);

        if (!this->clip.isempty() && !this->clip.intersect(rect).isempty()){

            RDPLineTo new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.pen.color = this->convert(cmd.pen.color);

            this->front.orders->send(new_cmd, clip);

            if (this->capture){
                RDPLineTo new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.pen.color = this->convert24(cmd.pen.color);

                this->capture->line_to(new_cmd24, clip);
            }
        }
    }

    virtual void glyph_index(const RDPGlyphIndex & cmd)
    {
        if (!this->clip.isempty() && !this->clip.intersect(cmd.bk).isempty()){
            this->send_global_palette();

            RDPGlyphIndex new_cmd = cmd;
            new_cmd.back_color = this->convert_opaque(cmd.back_color);
            new_cmd.fore_color = this->convert_opaque(cmd.fore_color);

            if (new_cmd.brush.style == 3){
                if (this->get_client_info().brush_cache_code == 1) {
                    uint8_t pattern[8];
                    pattern[0] = new_cmd.brush.hatch;
                    memcpy(pattern+1, new_cmd.brush.extra, 7);
                    int cache_idx = 0;
                    if (BRUSH_TO_SEND == this->front.cache.add_brush(pattern, cache_idx)){
                        this->brush_cache(cache_idx);
                    }
                    new_cmd.brush.hatch = cache_idx;
                    new_cmd.brush.style = 0x81;
                }
            }

            this->front.orders->send(new_cmd, this->clip);

            if (this->capture){
                RDPGlyphIndex new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24_opaque(cmd.back_color);
                new_cmd24.fore_color = this->convert24_opaque(cmd.fore_color);

                this->capture->glyph_index(new_cmd24, this->clip);
            }
        }
    }

    virtual void brush_cache(const int index)
    {
        RDPBrushCache cmd(index, 1, 8, 8, 0x81,
            sizeof(this->front.cache.brush_items[index].pattern),
            this->front.cache.brush_items[index].pattern);
        this->front.orders->send(cmd);
    }


    virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
    {
        RDPColCache cmd(cacheIndex, palette);
        this->front.orders->send(cmd);
    }

    virtual void bitmap_cache(const uint8_t cache_id, const uint16_t cache_idx)
    {
        BitmapCacheItem * entry =  this->front.bmp_cache->get_item(cache_id, cache_idx);

        RDPBmpCache cmd(this->get_front_bpp(), entry->pbmp, cache_id, cache_idx, &(this->get_client_info()));
        this->front.orders->send(cmd);

        if (this->capture){
            this->capture->bitmap_cache(cmd);
        }

    }

    virtual void glyph_cache(const FontChar & font_char, int font_index, int char_index)
    {
        RDPGlyphCache cmd(font_index, 1, char_index, font_char.offset, font_char.baseline, font_char.width, font_char.height, font_char.data);
        this->front.orders->send(cmd);
    }

    virtual void bitmap_update(Bitmap & bitmap, const Rect & dst, int srcx, int srcy)
    {
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const Rect src_r(srcx, srcy, width, height);
        const int palette_id = 0;
        const uint8_t * src_data = bitmap.data_co(this->get_front_bpp());
        const uint8_t rop = 0xCC;

        for (int y = 0; y < dst.cy ; y += 32) {
            int cy = std::min(32, dst.cy - y);
            for (int x = 0; x < dst.cx ; x += 32) {
                int cx = std::min(32, dst.cx - x);
                const Rect tile(x, y, cx, cy);
                if (!this->clip.intersect(tile.offset(dst.x, dst.y)).isempty()){
                     uint32_t cache_ref = this->front.bmp_cache->add_bitmap(
                                                src_r.cx, src_r.cy,
                                                src_data,
                                                tile.offset(src_r.x, src_r.y),
                                                this->get_front_bpp(),
                                                bitmap.original_palette);

                    uint8_t send_type = (cache_ref >> 24);
                    uint8_t cache_id  = (cache_ref >> 16);
                    uint16_t cache_idx = (cache_ref & 0xFFFF);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->bitmap_cache(cache_id, cache_idx);
                    }

                    const RDPMemBlt cmd(cache_id, tile.offset(dst.x, dst.y), rop, 0, 0, cache_idx);
                    if (!this->clip.isempty()
                    && !this->clip.intersect(cmd.rect).isempty()){
                        if (this->get_front_bpp() == 8){
                            if (!this->palette_memblt_sent[palette_id]) {
                                if (bitmap.original_bpp == 8){
                                    this->color_cache(this->mod_palette, palette_id);
                                }
                                else {
                                    this->color_cache(this->palette332, palette_id);
                                }
                                this->palette_memblt_sent[palette_id] = true;
                            }
                            this->palette_sent = false;
                        }
                        this->front.orders->send(cmd, this->clip);
                        #warning capture should have it's own reference to bmp_cache
                        if (this->capture){
                            this->capture->mem_blt(cmd, *this->front.bmp_cache, this->clip);
                        }
                    }
                }
            }
        }
    }

    virtual void set_pointer(int cache_idx)
    {
        this->front.set_pointer(cache_idx);
        this->current_pointer = cache_idx;
    }

    virtual void send_global_palette()
    {
        if (!this->palette_sent && (this->get_front_bpp() == 8)){
            if (this->mod_bpp == 8){
                this->front.send_global_palette(this->memblt_mod_palette);
            }
            else {
                this->front.send_global_palette(this->palette332);
            }
            this->palette_sent = true;
        }
    }

    virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
    {
        int cacheidx = 0;
        switch (this->front.cache.add_pointer(data, mask, x, y, cacheidx)){
        case POINTER_TO_SEND:
            this->front.send_pointer(cacheidx, data, mask, x, y);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cacheidx);
        break;
        }
    }

    virtual void set_mod_palette(const BGRPalette & palette)
    {
        this->mod_palette_setted = true;
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
            this->memblt_mod_palette[i] = RGBtoBGR(palette[i]);
        }
    }

    virtual const BGRColor convert(const BGRColor color) const
    {
        if (this->get_front_bpp() == 8 && this->mod_bpp == 8){
//            return ((color >> 5) & 7) |((color << 1) & 0x31)|((color<<6)&0xc0);
//            this->mod_palette[color]
            return color;
        }
        else{
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, this->get_front_bpp());
        }
    }

    virtual const BGRColor convert_opaque(const BGRColor color) const
    {
        if (this->get_front_bpp() == 8 && this->mod_bpp == 8){
//            LOG(LOG_INFO, "convert_opaque: front=%u back=%u setted=%u color=%u palette=%.06x", this->get_front_bpp(), this->mod_bpp, this->mod_palette_setted, color, this->mod_palette[color]);
//            return ((color >> 5) & 7) |((color << 1) & 0x31)|((color<<6)&0xc0);
//            this->mod_palette[color]
            return color;
        }
        else
        if (this->mod_bpp == 16 || this->mod_bpp == 15 || this->mod_bpp == 8){
            const BGRColor color24 = color_decode_opaquerect(
                        color, this->mod_bpp, this->mod_palette);
            return  color_encode(color24, this->get_front_bpp());
        }
        else {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, this->get_front_bpp());
        }
    }

    virtual const BGRColor convert24_opaque(const BGRColor color) const
    {
        if (this->mod_bpp == 16 || this->mod_bpp == 15){
            const BGRColor color24 = color_decode_opaquerect(
                        color, this->mod_bpp, this->mod_palette);
            return  color_encode(color24, 24);
        }
        else if (this->mod_bpp == 8) {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return RGBtoBGR(color_encode(color24, 24));
        }
        else {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, 24);
        }
    }

    virtual const BGRColor convert24(const BGRColor color) const
    {
        const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
        return color_encode(color24, 24);
    }

    virtual uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }


};


struct client_mod : public Callback {
    GraphicDeviceMod gd;

    bool clipboard_enable;
    bool pointer_displayed;
    int sck;
    char ip_source[256];
    int rdp_compression;
    int bitmap_cache_persist_enable;
    uint8_t socket;

    wait_obj * event;
    int signal;

    client_mod(Front & front)
        : gd(front),
          signal(0)    {
        this->pointer_displayed = false;

    }

    virtual ~client_mod()
    {
        if (this->gd.capture){
            delete this->gd.capture;
        }
    }

    void start_capture(int width, int height, bool flag, char * path,
                const char * codec_id, const char * quality, int timezone)
    {
        this->gd.front.timezone = timezone;
        if (flag){
            this->stop_capture();
            this->gd.capture = new Capture(width, height, 24, path, codec_id, quality);
        }
    }

    void stop_capture()
    {
        if (this->gd.capture){
            delete this->gd.capture;
            this->gd.capture = 0;
        }
    }

    void periodic_snapshot(bool pointer_is_displayed)
    {
        if (this->gd.capture){
            this->gd.capture->snapshot(this->gd.front.mouse_x, this->gd.front.mouse_y,
                    pointer_is_displayed|this->gd.front.nomouse, this->gd.front.notimestamp, this->gd.front.timezone);
        }
    }

    // draw_event is run when mod socket received some data (drawing order),
    // these order could also be auto-generated, say to comply to some refresh.

    // draw_event returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual int draw_event(void) = 0;

    virtual void front_resize() {
    }

    void server_resize(int width, int height, int bpp)
    {
        const ClientInfo & client_info = this->gd.get_client_info();

        if (client_info.width != width
        || client_info.height != height
        || client_info.bpp != bpp) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                return;
            }
            this->gd.palette_sent = false;
            for (size_t i = 0; i < 6 ; i++){
                this->gd.palette_memblt_sent[i] = false;
            }
            LOG(LOG_INFO, "// Resizing client to : %d x %d x %d\n", width, height, bpp);

            this->gd.front.set_client_info(width, height, bpp);
            this->front_resize();
            this->gd.front.reset();
        }

        this->gd.server_reset_clip();
    }

    int server_is_term()
    {
        return g_is_term();
    }

    void server_add_char(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
        struct FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data, data, fi.datasize());
        this->gd.glyph_cache(fi, font, character);
    }

    void server_send_to_channel_mod(const McsChannelItem & channel, uint8_t* data, int length, int flags)
    {
        for (size_t index = 0; index < this->gd.front.get_channel_list().size(); index++){
            const McsChannelItem & front_channel_item = this->gd.front.get_channel_list()[index];
            if (strcmp(channel.name, front_channel_item.name) == 0){
//                LOG(LOG_INFO, "found front channel chanid=%u flags=%x [channel_flags=%x] name=%s", front_channel_item.chanid, flags, front_channel_item.flags, front_channel_item.name);
                this->gd.front.send_to_channel(front_channel_item, data, length, flags);
                break;
            }
        }
    }

    bool get_pointer_displayed() {
        return this->pointer_displayed;
    }

    void set_pointer_display() {
        this->pointer_displayed = true;
    }

    virtual void rdp_input_invalidate(const Rect & r) = 0;
    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) = 0;
    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, const Keymap * keymap, const key_info* ki) = 0;
    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap) = 0;

};

#endif
