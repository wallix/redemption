/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/


#pragma once

// set by command line
#ifdef IN_IDE_PARSER
# define RECORD_PATH "/var/rdpproxy/recorded"
# define RECORD_TMP_PATH "/var/rdpproxy/tmp"
# define FLV_PATH "/tmp"
# define OCR_PATH "/tmp"
# define PNG_PATH "/tmp"
# define WRM_PATH "/tmp"
// still used in front and recorder
# define HASH_PATH "/tmp/hash"
# define LICENSE_PATH "/var/certs/redemption"
// still used in sockettransport
# define CERTIF_PATH "/etc/rdpproxy/certificate"
# define PERSISTENT_PATH "/var/lib/redemption/cache"
# define DRIVE_REDIRECTION_PATH "/var/rdpproxy/drive_redirection"
# define PID_PATH "/var/run"

// still used in front, checkfiles, session, transparent, some internal mods
# define SHARE_PATH "/usr/local/share/rdpproxy"
// still used in main and sockettransport
# define CFG_PATH "/etc/rdpproxy"
// # define CFG_THEMES_PATH "/etc/rdpproxy/themes"
#endif


// TODO move these into configuration
// #define LOGIN_LOGO24 "ad24b.png"
#define LOGIN_WAB_BLUE "wablogoblue.png"
#define LOGO_PNG "logo.png"
#define LOGO_BMP "logo.bmp"
// #define HELP_ICON "helpicon.png"
// #define CURSOR0 "cursor0.cur"
// #define CURSOR1 "cursor1.cur"
// #define FONT1 "sans-10.fv1"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.png"
#define LOCKFILE "rdpproxy.pid"


// #define RSAKEYS_INI "rsakeys.ini"
#define RDPPROXY_INI "rdpproxy.ini"
#define THEME_INI    "theme.ini"

#define RDPPROXY_CRT "rdpproxy.crt"
#define RDPPROXY_KEY "rdpproxy.key"
// #define DH1024_PEM "dh1024.pem"
#define DH2048_PEM "dh2048.pem"
#define DH_PEM DH2048_PEM
