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

#warning server_ naming convention is really confusing, it means internal RDP server, it would be clearer to call that front (like in front.hpp).

struct client_mod : public Callback {
    int (& keys)[256];
    int & key_flags;
    Keymap * &keymap;

    Rect clip;
    int current_pointer;
    RDPPen pen;
    RDPBrush brush;
    bool clipboard_enable;
    bool pointer_displayed;
    struct Front * front;
    int sck;
    vector<struct mcs_channel_item *> channel_list;
    char ip_source[256];
    int rdp_compression;
    int bitmap_cache_persist_enable;
    RGBPalette palette332;
    RGBPalette mod_palette;
    uint8_t mod_bpp;
    uint8_t socket;


    wait_obj * event;
    int signal;
    bool palette_sent;

    client_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front)
        : keys(keys),
          key_flags(key_flags),
          keymap(keymap),
          mod_bpp(24),
          signal(0),
          palette_sent(false)
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
    }


    int callback(int msg, long param1, long param2, long param3, long param4)
    {
        int rv = 0;
        switch (msg) {
        case 0: /* RDP_INPUT_SYNCHRONIZE */
            /* happens when client gets focus and sends key modifier info */
            this->key_flags = param1;
            // why do we not keep device flags ?
            this->mod_event(17, param1, param3, param1, param3);
            break;
        case RDP_INPUT_SCANCODE:
            this->scancode(param1, param2, param3, param4, this->key_flags, *this->keymap, this->keys);
            break;
        case 0x8001: /* RDP_INPUT_MOUSE */
            rv = this->input_mouse(param3, param1, param2);
            break;
        case WM_SCREENUPDATE:
            this->invalidate(Rect(param1, param2, param3, param4));
            break;
        case WM_CHANNELDATA:
            rv = this->mod_event(WM_CHANNELDATA, param1, param2, param3, param4);
            break;
        default:
            break;
        }
        return rv;
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
        return color_encode(color24, this->get_front_bpp(), this->palette332);
    }

    uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }

    /* client functions */
    virtual int mod_event(int msg, long param1, long param2, long param3, long param4) = 0;

    // mod_signal should be run when client socket received some data (mod_event is set)
    // In other words: when some data came from "server" and should be taken care of
    // mod_signal returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual int mod_signal(void) = 0;

    virtual void scancode(long param1, long param2, long param3, long param4, int & key_flags, Keymap & keymap, int keys[]){
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

    int server_begin_update() {
        this->front->begin_update();
        return 0;
    }

    int server_end_update(){
        this->front->end_update();
        return 0;
    }


    const ClientInfo & get_client_info() const {
        return this->front->rdp_layer.client_info;
    }

    int get_front_bpp() const {
        return this->front->rdp_layer.client_info.bpp;
    }

    int get_front_width() const {
        return this->front->rdp_layer.client_info.width;
    }

    int get_front_height() const {
        return this->front->rdp_layer.client_info.height;
    }

    virtual void front_resize() {
    }

    const Rect get_front_rect(){
        return Rect(0, 0, this->get_front_width(), get_front_height());
    }

    void server_resize(int width, int height, int bpp)
    {
        struct ClientInfo & client_info = this->front->rdp_layer.client_info;

        if (client_info.width != width
        || client_info.height != height
        || client_info.bpp != bpp) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                throw 0;
            }
            this->palette_sent = false;
            LOG(LOG_INFO, "// Resizing client to : %d x %d x %d\n", width, height, bpp);

            client_info.width = width;
            client_info.height = height;
            client_info.bpp = bpp;

            this->front_resize();

            /* shut down the rdp client */
            this->front->rdp_layer.server_rdp_send_deactive();

            /* this should do the resizing */
            this->front->rdp_layer.server_rdp_send_demand_active();
            this->front->common = RDPOrderCommon(0,  Rect(0, 0, 1, 1));
            this->front->memblt = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
            this->front->opaquerect = RDPOpaqueRect(Rect(), 0);
            this->front->scrblt = RDPScrBlt(Rect(), 0, 0, 0);
            this->front->destblt = RDPDestBlt(Rect(), 0);
            this->front->patblt = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
            this->front->lineto = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
            this->front->glyphindex = RDPGlyphIndex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"");
            this->front->common.order = RDP::PATBLT;

            this->front->order_count = 0;
            this->front->order_level = 0;

            this->front->cache.reset(client_info);

            if (this->front->bmp_cache){
                delete this->front->bmp_cache;
            }
            this->front->bmp_cache = new BitmapCache(&client_info);
//          this->front->reset(this->front->orders, this->front->cache, this->front->font);
        }

        this->server_reset_clip();
    }


    #warning this function is written in a quite insane way, so don't use it, and rewrite it in a saner way.
    #warning also merge with the similar code in widget.
    #warning implementation of the server_draw_text function below is totally broken, especially data. MS-RDPEGDI See 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)

    void server_draw_text(uint16_t x, uint16_t y, const char * utf8text, uint32_t fgcolor, uint32_t bgcolor)
    {
        // add text to glyph cache
        #warning use mbsrtowcs instead
        int len = mbstowcs(0, utf8text, 0);
        wchar_t* wstr = new wchar_t[len + 2];
        mbstowcs(wstr, utf8text, len + 1);
        int total_width = 0;
        int total_height = 0;
        uint8_t *data = new uint8_t[len * 4];
        memset(data, 0, len * 4);
        int f = 0;
        int c = 0;
        int distance_from_previous_fragment = 0;
        for (int index = 0; index < len; index++) {
            FontChar* font_item = this->front->font.font_items[wstr[index]];
            #warning avoid passing parameters by reference to get results
            switch (this->front->cache.add_glyph(font_item, f, c))
            {
                case Cache::GLYPH_ADDED_TO_CACHE:
                    this->front->glyph_cache(*font_item, f, c);
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

        #warning there seems to be some strange behavior with bk rect (and op ?). Same problem as usual, we have a rectangle but we don't know if boundaries (right, bottom) are included or not. Check actual behavior with rdesktop and with mstsc client. Nevertheless we shouldn't have to add 1 here.
        const Rect bk(x, y, total_width, total_height);

        RDPGlyphIndex text(
            f, // cache_id
            0x03, // fl_accel
            0x0, // ui_charinc
            1, // f_op_redundant,
            bgcolor, // bgcolor
            fgcolor, // fgcolor
            bk, // bk
            Rect(), // op
            this->brush, // brush
            x,  // glyph_x
            y + total_height, // glyph_y
            len * 2, // data_len in bytes
            data // data
        );
        this->server_glyph_index(text);

        delete [] wstr;
        delete [] data;
    }


    void server_glyph_index(const RDPGlyphIndex & cmd)
    {
        RDPGlyphIndex new_cmd = cmd;
        new_cmd.back_color = this->convert(cmd.back_color);
        new_cmd.fore_color = this->convert(cmd.fore_color);

        this->front->glyph_index(new_cmd, this->clip);
    }

    void scr_blt(const RDPScrBlt & scrblt)
    {
        this->front->scr_blt(scrblt, this->clip);
    }

    void dest_blt(const RDPDestBlt & cmd)
    {
        this->front->dest_blt(cmd, this->clip);
    }

    void pat_blt(const RDPPatBlt & cmd)
    {
        RDPPatBlt new_cmd = cmd;
        new_cmd.back_color = this->convert(cmd.back_color);
        new_cmd.fore_color = this->convert(cmd.fore_color);

        this->front->pat_blt(new_cmd, this->clip);
    }

    void opaque_rect(const RDPOpaqueRect & cmd)
    {
        if ((this->get_front_bpp() == 8)
        && !this->palette_sent) {
//            this->front->color_cache(this->palette332);
            this->front->rdp_layer.send_global_palette(this->palette332);
            this->palette_sent = true;
        }

        RDPOpaqueRect new_cmd = cmd;
        new_cmd.color = this->convert(cmd.color);
        this->front->opaque_rect(new_cmd, this->clip);
    }

    #warning move out server_set_pen
    int server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
        return 0;
    }

    void line_to(const RDPLineTo & cmd)
    {
        RDPLineTo new_cmd = cmd;
        new_cmd.back_color = this->convert(cmd.back_color);
        new_cmd.pen.color = this->convert(cmd.pen.color);
        this->front->line_to(new_cmd, this->clip);
    }

    void glyph_index(const RDPGlyphIndex & cmd)
    {
        RDPGlyphIndex new_cmd = cmd;
        this->front->glyph_index(new_cmd, this->clip);
    }

    #warning this should become BITMAP UPDATE, we should be able to send bitmaps either through orders and cache or through BITMAP UPDATE
    void server_paint_rect(Bitmap & bitmap, const Rect & dst, int srcx, int srcy, const RGBPalette & palette)
    {
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const Rect src_r(srcx, srcy, width, height);
        front->begin_update();
        this->front->send_bitmap_front(dst, src_r, 0xCC, bitmap.data_co(this->get_front_bpp()), 0, this->clip);
        front->end_update();
    }

    void mem_blt(const RDPMemBlt & memblt, Bitmap & bitmap, const RGBPalette & palette)
    {
        const Rect & dst = memblt.rect;
        const int srcx = memblt.srcx;
        const int srcy = memblt.srcy;
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const Rect src_r(srcx, srcy, width, height);
        front->begin_update();
        this->front->send_bitmap_front(dst, src_r, memblt.rop, bitmap.data_co(this->get_front_bpp()), 0, this->clip);
        front->end_update();
    }

    void set_pointer(int cache_idx)
    {
        this->front->set_pointer(cache_idx);
        this->current_pointer = cache_idx;
    }

    void send_global_palette(const RGBPalette & palette)
    {
        this->front->send_global_palette(palette);
    }

    void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
    {
        int cacheidx = 0;
        switch (this->front->cache.add_pointer(data, mask, x, y, cacheidx)){
        case POINTER_TO_SEND:
            this->front->send_pointer(cacheidx, data, mask, x, y);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cacheidx);
        break;
        }
    }

    virtual void invalidate(const Rect & rect)
    {
    }

    void color_cache(const uint32_t (& palette)[256])
    {
        this->front->color_cache(palette);
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
        this->clip = this->get_front_rect();
    }

    void server_set_brush(const RDPBrush & brush)
    {
        this->brush = brush;

        if (brush.style == 3){
            if (this->front->rdp_layer.client_info.brush_cache_code == 1) {
                uint8_t pattern[8];
                pattern[0] = this->brush.hatch;
                memcpy(pattern+1, this->brush.extra, 7);
                int cache_idx = 0;
                if (BRUSH_TO_SEND == this->front->cache.add_brush(pattern, cache_idx)){
                    this->front->brush_cache(cache_idx);
                }
                this->brush.hatch = cache_idx;
                this->brush.style = 0x81;
            }
        }
    }

    void server_add_char(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
        struct FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data, data, fi.datasize());
        this->front->glyph_cache(fi, font, character);
    }

    int server_get_channel_id(char* name)
    {
        return this->front->get_channel_id(name);
    }

    void server_send_to_channel_mod(int channel_id,
                           uint8_t* data, int data_len,
                           int total_data_len, int flags)
    {
        this->front->rdp_layer.server_send_to_channel(channel_id, data, data_len, total_data_len, flags);
    }

    bool get_pointer_displayed() {
        return this->pointer_displayed;
    }

    void set_pointer_display() {
        this->pointer_displayed = true;
    }

    int input_mouse(int device_flags, int x, int y)
    {
        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            this->mod_event(WM_MOUSEMOVE, x, y, 0, 0);
            this->front->mouse_x = x;
            this->front->mouse_y = y;

        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            this->mod_event(
                WM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            this->mod_event(
                WM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            this->mod_event(
                WM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
            this->mod_event(WM_BUTTON4DOWN, x, y, 0, 0);
            this->mod_event(WM_BUTTON4UP, x, y, 0, 0);
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
            this->mod_event(WM_BUTTON5DOWN, x, y, 0, 0);
            this->mod_event(WM_BUTTON5UP, x, y, 0, 0);
        }
        return 0;
    }



};

#endif
