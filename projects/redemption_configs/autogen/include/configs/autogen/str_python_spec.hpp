#include "config_variant.hpp"

"## Python spec file for RDP proxy.\n\n\n"
"[globals]\n\n"

"# Support of Bitmap Cache.\n"
"#_advanced\n"
"bitmap_cache = boolean(default=True)\n\n"

"#_advanced\n"
"glyph_cache = boolean(default=False)\n\n"

"#_advanced\n"
"port = integer(min=0, default=3389)\n\n"

"#_advanced\n"
"nomouse = boolean(default=False)\n\n"

"#_advanced\n"
"notimestamp = boolean(default=False)\n\n"

"#_advanced\n"
"encryptionLevel = option('low', 'medium', 'high', default='low')\n\n"

"#_advanced\n"
"authfile = string(default='" << (REDEMPTION_CONFIG_AUTHFILE) << "')\n\n"

"# Time out during RDP handshake stage.\n"
"# (is in second)\n"
"handshake_timeout = integer(min=0, default=10)\n\n"

"# No traffic auto disconnection.\n"
"# (is in second)\n"
"session_timeout = integer(min=0, default=900)\n\n"

"# No traffic auto disconnection.\n"
"# If value is 0, global value (session_timeout) is used.\n"
"# (is in second)\n"
"#_hidden\n"
"inactivity_timeout = integer(min=0, default=0)\n\n"

"# Keepalive.\n"
"# (is in second)\n"
"#_advanced\n"
"keepalive_grace_delay = integer(min=0, default=30)\n\n"

"# Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).\n"
"# (is in second)\n"
"#_advanced\n"
"authentication_timeout = integer(min=0, default=120)\n\n"

"# Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).\n"
"# (is in second)\n"
"#_advanced\n"
"close_timeout = integer(min=0, default=600)\n\n"

"# Session record options.\n"
"#   0: No encryption (faster).\n"
"#   1: No encryption, with checksum.\n"
"#   2: Encryption enabled.\n"
"# When session records are encrypted, they can be read only by the WAB where they have been generated.\n"
"#_advanced\n"
"trace_type = option(0, 1, 2, default=1)\n\n"

"#_advanced\n"
"listen_address = ip_addr(default='0.0.0.0')\n\n"

"# Allow Transparent mode.\n"
"#_iptables\n"
"enable_transparent_mode = boolean(default=False)\n\n"

"# Proxy certificate password.\n"
"#_advanced\n"
"#_password\n"
"certificate_password = string(max=254, default='inquisition')\n\n"

"#_advanced\n"
"png_path = string(max=4096, default='" << (app_path(AppPath::Png)) << "')\n\n"

"#_advanced\n"
"wrm_path = string(max=4096, default='" << (app_path(AppPath::Wrm)) << "')\n\n"

"#_advanced\n"
"movie_path = string(default='')\n\n"

"# Support of Bitmap Update.\n"
"#_advanced\n"
"enable_bitmap_update = boolean(default=True)\n\n"

"# Show close screen.\n"
"enable_close_box = boolean(default=True)\n\n"

"#_advanced\n"
"enable_osd = boolean(default=True)\n\n"

"#_advanced\n"
"enable_osd_display_remote_target = boolean(default=True)\n\n"

"#_advanced\n"
"persistent_path = string(max=4096, default='" << (app_path(AppPath::Persistent)) << "')\n\n"

"#_hidden\n"
"enable_wab_integration = boolean(default=" << (REDEMPTION_CONFIG_ENABLE_WAB_INTEGRATION) << ")\n\n"

"allow_using_multiple_monitors = boolean(default=True)\n\n"

"# Needed to refresh screen of Windows Server 2012.\n"
"#_advanced\n"
"bogus_refresh_rect = boolean(default=True)\n\n"

"#_advanced\n"
"codec_id = string(default='flv')\n\n"

"#_advanced\n"
"video_quality = option('low', 'medium', 'high', default='high')\n\n"

"#_advanced\n"
"large_pointer_support = boolean(default=True)\n\n"

"unicode_keyboard_event_support = boolean(default=True)\n\n"

"# (is in millisecond)\n"
"#_advanced\n"
"mod_recv_timeout = integer(min=100, max=10000, default=1000)\n\n"

"#_advanced\n"
"spark_view_specific_glyph_width = boolean(default=False)\n\n"

"#_advanced\n"
"experimental_enable_serializer_data_block_size_limit = boolean(default=False)\n\n"

"#_advanced\n"
"experimental_support_resize_session_during_recording = boolean(default=False)\n\n"

"# Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.\n"
"# !!!May cause FreeRDP-based client to CRASH!!!\n"
"# Set to 0 to disable this feature.\n"
"# (is in millisecond)\n"
"rdp_keepalive_connection_interval = integer(min=0, default=0)\n\n"

"[session_log]\n\n"

"enable_session_log = boolean(default=True)\n\n"

"enable_arcsight_log = boolean(default=False)\n\n"

"#_hidden\n"
"log_path = string(default='')\n\n"

"#   0: keyboard input are not masked\n"
"#   1: only passwords are masked\n"
"#   2: passwords and unidentified texts are masked\n"
"#   3: keyboard input are fully masked\n"
"#_hidden\n"
"keyboard_input_masking_level = option(0, 1, 2, 3, default=2)\n\n"

"#_advanced\n"
"hide_non_printable_kbd_input = boolean(default=False)\n\n"

"[client]\n\n"

"# cs-CZ, da-DK, de-DE, el-GR, en-US, es-ES, fi-FI.finnish, fr-FR, is-IS, it-IT, nl-NL, nb-NO, pl-PL.programmers, pt-BR.abnt, ro-RO, ru-RU, hr-HR, sk-SK, sv-SE, tr-TR.q, uk-UA, sl-SI, et-EE, lv-LV, lt-LT.ibm, mk-MK, fo-FO, mt-MT.47, se-NO, kk-KZ, ky-KG, tt-RU, mn-MN, cy-GB, lb-LU, mi-NZ, de-CH, en-GB, es-MX, fr-BE.fr, nl-BE, pt-PT, sr-La, se-SE, uz-Cy, iu-La, fr-CA, sr-Cy, en-CA.fr, fr-CH, bs-Cy, bg-BG.latin, cs-CZ.qwerty, en-IE.irish, de-DE.ibm, el-GR.220, es-ES.variation, hu-HU, en-US.dvorak, it-IT.142, pl-PL, pt-BR.abnt2, ru-RU.typewriter, sk-SK.qwerty, tr-TR.f, lv-LV.qwerty, lt-LT, mt-MT.48, se-NO.ext_norway, fr-BE, se-SE, en-CA.multilingual, en-IE, cs-CZ.programmers, el-GR.319, en-US.international, se-SE.ext_finland_sweden, bg-BG, el-GR.220_latin, en-US.dvorak_left, el-GR.319_latin, en-US.dvorak_right, el-GR.latin, el-GR.polytonic\n"
"#_advanced\n"
"keyboard_layout_proposals = string_list(default=list('en-US, fr-FR, de-DE, ru-RU'))\n\n"

"# If true, ignore password provided by RDP client, user need do login manually.\n"
"#_advanced\n"
"ignore_logon_password = boolean(default=False)\n\n"

"# Enable font smoothing (0x80).\n"
"#_advanced\n"
"#_hex\n"
"performance_flags_default = integer(min=0, default=128)\n\n"

"# Disable theme (0x8).\n"
"# Disable mouse cursor shadows (0x20).\n"
"#_advanced\n"
"#_hex\n"
"performance_flags_force_present = integer(min=0, default=40)\n\n"

"#_advanced\n"
"#_hex\n"
"performance_flags_force_not_present = integer(min=0, default=0)\n\n"

"# If enabled, avoid automatically font smoothing in recorded session.\n"
"#_advanced\n"
"auto_adjust_performance_flags = boolean(default=True)\n\n"

"# Fallback to RDP Legacy Encryption if client does not support TLS.\n"
"tls_fallback_legacy = boolean(default=False)\n\n"

"tls_support = boolean(default=True)\n\n"

"# Needed to connect with jrdp, based on bogus X224 layer code.\n"
"#_advanced\n"
"bogus_neg_request = boolean(default=False)\n\n"

"# Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code.\n"
"#_advanced\n"
"bogus_user_id = boolean(default=True)\n\n"

"# If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences.\n"
"#_advanced\n"
"disable_tsk_switch_shortcuts = boolean(default=False)\n\n"

"# Specifies the highest compression package support available on the front side\n"
"#   0: The RDP bulk compression is disabled\n"
"#   1: RDP 4.0 bulk compression\n"
"#   2: RDP 5.0 bulk compression\n"
"#   3: RDP 6.0 bulk compression\n"
"#   4: RDP 6.1 bulk compression\n"
"#_advanced\n"
"rdp_compression = option(0, 1, 2, 3, 4, default=4)\n\n"

"# Specifies the maximum color resolution (color depth) for client session:\n"
"#   8: 8-bit\n"
"#   15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)\n"
"#   16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)\n"
"#   24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)\n"
"#_advanced\n"
"max_color_depth = option(8, 15, 16, 24, default=24)\n\n"

"# Persistent Disk Bitmap Cache on the front side.\n"
"#_advanced\n"
"persistent_disk_bitmap_cache = boolean(default=True)\n\n"

"# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).\n"
"#_advanced\n"
"cache_waiting_list = boolean(default=False)\n\n"

"# If enabled, the contents of Persistent Bitmap Caches are stored on disk.\n"
"#_advanced\n"
"persist_bitmap_cache_on_disk = boolean(default=False)\n\n"

"# Support of Bitmap Compression.\n"
"#_advanced\n"
"bitmap_compression = boolean(default=True)\n\n"

"# Enables support of Clent Fast-Path Input Event PDUs.\n"
"#_advanced\n"
"fast_path = boolean(default=True)\n\n"

"enable_suppress_output = boolean(default=True)\n\n"

"# [Not configured]: Compatible with more RDP clients (less secure)\n"
"# HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)\n"
"ssl_cipher_list = string(default='HIGH:!ADH:!3DES:!SHA')\n\n"

"show_target_user_in_f12_message = boolean(default=False)\n\n"

"enable_new_pointer_update = boolean(default=False)\n\n"

"bogus_ios_glyph_support_level = boolean(default=True)\n\n"

"#_advanced\n"
"transform_glyph_to_bitmap = boolean(default=False)\n\n"

"#   0: disabled\n"
"#   1: pause key only\n"
"#   2: all input events\n"
"bogus_number_of_fastpath_input_event = option(0, 1, 2, default=1)\n\n"

"# (is in millisecond)\n"
"#_advanced\n"
"recv_timeout = integer(min=100, max=10000, default=1000)\n\n"

"#_advanced\n"
"enable_osd_4_eyes = boolean(default=False)\n\n"

"[mod_rdp]\n\n"

"# Specifies the highest compression package support available on the front side\n"
"#   0: The RDP bulk compression is disabled\n"
"#   1: RDP 4.0 bulk compression\n"
"#   2: RDP 5.0 bulk compression\n"
"#   3: RDP 6.0 bulk compression\n"
"#   4: RDP 6.1 bulk compression\n"
"#_advanced\n"
"rdp_compression = option(0, 1, 2, 3, 4, default=4)\n\n"

"#_advanced\n"
"disconnect_on_logon_user_change = boolean(default=False)\n\n"

"# (is in second)\n"
"#_advanced\n"
"open_session_timeout = integer(min=0, default=0)\n\n"

"# Enables support of additional drawing orders:\n"
"#   15: MultiDstBlt\n"
"#   16: MultiPatBlt\n"
"#   17: MultiScrBlt\n"
"#   18: MultiOpaqueRect\n"
"#   22: Polyline\n"
"#_advanced\n"
"extra_orders = string_list(default=list('15,16,17,18,22'))\n\n"

"# NLA authentication in secondary target.\n"
"#_hidden\n"
"enable_nla = boolean(default=True)\n\n"

"# If enabled, NLA authentication will try Kerberos before NTLM.\n"
"# (if enable_nla is disabled, this value is ignored).\n"
"#_hidden\n"
"enable_kerberos = boolean(default=False)\n\n"

"# Persistent Disk Bitmap Cache on the mod side.\n"
"#_advanced\n"
"persistent_disk_bitmap_cache = boolean(default=True)\n\n"

"# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).\n"
"#_advanced\n"
"cache_waiting_list = boolean(default=True)\n\n"

"# If enabled, the contents of Persistent Bitmap Caches are stored on disk.\n"
"#_advanced\n"
"persist_bitmap_cache_on_disk = boolean(default=False)\n\n"

"# Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority.\n"
"#_advanced\n"
"allow_channels = string_list(default=list('*'))\n\n"

"# Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority.\n"
"#_advanced\n"
"deny_channels = string_list(default=list())\n\n"

"# Enables support of Client/Server Fast-Path Input/Update PDUs.\n"
"# Fast-Path is required for Windows Server 2012 (or more recent)!\n"
"#_advanced\n"
"fast_path = boolean(default=True)\n\n"

"# Enables Server Redirection Support.\n"
"#_hidden\n"
"server_redirection_support = boolean(default=False)\n\n"

"# Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block.\n"
"#_advanced\n"
"bogus_sc_net_size = boolean(default=True)\n\n"

"#_advanced\n"
"proxy_managed_drives = string_list(default=list())\n\n"

"#_hidden\n"
"ignore_auth_channel = boolean(default=False)\n\n"

"# Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel.\n"
"auth_channel = string(max=7, default='*')\n\n"

"# Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel.\n"
"checkout_channel = string(max=7, default='')\n\n"

"#_hidden\n"
"alternate_shell = string(default='')\n\n"

"#_hidden\n"
"shell_arguments = string(default='')\n\n"

"#_hidden\n"
"shell_working_directory = string(default='')\n\n"

"# As far as possible, use client-provided initial program (Alternate Shell)\n"
"#_hidden\n"
"use_client_provided_alternate_shell = boolean(default=False)\n\n"

"# As far as possible, use client-provided remote program (RemoteApp)\n"
"#_hidden\n"
"use_client_provided_remoteapp = boolean(default=False)\n\n"

"# As far as possible, use native RemoteApp capability\n"
"#_hidden\n"
"use_native_remoteapp_capability = boolean(default=True)\n\n"

"#_hidden\n"
"enable_session_probe = boolean(default=False)\n\n"

"# Minimum supported server : Windows Server 2008.\n"
"# Clipboard redirection should be remain enabled on Terminal Server.\n"
"#_hidden\n"
"session_probe_use_smart_launcher = boolean(default=True)\n\n"

"#_hidden\n"
"session_probe_enable_launch_mask = boolean(default=True)\n\n"

"# Behavior on failure to launch Session Probe.\n"
"#   0: ignore failure and continue.\n"
"#   1: disconnect user.\n"
"#   2: reconnect without Session Probe.\n"
"#_hidden\n"
"session_probe_on_launch_failure = option(0, 1, 2, default=2)\n\n"

"# This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).\n"
"# 0 to disable timeout.\n"
"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_launch_timeout = integer(min=0, max=300000, default=40000)\n\n"

"# This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).\n"
"# 0 to disable timeout.\n"
"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_launch_fallback_timeout = integer(min=0, max=300000, default=10000)\n\n"

"# Minimum supported server : Windows Server 2008.\n"
"#_hidden\n"
"session_probe_start_launch_timeout_timer_only_after_logon = boolean(default=True)\n\n"

"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_keepalive_timeout = integer(min=0, max=60000, default=5000)\n\n"

"#   0: ignore and continue\n"
"#   1: disconnect user\n"
"#   2: freeze connection and wait\n"
"#_hidden\n"
"session_probe_on_keepalive_timeout = option(0, 1, 2, default=1)\n\n"

"# End automatically a disconnected session.\n"
"# Session Probe must be enabled to use this feature.\n"
"#_hidden\n"
"session_probe_end_disconnected_session = boolean(default=False)\n\n"

"#_advanced\n"
"session_probe_customize_executable_name = boolean(default=False)\n\n"

"#_hidden\n"
"session_probe_enable_log = boolean(default=False)\n\n"

"#_hidden\n"
"session_probe_enable_log_rotation = boolean(default=True)\n\n"

"# This policy setting allows you to configure a time limit for disconnected application sessions.\n"
"# 0 to disable timeout.\n"
"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_disconnected_application_limit = integer(min=0, max=172800000, default=0)\n\n"

"# This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.\n"
"# 0 to disable timeout.\n"
"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_disconnected_session_limit = integer(min=0, max=172800000, default=0)\n\n"

"# This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.\n"
"# 0 to disable timeout.\n"
"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_idle_session_limit = integer(min=0, max=172800000, default=0)\n\n"

"#_hidden\n"
"session_probe_exe_or_file = string(max=511, default='||CMD')\n\n"

"#_hidden\n"
"session_probe_arguments = string(max=511, default='" << (REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS) << "')\n\n"

"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_clipboard_based_launcher_clipboard_initialization_delay = integer(min=0, default=2000)\n\n"

"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_clipboard_based_launcher_start_delay = integer(min=0, default=0)\n\n"

"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_clipboard_based_launcher_long_delay = integer(min=0, default=500)\n\n"

"# (is in millisecond)\n"
"#_hidden\n"
"session_probe_clipboard_based_launcher_short_delay = integer(min=0, default=50)\n\n"

"#_advanced\n"
"session_probe_allow_multiple_handshake = boolean(default=False)\n\n"

"#_hidden\n"
"session_probe_enable_crash_dump = boolean(default=False)\n\n"

"#_hidden\n"
"session_probe_handle_usage_limit = integer(min=0, max=1000, default=0)\n\n"

"#_hidden\n"
"session_probe_memory_usage_limit = integer(min=0, max=200000000, default=0)\n\n"

"#_hidden\n"
"session_probe_ignore_ui_less_processes_during_end_of_session_check = boolean(default=True)\n\n"

"#_hidden\n"
"session_probe_childless_window_as_unidentified_input_field = boolean(default=True)\n\n"

"# If enabled, disconnected session can be recovered by a different primary user.\n"
"#_hidden\n"
"session_probe_public_session = boolean(default=False)\n\n"

"# Keep known server certificates on WAB\n"
"#_hidden\n"
"server_cert_store = boolean(default=True)\n\n"

"# Behavior of certificates check.\n"
"#   0: fails if certificates doesn't match or miss.\n"
"#   1: fails if certificate doesn't match, succeed if no known certificate.\n"
"#   2: succeed if certificates exists (not checked), fails if missing.\n"
"#   3: always succeed.\n"
"# System errors like FS access rights issues or certificate decode are always check errors leading to connection rejection.\n"
"#_hidden\n"
"server_cert_check = option(0, 1, 2, 3, default=1)\n\n"

"# Warn if check allow connexion to server.\n"
"#   0: nobody\n"
"#   1: message sent to syslog\n"
"#   2: User notified (through proxy interface)\n"
"#   4: admin notified (wab notification)\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"#_hidden\n"
"server_access_allowed_message = integer(min=0, max=7, default=1)\n\n"

"# Warn that new server certificate file was created.\n"
"#   0: nobody\n"
"#   1: message sent to syslog\n"
"#   2: User notified (through proxy interface)\n"
"#   4: admin notified (wab notification)\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"#_hidden\n"
"server_cert_create_message = integer(min=0, max=7, default=1)\n\n"

"# Warn that server certificate file was successfully checked.\n"
"#   0: nobody\n"
"#   1: message sent to syslog\n"
"#   2: User notified (through proxy interface)\n"
"#   4: admin notified (wab notification)\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"#_hidden\n"
"server_cert_success_message = integer(min=0, max=7, default=1)\n\n"

"# Warn that server certificate file checking failed.\n"
"#   0: nobody\n"
"#   1: message sent to syslog\n"
"#   2: User notified (through proxy interface)\n"
"#   4: admin notified (wab notification)\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"#_hidden\n"
"server_cert_failure_message = integer(min=0, max=7, default=1)\n\n"

"# Warn that server certificate check raised some internal error.\n"
"#   0: nobody\n"
"#   1: message sent to syslog\n"
"#   2: User notified (through proxy interface)\n"
"#   4: admin notified (wab notification)\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"#_hidden\n"
"server_cert_error_message = integer(min=0, max=7, default=1)\n\n"

"# Do not transmit client machine name or RDP server.\n"
"hide_client_name = boolean(default=False)\n\n"

"#_advanced\n"
"clean_up_32_bpp_cursor = boolean(default=False)\n\n"

"bogus_ios_rdpdr_virtual_channel = boolean(default=True)\n\n"

"#_advanced\n"
"enable_rdpdr_data_analysis = boolean(default=True)\n\n"

"# Delay before automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
"# Set to 0 to disable this feature.\n"
"# (is in millisecond)\n"
"#_advanced\n"
"remoteapp_bypass_legal_notice_delay = integer(min=0, default=0)\n\n"

"# Time limit to automatically bypass Windows's Legal Notice screen in RemoteApp mode.\n"
"# Set to 0 to disable this feature.\n"
"# (is in millisecond)\n"
"#_advanced\n"
"remoteapp_bypass_legal_notice_timeout = integer(min=0, default=20000)\n\n"

"#_advanced\n"
"log_only_relevant_clipboard_activities = boolean(default=True)\n\n"

"#_advanced\n"
"experimental_fix_input_event_sync = boolean(default=True)\n\n"

"#_advanced\n"
"experimental_fix_too_long_cookie = boolean(default=True)\n\n"

"[metrics]\n\n"

"#_advanced\n"
"enable_rdp_metrics = boolean(default=False)\n\n"

"#_advanced\n"
"enable_vnc_metrics = boolean(default=False)\n\n"

"#_hidden\n"
"log_dir_path = string(max=4096, default='" << (app_path(AppPath::Metrics)) << "')\n\n"

"# (is in second)\n"
"#_advanced\n"
"log_interval = integer(min=0, default=5)\n\n"

"# (is in hour)\n"
"#_advanced\n"
"log_file_turnover_interval = integer(min=0, default=24)\n\n"

"# signature key to digest log metrics header info\n"
"#_advanced\n"
"sign_key = string(default='')\n\n"

"[mod_vnc]\n\n"

"# Enable or disable the clipboard from client (client to server).\n"
"clipboard_up = boolean(default=False)\n\n"

"# Enable or disable the clipboard from server (server to client).\n"
"clipboard_down = boolean(default=False)\n\n"

"# Sets the encoding types in which pixel data can be sent by the VNC server:\n"
"#   0: Raw\n"
"#   1: CopyRect\n"
"#   2: RRE\n"
"#   16: ZRLE\n"
"#   -239 (0xFFFFFF11): Cursor pseudo-encoding\n"
"#_advanced\n"
"encodings = string_list(default=list())\n\n"

"# VNC server clipboard data encoding type.\n"
"#_advanced\n"
"server_clipboard_encoding_type = option('utf-8', 'latin1', default='latin1')\n\n"

"#   0: delayed\n"
"#   1: duplicated\n"
"#   2: continued\n"
"#_advanced\n"
"bogus_clipboard_infinite_loop = option(0, 1, 2, default=0)\n\n"

"#_hidden\n"
"server_is_apple = boolean(default=False)\n\n"

"#_hidden\n"
"server_unix_alt = boolean(default=False)\n\n"

"[mod_replay]\n\n"

"# 0 - Wait for Escape, 1 - End session\n"
"#_hidden\n"
"on_end_of_data = boolean(default=False)\n\n"

"# 0 - replay once, 1 - loop replay\n"
"#_hidden\n"
"replay_on_loop = boolean(default=False)\n\n"

"[ocr]\n\n"

"#   1: v1\n"
"#   2: v2\n"
"version = option(1, 2, default=2)\n\n"

"locale = option('latin', 'cyrillic', default='latin')\n\n"

"# (is in 1/100 second)\n"
"#_advanced\n"
"interval = integer(min=0, default=100)\n\n"

"#_advanced\n"
"on_title_bar_only = boolean(default=True)\n\n"

"# Expressed in percentage,\n"
"#   0   - all of characters need be recognized\n"
"#   100 - accept all results\n"
"#_advanced\n"
"max_unrecog_char_rate = integer(min=0, max=100, default=40)\n\n"

"[video]\n\n"

"#_advanced\n"
"capture_groupid = integer(min=0, default=33)\n\n"

"# Specifies the type of data to be captured:\n"
"#   0: none\n"
"#   1: png\n"
"#   2: wrm\n"
"#   4: video\n"
"#   8: ocr\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"#_advanced\n"
"capture_flags = integer(min=0, max=15, default=11)\n\n"

"# Frame interval.\n"
"# (is in 1/10 second)\n"
"#_advanced\n"
"png_interval = integer(min=0, default=10)\n\n"

"# Frame interval.\n"
"# (is in 1/100 second)\n"
"#_advanced\n"
"frame_interval = integer(min=0, default=40)\n\n"

"# Time between 2 wrm movies.\n"
"# (is in second)\n"
"#_advanced\n"
"break_interval = integer(min=0, default=600)\n\n"

"# Number of png captures to keep.\n"
"#_advanced\n"
"png_limit = integer(min=0, default=5)\n\n"

"#_advanced\n"
"replay_path = string(max=4096, default='/tmp/')\n\n"

"#_advanced\n"
"hash_path = string(max=4096, default='" << (app_path(AppPath::Hash)) << "')\n\n"

"#_advanced\n"
"record_tmp_path = string(max=4096, default='" << (app_path(AppPath::RecordTmp)) << "')\n\n"

"#_advanced\n"
"record_path = string(max=4096, default='" << (app_path(AppPath::Record)) << "')\n\n"

"# Disable keyboard log:\n"
"#   0: none\n"
"#   1: disable keyboard log in syslog\n"
"#   2: disable keyboard log in recorded sessions\n"
"#   4: disable keyboard log in recorded meta\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"disable_keyboard_log = integer(min=0, max=7, default=1)\n\n"

"# Disable clipboard log:\n"
"#   0: none\n"
"#   1: disable clipboard log in syslog\n"
"#   2: disable clipboard log in recorded sessions\n"
"#   4: disable clipboard log in recorded meta\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"disable_clipboard_log = integer(min=0, max=7, default=1)\n\n"

"# Disable (redirected) file system log:\n"
"#   0: none\n"
"#   1: disable (redirected) file system log in syslog\n"
"#   2: disable (redirected) file system log in recorded sessions\n"
"#   4: disable (redirected) file system log in recorded meta\n"
"# (note: values can be added (everyone: 1+2+4=7, mute: 0))\n"
"disable_file_system_log = integer(min=0, max=7, default=1)\n\n"

"#_hidden\n"
"rt_display = boolean(default=False)\n\n"

"# The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:\n"
"#   0: 24-bit\n"
"#   1: 16-bit\n"
"#_advanced\n"
"wrm_color_depth_selection_strategy = option(0, 1, default=1)\n\n"

"# The compression method of native video capture:\n"
"#   0: no compression\n"
"#   1: gzip\n"
"#   2: snappy\n"
"#_advanced\n"
"wrm_compression_algorithm = option(0, 1, 2, default=1)\n\n"

"# Needed to play a video with ffplay or VLC.\n"
"# Note: Useless with mpv and mplayer.\n"
"#_advanced\n"
"bogus_vlc_frame_rate = boolean(default=True)\n\n"

"# Bitrate for low quality.\n"
"#_advanced\n"
"l_bitrate = integer(min=0, default=10000)\n\n"

"# Framerate for low quality.\n"
"#_advanced\n"
"l_framerate = integer(min=0, default=5)\n\n"

"# Height for low quality.\n"
"#_advanced\n"
"l_height = integer(min=0, default=480)\n\n"

"# Width for low quality.\n"
"#_advanced\n"
"l_width = integer(min=0, default=640)\n\n"

"# Qscale (parameter given to ffmpeg) for low quality.\n"
"#_advanced\n"
"l_qscale = integer(min=0, default=28)\n\n"

"# Bitrate for medium quality.\n"
"#_advanced\n"
"m_bitrate = integer(min=0, default=20000)\n\n"

"# Framerate for medium quality.\n"
"#_advanced\n"
"m_framerate = integer(min=0, default=5)\n\n"

"# Height for medium quality.\n"
"#_advanced\n"
"m_height = integer(min=0, default=768)\n\n"

"# Width for medium quality.\n"
"#_advanced\n"
"m_width = integer(min=0, default=1024)\n\n"

"# Qscale (parameter given to ffmpeg) for medium quality.\n"
"#_advanced\n"
"m_qscale = integer(min=0, default=14)\n\n"

"# Bitrate for high quality.\n"
"#_advanced\n"
"h_bitrate = integer(min=0, default=30000)\n\n"

"# Framerate for high quality.\n"
"#_advanced\n"
"h_framerate = integer(min=0, default=5)\n\n"

"# Height for high quality.\n"
"#_advanced\n"
"h_height = integer(min=0, default=2048)\n\n"

"# Width for high quality.\n"
"#_advanced\n"
"h_width = integer(min=0, default=2048)\n\n"

"# Qscale (parameter given to ffmpeg) for high quality.\n"
"#_advanced\n"
"h_qscale = integer(min=0, default=7)\n\n"

"#   0: Disabled. When replaying the session video, the content of the RDP viewer matches the size of the client's desktop\n"
"#   1: When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session\n"
"#   2: When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session\n"
"smart_video_cropping = option(0, 1, 2, default=0)\n\n"

"# Needed to play a video with corrupted Bitmap Update.\n"
"# Note: Useless with mpv and mplayer.\n"
"#_advanced\n"
"play_video_with_corrupted_bitmap = boolean(default=False)\n\n"

"[crypto]\n\n"

"#_hidden\n"
"encryption_key = string(min=64, max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')\n\n"

"#_hidden\n"
"sign_key = string(min=64, max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')\n\n"

"[debug]\n\n"

"#_hidden\n"
"fake_target_ip = string(default='')\n\n"

"#_advanced\n"
"#_hex\n"
"x224 = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"mcs = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"sec = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"rdp = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"primary_orders = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"secondary_orders = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"bitmap_update = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"bitmap = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"capture = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"auth = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"session = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"front = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"mod_rdp = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"mod_vnc = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"mod_internal = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"mod_xup = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"widget = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"input = integer(min=0, default=0)\n\n"

"#_hidden\n"
"password = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"compression = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"cache = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"performance = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"pass_dialog_box = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"ocr = integer(min=0, default=0)\n\n"

"#_advanced\n"
"#_hex\n"
"ffmpeg = integer(min=0, default=0)\n\n"

"#_advanced\n"
"config = boolean(default=True)\n\n"

"[remote_program]\n\n"

"allow_resize_hosted_desktop = boolean(default=True)\n\n"

"[translation]\n\n"

"#_advanced\n"
"language = option('en', 'fr', default='en')\n\n"

"#_advanced\n"
"password_en = string(default='')\n\n"

"#_advanced\n"
"password_fr = string(default='')\n\n"

"[internal_mod]\n\n"

"#_advanced\n"
"load_theme = string(default='')\n\n"

