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
CONFIG_DEFINE_TYPE(Theme)
CONFIG_DEFINE_TYPE(Font)
//@}

namespace cfg_specs {

template<cfg_attributes::spec::attr value>
using spec_attr_t = std::integral_constant<cfg_attributes::spec::attr, value>;

template<cfg_attributes::spec::attr v1, cfg_attributes::spec::attr v2>
spec_attr_t<v1 | v2>
operator | (spec_attr_t<v1>, spec_attr_t<v2>)
{
    using namespace cfg_attributes;
    static_assert(!bool(v1 & spec::attr::no_ini_no_gui), "no_ini_no_gui is incompatible with other values");
    static_assert(!bool(v2 & spec::attr::no_ini_no_gui), "no_ini_no_gui is incompatible with other values");
    constexpr auto in_gui
      = spec::attr::hidden_in_gui
      | spec::attr::hex_in_gui
      | spec::attr::advanced_in_gui
      | spec::attr::iptables_in_gui
      | spec::attr::password_in_gui
    ;
    static_assert(!(bool(v1 & spec::attr::hidden_in_gui) && bool(v2 & in_gui)), "hidden_in_gui is incompatible with *_in_gui values");
    static_assert(!(bool(v2 & spec::attr::hidden_in_gui) && bool(v1 & in_gui)), "hidden_in_gui is incompatible with *_in_gui values");
    return {};
}


template<class Writer>
void config_spec_definition(Writer && W)
{
    using namespace cfg_attributes;

#ifdef IN_IDE_PARSER
    // for coloration...
    struct {
        void member(...);
        void section(char const * name, closure_section);
        void sep();
    } W;
#endif

    spec_attr_t<spec::attr::no_ini_no_gui>    const no_ini_no_gui{};
    spec_attr_t<spec::attr::ini_and_gui>      const ini_and_gui{};
    spec_attr_t<spec::attr::hidden_in_gui>    const hidden_in_gui{};
    spec_attr_t<spec::attr::hex_in_gui>       const hex_in_gui{};
    spec_attr_t<spec::attr::advanced_in_gui>  const advanced_in_gui{};
    spec_attr_t<spec::attr::iptables_in_gui>  const iptables_in_gui{};
    spec_attr_t<spec::attr::password_in_gui>  const password_in_gui{};

    sesman::io const w = sesman::io::write;
    sesman::io const r = sesman::io::read;
    sesman::io const rw = sesman::io::rw;

    prefix_value disable_prefix_val{"disable"};

    W.section("globals", [&]
    {
        W.member(no_ini_no_gui, type_<bool>(), "capture_chunk", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "auth_user", sesman::name{"login"}, rw);
        W.member(no_ini_no_gui, type_<std::string>(), "host", sesman::name{"ip_client"}, rw);
        W.member(no_ini_no_gui, type_<std::string>(), "target", sesman::name{"ip_target"}, rw);
        W.member(no_ini_no_gui, type_<std::string>(), "target_device", r);
        W.member(no_ini_no_gui, type_<std::string>(), "device_id", r);
        W.member(no_ini_no_gui, type_<std::string>(), "primary_user_id", r);
        W.member(no_ini_no_gui, type_<std::string>(), "target_user", sesman::name{"target_login"}, rw);
        W.member(no_ini_no_gui, type_<std::string>(), "target_application", r);
        W.member(no_ini_no_gui, type_<std::string>(), "target_application_account", r);
        W.member(no_ini_no_gui, type_<std::string>(), "target_application_password", r);
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "bitmap_cache", desc{"Support of Bitmap Cache."}, set(true));
        W.member(advanced_in_gui, type_<bool>(), "glyph_cache", set(false));
        W.member(advanced_in_gui, type_<unsigned>(), "port", set(3389));
        W.member(advanced_in_gui, type_<bool>(), "nomouse", set(false));
        W.member(advanced_in_gui, type_<bool>(), "notimestamp", set(false));
        W.member(advanced_in_gui, type_<Level>(), "encryptionLevel", set(Level::low));
        W.member(advanced_in_gui, type_<std::string>(), "authfile", set("/tmp/redemption-sesman-sock"));
        W.sep();
        W.member(ini_and_gui, type_<std::chrono::seconds>(), "handshake_timeout", desc{"Time out during RDP handshake stage."}, set(10));
        W.member(ini_and_gui, type_<std::chrono::seconds>(), "session_timeout", desc{"No traffic auto disconnection."}, set(900));
        W.member(advanced_in_gui, type_<std::chrono::seconds>(), "keepalive_grace_delay", desc{"Keepalive."}, set(30));
        W.member(advanced_in_gui, type_<std::chrono::seconds>(), "authentication_timeout", desc{"Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate)."}, set(120));
        W.member(advanced_in_gui, type_<std::chrono::seconds>(), "close_timeout", desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."}, set(600));
        W.sep();

        W.member(advanced_in_gui, type_<TraceType>(), "trace_type", set(TraceType::localfile_hashed), r);
        W.sep();

        W.member(advanced_in_gui, type_<types::ip_string>(), "listen_address", set("0.0.0.0"));
        W.member(iptables_in_gui, type_<bool>(), "enable_transparent_mode", desc{"Allow Transparent mode."}, set(false));
        W.member(advanced_in_gui | password_in_gui, type_<types::fixed_string<254>>(), "certificate_password", desc{"Proxy certificate password."}, set("inquisition"));
        W.sep();
        W.member(advanced_in_gui, type_<types::path>(), "png_path", set(CPP_MACRO(PNG_PATH)));
        W.member(advanced_in_gui, type_<types::path>(), "wrm_path", set(CPP_MACRO(WRM_PATH)));
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "is_rec", set(false), r);
        W.member(advanced_in_gui, type_<std::string>(), "movie_path", sesman::name{"rec_path"}, r);
        W.member(advanced_in_gui, type_<bool>(), "enable_bitmap_update", desc{"Support of Bitmap Update."}, set(true));
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "enable_close_box", desc{"Show close screen."}, set(true));
        W.member(advanced_in_gui, type_<bool>(), "enable_osd", set(true));
        W.member(advanced_in_gui, type_<bool>(), "enable_osd_display_remote_target", set(true));
        W.sep();
        W.member(advanced_in_gui, type_<types::path>(), "persistent_path", set(CPP_MACRO(PERSISTENT_PATH)));
        W.sep();
        W.member(hidden_in_gui, type_<bool>(), "enable_wab_integration", set(false));
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "allow_using_multiple_monitors", set(false));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "bogus_refresh_rect", desc{"Needed to refresh screen of Windows Server 2012."}, set(true));
        W.sep();

        W.member(advanced_in_gui, type_<std::string>(), "codec_id", set("flv"));
        W.member(advanced_in_gui, type_<Level>(), "video_quality", set(Level::high));
        W.sep();

        W.member(advanced_in_gui, type_<bool>(), "large_pointer_support", set(true));
    });

    W.section("session_log", [&]
    {
        W.member(ini_and_gui, type_<bool>(), "enable_session_log", set(true));
        W.member(advanced_in_gui, type_<bool>(), "session_log_redirection", desc{"Log redirection in a file"}, set(true), w);
        W.member(advanced_in_gui, type_<std::string>(), "log_path", sesman::name{"session_log_path"}, r);
        W.sep();
        W.member(advanced_in_gui, type_<KeyboardInputMaskingLevel>(), "keyboard_input_masking_level", set(KeyboardInputMaskingLevel::fully_masked));
    });

    W.section("client", [&]
    {
        W.member(no_ini_no_gui, type_<unsigned>(), "keyboard_layout", set(0), w);
        std::string keyboard_layout_proposals_desc;
        for (auto k :  keylayouts) {
            keyboard_layout_proposals_desc += k->locale_name;
            keyboard_layout_proposals_desc += ", ";
        }
        if (!keyboard_layout_proposals_desc.empty()) {
            keyboard_layout_proposals_desc.resize(keyboard_layout_proposals_desc.size() - 2);
        }
        W.member(advanced_in_gui, type_<types::list<std::string>>(), "keyboard_layout_proposals", desc{keyboard_layout_proposals_desc}, set("en-US, fr-FR, de-DE, ru-RU"));
        W.member(advanced_in_gui, type_<bool>(), "ignore_logon_password", desc{"If true, ignore password provided by RDP client, user need do login manually."}, set(false));
        W.sep();
        W.member(advanced_in_gui | hex_in_gui, type_<types::u32>(), "performance_flags_default", desc{"Enable font smoothing (0x80)."}, set(0x80));
        W.member(advanced_in_gui | hex_in_gui, type_<types::u32>(), "performance_flags_force_present", desc{
            "Disable theme (0x8).\n"
            "Disable mouse cursor shadows (0x20)."
        }, set(0x28));
        W.member(advanced_in_gui | hex_in_gui, type_<types::u32>(), "performance_flags_force_not_present", set(0));
        W.member(advanced_in_gui, type_<bool>(), "auto_adjust_performance_flags", desc{"If enabled, avoid automatically font smoothing in recorded session."}, set(true));
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "tls_fallback_legacy", desc{"Fallback to RDP Legacy Encryption if client does not support TLS."}, set(false));
        W.member(ini_and_gui, type_<bool>(), "tls_support", set(true));
        W.member(advanced_in_gui, type_<bool>(), "bogus_neg_request", desc{"Needed to connect with jrdp, based on bogus X224 layer code."}, set(false));
        W.member(advanced_in_gui, type_<bool>(), "bogus_user_id", desc{"Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code."}, set(true));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "disable_tsk_switch_shortcuts", desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."}, set(false), r);
        W.sep();
        W.member(advanced_in_gui, type_<RdpCompression>{}, "rdp_compression", set(RdpCompression::rdp6_1));
        W.sep();
        W.member(advanced_in_gui, type_<ColorDepth>{}, "max_color_depth", set(ColorDepth::depth24));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the front side."}, set(true));
        W.member(advanced_in_gui, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(false));
        W.member(advanced_in_gui, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "bitmap_compression", desc{"Support of Bitmap Compression."}, set(true));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "fast_path", desc{"Enables support of Clent Fast-Path Input Event PDUs."}, set(true));
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "enable_suppress_output", set(true));
        W.sep();
        W.member(advanced_in_gui, type_<std::string>(), "ssl_cipher_list", desc{
            "[Not configured]: Compatible with more RDP clients (less secure)\n"
            "HIGH:!ADH:!3DES:!SHA: Compatible only with MS Windows 2008 R2 client or more recent (more secure)"
        });
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "show_target_user_in_f12_message", set(false));
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "enable_new_pointer_update", set(false));
    });

    W.section("mod_rdp", [&]
    {
        W.member(advanced_in_gui, type_<RdpCompression>{}, "rdp_compression", set(RdpCompression::rdp6_1));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "disconnect_on_logon_user_change", set(false));
        W.sep();
        W.member(advanced_in_gui, type_<std::chrono::seconds>(), "open_session_timeout", set(0));
        W.sep();
        W.member(advanced_in_gui, type_<types::list<unsigned>>(), "extra_orders", desc{
            "Enables support of additional drawing orders:\n"
            "  15: MultiDstBlt\n"
            "  16: MultiPatBlt\n"
            "  17: MultiScrBlt\n"
            "  18: MultiOpaqueRect\n"
            "  22: Polyline"
        }, set("15,16,17,18,22"));
        W.sep();
        W.member(ini_and_gui, type_<bool>(), "enable_nla", desc{"NLA authentication in secondary target."}, set(true));
        W.member(ini_and_gui, type_<bool>(), "enable_kerberos", desc{
            "If enabled, NLA authentication will try Kerberos before NTLM.\n"
            "(if enable_nla is disabled, this value is ignored)."
        }, set(false));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the mod side."}, set(true));
        W.member(advanced_in_gui, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        W.member(advanced_in_gui, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        W.sep();
        W.member(advanced_in_gui, type_<types::list<std::string>>(), "allow_channels", desc{"Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority."}, set("*"));
        W.member(advanced_in_gui, type_<types::list<std::string>>(), "deny_channels", desc{"Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "fast_path", desc{"Enables support of Server Fast-Path Update PDUs."}, set(true));
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "server_redirection_support", desc{"Enables Server Redirection Support."}, set(false));
        W.sep();
        W.member(no_ini_no_gui, type_<RedirectionInfo>(), "redir_info");
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "bogus_sc_net_size", desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."}, sesman::name{"rdp_bogus_sc_net_size"}, set(true), r);
        W.sep();
        W.member(advanced_in_gui, type_<BogusLinuxCursor>(), "bogus_linux_cursor", set(BogusLinuxCursor::smart));
        W.member(advanced_in_gui, type_<types::list<std::string>>(), "proxy_managed_drives", r);
        W.sep();
        W.sep();
        W.member(hidden_in_gui, type_<bool>(), "ignore_auth_channel", set(false), r);
        W.member(ini_and_gui, type_<types::fixed_string<7>>(), "auth_channel", set("*"), desc{"Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel."});
        W.sep();
        W.member(hidden_in_gui, type_<std::string>(), "alternate_shell", r);
        W.member(hidden_in_gui, type_<std::string>(), "shell_arguments", r);
        W.member(hidden_in_gui, type_<std::string>(), "shell_working_directory", r);
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "use_client_provided_alternate_shell", set(false), r);
        W.member(advanced_in_gui, type_<bool>(), "use_client_provided_remoteapp", set(false), r);
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "use_native_remoteapp_capability", set(true), r);
        W.sep();
        W.member(hidden_in_gui, type_<bool>(), "enable_session_probe", sesman::name{"session_probe"}, set(false), r);
        W.member(hidden_in_gui, type_<bool>(), "session_probe_use_smart_launcher", desc{
            "Minimum supported server : Windows Server 2008.\n"
            "Clipboard redirection should be remain enabled on Terminal Server."
        }, cpp::name{"session_probe_use_clipboard_based_launcher"}, set(true), r);
        W.member(hidden_in_gui, type_<bool>(), "session_probe_enable_launch_mask", set(true), r);
        W.member(hidden_in_gui, type_<SessionProbeOnLaunchFailure>(), "session_probe_on_launch_failure", set(SessionProbeOnLaunchFailure::retry_without_session_probe), r);
        W.member(hidden_in_gui, type_<std::chrono::milliseconds>(), "session_probe_launch_timeout", desc{
            "This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).\n"
            "0 to disable timeout."
        }, set(20000), r);
        W.member(hidden_in_gui, type_<std::chrono::milliseconds>(), "session_probe_launch_fallback_timeout", desc{
            "This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).\n"
            "0 to disable timeout."
        }, set(7000), r);
        W.member(hidden_in_gui, type_<bool>(), "session_probe_start_launch_timeout_timer_only_after_logon", desc{
            "Minimum supported server : Windows Server 2008."
        }, set(true), r);
        W.member(hidden_in_gui, type_<std::chrono::milliseconds>(), "session_probe_keepalive_timeout", set(5000), r);
        W.member(hidden_in_gui, type_<bool>(), "session_probe_on_keepalive_timeout_disconnect_user", set(true), r);
        W.member(hidden_in_gui, type_<bool>(), "session_probe_end_disconnected_session", desc{"End automatically a disconnected session"}, set(false), r);
        W.member(advanced_in_gui, type_<bool>(), "session_probe_customize_executable_name", set(false));


        W.member(hidden_in_gui, type_<std::chrono::milliseconds>(), "session_probe_disconnected_application_limit", desc{
            "This policy setting allows you to configure a time limit for disconnected application sessions.\n"
            "0 to disable timeout."
        }, set(0), r);
        W.member(hidden_in_gui, type_<std::chrono::milliseconds>(), "session_probe_disconnected_session_limit", desc{
            "This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.\n"
            "0 to disable timeout."
        }, set(0), r);
        W.member(hidden_in_gui, type_<std::chrono::milliseconds>(), "session_probe_idle_session_limit", desc{
            "This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.\n"
            "0 to disable timeout."
        }, set(0), r);

        W.member(hidden_in_gui, type_<types::fixed_string<511>>(), "session_probe_exe_or_file", set("||CMD"));
        W.member(hidden_in_gui, type_<types::fixed_string<511>>(), "session_probe_arguments", set("/K"));
        W.sep();

        W.member(hidden_in_gui, type_<bool>(), "server_cert_store", desc{"Keep known server certificates on WAB"}, set(true), r);
        W.member(hidden_in_gui, type_<ServerCertCheck>(), "server_cert_check", set(ServerCertCheck::fails_if_no_match_and_succeed_if_no_know), r);

        struct P { char const * name; char const * desc; };
        for (P p : {
            P{"server_access_allowed_message", "Warn if check allow connexion to server."},
            P{"server_cert_create_message", "Warn that new server certificate file was created."},
            P{"server_cert_success_message", "Warn that server certificate file was successfully checked."},
            P{"server_cert_failure_message", "Warn that server certificate file checking failed."},
            P{"server_cert_error_message", "Warn that server certificate check raised some internal error."},
        }) {
            W.member(hidden_in_gui, type_<ServerNotification>(), p.name, desc{p.desc}, set(ServerNotification::syslog), r);
        }
        W.sep();

        W.member(ini_and_gui, type_<bool>(), "hide_client_name", desc{"Do not transmit client machine name or RDP server."}, set(false));
        W.sep();

        W.member(advanced_in_gui, type_<bool>(), "clean_up_32_bpp_cursor", set(false));
        W.sep();
    });

    W.section("mod_vnc", [&]
    {
        W.member(ini_and_gui, type_<bool>(), "clipboard_up", desc{"Enable or disable the clipboard from client (client to server)."}, r);
        W.member(ini_and_gui, type_<bool>(), "clipboard_down", desc{"Enable or disable the clipboard from server (server to client)."}, r);
        W.sep();
        W.member(advanced_in_gui, type_<types::list<int>>(), "encodings", desc{
            "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
            "  0: Raw\n"
            "  1: CopyRect\n"
            "  2: RRE\n"
            "  16: ZRLE\n"
            "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
        });
        W.sep();
        W.member(advanced_in_gui, type_<bool>(), "allow_authentification_retries", set(false));
        W.sep();
        W.member(advanced_in_gui, type_<ClipboardEncodingType>(), "server_clipboard_encoding_type", desc{
            "VNC server clipboard data encoding type."
        }, sesman::name{"vnc_server_clipboard_encoding_type"}, set(ClipboardEncodingType::latin1), r);
        W.sep();
        W.member(advanced_in_gui, type_<VncBogusClipboardInfiniteLoop>(), "bogus_clipboard_infinite_loop", sesman::name{"vnc_bogus_clipboard_infinite_loop"}, set(VncBogusClipboardInfiniteLoop::delayed), r);
    });

    W.section("mod_replay", [&]
    {
        W.member(hidden_in_gui, type_<bool>(), "on_end_of_data", desc{"0 - Wait for Escape, 1 - End session"}, set(0));
    });

    W.section("ocr", [&]
    {
        W.member(ini_and_gui, type_<OcrVersion>(), "version", set(OcrVersion::v2));
        W.member(ini_and_gui, type_<OcrLocale>(), "locale", set(OcrLocale::latin));
        W.member(advanced_in_gui, type_<std::chrono::duration<unsigned, std::centi>>(), "interval", set(100));
        W.member(advanced_in_gui, type_<bool>(), "on_title_bar_only", set(true));
        W.member(advanced_in_gui, type_<types::range<unsigned, 0, 100>>{}, "max_unrecog_char_rate", desc{
            "Expressed in percentage,\n"
          "  0   - all of characters need be recognized\n"
          "  100 - accept all results"
        }, set(40));
    });

    W.section("video", [&]
    {
        W.member(advanced_in_gui, type_<unsigned>(), "capture_groupid", set(33));
        W.sep();
        W.member(advanced_in_gui, type_<CaptureFlags>{}, "capture_flags", set(CaptureFlags::png | CaptureFlags::wrm));
        W.sep();
        W.member(advanced_in_gui, type_<std::chrono::duration<unsigned, std::deci>>(), "png_interval", desc{"Frame interval."}, set(10));
        W.member(advanced_in_gui, type_<std::chrono::duration<unsigned, std::centi>>(), "frame_interval", desc{"Frame interval."}, set(40));
        W.member(advanced_in_gui, type_<std::chrono::seconds>(), "break_interval", desc{"Time between 2 wrm movies."}, set(600));
        W.member(advanced_in_gui, type_<unsigned>(), "png_limit", desc{"Number of png captures to keep."}, set(5));
        W.sep();
        W.member(advanced_in_gui, type_<types::path>(), "replay_path", set("/tmp/"));
        W.sep();
        W.member(advanced_in_gui, type_<types::path>(), "hash_path", set(CPP_MACRO(HASH_PATH)));
        W.member(advanced_in_gui, type_<types::path>(), "record_tmp_path", set(CPP_MACRO(RECORD_TMP_PATH)));
        W.member(advanced_in_gui, type_<types::path>(), "record_path", set(CPP_MACRO(RECORD_PATH)));
        W.sep();
        W.member(no_ini_no_gui, type_<std::chrono::seconds>(), "inactivity_timeout", set(300));
        W.sep();

        W.member(ini_and_gui, type_<KeyboardLogFlags>{}, "disable_keyboard_log", desc{"Disable keyboard log:"}, disable_prefix_val, set(KeyboardLogFlags::syslog));
        W.sep();
        W.member(ini_and_gui, type_<ClipboardLogFlags>(), "disable_clipboard_log", desc{"Disable clipboard log:"}, disable_prefix_val, set(ClipboardLogFlags::syslog));
        W.sep();
        W.member(ini_and_gui, type_<FileSystemLogFlags>(), "disable_file_system_log", desc{"Disable (redirected) file system log:"}, disable_prefix_val, set(FileSystemLogFlags::syslog));
        W.sep();
        W.member(hidden_in_gui, type_<unsigned>(), "rt_display", set(0), r);
        W.sep();
        W.member(advanced_in_gui, type_<ColorDepthSelectionStrategy>{}, "wrm_color_depth_selection_strategy", set(ColorDepthSelectionStrategy::depth16));
        W.member(advanced_in_gui, type_<WrmCompressionAlgorithm>{}, "wrm_compression_algorithm", set(WrmCompressionAlgorithm::gzip));
        W.sep();
        W.member(no_ini_no_gui, type_<std::chrono::seconds>(), "flv_break_interval", set(0));
        W.sep();
        W.member(advanced_in_gui, type_<unsigned>(), "l_bitrate", desc{"Bitrate for low quality."}, set(10000));
        W.member(advanced_in_gui, type_<unsigned>(), "l_framerate", desc{"Framerate for low quality."}, set(5));
        W.member(advanced_in_gui, type_<unsigned>(), "l_height", desc{"Height for low quality."}, set(480));
        W.member(advanced_in_gui, type_<unsigned>(), "l_width", desc{"Width for low quality."}, set(640));
        W.member(advanced_in_gui, type_<unsigned>(), "l_qscale", desc{"Qscale (parameter given to ffmpeg) for low quality."}, set(28));
        W.sep();
        W.member(advanced_in_gui, type_<unsigned>(), "m_bitrate", desc{"Bitrate for medium quality."}, set(20000));
        W.member(advanced_in_gui, type_<unsigned>(), "m_framerate", desc{"Framerate for medium quality."}, set(5));
        W.member(advanced_in_gui, type_<unsigned>(), "m_height", desc{"Height for medium quality."}, set(768));
        W.member(advanced_in_gui, type_<unsigned>(), "m_width", desc{"Width for medium quality."}, set(1024));
        W.member(advanced_in_gui, type_<unsigned>(), "m_qscale", desc{"Qscale (parameter given to ffmpeg) for medium quality."}, set(14));
        W.sep();
        W.member(advanced_in_gui, type_<unsigned>(), "h_bitrate", desc{"Bitrate for high quality."}, set(30000));
        W.member(advanced_in_gui, type_<unsigned>(), "h_framerate", desc{"Framerate for high quality."}, set(5));
        W.member(advanced_in_gui, type_<unsigned>(), "h_height", desc{"Height for high quality."}, set(2048));
        W.member(advanced_in_gui, type_<unsigned>(), "h_width", desc{"Width for high quality."}, set(2048));
        W.member(advanced_in_gui, type_<unsigned>(), "h_qscale", desc{"Qscale (parameter given to ffmpeg) for high quality."}, set(7));
    });

    W.section("crypto", [&]
    {
        constexpr char default_key[] =
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ;
        W.member(hidden_in_gui, type_<types::fixed_binary<32>>(), "encryption_key",
            cpp::name{"key0"}, set(default_key), r);
        W.member(hidden_in_gui, type_<types::fixed_binary<32>>(), "sign_key",
            cpp::name{"key1"}, set(default_key), r);
    });

    W.section("debug", [&]
    {
        W.member(advanced_in_gui, type_<types::u32>(), "x224");
        W.member(advanced_in_gui, type_<types::u32>(), "mcs");
        W.member(advanced_in_gui, type_<types::u32>(), "sec");
        W.member(advanced_in_gui, type_<types::u32>(), "rdp");

        W.member(advanced_in_gui, type_<types::u32>(), "primary_orders");
        W.member(advanced_in_gui, type_<types::u32>(), "secondary_orders");
        W.member(advanced_in_gui, type_<types::u32>(), "bitmap_update");

        W.member(advanced_in_gui, type_<types::u32>(), "bitmap");
        W.member(advanced_in_gui, type_<types::u32>(), "capture");
        W.member(advanced_in_gui, type_<types::u32>(), "auth");
        W.member(advanced_in_gui, type_<types::u32>(), "session");
        W.member(advanced_in_gui, type_<types::u32>(), "front");

        W.member(advanced_in_gui, type_<types::u32>(), "mod_rdp");
        W.member(advanced_in_gui, type_<types::u32>(), "mod_vnc");
        W.member(advanced_in_gui, type_<types::u32>(), "mod_internal");
        W.member(advanced_in_gui, type_<types::u32>(), "mod_xup");

        W.member(advanced_in_gui, type_<types::u32>(), "widget");
        W.member(advanced_in_gui, type_<types::u32>(), "input");
        W.member(advanced_in_gui, type_<types::u32>(), "password");
        W.member(advanced_in_gui, type_<types::u32>(), "compression");
        W.member(advanced_in_gui, type_<types::u32>(), "cache");
        W.member(advanced_in_gui, type_<types::u32>(), "performance");
        W.member(advanced_in_gui, type_<types::u32>(), "pass_dialog_box");
        W.member(advanced_in_gui, type_<types::u32>(), "ocr");
        W.member(advanced_in_gui, type_<types::u32>(), "ffmpeg");
        W.sep();
        W.member(advanced_in_gui, type_<unsigned>(), spec::type_<bool>(), "config", set(2));
    });

    W.section("translation", [&]
    {
        W.member(advanced_in_gui, type_<Language>{}, "language", set(Language::en), r);

        W.member(advanced_in_gui, type_<std::string>(), "password_en", r);
        W.member(advanced_in_gui, type_<std::string>(), "password_fr", r);
    });

    W.section("internal_mod", [&]{
        W.member(advanced_in_gui, type_<std::string>(), "theme", spec::name{"load_theme"});
    });

    W.section("context", [&]
    {
        W.member(no_ini_no_gui, type_<types::path>(), "movie");
        W.sep();
        W.member(no_ini_no_gui, type_<unsigned>(), "opt_bitrate", sesman::name{"bitrate"}, set(40000), r);
        W.member(no_ini_no_gui, type_<unsigned>(), "opt_framerate", sesman::name{"framerate"}, set(5), r);
        W.member(no_ini_no_gui, type_<unsigned>(), "opt_qscale", sesman::name{"qscale"}, set(15), r);
        W.sep();
        W.member(no_ini_no_gui, type_<unsigned>(), "opt_bpp", sesman::name{"bpp"}, set(24), rw);
        W.member(no_ini_no_gui, type_<unsigned>(), "opt_height", sesman::name{"height"}, set(600), rw);
        W.member(no_ini_no_gui, type_<unsigned>(), "opt_width", sesman::name{"width"}, set(800), rw);
        W.sep();
        // auth_error_message is left as std::string type because SocketTransport and ReplayMod
        // take it as argument on constructor and modify it as a std::string
        W.member(no_ini_no_gui, type_<std::string>(), "auth_error_message");
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "selector", set(false), r);
        W.member(no_ini_no_gui, type_<unsigned>(), "selector_current_page", set(1), rw);
        W.member(no_ini_no_gui, type_<std::string>(), "selector_device_filter", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "selector_group_filter", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "selector_proto_filter", rw);
        W.member(no_ini_no_gui, type_<unsigned>(), "selector_lines_per_page", set(0), rw);
        W.member(no_ini_no_gui, type_<unsigned>(), "selector_number_of_pages", set(1), r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "target_password", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "target_host", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "target_str", r);
        W.member(no_ini_no_gui, type_<std::string>(), "target_service", r);
        W.member(no_ini_no_gui, type_<unsigned>(), "target_port", set(3389), rw);
        W.member(no_ini_no_gui, type_<std::string>(), "target_protocol", sesman::name{"proto_dest"}, set("RDP"), r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "password", rw);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "reporting", rw);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "auth_channel_answer", r);
        W.member(no_ini_no_gui, type_<std::string>(), "auth_channel_target", rw);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "message", r);
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "accept_message", w);
        W.member(no_ini_no_gui, type_<bool>(), "display_message", rw);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "rejected", rw);
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "authenticated", set(false), rw);
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "keepalive", set(false), r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "session_id", r);
        W.sep();
        W.member(no_ini_no_gui, type_<unsigned>(), "end_date_cnx", sesman::name{"timeclose"}, set(0), r);
        W.member(no_ini_no_gui, type_<std::string>(), "end_time", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "mode_console", set("allow"), r);
        W.member(no_ini_no_gui, type_<signed>(), "timezone", set(-3600), r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "real_target_device", rw);
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "authentication_challenge", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "ticket", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "comment", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "duration", rw);
        W.member(no_ini_no_gui, type_<std::string>(), "waitinforeturn", rw);
        W.member(no_ini_no_gui, type_<bool>(), "showform", set(false), r);
        W.member(no_ini_no_gui, type_<unsigned>(), "formflag", set(0), w);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "module", set("login"), rw);
        W.member(no_ini_no_gui, type_<bool>(), "forcemodule", set(false), r);
        W.member(no_ini_no_gui, type_<std::string>(), "proxy_opt", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "pattern_kill", r);
        W.member(no_ini_no_gui, type_<std::string>(), "pattern_notify", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "opt_message", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "session_probe_outbound_connection_monitoring_rules", r);
        W.member(no_ini_no_gui, type_<std::string>(), "session_probe_process_monitoring_rules", r);
        W.member(no_ini_no_gui, type_<std::string>(), "session_probe_extra_system_processes", r);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "manager_disconnect_reason");
        W.member(no_ini_no_gui, type_<std::string>(), "disconnect_reason", r);
        W.member(no_ini_no_gui, type_<bool>(), "disconnect_reason_ack", set(false), w);
        W.sep();
        W.member(no_ini_no_gui, type_<std::string>(), "ip_target");
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "recording_started", set(false), w);
        W.sep();
        W.member(no_ini_no_gui, type_<bool>(), "perform_automatic_reconnection", set(false));
        W.member(no_ini_no_gui, type_<types::fixed_binary<28>>(), "server_auto_reconnect_packet");
    });

    W.section("", [&]
    {
        W.member(no_ini_no_gui, type_<Theme>(), "theme");
        W.member(no_ini_no_gui, type_<Font>(), "font");
    });
}

}
