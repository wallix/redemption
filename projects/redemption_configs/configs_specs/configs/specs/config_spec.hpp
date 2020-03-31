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

#include "keyboard/keymap2.hpp" // keylayouts

#include <algorithm>
#include <chrono>
#include <vector>
#include <string>


/// ~"forward declaration" for redemption type
//@{
CONFIG_DEFINE_TYPE(RedirectionInfo)
//@}

namespace cfg_specs {

#ifdef IN_IDE_PARSER
    // for coloration...
    struct Writer
    {
        template<class... Args>
        void member(Args...);

        template<class F>
        void section(char const * name, F closure_section);

        struct Names {};
        template<class... Name>
        Names names(Name...);

        template<class F>
        void section(Names, F closure_section);
    };
#else
template<class Writer>
#endif
void config_spec_definition(Writer && W)
{
    using namespace cfg_attributes;

    using namespace cfg_attributes::spec::constants;
    using namespace cfg_attributes::sesman::constants;
    using namespace cfg_attributes::connpolicy::constants;

    // force ordering section
    {
        char const * sections_name[] = {
            "globals",
            "session_log",
            "client",
            "mod_rdp",
            "mod_vnc",
            "metrics",
            "file_verification",
            "file_storage",
            "icap_server_down",
            "icap_server_up",
            "mod_replay",
            "ocr",
            "video",
            "capture",
            "crypto",
            "websocket",
            "debug",
        };
        for (char const * section : sections_name) {
            W.section(section, [&]{});
        }
    }

    prefix_value disable_prefix_val{"disable"};

    constexpr char default_key[] =
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ;

    auto L = loggable;
    auto NL = unloggable;
    auto VNL = unloggable_if_value_contains_password;

    auto rdp_connpolicy = sesman::connection_policy{"rdp"};
    auto vnc_connpolicy = sesman::connection_policy{"vnc"};
    auto allow_connpolicy_and_gui = connpolicy::allow_connpolicy_and_gui;

    char const* disabled_orders_desc =
        "Disables supported drawing orders:\n"
        "   0: DstBlt\n"
        "   1: PatBlt\n"
        "   2: ScrBlt\n"
        "   3: MemBlt\n"
        "   4: Mem3Blt\n"
        "   8: LineTo\n"
        "  15: MultiDstBlt\n"
        "  16: MultiPatBlt\n"
        "  17: MultiScrBlt\n"
        "  18: MultiOpaqueRect\n"
        "  22: Polyline\n"
        "  25: EllipseSC\n"
        "  27: GlyphIndex\n"
    ;

    W.section("globals", [&]
    {
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "capture_chunk");

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "nla_auth_user", sesman::name{"nla_login"});
        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "auth_user", sesman::name{"login"});
        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "host", sesman::name{"ip_client"});
        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "target", sesman::name{"ip_target"});
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "target_device");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "device_id");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "primary_user_id");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "target_user", sesman::name{"target_login"});
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "target_application");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "target_application_account");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, NL, type_<std::string>(), "target_application_password");

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "glyph_cache", set(false));
        W.member(advanced_in_gui | iptables_in_gui, no_sesman, L, type_<unsigned>(), "port", desc{"Warning: Service will be automatically restarted and active sessions will be disconnected\nThe port set in this field must not be already used, otherwise the service will not run."}, set(3389));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "nomouse", set(false));
        W.member(advanced_in_gui, no_sesman, L, type_<Level>(), "encryptionLevel", set(Level::low));
        W.member(advanced_in_gui, no_sesman, L, type_<std::string>(), "authfile", set(CPP_EXPR(REDEMPTION_CONFIG_AUTHFILE)));

        W.member(ini_and_gui, no_sesman, L, type_<std::chrono::seconds>(), "handshake_timeout", desc{"Time out during RDP handshake stage."}, set(10));
        W.member(ini_and_gui, no_sesman, L, type_<std::chrono::seconds>(), "session_timeout", desc{"No traffic auto disconnection."}, set(900));
        W.member(hidden_in_gui, rdp_connpolicy, connpolicy::section{"session"}, L, type_<std::chrono::seconds>(), "inactivity_timeout", desc{"No traffic auto disconnection.\nIf value is 0, global value (session_timeout) is used."}, set(0));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), "keepalive_grace_delay", desc{"Keepalive."}, set(30));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), "authentication_timeout", desc{"Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate)."}, set(120));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), "close_timeout", desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."}, set(600));

        W.member(advanced_in_gui, sesman_to_proxy, is_target_ctx, L, type_<TraceType>(), "trace_type", set(TraceType::localfile_hashed));

        W.member(advanced_in_gui, no_sesman, L, type_<types::ip_string>(), "listen_address", set("0.0.0.0"));
        W.member(iptables_in_gui, no_sesman, L, type_<bool>(), "enable_transparent_mode", desc{"Allow Transparent mode."}, set(false));
        W.member(advanced_in_gui | password_in_gui, no_sesman, L, type_<types::fixed_string<254>>(), "certificate_password", desc{"Proxy certificate password."}, set("inquisition"));

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<bool>(), "is_rec", set(false));
        W.member(advanced_in_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "movie_path", sesman::name{"rec_path"});
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_bitmap_update", desc{"Support of Bitmap Update."}, set(true));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "enable_close_box", desc{"Show close screen."}, set(true));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_osd", set(true));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_osd_display_remote_target", set(true));

        W.member(hidden_in_gui, no_sesman, L, type_<bool>(), "enable_wab_integration", set((CPP_EXPR(REDEMPTION_CONFIG_ENABLE_WAB_INTEGRATION))));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "allow_using_multiple_monitors", set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "bogus_refresh_rect", desc{"Needed to refresh screen of Windows Server 2012."}, set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "large_pointer_support", set(true));

        W.member(ini_and_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "unicode_keyboard_event_support", set(true));

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::range<std::chrono::milliseconds, 100, 10000>>{}, "mod_recv_timeout", set(1000));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "spark_view_specific_glyph_width", set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "experimental_enable_serializer_data_block_size_limit",set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "experimental_support_resize_session_during_recording",set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "support_connection_redirection_during_recording",set(true));

        W.member(ini_and_gui, no_sesman, L, type_<std::chrono::milliseconds>(), "rdp_keepalive_connection_interval", desc{
            "Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.\n"
            "!!!May cause FreeRDP-based client to CRASH!!!\n"
            "Set to 0 to disable this feature."
        }, set(0));
    });

    W.section("session_log", [&]
    {
        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "enable_session_log", set(true));
        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "enable_arcsight_log", set(false));
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<KeyboardInputMaskingLevel>(), "keyboard_input_masking_level", desc{
            "Keyboard Input Masking Level:"
        }, set(KeyboardInputMaskingLevel::password_and_unidentified));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "hide_non_printable_kbd_input", set(false));
    });

    W.section("client", [&]
    {
        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<unsigned>(), "keyboard_layout", set(0));
        std::string keyboard_layout_proposals_desc;
        for (auto k : Keymap2::keylayouts()) {
            keyboard_layout_proposals_desc += k->locale_name;
            keyboard_layout_proposals_desc += ", ";
        }
        if (!keyboard_layout_proposals_desc.empty()) {
            keyboard_layout_proposals_desc.resize(keyboard_layout_proposals_desc.size() - 2);
        }
        W.member(advanced_in_gui, no_sesman, L, type_<types::list<std::string>>(), "keyboard_layout_proposals", desc{keyboard_layout_proposals_desc}, set("en-US, fr-FR, de-DE, ru-RU"));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "ignore_logon_password", desc{"If true, ignore password provided by RDP client, user need do login manually."}, set(false));

        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "performance_flags_default", desc{"Enable font smoothing (0x80)."}, set(0x80));
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "performance_flags_force_present", desc{
            "Disable theme (0x8).\n"
            "Disable mouse cursor shadows (0x20)."
        }, set(0x28));
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "performance_flags_force_not_present", set(0));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "auto_adjust_performance_flags", desc{"If enabled, avoid automatically font smoothing in recorded session."}, set(true));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "tls_fallback_legacy", desc{"Fallback to RDP Legacy Encryption if client does not support TLS."}, set(false));
        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "tls_support", set(true));
        W.member(ini_and_gui, no_sesman, L, type_<uint32_t>(), "tls_min_level", desc{"Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(2));
        W.member(ini_and_gui, no_sesman, L, type_<uint32_t>(), "tls_max_level", desc{"Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(0));
        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "show_common_cipher_list", desc{"Show common cipher list supported by client and server"}, set(false));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_nla",
                    desc{"Needed for primary NTLM or Kerberos connections over NLA."}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "bogus_neg_request", desc{"Needed to connect with jrdp, based on bogus X224 layer code."}, set(false));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "bogus_user_id", desc{"Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code."}, set(true));

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "disable_tsk_switch_shortcuts", desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<RdpCompression>{}, "rdp_compression", set(RdpCompression::rdp6_1));

        W.member(advanced_in_gui, no_sesman, L, type_<ColorDepth>{}, "max_color_depth", set(ColorDepth::depth24));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the front side."}, set(true));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(false));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "bitmap_compression", desc{"Support of Bitmap Compression."}, set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "fast_path", desc{"Enables support of Client Fast-Path Input Event PDUs."}, set(true));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "enable_suppress_output", set(true));

        W.member(ini_and_gui, no_sesman, L, type_<std::string>(), "ssl_cipher_list", desc{
            "[Not configured]: Compatible with more RDP clients (less secure)\n"
            "HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)"
            "HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)"
        }, set("HIGH:!ADH:!3DES:!SHA"));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "show_target_user_in_f12_message", set(false));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "enable_new_pointer_update", set(false));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "bogus_ios_glyph_support_level", set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "transform_glyph_to_bitmap", set(false));

        W.member(ini_and_gui, no_sesman, L, type_<BogusNumberOfFastpathInputEvent>(), "bogus_number_of_fastpath_input_event", set(BogusNumberOfFastpathInputEvent::pause_key_only));

        W.member(advanced_in_gui, no_sesman, L, type_<types::range<std::chrono::milliseconds, 100, 10000>>{}, "recv_timeout", set(1000));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>{}, "enable_osd_4_eyes", set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>{}, "enable_remotefx", desc{"Enable front remoteFx"}, set(true));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "bogus_pointer_xormask_padding", set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<types::list<unsigned>>(), "disabled_orders", desc{disabled_orders_desc}, set("25"));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(),
                 "force_bitmap_cache_v2_with_am",
                 desc{"Force usage of bitmap cache V2 for compatibility "
                         "with WALLIX Access Manager."},
                 set(true));
    });

    W.section(W.names("mod_rdp", connpolicy::name{"rdp"}), [&]
    {
        auto co_probe = connpolicy::section{"session_probe"};
        auto co_cert = connpolicy::section{"server_cert"};

        W.member(advanced_in_gui, no_sesman, L, type_<RdpCompression>{}, "rdp_compression", set(RdpCompression::rdp6_1));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "disconnect_on_logon_user_change", set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), "open_session_timeout", set(0));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<types::list<unsigned>>(), "disabled_orders", desc{disabled_orders_desc}, set(""));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "enable_nla", desc{"NLA authentication in secondary target."}, set(true));
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "enable_kerberos", desc{
            "If enabled, NLA authentication will try Kerberos before NTLM.\n"
            "(if enable_nla is disabled, this value is ignored)."
        }, set(false));
        W.member(no_ini_no_gui, rdp_connpolicy, L, type_<uint32_t>(), "tls_min_level", desc{"Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(0));
        W.member(no_ini_no_gui, rdp_connpolicy, L, type_<uint32_t>(), "tls_max_level", desc{"Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3"}, set(0));
        W.member(no_ini_no_gui, rdp_connpolicy, L, type_<std::string>(), "cipher_string", desc{"TLSv1.2 additional ciphers supported by client, default is empty to apply system-wide configuration (SSL security level 2), ALL for support of all ciphers to ensure highest compatibility with target servers."}, set("ALL"));
        W.member(no_ini_no_gui, rdp_connpolicy, L, type_<bool>(), "show_common_cipher_list", desc{"Show common cipher list supported by client and server"}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the mod side."}, set(true));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));

        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::list<std::string>>(), "allow_channels", desc{"Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority."}, set("*"));
        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::list<std::string>>(), "deny_channels", desc{"Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "fast_path", desc{"Enables support of Client/Server Fast-Path Input/Update PDUs.\nFast-Path is required for Windows Server 2012 (or more recent)!"}, set(true));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "server_redirection_support", desc{"Enables Server Redirection Support."}, connpolicy::name{"server_redirection"}, set(false));

        W.member(no_ini_no_gui, no_sesman, L, type_<RedirectionInfo>(), "redir_info");
        W.member(no_ini_no_gui, rdp_connpolicy, L, type_<std::string>(), "load_balance_info", desc{"Load balancing information"});

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "bogus_sc_net_size", desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."}, sesman::name{"rdp_bogus_sc_net_size"}, set(true));

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::list<std::string>>(), "proxy_managed_drives");

        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "ignore_auth_channel", set(false));
        W.member(ini_and_gui, no_sesman, L, type_<types::fixed_string<7>>(), "auth_channel", set("*"), desc{"Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel."});
        W.member(ini_and_gui, no_sesman, L, type_<types::fixed_string<7>>(), "checkout_channel", set(""), desc{"Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel."});

        W.member(hidden_in_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "alternate_shell");
        W.member(hidden_in_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "shell_arguments");
        W.member(hidden_in_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "shell_working_directory");

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "use_client_provided_alternate_shell", desc{"As far as possible, use client-provided initial program (Alternate Shell)"}, set(false));
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "use_client_provided_remoteapp", desc{"As far as possible, use client-provided remote program (RemoteApp)"}, set(false));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "use_native_remoteapp_capability", desc{"As far as possible, use native RemoteApp capability"}, set(true));

        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<bool>(), "enable_session_probe", set(false), connpolicy::set(true));
        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<bool>(), "session_probe_use_smart_launcher", cpp::name{"session_probe_use_clipboard_based_launcher"}, connpolicy::name{"use_smart_launcher"}, desc{
            "Minimum supported server : Windows Server 2008.\n"
            "Clipboard redirection should be remain enabled on Terminal Server."
        }, set(true));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "session_probe_enable_launch_mask", connpolicy::name{"enable_launch_mask"}, sesman::deprecated_names{"enable_session_probe_launch_mask"}, set(true));
        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<SessionProbeOnLaunchFailure>(), "session_probe_on_launch_failure", connpolicy::name{"on_launch_failure"}, set(SessionProbeOnLaunchFailure::disconnect_user));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 300000>>(), "session_probe_launch_timeout", connpolicy::name{"launch_timeout"}, desc{
            "This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).\n"
            "0 to disable timeout."
        }, set(40000));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 300000>>(), "session_probe_launch_fallback_timeout", connpolicy::name{"launch_fallback_timeout"}, desc{
            "This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).\n"
            "0 to disable timeout."
        }, set(10000));
        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<bool>(), "session_probe_start_launch_timeout_timer_only_after_logon", connpolicy::name{"start_launch_timeout_timer_only_after_logon"}, desc{
            "Minimum supported server : Windows Server 2008."
        }, set(true));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 60000>>(), "session_probe_keepalive_timeout", connpolicy::name{"keepalive_timeout"}, set(5000));
        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<SessionProbeOnKeepaliveTimeout>(), "session_probe_on_keepalive_timeout", connpolicy::name{"on_keepalive_timeout"}, set(SessionProbeOnKeepaliveTimeout::disconnect_user));

        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<bool>(), "session_probe_end_disconnected_session", connpolicy::name{"end_disconnected_session"}, desc{"End automatically a disconnected session.\nSession Probe must be enabled to use this feature."}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "session_probe_customize_executable_name", set(false));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "session_probe_enable_log", connpolicy::name{"enable_log"}, set(false));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "session_probe_enable_log_rotation", connpolicy::name{"enable_log_rotation"}, set(true));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(), "session_probe_disconnected_application_limit", connpolicy::name{"disconnected_application_limit"}, desc{
            "This policy setting allows you to configure a time limit for disconnected application sessions.\n"
            "0 to disable timeout."
        }, set(0));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(), "session_probe_disconnected_session_limit", connpolicy::name{"disconnected_session_limit"}, desc{
            "This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.\n"
            "0 to disable timeout."
        }, set(0));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 172'800'000>>(), "session_probe_idle_session_limit", connpolicy::name{"idle_session_limit"}, desc{
            "This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.\n"
            "0 to disable timeout."
        }, set(0));

        W.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<511>>(), "session_probe_exe_or_file", set("||CMD"));
        W.member(hidden_in_gui, no_sesman, L, type_<types::fixed_string<511>>(), "session_probe_arguments", set(CPP_EXPR(REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS)));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<std::chrono::milliseconds>(), "session_probe_clipboard_based_launcher_clipboard_initialization_delay", connpolicy::name{"smart_launcher_clipboard_initialization_delay"}, set(2000));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<std::chrono::milliseconds>(), "session_probe_clipboard_based_launcher_start_delay", connpolicy::name{"smart_launcher_start_delay"}, set(0));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<std::chrono::milliseconds>(), "session_probe_clipboard_based_launcher_long_delay", connpolicy::name{"smart_launcher_long_delay"}, set(500));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<std::chrono::milliseconds>(), "session_probe_clipboard_based_launcher_short_delay", connpolicy::name{"smart_launcher_short_delay"}, set(50));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 300000>>(), "session_probe_launcher_abort_delay", connpolicy::name{"launcher_abort_delay"}, set(2000));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "session_probe_allow_multiple_handshake", set(false));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "session_probe_enable_crash_dump", connpolicy::name{"enable_crash_dump"}, set(false));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<types::u32, 0, 1000>>(), "session_probe_handle_usage_limit", connpolicy::name{"handle_usage_limit"}, set(0));
        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<types::u32, 0, 200'000'000>>(), "session_probe_memory_usage_limit", connpolicy::name{"memory_usage_limit"}, set(0));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<types::range<std::chrono::milliseconds, 0, 60000>>(), "session_probe_end_of_session_check_delay_time", connpolicy::name{"end_of_session_check_delay_time"}, set(0));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "session_probe_ignore_ui_less_processes_during_end_of_session_check", connpolicy::name{"ignore_ui_less_processes_during_end_of_session_check"}, set(true));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "session_probe_childless_window_as_unidentified_input_field", connpolicy::name{"childless_window_as_unidentified_input_field"}, set(true));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<SessionProbeDisabledFeature>(), "session_probe_disabled_features", disable_prefix_val, connpolicy::name{"disabled_features"}, set(SessionProbeDisabledFeature::chrome_inspection | SessionProbeDisabledFeature::firefox_inspection | SessionProbeDisabledFeature::group_membership));

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_probe, L, type_<bool>(), "enable_bestsafe_integration", connpolicy::set(false));

        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<bool>(), connpolicy::name{"public_session"}, "session_probe_public_session", desc{"If enabled, disconnected session can be recovered by a different primary user."}, set(false));

        W.member(hidden_in_gui, rdp_connpolicy, co_probe, L, type_<SessionProbeOnAccountManipulation>(), "session_probe_on_account_manipulation", connpolicy::name{"on_account_manipulation"}, set(SessionProbeOnAccountManipulation::allow));

        W.member(hidden_in_gui, rdp_connpolicy, co_cert, L, type_<bool>(), "server_cert_store", desc{"Keep known server certificates on WAB"}, set(true));
        W.member(hidden_in_gui, rdp_connpolicy, co_cert, L, type_<ServerCertCheck>(), "server_cert_check", set(ServerCertCheck::fails_if_no_match_and_succeed_if_no_know));

        struct P { char const * name; char const * desc; };
        for (P p : {
            P{"server_access_allowed_message", "Warn if check allow connexion to server."},
            P{"server_cert_create_message", "Warn that new server certificate file was created."},
            P{"server_cert_success_message", "Warn that server certificate file was successfully checked."},
            P{"server_cert_failure_message", "Warn that server certificate file checking failed."},
        }) {
            W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, co_cert, L, type_<ServerNotification>(), p.name, desc{p.desc}, set(ServerNotification::syslog));
        }
        W.member(hidden_in_gui, no_sesman, L, type_<ServerNotification>(), "server_cert_error_message", desc{"Warn that server certificate check raised some internal error."}, set(ServerNotification::syslog));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "hide_client_name", desc{"Do not transmit client machine name or RDP server."}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "clean_up_32_bpp_cursor", set(false));

        W.member(ini_and_gui, no_sesman, L, type_<bool>(), "bogus_ios_rdpdr_virtual_channel", set(true));

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "enable_rdpdr_data_analysis", set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::milliseconds>(), "remoteapp_bypass_legal_notice_delay", desc{
            "Delay before automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
            "Set to 0 to disable this feature."
        }, set(0));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::milliseconds>(), "remoteapp_bypass_legal_notice_timeout", desc{
            "Time limit to automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
            "Set to 0 to disable this feature."
        }, set(20000));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "log_only_relevant_clipboard_activities", set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "experimental_fix_input_event_sync", set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "experimental_fix_too_long_cookie", set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "split_domain", desc{
            "Force to split target domain and username with '@' separator."
        }, set(false));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "wabam_uses_translated_remoteapp", set(false));

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "enable_server_cert_external_validation");
        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<std::string>(), "server_cert");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "server_cert_response", desc{"empty string for wait, 'Ok' or error message"});

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "session_shadowing_support", desc{"Enables Session Shadowing Support."}, set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "use_license_store", desc{"Stores CALs issued by the terminal servers."}, set(true));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "enable_remotefx", desc{"Enables support of the remoteFX codec."}, set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "accept_monitor_layout_change_if_capture_is_not_started", set(false));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(),
                 "enable_restricted_admin_mode",
                 desc{"Connect to the server in Restricted Admin mode.\n"
                         "This mode must be supported by the server "
                         "(available from Windows Server 2012 R2), "
                         "otherwise, connection will fail.\n"
                         "NLA must be enabled."},
                 set(false));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(),
                 "force_smartcard_authentication",
                 desc{"NLA will be disabled.\n"
                         "Target must be set for interactive login, otherwise server connection may not be guaranteed.\n"
                         "Smartcard device must be available on client desktop.\n"
                         "Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service."},
                 set(false));
    });

    W.section("metrics", [&]
    {
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_rdp_metrics", set(false));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_vnc_metrics", set(false));
        W.member(hidden_in_gui, no_sesman, L, type_<types::dirpath>(), "log_dir_path", set(CPP_EXPR(app_path(AppPath::Metrics).to_string())));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), "log_interval", set(5));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::hours>(), "log_file_turnover_interval", set(24));
        W.member(advanced_in_gui, no_sesman, L, type_<std::string>(), "sign_key", desc{"signature key to digest log metrics header info"}, set(default_key));
    });

    W.section(W.names("mod_vnc", connpolicy::name{"vnc"}), [&]
    {
        W.member(ini_and_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "clipboard_up", desc{"Enable or disable the clipboard from client (client to server)."});
        W.member(ini_and_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "clipboard_down", desc{"Enable or disable the clipboard from server (server to client)."});

        W.member(advanced_in_gui, no_sesman, L, type_<types::list<int>>(), "encodings", desc{
            "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
            "  0: Raw\n"
            "  1: CopyRect\n"
            "  2: RRE\n"
            "  16: ZRLE\n"
            "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
        });

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<ClipboardEncodingType>(), "server_clipboard_encoding_type", desc{
            "VNC server clipboard data encoding type."
        }, sesman::name{"vnc_server_clipboard_encoding_type"}, set(ClipboardEncodingType::latin1));

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<VncBogusClipboardInfiniteLoop>(), "bogus_clipboard_infinite_loop", sesman::name{"vnc_bogus_clipboard_infinite_loop"}, set(VncBogusClipboardInfiniteLoop::delayed));

        W.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), "server_is_macos", set(false));
        W.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), "server_unix_alt", set(false));

        W.member(hidden_in_gui, vnc_connpolicy, L, type_<bool>(), "support_cursor_pseudo_encoding", set(true));
    });

    W.section("mod_replay", [&]
    {
        W.member(hidden_in_gui, no_sesman, L, type_<bool>(), "on_end_of_data", desc{"0 - Wait for Escape, 1 - End session"}, set(false));
        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "replay_on_loop", desc{"0 - replay once, 1 - loop replay"}, set(false));
    });

    W.section("ocr", [&]
    {
        W.member(ini_and_gui, no_sesman, L, type_<OcrVersion>(), "version", set(OcrVersion::v2));
        W.member(ini_and_gui, no_sesman, L, type_<OcrLocale>(), "locale", set(OcrLocale::latin));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::duration<unsigned, std::centi>>(), "interval", set(100));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "on_title_bar_only", set(true));
        W.member(advanced_in_gui, no_sesman, L, type_<types::range<unsigned, 0, 100>>{}, "max_unrecog_char_rate", desc{
            "Expressed in percentage,\n"
          "  0   - all of characters need be recognized\n"
          "  100 - accept all results"
        }, set(40));
    });

    W.section("video", [&]
    {
        W.member(advanced_in_gui, no_sesman, L, type_<unsigned>(), "capture_groupid", set(33));

        W.member(advanced_in_gui, no_sesman, L, type_<CaptureFlags>{}, "capture_flags", set(CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr));

        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::duration<unsigned, std::deci>>(), "png_interval", desc{"Frame interval."}, set(10));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::duration<unsigned, std::centi>>(), "frame_interval", desc{"Frame interval."}, set(40));
        W.member(advanced_in_gui, no_sesman, L, type_<std::chrono::seconds>(), "break_interval", desc{"Time between 2 wrm movies."}, set(600));
        W.member(advanced_in_gui, no_sesman, L, type_<unsigned>(), "png_limit", desc{"Number of png captures to keep."}, set(5));

        W.member(advanced_in_gui, no_sesman, L, type_<types::dirpath>(), "replay_path", set("/tmp/"));

        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::dirpath>(), "hash_path", set(CPP_EXPR(app_path(AppPath::Hash).to_string())));
        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::dirpath>(), "record_tmp_path", set(CPP_EXPR(app_path(AppPath::RecordTmp).to_string())));
        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, L, type_<types::dirpath>(), "record_path", set(CPP_EXPR(app_path(AppPath::Record).to_string())));

        W.member(advanced_in_gui, allow_connpolicy_and_gui, rdp_connpolicy, L, type_<KeyboardLogFlags>{}, connpolicy::type_<KeyboardLogFlagsCP>{}, "disable_keyboard_log", desc{
            "Disable keyboard log:\n"
            "(Please see also \"Keyboard input masking level\" in \"session_log\" section of \"Connection Policy\".)"
        }, disable_prefix_val, set(KeyboardLogFlags::syslog), connpolicy::set(KeyboardLogFlagsCP::syslog));

        W.member(advanced_in_gui, no_sesman, L, type_<ClipboardLogFlags>(), "disable_clipboard_log", desc{"Disable clipboard log:"}, disable_prefix_val, set(ClipboardLogFlags::syslog));

        W.member(advanced_in_gui, no_sesman, L, type_<FileSystemLogFlags>(), "disable_file_system_log", desc{"Disable (redirected) file system log:"}, disable_prefix_val, set(FileSystemLogFlags::syslog));

        W.member(hidden_in_gui, sesman_to_proxy, is_target_ctx, L, type_<bool>(), "rt_display", set(false));

        W.member(advanced_in_gui, no_sesman, L, type_<ColorDepthSelectionStrategy>{}, "wrm_color_depth_selection_strategy", set(ColorDepthSelectionStrategy::depth16));
        W.member(advanced_in_gui, no_sesman, L, type_<WrmCompressionAlgorithm>{}, "wrm_compression_algorithm", set(WrmCompressionAlgorithm::gzip));

        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "bogus_vlc_frame_rate", desc{"Needed to play a video with ffplay or VLC.\nNote: Useless with mpv and mplayer."}, set(true));

        W.member(advanced_in_gui, no_sesman, L, type_<std::string>(), "codec_id", set("mp4"));
        W.member(advanced_in_gui, no_sesman, L, type_<unsigned>(), "framerate", set(5));
        W.member(advanced_in_gui, no_sesman, L, type_<std::string>(), "ffmpeg_options", desc{"FFmpeg options for video codec."}, set("profile=baseline preset=ultrafast flags=+qscale b=80000"));
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "notimestamp", set(false));

        W.member(ini_and_gui, no_sesman, L, type_<SmartVideoCropping>(), "smart_video_cropping", set(SmartVideoCropping::disable));

        // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
        W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "play_video_with_corrupted_bitmap", desc{"Needed to play a video with corrupted Bitmap Update.\nNote: Useless with mpv and mplayer."}, set(false));
    });

    W.section("capture", [&]
    {
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "record_filebase", desc{"basename without extension"});
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "record_subdirectory", desc{"subdirectory of record_path (video section)"});

        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<std::string>(), "fdx_path");
    });

    W.section("crypto", [&]
    {
        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, NL, type_<types::fixed_binary<32>>(), "encryption_key",
            cpp::name{"key0"}, set(default_key));
        W.member(hidden_in_gui, sesman_to_proxy, not_target_ctx, NL, type_<types::fixed_binary<32>>(), "sign_key",
            cpp::name{"key1"}, set(default_key));
    });

    W.section("websocket", [&]
    {
        W.member(hidden_in_gui, no_sesman, L, type_<bool>(), "enable_websocket", set(false), desc{"Enable websocket protocol (ws or wss with use_tls=1)"});
        W.member(hidden_in_gui, no_sesman, L, type_<bool>(), "use_tls", set(true), desc{"Use TLS with websocket (wss)"});
        W.member(hidden_in_gui, no_sesman, L, type_<std::string>(), "listen_address", desc{"${addr}:${port} or ${port} or ${unix_socket_path}"}, set(":3390"));
    });

    W.section("remote_program", [&]
    {
        W.member(ini_and_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "allow_resize_hosted_desktop", set(true));
    });

    W.section("debug", [&]
    {
        W.member(hidden_in_gui, no_sesman, L, type_<std::string>(), "fake_target_ip");

        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "primary_orders");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "secondary_orders");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "bitmap_update");

        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "capture");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "auth");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "session");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "front");

        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "mod_rdp");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "mod_vnc");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "mod_internal");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "mod_xup");

        W.member(hidden_in_gui, no_sesman, L, type_<types::u32>(), "password");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "compression");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "cache");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "performance");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "pass_dialog_box");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "ocr");
        W.member(advanced_in_gui | hex_in_gui, no_sesman, L, type_<types::u32>(), "ffmpeg");

        W.member(advanced_in_gui, no_sesman, L, type_<unsigned>(), spec::type_<bool>(), "config", set(2));
    });

    W.section("translation", [&]
    {
        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<Language>{}, "language", set(Language::en));

        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "password_en");
        W.member(advanced_in_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "password_fr");
    });

    W.section("internal_mod", [&]
    {
        W.member(advanced_in_gui, no_sesman, L, type_<std::string>(), "theme", spec::name{"load_theme"});
    });

    W.section("file_verification", [&]
    {
        W.member(hidden_in_gui, no_sesman, L, type_<std::string>(), "socket_path", set(CPP_EXPR(REDEMPTION_CONFIG_VALIDATOR_PATH)));

        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "enable_up", desc{"Enable use of ICAP service for file verification on upload."});
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "enable_down", desc{"Enable use of ICAP service for file verification on download."});
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "clipboard_text_up", desc{"Verify text data via clipboard from client to server.\nFile verification on upload must be enabled via option Enable up."});
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<bool>(), "clipboard_text_down", desc{"Verify text data via clipboard from server to client\nFile verification on download must be enabled via option Enable down."});

        W.member(hidden_in_gui, rdp_connpolicy | advanced_in_connpolicy, L, type_<bool>(), "log_if_accepted", set(true));
    });

    W.section("file_storage", [&]
    {
        W.member(hidden_in_gui, rdp_connpolicy, L, type_<RdpStoreFile>(), "store_file", set(RdpStoreFile::never), desc{"Enable storage of transferred files (via RDP Clipboard)."});
    });

    for (char const* section_name : {"icap_server_up", "icap_server_down"}) {
        // TODO temporary
        // please, update $REDEMPTION/tools/c++-analyzer/lua-checker/checkers/config.lua for each changement of value
        W.section(section_name, [&]
        {
            // for validator only
            W.member(ini_and_gui, no_sesman, L, type_<std::string>(), "host", desc{"Ip or fqdn of ICAP server"});
            // for validator only
            W.member(ini_and_gui, no_sesman, L, type_<unsigned>(), "port", desc{"Port of ICAP server"}, set(1344));
            // for validator only
            W.member(ini_and_gui, no_sesman, L, type_<std::string>(), "service_name", desc{"Service name on ICAP server"}, set("avscan"));

            // for validator only
            W.member(ini_and_gui, no_sesman, L, type_<bool>(), "tls", desc{"ICAP server uses tls"});
            // for validator only
            W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "enable_x_context",
                    desc{"Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server"}, set(true));
            // for validator only
            W.member(advanced_in_gui, no_sesman, L, type_<bool>(), "filename_percent_encoding",
                     desc{"Filename sent to ICAP as percent encoding"}, set(false));
        });
    }

    W.section("context", [&]
    {
        auto co_rdp = connpolicy::section{"rdp"};
        auto co_probe = connpolicy::section{"session_probe"};

        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<std::string>(), "psid", desc{"Proxy session log id"});

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<ColorDepth>(), "opt_bpp", sesman::name{"bpp"}, set(ColorDepth::depth24));
        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<types::u16>(), "opt_height", sesman::name{"height"}, set(600));
        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<types::u16>(), "opt_width", sesman::name{"width"}, set(800));

        // auth_error_message is left as std::string type because SocketTransport and ReplayMod
        // take it as argument on constructor and modify it as a std::string
        W.member(no_ini_no_gui, no_sesman, L, type_<std::string>(), "auth_error_message");

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "selector", set(false));
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<unsigned>(), "selector_current_page", set(1));
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "selector_device_filter");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "selector_group_filter");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "selector_proto_filter");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<unsigned>(), "selector_lines_per_page", set(0));
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<unsigned>(), "selector_number_of_pages", set(1));

        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, NL, type_<std::string>(), "target_password");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "target_host");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "target_str");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "target_service");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<unsigned>(), "target_port", set(3389));
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "target_protocol", sesman::name{"proto_dest"}, set("RDP"));

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, NL, type_<std::string>(), "password");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "nla_password_hash");

        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "reporting");

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, VNL, type_<std::string>(), "auth_channel_answer");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "auth_channel_target");

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "message");

        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<bool>(), "accept_message");
        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<bool>(), "display_message");

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "rejected");

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<bool>(), "authenticated", set(false));

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "keepalive", set(false));

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "session_id");

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<unsigned>(), "end_date_cnx", sesman::name{"timeclose"}, set(0));

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<RdpModeConsole>(), "mode_console", set(RdpModeConsole::allow));

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "real_target_device");

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "authentication_challenge");

        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "ticket");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "comment");
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "duration");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<unsigned>(), "duration_max", set(0));
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "waitinforeturn");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<bool>(), "showform", set(false));
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<unsigned>(), "formflag", set(0));

        W.member(no_ini_no_gui, sesman_rw, not_target_ctx, L, type_<std::string>(), "module", set("login"));
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<bool>(), "forcemodule", set(false));
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "proxy_opt");

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "pattern_kill");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "pattern_notify");

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "opt_message");

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "login_message");

        W.member(no_ini_no_gui, rdp_connpolicy, co_probe, L, type_<std::string>(), "session_probe_outbound_connection_monitoring_rules", connpolicy::name{"outbound_connection_monitoring_rules"}, desc{
            "Comma-separated rules (Ex.: $deny:192.168.0.0/24:*,$allow:host.domain.net:3389,$allow:192.168.0.110:*)\n"
            "(Ex. for backwards compatibility only: 10.1.0.0/16:22)\n"
            "Session Probe must be enabled to use this feature."
        });
        W.member(no_ini_no_gui, rdp_connpolicy, co_probe, L, type_<std::string>(), "session_probe_process_monitoring_rules", connpolicy::name{"process_monitoring_rules"}, desc{
            "Comma-separated rules (Ex.: $deny:Taskmgr)\n"
            "@ = All child processes of Bastion Application (Ex.: $deny:@)\n"
            "Session Probe must be enabled to use this feature."
        });
        W.member(no_ini_no_gui, rdp_connpolicy, co_probe, L, type_<std::string>(), "session_probe_extra_system_processes", connpolicy::name{"extra_system_processes"}, desc{"Comma-separated extra system processes (Ex.: dllhos.exe,TSTheme.exe)"});

        W.member(no_ini_no_gui, rdp_connpolicy, co_probe, L, type_<std::string>(), "session_probe_windows_of_these_applications_as_unidentified_input_field", connpolicy::name{"windows_of_these_applications_as_unidentified_input_field"}, desc{"Comma-separated processes (Ex.: chrome.exe,ngf.exe)"});

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "disconnect_reason");
        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<bool>(), "disconnect_reason_ack", set(false));

        W.member(no_ini_no_gui, no_sesman, L, type_<std::string>(), "ip_target");

        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<bool>(), "recording_started", set(false));
        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<bool>(), "rt_ready", set(false));

        W.member(no_ini_no_gui, no_sesman, L, type_<bool>(), "perform_automatic_reconnection", set(false));

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "auth_command");
        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<std::string>(), "auth_notify");

        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<unsigned>(), "auth_notify_rail_exec_flags");
        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<std::string>(), "auth_notify_rail_exec_exe_or_file");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<unsigned>(), "auth_command_rail_exec_exec_result");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<unsigned>(), "auth_command_rail_exec_flags");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "auth_command_rail_exec_original_exe_or_file");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "auth_command_rail_exec_exe_or_file");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "auth_command_rail_exec_working_dir");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "auth_command_rail_exec_arguments");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<std::string>(), "auth_command_rail_exec_account");
        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, NL, type_<std::string>(), "auth_command_rail_exec_password");

        W.member(no_ini_no_gui, rdp_connpolicy | advanced_in_connpolicy, co_rdp, L, type_<types::range<std::chrono::milliseconds, 3000, 120000>>(), "rail_disconnect_message_delay", connpolicy::name{"remote_programs_disconnect_message_delay"}, desc{"Delay before showing disconnect message after the last RemoteApp window is closed."}, set(3000));

        W.member(no_ini_no_gui, rdp_connpolicy, co_rdp, L, type_<bool>(), "use_session_probe_to_launch_remote_program", desc{"Use Session Probe to launch Remote Program as much as possible."}, set(true));

        W.member(no_ini_no_gui, proxy_to_sesman, not_target_ctx, L, type_<std::string>(), "session_probe_launch_error_message");

        W.member(no_ini_no_gui, no_sesman, L, type_<std::string>(), "close_box_extra_message");

        W.member(no_ini_no_gui, sesman_to_proxy, not_target_ctx, L, type_<bool>(), "is_wabam", set(false));

        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "pm_response");
        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<std::string>(), "pm_request");

        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<unsigned>(), "native_session_id");

        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<bool>(), "rd_shadow_available", set(false));

        W.member(no_ini_no_gui, sesman_rw, is_target_ctx, L, type_<std::string>(), "rd_shadow_userdata");
        W.member(no_ini_no_gui, sesman_to_proxy, is_target_ctx, L, type_<std::string>(), "rd_shadow_type");

        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<unsigned>(), "rd_shadow_invitation_error_code");
        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<std::string>(), "rd_shadow_invitation_error_message");
        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<std::string>(), "rd_shadow_invitation_id");
        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<std::string>(), "rd_shadow_invitation_addr");
        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<unsigned>(), "rd_shadow_invitation_port");

        W.member(no_ini_no_gui, no_sesman, L, type_<bool>(), "rail_module_host_mod_is_active", set(false));

        W.member(no_ini_no_gui, proxy_to_sesman, is_target_ctx, L, type_<std::string>(), "smartcard_login");
    });
}

}
