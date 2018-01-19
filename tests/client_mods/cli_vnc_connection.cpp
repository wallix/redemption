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

#include "core/report_message_api.hpp"
#include "front/client_front.hpp"
#include "utils/netutils.hpp"
#include "transport/socket_transport.hpp"
#include "core/client_info.hpp"
#include "mod/vnc/vnc.hpp"
#include "test_only/front/fake_front.hpp"
#include "utils/stream.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

int main()
{
    set_exception_handler_pretty_message();

  ClientInfo info;
  info.keylayout = 0x04C;
  info.console_session = 0;
  info.brush_cache_code = 0;
  info.bpp = 24;
  info.width = 800;
  info.height = 600;
  info.build = 420;
  int verbose = 511;
  int port = 5900;
  NullReportMessage report_message;
  int front_verbose = 255;
  FakeFront front(info, front_verbose);

//  VncFront front(false, false, info, verbose);

  const char * name = "VNC Target";
  const char * targetIP("10.10.47.0");
  int nbretry = 3;
  int retry_delai_ms = 1000;

  unique_fd sck = ip_connect(targetIP, port, nbretry, retry_delai_ms);

  std::string error_message;
  SocketTransport t(name, std::move(sck), targetIP, 5900, std::chrono::milliseconds(3000), to_verbose_flags(verbose), &error_message);
  t.connect();

  const bool is_socket_transport = true;

  if (verbose > 2){
      LOG(LOG_INFO, "========= CREATION OF MOD VNC =========================");
  }

  Font font;

  const VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop {};

  mod_vnc mod(
        t
      , targetIP
      , "SecureLinux"
      , front
      , info.width
      , info.height
      , font
      , "message"
      , "password"
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
      , report_message // nullptr       // acl
      , false // server is not apple
      , nullptr
      , to_verbose_flags(verbose));
    
    mod.get_event().set_trigger_time(wait_obj::NOW);

    using Ms = std::chrono::milliseconds;
    const char * screen_output = "./screen.png";
    return run_test_client(
        "VNC", t.sck, mod, front,
        Ms(3000), Ms(3000), screen_output);

}
