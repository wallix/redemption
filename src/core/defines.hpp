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

#include "utils/log.hpp"

// TODO move SHARE_PATH to configuration (still used in front, checkfiles, session, transparent, some internal mods)
#if !defined(SHARE_PATH) || defined(IN_IDE_PARSER)
#define SHARE_PATH "/usr/local/share/rdpproxy"
#endif

// TODO move CFG_PATH to configuration (still used in main and sockettransport)
#if !defined(CFG_PATH)
#define CFG_PATH "/etc/rdpproxy"
#endif

// TODO move PID_PATH to configuration
#if !defined(PID_PATH)
#define PID_PATH "/var/run"
#endif

// TODO move LICENSE_PATH to configuration (still used in mod/rdp and mod/rdp_transparent)
#if !defined(LICENSE_PATH)
#define LICENSE_PATH "/var/certs/redemption"
#endif

#if !defined(FLV_PATH)
#define FLV_PATH "/tmp"
#endif

#if !defined(OCR_PATH)
#define OCR_PATH "/tmp"
#endif

#if !defined(PNG_PATH)
#define PNG_PATH "/tmp"
#endif

#if !defined(WRM_PATH)
#define WRM_PATH "/tmp"
#endif

// TODO move HASH_PATH to configuration (still used in front and recorder)
#if !defined(HASH_PATH)
#define HASH_PATH "/tmp/hash"
#endif

// TODO move CERTIF_PATH to configuration (still used in sockettransport)
#if !defined(CERTIF_PATH)
#define CERTIF_PATH "/etc/rdpproxy/certificate"
#endif

#if !defined(RECORD_PATH)
#define RECORD_PATH "/var/rdpproxy/recorded"
#endif

#if !defined(PERSISTENT_PATH)
#define PERSISTENT_PATH "/var/lib/redemption/cache"
#endif

#if !defined(DRIVE_REDIRECTION_PATH)
#define DRIVE_REDIRECTION_PATH "/var/rdpproxy/drive_redirection"
#endif

#if !defined(RECORD_TMP_PATH)
#define RECORD_TMP_PATH "/var/rdpproxy/tmp"
#endif

// TODO move these into configuration
#define LOGIN_LOGO24 "ad24b.png"
#define LOGIN_WAB_BLUE "wablogoblue.png"
#define LOGO_PNG "logo.png"
#define LOGO_BMP "logo.bmp"
#define HELP_ICON "helpicon.png"
#define CURSOR0 "cursor0.cur"
#define CURSOR1 "cursor1.cur"
#define FONT1 "sans-10.fv1"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.png"
#define LOCKFILE "rdpproxy.pid"


#define RSAKEYS_INI "rsakeys.ini"
#define RDPPROXY_INI "rdpproxy.ini"
#define THEME_INI    "theme.ini"

#define RDPPROXY_CRT "rdpproxy.crt"
#define RDPPROXY_KEY "rdpproxy.key"
#define DH1024_PEM "dh1024.pem"
#define DH2048_PEM "dh2048.pem"
#define DH_PEM DH2048_PEM

