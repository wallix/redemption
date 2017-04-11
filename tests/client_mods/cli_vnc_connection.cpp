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

#define RED_TEST_MODULE TestVncConnection
#include "system/redemption_unit_tests.hpp"

//#define LOGNULL
#define LOGPRINT

// #include "transport/socket_transport.hpp"
// #include "core/client_info.hpp"
#include "mod/vnc/vnc.hpp"
// #include "core/vnc_front.hpp"
//jni #include "utils/stream.hpp

// #include <cstdio>
// #include <cstdlib>
// #include <cerrno>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include "openssl_tls.hpp"
// #include "system/openssl.hpp"


RED_AUTO_TEST_CASE(TestVncConnection0)
{
}


//RED_AUTO_TEST_CASE(TestVncConnection)
//{
//  RED_CHECK(1);

//  ClientInfo info;
//  info.keylayout = 0x04C;
//  info.console_session = 0;
//  info.brush_cache_code = 0;
//  info.bpp = 24;
//  info.width = 800;
//  info.height = 600;
//  info.build = 420;
//  int verbose = 511;
//  int port = 5900;
//  NullAuthentifier authentifier;
//  VncFront front(false, false, info, verbose);

////   BStream stream(65536);
//  const char * name = "VNC Target";
//  const char * targetIP("10.10.46.70");
//  int nbretry = 3;
//  int retry_delai_ms = 1000;

//  int sck = ip_connect(targetIP, port, nbretry, retry_delai_ms);
//
//  SocketTransport t(name, sck, targetIP, 5900, to_verbose_flags(verbose));
//  t.connect();

//  const bool is_socket_transport = true;

//  if (verbose > 2){
//      LOG(LOG_INFO, "========= CREATION OF MOD VNC =========================");
//  }

//  Font font;

//  const VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop {};

//  mod_vnc mod(
//        t
//      , targetIP
//      , "SecureLinux$42"
//      , front
//      , info.width
//      , info.height
//      , font
//      , Translator(Translation::EN)
//      , Theme()
//      , info.keylayout
//      , 0             /* key_flags */
//      , true          /* clipboard */
//      , true          /* clipboard */
//      , "0,1,-239"    /* encodings: Raw,CopyRect,Cursor pseudo-encoding */
//      , false         /* allow authentification retries */
//      , is_socket_transport
//      , mod_vnc::ClipboardEncodingType::UTF8
//      , bogus_clipboard_infinite_loop
//      , authentifier // nullptr       // acl
//      , verbose);
//  mod.get_event().set();

//  if (verbose > 2){
//    LOG(LOG_INFO, "========= CREATION OF MOD VNC DONE ====================\n\n");
//  }
// // RED_CHECK(t.status);


//  mod.draw_event(time(nullptr), front);
//  RED_CHECK(1);
//  mod.rdp_input_up_and_running();
//  mod.draw_event(time(nullptr), front);
//  RED_CHECK(1);
//  RED_CHECK_EQUAL(front.info.width, 800);
//  RED_CHECK_EQUAL(front.info.height, 600);
///*
//    while (!mod.rdp_input_up_and_running())
//            mod.draw_event(time(nullptr), front);
//*/

//    uint32_t    count = 0;
//    for (;;) {
//        LOG(LOG_INFO, "===================> count = %u", count);
//      //  if (count++ >= 8) break;
//        mod.draw_event(time(nullptr), front);
//    }



///*
// mod.draw_event(time(nullptr), front);
// // RED_CHECK(t.status);

// mod.draw_event(time(nullptr), front);
// // RED_CHECK(t.status);

// mod.draw_event(time(nullptr), front);
// // RED_CHECK(t.status);
//
//*/
//}
