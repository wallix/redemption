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

#if !defined(__MOD_VNC_HPP__)
#define __MOD_VNC_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>

#include "colors.hpp"

#include "stream.hpp"
#include "d3des.hpp"
#include "keymap2.hpp"
#include "keymapSym.hpp"
#include "client_mod.hpp"

// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto

struct mod_vnc : public client_mod {
    /* mod data */
    char mod_name[256];
    uint8_t mod_mouse_state;
    BGRPalette palette;
    int vnc_desktop;
    char username[256];
    char password[256];
    public:
    Transport *t;
    int clip_chanid;
    Stream clip_data;
    size_t clip_data_size;
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
    BGRPalette palette332;
    uint32_t verbose;
    KeymapSym keymapSym;
    int incr;
    wait_obj * event;

    mod_vnc ( Transport * t
            , wait_obj * event
            , const char * username
            , const char * password
            , struct FrontAPI & front
            , uint16_t front_width
            , uint16_t front_height
            , int keylayout
            , int key_flags
            , uint32_t verbose
            )
        : client_mod(front, front_width, front_height)
        , verbose(verbose)
        , keymapSym(verbose)
        , incr(0)
        , event(event)
    {
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

        this->clip_chanid = 0;
        this->clip_data_size = 0;

        strcpy(this->username, username);
        strcpy(this->password, password);

        int error = 0;

        /* protocol version */
        Stream stream(32768);
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
            case 2: /* the password and the server random */
            {
                LOG(LOG_INFO, "Receiving VNC Server Random");
                stream.init(8192);
                this->t->recv((char**)&stream.end, 16);

                /* taken from vncauth.c */
                {
                    char key[12];

                    /* key is simply password padded with nulls */
                    memset(key, 0, sizeof(key));
                    strncpy(key, this->password, 8);
                    rfbDesKey((unsigned char*)key, EN0); /* 0, encrypt */
                    rfbDes((unsigned char*)stream.data, (unsigned char*)stream.data);
                    rfbDes((unsigned char*)(stream.data + 8), (unsigned char*)(stream.data + 8));
                }
                LOG(LOG_INFO, "Sending Password");
                this->t->send(stream.data, 16);

                /* sec result */
                LOG(LOG_INFO, "Waiting for password ack");
                stream.init(8192);
                this->t->recv((char**)&stream.end, 4);
                int i = stream.in_uint32_be();
                if (i != 0) {
                    LOG(LOG_INFO, "vnc password failed\n");
                    throw 2;
                } else {
                    LOG(LOG_INFO, "vnc password ok\n");
                }
            }
            break;
            default:
                throw 1;
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
            Stream stream(32768);
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
            stream.in_skip_bytes(3); // skip padding

//            LOG(LOG_INFO, "VNC received: width=%d height=%d bpp=%d depth=%d endianess=%d true_color=%d red_max=%d green_max=%d blue_max=%d red_shift=%d green_shift=%d blue_shift=%d", this->width, this->height, this->bpp, this->depth, this->endianess, this->true_color_flag, this->red_max, this->green_max, this->blue_max, this->red_shift, this->green_shift, this->blue_shift);

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

            Stream stream(32768);
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
            this->t->send(stream.data, 20);

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
            Stream stream(32768);
            stream.out_uint8(2);
            stream.out_uint8(0);
            stream.out_uint16_be(3);
            stream.out_uint32_be(0); /* raw */
            stream.out_uint32_be(1); /* copy rect */
            stream.out_uint32_be(0xffffff11); /* cursor */

            this->t->send(stream.data, 4 + 3 * 4);
        }

        TODO("Maybe the resize should be done in session ?")
        switch (this->front.server_resize(this->width, this->height, this->bpp)){
        case 0:
            // no resizing needed
            break;
        case 1:
            // resizing done
            this->front_width = this->width;
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
        /* set almost null cursor, this is the little dot cursor */
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
        this->lib_open_clip_channel();
//            LOG(LOG_INFO, "VNC lib open clip channel ok\n");
        LOG(LOG_INFO, "VNC connection complete, connected ok\n");
        TODO("Clearing the front screen could be done in session")
        this->front.begin_update();
        RDPOpaqueRect orect(Rect(0, 0, this->width, this->height), 0);
        this->front.draw(orect, Rect(0, 0, this->width, this->height));
        this->front.end_update();

        this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));
    }

    virtual ~mod_vnc(){}

    void change_mouse_state(uint16_t x, uint16_t y, uint8_t button, bool set)
    {
        Stream stream(6);
        this->mod_mouse_state = set?(this->mod_mouse_state|button):(this->mod_mouse_state&~button); // set or clear bit
        stream.out_uint8(5);
        stream.out_uint8(this->mod_mouse_state);
        stream.out_uint16_be(x);
        stream.out_uint16_be(y);
        this->t->send(stream.data, 6);
    }


    TODO("It may be possible to change several mouse buttons at once ? Current code seems to perform several send if that occurs. Is it what we want ?")
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        Stream stream(32768);

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
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap){
        keymapSym.event(device_flags, param1);
        int key = keymapSym.get_sym();
        if (key > 0) {
            Stream stream(32768);
            stream.out_uint8(4);
            stream.out_uint8(!(device_flags & KBD_FLAG_UP)); /* down/up flag */
            stream.out_clear_bytes(2);
            stream.out_uint32_be(key);
            this->t->send(stream.data, 8);
            this->event->set(1000);
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        if (this->verbose){
            LOG(LOG_INFO, "KeymapSym::synchronize(time=%u, device_flags=%08x, param1=%04x, param1=%04x", time, device_flags, param1, param2);
        }
        this->keymapSym.synchronize(param1);
    }

    virtual void rdp_input_invalidate(const Rect & r)
    {
//        LOG(LOG_INFO, "rdp_input_invalidate");
        if (!r.isempty()) {
            Stream stream(32768);
            /* FrambufferUpdateRequest */
            stream.out_uint8(3);
            stream.out_uint8(this->incr);
            stream.out_uint16_be(r.x);
            stream.out_uint16_be(r.y);
            stream.out_uint16_be(r.cx);
            stream.out_uint16_be(r.cy);
            this->t->send(stream.data, 10);
            this->incr = 1;
        }
    }

    virtual BackEvent_t draw_event(void)
    {
        if (this->verbose){
            LOG(LOG_INFO, "vnc::draw_event");
        }
        BackEvent_t rv = BACK_EVENT_NONE;

        if (this->event->can_recv()){
            Stream stream(1);
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
                LOG(LOG_INFO, "VNC Stopped id=%u", e.id);
                rv = BACK_EVENT_1;
            }
            catch(...) {
                LOG(LOG_INFO, "exception raised");
                rv = BACK_EVENT_1;
            }
            this->event->set(1000);
        }
        else {
            this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));
        }
        return rv;
    }

    private:
    TODO(" use it for copy/paste  it is not called now")
    int lib_process_channel_data(int chanid, int flags, int size, Stream & stream, int total_size)
    {
//        if (chanid == this->clip_chanid) {
//            uint16_t type = stream.in_uint16_le();
//            uint16_t status = stream.in_uint16_le();
//            uint32_t length = stream.in_uint32_le();
////            LOG(LOG_DEBUG, "lib_process_channel_data: type=%u status=%u length=%u",
////                (unsigned)type, (unsigned)status, (unsigned)length);
//            switch (type) {
//            case 2:
//            { /* CLIPRDR_FORMAT_ANNOUNCE */
//                Stream out_s(8192);
//                out_s.out_uint16_le(3);
//                out_s.out_uint16_le(1);
//                out_s.out_uint32_le(0);
//                out_s.out_clear_bytes(4); /* pad */
//                // this->send_to_front_channel(
//            }
//            break;
//            case 3: /* CLIPRDR_FORMAT_ACK */
//                break;
//            case 4:
//            { /* CLIPRDR_DATA_REQUEST */
//                uint32_t format = 0;
//                if (length >= 4) {
//                    format = stream.in_uint32_le();
//                }
//                /* only support CF_TEXT and CF_UNICODETEXT */
//                if ((format != 1) && (format != 13)) {
//                    break;
//                }
//                Stream out_s(8192);
//                out_s.out_uint16_le(5);
//                out_s.out_uint16_le(1);
//                if (format == 13) { /* CF_UNICODETEXT */
//                    out_s.out_uint32_le(this->clip_data_size * 2 + 2);
//                    for (size_t index = 0; index < this->clip_data_size; index++) {
//                        out_s.out_uint8(this->clip_data.data[index]);
//                        out_s.out_uint8(0);
//                    }
//                    out_s.out_clear_bytes(2);
//                }
//                else if (format == 1) { /* CF_TEXT */
//                    out_s.out_uint32_le(this->clip_data_size + 1);
//                    for (size_t index = 0; index < this->clip_data_size; index++) {
//                        out_s.out_uint8(this->clip_data.data[index]);
//                    }
//                    out_s.out_clear_bytes(1);
//                }
//                out_s.out_clear_bytes(4); /* pad */
//                // this->send_to_front_channel(
//            }
//            break;
//            }
//        } else {
//            LOG(LOG_INFO, "lib_process_channel_data: unknown chanid %d"
//                   " this->clip_chanid %d\n",
//                      chanid, this->clip_chanid);
//        }
        return 0;
    }

    void lib_framebuffer_update() throw (Error)
    {
        Stream stream(256);
        this->t->recv((char**)&stream.end, 3);
        stream.in_skip_bytes(1);
        size_t num_recs = stream.in_uint16_be();

        uint8_t Bpp = nbbytes(this->bpp);
        stream.init(256);
        for (size_t i = 0; i < num_recs; i++) {
            this->t->recv((char**)&stream.end, 12);
            uint16_t x = stream.in_uint16_be();
            uint16_t y = stream.in_uint16_be();
            uint16_t cx = stream.in_uint16_be();
            uint16_t cy = stream.in_uint16_be();
            uint32_t encoding = stream.in_uint32_be();

            switch (encoding){
            case 0: /* raw */
            {
                uint8_t * raw = (uint8_t *)malloc(cx*16*Bpp);
                if (!raw){
                    LOG(LOG_ERR, "Memory allocation failed for raw buffer in VNC");
                    throw Error(ERR_VNC_MEMORY_ALLOCATION_FAILED);
                }

                for (uint16_t yy = y ; yy < y + cy ; yy += 16){
                    uint8_t * tmp = raw;
                    uint16_t cyy = std::min<uint16_t>(16, cy-(yy-y));
                    this->t->recv((char**)&tmp, cyy*cx*Bpp);
                    this->front.begin_update();
//                    LOG(LOG_INFO, "draw vnc: x=%d y=%d cx=%d cy=%d", x, yy, cx, cyy);
                    this->front.draw_vnc(Rect(x, yy, cx, cyy), this->bpp, this->palette332, raw, cx*16*Bpp);
                    this->front.end_update();
                }
                free(raw);
            }
            break;
            case 1: /* copy rect */
            {
                Stream stream(4);
                this->t->recv((char**)&stream.end, 4);
                const int srcx = stream.in_uint16_be();
                const int srcy = stream.in_uint16_be();
//                LOG(LOG_INFO, "copy rect: x=%d y=%d cx=%d cy=%d encoding=%d src_x=%d, src_y=%d", x, y, cx, cy, encoding, srcx, srcy);
                const RDPScrBlt scrblt(Rect(x, y, cx, cy), 0xCC, srcx, srcy);
                this->front.begin_update();
                this->front.draw(scrblt, Rect(0, 0, this->front_width, this->front_height));
                this->front.end_update();
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

                TODO("The code below is likely to explain the yellow pointer: we ask for 16 bits for VNC, but we work with cursor as if it were 24 bits. We should use decode primitives and reencode it appropriately. Cursor has the right shape because the mask use is 1 bit per pixel arrays")
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
                                TODO("temporary: force black cursor")
                                rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 0] = 0;
                                rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 1] = 0;
                                rdp_cursor_data[((31-yy) * 32 + xx) * 3 + 2] = 0;
                            }
                        }
                    }
                }

                /* keep these in 32x32, vnc cursor can be alot bigger */
                /* (anyway hotspot is usually 0, 0)                   */
                if (x > 31) { x = 31; }
                if (y > 31) { y = 31; }
TODO(" we should manage cursors bigger then 32 x 32  this is not an RDP protocol limitation")
                this->front.begin_update();
                this->front.server_set_pointer(x, y, rdp_cursor_data, rdp_cursor_mask);
                this->front.end_update();
            }
            break;
            default:
                LOG(LOG_INFO, "unexpected encoding %8x in lib_frame_buffer", encoding);
//                throw Error(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER);
                break;
            }
        }

        this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));
    }

    void lib_clip_data(void)
    {
        Stream stream(32768);
        this->t->recv((char**)&stream.end, 7);
        stream.in_skip_bytes(3);
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
        // this->send_to_front_channel(
    }

    /******************************************************************************/
    void lib_palette_update(void)
    {
        Stream stream(32768);
        this->t->recv((char**)&stream.end, 5);
        stream.in_skip_bytes(1);
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

        this->front.set_mod_palette(this->palette);
        this->front.send_global_palette();
        this->front.begin_update();
        this->front.color_cache(this->palette, 0);
        this->front.end_update();
    }

    /******************************************************************************/
    void lib_open_clip_channel(void)
    {
        TODO(" not working  see why")
        return;
//        uint8_t init_data[12] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//        this->clip_chanid = this->server_get_channel_id((char*)"cliprdr");

//        if (this->clip_chanid >= 0) {
            // this->send_to_front_channel(
//        }
    }
};

#endif
