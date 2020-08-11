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


[video]

# Disable keyboard log:
# (Please see also 'Keyboard input masking level' in 'session_log' section of 'Connection Policy'.)
#   0x0: none
#   0x1: disable keyboard log in syslog
#   0x2: disable keyboard log in recorded sessions
# Note: values can be added (disable all: 0x1 + 0x2 = 0x3)
#_advanced
disable_keyboard_log = integer(min=0, max=3, default=1)

# Disable clipboard log:
#   0x0: none
#   0x1: disable clipboard log in syslog
#   0x2: disable clipboard log in recorded sessions
#   0x4: disable clipboard log in session meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
disable_clipboard_log = integer(min=0, max=7, default=1)

# Disable (redirected) file system log:
#   0x0: none
#   0x1: disable (redirected) file system log in syslog
#   0x2: disable (redirected) file system log in recorded sessions
#   0x4: disable (redirected) file system log in session meta
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
disable_file_system_log = integer(min=0, max=7, default=1)

[rdp]

# NLA authentication in secondary target.
enable_nla = boolean(default=True)

# If enabled, NLA authentication will try Kerberos before NTLM.
# (if enable_nla is disabled, this value is ignored).
enable_kerberos = boolean(default=False)

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

# Connect to the server in Restricted Admin mode.
# This mode must be supported by the server (available from Windows Server 2012 R2), otherwise, connection will fail.
# NLA must be enabled.
enable_restricted_admin_mode = boolean(default=False)

# Delay before showing disconnect message after the last RemoteApp window is closed.
# (is in millisecond)
#_advanced
remote_programs_disconnect_message_delay = integer(min=3000, max=120000, default=3000)

# Use Session Probe to launch Remote Program as much as possible.
use_session_probe_to_launch_remote_program = boolean(default=True)

[session_log]

# Keyboard Input Masking Level:
#   0: keyboard input are not masked
#   1: only passwords are masked
#   2: passwords and unidentified texts are masked. See also childless_window_as_unidentified_input_field and windows_of_these_applications_as_unidentified_input_field in session_probe section
#   3: keyboard inputs are not logged
keyboard_input_masking_level = option(0, 1, 2, 3, default=2)

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
server_access_allowed_message = integer(min=0, max=7, default=1)

# Warn that new server certificate file was created.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
server_cert_create_message = integer(min=0, max=7, default=1)

# Warn that server certificate file was successfully checked.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
server_cert_success_message = integer(min=0, max=7, default=1)

# Warn that server certificate file checking failed.
#   0x0: nobody
#   0x1: message sent to syslog
#   0x2: User notified (through proxy interface)
#   0x4: admin notified (wab notification)
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
server_cert_failure_message = integer(min=0, max=7, default=1)

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

#_advanced
ignore_ui_less_processes_during_end_of_session_check = boolean(default=True)

#_advanced
childless_window_as_unidentified_input_field = boolean(default=True)

#   0x000: none
#   0x001: Java Access Bridge
#   0x002: MS Active Accessbility
#   0x004: MS UI Automation
#   0x008: Reserved (do not use)
#   0x010: Inspect Edge location URL
#   0x020: Inspect Chrome Address/Search bar
#   0x040: Inspect Firefox Address/Search bar
#   0x080: Monitor Internet Explorer event
# Note: values can be added (enable all: 0x001 + 0x002 + 0x004 + 0x008 + 0x010 + 0x020 + 0x040 + 0x080 = 0x0ff)
#_advanced
#_hex
disabled_features = integer(min=0, max=255, default=96)

# If enabled, disconnected session can be recovered by a different primary user.
public_session = boolean(default=False)

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

[session]

# No traffic auto disconnection.
# If value is 0, global value (session_timeout) is used.
# (is in second)
inactivity_timeout = integer(min=0, default=0)

