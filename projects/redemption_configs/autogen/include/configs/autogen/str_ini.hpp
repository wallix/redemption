#include "config_variant.hpp"

R"gen_config_ini(## Config file for RDP proxy.


[globals]

# value: 0 or 1
#_advanced
#glyph_cache = 0

# Warning: Service will be automatically restarted and active sessions will be disconnected
# The port set in this field must not be already used, otherwise the service will not run.
# min = 0
#_iptables
#_advanced
#port = 3389

# value: 0 or 1
#_advanced
#nomouse = 0

# values: 'low', 'medium', 'high'
#_advanced
#encryptionLevel = low

#_advanced
#authfile = )gen_config_ini" << (REDEMPTION_CONFIG_AUTHFILE) << R"gen_config_ini(

# Time out during RDP handshake stage.
# (is in second)
#handshake_timeout = 10

# No traffic auto disconnection.
# (is in second)
#session_timeout = 900

# No traffic auto disconnection.
# If value is 0, global value (session_timeout) is used.
# (is in second)
#_hidden
#inactivity_timeout = 0

# Keepalive.
# (is in second)
#_advanced
#keepalive_grace_delay = 30

# Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).
# (is in second)
#_advanced
#authentication_timeout = 120

# Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).
# (is in second)
#_advanced
#close_timeout = 600

# Session record options.
# min = 0, max = 2
#   0: No encryption (faster).
#   1: No encryption, with checksum.
#   2: Encryption enabled.
# When session records are encrypted, they can be read only by the WAB where they have been generated.
#_advanced
#trace_type = 1

#_advanced
#listen_address = 0.0.0.0

# Allow Transparent mode.
# value: 0 or 1
#_iptables
#enable_transparent_mode = 0

# Proxy certificate password.
# maxlen = 254
#_advanced
#_password
#certificate_password = inquisition

#_advanced
#movie_path = 

# Support of Bitmap Update.
# value: 0 or 1
#_advanced
#enable_bitmap_update = 1

# Show close screen.
# value: 0 or 1
#enable_close_box = 1

# value: 0 or 1
#_advanced
#enable_osd = 1

# value: 0 or 1
#_advanced
#enable_osd_display_remote_target = 1

# value: 0 or 1
#_hidden
#enable_wab_integration = )gen_config_ini" << (REDEMPTION_CONFIG_ENABLE_WAB_INTEGRATION) << R"gen_config_ini(

# value: 0 or 1
#allow_using_multiple_monitors = 1

# Needed to refresh screen of Windows Server 2012.
# value: 0 or 1
#_advanced
#bogus_refresh_rect = 1

# value: 0 or 1
#_advanced
#large_pointer_support = 1

# value: 0 or 1
#unicode_keyboard_event_support = 1

# min = 100, max = 10000
# (is in millisecond)
#_advanced
#mod_recv_timeout = 1000

# value: 0 or 1
#_advanced
#spark_view_specific_glyph_width = 0

# value: 0 or 1
#_advanced
#experimental_enable_serializer_data_block_size_limit = 0

# value: 0 or 1
#_advanced
#experimental_support_resize_session_during_recording = 1

# value: 0 or 1
#_advanced
#support_connection_redirection_during_recording = 1

# Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.
# !!!May cause FreeRDP-based client to CRASH!!!
# Set to 0 to disable this feature.
# (is in millisecond)
#rdp_keepalive_connection_interval = 0

[session_log]

# value: 0 or 1
#enable_session_log = 1

# value: 0 or 1
#enable_arcsight_log = 0

# Keyboard Input Masking Level:
# min = 0, max = 3
#   0: keyboard input are not masked
#   1: only passwords are masked
#   2: passwords and unidentified texts are masked
#   3: keyboard inputs are not logged
#_hidden
#keyboard_input_masking_level = 2

# value: 0 or 1
#_advanced
#hide_non_printable_kbd_input = 0

[client]

# cs-CZ, da-DK, de-DE, el-GR, en-US, es-ES, fi-FI.finnish, fr-FR, is-IS, it-IT, nl-NL, nb-NO, pl-PL.programmers, pt-BR.abnt, ro-RO, ru-RU, hr-HR, sk-SK, sv-SE, tr-TR.q, uk-UA, sl-SI, et-EE, lv-LV, lt-LT.ibm, mk-MK, fo-FO, mt-MT.47, se-NO, kk-KZ, ky-KG, tt-RU, mn-MN, cy-GB, lb-LU, mi-NZ, de-CH, en-GB, es-MX, fr-BE.fr, nl-BE, pt-PT, sr-La, se-SE, uz-Cy, iu-La, fr-CA, sr-Cy, en-CA.fr, fr-CH, bs-Cy, bg-BG.latin, cs-CZ.qwerty, en-IE.irish, de-DE.ibm, el-GR.220, es-ES.variation, hu-HU, en-US.dvorak, it-IT.142, pl-PL, pt-BR.abnt2, ru-RU.typewriter, sk-SK.qwerty, tr-TR.f, lv-LV.qwerty, lt-LT, mt-MT.48, se-NO.ext_norway, fr-BE, se-SE, en-CA.multilingual, en-IE, cs-CZ.programmers, el-GR.319, en-US.international, se-SE.ext_finland_sweden, bg-BG, el-GR.220_latin, en-US.dvorak_left, el-GR.319_latin, en-US.dvorak_right, el-GR.latin, el-GR.polytonic
#_advanced
#keyboard_layout_proposals = en-US, fr-FR, de-DE, ru-RU

# If true, ignore password provided by RDP client, user need do login manually.
# value: 0 or 1
#_advanced
#ignore_logon_password = 0

# Enable font smoothing (0x80).
# min = 0
#_advanced
#_hex
#performance_flags_default = 128

# Disable theme (0x8).
# Disable mouse cursor shadows (0x20).
# min = 0
#_advanced
#_hex
#performance_flags_force_present = 40

# min = 0
#_advanced
#_hex
#performance_flags_force_not_present = 0

# If enabled, avoid automatically font smoothing in recorded session.
# value: 0 or 1
#_advanced
#auto_adjust_performance_flags = 1

# Fallback to RDP Legacy Encryption if client does not support TLS.
# value: 0 or 1
#tls_fallback_legacy = 0

# value: 0 or 1
#tls_support = 1

# Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
# min = 0
#tls_min_level = 2

# Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
# min = 0
#tls_max_level = 0

# Show common cipher list supported by client and server
# value: 0 or 1
#show_common_cipher_list = 0

# Needed for primary NTLM or Kerberos connections over NLA.
# value: 0 or 1
#_advanced
#enable_nla = 0

# Needed to connect with jrdp, based on bogus X224 layer code.
# value: 0 or 1
#_advanced
#bogus_neg_request = 0

# Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code.
# value: 0 or 1
#_advanced
#bogus_user_id = 1

# If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences.
# value: 0 or 1
#_advanced
#disable_tsk_switch_shortcuts = 0

# Specifies the highest compression package support available on the front side
# min = 0, max = 4
#   0: The RDP bulk compression is disabled
#   1: RDP 4.0 bulk compression
#   2: RDP 5.0 bulk compression
#   3: RDP 6.0 bulk compression
#   4: RDP 6.1 bulk compression
#_advanced
#rdp_compression = 4

# Specifies the maximum color resolution (color depth) for client session:
#   8: 8-bit
#   15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)
#   16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)
#   24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)
#   32: 32-bit RGB mask (8 bits for alpha, 8 bits for red, 8 bits for green, and 8 bits for blue)
#_advanced
#max_color_depth = 24

# Persistent Disk Bitmap Cache on the front side.
# value: 0 or 1
#_advanced
#persistent_disk_bitmap_cache = 1

# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
# value: 0 or 1
#_advanced
#cache_waiting_list = 0

# If enabled, the contents of Persistent Bitmap Caches are stored on disk.
# value: 0 or 1
#_advanced
#persist_bitmap_cache_on_disk = 0

# Support of Bitmap Compression.
# value: 0 or 1
#_advanced
#bitmap_compression = 1

# Enables support of Client Fast-Path Input Event PDUs.
# value: 0 or 1
#_advanced
#fast_path = 1

# value: 0 or 1
#enable_suppress_output = 1

# [Not configured]: Compatible with more RDP clients (less secure)
# HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)
#ssl_cipher_list = HIGH:!ADH:!3DES:!SHA

# value: 0 or 1
#show_target_user_in_f12_message = 0

# value: 0 or 1
#enable_new_pointer_update = 0

# value: 0 or 1
#bogus_ios_glyph_support_level = 1

# value: 0 or 1
#_advanced
#transform_glyph_to_bitmap = 0

# min = 0, max = 2
#   0: disabled
#   1: pause key only
#   2: all input events
#bogus_number_of_fastpath_input_event = 1

# min = 100, max = 10000
# (is in millisecond)
#_advanced
#recv_timeout = 1000

# value: 0 or 1
#_advanced
#enable_osd_4_eyes = 0

# Enable front remoteFx
# value: 0 or 1
#_advanced
#enable_remotefx = 1

# value: 0 or 1
#bogus_pointer_xormask_padding = 0

# Disables supported drawing orders:
#    0: DstBlt
#    1: PatBlt
#    2: ScrBlt
#    3: MemBlt
#    4: Mem3Blt
#    8: LineTo
#   15: MultiDstBlt
#   16: MultiPatBlt
#   17: MultiScrBlt
#   18: MultiOpaqueRect
#   22: Polyline
#   25: EllipseSC
#   27: GlyphIndex
# 
#_advanced
#disabled_orders = 25

# Force usage of bitmap cache V2 for compatibility with WALLIX Access Manager.
# value: 0 or 1
#_advanced
#force_bitmap_cache_v2_with_am = 1

[mod_rdp]

# Specifies the highest compression package support available on the front side
# min = 0, max = 4
#   0: The RDP bulk compression is disabled
#   1: RDP 4.0 bulk compression
#   2: RDP 5.0 bulk compression
#   3: RDP 6.0 bulk compression
#   4: RDP 6.1 bulk compression
#_advanced
#rdp_compression = 4

# value: 0 or 1
#_advanced
#disconnect_on_logon_user_change = 0

# (is in second)
#_advanced
#open_session_timeout = 0

# Disables supported drawing orders:
#    0: DstBlt
#    1: PatBlt
#    2: ScrBlt
#    3: MemBlt
#    4: Mem3Blt
#    8: LineTo
#   15: MultiDstBlt
#   16: MultiPatBlt
#   17: MultiScrBlt
#   18: MultiOpaqueRect
#   22: Polyline
#   25: EllipseSC
#   27: GlyphIndex
# 
#_hidden
#disabled_orders = 

# NLA authentication in secondary target.
# value: 0 or 1
#_hidden
#enable_nla = 1

# If enabled, NLA authentication will try Kerberos before NTLM.
# (if enable_nla is disabled, this value is ignored).
# value: 0 or 1
#_hidden
#enable_kerberos = 0

# Persistent Disk Bitmap Cache on the mod side.
# value: 0 or 1
#_advanced
#persistent_disk_bitmap_cache = 1

# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
# value: 0 or 1
#_advanced
#cache_waiting_list = 1

# If enabled, the contents of Persistent Bitmap Caches are stored on disk.
# value: 0 or 1
#_advanced
#persist_bitmap_cache_on_disk = 0

# Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority.
#_hidden
#allow_channels = *

# Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority.
#_hidden
#deny_channels = 

# Enables support of Client/Server Fast-Path Input/Update PDUs.
# Fast-Path is required for Windows Server 2012 (or more recent)!
# value: 0 or 1
#_advanced
#fast_path = 1

# Enables Server Redirection Support.
# value: 0 or 1
#_hidden
#server_redirection_support = 0

# Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block.
# value: 0 or 1
#_advanced
#bogus_sc_net_size = 1

#_advanced
#proxy_managed_drives = 

# value: 0 or 1
#_hidden
#ignore_auth_channel = 0

# Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel.
# maxlen = 7
#auth_channel = *

# Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel.
# maxlen = 7
#checkout_channel = 

#_hidden
#alternate_shell = 

#_hidden
#shell_arguments = 

#_hidden
#shell_working_directory = 

# As far as possible, use client-provided initial program (Alternate Shell)
# value: 0 or 1
#_hidden
#use_client_provided_alternate_shell = 0

# As far as possible, use client-provided remote program (RemoteApp)
# value: 0 or 1
#_hidden
#use_client_provided_remoteapp = 0

# As far as possible, use native RemoteApp capability
# value: 0 or 1
#_hidden
#use_native_remoteapp_capability = 1

# value: 0 or 1
#_hidden
#enable_session_probe = 0

# Minimum supported server : Windows Server 2008.
# Clipboard redirection should be remain enabled on Terminal Server.
# value: 0 or 1
#_hidden
#session_probe_use_smart_launcher = 1

# value: 0 or 1
#_hidden
#session_probe_enable_launch_mask = 1

# Behavior on failure to launch Session Probe.
# min = 0, max = 2
#   0: ignore failure and continue.
#   1: disconnect user.
#   2: reconnect without Session Probe.
#_hidden
#session_probe_on_launch_failure = 1

# This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).
# 0 to disable timeout.
# min = 0, max = 300000
# (is in millisecond)
#_hidden
#session_probe_launch_timeout = 40000

# This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).
# 0 to disable timeout.
# min = 0, max = 300000
# (is in millisecond)
#_hidden
#session_probe_launch_fallback_timeout = 10000

# Minimum supported server : Windows Server 2008.
# value: 0 or 1
#_hidden
#session_probe_start_launch_timeout_timer_only_after_logon = 1

# min = 0, max = 60000
# (is in millisecond)
#_hidden
#session_probe_keepalive_timeout = 5000

# min = 0, max = 2
#   0: ignore and continue
#   1: disconnect user
#   2: freeze connection and wait
#_hidden
#session_probe_on_keepalive_timeout = 1

# End automatically a disconnected session.
# Session Probe must be enabled to use this feature.
# value: 0 or 1
#_hidden
#session_probe_end_disconnected_session = 0

# value: 0 or 1
#_advanced
#session_probe_customize_executable_name = 0

# value: 0 or 1
#_hidden
#session_probe_enable_log = 0

# value: 0 or 1
#_hidden
#session_probe_enable_log_rotation = 1

# This policy setting allows you to configure a time limit for disconnected application sessions.
# 0 to disable timeout.
# min = 0, max = 172800000
# (is in millisecond)
#_hidden
#session_probe_disconnected_application_limit = 0

# This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.
# 0 to disable timeout.
# min = 0, max = 172800000
# (is in millisecond)
#_hidden
#session_probe_disconnected_session_limit = 0

# This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.
# 0 to disable timeout.
# min = 0, max = 172800000
# (is in millisecond)
#_hidden
#session_probe_idle_session_limit = 0

# maxlen = 511
#_hidden
#session_probe_exe_or_file = ||CMD

# maxlen = 511
#_hidden
#session_probe_arguments = )gen_config_ini" << (REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS) << R"gen_config_ini(

# (is in millisecond)
#_hidden
#session_probe_clipboard_based_launcher_clipboard_initialization_delay = 2000

# (is in millisecond)
#_hidden
#session_probe_clipboard_based_launcher_start_delay = 0

# (is in millisecond)
#_hidden
#session_probe_clipboard_based_launcher_long_delay = 500

# (is in millisecond)
#_hidden
#session_probe_clipboard_based_launcher_short_delay = 50

# min = 0, max = 300000
# (is in millisecond)
#_hidden
#session_probe_launcher_abort_delay = 2000

# value: 0 or 1
#_advanced
#session_probe_allow_multiple_handshake = 0

# value: 0 or 1
#_hidden
#session_probe_enable_crash_dump = 0

# min = 0, max = 1000
# min = 0
#_hidden
#session_probe_handle_usage_limit = 0

# min = 0, max = 200000000
# min = 0
#_hidden
#session_probe_memory_usage_limit = 0

# min = 0, max = 60000
# (is in millisecond)
#_hidden
#session_probe_end_of_session_check_delay_time = 0

# value: 0 or 1
#_hidden
#session_probe_ignore_ui_less_processes_during_end_of_session_check = 1

# value: 0 or 1
#_hidden
#session_probe_childless_window_as_unidentified_input_field = 1

# min = 0, max = 1023
#   0x000: none
#   0x001: disable Java Access Bridge
#   0x002: disable MS Active Accessbility
#   0x004: disable MS UI Automation
#   0x010: disable Inspect Edge location URL
#   0x020: disable Inspect Chrome Address/Search bar
#   0x040: disable Inspect Firefox Address/Search bar
#   0x080: disable Monitor Internet Explorer event
#   0x100: disable Inspect group membership of user
# Note: values can be added (disable all: 0x001 + 0x002 + 0x004 + 0x010 + 0x020 + 0x040 + 0x080 + 0x100 = 0x1f7)
#_hidden
#_hex
#session_probe_disabled_features = 352

# value: 0 or 1
#_hidden
#enable_bestsafe_integration = 0

# If enabled, disconnected session can be recovered by a different primary user.
# value: 0 or 1
#_hidden
#session_probe_public_session = 0

# For targets running WALLIX BestSafe only.
# min = 0, max = 2
#   0: User action will be accepted
#   1: (Same thing as 'allow') 
#   2: User action will be rejected
#_hidden
#session_probe_on_account_manipulation = 0

# Keep known server certificates on WAB
# value: 0 or 1
#_hidden
#server_cert_store = 1

# Behavior of certificates check.
# min = 0, max = 3
#   0: fails if certificates doesn't match or miss.
#   1: fails if certificate doesn't match, succeed if no known certificate.
#   2: succeed if certificates exists (not checked), fails if missing.
#   3: always succeed.
# System errors like FS access rights issues or certificate decode are always check errors leading to connection rejection.
#_hidden
#server_cert_check = 1

# Warn if check allow connexion to server.
# min = 0, max = 7
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
#server_access_allowed_message = 1

# Warn that new server certificate file was created.
# min = 0, max = 7
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
#server_cert_create_message = 1

# Warn that server certificate file was successfully checked.
# min = 0, max = 7
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
#server_cert_success_message = 1

# Warn that server certificate file checking failed.
# min = 0, max = 7
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
#server_cert_failure_message = 1

# Warn that server certificate check raised some internal error.
# min = 0, max = 7
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
#server_cert_error_message = 1

# Do not transmit client machine name or RDP server.
# value: 0 or 1
#hide_client_name = 0

# value: 0 or 1
#_advanced
#clean_up_32_bpp_cursor = 0

# value: 0 or 1
#bogus_ios_rdpdr_virtual_channel = 1

# value: 0 or 1
#_advanced
#enable_rdpdr_data_analysis = 1

# Delay before automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.
# (is in millisecond)
#_advanced
#remoteapp_bypass_legal_notice_delay = 0

# Time limit to automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.
# (is in millisecond)
#_advanced
#remoteapp_bypass_legal_notice_timeout = 20000

# value: 0 or 1
#_advanced
#log_only_relevant_clipboard_activities = 1

# value: 0 or 1
#_advanced
#experimental_fix_input_event_sync = 1

# value: 0 or 1
#_advanced
#experimental_fix_too_long_cookie = 1

# Force to split target domain and username with '@' separator.
# value: 0 or 1
#_advanced
#split_domain = 0

# value: 0 or 1
#_hidden
#wabam_uses_translated_remoteapp = 0

# Enables Session Shadowing Support.
# value: 0 or 1
#_advanced
#session_shadowing_support = 1

# Stores CALs issued by the terminal servers.
# value: 0 or 1
#_advanced
#use_license_store = 1

# Enables support of the remoteFX codec.
# value: 0 or 1
#_hidden
#enable_remotefx = 0

# value: 0 or 1
#_advanced
#accept_monitor_layout_change_if_capture_is_not_started = 0

# Connect to the server in Restricted Admin mode.
# This mode must be supported by the server (available from Windows Server 2012 R2), otherwise, connection will fail.
# NLA must be enabled.
# value: 0 or 1
#_hidden
#enable_restricted_admin_mode = 0

# NLA will be disabled.
# Target must be set for interactive login, otherwise server connection may not be guaranteed.
# Smartcard device must be available on client desktop.
# Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service.
# value: 0 or 1
#_hidden
#force_smartcard_authentication = 0

[mod_vnc]

# Enable or disable the clipboard from client (client to server).
# value: 0 or 1
#clipboard_up = 0

# Enable or disable the clipboard from server (server to client).
# value: 0 or 1
#clipboard_down = 0

# Sets the encoding types in which pixel data can be sent by the VNC server:
#   0: Raw
#   1: CopyRect
#   2: RRE
#   16: ZRLE
#   -239 (0xFFFFFF11): Cursor pseudo-encoding
#_advanced
#encodings = 

# VNC server clipboard data encoding type.
# values: 'utf-8', 'latin1'
#_advanced
#server_clipboard_encoding_type = latin1

# min = 0, max = 2
#   0: delayed
#   1: duplicated
#   2: continued
#_advanced
#bogus_clipboard_infinite_loop = 0

# value: 0 or 1
#_hidden
#server_is_macos = 0

# value: 0 or 1
#_hidden
#server_unix_alt = 0

# value: 0 or 1
#_hidden
#support_cursor_pseudo_encoding = 1

[metrics]

# value: 0 or 1
#_advanced
#enable_rdp_metrics = 0

# value: 0 or 1
#_advanced
#enable_vnc_metrics = 0

# maxlen = 4096
#_hidden
#log_dir_path = )gen_config_ini" << (app_path(AppPath::Metrics).to_string()) << R"gen_config_ini(

# (is in second)
#_advanced
#log_interval = 5

# (is in hour)
#_advanced
#log_file_turnover_interval = 24

# signature key to digest log metrics header info
#_advanced
#sign_key = 

[file_verification]

#_hidden
#socket_path = )gen_config_ini" << (REDEMPTION_CONFIG_VALIDATOR_PATH) << R"gen_config_ini(

# Enable use of ICAP service for file verification on upload.
# value: 0 or 1
#_hidden
#enable_up = 0

# Enable use of ICAP service for file verification on download.
# value: 0 or 1
#_hidden
#enable_down = 0

# Verify text data via clipboard from client to server.
# File verification on upload must be enabled via option Enable up.
# value: 0 or 1
#_hidden
#clipboard_text_up = 0

# Verify text data via clipboard from server to client
# File verification on download must be enabled via option Enable down.
# value: 0 or 1
#_hidden
#clipboard_text_down = 0

# value: 0 or 1
#_hidden
#log_if_accepted = 1

[file_storage]

# Enable storage of transferred files (via RDP Clipboard).
# values: 'never', 'always', 'on_invalid_verification'
#   never: Never store transferred files.
#   always: Always store transferred files.
#   on_invalid_verification: Transferred files are stored only if file verification is invalid. File verification by ICAP service must be enabled (in section file_verification).
#_hidden
#store_file = never

[icap_server_down]

# Ip or fqdn of ICAP server
#host = 

# Port of ICAP server
# min = 0
#port = 1344

# Service name on ICAP server
#service_name = avscan

# ICAP server uses tls
# value: 0 or 1
#tls = 0

# Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server
# value: 0 or 1
#_advanced
#enable_x_context = 1

# Filename sent to ICAP as percent encoding
# value: 0 or 1
#_advanced
#filename_percent_encoding = 0

[icap_server_up]

# Ip or fqdn of ICAP server
#host = 

# Port of ICAP server
# min = 0
#port = 1344

# Service name on ICAP server
#service_name = avscan

# ICAP server uses tls
# value: 0 or 1
#tls = 0

# Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server
# value: 0 or 1
#_advanced
#enable_x_context = 1

# Filename sent to ICAP as percent encoding
# value: 0 or 1
#_advanced
#filename_percent_encoding = 0

[mod_replay]

# 0 - Wait for Escape, 1 - End session
# value: 0 or 1
#_hidden
#on_end_of_data = 0

# 0 - replay once, 1 - loop replay
# value: 0 or 1
#_hidden
#replay_on_loop = 0

[ocr]

#   1: v1
#   2: v2
#version = 2

# values: 'latin', 'cyrillic'
#locale = latin

# (is in 1/100 second)
#_advanced
#interval = 100

# value: 0 or 1
#_advanced
#on_title_bar_only = 1

# Expressed in percentage,
#   0   - all of characters need be recognized
#   100 - accept all results
# min = 0, max = 100
# min = 0
#_advanced
#max_unrecog_char_rate = 40

[video]

# min = 0
#_advanced
#capture_groupid = 33

# Specifies the type of data to be captured:
# min = 0, max = 15
#   0x00: none
#   0x01: png
#   0x02: wrm
#   0x04: video
#   0x08: ocr
# Note: values can be added (enable all: 0x01 + 0x02 + 0x04 + 0x08 = 0x0f)
#_advanced
#_hex
#capture_flags = 11

# Frame interval.
# (is in 1/10 second)
#_advanced
#png_interval = 10

# Frame interval.
# (is in 1/100 second)
#_advanced
#frame_interval = 40

# Time between 2 wrm movies.
# (is in second)
#_advanced
#break_interval = 600

# Number of png captures to keep.
# min = 0
#_advanced
#png_limit = 5

# maxlen = 4096
#_advanced
#replay_path = /tmp/

# maxlen = 4096
#_hidden
#hash_path = )gen_config_ini" << (app_path(AppPath::Hash).to_string()) << R"gen_config_ini(

# maxlen = 4096
#_hidden
#record_tmp_path = )gen_config_ini" << (app_path(AppPath::RecordTmp).to_string()) << R"gen_config_ini(

# maxlen = 4096
#_hidden
#record_path = )gen_config_ini" << (app_path(AppPath::Record).to_string()) << R"gen_config_ini(

# Disable keyboard log:
# (Please see also "Keyboard input masking level" in "session_log" section of "Connection Policy".)
# min = 0, max = 7
#   0x0: none
#   0x1: disable keyboard log in syslog
#   0x2: disable keyboard log in recorded sessions
#   0x4: disable keyboard log in recorded meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
#disable_keyboard_log = 1

# Disable clipboard log:
# min = 0, max = 7
#   0x0: none
#   0x1: disable clipboard log in syslog
#   0x2: disable clipboard log in recorded sessions
#   0x4: disable clipboard log in recorded meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
#disable_clipboard_log = 1

# Disable (redirected) file system log:
# min = 0, max = 7
#   0x0: none
#   0x1: disable (redirected) file system log in syslog
#   0x2: disable (redirected) file system log in recorded sessions
#   0x4: disable (redirected) file system log in recorded meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
#disable_file_system_log = 1

# value: 0 or 1
#_hidden
#rt_display = 0

# The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:
# min = 0, max = 1
#   0: 24-bit
#   1: 16-bit
#_advanced
#wrm_color_depth_selection_strategy = 1

# The compression method of native video capture:
# min = 0, max = 2
#   0: no compression
#   1: gzip
#   2: snappy
#_advanced
#wrm_compression_algorithm = 1

# Needed to play a video with ffplay or VLC.
# Note: Useless with mpv and mplayer.
# value: 0 or 1
#_advanced
#bogus_vlc_frame_rate = 1

#_advanced
#codec_id = mp4

# min = 0
#_advanced
#framerate = 5

# FFmpeg options for video codec.
#_advanced
#ffmpeg_options = profile=baseline preset=ultrafast flags=+qscale b=80000

# value: 0 or 1
#_advanced
#notimestamp = 0

# min = 0, max = 2
#   0: Disabled. When replaying the session video, the content of the RDP viewer matches the size of the client's desktop
#   1: When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session
#   2: When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session
#smart_video_cropping = 0

# Needed to play a video with corrupted Bitmap Update.
# Note: Useless with mpv and mplayer.
# value: 0 or 1
#_advanced
#play_video_with_corrupted_bitmap = 0

[crypto]

#_hidden
#encryption_key = 000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F

#_hidden
#sign_key = 000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F

[websocket]

# Enable websocket protocol (ws or wss with use_tls=1)
# value: 0 or 1
#_hidden
#enable_websocket = 0

# Use TLS with websocket (wss)
# value: 0 or 1
#_hidden
#use_tls = 1

# ${addr}:${port} or ${port} or ${unix_socket_path}
#_hidden
#listen_address = :3390

[debug]

#_hidden
#fake_target_ip = 

# min = 0
#_advanced
#_hex
#primary_orders = 0

# min = 0
#_advanced
#_hex
#secondary_orders = 0

# min = 0
#_advanced
#_hex
#bitmap_update = 0

# min = 0
#_advanced
#_hex
#capture = 0

# min = 0
#_advanced
#_hex
#auth = 0

# min = 0
#_advanced
#_hex
#session = 0

# min = 0
#_advanced
#_hex
#front = 0

# min = 0
#_advanced
#_hex
#mod_rdp = 0

# min = 0
#_advanced
#_hex
#mod_vnc = 0

# min = 0
#_advanced
#_hex
#mod_internal = 0

# min = 0
#_advanced
#_hex
#mod_xup = 0

# min = 0
#_hidden
#password = 0

# min = 0
#_advanced
#_hex
#compression = 0

# min = 0
#_advanced
#_hex
#cache = 0

# min = 0
#_advanced
#_hex
#performance = 0

# min = 0
#_advanced
#_hex
#pass_dialog_box = 0

# min = 0
#_advanced
#_hex
#ocr = 0

# min = 0
#_advanced
#_hex
#ffmpeg = 0

# value: 0 or 1
#_advanced
#config = 1

[remote_program]

# value: 0 or 1
#allow_resize_hosted_desktop = 1

[translation]

# values: 'en', 'fr'
#_advanced
#language = en

#_advanced
#password_en = 

#_advanced
#password_fr = 

[internal_mod]

#_advanced
#load_theme = 

)gen_config_ini"
