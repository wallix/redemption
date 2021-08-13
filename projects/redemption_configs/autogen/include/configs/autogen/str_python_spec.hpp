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
encryptionLevel = option('low', 'medium', 'high', default='low')

#_advanced
authfile = string(default=')gen_config_ini" << (REDEMPTION_CONFIG_AUTHFILE) << R"gen_config_ini(')

# Time out during RDP handshake stage.
# (in seconds)
handshake_timeout = integer(min=0, default=10)

# No automatic disconnection due to inactivity, timer is set on primary authentication.
# If value is between 1 and 30, then 30 is used.
# If value is set to 0, then session timeout value is unlimited.
# (in seconds)
session_timeout = integer(min=0, default=900)

# No automatic disconnection due to inactivity, timer is set on target session.
# If value is between 1 and 30, then 30 is used.
# If value is set to 0, then value set in "Session timeout" (in "RDP Proxy" configuration option) is used.
# (in seconds)
#_hidden
inactivity_timeout = integer(min=0, default=0)

# Internal keepalive between sesman and rdp proxy
# (in seconds)
#_hidden
keepalive_grace_delay = integer(min=0, default=30)

# Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).
# (in seconds)
#_advanced
authentication_timeout = integer(min=0, default=120)

# Specifies the time to spend on the close box of proxy RDP before closing client window (0 to desactivate).
# (in seconds)
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

allow_scale_factor = boolean(default=False)

# Needed to refresh screen of Windows Server 2012.
#_advanced
bogus_refresh_rect = boolean(default=True)

#_advanced
large_pointer_support = boolean(default=True)

new_pointer_update_support = boolean(default=True)

unicode_keyboard_event_support = boolean(default=True)

# (in milliseconds)
#_advanced
mod_recv_timeout = integer(min=100, max=10000, default=1000)

#_advanced
experimental_enable_serializer_data_block_size_limit = boolean(default=False)

#_advanced
experimental_support_resize_session_during_recording = boolean(default=True)

#_advanced
support_connection_redirection_during_recording = boolean(default=True)

# Prevent Remote Desktop session timeouts due to idle tcp sessions by sending periodically keep alive packet to client.
# !!!May cause FreeRDP-based client to CRASH!!!
# Set to 0 to disable this feature.
# (in milliseconds)
rdp_keepalive_connection_interval = integer(min=0, default=0)

# Enable primary connection on ipv6
#_hidden
enable_ipv6 = boolean(default=False)

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

# cs-CZ, da-DK, de-DE, el-GR, en-US, es-ES, fi-FI.finnish, fr-FR, is-IS, it-IT, nl-NL, nb-NO, pl-PL.programmers, pt-BR.abnt, ro-RO, ru-RU, hr-HR, sk-SK, sv-SE, tr-TR.q, uk-UA, sl-SI, et-EE, lv-LV, lt-LT.ibm, mk-MK, fo-FO, mt-MT.47, se-NO, kk-KZ, ky-KG, tt-RU, mn-MN, cy-GB, lb-LU, mi-NZ, de-CH, en-GB, es-MX, fr-BE.fr, nl-BE, pt-PT, sr-La, se-SE, uz-Cy, iu-La, fr-CA, sr-Cy, en-CA.fr, fr-CH, en-IE.irish, bs-Cy, bg-BG.latin, cs-CZ.qwerty, de-DE.ibm, el-GR.220, en-US.dvorak, es-ES.variation, hu-HU, it-IT.142, pl-PL, pt-BR.abnt2, ru-RU.typewriter, sk-SK.qwerty, tr-TR.f, lv-LV.qwerty, lt-LT, mt-MT.48, se-NO.ext_norway, fr-BE, se-SE, en-CA.multilingual, en-IE, cs-CZ.programmers, el-GR.319, en-US.international, se-SE.ext_finland_sweden, bg-BG, el-GR.220_latin, en-US.dvorak_left, el-GR.319_latin, en-US.dvorak_right, el-GR.latin, el-GR.polytonic
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

bogus_ios_glyph_support_level = boolean(default=True)

#_advanced
transform_glyph_to_bitmap = boolean(default=False)

#   0: disabled
#   1: pause key only
#   2: all input events
bogus_number_of_fastpath_input_event = option(0, 1, 2, default=1)

# (in milliseconds)
#_advanced
recv_timeout = integer(min=100, max=10000, default=1000)

# Enables display of message informing user that his/her session is being audited.
enable_osd_4_eyes = boolean(default=True)

# Enable front remoteFx
#_advanced
enable_remotefx = boolean(default=True)

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

# (in seconds)
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

# List of enabled (static) virtual channel (example: channel1,channel2,etc). Character * only, activate all with low priority.
# (values are comma-separated)
#_hidden
allow_channels = string(default='*')

# List of disabled (static) virtual channel (example: channel1,channel2,etc). Character * only, deactivate all with low priority.
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

# Client Address to send to target (in InfoPacket)
#   0: Send 0.0.0.0
#   1: Send proxy client address or target connexion
#   2: Send user client address of front connexion
#_advanced
client_address_sent = option(0, 1, 2, default=0)

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
# (in milliseconds)
#_hidden
session_probe_launch_timeout = integer(min=0, max=300000, default=40000)

# This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).
# 0 to disable timeout.
# (in milliseconds)
#_hidden
session_probe_launch_fallback_timeout = integer(min=0, max=300000, default=40000)

# Minimum supported server : Windows Server 2008.
#_hidden
session_probe_start_launch_timeout_timer_only_after_logon = boolean(default=True)

# (in milliseconds)
#_hidden
session_probe_keepalive_timeout = integer(min=0, max=60000, default=5000)

#   0: ignore and continue
#   1: disconnect user
#   2: freeze connection and wait
#_hidden
session_probe_on_keepalive_timeout = option(0, 1, 2, default=1)

# End automatically a disconnected session.
# This option is recommended for Web applications running in Desktop mode.
# Session Probe must be enabled to use this feature.
#_hidden
session_probe_end_disconnected_session = boolean(default=False)

#_advanced
session_probe_customize_executable_name = boolean(default=False)

#_hidden
session_probe_enable_log = boolean(default=False)

#_hidden
session_probe_enable_log_rotation = boolean(default=False)

#   1: Fatal
#   2: Error
#   3: Info
#   4: Warning
#   5: Debug
#   6: Detail
#_hidden
session_probe_log_level = option(1, 2, 3, 4, 5, 6, default=5)

# (Deprecated!) This policy setting allows you to configure a time limit for disconnected application sessions.
# 0 to disable timeout.
# (in milliseconds)
#_hidden
session_probe_disconnected_application_limit = integer(min=0, max=172800000, default=0)

# This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.
# 0 to disable timeout.
# (in milliseconds)
#_hidden
session_probe_disconnected_session_limit = integer(min=0, max=172800000, default=0)

# This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.
# 0 to disable timeout.
# (in milliseconds)
#_hidden
session_probe_idle_session_limit = integer(min=0, max=172800000, default=0)

#_hidden
session_probe_exe_or_file = string(max=511, default='||CMD')

#_hidden
session_probe_arguments = string(max=511, default=')gen_config_ini" << (REDEMPTION_CONFIG_SESSION_PROBE_ARGUMENTS) << R"gen_config_ini(')

# (in milliseconds)
#_hidden
session_probe_clipboard_based_launcher_clipboard_initialization_delay = integer(min=0, default=2000)

# (in milliseconds)
#_hidden
session_probe_clipboard_based_launcher_start_delay = integer(min=0, default=0)

# (in milliseconds)
#_hidden
session_probe_clipboard_based_launcher_long_delay = integer(min=0, default=500)

# (in milliseconds)
#_hidden
session_probe_clipboard_based_launcher_short_delay = integer(min=0, default=50)

# (in milliseconds)
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

# (in milliseconds)
#_hidden
session_probe_end_of_session_check_delay_time = integer(min=0, max=60000, default=0)

#_hidden
session_probe_ignore_ui_less_processes_during_end_of_session_check = boolean(default=True)

#_hidden
session_probe_childless_window_as_unidentified_input_field = boolean(default=True)

#_hidden
session_probe_update_disabled_features = boolean(default=True)

#   0x000: none
#   0x001: Java Access Bridge
#   0x002: MS Active Accessbility
#   0x004: MS UI Automation
#   0x010: Inspect Edge location URL
#   0x020: Inspect Chrome Address/Search bar
#   0x040: Inspect Firefox Address/Search bar
#   0x080: Monitor Internet Explorer event
#   0x100: Inspect group membership of user
# Note: values can be added (enable all: 0x001 + 0x002 + 0x004 + 0x010 + 0x020 + 0x040 + 0x080 + 0x100 = 0x1f7)
#_hidden
#_hex
session_probe_disabled_features = integer(min=0, max=511, default=352)

#_hidden
session_probe_bestsafe_integration = boolean(default=False)

# For targets running WALLIX BestSafe only.
#   0: User action will be accepted
#   1: (Same thing as 'allow') 
#   2: User action will be rejected
#_hidden
session_probe_on_account_manipulation = option(0, 1, 2, default=0)

# The name of the environment variable pointing to the alternative directory to launch Session Probe.
# If empty, the environment variable TMP will be used.
#_hidden
session_probe_alternate_directory_environment_variable = string(max=3, default='')

# If enabled, disconnected session can be recovered by a different primary user.
#_hidden
session_probe_public_session = boolean(default=False)

#_advanced
session_probe_at_end_of_session_freeze_connection_and_wait = boolean(default=True)

#_advanced
session_probe_enable_cleaner = boolean(default=True)

#_advanced
session_probe_clipboard_based_launcher_reset_keyboard_status = boolean(default=True)

#_hidden
application_driver_exe_or_file = string(max=256, default=')gen_config_ini" << (REDEMPTION_CONFIG_APPLICATION_DRIVER_EXE_OR_FILE) << R"gen_config_ini(')

#_hidden
application_driver_script_argument = string(max=256, default=')gen_config_ini" << (REDEMPTION_CONFIG_APPLICATION_DRIVER_SCRIPT_ARGUMENT) << R"gen_config_ini(')

#_hidden
application_driver_chrome_dt_script = string(max=256, default=')gen_config_ini" << (REDEMPTION_CONFIG_APPLICATION_DRIVER_CHROME_DT_SCRIPT) << R"gen_config_ini(')

#_hidden
application_driver_chrome_uia_script = string(max=256, default=')gen_config_ini" << (REDEMPTION_CONFIG_APPLICATION_DRIVER_CHROME_UIA_SCRIPT) << R"gen_config_ini(')

#_hidden
application_driver_ie_script = string(max=256, default=')gen_config_ini" << (REDEMPTION_CONFIG_APPLICATION_DRIVER_IE_SCRIPT) << R"gen_config_ini(')

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

bogus_ios_rdpdr_virtual_channel = boolean(default=True)

#_advanced
enable_rdpdr_data_analysis = boolean(default=True)

# Delay before automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.
# (in milliseconds)
#_advanced
remoteapp_bypass_legal_notice_delay = integer(min=0, default=0)

# Time limit to automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.
# (in milliseconds)
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

# Enable target connection on ipv6
#_hidden
enable_ipv6 = boolean(default=False)

#_hidden
auto_reconnection_on_losing_target_link = boolean(default=False)

# Forward the build number advertised by the client to the server. If forwarding is disabled a default (static) build number will be sent to the server.
#_hidden
forward_client_build_number = boolean(default=True)

# To resolve the session freeze issue with Windows 7/Windows Server 2008 target.
#_hidden
bogus_monitor_layout_treatment = boolean(default=False)

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

# Enable target connection on ipv6
#_hidden
enable_ipv6 = boolean(default=False)

[metrics]

#_advanced
enable_rdp_metrics = boolean(default=False)

#_advanced
enable_vnc_metrics = boolean(default=False)

#_hidden
log_dir_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::Metrics)) << R"gen_config_ini(')

# (in seconds)
#_advanced
log_interval = integer(min=0, default=5)

# (in hours)
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

# Block file transfer from client to server on invalid file verification.
# File verification on upload must be enabled via option Enable up.
#_hidden
block_invalid_file_up = boolean(default=False)

# Block file transfer from server to client on invalid file verification.
# File verification on download must be enabled via option Enable down.
#_hidden
block_invalid_file_down = boolean(default=False)

# Block text transfer from client to server on invalid text verification.
# Text verification on upload must be enabled via option Clipboard text up.
#_hidden
block_invalid_clipboard_text_up = boolean(default=False)

# Block text transfer from server to client on invalid text verification.
# Text verification on download must be enabled via option Clipboard text down.
#_hidden
block_invalid_clipboard_text_down = boolean(default=False)

#_hidden
log_if_accepted = boolean(default=True)

# If option Block invalid file (up or down) is enabled, automatically reject file with greater filesize (in megabytes).
# Warning: This value affects the RAM used by the session.
#_hidden
max_file_size_rejected = integer(min=0, default=256)

# Temporary path used when files take up too much memory.
#_hidden
tmpdir = string(max=4096, default='/tmp/')

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

# (in 1/100 seconds)
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
#   0x08: ocr
# Note: values can be added (enable all: 0x01 + 0x02 + 0x08 = 0x0b)
#_advanced
#_hex
capture_flags = integer(min=0, max=15, default=11)

# Frame interval.
# (in 1/10 seconds)
#_advanced
png_interval = integer(min=0, default=10)

# Time between 2 wrm movies.
# (in seconds)
#_advanced
break_interval = integer(min=0, default=600)

# Number of png captures to keep.
#_advanced
png_limit = integer(min=0, default=5)

#_advanced
replay_path = string(max=4096, default='/tmp/')

#_hidden
hash_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::Hash)) << R"gen_config_ini(')

#_hidden
record_tmp_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::RecordTmp)) << R"gen_config_ini(')

#_hidden
record_path = string(max=4096, default=')gen_config_ini" << (app_path(AppPath::Record)) << R"gen_config_ini(')

# Disable keyboard log:
# (Please see also "Keyboard input masking level" in "session_log".)
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

# Needed to play a video with old ffplay or VLC v1.
# Note: Useless with mpv, MPlayer or VLC v2.
#_advanced
#_display_name=Bogus VLC frame rate
bogus_vlc_frame_rate = boolean(default=True)

#_advanced
codec_id = string(default='mp4')

#_advanced
#_display_name=Frame rate
framerate = integer(min=1, max=120, default=5)

# FFmpeg options for video codec. See https://trac.ffmpeg.org/wiki/Encode/H.264
# /!\ Some browsers and video decoders don't support crf=0
#_advanced
ffmpeg_options = string(default='crf=35 preset=superfast')

#_advanced
notimestamp = boolean(default=False)

#   0: Disabled. When replaying the session video, the content of the RDP viewer matches the size of the client's desktop
#   1: When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session
#   2: When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session
smart_video_cropping = option(0, 1, 2, default=0)

# Needed to play a video with corrupted Bitmap Update.
#_advanced
play_video_with_corrupted_bitmap = boolean(default=False)

# Allow real-time view (4 eyes) without session recording enabled in the authorization
allow_rt_without_recording = boolean(default=False)

# Allow to control permissions on recorded files with octal number
# (in octal or symbolic mode format (as chmod Linux command))
#_hidden
file_permissions = string(default='440')

# Use only session id for basename
#_hidden
rt_basename_only_sid = boolean(default=False)

[crypto]

# (in hexadecimal format)
#_hidden
encryption_key = string(min=64, max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')

# (in hexadecimal format)
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

# - kbd / ocr when != 0
# 
# (Wrm)
# - pointer             = 0x0004
# - primary_orders      = 0x0020
# - secondary_orders    = 0x0040
# - bitmap_update       = 0x0080
# - surface_commands    = 0x0100
# - bmp_cache           = 0x0200
# - internal_buffer     = 0x0400
# - sec_decrypted       = 0x1000
#_advanced
#_hex
capture = integer(min=0, default=0)

# - variable = 0x0002
# - buffer   = 0x0040
# - dump     = 0x1000
#_advanced
#_hex
auth = integer(min=0, default=0)

# - Event   = 0x02
# - Acl     = 0x04
# - Trace   = 0x08
#_advanced
#_hex
session = integer(min=0, default=0)

# - basic_trace     = 0x00000001
# - basic_trace2    = 0x00000002
# - basic_trace3    = 0x00000004
# - basic_trace4    = 0x00000008
# - basic_trace5    = 0x00000020
# - graphic         = 0x00000040
# - channel         = 0x00000080
# - cache_from_disk = 0x00000400
# - bmp_info        = 0x00000800
# - global_channel  = 0x00002000
# - sec_decrypted   = 0x00004000
# - keymap          = 0x00008000
# 
# (Serializer)
# - pointer             = 0x00040000
# - primary_orders      = 0x00200000
# - secondary_orders    = 0x00400000
# - bitmap_update       = 0x00800000
# - surface_commands    = 0x01000000
# - bmp_cache           = 0x02000000
# - internal_buffer     = 0x04000000
# - sec_decrypted       = 0x10000000
#_advanced
#_hex
front = integer(min=0, default=0)

# - basic_trace         = 0x00000001
# - connection          = 0x00000002
# - security            = 0x00000004
# - capabilities        = 0x00000008
# - license             = 0x00000010
# - asynchronous_task   = 0x00000020
# - graphics_pointer    = 0x00000040
# - graphics            = 0x00000080
# - input               = 0x00000100
# - rail_order          = 0x00000200
# - credssp             = 0x00000400
# - negotiation         = 0x00000800
# - cache_persister     = 0x00001000
# - fsdrvmgr            = 0x00002000
# - sesprobe_launcher   = 0x00004000
# - sesprobe_repetitive = 0x00008000
# - drdynvc             = 0x00010000
# - surfaceCmd          = 0x00020000
# - cache_from_disk     = 0x00040000
# - bmp_info            = 0x00080000
# - drdynvc_dump        = 0x00100000
# - printer             = 0x00200000
# - rdpsnd              = 0x00400000
# - channels            = 0x00800000
# - rail                = 0x01000000
# - sesprobe            = 0x02000000
# - cliprdr             = 0x04000000
# - rdpdr               = 0x08000000
# - rail_dump           = 0x10000000
# - sesprobe_dump       = 0x20000000
# - cliprdr_dump        = 0x40000000
# - rdpdr_dump          = 0x80000000
#_advanced
#_hex
mod_rdp = integer(min=0, default=0)

# - basic_trace     = 0x00000001
# - keymap_stack    = 0x00000002
# - draw_event      = 0x00000004
# - input           = 0x00000008
# - connection      = 0x00000010
# - hextile_encoder = 0x00000020
# - cursor_encoder  = 0x00000040
# - clipboard       = 0x00000080
# - zrle_encoder    = 0x00000100
# - zrle_trace      = 0x00000200
# - hextile_trace   = 0x00000400
# - cursor_trace    = 0x00001000
# - rre_encoder     = 0x00002000
# - rre_trace       = 0x00004000
# - raw_encoder     = 0x00008000
# - raw_trace       = 0x00010000
# - copyrect_encoder= 0x00020000
# - copyrect_trace  = 0x00040000
# - keymap          = 0x00080000
#_advanced
#_hex
mod_vnc = integer(min=0, default=0)

# - copy_paste != 0
# - client_execute = 0x01
#_advanced
#_hex
mod_internal = integer(min=0, default=0)

# - basic    = 0x0001
# - dump     = 0x0002
# - watchdog = 0x0004
#_advanced
#_hex
sck_mod = integer(min=0, default=0)

# - basic    = 0x0001
# - dump     = 0x0002
# - watchdog = 0x0004
#_advanced
#_hex
sck_front = integer(min=0, default=0)

#_hidden
password = integer(min=0, default=0)

# - when != 0
#_advanced
#_hex
compression = integer(min=0, default=0)

# - life       = 0x0001
# - persistent = 0x0200
#_advanced
#_hex
cache = integer(min=0, default=0)

# - when != 0
#_advanced
#_hex
ocr = integer(min=0, default=0)

# avlog level
#_advanced
#_hex
ffmpeg = integer(min=0, default=0)

#_advanced
config = boolean(default=True)

#   0: Off
#   1: SimulateErrorRead
#   2: SimulateErrorWrite
#_hidden
mod_rdp_use_failure_simulation_socket_transport = option(0, 1, 2, default=0)

[remote_program]

allow_resize_hosted_desktop = boolean(default=True)

[translation]

#_hidden
language = option('en', 'fr', default='en')

#_advanced
login_language = option('Auto', 'EN', 'FR', default='Auto')

[internal_mod]

# Enable target edit field in login page.
#_advanced
enable_target_field = boolean(default=True)

[theme]

# Enable custom theme color configuration. Each theme color can be defined as HTML color code (white: #FFFFFF, black: #000000, blue: #0000FF, etc)
enable_theme = boolean(default=False)

# Logo displayed when theme is enabled
#_image=/var/wab/images/rdp-oem-logo.png
logo = string(default=')gen_config_ini" << (REDEMPTION_CONFIG_THEME_LOGO) << R"gen_config_ini(')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
bgcolor = string(default='#081F60')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
fgcolor = string(default='#FFFFFF')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
separator_color = string(default='#CFD5EB')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
focus_color = string(default='#004D9C')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
error_color = string(default='#FFFF00')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
edit_bgcolor = string(default='#FFFFFF')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
edit_fgcolor = string(default='#000000')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
edit_focus_color = string(default='#004D9C')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
tooltip_bgcolor = string(default='#000000')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
tooltip_fgcolor = string(default='#FFFF9F')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
tooltip_border_color = string(default='#000000')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_line1_bgcolor = string(default='#E9ECF6')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_line1_fgcolor = string(default='#000000')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_line2_bgcolor = string(default='#CFD5EB')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_line2_fgcolor = string(default='#000000')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_selected_bgcolor = string(default='#4472C4')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_selected_fgcolor = string(default='#FFFFFF')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_focus_bgcolor = string(default='#004D9C')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_focus_fgcolor = string(default='#FFFFFF')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_label_bgcolor = string(default='#4472C4')

# (is in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)
selector_label_fgcolor = string(default='#FFFFFF')

)gen_config_ini"
