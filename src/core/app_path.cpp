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

#include "core/app_path.hpp"
#include <cassert>

#ifdef IN_IDE_PARSER
# define METRICS_PATH "/var/rdpproxy/recorded/metrics"
# define RECORD_PATH "/var/rdpproxy/recorded"
# define RECORD_TMP_PATH "/var/rdpproxy/tmp"
# define VIDEO_PATH "/tmp"
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

char const * app_path(AppPath k) noexcept
{
    switch (k)
    {
        case AppPath::Metrics: return METRICS_PATH;
        case AppPath::Record: return RECORD_PATH;
        case AppPath::RecordTmp: return RECORD_TMP_PATH;
        case AppPath::Video: return VIDEO_PATH;
        case AppPath::Ocr: return OCR_PATH;
        case AppPath::Png: return PNG_PATH;
        case AppPath::Wrm: return WRM_PATH;
        case AppPath::Hash: return HASH_PATH;
        case AppPath::License: return LICENSE_PATH;
        case AppPath::Certif: return CERTIF_PATH;
        case AppPath::Persistent: return PERSISTENT_PATH;
        case AppPath::PersistentRdp: return PERSISTENT_PATH "/mod_rdp";
        case AppPath::DriveRedirection: return DRIVE_REDIRECTION_PATH;
        case AppPath::Share: return SHARE_PATH;
        case AppPath::Cfg: return CFG_PATH;
        case AppPath::LoginWabBlue: return SHARE_PATH "/" LOGIN_WAB_BLUE;
        case AppPath::RedemptionLogo24: return SHARE_PATH "/" REDEMPTION_LOGO24;
        case AppPath::WallixIconMin: return SHARE_PATH "/wallix-icon-min.png";
        case AppPath::CfgIni: return CFG_PATH "/" RDPPROXY_INI;
        case AppPath::CfgCrt: return CFG_PATH "/rdpproxy.crt";
        case AppPath::CfgKey: return CFG_PATH "/rdpproxy.key";
        case AppPath::CfgDhPem: return CFG_PATH "/dh2048.pem";
        case AppPath::LockDir: return PID_PATH "/redemption";
        case AppPath::LockFile: return PID_PATH "/redemption/" LOCKFILE;
        case AppPath::DefaultFontFile: return SHARE_PATH "/" DEFAULT_FONT_NAME;
    }

    assert(false);
    return "";
}

std::string app_path_s(AppPath k)
{
    return app_path(k);
}
