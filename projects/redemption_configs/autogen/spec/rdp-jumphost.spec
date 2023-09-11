[general]

# Secondary login Transformation rule
# ${LOGIN} will be replaced by login
# ${DOMAIN} (optional) will be replaced by domain if it exists.
# Empty value means no transformation rule.
transformation_rule = string(default="")

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
vault_transformation_rule = string(default="")


[session]

# No automatic disconnection due to inactivity, timer is set on target session.
# If value is between 1 and 30, then 30 is used.
# If value is set to 0, then value set in "Base inactivity timeout" (in "RDP Proxy" configuration option) is used.<br/>
# (in seconds)
inactivity_timeout = integer(min=0, default=0)

[all_target_mod]

# This parameter allows you to specify max timeout before a TCP connection is aborted. If the option value is specified as 0, TCP will use the system default.<br/>
# (in milliseconds)
#_advanced
tcp_user_timeout = integer(min=0, max=3600000, default=0)

[rdp]

# This option should only be used if the server or client is showing graphical issues.
# In general, disabling RDP orders has a negative impact on performance.<br/>
# Disables supported drawing orders:
# &nbsp; &nbsp;    0: DstBlt
# &nbsp; &nbsp;    1: PatBlt
# &nbsp; &nbsp;    2: ScrBlt
# &nbsp; &nbsp;    3: MemBlt
# &nbsp; &nbsp;    4: Mem3Blt
# &nbsp; &nbsp;    9: LineTo
# &nbsp; &nbsp;   15: MultiDstBlt
# &nbsp; &nbsp;   16: MultiPatBlt
# &nbsp; &nbsp;   17: MultiScrBlt
# &nbsp; &nbsp;   18: MultiOpaqueRect
# &nbsp; &nbsp;   22: Polyline
# &nbsp; &nbsp;   25: EllipseSC
# &nbsp; &nbsp;   27: GlyphIndex<br/>
# (values are comma-separated)
#_advanced
disabled_orders = string(default="27")

# Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_min_level = integer(min=0, default=0)

# Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_max_level = integer(min=0, default=0)

# TLSv1.2 additional ciphers supported by client, default is empty to apply system-wide configuration (SSL security level 2), ALL for support of all ciphers to ensure highest compatibility with target servers.
cipher_string = string(default="ALL")

# Show in the logs the common cipher list supported by client and server
# ⚠ Only for debug purposes
#_advanced
show_common_cipher_list = boolean(default=False)

# List of (comma-separated) enabled dynamic virtual channel. If character '*' is used as a name then enables everything.
# An explicit name in 'Allowed dynamic channels' and 'Denied dynamic channels' will have higher priority than '*'.
#_advanced
allowed_dynamic_channels = string(default="*")

# List of (comma-separated) disabled dynamic virtual channel. If character '*' is used as a name then disables everything.
# An explicit name in 'Allowed dynamic channels' and 'Denied dynamic channels' will have higher priority than '*'.
#_advanced
denied_dynamic_channels = string(default="")

# Adds RDPDR channel metadata to session logs. Disabling this option makes shared disks more responsive, but metadata will no longer be collected.if at least one authorization of RDPDR is missing (Printer, ComPort, SmartCard, Drive), then this option is considered enabled.
#_advanced
enable_rdpdr_data_analysis = boolean(default=True)

# NLA will be disabled.
# Target must be set for interactive login, otherwise server connection may not be guaranteed.
# Smartcard device must be available on client desktop.
# Smartcard redirection (Proxy option RDP_SMARTCARD) must be enabled on service.
force_smartcard_authentication = boolean(default=False)

# Enable target connection on ipv6
enable_ipv6 = boolean(default=True)

# Allows the proxy to automatically reconnect to secondary target when a network error occurs.
# The server must support reconnection cookie.
#_advanced
auto_reconnection_on_losing_target_link = boolean(default=False)

# Forward the build number advertised by the client to the server. If forwarding is disabled a default (static) build number will be sent to the server.
#_advanced
forward_client_build_number = boolean(default=True)

[server_cert]

# Warn if check allow connexion to server.
# &nbsp; &nbsp;   0x0: nobody
# &nbsp; &nbsp;   0x1: message sent to syslog
# &nbsp; &nbsp;   0x2: User notified (through proxy interface)
# &nbsp; &nbsp;   0x4: admin notified (Bastion notification)<br/>
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_access_allowed_message = integer(min=0, max=7, default=1)

# Warn that new server certificate file was created.
# &nbsp; &nbsp;   0x0: nobody
# &nbsp; &nbsp;   0x1: message sent to syslog
# &nbsp; &nbsp;   0x2: User notified (through proxy interface)
# &nbsp; &nbsp;   0x4: admin notified (Bastion notification)<br/>
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_cert_create_message = integer(min=0, max=7, default=1)

# Warn that server certificate file was successfully checked.
# &nbsp; &nbsp;   0x0: nobody
# &nbsp; &nbsp;   0x1: message sent to syslog
# &nbsp; &nbsp;   0x2: User notified (through proxy interface)
# &nbsp; &nbsp;   0x4: admin notified (Bastion notification)<br/>
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_cert_success_message = integer(min=0, max=7, default=1)

# Warn that server certificate file checking failed.
# &nbsp; &nbsp;   0x0: nobody
# &nbsp; &nbsp;   0x1: message sent to syslog
# &nbsp; &nbsp;   0x2: User notified (through proxy interface)
# &nbsp; &nbsp;   0x4: admin notified (Bastion notification)<br/>
# Note: values can be added (enable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
server_cert_failure_message = integer(min=0, max=7, default=1)

[session_log]

# Classification of input data is performed using Session Probe.
# Without Session Probe, all the texts entered are considered unidentified.
# &nbsp; &nbsp;   0: keyboard input are not masked
# &nbsp; &nbsp;   1: only passwords are masked
# &nbsp; &nbsp;   2: passwords and unidentified texts are masked
# &nbsp; &nbsp;   3: keyboard inputs are not logged
keyboard_input_masking_level = option(0, 1, 2, 3, default=2)

[video]

# Disable keyboard log:
# (Please see also "Keyboard input masking level" in "session_log" section of "Connection Policy".)
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: disable keyboard log in session log
# &nbsp; &nbsp;   0x2: disable keyboard log in recorded sessions<br/>
# Note: values can be added (disable all: 0x1 + 0x2 = 0x3)
#_advanced
#_hex
disable_keyboard_log = integer(min=0, max=3, default=0)

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

# Log the files and clipboard texts that are verified and accepted. By default, only those rejected are logged.
#_advanced
log_if_accepted = boolean(default=True)

# ⚠ This value affects the RAM used by the session.<br/>
# If option Block invalid file (up or down) is enabled, automatically reject file with greater filesize.<br/>
# (in megabytes)
#_advanced
max_file_size_rejected = integer(min=0, default=256)

[file_storage]

# Enable storage of transferred files (via RDP Clipboard).
# ⚠ Saving files can take up a lot of disk space
# &nbsp; &nbsp;   never: Never store transferred files.
# &nbsp; &nbsp;   always: Always store transferred files.
# &nbsp; &nbsp;   on_invalid_verification: Transferred files are stored only if file verification is invalid. File verification by ICAP service must be enabled (in section file_verification).
store_file = option('never', 'always', 'on_invalid_verification', default="never")

