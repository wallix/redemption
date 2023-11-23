#include "config_variant.hpp"

R"gen_config_ini(## Python spec file for RDP proxy.


[globals]

# Port of RDP Proxy service.<br/>
# Service will be automatically restarted and active sessions will be disconnected.
# The port set in this field must not be already used, otherwise the service will not run.
# Changing the port number will prevent WALLIX Access Manager from working properly.
#_iptables
#_advanced
#_logged
port = integer(min=0, default=3389)

# Time out during RDP connection initialization.
# Increase the value if connection between workstations and Bastion can be slow.<br/>
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

# The transparent mode allows to intercept network traffic for a target even when the user specifies the target's address directly, instead of using the proxy address.
#_iptables
enable_transparent_mode = boolean(default=False)

# Displays a reminder box at the top of the session when a session is limited in time (timeframe or approval).
# The reminder is displayed successively 30min, 10min, 5min and 1min before the session is closed.
#_advanced
enable_end_time_warning_osd = boolean(default=True)

# Allow to show target device name with F12 during the session
#_advanced
enable_osd_display_remote_target = boolean(default=True)

# Show in session the target username when F12 is pressed.
# This option needs "Enable Osd Display Remote Target".
show_target_user_in_f12_message = boolean(default=False)

# Prevent Remote Desktop session timeouts due to idle TCP sessions by sending periodically keep alive packet to client.
# !!!May cause FreeRDP-based client to CRASH!!!
# Set to 0 to disable this feature.<br/>
# (in milliseconds)
rdp_keepalive_connection_interval = integer(min=0, default=0)

# ⚠ Service need to be manually restarted to take changes into account<br/>
# Enable primary connection on ipv6.
#_advanced
enable_ipv6 = boolean(default=True)

[client]

# If true, ignore password provided by RDP client, user need do login manually.
#_advanced
ignore_logon_password = boolean(default=False)

# Sends the client screen count to the server. Not supported for VNC targets.
# Uncheck to disable multiple monitor.
allow_using_multiple_monitors = boolean(default=True)

# Sends Scale &amp; Layout configuration to the server.
# On Windows 11, this corresponds to options Sclale, Display Resolution and Display Orientation of Settings > System > Display.
# ⚠ Title bar detection via OCR will no longer work.
allow_scale_factor = boolean(default=False)

# Fallback to RDP Legacy Encryption if client does not support TLS.
# ⚠ Enabling this option is a security risk.
tls_fallback_legacy = boolean(default=False)

# Minimal incoming TLS level 0=TLSv1, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
# ⚠ Lower this value only for compatibility reasons.
tls_min_level = integer(min=0, default=2)

# Maximal incoming TLS level 0=no restriction, 1=TLSv1.1, 2=TLSv1.2, 3=TLSv1.3
# ⚠ Change this value only for compatibility reasons.
tls_max_level = integer(min=0, default=0)

# Show in the logs the common cipher list supported by client and server
# ⚠ Only for debug purposes
#_advanced
show_common_cipher_list = boolean(default=False)

# [Not configured]: Compatible with more RDP clients (less secure)
# HIGH:!ADH:!3DES: Compatible only with MS Windows 7 client or more recent (moderately secure)
# HIGH:!ADH:!3DES:!SHA: Compatible only with MS Server Windows 2008 R2 client or more recent (more secure)
ssl_cipher_list = string(default="HIGH:!ADH:!3DES:!SHA")

# Specifies the highest RDP compression support available on client connection session.
# &nbsp; &nbsp;   0: The RDP bulk compression is disabled
# &nbsp; &nbsp;   1: RDP 4.0 bulk compression
# &nbsp; &nbsp;   2: RDP 5.0 bulk compression
# &nbsp; &nbsp;   3: RDP 6.0 bulk compression
# &nbsp; &nbsp;   4: RDP 6.1 bulk compression
#_advanced
rdp_compression = option(0, 1, 2, 3, 4, default=4)

# Specifies the maximum color resolution (color depth) for client connection session:
# &nbsp; &nbsp;   8: 8-bit
# &nbsp; &nbsp;   15: 15-bit 555 RGB mask
# &nbsp; &nbsp;   16: 16-bit 565 RGB mask
# &nbsp; &nbsp;   24: 24-bit RGB mask
# &nbsp; &nbsp;   32: 32-bit RGB mask + alpha
#_advanced
max_color_depth = option(8, 15, 16, 24, 32, default=24)

# Persistent Disk Bitmap Cache on the primary connection side. If supported by the RDP client, the size of image caches will be increased
#_advanced
persistent_disk_bitmap_cache = boolean(default=True)

# If enabled, the contents of Persistent Bitmap Caches are stored on disk for reusing them later (this value is ignored if Persistent Disk Bitmap Cache is disabled).
#_advanced
persist_bitmap_cache_on_disk = boolean(default=False)

# Enable Bitmap Compression when supported by the RDP client.
# Disable this option will increase network bandwith usage.
#_advanced
bitmap_compression = boolean(default=True)

# Allows the client to request the server to stop graphical updates. This can occur when the RDP client window is minimized to reduce bandwidth.
# ⚠ If changes occur on the target, they will not be visible in the recordings either.
enable_suppress_output = boolean(default=True)

# Same effect as "Transform glyph to bitmap", but only for RDP client on iOS platform.
bogus_ios_glyph_support_level = boolean(default=True)

# Some RDP clients advertise glyph support, but this does not work properly with the RDP proxy. This option replaces glyph orders with bitmap orders.
#_advanced
transform_glyph_to_bitmap = boolean(default=False)

# Enables display of message informing user that his/her session is being audited.
enable_osd_4_eyes = boolean(default=True)

# Enable RemoteFx on client connection.
# Needs - "Max Color Depth" option set to 32 (32-bit RGB mask + alpha)
# &nbsp; &nbsp;       - "Enable RemoteFX" option enabled in target connection policy
#_advanced
enable_remotefx = boolean(default=True)

# This option should only be used if the server or client is showing graphical issues.
# In general, disabling RDP orders has a negative impact on performance.<br/>
# Drawing orders that can be disabled:
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

# It specifies a list of (comma-separated) RDP server desktop features to enable or disable in the session (with the goal of optimizing bandwidth usage).<br/>
# If a feature is preceded by a "-" sign, it is disabled; if it is preceded by a "+" sign or no sign, it is enabled. Unconfigured features can be controlled by the RPD client.<br/>
# Available features:
# &nbsp; &nbsp;   - wallpaper
# &nbsp; &nbsp;   - full_window_drag
# &nbsp; &nbsp;   - menu_animations
# &nbsp; &nbsp;   - theme
# &nbsp; &nbsp;   - mouse_cursor_shadows
# &nbsp; &nbsp;   - cursor_blinking
# &nbsp; &nbsp;   - font_smoothing
# &nbsp; &nbsp;   - desktop_composition
#_advanced
force_performance_flags = string(default="-mouse_cursor_shadows,-theme,-menu_animations")

# If enabled, avoid automatically font smoothing in recorded session.
# This allows OCR (when session probe is disabled) to better detect window titles.
# If disabled, allows font smoothing in recorded session, but OCR will not work when Session is disabled.
# In this case, windows titles will not be detected.
#_advanced
auto_adjust_performance_flags = boolean(default=True)

# Specifies the highest RDP compression support available on server connection.
# &nbsp; &nbsp;   0: The RDP bulk compression is disabled
# &nbsp; &nbsp;   1: RDP 4.0 bulk compression
# &nbsp; &nbsp;   2: RDP 5.0 bulk compression
# &nbsp; &nbsp;   3: RDP 6.0 bulk compression
# &nbsp; &nbsp;   4: RDP 6.1 bulk compression
#_advanced
rdp_compression = option(0, 1, 2, 3, 4, default=4)

#_advanced
disconnect_on_logon_user_change = boolean(default=False)

# The maximum time that the proxy will wait while attempting to logon to an RDP session.
# Value 0 is equivalent to 15 seconds.<br/>
# (in seconds)
#_advanced
open_session_timeout = integer(min=0, default=0)

# Persistent Disk Bitmap Cache on the secondary connection side. If supported by the RDP server, the size of image caches will be increased
#_advanced
persistent_disk_bitmap_cache = boolean(default=True)

# Support of Cache Waiting List (this value is ignored if Persistent Disk Bitmap Cache is disabled).
#_advanced
cache_waiting_list = boolean(default=True)

# If enabled, the contents of Persistent Bitmap Caches are stored on disk for reusing them later (this value is ignored if Persistent Disk Bitmap Cache is disabled).
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

# Workaround option to support partial clipboard initialization performed by some versions of FreeRDP.
#_advanced
bogus_freerdp_clipboard = boolean(default=False)

# Workaround option to disable shared disk for RDP client on iOS platform only.
#_advanced
bogus_ios_rdpdr_virtual_channel = boolean(default=True)

# Workaround option to fix some drawing issues with Windows Server 2012.
# Can be disabled when none of the targets are Windows Server 2012.
#_advanced
bogus_refresh_rect = boolean(default=True)

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

# Some events such as 'Preferred DropEffect' have no particular meaning. This option allows you to exclude these types of events from the logs.
#_advanced
log_only_relevant_clipboard_activities = boolean(default=True)

# Force to split target domain and username with '@' separator.
#_advanced
split_domain = boolean(default=False)

# Enables Session Shadowing Support.
# Session probe must be enabled on target connection policy.
# Target server must support "Remote Desktop Shadowing" feature.
# When enabled, users can share their RDP sessions with auditors who request it.
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

# Check this option to enable the upload clipboard (from client to server).
# This only support text data clipboard (not files).
clipboard_up = boolean(default=False)

# Check this option to enable the clipboard download (from server to client).This only support text data clipboard (not files).
clipboard_down = boolean(default=False)

# Sets additional graphics encoding types that will be negotiated with the VNC server:
# &nbsp; &nbsp;   2: RRE
# &nbsp; &nbsp;   5: HEXTILE
# &nbsp; &nbsp;   16: ZRLE<br/>
# (values are comma-separated)
#_advanced
encodings = string(default="")

# VNC server clipboard text data encoding type.
#_advanced
server_clipboard_encoding_type = option('utf-8', 'latin1', default="latin1")

# The RDP clipboard is based on a token that indicates who owns data between server and client. However, some RDP clients, such as Freerpd, always appropriate this token. This conflicts with VNC, which also appropriates this token, causing clipboard data to be sent in loops.
# This option indicates the strategy to adopt in such situations.
# &nbsp; &nbsp;   0: delayed: Clipboard processing is deferred and, if necessary, the token is left with the client.
# &nbsp; &nbsp;   1: duplicated: When 2 identical requests are received, the second is ignored. This can block clipboard data reception until a clipboard event is triggered on the server when the client clipboard is blocked, and vice versa.
# &nbsp; &nbsp;   2: continued: No special processing is done, the proxy always responds immediately.
#_advanced
bogus_clipboard_infinite_loop = option(0, 1, 2, default=0)

[ocr]

# Selects the OCR (Optical Character Recognition) version used to detect title bars when Session Probe is not running.
# Version 1 is a bit faster, but has a higher failure rate in character recognition.
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

[capture]

# Specifies the type of data to be captured:
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: png
# &nbsp; &nbsp;   0x2: wrm: Session recording file.
# &nbsp; &nbsp;   0x8: ocr<br/>
# Note: values can be added (enable all: 0x1 + 0x2 + 0x8 = 0xb)
#_advanced
#_hex
capture_flags = integer(min=0, max=15, default=11)

# Disable clipboard log:
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: disable clipboard log in recorded sessions
# &nbsp; &nbsp;   0x2: disable clipboard log in recorded meta<br/>
# Note: values can be added (disable all: 0x1 + 0x2 = 0x3)
#_advanced
#_hex
disable_clipboard_log = integer(min=0, max=3, default=0)

# Disable (redirected) file system log:
# &nbsp; &nbsp;   0x0: none
# &nbsp; &nbsp;   0x1: disable (redirected) file system log in recorded sessions
# &nbsp; &nbsp;   0x2: disable (redirected) file system log in recorded meta<br/>
# Note: values can be added (disable all: 0x1 + 0x2 = 0x3)
#_advanced
#_hex
disable_file_system_log = integer(min=0, max=3, default=0)

# Time between 2 wrm recording file.
# ⚠ A value that is too small increases the disk space required for recordings.<br/>
# (in seconds)
#_advanced
wrm_break_interval = integer(min=0, default=600)

# The method by which the proxy RDP establishes criteria on which to chosse a color depth for Session recording file (wrm):
# &nbsp; &nbsp;   0: 24-bit
# &nbsp; &nbsp;   1: 16-bit
#_advanced
wrm_color_depth_selection_strategy = option(0, 1, default=1)

# The compression method of Session recording file (wrm):
# &nbsp; &nbsp;   0: no compression
# &nbsp; &nbsp;   1: GZip: Files are better compressed, but this takes more time and CPU load
# &nbsp; &nbsp;   2: Snappy: Faster than GZip, but files are less compressed
#_advanced
wrm_compression_algorithm = option(0, 1, 2, default=1)

[audit]

# Show keyboard input event in meta file
# (Please see also "Keyboard input masking level" in "session_log".)
#_advanced
enable_keyboard_log = boolean(default=True)

# Maximum number of images per second for video generation.
# A higher value will produce smoother videos, but the file weight is higher and the generation time longer.
#_advanced
#_display_name=Frame rate
framerate = integer(min=1, max=120, default=5)

# FFmpeg options for video codec. See https://trac.ffmpeg.org/wiki/Encode/H.264
# ⚠ Some browsers and video decoders don't support crf=0
#_advanced
#_display_name=FFmpeg options
ffmpeg_options = string(default="crf=35 preset=superfast")

# In the generated video of the session record traces, remove the top left banner with the timestamp.
# Can slightly speed up the video generation.
#_advanced
notimestamp = boolean(default=False)

# &nbsp; &nbsp;   0: disable: When replaying the session video, the content of the RDP viewer matches the size of the client's desktop
# &nbsp; &nbsp;   1: v1: When replaying the session video, the content of the RDP viewer is restricted to the greatest area covered by the application during session
# &nbsp; &nbsp;   2: v2: When replaying the session video, the content of the RDP viewer is fully covered by the size of the greatest application window during session
smart_video_cropping = option(0, 1, 2, default=2)

# Check this option will allow to play a video with corrupted Bitmap Update.
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

# Activate TLS on ICAP server connection
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

# Activate TLS on ICAP server connection
tls = boolean(default=False)

# Send X Context (Client-IP, Server-IP, Authenticated-User) to ICAP server
#_advanced
enable_x_context = boolean(default=True)

# Filename sent to ICAP as percent encoding
#_advanced
filename_percent_encoding = boolean(default=False)

[internal_mod]

# Enable target edit field in login page. This target edit field allows to enter the target and the login separately.
#_advanced
enable_target_field = boolean(default=True)

# List of keyboard layouts available by the internal pages button located at bottom left of some internal pages (login, selector, etc).
# Possible values: bg-BG, bg-BG.latin, bs-Cy, bépo, cs-CZ, cs-CZ.programmers, cs-CZ.qwerty, cy-GB, da-DK, de-CH, de-DE, de-DE.ibm, el-GR, el-GR.220, el-GR.220_latin, el-GR.319, el-GR.319_latin, el-GR.latin, el-GR.polytonic, en-CA.fr, en-CA.multilingual, en-GB, en-IE, en-IE.irish, en-US, en-US.dvorak, en-US.dvorak_left, en-US.dvorak_right, en-US.international, es-ES, es-ES.variation, es-MX, et-EE, fi-FI.finnish, fo-FO, fr-BE, fr-BE.fr, fr-CA, fr-CH, fr-FR, fr-FR.standard, hr-HR, hu-HU, is-IS, it-IT, it-IT.142, iu-La, kk-KZ, ky-KG, lb-LU, lt-LT, lt-LT.ibm, lv-LV, lv-LV.qwerty, mi-NZ, mk-MK, mn-MN, mt-MT.47, mt-MT.48, nb-NO, nl-BE, nl-NL, pl-PL, pl-PL.programmers, pt-BR.abnt, pt-BR.abnt2, pt-PT, ro-RO, ru-RU, ru-RU.typewriter, se-NO, se-NO.ext_norway, se-SE, se-SE, se-SE.ext_finland_sweden, sk-SK, sk-SK.qwerty, sl-SI, sr-Cy, sr-La, sv-SE, tr-TR.f, tr-TR.q, tt-RU, uk-UA, uz-Cy<br/>
# (values are comma-separated)
#_advanced
keyboard_layout_proposals = string(default="en-US, fr-FR, de-DE, ru-RU")

# Show close screen.
# This displays errors related to the secondary connection then closes automatically after a timeout specified by "Close Timeout" or on user request.
enable_close_box = boolean(default=True)

# Specifies the time to spend on the close box of proxy RDP before closing client window.
# ⚠ Value 0 deactivates the timer and the connection remains open until the client disconnects.<br/>
# (in seconds)
#_advanced
close_box_timeout = integer(min=0, default=600)

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
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
bgcolor = string(default="#081F60")

# Foreground color for window, label and button<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
fgcolor = string(default="#FFFFFF")

# Separator line color used with some widgets<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
separator_color = string(default="#CFD5EB")

# Background color used by buttons when they have focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
focus_color = string(default="#004D9C")

# Text color for error messages. For example, an authentication error in the login<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
error_color = string(default="#FFFF00")

# Background color for editing field<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
edit_bgcolor = string(default="#FFFFFF")

# Foreground color for editing field<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
edit_fgcolor = string(default="#000000")

# Outline color for editing field that has focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
edit_focus_color = string(default="#004D9C")

# Background color for tooltip<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
tooltip_bgcolor = string(default="#000000")

# Foreground color for tooltip<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
tooltip_fgcolor = string(default="#FFFF9F")

# Border color for tooltip<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
tooltip_border_color = string(default="#000000")

# Background color for even rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_line1_bgcolor = string(default="#E9ECF6")

# Foreground color for even rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_line1_fgcolor = string(default="#000000")

# Background color for odd rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_line2_bgcolor = string(default="#CFD5EB")

# Foreground color for odd rows in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_line2_fgcolor = string(default="#000000")

# Background color for the row that has focus in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_focus_bgcolor = string(default="#004D9C")

# Foreground color for the row that has focus in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_focus_fgcolor = string(default="#FFFFFF")

# Background color for the row that is selected in the selector widget but does not have focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_selected_bgcolor = string(default="#4472C4")

# Foreground color for the row that is selected in the selector widget but does not have focus<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_selected_fgcolor = string(default="#FFFFFF")

# Background color for name of filter fields in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
selector_label_bgcolor = string(default="#4472C4")

# Foreground color for name of filter fields in the selector widget<br/>
# (in rgb format: hexadecimal (0x21AF21), #rgb (#2fa), #rrggbb (#22ffaa) or a <a href="https://en.wikipedia.org/wiki/Web_colors#Extended_colors">named color</a> case insensitive (red, skyBlue, etc))
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
#_display_name=FFmpeg
ffmpeg = integer(min=0, default=0)

# Log unknown members or sections
#_advanced
config = boolean(default=True)

# List of client probe IP addresses (ex: ip1,ip2,etc) to prevent some continuous logs<br/>
# (values are comma-separated)
#_advanced
probe_client_addresses = string(default="")

)gen_config_ini"
