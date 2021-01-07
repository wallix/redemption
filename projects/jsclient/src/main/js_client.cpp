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

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/channels_authorizations.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "utils/genrandom.hpp"
#include "utils/redirection_info.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "utils/theme.hpp"
#include "utils/timebase.hpp"
#include "gdi/osd_api.hpp"

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
    struct val_as_impl
    {
        T operator()(emscripten::val const& prop) const
        {
            if constexpr (std::is_enum_v<T>) {
                static_assert(sizeof(T) <= 4);
                using U = std::underlying_type_t<T>;
                return T(prop.as<U>());
            }
            else if constexpr (std::is_integral_v<T>) {
                static_assert(sizeof(T) <= 4);
                return prop.as<T>();
            }
            else {
                return prop.as<T>();
            }
        }
    };

    template<class Rep, class Period>
    struct val_as_impl<std::chrono::duration<Rep, Period>>
    {
        using result_type = std::chrono::duration<Rep, Period>;

        result_type operator()(emscripten::val const& prop) const
        {
            using Int = std::conditional_t<(sizeof(Rep) > 4), uint32_t, std::make_unsigned_t<Rep>>;
            return result_type(prop.as<Int>());
        }
    };

    template<class T>
    struct val_as_impl<::utils::flags_t<T>>
    {
        using result_type = ::utils::flags_t<T>;

        result_type operator()(emscripten::val const& prop) const
        {
            using Int = typename result_type::bitfield;
            static_assert(sizeof(Int) <= 4);
            return result_type(prop.as<Int>());
        }
    };

    template<class T>
    constexpr inline val_as_impl<T> val_as {};


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
        return not prop ? default_value : val_as<T>(prop);
    };

    std::string get_or(emscripten::val const& v, char const* name, char const* default_value)
    {
        auto prop = v[name];
        return not prop ? default_value : prop.as<std::string>();
    }


    template<class T>
    void set_if(emscripten::val const& v, char const* name, T& value)
    {
        auto prop = v[name];
        if (not not prop) {
            value = val_as<T>(prop);
        }
    }


    template<class F>
    void extract_if(emscripten::val const& v, char const* name, F f)
    {
        auto prop = v[name];
        if (!!prop) {
            f(prop);
        }
    };

    writable_bytes_view extract_bytes(
        emscripten::val const& v, char const* name, writable_bytes_view view)
    {
        auto prop = v[name];
        if (not prop) {
            return view.first(0);
        }

        auto str = prop.as<std::string>();
        auto len = str.size();
        if (view.size() >= len) {
            LOG(LOG_NOTICE, "RdpClient: %s.length = %zu too large (max = %zu)",
                name, len, view.size());
            len = view.size();
        }

        memcpy(view.as_u8p(), str.data(), len);
        return view.first(len);
    }

    writable_bytes_view extract_str(
        emscripten::val const& v, char const* name, writable_bytes_view view)
    {
        auto av = extract_bytes(v, name, view.drop_back(1));
        av.data()[av.size()] = '\0';
        return av;
    }

    void extract_datetime(
        emscripten::val const& v,
        uint8_t (&name)[64],
        SystemTime& system_time,
        uint32_t& bias)
    {
        auto writable_name = make_writable_array_view(name);
        auto av_name = extract_bytes(v, "name", writable_name);
        memset(av_name.end(), 0, writable_name.size() - av_name.size());

        set_if(v, "year", system_time.wYear);
        set_if(v, "month", system_time.wMonth);
        set_if(v, "dayOfWeek", system_time.wDayOfWeek);
        set_if(v, "day", system_time.wDay);
        set_if(v, "hour", system_time.wHour);
        set_if(v, "minute", system_time.wMinute);
        set_if(v, "second", system_time.wSecond);
        set_if(v, "millisecond", system_time.wMilliseconds);

        set_if(v, "bias", bias);
    }


    ScreenInfo extract_screen_info(emscripten::val const& config)
    {
        return ScreenInfo{
            get_or(config, "width", uint16_t(800)),
            get_or(config, "height", uint16_t(600)),
            get_or(config, "bpp", BitsPerPixel(16))
        };
    }

    RDPVerbose extract_rdp_verbose(emscripten::val const& config)
    {
        static_assert(sizeof(RDPVerbose) == 4, "verbose is truncated");
        return get_or(config, "verbose", RDPVerbose(0));
        // | (get_or(config, "verbose2", uint32_t(0)) << 32);
    }
}

class RdpClient
{
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

    EventContainer events;

    ModRdpFactory mod_rdp_factory;
    std::unique_ptr<mod_api> mod;

    redjs::Transport trans;

    ClientInfo client_info;

    redjs::Front front;
    gdi::GraphicApi& gd;
    gdi::NullOsd osd;

    Inifile ini;

    JsRandom js_rand;
    NullSessionLog session_log;
    NullLicenseStore license_store;
    RedirectionInfo redir_info;

    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;

    ModRDPParams::ServerInfo server_info {};

public:
    RdpClient(emscripten::val&& graphics, emscripten::val const& config)
    : RdpClient(
        std::move(graphics), config,
        extract_screen_info(config),
        extract_rdp_verbose(config))
    {}

    RdpClient(
        emscripten::val&& graphics,
        emscripten::val const& config,
        ScreenInfo screen_info,
        RDPVerbose verbose)
    : front(std::move(graphics), screen_info.width, screen_info.height, verbose)
    , gd(front.graphic_api())
    , js_rand(config)
    {
        using namespace std::chrono_literals;

        {
            uint32_t seconds = 0;
            uint32_t milliseconds = 0;
            extract_if(config, "time", [&](emscripten::val const& time){
                set_if(time, "second", seconds);
                set_if(time, "millisecond", milliseconds);
            });
            events.set_current_time({checked_int{seconds}, checked_int{milliseconds}});
        }

        client_info.screen_info = screen_info;
        client_info.build = get_or(config, "build", 420);

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

        auto cookie = extract_bytes(config, "cookie",
            make_writable_array_view(client_info.autoReconnectCookie));
        client_info.cbAutoReconnectCookie = cookie.size();

        set_if(config, "keylayout", client_info.keylayout);
        set_if(config, "consoleSession", client_info.console_session);
        set_if(config, "performanceFlags", client_info.rdp5_performanceflags);

        extract_if(config, "timezone", [&](emscripten::val const& timezone){
            extract_if(timezone, "standard", [&](emscripten::val const& datetime){
                extract_datetime(datetime,
                    client_info.client_time_zone.DaylightName,
                    client_info.client_time_zone.DaylightDate,
                    client_info.client_time_zone.DaylightBias
                );
            });
            extract_if(timezone, "daylight", [&](emscripten::val const& datetime){
                extract_datetime(datetime,
                    client_info.client_time_zone.StandardName,
                    client_info.client_time_zone.StandardDate,
                    client_info.client_time_zone.StandardBias
                );
            });
        });

        client_info.bitmap_codec_caps.haveRemoteFxCodec = enable_remotefx;
        client_info.has_sound_code = get_or(config, "enableSound", false);
        client_info.has_sound_capture_code = get_or(config, "enableSoundCapture", false);

        uint32_t maxRequestSize = client_info.multi_fragment_update_caps.MaxRequestSize;
        set_if(config, "fragmentUpdateMaxRequestSize", maxRequestSize);

        extract_str(config, "alternateShell",
            make_writable_array_view(client_info.alternate_shell));
        extract_str(config, "workingDirectory",
            make_writable_array_view(client_info.working_dir));

        set_if(config, "desktopPhysicalWidth", client_info.desktop_physical_width);
        set_if(config, "desktopPhysicalHeight", client_info.desktop_physical_height);
        set_if(config, "desktopOrientation", client_info.desktop_orientation);
        set_if(config, "desktopScaleFactor", client_info.desktop_scale_factor);
        set_if(config, "deviceScaleFactor", client_info.device_scale_factor);

        if (enable_large_pointer) {
            client_info.large_pointer_caps.largePointerSupportFlags = LARGE_POINTER_FLAG_96x96;
            maxRequestSize = std::max(maxRequestSize, uint32_t(38'055));
        }

        client_info.multi_fragment_update_caps.MaxRequestSize = maxRequestSize;
        //@}


        // init rdp_params
        //@{
        const auto username = get_or_default<std::string>(config, "username");
        const auto password = get_or_default<std::string>(config, "password");
        const auto client_addr = get_or(config, "clientAddress", "0.0.0.0");

        ModRDPParams rdp_params(
            username.c_str(),
            password.c_str(),
            "0.0.0.0",
            client_addr.c_str(),
            get_or(config, "keyFlags", 0),
            font,
            theme,
            server_auto_reconnect_packet,
            close_box_extra_message,
            verbose);

        rdp_params.server_info_ref = &server_info;

        rdp_params.cache_verbose = get_or(config, "cacheVerbose", BmpCache::Verbose(0));

        rdp_params.device_id           = "device_id"; // for certificate path only
        rdp_params.enable_tls          = false;
        rdp_params.enable_nla          = false;
        rdp_params.server_cert_check   = ServerCertCheck::always_succeed;
        rdp_params.ignore_auth_channel = true;

        set_if(config, "enableFastPath", rdp_params.enable_fastpath);
        set_if(config, "enableNewPointer", rdp_params.enable_new_pointer);

        rdp_params.enable_remotefx = enable_remotefx;
        set_if(config, "restrictedAdminMode", rdp_params.enable_restricted_admin_mode);

        rdp_params.rdp_compression = RdpCompression::rdp6_1;

        rdp_params.open_session_timeout = get_or(config, "openSessionTimeout", 20s);

        set_if(config, "disconnectOnLogonUserChange", rdp_params.disconnect_on_logon_user_change);

        set_if(config, "hideClientName", rdp_params.hide_client_name);
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

        set_if(config, "lang", rdp_params.lang);

        rdp_params.allow_scale_factor
            = client_info.desktop_scale_factor && client_info.device_scale_factor;

        rdp_params.adjust_performance_flags_for_recording = false;
        rdp_params.large_pointer_support = enable_large_pointer;
        set_if(config, "performAutomaticReconnection", rdp_params.perform_automatic_reconnection);

        set_if(config, "splitDomain", rdp_params.split_domain);

        rdp_params.error_message = &ini.get_mutable_ref<cfg::context::auth_error_message>();
        //@}


        if (bool(verbose & RDPVerbose::basic_trace)) {
            rdp_params.log();
        }

        this->mod = new_mod_rdp(
            trans, gd, osd, events, session_log, front, client_info,
            redir_info, js_rand, ChannelsAuthorizations("*", ""),
            rdp_params, TLSClientParams{},
            license_store, ini, nullptr, nullptr, this->mod_rdp_factory);
    }

    void write_first_packet()
    {
        this->set_time(this->events.get_current_time());
    }

    void set_time(timeval time)
    {
        this->events.set_current_time(time);

        event_loop(this->events.queue, [time](Event& event){
            if (event.alarm.trigger(time)) {
                event.actions.exec_timeout(event);
            }
        });
    }

    void process_input_data(std::string data)
    {
        this->trans.push_input_buffer(std::move(data));

        auto time = this->events.get_current_time();
        event_loop(this->events.queue, [time](Event& event){
            /*if (event.alarm.fd != -1)*/ {
                event.alarm.reset_timeout(time + event.alarm.grace_delay);
                event.actions.exec_action(event);
            }
        });

        this->events.garbage_collector();
    }

private:
    template<class Fn>
    static void event_loop(std::vector<Event*> events, Fn&& fn)
    {
        size_t iend = events.size();
        // ignore events created in the loop
        for (size_t i = 0 ; i < iend; ++i){ /*NOLINT*/
            assert(iend <= events.size());
            Event& event = *events[i];
            if (!event.garbage){
                fn(event);
            }
        }
    }

public:
    bytes_view get_output_buffer() const
    {
        return this->trans.get_output_buffer();
    }

    void reset_output_data()
    {
        this->trans.clear_output_buffer();
    }

    void write_scancode_event(uint16_t scancode)
    {
        uint16_t key = scancode & 0xFF;
        uint16_t flag = scancode & 0xFF00;
        this->mod->rdp_input_scancode(
            key, 0, flag,
            this->events.get_current_time().tv_sec,
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

    uint16_t get_input_flags() const
    {
        return this->server_info.input_flags;
    }

    uint32_t get_keyboard_layout() const
    {
        return this->server_info.keyboard_layout;
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
        .function("getInputFlags", &RdpClient::get_input_flags)
        .function("getKeyboardLayout", &RdpClient::get_keyboard_layout)
    ;
}
