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
#include <stdint.h>"

#include "colors.hpp"

#include "stream.hpp"
#include "d3des.hpp"

// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto

#warning remove this inheritance. Client_mod should be an interface object provided to mod_vnc (and other mods)
struct mod_vnc : public client_mod {
    char dummy[1024];
    /* mod data */
    char mod_name[256];
    int mod_mouse_state;
    BGRPalette palette;
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
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint8_t depth;
    uint8_t endianess;
    uint8_t true_color_flag;
    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;

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
            memset(this->mod_name, 0, 256);
            this->mod_mouse_state = 0;
            memset(this->palette, 0, sizeof(BGRPalette));
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

            int error = 0;

            try {
                /* protocol version */
                Stream stream(8192);
                this->t->recv((char**)&stream.end, 12);
                this->t->send("RFB 003.003\n", 12);

                /* sec type */
                stream.init(8192);
                this->t->recv((char**)&stream.end, 4);
                int security_level = stream.in_uint32_be();
                LOG(LOG_INFO, "security level is %d (1 = none, 2 = standard)\n",
                              security_level);

                switch (security_level){
                    case 1: /* none */
                        break;
                    case 2: /* dec the password and the server random */
                    {
                        stream.init(8192);
                        this->t->recv((char**)&stream.end, 16);
                        this->rfbEncryptBytes(stream.data, this->password);
                        this->t->send((char*)stream.data, 16);

                        /* sec result */
                        stream.init(8192);
                        this->t->recv((char**)&stream.end, 4);
                        int i = stream.in_uint32_be();
                        if (i != 0) {
                            LOG(LOG_INFO, "vnc password failed\n");
                            throw 2;
                        } else {
//                            LOG(LOG_INFO, "vnc password ok\n");
                        }
                    }
                    break;
                    default:
                        throw 1;
                }
                {
                    Stream stream(8192);
                    stream.data[0] = 1;
                    #warning send and recv should be stream aware
                    #warning we should always send at stream.p, not stream.data
                    this->t->send((char*)stream.data, 1); /* share flag */
                }

                // 7.3.2   ServerInit
                // ------------------

                // After receiving the ClientInit message, the server sends a
                // ServerInit message. This tells the client the width and
                // height of the server's framebuffer, its pixel format and the
                // name associated with the desktop:

                // framebuffer-width  : 2 bytes
                // framebuffer-height : 2 bytes

                // PIXEL_FORMAT       : 16 bytes
                // VNC pixel_format capabilities
                // -----------------------------
                // Server-pixel-format specifies the server's natural pixel
                // format. This pixel format will be used unless the client
                // requests a different format using the SetPixelFormat message
                // (SetPixelFormat).

                // PIXEL_FORMAT::bits per pixel  : 1 byte
                // PIXEL_FORMAT::color depth     : 1 byte

                // Bits-per-pixel is the number of bits used for each pixel
                // value on the wire. This must be greater than or equal to the
                // depth which is the number of useful bits in the pixel value.
                // Currently bits-per-pixel must be 8, 16 or 32. Less than 8-bit
                // pixels are not yet supported.

                // PIXEL_FORMAT::endianess       : 1 byte (0 = LE, 1 = BE)

                // Big-endian-flag is non-zero (true) if multi-byte pixels are
                // interpreted as big endian. Of course this is meaningless
                // for 8 bits-per-pixel.

                // PIXEL_FORMAT::true color flag : 1 byte
                // PIXEL_FORMAT::red max         : 2 bytes
                // PIXEL_FORMAT::green max       : 2 bytes
                // PIXEL_FORMAT::blue max        : 2 bytes
                // PIXEL_FORMAT::red shift       : 1 bytes
                // PIXEL_FORMAT::green shift     : 1 bytes
                // PIXEL_FORMAT::blue shift      : 1 bytes

                // If true-colour-flag is non-zero (true) then the last six
                // items specify how to extract the red, green and blue
                // intensities from the pixel value. Red-max is the maximum
                // red value (= 2^n - 1 where n is the number of bits used
                // for red). Note this value is always in big endian order.
                // Red-shift is the number of shifts needed to get the red
                // value in a pixel to the least significant bit. Green-max,
                // green-shift and blue-max, blue-shift are similar for green
                // and blue. For example, to find the red value (between 0 and
                // red-max) from a given pixel, do the following:

                // * Swap the pixel value according to big-endian-flag (e.g.
                // if big-endian-flag is zero (false) and host byte order is
                // big endian, then swap).
                // * Shift right by red-shift.
                // * AND with red-max (in host byte order).

                // If true-colour-flag is zero (false) then the server uses
                // pixel values which are not directly composed from the red,
                // green and blue intensities, but which serve as indices into
                // a colour map. Entries in the colour map are set by the
                // server using the SetColourMapEntries message
                // (SetColourMapEntries).

                // PIXEL_FORMAT::padding         : 3 bytes

                // name-length        : 4 bytes
                // name-string        : variable

                // The text encoding used for name-string is historically undefined but it is strongly recommended to use UTF-8 (see String Encodings for more details).


                #warning not yet supported
                // If the Tight Security Type is activated, the server init
                // message is extended with an interaction capabilities section.

                {
                    Stream stream(8192);
                    this->t->recv((char**)&stream.end, 24); /* server init */
                    this->width = stream.in_uint16_be();
                    this->height = stream.in_uint16_be();
                    this->bpp    = stream.in_uint8();
                    this->depth  = stream.in_uint8();
                    this->endianess = stream.in_uint8();
                    this->true_color_flag = stream.in_uint8();
                    this->red_max = stream.in_uint16_be();
                    this->green_max = stream.in_uint16_be();
                    this->blue_max = stream.in_uint16_be();
                    this->red_shift = stream.in_uint8();
                    this->green_shift = stream.in_uint8();
                    this->blue_shift = stream.in_uint8();
                    stream.skip_uint8(3); // skip padding

                    LOG(LOG_INFO, "VNC received: width=%d height=%d bpp=%d depth=%d endianess=%d true_color=%d red_max=%d green_max=%d blue_max=%d red_shift=%d green_shift=%d blue_shift=%d", this->width, this->height, this->bpp, this->depth, this->endianess, this->true_color_flag, this->red_max, this->green_max, this->blue_max, this->red_shift, this->green_shift, this->blue_shift);

                    this->server_set_clip(Rect(0, 0, width, height));

                    int lg = stream.in_uint32_be();

                    if (lg > 255 || lg < 0) {
                        throw 3;
                    }
                    char * end = this->mod_name;
                    this->t->recv(&end, lg);
                    this->mod_name[lg] = 0;
                }

                /* should be connected */

                {

                // 7.4.1   SetPixelFormat
                // ----------------------

                // Sets the format in which pixel values should be sent in
                // FramebufferUpdate messages. If the client does not send
                // a SetPixelFormat message then the server sends pixel values
                // in its natural format as specified in the ServerInit message
                // (ServerInit).

                // If true-colour-flag is zero (false) then this indicates that
                // a "colour map" is to be used. The server can set any of the
                // entries in the colour map using the SetColourMapEntries
                // message (SetColourMapEntries). Immediately after the client
                // has sent this message the colour map is empty, even if
                // entries had previously been set by the server.

                // Note that a client must not have an outstanding
                // FramebufferUpdateRequest when it sends SetPixelFormat
                // as it would be impossible to determine if the next *
                // FramebufferUpdate is using the new or the previous pixel
                // format.

                    Stream stream(8192);
                    // Set Pixel format
                    stream.out_uint8(0);

                    // Padding 3 bytes
                    stream.out_uint8(0);
                    stream.out_uint8(0);
                    stream.out_uint8(0);

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

                    // 8 bpp
                    // -----
                    // "\x08\x08\x00"
                    // "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    // "\0\0\0"

                    // 15 bpp
                    // ------
                    // "\x10\x0F\x00"
                    // "\x01\x00\x1F\x00\x1F\x00\x1F\x0A\x05\x00"
                    // "\0\0\0"

                    // 24 bpp
                    // ------
                    // "\x20\x18\x00"
                    // "\x01\x00\xFF\x00\xFF\x00\xFF\x10\x08\x00"
                    // "\0\0\0"

                    // 16 bpp
                    // ------
                    // "\x10\x10\x00"
                    // "\x01\x00\x1F\x00\x2F\x00\x1F\x0B\x05\x00"
                    // "\0\0\0"

                    const char * pixel_format =
                        "\x10" // bits per pixel  : 1 byte =  16
                        "\x10" // color depth     : 1 byte =  16
                        "\x00" // endianess       : 1 byte =  LE
                        "\x01" // true color flag : 1 byte = yes
                        "\x00\x1F" // red max     : 2 bytes = 31
                        "\x00\x3F" // green max   : 2 bytes = 63
                        "\x00\x1F" // blue max    : 2 bytes = 31
                        "\x0B" // red shift       : 1 bytes = 10
                        "\x05" // green shift     : 1 bytes =  6
                        "\x00" // blue shift      : 1 bytes =  0
                        "\0\0\0"; // padding      : 3 bytes
                    stream.out_copy_bytes(pixel_format, 16);
                    this->t->send((char*)stream.data, 20);

                    this->bpp = 16;
                    this->depth  = 16;
                    this->endianess = 0;
                    this->true_color_flag = 1;
                    this->red_max = 0x1F;
                    this->green_max = 0x3F;
                    this->blue_max = 0x1F;
                    this->red_shift = 10;
                    this->green_shift = 6;
                    this->blue_shift = 0;
                }

                // 7.4.2   SetEncodings
                // --------------------

                // Sets the encoding types in which pixel data can be sent by
                // the server. The order of the encoding types given in this
                // message is a hint by the client as to its preference (the
                // first encoding specified being most preferred). The server
                // may or may not choose to make use of this hint. Pixel data
                // may always be sent in raw encoding even if not specified
                // explicitly here.

                // In addition to genuine encodings, a client can request
                // "pseudo-encodings" to declare to the server that it supports
                // certain extensions to the protocol. A server which does not
                // support the extension will simply ignore the pseudo-encoding.
                // Note that this means the client must assume that the server
                // does not support the extension until it gets some extension-
                // -specific confirmation from the server.
                {
                    /* SetEncodings */
                    Stream stream(8192);
                    stream.out_uint8(2);
                    stream.out_uint8(0);
                    stream.out_uint16_be(3);
                    stream.out_uint32_be(0); /* raw */
                    stream.out_uint32_be(1); /* copy rect */
                    stream.out_uint32_be(0xffffff11); /* cursor */

                    this->t->send((char*)stream.data, 4 + 3 * 4);
                }

                this->server_resize(this->width, this->height, this->get_front_bpp());

                {
                    /* FrambufferUpdateRequest */
                    Stream stream(8192);
                    stream.out_uint8(3);
                    stream.out_uint8(0);
                    #warning we could create some out_rect primitive at stream level
                    stream.out_uint16_be(0);
                    stream.out_uint16_be(0);
                    stream.out_uint16_be(width);
                    stream.out_uint16_be(height);

                    // sending framebuffer update request
                    this->t->send((char*)stream.data, 10);
                }

                #warning define some constants, not need to use dynamic data
                /* set almost null cursor, this is the little dot cursor */
                uint8_t rdp_cursor_data[32 * (32 * 3)];
                uint8_t rdp_cursor_mask[32 * (32 / 8)];
                memset(rdp_cursor_data, 0, 32 * (32 * 3));
                memset(rdp_cursor_data + (32 * (32 * 3) - 1 * 32 * 3), 0xff, 9);
                memset(rdp_cursor_data + (32 * (32 * 3) - 2 * 32 * 3), 0xff, 9);
                memset(rdp_cursor_data + (32 * (32 * 3) - 3 * 32 * 3), 0xff, 9);
                memset(rdp_cursor_mask, 0xff, 32 * (32 / 8));
                this->server_set_pointer(3, 3, rdp_cursor_data, rdp_cursor_mask);
            } catch(int i) {
                error = i;
            } catch(...) {
                error = 1;
            };

            if (error) {
                LOG(LOG_INFO, "error - problem connecting\n");
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

//            LOG(LOG_INFO, "VNC connection complete, connected ok\n");
            this->lib_open_clip_channel();
//            LOG(LOG_INFO, "VNC lib open clip channel ok\n");
        } catch(...){
            delete this->t;
            throw;
        }
    }

    virtual ~mod_vnc(){
        delete this->t;
    }

    #warning optimize this, much duplicated code and several send at once when not necessary
    virtual void rdp_input_mouse(int device_flags, int x, int y, const int key_flags, const int (& keys)[256])
    {
        Stream stream(8192);

        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            stream.init(8192);
            stream.out_uint8(5);
            stream.out_uint8(this->mod_mouse_state);
            stream.out_uint16_be(x);
            stream.out_uint16_be(y);
            this->t->send((char*)stream.data, 6);
            #warning this should not be here!!! Move it to front
            this->front.mouse_x = x;
            this->front.mouse_y = y;
        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            if (device_flags & MOUSE_FLAG_DOWN){
                stream.init(8192);
                this->mod_mouse_state |= 1; // set bit 0
                stream.out_uint8(5);
                stream.out_uint8(this->mod_mouse_state);
                stream.out_uint16_be(x);
                stream.out_uint16_be(y);
                this->t->send((char*)stream.data, 6);
            }
            else {
                stream.init(8192);
                this->mod_mouse_state &= ~1; // clear bit 0
                stream.out_uint8(5);
                stream.out_uint8(this->mod_mouse_state);
                stream.out_uint16_be(x);
                stream.out_uint16_be(y);
                this->t->send((char*)stream.data, 6);
            }
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            if (device_flags & MOUSE_FLAG_DOWN){
                stream.init(8192);
                this->mod_mouse_state |= 4; // set bit 0
                stream.out_uint8(5);
                stream.out_uint8(this->mod_mouse_state);
                stream.out_uint16_be(x);
                stream.out_uint16_be(y);
                this->t->send((char*)stream.data, 6);
            }
            else {
                stream.init(8192);
                this->mod_mouse_state &= ~4; // clear bit 0
                stream.out_uint8(5);
                stream.out_uint8(this->mod_mouse_state);
                stream.out_uint16_be(x);
                stream.out_uint16_be(y);
                this->t->send((char*)stream.data, 6);
            }
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            if (device_flags & MOUSE_FLAG_DOWN){
                stream.init(8192);
                this->mod_mouse_state |= 2; // set bit 0
                stream.out_uint8(5);
                stream.out_uint8(this->mod_mouse_state);
                stream.out_uint16_be(x);
                stream.out_uint16_be(y);
                this->t->send((char*)stream.data, 6);
            }
            else {
                stream.init(8192);
                this->mod_mouse_state &= ~2; // clear bit 0
                stream.out_uint8(5);
                stream.out_uint8(this->mod_mouse_state);
                stream.out_uint16_be(x);
                stream.out_uint16_be(y);
                this->t->send((char*)stream.data, 6);
            }
        }

        // Wheel buttons
        if (device_flags == MOUSE_FLAG_BUTTON4 /* 0x0280 */
        ||  device_flags == 0x0278) {
            // DOWN
            stream.init(8192);
            this->mod_mouse_state |= 8; // set bit 3
            stream.out_uint8(5);
            stream.out_uint8(this->mod_mouse_state);
            stream.out_uint16_be(x);
            stream.out_uint16_be(y);
            this->t->send((char*)stream.data, 6);
            // UP
            stream.init(8192);
            this->mod_mouse_state &= ~8; // clear bit 3
            stream.out_uint8(5);
            stream.out_uint8(this->mod_mouse_state);
            stream.out_uint16_be(x);
            stream.out_uint16_be(y);
            this->t->send((char*)stream.data, 6);
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 /* 0x0380 */
        ||  device_flags == 0x0388) {
            // DOWN
            this->mod_mouse_state |= 16; // set bit 4
            stream.out_uint8(5);
            stream.out_uint8(this->mod_mouse_state);
            stream.out_uint16_be(x);
            stream.out_uint16_be(y);
            this->t->send((char*)stream.data, 6);
            // UP
            stream.init(8192);
            this->mod_mouse_state &= ~16; // clear bit 4
            stream.out_uint8(5);
            stream.out_uint8(this->mod_mouse_state);
            stream.out_uint16_be(x);
            stream.out_uint16_be(y);
            this->t->send((char*)stream.data, 6);
        }
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, const int key_flags, const int (& keys)[256], struct key_info* ki){
        if (ki) {
            int msg = (device_flags & KBD_FLAG_UP)?WM_KEYUP:WM_KEYDOWN;
            int key = ki->sym;
            if (key > 0) {
                Stream stream(8192);
                stream.out_uint8(4);
                stream.out_uint8(msg == WM_KEYDOWN); /* down/up flag */
                stream.out_clear_bytes(2);
                stream.out_uint32_be(key);
                this->t->send((char*)stream.data, 8);
            }
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    virtual void rdp_input_invalidate(const Rect & r)
    {
        LOG(LOG_INFO, "rdp_input_invalidate");
        if (!r.isempty()) {
            Stream stream(8192);
            /* FrambufferUpdateRequest */
            stream.out_uint8(3);
            stream.out_uint8(0);
            stream.out_uint16_be(r.x);
            stream.out_uint16_be(r.y);
            stream.out_uint16_be(r.cx);
            stream.out_uint16_be(r.cy);
            this->t->send((char*)stream.data, 10);
        }
    }

    virtual int draw_event(void)
    {
        Stream stream(1);
        int rv = 0;

        try {
            this->t->recv((char**)&stream.end, 1);
            char type = stream.in_uint8();
            switch (type)
            {
                case 0: /* framebuffer update */
                    this->lib_framebuffer_update();
                break;
                case 1: /* palette */
                    this->lib_palette_update();
                break;
                case 3: /* clipboard */
                    this->lib_clip_data();
                break;
                default:
                    LOG(LOG_INFO, "unknown in vnc_lib_draw_event %d\n", type);
            }
        }
        catch(const Error & e) {
            LOG(LOG_INFO, "exception raised id=%u", e.id);
            rv = 1;
        }
        catch(...) {
            LOG(LOG_INFO, "exception raised");
            rv = 1;
        }
        return rv;
    }

    private:
    #warning use it for copy/paste
    int lib_process_channel_data(int chanid, int flags, int size, Stream & stream, int total_size)
    {
        if (chanid == this->clip_chanid) {
            uint16_t type = stream.in_uint16_le();
            uint16_t status = stream.in_uint16_le();
            uint32_t length = stream.in_uint32_le();
            switch (type) {
            case 2:
            { /* CLIPRDR_FORMAT_ANNOUNCE */
                Stream out_s(8192);
                out_s.out_uint16_le(3);
                out_s.out_uint16_le(1);
                out_s.out_uint32_le(0);
                out_s.out_clear_bytes(4); /* pad */
                out_s.mark_end();
                length = (int)(out_s.end - out_s.data);
//                this->server_send_to_channel_mod(this->clip_chanid, out_s.data, length, length, 3);
            }
            break;
            case 3: /* CLIPRDR_FORMAT_ACK */
                break;
            case 4:
            { /* CLIPRDR_DATA_REQUEST */
                uint32_t format = 0;
                if (length >= 4) {
                    format = stream.in_uint32_le();
                }
                /* only support CF_TEXT and CF_UNICODETEXT */
                if ((format != 1) && (format != 13)) {
                    break;
                }
                Stream out_s(8192);
                out_s.out_uint16_le(5);
                out_s.out_uint16_le(1);
                if (format == 13) { /* CF_UNICODETEXT */
                    out_s.out_uint32_le( this->clip_data_size * 2 + 2);
                    for (size_t index = 0; index < this->clip_data_size; index++) {
                        out_s.out_uint8(this->clip_data.data[index]);
                        out_s.out_uint8(0);
                    }
                    out_s.out_clear_bytes(2);
                }
                else if (format == 1) { /* CF_TEXT */
                    out_s.out_uint32_le(this->clip_data_size + 1);
                    for (size_t index = 0; index < this->clip_data_size; index++) {
                        out_s.out_uint8(this->clip_data.data[index]);
                    }
                    out_s.out_clear_bytes( 1);
                }
                out_s.out_clear_bytes( 4); /* pad */
                out_s.mark_end();
                length = (int)(out_s.end - out_s.data);
//                this->server_send_to_channel_mod(this->clip_chanid, out_s.data, length, length, 3);
            }
            break;
            }
        } else {
            printf("lib_process_channel_data: unknown chanid %d"
                   " this->clip_chanid %d\n",
                      chanid, this->clip_chanid);
        }
        return 0;
    }

    void lib_framebuffer_update() throw (Error)
    {
        size_t num_recs = 0;
        int Bpp = nbbytes(this->bpp);
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
            uint32_t encoding = stream.in_uint32_be();

            switch (encoding){
            case 0: /* raw */
            {
                int need_size = cx * cy * Bpp;
                uint8_t * raw = (uint8_t *)malloc(need_size);
                if (!raw){
                    LOG(LOG_ERR, "Memory allocation failed for raw buffer in VNC");
                    assert(raw);
                }
                uint8_t * tmp = raw;
                this->t->recv((char**)&tmp, need_size);

                #warning there is still an alignement issue in bitmaps, fixed, but my fix is quite evil.
                Bitmap bmp(this->bpp, &this->palette332, cx, cy, raw, need_size, false, true);
                free(raw);
                this->bitmap_update(bmp, Rect(x, y, cx, cy), 0, 0);
            }
            break;
            case 1: /* copy rect */
            {
//                    LOG(LOG_INFO, "copy rect");
                Stream stream(4);
                this->t->recv((char**)&stream.end, 4);
                const int srcx = stream.in_uint16_be();
                const int srcy = stream.in_uint16_be();
//                    LOG(LOG_INFO, "copy rect: x=%d y=%d cx=%d cy=%d encoding=%d src_x=%d, src_y=%d", x, y, cx, cy, encoding, srcx, srcy);
                #warning should we not set clip rectangle ?
                const RDPScrBlt scrblt(Rect(x, y, cx, cy), 0xCC, srcx, srcy);
                this->scr_blt(scrblt);
            }
            break;
            case 0xffffff11: /* cursor */
            #warning see why we get these empty rects ?
            if (cx > 0 && cy > 0) {
                // 7.7.2   Cursor Pseudo-encoding
                // ------------------------------

                // A client which requests the Cursor pseudo-encoding is
                // declaring that it is capable of drawing a mouse cursor
                // locally. This can significantly improve perceived performance
                // over slow links.

                // The server sets the cursor shape by sending a pseudo-rectangle
                // with the Cursor pseudo-encoding as part of an update.

                // x, y : The pseudo-rectangle's x-position and y-position
                // indicate the hotspot of the cursor,

                // cx, cy : width and height indicate the width and height of
                // the cursor in pixels.

                // The data consists of width * height pixel values followed by
                // a bitmask.

                // PIXEL array : width * height * bytesPerPixel
                // bitmask     : floor((width + 7) / 8) * height

                // The bitmask consists of left-to-right, top-to-bottom
                // scanlines, where each scanline is padded to a whole number of
                // bytes. Within each byte the most significant bit represents
                // the leftmost pixel, with a 1-bit meaning the corresponding
                // pixel in the cursor is valid.

                const int sz_pixel_array = cx * cy * Bpp;
                const int sz_bitmask = nbbytes(cx) * cy;
                Stream stream(sz_pixel_array + sz_bitmask);
                this->t->recv((char**)&stream.end, sz_pixel_array + sz_bitmask);

                const uint8_t *vnc_pointer_data = stream.in_uint8p(sz_pixel_array);
                const uint8_t *vnc_pointer_mask = stream.in_uint8p(sz_bitmask);

                uint8_t rdp_cursor_mask[32 * (32 / 8)] = {};

                // clear target cursor mask
                for (size_t tmpy = 0; tmpy < 32; tmpy++) {
                    for (size_t mask_x = 0; mask_x < nbbytes(32); mask_x++) {
                        rdp_cursor_mask[tmpy*nbbytes(32) + mask_x] = 0xFF;
                    }
                }

                // copy vnc pointer and mask to rdp pointer and mask
                uint8_t rdp_cursor_data[32 * (32 * 3)] = {};
                for (int yy = 0; yy < cy; yy++) {
                    for (int xx = 0 ; xx < cx ; xx++){
                        if (vnc_pointer_mask[yy * nbbytes(cx) + xx / 8 ] & (0x80 >> (xx&7))){
                            if ((yy < 32) && (xx < 32)){
                                rdp_cursor_mask[(31-yy) * nbbytes(32) + (xx / 8)] &= ~(0x80 >> (xx&7));
                                int pixel = 0;
                                for (int tt = 0 ; tt < Bpp; tt++){
                                    pixel += vnc_pointer_data[(yy * cx + xx) * Bpp + tt] << (8 * tt);
                                }
                                rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 0] = pixel >> 16;
                                rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 1] = pixel >> 8;
                                rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 2] = pixel;
                            }
                        }
                    }
                }

                /* keep these in 32x32, vnc cursor can be alot bigger */
                /* (anyway hotspot is usually 0, 0)                   */
                if (x > 31) { x = 31; }
                if (y > 31) { y = 31; }
#warning we should manage cursors bigger then 32 x 32, this is not an RDP protocol limitation
                this->server_set_pointer(x, y, rdp_cursor_data, rdp_cursor_mask);
            }
            break;
            default:
                LOG(LOG_INFO, "unexpected encoding %8x in lib_frame_buffer", encoding);
//                throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
                break;
            }
        }
        this->server_end_update();

        {
//                LOG(LOG_INFO, "Frame buffer Update");
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

    void lib_clip_data(void)
    {
        Stream stream(8192);
        this->t->recv((char**)&stream.end, 7);
        stream.skip_uint8(3);
        int size = stream.in_uint32_be();

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
//        this->server_send_to_channel_mod(this->clip_chanid, out_s.data, size, size, 3);
    }

    /******************************************************************************/
    void lib_palette_update(void)
    {
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
        memcpy(this->mod_palette, this->palette, sizeof(BGRPalette));
        this->send_global_palette();
        this->server_begin_update();
        this->color_cache(this->palette, 0);
        this->server_end_update();
    }

    /******************************************************************************/
    void lib_open_clip_channel(void)
    {
        #warning not working, see why
        return;
        uint8_t init_data[12] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//        this->clip_chanid = this->server_get_channel_id((char*)"cliprdr");

//        if (this->clip_chanid >= 0) {
//            this->server_send_to_channel_mod(this->clip_chanid, init_data, 12, 12, 3);
//        }
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
