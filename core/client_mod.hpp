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
#if !defined(__CLIENT_MOD_HPP__)
#define __CLIENT_MOD_HPP__

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
#include "bmpcache.hpp"
#include "wait_obj.hpp"
#include "keymap.hpp"
#include "callback.hpp"
#include "modcontext.hpp"


struct GraphicDeviceMod : public RDPGraphicDevice
{
    FrontAPI & front;
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

    GraphicDeviceMod(FrontAPI & front)
    : front(front),
      clip(clip),
      palette_sent(false),
      mod_bpp(24),
      mod_palette_setted(0)

    {
        for (size_t i = 0; i < 6 ; i++){
            this->palette_memblt_sent[i] = false;
        }
        this->current_pointer = 0;
        init_palette332(this->palette332);
    }

    void flush() {}

    void text_metrics(const char * text, int & width, int & height){
        height = 0;
        width = 0;
        if (text) {
            size_t len = mbstowcs(0, text, 0);
            wchar_t wstr[len + 2];
            mbstowcs(wstr, text, len + 1);
            for (size_t index = 0; index < len; index++) {
                FontChar *font_item = this->front.font.font_items[wstr[index]];
                width += font_item->incby;
                height = std::max(height, font_item->height);
            }
        }
    }


    int server_begin_update() {
        this->front.begin_update();
        return 0;
    }

    int server_end_update(){
        this->front.end_update();
        return 0;
    }

    int get_front_bpp() const {
        return this->front.get_front_bpp();
    }

    int get_front_width() const {
        return this->front.get_front_width();
    }

    int get_front_height() const {
        return this->front.get_front_height();
    }

     const Rect get_front_rect(){
        return Rect(0, 0, this->get_front_width(), get_front_height());
    }

    void draw_window(const Rect & r, uint32_t bgcolor, const char * caption, bool has_focus, const Rect & clip){

        // Window surface and border
        this->draw(RDPOpaqueRect(r, bgcolor), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + r.cy - 2, r.cx - 2, 1), DARK_GREY), clip);
        this->draw(RDPOpaqueRect(Rect(r.x+r.cx-2, r.y + 1, 1, r.cy), DARK_GREY), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), BLACK), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), BLACK), clip);

        // Title bar
        this->draw(RDPOpaqueRect(Rect(r.x + 3, r.y + 3, r.cx - 5, 18),
                          has_focus?WABGREEN:DARK_GREY), clip);

        this->server_draw_text(r.x + 4, r.y + 4, caption,
                has_focus?WABGREEN:DARK_GREY,
                has_focus?WHITE:BLACK, clip);
    }

    void draw_combo(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip)
    {
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, r.cy), GREY), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 3, r.cy - 3), WHITE), clip);
        if (has_focus) {
            this->draw(RDPOpaqueRect(Rect(r.x + 3, r.y + 3, (r.cx - 6) - 18, r.cy - 5), DARK_WABGREEN), clip);
        }
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), DARK_GREY), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), DARK_GREY), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy- 1, r.cx, 1), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), BLACK), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), BLACK), clip);
        this->server_draw_text(r.x + 4, r.y + 3, caption, has_focus?DARK_WABGREEN:WHITE, has_focus?WHITE:BLACK, clip);
        this->draw_button(Rect(r.x + r.cx - 20, r.y + 2, 18, r.cy - 4), "", state, false, clip);
    }

    void draw_button(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip){

        int bevel = (state == BUTTON_STATE_DOWN)?1:0;

        this->draw(RDPOpaqueRect(r, GREY), clip);
        if (state == BUTTON_STATE_DOWN) {
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        } else {
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), WHITE), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), WHITE), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        }
        int w = 0;
        int h = 0;
        this->text_metrics(caption, w, h);
        this->server_draw_text(
            r.x + r.cx / 2 - w / 2 + bevel,
            r.y + r.cy / 2 - h / 2 + bevel,
            caption, GREY, BLACK, clip);
        // focus rect
        if (has_focus) {
            this->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->draw(
                RDPPatBlt(Rect(r.x + r.cx - 6 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + r.cy - 6 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
        }
    }

    void draw_edit(const Rect & r, char password_char, char * buffer, size_t edit_pos, bool has_focus, const Rect & clip){
        this->draw(RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), DARK_GREEN), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE), clip);

        /* draw text */
        char text[255];
        wchar_t wtext[255];

        if (password_char != 0) {
            int i = mbstowcs(0, buffer, 0);
            memset(text, password_char, i);
            text[i] = 0;
            this->server_draw_text(r.x + 4, r.y + 2, text, DARK_GREEN, LIGHT_GREEN, clip);
        }
        else {
            this->server_draw_text(r.x + 4, r.y + 2, buffer, DARK_GREEN, LIGHT_GREEN, clip);
        }
        /* draw xor box(cursor) */
        if (has_focus) {
            if (password_char != 0) {
                for (size_t index = 0; index < edit_pos; index++) {
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

            int w = 0;
            int h = 0;
            this->text_metrics(text, w, h);
            this->draw(
                RDPOpaqueRect(Rect(r.x + 4 + w, r.y + 3, 2, r.cy - 6), PALE_GREEN),
                clip);
        }
    }

    TODO(" implementation of the server_draw_text function below is quite broken (a small subset of possibilities is implemented  especially for data). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)")
    void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        setlocale(LC_CTYPE, "fr_FR.UTF-8");
        this->send_global_palette();

        // add text to glyph cache
        TODO(" use mbsrtowcs instead")
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
            TODO(" avoid passing parameters by reference to get results")
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

        this->draw(glyphindex, clip);

        delete [] wstr;
        delete [] data;
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPOpaqueRect new_cmd = cmd;
            new_cmd.color = this->convert_opaque(cmd.color);
            this->front.orders->draw(new_cmd, clip);

            if (this->front.capture){
                RDPOpaqueRect new_cmd24 = cmd;
                new_cmd24.color = this->convert24_opaque(cmd.color);
                this->front.capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){
            this->front.orders->draw(cmd, clip);

            if (this->front.capture){
                this->front.capture->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){
            this->front.orders->draw(cmd, clip);
            if (this->front.capture){
                this->front.capture->draw(cmd, clip);
            }
        }
    }


    void cache_brush(RDPBrush & brush)
    {
        if ((brush.style == 3)
        && (this->front.get_front_brush_cache_code() == 1)) {
            uint8_t pattern[8];
            pattern[0] = brush.hatch;
            memcpy(pattern+1, brush.extra, 7);
            int cache_idx = 0;
            if (BRUSH_TO_SEND == this->front.cache.add_brush(pattern, cache_idx)){
                RDPBrushCache cmd(cache_idx, 1, 8, 8, 0x81,
                    sizeof(this->front.cache.brush_items[cache_idx].pattern),
                    this->front.cache.brush_items[cache_idx].pattern);
                this->front.orders->draw(cmd);
            }
            brush.hatch = cache_idx;
            brush.style = 0x81;
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty()
        && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPPatBlt new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.fore_color = this->convert(cmd.fore_color);

            TODO("Shouldn't this be done before calling draw");
            // this may change the brush add send it to to remote cache
            this->cache_brush(new_cmd.brush);

            this->front.orders->draw(new_cmd, clip);

            if (this->front.capture){
                RDPPatBlt new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.fore_color = this->convert24(cmd.fore_color);

                this->front.capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap)
    {
        if (bitmap.cx < cmd.srcx
        ||  bitmap.cy < cmd.srcy){
            return;
        }

        const uint8_t palette_id = 0;
        if (this->get_front_bpp() == 8){
            this->palette_sent = false;
            this->send_global_palette();
            if (!this->palette_memblt_sent[palette_id]) {
                this->color_cache(bitmap.original_palette, palette_id);
                this->palette_memblt_sent[palette_id] = true;
            }
            this->palette_sent = false;
        }

        const uint16_t dst_x = cmd.rect.x;
        const uint16_t dst_y = cmd.rect.y;
        // clip dst as it can be larger than source bitmap
        const uint16_t dst_cx = std::min<uint16_t>(bitmap.cx - cmd.srcx, cmd.rect.cx);
        const uint16_t dst_cy = std::min<uint16_t>(bitmap.cy - cmd.srcy, cmd.rect.cy);

        for (int y = 0; y < dst_cy ; y += 32) {
            int cy = std::min(32, dst_cy - y);

            for (int x = 0; x < dst_cx ; x += 32) {
                int cx = std::min(32, dst_cx - x);

                const Rect dst_tile(dst_x + x, dst_y + y, cx, cy);
                const Rect src_tile(cmd.srcx + x, cmd.srcy + y, cx, cy);

                const Bitmap tiled_bmp(bitmap, src_tile);

                const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);

                this->front.draw(
                    RDPMemBlt(0, dst_tile, cmd.rop, 0, 0, 0),
                    clip,
                    Bitmap(bitmap, src_tile));
            }
        }
    }

    void server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
    }

    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        const uint16_t minx = std::min(cmd.startx, cmd.endx);
        const uint16_t miny = std::min(cmd.starty, cmd.endy);
        const Rect rect(minx, miny,
                        std::max(cmd.startx, cmd.endx)-minx+1,
                        std::max(cmd.starty, cmd.endy)-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()){

            RDPLineTo new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.pen.color = this->convert(cmd.pen.color);

            this->front.orders->draw(new_cmd, clip);

            if (this->front.capture){
                RDPLineTo new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.pen.color = this->convert24(cmd.pen.color);

                this->front.capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.bk).isempty()){
            this->send_global_palette();

            RDPGlyphIndex new_cmd = cmd;
            new_cmd.back_color = this->convert_opaque(cmd.back_color);
            new_cmd.fore_color = this->convert_opaque(cmd.fore_color);

            TODO("Shouldn't this be done before calling draw");
            // this may change the brush and send it to to remote cache
            this->cache_brush(new_cmd.brush);

            this->front.orders->draw(new_cmd, clip);

            if (this->front.capture){
                RDPGlyphIndex new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24_opaque(cmd.back_color);
                new_cmd24.fore_color = this->convert24_opaque(cmd.fore_color);

                this->front.capture->draw(new_cmd24, clip);
            }
        }
    }

    void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
    {
        RDPColCache cmd(cacheIndex, palette);
        this->front.orders->draw(cmd);
    }

    void glyph_cache(const FontChar & font_char, int font_index, int char_index)
    {
        RDPGlyphCache cmd(font_index, 1, char_index, font_char.offset, font_char.baseline, font_char.width, font_char.height, font_char.data);
        this->front.orders->draw(cmd);
    }


    void set_pointer(int cache_idx)
    {
        this->front.set_pointer(cache_idx);
        this->current_pointer = cache_idx;
    }

    void send_global_palette()
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

    void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
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

    void set_mod_palette(const BGRPalette & palette)
    {
        this->mod_palette_setted = true;
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
            this->memblt_mod_palette[i] = RGBtoBGR(palette[i]);
        }
    }

    const BGRColor convert(const BGRColor color) const
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

    const BGRColor convert_opaque(const BGRColor color) const
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

    const BGRColor convert24_opaque(const BGRColor color) const
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

    const BGRColor convert24(const BGRColor color) const
    {
        const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
        return color_encode(color24, 24);
    }

    uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }


};


enum BackEvent_t {
    BACK_EVENT_NONE = 0,
    BACK_EVENT_1,
    BACK_EVENT_2,
    BACK_EVENT_3,
    BACK_EVENT_STOP = 4,
    BACK_EVENT_5,
    BACK_EVENT_REFRESH,
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
    BackEvent_t signal;

    client_mod(FrontAPI & front)
        : gd(front),
          signal(BACK_EVENT_NONE)    {
        this->pointer_displayed = false;

    }

    virtual ~client_mod()
    {
        if (this->gd.front.capture){
            delete this->gd.front.capture;
        }
    }

    void start_capture(int width, int height, bool flag, char * path,
                const char * codec_id, const char * quality)
    {
        if (flag){
            this->stop_capture();
            this->gd.front.capture = new Capture(width, height, 24, this->gd.palette332,
                                           path, codec_id, quality);
        }
    }

    void stop_capture()
    {
        if (this->gd.front.capture){
            delete this->gd.front.capture;
            this->gd.front.capture = 0;
        }
    }

    void periodic_snapshot(bool pointer_is_displayed)
    {
        if (this->gd.front.capture){
            this->gd.front.capture->snapshot(this->mouse_x, this->mouse_y,
                    pointer_is_displayed|this->gd.front.nomouse, this->gd.front.notimestamp);
        }
    }

    // draw_event is run when mod socket received some data (drawing order),
    // these order could also be auto-generated, say to comply to some refresh.

    // draw_event returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual BackEvent_t draw_event(void) = 0;

    virtual void refresh_context(ModContext & context)
    {
        return; // used when context changed to avoid creating a new module
    }

    virtual void front_resize() {
    }

    void server_resize(int width, int height, int bpp)
    {
        int client_info_width = this->gd.front.get_front_width();
        int client_info_height = this->gd.front.get_front_height();
        int client_info_bpp = this->gd.front.get_front_bpp();
        int client_info_build = this->gd.front.get_front_build();

        if (client_info_width != width
        || client_info_height != height
        || client_info_bpp != bpp) {
            /* older client can't resize */
            if (client_info_build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                return;
            }
            this->gd.palette_sent = false;
            for (size_t i = 0; i < 6 ; i++){
                this->gd.palette_memblt_sent[i] = false;
            }
            LOG(LOG_INFO, "// Resizing client to : %d x %d x %d\n", width, height, bpp);

            this->gd.front.set_front_resolution(width, height, bpp);
            TODO("This warns modules that the resolution has changed, it is only used in internal modules.")
            this->front_resize();
//            this->gd.front.reset();
        }
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

    void send_to_front_channel(const char * const mod_channel_name, uint8_t* data, size_t length, size_t chunk_size, int flags)
    {
        const McsChannelItem * front_channel = this->gd.front.get_channel_list().get(mod_channel_name);
        if (front_channel){
            this->gd.front.send_to_channel(*front_channel, data, length, chunk_size, flags);
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
