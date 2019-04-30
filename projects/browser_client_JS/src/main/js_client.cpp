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

#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "utils/genrandom.hpp"
#include "utils/redirection_info.hpp"
#include "utils/theme.hpp"

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
    struct JsReportMessage : NullReportMessage
    {
        void report(const char * reason, const char * message) override
        {
            RED_EM_ASM({
                console.log("RdpClient: " + UTF8ToString($0, $1) + ": " + UTF8ToString($2, $3));
            }, reason, strlen(reason), message, strlen(message));
        }
    };

    struct JsRandom : Random
    {
        JsRandom(emscripten::val callbacks) noexcept
        : callbacks(std::move(callbacks))
        {}

        void random(void* dest, std::size_t size) override
        {
            redjs::emval_call(this->callbacks, "random", dest, size);
        }

        emscripten::val callbacks;
    };

    struct JsAuth : NullAuthentifier
    {
        void set_auth_error_message(const char * error_message) override
        {
            RED_EM_ASM({
                console.log("RdpClient: " + UTF8ToString($0, $1));
            }, error_message, strlen(error_message));
        }
    };

    std::unique_ptr<mod_api> mod;

    redjs::BrowserTransport browser_trans;

    ClientInfo client_info;

    redjs::BrowserFront front;
    gdi::GraphicApi& gd;
    JsReportMessage report_message;
    SessionReactor session_reactor;

    Inifile ini;

    JsRandom js_rand;
    LCGTime lcg_timeobj;
    JsAuth authentifier;
    RedirectionInfo redir_info;

    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;

    RdpClient(
        emscripten::val callbacks, uint16_t width, uint16_t height,
        std::string const& username, std::string const& password,
        unsigned long verbose)
    : front(callbacks, width, height, client_info.order_caps, RDPVerbose(verbose))
    , gd(front.graphic_api())
    , js_rand(callbacks)
    {
        client_info.screen_info.width = width;
        client_info.screen_info.height = height;
        client_info.screen_info.bpp = BitsPerPixel{24};

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
            RDPVerbose(verbose)
        );

        mod_rdp_params.device_id                  = "device_id";
        mod_rdp_params.enable_tls                 = false;
        mod_rdp_params.enable_nla                 = false;
        mod_rdp_params.enable_fastpath            = true;
        mod_rdp_params.enable_new_pointer         = true;
        mod_rdp_params.enable_glyph_cache         = true;
        mod_rdp_params.server_cert_check          = ServerCertCheck::always_succeed;
        mod_rdp_params.primary_drawing_orders_support -= TS_NEG_GLYPH_INDEX;
        mod_rdp_params.primary_drawing_orders_support -= TS_NEG_DSTBLT_INDEX;
        mod_rdp_params.primary_drawing_orders_support -= TS_NEG_PATBLT_INDEX;
        mod_rdp_params.primary_drawing_orders_support += TS_NEG_POLYLINE_INDEX;
        mod_rdp_params.primary_drawing_orders_support += TS_NEG_MULTISCRBLT_INDEX;
        mod_rdp_params.primary_drawing_orders_support += TS_NEG_MULTIOPAQUERECT_INDEX;
        mod_rdp_params.ignore_auth_channel = true;

        if (bool(RDPVerbose(verbose) & RDPVerbose::basic_trace)) {
            mod_rdp_params.log();
        }

        const ChannelsAuthorizations channels_authorizations("*", std::string{});

        this->mod = new_mod_rdp(
            browser_trans, session_reactor, gd, front, client_info,
            redir_info, js_rand, lcg_timeobj, channels_authorizations,
            mod_rdp_params, authentifier, report_message, ini, nullptr);
    }

    /// \return milliseconds before next timer, or 0 if no timer
    Ms update_time(timeval current_time)
    {
        session_reactor.set_current_time(current_time);

        session_reactor.execute_timers(
            SessionReactor::EnableGraphics{true},
            [&]() -> gdi::GraphicApi& { return gd; });

        std::chrono::microseconds us =
            session_reactor.get_next_timeout(SessionReactor::EnableGraphics{true}, 1h)
          - session_reactor.get_current_time();

        if (us <= 1ms) {
            return 1ms;
        }
        return std::chrono::duration_cast<Ms>(us);
    }

    const_bytes_view get_sending_data_view() const
    {
        return browser_trans.get_out_buffer();
    }

    void clear_sending_data()
    {
        browser_trans.clear_out_buffer();
    }

    void add_receiving_data(std::string data)
    {
        browser_trans.add_in_buffer(std::move(data));
        // browser_trans.out_buffers.insert(browser_trans.out_buffers.end(), data.begin(), data.end());
        session_reactor.execute_callbacks(*mod);
        auto fd_isset = [fd_trans = browser_trans.get_fd()](int fd, auto& /*e*/){
            return fd == fd_trans;
        };
        session_reactor.execute_graphics(fd_isset, gd);
    }

    void rdp_input_scancode(uint16_t key, uint16_t flag)
    {
        this->mod->rdp_input_scancode(
            key, 0, flag,
            session_reactor.get_current_time().tv_sec,
            nullptr);
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag)
    {
        this->mod->rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y)
    {
        this->mod->rdp_input_mouse(device_flags, x, y, nullptr);
    }

    void add_channel_receiver(redjs::ChannelReceiver&& receiver)
    {
        this->front.add_channel_receiver(std::move(receiver));
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
        .constructor<emscripten::val, uint16_t, uint16_t, std::string, std::string, unsigned long>()
        .function_ptr("updateTime", [](RdpClient& client) {
            Ms ms = client.update_time(tvtime());
            // long long is not embind type. Use long or double (safe for 53 bits);
            return static_cast<unsigned long>(ms.count());
        })
        .function_ptr("getSendingData", [](RdpClient& client) {
            return redjs::emval_from_view(client.get_sending_data_view());
        })
        .function_ptr("getCallbackAsVoidPtr", [](RdpClient& client) {
            return reinterpret_cast<uintptr_t>(&client.get_callback());
        })
        .function("addChannelReceiver", &RdpClient::add_channel_receiver)
        .function("clearSendingData", &RdpClient::clear_sending_data)
        .function("addReceivingData", &RdpClient::add_receiving_data)
        .function("sendUnicode", &RdpClient::rdp_input_unicode)
        .function("sendScancode", &RdpClient::rdp_input_scancode)
        .function("sendMouseEvent", &RdpClient::rdp_input_mouse)
    ;
}
