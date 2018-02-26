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

#define RED_TEST_MODULE TestVncClientSimple
#include "system/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/client_info.hpp"
#include "mod/vnc/vnc.hpp"
#include "test_only/front/fake_front.hpp"


RED_AUTO_TEST_CASE(TestDecodePacket)
{
    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    auto vnc_verbose = VNCVerbose::none;
    auto front_verbose = 0;

    FakeFront front(info, front_verbose);

//    BStream stream(65536);
//    const char * name = "VNC Target";
//    ClientSocketTransport t(name, "10.10.3.103", 5900, 3, 1000, verbose);
//    t.connect();
//    const bool is_socket_transport = true;

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
 /* 0000 */ "\x03\x01\x00\x00\x00\x00\x04\x00\x03\x00"                         // ..........
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

    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    const bool is_socket_transport = false;

    // To always get the same client random, in tests
//    LCGRandom gen(0);

    if (front_verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD VNC ------------------------");
    }

    Font font;

    const VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop {};

    NullReportMessage report_message;

    SessionReactor session_reactor;
    mod_vnc mod(
          t
        , session_reactor
        , "10.10.3.103"
        , "SecureLinux"
        , front
        , info.width
        , info.height
        , font
        , "label message", "label pass"
        , Theme()
        , info.keylayout
        , 0             /* key_flags */
        , true          /* clipboard */
        , true          /* clipboard */
        , "0,1,-239"    /* encodings: Raw,CopyRect,Cursor pseudo-encoding */
        , false         /* allow authentification retries */
        , is_socket_transport
        , mod_vnc::ClipboardEncodingType::UTF8
        , bogus_clipboard_infinite_loop
        , report_message
        , false
        , nullptr
        , vnc_verbose);
    mod.get_event().set_trigger_time(wait_obj::NOW);

    if (front_verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD VNC DONE ====================\n\n");
    }
//    RED_CHECK(t.status);

    mod.draw_event(time(nullptr), front);
    mod.rdp_input_up_and_running();
    mod.draw_event(time(nullptr), front);

    RED_CHECK_EQUAL(front.info.width, 800);
    RED_CHECK_EQUAL(front.info.height, 600);

    t.disable_remaining_error();

//    mod.draw_event(time(nullptr), front);
//    mod.draw_event(time(nullptr), front);
//    mod.draw_event(time(nullptr), front);
}
