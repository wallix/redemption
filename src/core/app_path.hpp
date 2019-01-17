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

#include <string>

enum class AppPath
{
    Metrics, 
    Record,
    RecordTmp,
    Video,
    Ocr,
    Png,
    Wrm,
    Hash,
    License,
    Certif,
    Persistent,
    PersistentRdp,
    DriveRedirection,
    Share,
    Cfg,
    LoginWabBlue,
    RedemptionLogo24,
    WallixIconMin,
    CfgIni,
    CfgCrt,
    CfgKey,
    CfgDhPem,
    LockDir,
    LockFile,
    DefaultFontFile,
};
char const * app_path(AppPath) noexcept;
std::string app_path_s(AppPath);

// TODO move these into configuration
// #define LOGIN_LOGO24 "ad24b.png"
#define LOGIN_WAB_BLUE "wablogoblue.png"
#define LOGO_PNG "logo.png"
#define LOGO_BMP "logo.bmp"
// #define HELP_ICON "helpicon.png"
// #define CURSOR0 "cursor0.cur"
// #define CURSOR1 "cursor1.cur"
// #define FONT1 "sans-10.rbf"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.png"
#define LOCKFILE "rdpproxy.pid"

// #define RSAKEYS_INI "rsakeys.ini"
#define RDPPROXY_INI "rdpproxy.ini"
#define THEME_INI    "theme.ini"

#define DEFAULT_FONT_NAME "dejavu_14.rbf"
