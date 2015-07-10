#include "font.hpp"
#include "configs/types.hpp"
#include "configs/includes.hpp"

namespace configs {

struct VariablesConfiguration {
    explicit VariablesConfiguration(char const * default_font_name)
    : font(default_font_name)
    {}

    Theme theme;
    IniAccounts account;
    Font font;

    struct Inifile_client {
        UnsignedField keyboard_layout;  // AUTHID_KEYBOARD_LAYOUT
        // If true, ignore password provided by RDP client, user need do login manually.
        bool ignore_logon_password{0};

        uint32_t performance_flags_default{0};
        // Disable theme (0x8).
        uint32_t performance_flags_force_present{8};
        // Disable font smoothing (0x80).
        uint32_t performance_flags_force_not_present{128};

        // Fallback to RDP Legacy Encryption if client does not support TLS.
        bool tls_fallback_legacy{1};
        bool tls_support{1};
        // Needed to connect with jrdp, based on bogus X224 layer code.
        bool bogus_neg_request{0};
        // Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code.
        bool bogus_user_id{1};

        // If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences.
        BoolField disable_tsk_switch_shortcuts;  // AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS

        // Specifies the highest compression package support available on the front side:
        //   0: the RDP bulk compression is disabled
        //   1: RDP 4.0 bulk compression
        //   2: RDP 5.0 bulk compression
        //   3: RDP 6.0 bulk compression
        //   4: RDP 6.1 bulk compression
        Range<unsigned, 0, 4, 0> rdp_compression{4};

        // Specifies the maximum color resolution (color depth) for client session:
        //   8: 8 bbp
        //   15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)
        //   16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)
        //   24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)
        ColorDepth max_color_depth{static_cast<ColorDepth>(24)};

        // Persistent Disk Bitmap Cache on the front side.
        bool persistent_disk_bitmap_cache{0};
        // Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
        bool cache_waiting_list{1};
        // If enabled, the contents of Persistent Bitmap Caches are stored on disk.
        bool persist_bitmap_cache_on_disk{0};

        // Support of Bitmap Compression.
        bool bitmap_compression{1};

        // Enables support of Clent Fast-Path Input Event PDUs.
        bool fast_path{1};
        Inifile_client() = default;
    } client;

    struct Inifile_context {
        StaticString<1024> movie;

        UnsignedField opt_bitrate;  // AUTHID_OPT_BITRATE
        UnsignedField opt_framerate;  // AUTHID_OPT_FRAMERATE
        UnsignedField opt_qscale;  // AUTHID_OPT_QSCALE

        UnsignedField opt_bpp;  // AUTHID_OPT_BPP
        UnsignedField opt_height;  // AUTHID_OPT_HEIGHT
        UnsignedField opt_width;  // AUTHID_OPT_WIDTH

        // auth_error_message is left as std::string type
        // because SocketTransport and ReplayMod take it as argument on
        // constructor and modify it as a std::string
        ReadOnlyStringField auth_error_message;  // AUTHID_AUTH_ERROR_MESSAGE

        BoolField selector;  // AUTHID_SELECTOR
        UnsignedField selector_current_page;  // AUTHID_SELECTOR_CURRENT_PAGE
        StringField selector_device_filter;  // AUTHID_SELECTOR_DEVICE_FILTER
        StringField selector_group_filter;  // AUTHID_SELECTOR_GROUP_FILTER
        StringField selector_proto_filter;  // AUTHID_SELECTOR_PROTO_FILTER
        UnsignedField selector_lines_per_page;  // AUTHID_SELECTOR_LINES_PER_PAGE
        UnsignedField selector_number_of_pages;  // AUTHID_SELECTOR_NUMBER_OF_PAGES

        StringField target_password;  // AUTHID_TARGET_PASSWORD
        StringField target_host;  // AUTHID_TARGET_HOST
        UnsignedField target_port;  // AUTHID_TARGET_PORT
        StringField target_protocol;  // AUTHID_TARGET_PROTOCOL

        StringField password;  // AUTHID_PASSWORD

        StringField reporting;  // AUTHID_REPORTING

        StringField auth_channel_answer;  // AUTHID_AUTH_CHANNEL_ANSWER
        StringField auth_channel_result;  // AUTHID_AUTH_CHANNEL_RESULT
        StringField auth_channel_target;  // AUTHID_AUTH_CHANNEL_TARGET

        StringField message;  // AUTHID_MESSAGE
        StringField pattern_kill;  // AUTHID_PATTERN_KILL
        StringField pattern_notify;  // AUTHID_PATTERN_NOTIFY

        TODO("why are the field below Strings ? They should be booleans. As they can only contain True/False to know if a user clicked on a button")
        StringField accept_message;  // AUTHID_ACCEPT_MESSAGE
        StringField display_message;  // AUTHID_DISPLAY_MESSAGE

        StringField rejected;  // AUTHID_REJECTED

        BoolField authenticated;  // AUTHID_AUTHENTICATED

        BoolField keepalive;  // AUTHID_KEEPALIVE

        StringField session_id;  // AUTHID_SESSION_ID

        UnsignedField end_date_cnx;  // AUTHID_END_DATE_CNX
        StringField end_time;  // AUTHID_END_TIME

        StringField mode_console;  // AUTHID_MODE_CONSOLE
        SignedField timezone;  // AUTHID_TIMEZONE

        StringField real_target_device;  // AUTHID_REAL_TARGET_DEVICE

        BoolField authentication_challenge;  // AUTHID_AUTHENTICATION_CHALLENGE

        StringField ticket;  // AUTHID_TICKET
        StringField comment;  // AUTHID_COMMENT
        StringField duration;  // AUTHID_DURATION
        StringField waitinforeturn;  // AUTHID_WAITINFORETURN
        BoolField showform;  // AUTHID_SHOWFORM
        UnsignedField formflag;  // AUTHID_FORMFLAG

        StringField module;  // AUTHID_MODULE
        BoolField forcemodule;  // AUTHID_FORCEMODULE
        StringField proxy_opt;  // AUTHID_PROXY_OPT
        Inifile_context() = default;
    } context;

    struct Inifile_crypto {
        StaticKeyString<32> key0{"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"};
        StaticKeyString<32> key1{"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"};
        Inifile_crypto() = default;
    } crypto;

    struct Inifile_debug {
        uint32_t x224{0};
        uint32_t mcs{0};
        uint32_t sec{0};
        uint32_t rdp{0};
        uint32_t primary_orders{0};
        uint32_t secondary_orders{0};
        uint32_t bitmap{0};
        uint32_t capture{0};
        uint32_t auth{0};
        uint32_t session{0};
        uint32_t front{0};
        uint32_t mod_rdp{0};
        uint32_t mod_vnc{0};
        uint32_t mod_int{0};
        uint32_t mod_xup{0};
        uint32_t widget{0};
        uint32_t input{0};
        uint32_t password{0};
        uint32_t compression{0};
        uint32_t cache{0};
        uint32_t bitmap_update{0};
        uint32_t performance{0};
        uint32_t pass_dialog_box{0};

        Range<unsigned, 0, 2, 0> config{2};
        Inifile_debug() = default;
    } debug;

    struct Inifile_globals {
        BoolField capture_chunk;

        StringField auth_user;  // AUTHID_AUTH_USER
        StringField host;  // AUTHID_HOST
        StringField target;  // AUTHID_TARGET
        StringField target_device;  // AUTHID_TARGET_DEVICE
        StringField target_user;  // AUTHID_TARGET_USER
        StringField target_application;  // AUTHID_TARGET_APPLICATION
        StringField target_application_account;  // AUTHID_TARGET_APPLICATION_ACCOUNT
        StringField target_application_password;  // AUTHID_TARGET_APPLICATION_PASSWORD

        // Support of Bitmap Cache.
        bool bitmap_cache{1};
        bool glyph_cache{0};
        unsigned port{3389};
        bool nomouse{0};
        bool notimestamp{0};
        // low, medium or high.
        Level encryptionLevel{static_cast<Level>(0)};
        StaticIpString authip{"127.0.0.1"};
        unsigned authport{3350};

        // No traffic auto disconnection (in seconds).
        unsigned session_timeout{900};
        // Keepalive (in seconds).
        unsigned keepalive_grace_delay{30};
        // Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).
        unsigned close_timeout{600};

        StaticNilString<8> auth_channel{null_fill()};
        BoolField enable_file_encryption;  // AUTHID_OPT_FILE_ENCRYPTION
        StaticIpString listen_address{"0.0.0.0"};
        // Allow IP Transparent.
        bool enable_ip_transparent{0};
        // Proxy certificate password.
        StaticString<256> certificate_password{"inquisition"};

        StaticString<1024> png_path{PNG_PATH};
        StaticString<1024> wrm_path{WRM_PATH};

        StringField alternate_shell;  // AUTHID_ALTERNATE_SHELL
        StringField shell_working_directory;  // AUTHID_SHELL_WORKING_DIRECTORY

        StringField codec_id;  // AUTHID_OPT_CODEC_ID
        BoolField movie;  // AUTHID_OPT_MOVIE
        StringField movie_path;  // AUTHID_OPT_MOVIE_PATH
        TODO("this could be some kind of enumeration")
        // low, medium or high.
        LevelField video_quality;  // AUTHID_VIDEO_QUALITY
        // Support of Bitmap Update.
        bool enable_bitmap_update{1};

        // Show close screen.
        bool enable_close_box{1};
        bool enable_osd{1};
        bool enable_osd_display_remote_target{1};

        BoolField enable_wab_agent;  // AUTHID_OPT_WABAGENT
        UnsignedField wab_agent_launch_timeout;  // AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT
        UnsignedField wab_agent_keepalive_timeout;  // AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT

        StaticString<512> wab_agent_alternate_shell{""};

        StaticPath<1024> persistent_path{PERSISTENT_PATH};

        bool disable_proxy_opt{0};
        //  The maximum length of the chunked virtual channel data.
        uint32_t max_chunked_virtual_channel_data_length{2097152};
        Inifile_globals() = default;
    } globals;

    struct Inifile_internal_mod {
        std::string theme{""};
        Inifile_internal_mod() = default;
    } internal_mod;

    struct Inifile_mod_rdp {
        // Specifies the highest compression package support available on the front side:
        //   0: the RDP bulk compression is disabled
        //   1: RDP 4.0 bulk compression
        //   2: RDP 5.0 bulk compression
        //   3: RDP 6.0 bulk compression
        //   4: RDP 6.1 bulk compression
        Range<unsigned, 0, 4, 0> rdp_compression{4};

        bool disconnect_on_logon_user_change{0};

        uint32_t open_session_timeout{0};

        // 0: Cancel connection and reports error.
        // 1: Replace existing certificate and continue connection.
        Range<unsigned, 0, 1, 0> certificate_change_action{0};

        // Enables support of additional drawing orders:
        //   15: MultiDstBlt
        //   16: MultiPatBlt
        //   17: MultiScrBlt
        //   18: MultiOpaqueRect
        //   22: Polyline
        std::string extra_orders{"15,16,17,18,22"};

        // NLA authentication in secondary target.
        bool enable_nla{1};
        // If enabled, NLA authentication will try Kerberos before NTLM.
        // (if enable_nla is disabled, this value is ignored).
        bool enable_kerberos{0};

        // Persistent Disk Bitmap Cache on the mod side.
        bool persistent_disk_bitmap_cache{0};
        // Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
        bool cache_waiting_list{1};
        // If enabled, the contents of Persistent Bitmap Caches are stored on disk.
        bool persist_bitmap_cache_on_disk{0};

        // Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority.
        std::string allow_channels{"*"};
        // Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority.
        std::string deny_channels;

        // Enables support of Server Fast-Path Update PDUs.
        bool fast_path{1};

        // Enables Server Redirection Support.
        bool server_redirection_support{0};

        RedirectionInfo redir_info;

        // Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block.
        BoolField bogus_sc_net_size;  // AUTHID_RDP_BOGUS_SC_NET_SIZE

        UnsignedField client_device_announce_timeout;  // AUTHID_OPT_CLIENT_DEVICE_ANNOUNCE_TIMEOUT

        StringField proxy_managed_drives;  // AUTHID_OPT_PROXY_MANAGED_DRIVES
        Inifile_mod_rdp() = default;
    } mod_rdp;

    struct Inifile_mod_replay {
        // 0 - Wait for Escape, 1 - End session
        Range<int, 0, 1, 0> on_end_of_data{0};
        Inifile_mod_replay() = default;
    } mod_replay;

    struct Inifile_mod_vnc {
        // Enable or disable the clipboard from client (client to server).
        BoolField clipboard_up;  // AUTHID_VNC_CLIPBOARD_UP
        // Enable or disable the clipboard from server (server to client).
        BoolField clipboard_down;  // AUTHID_VNC_CLIPBOARD_DOWN

        // Sets the encoding types in which pixel data can be sent by the VNC server:
        //   0: Raw
        //   1: CopyRect
        //   2: RRE
        //   16: ZRLE
        //   -239 (0xFFFFFF11): Cursor pseudo-encoding
        std::string encodings;

        bool allow_authentification_retries{0};

        // VNC server clipboard data encoding type.
        //   latin1 (default) or utf-8
        ClipboardEncodingTypeField server_clipboard_encoding_type;  // AUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE

        UnsignedField bogus_clipboard_infinite_loop;  // AUTHID_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP
        Inifile_mod_vnc() = default;
    } mod_vnc;

    struct Inifile_translation {
        LanguageField language;  // AUTHID_LANGUAGE
        Inifile_translation() = default;
    } translation;

    struct Inifile_video {
        unsigned capture_groupid{33};

        // Specifies the type of data to be captured:
        //   1: PNG
        //   2: WRM
        //   4: FLV
        //   8: OCR
        //  16: OCR2
        CaptureFlags capture_flags{static_cast<CaptureFlags>(3)};


        // latin (default) or cyrillic
        std::string ocr_locale;
        // Is in 1/100 s
        unsigned ocr_interval{100};
        bool ocr_on_title_bar_only{0};
        // Expressed in percentage,
        //   0   - all of characters need be recognized
        //   100 - accept all results
        Range<unsigned, 0, 100, 0> ocr_max_unrecog_char_rate{40};

        // Frame interval is in 1/10 s.
        unsigned png_interval{3000};
        // Frame interval is in 1/100 s.
        unsigned frame_interval{40};
        // Time between 2 wrm movies (in seconds).
        unsigned break_interval{600};
        // Number of png captures to keep.
        unsigned png_limit{5};

        uint64_t flv_break_interval{0};

        StaticString<1024> replay_path{"/tmp/"};

        // Bitrate for low quality.
        unsigned l_bitrate{10000};
        // Framerate for low quality.
        unsigned l_framerate{5};
        // Height for low quality.
        unsigned l_height{480};
        // Width for low quality.
        unsigned l_width{640};
        // Qscale (parameter given to ffmpeg) for low quality.
        unsigned l_qscale{28};

        // Bitrate for medium quality.
        unsigned m_bitrate{20000};
        // Framerate for medium quality.
        unsigned m_framerate{5};
        // Height for medium quality.
        unsigned m_height{768};
        // Width for medium quality.
        unsigned m_width{1024};
        // Qscale (parameter given to ffmpeg) for medium quality.
        unsigned m_qscale{14};

        // Bitrate for high quality.
        unsigned h_bitrate{30000};
        // Framerate for high quality.
        unsigned h_framerate{5};
        // Height for high quality.
        unsigned h_height{2048};
        // Width for high quality.
        unsigned h_width{2048};
        // Qscale (parameter given to ffmpeg) for high quality.
        unsigned h_qscale{7};

        StaticPath<1024> hash_path{HASH_PATH};
        StaticPath<1024> record_tmp_path{RECORD_TMP_PATH};
        StaticPath<1024> record_path{RECORD_PATH};

        bool inactivity_pause{0};
        unsigned inactivity_timeout{300};

        // Disable keyboard log:
        //   1: disable keyboard log in syslog
        //   2: disable keyboard log in recorded sessions
        //   4: disable keyboard log in META files
        KeyboardLogFlagsField disable_keyboard_log;  // AUTHID_DISABLE_KEYBOARD_LOG

        // Disable clipboard log:
        //   1: disable clipboard log in syslog
        ClipboardLogFlagsField disable_clipboard_log;  // AUTHID_DISABLE_CLIPBOARD_LOG

        UnsignedField rt_display;  // AUTHID_RT_DISPLAY

        // The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:
        //   0: 24-bit
        //   1: 16-bit
        Range<unsigned, 0, 1, 0> wrm_color_depth_selection_strategy{0};
        // The compression method of native video capture:
        //   0: No compression
        //   1: GZip
        //   2: Snappy
        Range<unsigned, 0, 2, 0> wrm_compression_algorithm{0};
        Inifile_video() = default;
    } video;

};

}
