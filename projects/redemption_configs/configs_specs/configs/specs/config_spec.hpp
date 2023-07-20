/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/attributes/spec.hpp"
#include "configs/enumeration.hpp"

#include "configs/autogen/enums.hpp"
#include "configs/type_name.hpp"

#include "include/debug_verbose_description.hpp"

#include "keyboard/keylayouts.hpp"
#include "utils/colors.hpp"
#include "utils/file_permissions.hpp"

#include <chrono>
#include <vector>
#include <string>


namespace cfg_specs {

#ifdef IN_IDE_PARSER
    constexpr char const* CONFIG_DESC_AUTH = "";
    constexpr char const* CONFIG_DESC_FRONT = "";
    constexpr char const* CONFIG_DESC_RDP = "";
    constexpr char const* CONFIG_DESC_VNC = "";
    constexpr char const* CONFIG_DESC_SCK = "";
    constexpr char const* CONFIG_DESC_CAPTURE = "";
    constexpr char const* CONFIG_DESC_SESSION = "";
    constexpr char const* CONFIG_DESC_MOD_INTERNAL = "";
    constexpr char const* CONFIG_DESC_COMPRESSION = "";
    constexpr char const* CONFIG_DESC_CACHE = "";
    constexpr char const* CONFIG_DESC_OCR = "";

    // for coloration...
    struct Writer
    {
        void set_sections(std::initializer_list<char const*> l);

        template<class F>
        void section(char const * name, F closure_section);

        template<class F>
        void section(cfg_attributes::names, F closure_section);

        template<class... Args>
        void member(Args...);

        void build();
    };
#else
template<class Writer>
#endif
void config_spec_definition(Writer && _)
{

using namespace cfg_attributes;

using namespace cfg_attributes::spec::constants;
using namespace cfg_attributes::sesman::constants;
using namespace cfg_attributes::connpolicy::constants;

// force ordering section
_.set_sections({
    "globals",

    "client",

    "all_target_mod",
    "remote_program",
    "mod_rdp",
    "protocol",
    "session_probe",
    "server_cert",
    "mod_vnc",

    "session_log",
    "ocr",
    "capture",
    "video",
    "audit",

    "file_verification",
    "file_storage",
    "icap_server_down",
    "icap_server_up",

    "crypto",
    "websocket",

    "vnc_over_ssh",

    "context",

    "internal_mod",
    "mod_replay",
    "translation",
    "theme",

    "debug",
});

prefix_value disable_prefix_val{"disable"};

// updated by sesman
constexpr char default_key[] =
    "\x00\x01\x02\x03\x04\x05\x06\x07"
    "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
    "\x10\x11\x12\x13\x14\x15\x16\x17"
    "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
;

auto L = spec::log_policy::loggable;
auto NL = spec::log_policy::unloggable;
auto VNL = spec::log_policy::unloggable_if_value_contains_password;

auto D = Tags{TagList::Debug};
auto W = Tags{TagList::Workaround};

auto vnc_connpolicy = sesman::connection_policy{"vnc"};
auto rdp_without_jh_connpolicy = sesman::connection_policy{"rdp"};
auto jh_without_rdp_connpolicy = sesman::connection_policy{"rdp-jumphost"};
auto rdp_and_jh_connpolicy = rdp_without_jh_connpolicy | jh_without_rdp_connpolicy;
auto all_connpolicy = rdp_and_jh_connpolicy | vnc_connpolicy;

char const* disabled_orders_desc =
    "Disables supported drawing orders:\n"
    "   0: DstBlt\n"
    "   1: PatBlt\n"
    "   2: ScrBlt\n"
    "   3: MemBlt\n"
    "   4: Mem3Blt\n"
    "   9: LineTo\n"
    "  15: MultiDstBlt\n"
    "  16: MultiPatBlt\n"
    "  17: MultiScrBlt\n"
    "  18: MultiOpaqueRect\n"
    "  22: Polyline\n"
    "  25: EllipseSC\n"
    "  27: GlyphIndex"
;

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wc99-designator")

_.section("globals", [&]
{
    auto co_session = connpolicy::section{"session"};

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"front_connection_time"},
             type_<std::chrono::milliseconds>(),
             desc{"from incoming connection to \"up_and_running\" state"});

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"target_connection_time"},
             type_<std::chrono::milliseconds>(),
             desc{"from Module rdp creation to \"up_and_running\" state"});


    _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L,
             names{
                .all="auth_user",
                .sesman="login"
             },
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L,
             names{
                .all="host",
                .sesman="ip_client"
             },
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{
                .all="target",
                .sesman="ip_target"
             },
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"target_device"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"device_id"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"primary_user_id"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L,
             names{
                .all="target_user",
                .sesman="target_login"
             },
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"target_application"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"target_application_account"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, NL,
             names{"target_application_password"},
             type_<std::string>());

    _.member(advanced_in_gui | iptables_in_gui | logged_in_gui, no_sesman, L,
             names{"port"},
             type_<types::unsigned_>(),
             set(3389),
             desc{
                "Warning: Service will be automatically restarted and active sessions will be disconnected.\n"
                "The port set in this field must not be already used, otherwise the service will not run.\n"
                "Changing the port number will prevent WALLIX Access Manager from working properly."
             });

    _.member(advanced_in_gui, no_sesman, L,
             names{"encryptionLevel"},
             type_<Level>(),
             spec::type_<std::string>(),
             set(Level::low));

    _.member(hidden_in_gui, no_sesman, L,
             names{"authfile"},
             type_<std::string>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_AUTHFILE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"handshake_timeout"},
             type_<std::chrono::seconds>(),
             set(10),
             desc{"Time out during RDP handshake stage."});

    _.member(ini_and_gui, no_sesman, L,
             names{"base_inactivity_timeout"},
             type_<std::chrono::seconds>(),
             set(900),
             desc{"No automatic disconnection due to inactivity, timer is set on primary authentication.\nIf value is between 1 and 30, then 30 is used.\nIf value is set to 0, then inactivity timeout value is unlimited."});

    _.member(hidden_in_gui, all_connpolicy, co_session, L,
             names{"inactivity_timeout"},
             type_<std::chrono::seconds>(),
             set(0),
             desc{
                "No automatic disconnection due to inactivity, timer is set on target session.\n"
                "If value is between 1 and 30, then 30 is used.\n"
                "If value is set to 0, then value set in \"Base inactivity timeout\" (in \"RDP Proxy\" configuration option) is used."
             });

    _.member(hidden_in_gui, no_sesman, L,
             names{"keepalive_grace_delay"},
             type_<std::chrono::seconds>(),
             set(30),
             desc{"Internal keepalive between sesman and rdp proxy"});

    _.member(advanced_in_gui, no_sesman, L,
             names{"authentication_timeout"},
             type_<std::chrono::seconds>(),
             set(120),
             desc{"Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate)."});

    _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"trace_type"},
             type_<TraceType>(),
             set(TraceType::localfile_hashed));

    _.member(advanced_in_gui, no_sesman, L,
             names{"listen_address"},
             type_<types::ip_string>(),
             set("0.0.0.0"));

    _.member(iptables_in_gui, no_sesman, L,
             names{"enable_transparent_mode"},
             type_<bool>(),
             set(false),
             desc{"Allow Transparent mode."});

    _.member(advanced_in_gui | password_in_gui, no_sesman, L,
             names{"certificate_password"},
             type_<types::fixed_string<254>>(),
             set("inquisition"),
             desc{"Proxy certificate password."});

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"is_rec"},
             set(false),
             type_<bool>());

    _.member(advanced_in_gui, no_sesman, L,
             names{"enable_bitmap_update"},
             type_<bool>(),
             set(true),
             desc{"Support of Bitmap Update."});


    _.member(ini_and_gui, no_sesman, L,
             names{"enable_close_box"},
             type_<bool>(),
             set(true),
             desc{"Show close screen."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"close_timeout"},
             type_<std::chrono::seconds>(),
             set(600),
             desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."});


    _.member(advanced_in_gui, no_sesman, L,
             names{"enable_osd"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"enable_osd_display_remote_target"},
             type_<bool>(),
             set(true),
             desc{"Show target address with F12."});


    _.member(hidden_in_gui, no_sesman, L,
             names{"enable_wab_integration"},
             type_<bool>(),
             set((CPP_EXPR(REDEMPTION_CONFIG_ENABLE_WAB_INTEGRATION))));


    // TODO move to [client]
    _.member(ini_and_gui, no_sesman, L,
             names{"allow_using_multiple_monitors"},
             type_<bool>(),
             set(true));

    // TODO move to [client] / [mod_rdp]
    _.member(ini_and_gui, no_sesman, L,
             names{"allow_scale_factor"},
             type_<bool>(),
             set(false));


    _.member(advanced_in_gui, no_sesman, L,
             names{"bogus_refresh_rect"},
             type_<bool>(),
             set(true),
             desc{"Needed to refresh screen of Windows Server 2012."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"large_pointer_support"},
             type_<bool>(),
             set(true));

    _.member(ini_and_gui, no_sesman, L,
             names{"new_pointer_update_support"},
             type_<bool>(),
             set(true));

    _.member(ini_and_gui, no_sesman, L,
             names{"unicode_keyboard_event_support"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"mod_recv_timeout"},
             type_<types::range<std::chrono::milliseconds, 100, 10000>>(),
             set(1000));

    _.member(advanced_in_gui, no_sesman, L,
             names{"experimental_enable_serializer_data_block_size_limit"},
             type_<bool>(),
             set(false));

    _.member(advanced_in_gui, no_sesman, L,
             names{"experimental_support_resize_session_during_recording"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"support_connection_redirection_during_recording"},
             type_<bool>(),
             set(true));

    _.member(ini_and_gui, no_sesman, L,
             names{"rdp_keepalive_connection_interval"},
             type_<std::chrono::milliseconds>(),
             set(0),
             desc{
                "Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.\n"
                "!!!May cause FreeRDP-based client to CRASH!!!\n"
                "Set to 0 to disable this feature."
             });

    _.member(ini_and_gui, no_sesman, L,
             names{"enable_ipv6"},
             type_<bool>(),
             set(true),
             desc{
                "Enable primary connection on ipv6.\n"
                "Warning: Service need to be manually restarted to take changes into account"
             });

    _.member(hidden_in_gui, no_sesman, L,
             names{"minimal_memory_available_before_connection_silently_closed"},
             type_<types::u64>(),
             set(100),
             desc{"In megabytes. 0 for disabled."});
});

_.section("session_log", [&]
{
    _.member(ini_and_gui, no_sesman, L,
             names{"enable_session_log"},
             type_<bool>(),
             set(true));

    _.member(ini_and_gui, no_sesman, L,
             names{"enable_arcsight_log"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"keyboard_input_masking_level"},
             type_<KeyboardInputMaskingLevel>(),
             set(KeyboardInputMaskingLevel::password_and_unidentified),
             desc{"Keyboard Input Masking Level:"});
});

_.section("client", [&]
{
    // to_string()
    std::string keyboard_layout_proposals_desc;
    for (KeyLayout const& layout : keylayouts_sorted_by_name()) {
        keyboard_layout_proposals_desc += layout.name;
        keyboard_layout_proposals_desc += ", ";
    }
    if (!keyboard_layout_proposals_desc.empty()) {
        keyboard_layout_proposals_desc.resize(keyboard_layout_proposals_desc.size() - 2);
    }

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"keyboard_layout"},
             type_<types::unsigned_>(),
             set(0));

    _.member(advanced_in_gui, no_sesman, L,
             names{"keyboard_layout_proposals"},
             type_<types::list<std::string>>(),
             set("en-US, fr-FR, de-DE, ru-RU"),
             desc{keyboard_layout_proposals_desc});


    _.member(advanced_in_gui, no_sesman, L,
             names{"ignore_logon_password"},
             type_<bool>(),
             set(false),
             desc{"If true, ignore password provided by RDP client, user need do login manually."});

    _.member(hidden_in_gui, no_sesman, L,
             names{"performance_flags_default"},
             type_<types::u32>(),
             set(0x80),
             desc{"Enable font smoothing (0x80)."});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"performance_flags_force_present"},
             type_<types::u32>(),
             set(0x28),
             desc{
                "Disable wallpaper (0x1).\n"
                "Disable full-window drag (0x2).\n"
                "Disable menu animations (0x4).\n"
                "Disable theme (0x8).\n"
                "Disable mouse cursor shadows (0x20).\n"
                "Disable cursor blinking (0x40).\n"
                "Enable font smoothing (0x80).\n"
                "Enable Desktop Composition (0x100)."
             });

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"performance_flags_force_not_present"},
             type_<types::u32>(),
             set(0),
             desc{"See the comment of \"Performance flags force present\" above for available values."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"auto_adjust_performance_flags"},
             type_<bool>(),
             set(true),
             desc{"If enabled, avoid automatically font smoothing in recorded session."});


    _.member(ini_and_gui, no_sesman, L,
             names{"tls_fallback_legacy"},
             type_<bool>(),
             set(false),
             desc{"Fallback to RDP Legacy Encryption if client does not support TLS."});

    _.member(ini_and_gui, no_sesman, L,
             names{"tls_support"},
             type_<bool>(),
             set(true));

    _.member(ini_and_gui, no_sesman, L,
             names{"tls_min_level"},
             type_<types::u32>(),
             set(2),
             desc{"Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"});

    _.member(ini_and_gui, no_sesman, L,
             names{"tls_max_level"},
             type_<types::u32>(),
             set(0),
             desc{"Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"});

    _.member(ini_and_gui, no_sesman, L, D,
             names{"show_common_cipher_list"},
             type_<bool>(),
             set(false),
             desc{"Show common cipher list supported by client and server"});


    _.member(advanced_in_gui, no_sesman, L,
             names{"enable_nla"},
             type_<bool>(),
             set(false),
             desc{"Needed for primary NTLM or Kerberos connections over NLA."});

    _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"disable_tsk_switch_shortcuts"},
             type_<bool>(),
             set(false),
             desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"rdp_compression"},
             type_<RdpCompression>(),
             set(RdpCompression::rdp6_1));

    _.member(advanced_in_gui, no_sesman, L,
             names{"max_color_depth"},
             type_<ColorDepth>(),
             set(ColorDepth::depth24));

    _.member(advanced_in_gui, no_sesman, L,
             names{"persistent_disk_bitmap_cache"},
             type_<bool>(),
             set(true),
             desc{"Persistent Disk Bitmap Cache on the front side."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"cache_waiting_list"},
             type_<bool>(),
             set(false),
             desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"persist_bitmap_cache_on_disk"},
             type_<bool>(),
             set(false),
             desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"bitmap_compression"},
             type_<bool>(),
             set(true),
             desc{"Support of Bitmap Compression."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"fast_path"},
             type_<bool>(),
             set(true),
             desc{"Enables support of Client Fast-Path Input Event PDUs."});

    _.member(ini_and_gui, no_sesman, L,
             names{"enable_suppress_output"},
             set(true),
             type_<bool>());

    _.member(ini_and_gui, no_sesman, L,
             names{"ssl_cipher_list"},
             type_<std::string>(),
             set("HIGH:!ADH:!3DES:!SHA"),
             desc{
                "[Not configured]: Compatible with more RDP clients (less secure)\n"
                "HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)\n"
                "HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)"
             });

    _.member(ini_and_gui, no_sesman, L,
             names{"show_target_user_in_f12_message"},
             type_<bool>(),
             set(false));

    _.member(ini_and_gui, no_sesman, L,
             names{"bogus_ios_glyph_support_level"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"transform_glyph_to_bitmap"},
             type_<bool>(),
             set(false));

    _.member(ini_and_gui, no_sesman, L,
             names{"bogus_number_of_fastpath_input_event"},
             type_<BogusNumberOfFastpathInputEvent>(),
             set(BogusNumberOfFastpathInputEvent::pause_key_only));

    _.member(advanced_in_gui, no_sesman, L,
             names{"recv_timeout"},
             type_<types::range<std::chrono::milliseconds, 100, 10000>>(),
             set(1000));

    _.member(ini_and_gui, no_sesman, L,
             names{"enable_osd_4_eyes"},
             type_<bool>(),
             set(true),
             desc{"Enables display of message informing user that his/her session is being audited."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"enable_remotefx"},
             type_<bool>(),
             set(true),
             desc{"Enable front remoteFx"});

    _.member(advanced_in_gui, no_sesman, L, D,
             names{"disabled_orders"},
             type_<types::list<types::unsigned_>>(),
             set("25"),
             desc{disabled_orders_desc});
});

_.section("all_target_mod", [&]
{
    _.member(advanced_in_gui, no_sesman, L,
             names{"connection_establishment_timeout"},
             type_<types::range<std::chrono::milliseconds, 1000, 10000>>(),
             set(3000),
             desc{"The maximum time in milliseconds that the proxy will wait while attempting to connect to an target."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"tcp_user_timeout"},
             type_<types::range<std::chrono::milliseconds, 0, 3'600'000>>(),
             set(0),
             desc{"This parameter allows you to specify max timeout in milliseconds before a TCP connection is aborted. If the option value is specified as 0, TCP will use the system default."});
});

_.section(names{.all="mod_rdp", .connpolicy="rdp"}, [&]
{
    _.member(advanced_in_gui, no_sesman, L,
             names{"rdp_compression"},
             type_<RdpCompression>(),
             set(RdpCompression::rdp6_1));

    _.member(advanced_in_gui, no_sesman, L,
             names{"disconnect_on_logon_user_change"},
             type_<bool>(),
             set(false));

    _.member(advanced_in_gui, no_sesman, L,
             names{"open_session_timeout"},
             type_<std::chrono::seconds>(),
             set(0));

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, D,
             names{"disabled_orders"},
             type_<types::list<types::unsigned_>>(),
             // disable glyph_index / glyph_cache
             set("27"),
             desc{disabled_orders_desc});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_nla"},
             type_<bool>(),
             set(true),
             jh_without_rdp_connpolicy.always(false),
             desc{"NLA authentication in secondary target."});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_kerberos"},
             type_<bool>(),
             set(false),
             desc{
                "If enabled, NLA authentication will try Kerberos before NTLM.\n"
                "(if enable_nla is disabled, this value is ignored)."
             });

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L,
             names{"tls_min_level"},
             type_<types::u32>(),
             set(0),
             desc{"Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"});

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L,
             names{"tls_max_level"},
             type_<types::u32>(),
             set(0),
             desc{"Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"});

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L,
             names{"cipher_string"},
             type_<std::string>(),
             set("ALL"),
             desc{"TLSv1.2 additional ciphers supported by client, default is empty to apply system-wide configuration (SSL security level 2), ALL for support of all ciphers to ensure highest compatibility with target servers."});

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L, D,
             names{"show_common_cipher_list"},
             type_<bool>(),
             set(false),
             desc{"Show common cipher list supported by client and server"});

    _.member(advanced_in_gui, no_sesman, L,
             names{"persistent_disk_bitmap_cache"},
             type_<bool>(),
             set(true),
             desc{"Persistent Disk Bitmap Cache on the mod side."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"cache_waiting_list"},
             type_<bool>(),
             set(true),
             desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"persist_bitmap_cache_on_disk"},
             type_<bool>(),
             set(false),
             desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."});

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"allow_channels"},
             type_<types::list<std::string>>(),
             set("*"),
             desc{"List of enabled (static) virtual channel (example: channel1,channel2,etc). Character * only, activate all with low priority."});

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"deny_channels"},
             type_<types::list<std::string>>(),
             desc{"List of disabled (static) virtual channel (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"allowed_dynamic_channels"},
             type_<std::string>(),
             set("*"),
             desc{"List of enabled dynamic virtual channel (example: channel1,channel2,etc). Character * only, activate all."});

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"denied_dynamic_channels"},
             type_<std::string>(),
             desc{"List of disabled dynamic virtual channel (example: channel1,channel2,etc). Character * only, deactivate all."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"fast_path"},
             type_<bool>(),
             set(true),
             desc{"Enables support of Client/Server Fast-Path Input/Update PDUs.\nFast-Path is required for Windows Server 2012 (or more recent)!"});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{
                .all="server_redirection_support",
                .connpolicy="server_redirection"
             },
             type_<bool>(),
             set(false),
             desc{"Enables Server Redirection Support."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"client_address_sent"},
             type_<ClientAddressSent>(),
             set(ClientAddressSent::no_address),
             desc{"Client Address to send to target (in InfoPacket)"});

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{"load_balance_info"},
             type_<std::string>(),
             desc{"Load balancing information"});

    _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{
                .all="bogus_sc_net_size",
                .sesman="rdp_bogus_sc_net_size"
             },
             type_<bool>(),
             set(true),
             desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."});

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"proxy_managed_drives"},
             type_<types::list<std::string>>());

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"ignore_auth_channel"},
             set(false),
             type_<bool>());

    _.member(ini_and_gui, no_sesman, L,
             names{"auth_channel"},
             type_<types::fixed_string<7>>(),
             set("*"),
             desc{"Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel."});

    _.member(ini_and_gui, no_sesman, L,
             names{"checkout_channel"},
             type_<types::fixed_string<7>>(),
             desc{"Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel."});

    _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"alternate_shell"},
             type_<std::string>());

    _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"shell_arguments"},
             type_<std::string>());

    _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"shell_working_directory"},
             type_<std::string>());

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"use_client_provided_alternate_shell"},
             type_<bool>(),
             set(false),
             desc{"As far as possible, use client-provided initial program (Alternate Shell)"});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"use_client_provided_remoteapp"},
             type_<bool>(),
             set(false),
             desc{"As far as possible, use client-provided remote program (RemoteApp)"});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"use_native_remoteapp_capability"},
             type_<bool>(),
             set(true),
             desc{"As far as possible, use native RemoteApp capability"});

    _.member(hidden_in_gui, no_sesman, L,
             names{"application_driver_exe_or_file"},
             type_<types::fixed_string<256>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_EXE_OR_FILE)));

    _.member(hidden_in_gui, no_sesman, L,
             names{"application_driver_script_argument"},
             type_<types::fixed_string<256>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_SCRIPT_ARGUMENT)));

    _.member(hidden_in_gui, no_sesman, L,
             names{"application_driver_chrome_dt_script"},
             type_<types::fixed_string<256>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_CHROME_DT_SCRIPT)));

    _.member(hidden_in_gui, no_sesman, L,
             names{"application_driver_chrome_uia_script"},
             type_<types::fixed_string<256>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_CHROME_UIA_SCRIPT)));

    _.member(hidden_in_gui, no_sesman, L,
             names{"application_driver_firefox_uia_script"},
             type_<types::fixed_string<256>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_FIREFOX_UIA_SCRIPT)));

    _.member(hidden_in_gui, no_sesman, L,
             names{"application_driver_ie_script"},
             type_<types::fixed_string<256>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_IE_SCRIPT)));

    _.member(ini_and_gui, no_sesman, L,
             names{"hide_client_name"},
             type_<bool>(),
             set(true),
             desc{
                "Do not transmit client machine name to RDP server.\n"
                "If Per-Device licensing mode is configured on the RD host, this Bastion will consume a CAL for all of these connections to the RD host."
             });

    _.member(advanced_in_gui, no_sesman, L,
             names{"use_license_store"},
             type_<bool>(),
             set(true),
             desc{"Stores CALs issued by the terminal servers."});

    _.member(ini_and_gui, no_sesman, L,
             names{"bogus_ios_rdpdr_virtual_channel"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"enable_rdpdr_data_analysis"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"remoteapp_bypass_legal_notice_delay"},
             type_<std::chrono::milliseconds>(),
             set(0),
             desc{
                "Delay in milliseconds before automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
                "Set to 0 to disable this feature."
             });

    _.member(advanced_in_gui, no_sesman, L,
             names{"remoteapp_bypass_legal_notice_timeout"},
             type_<std::chrono::milliseconds>(),
             set(20000),
             desc{
                "Time limit in milliseconds to automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
                "Set to 0 to disable this feature."
             });

    _.member(advanced_in_gui, no_sesman, L,
             names{"log_only_relevant_clipboard_activities"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"experimental_fix_too_long_cookie"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"split_domain"},
             type_<bool>(),
             set(false),
             desc{"Force to split target domain and username with '@' separator."});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{
                .all="wabam_uses_translated_remoteapp",
                .display="Enable translated RemoteAPP with AM",
             },
             type_<bool>(),
             set(false));

    _.member(advanced_in_gui, no_sesman, L,
             names{"session_shadowing_support"},
             type_<bool>(),
             set(true),
             desc{"Enables Session Shadowing Support."});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_remotefx"},
             type_<bool>(),
             set(false),
             desc{"Enables support of the remoteFX codec."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"accept_monitor_layout_change_if_capture_is_not_started"},
             set(false),
             type_<bool>());

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_restricted_admin_mode"},
             type_<bool>(),
             set(false),
             desc{
                 "Connect to the server in Restricted Admin mode.\n"
                 "This mode must be supported by the server (available from Windows Server 2012 R2), otherwise, connection will fail.\n"
                 "NLA must be enabled."
             });

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"force_smartcard_authentication"},
             type_<bool>(),
             set(false),
             desc{
                "NLA will be disabled.\n"
                "Target must be set for interactive login, otherwise server connection may not be guaranteed.\n"
                "Smartcard device must be available on client desktop.\n"
                "Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service."
             });

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"enable_ipv6"},
             type_<bool>(),
             set(true),
             desc{"Enable target connection on ipv6"});

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{
                .all="mode_console",
                .display="Console mode"
             },
             type_<RdpModeConsole>(),
             spec::type_<std::string>(),
             set(RdpModeConsole::allow),
             desc{"Console mode management for targets on Windows Server 2003 (requested with /console or /admin mstsc option)"});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"auto_reconnection_on_losing_target_link"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, W,
             names{"allow_session_reconnection_by_shortcut"},
             type_<bool>(),
             set(false),
             desc{
                "If the feature is enabled, the end user can trigger a session disconnection/reconnection with the shortcut Ctrl+F12.\n"
                "This feature should not be used together with the End disconnected session option (section session_probe).\n"
                "The keyboard shortcut is fixed and cannot be changed."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"session_reconnection_delay"},
             type_<types::range<std::chrono::milliseconds, 0, 15000>>(),
             set(0),
             desc{"The delay in milliseconds between a session disconnection and the automatic reconnection that follows."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"forward_client_build_number"},
             type_<bool>(),
             set(true),
             desc{
                "Forward the build number advertised by the client to the server. "
                "If forwarding is disabled a default (static) build number will be sent to the server."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"bogus_monitor_layout_treatment"},
             type_<bool>(),
             set(false),
             desc{"To resolve the session freeze issue with Windows 7/Windows Server 2008 target."});

    _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"krb_armoring_account"},
             type_<std::string>(),
             desc{
            "Account to be used for armoring Kerberos tickets. "
            "Must be in the form 'account_name@domain_name[@device_name]'. "
            "If account resolution succeeds the username and password associated with this account will be used; "
            "otherwise the below fallback username and password will be used instead."
            });

    _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"krb_armoring_realm"},
             type_<std::string>(),
             desc{"Realm to be used for armoring Kerberos tickets."});

    _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"krb_armoring_fallback_user"},
             type_<std::string>(),
             desc{"Fallback username to be used for armoring Kerberos tickets."}
    );

    _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, NL,
             names{"krb_armoring_fallback_password"},
             type_<std::string>(),
             desc{"Fallback password to be used for armoring Kerberos tickets."});

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"effective_krb_armoring_user"},
             type_<std::string>(),
             desc{"Effective username to be used for armoring Kerberos tickets."}
    );

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, NL,
             names{"effective_krb_armoring_password"},
             type_<std::string>(),
             desc{"Effective password to be used for armoring Kerberos tickets."}
    );

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"remote_programs_disconnect_message_delay"},
             type_<types::range<std::chrono::milliseconds, 3000, 120000>>(),
             set(3000),
             desc{"Delay in milliseconds before showing disconnect message after the last RemoteApp window is closed."});

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{"use_session_probe_to_launch_remote_program"},
             type_<bool>(),
             set(true),
             desc{"Use Session Probe to launch Remote Program as much as possible."});

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, W,
             names{"replace_null_pointer_by_default_pointer"},
             type_<bool>(),
             set(false),
             desc{"Replace an empty mouse pointer with normal pointer."});
});

_.section("protocol", [&]
{
    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"save_session_info_pdu"},
             type_<RdpSaveSessionInfoPDU>(),
             set(RdpSaveSessionInfoPDU::UnsupportedOrUnknown));
});

_.section("session_probe", [&]
{
    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_session_probe"},
             type_<bool>(),
             set(false),
             rdp_without_jh_connpolicy.set(true) | jh_without_rdp_connpolicy.always(false));

    _.member(hidden_in_gui, no_sesman, L,
             names{"exe_or_file"},
             type_<types::fixed_string<511>>(),
             set("||CMD"));

    _.member(hidden_in_gui, no_sesman, L,
             names{"arguments"},
             type_<types::fixed_string<511>>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS)));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"use_smart_launcher"},
             type_<bool>(),
             set(true),
             desc{
                "Minimum supported server : Windows Server 2008.\n"
                "Clipboard redirection should be remain enabled on Terminal Server."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"enable_launch_mask"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"on_launch_failure"},
             type_<SessionProbeOnLaunchFailure>(),
             set(SessionProbeOnLaunchFailure::disconnect_user));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"launch_timeout"},
             type_<types::range<std::chrono::milliseconds, 0, 300000>>(),
             set(40000),
             desc{
                 "This parameter in milliseconds is used if on_launch_failure is 1 (disconnect user).\n"
                 "0 to disable timeout."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"launch_fallback_timeout"},
             type_<types::range<std::chrono::milliseconds, 0, 300000>>(),
             set(40000),
             desc{
                "This parameter in milliseconds is used if on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).\n"
                "0 to disable timeout."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"start_launch_timeout_timer_only_after_logon"},
             type_<bool>(),
             set(true),
             desc{"Minimum supported server : Windows Server 2008."});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"keepalive_timeout"},
             type_<types::range<std::chrono::milliseconds, 0, 60000>>(),
             set(5000));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"on_keepalive_timeout"},
             type_<SessionProbeOnKeepaliveTimeout>(),
             set(SessionProbeOnKeepaliveTimeout::disconnect_user));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"end_disconnected_session"},
             type_<bool>(),
             set(false),
             desc{
                "Automatically end a disconnected Desktop session or clean up a disconnected RemoteApp session.\n"
                "This option is recommended for Web applications running in Desktop mode.\n"
                "Session Probe must be enabled to use this feature."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_autodeployed_appdriver_affinity"},
             type_<bool>(),
             set(true),
             desc{"End automatically a disconnected auto-deployed Application Driver session.\n"});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"enable_log"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"enable_log_rotation"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, D,
             names{"log_level"},
             type_<SessionProbeLogLevel>(),
             set(SessionProbeLogLevel::Debug));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"disconnected_application_limit"},
             type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(),
             set(0),
             desc{
                "(Deprecated!) This policy setting allows you to configure a time limit in milliseconds for disconnected application sessions.\n"
                "0 to disable timeout."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"disconnected_session_limit"},
             type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(),
             set(0),
             desc{
                "This policy setting allows you to configure a time limit in milliseconds for disconnected Terminal Services sessions.\n"
                "0 to disable timeout."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(),
             names{"idle_session_limit"},
             set(0),
             desc{
                "This parameter allows you to specify the maximum amount of time in milliseconds that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.\n"
                "0 to disable timeout."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"smart_launcher_clipboard_initialization_delay"},
             type_<std::chrono::milliseconds>(),
             set(2000));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"smart_launcher_start_delay"},
             type_<std::chrono::milliseconds>(),
             set(0));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"smart_launcher_long_delay"},
             type_<std::chrono::milliseconds>(),
             set(500));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"smart_launcher_short_delay"},
             type_<std::chrono::milliseconds>(),
             set(50));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{
                .all="smart_launcher_enable_wabam_affinity",
                .display="Enable Smart launcher with AM affinity",
             },
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"launcher_abort_delay"},
             type_<types::range<std::chrono::milliseconds, 0, 300000>>(),
             set(2000));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, D,
             names{"enable_crash_dump"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"handle_usage_limit"},
             type_<types::range<types::u32, 0, 1000>>(),
             set(0));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"memory_usage_limit"},
             type_<types::range<types::u32, 0, 200'000'000>>(),
             set(0));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"cpu_usage_alarm_threshold"},
             type_<types::range<types::u32, 0, 10000>>(),
             set(0),
             desc{
                "As a percentage, the effective alarm threshold is calculated in relation to the reference consumption determined at the start of the program.\n"
                "The alarm is deactivated if this value is less than 200."
             });

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"cpu_usage_alarm_action"},
             type_<SessionProbeCPUUsageAlarmAction>(),
             set(SessionProbeCPUUsageAlarmAction::Restart));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"end_of_session_check_delay_time"},
             type_<types::range<std::chrono::milliseconds, 0, 60000>>(),
             set(0));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"ignore_ui_less_processes_during_end_of_session_check"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"childless_window_as_unidentified_input_field"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"update_disabled_features"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"disabled_features"},
             type_<SessionProbeDisabledFeature>(),
             set(SessionProbeDisabledFeature::chrome_inspection | SessionProbeDisabledFeature::firefox_inspection | SessionProbeDisabledFeature::group_membership));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"enable_bestsafe_interaction"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"on_account_manipulation"},
             type_<SessionProbeOnAccountManipulation>(),
             set(SessionProbeOnAccountManipulation::allow));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"alternate_directory_environment_variable"},
             type_<types::fixed_string<3>>(),
             desc{
            "The name of the environment variable pointing to the alternative directory to launch Session Probe.\n"
            "If empty, the environment variable TMP will be used."});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"public_session"},
             type_<bool>(),
             set(false),
             desc{"If enabled, disconnected session can be recovered by a different primary user."});

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{"outbound_connection_monitoring_rules"},
             type_<std::string>(),
             desc{
        "Comma-separated rules\n"
        "(Ex. IPv4 addresses: $deny:192.168.0.0/24:5900,$allow:192.168.0.110:21)\n"
        "(Ex. IPv6 addresses: $deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389,$allow:[20D1:0:3238:DFE1:63::FEFB]:21)\n"
        "(Ex. hostname can be used to resolve to both IPv4 and IPv6 addresses: $allow:host.domain.net:3389)\n"
        "(Ex. for backwards compatibility only: 10.1.0.0/16:22)\n"
        "Session Probe must be enabled to use this feature."
    });

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{"process_monitoring_rules"},
             type_<std::string>(),
             desc{
        "Comma-separated rules (Ex.: $deny:Taskmgr)\n"
        "@ = All child processes of Bastion Application (Ex.: $deny:@)\n"
        "Session Probe must be enabled to use this feature."
    });

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{"extra_system_processes"},
             type_<std::string>(),
             desc{"Comma-separated extra system processes (Ex.: dllhos.exe,TSTheme.exe)"});

    _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L,
             names{"windows_of_these_applications_as_unidentified_input_field"},
             type_<std::string>(),
             desc{"Comma-separated processes (Ex.: chrome.exe,ngf.exe)"});

    _.member(advanced_in_gui, no_sesman, L,
             names{"customize_executable_name"},
             type_<bool>(),
             set(false));

    _.member(advanced_in_gui, no_sesman, L,
             names{"allow_multiple_handshake"},
             type_<bool>(),
             set(false));

    _.member(advanced_in_gui, no_sesman, L,
             names{"at_end_of_session_freeze_connection_and_wait"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"enable_cleaner"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"clipboard_based_launcher_reset_keyboard_status"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"process_command_line_retrieve_method"},
             type_<SessionProbeProcessCommandLineRetrieveMethod>(),
             set(SessionProbeProcessCommandLineRetrieveMethod::both));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"periodic_task_run_interval"},
             type_<types::range<std::chrono::milliseconds, 300, 2000>>(),
             set(500));

    _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
             names{"pause_if_session_is_disconnected"},
             type_<bool>(),
             set(false));
});

_.section(names{"server_cert"}, [&]
{
    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"server_cert_store"},
             type_<bool>(),
             set(true),
             jh_without_rdp_connpolicy.always(false),
             desc{"Keep known server certificates on Bastion"});

    _.member(hidden_in_gui, rdp_without_jh_connpolicy, L,
             names{"server_cert_check"},
             type_<ServerCertCheck>(),
             set(ServerCertCheck::fails_if_no_match_and_succeed_if_no_know),
             jh_without_rdp_connpolicy.always(ServerCertCheck::always_succeed));

    struct P { char const * name; char const * desc; };
    for (P p : {
        P{"server_access_allowed_message", "Warn if check allow connexion to server."},
        P{"server_cert_create_message", "Warn that new server certificate file was created."},
        P{"server_cert_success_message", "Warn that server certificate file was successfully checked."},
        P{"server_cert_failure_message", "Warn that server certificate file checking failed."},
    }) {
        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
                 names{p.name},
                 type_<ServerNotification>(),
                 set(ServerNotification::syslog),
                 desc{p.desc});
    }

    _.member(hidden_in_gui, no_sesman, L,
             names{"error_message"},
             type_<ServerNotification>(),
             set(ServerNotification::syslog),
             desc{"Warn that server certificate check raised some internal error."});

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"enable_external_validation"},
             type_<bool>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"external_cert"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"external_response"},
             type_<std::string>(),
             desc{"empty string for wait, 'Ok' or error message"});
});

_.section(names{.all="mod_vnc", .connpolicy="vnc"}, [&]
{
    _.member(ini_and_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"clipboard_up"},
             type_<bool>(),
             desc{"Enable or disable the clipboard from client (client to server)."});

    _.member(ini_and_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"clipboard_down"},
             type_<bool>(),
             desc{"Enable or disable the clipboard from server (server to client)."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"encodings"},
             type_<types::list<types::int_>>(),
             desc{
                "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
                "  0: Raw\n"
                "  1: CopyRect\n"
                "  2: RRE\n"
                "  16: ZRLE\n"
                "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
             });

    _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{
                .all="server_clipboard_encoding_type",
                .sesman="vnc_server_clipboard_encoding_type"
             },
             type_<ClipboardEncodingType>(), spec::type_<std::string>(),
             set(ClipboardEncodingType::latin1),
             desc{"VNC server clipboard data encoding type."});

    _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{
                .all="bogus_clipboard_infinite_loop",
                .sesman="vnc_bogus_clipboard_infinite_loop"
             },
             type_<VncBogusClipboardInfiniteLoop>(),
             set(VncBogusClipboardInfiniteLoop::delayed));

    _.member(hidden_in_gui, vnc_connpolicy, L,
             names{"server_is_macos"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, vnc_connpolicy, L,
             names{"server_unix_alt"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, vnc_connpolicy, L,
             names{"support_cursor_pseudo_encoding"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, vnc_connpolicy, L,
             names{"enable_ipv6"},
             type_<bool>(),
             set(true),
             desc{"Enable target connection on ipv6" });
});

_.section(names{"vnc_over_ssh"}, [&]
{
    _.member(external, vnc_connpolicy, L,
             names{"enable"},
             type_<bool>(),
             set(false));

    _.member(external, vnc_connpolicy, L,
             names{"ssh_port"},
             type_<types::unsigned_>(),
             set(22));

    _.member(external, vnc_connpolicy, L,
             names{"tunneling_credential_source"},
             type_<VncTunnelingCredentialSource>(),
             spec::type_<std::string>(),
             set(VncTunnelingCredentialSource::scenario_account),
             desc{
                 "static_login: Static values provided in \"Ssh login\" & \"Ssh password\" fields will be used to establish the SSH tunnel.\n"
                 "scenario_account: Scenario account provided in \"Scenario account name\" field will be used to establish the SSH tunnel. (Recommended)"
             });

    _.member(external, vnc_connpolicy, L,
             names{"ssh_login"},
             type_<std::string>(),
             desc{"Login to be used for SSH tunneling."});

    _.member(external, vnc_connpolicy, NL,
             names{"ssh_password"},
             type_<std::string>(),
             desc{"Password to be used for SSH tunneling."});

    _.member(external, vnc_connpolicy, L,
             names{"scenario_account_name"},
             type_<std::string>(),
             desc{
        "With the following syntax: \"account_name@domain_name[@[device_name]]\".\n"
        "\n"
        "Syntax for using global domain scenario account:\n"
        "  \"account_name@global_domain_name\"\n"
        "\n"
        "Syntax for using local domain scenario account (with automatic device name deduction):\n"
        "  \"account_name@local_domain_name@\""
    });

    _.member(external, vnc_connpolicy | advanced_in_connpolicy, L,
             names{"tunneling_type"},
             type_<VncTunnelingType>(),
             spec::type_<std::string>(),
             set(VncTunnelingType::pxssh), desc{"Only for debugging purposes."});
});

_.section("file_verification", [&]
{
    _.member(hidden_in_gui, no_sesman, L,
             names{"socket_path"},
             type_<std::string>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_VALIDATOR_PATH)));

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"enable_up"},
             type_<bool>(),
             desc{"Enable use of ICAP service for file verification on upload."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"enable_down"},
             type_<bool>(),
             desc{"Enable use of ICAP service for file verification on download."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"clipboard_text_up"},
             type_<bool>(),
             desc{"Verify text data via clipboard from client to server.\nFile verification on upload must be enabled via option Enable up."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"clipboard_text_down"},
             type_<bool>(),
             desc{"Verify text data via clipboard from server to client\nFile verification on download must be enabled via option Enable down."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"block_invalid_file_up"},
             type_<bool>(),
             set(false),
             desc{"Block file transfer from client to server on invalid file verification.\nFile verification on upload must be enabled via option Enable up."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"block_invalid_file_down"},
             type_<bool>(),
             set(false),
             desc{"Block file transfer from server to client on invalid file verification.\nFile verification on download must be enabled via option Enable down."});

    _.member(hidden_in_gui, no_sesman, L,
             names{"block_invalid_clipboard_text_up"},
             type_<bool>(),
             set(false),
             desc{"Block text transfer from client to server on invalid text verification.\nText verification on upload must be enabled via option Clipboard text up."});

    _.member(hidden_in_gui, no_sesman, L,
             names{"block_invalid_clipboard_text_down"},
             type_<bool>(),
             set(false),
             desc{"Block text transfer from server to client on invalid text verification.\nText verification on download must be enabled via option Clipboard text down."});

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"log_if_accepted"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L,
             names{"max_file_size_rejected"},
             type_<types::u32>(),
             set(256),
             desc{
                "If option Block invalid file (up or down) is enabled, automatically reject file with greater filesize (in megabytes).\n"
                "Warning: This value affects the RAM used by the session."
             });

    _.member(hidden_in_gui, no_sesman, L,
             names{"tmpdir"},
             type_<types::dirpath>(),
             set("/tmp/"),
             desc{"Temporary path used when files take up too much memory."});
});

_.section("file_storage", [&]
{
    _.member(hidden_in_gui, rdp_and_jh_connpolicy, L,
             names{"store_file"},
             type_<RdpStoreFile>(),
             spec::type_<std::string>(),
             set(RdpStoreFile::never),
             desc{"Enable storage of transferred files (via RDP Clipboard)."});
});

// for validator only
for (char const* section_name : {"icap_server_down", "icap_server_up"}) {
    // TODO temporary
    // please, update $REDEMPTION/tools/c++-analyzer/lua-checker/checkers/config.lua for each changement of value
    _.section(section_name, [&]
    {
        _.member(external | ini_and_gui, no_sesman, L,
                 names{"host"},
                 type_<std::string>(),
                 desc{"Ip or fqdn of ICAP server"});

        _.member(external | ini_and_gui, no_sesman, L,
                 names{"port"},
                 type_<types::unsigned_>(),
                 set(1344),
                 desc{"Port of ICAP server"});

        _.member(external | ini_and_gui, no_sesman, L,
                 names{"service_name"},
                 type_<std::string>(),
                 set("avscan"),
                 desc{"Service name on ICAP server"});

        _.member(external | ini_and_gui, no_sesman, L,
                 names{"tls"},
                 type_<bool>(),
                 desc{"ICAP server uses tls"});

        _.member(external | advanced_in_gui, no_sesman, L,
                 names{"enable_x_context"},
                 type_<bool>(),
                 set(true),
                 desc{"Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server"});

        _.member(external | advanced_in_gui, no_sesman, L,
                 names{"filename_percent_encoding"},
                 type_<bool>(),
                 set(false),
                 desc{"Filename sent to ICAP as percent encoding"});
    });
}

_.section("mod_replay", [&]
{
    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"replay_path"},
             type_<types::dirpath>(),
             set("/tmp/"));

    _.member(hidden_in_gui, no_sesman, L,
             names{"on_end_of_data"},
             type_<bool>(),
             set(false),
             desc{"0 - Wait for Escape, 1 - End session"});

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"replay_on_loop"},
             type_<bool>(),
             set(false),
             desc{"0 - replay once, 1 - loop replay"});
});

_.section("ocr", [&]
{
    _.member(ini_and_gui, no_sesman, L,
             names{"version"},
             type_<OcrVersion>(),
             set(OcrVersion::v2));

    _.member(ini_and_gui, no_sesman, L,
             names{"locale"},
             type_<OcrLocale>(),
             spec::type_<std::string>(),
             set(OcrLocale::latin));

    _.member(advanced_in_gui, no_sesman, L,
             names{"interval"},
             type_<std::chrono::duration<unsigned, std::centi>>(),
             set(100));

    _.member(advanced_in_gui, no_sesman, L,
             names{"on_title_bar_only"},
             type_<bool>(),
             set(true));

    _.member(advanced_in_gui, no_sesman, L,
             names{"max_unrecog_char_rate"},
             type_<types::range<types::unsigned_, 0, 100>>(),
             set(40),
             desc{
                 "Expressed in percentage,\n"
                 "  0   - all of characters need be recognized\n"
                 "  100 - accept all results"
             });
});

_.section("video", [&]
{
    _.member(advanced_in_gui, no_sesman, L,
             names{"capture_flags"},
             type_<CaptureFlags>(),
             set(CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr));

    _.member(advanced_in_gui, no_sesman, L,
             names{"png_interval"},
             type_<std::chrono::duration<unsigned, std::deci>>(),
             set(10),
             desc{"Frame interval."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"break_interval"},
             type_<std::chrono::seconds>(),
             set(600),
             desc{"Time between 2 wrm movies."});

    _.member(advanced_in_gui, no_sesman, L,
             names{"png_limit"},
             type_<types::unsigned_>(),
             set(5),
             desc{"Number of png captures to keep."});

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"hash_path"},
             type_<types::dirpath>(),
             set(CPP_EXPR(app_path(AppPath::Hash))));

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"record_tmp_path"},
             type_<types::dirpath>(),
             set(CPP_EXPR(app_path(AppPath::RecordTmp))));

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"record_path"},
             type_<types::dirpath>(),
             set(CPP_EXPR(app_path(AppPath::Record))));

    _.member(advanced_in_gui, no_sesman, L,
             names{"disable_keyboard_log"},
             type_<KeyboardLogFlags>(),
             disable_prefix_val,
             set(KeyboardLogFlags::syslog),
             desc{
                "Disable keyboard log:\n"
                "(Please see also \"Keyboard input masking level\" in \"session_log\".)"
             });

    _.member(advanced_in_gui, no_sesman, L,
             names{"disable_clipboard_log"},
             type_<ClipboardLogFlags>(),
             set(ClipboardLogFlags::syslog),
             disable_prefix_val,
             desc{"Disable clipboard log:"});

    _.member(advanced_in_gui, no_sesman, L,
             names{"disable_file_system_log"},
             type_<FileSystemLogFlags>(),
             set(FileSystemLogFlags::syslog),
             disable_prefix_val,
             desc{"Disable (redirected) file system log:"});

    _.member(advanced_in_gui, no_sesman, L,
             names{"wrm_color_depth_selection_strategy"},
             type_<ColorDepthSelectionStrategy>(),
             set(ColorDepthSelectionStrategy::depth16));

    _.member(advanced_in_gui, no_sesman, L,
             names{"wrm_compression_algorithm"},
             type_<WrmCompressionAlgorithm>(),
             set(WrmCompressionAlgorithm::gzip));

    _.member(advanced_in_gui, no_sesman, L,
             names{"codec_id"},
             type_<std::string>(),
             set("mp4"));

    _.member(advanced_in_gui, no_sesman, L,
             names{
                .all="framerate",
                .display="Frame rate"
             },
             type_<types::range<types::unsigned_, 1, 120>>(),
             set(5));

    _.member(advanced_in_gui, no_sesman, L,
             names{"ffmpeg_options"},
             type_<std::string>(),
             set("crf=35 preset=superfast"),
             desc{
                 "FFmpeg options for video codec. See https://trac.ffmpeg.org/wiki/Encode/H.264\n"
                 "/!\\ Some browsers and video decoders don't support crf=0"
             });

    _.member(advanced_in_gui, no_sesman, L,
             names{"notimestamp"},
             type_<bool>(),
             set(false),
             desc{"Remove the top left banner that adds the date of the video"});

    _.member(ini_and_gui, no_sesman, L,
             names{"smart_video_cropping"},
             type_<SmartVideoCropping>(),
             set(SmartVideoCropping::v2));

    // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
    _.member(advanced_in_gui, no_sesman, L,
             names{"play_video_with_corrupted_bitmap"},
             type_<bool>(),
             set(false),
             desc{"Needed to play a video with corrupted Bitmap Update."});

    _.member(ini_and_gui, no_sesman, L,
             names{"allow_rt_without_recording"},
             type_<bool>(),
             set(false),
             desc{"Allow real-time view (4 eyes) without session recording enabled in the authorization"});

    _.member(hidden_in_gui, no_sesman, L,
             names{"file_permissions"},
             type_<FilePermissions>(),
             set(0440),
             desc{"Allow to control permissions on recorded files with octal number"});
});

_.section("capture", [&]
{
    auto co_video = connpolicy::section{"video"};

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"record_filebase"},
             type_<std::string>(),
             desc{"basename without extension"});

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"record_subdirectory"},
             type_<std::string>(),
             desc{"subdirectory of record_path (video section)"});

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"fdx_path"},
             type_<std::string>());

    _.member(no_ini_no_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, co_video, L,
             names{"disable_keyboard_log"},
             type_<KeyboardLogFlagsCP>(),
             set(KeyboardLogFlagsCP::syslog),
             disable_prefix_val,
             desc{
                "Disable keyboard log:\n"
                "(Please see also \"Keyboard input masking level\" in \"session_log\" section of \"Connection Policy\".)"
             });
});

_.section("audit", [&]
{
    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"rt_display"},
             type_<bool>(),
             set(false));

    _.member(hidden_in_gui, no_sesman, L,
             names{"use_redis"},
             type_<bool>(),
             set(true));

    _.member(hidden_in_gui, no_sesman, L,
             names{"redis_timeout"},
             type_<std::chrono::milliseconds>(),
             set(500));

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_address"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_port"},
             type_<types::unsigned_>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_password"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_db"},
             type_<types::unsigned_>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_use_tls"},
             type_<bool>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_tls_cacert"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_tls_cert"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"redis_tls_key"},
             type_<std::string>());
});

_.section("crypto", [&]
{
    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, NL,
             names{"encryption_key"},
             type_<types::fixed_binary<32>>(),
             set(default_key));

    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, NL,
             names{"sign_key"},
             type_<types::fixed_binary<32>>(),
             set(default_key));
});

_.section("websocket", [&]
{
    _.member(hidden_in_gui, no_sesman, L,
             names{"enable_websocket"},
             type_<bool>(),
             set(false),
             desc{"Enable websocket protocol (ws or wss with use_tls=1)"});

    _.member(hidden_in_gui, no_sesman, L,
             names{"use_tls"},
             type_<bool>(),
             set(true),
             desc{"Use TLS with websocket (wss)"});

    _.member(hidden_in_gui, no_sesman, L,
             names{"listen_address"},
             type_<std::string>(),
             set(":3390"),
             desc{"${addr}:${port} or ${port} or ${unix_socket_path}"});
});

_.section("debug", [&]
{
    _.member(hidden_in_gui, no_sesman, L,
             names{"fake_target_ip"},
             type_<std::string>());

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"capture"},
             type_<types::u32>(),
             desc{CONFIG_DESC_CAPTURE});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"auth"},
             type_<types::u32>(),
             desc{CONFIG_DESC_AUTH});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"session"},
             type_<types::u32>(),
             desc{CONFIG_DESC_SESSION});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"front"},
             type_<types::u32>(),
             desc{CONFIG_DESC_FRONT});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"mod_rdp"},
             type_<types::u32>(),
             desc{CONFIG_DESC_RDP});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"mod_vnc"},
             type_<types::u32>(),
             desc{CONFIG_DESC_VNC});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"mod_internal"},
             type_<types::u32>(),
             desc{CONFIG_DESC_MOD_INTERNAL});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"sck_mod"},
             type_<types::u32>(),
             desc{CONFIG_DESC_SCK});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"sck_front"},
             type_<types::u32>(),
             desc{CONFIG_DESC_SCK});

    _.member(hidden_in_gui, no_sesman, L,
             names{"password"},
             type_<types::u32>());

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"compression"},
             type_<types::u32>(),
             desc{CONFIG_DESC_COMPRESSION});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"cache"},
             type_<types::u32>(),
             desc{CONFIG_DESC_CACHE});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"ocr"},
             type_<types::u32>(),
             desc{CONFIG_DESC_OCR});

    _.member(advanced_in_gui | hex_in_gui, no_sesman, L,
             names{"ffmpeg"},
             type_<types::u32>(),
             desc{"avlog level"});

    _.member(advanced_in_gui, no_sesman, L,
             names{"config"},
             type_<types::unsigned_>(),
             spec::type_<bool>(),
             set(2));

    _.member(hidden_in_gui, no_sesman, L,
             names{"mod_rdp_use_failure_simulation_socket_transport"},
             type_<ModRdpUseFailureSimulationSocketTransport>(),
             set(ModRdpUseFailureSimulationSocketTransport::Off));

    _.member(advanced_in_gui, no_sesman, L,
             names{"probe_client_addresses"},
             type_<types::list<std::string>>(),
             desc{"List of client probe IP addresses (ex: ip1,ip2,etc) to prevent some continuous logs"});
});

_.section("remote_program", [&]
{
    _.member(ini_and_gui, no_sesman, L,
             names{"allow_resize_hosted_desktop"},
             type_<bool>(),
             set(true));
});

_.section("translation", [&]
{
    _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"language"},
             type_<Language>{},
             spec::type_<std::string>(),
             set(Language::en));

    _.member(advanced_in_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"login_language"},
             type_<LoginLanguage>(),
             spec::type_<std::string>(),
             set(LoginLanguage::Auto));
});

_.section("internal_mod", [&]
{
    _.member(advanced_in_gui, no_sesman, L,
             names{"enable_target_field"},
             type_<bool>(),
             set(true),
             desc{"Enable target edit field in login page."});
});

_.section("context", [&]
{
    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"psid"},
             type_<std::string>(),
             desc{"Proxy session log id"});

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{
                .all="opt_bpp",
                .sesman="bpp"
             },
             type_<ColorDepth>(),
             set(ColorDepth::depth24));

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{
                .all="opt_height",
                .sesman="height"
             },
             type_<types::u16>(),
             set(600));

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{
                .all="opt_width",
                .sesman="width"
             },
             type_<types::u16>(),
             set(800));

    // auth_error_message is left as std::string type because SocketTransport and ReplayMod
    // take it as argument on constructor and modify it as a std::string
    _.member(no_ini_no_gui, no_sesman, L,
             names{"auth_error_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"selector"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L,
             names{"selector_current_page"},
             type_<types::unsigned_>(),
             set(1));

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"selector_device_filter"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"selector_group_filter"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"selector_proto_filter"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L,
             names{"selector_lines_per_page"},
             type_<types::unsigned_>(),
             set(0));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"selector_number_of_pages"},
             type_<types::unsigned_>(),
             set(1));

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, NL,
             names{"target_password"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L,
             names{"target_host"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"tunneling_target_host"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"target_str"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"target_service"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"target_port"},
             type_<types::unsigned_>(),
             set(3389));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{
                .all="target_protocol",
                .sesman="proto_dest"
             },
             type_<std::string>(),
             set("RDP"));

    _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, NL,
             names{"password"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"reporting"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, VNL,
             names{"auth_channel_answer"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"auth_channel_target"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"message"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"display_link"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"mod_timeout"},
             type_<std::chrono::seconds>(),
             set(0));

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"accept_message"},
             type_<bool>());
    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"display_message"},
             type_<bool>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"rejected"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"keepalive"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"session_id"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{
                .all="end_date_cnx",
                .sesman="timeclose"
             },
             type_<std::chrono::seconds>(),
             set(0));

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"real_target_device"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"authentication_challenge"},
             type_<bool>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"ticket"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"comment"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"duration"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"duration_max"},
             type_<std::chrono::minutes>(),
             set(0));

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"waitinforeturn"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"showform"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"formflag"},
             type_<types::unsigned_>(),
             set(0));

    _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L,
             names{"module"},
             type_<ModuleName>(),
             spec::type_<std::string>(),
             set(ModuleName::login));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"try_alternate_target"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"has_more_target"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             type_<std::string>(),
             names{"proxy_opt"});

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"pattern_kill"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"pattern_notify"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"opt_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"login_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"disconnect_reason"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"disconnect_reason_ack"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, no_sesman, L,
             names{"ip_target"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"recording_started"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L,
             names{"rt_ready"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"sharing_ready"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"auth_notify"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"auth_notify_rail_exec_flags"},
             type_<types::unsigned_>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"auth_notify_rail_exec_exe_or_file"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_exec_result"},
             type_<types::u16>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_flags"},
             type_<types::u16>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_original_exe_or_file"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_exe_or_file"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_working_dir"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_arguments"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"auth_command_rail_exec_account"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, NL,
             names{"auth_command_rail_exec_password"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"session_probe_launch_error_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, no_sesman, L,
             names{"close_box_extra_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"is_wabam"},
             type_<bool>(),
             set(false));


    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"pm_response"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"pm_request"},
             type_<std::string>());


    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"native_session_id"},
             type_<types::u32>());

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"rd_shadow_available"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L,
             names{"rd_shadow_userdata"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"rd_shadow_type"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"rd_shadow_invitation_error_code"},
             type_<types::u32>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"rd_shadow_invitation_error_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"rd_shadow_invitation_id"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"rd_shadow_invitation_addr"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"rd_shadow_invitation_port"},
             type_<types::u16>());

    _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L,
             names{"session_sharing_userdata"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"session_sharing_enable_control"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L,
             names{"session_sharing_ttl"},
             type_<std::chrono::seconds>(),
             set(60*10));

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"session_sharing_invitation_error_code"},
             type_<types::u32>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"session_sharing_invitation_error_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, NL,
             names{"session_sharing_invitation_id"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"session_sharing_invitation_addr"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"session_sharing_target_ip"},
             type_<std::string>());

    _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
             names{"session_sharing_target_login"},
             type_<std::string>());

    _.member(no_ini_no_gui, no_sesman, L,
             names{"rail_module_host_mod_is_active"},
             type_<bool>(),
             set(false));

    _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L,
             names{"smartcard_login"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"banner_message"},
             type_<std::string>());

    _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L,
             names{"banner_type"},
             type_<BannerType>());

    _.member(no_ini_no_gui, no_sesman, L,
             names{"redirection_password_or_cookie"},
             type_<std::vector<uint8_t>>());
});

_.section("theme", [&]
{
    _.member(ini_and_gui, no_sesman, L,
             names{"enable_theme"},
             type_<bool>(),
             set(false),
             desc{"Enable custom theme color configuration. Each theme color can be defined as HTML color code (white: #FFFFFF, black: #000000, blue: #0000FF, etc)"});

    _.member(image_in_gui, no_sesman, L,
             names{
                .all="logo_path",
                .ini="logo"
             },
             type_<std::string>(),
             set(CPP_EXPR(REDEMPTION_CONFIG_THEME_LOGO)),
             desc{"Logo displayed when theme is enabled"});

    auto to_rgb = [](NamedBGRColor color){
        return BGRColor(BGRasRGBColor(color)).as_u32();
    };

    _.member(ini_and_gui, no_sesman, L,
             names{"bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(DARK_BLUE_BIS)));

    _.member(ini_and_gui, no_sesman, L,
             names{"fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(WHITE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"separator_color"},
             type_<types::rgb>(),
             set(to_rgb(LIGHT_BLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"focus_color"},
             type_<types::rgb>(),
             set(to_rgb(WINBLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"error_color"},
             type_<types::rgb>(),
             set(to_rgb(YELLOW)));

    _.member(ini_and_gui, no_sesman, L,
             names{"edit_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(WHITE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"edit_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(BLACK)));

    _.member(ini_and_gui, no_sesman, L,
             names{"edit_focus_color"},
             type_<types::rgb>(),
             set(to_rgb(WINBLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"tooltip_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(BLACK)));

    _.member(ini_and_gui, no_sesman, L,
             names{"tooltip_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(LIGHT_YELLOW)));

    _.member(ini_and_gui, no_sesman, L,
             names{"tooltip_border_color"},
             type_<types::rgb>(),
             set(to_rgb(BLACK)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_line1_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(PALE_BLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_line1_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(BLACK)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_line2_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(LIGHT_BLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_line2_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(BLACK)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_selected_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(MEDIUM_BLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_selected_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(WHITE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_focus_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(WINBLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_focus_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(WHITE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_label_bgcolor"},
             type_<types::rgb>(),
             set(to_rgb(MEDIUM_BLUE)));

    _.member(ini_and_gui, no_sesman, L,
             names{"selector_label_fgcolor"},
             type_<types::rgb>(),
             set(to_rgb(WHITE)));
});

REDEMPTION_DIAGNOSTIC_POP()

_.build();

}

}
