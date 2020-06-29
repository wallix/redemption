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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#ifdef IN_IDE_PARSER
# define __EMSCRIPTEN__
#endif

#include "core/session_reactor.hpp"
#include "acl/auth_api.hpp"
#include "acl/gd_provider.hpp"
#include "acl/license_api.hpp"
#include "acl/sesman.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/report_message_api.hpp"
#include "core/session_reactor.hpp"
#include "core/channels_authorizations.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "utils/genrandom.hpp"
#include "utils/redirection_info.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "utils/theme.hpp"
#include "utils/timebase.hpp"

#include "red_emscripten/bind.hpp"
#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/val.hpp"

#include "redjs/image_data.hpp"
#include "redjs/browser_transport.hpp"
#include "redjs/browser_front.hpp"
#include "redjs/channel_receiver.hpp"

#include <chrono>

using Ms = std::chrono::milliseconds;

struct RdpClient
{
    struct JsReportMessage : ReportMessageApi
    {
        void report(const char * reason, const char * message) override
        {
            LOG(LOG_NOTICE, "RdpClient: %s: %s", reason, message);
        }

        void log6(LogId /*id*/, KVList /*kv_list*/) override
        {}
    };

    struct JsRandom : Random
    {
        static constexpr char const* get_random_values = "getRandomValues";

        JsRandom(emscripten::val const& random)
        : crypto(not random[get_random_values]
            ? emscripten::val::global("crypto")
            : random)
        {}

        void random(void* dest, std::size_t size) override
        {
            redjs::emval_call(this->crypto, get_random_values,
                array_view{static_cast<uint8_t*>(dest), size});
        }

        emscripten::val crypto;
    };

    struct JsAuth : NullAuthentifier
    {
        void set_auth_error_message(const char * error_message) override
        {
            LOG(LOG_ERR, "RdpClient: %s", error_message);
        }
    };

    ModRdpFactory mod_rdp_factory;
    std::unique_ptr<mod_api> mod;

    redjs::BrowserTransport browser_trans;

    ClientInfo client_info;

    redjs::BrowserFront front;
    gdi::GraphicApi& gd;
    GdForwarder<gdi::GraphicApi> gd_forwarder{gd};

    JsReportMessage report_message;
    TimeBase time_base;
    TopFdContainer fd_events;
    TimerContainer timer_events;
    EventContainer events;

    Inifile ini;
    SesmanInterface sesman;

    JsRandom js_rand;
    LCGTime lcg_timeobj;
    JsAuth authentifier;
    NullLicenseStore license_store;
    RedirectionInfo redir_info;

    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;

    RdpClient(
        emscripten::val const& random, zstring_view username, zstring_view password,
        emscripten::val graphics, ScreenInfo screen_info,
        PrimaryDrawingOrdersSupport disabled_orders, RDPVerbose verbose)
    : front(std::move(graphics), screen_info.width, screen_info.height, verbose)
    , gd(front.graphic_api())
    , time_base({0,0})
    , sesman(ini)
    , js_rand(random)
    {
        client_info.screen_info = screen_info;

        PrimaryDrawingOrdersSupport supported_orders
            = front.get_supported_orders() - disabled_orders;
        for (unsigned i = 0; i < NB_ORDER_SUPPORT; ++i) {
            client_info.order_caps.orderSupport[i] = supported_orders.test(OrdersIndexes(i));
        }


        ini.set<cfg::mod_rdp::server_redirection_support>(false);
        ini.set<cfg::mod_rdp::enable_nla>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);


        ModRDPParams mod_rdp_params(
            username.c_str(),
            password.c_str(),
            "0.0.0.0",
            "0.0.0.0", // client ip is silenced
            /*front.keymap.key_flags*/ 0,
            font,
            theme,
            server_auto_reconnect_packet,
            close_box_extra_message,
            verbose
        );

        mod_rdp_params.device_id           = "device_id";
        mod_rdp_params.enable_tls          = false;
        mod_rdp_params.enable_nla          = false;
        mod_rdp_params.enable_fastpath     = true;
        mod_rdp_params.enable_new_pointer  = true;
        mod_rdp_params.enable_glyph_cache  = true;
        mod_rdp_params.server_cert_check   = ServerCertCheck::always_succeed;
        mod_rdp_params.ignore_auth_channel = true;


        if (bool(verbose & RDPVerbose::basic_trace)) {
            mod_rdp_params.log();
        }

        const ChannelsAuthorizations channels_authorizations("*", std::string_view{});

        this->mod = new_mod_rdp(
            browser_trans, ini, time_base, gd_forwarder,
            fd_events, timer_events, events, sesman, gd, front, client_info,
            redir_info, js_rand, lcg_timeobj, channels_authorizations,
            mod_rdp_params, TLSClientParams{}, authentifier, report_message,
            license_store, ini, nullptr, nullptr, this->mod_rdp_factory);
    }

    void write_first_packet()
    {
        this->fd_events.exec_timeout(this->time_base.get_current_time());
    }

    void set_time(timeval time)
    {
        this->time_base.set_current_time(time);
        this->fd_events.exec_timeout(this->time_base.get_current_time());
    }

    bytes_view get_output_buffer() const
    {
        return this->browser_trans.get_output_buffer();
    }

    void reset_output_data()
    {
        this->browser_trans.clear_output_buffer();
    }

    void process_input_data(std::string data)
    {
        this->browser_trans.push_input_buffer(std::move(data));
        this->fd_events.exec_action([](int /*fd*/, auto& /*top*/){ return true; });
    }

    void write_scancode_event(uint16_t scancode)
    {
        uint16_t key = scancode & 0xFF;
        uint16_t flag = scancode & 0xFF00;
        this->mod->rdp_input_scancode(
            key, 0, flag,
            this->time_base.get_current_time().tv_sec,
            nullptr);
    }

    void write_unicode_event(uint16_t unicode, uint16_t flag)
    {
        this->mod->rdp_input_unicode(unicode, flag);
    }

    void write_mouse_event(int x, int y, int device_flags)
    {
        this->mod->rdp_input_mouse(device_flags, x, y, nullptr);
    }

    void add_channel_receiver(redjs::ChannelReceiver receiver)
    {
        this->front.add_channel_receiver(receiver);
    }

    Callback& get_callback()
    {
        return *this->mod;
    }
};

// Binding code
EMSCRIPTEN_BINDINGS(client)
{
    redjs::class_<RdpClient>("RdpClient")
        .constructor([](emscripten::val&& graphics, emscripten::val&& config) {
            auto get_string_or_empty = [](emscripten::val const& v, char const* name){
                auto prop = v[name];
                return not prop ? std::string() : prop.as<std::string>();
            };

            auto get_number_or = [](
                emscripten::val const& v, char const* name, auto default_value
            ){
                auto prop = v[name];
                return not prop ? default_value : prop.as<decltype(default_value)>();
            };

            ScreenInfo screen_info{
                get_number_or(config, "width", uint16_t(800)),
                get_number_or(config, "height", uint16_t(600)),
                BitsPerPixel(get_number_or(config, "bpp", uint16_t(16)))
            };

            auto disabled_orders = get_number_or(config, "disabledDrawingOrders", uint32_t(0));

            static_assert(sizeof(RDPVerbose) == 4, "verbose is truncated");
            auto verbose_flags = get_number_or(config, "verbose", uint32_t(0));

            auto username = get_string_or_empty(config, "username");
            auto password = get_string_or_empty(config, "password");

            return new RdpClient(
                std::move(config), username, password,
                std::move(graphics), screen_info,
                PrimaryDrawingOrdersSupport(disabled_orders),
                RDPVerbose(verbose_flags)
            );
        })
        .function_ptr("setTime", [](RdpClient& client, uint32_t seconds, uint32_t milliseconds) {
            client.set_time({checked_int(seconds), checked_int(milliseconds*1000u)});
        })
        .function_ptr("getOutputData", [](RdpClient& client) {
            return redjs::emval_from_view(client.get_output_buffer());
        })
        .function_ptr("getCallbackAsVoidPtr", [](RdpClient& client) {
            return redjs::to_memory_offset(client.get_callback());
        })
        .function_ptr("addChannelReceiver", [](RdpClient& client, uintptr_t ichannel_receiver) {
            client.add_channel_receiver(
                redjs::from_memory_offset<redjs::ChannelReceiver const&>(ichannel_receiver));
        })
        .function("writeFirstPacket", &RdpClient::write_first_packet)
        .function("resetOutputData", &RdpClient::reset_output_data)
        .function("processInputData", &RdpClient::process_input_data)
        .function("writeUnicodeEvent", &RdpClient::write_unicode_event)
        .function("writeScancodeEvent", &RdpClient::write_scancode_event)
        .function("writeMouseEvent", &RdpClient::write_mouse_event)
    ;
}
