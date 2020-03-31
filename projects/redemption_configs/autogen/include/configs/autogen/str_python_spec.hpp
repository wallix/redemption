#include "config_variant.hpp"

R"gen_config_ini(## Python spec file for RDP proxy.


[globals]

#_advanced
glyph_cache = boolean(default=False)

# Warning: Service will be automatically restarted and active sessions will be disconnected
# The port set in this field must not be already used, otherwise the service will not run.
#_iptables
#_advanced
port = integer(min=0, default=3389)

#_advanced
nomouse = boolean(default=False)

#_advanced
encryptionLevel = option('low', 'medium', 'high', default='low')

#_advanced
authfile = string(default=')gen_config_ini" << (REDEMPTION_CONFIG_AUTHFILE) << R"gen_config_ini(')

# Time out during RDP handshake stage.
# (is in second)
handshake_timeout = integer(min=0, default=10)

# No traffic auto disconnection.
# (is in second)
session_timeout = integer(min=0, default=900)

# No traffic auto disconnection.
# If value is 0, global value (session_timeout) is used.
# (is in second)
#_hidden
inactivity_timeout = integer(min=0, default=0)

# Keepalive.
# (is in second)
#_advanced
keepalive_grace_delay = integer(min=0, default=30)

# Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).
# (is in second)
#_advanced
authentication_timeout = integer(min=0, default=120)

# Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).
# (is in second)
#_advanced
close_timeout = integer(min=0, default=600)

# Session record options.
#   0: No encryption (faster).
#   1: No encryption, with checksum.
#   2: Encryption enabled.
# When session records are encrypted, they can be read only by the WAB where they have been generated.
#_advanced
trace_type = option(0, 1, 2, default=1)

#_advanced
listen_address = ip_addr(default='0.0.0.0')

# Allow Transparent mode.
#_iptables
enable_transparent_mode = boolean(default=False)

# Proxy certificate password.
#_advanced
#_password
certificate_password = string(max=254, default='inquisition')

#_advanced
movie_path = string(default='')

# Support of Bitmap Update.
#_advanced
enable_bitmap_update = boolean(default=True)

# Show close screen.
enable_close_box = boolean(default=True)

#_advanced
enable_osd = boolean(default=True)

#_advanced
enable_osd_display_remote_target = boolean(default=True)

#_hidden
enable_wab_integration = boolean(default=)gen_config_ini" << (REDEMPTION_CONFIG_ENABLE_WAB_INTEGRATION) << R"gen_config_ini()

allow_using_multiple_monitors = boolean(default=True)

# Needed to refresh screen of Windows Server 2012.
#_advanced
bogus_refresh_rect = boolean(default=True)

#_advanced
large_pointer_support = boolean(default=True)

unicode_keyboard_event_support = boolean(default=True)

# (is in millisecond)
#_advanced
mod_recv_timeout = integer(min=100, max=10000, default=1000)

#_advanced
spark_view_specific_glyph_width = boolean(default=False)

#_advanced
experimental_enable_serializer_data_block_size_limit = boolean(default=False)

#_advanced
experimental_support_resize_session_during_recording = boolean(default=True)

#_advanced
support_connection_redirection_during_recording = boolean(default=True)

# Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.
# !!!May cause FreeRDP-based client to CRASH!!!
# Set to 0 to disable this feature.
# (is in millisecond)
rdp_keepalive_connection_interval = integer(min=0, default=0)

[session_log]

enable_session_log = boolean(default=True)

enable_arcsight_log = boolean(default=False)

# Keyboard Input Masking Level:
#   0: keyboard input are not masked
#   1: only passwords are masked
#   2: passwords and unidentified texts are masked
#   3: keyboard inputs are not logged
#_hidden
keyboard_input_masking_level = option(0, 1, 2, 3, default=2)

#_advanced
hide_non_printable_kbd_input = boolean(default=False)

[client]

# cs-CZ, da-DK, de-DE, el-GR, en-US, es-ES, fi-FI.finnish, fr-FR, is-IS, it-IT, nl-NL, nb-NO, pl-PL.programmers, pt-BR.abnt, ro-RO, ru-RU, hr-HR, sk-SK, sv-SE, tr-TR.q, uk-UA, sl-SI, et-EE, lv-LV, lt-LT.ibm, mk-MK, fo-FO, mt-MT.47, se-NO, kk-KZ, ky-KG, tt-RU, mn-MN, cy-GB, lb-LU, mi-NZ, de-CH, en-GB, es-MX, fr-BE.fr, nl-BE, pt-PT, sr-La, se-SE, uz-Cy, iu-La, fr-CA, sr-Cy, en-CA.fr, fr-CH, bs-Cy, bg-BG.latin, cs-CZ.qwerty, en-IE.irish, de-DE.ibm, el-GR.220, es-ES.variation, hu-HU, en-US.dvorak, it-IT.142, pl-PL, pt-BR.abnt2, ru-RU.typewriter, sk-SK.qwerty, tr-TR.f, lv-LV.qwerty, lt-LT, mt-MT.48, se-NO.ext_norway, fr-BE, se-SE, en-CA.multilingual, en-IE, cs-CZ.programmers, el-GR.319, en-US.international, se-SE.ext_finland_sweden, bg-BG, el-GR.220_latin, en-US.dvorak_left, el-GR.319_latin, en-US.dvorak_right, el-GR.latin, el-GR.polytonic
# (values are comma-separated)
#_advanced
keyboard_layout_proposals = string(default='en-US, fr-FR, de-DE, ru-RU')

# If true, ignore password provided by RDP client, user need do login manually.
#_advanced
ignore_logon_password = boolean(default=False)

# Enable font smoothing (0x80).
#_advanced
#_hex
performance_flags_default = integer(min=0, default=128)

# Disable theme (0x8).
# Disable mouse cursor shadows (0x20).
#_advanced
#_hex
performance_flags_force_present = integer(min=0, default=40)

#_advanced
#_hex
performance_flags_force_not_present = integer(min=0, default=0)

# If enabled, avoid automatically font smoothing in recorded session.
#_advanced
auto_adjust_performance_flags = boolean(default=True)

# Fallback to RDP Legacy Encryption if client does not support TLS.
tls_fallback_legacy = boolean(default=False)

tls_support = boolean(default=True)

# Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_min_level = integer(min=0, default=2)

# Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_max_level = integer(min=0, default=0)

# Show common cipher list supported by client and server
show_common_cipher_list = boolean(default=False)

# Needed for primary NTLM or Kerberos connections over NLA.
#_advanced
enable_nla = boolean(default=False)

# Needed to connect with jrdp, based on bogus X224 layer code.
#_advanced
bogus_neg_request = boolean(default=False)

# Needed to connect with Remmina 0.8.3 and freerdp 0.9.4, based on bogus MCS layer code.
#_advanced
bogus_user_id = boolean(default=True)

# If enabled, ignore CTRL+ALT+DEL and CTRL+SHIFT+ESCAPE (or the equivalents) keyboard sequences.
#_advanced
disable_tsk_switch_shortcuts = boolean(default=False)

# Specifies the highest compression package support available on the front side
#   0: The RDP bulk compression is disabled
#   1: RDP 4.0 bulk compression
#   2: RDP 5.0 bulk compression
#   3: RDP 6.0 bulk compression
#   4: RDP 6.1 bulk compression
#_advanced
rdp_compression = option(0, 1, 2, 3, 4, default=4)

# Specifies the maximum color resolution (color depth) for client session:
#   8: 8-bit
#   15: 15-bit 555 RGB mask (5 bits for red, 5 bits for green, and 5 bits for blue)
#   16: 16-bit 565 RGB mask (5 bits for red, 6 bits for green, and 5 bits for blue)
#   24: 24-bit RGB mask (8 bits for red, 8 bits for green, and 8 bits for blue)
#   32: 32-bit RGB mask (8 bits for alpha, 8 bits for red, 8 bits for green, and 8 bits for blue)
#_advanced
max_color_depth = option(8, 15, 16, 24, 32, default=24)

# Persistent Disk Bitmap Cache on the front side.
#_advanced
persistent_disk_bitmap_cache = boolean(default=True)

# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
#_advanced
cache_waiting_list = boolean(default=False)

# If enabled, the contents of Persistent Bitmap Caches are stored on disk.
#_advanced
persist_bitmap_cache_on_disk = boolean(default=False)

# Support of Bitmap Compression.
#_advanced
bitmap_compression = boolean(default=True)

# Enables support of Client Fast-Path Input Event PDUs.
#_advanced
fast_path = boolean(default=True)

enable_suppress_output = boolean(default=True)

# [Not configured]: Compatible with more RDP clients (less secure)
# HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)
ssl_cipher_list = string(default='HIGH:!ADH:!3DES:!SHA')

show_target_user_in_f12_message = boolean(default=False)

enable_new_pointer_update = boolean(default=False)

bogus_ios_glyph_support_level = boolean(default=True)

#_advanced
transform_glyph_to_bitmap = boolean(default=False)

#   0: disabled
#   1: pause key only
#   2: all input events
bogus_number_of_fastpath_input_event = option(0, 1, 2, default=1)

# (is in millisecond)
#_advanced
recv_timeout = integer(min=100, max=10000, default=1000)

#_advanced
enable_osd_4_eyes = boolean(default=False)

# Enable front remoteFx
#_advanced
enable_remotefx = boolean(default=True)

bogus_pointer_xormask_padding = boolean(default=False)

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
# (values are comma-separated)
#_advanced
disabled_orders = string(default='25')

# Force usage of bitmap cache V2 for compatibility with WALLIX Access Manager.
#_advanced
force_bitmap_cache_v2_with_am = boolean(default=True)

[mod_rdp]

# Specifies the highest compression package support available on the front side
#   0: The RDP bulk compression is disabled
#   1: RDP 4.0 bulk compression
#   2: RDP 5.0 bulk compression
#   3: RDP 6.0 bulk compression
#   4: RDP 6.1 bulk compression
#_advanced
rdp_compression = option(0, 1, 2, 3, 4, default=4)

#_advanced
disconnect_on_logon_user_change = boolean(default=False)

# (is in second)
#_advanced
open_session_timeout = integer(min=0, default=0)

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
# (values are comma-separated)
#_hidden
disabled_orders = string(default='')

# NLA authentication in secondary target.
#_hidden
enable_nla = boolean(default=True)

# If enabled, NLA authentication will try Kerberos before NTLM.
# (if enable_nla is disabled, this value is ignored).
#_hidden
enable_kerberos = boolean(default=False)

# Persistent Disk Bitmap Cache on the mod side.
#_advanced
persistent_disk_bitmap_cache = boolean(default=True)

# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
#_advanced
cache_waiting_list = boolean(default=True)

# If enabled, the contents of Persistent Bitmap Caches are stored on disk.
#_advanced
persist_bitmap_cache_on_disk = boolean(default=False)

# Enables channels names (example: channel1,channel2,etc). Character * only, activate all with low priority.
# (values are comma-separated)
#_hidden
allow_channels = string(default='*')

# Disable channels names (example: channel1,channel2,etc). Character * only, deactivate all with low priority.
# (values are comma-separated)
#_hidden
deny_channels = string(default='')

# Enables support of Client/Server Fast-Path Input/Update PDUs.
# Fast-Path is required for Windows Server 2012 (or more recent)!
#_advanced
fast_path = boolean(default=True)

# Enables Server Redirection Support.
#_hidden
server_redirection_support = boolean(default=False)

# Needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block.
#_advanced
bogus_sc_net_size = boolean(default=True)

# (values are comma-separated)
#_advanced
proxy_managed_drives = string(default='')

#_hidden
ignore_auth_channel = boolean(default=False)

# Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel.
auth_channel = string(max=7, default='*')

# Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel.
checkout_channel = string(max=7, default='')

#_hidden
alternate_shell = string(default='')

#_hidden
shell_arguments = string(default='')

#_hidden
shell_working_directory = string(default='')

# As far as possible, use client-provided initial program (Alternate Shell)
#_hidden
use_client_provided_alternate_shell = boolean(default=False)

# As far as possible, use client-provided remote program (RemoteApp)
#_hidden
use_client_provided_remoteapp = boolean(default=False)

# As far as possible, use native RemoteApp capability
#_hidden
use_native_remoteapp_capability = boolean(default=True)

#_hidden
enable_session_probe = boolean(default=False)

# Minimum supported server : Windows Server 2008.
# Clipboard redirection should be remain enabled on Terminal Server.
#_hidden
session_probe_use_smart_launcher = boolean(default=True)

#_hidden
session_probe_enable_launch_mask = boolean(default=True)

# Behavior on failure to launch Session Probe.
#   0: ignore failure and continue.
#   1: disconnect user.
#   2: reconnect without Session Probe.
#_hidden
session_probe_on_launch_failure = option(0, 1, 2, default=1)

# This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).
# 0 to disable timeout.
# (is in millisecond)
#_hidden
session_probe_launch_timeout = integer(min=0, max=300000, default=40000)

# This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).
# 0 to disable timeout.
# (is in millisecond)
#_hidden
session_probe_launch_fallback_timeout = integer(min=0, max=300000, default=10000)

# Minimum supported server : Windows Server 2008.
#_hidden
session_probe_start_launch_timeout_timer_only_after_logon = boolean(default=True)

# (is in millisecond)
#_hidden
session_probe_keepalive_timeout = integer(min=0, max=60000, default=5000)

#   0: ignore and continue
#   1: disconnect user
#   2: freeze connection and wait
#_hidden
session_probe_on_keepalive_timeout = option(0, 1, 2, default=1)

# End automatically a disconnected session.
# Session Probe must be enabled to use this feature.
#_hidden
session_probe_end_disconnected_session = boolean(default=False)

#_advanced
session_probe_customize_executable_name = boolean(default=False)

#_hidden
session_probe_enable_log = boolean(default=False)

#_hidden
session_probe_enable_log_rotation = boolean(default=True)

# This policy setting allows you to configure a time limit for disconnected application sessions.
# 0 to disable timeout.
# (is in millisecond)
#_hidden
session_probe_disconnected_application_limit = integer(min=0, max=172800000, default=0)

# This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.
# 0 to disable timeout.
# (is in millisecond)
#_hidden
session_probe_disconnected_session_limit = integer(min=0, max=172800000, default=0)

# This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.
# 0 to disable timeout.
# (is in millisecond)
#_hidden
session_probe_idle_session_limit = integer(min=0, max=172800000, default=0)

#_hidden
session_probe_exe_or_file = string(max=511, default='||CMD')

#_hidden
session_probe_arguments = string(max=511, default=')gen_config_ini" << (REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS) << R"gen_config_ini(')

# (is in millisecond)
#_hidden
session_probe_clipboard_based_launcher_clipboard_initialization_delay = integer(min=0, default=2000)

# (is in millisecond)
#_hidden
session_probe_clipboard_based_launcher_start_delay = integer(min=0, default=0)

# (is in millisecond)
#_hidden
session_probe_clipboard_based_launcher_long_delay = integer(min=0, default=500)

# (is in millisecond)
#_hidden
session_probe_clipboard_based_launcher_short_delay = integer(min=0, default=50)

# (is in millisecond)
#_hidden
session_probe_launcher_abort_delay = integer(min=0, max=300000, default=2000)

#_advanced
session_probe_allow_multiple_handshake = boolean(default=False)

#_hidden
session_probe_enable_crash_dump = boolean(default=False)

#_hidden
session_probe_handle_usage_limit = integer(min=0, max=1000, default=0)

#_hidden
session_probe_memory_usage_limit = integer(min=0, max=200000000, default=0)

# (is in millisecond)
#_hidden
session_probe_end_of_session_check_delay_time = integer(min=0, max=60000, default=0)

#_hidden
session_probe_ignore_ui_less_processes_during_end_of_session_check = boolean(default=True)

#_hidden
session_probe_childless_window_as_unidentified_input_field = boolean(default=True)

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
session_probe_disabled_features = integer(min=0, max=1023, default=352)

#_hidden
enable_bestsafe_integration = boolean(default=False)

# If enabled, disconnected session can be recovered by a different primary user.
#_hidden
session_probe_public_session = boolean(default=False)

# For targets running WALLIX BestSafe only.
#   0: User action will be accepted
#   1: (Same thing as 'allow') 
#   2: User action will be rejected
#_hidden
session_probe_on_account_manipulation = option(0, 1, 2, default=0)

# Keep known server certificates on WAB
#_hidden
server_cert_store = boolean(default=True)

# Behavior of certificates check.
#   0: fails if certificates doesn't match or miss.
#   1: fails if certificate doesn't match, succeed if no known certificate.
#   2: succeed if certificates exists (not checked), fails if missing.
#   3: always succeed.
# System errors like FS access rights issues or certificate decode are always check errors leading to connection rejection.
#_hidden
server_cert_check = option(0, 1, 2, 3, default=1)

# Warn if check allow connexion to server.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
server_access_allowed_message = integer(min=0, max=7, default=1)

# Warn that new server certificate file was created.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
server_cert_create_message = integer(min=0, max=7, default=1)

# Warn that server certificate file was successfully checked.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
server_cert_success_message = integer(min=0, max=7, default=1)

# Warn that server certificate file checking failed.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
server_cert_failure_message = integer(min=0, max=7, default=1)

# Warn that server certificate check raised some internal error.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_hidden
#_hex
server_cert_error_message = integer(min=0, max=7, default=1)

# Do not transmit client machine name or RDP server.
hide_client_name = boolean(default=False)

#_advanced
clean_up_32_bpp_cursor = boolean(default=False)

bogus_ios_rdpdr_virtual_channel = boolean(default=True)

#_advanced
enable_rdpdr_data_analysis = boolean(default=True)

# Delay before automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.
# (is in millisecond)
#_advanced
remoteapp_bypass_legal_notice_delay = integer(min=0, default=0)

# Time limit to automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.
# (is in millisecond)
#_advanced
remoteapp_bypass_legal_notice_timeout = integer(min=0, default=20000)

#_advanced
log_only_relevant_clipboard_activities = boolean(default=True)

#_advanced
experimental_fix_input_event_sync = boolean(default=True)

#_advanced
experimental_fix_too_long_cookie = boolean(default=True)

# Force to split target domain and username with '@' separator.
#_advanced
split_domain = boolean(default=False)

#_hidden
wabam_uses_translated_remoteapp = boolean(default=False)

# Enables Session Shadowing Support.
#_advanced
session_shadowing_support = boolean(default=True)

# Stores CALs issued by the terminal servers.
#_advanced
use_license_store = boolean(default=True)

# Enables support of the remoteFX codec.
#_hidden
enable_remotefx = boolean(default=False)

#_advanced
accept_monitor_layout_change_if_capture_is_not_started = boolean(default=False)

# Connect to the server in Restricted Admin mode.
# This mode must be supported by the server (available from Windows Server 2012 R2), otherwise, connection will fail.
# NLA must be enabled.
#_hidden
enable_restricted_admin_mode = boolean(default=False)

# NLA will be disabled.
# Target must be set for interactive login, otherwise server connection may not be guaranteed.
# Smartcard device must be available on client desktop.
# Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service.
#_hidden
force_smartcard_authentication = boolean(default=False)

[mod_vnc]

# Enable or disable the clipboard from client (client to server).
clipboard_up = boolean(default=False)

# Enable or disable the clipboard from server (server to client).
clipboard_down = boolean(default=False)

# Sets the encoding types in which pixel data can be sent by the VNC server:
#   0: Raw
#   1: CopyRect
#   2: RRE
#   16: ZRLE
#   -239 (0xFFFFFF11): Cursor pseudo-encoding
# (values are comma-separated)
#_advanced
encodings = string(default='')

# VNC server clipboard data encoding type.
#_advanced
server_clipboard_encoding_type = option('utf-8', 'latin1', default='latin1')

#   0: delayed
#   1: duplicated
#   2: continued
#_advanced
bogus_clipboard_infinite_loop = option(0, 1, 2, default=0)

#_hidden
server_is_macos = boolean(default=False)

#_hidden
server_unix_alt = boolean(default=False)

#_hidden
support_cursor_pseudo_encoding = boolean(default=True)

[metrics]

#_advanced
enable_rdp_metrics = boolean(default=False)

#_advanced
enable_vnc_metrics = boolean(default=False)

#_hidden
log_dir_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::Metrics).to_string()) << R"gen_config_ini(')

# (is in second)
#_advanced
log_interval = integer(min=0, default=5)

# (is in hour)
#_advanced
log_file_turnover_interval = integer(min=0, default=24)

# signature key to digest log metrics header info
#_advanced
sign_key = string(default='')

[file_verification]

#_hidden
socket_path = string(default=')gen_config_ini" << (REDEMPTION_CONFIG_VALIDATOR_PATH) << R"gen_config_ini(')

# Enable use of ICAP service for file verification on upload.
#_hidden
enable_up = boolean(default=False)

# Enable use of ICAP service for file verification on download.
#_hidden
enable_down = boolean(default=False)

# Verify text data via clipboard from client to server.
# File verification on upload must be enabled via option Enable up.
#_hidden
clipboard_text_up = boolean(default=False)

# Verify text data via clipboard from server to client
# File verification on download must be enabled via option Enable down.
#_hidden
clipboard_text_down = boolean(default=False)

#_hidden
log_if_accepted = boolean(default=True)

[file_storage]

# Enable storage of transferred files (via RDP Clipboard).
#   never: Never store transferred files.
#   always: Always store transferred files.
#   on_invalid_verification: Transferred files are stored only if file verification is invalid. File verification by ICAP service must be enabled (in section file_verification).
#_hidden
store_file = option('never', 'always', 'on_invalid_verification', default='never')

[icap_server_down]

# Ip or fqdn of ICAP server
host = string(default='')

# Port of ICAP server
port = integer(min=0, default=1344)

# Service name on ICAP server
service_name = string(default='avscan')

# ICAP server uses tls
tls = boolean(default=False)

# Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server
#_advanced
enable_x_context = boolean(default=True)

# Filename sent to ICAP as percent encoding
#_advanced
filename_percent_encoding = boolean(default=False)

[icap_server_up]

# Ip or fqdn of ICAP server
host = string(default='')

# Port of ICAP server
port = integer(min=0, default=1344)

# Service name on ICAP server
service_name = string(default='avscan')

# ICAP server uses tls
tls = boolean(default=False)

# Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server
#_advanced
enable_x_context = boolean(default=True)

# Filename sent to ICAP as percent encoding
#_advanced
filename_percent_encoding = boolean(default=False)

[mod_replay]

# 0 - Wait for Escape, 1 - End session
#_hidden
on_end_of_data = boolean(default=False)

# 0 - replay once, 1 - loop replay
#_hidden
replay_on_loop = boolean(default=False)

[ocr]

#   1: v1
#   2: v2
version = option(1, 2, default=2)

locale = option('latin', 'cyrillic', default='latin')

# (is in 1/100 second)
#_advanced
interval = integer(min=0, default=100)

#_advanced
on_title_bar_only = boolean(default=True)

# Expressed in percentage,
#   0   - all of characters need be recognized
#   100 - accept all results
#_advanced
max_unrecog_char_rate = integer(min=0, max=100, default=40)

[video]

#_advanced
capture_groupid = integer(min=0, default=33)

# Specifies the type of data to be captured:
#   0x00: none
#   0x01: png
#   0x02: wrm
#   0x04: video
#   0x08: ocr
# Note: values can be added (enable all: 0x01 + 0x02 + 0x04 + 0x08 = 0x0f)
#_advanced
#_hex
capture_flags = integer(min=0, max=15, default=11)

# Frame interval.
# (is in 1/10 second)
#_advanced
png_interval = integer(min=0, default=10)

# Frame interval.
# (is in 1/100 second)
#_advanced
frame_interval = integer(min=0, default=40)

# Time between 2 wrm movies.
# (is in second)
#_advanced
break_interval = integer(min=0, default=600)

# Number of png captures to keep.
#_advanced
png_limit = integer(min=0, default=5)

#_advanced
replay_path = string(max=4096, default='/tmp/')

#_hidden
hash_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::Hash).to_string()) << R"gen_config_ini(')

#_hidden
record_tmp_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::RecordTmp).to_string()) << R"gen_config_ini(')

#_hidden
record_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::Record).to_string()) << R"gen_config_ini(')

# Disable keyboard log:
# (Please see also "Keyboard input masking level" in "session_log" section of "Connection Policy".)
#   0x0: none
#   0x1: disable keyboard log in syslog
#   0x2: disable keyboard log in recorded sessions
#   0x4: disable keyboard log in recorded meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
disable_keyboard_log = integer(min=0, max=7, default=1)

# Disable clipboard log:
#   0x0: none
#   0x1: disable clipboard log in syslog
#   0x2: disable clipboard log in recorded sessions
#   0x4: disable clipboard log in recorded meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
disable_clipboard_log = integer(min=0, max=7, default=1)

# Disable (redirected) file system log:
#   0x0: none
#   0x1: disable (redirected) file system log in syslog
#   0x2: disable (redirected) file system log in recorded sessions
#   0x4: disable (redirected) file system log in recorded meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
disable_file_system_log = integer(min=0, max=7, default=1)

#_hidden
rt_display = boolean(default=False)

# The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:
#   0: 24-bit
#   1: 16-bit
#_advanced
wrm_color_depth_selection_strategy = option(0, 1, default=1)

# The compression method of native video capture:
#   0: no compression
#   1: gzip
#   2: snappy
#_advanced
wrm_compression_algorithm = option(0, 1, 2, default=1)

# Needed to play a video with ffplay or VLC.
# Note: Useless with mpv and mplayer.
#_advanced
bogus_vlc_frame_rate = boolean(default=True)

#_advanced
codec_id = string(default='mp4')

#_advanced
framerate = integer(min=0, default=5)

# FFmpeg options for video codec.
#_advanced
ffmpeg_options = string(default='profile=baseline preset=ultrafast flags=+qscale b=80000')

#_advanced
notimestamp = boolean(default=False)

#   0: Disabled. When replaying the session video, the content of the RDP viewer matches the size of the client's desktop
#   1: When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session
#   2: When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session
smart_video_cropping = option(0, 1, 2, default=0)

# Needed to play a video with corrupted Bitmap Update.
# Note: Useless with mpv and mplayer.
#_advanced
play_video_with_corrupted_bitmap = boolean(default=False)

[crypto]

#_hidden
encryption_key = string(min=64, max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')

#_hidden
sign_key = string(min=64, max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')

[websocket]

# Enable websocket protocol (ws or wss with use_tls=1)
#_hidden
enable_websocket = boolean(default=False)

# Use TLS with websocket (wss)
#_hidden
use_tls = boolean(default=True)

# ${addr}:${port} or ${port} or ${unix_socket_path}
#_hidden
listen_address = string(default=':3390')

[debug]

#_hidden
fake_target_ip = string(default='')

#_advanced
#_hex
primary_orders = integer(min=0, default=0)

#_advanced
#_hex
secondary_orders = integer(min=0, default=0)

#_advanced
#_hex
bitmap_update = integer(min=0, default=0)

#_advanced
#_hex
capture = integer(min=0, default=0)

#_advanced
#_hex
auth = integer(min=0, default=0)

#_advanced
#_hex
session = integer(min=0, default=0)

#_advanced
#_hex
front = integer(min=0, default=0)

#_advanced
#_hex
mod_rdp = integer(min=0, default=0)

#_advanced
#_hex
mod_vnc = integer(min=0, default=0)

#_advanced
#_hex
mod_internal = integer(min=0, default=0)

#_advanced
#_hex
mod_xup = integer(min=0, default=0)

#_hidden
password = integer(min=0, default=0)

#_advanced
#_hex
compression = integer(min=0, default=0)

#_advanced
#_hex
cache = integer(min=0, default=0)

#_advanced
#_hex
performance = integer(min=0, default=0)

#_advanced
#_hex
pass_dialog_box = integer(min=0, default=0)

#_advanced
#_hex
ocr = integer(min=0, default=0)

#_advanced
#_hex
ffmpeg = integer(min=0, default=0)

#_advanced
config = boolean(default=True)

[remote_program]

allow_resize_hosted_desktop = boolean(default=True)

[translation]

#_advanced
language = option('en', 'fr', default='en')

#_advanced
password_en = string(default='')

#_advanced
password_fr = string(default='')

[internal_mod]

#_advanced
load_theme = string(default='')

)gen_config_ini"
