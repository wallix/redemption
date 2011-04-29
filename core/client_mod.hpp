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

#include "widget.hpp"
#include "client_info.hpp"
#include "font.hpp"
#include "cache.hpp"
#include "front.hpp"
#include "mainloop.hpp"
#include "bitmap_cache.hpp"

struct client_mod {
    #warning there is some overlapping betwwen screen.rect and clip, correct that
    widget_screen screen;
    int (& keys)[256];
    int & key_flags;
    Keymap * &keymap;

    Rect clip;
    int current_pointer;
    RDPPen pen;
    RDPBrush brush;
    bool clipboard_enable;
    bool pointer_displayed;
    struct Front* front;
    int sck;
    vector<struct mcs_channel_item *> channel_list;
    char ip_source[256];
    int rdp_compression;
    int bitmap_cache_persist_enable;
    RGBPalette palette332;
    RGBPalette mod_palette;
    uint8_t default_bpp;
    uint8_t mod_bpp;

    wait_obj * event;
    int signal;

    client_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front)
        : screen(this,
                 front.orders->rdp_layer->client_info.width,
                 front.orders->rdp_layer->client_info.height,
                 front.orders->rdp_layer->client_info.bpp),
          keys(keys),
          key_flags(key_flags),
          keymap(keymap),
          default_bpp(24),
          mod_bpp(24),
          signal(0)
    {
        this->current_pointer = 0;
        this->front = &front;
        this->clip = Rect(0,0,4096,2048);
        this->pointer_displayed = false;

        /* rgb332 palette */
        for (int bindex = 0; bindex < 4; bindex++) {
            for (int gindex = 0; gindex < 8; gindex++) {
                for (int rindex = 0; rindex < 8; rindex++) {
                    this->palette332[(rindex << 5) | (gindex << 2) | bindex] =
                    (RGBcolor)(
                    // r1 r2 r2 r1 r2 r3 r1 r2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                        (((rindex<<5)|(rindex<<2)|(rindex>>1))<<16)
                    // 0 0 0 0 0 0 0 0 g1 g2 g3 g1 g2 g3 g1 g2 0 0 0 0 0 0 0 0
                       | (((gindex<<5)|(gindex<<2)|(gindex>>1))<< 8)
                    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b1 b2 b1 b2 b1 b2
                       | ((bindex<<6)|(bindex<<4)|(bindex<<2)|(bindex)));
                }
            }
        }

    }

    virtual ~client_mod()
    {
        this->screen.delete_all_childs();
    }

    void set_mod_palette(RGBPalette palette)
    {
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
        }
        this->mod_bpp = 8;
    }

    uint32_t convert(uint32_t color)
    {
        uint32_t color24 = color_decode(color, this->mod_bpp, this->mod_palette);
        return color_encode(color24, this->front->orders->rdp_layer->client_info.bpp, this->palette332);
    }

    uint32_t convert_to_black(uint32_t color)
    {
        return 0;
    }

    uint32_t convert_le(uint32_t color)
    {
        uint32_t color24 = color_decode(color, this->mod_bpp, this->mod_palette);

        color24 = (color24 & 0x00FF00)
                | ((color24 >> 16) & 0xFF)
                | ((color24 & 0xFF) << 16);

        return color_encode(color24, this->front->orders->rdp_layer->client_info.bpp, this->palette332);
    }

    /* client functions */
    virtual int mod_event(int msg, long param1, long param2, long param3, long param4) = 0;

    // mod_signal should be run when client socket received some data (mod_event is set)
    // In other words: when some data came from "server" and should be taken care of
    // mod_signal returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual int mod_signal(void) = 0;

    virtual void mod_event_scancode(long param1, long param2, long param3, long param4, int & key_flags, Keymap & keymap, int keys[]){
        param1 = param1 % 128;
        int msg = WM_KEYUP;
        keys[param1] = 1 | param3;
        if ((param3 & KBD_FLAG_UP) == 0) { /* 0x8000 */
            /* key down */
            msg = WM_KEYDOWN;
            switch (param1) {
            case 58:
                key_flags ^= 4;
                break; /* caps lock */
            case 69:
                key_flags ^= 2;
                break; /* num lock */
            case 70:
                key_flags ^= 1;
                break; /* scroll lock */
            default:
                ;
            }
        }
        if (&keymap != NULL)
        {
            struct key_info* ki = keymap.get_key_info_from_scan_code(
                            param3,
                            param1,
                            keys,
                            key_flags);
            if (ki != 0) {
                this->mod_event(msg, ki->chr, ki->sym, param1, param3);
            }
        }
        if (msg == WM_KEYUP){
            keys[param1] = 0;
        }
    }


    widget_screen * get_screen_wdg(){
        return &(this->screen);
    }

    int server_begin_update() {
        this->front->begin_update();
        return 0;
    }

    int server_end_update(){
        this->front->end_update();
        return 0;
    }

    void invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->server_begin_update();
            Rect r(0, 0, this->screen.rect.cx, this->screen.rect.cy);
            this->mod_event(WM_INVALIDATE,
                ((r.x & 0xffff) << 16) | (r.y & 0xffff),
                ((r.cx & 0xffff) << 16) | (r.cy & 0xffff),
                0, 0);

            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    b->Widget_invalidate_clip(r2);
                }
            }
            this->server_end_update();
        }
    }

    void server_resize(int width, int height, int bpp)
    {
        struct ClientInfo & client_info = this->front->orders->rdp_layer->client_info;

        if (client_info.width != width
        || client_info.height != height
        || client_info.bpp != bpp) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                throw 0;
            }
            LOG(LOG_INFO, "Resizing client to : %d x %d x %d\n", width, height, bpp);

            this->screen.rect.cx = client_info.width = width;
            this->screen.rect.cy = client_info.height = height;

            /* shut down the rdp client */
            this->front->orders->rdp_layer->server_rdp_send_deactive();

            /* this should do the resizing */
            this->front->orders->rdp_layer->server_rdp_send_demand_active();

            this->front->orders->reset();
            this->front->cache->reset(&client_info);

            if (this->front->bmp_cache){
                delete this->front->bmp_cache;
            }
            this->front->bmp_cache = new BitmapCache(&client_info);
//          this->front->reset(this->front->orders, this->front->cache, this->front->font);
        }

        this->server_reset_clip();
    }

    Widget * window(int i)
    {
        return this->screen.child_list[i];
    }

    int server_draw_dragging_rect(const Rect & r, const Rect & clip)
    {
        LOG(LOG_INFO, "client_mod::server_draw_dragging_rect");

        this->front->begin_update();
        this->set_domino_brush(0, 0);

        // draw rectangles by drawing each edge top/bottom/left/right
        // 0x66 = xor -> pat_blt( ... 0x5A ...
        // 0xAA = noop -> pat_blt( ... 0xFB ...
        // 0xCC = copy -> pat_blt( ... 0xF0 ...
        // 0x88 = and -> pat_blt( ...  0xC0 ...
        int xor_rop = 0x5A;

        this->front->pat_blt(Rect(r.x, r.y, r.cx, 5),
                             xor_rop, convert_to_black(BLACK), convert_to_black(WHITE),
                             this->brush, clip);
        this->front->pat_blt(Rect(r.x, r.y + (r.cy - 5), r.cx, 5),
                             xor_rop, convert_to_black(BLACK), convert_to_black(WHITE),
                             this->brush, clip);
        this->front->pat_blt(Rect(r.x, r.y + 5, 5, r.cy - 10),
                             xor_rop, convert_to_black(BLACK), convert_to_black(WHITE),
                             this->brush, clip);
        this->front->pat_blt(Rect(r.x + (r.cx - 5), r.y + 5, 5, r.cy - 10),
                             xor_rop, convert_to_black(BLACK), convert_to_black(WHITE),
                             this->brush, clip);

        this->front->end_update();
        return 0;
    }

    void server_draw_text(struct Widget* wdg, int x, int y, const char* text, const uint32_t fgcolor, const Rect & clip){
        setlocale(LC_CTYPE, "fr_FR.UTF-8");
        assert(wdg->type != WND_TYPE_BITMAP);
        int len = mbstowcs(0, text, 0);
        if (len < 1) {
            return;
        }

        const Rect & clip_rect = wdg->to_screen_rect(clip);
        /* convert to wide char */
        wchar_t* wstr = new wchar_t[len + 2];
        mbstowcs(wstr, text, len + 1);
        int k = 0;
        int total_width = 0;
        int total_height = 0;
        uint8_t *data = new uint8_t[len * 4];
        memset(data, 0, len * 4);
        int f = 0;
        int c = 0;
        for (int index = 0; index < len; index++) {
            FontChar* font_item = this->front->font->font_items[wstr[index]];
            switch (this->front->cache->add_glyph(font_item, f, c))
            {
                case Cache::GLYPH_ADDED_TO_CACHE:
                    this->front->send_glyph(font_item, f, c);
                break;
                default:
                break;
            }
            data[index * 2] = c;
            data[index * 2 + 1] = k;
            k = font_item->incby;
            total_width += k;
            total_height = std::max(total_height, font_item->height);
        }

        Rect initial_region = wdg->to_screen_rect(Rect(x, y, total_width, total_height));

        struct Region region;
        region.rects.push_back(initial_region);
        /* loop through all windows in z order */
        for (size_t i = 0; i < this->nb_windows(); i++) {
            Widget *p = this->window(i);
            if (p == wdg || p == &wdg->parent) {
                break;
            }
            region.subtract_rect(p->rect);
        }
        x += wdg->to_screenx();
        y += wdg->to_screeny();

        for (size_t ir = 0 ; ir < region.rects.size(); ir++){
            Rect draw_rect = region.rects[ir].intersect(clip_rect);
            if (!draw_rect.isempty()) {
                const Rect box(0, 0, 0, 0);
                const Rect rect(x-1, y-1, total_width + 1, total_height + 1);
                /* 0x03 0x73; TEXT2_IMPLICIT_X and something else */
                this->front->draw_text2(f, 0x03, 0, box, rect,
                    x, y + total_height,
                    data, len * 2, convert_to_black(fgcolor), convert_to_black(BLACK), clip_rect);
            }
        }
        delete [] data;
        delete [] wstr;
    }

    void screen_blt(int rop, const Rect & rect, int srcx, int srcy)
    {
        LOG(LOG_INFO, "client_mod::screen_blt(rop=%x, r(%d, %d, %d, %d), srcx=%d, srcy=%d", rop, rect.x, rect.y, rect.cx, rect.cy, srcx, srcy);
        if (!rect.intersect(this->clip).isempty()) {
            this->front->screen_blt(0xcc, rect, srcx, srcy, this->clip);
        }
    }

    void dest_blt(int rop, const Rect & rect)
    {
        LOG(LOG_INFO, "client_mod::dest_blt(rop=%x, r(%d, %d, %d, %d)", rop, rect.x, rect.y, rect.cx, rect.cy);
        if (!rect.intersect(this->clip).isempty()) {
            this->front->dest_blt(rect, rop, this->clip);
        }
    }


    void pat_blt(int rop, const Rect & rect, const uint32_t fgcolor, const uint32_t bgcolor)
    {
        LOG(LOG_INFO, "client_mod::pat_blt(rop=%x, r(%d, %d, %d, %d), fg=%x, bg=%x", rop, rect.x, rect.y, rect.cx, rect.cy, fgcolor, bgcolor);
        if (!rect.intersect(this->clip).isempty()) {
            this->front->pat_blt(rect, rop, convert_to_black(bgcolor), convert_to_black(fgcolor), this->brush, this->clip);
        }
    }

    void opaque_rect(const Rect & rect, const uint32_t color)
    {
        LOG(LOG_INFO, "client_mod::opaque_rect(r(%d, %d, %d, %d), color=%x", rect.x, rect.y, rect.cx, rect.cy, color);
        if (!rect.intersect(clip).isempty()) {
            this->front->opaque_rect(rect, convert_to_black(color), this->clip);
        }
    }

    void server_paint_rect(Bitmap & bitmap, const Rect & dst, int srcx, int srcy, const RGBPalette & palette)
    {
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const uint8_t * src = bitmap.data_co;
        const uint8_t in_bpp = bitmap.bpp;
        const uint8_t out_bpp = this->front->orders->rdp_layer->client_info.bpp;
        uint8_t * bmpdata = (uint8_t*)malloc(width * height * nbbytes(out_bpp));
        uint8_t * dest = bmpdata;
        for (int i = 0; i < width * height; i++) {
            uint32_t pixel = color_decode(in_bytes_le(nbbytes(in_bpp), src),
                                          in_bpp,
                                          palette);
            uint32_t target_pixel = color_encode(pixel, out_bpp, this->palette332);
//            target_pixel = 0;
            out_bytes_le(dest, nbbytes(out_bpp), target_pixel);
            src += nbbytes(in_bpp);
            dest += nbbytes(out_bpp);
        }
        const Rect src_r(srcx, srcy, width, height);
        this->front->send_bitmap_front(dst, src_r, bmpdata, 0, this->clip);

        free(bmpdata);
    }

    int send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y)
    {
        try {
            this->front->send_pointer(cache_idx, data, mask, x, y);
        }
        catch (...){
        }
        return 0;
    }

    int set_pointer(int cache_idx)
    {
        try {
            this->front->set_pointer(cache_idx);
            this->current_pointer = cache_idx;
        }
        catch (...){
        }
        return 0;
    }

    void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
    {
        int cacheidx = 0;
        switch (this->front->cache->add_pointer(data, mask, x, y, cacheidx)){
        case POINTER_TO_SEND:
            this->send_pointer(cacheidx, data, mask, x, y);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cacheidx);
        break;
        }
        this->screen.pointer = cacheidx;
    }

    void server_palette(const uint32_t (& palette)[256])
    {
        this->front->send_palette(palette);
    }

    int server_is_term()
    {
        return g_is_term();
    }

    void server_set_clip(const Rect & rect)
    {
        this->clip = rect;
    }

    void server_reset_clip()
    {
        this->clip = this->screen.rect;
    }

    void server_set_brush(const RDPBrush & brush)
    {
        this->brush = brush;

        if (brush.style == 3){
            if (this->front->orders->rdp_layer->client_info.brush_cache_code == 1) {
                uint8_t pattern[8];
                pattern[0] = this->brush.hatch;
                memcpy(pattern+1, this->brush.extra, 7);
                int cache_idx = 0;
                if (BRUSH_TO_SEND == this->front->cache->add_brush(pattern, cache_idx)){
                    this->front->send_brush(cache_idx);
                }
                this->brush.hatch = cache_idx;
                this->brush.style = 0x81;
            }
        }
    }

    int server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
        return 0;
    }

    int text_width(char* text){
        int rv = 0;
        if (text) {
            size_t len = mbstowcs(0, text, 0);
            wchar_t wstr[len + 2];
            mbstowcs(wstr, text, len + 1);
            for (size_t index = 0; index < len; index++) {
                FontChar *font_item = this->front->font->font_items[wstr[index]];
                rv = rv + font_item->incby;
            }
        }
        return rv;
    }

    int text_height(char* text){
        int rv = 0;
        if (text) {
            int len = mbstowcs(0, text, 0);
            wchar_t *wstr = new wchar_t[len + 2];
            mbstowcs(wstr, text, len + 1);
            for (int index = 0; index < len; index++) {
                FontChar *font_item = this->front->font->font_items[wstr[index]];
                rv = std::max(rv, font_item->height);
            }
            delete [] wstr;
        }
        return rv;
    }

    #warning we should be able to pass only one pointer, either window if we are dealing with a window or this->parent if we are dealing with any other kind of widget
    const Region get_visible_region(Widget * window, Widget * widget, const Rect & rect)
    {
        Region region;
        region.rects.push_back(rect);
        /* loop through all windows in z order */
        for (size_t i = 0; i < this->nb_windows(); i++) {
            Widget *p = this->window(i);
            if (p == window || p == widget) {
                break;
            }
            region.subtract_rect(p->rect);
        }
        return region;
    }

    void server_draw_line(int rop, int x1, int y1, int x2, int y2, uint32_t pen_color, uint32_t back_color)
    {
        this->pen.color = convert_to_black(pen_color);
        this->front->line(rop, x1, y1, x2, y2, convert_to_black(back_color), this->pen, this->clip);
    }


    void server_add_char(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
        this->front->send_glyph(font, character, offset, baseline, width, height, data);
    }

    void server_glyph_index(RDPGlyphIndex & glyph_index)
    {
        this->front->draw_text2(
            glyph_index.cache_id,
            glyph_index.fl_accel,
            glyph_index.f_op_redundant,
            glyph_index.op,
            glyph_index.bk,
            glyph_index.glyph_x,
            glyph_index.glyph_y,
            glyph_index.data,
            glyph_index.data_len,
            convert_to_black(glyph_index.back_color),
            convert_to_black(glyph_index.fore_color),
            this->clip);
    }


    int server_get_channel_id(char* name)
    {
        return this->front->get_channel_id(name);
    }

    void server_send_to_channel_mod(int channel_id,
                           uint8_t* data, int data_len,
                           int total_data_len, int flags)
    {
        this->front->orders->rdp_layer->server_send_to_channel(channel_id, data, data_len, total_data_len, flags);


    }

    size_t nb_windows()
    {
        return this->screen.child_list.size();
    }


    void set_domino_brush(int x, int y)
    {
        this->brush.hatch = 0xaa;
        this->brush.extra[0] = 0x55;
        this->brush.extra[1] = 0xaa;
        this->brush.extra[2] = 0x55;
        this->brush.extra[3] = 0xaa;
        this->brush.extra[4] = 0x55;
        this->brush.extra[5] = 0xaa;
        this->brush.extra[6] = 0x55;
        this->brush.org_x = x;
        this->brush.org_y = y;
        this->brush.style = 3;

        // brush style 3 is not supported by windows 7, we **MUST** use cache
        if (this->front->orders->rdp_layer->client_info.brush_cache_code == 1) {
            uint8_t pattern[8];
            pattern[0] = this->brush.hatch;
            memcpy(pattern+1, this->brush.extra, 7);
            int cache_idx = 0;
            if (BRUSH_TO_SEND == this->front->cache->add_brush(pattern, cache_idx)){
                this->front->send_brush(cache_idx);
            }
            this->brush.hatch = cache_idx;
            this->brush.style = 0x81;
        }
    }

    bool get_pointer_displayed() {
        return this->pointer_displayed;
    }

    void set_pointer_display() {
        this->pointer_displayed = true;
    }

};

#endif
