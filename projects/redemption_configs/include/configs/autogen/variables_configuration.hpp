namespace cfg {
    struct globals {
        // AUTHID_GLOBALS_CAPTURE_CHUNK
        struct capture_chunk {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "capture_chunk"; }
            static constexpr unsigned index() { return 0; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{};
        };

        // AUTHID_GLOBALS_AUTH_USER
        struct auth_user {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "auth_user"; }
            static constexpr unsigned index() { return 1; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_HOST
        struct host {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "host"; }
            static constexpr unsigned index() { return 2; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET
        struct target {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target"; }
            static constexpr unsigned index() { return 3; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_DEVICE
        struct target_device {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_device"; }
            static constexpr unsigned index() { return 4; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_DEVICE_ID
        struct device_id {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "device_id"; }
            static constexpr unsigned index() { return 5; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_USER
        struct target_user {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 1; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_user"; }
            static constexpr unsigned index() { return 6; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_APPLICATION
        struct target_application {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_application"; }
            static constexpr unsigned index() { return 7; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_APPLICATION_ACCOUNT
        struct target_application_account {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_application_account"; }
            static constexpr unsigned index() { return 8; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // AUTHID_GLOBALS_TARGET_APPLICATION_PASSWORD
        struct target_application_password {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "target_application_password"; }
            static constexpr unsigned index() { return 9; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };

        // Support of Bitmap Cache.
        struct bitmap_cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "bitmap_cache"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{1};
        };
        struct glyph_cache {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "glyph_cache"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };
        struct port {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "port"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            type value{3389};
        };
        struct nomouse {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "nomouse"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };
        struct notimestamp {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "notimestamp"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };
        struct encryptionLevel {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "encryptionLevel"; }
            using type = configs::Level;
            using sesman_and_spec_type = configs::Level;
            type value{static_cast<type>(0)};
        };
        struct authfile {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "authfile"; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value = "/var/run/redemption-sesman-sock";
        };

        // Time out during RDP handshake stage (in seconds).
        struct handshake_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "handshake_timeout"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            type value{10};
        };
        // No traffic auto disconnection (in seconds).
        struct session_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "session_timeout"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            type value{900};
        };
        // Keepalive (in seconds).
        struct keepalive_grace_delay {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "keepalive_grace_delay"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            type value{30};
        };
        // Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).
        struct authentication_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "authentication_timeout"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            type value{120};
        };
        // Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).
        struct close_timeout {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "close_timeout"; }
            using type = unsigned int;
            using sesman_and_spec_type = unsigned int;
            type value{600};
        };

        // AUTHID_GLOBALS_TRACE_TYPE
        struct trace_type {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "trace_type"; }
            static constexpr unsigned index() { return 10; }
            using type = configs::TraceType;
            using sesman_and_spec_type = configs::TraceType;
            type value{static_cast<type>(1)};
        };

        struct listen_address {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "listen_address"; }
            using type = std::string;
            using sesman_and_spec_type = ::configs::spec_types::ip;
            type value = "0.0.0.0";
        };
        // Allow IP Transparent.
        struct enable_ip_transparent {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_ip_transparent"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };
        // Proxy certificate password.
        struct certificate_password {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "certificate_password"; }
            using type = char[255 + 1];
            using sesman_and_spec_type = char[255 + 1];
            type value = "inquisition";
        };

        struct png_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "png_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            type value = PNG_PATH;
        };
        struct wrm_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "wrm_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            type value = WRM_PATH;
        };

        // AUTHID_GLOBALS_IS_REC
        struct is_rec {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "is_rec"; }
            static constexpr unsigned index() { return 11; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };
        // AUTHID_GLOBALS_MOVIE_PATH
        struct movie_path {
            static constexpr bool is_readable() { return 1; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "movie_path"; }
            static constexpr unsigned index() { return 12; }
            using type = std::basic_string<char>;
            using sesman_and_spec_type = std::basic_string<char>;
            type value{};
        };
        // Support of Bitmap Update.
        struct enable_bitmap_update {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_bitmap_update"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{1};
        };

        // Show close screen.
        struct enable_close_box {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_close_box"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{1};
        };
        struct enable_osd {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_osd"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{1};
        };
        struct enable_osd_display_remote_target {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "enable_osd_display_remote_target"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{1};
        };

        struct persistent_path {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "persistent_path"; }
            using type = ::configs::spec_types::directory_path;
            using sesman_and_spec_type = ::configs::spec_types::directory_path;
            type value = PERSISTENT_PATH;
        };

        struct disable_proxy_opt {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "disable_proxy_opt"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };

        struct allow_using_multiple_monitors {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "allow_using_multiple_monitors"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{0};
        };

        // Needed to refresh screen of Windows Server 2012.
        struct bogus_refresh_rect {
            static constexpr bool is_readable() { return 0; }
            static constexpr bool is_writable() { return 0; }
            static constexpr char const * section() { return "globals"; }
            static constexpr char const * name() { return "bogus_refresh_rect"; }
            using type = bool;
            using sesman_and_spec_type = bool;
            type value{1};
        };
    };

    struct session_log {
            struct enable_session_log {
                static constexpr bool is_readable() { return 0; }
                static constexpr bool is_writable() { return 0; }
                static constexpr char const * section() { return "session_log"; }
                static constexpr char const * name() { return "enable_session_log"; }
                using type = bool;
                using sesman_and_spec_type = bool;
                type value{1};
            };

            struct keyboard_input_masking_level {
                static constexpr bool is_readable() { return 0; }
                static constexpr bool is_writable() { return 0; }
                static constexpr char const * section() { return "session_log"; }
                static constexpr char const * name() { return "keyboard_input_masking_level"; }
                using type = configs::KeyboardInputMaskingLevel;
                using sesman_and_spec_type = configs::KeyboardInputMaskingLevel;
                type value{static_cast<type>(3)};
            };
    };

    struct client {
                // AUTHID_CLIENT_KEYBOARD_LAYOUT
                struct keyboard_layout {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 1; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "keyboard_layout"; }
                    static constexpr unsigned index() { return 13; }
                    using type = unsigned int;
                    using sesman_and_spec_type = unsigned int;
                    type value{0};
                };
        // cs-CZ, da-DK, de-DE, el-GR, en-US, es-ES, fi-FI.finnish, fr-FR, is-IS, it-IT, nl-NL, nb-NO, pl-PL.programmers, pt-BR.abnt, ro-RO, ru-RU, hr-HR, sk-SK, sv-SE, tr-TR.q, uk-UA, sl-SI, et-EE, lv-LV, lt-LT.ibm, mk-MK, fo-FO, mt-MT.47, se-NO, kk-KZ, ky-KG, tt-RU, mn-MN, cy-GB, lb-LU, mi-NZ, de-CH, en-GB, es-MX, fr-BE.fr, nl-BE, pt-PT, sr-La, se-SE, uz-Cy, iu-La, fr-CA, sr-Cy, en-CA.fr, fr-CH, bs-Cy, bg-BG.latin, cs-CZ.qwerty, en-IE.irish, de-DE.ibm, el-GR.220, es-ES.variation, hu-HU, en-US.dvorak, it-IT.142, pl-PL, pt-BR.abnt2, ru-RU.typewriter, sk-SK.qwerty, tr-TR.f, lv-LV.qwerty, lt-LT, mt-MT.48, se-NO.ext_norway, fr-BE, se-SE, en-CA.multilingual, en-IE, cs-CZ.programmers, el-GR.319, en-US.international, se-SE.ext_finland_sweden, bg-BG, el-GR.220_latin, en-US.dvorak_left, el-GR.319_latin, en-US.dvorak_right, el-GR.latin, el-GR.polytonic
                struct keyboard_layout_proposals {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "keyboard_layout_proposals"; }
                    using type = std::string;
                    using sesman_and_spec_type = ::configs::spec_types::list<std::basic_string<char>>;
                    type value = "en-US, fr-FR, de-DE, ru-RU";
                };
        // If true, ignore password provided by RDP client, user need do login manually.
                struct ignore_logon_password {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "ignore_logon_password"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{0};
                };

        // Enable font smoothing (0x80).
                struct performance_flags_default {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "performance_flags_default"; }
                    using type = uint32_t;
                    using sesman_and_spec_type = uint32_t;
                    type value{128};
                };
        // Disable theme (0x8).
                struct performance_flags_force_present {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "performance_flags_force_present"; }
                    using type = uint32_t;
                    using sesman_and_spec_type = uint32_t;
                    type value{8};
                };
                struct performance_flags_force_not_present {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "performance_flags_force_not_present"; }
                    using type = uint32_t;
                    using sesman_and_spec_type = uint32_t;
                    type value{0};
                };
        // If enabled, avoid automatically font smoothing in recorded session.
                struct auto_adjust_performance_flags {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "auto_adjust_performance_flags"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };

        // Fallback to RDP Legacy Encryption if client does not support TLS.
                struct tls_fallback_legacy {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "tls_fallback_legacy"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{0};
                };
                struct tls_support {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "tls_support"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };
        // Needed to connect with jrdp, based on bogus X224 layer code.
                struct bogus_neg_request {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "bogus_neg_request"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{0};
                };
        // Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code.
                struct bogus_user_id {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "bogus_user_id"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };

        // If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences.
                // AUTHID_CLIENT_DISABLE_TSK_SWITCH_SHORTCUTS
                struct disable_tsk_switch_shortcuts {
                    static constexpr bool is_readable() { return 1; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "disable_tsk_switch_shortcuts"; }
                    static constexpr unsigned index() { return 14; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{0};
                };

                struct rdp_compression {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "rdp_compression"; }
                    using type = configs::RdpCompression;
                    using sesman_and_spec_type = configs::RdpCompression;
                    type value{static_cast<type>(4)};
                };

                struct max_color_depth {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "max_color_depth"; }
                    using type = configs::ColorDepth;
                    using sesman_and_spec_type = configs::ColorDepth;
                    type value{static_cast<type>(24)};
                };

        // Persistent Disk Bitmap Cache on the front side.
                struct persistent_disk_bitmap_cache {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "persistent_disk_bitmap_cache"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };
        // Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
                struct cache_waiting_list {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "cache_waiting_list"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{0};
                };
        // If enabled, the contents of Persistent Bitmap Caches are stored on disk.
                struct persist_bitmap_cache_on_disk {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "persist_bitmap_cache_on_disk"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{0};
                };

        // Support of Bitmap Compression.
                struct bitmap_compression {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "bitmap_compression"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };

        // Enables support of Clent Fast-Path Input Event PDUs.
                struct fast_path {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "fast_path"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };

                struct enable_suppress_output {
                    static constexpr bool is_readable() { return 0; }
                    static constexpr bool is_writable() { return 0; }
                    static constexpr char const * section() { return "client"; }
                    static constexpr char const * name() { return "enable_suppress_output"; }
                    using type = bool;
                    using sesman_and_spec_type = bool;
                    type value{1};
                };
    };

    struct mod_rdp {
                    struct rdp_compression {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "rdp_compression"; }
                        using type = configs::RdpCompression;
                        using sesman_and_spec_type = configs::RdpCompression;
                        type value{static_cast<type>(4)};
                    };

                    struct disconnect_on_logon_user_change {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "disconnect_on_logon_user_change"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };

                    struct open_session_timeout {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "open_session_timeout"; }
                        using type = uint32_t;
                        using sesman_and_spec_type = uint32_t;
                        type value{0};
                    };

        // Enables support of additional drawing orders:
        //   15: MultiDstBlt
        //   16: MultiPatBlt
        //   17: MultiScrBlt
        //   18: MultiOpaqueRect
        //   22: Polyline
                    struct extra_orders {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "extra_orders"; }
                        using type = std::string;
                        using sesman_and_spec_type = ::configs::spec_types::list<unsigned int>;
                        type value = "15,16,17,18,22";
                    };

        // NLA authentication in secondary target.
                    struct enable_nla {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "enable_nla"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
        // If enabled, NLA authentication will try Kerberos before NTLM.
        // (if enable_nla is disabled, this value is ignored).
                    struct enable_kerberos {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "enable_kerberos"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };

        // Persistent Disk Bitmap Cache on the mod side.
                    struct persistent_disk_bitmap_cache {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "persistent_disk_bitmap_cache"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
        // Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
                    struct cache_waiting_list {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "cache_waiting_list"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
        // If enabled, the contents of Persistent Bitmap Caches are stored on disk.
                    struct persist_bitmap_cache_on_disk {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "persist_bitmap_cache_on_disk"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };

        // Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority.
                    struct allow_channels {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "allow_channels"; }
                        using type = std::string;
                        using sesman_and_spec_type = ::configs::spec_types::list<std::basic_string<char>>;
                        type value = "*";
                    };
        // Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority.
                    struct deny_channels {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "deny_channels"; }
                        using type = std::string;
                        using sesman_and_spec_type = ::configs::spec_types::list<std::basic_string<char>>;
                        type value{};
                    };

        // Enables support of Server Fast-Path Update PDUs.
                    struct fast_path {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "fast_path"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };

        // Enables Server Redirection Support.
                    struct server_redirection_support {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_redirection_support"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };

                    struct redir_info {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "redir_info"; }
                        using type = RedirectionInfo;
                        type value{};
                    };

        // Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block.
                    // AUTHID_MOD_RDP_BOGUS_SC_NET_SIZE
                    struct bogus_sc_net_size {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "bogus_sc_net_size"; }
                        static constexpr unsigned index() { return 15; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };

        // Needed to get the old behavior of cursor rendering.
                    struct bogus_linux_cursor {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "bogus_linux_cursor"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };
                    // AUTHID_MOD_RDP_PROXY_MANAGED_DRIVES
                    struct proxy_managed_drives {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "proxy_managed_drives"; }
                        static constexpr unsigned index() { return 16; }
                        using type = std::string;
                        using sesman_and_spec_type = ::configs::spec_types::list<std::basic_string<char>>;
                        type value{};
                    };


                    // AUTHID_MOD_RDP_IGNORE_AUTH_CHANNEL
                    struct ignore_auth_channel {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "ignore_auth_channel"; }
                        static constexpr unsigned index() { return 17; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };
        // Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel.
                    struct auth_channel {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "auth_channel"; }
                        using type = char[7 + 1];
                        using sesman_and_spec_type = char[7 + 1];
                        type value = "*";
                    };

                    // AUTHID_MOD_RDP_ALTERNATE_SHELL
                    struct alternate_shell {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "alternate_shell"; }
                        static constexpr unsigned index() { return 18; }
                        using type = std::basic_string<char>;
                        using sesman_and_spec_type = std::basic_string<char>;
                        type value{};
                    };
                    // AUTHID_MOD_RDP_SHELL_WORKING_DIRECTORY
                    struct shell_working_directory {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "shell_working_directory"; }
                        static constexpr unsigned index() { return 19; }
                        using type = std::basic_string<char>;
                        using sesman_and_spec_type = std::basic_string<char>;
                        type value{};
                    };

                    // AUTHID_MOD_RDP_USE_CLIENT_PROVIDED_ALTERNATE_SHELL
                    struct use_client_provided_alternate_shell {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "use_client_provided_alternate_shell"; }
                        static constexpr unsigned index() { return 20; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };

                    // AUTHID_MOD_RDP_ENABLE_SESSION_PROBE
                    struct enable_session_probe {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "enable_session_probe"; }
                        static constexpr unsigned index() { return 21; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };
        // Minimum supported server : Windows Server 2008.
        // Clipboard redirection should be remain enabled on Terminal Server.
                    // AUTHID_MOD_RDP_SESSION_PROBE_USE_CLIPBOARD_BASED_LAUNCHER
                    struct session_probe_use_clipboard_based_launcher {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_use_clipboard_based_launcher"; }
                        static constexpr unsigned index() { return 22; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
                    // AUTHID_MOD_RDP_ENABLE_SESSION_PROBE_LAUNCH_MASK
                    struct enable_session_probe_launch_mask {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "enable_session_probe_launch_mask"; }
                        static constexpr unsigned index() { return 23; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
                    // AUTHID_MOD_RDP_SESSION_PROBE_ON_LAUNCH_FAILURE
                    struct session_probe_on_launch_failure {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_on_launch_failure"; }
                        static constexpr unsigned index() { return 24; }
                        using type = configs::SessionProbeOnLaunchFailure;
                        using sesman_and_spec_type = configs::SessionProbeOnLaunchFailure;
                        type value{static_cast<type>(2)};
                    };
        // This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).
        // In milliseconds, 0 to disable timeout.
                    // AUTHID_MOD_RDP_SESSION_PROBE_LAUNCH_TIMEOUT
                    struct session_probe_launch_timeout {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_launch_timeout"; }
                        static constexpr unsigned index() { return 25; }
                        using type = unsigned int;
                        using sesman_and_spec_type = unsigned int;
                        type value{20000};
                    };
        // This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).
        // In milliseconds, 0 to disable timeout.
                    // AUTHID_MOD_RDP_SESSION_PROBE_LAUNCH_FALLBACK_TIMEOUT
                    struct session_probe_launch_fallback_timeout {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_launch_fallback_timeout"; }
                        static constexpr unsigned index() { return 26; }
                        using type = unsigned int;
                        using sesman_and_spec_type = unsigned int;
                        type value{7000};
                    };
        // Minimum supported server : Windows Server 2008.
                    // AUTHID_MOD_RDP_SESSION_PROBE_START_LAUNCH_TIMEOUT_TIMER_ONLY_AFTER_LOGON
                    struct session_probe_start_launch_timeout_timer_only_after_logon {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_start_launch_timeout_timer_only_after_logon"; }
                        static constexpr unsigned index() { return 27; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
                    // AUTHID_MOD_RDP_SESSION_PROBE_KEEPALIVE_TIMEOUT
                    struct session_probe_keepalive_timeout {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_keepalive_timeout"; }
                        static constexpr unsigned index() { return 28; }
                        using type = unsigned int;
                        using sesman_and_spec_type = unsigned int;
                        type value{5000};
                    };
                    // AUTHID_MOD_RDP_SESSION_PROBE_ON_KEEPALIVE_TIMEOUT_DISCONNECT_USER
                    struct session_probe_on_keepalive_timeout_disconnect_user {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_on_keepalive_timeout_disconnect_user"; }
                        static constexpr unsigned index() { return 29; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
        // End automatically a disconnected session
                    // AUTHID_MOD_RDP_SESSION_PROBE_END_DISCONNECTED_SESSION
                    struct session_probe_end_disconnected_session {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_end_disconnected_session"; }
                        static constexpr unsigned index() { return 30; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };
                    struct session_probe_customize_executable_name {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_customize_executable_name"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };
                    struct session_probe_alternate_shell {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "session_probe_alternate_shell"; }
                        using type = char[511 + 1];
                        using sesman_and_spec_type = char[511 + 1];
                        type value = "cmd /k";
                    };
        // Keep known server certificates on WAB
                    // AUTHID_MOD_RDP_SERVER_CERT_STORE
                    struct server_cert_store {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_cert_store"; }
                        static constexpr unsigned index() { return 31; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{1};
                    };
                    // AUTHID_MOD_RDP_SERVER_CERT_CHECK
                    struct server_cert_check {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_cert_check"; }
                        static constexpr unsigned index() { return 32; }
                        using type = configs::ServerCertCheck;
                        using sesman_and_spec_type = configs::ServerCertCheck;
                        type value{static_cast<type>(1)};
                    };
        // Warn if check allow connexion to server.
                    // AUTHID_MOD_RDP_SERVER_ACCESS_ALLOWED_MESSAGE
                    struct server_access_allowed_message {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_access_allowed_message"; }
                        static constexpr unsigned index() { return 33; }
                        using type = configs::ServerNotification;
                        using sesman_and_spec_type = configs::ServerNotification;
                        type value{static_cast<type>(1)};
                    };
        // Warn that new server certificate file was created.
                    // AUTHID_MOD_RDP_SERVER_CERT_CREATE_MESSAGE
                    struct server_cert_create_message {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_cert_create_message"; }
                        static constexpr unsigned index() { return 34; }
                        using type = configs::ServerNotification;
                        using sesman_and_spec_type = configs::ServerNotification;
                        type value{static_cast<type>(1)};
                    };
        // Warn that server certificate file was successfully checked.
                    // AUTHID_MOD_RDP_SERVER_CERT_SUCCESS_MESSAGE
                    struct server_cert_success_message {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_cert_success_message"; }
                        static constexpr unsigned index() { return 35; }
                        using type = configs::ServerNotification;
                        using sesman_and_spec_type = configs::ServerNotification;
                        type value{static_cast<type>(1)};
                    };
        // Warn that server certificate file checking failed.
                    // AUTHID_MOD_RDP_SERVER_CERT_FAILURE_MESSAGE
                    struct server_cert_failure_message {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_cert_failure_message"; }
                        static constexpr unsigned index() { return 36; }
                        using type = configs::ServerNotification;
                        using sesman_and_spec_type = configs::ServerNotification;
                        type value{static_cast<type>(1)};
                    };
        // Warn that server certificate check raised some internal error.
                    // AUTHID_MOD_RDP_SERVER_CERT_ERROR_MESSAGE
                    struct server_cert_error_message {
                        static constexpr bool is_readable() { return 1; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "server_cert_error_message"; }
                        static constexpr unsigned index() { return 37; }
                        using type = configs::ServerNotification;
                        using sesman_and_spec_type = configs::ServerNotification;
                        type value{static_cast<type>(1)};
                    };
        // Do not transmit client machine name or RDP server.
                    struct hide_client_name {
                        static constexpr bool is_readable() { return 0; }
                        static constexpr bool is_writable() { return 0; }
                        static constexpr char const * section() { return "mod_rdp"; }
                        static constexpr char const * name() { return "hide_client_name"; }
                        using type = bool;
                        using sesman_and_spec_type = bool;
                        type value{0};
                    };
    };

    struct mod_vnc {
        // Enable or disable the clipboard from client (client to server).
                        // AUTHID_MOD_VNC_CLIPBOARD_UP
                        struct clipboard_up {
                            static constexpr bool is_readable() { return 1; }
                            static constexpr bool is_writable() { return 0; }
                            static constexpr char const * section() { return "mod_vnc"; }
                            static constexpr char const * name() { return "clipboard_up"; }
                            static constexpr unsigned index() { return 38; }
                            using type = bool;
                            using sesman_and_spec_type = bool;
                            type value{};
                        };
        // Enable or disable the clipboard from server (server to client).
                        // AUTHID_MOD_VNC_CLIPBOARD_DOWN
                        struct clipboard_down {
                            static constexpr bool is_readable() { return 1; }
                            static constexpr bool is_writable() { return 0; }
                            static constexpr char const * section() { return "mod_vnc"; }
                            static constexpr char const * name() { return "clipboard_down"; }
                            static constexpr unsigned index() { return 39; }
                            using type = bool;
                            using sesman_and_spec_type = bool;
                            type value{};
                        };

        // Sets the encoding types in which pixel data can be sent by the VNC server:
        //   0: Raw
        //   1: CopyRect
        //   2: RRE
        //   16: ZRLE
        //   -239 (0xFFFFFF11): Cursor pseudo-encoding
                        struct encodings {
                            static constexpr bool is_readable() { return 0; }
                            static constexpr bool is_writable() { return 0; }
                            static constexpr char const * section() { return "mod_vnc"; }
                            static constexpr char const * name() { return "encodings"; }
                            using type = std::string;
                            using sesman_and_spec_type = ::configs::spec_types::list<int>;
                            type value{};
                        };

                        struct allow_authentification_retries {
                            static constexpr bool is_readable() { return 0; }
                            static constexpr bool is_writable() { return 0; }
                            static constexpr char const * section() { return "mod_vnc"; }
                            static constexpr char const * name() { return "allow_authentification_retries"; }
                            using type = bool;
                            using sesman_and_spec_type = bool;
                            type value{0};
                        };

        // VNC server clipboard data encoding type.
                        // AUTHID_MOD_VNC_SERVER_CLIPBOARD_ENCODING_TYPE
                        struct server_clipboard_encoding_type {
                            static constexpr bool is_readable() { return 1; }
                            static constexpr bool is_writable() { return 0; }
                            static constexpr char const * section() { return "mod_vnc"; }
                            static constexpr char const * name() { return "server_clipboard_encoding_type"; }
                            static constexpr unsigned index() { return 40; }
                            using type = configs::ClipboardEncodingType;
                            using sesman_and_spec_type = configs::ClipboardEncodingType;
                            type value{static_cast<type>(1)};
                        };

                        // AUTHID_MOD_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP
                        struct bogus_clipboard_infinite_loop {
                            static constexpr bool is_readable() { return 1; }
                            static constexpr bool is_writable() { return 0; }
                            static constexpr char const * section() { return "mod_vnc"; }
                            static constexpr char const * name() { return "bogus_clipboard_infinite_loop"; }
                            static constexpr unsigned index() { return 41; }
                            using type = configs::VncBogusClipboardInfiniteLoop;
                            using sesman_and_spec_type = configs::VncBogusClipboardInfiniteLoop;
                            type value{static_cast<type>(0)};
                        };
    };

    struct mod_replay {
        // 0 - Wait for Escape, 1 - End session
                            struct on_end_of_data {
                                static constexpr bool is_readable() { return 0; }
                                static constexpr bool is_writable() { return 0; }
                                static constexpr char const * section() { return "mod_replay"; }
                                static constexpr char const * name() { return "on_end_of_data"; }
                                using type = bool;
                                using sesman_and_spec_type = bool;
                                type value{0};
                            };
    };

    struct video {
                                struct capture_groupid {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "capture_groupid"; }
                                    using type = unsigned int;
                                    using sesman_and_spec_type = unsigned int;
                                    type value{33};
                                };

                                struct capture_flags {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "capture_flags"; }
                                    using type = configs::CaptureFlags;
                                    using sesman_and_spec_type = configs::CaptureFlags;
                                    type value{static_cast<type>(3)};
                                };

        // Frame interval is in 1/10 s.
                                struct png_interval {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "png_interval"; }
                                    using type = unsigned int;
                                    using sesman_and_spec_type = unsigned int;
                                    type value{10};
                                };
        // Frame interval is in 1/100 s.
                                struct frame_interval {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "frame_interval"; }
                                    using type = unsigned int;
                                    using sesman_and_spec_type = unsigned int;
                                    type value{40};
                                };
        // Time between 2 wrm movies (in seconds).
                                struct break_interval {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "break_interval"; }
                                    using type = unsigned int;
                                    using sesman_and_spec_type = unsigned int;
                                    type value{600};
                                };
        // Number of png captures to keep.
                                struct png_limit {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "png_limit"; }
                                    using type = unsigned int;
                                    using sesman_and_spec_type = unsigned int;
                                    type value{5};
                                };

                                struct replay_path {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "replay_path"; }
                                    using type = ::configs::spec_types::directory_path;
                                    using sesman_and_spec_type = ::configs::spec_types::directory_path;
                                    type value = "/tmp/";
                                };

                                struct hash_path {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "hash_path"; }
                                    using type = ::configs::spec_types::directory_path;
                                    using sesman_and_spec_type = ::configs::spec_types::directory_path;
                                    type value = HASH_PATH;
                                };
                                struct record_tmp_path {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "record_tmp_path"; }
                                    using type = ::configs::spec_types::directory_path;
                                    using sesman_and_spec_type = ::configs::spec_types::directory_path;
                                    type value = RECORD_TMP_PATH;
                                };
                                struct record_path {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "record_path"; }
                                    using type = ::configs::spec_types::directory_path;
                                    using sesman_and_spec_type = ::configs::spec_types::directory_path;
                                    type value = RECORD_PATH;
                                };

                                struct inactivity_pause {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "inactivity_pause"; }
                                    using type = bool;
                                    type value{0};
                                };
                                struct inactivity_timeout {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "inactivity_timeout"; }
                                    using type = unsigned int;
                                    type value{300};
                                };

        // Disable keyboard log:
                                // AUTHID_VIDEO_DISABLE_KEYBOARD_LOG
                                struct disable_keyboard_log {
                                    static constexpr bool is_readable() { return 1; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "disable_keyboard_log"; }
                                    static constexpr unsigned index() { return 42; }
                                    using type = configs::KeyboardLogFlags;
                                    using sesman_and_spec_type = configs::KeyboardLogFlags;
                                    type value{static_cast<type>(1)};
                                };

        // Disable clipboard log:
                                // AUTHID_VIDEO_DISABLE_CLIPBOARD_LOG
                                struct disable_clipboard_log {
                                    static constexpr bool is_readable() { return 1; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "disable_clipboard_log"; }
                                    static constexpr unsigned index() { return 43; }
                                    using type = configs::ClipboardLogFlags;
                                    using sesman_and_spec_type = configs::ClipboardLogFlags;
                                    type value{static_cast<type>(1)};
                                };

        // Disable (redirected) file system log:
                                // AUTHID_VIDEO_DISABLE_FILE_SYSTEM_LOG
                                struct disable_file_system_log {
                                    static constexpr bool is_readable() { return 1; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "disable_file_system_log"; }
                                    static constexpr unsigned index() { return 44; }
                                    using type = configs::FileSystemLogFlags;
                                    using sesman_and_spec_type = configs::FileSystemLogFlags;
                                    type value{static_cast<type>(1)};
                                };

                                // AUTHID_VIDEO_RT_DISPLAY
                                struct rt_display {
                                    static constexpr bool is_readable() { return 1; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "rt_display"; }
                                    static constexpr unsigned index() { return 45; }
                                    using type = unsigned int;
                                    using sesman_and_spec_type = unsigned int;
                                    type value{0};
                                };

                                struct wrm_color_depth_selection_strategy {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "wrm_color_depth_selection_strategy"; }
                                    using type = configs::ColorDepthSelectionStrategy;
                                    using sesman_and_spec_type = configs::ColorDepthSelectionStrategy;
                                    type value{static_cast<type>(1)};
                                };
                                struct wrm_compression_algorithm {
                                    static constexpr bool is_readable() { return 0; }
                                    static constexpr bool is_writable() { return 0; }
                                    static constexpr char const * section() { return "video"; }
                                    static constexpr char const * name() { return "wrm_compression_algorithm"; }
                                    using type = configs::WrmCompressionAlgorithm;
                                    using sesman_and_spec_type = configs::WrmCompressionAlgorithm;
                                    type value{static_cast<type>(1)};
                                };
    };

    struct crypto {
                                    // AUTHID_CRYPTO_KEY0
                                    struct key0 {
                                        static constexpr bool is_readable() { return 1; }
                                        static constexpr bool is_writable() { return 0; }
                                        static constexpr char const * section() { return "crypto"; }
                                        static constexpr char const * name() { return "key0"; }
                                        static constexpr unsigned index() { return 46; }
                                        using type = std::array<unsigned char, 32>;
                                        using sesman_and_spec_type = ::configs::spec_types::fixed_binary<32>;
                                        type value{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, }};
                                    };
                                    // AUTHID_CRYPTO_KEY1
                                    struct key1 {
                                        static constexpr bool is_readable() { return 1; }
                                        static constexpr bool is_writable() { return 0; }
                                        static constexpr char const * section() { return "crypto"; }
                                        static constexpr char const * name() { return "key1"; }
                                        static constexpr unsigned index() { return 47; }
                                        using type = std::array<unsigned char, 32>;
                                        using sesman_and_spec_type = ::configs::spec_types::fixed_binary<32>;
                                        type value{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, }};
                                    };
    };

    struct debug {
                                        struct x224 {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "x224"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct mcs {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "mcs"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct sec {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "sec"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct rdp {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "rdp"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct primary_orders {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "primary_orders"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct secondary_orders {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "secondary_orders"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct bitmap {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "bitmap"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct capture {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "capture"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct auth {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "auth"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct session {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "session"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct front {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "front"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct mod_rdp {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "mod_rdp"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct mod_vnc {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "mod_vnc"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct mod_int {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "mod_int"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct mod_xup {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "mod_xup"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct widget {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "widget"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct input {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "input"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct password {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "password"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct compression {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "compression"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct cache {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "cache"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct bitmap_update {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "bitmap_update"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct performance {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "performance"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };
                                        struct pass_dialog_box {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "pass_dialog_box"; }
                                            using type = uint32_t;
                                            using sesman_and_spec_type = uint32_t;
                                            type value{};
                                        };

                                        struct config {
                                            static constexpr bool is_readable() { return 0; }
                                            static constexpr bool is_writable() { return 0; }
                                            static constexpr char const * section() { return "debug"; }
                                            static constexpr char const * name() { return "config"; }
                                            using type = unsigned int;
                                            using sesman_and_spec_type = unsigned int;
                                            type value{2};
                                        };
    };

    struct translation {
                                            // AUTHID_TRANSLATION_LANGUAGE
                                            struct language {
                                                static constexpr bool is_readable() { return 1; }
                                                static constexpr bool is_writable() { return 0; }
                                                static constexpr char const * section() { return "translation"; }
                                                static constexpr char const * name() { return "language"; }
                                                static constexpr unsigned index() { return 48; }
                                                using type = configs::Language;
                                                using sesman_and_spec_type = configs::Language;
                                                type value{static_cast<type>(0)};
                                            };
                                            // AUTHID_TRANSLATION_PASSWORD_EN
                                            struct password_en {
                                                static constexpr bool is_readable() { return 1; }
                                                static constexpr bool is_writable() { return 0; }
                                                static constexpr char const * section() { return "translation"; }
                                                static constexpr char const * name() { return "password_en"; }
                                                static constexpr unsigned index() { return 49; }
                                                using type = std::basic_string<char>;
                                                using sesman_and_spec_type = std::basic_string<char>;
                                                type value{};
                                            };
                                            // AUTHID_TRANSLATION_PASSWORD_FR
                                            struct password_fr {
                                                static constexpr bool is_readable() { return 1; }
                                                static constexpr bool is_writable() { return 0; }
                                                static constexpr char const * section() { return "translation"; }
                                                static constexpr char const * name() { return "password_fr"; }
                                                static constexpr unsigned index() { return 50; }
                                                using type = std::basic_string<char>;
                                                using sesman_and_spec_type = std::basic_string<char>;
                                                type value{};
                                            };
    };

    struct internal_mod {
                                                struct theme {
                                                    static constexpr bool is_readable() { return 0; }
                                                    static constexpr bool is_writable() { return 0; }
                                                    static constexpr char const * section() { return "internal_mod"; }
                                                    static constexpr char const * name() { return "theme"; }
                                                    using type = std::basic_string<char>;
                                                    using sesman_and_spec_type = std::basic_string<char>;
                                                    type value{};
                                                };
    };

    struct context {
                                                    struct movie {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "movie"; }
                                                        using type = ::configs::spec_types::directory_path;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_OPT_BITRATE
                                                    struct opt_bitrate {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_bitrate"; }
                                                        static constexpr unsigned index() { return 51; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{40000};
                                                    };
                                                    // AUTHID_CONTEXT_OPT_FRAMERATE
                                                    struct opt_framerate {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_framerate"; }
                                                        static constexpr unsigned index() { return 52; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{5};
                                                    };
                                                    // AUTHID_CONTEXT_OPT_QSCALE
                                                    struct opt_qscale {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_qscale"; }
                                                        static constexpr unsigned index() { return 53; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{15};
                                                    };

                                                    // AUTHID_CONTEXT_OPT_BPP
                                                    struct opt_bpp {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_bpp"; }
                                                        static constexpr unsigned index() { return 54; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{24};
                                                    };
                                                    // AUTHID_CONTEXT_OPT_HEIGHT
                                                    struct opt_height {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_height"; }
                                                        static constexpr unsigned index() { return 55; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{600};
                                                    };
                                                    // AUTHID_CONTEXT_OPT_WIDTH
                                                    struct opt_width {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_width"; }
                                                        static constexpr unsigned index() { return 56; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{800};
                                                    };

                                                    // AUTHID_CONTEXT_AUTH_ERROR_MESSAGE
                                                    struct auth_error_message {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "auth_error_message"; }
                                                        static constexpr unsigned index() { return 57; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_SELECTOR
                                                    struct selector {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector"; }
                                                        static constexpr unsigned index() { return 58; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{0};
                                                    };
                                                    // AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE
                                                    struct selector_current_page {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector_current_page"; }
                                                        static constexpr unsigned index() { return 59; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{1};
                                                    };
                                                    // AUTHID_CONTEXT_SELECTOR_DEVICE_FILTER
                                                    struct selector_device_filter {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector_device_filter"; }
                                                        static constexpr unsigned index() { return 60; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_SELECTOR_GROUP_FILTER
                                                    struct selector_group_filter {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector_group_filter"; }
                                                        static constexpr unsigned index() { return 61; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_SELECTOR_PROTO_FILTER
                                                    struct selector_proto_filter {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector_proto_filter"; }
                                                        static constexpr unsigned index() { return 62; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_SELECTOR_LINES_PER_PAGE
                                                    struct selector_lines_per_page {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector_lines_per_page"; }
                                                        static constexpr unsigned index() { return 63; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{0};
                                                    };
                                                    // AUTHID_CONTEXT_SELECTOR_NUMBER_OF_PAGES
                                                    struct selector_number_of_pages {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "selector_number_of_pages"; }
                                                        static constexpr unsigned index() { return 64; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{1};
                                                    };

                                                    // AUTHID_CONTEXT_TARGET_PASSWORD
                                                    struct target_password {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "target_password"; }
                                                        static constexpr unsigned index() { return 65; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_TARGET_HOST
                                                    struct target_host {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "target_host"; }
                                                        static constexpr unsigned index() { return 66; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_TARGET_SERVICE
                                                    struct target_service {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "target_service"; }
                                                        static constexpr unsigned index() { return 67; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_TARGET_PORT
                                                    struct target_port {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "target_port"; }
                                                        static constexpr unsigned index() { return 68; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{3389};
                                                    };
                                                    // AUTHID_CONTEXT_TARGET_PROTOCOL
                                                    struct target_protocol {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "target_protocol"; }
                                                        static constexpr unsigned index() { return 69; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value = "RDP";
                                                    };

                                                    // AUTHID_CONTEXT_PASSWORD
                                                    struct password {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "password"; }
                                                        static constexpr unsigned index() { return 70; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_REPORTING
                                                    struct reporting {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "reporting"; }
                                                        static constexpr unsigned index() { return 71; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_AUTH_CHANNEL_ANSWER
                                                    struct auth_channel_answer {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "auth_channel_answer"; }
                                                        static constexpr unsigned index() { return 72; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_AUTH_CHANNEL_RESULT
                                                    struct auth_channel_result {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "auth_channel_result"; }
                                                        static constexpr unsigned index() { return 73; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_AUTH_CHANNEL_TARGET
                                                    struct auth_channel_target {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "auth_channel_target"; }
                                                        static constexpr unsigned index() { return 74; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_MESSAGE
                                                    struct message {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "message"; }
                                                        static constexpr unsigned index() { return 75; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_ACCEPT_MESSAGE
                                                    struct accept_message {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "accept_message"; }
                                                        static constexpr unsigned index() { return 76; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_DISPLAY_MESSAGE
                                                    struct display_message {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "display_message"; }
                                                        static constexpr unsigned index() { return 77; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_REJECTED
                                                    struct rejected {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "rejected"; }
                                                        static constexpr unsigned index() { return 78; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_AUTHENTICATED
                                                    struct authenticated {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "authenticated"; }
                                                        static constexpr unsigned index() { return 79; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{0};
                                                    };

                                                    // AUTHID_CONTEXT_KEEPALIVE
                                                    struct keepalive {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "keepalive"; }
                                                        static constexpr unsigned index() { return 80; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{0};
                                                    };

                                                    // AUTHID_CONTEXT_SESSION_ID
                                                    struct session_id {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "session_id"; }
                                                        static constexpr unsigned index() { return 81; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_END_DATE_CNX
                                                    struct end_date_cnx {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "end_date_cnx"; }
                                                        static constexpr unsigned index() { return 82; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{0};
                                                    };
                                                    // AUTHID_CONTEXT_END_TIME
                                                    struct end_time {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "end_time"; }
                                                        static constexpr unsigned index() { return 83; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_MODE_CONSOLE
                                                    struct mode_console {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "mode_console"; }
                                                        static constexpr unsigned index() { return 84; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value = "allow";
                                                    };
                                                    // AUTHID_CONTEXT_TIMEZONE
                                                    struct timezone {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "timezone"; }
                                                        static constexpr unsigned index() { return 85; }
                                                        using type = int;
                                                        using sesman_and_spec_type = int;
                                                        type value{-3600};
                                                    };

                                                    // AUTHID_CONTEXT_REAL_TARGET_DEVICE
                                                    struct real_target_device {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "real_target_device"; }
                                                        static constexpr unsigned index() { return 86; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_AUTHENTICATION_CHALLENGE
                                                    struct authentication_challenge {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "authentication_challenge"; }
                                                        static constexpr unsigned index() { return 87; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_TICKET
                                                    struct ticket {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "ticket"; }
                                                        static constexpr unsigned index() { return 88; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_COMMENT
                                                    struct comment {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "comment"; }
                                                        static constexpr unsigned index() { return 89; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_DURATION
                                                    struct duration {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "duration"; }
                                                        static constexpr unsigned index() { return 90; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_WAITINFORETURN
                                                    struct waitinforeturn {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "waitinforeturn"; }
                                                        static constexpr unsigned index() { return 91; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_SHOWFORM
                                                    struct showform {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "showform"; }
                                                        static constexpr unsigned index() { return 92; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{0};
                                                    };
                                                    // AUTHID_CONTEXT_FORMFLAG
                                                    struct formflag {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "formflag"; }
                                                        static constexpr unsigned index() { return 93; }
                                                        using type = unsigned int;
                                                        using sesman_and_spec_type = unsigned int;
                                                        type value{0};
                                                    };

                                                    // AUTHID_CONTEXT_MODULE
                                                    struct module {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "module"; }
                                                        static constexpr unsigned index() { return 94; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value = "login";
                                                    };
                                                    // AUTHID_CONTEXT_FORCEMODULE
                                                    struct forcemodule {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "forcemodule"; }
                                                        static constexpr unsigned index() { return 95; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{0};
                                                    };
                                                    // AUTHID_CONTEXT_PROXY_OPT
                                                    struct proxy_opt {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "proxy_opt"; }
                                                        static constexpr unsigned index() { return 96; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_PATTERN_KILL
                                                    struct pattern_kill {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "pattern_kill"; }
                                                        static constexpr unsigned index() { return 97; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_PATTERN_NOTIFY
                                                    struct pattern_notify {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "pattern_notify"; }
                                                        static constexpr unsigned index() { return 98; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_OPT_MESSAGE
                                                    struct opt_message {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "opt_message"; }
                                                        static constexpr unsigned index() { return 99; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    // AUTHID_CONTEXT_OUTBOUND_CONNECTION_BLOCKING_RULES
                                                    struct outbound_connection_blocking_rules {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "outbound_connection_blocking_rules"; }
                                                        static constexpr unsigned index() { return 100; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };

                                                    struct manager_disconnect_reason {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "manager_disconnect_reason"; }
                                                        using type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_DISCONNECT_REASON
                                                    struct disconnect_reason {
                                                        static constexpr bool is_readable() { return 1; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "disconnect_reason"; }
                                                        static constexpr unsigned index() { return 101; }
                                                        using type = std::basic_string<char>;
                                                        using sesman_and_spec_type = std::basic_string<char>;
                                                        type value{};
                                                    };
                                                    // AUTHID_CONTEXT_DISCONNECT_REASON_ACK
                                                    struct disconnect_reason_ack {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 1; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "disconnect_reason_ack"; }
                                                        static constexpr unsigned index() { return 102; }
                                                        using type = bool;
                                                        using sesman_and_spec_type = bool;
                                                        type value{0};
                                                    };

                                                    struct ip_target {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return "context"; }
                                                        static constexpr char const * name() { return "ip_target"; }
                                                        using type = std::basic_string<char>;
                                                        type value{};
                                                    };
    };

                                                    struct theme {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return ""; }
                                                        static constexpr char const * name() { return "theme"; }
                                                        using type = Theme;
                                                        type value{};
                                                    };
                                                    struct font {
                                                        static constexpr bool is_readable() { return 0; }
                                                        static constexpr bool is_writable() { return 0; }
                                                        static constexpr char const * section() { return ""; }
                                                        static constexpr char const * name() { return "font"; }
                                                        using type = Font;
                                                        font(char const * filename) : value(filename) {}
                                                        type value;
                                                    };

}

namespace cfg_section {
struct client
: cfg::client::keyboard_layout
, cfg::client::keyboard_layout_proposals
, cfg::client::ignore_logon_password
, cfg::client::performance_flags_default
, cfg::client::performance_flags_force_present
, cfg::client::performance_flags_force_not_present
, cfg::client::auto_adjust_performance_flags
, cfg::client::tls_fallback_legacy
, cfg::client::tls_support
, cfg::client::bogus_neg_request
, cfg::client::bogus_user_id
, cfg::client::disable_tsk_switch_shortcuts
, cfg::client::rdp_compression
, cfg::client::max_color_depth
, cfg::client::persistent_disk_bitmap_cache
, cfg::client::cache_waiting_list
, cfg::client::persist_bitmap_cache_on_disk
, cfg::client::bitmap_compression
, cfg::client::fast_path
, cfg::client::enable_suppress_output
{ static constexpr bool is_section = true; };

struct context
: cfg::context::movie
, cfg::context::opt_bitrate
, cfg::context::opt_framerate
, cfg::context::opt_qscale
, cfg::context::opt_bpp
, cfg::context::opt_height
, cfg::context::opt_width
, cfg::context::auth_error_message
, cfg::context::selector
, cfg::context::selector_current_page
, cfg::context::selector_device_filter
, cfg::context::selector_group_filter
, cfg::context::selector_proto_filter
, cfg::context::selector_lines_per_page
, cfg::context::selector_number_of_pages
, cfg::context::target_password
, cfg::context::target_host
, cfg::context::target_service
, cfg::context::target_port
, cfg::context::target_protocol
, cfg::context::password
, cfg::context::reporting
, cfg::context::auth_channel_answer
, cfg::context::auth_channel_result
, cfg::context::auth_channel_target
, cfg::context::message
, cfg::context::accept_message
, cfg::context::display_message
, cfg::context::rejected
, cfg::context::authenticated
, cfg::context::keepalive
, cfg::context::session_id
, cfg::context::end_date_cnx
, cfg::context::end_time
, cfg::context::mode_console
, cfg::context::timezone
, cfg::context::real_target_device
, cfg::context::authentication_challenge
, cfg::context::ticket
, cfg::context::comment
, cfg::context::duration
, cfg::context::waitinforeturn
, cfg::context::showform
, cfg::context::formflag
, cfg::context::module
, cfg::context::forcemodule
, cfg::context::proxy_opt
, cfg::context::pattern_kill
, cfg::context::pattern_notify
, cfg::context::opt_message
, cfg::context::outbound_connection_blocking_rules
, cfg::context::manager_disconnect_reason
, cfg::context::disconnect_reason
, cfg::context::disconnect_reason_ack
, cfg::context::ip_target
{ static constexpr bool is_section = true; };

struct crypto
: cfg::crypto::key0
, cfg::crypto::key1
{ static constexpr bool is_section = true; };

struct debug
: cfg::debug::x224
, cfg::debug::mcs
, cfg::debug::sec
, cfg::debug::rdp
, cfg::debug::primary_orders
, cfg::debug::secondary_orders
, cfg::debug::bitmap
, cfg::debug::capture
, cfg::debug::auth
, cfg::debug::session
, cfg::debug::front
, cfg::debug::mod_rdp
, cfg::debug::mod_vnc
, cfg::debug::mod_int
, cfg::debug::mod_xup
, cfg::debug::widget
, cfg::debug::input
, cfg::debug::password
, cfg::debug::compression
, cfg::debug::cache
, cfg::debug::bitmap_update
, cfg::debug::performance
, cfg::debug::pass_dialog_box
, cfg::debug::config
{ static constexpr bool is_section = true; };

struct globals
: cfg::globals::capture_chunk
, cfg::globals::auth_user
, cfg::globals::host
, cfg::globals::target
, cfg::globals::target_device
, cfg::globals::device_id
, cfg::globals::target_user
, cfg::globals::target_application
, cfg::globals::target_application_account
, cfg::globals::target_application_password
, cfg::globals::bitmap_cache
, cfg::globals::glyph_cache
, cfg::globals::port
, cfg::globals::nomouse
, cfg::globals::notimestamp
, cfg::globals::encryptionLevel
, cfg::globals::authfile
, cfg::globals::handshake_timeout
, cfg::globals::session_timeout
, cfg::globals::keepalive_grace_delay
, cfg::globals::authentication_timeout
, cfg::globals::close_timeout
, cfg::globals::trace_type
, cfg::globals::listen_address
, cfg::globals::enable_ip_transparent
, cfg::globals::certificate_password
, cfg::globals::png_path
, cfg::globals::wrm_path
, cfg::globals::is_rec
, cfg::globals::movie_path
, cfg::globals::enable_bitmap_update
, cfg::globals::enable_close_box
, cfg::globals::enable_osd
, cfg::globals::enable_osd_display_remote_target
, cfg::globals::persistent_path
, cfg::globals::disable_proxy_opt
, cfg::globals::allow_using_multiple_monitors
, cfg::globals::bogus_refresh_rect
{ static constexpr bool is_section = true; };

struct internal_mod
: cfg::internal_mod::theme
{ static constexpr bool is_section = true; };

struct mod_rdp
: cfg::mod_rdp::rdp_compression
, cfg::mod_rdp::disconnect_on_logon_user_change
, cfg::mod_rdp::open_session_timeout
, cfg::mod_rdp::extra_orders
, cfg::mod_rdp::enable_nla
, cfg::mod_rdp::enable_kerberos
, cfg::mod_rdp::persistent_disk_bitmap_cache
, cfg::mod_rdp::cache_waiting_list
, cfg::mod_rdp::persist_bitmap_cache_on_disk
, cfg::mod_rdp::allow_channels
, cfg::mod_rdp::deny_channels
, cfg::mod_rdp::fast_path
, cfg::mod_rdp::server_redirection_support
, cfg::mod_rdp::redir_info
, cfg::mod_rdp::bogus_sc_net_size
, cfg::mod_rdp::bogus_linux_cursor
, cfg::mod_rdp::proxy_managed_drives
, cfg::mod_rdp::ignore_auth_channel
, cfg::mod_rdp::auth_channel
, cfg::mod_rdp::alternate_shell
, cfg::mod_rdp::shell_working_directory
, cfg::mod_rdp::use_client_provided_alternate_shell
, cfg::mod_rdp::enable_session_probe
, cfg::mod_rdp::session_probe_use_clipboard_based_launcher
, cfg::mod_rdp::enable_session_probe_launch_mask
, cfg::mod_rdp::session_probe_on_launch_failure
, cfg::mod_rdp::session_probe_launch_timeout
, cfg::mod_rdp::session_probe_launch_fallback_timeout
, cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon
, cfg::mod_rdp::session_probe_keepalive_timeout
, cfg::mod_rdp::session_probe_on_keepalive_timeout_disconnect_user
, cfg::mod_rdp::session_probe_end_disconnected_session
, cfg::mod_rdp::session_probe_customize_executable_name
, cfg::mod_rdp::session_probe_alternate_shell
, cfg::mod_rdp::server_cert_store
, cfg::mod_rdp::server_cert_check
, cfg::mod_rdp::server_access_allowed_message
, cfg::mod_rdp::server_cert_create_message
, cfg::mod_rdp::server_cert_success_message
, cfg::mod_rdp::server_cert_failure_message
, cfg::mod_rdp::server_cert_error_message
, cfg::mod_rdp::hide_client_name
{ static constexpr bool is_section = true; };

struct mod_replay
: cfg::mod_replay::on_end_of_data
{ static constexpr bool is_section = true; };

struct mod_vnc
: cfg::mod_vnc::clipboard_up
, cfg::mod_vnc::clipboard_down
, cfg::mod_vnc::encodings
, cfg::mod_vnc::allow_authentification_retries
, cfg::mod_vnc::server_clipboard_encoding_type
, cfg::mod_vnc::bogus_clipboard_infinite_loop
{ static constexpr bool is_section = true; };

struct session_log
: cfg::session_log::enable_session_log
, cfg::session_log::keyboard_input_masking_level
{ static constexpr bool is_section = true; };

struct translation
: cfg::translation::language
, cfg::translation::password_en
, cfg::translation::password_fr
{ static constexpr bool is_section = true; };

struct video
: cfg::video::capture_groupid
, cfg::video::capture_flags
, cfg::video::png_interval
, cfg::video::frame_interval
, cfg::video::break_interval
, cfg::video::png_limit
, cfg::video::replay_path
, cfg::video::hash_path
, cfg::video::record_tmp_path
, cfg::video::record_path
, cfg::video::inactivity_pause
, cfg::video::inactivity_timeout
, cfg::video::disable_keyboard_log
, cfg::video::disable_clipboard_log
, cfg::video::disable_file_system_log
, cfg::video::rt_display
, cfg::video::wrm_color_depth_selection_strategy
, cfg::video::wrm_compression_algorithm
{ static constexpr bool is_section = true; };

}

namespace configs {
struct VariablesConfiguration
: cfg_section::client
, cfg_section::context
, cfg_section::crypto
, cfg_section::debug
, cfg_section::globals
, cfg_section::internal_mod
, cfg_section::mod_rdp
, cfg_section::mod_replay
, cfg_section::mod_vnc
, cfg_section::session_log
, cfg_section::translation
, cfg_section::video
, cfg::theme
, cfg::font
{
    explicit VariablesConfiguration(char const * default_font_name)
    : cfg::font{default_font_name}
    {}
};

using VariablesAclPack = Pack<
  cfg::globals::capture_chunk
, cfg::globals::auth_user
, cfg::globals::host
, cfg::globals::target
, cfg::globals::target_device
, cfg::globals::device_id
, cfg::globals::target_user
, cfg::globals::target_application
, cfg::globals::target_application_account
, cfg::globals::target_application_password
, cfg::globals::trace_type
, cfg::globals::is_rec
, cfg::globals::movie_path
, cfg::client::keyboard_layout
, cfg::client::disable_tsk_switch_shortcuts
, cfg::mod_rdp::bogus_sc_net_size
, cfg::mod_rdp::proxy_managed_drives
, cfg::mod_rdp::ignore_auth_channel
, cfg::mod_rdp::alternate_shell
, cfg::mod_rdp::shell_working_directory
, cfg::mod_rdp::use_client_provided_alternate_shell
, cfg::mod_rdp::enable_session_probe
, cfg::mod_rdp::session_probe_use_clipboard_based_launcher
, cfg::mod_rdp::enable_session_probe_launch_mask
, cfg::mod_rdp::session_probe_on_launch_failure
, cfg::mod_rdp::session_probe_launch_timeout
, cfg::mod_rdp::session_probe_launch_fallback_timeout
, cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon
, cfg::mod_rdp::session_probe_keepalive_timeout
, cfg::mod_rdp::session_probe_on_keepalive_timeout_disconnect_user
, cfg::mod_rdp::session_probe_end_disconnected_session
, cfg::mod_rdp::server_cert_store
, cfg::mod_rdp::server_cert_check
, cfg::mod_rdp::server_access_allowed_message
, cfg::mod_rdp::server_cert_create_message
, cfg::mod_rdp::server_cert_success_message
, cfg::mod_rdp::server_cert_failure_message
, cfg::mod_rdp::server_cert_error_message
, cfg::mod_vnc::clipboard_up
, cfg::mod_vnc::clipboard_down
, cfg::mod_vnc::server_clipboard_encoding_type
, cfg::mod_vnc::bogus_clipboard_infinite_loop
, cfg::video::disable_keyboard_log
, cfg::video::disable_clipboard_log
, cfg::video::disable_file_system_log
, cfg::video::rt_display
, cfg::crypto::key0
, cfg::crypto::key1
, cfg::translation::language
, cfg::translation::password_en
, cfg::translation::password_fr
, cfg::context::opt_bitrate
, cfg::context::opt_framerate
, cfg::context::opt_qscale
, cfg::context::opt_bpp
, cfg::context::opt_height
, cfg::context::opt_width
, cfg::context::auth_error_message
, cfg::context::selector
, cfg::context::selector_current_page
, cfg::context::selector_device_filter
, cfg::context::selector_group_filter
, cfg::context::selector_proto_filter
, cfg::context::selector_lines_per_page
, cfg::context::selector_number_of_pages
, cfg::context::target_password
, cfg::context::target_host
, cfg::context::target_service
, cfg::context::target_port
, cfg::context::target_protocol
, cfg::context::password
, cfg::context::reporting
, cfg::context::auth_channel_answer
, cfg::context::auth_channel_result
, cfg::context::auth_channel_target
, cfg::context::message
, cfg::context::accept_message
, cfg::context::display_message
, cfg::context::rejected
, cfg::context::authenticated
, cfg::context::keepalive
, cfg::context::session_id
, cfg::context::end_date_cnx
, cfg::context::end_time
, cfg::context::mode_console
, cfg::context::timezone
, cfg::context::real_target_device
, cfg::context::authentication_challenge
, cfg::context::ticket
, cfg::context::comment
, cfg::context::duration
, cfg::context::waitinforeturn
, cfg::context::showform
, cfg::context::formflag
, cfg::context::module
, cfg::context::forcemodule
, cfg::context::proxy_opt
, cfg::context::pattern_kill
, cfg::context::pattern_notify
, cfg::context::opt_message
, cfg::context::outbound_connection_blocking_rules
, cfg::context::disconnect_reason
, cfg::context::disconnect_reason_ack
>;
}
