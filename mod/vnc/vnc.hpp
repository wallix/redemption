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

   Vnc module

*/

#if !defined(__VNC_HPP__)
#define __VNC_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "../../core/colors.hpp"

#include "stream.hpp"
#include "d3des.hpp"

struct mod_vnc : public client_mod {
    /* mod data */
    char mod_name[256];
    int mod_mouse_state;
    RGBPalette palette;
    int vnc_desktop;
    char username[256];
    char password[256];
    public:
    Transport *t;
    int shift_state; /* 0 up, 1 down */
    int keylayout;
    int clip_chanid;
    Stream clip_data;
    int clip_data_size;

    mod_vnc(Transport * t,
            int (& keys)[256], int & key_flags, Keymap * &keymap,
            struct ModContext & context, struct Front & front, int keylayout)
        :
        client_mod(keys, key_flags, keymap, front)
    {
        const char * password = context.get(STRAUTHID_TARGET_PASSWORD);
        const char * username = context.get(STRAUTHID_TARGET_USER);
        this->t = t;
        try {
            int bpp = this->screen.bpp;
            memset(this->mod_name, 0, 256);
            this->mod_mouse_state = 0;
            memset(this->palette, 0, sizeof(RGBPalette));
            this->vnc_desktop = 0;
            memset(this->username, 0, 256);
            memset(this->password, 0, 256);

            this->shift_state = 0; /* 0 up, 1 down */
            this->keylayout = 0;
            this->clip_chanid = 0;
            this->clip_data_size = 0;

            strcpy(this->username, username);
            strcpy(this->password, password);
            // not used for vnc
            // strcpy(this->hostname, hostname);
            if (0 != keylayout) { this->keylayout = keylayout; }

            uint8_t cursor_data[32 * (32 * 3)];
            uint8_t cursor_mask[32 * (32 / 8)];
            int error = 0;
            int i;
            int check_sec_result;

            check_sec_result = 1;
            if ((bpp != 8) && (bpp != 15) && (bpp != 16) && (bpp != 24)) {
                LOG(LOG_INFO, "error - only supporting 8, 15, 16 and 24 bpp rdp connections\n");
                throw Error(ERR_VNC_BAD_BPP);
            }

            try {
                /* protocol version */
                Stream stream(8192);
                this->t->recv((char**)&stream.end, 12);
                this->t->send("RFB 003.003\n", 12);

                /* sec type */
                stream.init(8192);
                this->t->recv((char**)&stream.end, 4);
                int security_level = stream.in_uint32_be();
                LOG(LOG_INFO, "security level is %d "
                              "(1 = none, 2 = standard)\n",
                              security_level);
                switch (security_level){
                    case 1: /* none */
                        break;
                    case 2: /* dec the password and the server random */
                        stream.init(8192);
                        this->t->recv((char**)&stream.end, 16);
                        this->rfbEncryptBytes(stream.data, this->password);
                        this->t->send((char*)stream.data, 16);

                        /* sec result */
                        stream.init(8192);
                        this->t->recv((char**)&stream.end, 4);
                        i = stream.in_uint32_be();
                        if (i != 0) {
                            LOG(LOG_INFO, "vnc password failed\n");
                            throw 2;
                        } else {
                            LOG(LOG_INFO, "vnc password ok\n");
                        }
                        break;
                    default:
                        throw 1;
                }

                stream.init(8192);
                stream.data[0] = 1;
                #warning send and recv should be stream aware
                #warning we should always send at stream.p, not stream.data
                this->t->send((char*)stream.data, 1); /* share flag */

                stream.init(8192);
                #warning send and recv should be stream aware
                this->t->recv((char**)&stream.end, 4); /* server init */
                int width = stream.in_uint16_be();
                int height = stream.in_uint16_be();
                this->server_set_clip(Rect(0, 0, width, height));

                stream.init(8192); /* pixel format */
                #warning send and recv should be stream aware
                this->t->recv((char**)&stream.end, 16);
                #warning why do we not use what is received ?

                stream.init(8192);
                this->t->recv((char**)&stream.end, 4); /* name len */

                i = stream.in_uint32_be();
                if (i > 255 || i < 0) {
                    throw 3;
                }
                char * end = this->mod_name;
                this->t->recv(&end, i);
                this->mod_name[i] = 0;

                /* should be connected */

                /* SetPixelFormat */
                stream.init(8192);
                stream.out_uint8(0);
                stream.out_uint8(0);
                stream.out_uint8(0);
                stream.out_uint8(0);

                stream.out_copy_bytes(get_pixel_format(bpp), 16);
                this->t->send((char*)stream.data, 20);

                /* SetEncodings */
                stream.init(8192);
                stream.out_uint8(2);
                stream.out_uint8(0);
                stream.out_uint16_be(3);
                stream.out_uint32_be(0); /* raw */
                stream.out_uint32_be(1); /* copy rect */
                stream.out_uint32_be(0xffffff11); /* cursor */

                this->t->send((char*)stream.data, 4 + 3 * 4);

                this->server_resize(width, height, bpp);

                /* FrambufferUpdateRequest */
                stream.init(8192);
                stream.out_uint8(3);
                stream.out_uint8(0);
                #warning we could create some out_rect primitive at stream level
                stream.out_uint16_be(0);
                stream.out_uint16_be(0);
                stream.out_uint16_be(width);
                stream.out_uint16_be(height);

                // sending framebuffer update request
                this->t->send((char*)stream.data, 10);

                /* set almost null cursor, this is the little dot cursor */
                memset(cursor_data, 0, 32 * (32 * 3));
                memset(cursor_data + (32 * (32 * 3) - 1 * 32 * 3), 0xff, 9);
                memset(cursor_data + (32 * (32 * 3) - 2 * 32 * 3), 0xff, 9);
                memset(cursor_data + (32 * (32 * 3) - 3 * 32 * 3), 0xff, 9);
                memset(cursor_mask, 0xff, 32 * (32 / 8));

                // sending cursor
                this->server_set_pointer(3, 3, cursor_data, cursor_mask);
            } catch(int i) {
                error = i;
            } catch(...) {
                error = 1;
            };

            if (error) {
                LOG(LOG_INFO, "error - problem connecting\n");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            LOG(LOG_INFO, "VNC connection complete, connected ok\n");
            this->lib_open_clip_channel();
        } catch(...){
            delete this->t;
            throw;
        }
    }

    virtual ~mod_vnc(){
        delete this->t;
    }


    static const char * get_pixel_format(int bpp){
    // VNC pixel_format capabilities
    // -----------------------------
    // bits per pixel  : 1 byte
    // color depth     : 1 byte
    // endianess       : 1 byte (0 = LE, 1 = BE)
    // true color flag : 1 byte
    // red max         : 2 bytes
    // green max       : 2 bytes
    // blue max        : 2 bytes
    // red shift       : 1 bytes
    // green shift     : 1 bytes
    // blue shift      : 1 bytes
    // padding         : 3 bytes

        switch (bpp){
        case 8:
            return "\x08\x08\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\0\0\0";
        break;
        case 15:
            return "\x10\x0F\x00"
            "\x01\x00\x1F\x00\x1F\x00\x1F\x0A\x05\x00"
            "\0\0\0";
        break;
        case 16:
            return "\x10\x10\x00"
            "\x01\x00\x1F\x00\x2F\x00\x1F\x0B\x05\x00"
            "\0\0\0";
        break;
        default:
        case 24:
            return "\x20\x18\x00"
            "\x01\x00\xFF\x00\xFF\x00\xFF\x10\x08\x00"
            "\0\0\0";
        break;
        }
    }

    virtual int mod_event(int msg, long param1, long param2, long param3, long param4)
    {
        int error = 0;

        Stream stream(8192);
        switch (msg){
        case WM_CHANNELDATA:
        {
            int chanid = param1  & 0xffff;
            int flags =  (param1 & 0xffff0000) >> 16;
            int size = (int)param2;
            char *data = (char*)param3;
            int total_size = (int)param4;
            if ((size >= 0) && (size <= (32 * 1024)) && (data != 0)) {
                stream.init(size);
                stream.out_copy_bytes(data, size);
                stream.mark_end();
                stream.p = stream.data;
                error = this->lib_process_channel_data(chanid, flags, size, &stream, total_size);
            } else {
                error = 1;
            }
        }
        break;
        case WM_KEYDOWN:
        case WM_KEYUP:
        { /* key events */
            int key = param2;
            if (key > 0) {
                stream.out_uint8(4);
                stream.out_uint8(msg == 15); /* down flag */
                stream.out_clear_bytes(2);
                stream.out_uint32_be(key);
                try {
                    this->t->send((char*)stream.data, 8);
                    error = 0;
                } catch (...) {
                    error = 1;
                }
            }
        }
        break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_BUTTON3UP:
        case WM_BUTTON3DOWN:
        case WM_BUTTON4UP:
        case WM_BUTTON4DOWN:
        case WM_BUTTON5UP:
        case WM_BUTTON5DOWN:
        #warning switch below should be easy to simplify using some math it is basically clear_bit/set_bit
            switch (msg) {
            case WM_MOUSEMOVE:
                break;
            case WM_LBUTTONUP:
                this->mod_mouse_state &= ~1;
                break;
            case WM_LBUTTONDOWN:
                this->mod_mouse_state |= 1;
                break;
            case WM_RBUTTONUP:
                this->mod_mouse_state &= ~4;
                break;
            case WM_RBUTTONDOWN:
                this->mod_mouse_state |= 4;
                break;
            case WM_BUTTON3UP:
                this->mod_mouse_state &= ~2;
                break;
            case WM_BUTTON3DOWN:
                this->mod_mouse_state |= 2;
                break;
            case WM_BUTTON4UP:
                this->mod_mouse_state &= ~8;
                break;
            case WM_BUTTON4DOWN:
                this->mod_mouse_state |= 8;
                break;
            case WM_BUTTON5UP:
                this->mod_mouse_state &= ~16;
                break;
            case WM_BUTTON5DOWN:
                this->mod_mouse_state |= 16;
                break;
            }
            stream.out_uint8(5);
            stream.out_uint8(this->mod_mouse_state);
            stream.out_uint16_be(param1);
            stream.out_uint16_be(param2);
            try {
                this->t->send((char*)stream.data, 6);
                error = 0;
            } catch (...) {
                error = 1;
            }
        break;
        case WM_INVALIDATE:
        { /* invalidate */
            /* FrambufferUpdateRequest */
            stream.out_uint8(3);
            stream.out_uint8(0);
            int x = (param1 >> 16) & 0xffff;
            stream.out_uint16_be(x);
            int y = param1 & 0xffff;
            stream.out_uint16_be(y);
            int cx = (param2 >> 16) & 0xffff;
            stream.out_uint16_be(cx);
            int cy = param2 & 0xffff;
            stream.out_uint16_be(cy);

            try {
                this->t->send((char*)stream.data, 10);
                error = 0;
            } catch (...) {
                error = 1;
            }
        }
        break;
        default:
            LOG(LOG_WARNING, "unexpected message %d\n", msg);
            break;
        }
        return error;
    }

    virtual int mod_signal(void)
    {
        Stream stream(1);
        int rv = 0;

        try {
            this->t->recv((char**)&stream.end, 1);
            char type = stream.in_uint8();
            switch (type)
            {
                case 0: /* framebuffer update */
                    rv = this->lib_framebuffer_update();
                break;
                case 1: /* palette */
                    rv = this->lib_palette_update();
                break;
                case 3: /* clipboard */
                    rv = this->lib_clip_data();
                break;
                default:
                LOG(LOG_INFO, "unknown in vnc_lib_mod_signal %d\n", type);
            }
        }
        catch(...) {
            rv = 1;
        }
        return rv;
    }

    private:
    #warning does it work ? Seems designed for copy/paste
    int lib_process_channel_data(int chanid, int flags, int size, Stream* s, int total_size)
    {
        int type;
        int status;
        int length;
        int index;
        int format;

        if (chanid == this->clip_chanid) {
            type = s->in_uint16_le();
            status = s->in_uint16_le();
            length = s->in_uint32_le();
            switch (type) {
            case 2: { /* CLIPRDR_FORMAT_ANNOUNCE */
                Stream* out_s = new Stream(8192);
                out_s->out_uint16_le(3);
                out_s->out_uint16_le(1);
                out_s->out_uint32_le(0);
                out_s->out_clear_bytes(4); /* pad */
                out_s->mark_end();
                length = (int)(out_s->end - out_s->data);
                this->server_send_to_channel_mod(this->clip_chanid, out_s->data, length, length, 3);
                delete out_s;
            }
            break;
            case 3: /* CLIPRDR_FORMAT_ACK */
                break;
            case 4: { /* CLIPRDR_DATA_REQUEST */
                format = 0;
                if (length >= 4) {
                    format = s->in_uint32_le();
                }
                /* only support CF_TEXT and CF_UNICODETEXT */
                if ((format != 1) && (format != 13)) {
                    break;
                }
                Stream* out_s = new Stream(8192);
                out_s->out_uint16_le(5);
                out_s->out_uint16_le(1);
                if (format == 13) { /* CF_UNICODETEXT */
                    out_s->out_uint32_le( this->clip_data_size * 2 + 2);
                    for (index = 0; index < this->clip_data_size; index++) {
                        out_s->out_uint8(this->clip_data.data[index]);
                        out_s->out_uint8(0);
                    }
                    out_s->out_clear_bytes(2);
                } else if (format == 1) { /* CF_TEXT */
                    out_s->out_uint32_le(this->clip_data_size + 1);
                    for (index = 0; index < this->clip_data_size; index++) {
                        out_s->out_uint8(this->clip_data.data[index]);
                    }
                    out_s->out_clear_bytes( 1);
                }
                out_s->out_clear_bytes( 4); /* pad */
                out_s->mark_end();
                length = (int)(out_s->end - out_s->data);
                this->server_send_to_channel_mod(this->clip_chanid, out_s->data, length,
                                       length, 3);
                delete out_s;
            }
            break;
            }
        } else {
            printf("lib_process_channel_data: unknown chanid %d this->clip_chanid %d\n",
                      chanid, this->clip_chanid);
        }
        return 0;
    }

    static void build_pointer(
        uint8_t cursor_data[32 * (32 * 3)],
        uint8_t cursor_mask[32 * (32 / 8)],
        const uint8_t * d1, const uint8_t * d2, int cx, int cy, int bpp, const RGBPalette & palette)
    {
        int r;
        int g;
        int b;
        int pixel;
        memset(cursor_data, 0, 32 * (32 * 3));
        memset(cursor_mask, 0, 32 * (32 / 8));

        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                pixel = get_pixel_safe(d2, k, 31 - j, cx, cy, 1);
                set_pixel_safe(cursor_mask, k, j, 32, 32, 1, !pixel);
                if (pixel) {
                    pixel = get_pixel_safe(d1, k, 31 - j, cx, cy, bpp);
                    split_color(pixel, &r, &g, &b, bpp, palette);
                    pixel = color24RGB(r, g, b);
                    set_pixel_safe(cursor_data, k, j, 32, 32, 24, pixel);
                }
            }
        }
    }

    static int get_pixel_safe(const uint8_t* data, int x, int y, int width, int height, int bpp)
    {
        int start;
        int shift;

        if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) {
            return 0;
        }
        switch (bpp){
        case 1:
            width = nbbytes(width);
            start = (y * width) + x / 8;
            shift = x % 8;
            return (data[start] & (0x80 >> shift)) != 0;
        case 4:
            width = (width + 1) / 2;
            start = y * width + x / 2;
            shift = x % 2;
            return (shift == 0)?(data[start] & 0xf0) >> 4:data[start] & 0x0f;
        case 8:
            return *(((unsigned char*)data) + (y * width + x));
        case 15: case 16:
            return *(((unsigned short*)data) + (y * width + x));
        case 24: case 32:
            return *(((unsigned int*)data) + (y * width + x));
        default:
            LOG(LOG_ERR,"error in get_pixel_safe bpp %d\n", bpp);
        }
        return 0;
    }

    static void set_pixel_safe(uint8_t* data, int x, int y, int width, int height, int bpp,
                   int pixel)
    {
        if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) {
            return;
        }
        switch (bpp){
        case 1:
        {
            width = nbbytes(width);
            int start = (y * width) + x / 8;
            int shift = x % 8;
            if (pixel & 1) {
                data[start] = data[start] | (0x80 >> shift);
            } else {
                data[start] = data[start] & ~(0x80 >> shift);
            }
        }
        break;
        case 15: case 16:
            *(((unsigned short*)data) + (y * width + x)) = pixel;
        break;
        case 24:
            *(data + (3 * (y * width + x)) + 0) = pixel >> 0;
            *(data + (3 * (y * width + x)) + 1) = pixel >> 8;
            *(data + (3 * (y * width + x)) + 2) = pixel >> 16;
        break;
        default:
            LOG(LOG_ERR,"error in set_pixel_safe bpp %d\n", bpp);
        }
    }

    #warning merge into build pointer
    static int split_color(int pixel, int * pr, int* pg, int* pb, int bpp, const RGBPalette & palette)
    {
        uint8_t r = *pr;
        uint8_t g = *pg;
        uint8_t b = *pb;

        switch (bpp) {
        case 8:
            if (pixel >= 0 && pixel < 256) {
                splitcolor32RGB(r, g, b, palette[pixel]);
            }
            else {
                LOG(LOG_ERR, "error in split_color, pixel value outside palette (bpp=%d, pixel=%d)\n", bpp, pixel);
                r = g = b = 0;
            }
        break;
        case 15:
            splitcolor15(r, g, b, pixel);
        break;
        case 16:
            splitcolor16(r, g, b, pixel);
        break;
        case 24: case 32:
            splitcolor32RGB(r, g, b, pixel);
        break;
        default:
            LOG(LOG_ERR, "error in split_color bpp %d\n", bpp);
        }
        *pr = r;
        *pg = g;
        *pb = b;
        return 0;
    }

    int lib_framebuffer_update() throw (Error)
    {
        int encoding;
        int error = 0;
        size_t num_recs = 0;
        int Bpp = nbbytes(this->screen.bpp);
        if (Bpp == 3) {
            Bpp = 4;
        }
        try {
                {
                    Stream stream(8192);
                    this->t->recv((char**)&stream.end, 3);
                    stream.skip_uint8(1);
                    num_recs = stream.in_uint16_be();
                }

            this->server_begin_update();

            for (size_t i = 0; i < num_recs; i++) {
                Stream stream(8192);
                this->t->recv((char**)&stream.end, 12);
                int x = stream.in_uint16_be();
                int y = stream.in_uint16_be();
                int cx = stream.in_uint16_be();
                int cy = stream.in_uint16_be();
                encoding = stream.in_uint32_be();
                switch (encoding){
                case 0: /* raw */
                {
                    int need_size = cx * cy * Bpp;
                    Stream raw(need_size);
                    this->t->recv((char**)&raw.end, need_size);
                    if (0 != this->server_paint_rect(0xcc, Rect(x, y, cx, cy), raw.data, cx, cy, 0, 0))
                    {
                        throw Error(ERR_SERVER_PAINT_RECT);
                    }
                }
                break;
                case 1: /* copy rect */
                {
                    Stream stream(4);
                    this->t->recv((char**)&stream.end, 4);
                    int srcx = stream.in_uint16_be();
                    int srcy = stream.in_uint16_be();
                    if (0 != this->server_screen_blt(0xcc, Rect(x, y, cx, cy), srcx, srcy)){
                        throw Error(ERR_SERVER_SCREEN_BLT);
                    }
                }
                break;
                case 0xffffff11: /* cursor */
                {
                    int j = cx * cy * Bpp;
                    int k = nbbytes(cx) * cy;
                    Stream stream(j + k);
                    this->t->recv((char**)&stream.end, j + k);
                    const uint8_t *d1 = stream.in_uint8p(j);
                    const uint8_t *d2 = stream.in_uint8p(k);


                    #warning check that, smells like buffer overflow
                    uint8_t cursor_data[32 * (32 * 3)];
                    uint8_t cursor_mask[32 * (32 / 8)];

                    this->build_pointer(cursor_data, cursor_mask,
                                        d1, d2, cx, cy,
                                        this->screen.bpp,
                                        this->palette);

                    /* keep these in 32x32, vnc cursor can be alot bigger */
                    if (x > 31) { x = 31; }
                    if (y > 31) { y = 31; }
                    this->server_set_pointer(x, y, cursor_data, cursor_mask);
                }
                break;
                default:
                    throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
                    break;
                }
            }
            this->server_end_update();
            {
                /* FrambufferUpdateRequest */
                Stream stream(8192);
                stream.out_uint8(3);
                stream.out_uint8(1);
                stream.out_uint16_be(0);
                stream.out_uint16_be(0);

                stream.out_uint16_be(this->clip.cx);
                stream.out_uint16_be(this->clip.cy);
                this->t->send((char*)stream.data, 10);
            }
        }
        catch(...) {
            error = 1;
        }
        return error;
    }

    int lib_clip_data(void)
    {
        int size;
        int error;
        try {
            Stream stream(8192);
            this->t->recv((char**)&stream.end, 7);
            stream.skip_uint8(3);
            size = stream.in_uint32_be();

            this->clip_data.init(size);
            this->clip_data_size = size;
            this->t->recv((char**)&this->clip_data.end, size);

            Stream out_s(8192);
            out_s.out_uint16_le(2);
            out_s.out_uint16_le(0);
            out_s.out_uint32_le(0x90);
            out_s.out_uint8(0x0d);
            out_s.out_clear_bytes(35);
            out_s.out_uint8(0x10);
            out_s.out_clear_bytes(35);
            out_s.out_uint8(0x01);
            out_s.out_clear_bytes(35);
            out_s.out_uint8(0x07);
            out_s.out_clear_bytes(35);
            out_s.out_clear_bytes(4);
            out_s.mark_end();
            size = (int)(out_s.end - out_s.data);
            this->server_send_to_channel_mod(this->clip_chanid, out_s.data, size, size, 3);
        }
        catch(...) {
            error = 1;
        }
        return error;
    }

    /******************************************************************************/
    int lib_palette_update(void)
    {
        int error = 0;
        try {
            Stream stream(8192);
            this->t->recv((char**)&stream.end, 5);
            stream.skip_uint8(1);
            int first_color = stream.in_uint16_be();
            int num_colors = stream.in_uint16_be();

            Stream stream2(8192);
            this->t->recv((char**)&stream2.end, num_colors * 6);

            if (num_colors <= 256){
                for (int i = 0; i < num_colors; i++) {
                    int r = stream2.in_uint16_be() >> 8;
                    int g = stream2.in_uint16_be() >> 8;
                    int b = stream2.in_uint16_be() >> 8;
                    this->palette[first_color + i] = (r << 16) | (g << 8) | b;
                }
            }
            else {
                LOG(LOG_ERR, "VNC: number of palette colors too large: %d\n", num_colors);
            }
            this->server_begin_update();
            this->server_palette(this->palette);
            this->server_end_update();
        } catch (...) {
            error = 1;
        }
        return error;

    }

    /******************************************************************************/
    int lib_open_clip_channel(void)
    {
        uint8_t init_data[12] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        this->clip_chanid = this->server_get_channel_id((char*)"cliprdr");

        if (this->clip_chanid >= 0) {
            this->server_send_to_channel_mod(this->clip_chanid, init_data, 12, 12, 3);
        }
        return 0;
    }


    private:
    /* taken from vncauth.c */
    static void rfbEncryptBytes(uint8_t* bytes, char* passwd)
    {
        char key[12];

        /* key is simply password padded with nulls */
        memset(key, 0, sizeof(key));
        strncpy(key, passwd, 8);
        rfbDesKey((unsigned char*)key, EN0); /* 0, encrypt */
        rfbDes((unsigned char*)bytes, (unsigned char*)bytes);
        rfbDes((unsigned char*)(bytes + 8), (unsigned char*)(bytes + 8));
    }
};

#endif
