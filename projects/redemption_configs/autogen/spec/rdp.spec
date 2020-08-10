[general]

# Secondary login Transformation rule
# ${LOGIN} will be replaced by login
# ${DOMAIN} (optional) will be replaced by domain if it exists.
# Empty value means no transformation rule.
transformation_rule = string(default='')

# Account Mapping password retriever
# Transformation to apply to find the correct account.
# ${USER} will be replaced by the user's login.
# ${DOMAIN} will be replaced by the user's domain (in case of LDAP mapping).
# ${USER_DOMAIN} will be replaced by the user's login + "@" + user's domain (or just user's login if there's no domain).
# ${GROUP} will be replaced by the authorization's user group.
# ${DEVICE} will be replaced by the device's name.
# A regular expression is allowed to transform a variable, with the syntax: ${USER:/regex/replacement}, groups can be captured with parentheses and used with \1, \2, ...
# For example to replace leading "A" by "B" in the username: ${USER:/^A/B}
# Empty value means no transformation rule.
vault_transformation_rule = string(default='')


[session]

# No traffic auto disconnection.
# If value is 0, global value (session_timeout) is used.
# (is in second)
inactivity_timeout = integer(min=0, default=0)

[session_log]

# Keyboard Input Masking Level:
#   0: keyboard input are not masked
#   1: only passwords are masked
#   2: passwords and unidentified texts are masked
#   3: keyboard inputs are not logged
keyboard_input_masking_level = option(0, 1, 2, 3, default=2)

[rdp]

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
disabled_orders = string(default='')

# NLA authentication in secondary target.
enable_nla = boolean(default=True)

# If enabled, NLA authentication will try Kerberos before NTLM.
# (if enable_nla is disabled, this value is ignored).
enable_kerberos = boolean(default=False)

# Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_min_level = integer(min=0, default=0)

# Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_max_level = integer(min=0, default=0)

# TLSv1.2 additional ciphers supported by client, default is empty to apply system-wide configuration (SSL security level 2), ALL for support of all ciphers to ensure highest compatibility with target servers.
cipher_string = string(default='ALL')

# Show common cipher list supported by client and server
show_common_cipher_list = boolean(default=False)

# List of enabled dynamic virtual channel (example: channel1,channel2,etc). Character * only, activate all.
#_advanced
allowed_dynamic_channels = string(default='*')

# List of disabled dynamic virtual channel (example: channel1,channel2,etc). Character * only, deactivate all.
#_advanced
denied_dynamic_channels = string(default='')

# Enables Server Redirection Support.
server_redirection = boolean(default=False)

# Load balancing information
load_balance_info = string(default='')

# As far as possible, use client-provided initial program (Alternate Shell)
use_client_provided_alternate_shell = boolean(default=False)

# As far as possible, use client-provided remote program (RemoteApp)
use_client_provided_remoteapp = boolean(default=False)

# As far as possible, use native RemoteApp capability
use_native_remoteapp_capability = boolean(default=True)

wabam_uses_translated_remoteapp = boolean(default=False)

# Enables support of the remoteFX codec.
enable_remotefx = boolean(default=False)

# Connect to the server in Restricted Admin mode.
# This mode must be supported by the server (available from Windows Server 2012 R2), otherwise, connection will fail.
# NLA must be enabled.
enable_restricted_admin_mode = boolean(default=False)

# NLA will be disabled.
# Target must be set for interactive login, otherwise server connection may not be guaranteed.
# Smartcard device must be available on client desktop.
# Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service.
force_smartcard_authentication = boolean(default=False)

# Enable target connection on ipv6
enable_ipv6 = boolean(default=False)

# Console mode management for targets on Windows Server 2003 (requested with /console or /admin mstsc option)
#   allow: Forward Console mode request from client to the target.
#   force: Force Console mode on target regardless of client request.
#   forbid: Block Console mode request from client.
mode_console = option('allow', 'force', 'forbid', default='allow')

# Delay before showing disconnect message after the last RemoteApp window is closed.
# (is in millisecond)
#_advanced
remote_programs_disconnect_message_delay = integer(min=3000, max=120000, default=3000)

# Use Session Probe to launch Remote Program as much as possible.
use_session_probe_to_launch_remote_program = boolean(default=True)

[session_probe]

enable_session_probe = boolean(default=True)

# Minimum supported server : Windows Server 2008.
# Clipboard redirection should be remain enabled on Terminal Server.
use_smart_launcher = boolean(default=True)

#_advanced
enable_launch_mask = boolean(default=True)

# Behavior on failure to launch Session Probe.
#   0: ignore failure and continue.
#   1: disconnect user.
#   2: reconnect without Session Probe.
on_launch_failure = option(0, 1, 2, default=1)

# This parameter is used if session_probe_on_launch_failure is 1 (disconnect user).
# 0 to disable timeout.
# (is in millisecond)
#_advanced
launch_timeout = integer(min=0, max=300000, default=40000)

# This parameter is used if session_probe_on_launch_failure is 0 (ignore failure and continue) or 2 (reconnect without Session Probe).
# 0 to disable timeout.
# (is in millisecond)
#_advanced
launch_fallback_timeout = integer(min=0, max=300000, default=10000)

# Minimum supported server : Windows Server 2008.
start_launch_timeout_timer_only_after_logon = boolean(default=True)

# (is in millisecond)
#_advanced
keepalive_timeout = integer(min=0, max=60000, default=5000)

#   0: ignore and continue
#   1: disconnect user
#   2: freeze connection and wait
on_keepalive_timeout = option(0, 1, 2, default=1)

# End automatically a disconnected session.
# Session Probe must be enabled to use this feature.
end_disconnected_session = boolean(default=False)

#_advanced
enable_log = boolean(default=False)

#_advanced
enable_log_rotation = boolean(default=True)

#   0: Off
#   1: Fatal
#   2: Error
#   3: Info
#   4: Warning
#   5: Debug
#   6: Detail
#_advanced
log_level = option(0, 1, 2, 3, 4, 5, 6, default=5)

# This policy setting allows you to configure a time limit for disconnected application sessions.
# 0 to disable timeout.
# (is in millisecond)
#_advanced
disconnected_application_limit = integer(min=0, max=172800000, default=0)

# This policy setting allows you to configure a time limit for disconnected Terminal Services sessions.
# 0 to disable timeout.
# (is in millisecond)
#_advanced
disconnected_session_limit = integer(min=0, max=172800000, default=0)

# This parameter allows you to specify the maximum amount of time that an active Terminal Services session can be idle (without user input) before it is automatically locked by Session Probe.
# 0 to disable timeout.
# (is in millisecond)
#_advanced
idle_session_limit = integer(min=0, max=172800000, default=0)

# (is in millisecond)
#_advanced
smart_launcher_clipboard_initialization_delay = integer(min=0, default=2000)

# (is in millisecond)
#_advanced
smart_launcher_start_delay = integer(min=0, default=0)

# (is in millisecond)
#_advanced
smart_launcher_long_delay = integer(min=0, default=500)

# (is in millisecond)
#_advanced
smart_launcher_short_delay = integer(min=0, default=50)

# (is in millisecond)
#_advanced
launcher_abort_delay = integer(min=0, max=300000, default=2000)

#_advanced
enable_crash_dump = boolean(default=False)

#_advanced
handle_usage_limit = integer(min=0, max=1000, default=0)

#_advanced
memory_usage_limit = integer(min=0, max=200000000, default=0)

# (is in millisecond)
#_advanced
end_of_session_check_delay_time = integer(min=0, max=60000, default=0)

#_advanced
ignore_ui_less_processes_during_end_of_session_check = boolean(default=True)

#_advanced
update_disabled_features = boolean(default=True)

#_advanced
childless_window_as_unidentified_input_field = boolean(default=True)

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
#_advanced
#_hex
disabled_features = integer(min=0, max=511, default=352)

enable_bestsafe_interaction = boolean(default=False)

# The name of the environment variable pointing to the alternative directory to launch Session Probe.
# If empty, the environment variable TMP will be used.
#_advanced
alternate_directory_environment_variable = string(max=3, default='')

# If enabled, disconnected session can be recovered by a different primary user.
public_session = boolean(default=False)

# For targets running WALLIX BestSafe only.
#   0: User action will be accepted
#   1: (Same thing as 'allow') 
#   2: User action will be rejected
on_account_manipulation = option(0, 1, 2, default=0)

#_advanced
auto_reconnection_on_losing_target_link = boolean(default=False)

# Comma-separated rules (Ex.: $deny:192.168.0.0/24:*,$allow:host.domain.net:3389,$allow:192.168.0.110:*)
# (Ex. for backwards compatibility only: 10.1.0.0/16:22)
# Session Probe must be enabled to use this feature.
outbound_connection_monitoring_rules = string(default='')

# Comma-separated rules (Ex.: $deny:Taskmgr)
# @ = All child processes of Bastion Application (Ex.: $deny:@)
# Session Probe must be enabled to use this feature.
process_monitoring_rules = string(default='')

# Comma-separated extra system processes (Ex.: dllhos.exe,TSTheme.exe)
extra_system_processes = string(default='')

# Comma-separated processes (Ex.: chrome.exe,ngf.exe)
windows_of_these_applications_as_unidentified_input_field = string(default='')

[server_cert]

# Keep known server certificates on WAB
server_cert_store = boolean(default=True)

# Behavior of certificates check.
#   0: fails if certificates doesn't match or miss.
#   1: fails if certificate doesn't match, succeed if no known certificate.
#   2: succeed if certificates exists (not checked), fails if missing.
#   3: always succeed.
# System errors like FS access rights issues or certificate decode are always check errors leading to connection rejection.
server_cert_check = option(0, 1, 2, 3, default=1)

# Warn if check allow connexion to server.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_access_allowed_message = integer(min=0, max=7, default=1)

# Warn that new server certificate file was created.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_cert_create_message = integer(min=0, max=7, default=1)

# Warn that server certificate file was successfully checked.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_cert_success_message = integer(min=0, max=7, default=1)

# Warn that server certificate file checking failed.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_cert_failure_message = integer(min=0, max=7, default=1)

[file_verification]

# Enable use of ICAP service for file verification on upload.
enable_up = boolean(default=False)

# Enable use of ICAP service for file verification on download.
enable_down = boolean(default=False)

# Verify text data via clipboard from client to server.
# File verification on upload must be enabled via option Enable up.
clipboard_text_up = boolean(default=False)

# Verify text data via clipboard from server to client
# File verification on download must be enabled via option Enable down.
clipboard_text_down = boolean(default=False)

# Block file transfer from client to server on invalid file verification.
# File verification on upload must be enabled via option Enable up.
block_invalid_file_up = boolean(default=False)

# Block file transfer from server to client on invalid file verification.
# File verification on download must be enabled via option Enable down.
block_invalid_file_down = boolean(default=False)

#_advanced
log_if_accepted = boolean(default=True)

# If option Block invalid file (up or down) is enabled, automatically reject file with greater filesize (in megabytes).
# Warning: This value affects the RAM used by the session.
#_advanced
max_file_size_rejected = integer(min=0, default=50)

[file_storage]

# Enable storage of transferred files (via RDP Clipboard).
#   never: Never store transferred files.
#   always: Always store transferred files.
#   on_invalid_verification: Transferred files are stored only if file verification is invalid. File verification by ICAP service must be enabled (in section file_verification).
store_file = option('never', 'always', 'on_invalid_verification', default='never')

[video]

# Disable keyboard log:
# (Please see also "Keyboard input masking level" in "session_log" section of "Connection Policy".)
#   0x0: none
#   0x1: disable keyboard log in syslog
#   0x2: disable keyboard log in recorded sessions
# Note: values can be added (disable all: 0x1 + 0x2 = 0x3)
#_advanced
#_hex
disable_keyboard_log = integer(min=0, max=3, default=1)

