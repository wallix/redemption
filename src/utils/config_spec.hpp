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

struct link { constexpr link() {} };
struct attach { constexpr attach() {} };
struct ask { constexpr ask() {} };
struct use { constexpr use() {} };

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
void config_spec_definition(Writer && w)
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
    } w;
#endif

    Attribute const X = Attribute::hex;
    Attribute const H = Attribute::hidden;
    Attribute const V = Attribute::visible;
    Attribute const A = Attribute::advanced;
    Attribute const IPT = Attribute::iptables;

    link const linked;
    attach const attached;
    ask const asked;
    use const used;

    w.start_section("globals");
    {
        w.member(type_<BoolField>(), "capture_chunk");
        w.sep();
        w.member(type_<StringField>(), "auth_user", linked, attached, asked, str_authid{"login"});
        w.member(type_<StringField>(), "host", linked, attached, str_authid{"ip_client"});
        w.member(type_<StringField>(), "target", linked, attached, str_authid{"ip_target"});
        w.member(type_<StringField>(), "target_device", linked, attached, asked);
        w.member(type_<StringField>(), "target_user", linked, attached, asked, str_authid{"target_login"});
        w.member(type_<StringField>(), "target_application", attached);
        w.member(type_<StringField>(), "target_application_account", attached);
        w.member(type_<StringField>(), "target_application_password", attached);
        w.sep();
        w.member(A, type_<bool>(), "bitmap_cache", desc{"Support of Bitmap Cache."}, set(true));
        w.member(A, type_<bool>(), "glyph_cache", set(false));
        w.member(A, type_<unsigned>(), "port", set(3389));
        w.member(A, type_<bool>(), "nomouse", set(false));
        w.member(A, type_<bool>(), "notimestamp", set(false));
        w.member(A, type_<Level>(), "encryptionLevel", rdp_level_desc, set(Level::low));
        w.member(A, type_<StaticIpString>(), "authip", set("127.0.0.1"));
        w.member(A, type_<unsigned>(), "authport", set(3350));
        w.sep();
        w.member(V, type_<unsigned>(), "session_timeout", desc{"No traffic auto disconnection (in seconds)."}, set(900));
        w.member(H, type_<unsigned>(), "keepalive_grace_delay", desc{"Keepalive (in seconds)."}, set(30));
        w.member(A, type_<unsigned>(), "close_timeout", desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."}, set(600));
        w.sep();
        w.member(V, type_<StaticNilString<8>>(), "auth_channel", set(null_fill()));
        w.member(A, type_<BoolField>(), "enable_file_encryption", attached, def_authid{"opt_file_encryption"}, str_authid{"file_encryption"});
        w.member(A, type_<StaticIpString>(), "listen_address", set("0.0.0.0"));
        w.member(IPT, type_<bool>(), "enable_ip_transparent", desc{"Allow IP Transparent."}, set(false));
        w.member(V, type_<StaticString<256>>(), "certificate_password", desc{"Proxy certificate password."}, set("inquisition"));
        w.sep();
        w.member(A, type_<StaticString<1024>>(), "png_path", set(MACRO(PNG_PATH)));
        w.member(A, type_<StaticString<1024>>(), "wrm_path", set(MACRO(WRM_PATH)));
        w.sep();
        w.member(H, type_<StringField>(), "alternate_shell", attached);
        w.member(H, type_<StringField>(), "shell_working_directory", attached);
        w.sep();
        w.member(H, type_<BoolField>(), "movie", attached, set(false), def_authid{"opt_movie"}, str_authid{"is_rec"});
        w.member(A, type_<StringField>(), "movie_path", attached, def_authid{"opt_movie_path"}, str_authid{"rec_patch"});
        w.member(A, type_<bool>(), "enable_bitmap_update", desc{"Support of Bitmap Update."}, set(true));
        w.sep();
        w.member(V, type_<bool>(), "enable_close_box", desc{"Show close screen."}, set(true));
        w.member(A, type_<bool>(), "enable_osd", set(true));
        w.member(A, type_<bool>(), "enable_osd_display_remote_target", set(true));
        w.sep();
        w.member(A, type_<BoolField>(), "enable_wab_agent", attached, set(false), def_authid{"opt_wabagent"}, str_authid{"wab_agent"});
        w.member(A, type_<UnsignedField>(), "wab_agent_launch_timeout", attached, set(0), def_authid{"opt_wabagent_launch_timeout"});
        w.member(A, type_<UnsignedField>(), "wab_agent_keepalive_timeout", attached, set(0), def_authid{"opt_wabagent_keepalive_timeout"});
        w.sep();
        w.member(H, type_<StaticString<512>>(), "wab_agent_alternate_shell", set(""));
        w.sep();
        w.member(A, type_<StaticPath<1024>>(), "persistent_path", set(MACRO(PERSISTENT_PATH)));
        w.sep();
        w.member(H, type_<bool>(), "disable_proxy_opt", set(false));
        w.member(A, type_<uint32_>(), "max_chunked_virtual_channel_data_length", desc{" The maximum length of the chunked virtual channel data."}, set(2 * 1024 * 1024));
    }
    w.stop_section();

    w.start_section("client");
    {
        w.member(type_<UnsignedField>(), "keyboard_layout", attached, set(0), linked);
        w.member(A, type_<bool>(), "ignore_logon_password", desc{"If true, ignore password provided by RDP client, user need do login manually."}, set(false));
        w.sep();
        w.member(A | X, type_<uint32_>(), "performance_flags_default", set(0));
        w.member(A | X, type_<uint32_>(), "performance_flags_force_present", desc{"Disable theme (0x8)."}, set(0x8));
        w.member(A | X, type_<uint32_>(), "performance_flags_force_not_present", desc{"Disable font smoothing (0x80)."}, set(0x80));
        w.sep();
        w.member(V, type_<bool>(), "tls_fallback_legacy", desc{"Fallback to RDP Legacy Encryption if client does not support TLS."}, set(true));
        w.member(V, type_<bool>(), "tls_support", set(true));
        w.member(A, type_<bool>(), "bogus_neg_request", desc{"Needed to connect with jrdp, based on bogus X224 layer code."}, set(false));
        w.member(A, type_<bool>(), "bogus_user_id", desc{"Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code."}, set(true));
        w.sep();
        w.member(A, type_<BoolField>(), "disable_tsk_switch_shortcuts", desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."}, attached, set(false));
        w.sep();
        w.member(A, type_<rdp_compression_t>{}, "rdp_compression", rdp_compression_desc, set(4));
        w.sep();
        w.member(A, type_<ColorDepth>{}, "max_color_depth", desc{
            "Specifies the maximum color resolution (color depth) for client session:\n"
            "  8: 8 bbp\n"
            "  15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)\n"
            "  16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)\n"
            "  24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)"
        }, set(ColorDepth::depth24));
        w.sep();
        w.member(A, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the front side."}, set(false));
        w.member(A, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        w.member(A, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        w.sep();
        w.member(A, type_<bool>(), "bitmap_compression", desc{"Support of Bitmap Compression."}, set(true));
        w.sep();
        w.member(A, type_<bool>(), "fast_path", desc{"Enables support of Clent Fast-Path Input Event PDUs."}, set(true));
    }
    w.stop_section();

    w.start_section("mod_rdp");
    {
        w.member(A, type_<rdp_compression_t>{}, "rdp_compression", rdp_compression_desc, set(4));
        w.sep();
        w.member(A, type_<bool>(), "disconnect_on_logon_user_change", set(false));
        w.sep();
        w.member(A, type_<uint32_>(), "open_session_timeout", set(0));
        w.sep();
        w.member(A, type_<Range<unsigned, 0, 1>>(), "certificate_change_action", desc{
            "0: Cancel connection and reports error.\n"
            "1: Replace existing certificate and continue connection."
        }, set(0));
        w.sep();
        w.member(A, type_<std::string>(), "extra_orders", desc{
            "Enables support of additional drawing orders:\n"
            "  15: MultiDstBlt\n"
            "  16: MultiPatBlt\n"
            "  17: MultiScrBlt\n"
            "  18: MultiOpaqueRect\n"
            "  22: Polyline"
        }, set("15,16,17,18,22"));
        w.sep();
        w.member(V, type_<bool>(), "enable_nla", desc{"NLA authentication in secondary target."}, set(true));
        w.member(V, type_<bool>(), "enable_kerberos", desc{
            "If enabled, NLA authentication will try Kerberos before NTLM.\n"
            "(if enable_nla is disabled, this value is ignored)."
        }, set(false));
        w.sep();
        w.member(A, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the mod side."}, set(false));
        w.member(A, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        w.member(A, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        w.sep();
        w.member(A, type_<std::string>(), "allow_channels", desc{"Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority."}, set("*"));
        w.member(A, type_<std::string>(), "deny_channels", desc{"Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});
        w.sep();
        w.member(A, type_<bool>(), "fast_path", desc{"Enables support of Server Fast-Path Update PDUs."}, set(true));
        w.sep();
        w.member(A, type_<bool>(), "server_redirection_support", desc{"Enables Server Redirection Support."}, set(false));
        w.sep();
        w.member(type_<RedirectionInfo>(), "redir_info");
        w.sep();
        w.member(A, type_<BoolField>(), "bogus_sc_net_size", desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."}, attached, set(true), def_authid{"rdp_bogus_sc_net_size"}, str_authid{"rdp_bogus_sc_net_size"});
        w.sep();
        w.member(A, type_<UnsignedField>(), "client_device_announce_timeout", attached, set(1000), def_authid{"opt_client_device_announce_timeout"});
        w.sep();
        w.member(V, type_<StringField>(), "proxy_managed_drives", attached, def_authid{"opt_proxy_managed_drives"});
    }
    w.stop_section();

    w.start_section("mod_vnc");
    {
        w.member(V, type_<BoolField>(), "clipboard_up", desc{"Enable or disable the clipboard from client (client to server)."}, attached, def_authid{"vnc_clipboard_up"});
        w.member(V, type_<BoolField>(), "clipboard_down", desc{"Enable or disable the clipboard from server (server to client)."}, attached, def_authid{"vnc_clipboard_down"});
        w.sep();
        w.member(A, type_<std::string>(), "encodings", desc{
            "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
            "  0: Raw\n"
            "  1: CopyRect\n"
            "  2: RRE\n"
            "  16: ZRLE\n"
            "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
        });
        w.sep();
        w.member(A, type_<bool>(), "allow_authentification_retries", set(false));
        w.sep();
        w.member(A, type_<ClipboardEncodingTypeField>(), "server_clipboard_encoding_type", desc{
            "VNC server clipboard data encoding type.\n"
            "  latin1 (default) or utf-8"
        }, attached, set(ClipboardEncodingType::latin1), def_authid{"vnc_server_clipboard_encoding_type"}, str_authid{"vnc_server_clipboard_encoding_type"});
        w.sep();
        w.member(A, type_<UnsignedField>(), user_type<Range<unsigned, 0, 2>>(), "bogus_clipboard_infinite_loop", attached, set(0), def_authid{"vnc_bogus_clipboard_infinite_loop"}, str_authid{"vnc_bogus_clipboard_infinite_loop"});
    }
    w.stop_section();

    w.start_section("mod_replay");
    {
        w.member(A, type_<Range<int, 0, 1>>(), "on_end_of_data", desc{"0 - Wait for Escape, 1 - End session"}, set(0));
    }
    w.stop_section();

    w.start_section("video");
    {
        w.member(A, type_<unsigned>(), "capture_groupid", set(33));
        w.sep();
        w.member(A, type_<CaptureFlags>{}, "capture_flags", desc{
            "Specifies the type of data to be captured:\n"
            "  1: PNG\n"
            "  2: WRM"
        }, set(CaptureFlags::png | CaptureFlags::wrm));
        w.sep();
        w.member(A, type_<unsigned>(), "png_interval", desc{"Frame interval is in 1/10 s."}, set(3000));
        w.member(A, type_<unsigned>(), "frame_interval", desc{"Frame interval is in 1/100 s."}, set(40));
        w.member(A, type_<unsigned>(), "break_interval", desc{"Time between 2 wrm movies (in seconds)."}, set(600));
        w.member(A, type_<unsigned>(), "png_limit", desc{"Number of png captures to keep."}, set(5));
        w.sep();
        w.member(A, type_<StaticString<1024>>(), "replay_path", set("/tmp/"));
        w.sep();
        w.member(A, type_<StaticPath<1024>>(), "hash_path", set(MACRO(HASH_PATH)));
        w.member(A, type_<StaticPath<1024>>(), "record_tmp_path", set(MACRO(RECORD_TMP_PATH)));
        w.member(A, type_<StaticPath<1024>>(), "record_path", set(MACRO(RECORD_PATH)));
        w.sep();
        w.member(type_<bool>(), "inactivity_pause", set(false));
        w.member(type_<unsigned>(), "inactivity_timeout", set(300));
        w.sep();
        w.member(V, type_<KeyboardLogFlagsField>{}, "disable_keyboard_log", desc{
            "Disable keyboard log:\n"
            "  1: disable keyboard log in recorded sessions\n"
        }, attached, linked);
        w.sep();
        w.member(V, type_<ClipboardLogFlagsField>(), "disable_clipboard_log", desc{
            "Disable clipboard log:\n"
            "  1: disable clipboard log in syslog"
        }, attached);
        w.sep();
        w.member(H, type_<UnsignedField>(), "rt_display", attached, set(0));
        w.sep();
        w.member(A, type_<Range<unsigned, 0, 1>>{}, "wrm_color_depth_selection_strategy", desc{
            "The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:\n"
            "  0: 24-bit\n"
            "  1: 16-bit"
        }, set(0));
        w.member(A, type_<Range<unsigned, 0, 2>>{}, "wrm_compression_algorithm", desc{
            "The compression method of native video capture:\n"
            "  0: No compression\n"
            "  1: GZip\n"
            "  2: Snappy"
        }, set(0));
    }
    w.stop_section();

    w.start_section("crypto");
    {
        w.member(H, type_<StaticKeyString<32>>(), "key0", set(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ));
        w.member(H, type_<StaticKeyString<32>>(), "key1", set(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ));
    }
    w.stop_section();

    w.start_section("debug");
    {
        w.member(A, type_<uint32_>(), "x224", set(0));
        w.member(A, type_<uint32_>(), "mcs", set(0));
        w.member(A, type_<uint32_>(), "sec", set(0));
        w.member(A, type_<uint32_>(), "rdp", set(0));
        w.member(A, type_<uint32_>(), "primary_orders", set(0));
        w.member(A, type_<uint32_>(), "secondary_orders", set(0));
        w.member(A, type_<uint32_>(), "bitmap", set(0));
        w.member(A, type_<uint32_>(), "capture", set(0));
        w.member(A, type_<uint32_>(), "auth", set(0));
        w.member(A, type_<uint32_>(), "session", set(0));
        w.member(A, type_<uint32_>(), "front", set(0));
        w.member(A, type_<uint32_>(), "mod_rdp", set(0));
        w.member(A, type_<uint32_>(), "mod_vnc", set(0));
        w.member(A, type_<uint32_>(), "mod_int", set(0));
        w.member(A, type_<uint32_>(), "mod_xup", set(0));
        w.member(A, type_<uint32_>(), "widget", set(0));
        w.member(A, type_<uint32_>(), "input", set(0));
        w.member(A, type_<uint32_>(), "password", set(0));
        w.member(A, type_<uint32_>(), "compression", set(0));
        w.member(A, type_<uint32_>(), "cache", set(0));
        w.member(A, type_<uint32_>(), "bitmap_update", set(0));
        w.member(A, type_<uint32_>(), "performance", set(0));
        w.member(A, type_<uint32_>(), "pass_dialog_box", set(0));
        w.sep();
        w.member(A, type_<Range<unsigned, 0, 2>>(), user_type<bool>(), "config", set(2));
    }
    w.stop_section();

    w.start_section("translation");
    {
        w.member(A, type_<LanguageField>{}, "language", set(Language::en), attached);
    }
    w.stop_section();

    w.start_section("internal_mod");
    {
        w.member(A, type_<std::string>(), "load_theme", real_name{"theme"}, set(""));
    }
    w.stop_section();

    w.start_section("context");
    {
        w.member(type_<StaticString<1024>>(), "movie");
        w.sep();
        w.member(type_<UnsignedField>(), "opt_bitrate", set(40000), attached, str_authid{"bitrate"});
        w.member(type_<UnsignedField>(), "opt_framerate", set(5), attached, str_authid{"framerate"});
        w.member(type_<UnsignedField>(), "opt_qscale", set(15), attached, str_authid{"qscale"});
        w.sep();
        w.member(type_<UnsignedField>(), "opt_bpp", linked, set(24), attached, str_authid{"bpp"});
        w.member(type_<UnsignedField>(), "opt_height", linked, set(600), attached, str_authid{"height"});
        w.member(type_<UnsignedField>(), "opt_width", linked, set(800), attached, str_authid{"width"});
        w.sep();
        w.member(type_<ReadOnlyStringField>(), "auth_error_message", info{
            "auth_error_message is left as std::string type\n"
            "because SocketTransport and ReplayMod take it as argument on\n"
            "constructor and modify it as a std::string"
        }, attached, used);
        w.sep();
        w.member(type_<BoolField>(), "selector", linked, set(false), attached);
        w.member(type_<UnsignedField>(), "selector_current_page", linked, set(1), attached);
        w.member(type_<StringField>(), "selector_device_filter", linked, attached);
        w.member(type_<StringField>(), "selector_group_filter", linked, attached);
        w.member(type_<StringField>(), "selector_proto_filter", linked, attached);
        w.member(type_<UnsignedField>(), "selector_lines_per_page", linked, set(0), attached);
        w.member(type_<UnsignedField>(), "selector_number_of_pages", set(1), attached);
        w.sep();
        w.member(type_<StringField>(), "target_password", linked, asked, attached);
        w.member(type_<StringField>(), "target_host", linked, set(""), asked, attached);
        w.member(type_<UnsignedField>(), "target_port", set(3389), asked, attached);
        w.member(type_<StringField>(), "target_protocol", linked, set("RDP"), asked, attached, str_authid{"proto_dest"});
        w.sep();
        w.member(type_<StringField>(), "password", linked, asked, attached);
        w.sep();
        w.member(type_<StringField>(), "reporting", linked, attached);
        w.sep();
        w.member(type_<StringField>(), "auth_channel_answer", attached);
        w.member(type_<StringField>(), "auth_channel_result", linked, attached);
        w.member(type_<StringField>(), "auth_channel_target", linked, attached);
        w.sep();
        w.member(type_<StringField>(), "message", attached);
        w.member(type_<StringField>(), "pattern_kill", attached);
        w.member(type_<StringField>(), "pattern_notify", attached);
        w.sep();
        w.member(type_<StringField>(), "accept_message", todo{"why are the field below Strings ? They should be booleans. As they can only contain True/False to know if a user clicked on a button"}, linked, attached);
        w.member(type_<StringField>(), "display_message", linked, attached);
        w.sep();
        w.member(type_<StringField>(), "rejected", attached);
        w.sep();
        w.member(type_<BoolField>(), "authenticated", set(false), attached);
        w.sep();
        w.member(type_<BoolField>(), "keepalive", set(false), attached, linked);
        w.sep();
        w.member(type_<StringField>(), "session_id", attached);
        w.sep();
        w.member(type_<UnsignedField>(), "end_date_cnx", set(0), attached, str_authid{"timeclose"});
        w.member(type_<StringField>(), "end_time", attached);
        w.sep();
        w.member(type_<StringField>(), "mode_console", set("allow"), attached);
        w.member(type_<SignedField>(), "timezone", set(-3600), attached);
        w.sep();
        w.member(type_<StringField>(), "real_target_device", linked, attached);
        w.sep();
        w.member(type_<BoolField>(), "authentication_challenge", asked, attached);
        w.sep();
        w.member(type_<StringField>(), "ticket", linked, set(""), attached, used);
        w.member(type_<StringField>(), "comment", linked, set(""), attached, used);
        w.member(type_<StringField>(), "duration", linked, set(""), attached, used);
        w.member(type_<StringField>(), "waitinforeturn", linked, set(""), attached, used);
        w.member(type_<BoolField>(), "showform", linked, set(false), attached, used);
        w.member(type_<UnsignedField>(), "formflag", linked, set(0), attached, used);
        w.sep();
        w.member(type_<StringField>(), "module", linked, set("login"), attached, used);
        w.member(type_<BoolField>(), "forcemodule", set(false), attached);
        w.member(type_<StringField>(), "proxy_opt", attached);
    }
    w.stop_section();

    w.start_section("");
    w.member(type_<Theme>(), "theme");
    w.member(type_<IniAccounts>(), "account");
    w.member(type_<Font>(), "font");
    w.stop_section();
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

template<class T, class Result = void>
using enable_if_basefield = typename std::enable_if<
    std::is_base_of<FieldObserver::BaseField, T>::value,
    Result
>::type;

template<class T, class Result = void>
using disable_if_basefield = typename std::enable_if<
    !std::is_base_of<FieldObserver::BaseField, T>::value,
    Result
>::type;

#define MK_PACK(Ts)                           \
    struct Pack : ::config_spec::ref<Ts>... { \
        explicit Pack(Ts const &... x)        \
        : ::config_spec::ref<Ts>{x}...        \
        {}                                    \
    }


}

#endif
