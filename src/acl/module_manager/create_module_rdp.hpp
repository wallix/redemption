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

#include "keyboard/kbdtypes.hpp"
#include "acl/mod_pack.hpp"
#include "utils/ref.hpp"

#include <array>
#include <type_traits>

class RedirectionInfo;
class Inifile;
class FrontAPI;
class ClientExecute;
class Font;
class Theme;
class EventContainer;
class LicenseApi;
class Random;
class CryptoContext;
class SessionLogApi;
class ClientInfo;
class Transport;

namespace gdi
{
    class GraphicApi;
    class OsdApi;
}


enum class PerformAutomaticReconnection : bool { No, Yes, };


struct TransportWrapperFnView
{
    TransportWrapperFnView() noexcept
    {}

    TransportWrapperFnView(TransportWrapperFnView&&) noexcept = default;
    TransportWrapperFnView(TransportWrapperFnView const&) noexcept = default;

    template<class F>
    TransportWrapperFnView(F&& f) noexcept
    {
        using DF = std::decay_t<F>;

        if constexpr (std::is_pointer_v<DF>) {
            if (!f) {
                return;
            }
        }

        data = &f;
        fn = [](void* d, Transport& trans) -> Transport& {
            return static_cast<F&&>(*static_cast<DF*>(d))(trans);
        };
    }

    Transport& operator()(Transport& trans)
    {
        return !data ? trans : fn(data, trans);
    }

private:
    Transport&(*fn)(void*, Transport&) = nullptr;
    void* data;
};


ModPack create_mod_rdp(
    gdi::GraphicApi & drawable,
    gdi::OsdApi & osd,
    RedirectionInfo & redir_info,
    Inifile & ini,
    FrontAPI & front,
    ClientInfo const& client_info_,
    ClientExecute & rail_client_execute,
    kbdtypes::KeyLocks key_locks,
    Ref<Font const> glyphs,
    Theme & theme,
    EventContainer& events,
    SessionLogApi& session_log,
    LicenseApi & file_system_license_store,
    Random & gen,
    CryptoContext & cctx,
    std::array<uint8_t, 28>& server_auto_reconnect_packet,
    PerformAutomaticReconnection perform_automatic_reconnection,
    TransportWrapperFnView transport_wrapper_fn = {} /* NOLINT */
);
