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
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#pragma once

#include "core/client_info.hpp"
#include "core/report_message_api.hpp"
#include "keyboard/keymap2.hpp"
#include "utils/timebase.hpp"
#include "acl/mod_wrapper.hpp"
#include "acl/mod_pack.hpp"

class AuthApi;
class Random;
class CryptoContext;

extern ModPack create_mod_rdp(ModWrapper & mod_wrapper,
    ReportMessageApi& report_message,
    Inifile& ini, gdi::GraphicApi & drawable, FrontAPI& front, ClientInfo client_info /* /!\ modified */,
    ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags,
    Font & glyphs,
    Theme & theme,
    TimeBase & time_base,
    EventContainer& events,
    AuthApi & sesman,
    LicenseApi & file_system_license_store,
    Random & gen,
    TimeObj & timeobj,
    CryptoContext & cctx,
    std::array<uint8_t, 28>& server_auto_reconnect_packet
);
