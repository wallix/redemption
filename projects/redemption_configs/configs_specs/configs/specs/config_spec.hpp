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
            type_<std::chrono::milliseconds>(), names{"front_connection_time"},
            desc{"from incoming connection to \"up_and_running\" state"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L,
            type_<std::chrono::milliseconds>(), names{"target_connection_time"},
            desc{"from Module rdp creation to \"up_and_running\" state"});

        _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L, type_<std::string>(), names{.all="auth_user", .sesman="login"});
        _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L, type_<std::string>(), names{.all="host", .sesman="ip_client"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{.all="target", .sesman="ip_target"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"target_device"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"device_id"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"primary_user_id"});
        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L, type_<std::string>(), names{.all="target_user", .sesman="target_login"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"target_application"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"target_application_account"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, NL, type_<std::string>(), names{"target_application_password"});

        _.member(advanced_in_gui | iptables_in_gui | logged_in_gui, no_sesman, L, type_<types::unsigned_>(), names{"port"}, desc{"Warning: Service will be automatically restarted and active sessions will be disconnected\nThe port set in this field must not be already used, otherwise the service will not run.\nChanging the port number will prevent WALLIX Access Manager from working properly."}, set(3389));
        _.member(advanced_in_gui, no_sesman, L, type_<Level>(), spec::type_<std::string>(), names{"encryptionLevel"}, set(Level::low));
        _.member(advanced_in_gui, no_sesman, L, type_<std::string>(), names{"authfile"}, set(CPP_EXPR(REDEMPTION_CONFIG_AUTHFILE)));

        _.member(ini_and_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"handshake_timeout"}, desc{"Time out during RDP handshake stage."}, set(10));
        _.member(ini_and_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"base_inactivity_timeout"}, desc{"No automatic disconnection due to inactivity, timer is set on primary authentication.\nIf value is between 1 and 30, then 30 is used.\nIf value is set to 0, then inactivity timeout value is unlimited."}, set(900));
        _.member(hidden_in_gui, all_connpolicy, co_session,
            L, type_<std::chrono::seconds>(), names{"inactivity_timeout"}, set(0), desc{
                "No automatic disconnection due to inactivity, timer is set on target session.\n"
                "If value is between 1 and 30, then 30 is used.\n"
                "If value is set to 0, then value set in \"Base inactivity timeout\" (in \"RDP Proxy\" configuration option) is used."
            });
        _.member(hidden_in_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"keepalive_grace_delay"}, desc{"Internal keepalive between sesman and rdp proxy"}, set(30));
        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"authentication_timeout"}, desc{"Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate)."}, set(120));
        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"close_timeout"}, desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."}, set(600));

        _.member(advanced_in_gui, sesman_to_proxy, reset_back_to_selector, L, type_<TraceType>(), names{"trace_type"}, set(TraceType::localfile_hashed));

        _.member(advanced_in_gui, no_sesman, L, type_<types::ip_string>(), names{"listen_address"}, set("0.0.0.0"));
        _.member(iptables_in_gui, no_sesman, L, type_<bool>(), names{"enable_transparent_mode"}, desc{"Allow Transparent mode."}, set(false));
        _.member(advanced_in_gui | password_in_gui, no_sesman, L, type_<types::fixed_string<254>>(), names{"certificate_password"}, desc{"Proxy certificate password."}, set("inquisition"));

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<bool>(), names{"is_rec"}, set(false));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"enable_bitmap_update"}, desc{"Support of Bitmap Update."}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"enable_close_box"}, desc{"Show close screen."}, set(true));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"enable_osd"}, set(true));
        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"enable_osd_display_remote_target"}, set(true), desc{"Show target address with F12."});

        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"enable_wab_integration"}, set((CPP_EXPR(REDEMPTION_CONFIG_ENABLE_WAB_INTEGRATION))));

        // TODO move to [client]
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"allow_using_multiple_monitors"}, set(true));
        // TODO move to [client] / [mod_rdp]
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"allow_scale_factor"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"bogus_refresh_rect"}, desc{"Needed to refresh screen of Windows Server 2012."}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"large_pointer_support"}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"new_pointer_update_support"}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"unicode_keyboard_event_support"}, set(true));

        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::range<std::chrono::milliseconds, 100, 10000>>{}, names{"mod_recv_timeout"}, set(1000));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"experimental_enable_serializer_data_block_size_limit"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"experimental_support_resize_session_during_recording"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"support_connection_redirection_during_recording"}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<std::chrono::milliseconds>(), names{"rdp_keepalive_connection_interval"}, desc{
            "Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.\n"
            "!!!May cause FreeRDP-based client to CRASH!!!\n"
            "Set to 0 to disable this feature."
        }, set(0));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"enable_ipv6"}, desc{"Enable primary connection on ipv6.\nWarning: Service need to be manually restarted to take changes into account" }, set(true));

        _.member(hidden_in_gui, no_sesman, L, type_<types::u64>(), names{"minimal_memory_available_before_connection_silently_closed"}, desc{"In megabytes. 0 for disabled." }, set(100));
    });

    _.section("session_log", [&]
    {
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"enable_session_log"}, set(true));
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"enable_arcsight_log"}, set(false));
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<KeyboardInputMaskingLevel>(), names{"keyboard_input_masking_level"},
            desc{"Keyboard Input Masking Level:"}, set(KeyboardInputMaskingLevel::password_and_unidentified));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"hide_non_printable_kbd_input"}, set(false));
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

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::unsigned_>(), names{"keyboard_layout"}, set(0));
        _.member(advanced_in_gui, no_sesman, L, type_<types::list<std::string>>(), names{"keyboard_layout_proposals"}, desc{keyboard_layout_proposals_desc}, set("en-US, fr-FR, de-DE, ru-RU"));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"ignore_logon_password"}, desc{"If true, ignore password provided by RDP client, user need do login manually."}, set(false));

        _.member(hidden_in_gui, no_sesman, L, type_<types::u32>(), names{"performance_flags_default"}, desc{
            "Enable font smoothing (0x80)."
        }, set(0x80));
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"performance_flags_force_present"}, desc{
            "Disable wallpaper (0x1).\n"
            "Disable full-window drag (0x2).\n"
            "Disable menu animations (0x4).\n"
            "Disable theme (0x8).\n"
            "Disable mouse cursor shadows (0x20).\n"
            "Disable cursor blinking (0x40).\n"
            "Enable font smoothing (0x80).\n"
            "Enable Desktop Composition (0x100)."
        }, set(0x28));
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"performance_flags_force_not_present"}, desc{
            "See the comment of \"Performance flags force present\" above for available values."
        }, set(0));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"auto_adjust_performance_flags"}, desc{"If enabled, avoid automatically font smoothing in recorded session."}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"tls_fallback_legacy"}, desc{"Fallback to RDP Legacy Encryption if client does not support TLS."}, set(false));
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"tls_support"}, set(true));
        _.member(ini_and_gui, no_sesman, L, type_<types::u32>(), names{"tls_min_level"}, desc{"Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(2));
        _.member(ini_and_gui, no_sesman, L, type_<types::u32>(), names{"tls_max_level"}, desc{"Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(0));
        _.member(ini_and_gui, no_sesman, L, D, type_<bool>(), names{"show_common_cipher_list"}, desc{"Show common cipher list supported by client and server"}, set(false));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"enable_nla"},
                    desc{"Needed for primary NTLM or Kerberos connections over NLA."}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"bogus_neg_request"}, desc{"Needed to connect with jrdp, based on bogus X224 layer code."}, set(false));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"bogus_user_id"}, desc{"Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code."}, set(true));

        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"disable_tsk_switch_shortcuts"}, desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<RdpCompression>{}, names{"rdp_compression"}, set(RdpCompression::rdp6_1));

        _.member(advanced_in_gui, no_sesman, L, type_<ColorDepth>{}, names{"max_color_depth"}, set(ColorDepth::depth24));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"persistent_disk_bitmap_cache"}, desc{"Persistent Disk Bitmap Cache on the front side."}, set(true));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"cache_waiting_list"}, desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(false));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"persist_bitmap_cache_on_disk"}, desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"bitmap_compression"}, desc{"Support of Bitmap Compression."}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"fast_path"}, desc{"Enables support of Client Fast-Path Input Event PDUs."}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"enable_suppress_output"}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<std::string>(), names{"ssl_cipher_list"}, desc{
            "[Not configured]: Compatible with more RDP clients (less secure)\n"
            "HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)"
            "HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)"
        }, set("HIGH:!ADH:!3DES:!SHA"));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"show_target_user_in_f12_message"}, set(false));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"bogus_ios_glyph_support_level"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"transform_glyph_to_bitmap"}, set(false));

        _.member(ini_and_gui, no_sesman, L, type_<BogusNumberOfFastpathInputEvent>(), names{"bogus_number_of_fastpath_input_event"}, set(BogusNumberOfFastpathInputEvent::pause_key_only));

        _.member(advanced_in_gui, no_sesman, L, type_<types::range<std::chrono::milliseconds, 100, 10000>>{}, names{"recv_timeout"}, set(1000));

        _.member(ini_and_gui, no_sesman, L, type_<bool>{}, names{"enable_osd_4_eyes"}, desc{"Enables display of message informing user that his/her session is being audited."}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>{}, names{"enable_remotefx"}, desc{"Enable front remoteFx"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, D, type_<types::list<types::unsigned_>>(), names{"disabled_orders"}, desc{disabled_orders_desc}, set("25"));
    });

    _.section("all_target_mod", [&]
    {
        _.member(advanced_in_gui, no_sesman, L, type_<types::range<std::chrono::milliseconds, 1000, 10000>>{}, names{"connection_establishment_timeout"}, desc{
            "The maximum time in milliseconds that the proxy will wait while attempting to connect to an target."
        }, set(3000));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 3'600'000>>{}, names{"tcp_user_timeout"}, desc{
            "This parameter allows you to specify max timeout in milliseconds before a TCP connection is aborted. If the option value is specified as 0, TCP will use the system default."
        }, set(0));
    });

    _.section(names{.all="mod_rdp", .connpolicy="rdp"}, [&]
    {
        _.member(advanced_in_gui, no_sesman, L, type_<RdpCompression>{}, names{"rdp_compression"}, set(RdpCompression::rdp6_1));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"disconnect_on_logon_user_change"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"open_session_timeout"}, set(0));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"glyph_cache"}, set(false));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, D, type_<types::list<types::unsigned_>>(), names{"disabled_orders"}, desc{disabled_orders_desc});

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"enable_nla"}, desc{"NLA authentication in secondary target."}, set(true), jh_without_rdp_connpolicy.always(false));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"enable_kerberos"}, desc{
            "If enabled, NLA authentication will try Kerberos before NTLM.\n"
            "(if enable_nla is disabled, this value is ignored)."
        }, set(false));
        _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L, type_<types::u32>(), names{"tls_min_level"},
            desc{"Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(0));
        _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L, type_<types::u32>(), names{"tls_max_level"},
            desc{"Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(0));
        _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L, type_<std::string>(), names{"cipher_string"},
            desc{"TLSv1.2 additional ciphers supported by client, default is empty to apply system-wide configuration (SSL security level 2), ALL for support of all ciphers to ensure highest compatibility with target servers."}, set("ALL"));
        _.member(no_ini_no_gui, rdp_and_jh_connpolicy, L, D, type_<bool>(), names{"show_common_cipher_list"}, desc{"Show common cipher list supported by client and server"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"persistent_disk_bitmap_cache"}, desc{"Persistent Disk Bitmap Cache on the mod side."}, set(true));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"cache_waiting_list"}, desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"persist_bitmap_cache_on_disk"}, desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));

        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::list<std::string>>(), names{"allow_channels"}, desc{"List of enabled (static) virtual channel (example: channel1,channel2,etc). Character * only, activate all with low priority."}, set("*"));
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::list<std::string>>(), names{"deny_channels"}, desc{"List of disabled (static) virtual channel (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});

        _.member(no_ini_no_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<std::string>(), names{"allowed_dynamic_channels"}, desc{"List of enabled dynamic virtual channel (example: channel1,channel2,etc). Character * only, activate all."}, set("*"));
        _.member(no_ini_no_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<std::string>(), names{"denied_dynamic_channels"}, desc{"List of disabled dynamic virtual channel (example: channel1,channel2,etc). Character * only, deactivate all."});

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"fast_path"}, desc{"Enables support of Client/Server Fast-Path Input/Update PDUs.\nFast-Path is required for Windows Server 2012 (or more recent)!"}, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{.all="server_redirection_support", .connpolicy="server_redirection"}, desc{"Enables Server Redirection Support."}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<ClientAddressSent>(), names{"client_address_sent"}, desc { "Client Address to send to target (in InfoPacket)" }, set(ClientAddressSent::no_address));

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<std::string>(), names{"load_balance_info"}, desc{"Load balancing information"});

        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{.all="bogus_sc_net_size", .sesman="rdp_bogus_sc_net_size"}, desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."}, set(true));

        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::list<std::string>>(), names{"proxy_managed_drives"});

        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"ignore_auth_channel"}, set(false));
        _.member(ini_and_gui, no_sesman, L, type_<types::fixed_string<7>>(), names{"auth_channel"}, set("*"), desc{"Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel."});
        _.member(ini_and_gui, no_sesman, L, type_<types::fixed_string<7>>(), names{"checkout_channel"}, desc{"Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel."});

        _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"alternate_shell"});
        _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"shell_arguments"});
        _.member(hidden_in_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"shell_working_directory"});

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"use_client_provided_alternate_shell"}, desc{"As far as possible, use client-provided initial program (Alternate Shell)"}, set(false));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"use_client_provided_remoteapp"}, desc{"As far as possible, use client-provided remote program (RemoteApp)"}, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"use_native_remoteapp_capability"}, desc{"As far as possible, use native RemoteApp capability"}, set(true));

        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<256>>(), names{"application_driver_exe_or_file"}, set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_EXE_OR_FILE)));
        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<256>>(), names{"application_driver_script_argument"}, set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_SCRIPT_ARGUMENT)));
        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<256>>(), names{"application_driver_chrome_dt_script"}, set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_CHROME_DT_SCRIPT)));
        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<256>>(), names{"application_driver_chrome_uia_script"}, set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_CHROME_UIA_SCRIPT)));
        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<256>>(), names{"application_driver_firefox_uia_script"}, set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_FIREFOX_UIA_SCRIPT)));
        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<256>>(), names{"application_driver_ie_script"}, set(CPP_EXPR(REDEMPTION_CONFIG_APPLICATION_DRIVER_IE_SCRIPT)));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"hide_client_name"}, desc{
            "Do not transmit client machine name to RDP server.\n"
            "If Per-Device licensing mode is configured on the RD host, this Bastion will consume a CAL for all of these connections to the RD host."
        }, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"use_license_store"}, desc{"Stores CALs issued by the terminal servers."}, set(true));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"bogus_ios_rdpdr_virtual_channel"}, set(true));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"enable_rdpdr_data_analysis"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::milliseconds>(), names{"remoteapp_bypass_legal_notice_delay"}, desc{
            "Delay in milliseconds before automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
            "Set to 0 to disable this feature."
        }, set(0));
        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::milliseconds>(), names{"remoteapp_bypass_legal_notice_timeout"}, desc{
            "Time limit in milliseconds to automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
            "Set to 0 to disable this feature."
        }, set(20000));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"log_only_relevant_clipboard_activities"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"experimental_fix_too_long_cookie"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"split_domain"}, desc{
            "Force to split target domain and username with '@' separator."
        }, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{.all="wabam_uses_translated_remoteapp", .display="Enable translated RemoteAPP with AM"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"session_shadowing_support"}, desc{"Enables Session Shadowing Support."}, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"enable_remotefx"}, desc{"Enables support of the remoteFX codec."}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"accept_monitor_layout_change_if_capture_is_not_started"}, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(),
                 names{"enable_restricted_admin_mode"},
                 desc{"Connect to the server in Restricted Admin mode.\n"
                         "This mode must be supported by the server "
                         "(available from Windows Server 2012 R2), "
                         "otherwise, connection will fail.\n"
                         "NLA must be enabled."},
                 set(false));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(),
                 names{"force_smartcard_authentication"},
                 desc{"NLA will be disabled.\n"
                         "Target must be set for interactive login, otherwise server connection may not be guaranteed.\n"
                         "Smartcard device must be available on client desktop.\n"
                         "Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service."},
                 set(false));
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"enable_ipv6"}, desc { "Enable target connection on ipv6" }, set(true));

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<RdpModeConsole>(), spec::type_<std::string>(), names{.all="mode_console", .display="Console mode"}, set(RdpModeConsole::allow), desc{"Console mode management for targets on Windows Server 2003 (requested with /console or /admin mstsc option)"});

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"auto_reconnection_on_losing_target_link"}, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, W, type_<bool>(),
                 names{"allow_session_reconnection_by_shortcut"},
                 desc{"If the feature is enabled, the end user can trigger a session disconnection/reconnection with the shortcut Ctrl+F12.\n"
                      "This feature should not be used together with the End disconnected session option (section session_probe).\n"
                      "The keyboard shortcut is fixed and cannot be changed."},
                 set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 15000>>(),
                 names{"session_reconnection_delay"},
                 desc{"The delay in milliseconds between a session disconnection and the automatic reconnection that follows."},
                 set(0));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"forward_client_build_number"},
            desc{
                "Forward the build number advertised by the client to the server. "
                "If forwarding is disabled a default (static) build number will be sent to the server."
            },
            set(true)
        );

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"bogus_monitor_layout_treatment"},
            desc{
                "To resolve the session freeze issue with Windows 7/Windows Server 2008 target."
            },
            set(false));

        _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::string>(), names{"krb_armoring_account"},
            desc{
                "Account to be used for armoring Kerberos tickets. "
                "Must be in the form 'account_name@domain_name[@device_name]'. "
                "If account resolution succeeds the username and password associated with this account will be used; "
                "otherwise the below fallback username and password will be used instead."
            }
        );
        _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::string>(), names{"krb_armoring_realm"},
            desc{
                "Realm to be used for armoring Kerberos tickets. "
            }
        );
        _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::string>(), names{"krb_armoring_fallback_user"},
            desc{
                "Fallback username to be used for armoring Kerberos tickets. "
            }
        );
        _.member(external, rdp_without_jh_connpolicy | advanced_in_connpolicy, NL, type_<std::string>(), names{"krb_armoring_fallback_password"},
            desc{
                "Fallback password to be used for armoring Kerberos tickets."
            }
        );
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"effective_krb_armoring_user"},
            desc{
                "Effective username to be used for armoring Kerberos tickets."
            }
        );
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, NL, type_<std::string>(), names{"effective_krb_armoring_password"},
            desc{
                "Effective password to be used for armoring Kerberos tickets."
            }
        );

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 3000, 120000>>(),
                 names{"remote_programs_disconnect_message_delay"},
                 desc{"Delay in milliseconds before showing disconnect message after the last RemoteApp window is closed."}, set(3000));

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"use_session_probe_to_launch_remote_program"}, desc{"Use Session Probe to launch Remote Program as much as possible."}, set(true));

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, W, type_<bool>(), names{"replace_null_pointer_by_default_pointer"},
            desc{"Replace an empty mouse pointer with normal pointer."}, set(false));
    });

    _.section("protocol", [&]
    {
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<RdpSaveSessionInfoPDU>(), names{"save_session_info_pdu"}, set(RdpSaveSessionInfoPDU::UnsupportedOrUnknown));
    });

    _.section("session_probe", [&]
    {
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"enable_session_probe"}, set(false), rdp_without_jh_connpolicy.set(true) | jh_without_rdp_connpolicy.always(false));

        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<511>>(), names{"exe_or_file"}, set("||CMD"));
        _.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<511>>(), names{"arguments"}, set(CPP_EXPR(REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS)));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(),
            names{"use_smart_launcher"},
            desc{
                "Minimum supported server : Windows Server 2008.\n"
                "Clipboard redirection should be remain enabled on Terminal Server."},
            set(true));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"enable_launch_mask"}, set(true));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<SessionProbeOnLaunchFailure>(), names{"on_launch_failure"}, set(SessionProbeOnLaunchFailure::disconnect_user));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 300000>>(), names{"launch_timeout"}, desc{
            "This parameter in milliseconds is used if on_launch_failure is 1 (disconnect user).\n"
            "0 to disable timeout."
        }, set(40000));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 300000>>(), names{"launch_fallback_timeout"}, desc{
            "This parameter in milliseconds is used if on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).\n"
            "0 to disable timeout."
        }, set(40000));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"start_launch_timeout_timer_only_after_logon"}, desc{
            "Minimum supported server : Windows Server 2008."
        }, set(true));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 60000>>(), names{"keepalive_timeout"}, set(5000));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<SessionProbeOnKeepaliveTimeout>(), names{"on_keepalive_timeout"}, set(SessionProbeOnKeepaliveTimeout::disconnect_user));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"end_disconnected_session"}, desc{
            "Automatically end a disconnected Desktop session or clean up a disconnected RemoteApp session.\n"
            "This option is recommended for Web applications running in Desktop mode.\n"
            "Session Probe must be enabled to use this feature."
        }, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"enable_autodeployed_appdriver_affinity"}, desc{
            "End automatically a disconnected auto-deployed Application Driver session.\n"
        }, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"enable_log"}, set(false));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"enable_log_rotation"}, set(false));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, D, type_<SessionProbeLogLevel>(), names{"log_level"}, set(SessionProbeLogLevel::Debug));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(), names{"disconnected_application_limit"}, desc{
            "(Deprecated!) This policy setting allows you to configure a time limit in milliseconds for disconnected application sessions.\n"
            "0 to disable timeout."
        }, set(0));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(), names{"disconnected_session_limit"}, desc{
            "This policy setting allows you to configure a time limit in milliseconds for disconnected Terminal Services sessions.\n"
            "0 to disable timeout."
        }, set(0));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(), names{"idle_session_limit"}, desc{
            "This parameter allows you to specify the maximum amount of time in milliseconds that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.\n"
            "0 to disable timeout."
        }, set(0));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::chrono::milliseconds>(), names{"smart_launcher_clipboard_initialization_delay"}, set(2000));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::chrono::milliseconds>(), names{"smart_launcher_start_delay"}, set(0));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::chrono::milliseconds>(), names{"smart_launcher_long_delay"}, set(500));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<std::chrono::milliseconds>(), names{"smart_launcher_short_delay"}, set(50));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{.all="smart_launcher_enable_wabam_affinity", .display="Enable Smart launcher with AM affinity"}, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 300000>>(), names{"launcher_abort_delay"}, set(2000));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, D, type_<bool>(), names{"enable_crash_dump"}, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<types::u32, 0, 1000>>(), names{"handle_usage_limit"}, set(0));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<types::u32, 0, 200'000'000>>(), names{"memory_usage_limit"}, set(0));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<types::u32, 0, 10000>>(), names{"cpu_usage_alarm_threshold"}, desc{
            "As a percentage, the effective alarm threshold is calculated in relation to the reference consumption determined at the start of the program.\n"
            "The alarm is deactivated if this value is less than 200."
        }, set(0));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<SessionProbeCPUUsageAlarmAction>(), names{"cpu_usage_alarm_action"}, set(SessionProbeCPUUsageAlarmAction::Restart));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 0, 60000>>(), names{"end_of_session_check_delay_time"}, set(0));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"ignore_ui_less_processes_during_end_of_session_check"}, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"childless_window_as_unidentified_input_field"}, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"update_disabled_features"}, set(true));
        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<SessionProbeDisabledFeature>(), names{"disabled_features"}, set(SessionProbeDisabledFeature::chrome_inspection | SessionProbeDisabledFeature::firefox_inspection | SessionProbeDisabledFeature::group_membership));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"enable_bestsafe_interaction"}, set(false));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<SessionProbeOnAccountManipulation>(), names{"on_account_manipulation"}, set(SessionProbeOnAccountManipulation::allow));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L,
            type_<types::fixed_string<3>>(),
            names{"alternate_directory_environment_variable"},
            desc{
                "The name of the environment variable pointing to the alternative directory to launch Session Probe.\n"
                "If empty, the environment variable TMP will be used."});

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"public_session"}, desc{"If enabled, disconnected session can be recovered by a different primary user."}, set(false));

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<std::string>(), names{"outbound_connection_monitoring_rules"}, desc{
            "Comma-separated rules\n"
            "(Ex. IPv4 addresses: $deny:192.168.0.0/24:5900,$allow:192.168.0.110:21)\n"
            "(Ex. IPv6 addresses: $deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389,$allow:[20D1:0:3238:DFE1:63::FEFB]:21)\n"
            "(Ex. hostname can be used to resolve to both IPv4 and IPv6 addresses: $allow:host.domain.net:3389)\n"
            "(Ex. for backwards compatibility only: 10.1.0.0/16:22)\n"
            "Session Probe must be enabled to use this feature."
        });
        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<std::string>(), names{"process_monitoring_rules"}, desc{
            "Comma-separated rules (Ex.: $deny:Taskmgr)\n"
            "@ = All child processes of Bastion Application (Ex.: $deny:@)\n"
            "Session Probe must be enabled to use this feature."
        });
        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<std::string>(), names{"extra_system_processes"}, desc{"Comma-separated extra system processes (Ex.: dllhos.exe,TSTheme.exe)"});

        _.member(no_ini_no_gui, rdp_without_jh_connpolicy, L, type_<std::string>(), names{"windows_of_these_applications_as_unidentified_input_field"}, desc{"Comma-separated processes (Ex.: chrome.exe,ngf.exe)"});

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"customize_executable_name"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"allow_multiple_handshake"}, set(false));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"at_end_of_session_freeze_connection_and_wait"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"enable_cleaner"}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"clipboard_based_launcher_reset_keyboard_status"}, set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<SessionProbeProcessCommandLineRetrieveMethod>(), names{"process_command_line_retrieve_method"}, set(SessionProbeProcessCommandLineRetrieveMethod::both));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<types::range<std::chrono::milliseconds, 300, 2000>>(), names{"periodic_task_run_interval"}, set(500));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"pause_if_session_is_disconnected"}, set(false));
    });

    _.section(names{"server_cert"}, [&]
    {
        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<bool>(), names{"server_cert_store"},
            desc{"Keep known server certificates on Bastion"},
            jh_without_rdp_connpolicy.always(false),
            set(true));

        _.member(hidden_in_gui, rdp_without_jh_connpolicy, L, type_<ServerCertCheck>(), names{"server_cert_check"},
            jh_without_rdp_connpolicy.always(ServerCertCheck::always_succeed),
            set(ServerCertCheck::fails_if_no_match_and_succeed_if_no_know));

        struct P { char const * name; char const * desc; };
        for (P p : {
            P{"server_access_allowed_message", "Warn if check allow connexion to server."},
            P{"server_cert_create_message", "Warn that new server certificate file was created."},
            P{"server_cert_success_message", "Warn that server certificate file was successfully checked."},
            P{"server_cert_failure_message", "Warn that server certificate file checking failed."},
        }) {
            _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<ServerNotification>(), names{p.name}, desc{p.desc}, set(ServerNotification::syslog));
        }
        _.member(hidden_in_gui, no_sesman, L, type_<ServerNotification>(), names{"error_message"}, desc{"Warn that server certificate check raised some internal error."}, set(ServerNotification::syslog));

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"enable_external_validation"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"external_cert"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"external_response"}, desc{"empty string for wait, 'Ok' or error message"});
    });

    _.section(names{.all="mod_vnc", .connpolicy="vnc"}, [&]
    {
        _.member(ini_and_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"clipboard_up"}, desc{"Enable or disable the clipboard from client (client to server)."});
        _.member(ini_and_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"clipboard_down"}, desc{"Enable or disable the clipboard from server (server to client)."});

        _.member(advanced_in_gui, no_sesman, L, type_<types::list<types::int_>>(), names{"encodings"}, desc{
            "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
            "  0: Raw\n"
            "  1: CopyRect\n"
            "  2: RRE\n"
            "  16: ZRLE\n"
            "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
        });

        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<ClipboardEncodingType>(), spec::type_<std::string>(),
            names{.all="server_clipboard_encoding_type", .sesman="vnc_server_clipboard_encoding_type"},
            desc{"VNC server clipboard data encoding type."},
            set(ClipboardEncodingType::latin1));

        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<VncBogusClipboardInfiniteLoop>(), names{.all="bogus_clipboard_infinite_loop", .sesman="vnc_bogus_clipboard_infinite_loop"}, set(VncBogusClipboardInfiniteLoop::delayed));

        _.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), names{"server_is_macos"}, set(false));
        _.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), names{"server_unix_alt"}, set(false));

        _.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), names{"support_cursor_pseudo_encoding"}, set(true));

        _.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), names{"enable_ipv6"}, desc { "Enable target connection on ipv6" }, set(true));
    });

    _.section(names{"vnc_over_ssh"}, [&]
    {
        _.member(external, vnc_connpolicy, L, type_<bool>(), names{"enable"}, set(false));
        _.member(external, vnc_connpolicy, L, type_<types::unsigned_>(), names{"ssh_port"}, set(22));

        _.member(external, vnc_connpolicy, L, type_<VncTunnelingCredentialSource>(), spec::type_<std::string>(), names{"tunneling_credential_source"}, set(VncTunnelingCredentialSource::scenario_account), desc{
            "static_login: Static values provided in \"Ssh login\" & \"Ssh password\" fields will be used to establish the SSH tunnel.\n"
            "scenario_account: Scenario account provided in \"Scenario account name\" field will be used to establish the SSH tunnel. (Recommended)"
        });

        _.member(external, vnc_connpolicy, L, type_<std::string>(), names{"ssh_login"}, desc{"Login to be used for SSH tunneling."});
        _.member(external, vnc_connpolicy, NL, type_<std::string>(), names{"ssh_password"}, desc{"Password to be used for SSH tunneling."});

        _.member(external, vnc_connpolicy, L, type_<std::string>(), names{"scenario_account_name"}, desc{
            "With the following syntax: \"account_name@domain_name[@[device_name]]\".\n"
            "\n"
            "Syntax for using global domain scenario account:\n"
            "  \"account_name@global_domain_name\"\n"
            "\n"
            "Syntax for using local domain scenario account (with automatic device name deduction):\n"
            "  \"account_name@local_domain_name@\""
        });

        _.member(external, vnc_connpolicy | advanced_in_connpolicy, L, type_<VncTunnelingType>(), spec::type_<std::string>(), names{"tunneling_type"}, set(VncTunnelingType::pxssh), desc{"Only for debugging purposes."});
    });

    _.section("file_verification", [&]
    {
        _.member(hidden_in_gui, no_sesman, L, type_<std::string>(), names{"socket_path"}, set(CPP_EXPR(REDEMPTION_CONFIG_VALIDATOR_PATH)));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"enable_up"}, desc{"Enable use of ICAP service for file verification on upload."});
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"enable_down"}, desc{"Enable use of ICAP service for file verification on download."});
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"clipboard_text_up"}, desc{"Verify text data via clipboard from client to server.\nFile verification on upload must be enabled via option Enable up."});
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"clipboard_text_down"}, desc{"Verify text data via clipboard from server to client\nFile verification on download must be enabled via option Enable down."});

        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"block_invalid_file_up"}, desc{"Block file transfer from client to server on invalid file verification.\nFile verification on upload must be enabled via option Enable up."}, set(false));
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<bool>(), names{"block_invalid_file_down"}, desc{"Block file transfer from server to client on invalid file verification.\nFile verification on download must be enabled via option Enable down."}, set(false));

        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"block_invalid_clipboard_text_up"}, desc{"Block text transfer from client to server on invalid text verification.\nText verification on upload must be enabled via option Clipboard text up."}, set(false));
        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"block_invalid_clipboard_text_down"}, desc{"Block text transfer from server to client on invalid text verification.\nText verification on download must be enabled via option Clipboard text down."}, set(false));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<bool>(), names{"log_if_accepted"}, set(true));

        _.member(hidden_in_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, L, type_<types::u32>(), names{"max_file_size_rejected"}, desc{"If option Block invalid file (up or down) is enabled, automatically reject file with greater filesize (in megabytes).\nWarning: This value affects the RAM used by the session."}, set(256));

        _.member(hidden_in_gui, no_sesman, L, type_<types::dirpath>(), names{"tmpdir"}, desc{"Temporary path used when files take up too much memory."}, set("/tmp/"));
    });

    _.section("file_storage", [&]
    {
        _.member(hidden_in_gui, rdp_and_jh_connpolicy, L, type_<RdpStoreFile>(), spec::type_<std::string>(), names{"store_file"}, set(RdpStoreFile::never), desc{"Enable storage of transferred files (via RDP Clipboard)."});
    });

    // for validator only
    for (char const* section_name : {"icap_server_down", "icap_server_up"}) {
        // TODO temporary
        // please, update $REDEMPTION/tools/c++-analyzer/lua-checker/checkers/config.lua for each changement of value
        _.section(section_name, [&]
        {
            _.member(external | ini_and_gui, no_sesman, L, type_<std::string>(), names{"host"},
                desc{"Ip or fqdn of ICAP server"});
            _.member(external | ini_and_gui, no_sesman, L, type_<types::unsigned_>(), names{"port"},
                desc{"Port of ICAP server"}, set(1344));
            _.member(external | ini_and_gui, no_sesman, L, type_<std::string>(), names{"service_name"},
                desc{"Service name on ICAP server"}, set("avscan"));

            _.member(external | ini_and_gui, no_sesman, L, type_<bool>(), names{"tls"},
                desc{"ICAP server uses tls"});
            _.member(external | advanced_in_gui, no_sesman, L, type_<bool>(), names{"enable_x_context"},
                desc{"Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server"},
                set(true));
            _.member(external | advanced_in_gui, no_sesman, L, type_<bool>(), names{"filename_percent_encoding"},
                desc{"Filename sent to ICAP as percent encoding"}, set(false));
        });
    }

    _.section("mod_replay", [&]
    {
        _.member(advanced_in_gui, sesman_to_proxy, no_reset_back_to_selector, L,
            type_<types::dirpath>(), names{"replay_path"}, set("/tmp/"));
        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"on_end_of_data"}, desc{"0 - Wait for Escape, 1 - End session"}, set(false));
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"replay_on_loop"}, desc{"0 - replay once, 1 - loop replay"}, set(false));
    });

    _.section("ocr", [&]
    {
        _.member(ini_and_gui, no_sesman, L, type_<OcrVersion>(), names{"version"}, set(OcrVersion::v2));
        _.member(ini_and_gui, no_sesman, L, type_<OcrLocale>(), spec::type_<std::string>(), names{"locale"}, set(OcrLocale::latin));
        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::duration<unsigned, std::centi>>(), names{"interval"}, set(100));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"on_title_bar_only"}, set(true));
        _.member(advanced_in_gui, no_sesman, L, type_<types::range<types::unsigned_, 0, 100>>{}, names{"max_unrecog_char_rate"}, desc{
            "Expressed in percentage,\n"
          "  0   - all of characters need be recognized\n"
          "  100 - accept all results"
        }, set(40));
    });

    _.section("video", [&]
    {
        _.member(advanced_in_gui, no_sesman, L, type_<CaptureFlags>{}, names{"capture_flags"}, set(CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr));

        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::duration<unsigned, std::deci>>(), names{"png_interval"}, desc{"Frame interval."}, set(10));
        _.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), names{"break_interval"}, desc{"Time between 2 wrm movies."}, set(600));
        _.member(advanced_in_gui, no_sesman, L, type_<types::unsigned_>(), names{"png_limit"}, desc{"Number of png captures to keep."}, set(5));

        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::dirpath>(), names{"hash_path"}, set(CPP_EXPR(app_path(AppPath::Hash))));
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::dirpath>(), names{"record_tmp_path"}, set(CPP_EXPR(app_path(AppPath::RecordTmp))));
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::dirpath>(), names{"record_path"}, set(CPP_EXPR(app_path(AppPath::Record))));

        _.member(advanced_in_gui, no_sesman, L, type_<KeyboardLogFlags>{}, names{"disable_keyboard_log"}, desc{
            "Disable keyboard log:\n"
            "(Please see also \"Keyboard input masking level\" in \"session_log\".)"
        }, disable_prefix_val, set(KeyboardLogFlags::syslog));

        _.member(advanced_in_gui, no_sesman, L, type_<ClipboardLogFlags>(), names{"disable_clipboard_log"}, desc{"Disable clipboard log:"}, disable_prefix_val, set(ClipboardLogFlags::syslog));

        _.member(advanced_in_gui, no_sesman, L, type_<FileSystemLogFlags>(), names{"disable_file_system_log"}, desc{"Disable (redirected) file system log:"}, disable_prefix_val, set(FileSystemLogFlags::syslog));

        _.member(advanced_in_gui, no_sesman, L, type_<ColorDepthSelectionStrategy>{}, names{"wrm_color_depth_selection_strategy"}, set(ColorDepthSelectionStrategy::depth16));
        _.member(advanced_in_gui, no_sesman, L, type_<WrmCompressionAlgorithm>{}, names{"wrm_compression_algorithm"}, set(WrmCompressionAlgorithm::gzip));

        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{.all="bogus_vlc_frame_rate", .display="Bogus VLC frame rate"}, desc{"Needed to play a video with old ffplay or VLC v1.\nNote: Useless with mpv, MPlayer or VLC v2."}, set(true));

        _.member(advanced_in_gui, no_sesman, L, type_<std::string>(), names{"codec_id"}, set("mp4"));
        _.member(advanced_in_gui, no_sesman, L, type_<types::range<types::unsigned_, 1, 120>>(), names{.all="framerate", .display="Frame rate"}, set(5));
        _.member(advanced_in_gui, no_sesman, L, type_<std::string>(), names{"ffmpeg_options"}, desc{
            "FFmpeg options for video codec. See https://trac.ffmpeg.org/wiki/Encode/H.264\n"
            "/!\\ Some browsers and video decoders don't support crf=0"
        }, set("crf=35 preset=superfast"));
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"notimestamp"}, set(false), desc{"Remove the top left banner that adds the date of the video"});

        _.member(ini_and_gui, no_sesman, L, type_<SmartVideoCropping>(), names{"smart_video_cropping"}, set(SmartVideoCropping::v2));

        // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"play_video_with_corrupted_bitmap"}, desc{"Needed to play a video with corrupted Bitmap Update."}, set(false));

        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"allow_rt_without_recording"}, desc { "Allow real-time view (4 eyes) without session recording enabled in the authorization" }, set(false));

        _.member(hidden_in_gui, no_sesman, L, type_<FilePermissions>(), names{"file_permissions"}, desc { "Allow to control permissions on recorded files with octal number" }, set(0440));
    });

    _.section("capture", [&]
    {
        auto co_video = connpolicy::section{"video"};

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"record_filebase"}, desc{"basename without extension"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"record_subdirectory"}, desc{"subdirectory of record_path (video section)"});

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"fdx_path"});

        _.member(no_ini_no_gui, rdp_and_jh_connpolicy | advanced_in_connpolicy, co_video, L, type_<KeyboardLogFlagsCP>{}, names{"disable_keyboard_log"}, desc{
            "Disable keyboard log:\n"
            "(Please see also \"Keyboard input masking level\" in \"session_log\" section of \"Connection Policy\".)"
        }, disable_prefix_val, set(KeyboardLogFlagsCP::syslog));
    });

    _.section("audit", [&]
    {
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<bool>(), names{"rt_display"}, set(false));

        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"use_redis"}, set(true));
        _.member(hidden_in_gui, no_sesman, L, type_<std::chrono::milliseconds>(), names{"redis_timeout"}, set(500));

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"redis_address"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::unsigned_>(), names{"redis_port"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"redis_password"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::unsigned_>(), names{"redis_db"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"redis_use_tls"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"redis_tls_cacert"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"redis_tls_cert"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"redis_tls_key"});
    });

    _.section("crypto", [&]
    {
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, NL, type_<types::fixed_binary<32>>(), names{"encryption_key"}, set(default_key));
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, NL, type_<types::fixed_binary<32>>(), names{"sign_key"}, set(default_key));
    });

    _.section("websocket", [&]
    {
        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"enable_websocket"}, set(false), desc{"Enable websocket protocol (ws or wss with use_tls=1)"});
        _.member(hidden_in_gui, no_sesman, L, type_<bool>(), names{"use_tls"}, set(true), desc{"Use TLS with websocket (wss)"});
        _.member(hidden_in_gui, no_sesman, L, type_<std::string>(), names{"listen_address"}, desc{"${addr}:${port} or ${port} or ${unix_socket_path}"}, set(":3390"));
    });

    _.section("debug", [&]
    {
        _.member(hidden_in_gui, no_sesman, L, type_<std::string>(), names{"fake_target_ip"});

        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"capture"},
            desc{CONFIG_DESC_CAPTURE});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"auth"},
            desc{CONFIG_DESC_AUTH});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"session"},
            desc{CONFIG_DESC_SESSION});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"front"},
            desc{CONFIG_DESC_FRONT});

        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"mod_rdp"},
            desc{CONFIG_DESC_RDP});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"mod_vnc"},
            desc{CONFIG_DESC_VNC});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"mod_internal"},
            desc{CONFIG_DESC_MOD_INTERNAL});

        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"sck_mod"},
            desc{CONFIG_DESC_SCK});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"sck_front"},
            desc{CONFIG_DESC_SCK});

        _.member(hidden_in_gui, no_sesman, L, type_<types::u32>(), names{"password"});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"compression"},
            desc{CONFIG_DESC_COMPRESSION});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"cache"},
            desc{CONFIG_DESC_CACHE});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"ocr"},
            desc{CONFIG_DESC_OCR});
        _.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), names{"ffmpeg"},
            desc{"avlog level"});

        _.member(advanced_in_gui, no_sesman, L, type_<types::unsigned_>(), spec::type_<bool>(), names{"config"}, set(2));

        _.member(hidden_in_gui, no_sesman, L, type_<ModRdpUseFailureSimulationSocketTransport>(), names{"mod_rdp_use_failure_simulation_socket_transport"}, set(ModRdpUseFailureSimulationSocketTransport::Off));

        _.member(advanced_in_gui, no_sesman, L, type_<types::list<std::string>>(), names{"probe_client_addresses"}, desc{"List of client probe IP addresses (ex: ip1,ip2,etc) to prevent some continuous logs"});
    });

    _.section("remote_program", [&]
    {
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"allow_resize_hosted_desktop"}, set(true));
    });

    _.section("translation", [&]
    {
        _.member(hidden_in_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<Language>{}, spec::type_<std::string>(), names{"language"}, set(Language::en));

        _.member(advanced_in_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<LoginLanguage>(), spec::type_<std::string>(), names{"login_language"}, set(LoginLanguage::Auto));
    });

    _.section("internal_mod", [&]
    {
        _.member(advanced_in_gui, no_sesman, L, type_<bool>(), names{"enable_target_field"},
                 desc{"Enable target edit field in login page."}, set(true));
    });

    _.section("context", [&]
    {
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"psid"}, desc{"Proxy session log id"});

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<ColorDepth>(), names{.all="opt_bpp", .sesman="bpp"}, set(ColorDepth::depth24));
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::u16>(), names{.all="opt_height", .sesman="height"}, set(600));
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::u16>(), names{.all="opt_width", .sesman="width"}, set(800));

        // auth_error_message is left as std::string type because SocketTransport and ReplayMod
        // take it as argument on constructor and modify it as a std::string
        _.member(no_ini_no_gui, no_sesman, L, type_<std::string>(), names{"auth_error_message"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"selector"}, set(false));
        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L, type_<types::unsigned_>(), names{"selector_current_page"}, set(1));
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"selector_device_filter"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"selector_group_filter"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"selector_proto_filter"});
        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L, type_<types::unsigned_>(), names{"selector_lines_per_page"}, set(0));
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<types::unsigned_>(), names{"selector_number_of_pages"}, set(1));

        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, NL, type_<std::string>(), names{"target_password"});
        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L, type_<std::string>(), names{"target_host"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"tunneling_target_host"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"target_str"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"target_service"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<types::unsigned_>(), names{"target_port"}, set(3389));
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{.all="target_protocol", .sesman="proto_dest"}, set("RDP"));

        _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, NL, type_<std::string>(), names{"password"});

        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"reporting"});

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, VNL, type_<std::string>(), names{"auth_channel_answer"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"auth_channel_target"});

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"message"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"display_link"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::chrono::seconds>(), names{"mod_timeout"}, set(0));

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<bool>(), names{"accept_message"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<bool>(), names{"display_message"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"rejected"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"keepalive"}, set(false));

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"session_id"});

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::chrono::seconds>(), names{.all="end_date_cnx", .sesman="timeclose"}, set(0));

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"real_target_device"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"authentication_challenge"});

        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"ticket"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"comment"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"duration"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::chrono::minutes>(), names{"duration_max"}, set(0));
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"waitinforeturn"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<bool>(), names{"showform"}, set(false));
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<types::unsigned_>(), names{"formflag"}, set(0));

        _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L, type_<ModuleName>(), spec::type_<std::string>(), names{"module"}, set(ModuleName::login));

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<bool>(), names{"try_alternate_target"}, set(false));
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<bool>(), names{"has_more_target"}, set(false));

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"proxy_opt"});

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"pattern_kill"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"pattern_notify"});

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"opt_message"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"login_message"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"disconnect_reason"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<bool>(), names{"disconnect_reason_ack"}, set(false));

        _.member(no_ini_no_gui, no_sesman, L, type_<std::string>(), names{"ip_target"});

        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<bool>(), names{"recording_started"}, set(false));
        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L, type_<bool>(), names{"rt_ready"}, set(false));
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<bool>(), names{"sharing_ready"}, set(false));

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_command"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"auth_notify"});

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::unsigned_>(), names{"auth_notify_rail_exec_flags"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_notify_rail_exec_exe_or_file"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::u16>(), names{"auth_command_rail_exec_exec_result"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<types::u16>(), names{"auth_command_rail_exec_flags"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_command_rail_exec_original_exe_or_file"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_command_rail_exec_exe_or_file"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_command_rail_exec_working_dir"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_command_rail_exec_arguments"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"auth_command_rail_exec_account"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, NL, type_<std::string>(), names{"auth_command_rail_exec_password"});

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"session_probe_launch_error_message"});

        _.member(no_ini_no_gui, no_sesman, L, type_<std::string>(), names{"close_box_extra_message"});

        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"is_wabam"}, set(false));

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"pm_response"});
        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"pm_request"});

        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<types::u32>(), names{"native_session_id"});

        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<bool>(), names{"rd_shadow_available"}, set(false));

        _.member(no_ini_no_gui, sesman_rw, reset_back_to_selector, L, type_<std::string>(), names{"rd_shadow_userdata"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::string>(), names{"rd_shadow_type"});

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::u32>(), names{"rd_shadow_invitation_error_code"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"rd_shadow_invitation_error_message"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"rd_shadow_invitation_id"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"rd_shadow_invitation_addr"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::u16>(), names{"rd_shadow_invitation_port"});

        _.member(no_ini_no_gui, sesman_rw, no_reset_back_to_selector, L, type_<std::string>(), names{"session_sharing_userdata"});
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<bool>(), names{"session_sharing_enable_control"}, set(false));
        _.member(no_ini_no_gui, sesman_to_proxy, no_reset_back_to_selector, L, type_<std::chrono::seconds>(), names{"session_sharing_ttl"}, set(60*10));

        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<types::u32>(), names{"session_sharing_invitation_error_code"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"session_sharing_invitation_error_message"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, NL, type_<std::string>(), names{"session_sharing_invitation_id"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"session_sharing_invitation_addr"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"session_sharing_target_ip"});
        _.member(no_ini_no_gui, proxy_to_sesman, no_reset_back_to_selector, L, type_<std::string>(), names{"session_sharing_target_login"});

        _.member(no_ini_no_gui, no_sesman, L, type_<bool>(), names{"rail_module_host_mod_is_active"}, set(false));

        _.member(no_ini_no_gui, proxy_to_sesman, reset_back_to_selector, L, type_<std::string>(), names{"smartcard_login"});

        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<std::string>(), names{"banner_message"});
        _.member(no_ini_no_gui, sesman_to_proxy, reset_back_to_selector, L, type_<BannerType>(), names{"banner_type"});

        _.member(no_ini_no_gui, no_sesman, L, type_<std::vector<uint8_t>>(), names{"redirection_password_or_cookie"});
    });

    _.section("theme", [&]
    {
        _.member(ini_and_gui, no_sesman, L, type_<bool>(), names{"enable_theme"}, desc{"Enable custom theme color configuration. Each theme color can be defined as HTML color code (white: #FFFFFF, black: #000000, blue: #0000FF, etc)"}, set(false));

        _.member(image_in_gui, no_sesman, L, type_<std::string>(), names{.all="logo_path", .ini="logo"},
        desc{"Logo displayed when theme is enabled"}, set(CPP_EXPR(REDEMPTION_CONFIG_THEME_LOGO)));

        auto to_rgb = [](NamedBGRColor color){
            return BGRColor(BGRasRGBColor(color)).as_u32();
        };

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"bgcolor"}, set(to_rgb(DARK_BLUE_BIS)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"fgcolor"}, set(to_rgb(WHITE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"separator_color"}, set(to_rgb(LIGHT_BLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"focus_color"}, set(to_rgb(WINBLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"error_color"}, set(to_rgb(YELLOW)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"edit_bgcolor"}, set(to_rgb(WHITE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"edit_fgcolor"}, set(to_rgb(BLACK)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"edit_focus_color"}, set(to_rgb(WINBLUE)));

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"tooltip_bgcolor"}, set(to_rgb(BLACK)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"tooltip_fgcolor"}, set(to_rgb(LIGHT_YELLOW)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"tooltip_border_color"}, set(to_rgb(BLACK)));

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_line1_bgcolor"}, set(to_rgb(PALE_BLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_line1_fgcolor"}, set(to_rgb(BLACK)));

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_line2_bgcolor"}, set(to_rgb(LIGHT_BLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_line2_fgcolor"}, set(to_rgb(BLACK)));

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_selected_bgcolor"}, set(to_rgb(MEDIUM_BLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_selected_fgcolor"}, set(to_rgb(WHITE)));

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_focus_bgcolor"}, set(to_rgb(WINBLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_focus_fgcolor"}, set(to_rgb(WHITE)));

        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_label_bgcolor"}, set(to_rgb(MEDIUM_BLUE)));
        _.member(ini_and_gui, no_sesman, L, type_<types::rgb>(), names{"selector_label_fgcolor"}, set(to_rgb(WHITE)));
    });

    REDEMPTION_DIAGNOSTIC_POP()

    _.build();
}

}
