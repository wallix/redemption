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
#include "redjs/transport.hpp"
#include "redjs/front.hpp"
#include "redjs/channel_receiver.hpp"

#include <chrono>

namespace
{
    template<class T>
    std::string get_or_default(emscripten::val const& v, char const* name)
    {
        auto prop = v[name];
        return not prop ? T() : prop.as<T>();
    };

    template<class T>
    T get_or(emscripten::val const& v, char const* name, T default_value)
    {
        auto prop = v[name];
        if constexpr (std::is_enum_v<T>) {
            static_assert(sizeof(T) <= 4);
            using U = std::underlying_type_t<T>;
            return not prop ? default_value : T(prop.as<U>());
        }
        else {
            return not prop ? default_value : prop.as<T>();
        }
    };

    std::string get_or(emscripten::val const& v, char const* name, char const* default_value)
    {
        auto prop = v[name];
        return not prop ? default_value : prop.as<std::string>();
    }

    template<class T>
    ::utils::flags_t<T> get_or(
        emscripten::val const& v, char const* name,
        ::utils::flags_t<T> default_value)
    {
        using return_type = ::utils::flags_t<T>;

        using Int = typename return_type::bitfield;
        static_assert(sizeof(Int) <= 4);

        auto prop = v[name];
        return not prop ? default_value : return_type(prop.as<Int>());
    }

    template<class Rep, class Period>
    std::chrono::duration<Rep, Period> get_or(
        emscripten::val const& v, char const* name,
        std::chrono::duration<Rep, Period> default_value)
    {
        using Duration = std::chrono::duration<Rep, Period>;
        using Int = std::conditional_t<(sizeof(Rep) > 4), uint32_t, std::make_unsigned_t<Rep>>;

        auto prop = v[name];
        return not prop ? default_value : Duration(prop.as<Int>());
    };


    ScreenInfo make_screen_info(emscripten::val const& config)
    {
        return ScreenInfo{
            get_or(config, "width", uint16_t(800)),
            get_or(config, "height", uint16_t(600)),
            get_or(config, "bpp", BitsPerPixel(16))
        };
    }

    RDPVerbose make_rdp_verbose(emscripten::val const& config)
    {
        static_assert(sizeof(RDPVerbose) == 4, "verbose is truncated");
        return get_or(config, "verbose", RDPVerbose(0));
        // | (get_or(config, "verbose2", uint32_t(0)) << 32);
    }
}

class RdpClient
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

    redjs::Transport trans;

    ClientInfo client_info;

    redjs::Front front;
    gdi::GraphicApi& gd;
    GdForwarder gd_forwarder{gd};

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

public:
    RdpClient(
        emscripten::val&& graphics,
        emscripten::val const& config)
    : RdpClient(std::move(graphics), config, make_screen_info(config), make_rdp_verbose(config))
    {}

    RdpClient(
        emscripten::val&& graphics,
        emscripten::val const& config,
        ScreenInfo screen_info,
        RDPVerbose verbose)
    : front(std::move(graphics), screen_info.width, screen_info.height, verbose)
    , gd(front.graphic_api())
    , time_base({0,0})
    , sesman(ini)
    , js_rand(config)
    {
        using namespace std::chrono_literals;

        client_info.screen_info = screen_info;

        const auto disabled_orders
            = get_or(config, "disabledGraphicOrders", PrimaryDrawingOrdersSupport());

        const bool enable_remotefx = get_or(config, "enableRemoteFx", false);
        const bool enable_large_pointer = get_or(config, "enableLargePointer", true);


        // init client_info
        //@{
        const PrimaryDrawingOrdersSupport supported_orders
            = front.get_supported_orders() - disabled_orders;
        for (unsigned i = 0; i < NB_ORDER_SUPPORT; ++i) {
            client_info.order_caps.orderSupport[i] = supported_orders.test(OrdersIndexes(i));
        }

        auto cookie = get_or_default<std::string>(config, "reconnectCookie");
        auto cookie_len = cookie.size();
        if (cookie.size() > cookie_len) {
            LOG(LOG_NOTICE, "RdpClient: cookie.length = %zu too large", cookie_len);
            cookie_len = std::size(client_info.autoReconnectCookie);
        }
        client_info.cbAutoReconnectCookie = cookie_len;
        memcpy(client_info.autoReconnectCookie, cookie.data(), cookie_len);

        client_info.keylayout = get_or(config, "keylayout", 0);
        client_info.console_session = get_or(config, "consoleSession", false);
        client_info.rdp5_performanceflags = get_or(config, "performanceFlags", false);
        // TODO client_info.client_time_zone = get_or(config, "timezone", false);

        client_info.bitmap_codec_caps.haveRemoteFxCodec = enable_remotefx;
        client_info.has_sound_code = get_or(config, "enableSound", false);

        // TODO client_info.alternate_shell = get_or(config, "enableSound", false);
        // TODO client_info.working_dir = get_or(config, "enableSound", false);

        client_info.large_pointer_caps.largePointerSupportFlags
            = enable_large_pointer ? LARGE_POINTER_FLAG_96x96 : 0;
        client_info.multi_fragment_update_caps.MaxRequestSize
            = get_or(config, "fragmentUpdateMaxRequestSize", uint32_t(0));
        if (enable_large_pointer) {
            client_info.multi_fragment_update_caps.MaxRequestSize = std::max(
                client_info.multi_fragment_update_caps.MaxRequestSize,
                uint32_t(38'055)
            );
        }
        // TODO
        // client_info.general_caps.extraflags
        //     = get_or(config, "fragmentUpdateMaxRequestSize", uint32_t(0));
        // TODO
        // client_info.rail_caps.RailSupportLevel
        //     = get_or(config, "fragmentUpdateMaxRequestSize", uint32_t(0));
        // TODO
        // client_info.window_list_caps
        //     = get_or(config, "fragmentUpdateMaxRequestSize", uint32_t(0));
        //@}


        // init rdp_params
        //@{
        ModRDPParams rdp_params(
            get_or_default<std::string>(config, "username").c_str(),
            get_or_default<std::string>(config, "password").c_str(),
            "0.0.0.0",
            get_or(config, "clientAddress", "0.0.0.0").c_str(),
            get_or(config, "keyFlags", 0),
            font,
            theme,
            server_auto_reconnect_packet,
            close_box_extra_message,
            verbose);

        rdp_params.cache_verbose = get_or(config, "cacheVerbose", BmpCache::Verbose(0));

        rdp_params.device_id           = "device_id"; // for certificate path only
        rdp_params.enable_tls          = false;
        rdp_params.enable_nla          = false;
        rdp_params.enable_fastpath     = true;
        rdp_params.enable_new_pointer  = true;
        rdp_params.server_cert_check   = ServerCertCheck::always_succeed;
        rdp_params.ignore_auth_channel = true;

        rdp_params.enable_remotefx = enable_remotefx;
        rdp_params.enable_restricted_admin_mode = get_or(config, "restrictedAdminMode", false);

        rdp_params.rdp_compression = RdpCompression::rdp6_1;

        rdp_params.open_session_timeout = get_or(config, "openSessionTimeout", 20s);

        rdp_params.disconnect_on_logon_user_change
            = get_or(config, "disconnectOnLogonUserChange", false);

        rdp_params.hide_client_name = get_or(config, "hideClientName", false);
        rdp_params.disabled_orders = disabled_orders;
        rdp_params.enable_glyph_cache = supported_orders.test(TS_NEG_GLYPH_INDEX);

        rdp_params.enable_persistent_disk_bitmap_cache
            = get_or(config, "enablePersistentDiskBitmapCache", false);
        rdp_params.enable_cache_waiting_list
            = get_or(config, "enableCacheWaitingList", false);
        rdp_params.persist_bitmap_cache_on_disk
            = get_or(
                config, "persistBitmapCacheOnDisk",
                rdp_params.enable_persistent_disk_bitmap_cache);

        rdp_params.lang = get_or(config, "lang", Translation::EN);

        rdp_params.adjust_performance_flags_for_recording = false;
        rdp_params.large_pointer_support = enable_large_pointer;
        rdp_params.perform_automatic_reconnection
            = get_or(config, "performAutomaticReconnection", false);

        rdp_params.split_domain = get_or(config, "splitDomain", false);

        rdp_params.error_message = &ini.get_mutable_ref<cfg::context::auth_error_message>();
        //@}


        if (bool(verbose & RDPVerbose::basic_trace)) {
            rdp_params.log();
        }

        this->mod = new_mod_rdp(
            trans, time_base, gd_forwarder,
            fd_events, timer_events, events, sesman, gd, front, client_info,
            redir_info, js_rand, lcg_timeobj, ChannelsAuthorizations("*", ""),
            rdp_params, TLSClientParams{}, authentifier, report_message,
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
        return this->trans.get_output_buffer();
    }

    void reset_output_data()
    {
        this->trans.clear_output_buffer();
    }

    void process_input_data(std::string data)
    {
        this->trans.push_input_buffer(std::move(data));
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

EMSCRIPTEN_BINDINGS(client)
{
    redjs::class_<RdpClient>("RdpClient")
        .constructor<emscripten::val /*gd*/, emscripten::val /*config*/>()
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
