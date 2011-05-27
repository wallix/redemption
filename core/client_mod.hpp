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
    uint8_t default_bpp;
    uint8_t mod_bpp;

    wait_obj * event;
    int signal;

    client_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front)
        : keys(keys),
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
    }


    int callback(int msg, long param1, long param2, long param3, long param4)
    {
        //printf("msg=%x param1=%lx param2=%lx param3=%lx param4=%lx\n",msg, param1, param2, param3, param4);
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
            /* invalidate, this is not from RDP_DATA_PDU_INPUT */
            /* like the rest, its from RDP_PDU_DATA with code 33 */
            /* its the rdp client asking for a screen update */
            this->invalidate(Rect(param1, param2, param3, param4));
            break;
        case WM_CHANNELDATA:
            /* called from server_channel.c, channel data has come in,
            pass it to module if there is one */
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
            LOG(LOG_INFO, "Resizing client to : %d x %d x %d\n", width, height, bpp);

            client_info.width = width;
            client_info.height = height;
            client_info.bpp = bpp;

            this->front_resize();

            /* shut down the rdp client */
            this->front->rdp_layer.server_rdp_send_deactive();

            /* this should do the resizing */
            this->front->rdp_layer.server_rdp_send_demand_active();


            this->front->orders.reset_xx();
            this->front->cache.reset(client_info);

            if (this->front->bmp_cache){
                delete this->front->bmp_cache;
            }
            this->front->bmp_cache = new BitmapCache(&client_info);
//          this->front->reset(this->front->orders, this->front->cache, this->front->font);
        }

        this->server_reset_clip();
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
            this->convert(glyph_index.back_color),
            this->convert(glyph_index.fore_color),
            this->clip);
    }

    void scr_blt(const RDPScrBlt & scrblt)
    {
        const int rop = scrblt.rop;
        const Rect & rect = scrblt.rect;
        const int srcx = scrblt.srcx;
        const int srcy = scrblt.srcy;
//        LOG(LOG_INFO, "client_mod::screen_blt(rop=%x, r(%d, %d, %d, %d), srcx=%d, srcy=%d", rop, rect.x, rect.y, rect.cx, rect.cy, srcx, srcy);
        if (!rect.intersect(this->clip).isempty()) {
            this->front->screen_blt(rop, rect, srcx, srcy, this->clip);
        }
    }

    void dest_blt(const RDPDestBlt & destblt)
    {
        const int rop = destblt.rop;
        const Rect & rect = destblt.rect;
//        LOG(LOG_INFO, "client_mod::dest_blt(rop=%x, r(%d, %d, %d, %d)", rop, rect.x, rect.y, rect.cx, rect.cy);
        if (!rect.intersect(this->clip).isempty()) {
            this->front->dest_blt(rect, rop, this->clip);
        }
    }


    void pat_blt(int rop, const Rect & rect, const uint32_t fgcolor, const uint32_t bgcolor)
    {
//        LOG(LOG_INFO, "client_mod::pat_blt(rop=%x, r(%d, %d, %d, %d), fg=%x, bg=%x", rop, rect.x, rect.y, rect.cx, rect.cy, fgcolor, bgcolor);
        if (!rect.intersect(this->clip).isempty()) {
            this->front->pat_blt(rect, rop, this->convert(bgcolor), this->convert(fgcolor), this->brush, this->clip);
        }
    }


    void opaque_rect(const RDPOpaqueRect & opaquerect)
    {
//        LOG(LOG_INFO, "client_mod::opaque_rect(r(%d, %d, %d, %d), color=%x", rect.x, rect.y, rect.cx, rect.cy, color);
        const Rect & rect = opaquerect.rect;
        const uint32_t color = opaquerect.color;
        if (!rect.intersect(clip).isempty()) {

            #warning dirty hack to fix color problems with opaque_rect
            uint32_t color24 = color_decode(color, this->mod_bpp, this->mod_palette);

            if (this->get_front_bpp() == 24){
                color24 = ((color24 << 16) & 0xFF0000)
                        |  (color24 & 0x00FF00)
                        | ((color24 >> 16) & 0x0000FF);
            }

            uint32_t target_color = color_encode(color24,
                                this->get_front_bpp(),
                                this->palette332);


            this->front->opaque_rect(rect, target_color, this->clip);
        }
    }

    #warning move out server_set_pen
    int server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
        return 0;
    }

    void line_to(const RDPLineTo & lineto)
    {
        this->server_set_pen(lineto.pen.style, lineto.pen.width);
        const int rop = lineto.rop2;
        const int x1 = lineto.startx;
        const int y1 = lineto.starty;
        const int x2 = lineto.endx;
        const int y2 = lineto.endy;
        const uint32_t pen_color = lineto.pen.color;
        const uint32_t back_color = lineto.back_color;
        this->pen.color = this->convert(pen_color);
        this->front->line(rop, x1, y1, x2, y2, this->convert(back_color), this->pen, this->clip);
    }


    #warning this should become BITMAP UPDATE, we should be able to send bitmaps either through orders and cache or through BITMAP UPDATE
    void server_paint_rect(Bitmap & bitmap, const Rect & dst, int srcx, int srcy, const RGBPalette & palette)
    {
        #warning color conversion should probably go into bitmap. Something like a copy constructor that change color on the fly ? We may even choose to keep several versions of the same bitmap with different bpp ?
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const uint8_t * src = bitmap.data_co;
        const uint8_t in_bpp = bitmap.bpp;
        const uint8_t out_bpp = this->get_front_bpp();
        uint8_t * bmpdata = (uint8_t*)malloc(width * height * nbbytes(out_bpp));
        uint8_t * dest = bmpdata;
        for (int i = 0; i < width * height; i++) {
            uint32_t pixel = color_decode(in_bytes_le(nbbytes(in_bpp), src),
                                          in_bpp,
                                          palette);
            uint32_t target_pixel = color_encode(pixel, out_bpp, this->palette332);
            target_pixel = 0xFFFFFF & target_pixel;
            out_bytes_le(dest, nbbytes(out_bpp), target_pixel);
            src += nbbytes(in_bpp);
            dest += nbbytes(out_bpp);
        }
        const Rect src_r(srcx, srcy, width, height);
        front->begin_update();
        this->front->send_bitmap_front(dst, src_r, 0xCC, bmpdata, 0, this->clip);
        front->end_update();

        free(bmpdata);
    }

    void mem_blt(const RDPMemBlt & memblt, Bitmap & bitmap, const RGBPalette & palette)
    {
        const Rect & dst = memblt.rect;
        const int srcx = memblt.srcx;
        const int srcy = memblt.srcy;
        #warning color conversion should probably go into bitmap. Something like a copy constructor that change color on the fly ? We may even choose to keep several versions of the same bitmap with different bpp ?
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const uint8_t * src = bitmap.data_co;
        const uint8_t in_bpp = bitmap.bpp;
        const uint8_t out_bpp = this->get_front_bpp();
        uint8_t * bmpdata = (uint8_t*)malloc(width * height * nbbytes(out_bpp));
        uint8_t * dest = bmpdata;
        for (int i = 0; i < width * height; i++) {
            uint32_t pixel = color_decode(in_bytes_le(nbbytes(in_bpp), src),
                                          in_bpp,
                                          palette);
            uint32_t target_pixel = color_encode(pixel, out_bpp, this->palette332);
            target_pixel = 0xFFFFFF & target_pixel;
            out_bytes_le(dest, nbbytes(out_bpp), target_pixel);
            src += nbbytes(in_bpp);
            dest += nbbytes(out_bpp);
        }
        const Rect src_r(srcx, srcy, width, height);
        front->begin_update();
        this->front->send_bitmap_front2(dst, src_r, memblt.rop, bmpdata, 0, this->clip);
        front->end_update();

        free(bmpdata);
    }

    void set_pointer(int cache_idx)
    {
        this->front->set_pointer(cache_idx);
        this->current_pointer = cache_idx;
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
                    this->front->send_brush(cache_idx);
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
        this->front->send_glyph(font, character, offset, baseline, width, height, data);
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
