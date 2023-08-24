#include "config_variant.hpp"

R"gen_config_ini(## Python spec file for RDP proxy.


[globals]

# ⚠ IP tables rules are reloaded and active sessions will be disconnected.<br/>
# The port set in this field must not be already used, otherwise the service will not run.
# Changing the port number will prevent WALLIX Access Manager from working properly.
#_iptables
#_advanced
#_logged
port = integer(min=0, default=3389)

#_advanced
encryptionLevel = option('low', 'medium', 'high', default="low")

# Time out during RDP handshake stage.<br/>
# (in seconds)
handshake_timeout = integer(min=0, default=10)

# No automatic disconnection due to inactivity, timer is set on primary authentication.
# If value is between 1 and 30, then 30 is used.
# If value is set to 0, then inactivity timeout value is unlimited.<br/>
# (in seconds)
base_inactivity_timeout = integer(min=0, default=900)

# Specifies the time to spend on the login screen of proxy RDP before closing client window (0 to desactivate).<br/>
# (in seconds)
#_advanced
authentication_timeout = integer(min=0, default=120)

# ⚠ IP tables rules are reloaded and active sessions will be disconnected.<br/>
# Allow Transparent mode.
#_iptables
enable_transparent_mode = boolean(default=False)

# Support of Bitmap Update.
#_advanced
enable_bitmap_update = boolean(default=True)

# Show close screen.
# This displays errors related to the secondary connection then closes automatically after a timeout specified by "close_timeout" or on user request.
enable_close_box = boolean(default=True)

# Specifies the time to spend on the close box of proxy RDP before closing client window.
# ⚠ Value 0 deactivates the timer and the connection remains open until the client disconnects.<br/>
# (in seconds)
#_advanced
close_timeout = integer(min=0, default=600)

# Displays a reminder box at the top of the session when a session duration is configured.
# The reminder is displayed successively 30min, 10min, 5min and 1min before the session is closed.
#_advanced
enable_osd = boolean(default=True)

# Show target address with F12.
#_advanced
enable_osd_display_remote_target = boolean(default=True)

# Sends the client screen count to the server. Not supported in VNC.
allow_using_multiple_monitors = boolean(default=True)

# Sends Scale &amp; Layout configuration to the server.
# On Windows 11, this corresponds to options Sclale, Display Resolution and Display Orientation of Settings > System > Display.
# ⚠ Title bar detection via OCR will no longer work.
allow_scale_factor = boolean(default=False)

# Needed to refresh screen of Windows Server 2012.
#_advanced
bogus_refresh_rect = boolean(default=True)

# Enable support for pointers of size 96x96.
# ⚠ If this option is disabled and the application doesn't support smaller pointers, the pointer may not change and remain on the last active pointer. For example, the resize window pointer would remain visible rather than change to a 'normal' pointer.
#_advanced
large_pointer_support = boolean(default=True)

new_pointer_update_support = boolean(default=True)

# Allows the client to use unicode characters.
# This is useful for displaying characters that are not available on the keyboard layout used, such as some special characters or emojis.
unicode_keyboard_event_support = boolean(default=True)

#_advanced
experimental_enable_serializer_data_block_size_limit = boolean(default=False)

# Prevent Remote Desktop session timeouts due to idle TCP sessions by sending periodically keep alive packet to client.
# !!!May cause FreeRDP-based client to CRASH!!!
# Set to 0 to disable this feature.<br/>
# (in milliseconds)
rdp_keepalive_connection_interval = integer(min=0, default=0)

# ⚠ Service need to be manually restarted to take changes into account<br/>
# Enable primary connection on ipv6.
enable_ipv6 = boolean(default=True)

[client]

# If true, ignore password provided by RDP client, user need do login manually.
#_advanced
ignore_logon_password = boolean(default=False)

# It specifies a list of server desktop features to enable or disable in the session (with the goal of optimizing bandwidth usage).<br/>
# &nbsp; &nbsp;     0x1: Disable wallpaper
# &nbsp; &nbsp;     0x4: Disable menu animations
# &nbsp; &nbsp;     0x8: Disable theme
# &nbsp; &nbsp;    0x20: Disable mouse cursor shadows
# &nbsp; &nbsp;    0x40: Disable cursor blinking
# &nbsp; &nbsp;    0x80: Enable font smoothing
# &nbsp; &nbsp;   0x100: Enable Desktop Composition
#_advanced
#_hex
performance_flags_force_present = integer(min=0, default=40)

# Value that will be deleted by the proxy.
# See "Performance flags force present" above for available values.
#_advanced
#_hex
performance_flags_force_not_present = integer(min=0, default=0)

# If enabled, avoid automatically font smoothing in recorded session.
# This allows OCR (when session probe is disabled) to better detect window titles.
#_advanced
auto_adjust_performance_flags = boolean(default=True)

# Fallback to RDP Legacy Encryption if client does not support TLS.
tls_fallback_legacy = boolean(default=False)

tls_support = boolean(default=True)

# Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_min_level = integer(min=0, default=2)

# Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
tls_max_level = integer(min=0, default=0)

# Show in the logs the common cipher list supported by client and server
#_advanced
show_common_cipher_list = boolean(default=False)

# Needed for primary NTLM or Kerberos connections over NLA.
#_advanced
enable_nla = boolean(default=False)

# If enabled, ignore Ctrl+Alt+Del, Ctrl+Shift+Esc and Windows+Tab keyboard sequences.
#_advanced
disable_tsk_switch_shortcuts = boolean(default=False)

# Specifies the highest compression package support available on the front side
# &nbsp; &nbsp;   0: The RDP bulk compression is disabled
# &nbsp; &nbsp;   1: RDP 4.0 bulk compression
# &nbsp; &nbsp;   2: RDP 5.0 bulk compression
# &nbsp; &nbsp;   3: RDP 6.0 bulk compression
# &nbsp; &nbsp;   4: RDP 6.1 bulk compression
#_advanced
rdp_compression = option(0, 1, 2, 3, 4, default=4)

# Specifies the maximum color resolution (color depth) for client session:
# &nbsp; &nbsp;   8: 8-bit
# &nbsp; &nbsp;   15: 15-bit 555 RGB mask
# &nbsp; &nbsp;   16: 16-bit 565 RGB mask
# &nbsp; &nbsp;   24: 24-bit RGB mask
# &nbsp; &nbsp;   32: 32-bit RGB mask (24-bit RGB + alpha)
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

# Allows the client to request the server to stop graphical updates. This can occur when the RDP client window is minimized to reduce bandwidth.
# If changes occur on the target, they will not be visible in the recordings either.
enable_suppress_output = boolean(default=True)

# [Not configured]: Compatible with more RDP clients (less secure)
# HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)
# HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)
ssl_cipher_list = string(default="HIGH:!ADH:!3DES:!SHA")

# Show in session the target username when F12 is pressed
show_target_user_in_f12_message = boolean(default=False)

bogus_ios_glyph_support_level = boolean(default=True)

#_advanced
transform_glyph_to_bitmap = boolean(default=False)

# Enables display of message informing user that his/her session is being audited.
enable_osd_4_eyes = boolean(default=True)

# Enable front remoteFx
#_advanced
enable_remotefx = boolean(default=True)

# This option should only be used if the server or client is showing graphical issues, to make it easier to determine which RDP order is the cause.
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
disabled_orders = string(default="25")

[all_target_mod]

# The maximum time that the proxy will wait while attempting to connect to an target.<br/>
# (in milliseconds)
#_advanced
connection_establishment_timeout = integer(min=1000, max=10000, default=3000)

[remote_program]

# Allows resizing of a desktop session opened in a RemoteApp window.
# This happens when an RDP client opened in RemoteApp accesses a desktop target.
allow_resize_hosted_desktop = boolean(default=True)

[mod_rdp]

# Specifies the highest compression package support available on the front side
# &nbsp; &nbsp;   0: The RDP bulk compression is disabled
# &nbsp; &nbsp;   1: RDP 4.0 bulk compression
# &nbsp; &nbsp;   2: RDP 5.0 bulk compression
# &nbsp; &nbsp;   3: RDP 6.0 bulk compression
# &nbsp; &nbsp;   4: RDP 6.1 bulk compression
#_advanced
rdp_compression = option(0, 1, 2, 3, 4, default=4)

#_advanced
disconnect_on_logon_user_change = boolean(default=False)

# (in seconds)
#_advanced
open_session_timeout = integer(min=0, default=0)

# Persistent Disk Bitmap Cache on the mod side.
#_advanced
persistent_disk_bitmap_cache = boolean(default=True)

# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
#_advanced
cache_waiting_list = boolean(default=True)

# If enabled, the contents of Persistent Bitmap Caches are stored on disk.
#_advanced
persist_bitmap_cache_on_disk = boolean(default=False)

# Client Address to send to target (in InfoPacket)
# &nbsp; &nbsp;   0: Send 0.0.0.0
# &nbsp; &nbsp;   1: Send proxy client address or target connexion
# &nbsp; &nbsp;   2: Send user client address of front connexion
#_advanced
client_address_sent = option(0, 1, 2, default=0)

# Authentication channel used by Auto IT scripts. May be '*' to use default name. Keep empty to disable virtual channel.
auth_channel = string(max=7, default="*")

# Authentication channel used by other scripts. No default name. Keep empty to disable virtual channel.
checkout_channel = string(max=7, default="")

# Do not transmit client machine name to RDP server.
# If Per-Device licensing mode is configured on the RD host, this Bastion will consume a CAL for all of these connections to the RD host.
hide_client_name = boolean(default=True)

# Stores CALs issued by the terminal servers.
#_advanced
use_license_store = boolean(default=True)

bogus_ios_rdpdr_virtual_channel = boolean(default=True)

# Delay before automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.<br/>
# (in milliseconds)
#_advanced
remoteapp_bypass_legal_notice_delay = integer(min=0, default=0)

# Time limit to automatically bypass Windows's Legal Notice screen in RemoteApp mode.
# Set to 0 to disable this feature.<br/>
# (in milliseconds)
#_advanced
remoteapp_bypass_legal_notice_timeout = integer(min=0, default=20000)

#_advanced
log_only_relevant_clipboard_activities = boolean(default=True)

# Force to split target domain and username with '@' separator.
#_advanced
split_domain = boolean(default=False)

# Enables Session Shadowing Support.
#_advanced
session_shadowing_support = boolean(default=True)

[session_probe]

# If enabled, a string of random characters will be added to the name of the executable of Session Probe.
# The result could be: SesProbe-5420.exe
# Some other features automatically enable customization of the Session Probe executable name. Application Driver auto-deployment for example.
#_advanced
customize_executable_name = boolean(default=False)

# If enabled, the RDP Proxy accepts to perform the handshake several times during the same RDP session. Otherwise, any new handshake attempt will interrupt the current session with the display of an alert message.
#_advanced
#_display_name=Allow multiple handshakes
allow_multiple_handshake = boolean(default=False)

[mod_vnc]

# Enable or disable the clipboard from client (client to server).
clipboard_up = boolean(default=False)

# Enable or disable the clipboard from server (server to client).
clipboard_down = boolean(default=False)

# Sets additional graphics encoding types that will be negotiated with the VNC server:
# &nbsp; &nbsp;   2: RRE
# &nbsp; &nbsp;   5: HEXTILE
# &nbsp; &nbsp;   16: ZRLE<br/><br/>
# (values are comma-separated)
#_advanced
encodings = string(default="")

# VNC server clipboard data encoding type.
#_advanced
server_clipboard_encoding_type = option('utf-8', 'latin1', default="latin1")

# The RDP clipboard is based on a token that indicates who owns data between server and client. However, some RDP clients, such as Freerpd, always appropriate this token. This conflicts with VNC, which also appropriates this token, causing clipboard data to be sent in loops.
# This option indicates the strategy to adopt in such situations.
# &nbsp; &nbsp;   0: Clipboard processing is deferred and, if necessary, the token is left with the client.
# &nbsp; &nbsp;   1: When 2 identical requests are received, the second is ignored. This can block clipboard data reception until a clipboard event is triggered on the server when the client clipboard is blocked, and vice versa.
# &nbsp; &nbsp;   2: No special processing is done, the proxy always responds immediately.
#_advanced
bogus_clipboard_infinite_loop = option(0, 1, 2, default=0)

[session_log]

enable_session_log = boolean(default=True)

enable_arcsight_log = boolean(default=False)

[ocr]

# &nbsp; &nbsp;   1: v1
# &nbsp; &nbsp;   2: v2
version = option(1, 2, default=2)

# &nbsp; &nbsp;   latin: Recognizes Latin characters
# &nbsp; &nbsp;   cyrillic: Recognizes Latin and Cyrillic characters
locale = option('latin', 'cyrillic', default="latin")

# Time interval between 2 analyzes.
# Too low a value will affect session reactivity.<br/>
# (in 1/100 seconds)
#_advanced
interval = integer(min=0, default=100)

# Checks shape and color to determine if the text is on a title bar
#_advanced
on_title_bar_only = boolean(default=True)

# Expressed in percentage,
# &nbsp; &nbsp;   0   - all of characters need be recognized
# &nbsp; &nbsp;   100 - accept all results
#_advanced
max_unrecog_char_rate = integer(min=0, max=100, default=40)

[video]

# Specifies the type of data to be captured:
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: png
# &nbsp; &nbsp;   0x2: wrm: Session recording file. Also know as native video capture.
# &nbsp; &nbsp;   0x8: ocr<br/>
# Note: values can be added (enable all: 0x1 + 0x2 + 0x8 = 0xb)
#_advanced
#_hex
capture_flags = integer(min=0, max=15, default=11)

# Frame interval.<br/>
# (in 1/10 seconds)
#_advanced
png_interval = integer(min=0, default=10)

# Time between 2 wrm recording file.
# ⚠ A value that is too small increases the disk space required for recordings.<br/>
# (in seconds)
#_advanced
break_interval = integer(min=0, default=600)

# Number of png captures to keep.
#_advanced
png_limit = integer(min=0, default=5)

# Disable keyboard log:
# (Please see also "Keyboard input masking level" in "session_log".)
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: disable keyboard log in syslog
# &nbsp; &nbsp;   0x2: disable keyboard log in recorded sessions
# &nbsp; &nbsp;   0x4: disable keyboard log in recorded meta<br/>
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
disable_keyboard_log = integer(min=0, max=7, default=1)

# Disable clipboard log:
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: disable clipboard log in syslog
# &nbsp; &nbsp;   0x2: disable clipboard log in recorded sessions
# &nbsp; &nbsp;   0x4: disable clipboard log in recorded meta<br/>
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
disable_clipboard_log = integer(min=0, max=7, default=1)

# Disable (redirected) file system log:
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: disable (redirected) file system log in syslog
# &nbsp; &nbsp;   0x2: disable (redirected) file system log in recorded sessions
# &nbsp; &nbsp;   0x4: disable (redirected) file system log in recorded meta<br/>
# Note: values can be added (disable all: 0x1 + 0x2 + 0x4 = 0x7)
#_advanced
#_hex
disable_file_system_log = integer(min=0, max=7, default=1)

# The method by which the proxy RDP establishes criteria on which to chosse a color depth for native video capture:
# &nbsp; &nbsp;   0: 24-bit
# &nbsp; &nbsp;   1: 16-bit
#_advanced
wrm_color_depth_selection_strategy = option(0, 1, default=1)

# The compression method of wrm recording file:
# &nbsp; &nbsp;   0: no compression
# &nbsp; &nbsp;   1: GZip: Files are better compressed, but this takes more time and CPU load
# &nbsp; &nbsp;   2: Snappy: Faster than GZip, but files are less compressed
#_advanced
wrm_compression_algorithm = option(0, 1, 2, default=1)

# Maximum number of images per second for video generation.
# A higher value will produce smoother videos, but the file weight is higher and the generation time longer.
#_advanced
#_display_name=Frame rate
framerate = integer(min=1, max=120, default=5)

# FFmpeg options for video codec. See https://trac.ffmpeg.org/wiki/Encode/H.264
# ⚠ Some browsers and video decoders don't support crf=0
#_advanced
ffmpeg_options = string(default="crf=35 preset=superfast")

# Remove the top left banner that adds the date of the video
#_advanced
notimestamp = boolean(default=False)

# &nbsp; &nbsp;   0: Disabled. When replaying the session video, the content of the RDP viewer matches the size of the client's desktop
# &nbsp; &nbsp;   1: When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session
# &nbsp; &nbsp;   2: When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session
smart_video_cropping = option(0, 1, 2, default=2)

# Needed to play a video with corrupted Bitmap Update.
#_advanced
play_video_with_corrupted_bitmap = boolean(default=False)

# Allow real-time view (4 eyes) without session recording enabled in the authorization
allow_rt_without_recording = boolean(default=False)

[icap_server_down]

# Ip or fqdn of ICAP server
host = string(default="")

# Port of ICAP server
port = integer(min=0, default=1344)

# Service name on ICAP server
service_name = string(default="avscan")

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
host = string(default="")

# Port of ICAP server
port = integer(min=0, default=1344)

# Service name on ICAP server
service_name = string(default="avscan")

# ICAP server uses tls
tls = boolean(default=False)

# Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server
#_advanced
enable_x_context = boolean(default=True)

# Filename sent to ICAP as percent encoding
#_advanced
filename_percent_encoding = boolean(default=False)

[internal_mod]

# Enable target edit field in login page.
#_advanced
enable_target_field = boolean(default=True)

# List of keyboard layouts available by the internal pages button located at bottom left of some internal pages (login, selector, etc).
# Possible values: bg-BG, bg-BG.latin, bs-Cy, bépo, cs-CZ, cs-CZ.programmers, cs-CZ.qwerty, cy-GB, da-DK, de-CH, de-DE, de-DE.ibm, el-GR, el-GR.220, el-GR.220_latin, el-GR.319, el-GR.319_latin, el-GR.latin, el-GR.polytonic, en-CA.fr, en-CA.multilingual, en-GB, en-IE, en-IE.irish, en-US, en-US.dvorak, en-US.dvorak_left, en-US.dvorak_right, en-US.international, es-ES, es-ES.variation, es-MX, et-EE, fi-FI.finnish, fo-FO, fr-BE, fr-BE.fr, fr-CA, fr-CH, fr-FR, fr-FR.standard, hr-HR, hu-HU, is-IS, it-IT, it-IT.142, iu-La, kk-KZ, ky-KG, lb-LU, lt-LT, lt-LT.ibm, lv-LV, lv-LV.qwerty, mi-NZ, mk-MK, mn-MN, mt-MT.47, mt-MT.48, nb-NO, nl-BE, nl-NL, pl-PL, pl-PL.programmers, pt-BR.abnt, pt-BR.abnt2, pt-PT, ro-RO, ru-RU, ru-RU.typewriter, se-NO, se-NO.ext_norway, se-SE, se-SE, se-SE.ext_finland_sweden, sk-SK, sk-SK.qwerty, sl-SI, sr-Cy, sr-La, sv-SE, tr-TR.f, tr-TR.q, tt-RU, uk-UA, uz-Cy<br/>
# (values are comma-separated)
#_advanced
keyboard_layout_proposals = string(default="en-US, fr-FR, de-DE, ru-RU")

[translation]

# Language used on the login page. When the user logs in, their user preference language is used.
# &nbsp; &nbsp;   Auto: The language will be deduced according to the keyboard layout announced by the client
# &nbsp; &nbsp;   EN: 
# &nbsp; &nbsp;   FR: 
#_advanced
login_language = option('Auto', 'EN', 'FR', default="Auto")

[theme]

# Enable custom theme color configuration
enable_theme = boolean(default=False)

# Logo displayed when theme is enabled
#_image=/var/wab/images/rdp-oem-logo.png
logo = string(default=")gen_config_ini" << (REDEMPTION_CONFIG_THEME_LOGO) << R"gen_config_ini(")

# Background color for window, label and button<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
bgcolor = string(default="#081F60")

# Foreground color for window, label and button<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
fgcolor = string(default="#FFFFFF")

# Separator line color used with some widgets<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
separator_color = string(default="#CFD5EB")

# Background color used by buttons when they have focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
focus_color = string(default="#004D9C")

# Text color for error messages. For example, an authentication error in the login<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
error_color = string(default="#FFFF00")

# Background color for editing field<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
edit_bgcolor = string(default="#FFFFFF")

# Foreground color for editing field<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
edit_fgcolor = string(default="#000000")

# Outline color for editing field that has focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
edit_focus_color = string(default="#004D9C")

# Background color for tooltip<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
tooltip_bgcolor = string(default="#000000")

# Foreground color for tooltip<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
tooltip_fgcolor = string(default="#FFFF9F")

# Border color for tooltip<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
tooltip_border_color = string(default="#000000")

# Background color for even rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_line1_bgcolor = string(default="#E9ECF6")

# Foreground color for even rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_line1_fgcolor = string(default="#000000")

# Background color for odd rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_line2_bgcolor = string(default="#CFD5EB")

# Foreground color for odd rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_line2_fgcolor = string(default="#000000")

# Background color for the row that has focus in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_focus_bgcolor = string(default="#004D9C")

# Foreground color for the row that has focus in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_focus_fgcolor = string(default="#FFFFFF")

# Background color for the row that is selected in the selector widget but does not have focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_selected_bgcolor = string(default="#4472C4")

# Foreground color for the row that is selected in the selector widget but does not have focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_selected_fgcolor = string(default="#FFFFFF")

# Background color for name of filter fields in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_label_bgcolor = string(default="#4472C4")

# Foreground color for name of filter fields in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa))
selector_label_fgcolor = string(default="#FFFFFF")

[debug]

# - kbd / ocr when != 0<br/>
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

# - Log   = 0x01
# - Event = 0x02
# - Acl   = 0x04
# - Trace = 0x08
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
# - keymap          = 0x00008000<br/>
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
# - meta     = 0x0008
#_advanced
#_hex
sck_mod = integer(min=0, default=0)

# - basic    = 0x0001
# - dump     = 0x0002
# - watchdog = 0x0004
# - meta     = 0x0008
#_advanced
#_hex
sck_front = integer(min=0, default=0)

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

# Value passed to function av_log_set_level()
# See https://www.ffmpeg.org/doxygen/2.3/group__lavu__log__constants.html
#_advanced
#_hex
ffmpeg = integer(min=0, default=0)

# Log unknown members or sections
#_advanced
config = boolean(default=True)

# List of client probe IP addresses (ex: ip1,ip2,etc) to prevent some continuous logs<br/>
# (values are comma-separated)
#_advanced
probe_client_addresses = string(default="")

)gen_config_ini"
