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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_UTILS_CONFIG_SPEC_HPP
#define REDEMPTION_SRC_UTILS_CONFIG_SPEC_HPP

#include "config_types.hpp"

#include <type_traits>
#include <sstream>
#include <vector>
#include <string>
#include <map>


namespace config_spec {

using namespace configs;

struct real_name { char const * name; };
struct str_authid { char const * name; };
struct def_authid { char const * name; };

template<class T> struct type_ { };
template<class T> struct user_type { };

template<class T>
struct default_
{
    using type = T;
    T const & value;
};

template<class T>
default_<T> set(T const & x)
{ return {x}; }

enum class PropertyFieldFlags {
    none,
    read    = 1 << 0,
    write   = 1 << 1,
};

constexpr PropertyFieldFlags operator | (PropertyFieldFlags x, PropertyFieldFlags y) {
    return static_cast<PropertyFieldFlags>(underlying_cast(x) | underlying_cast(y));
}

constexpr PropertyFieldFlags operator & (PropertyFieldFlags x, PropertyFieldFlags y) {
    return static_cast<PropertyFieldFlags>(underlying_cast(x) & underlying_cast(y));
}

struct macro { char const * name; char const * value; };
#define MACRO(name) macro{#name, name}
struct info { char const * value; };
struct todo { char const * value; };
struct desc { char const * value; };

struct uint32_ { uint32_(uint32_t = 0) {} };
struct uint64_ { uint64_(uint64_t = 0) {} };

struct RedirectionInfo {};
struct Theme {};
struct Font {};

enum class Attribute : unsigned {
    none,
    hex = 1 << 1,
    hidden = 1 << 3,
    visible = 1 << 4,
    advanced = 1 << 5,
    iptables = 1 << 6,
};

constexpr Attribute operator | (Attribute x, Attribute y) {
    return static_cast<Attribute>(underlying_cast(x) | underlying_cast(y));
}

constexpr Attribute operator & (Attribute x, Attribute y) {
    return static_cast<Attribute>(underlying_cast(x) & underlying_cast(y));
}

template<class Writer>
void config_spec_definition(Writer && W)
{
    desc const rdp_compression_desc{
        "Specifies the highest compression package support available on the front side:\n"
        "  0: the RDP bulk compression is disabled\n"
        "  1: RDP 4.0 bulk compression\n"
        "  2: RDP 5.0 bulk compression\n"
        "  3: RDP 6.0 bulk compression\n"
        "  4: RDP 6.1 bulk compression"
    };

    using rdp_compression_t = Range<unsigned, 0, 4>;

    desc const rdp_level_desc{"low, medium or high."};

#ifdef IN_IDE_PARSER
    // for coloration...
    struct {
        void member();
        void start_section();
        void stop_section();
        void sep();
    } W;
#endif

    Attribute const X = Attribute::hex;
    Attribute const H = Attribute::hidden;
    Attribute const V = Attribute::visible;
    Attribute const A = Attribute::advanced;
    Attribute const IPT = Attribute::iptables;

    PropertyFieldFlags const w = PropertyFieldFlags::write;
    PropertyFieldFlags const r = PropertyFieldFlags::read;
    PropertyFieldFlags const rw = w | r;

    W.start_section("globals");
    {
        W.member(type_<bool>(), "capture_chunk", r);
        W.sep();
        W.member(type_<std::string>(), "auth_user", str_authid{"login"}, rw);
        W.member(type_<std::string>(), "host", str_authid{"ip_client"}, r);
        W.member(type_<std::string>(), "target", str_authid{"ip_target"}, r);
        W.member(type_<std::string>(), "target_device", r);
        W.member(type_<std::string>(), "target_user", str_authid{"target_login"}, r);
        W.member(type_<std::string>(), "target_application", r);
        W.member(type_<std::string>(), "target_application_account", r);
        W.member(type_<std::string>(), "target_application_password", r);
        W.sep();
        W.member(A, type_<bool>(), "bitmap_cache", desc{"Support of Bitmap Cache."}, set(true));
        W.member(A, type_<bool>(), "glyph_cache", set(false));
        W.member(A, type_<unsigned>(), "port", set(3389));
        W.member(A, type_<bool>(), "nomouse", set(false));
        W.member(A, type_<bool>(), "notimestamp", set(false));
        W.member(A, type_<Level>(), "encryptionLevel", rdp_level_desc, set(Level::low));
        W.member(A, type_<StaticIpString>(), "authip", set("127.0.0.1"));
        W.member(A, type_<unsigned>(), "authport", set(3350));
        W.sep();
        W.member(V, type_<unsigned>(), "session_timeout", desc{"No traffic auto disconnection (in seconds)."}, set(900));
        W.member(H, type_<unsigned>(), "keepalive_grace_delay", desc{"Keepalive (in seconds)."}, set(30));
        W.member(A, type_<unsigned>(), "close_timeout", desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."}, set(600));
        W.sep();
        W.member(V, type_<StaticNilString<8>>(), "auth_channel", set(null_fill()));
        W.member(A, type_<bool>(), "enable_file_encryption", def_authid{"opt_file_encryption"}, str_authid{"file_encryption"}, r);
        W.member(A, type_<StaticIpString>(), "listen_address", set("0.0.0.0"));
        W.member(IPT, type_<bool>(), "enable_ip_transparent", desc{"Allow IP Transparent."}, set(false));
        W.member(V, type_<StaticString<256>>(), "certificate_password", desc{"Proxy certificate password."}, set("inquisition"));
        W.sep();
        W.member(A, type_<StaticString<1024>>(), "png_path", set(MACRO(PNG_PATH)));
        W.member(A, type_<StaticString<1024>>(), "wrm_path", set(MACRO(WRM_PATH)));
        W.sep();
        W.member(H, type_<std::string>(), "alternate_shell", r);
        W.member(H, type_<std::string>(), "shell_working_directory", r);
        W.sep();
        W.member(H, type_<bool>(), "movie", def_authid{"opt_movie"}, str_authid{"is_rec"}, set(false), r);
        W.member(A, type_<std::string>(), "movie_path", def_authid{"opt_movie_path"}, str_authid{"rec_patch"}, r);
        W.member(A, type_<bool>(), "enable_bitmap_update", desc{"Support of Bitmap Update."}, set(true));
        W.sep();
        W.member(V, type_<bool>(), "enable_close_box", desc{"Show close screen."}, set(true));
        W.member(A, type_<bool>(), "enable_osd", set(true));
        W.member(A, type_<bool>(), "enable_osd_display_remote_target", set(true));
        W.sep();
        W.member(A, type_<bool>(), "enable_wab_agent", def_authid{"opt_wabagent"}, str_authid{"wab_agent"}, set(false), r);
        W.member(A, type_<unsigned>(), "wab_agent_launch_timeout", def_authid{"opt_wabagent_launch_timeout"}, set(0), r);
        W.member(A, type_<unsigned>(), "wab_agent_on_launch_failure", def_authid{"opt_wabagent_on_launch_failure"}, set(0), r);
        W.member(A, type_<unsigned>(), "wab_agent_keepalive_timeout", def_authid{"opt_wabagent_keepalive_timeout"}, set(0), r);
        W.sep();
        W.member(H, type_<StaticString<512>>(), "wab_agent_alternate_shell", set(""));
        W.sep();
        W.member(A, type_<StaticPath<1024>>(), "persistent_path", set(MACRO(PERSISTENT_PATH)));
        W.sep();
        W.member(H, type_<bool>(), "disable_proxy_opt", set(false));
        W.member(A, type_<uint32_>(), "max_chunked_virtual_channel_data_length", desc{" The maximum length of the chunked virtual channel data."}, set(2 * 1024 * 1024));
    }
    W.stop_section();

    W.start_section("client");
    {
        W.member(type_<unsigned>(), "keyboard_layout", set(0), r);
        W.member(A, type_<bool>(), "ignore_logon_password", desc{"If true, ignore password provided by RDP client, user need do login manually."}, set(false));
        W.sep();
        W.member(A | X, type_<uint32_>(), "performance_flags_default", set(0));
        W.member(A | X, type_<uint32_>(), "performance_flags_force_present", desc{"Disable theme (0x8)."}, set(0x8));
        W.member(A | X, type_<uint32_>(), "performance_flags_force_not_present", desc{"Disable font smoothing (0x80)."}, set(0x80));
        W.sep();
        W.member(V, type_<bool>(), "tls_fallback_legacy", desc{"Fallback to RDP Legacy Encryption if client does not support TLS."}, set(true));
        W.member(V, type_<bool>(), "tls_support", set(true));
        W.member(A, type_<bool>(), "bogus_neg_request", desc{"Needed to connect with jrdp, based on bogus X224 layer code."}, set(false));
        W.member(A, type_<bool>(), "bogus_user_id", desc{"Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code."}, set(true));
        W.sep();
        W.member(A, type_<bool>(), "disable_tsk_switch_shortcuts", desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."}, set(false), r);
        W.sep();
        W.member(A, type_<rdp_compression_t>{}, "rdp_compression", rdp_compression_desc, set(4));
        W.sep();
        W.member(A, type_<ColorDepth>{}, "max_color_depth", desc{
            "Specifies the maximum color resolution (color depth) for client session:\n"
            "  8: 8 bbp\n"
            "  15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)\n"
            "  16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)\n"
            "  24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)"
        }, set(ColorDepth::depth24));
        W.sep();
        W.member(A, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the front side."}, set(false));
        W.member(A, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        W.member(A, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        W.sep();
        W.member(A, type_<bool>(), "bitmap_compression", desc{"Support of Bitmap Compression."}, set(true));
        W.sep();
        W.member(A, type_<bool>(), "fast_path", desc{"Enables support of Clent Fast-Path Input Event PDUs."}, set(true));
    }
    W.stop_section();

    W.start_section("mod_rdp");
    {
        W.member(A, type_<rdp_compression_t>{}, "rdp_compression", rdp_compression_desc, set(4));
        W.sep();
        W.member(A, type_<bool>(), "disconnect_on_logon_user_change", set(false));
        W.sep();
        W.member(A, type_<uint32_>(), "open_session_timeout", set(0));
        W.sep();
        W.member(A, type_<Range<unsigned, 0, 1>>(), "certificate_change_action", desc{
            "0: Cancel connection and reports error.\n"
            "1: Replace existing certificate and continue connection."
        }, set(0));
        W.sep();
        W.member(A, type_<std::string>(), "extra_orders", desc{
            "Enables support of additional drawing orders:\n"
            "  15: MultiDstBlt\n"
            "  16: MultiPatBlt\n"
            "  17: MultiScrBlt\n"
            "  18: MultiOpaqueRect\n"
            "  22: Polyline"
        }, set("15,16,17,18,22"));
        W.sep();
        W.member(V, type_<bool>(), "enable_nla", desc{"NLA authentication in secondary target."}, set(true));
        W.member(V, type_<bool>(), "enable_kerberos", desc{
            "If enabled, NLA authentication will try Kerberos before NTLM.\n"
            "(if enable_nla is disabled, this value is ignored)."
        }, set(false));
        W.sep();
        W.member(A, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the mod side."}, set(false));
        W.member(A, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        W.member(A, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        W.sep();
        W.member(A, type_<std::string>(), "allow_channels", desc{"Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority."}, set("*"));
        W.member(A, type_<std::string>(), "deny_channels", desc{"Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});
        W.sep();
        W.member(A, type_<bool>(), "fast_path", desc{"Enables support of Server Fast-Path Update PDUs."}, set(true));
        W.sep();
        W.member(A, type_<bool>(), "server_redirection_support", desc{"Enables Server Redirection Support."}, set(false));
        W.sep();
        W.member(type_<RedirectionInfo>(), "redir_info");
        W.sep();
        W.member(A, type_<bool>(), "bogus_sc_net_size", desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."}, def_authid{"rdp_bogus_sc_net_size"}, str_authid{"rdp_bogus_sc_net_size"}, set(true), r);
        W.sep();
        W.member(A, type_<unsigned>(), "client_device_announce_timeout", def_authid{"opt_client_device_announce_timeout"}, set(1000), r);
        W.sep();
        W.member(V, type_<std::string>(), "proxy_managed_drives", def_authid{"opt_proxy_managed_drives"}, r);
    }
    W.stop_section();

    W.start_section("mod_vnc");
    {
        W.member(V, type_<bool>(), "clipboard_up", desc{"Enable or disable the clipboard from client (client to server)."}, def_authid{"vnc_clipboard_up"}, r);
        W.member(V, type_<bool>(), "clipboard_down", desc{"Enable or disable the clipboard from server (server to client)."}, def_authid{"vnc_clipboard_down"}, r);
        W.sep();
        W.member(A, type_<std::string>(), "encodings", desc{
            "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
            "  0: Raw\n"
            "  1: CopyRect\n"
            "  2: RRE\n"
            "  16: ZRLE\n"
            "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
        });
        W.sep();
        W.member(A, type_<bool>(), "allow_authentification_retries", set(false));
        W.sep();
        W.member(A, type_<ClipboardEncodingType>(), "server_clipboard_encoding_type", desc{
            "VNC server clipboard data encoding type.\n"
            "  latin1 (default) or utf-8"
        }, def_authid{"vnc_server_clipboard_encoding_type"}, str_authid{"vnc_server_clipboard_encoding_type"}, set(ClipboardEncodingType::latin1), r);
        W.sep();
        W.member(A, type_<unsigned>(), user_type<Range<unsigned, 0, 2>>(), "bogus_clipboard_infinite_loop", def_authid{"vnc_bogus_clipboard_infinite_loop"}, str_authid{"vnc_bogus_clipboard_infinite_loop"}, set(0), r);
    }
    W.stop_section();

    W.start_section("mod_replay");
    {
        W.member(H, type_<bool>(), "on_end_of_data", desc{"0 - Wait for Escape, 1 - End session"}, set(0));
    }
    W.stop_section();

    W.start_section("video");
    {
        W.member(A, type_<unsigned>(), "capture_groupid", set(33));
        W.sep();
        W.member(A, type_<CaptureFlags>{}, "capture_flags", desc{
            "Specifies the type of data to be captured:\n"
            "  1: PNG\n"
            "  2: WRM"
        }, set(CaptureFlags::png | CaptureFlags::wrm));
        W.sep();
        W.member(A, type_<unsigned>(), "png_interval", desc{"Frame interval is in 1/10 s."}, set(3000));
        W.member(A, type_<unsigned>(), "frame_interval", desc{"Frame interval is in 1/100 s."}, set(40));
        W.member(A, type_<unsigned>(), "break_interval", desc{"Time between 2 wrm movies (in seconds)."}, set(600));
        W.member(A, type_<unsigned>(), "png_limit", desc{"Number of png captures to keep."}, set(5));
        W.sep();
        W.member(A, type_<StaticString<1024>>(), "replay_path", set("/tmp/"));
        W.sep();
        W.member(A, type_<StaticPath<1024>>(), "hash_path", set(MACRO(HASH_PATH)));
        W.member(A, type_<StaticPath<1024>>(), "record_tmp_path", set(MACRO(RECORD_TMP_PATH)));
        W.member(A, type_<StaticPath<1024>>(), "record_path", set(MACRO(RECORD_PATH)));
        W.sep();
        W.member(type_<bool>(), "inactivity_pause", set(false));
        W.member(type_<unsigned>(), "inactivity_timeout", set(300));
        W.sep();
        W.member(V, type_<KeyboardLogFlags>{}, "disable_keyboard_log", desc{
            "Disable keyboard log:\n"
            "  1: disable keyboard log in recorded sessions\n"
        }, r);
        W.sep();
        W.member(V, type_<ClipboardLogFlags>(), "disable_clipboard_log", desc{
            "Disable clipboard log:\n"
            "  1: disable clipboard log in syslog"
        }, r);
        W.sep();
        W.member(H, type_<unsigned>(), "rt_display", set(0), r);
        W.sep();
        W.member(A, type_<Range<unsigned, 0, 1>>{}, "wrm_color_depth_selection_strategy", desc{
            "The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:\n"
            "  0: 24-bit\n"
            "  1: 16-bit"
        }, set(0));
        W.member(A, type_<Range<unsigned, 0, 2>>{}, "wrm_compression_algorithm", desc{
            "The compression method of native video capture:\n"
            "  0: No compression\n"
            "  1: GZip\n"
            "  2: Snappy"
        }, set(0));
    }
    W.stop_section();

    W.start_section("crypto");
    {
        W.member(H, type_<StaticKeyString<32>>(), "key0", set(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ));
        W.member(H, type_<StaticKeyString<32>>(), "key1", set(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ));
    }
    W.stop_section();

    W.start_section("debug");
    {
        W.member(A, type_<uint32_>(), "x224", set(0));
        W.member(A, type_<uint32_>(), "mcs", set(0));
        W.member(A, type_<uint32_>(), "sec", set(0));
        W.member(A, type_<uint32_>(), "rdp", set(0));
        W.member(A, type_<uint32_>(), "primary_orders", set(0));
        W.member(A, type_<uint32_>(), "secondary_orders", set(0));
        W.member(A, type_<uint32_>(), "bitmap", set(0));
        W.member(A, type_<uint32_>(), "capture", set(0));
        W.member(A, type_<uint32_>(), "auth", set(0));
        W.member(A, type_<uint32_>(), "session", set(0));
        W.member(A, type_<uint32_>(), "front", set(0));
        W.member(A, type_<uint32_>(), "mod_rdp", set(0));
        W.member(A, type_<uint32_>(), "mod_vnc", set(0));
        W.member(A, type_<uint32_>(), "mod_int", set(0));
        W.member(A, type_<uint32_>(), "mod_xup", set(0));
        W.member(A, type_<uint32_>(), "widget", set(0));
        W.member(A, type_<uint32_>(), "input", set(0));
        W.member(A, type_<uint32_>(), "password", set(0));
        W.member(A, type_<uint32_>(), "compression", set(0));
        W.member(A, type_<uint32_>(), "cache", set(0));
        W.member(A, type_<uint32_>(), "bitmap_update", set(0));
        W.member(A, type_<uint32_>(), "performance", set(0));
        W.member(A, type_<uint32_>(), "pass_dialog_box", set(0));
        W.sep();
        W.member(A, type_<Range<unsigned, 0, 2>>(), user_type<bool>(), "config", set(2));
    }
    W.stop_section();

    W.start_section("translation");
    {
        W.member(A, type_<Language>{}, "language", set(Language::en), r);
    }
    W.stop_section();

    W.start_section("internal_mod");
    {
        W.member(A, type_<std::string>(), "load_theme", real_name{"theme"}, set(""));
    }
    W.stop_section();

    W.start_section("context");
    {
        W.member(type_<StaticString<1024>>(), "movie");
        W.sep();
        W.member(type_<unsigned>(), "opt_bitrate", str_authid{"bitrate"}, set(40000), r);
        W.member(type_<unsigned>(), "opt_framerate", str_authid{"framerate"}, set(5), r);
        W.member(type_<unsigned>(), "opt_qscale", str_authid{"qscale"}, set(15), r);
        W.sep();
        W.member(type_<unsigned>(), "opt_bpp", str_authid{"bpp"}, set(24), rw);
        W.member(type_<unsigned>(), "opt_height", str_authid{"height"}, set(600), rw);
        W.member(type_<unsigned>(), "opt_width", str_authid{"width"}, set(800), rw);
        W.sep();
        W.member(type_<std::string>(), "auth_error_message", info{
            "auth_error_message is left as std::string type\n"
            "because SocketTransport and ReplayMod take it as argument on\n"
            "constructor and modify it as a std::string"
        }, r);
        W.sep();
        W.member(type_<bool>(), "selector", set(false), r);
        W.member(type_<unsigned>(), "selector_current_page", set(1), rw);
        W.member(type_<std::string>(), "selector_device_filter", r);
        W.member(type_<std::string>(), "selector_group_filter", r);
        W.member(type_<std::string>(), "selector_proto_filter", r);
        W.member(type_<unsigned>(), "selector_lines_per_page", set(0), rw);
        W.member(type_<unsigned>(), "selector_number_of_pages", set(1), r);
        W.sep();
        W.member(type_<std::string>(), "target_password", rw);
        W.member(type_<std::string>(), "target_host", set(""), rw);
        W.member(type_<unsigned>(), "target_port", set(3389), rw);
        W.member(type_<std::string>(), "target_protocol", str_authid{"proto_dest"}, set("RDP"), r);
        W.sep();
        W.member(type_<std::string>(), "password", rw);
        W.sep();
        W.member(type_<std::string>(), "reporting", r);
        W.sep();
        W.member(type_<std::string>(), "auth_channel_answer", r);
        W.member(type_<std::string>(), "auth_channel_result", r);
        W.member(type_<std::string>(), "auth_channel_target", r);
        W.sep();
        W.member(type_<std::string>(), "message", r);
        W.member(type_<std::string>(), "pattern_kill", r);
        W.member(type_<std::string>(), "pattern_notify", r);
        W.sep();
        W.member(type_<std::string>(), "accept_message", todo{"why are the field below Strings ? They should be booleans. As they can only contain True/False to know if a user clicked on a button"}, w);
        W.member(type_<std::string>(), "display_message", w);
        W.sep();
        W.member(type_<std::string>(), "rejected", r);
        W.sep();
        W.member(type_<bool>(), "authenticated", set(false), r);
        W.sep();
        W.member(type_<bool>(), "keepalive", set(false), r);
        W.sep();
        W.member(type_<std::string>(), "session_id", r);
        W.sep();
        W.member(type_<unsigned>(), "end_date_cnx", str_authid{"timeclose"}, set(0), r);
        W.member(type_<std::string>(), "end_time", r);
        W.sep();
        W.member(type_<std::string>(), "mode_console", set("allow"), r);
        W.member(type_<signed>(), "timezone", set(-3600), r);
        W.sep();
        W.member(type_<std::string>(), "real_target_device", r);
        W.sep();
        W.member(type_<bool>(), "authentication_challenge", r);
        W.sep();
        W.member(type_<std::string>(), "ticket", set(""), r);
        W.member(type_<std::string>(), "comment", set(""), r);
        W.member(type_<std::string>(), "duration", set(""), r);
        W.member(type_<std::string>(), "waitinforeturn", set(""), r);
        W.member(type_<bool>(), "showform", set(false), r);
        W.member(type_<unsigned>(), "formflag", set(0), w);
        W.sep();
        W.member(type_<std::string>(), "module", set("login"), rw);
        W.member(type_<bool>(), "forcemodule", set(false), r);
        W.member(type_<std::string>(), "proxy_opt", r);
    }
    W.stop_section();

    W.start_section("");
    W.member(type_<Theme>(), "theme");
    W.member(type_<Font>(), "font");
    W.stop_section();
}


template<class T>
struct ref
{
    T const & x;
    operator T const & () const { return x; }
};

template<class Inherit>
struct ConfigSpecWriterBase
{
    std::string section_name;
    unsigned depth = 0;
    std::ostringstream out_member_;
    std::ostream * out_;

    std::map<std::string, std::string> sections_member;

    std::ostream & out() {
        return *this->out_;
    }

    void start_section(std::string name) {
        this->out_ = &this->out_member_;
        this->section_name = std::move(name);
        if (!this->section_name.empty()) {
            ++this->depth;
        }
        this->inherit().do_start_section();
    }

    void stop_section() {
        this->out_ = &this->out_member_;
        if (!this->section_name.empty()) {
            --this->depth;
        }
        this->sections_member[this->section_name] += this->out_member_.str();
        this->out_member_.str("");
        this->inherit().do_stop_section();
    }

    void sep() { this->inherit().do_sep(); }
    void tab() { this->inherit().do_tab(); }

    template<class Pack, class To>
    typename std::enable_if<std::is_convertible<Pack, To>::value>::type
    write_if_convertible(Pack const & x, type_<To>)
    { this->inherit().write(static_cast<To const &>(x)); }

    template<class Pack, class To>
    void write_if_convertible(Pack const &, To)
    { }

    void write_comment(char const * start_line_comment, char const * s) {
        auto p = s;
        while (*s) {
            while (*p && *p != '\n') {
                ++p;
            }
            if (*p == '\n') {
                ++p;
            }
            this->tab();
            this->out() << start_line_comment << " ";
            this->out().write(s, p-s);
            s = p;
        }
        this->out() << "\n";
    }

    void write_key(char const * k, std::size_t n, char const * prefix = "") {
        int c;
        for (const char * e = k + n; k != e; ++k) {
            this->out() << prefix;
            c = (*k >> 4);
            c += (c > 9) ? 'A' - 10 : '0';
            this->out() << char(c);
            c = (*k & 0xf);
            c += (c > 9) ? 'A' - 10 : '0';
            this->out() << char(c);
        }
    }

protected:
    Inherit & inherit() {
        return static_cast<Inherit&>(*this);
    }

private:
    void do_start_section() {}
    void do_stop_section() {}
    void do_sep() {}
    void do_tab() {}
};

#define MK_PACK(Ts)                           \
    struct Pack : ::config_spec::ref<Ts>... { \
        explicit Pack(Ts const &... x)        \
        : ::config_spec::ref<Ts>{x}...        \
        {}                                    \
    }


}

#endif
