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

   Unit test to writing RDP orders to file and rereading them

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVncClientSimple
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "test_orders.hpp"
#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#include "stream.hpp"
#include "transport.hpp"
#include "testtransport.hpp"
#include "wait_obj.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "channel_list.hpp"
#include "front_api.hpp"
#include "client_info.hpp"
#include "vnc/vnc.hpp"
#include "RDP/pointer.hpp"

BOOST_AUTO_TEST_CASE(TestDecodePacket)
{
    BOOST_CHECK(1);

    ClientInfo info(1, true, true);
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    int verbose = 0;

    class Front : public FrontAPI {
        public:
        uint8_t mod_bpp;
        uint32_t verbose;
        const ClientInfo & info;
        CHANNELS::ChannelDefArray cl;

        virtual void flush()
        {
            if (verbose > 10){
                 LOG(LOG_INFO, "--------- FRONT ------------------------");
                 LOG(LOG_INFO, "flush()");
                 LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
            if (verbose > 10) {
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
            if (verbose > 10) {
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
            if (verbose > 10) {
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPMem3Blt& cmd, const Rect& clip, const Bitmap& bmp)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPLineTo& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip, const GlyphCache * gly_cache)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPPolygonSC & cmd, const Rect & clip) {
            if (verbose > 10) {
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPPolygonCB & cmd, const Rect & clip) {
            if (verbose > 10) {
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPPolyline & cmd, const Rect & clip) {
            if (verbose > 10) {
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPEllipseSC& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPEllipseCB& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
        }

        virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const CHANNELS::ChannelDef & channel, uint8_t* data, size_t length, size_t chunk_size, int flags)
        {
        }

        virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "send_pointer(cache_idx=%d, data=%p, mask=%p, x=%d, y=%d",
                    cache_idx, data, mask, x, y);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void send_global_palette() throw (Error)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "send_global_palette()");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void set_pointer(int cache_idx) throw (Error)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "set_pointer");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw_vnc(const Rect & rect, const uint8_t bpp, const BGRPalette & palette332, const uint8_t * raw, uint32_t need_size)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "draw_vnc(rect(%u, %u, %u, %u), bpp=%u, need_size=%u",
                    rect.x, rect.y, rect.cx, rect.cy, bpp, need_size);
                LOG(LOG_INFO, "========================================\n");
            }
        }

        virtual void begin_update()
        {
//            if (verbose > 10){
//                LOG(LOG_INFO, "--------- FRONT ------------------------");
//                LOG(LOG_INFO, "begin_update");
//                LOG(LOG_INFO, "========================================\n");
//            }
        }
        virtual void end_update()
        {
//            if (verbose > 10){
//                LOG(LOG_INFO, "--------- FRONT ------------------------");
//                LOG(LOG_INFO, "end_update");
//                LOG(LOG_INFO, "========================================\n");
//            }
        }
        virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "color_cache");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void set_mod_palette(const BGRPalette & palette)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "set_mod_palette");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void server_set_pointer(const Pointer & cursor)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_set_pointer");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_draw_text %s", text);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void text_metrics(const char * text, int & width, int & height)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "text_metrics");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual int server_resize(int width, int height, int bpp)
        {
            this->mod_bpp = bpp;
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
                LOG(LOG_INFO, "========================================\n");
            }
            // resize done
            return 1;
        }

        int mouse_x;
        int mouse_y;
        bool notimestamp;
        bool nomouse;

        Front(const ClientInfo & info, uint32_t verbose) :
              FrontAPI(false, false),
              verbose(verbose),
              info(info),
              mouse_x(0),
              mouse_y(0),
              notimestamp(true),
              nomouse(true)
            {}

    } front(info, verbose);

//    BStream stream(65536);
//    const char * name = "VNC Target";
//    ClientSocketTransport t(name, "10.10.3.103", 5900, 3, 1000, verbose);
//    t.connect();


//    wait_obj back_event(t.sck);


    const char outdata[] =
    {
// connecting to VNC Target (10.10.3.103:5900)

// connection to 10.10.3.103 succeeded : socket 3

// --------- CREATION OF MOD VNC ------------------------
// Connecting to VNC Server
// Socket VNC Target (3) receiving 12 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x52\x46\x42\x20\x30\x30\x33\x2e\x30\x30\x38\x0a"                 // RFB 003.008.
// Dump done on VNC Target (3)
// Socket VNC Target (3) sending 12 bytes
 /* 0000 */ "\x52\x46\x42\x20\x30\x30\x33\x2e\x30\x30\x33\x0a"                 // RFB 003.003.
// Dump done VNC Target (3) sending 12 bytes
// Send done on VNC Target (3)
// Socket VNC Target (3) receiving 4 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x00\x00\x00\x02"                                                 // ....
// Dump done on VNC Target (3)
// security level is 2 (1 = none, 2 = standard)

// Receiving VNC Server Random
// Socket VNC Target (3) receiving 16 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x18\xe7\xf7\xcc\x94\x50\x1d\x78\xa6\x36\x25\xf0\x4a\x7b\x00\x2c" // .....P.x.6%.J{.,
// Dump done on VNC Target (3)
// Sending Password
// Socket VNC Target (3) sending 16 bytes
 /* 0000 */ "\x1a\x8a\x6d\x99\xd1\xdb\x0f\x11\x6e\xf2\xbc\x29\xe1\x42\x61\x98" // ..m.....n..).Ba.
// Dump done VNC Target (3) sending 16 bytes
// Send done on VNC Target (3)
// Waiting for password ack
// Socket VNC Target (3) receiving 4 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x00\x00\x00\x00"                                                 // ....
// Dump done on VNC Target (3)
// vnc password ok

// Socket VNC Target (3) sending 1 bytes
 /* 0000 */ "\x01"                                                             // .
// Dump done VNC Target (3) sending 1 bytes
// Send done on VNC Target (3)
// Socket VNC Target (3) receiving 24 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x04\x00\x03\x00\x20\x18\x00\x01\x00\xff\x00\xff\x00\xff\x10\x08" // .... ...........
// /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x0d"                                 // ........
// Dump done on VNC Target (3)
// VNC received: width=1024 height=768 bpp=32 depth=24 endianess=0 true_color=1 red_max=255 green_max=255 blue_max=255 red_shift=16 green_shift=8 blue_shift=0
// Socket VNC Target (3) receiving 13 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x41\x55\x54\x48\x2d\x32\x4b\x33\x2d\x4c\x44\x41\x50"             // AUTH-2K3-LDAP
// Dump done on VNC Target (3)
// Socket VNC Target (3) sending 20 bytes
 /* 0000 */ "\x00\x00\x00\x00\x10\x10\x00\x01\x00\x1f\x00\x3f\x00\x1f\x0b\x05" // ...........?....
 /* 0010 */ "\x00\x00\x00\x00"                                                 // ....
// Dump done VNC Target (3) sending 20 bytes
// Send done on VNC Target (3)
// Socket VNC Target (3) sending 16 bytes
 /* 0000 */ "\x02\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x01\xff\xff\xff\x11" // ................
// Dump done VNC Target (3) sending 16 bytes
// Send done on VNC Target (3)
// --------- FRONT ------------------------
// server_resize(width=1024, height=768, bpp=16
// ========================================

// --------- FRONT ------------------------
// set_mod_bpp(bpp=16)
// ========================================

// Socket VNC Target (3) sending 10 bytes
 /* 0000 */ "\x03\x00\x00\x00\x00\x00\x04\x00\x03\x00"                         // ..........
// Dump done VNC Target (3) sending 10 bytes
// Send done on VNC Target (3)
// --------- FRONT ------------------------
// server_set_pointer
// ========================================

// VNC connection complete, connected ok

// --------- FRONT ------------------------
// order(10 clip(0,0,1024,768)):opaquerect(rect(0,0,1024,768) color=0x000000)
// ========================================

// ========= CREATION OF MOD VNC DONE ====================


// Socket VNC Target (3) : closing connection
    };


    const char indata[] = {
// connecting to VNC Target (10.10.3.103:5900)

// connection to 10.10.3.103 succeeded : socket 3

// --------- CREATION OF MOD VNC ------------------------
// Connecting to VNC Server
// Socket VNC Target (3) receiving 12 bytes
// Recv done on VNC Target (3)
 /* 0000 */ "\x52\x46\x42\x20\x30\x30\x33\x2e\x30\x30\x38\x0a"                 // RFB 003.008.
// Dump done on VNC Target (3)
// Socket VNC Target (3) sending 12 bytes
// /* 0000 */ "\x52\x46\x42\x20\x30\x30\x33\x2e\x30\x30\x33\x0a"                 // RFB 003.003.
// Dump done VNC Target (3) sending 12 bytes
// Send done on VNC Target (3)
// Socket VNC Target (3) receiving 4 bytes
// Recv done on VNC Target (3)
 /* 0000 */ "\x00\x00\x00\x02"                                                 // ....
// Dump done on VNC Target (3)
// security level is 2 (1 = none, 2 = standard)

// Receiving VNC Server Random
// Socket VNC Target (3) receiving 16 bytes
// Recv done on VNC Target (3)
 /* 0000 */ "\x18\xe7\xf7\xcc\x94\x50\x1d\x78\xa6\x36\x25\xf0\x4a\x7b\x00\x2c" // .....P.x.6%.J{.,
// Dump done on VNC Target (3)
// Sending Password
// Socket VNC Target (3) sending 16 bytes
// /* 0000 */ "\x1a\x8a\x6d\x99\xd1\xdb\x0f\x11\x6e\xf2\xbc\x29\xe1\x42\x61\x98" // ..m.....n..).Ba.
// Dump done VNC Target (3) sending 16 bytes
// Send done on VNC Target (3)
// Waiting for password ack
// Socket VNC Target (3) receiving 4 bytes
// Recv done on VNC Target (3)
 /* 0000 */ "\x00\x00\x00\x00"                                                 // ....
// Dump done on VNC Target (3)
// vnc password ok

// Socket VNC Target (3) sending 1 bytes
// /* 0000 */ "\x01"                                                             // .
// Dump done VNC Target (3) sending 1 bytes
// Send done on VNC Target (3)
// Socket VNC Target (3) receiving 24 bytes
// Recv done on VNC Target (3)
 /* 0000 */ "\x04\x00\x03\x00\x20\x18\x00\x01\x00\xff\x00\xff\x00\xff\x10\x08" // .... ...........
 /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x0d"                                 // ........
// Dump done on VNC Target (3)
// VNC received: width=1024 height=768 bpp=32 depth=24 endianess=0 true_color=1 red_max=255 green_max=255 blue_max=255 red_shift=16 green_shift=8 blue_shift=0
// Socket VNC Target (3) receiving 13 bytes
// Recv done on VNC Target (3)
// /* 0000 */ "\x41\x55\x54\x48\x2d\x32\x4b\x33\x2d\x4c\x44\x41\x50"             // AUTH-2K3-LDAP
// Dump done on VNC Target (3)
// Socket VNC Target (3) sending 20 bytes
 /* 0000 */ "\x00\x00\x00\x00\x10\x10\x00\x01\x00\x1f\x00\x3f\x00\x1f\x0b\x05" // ...........?....
 /* 0010 */ "\x00\x00\x00\x00"                                                 // ....
// Dump done VNC Target (3) sending 20 bytes
// Send done on VNC Target (3)
// Socket VNC Target (3) sending 16 bytes
 /* 0000 */ "\x02\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x01\xff\xff\xff\x11" // ................
// Dump done VNC Target (3) sending 16 bytes
// Send done on VNC Target (3)
// --------- FRONT ------------------------
// server_resize(width=1024, height=768, bpp=16
// ========================================

// --------- FRONT ------------------------
// set_mod_bpp(bpp=16)
// ========================================

// Socket VNC Target (3) sending 10 bytes
// /* 0000 */ "\x03\x00\x00\x00\x00\x00\x04\x00\x03\x00"                         // ..........
// Dump done VNC Target (3) sending 10 bytes
// Send done on VNC Target (3)
// --------- FRONT ------------------------
// server_set_pointer
// ========================================

// VNC connection complete, connected ok

// --------- FRONT ------------------------
// order(10 clip(0,0,1024,768)):opaquerect(rect(0,0,1024,768) color=0x000000)
// ========================================

// ========= CREATION OF MOD VNC DONE ====================

// Socket VNC Target (3) : closing connection
    };

    TestTransport t("test_vnc_client_simple", indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    // To always get the same client random, in tests
//    LCGRandom gen(0);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD VNC ------------------------");
    }

    Inifile ini;

    struct mod_api * mod = new mod_vnc(
          &t
        , ini
        , "10.10.3.103"
        , "SecureLinux"
        , front
        , info.width
        , info.height
        , info.keylayout
        , 0             /* key_flags */
        , true          /* clipboard */
        , "0,1,-239"    /* encodings: Raw,CopyRect,Cursor pseudo-encoding */
        , false         /* allow authentification retries */
        , verbose);
    mod->event.set();

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD VNC DONE ====================\n\n");
    }
//    BOOST_CHECK(t.status);

    mod->draw_event(time(NULL));
    mod->on_front_up_and_running();
    mod->draw_event(time(NULL));

    BOOST_CHECK_EQUAL(mod->front_width, 1024);
    BOOST_CHECK_EQUAL(mod->front_height, 768);

//    mod->draw_event(time(NULL));
////    BOOST_CHECK(t.status);

//    mod->draw_event(time(NULL));
////    BOOST_CHECK(t.status);

//    mod->draw_event(time(NULL));
////    BOOST_CHECK(t.status);

}
