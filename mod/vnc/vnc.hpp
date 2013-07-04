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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Vnc module
*/

#ifndef _REDEMPTION_MOD_VNC_VNC_HPP_
#define _REDEMPTION_MOD_VNC_VNC_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>

#include "colors.hpp"

#include "stream.hpp"
#include "d3des.hpp"
#include "keymap2.hpp"
#include "keymapSym.hpp"
#include "mod_api.hpp"
#include "channel_list.hpp"

#include "RDP/clipboard.hpp"

// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto

#define MAX_VNC_2_RDP_CLIP_DATA_SIZE 8000

//###############################################################################################################
struct mod_vnc : public mod_api {
//###############################################################################################################
    /* mod data */
    FrontAPI & front;
    char mod_name[256];
    uint8_t mod_mouse_state;
    BGRPalette palette;
    int vnc_desktop;
    char username[256];
    char password[256];
    public:
    Transport *t;

    uint16_t width;
    uint16_t height;
    uint8_t  bpp;
    uint8_t  depth;

    uint8_t endianess;
    uint8_t true_color_flag;

    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;

    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;

    BGRPalette palette332;
    uint32_t   verbose;
    KeymapSym  keymapSym;
    int        incr;

    BStream to_rdp_clipboard_data;
    BStream to_vnc_large_clipboard_data;

    bool opt_clipboard;  // true clipboard available, false clipboard unavailable

    //==============================================================================================================
    mod_vnc( Transport * t
           , const char * username
           , const char * password
           , struct FrontAPI & front
           , uint16_t front_width
           , uint16_t front_height
           , int keylayout
           , int key_flags
           , bool clipboard
           , bool new_encoding
           , uint32_t verbose
           )
    //==============================================================================================================
            : mod_api(front_width, front_height)
            , front(front)
            , verbose(verbose)
            , keymapSym(verbose)
            , incr(0)
            , to_vnc_large_clipboard_data(2 * MAX_VNC_2_RDP_CLIP_DATA_SIZE + 2)
            , opt_clipboard(clipboard) {
    //--------------------------------------------------------------------------------------------------------------
        LOG(LOG_INFO, "Connecting to VNC Server");
        init_palette332(this->palette332);
        this->t = t;
        keymapSym.init_layout_sym(keylayout);
        // Initial state of keys (at least lock keys) is copied from Keymap2
        keymapSym.key_flags = key_flags;

        memset(this->mod_name, 0, 256);
        this->mod_mouse_state = 0;
        memset(this->palette, 0, sizeof(BGRPalette));
        this->vnc_desktop = 0;
        memset(this->username, 0, 256);
        memset(this->password, 0, 256);

//        this->clip_chanid = 0;

        strcpy(this->username, username);
        strcpy(this->password, password);

        int error = 0;

        /* protocol version */
        BStream stream(32768);
        this->t->recv(&stream.end, 12);
        this->t->send("RFB 003.003\n", 12);

        /* sec type */
        stream.init(8192);
        this->t->recv(&stream.end, 4);
        int security_level = stream.in_uint32_be();
        LOG(LOG_INFO, "security level is %d (1 = none, 2 = standard)\n",
                      security_level);

        switch (security_level){
            case 1: /* none */
                break;
            case 2: /* the password and the server random */
            {
                LOG(LOG_INFO, "Receiving VNC Server Random");
                stream.init(8192);
                this->t->recv(&stream.end, 16);

                /* taken from vncauth.c */
                {
                    char key[12];

                    /* key is simply password padded with nulls */
                    memset(key, 0, sizeof(key));
                    strncpy(key, this->password, 8);
                    rfbDesKey((unsigned char*)key, EN0); /* 0, encrypt */
                    rfbDes((unsigned char*)stream.get_data(), (unsigned char*)stream.get_data());
                    rfbDes((unsigned char*)(stream.get_data() + 8), (unsigned char*)(stream.get_data() + 8));
                }
                LOG(LOG_INFO, "Sending Password");
                this->t->send(stream.get_data(), 16);

                /* sec result */
                LOG(LOG_INFO, "Waiting for password ack");
                stream.init(8192);
                this->t->recv(&stream.end, 4);
                int i = stream.in_uint32_be();
                if (i != 0) {
                    LOG(LOG_INFO, "vnc password failed\n");
                    throw Error(ERR_VNC_CONNECTION_ERROR);
                } else {
                    LOG(LOG_INFO, "vnc password ok\n");
                }
            }
            break;
            default:
                throw Error(ERR_VNC_CONNECTION_ERROR);
        }
        this->t->send("\x01", 1); /* share flag */

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

        TODO(" not yet supported")
        // If the Tight Security Type is activated, the server init
        // message is extended with an interaction capabilities section.

        {
            BStream stream(32768);
            this->t->recv(&stream.end, 24); /* server init */
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
            stream.in_skip_bytes(3); // skip padding

//            LOG(LOG_INFO, "VNC received: width=%d height=%d bpp=%d depth=%d endianess=%d true_color=%d red_max=%d green_max=%d blue_max=%d red_shift=%d green_shift=%d blue_shift=%d", this->width, this->height, this->bpp, this->depth, this->endianess, this->true_color_flag, this->red_max, this->green_max, this->blue_max, this->red_shift, this->green_shift, this->blue_shift);

            int lg = stream.in_uint32_be();

            if (lg > 255 || lg < 0) {
                throw Error(ERR_VNC_CONNECTION_ERROR);
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

            BStream stream(32768);
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
                "\x0B" // red shift       : 1 bytes = 11
                "\x05" // green shift     : 1 bytes =  5
                "\x00" // blue shift      : 1 bytes =  0
                "\0\0\0"; // padding      : 3 bytes
            stream.out_copy_bytes(pixel_format, 16);
            this->t->send(stream.get_data(), 20);

            this->bpp = 16;
            this->depth  = 16;
            this->endianess = 0;
            this->true_color_flag = 1;
            this->red_max       = 0x1F;
            this->green_max     = 0x3F;
            this->blue_max      = 0x1F;
            this->red_shift     = 0x0B;
            this->green_shift   = 0x05;
            this->blue_shift    = 0;
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
            BStream stream(32768);
            stream.out_uint8(2);
            stream.out_uint8(0);
//            stream.out_uint16_be(3);
            stream.out_uint16_be(new_encoding ? 4 : 3);
            if (new_encoding) {
                stream.out_uint32_be(2);        /* RRE */
            }
            stream.out_uint32_be(0);            /* raw */
            stream.out_uint32_be(1);            /* copy rect */
            stream.out_uint32_be(0xffffff11);   /* cursor */

//            this->t->send(stream.get_data(), 4 + 3 * 4);
            this->t->send(stream.get_data(), 4 + (new_encoding ? 4 : 3) * 4);
        }

        TODO("Maybe the resize should be done in session ?")
        switch (this->front.server_resize(this->width, this->height, this->bpp)){
        case 0:
            // no resizing needed
            break;
        case 1:
            // resizing done
            this->front_width  = this->width;
            this->front_height = this->height;
            break;
        case -1:
            // resizing failed
            // thow an Error ?
            LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
            throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
            break;
        }

        TODO(" define some constants  not need to use dynamic data")
//        /* set almost null cursor, this is the little dot cursor */
        uint8_t rdp_cursor_data[32 * (32 * 3)];
        uint8_t rdp_cursor_mask[32 * (32 / 8)];
        memset(rdp_cursor_data, 0, 32 * (32 * 3));
        memset(rdp_cursor_data + (32 * (32 * 3) - 1 * 32 * 3), 0xff, 9);
        memset(rdp_cursor_data + (32 * (32 * 3) - 2 * 32 * 3), 0xff, 9);
        memset(rdp_cursor_data + (32 * (32 * 3) - 3 * 32 * 3), 0xff, 9);
        memset(rdp_cursor_mask, 0xff, 32 * (32 / 8));

        this->front.server_set_pointer(3, 3, rdp_cursor_data, rdp_cursor_mask);

        if (error) {
            LOG(LOG_INFO, "error - problem connecting\n");
            throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        LOG(LOG_INFO, "VNC connection complete, connected ok\n");
        TODO("Clearing the front screen could be done in session")
        this->front.begin_update();
        RDPOpaqueRect orect(Rect(0, 0, this->width, this->height), 0);
        this->front.draw(orect, Rect(0, 0, this->width, this->height));
        this->front.end_update();

        this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));

        this->lib_open_clip_channel();
    } // Constructor

    //==============================================================================================================
    virtual ~mod_vnc(){}
    //==============================================================================================================

    //==============================================================================================================
    void change_mouse_state( uint16_t x
                           , uint16_t y
                           , uint8_t button
                           , bool set
                           ) {
    //==============================================================================================================
        BStream stream(6);
        this->mod_mouse_state = set?(this->mod_mouse_state|button):(this->mod_mouse_state&~button); // set or clear bit
        stream.out_uint8(5);
        stream.out_uint8(this->mod_mouse_state);
        stream.out_uint16_be(x);
        stream.out_uint16_be(y);
        this->t->send(stream.get_data(), 6);
    } // change_mouse_state

    TODO("It may be possible to change several mouse buttons at once ? Current code seems to perform several send if that occurs. Is it what we want ?")
    //==============================================================================================================
    virtual void rdp_input_mouse( int device_flags
                                , int x
                                , int y
                                , Keymap2 * keymap
                                ) {
    //==============================================================================================================
        BStream stream(32768);

        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            this->change_mouse_state(x, y, 0, true);
        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            this->change_mouse_state(x, y, 1, device_flags & MOUSE_FLAG_DOWN);
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            this->change_mouse_state(x, y, 4, device_flags & MOUSE_FLAG_DOWN);
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            this->change_mouse_state(x, y, 2, device_flags & MOUSE_FLAG_DOWN);
        }

        // Wheel buttons
        if (device_flags == MOUSE_FLAG_BUTTON4 /* 0x0280 */
        ||  device_flags == 0x0278) {
            this->change_mouse_state(x, y, 8, true); // DOWN
            this->change_mouse_state(x, y, 8, false); // UP
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 /* 0x0380 */
        ||  device_flags == 0x0388) {
            this->change_mouse_state(x, y, 16, true); // DOWN
            this->change_mouse_state(x, y, 16, false); // UP
        }
    } // rdp_input_mouse

    //==============================================================================================================
    virtual void rdp_input_scancode( long param1
                                   , long param2
                                   , long device_flags
                                   , long param4
                                   , Keymap2 * keymap
                                   ) {
    //==============================================================================================================
        TODO("As down/up state is not stored in keymapSym, code below is quite dangerous")
        keymapSym.event(device_flags, param1);
        int key = keymapSym.get_sym();
        if (key > 0) {
            BStream stream(32768);
            stream.out_uint8(4);
            stream.out_uint8(!(device_flags & KBD_FLAG_UP)); /* down/up flag */
            stream.out_clear_bytes(2);
            stream.out_uint32_be(key);
            this->t->send(stream.get_data(), 8);
            this->event.set(1000);
        }
    } // rdp_input_scancode

    //==============================================================================================================
    virtual void rdp_input_clip_data(uint8_t * data, uint32_t length) {
    //==============================================================================================================
        BStream stream(length + 8);

        stream.out_uint8(6);                      // message-type : ClientCutText
        stream.out_clear_bytes(3);                // padding
        stream.out_uint32_be(length);             // length
        stream.out_copy_bytes(data, length);      // text

        this->t->send(stream.get_data(), (length + 8)); // message-type(1) + padding(3) + length(4)

        this->event.set(1000);
    } // rdp_input_clip_data

    //==============================================================================================================
    virtual void rdp_input_synchronize( uint32_t time
                                      , uint16_t device_flags
                                      , int16_t param1
                                      , int16_t param2
                                      ) {
    //==============================================================================================================
        if (this->verbose) {
            LOG( LOG_INFO
               , "KeymapSym::synchronize(time=%u, device_flags=%08x, param1=%04x, param1=%04x"
                , time, device_flags, param1, param2);
        }
        this->keymapSym.synchronize(param1);
    } // rdp_input_synchronize

    //==============================================================================================================
    virtual void rdp_input_invalidate(const Rect & r) {
    //==============================================================================================================
        if (!r.isempty()) {
            BStream stream(32768);
            /* FrambufferUpdateRequest */
            stream.out_uint8(3);
            stream.out_uint8(this->incr);
            stream.out_uint16_be(r.x);
            stream.out_uint16_be(r.y);
            stream.out_uint16_be(r.cx);
            stream.out_uint16_be(r.cy);
            this->t->send(stream.get_data(), 10);
            this->incr = 1;
        }
    } // rdp_input_invalidate

    //==============================================================================================================
    virtual BackEvent_t draw_event(void) {
    //==============================================================================================================
        if (this->verbose) {
            LOG(LOG_INFO, "vnc::draw_event");
        }
        BackEvent_t rv = BACK_EVENT_NONE;

        if (this->event.can_recv()) {
            BStream stream(1);
            try {
                this->t->recv(&stream.end, 1);
                char type = stream.in_uint8(); /* message-type */
                switch (type) {
                    case 0: /* framebuffer update */
                        this->lib_framebuffer_update();
                    break;
                    case 1: /* palette */
                        this->lib_palette_update();
                    break;
                    case 3: /* clipboard */ /* ServerCutText */
                        this->lib_clip_data();
                    break;
                    default:
                        LOG(LOG_INFO, "unknown in vnc_lib_draw_event %d\n", type);
                }
            }
            catch(const Error & e) {
                LOG(LOG_INFO, "VNC Stopped [reason id=%u]", e.id);
                rv = BACK_EVENT_NEXT;
            }
            catch(...) {
                LOG(LOG_INFO, "unexpected exception raised in VNC");
                rv = BACK_EVENT_NEXT;
            }
            if (rv != BACK_EVENT_NEXT) {
                this->event.set(1000);
            }
        }
        else {
            this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));
        }
        return rv;
    } // draw_event

    private:
    //==============================================================================================================
    void lib_framebuffer_update() throw (Error) {
    //==============================================================================================================
        BStream stream(256);
        this->t->recv(&stream.end, 3);
        stream.in_skip_bytes(1);
        size_t num_recs = stream.in_uint16_be();

        uint8_t Bpp = nbbytes(this->bpp);
        stream.init(256);
        for (size_t i = 0; i < num_recs; i++) {
            this->t->recv(&stream.end, 12);
            uint16_t x = stream.in_uint16_be();
            uint16_t y = stream.in_uint16_be();
            uint16_t cx = stream.in_uint16_be();
            uint16_t cy = stream.in_uint16_be();
            uint32_t encoding = stream.in_uint32_be();

            switch (encoding) {
            case 0: /* raw */
            {
                uint8_t * raw = (uint8_t *)malloc(cx * 16 * Bpp);
                if (!raw) {
                    LOG(LOG_ERR, "Memory allocation failed for raw buffer in VNC");
                    throw Error(ERR_VNC_MEMORY_ALLOCATION_FAILED);
                }

                this->front.begin_update();
                for (uint16_t yy = y ; yy < y + cy ; yy += 16) {
                    uint8_t * tmp = raw;
                    uint16_t cyy = std::min<uint16_t>(16, cy-(yy-y));
                    this->t->recv(&tmp, cyy*cx*Bpp);
//                    LOG(LOG_INFO, "draw vnc: x=%d y=%d cx=%d cy=%d", x, yy, cx, cyy);
                    this->front.draw_vnc(Rect(x, yy, cx, cyy), this->bpp, this->palette332, raw, cx*16*Bpp);
                }
                this->front.end_update();
                free(raw);
            }
            break;
            case 1: /* copy rect */
            {
                BStream stream(4);
                this->t->recv(&stream.end, 4);
                const int srcx = stream.in_uint16_be();
                const int srcy = stream.in_uint16_be();
//                LOG(LOG_INFO, "copy rect: x=%d y=%d cx=%d cy=%d encoding=%d src_x=%d, src_y=%d", x, y, cx, cy, encoding, srcx, srcy);
                const RDPScrBlt scrblt(Rect(x, y, cx, cy), 0xCC, srcx, srcy);
                this->front.begin_update();
                this->front.draw(scrblt, Rect(0, 0, this->front_width, this->front_height));
                this->front.end_update();
            }
            break;
            case 2:
            {
//LOG(LOG_INFO, "VNC Encoding: RRE, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
                uint8_t * raw = (uint8_t *)malloc(cx * cy * Bpp);
                if (!raw) {
                    LOG(LOG_ERR, "Memory allocation failed for RRE buffer in VNC");
                    throw Error(ERR_VNC_MEMORY_ALLOCATION_FAILED);
                }

                this->t->recv(&stream.end,
                      4   /* number-of-subrectangles */
                    + Bpp /* background-pixel-value */
                    );

                uint32_t number_of_subrectangles;
                uint32_t number_of_subrectangles_remain;
                uint32_t number_of_subrectangles_read;

                number_of_subrectangles_remain =
                number_of_subrectangles        = stream.in_uint32_be();

                char * bytes_per_pixel;
                char * point_cur;
                char * point_end;

                bytes_per_pixel = reinterpret_cast<char *>(stream.p);
                stream.in_skip_bytes(Bpp);

                for (point_cur = reinterpret_cast<char *>(raw), point_end = point_cur + cx * cy * Bpp;
                     point_cur < point_end; point_cur += Bpp) {
                    memcpy(point_cur, bytes_per_pixel, Bpp);
                }

                BStream    subrectangles(65535);
                uint16_t   subrec_x, subrec_y, subrec_width, subrec_height;
                char     * point_line_cur;
                char     * point_line_end;
                uint32_t   i;
                uint32_t   ling_boundary;

                while (number_of_subrectangles_remain > 0) {
                    number_of_subrectangles_read = min<uint32_t>(4096, number_of_subrectangles_remain);

                    subrectangles.reset();
                    this->t->recv(&subrectangles.end, (Bpp + 8) * number_of_subrectangles_read);

                    number_of_subrectangles_remain -= number_of_subrectangles_read;

                    for (i = 0; i < number_of_subrectangles_read; i++) {
                        bytes_per_pixel = reinterpret_cast<char *>(subrectangles.p);
                        subrectangles.in_skip_bytes(Bpp);

                        subrec_x        = subrectangles.in_uint16_be();
                        subrec_y        = subrectangles.in_uint16_be();
                        subrec_width    = subrectangles.in_uint16_be();
                        subrec_height   = subrectangles.in_uint16_be();

                        for (ling_boundary = cx * Bpp,
                                 point_line_cur = reinterpret_cast<char *>(raw) + subrec_y * ling_boundary,
                                 point_line_end = point_line_cur + subrec_height * ling_boundary;
                             point_line_cur < point_line_end; point_line_cur += ling_boundary)
                            for (point_cur = point_line_cur + subrec_x * Bpp,
                                    point_end = point_cur + subrec_width * Bpp;
                                 point_cur < point_end; point_cur += Bpp) {
                                memcpy(point_cur, bytes_per_pixel, Bpp);
                            }
                    }
                }

                this->front.begin_update();
                this->front.draw_vnc(Rect(x, y, cx, cy), this->bpp, this->palette332, raw, cx*cy*Bpp);
                this->front.end_update();

                free(raw);
            }
            break;
            case 0xffffff11: /* cursor */
            TODO(" see why we get these empty rects ?")
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
                BStream stream(sz_pixel_array + sz_bitmask);
                this->t->recv(&stream.end, sz_pixel_array + sz_bitmask);

                const uint8_t *vnc_pointer_data = stream.in_uint8p(sz_pixel_array);
                const uint8_t *vnc_pointer_mask = stream.in_uint8p(sz_bitmask);

                uint8_t rdp_cursor_mask[32 * (32 / 8)] = {};
                uint8_t rdp_cursor_data[32 * (32 * 3)] = {};

                // a VNC pointer of 1x1 size is not visible, so a default minimal pointer (dot pointer) is provided instead
                if (cx == 1 && cy == 1) {
                    memset(rdp_cursor_data, 0, sizeof(rdp_cursor_data));
                    rdp_cursor_data[2883] = 0xFF;
                    rdp_cursor_data[2884] = 0xFF;
                    rdp_cursor_data[2885] = 0xFF;
                    memset(rdp_cursor_mask, 0xFF, sizeof(rdp_cursor_mask));
                    rdp_cursor_mask[116] = 0x1F;
                    rdp_cursor_mask[120] = 0x1F;
                    rdp_cursor_mask[124] = 0x1F;
                }
                else {
                    // clear target cursor mask
                    for (size_t tmpy = 0; tmpy < 32; tmpy++) {
                        for (size_t mask_x = 0; mask_x < nbbytes(32); mask_x++) {
                            rdp_cursor_mask[tmpy*nbbytes(32) + mask_x] = 0xFF;
                        }
                    }
                    TODO("The code below is likely to explain the yellow pointer: we ask for 16 bits for VNC, but we work with cursor as if it were 24 bits. We should use decode primitives and reencode it appropriately. Cursor has the right shape because the mask use is 1 bit per pixel arrays")
                    // copy vnc pointer and mask to rdp pointer and mask

                    for (int yy = 0; yy < cy; yy++) {
                        for (int xx = 0 ; xx < cx ; xx++){
                            if (vnc_pointer_mask[yy * nbbytes(cx) + xx / 8 ] & (0x80 >> (xx&7))){
                                if ((yy < 32) && (xx < 32)){
                                    rdp_cursor_mask[(31-yy) * nbbytes(32) + (xx / 8)] &= ~(0x80 >> (xx&7));
                                    int pixel = 0;
                                    for (int tt = 0 ; tt < Bpp; tt++){
                                        pixel += vnc_pointer_data[(yy * cx + xx) * Bpp + tt] << (8 * tt);
                                    }
                                    TODO("temporary: force black cursor")
                                    int red   = (pixel >> this->red_shift) & red_max;
                                    int green = (pixel >> this->green_shift) & green_max;
                                    int blue  = (pixel >> this->blue_shift) & blue_max;
                                    rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 0] = (red << 3) | (red >> 2);
                                    rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 1] = (green << 2) | (green >> 4);;
                                    rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 2] = (blue << 3) | (blue >> 2);
                                }
                            }
                        }
                    }
                    /* keep these in 32x32, vnc cursor can be alot bigger */
                    /* (anyway hotspot is usually 0, 0)                   */
                    if (x > 31) { x = 31; }
                    if (y > 31) { y = 31; }
                }
TODO(" we should manage cursors bigger then 32 x 32  this is not an RDP protocol limitation")
                this->front.begin_update();
                this->front.server_set_pointer(x, y, rdp_cursor_data, rdp_cursor_mask);
                this->front.end_update();
            }
            break;
            default:
                LOG(LOG_INFO, "unexpected encoding %8x in lib_frame_buffer", encoding);
                throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
                break;
            }
        }

        this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));
    } // lib_framebuffer_update

    //==============================================================================================================
    void lib_palette_update(void) {
    //==============================================================================================================
        BStream stream(32768);
        this->t->recv(&stream.end, 5);
        stream.in_skip_bytes(1);
        int first_color = stream.in_uint16_be();
        int num_colors = stream.in_uint16_be();

        BStream stream2(8192);
        this->t->recv(&stream2.end, num_colors * 6);

        if (num_colors <= 256) {
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

        this->front.set_mod_palette(this->palette);
        this->front.send_global_palette();
        this->front.begin_update();
        RDPColCache cmd(0, this->palette);
        this->front.draw(cmd);
        this->front.end_update();
    } // lib_palette_update

    /******************************************************************************/
    void lib_open_clip_channel(void) {
        CHANNELS::ChannelDefArray chanlist   = this->front.get_channel_list();
        const CHANNELS::ChannelDef * channel = chanlist.get((char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME);

        if (channel) {
            // Monitor ready PDU send to front
            RDPECLIP::ServerMonitorReadyPDU server_monitor_ready_pdu;
            BStream                         out_s(64);

/*
            //- Beginning of clipboard PDU Header ----------------------------
            out_s.out_uint16_le(1); // MSG Type 2 bytes
            out_s.out_uint16_le(0); // MSG flags 2 bytes
            out_s.out_uint32_le(0); // Datalen of the rest of the message
            //- End of clipboard PDU Header ----------------------------------
            //- Beginning of Monitor Ready PDU payload ----------------------------
            //- End of Monitor Ready PDU payload -------------------------------
            out_s.out_clear_bytes(4);
            out_s.mark_end();
*/
            server_monitor_ready_pdu.emit(out_s);

            size_t length     = out_s.size();
            size_t chunk_size = length;

            this->send_to_front_channel( (char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                       , out_s.get_data()
                                       , length
                                       , chunk_size
                                       ,   CHANNELS::CHANNEL_FLAG_FIRST
                                         | CHANNELS::CHANNEL_FLAG_LAST
                                       );
        }
        else {
            LOG(LOG_INFO, "Clipboard Channel Redirection unavailable");
        }
    } // lib_open_clip_channel

    //==============================================================================================================
    const CHANNELS::ChannelDef * get_channel_from_front_by_name(char * channel_name) {
    //==============================================================================================================
        CHANNELS::ChannelDefArray    channel_list = this->front.get_channel_list();
        const CHANNELS::ChannelDef * channel      = channel_list.get(channel_name);
        return channel;
    } // get_channel_from_front_by_name

    //******************************************************************************
    // Entry point for VNC server clipboard content reception
    // Conversion to RDP behaviour :
    //  - store this content in a buffer, waiting for an explicit request from the front
    //  - send a notification to the front (Format List PDU) that the server clipboard
    //    status has changed
    //******************************************************************************
    //==============================================================================================================
    void lib_clip_data(void) {
    //==============================================================================================================
        CHANNELS::ChannelDefArray    chanlist = this->front.get_channel_list();
        const CHANNELS::ChannelDef * channel  = chanlist.get((char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME);

        TODO("change code below. It will overflow for long VNC data to copy."
        " If clip_data_size is large is will also allocate an undecent amoutn of memory")

        // NB : Whether the clipboard is available or not, read the incoming data to prevent a jam in transport layer
        // Store the clipboard into *to_rdp_clipboard_data*, data length will be (to_rdp_clipboard_data.size())
        BStream stream(32768);
        this->t->recv(&stream.end, 7);
        stream.in_skip_bytes(3); /* padding */
        size_t clip_data_size = stream.in_uint32_be(); /* length */

        size_t chunk_size = 0;

        if (this->opt_clipboard) {
            chunk_size = std::min<size_t>(clip_data_size, MAX_VNC_2_RDP_CLIP_DATA_SIZE);
            if (this->verbose) {
                LOG(LOG_INFO, "clip_data_size=%u chunk_size=%u", clip_data_size, chunk_size);
            }

            // The size of <stream> must be larger than MAX_VNC_2_RDP_CLIP_DATA_SIZE.
            this->t->recv(&stream.end, chunk_size); /* text */
            // Add two trailing zero if not already there to ensure we have UTF8sz content
            if (stream.end[-1]) { *stream.end = 0; stream.end++; }
            if (stream.end[-1]) { *stream.end = 0; stream.end++; }

            size_t clipboard_payload_size = UTF8Check(stream.p, chunk_size);
            stream.p[clipboard_payload_size] = 0;

            this->to_rdp_clipboard_data.init(4 * (MAX_VNC_2_RDP_CLIP_DATA_SIZE + 1) + 8 /* clipboard PDU Header size */);

            bool response_ok = true;

            RDPECLIP::FormatDataResponsePDU format_data_response_pdu(response_ok);

            format_data_response_pdu.emit(this->to_rdp_clipboard_data, reinterpret_cast<const char *>(stream.p));
        }

        // drop remaining clipboard content if larger that about 8000 bytes
        if (clip_data_size > chunk_size) {
            size_t remaining = clip_data_size - chunk_size;
            BStream drop(4096);
            while (remaining > 4096) {
                drop.end = drop.get_data();
                this->t->recv(&drop.end, 4096);
                remaining -= 4096;
            }
            drop.end = drop.get_data();
            this->t->recv(&drop.end, remaining);
        }

        if (this->opt_clipboard && channel) {
            LOG(LOG_INFO, "Clipboard Channel Redirection available");

            RDPECLIP::FormatListPDU format_list_pdu;
            BStream                 out_s(16384);

            format_list_pdu.emit(out_s);

            size_t length     = out_s.size();
            size_t chunk_size =   (length < CHANNELS::CHANNEL_CHUNK_LENGTH)
                                ? length
                                : CHANNELS::CHANNEL_CHUNK_LENGTH;

            this->send_to_front_channel( (char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                       , out_s.get_data()
                                       , length
                                       , chunk_size
                                       ,   CHANNELS::CHANNEL_FLAG_FIRST
                                         | CHANNELS::CHANNEL_FLAG_LAST
                                       );
        }
        else {
            LOG(LOG_INFO, "Clipboard Channel Redirection unavailable");
        }
    } // lib_clip_data

    //==============================================================================================================
    virtual void send_to_mod_channel( const char * const front_channel_name
                                    , Stream & chunk
                                    , size_t length
                                    , uint32_t flags) {
    //==============================================================================================================
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::send_to_mod_channel");
        }

        CHANNELS::ChannelDefArray    chanlist    = this->front.get_channel_list();
        const CHANNELS::ChannelDef * mod_channel = chanlist.get(front_channel_name);

        // send it if module has a matching channel, if no matching channel is found just forget it
        if (mod_channel) {
            this->send_to_vnc(*mod_channel, chunk, length, flags);
        }
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::send_to_mod_channel done");
        }
    } // send_to_mod_channel

    //==============================================================================================================
    void send_to_vnc( const CHANNELS::ChannelDef & channel
                    , Stream & chunk
                    , size_t length
                    , uint32_t flags) {
    //==============================================================================================================
        if (this->verbose) {
            LOG( LOG_INFO, "mod_vnc::send_to_vnc length=%u chunk_size=%u flags=0x%08X"
               , (unsigned)length, (unsigned)chunk.size(), flags);
        }

        // specific treatement depending on msgType
        BStream stream(chunk.size());
        TODO("Avoid useless buffer copy, parse data (we shoudl probably pass a (sub)stream instead)")
        stream.out_copy_bytes(chunk.get_data(), chunk.size());
        stream.mark_end();
        stream.rewind();

        RDPECLIP::RecvFactory recv_factory(stream);

        switch (recv_factory.msgType) {
            // Client notify that a copy operation have occured. Two operations should be done :
            //  - Always: send a RDP acknowledge (CB_FORMAT_LIST_RESPONSE)
            //  - Only if clipboard content formats list include "NICODETEXT: send a request for it in that format
            case RDPECLIP::CB_FORMAT_LIST: {
                // Always coming from front
                LOG(LOG_INFO, "mod_vnc::send_to_vnc - receiving CB_FORMAT_LIST");

                RDPECLIP::FormatListPDU format_list_pdu;

                format_list_pdu.recv(stream, recv_factory);

                if (this->opt_clipboard && format_list_pdu.contians_data_in_text_format) {
                    //--------------------------- Beginning of clipboard PDU Header ----------------------------

                    TODO("Create a unit tested class for clipboard messages")

                    bool response_ok = true;

                    // Build and send the CB_FORMAT_LIST_RESPONSE (with status = OK)
                    // 03 00 01 00 00 00 00 00 00 00 00 00
                    RDPECLIP::FormatListResponsePDU format_list_response_pdu(response_ok);
                    BStream                         out_s(256);

                    format_list_response_pdu.emit(out_s);

                    size_t length     = out_s.size();
                    size_t chunk_size = length;

                    this->send_to_front_channel( (char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                               , out_s.get_data()
                                               , length
                                               , chunk_size
                                               ,   CHANNELS::CHANNEL_FLAG_FIRST
                                                 | CHANNELS::CHANNEL_FLAG_LAST
                                               );


                    // Build and send a CB_FORMAT_DATA_REQUEST to front (for format CF_UNICODETEXT)
                    // 04 00 00 00 04 00 00 00 0d 00 00 00
                    // 00 00 00 00
                    RDPECLIP::FormatDataRequestPDU format_data_request_pdu(RDPECLIP::CF_UNICODETEXT);
                    BStream                        out_s2(256);

                    format_data_request_pdu.emit(out_s2);

                    length     = out_s2.size();
                    chunk_size = length;

                    this->send_to_front_channel( (char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                               , out_s2.get_data()
                                               , length
                                               , chunk_size
                                               , CHANNELS::CHANNEL_FLAG_FIRST
                                               | CHANNELS::CHANNEL_FLAG_LAST
                                               );
                }
                else {
                    TODO("RZ: Don't reject clipboard update, this can block rdesktop.")
                    TODO("RZ: Create a unit tested class for clipboard messages")

                    bool response_ok = false;

                    // Build and send the CB_FORMAT_LIST_RESPONSE (with status = FAILED)
                    // 03 00 02 00 00 00 00 00
                    RDPECLIP::FormatListResponsePDU format_list_response_pdu(response_ok);
                    BStream                         out_s(256);

                    format_list_response_pdu.emit(out_s);

                    size_t length     = out_s.size();
                    size_t chunk_size = length;

                    this->send_to_front_channel( (char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                               , out_s.get_data()
                                               , length
                                               , chunk_size
                                               , CHANNELS::CHANNEL_FLAG_FIRST
                                               | CHANNELS::CHANNEL_FLAG_LAST
                                               );
                }
                break;
            }

            case RDPECLIP::CB_FORMAT_LIST_RESPONSE: {
                // Always coming from front ; do nothing, should not happen
                LOG(LOG_INFO, "mod_vnc::send_to_vnc - receiving CB_FORMAT_LIST_RESPONSE");
                break;
            }

            case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
                // Always coming from front ; Send back the clipboard buffer content
                LOG(LOG_INFO, "mod_vnc::send_to_vnc: CB_FORMAT_DATA_REQUEST");

                const unsigned expected = 10; /* msgFlags(2) + datalen(4) + requestedFormatId(4) */
                if (!stream.in_check_rem(expected)) {
                    LOG( LOG_INFO
                       , "mod_vnc::send_to_vnc truncated CB_FORMAT_DATA_REQUEST data, need=%u remains=%u"
                       , expected, stream.in_remain());
                    throw Error(ERR_VNC);
                }

                // This is a fake treatment that pretends to send the Request
                //  to VNC server. Instead, the RDP PDU is handled localy and
                //  the clipboard PDU, if any, is likewise built localy and
                //  sent back to front.
                RDPECLIP::FormatDataRequestPDU format_data_request_pdu;

                // 04 00 00 00 04 00 00 00 0d 00 00 00 00 00 00 00
                format_data_request_pdu.recv(stream, recv_factory);

                if (this->verbose) {
                    LOG( LOG_INFO
                       , "mod_vnc::send_to_vnc: CB_FORMAT_DATA_REQUEST msgFlags=0x%02x datalen=%u requestedFormatId=0x%02x"
                       , format_data_request_pdu.msgFlags
                       , format_data_request_pdu.dataLen
                       , format_data_request_pdu.requestedFormatId
                       );
                }

                // only support CF_UNICODETEXT
                if (format_data_request_pdu.requestedFormatId == RDPECLIP::CF_UNICODETEXT) {
                    // <this->to_rdp_clipboard_data> contains pre-formatted clipboard PDU.

                    size_t length     = this->to_rdp_clipboard_data.size(); /* Size of clipboard PDU header + clip data */
                    size_t PDU_remain = length;

                    uint8_t *chunk_data = this->to_rdp_clipboard_data.get_data();
                    uint32_t chunk_size;

                    int send_flags = CHANNELS::CHANNEL_FLAG_FIRST;

                    do {
                        chunk_size  = std::min<size_t>( CHANNELS::CHANNEL_CHUNK_LENGTH
                                                      , PDU_remain);
                        PDU_remain -= chunk_size;

                        send_flags |= (   (chunk_size <= 0)
                                        ? CHANNELS::CHANNEL_FLAG_LAST
                                        : CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                      );

                        this->send_to_front_channel( CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                                   , chunk_data
                                                   , length
                                                   , chunk_size
                                                   , send_flags
                                                   );

                        if ((send_flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                            break;
                        }

                        send_flags &= ~CHANNELS::CHANNEL_FLAG_FIRST;

                        chunk_data += chunk_size;
                    }
                    while (true);

                    if (this->verbose) {
                        LOG(LOG_INFO, "mod_vnc::send_to_vnc done");
                    }
                }
                else {
                    LOG( LOG_INFO
                       , "mod_vnc::send_to_vnc: resquested clipboard format Id 0x%02x is not supported by VNC PROXY"
                       , format_data_request_pdu.requestedFormatId);
                }
                break;
            }

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                LOG(LOG_INFO, "mod_vnc::send_to_vnc - receiving CB_FORMAT_DATA_RESPONSE");

                RDPECLIP::FormatDataResponsePDU format_data_response_pdu;

                format_data_response_pdu.recv(stream, recv_factory);

                if (format_data_response_pdu.msgFlags == RDPECLIP::CB_RESPONSE_OK) {

                    if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                        if (!stream.in_check_rem(format_data_response_pdu.dataLen)) {
                            LOG( LOG_INFO
                               , "mod_vnc::send_to_vnc truncated CB_FORMAT_DATA_RESPONSE dataU16, need=%u remains=%u"
                               , format_data_response_pdu.dataLen, stream.in_remain());
                            throw Error(ERR_VNC);
                        }

                        BStream dataU8(
                              format_data_response_pdu.dataLen
                            + format_data_response_pdu.dataLen / 2
                            + 1);

                        size_t len_utf8 = UTF16toUTF8( stream.p
                                                     , format_data_response_pdu.dataLen / 2
                                                     , dataU8.get_data()
                                                     , dataU8.get_capacity());

                        (dataU8.get_data())[len_utf8] = 0;

                        this->rdp_input_clip_data(dataU8.get_data(), len_utf8 + 1);
                    }
                    else {
                        // Virtual channel data span in multiple Virtual Channel PDUs.

                        if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
                            LOG(LOG_INFO, "mod_vnc::send_to_vnc flag CHANNEL_FLAG_FIRST expected");
                            throw Error(ERR_VNC);
                        }

                        if (this->verbose) {
                            LOG( LOG_INFO
                               , "mod_vnc::send_to_vnc Virtual channel data span in multiple Virtual Channel PDUs: total=%u"
                               , format_data_response_pdu.dataLen);
                        }

                        this->to_vnc_large_clipboard_data.init(2 * (MAX_VNC_2_RDP_CLIP_DATA_SIZE + 1));

                        size_t dataLenU16 = std::min<size_t>( stream.in_remain()
                                                            , this->to_vnc_large_clipboard_data.room());

                        REDASSERT(dataLenU16 != 0);

                        this->to_vnc_large_clipboard_data.out_copy_bytes(stream.p, dataLenU16);
                    }
                }
                break;
            }

            default:
                if (this->to_vnc_large_clipboard_data.get_offset() != 0) {
                    // msgType is non msgType, is a part of data.
                    stream.rewind();

                    // Virtual channel data span in multiple Virtual Channel PDUs.
                    if (this->verbose) {
                        LOG(LOG_INFO, "mod_vnc::send_to_vnc an other trunk");
                    }

                    if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) != 0) {
                        LOG(LOG_INFO, "mod_vnc::send_to_vnc flag CHANNEL_FLAG_FIRST unexpected");
                        throw Error(ERR_VNC);
                    }

                    if (this->verbose) {
                        LOG( LOG_INFO, "mod_vnc::send_to_vnc trunk size=%u, capacity=%u"
                           , stream.in_remain(), this->to_vnc_large_clipboard_data.room());
                    }

                    size_t dataLenU16 = std::min<size_t>( stream.in_remain()
                                                        , this->to_vnc_large_clipboard_data.room());

                    if (dataLenU16 != 0) {
                        this->to_vnc_large_clipboard_data.out_copy_bytes(stream.p, dataLenU16);
                    }

                    if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                        // Last chunk

                        this->to_vnc_large_clipboard_data.mark_end();

                        dataLenU16 = this->to_vnc_large_clipboard_data.size();

                        BStream dataU8(dataLenU16 + 2);

                        size_t len_utf8 = UTF16toUTF8( this->to_vnc_large_clipboard_data.get_data()
                                                     , dataLenU16 / 2, dataU8.p, dataU8.get_capacity());

                        (dataU8.get_data())[len_utf8] = 0;

                        this->rdp_input_clip_data(dataU8.get_data(), len_utf8 + 1);
                    }
                }
                else {
                    LOG(LOG_INFO, "mod_vnc::send_to_vnc: unknown message type %d", recv_factory.msgType);
                }
                break;
        }
        if (this->verbose) {
            LOG(LOG_INFO, "mod_vnc::send_to_vnc done");
        }
    } // send_to_vnc

    virtual void send_to_front_channel( const char * const mod_channel_name, uint8_t * data
                                      , size_t length, size_t chunk_size, int flags) {
        const CHANNELS::ChannelDef * front_channel = this->front.get_channel_list().get(mod_channel_name);
        if (front_channel) {
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
    }

    virtual void begin_update() {
        this->front.begin_update();
    }

    virtual void end_update() {
        this->front.begin_update();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void server_draw_text( int16_t x, int16_t y, const char * text, uint32_t fgcolor
                                 , uint32_t bgcolor, const Rect & clip) {
        this->front.server_draw_text(x, y, text, fgcolor, bgcolor, clip);
    }

    virtual void text_metrics(const char * text, int & width, int & height) {
        this->front.text_metrics(text, width, height);
    }
};

#endif
