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
#include "RDP/pointer.hpp"

#include "version.hpp"

#include "internal/widget2/flat_vnc_authentification.hpp"
#include "internal/internal_mod.hpp"
#include "internal/widget2/notify_api.hpp"

// got extracts of VNC documentation from
// http://tigervnc.sourceforge.net/cgi-bin/rfbproto

#define MAX_VNC_2_RDP_CLIP_DATA_SIZE 8000

//###############################################################################################################
struct mod_vnc : public InternalMod, public NotifyApi {
//###############################################################################################################
    FlatVNCAuthentification challenge;

    /* mod data */
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

    z_stream zstrm;

    enum {
        ASK_PASSWORD,
        DO_INITIAL_CLEAR_SCREEN,
        RETRY_CONNECTION,
        UP_AND_RUNNING,
        WAIT_PASSWORD,
        WAIT_SECURITY_TYPES,
        WAIT_CLIENT_UP_AND_RUNNING
    };

    redemption::string encodings;

    int state;

    Inifile & ini;

    //==============================================================================================================
    mod_vnc( Transport * t
           , Inifile & ini
           , const char * username
           , const char * password
           , struct FrontAPI & front
           , uint16_t front_width
           , uint16_t front_height
           , int keylayout
           , int key_flags
           , bool clipboard
           , const char * encodings
           , uint32_t verbose
           )
    //==============================================================================================================
            : InternalMod(front, front_width, front_height)
            , challenge(*this, front_width, front_height, this->screen, this, "Redemption " VERSION,
                  0, 0, WHITE, DARK_BLUE_BIS,
                  TR("password", ini))
            , verbose(verbose)
            , keymapSym(verbose)
            , incr(0)
            , to_vnc_large_clipboard_data(2 * MAX_VNC_2_RDP_CLIP_DATA_SIZE + 2)
            , opt_clipboard(clipboard)
            , state(WAIT_SECURITY_TYPES)
            , ini(ini) {
    //--------------------------------------------------------------------------------------------------------------
        LOG(LOG_INFO, "Creation of new mod 'VNC'");

        memset(&zstrm, 0, sizeof(zstrm));
        if (inflateInit(&this->zstrm) != Z_OK)
        {
            LOG(LOG_ERR, "vnc zlib initialization failed");
            throw Error(ERR_VNC_ZLIB_INITIALIZATION);
        }

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

        memcpy(this->username, username, sizeof(this->username)-1);
        this->username[sizeof(this->username)-1] = 0;
        memcpy(this->password, password, sizeof(this->password)-1);
        this->password[sizeof(this->password)-1] = 0;

        this->encodings.copy_c_str(encodings);

        LOG(LOG_INFO, "Creation of new mod 'VNC' done");
    } // Constructor

    //==============================================================================================================
    virtual ~mod_vnc()
    {
        inflateEnd(&this->zstrm);

        this->screen.clear();
    }
    //==============================================================================================================

    static void fill_encoding_types_buffer(const char * encodings, Stream & stream, uint16_t & number_of_encodings, uint32_t verbose)
    {
        number_of_encodings = 0;

        const char * tmp_encoding  = encodings;
        int32_t      encoding_type;

        if (verbose) {
            LOG(LOG_INFO, "VNC Encodings=\"%s\"", encodings);
        }

        while (*tmp_encoding)
        {
            encoding_type = long_from_cstr(tmp_encoding);
            if (verbose) {
                LOG(LOG_INFO, "VNC Encoding type=0x%08X", encoding_type);
            }
            stream.out_uint32_be(*(uint32_t *)((void *)&encoding_type));
            number_of_encodings++;

            tmp_encoding = strchr(tmp_encoding, ',');
            if (!tmp_encoding)
            {
                break;
            }

            while ((*tmp_encoding == ',') || (*tmp_encoding == ' ') || (*tmp_encoding == '\t'))
                tmp_encoding++;
        }
    }

    static long long_from_cstr(const char * str)
    { // 10 = 10, 0x10 = 16
        if ((*str == '0') && (*(str + 1) == 'x')){
            return strtol(str + 2, 0, 16);
        }

        return atol(str);
    }

    //==============================================================================================================
    void change_mouse_state( uint16_t x
                           , uint16_t y
                           , uint8_t button
                           , bool set
                           ) {
    //==============================================================================================================
        if (this->state != UP_AND_RUNNING) {
            return;
        }

        BStream stream(6);
        this->mod_mouse_state = set?(this->mod_mouse_state|button):(this->mod_mouse_state&~button); // set or clear bit
        stream.out_uint8(5);
        stream.out_uint8(this->mod_mouse_state);
        stream.out_uint16_be(x);
        stream.out_uint16_be(y);
        this->t->send(stream.get_data(), 6);
    } // change_mouse_state

    TODO("It may be possible to change several mouse buttons at once ? Current code seems to perform several send if that occurs. Is it what we want ?");
    //==============================================================================================================
    virtual void rdp_input_mouse( int device_flags
                                , int x
                                , int y
                                , Keymap2 * keymap
                                ) {
    //==============================================================================================================
        if (this->state == WAIT_PASSWORD) {
            this->screen.rdp_input_mouse(device_flags, x, y, keymap);
            return;
        }

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
        if (this->state == WAIT_PASSWORD) {
            this->screen.rdp_input_scancode(param1, param2, device_flags, param4, keymap);
            return;
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

        TODO("As down/up state is not stored in keymapSym, code below is quite dangerous");
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
        if (this->state != UP_AND_RUNNING) {
            return;
        }

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
        if (this->state == WAIT_PASSWORD) {
            this->screen.rdp_input_invalidate(r);
            return;
        }

        if (this->state != UP_AND_RUNNING) {
            return;
        }

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
    virtual void draw_event(time_t now)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "vnc::draw_event");
        }
        switch (this->state)
        {
        case ASK_PASSWORD:
            if (this->verbose) {
                LOG(LOG_INFO, "state=ASK_PASSWORD");
            }
            this->screen.add_widget(&this->challenge);

            this->screen.set_widget_focus(&this->challenge);

            this->challenge.password_edit.set_text("");

            this->challenge.set_widget_focus(&this->challenge.password_edit);

            this->screen.refresh(this->screen.rect);

            this->state = WAIT_PASSWORD;
            break;
        case DO_INITIAL_CLEAR_SCREEN:
            {
                if (this->verbose) {
                    LOG(LOG_INFO, "state=DO_INITIAL_CLEAR_SCREEN");
                }
                // set almost null cursor, this is the little dot cursor
                struct Pointer cursor;
                cursor.x = 3;
                cursor.y = 3;
                memset(cursor.data + 31 * (32 * 3), 0xff, 9);
                memset(cursor.data + 30 * (32 * 3), 0xff, 9);
                memset(cursor.data + 29 * (32 * 3), 0xff, 9);
                memset(cursor.mask, 0xff, 32 * (32 / 8));
                this->front.server_set_pointer(cursor);

                LOG(LOG_INFO, "VNC connection complete, connected ok\n");
                TODO("Clearing the front screen could be done in session");
                this->front.begin_update();
                RDPOpaqueRect orect(Rect(0, 0, this->width, this->height), 0);
                this->front.draw(orect, Rect(0, 0, this->width, this->height));
                this->front.end_update();

                this->state = UP_AND_RUNNING;

                this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));

                this->lib_open_clip_channel();

                this->event.object_and_time = false;
                if (this->verbose) {
                    LOG(LOG_INFO, "VNC screen cleaning ok\n");
                }
            }
            break;
        case RETRY_CONNECTION:
            if (this->verbose) {
                LOG(LOG_INFO, "state=RETRY_CONNECTION");
            }
            try
            {
                this->t->connect();
                this->event.obj = ((SocketTransport *)this->t)->sck;
            }
            catch (Error e)
            {
                throw Error(ERR_VNC_CONNECTION_ERROR);
            }

            this->state = WAIT_SECURITY_TYPES;
            this->event.set();
            break;
        case UP_AND_RUNNING:
            if (this->verbose) {
                LOG(LOG_INFO, "state=UP_AND_RUNNING");
            }
            if (this->event.can_recv()) {
                BStream stream(1);
                try {
                    this->t->recv(&stream.end, 1);
                    char type = stream.in_uint8();  /* message-type */
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
                        break;
                    }
                }
                catch (const Error & e) {
                    LOG(LOG_INFO, "VNC Stopped [reason id=%u]", e.id);
                    this->event.signal = BACK_EVENT_NEXT;
                }
                catch (...) {
                    LOG(LOG_INFO, "unexpected exception raised in VNC");
                    this->event.signal = BACK_EVENT_NEXT;
                }
                if (this->event.signal != BACK_EVENT_NEXT) {
                    this->event.set(1000);
                }
            }
            else {
                this->rdp_input_invalidate(Rect(0, 0, this->width, this->height));
            }
            break;
        case WAIT_PASSWORD:
            if (this->verbose) {
                LOG(LOG_INFO, "state=WAIT_PASSWORD");
            }
            this->event.object_and_time = false;
            this->event.reset();
            break;
        case WAIT_SECURITY_TYPES:
            {
                if (this->verbose) {
                    LOG(LOG_INFO, "state=WAIT_SECURITY_TYPES");
                }
                BStream stream(32768);

                /* protocol version */
                this->t->recv(&stream.end, 12);
                this->t->send("RFB 003.003\n", 12);

                // sec type
                stream.init(8192);
                this->t->recv(&stream.end, 4);
                int security_level = stream.in_uint32_be();
                if (this->verbose) {
                    LOG(LOG_INFO, "security level is %d (1 = none, 2 = standard)\n",
                        security_level);
                }

                switch (security_level){
                    case 1: // none
                        break;
                    case 2: // the password and the server random
                    {
                        if (this->verbose) {
                            LOG(LOG_INFO, "Receiving VNC Server Random");
                        }
                        stream.init(8192);
                        this->t->recv(&stream.end, 16);

                        // taken from vncauth.c
                        {
                            char key[12];

                            // key is simply password padded with nulls
                            memset(key, 0, sizeof(key));
                            strncpy(key, this->password, 8);
                            rfbDesKey((unsigned char*)key, EN0); // 0, encrypt
                            rfbDes((unsigned char*)stream.get_data(), (unsigned char*)stream.get_data());
                            rfbDes((unsigned char*)(stream.get_data() + 8), (unsigned char*)(stream.get_data() + 8));
                        }
                        if (this->verbose) {
                            LOG(LOG_INFO, "Sending Password");
                        }
                        this->t->send(stream.get_data(), 16);

                        // sec result
                        if (this->verbose) {
                            LOG(LOG_INFO, "Waiting for password ack");
                        }
                        stream.init(8192);
                        this->t->recv(&stream.end, 4);
                        int i = stream.in_uint32_be();
                        if (i != 0) {
                            LOG(LOG_ERR, "vnc password failed");

                            // Optionnal
                            try
                            {
                                this->t->recv(&stream.end, 4);
                                uint32_t reason_length = stream.in_uint32_be();

                                char   reason[256];
                                char * preason = reason;

                                this->t->recv(&preason, std::min<size_t>(sizeof(reason) - 1, reason_length));
                                *preason = 0;

                                LOG(LOG_INFO, "Reason for the connection failure: %s", preason);
                            }
                            catch (Error e)
                            {
                            }

                            this->event.obj = -1;
                            this->t->disconnect();

                            this->state = ASK_PASSWORD;
                            this->event.object_and_time = true;
                            this->event.set();

                            return;
                        } else {
                            if (this->verbose) {
                                LOG(LOG_INFO, "vnc password ok\n");
                            }
                        }
                    }
                    break;
                    default:
                        LOG(LOG_ERR, "vnc unexpected security level");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                }
                this->t->send("\x01", 1); // share flag

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

                TODO(" not yet supported");
                // If the Tight Security Type is activated, the server init
                // message is extended with an interaction capabilities section.

                {
                    BStream stream(32768);
                    this->t->recv(&stream.end, 24); // server init
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
                        LOG(LOG_ERR, "VNC connection error");
                        throw Error(ERR_VNC_CONNECTION_ERROR);
                    }
                    char * end = this->mod_name;
                    this->t->recv(&end, lg);
                    this->mod_name[lg] = 0;
                }

                // should be connected

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
                    const char * encodings           = this->encodings.c_str();
                    uint16_t     number_of_encodings = 0;

                    // SetEncodings
                    BStream stream(32768);
                    stream.out_uint8(2);
                    stream.out_uint8(0);

                    uint32_t number_of_encodings_offset = stream.get_offset();
                    stream.out_clear_bytes(2);

                    this->fill_encoding_types_buffer(encodings, stream, number_of_encodings, this->verbose);

                    if (!number_of_encodings)
                    {
                        if (this->verbose) {
                            LOG(LOG_WARNING, "mdo_vnc: using default encoding types - RRE(2),Raw(0),CopyRect(1),Cursor pseudo-encoding(-239)");
                        }

                        stream.out_uint32_be(2);            // RRE
                        stream.out_uint32_be(0);            // raw
                        stream.out_uint32_be(1);            // copy rect
                        stream.out_uint32_be(0xffffff11);   // cursor

                        stream.set_out_uint16_be(4, number_of_encodings_offset);
                        this->t->send(stream.get_data(),
                            20  // 4 + 4 * 4
                            );
                    }
                    else
                    {
                        stream.set_out_uint16_be(number_of_encodings, number_of_encodings_offset);
                        this->t->send(stream.get_data(), 4 + number_of_encodings * 4);
                    }
                }

                TODO("Maybe the resize should be done in session ?");
                switch (this->front.server_resize(this->width, this->height, this->bpp)){
                case 0:
                    if (this->verbose) {
                        LOG(LOG_INFO, "no resizing needed");
                    }
                    // no resizing needed
                    this->state = DO_INITIAL_CLEAR_SCREEN;
                    this->event.object_and_time = true;
                    this->event.set();
                    break;
                case 1:
                    if (this->verbose) {
                        LOG(LOG_INFO, "resizing done");
                    }
                    // resizing done
                    this->front_width  = this->width;
                    this->front_height = this->height;

                    this->state = WAIT_CLIENT_UP_AND_RUNNING;
                    this->event.object_and_time = true;
                    break;
                case -1:
                    // resizing failed
                    // thow an Error ?
                    LOG(LOG_WARNING, "Older RDP client can't resize to server asked resolution, disconnecting");
                    throw Error(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE);
                    break;
                }
            }
            break;
        default:
            LOG(LOG_ERR, "Unknown state=%d", this->state);
            throw Error(ERR_VNC);
            break;
        }
    } // draw_event

    private:
    struct ZRLEUpdateContext
    {
        uint8_t Bpp;

        uint16_t x;
        uint16_t cx;

        uint16_t cx_remain;
        uint16_t cy_remain;

        uint16_t tile_x;
        uint16_t tile_y;

        BStream data_remain;

        ZRLEUpdateContext() : data_remain(16384) {}
    };
    void lib_framebuffer_update_zrle(HStream & uncompressed_data_buffer,
        ZRLEUpdateContext & update_context)
    {
        if (this->verbose) {
            LOG(LOG_INFO,
                "VNC Encoding: ZRLE, uncompressed length=%lu remaining data size=%lu",
                uncompressed_data_buffer.in_remain(),
                update_context.data_remain.size());
        }

        if (update_context.data_remain.size())
        {
            uncompressed_data_buffer.copy_to_head(
                update_context.data_remain.get_data(),
                update_context.data_remain.size());
            uncompressed_data_buffer.p = uncompressed_data_buffer.get_data();

            update_context.data_remain.reset();
        }

        uint16_t   tile_cx;
        uint16_t   tile_cy;
        uint8_t    tile_data[16384];    // max size with 16 bpp
        uint16_t   tile_data_length;
        uint16_t   tile_data_length_remain;

        uint8_t  * remaining_data        = NULL;
        uint16_t   remaining_data_length = 0;
        uint8_t    subencoding           = 127;     // Unauthorized value

        try
        {
            while (uncompressed_data_buffer.in_remain())
            {
                tile_cx = std::min<uint16_t>(update_context.cx_remain, 64);
                tile_cy = std::min<uint16_t>(update_context.cy_remain, 64);

                const uint8_t * tile_data_p = tile_data;

                tile_data_length = tile_cx * tile_cy * update_context.Bpp;
                if (tile_data_length > sizeof(tile_data))
                {
                    LOG(LOG_ERR,
                        "VNC Encoding: ZRLE, tile buffer too small (%u < %u)",
                        sizeof(tile_data), tile_data_length);
                    throw Error(ERR_BUFFER_TOO_SMALL);
                }

                remaining_data        = uncompressed_data_buffer.p;
                remaining_data_length = uncompressed_data_buffer.in_remain();

                subencoding = uncompressed_data_buffer.in_uint8();

                if (this->verbose) {
                    LOG(LOG_INFO, "VNC Encoding: ZRLE, subencoding = %d",
                        subencoding);
                }

                if (!subencoding)
                {
                    if (this->verbose) {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, Raw pixel data");
                    }

                    if (uncompressed_data_buffer.in_remain() < tile_data_length)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    tile_data_p = uncompressed_data_buffer.in_uint8p(tile_data_length);
                }
                else if (subencoding == 1)
                {
                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "VNC Encoding: ZRLE, Solid tile (single color)");
                    }

                    if (uncompressed_data_buffer.in_remain() < update_context.Bpp)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(update_context.Bpp);

                    uint8_t * tmp_tile_data = tile_data;

                    for (int i = 0; i < tile_cx; i++, tmp_tile_data += update_context.Bpp)
                        memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                    uint16_t line_size = tile_cx * update_context.Bpp;

                    for (int i = 1; i < tile_cy; i++, tmp_tile_data += line_size)
                        memcpy(tmp_tile_data, tile_data, line_size);
                }
                else if ((subencoding >= 2) && (subencoding <= 16))
                {
                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "VNC Encoding: ZRLE, Packed palette types, "
                                "palette size=%d",
                            subencoding);
                    }

                    const uint8_t  * palette;
                    const uint8_t    palette_count = subencoding;
                    const uint16_t   palette_size  = palette_count * update_context.Bpp;

                    if (uncompressed_data_buffer.in_remain() < palette_size)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    palette = uncompressed_data_buffer.in_uint8p(palette_size);

                    uint16_t   packed_pixels_length;

                    if (palette_count == 2)
                    {
                        packed_pixels_length = (tile_cx + 7) / 8 * tile_cy;
                    }
                    else if ((palette_count == 3) || (palette_count == 4))
                    {
                        packed_pixels_length = (tile_cx + 3) / 4 * tile_cy;
                    }
                    else// if ((palette_count >= 5) && (palette_count <= 16))
                    {
                        packed_pixels_length = (tile_cx + 1) / 2 * tile_cy;
                    }

                    if (uncompressed_data_buffer.in_remain() < packed_pixels_length)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    const uint8_t * packed_pixels = uncompressed_data_buffer.in_uint8p(packed_pixels_length);

                    uint8_t * tmp_tile_data = tile_data;

                    tile_data_length_remain = tile_data_length;

                    uint8_t         pixel_remain         = tile_cx;
                    const uint8_t * packed_pixels_remain = packed_pixels;
                    uint8_t         current              = 0;
                    uint8_t         index                = 0;

                    uint8_t palette_index;

                    while (tile_data_length_remain >= update_context.Bpp)
                    {
                        pixel_remain--;

                        if (!index)
                        {
                            current = *packed_pixels_remain;
                            packed_pixels_remain++;
                        }

                        if (palette_count == 2)
                        {
                            palette_index = (current & 0x80) >> 7;
                            current <<= 1;
                            index++;

                            if (!pixel_remain || (index > 7))
                            {
                                index = 0;
                            }
                        }
                        else if ((palette_count == 3) || (palette_count == 4))
                        {
                            palette_index = (current & 0xC0) >> 6;
                            current <<= 2;
                            index++;

                            if (!pixel_remain || (index > 3))
                            {
                                index = 0;
                            }
                        }
                        else// if ((palette_count >= 5) && (palette_count <= 16))
                        {
                            palette_index = (current & 0xF0) >> 4;
                            current <<= 4;
                            index++;

                            if (!pixel_remain || (index > 1))
                            {
                                index = 0;
                            }
                        }

                        if (!pixel_remain)
                        {
                            pixel_remain = tile_cx;
                        }

                        const uint8_t * cpixel_pattern = palette + palette_index * update_context.Bpp;

                        memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                        tmp_tile_data           += update_context.Bpp;
                        tile_data_length_remain -= update_context.Bpp;
                    }
                }
                else if ((subencoding >= 17) && (subencoding <= 127))
                {
                    LOG(LOG_ERR, "VNC Encoding: ZRLE, unused");
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                else if (subencoding == 128)
                {
                    if (this->verbose) {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, Plain RLE");
                    }

                    tile_data_length_remain = tile_data_length;

                    uint16_t   run_length    = 0;
                    uint8_t  * tmp_tile_data = tile_data;

                    while (tile_data_length_remain >= update_context.Bpp)
                    {

                        if (uncompressed_data_buffer.in_remain() < update_context.Bpp)
                        {
                            throw Error(ERR_VNC_NEED_MORE_DATA);
                        }

                        const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(update_context.Bpp);

                        run_length = 1;

                        while (true)
                        {
                            if (uncompressed_data_buffer.in_remain() < 1)
                            {
                                throw Error(ERR_VNC_NEED_MORE_DATA);
                            }

                            uint8_t byte_value = uncompressed_data_buffer.in_uint8();
                            run_length += byte_value;

                            if (byte_value != 255)
                                break;
                        }

                        // LOG(LOG_INFO, "VNC Encoding: ZRLE, run length=%u", run_length);

                        while ((tile_data_length_remain >= update_context.Bpp) && run_length)
                        {
                            memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                            tmp_tile_data           += update_context.Bpp;
                            tile_data_length_remain -= update_context.Bpp;

                            run_length--;
                        }
                    }

                    // LOG(LOG_INFO, "VNC Encoding: ZRLE, run_length=%u", run_length);

                    REDASSERT(!run_length);
                    REDASSERT(!tile_data_length_remain);
                }
                else if (subencoding == 129)
                {
                    LOG(LOG_ERR, "VNC Encoding: ZRLE, unused");
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                else
                {
                    if (this->verbose) {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, Palette RLE");
                    }

                    const uint8_t  * palette;
                    const uint8_t    palette_count = subencoding - 128;
                    const uint16_t   palette_size  = palette_count * update_context.Bpp;

                    if (uncompressed_data_buffer.in_remain() < palette_size)
                    {
                        throw Error(ERR_VNC_NEED_MORE_DATA);
                    }

                    palette = uncompressed_data_buffer.in_uint8p(palette_size);

                    tile_data_length_remain = tile_data_length;

                    uint16_t   run_length    = 0;
                    uint8_t  * tmp_tile_data = tile_data;

                    while (tile_data_length_remain >= update_context.Bpp)
                    {
                        if (uncompressed_data_buffer.in_remain() < 1)
                        {
                            throw Error(ERR_VNC_NEED_MORE_DATA);
                        }

                        uint8_t         palette_index  = uncompressed_data_buffer.in_uint8();
                        const uint8_t * cpixel_pattern = palette + (palette_index & 0x7F) * update_context.Bpp;

                        run_length = 1;

                        if (palette_index & 0x80)
                        {
                            while (true)
                            {
                                if (uncompressed_data_buffer.in_remain() < 1)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                uint8_t byte_value = uncompressed_data_buffer.in_uint8();
                                run_length += byte_value;

                                if (byte_value != 255)
                                    break;
                            }
                        }

                        // LOG(LOG_INFO, "VNC Encoding: ZRLE, run length=%u", run_length);

                        while ((tile_data_length_remain >= update_context.Bpp) && run_length)
                        {
                            memcpy(tmp_tile_data, cpixel_pattern, update_context.Bpp);

                            tmp_tile_data           += update_context.Bpp;
                            tile_data_length_remain -= update_context.Bpp;

                            run_length--;
                        }
                    }

                    // LOG(LOG_INFO, "VNC Encoding: ZRLE, run_length=%u", run_length);

                    REDASSERT(!run_length);
                    REDASSERT(!tile_data_length_remain);
                }

                this->front.begin_update();
                this->front.draw_vnc(
                    Rect(update_context.tile_x, update_context.tile_y,
                        tile_cx, tile_cy),
                    this->bpp, this->palette332, tile_data_p,
                    tile_data_length);
                this->front.end_update();

                update_context.cx_remain -= tile_cx;
                update_context.tile_x    += tile_cx;

                if (!update_context.cx_remain)
                {
                    update_context.cx_remain =  update_context.cx;
                    update_context.cy_remain -= tile_cy;

                    update_context.tile_x =  update_context.x;
                    update_context.tile_y += tile_cy;
                }
            }
        }
        catch (Error & e)
        {
            if (e.id != ERR_VNC_NEED_MORE_DATA)
                throw;
            else
            {
                update_context.data_remain.out_copy_bytes(remaining_data,
                    remaining_data_length);
                update_context.data_remain.mark_end();
            }
        }
    }
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
            case 2: /* RRE */
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
            case 5: /* Hextile */
LOG(LOG_INFO, "VNC Encoding: Hextile, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
            break;
            case 16:    /* ZRLE */
            {
//LOG(LOG_INFO, "VNC Encoding: ZRLE, Bpp = %u, x=%u, y=%u, cx=%u, cy=%u", Bpp, x, y, cx, cy);
                this->t->recv(&stream.end, 4);

                uint32_t zlib_compressed_data_length = stream.in_uint32_be();

                if (this->verbose)
                {
                    LOG(LOG_INFO, "VNC Encoding: ZRLE, compressed length = %u",
                        zlib_compressed_data_length);
                }

                if (zlib_compressed_data_length > 65536)
                {
                    LOG(LOG_ERR,
                        "VNC Encoding: ZRLE, compressed data buffer too small "
                            "(65536 < %lu)",
                        zlib_compressed_data_length);
                    throw Error(ERR_BUFFER_TOO_SMALL);
                }

                BStream zlib_compressed_data(65536);
                this->t->recv(&zlib_compressed_data.end,
                    zlib_compressed_data_length);
                REDASSERT(zlib_compressed_data.in_remain() == zlib_compressed_data_length);

                ZRLEUpdateContext zrle_update_context;

                zrle_update_context.Bpp       = Bpp;
                zrle_update_context.x         = x;
                zrle_update_context.cx        = cx;
                zrle_update_context.cx_remain = cx;
                zrle_update_context.cy_remain = cy;
                zrle_update_context.tile_x    = x;
                zrle_update_context.tile_y    = y;

                zstrm.avail_in = zlib_compressed_data_length;
                zstrm.next_in  = zlib_compressed_data.get_data();

                while (zstrm.avail_in > 0)
                {
                    HStream zlib_uncompressed_data_buffer(16384, 49152);

                    zstrm.avail_out = zlib_uncompressed_data_buffer.get_capacity();
                    zstrm.next_out  = zlib_uncompressed_data_buffer.get_data();

                    int zlib_result = inflate(&zstrm, Z_NO_FLUSH);

                    if (zlib_result != Z_OK)
                    {
                        LOG(LOG_ERR, "vnc zlib decompression failed (%d)", zlib_result);
                        throw Error(ERR_VNC_ZLIB_INFLATE);
                    }

                    zlib_uncompressed_data_buffer.out_skip_bytes(
                        zlib_uncompressed_data_buffer.get_capacity() - zstrm.avail_out);
                    zlib_uncompressed_data_buffer.mark_end();
                    zlib_uncompressed_data_buffer.rewind();

                    this->lib_framebuffer_update_zrle(
                        zlib_uncompressed_data_buffer, zrle_update_context);
                }
            }
            break;
            case 0xffffff11: /* cursor */
            TODO(" see why we get these empty rects ?");
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

                struct Pointer cursor;
                cursor.x = 3;
                cursor.y = 3;

                // a VNC pointer of 1x1 size is not visible, so a default minimal pointer (dot pointer) is provided instead
                if (cx == 1 && cy == 1) {
                    TODO("Appearence of this 1x1 cursor looks broken, check what we actually get");
                    memset(cursor.data, 0, sizeof(cursor.data));
                    cursor.data[2883] = 0xFF;
                    cursor.data[2884] = 0xFF;
                    cursor.data[2885] = 0xFF;
                    memset(cursor.mask, 0xFF, sizeof(cursor.mask));
                    cursor.mask[116] = 0x1F;
                    cursor.mask[120] = 0x1F;
                    cursor.mask[124] = 0x1F;
                }
                else {
                    // clear target cursor mask
                    for (size_t tmpy = 0; tmpy < 32; tmpy++) {
                        for (size_t mask_x = 0; mask_x < nbbytes(32); mask_x++) {
                            cursor.mask[tmpy*nbbytes(32) + mask_x] = 0xFF;
                        }
                    }
                    TODO("The code below is likely to explain the yellow pointer: we ask for 16 bits for VNC, but we work with cursor as if it were 24 bits. We should use decode primitives and reencode it appropriately. Cursor has the right shape because the mask used is 1 bit per pixel arrays");
                    // copy vnc pointer and mask to rdp pointer and mask

                    for (int yy = 0; yy < cy; yy++) {
                        for (int xx = 0 ; xx < cx ; xx++){
                            if (vnc_pointer_mask[yy * nbbytes(cx) + xx / 8 ] & (0x80 >> (xx&7))){
                                if ((yy < 32) && (xx < 32)){
                                    cursor.mask[(31-yy) * nbbytes(32) + (xx / 8)] &= ~(0x80 >> (xx&7));
                                    int pixel = 0;
                                    for (int tt = 0 ; tt < Bpp; tt++){
                                        pixel += vnc_pointer_data[(yy * cx + xx) * Bpp + tt] << (8 * tt);
                                    }
                                    TODO("temporary: force black cursor");
                                    int red   = (pixel >> this->red_shift) & red_max;
                                    int green = (pixel >> this->green_shift) & green_max;
                                    int blue  = (pixel >> this->blue_shift) & blue_max;
                                    cursor.data[((31-yy) * 32 + xx) * 3 + 0] = (red << 3) | (red >> 2);
                                    cursor.data[((31-yy) * 32 + xx) * 3 + 1] = (green << 2) | (green >> 4);
                                    cursor.data[((31-yy) * 32 + xx) * 3 + 2] = (blue << 3) | (blue >> 2);
                                }
                            }
                        }
                    }
                    /* keep these in 32x32, vnc cursor can be alot bigger */
                    /* (anyway hotspot is usually 0, 0)                   */
                    if (x > 31) { x = 31; }
                    if (y > 31) { y = 31; }
                }
TODO(" we should manage cursors bigger then 32 x 32  this is not an RDP protocol limitation");
                this->front.begin_update();
                this->front.server_set_pointer(cursor);
                this->front.end_update();
            }
            break;
            default:
                LOG(LOG_ERR, "unexpected encoding %8x in lib_frame_buffer", encoding);
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
        const CHANNELS::ChannelDef * channel =
            chanlist.get_by_name(CLIPBOARD_VIRTUAL_CHANNEL_NAME);

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
        const CHANNELS::ChannelDef * channel      = channel_list.get_by_name(channel_name);
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
        const CHANNELS::ChannelDef * channel  = chanlist.get_by_name(CLIPBOARD_VIRTUAL_CHANNEL_NAME);

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

        if (this->state == UP_AND_RUNNING) {
            return;
        }

        CHANNELS::ChannelDefArray    chanlist    = this->front.get_channel_list();
        const CHANNELS::ChannelDef * mod_channel = chanlist.get_by_name(front_channel_name);

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
        TODO("Avoid useless buffer copy, parse data (we shoudl probably pass a (sub)stream instead)");
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

                    TODO("Create a unit tested class for clipboard messages");

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
                    TODO("RZ: Don't reject clipboard update, this can block rdesktop.");
                    TODO("RZ: Create a unit tested class for clipboard messages");

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
                    LOG( LOG_ERR
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
                            LOG( LOG_ERR
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
                            LOG(LOG_ERR, "mod_vnc::send_to_vnc flag CHANNEL_FLAG_FIRST expected");
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
                        LOG(LOG_ERR, "mod_vnc::send_to_vnc flag CHANNEL_FLAG_FIRST unexpected");
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

    // Front calls this member function when it became up and running.
    virtual void on_front_up_and_running()
    {
        if (this->state == WAIT_CLIENT_UP_AND_RUNNING) {
            if (this->verbose) {
                LOG(LOG_INFO, "Front up and running");
            }
            this->state = DO_INITIAL_CLEAR_SCREEN;
            this->event.set();
        }
    }

    virtual void notify(Widget2* sender, notify_event_t event)
    {
        switch (event) {
        case NOTIFY_SUBMIT:
            this->ini.context_set_value(AUTHID_PASSWORD, this->challenge.password_edit.get_text());

            this->screen.clear();

            memset(this->password, 0, 256);

            memcpy(this->password, this->challenge.password_edit.get_text(), sizeof(this->password) - 1);
            this->password[sizeof(this->password) - 1] = 0;

            this->state = RETRY_CONNECTION;
            this->event.set();
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();

            this->screen.clear();
            break;
        default:
            break;
        }
    }
};

#endif
