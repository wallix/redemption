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

#include <iosfwd>
#include <vector>

#include "configs/types.hpp"

#include "theme.hpp"
#include "font.hpp"
#include "redirection_info.hpp"

namespace config_spec {

using namespace configs;

struct real_name { char const * name; };
struct str_authid { char const * name; };
struct def_authid { char const * name; };

template<class T>
struct type_ { };

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

struct expr { char const * value; };
struct info { char const * value; };
struct todo { char const * value; };
struct desc { char const * value; };

struct uint32_ {};
struct uint64_ {};

enum class Attribute {
    none,
    hex = 1 << 1,
    hidden = 1 << 3,
    visible = 1 << 4,
    advanced = 1 << 5,
    ip_tables = 1 << 6,
};

constexpr Attribute operator | (Attribute x, Attribute y) {
    return static_cast<Attribute>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
}

constexpr Attribute operator & (Attribute x, Attribute y) {
    return static_cast<Attribute>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
}

template<class Writer>
void writer_config_spec(Writer && w)
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
        void field();
        void start_section();
        void stop_section();
        void sep();
    } w;
#endif

    Attribute const X = Attribute::hex;
    Attribute const H = Attribute::hidden;
    Attribute const V = Attribute::visible;
    Attribute const A = Attribute::advanced;
    Attribute const IPT = Attribute::ip_tables;

    link const linked;
    attach const attached;
    ask const asked;
    use const used;

    w.start_section("globals");
    {
        w.field(type_<BoolField>(), "capture_chunk");
        w.sep();
        w.field(type_<StringField>(), "auth_user", linked, attached, asked, str_authid{"login"});
        w.field(type_<StringField>(), "host", linked, attached, str_authid{"ip_client"});
        w.field(type_<StringField>(), "target", linked, attached, str_authid{"ip_target"});
        w.field(type_<StringField>(), "target_device", linked, attached, asked);
        w.field(type_<StringField>(), "target_user", linked, attached, asked, str_authid{"target_login"});
        w.field(type_<StringField>(), "target_application", attached);
        w.field(type_<StringField>(), "target_application_account", attached);
        w.field(type_<StringField>(), "target_application_password", attached);
        w.sep();
        w.field(A, type_<bool>(), "bitmap_cache", desc{"Support of Bitmap Cache."}, set(true));
        w.field(A, type_<bool>(), "glyph_cache", set(false));
        w.field(A, type_<unsigned>(), "port", set(3389));
        w.field(A, type_<bool>(), "nomouse", set(false));
        w.field(A, type_<bool>(), "notimestamp", set(false));
        w.field(A, type_<Level>(), "encryptionLevel", rdp_level_desc, set(Level::low));
        w.field(A, type_<StaticIpString>(), "authip", set("127.0.0.1"));
        w.field(A, type_<unsigned>(), "authport", set(3350));
        w.sep();
        w.field(V, type_<unsigned>(), "session_timeout", desc{"No traffic auto disconnection (in seconds)."}, set(900));
        w.field(H, type_<unsigned>(), "keepalive_grace_delay", desc{"Keepalive (in seconds)."}, set(30));
        w.field(A, type_<unsigned>(), "close_timeout", desc{"Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate)."}, set(600));
        w.sep();
        w.field(V, type_<StaticNilString<8>>(), "auth_channel", set(null_fill()));
        w.field(A, type_<BoolField>(), "enable_file_encryption", attached, def_authid{"opt_file_encryption"}, str_authid{"file_encryption"});
        w.field(A, type_<StaticIpString>(), "listen_address", set("0.0.0.0"));
        w.field(IPT, type_<bool>(), "enable_ip_transparent", desc{"Allow IP Transparent."}, set(false));
        w.field(V, type_<StaticString<256>>(), "certificate_password", desc{"Proxy certificate password."}, set("inquisition"));
        w.sep();
        w.field(A, type_<StaticString<1024>>(), "png_path", set(expr{"PNG_PATH"}));
        w.field(A, type_<StaticString<1024>>(), "wrm_path", set(expr{"WRM_PATH"}));
        w.sep();
        w.field(H, type_<StringField>(), "alternate_shell", attached);
        w.field(H, type_<StringField>(), "shell_working_directory", attached);
        w.sep();
        w.field(A, type_<StringField>(), "codec_id", attached, set("flv"), def_authid{"opt_codec_id"});
        w.field(H, type_<BoolField>(), "movie", attached, set(false), def_authid{"opt_movie"}, str_authid{"is_rec"});
        w.field(A, type_<StringField>(), "movie_path", attached, def_authid{"opt_movie_path"}, str_authid{"rec_patch"});
        w.field(A, type_<LevelField>(), "video_quality", rdp_level_desc, todo{"this could be some kind of enumeration"}, attached, set(Level::medium));
        w.field(A, type_<bool>(), "enable_bitmap_update", desc{"Support of Bitmap Update."}, set(true));
        w.sep();
        w.field(V, type_<bool>(), "enable_close_box", desc{"Show close screen."}, set(true));
        w.field(A, type_<bool>(), "enable_osd", set(true));
        w.field(V, type_<bool>(), "enable_osd_display_remote_target", set(true));
        w.sep();
        w.field(A, type_<BoolField>(), "enable_wab_agent", attached, set(false), def_authid{"opt_wabagent"}, str_authid{"wab_agent"});
        w.field(A, type_<UnsignedField>(), "wab_agent_launch_timeout", attached, set(0), def_authid{"opt_wabagent_launch_timeout"});
        w.field(A, type_<UnsignedField>(), "wab_agent_keepalive_timeout", attached, set(0), def_authid{"opt_wabagent_keepalive_timeout"});
        w.sep();
        w.field(type_<StaticString<512>>(), "wab_agent_alternate_shell", set(""));
        w.sep();
        w.field(A, type_<StaticPath<1024>>(), "persistent_path", set(expr{"PERSISTENT_PATH"}));
        w.sep();
        w.field(H, type_<bool>(), "disable_proxy_opt", set(false));
        w.field(A, type_<uint32_>(), "max_chunked_virtual_channel_data_length", desc{" The maximum length of the chunked virtual channel data."}, set(expr{"2 * 1024 * 1024"}));
    }
    w.stop_section();

    w.start_section("client");
    {
        w.field(type_<UnsignedField>(), "keyboard_layout", attached, set(0), linked);
        w.field(A, type_<bool>(), "ignore_logon_password", desc{"If true, ignore password provided by RDP client, user need do login manually."}, set(false));
        w.sep();
        w.field(A | X, type_<uint32_>(), "performance_flags_default", set(0));
        w.field(A | X, type_<uint32_>(), "performance_flags_force_present", desc{"Disable theme (0x8)."}, set(0x8));
        w.field(A | X, type_<uint32_>(), "performance_flags_force_not_present", desc{"Disable font smoothing (0x80)."}, set(0x80));
        w.sep();
        w.field(V, type_<bool>(), "tls_fallback_legacy", desc{"Fallback to RDP Legacy Encryption if client does not support TLS."}, set(true));
        w.field(V, type_<bool>(), "tls_support", set(true));
        w.field(A, type_<bool>(), "bogus_neg_request", desc{"Needed to connect with jrdp, based on bogus X224 layer code."}, set(false));
        w.field(A, type_<bool>(), "bogus_user_id", desc{"Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code."}, set(true));
        w.sep();
        w.field(A, type_<BoolField>(), "disable_tsk_switch_shortcuts", desc{"If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences."}, attached, set(0));
        w.sep();
        w.field(A, type_<rdp_compression_t>{}, "rdp_compression", rdp_compression_desc, set(4));
        w.sep();
        w.field(A, type_<ColorDepth>{}, "max_color_depth", desc{
            "Specifies the maximum color resolution (color depth) for client session:\n"
            "  8: 8 bbp\n"
            "  15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)\n"
            "  16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)\n"
            "  24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)"
        }, set(ColorDepth::depth24));
        w.sep();
        w.field(A, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the front side."}, set(false));
        w.field(A, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        w.field(A, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        w.sep();
        w.field(A, type_<bool>(), "bitmap_compression", desc{"Support of Bitmap Compression."}, set(true));
        w.sep();
        w.field(A, type_<bool>(), "fast_path", desc{"Enables support of Clent Fast-Path Input Event PDUs."}, set(true));
    }
    w.stop_section();

    w.start_section("mod_rdp");
    {
        w.field(A, type_<rdp_compression_t>{}, "rdp_compression", rdp_compression_desc, set(4));
        w.sep();
        w.field(A, type_<bool>(), "disconnect_on_logon_user_change", set(false));
        w.sep();
        w.field(A, type_<uint32_>(), "open_session_timeout", set(0));
        w.sep();
        w.field(A, type_<unsigned>(), "certificate_change_action", desc{
            "0: Cancel connection and reports error.\n"
            "1: Replace existing certificate and continue connection."
        }, set(0));
        w.sep();
        w.field(A, type_<std::string>(), "extra_orders", desc{
            "Enables support of additional drawing orders:\n"
            "  15: MultiDstBlt\n"
            "  16: MultiPatBlt\n"
            "  17: MultiScrBlt\n"
            "  18: MultiOpaqueRect\n"
            "  22: Polyline"
        }, set("15,16,17,18,22"));
        w.sep();
        w.field(V, type_<bool>(), "enable_nla", desc{"NLA authentication in secondary target."}, set(true));
        w.field(V, type_<bool>(), "enable_kerberos", desc{
            "If enabled, NLA authentication will try Kerberos before NTLM.\n"
            "(if enable_nla is disabled, this value is ignored)."
        }, set(false));
        w.sep();
        w.field(A, type_<bool>(), "persistent_disk_bitmap_cache", desc{"Persistent Disk Bitmap Cache on the mod side."}, set(false));
        w.field(A, type_<bool>(), "cache_waiting_list", desc{"Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled)."}, set(true));
        w.field(A, type_<bool>(), "persist_bitmap_cache_on_disk", desc{"If enabled, the contents of Persistent Bitmap Caches are stored on disk."}, set(false));
        w.sep();
        w.field(A, type_<std::string>(), "allow_channels", desc{"Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority."}, set("*"));
        w.field(A, type_<std::string>(), "deny_channels", desc{"Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority."});
        w.sep();
        w.field(A, type_<bool>(), "fast_path", desc{"Enables support of Server Fast-Path Update PDUs."}, set(true));
        w.sep();
        w.field(A, type_<bool>(), "server_redirection_support", desc{"Enables Server Redirection Support."}, set(false));
        w.sep();
        w.field(type_<RedirectionInfo>(), "redir_info");
        w.sep();
        w.field(A, type_<BoolField>(), "bogus_sc_net_size", desc{"Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block."}, attached, set(true), def_authid{"rdp_bogus_sc_net_size"}, str_authid{"rdp_bogus_sc_net_size"});
        w.sep();
        w.field(A, type_<UnsignedField>(), "client_device_announce_timeout", attached, set(1000), def_authid{"opt_client_device_announce_timeout"});
        w.sep();
        w.field(V, type_<StringField>(), "proxy_managed_drives", attached, def_authid{"opt_proxy_managed_drives"});
    }
    w.stop_section();

    w.start_section("mod_vnc");
    {
        w.field(V, type_<BoolField>(), "clipboard_up", desc{"Enable or disable the clipboard from client (client to server)."}, attached, def_authid{"vnc_clipboard_up"});
        w.field(V, type_<BoolField>(), "clipboard_down", desc{"Enable or disable the clipboard from server (server to client)."}, attached, def_authid{"vnc_clipboard_down"});
        w.sep();
        w.field(A, type_<std::string>(), "encodings", desc{
            "Sets the encoding types in which pixel data can be sent by the VNC server:\n"
            "  0: Raw\n"
            "  1: CopyRect\n"
            "  2: RRE\n"
            "  16: ZRLE\n"
            "  -239 (0xFFFFFF11): Cursor pseudo-encoding"
        });
        w.sep();
        w.field(A, type_<bool>(), "allow_authentification_retries", set(false));
        w.sep();
        w.field(A, type_<StringField>(), "server_clipboard_encoding_type", desc{
            "VNC server clipboard data encoding type.\n"
            "  latin1 (default) or utf-8"
        }, attached, set("latin1"), def_authid{"vnc_server_clipboard_encoding_type"}, str_authid{"vnc_server_clipboard_encoding_type"});
        w.sep();
        w.field(A, type_<UnsignedField>(), "bogus_clipboard_infinite_loop", attached, set(0), def_authid{"vnc_bogus_clipboard_infinite_loop"}, str_authid{"vnc_bogus_clipboard_infinite_loop"});
    }
    w.stop_section();

    w.start_section("mod_replay");
    {
        w.field(A, type_<int>(), "on_end_of_data", desc{"0 - Wait for Escape, 1 - End session"}, set(0));
    }
    w.stop_section();

    w.start_section("video");
    {
        w.field(A, type_<unsigned>(), "capture_groupid", set(33));
        w.sep();
        w.field(A, type_<CaptureFlags>{}, "capture_flags", desc{
            "Specifies the type of data to be captured:\n"
            "  1: PNG\n"
            "  2: WRM\n"
            "  4: FLV\n"
            "  8: OCR\n"
            " 16: OCR2"
        }, set(CaptureFlags::png | CaptureFlags::wrm));
        w.sep();
        w.sep();
        w.field(A, type_<std::string>(), "ocr_locale", desc{"latin (default) or cyrillic"});
        w.field(A, type_<unsigned>(), "ocr_interval", desc{"Is in 1/100 s"}, set(100));
        w.field(A, type_<bool>(), "ocr_on_title_bar_only", set(false));
        w.field(A, type_<Range<unsigned, 0, 100>>{}, "ocr_max_unrecog_char_rate", desc{
            "Expressed in percentage,\n"
          "  0   - all of characters need be recognized\n"
          "  100 - accept all results"
        }, set(40));
        w.sep();
        w.field(A, type_<unsigned>(), "png_interval", desc{"Frame interval is in 1/10 s."}, set(3000));
        w.field(A, type_<unsigned>(), "frame_interval", desc{"Frame interval is in 1/100 s."}, set(40));
        w.field(A, type_<unsigned>(), "break_interval", desc{"Time between 2 wrm movies (in seconds)."}, set(600));
        w.field(A, type_<unsigned>(), "png_limit", desc{"Number of png captures to keep."}, set(5));
        w.sep();
        w.field(type_<uint64_>(), "flv_break_interval", set(0));
        w.sep();
        w.field(A, type_<StaticString<1024>>(), "replay_path", set("/tmp/"));
        w.sep();
        w.field(A, type_<unsigned>(), "l_bitrate", desc{"Bitrate for low quality."}, set(10000));
        w.field(A, type_<unsigned>(), "l_framerate", desc{"Framerate for low quality."}, set(5));
        w.field(A, type_<unsigned>(), "l_height", desc{"Height for low quality."}, set(480));
        w.field(A, type_<unsigned>(), "l_width", desc{"Width for low quality."}, set(640));
        w.field(A, type_<unsigned>(), "l_qscale", desc{"Qscale (parameter given to ffmpeg) for low quality."}, set(28));
        w.sep();
        w.field(A, type_<unsigned>(), "m_bitrate", desc{"Bitrate for medium quality."}, set(20000));
        w.field(A, type_<unsigned>(), "m_framerate", desc{"Framerate for medium quality."}, set(5));
        w.field(A, type_<unsigned>(), "m_height", desc{"Height for medium quality."}, set(768));
        w.field(A, type_<unsigned>(), "m_width", desc{"Width for medium quality."}, set(1024));
        w.field(A, type_<unsigned>(), "m_qscale", desc{"Qscale (parameter given to ffmpeg) for medium quality."}, set(14));
        w.sep();
        w.field(A, type_<unsigned>(), "h_bitrate", desc{"Bitrate for high quality."}, set(30000));
        w.field(A, type_<unsigned>(), "h_framerate", desc{"Framerate for high quality."}, set(5));
        w.field(A, type_<unsigned>(), "h_height", desc{"Height for high quality."}, set(2048));
        w.field(A, type_<unsigned>(), "h_width", desc{"Width for high quality."}, set(2048));
        w.field(A, type_<unsigned>(), "h_qscale", desc{"Qscale (parameter given to ffmpeg) for high quality."}, set(7));
        w.sep();
        w.field(A, type_<StaticPath<1024>>(), "hash_path", set(HASH_PATH));
        w.field(A, type_<StaticPath<1024>>(), "record_tmp_path", set(RECORD_TMP_PATH));
        w.field(A, type_<StaticPath<1024>>(), "record_path", set(RECORD_PATH));
        w.sep();
        w.field(type_<bool>(), "inactivity_pause", set(false));
        w.field(type_<unsigned>(), "inactivity_timeout", set(300));
        w.sep();
        w.field(V, type_<KeyboardLogFlagsField>{}, "disable_keyboard_log", desc{
            "Disable keyboard log:\n"
            "  1: disable keyboard log in syslog\n"
            "  2: disable keyboard log in recorded sessions\n"
            "  4: disable keyboard log in META files"
        }, attached, linked);
        w.sep();
        w.field(V, type_<ClipboardLogFlagsField>(), "disable_clipboard_log", desc{
            "Disable clipboard log:\n"
            "  1: disable clipboard log in syslog"
        }, attached);
        w.sep();
        w.field(H, type_<UnsignedField>(), "rt_display", attached, set(0));
        w.sep();
        w.field(A, type_<Range<unsigned, 0, 1>>{}, "wrm_color_depth_selection_strategy", desc{
            "The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:\n"
            "  0: 24-bit\n"
            "  1: 16-bit"
        }, set(0));
        w.field(A, type_<Range<unsigned, 0, 2>>{}, "wrm_compression_algorithm", desc{
            "The compression method of native video capture:\n"
            "  0: No compression\n"
            "  1: GZip\n"
            "  2: Snappy"
        }, set(0));
    }
    w.stop_section();

    w.start_section("crypto");
    {
        w.field(H, type_<StaticKeyString<32>>(), "key0", set(expr{R"(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        )"}));
        w.field(H, type_<StaticKeyString<32>>(), "key1", set(expr{R"(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        )"}));
    }
    w.stop_section();

    w.start_section("debug");
    {
        w.field(A, type_<uint32_>(), "x224", set(0));
        w.field(A, type_<uint32_>(), "mcs", set(0));
        w.field(A, type_<uint32_>(), "sec", set(0));
        w.field(A, type_<uint32_>(), "rdp", set(0));
        w.field(A, type_<uint32_>(), "primary_orders", set(0));
        w.field(A, type_<uint32_>(), "secondary_orders", set(0));
        w.field(A, type_<uint32_>(), "bitmap", set(0));
        w.field(A, type_<uint32_>(), "capture", set(0));
        w.field(A, type_<uint32_>(), "auth", set(0));
        w.field(A, type_<uint32_>(), "session", set(0));
        w.field(A, type_<uint32_>(), "front", set(0));
        w.field(A, type_<uint32_>(), "mod_rdp", set(0));
        w.field(A, type_<uint32_>(), "mod_vnc", set(0));
        w.field(A, type_<uint32_>(), "mod_int", set(0));
        w.field(A, type_<uint32_>(), "mod_xup", set(0));
        w.field(A, type_<uint32_>(), "widget", set(0));
        w.field(A, type_<uint32_>(), "input", set(0));
        w.field(A, type_<uint32_>(), "password", set(0));
        w.field(A, type_<uint32_>(), "compression", set(0));
        w.field(A, type_<uint32_>(), "cache", set(0));
        w.field(A, type_<uint32_>(), "bitmap_update", set(0));
        w.field(A, type_<uint32_>(), "performance", set(0));
        w.field(A, type_<uint32_>(), "pass_dialog_box", set(0));
        w.sep();
        w.field(A, type_<Range<unsigned, 0, 2>>(), "config", desc{"0 = disable, 1 = enable, 2 = enable but no specified"}, set(2));
    }
    w.stop_section();

    w.start_section("translation");
    {
        w.field(A, type_<LanguageField>{}, "language", set(Language::en), attached);
    }
    w.stop_section();

    w.start_section("internal_mod");
    {
        w.field(A, type_<std::string>(), "load_theme", real_name{"theme"}, set(""));
    }
    w.stop_section();

    w.start_section("context");
    {
        w.field(type_<StaticString<1024>>(), "movie");
        w.sep();
        w.field(type_<UnsignedField>(), "opt_bitrate", set(40000), attached, str_authid{"bitrate"});
        w.field(type_<UnsignedField>(), "opt_framerate", set(5), attached, str_authid{"framerate"});
        w.field(type_<UnsignedField>(), "opt_qscale", set(15), attached, str_authid{"qscale"});
        w.sep();
        w.field(type_<UnsignedField>(), "opt_bpp", linked, set(24), attached, str_authid{"bpp"});
        w.field(type_<UnsignedField>(), "opt_height", linked, set(600), attached, str_authid{"height"});
        w.field(type_<UnsignedField>(), "opt_width", linked, set(800), attached, str_authid{"width"});
        w.sep();
        w.field(type_<ReadOnlyStringField>(), "auth_error_message", info{
            "auth_error_message is left as std::string type\n"
            "because SocketTransport and ReplayMod take it as argument on\n"
            "constructor and modify it as a std::string"
        }, attached, used);
        w.sep();
        w.field(type_<BoolField>(), "selector", linked, set(false), attached);
        w.field(type_<UnsignedField>(), "selector_current_page", linked, set(1), attached);
        w.field(type_<StringField>(), "selector_device_filter", linked, attached);
        w.field(type_<StringField>(), "selector_group_filter", linked, attached);
        w.field(type_<StringField>(), "selector_proto_filter", linked, attached);
        w.field(type_<UnsignedField>(), "selector_lines_per_page", linked, set(0), attached);
        w.field(type_<UnsignedField>(), "selector_number_of_pages", set(1), attached);
        w.sep();
        w.field(type_<StringField>(), "target_password", linked, asked, attached);
        w.field(type_<StringField>(), "target_host", linked, set(""), asked, attached);
        w.field(type_<UnsignedField>(), "target_port", set(3389), asked, attached);
        w.field(type_<StringField>(), "target_protocol", linked, set("RDP"), asked, attached, str_authid{"proto_dest"});
        w.sep();
        w.field(type_<StringField>(), "password", linked, asked, attached);
        w.sep();
        w.field(type_<StringField>(), "reporting", linked, attached);
        w.sep();
        w.field(type_<StringField>(), "auth_channel_answer", attached);
        w.field(type_<StringField>(), "auth_channel_result", linked, attached);
        w.field(type_<StringField>(), "auth_channel_target", linked, attached);
        w.sep();
        w.field(type_<StringField>(), "message", attached);
        w.field(type_<StringField>(), "pattern_kill", attached);
        w.field(type_<StringField>(), "pattern_notify", attached);
        w.sep();
        w.field(type_<StringField>(), "accept_message", todo{"why are the field below Strings ? They should be booleans. As they can only contain True/False to know if a user clicked on a button"}, linked, attached);
        w.field(type_<StringField>(), "display_message", linked, attached);
        w.sep();
        w.field(type_<StringField>(), "rejected", attached);
        w.sep();
        w.field(type_<BoolField>(), "authenticated", set(false), attached);
        w.sep();
        w.field(type_<BoolField>(), "keepalive", set(false), attached, linked);
        w.sep();
        w.field(type_<StringField>(), "session_id", attached);
        w.sep();
        w.field(type_<UnsignedField>(), "end_date_cnx", set(0), attached, str_authid{"timeclose"});
        w.field(type_<StringField>(), "end_time", attached);
        w.sep();
        w.field(type_<StringField>(), "mode_console", set("allow"), attached);
        w.field(type_<SignedField>(), "timezone", set(-3600), attached);
        w.sep();
        w.field(type_<StringField>(), "real_target_device", linked, attached);
        w.sep();
        w.field(type_<BoolField>(), "authentication_challenge", asked, attached);
        w.sep();
        w.field(type_<StringField>(), "ticket", linked, set(""), attached, used);
        w.field(type_<StringField>(), "comment", linked, set(""), attached, used);
        w.field(type_<StringField>(), "duration", linked, set(""), attached, used);
        w.field(type_<StringField>(), "waitinforeturn", linked, set(""), attached, used);
        w.field(type_<BoolField>(), "showform", linked, set(false), attached, used);
        w.field(type_<UnsignedField>(), "formflag", linked, set(0), attached, used);
        w.sep();
        w.field(type_<StringField>(), "module", linked, set("login"), attached, used);
        w.field(type_<BoolField>(), "forcemodule", set(false), attached);
        w.field(type_<StringField>(), "proxy_opt", attached);
    }
    w.stop_section();

    w.start_section("");
    w.field(type_<Theme>(), "theme");
    w.field(type_<IniAccounts>(), "account");
    w.field(type_<Font>(), "font");
    w.stop_section();
}

}

#endif
