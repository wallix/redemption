/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/app_path.hpp"

#ifndef REDEMPTION_DONT_USE_BOOST_TEST
# include "test_only/test_framework/impl/test_paths.hpp"
#endif

#include <array>

#ifdef IN_IDE_PARSER
// configured in jam/redemption-dependencies.jam
# define SHARE_PATH "/usr/local/share/rdpproxy"
# define CFG_PATH "/etc/rdpproxy"
#endif

#define LOGIN_WAB_BLUE "wablogoblue.png"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.png"
#define RDPPROXY_INI "rdpproxy.ini"
#define DEFAULT_FONT_NAME "dejavu_14.rbf2"

namespace
{
    std::string app_path_buffer[32];
}

zstring_view app_path(AppPath k) noexcept
{
    auto set_cache = [&](std::string_view str) {
        static std::array<std::string, 32> app_path_buffer {};
        auto i = static_cast<std::size_t>(k);
#ifndef REDEMPTION_DONT_USE_BOOST_TEST
        // copy, don't move for address consistence
        auto path = ut_impl::compute_test_path(str);
        str = path;
#endif
        app_path_buffer[i] = str;
        return zstring_view(app_path_buffer[i]);
    };

    switch (k)
    {
        case AppPath::Share: return SHARE_PATH ""_zv;
        case AppPath::Cfg: return CFG_PATH ""_zv;
        case AppPath::LoginWabBlue: return SHARE_PATH "/" LOGIN_WAB_BLUE ""_zv;
        case AppPath::RedemptionLogo24: return SHARE_PATH "/" REDEMPTION_LOGO24 ""_zv;
        case AppPath::WallixIconMin: return SHARE_PATH "/wallix-icon-min.png"_zv;
        case AppPath::CfgIni: return CFG_PATH "/" RDPPROXY_INI ""_zv;
        case AppPath::CfgCrt: return CFG_PATH "/rdpproxy.crt"_zv;
        case AppPath::CfgKey: return CFG_PATH "/rdpproxy.key"_zv;
        case AppPath::CfgDhPem: return CFG_PATH "/dh3072.pem"_zv;
        case AppPath::DefaultFontFile: return SHARE_PATH "/" DEFAULT_FONT_NAME ""_zv;
        case AppPath::LockDir: return set_cache("/LockDir");
        case AppPath::LockFile: return set_cache("/LockFile");
        case AppPath::Record: return set_cache("/Record");
        case AppPath::RecordTmp: return set_cache("/RecordTmp");
        case AppPath::Video: return set_cache("/Video");
        case AppPath::Ocr: return set_cache("/Ocr");
        case AppPath::Png: return set_cache("/Png");
        case AppPath::Wrm: return set_cache("/Wrm");
        case AppPath::Hash: return set_cache("/Hash");
        case AppPath::License: return set_cache("/License");
        case AppPath::Certif: return set_cache("/Certif");
        case AppPath::Persistent: return set_cache("/Persistent");
        case AppPath::PersistentRdp: return set_cache("/PersistentRdp");
        case AppPath::DriveRedirection: return set_cache("/DriveRedirection");
        case AppPath::SessionTmpDir: return set_cache("/SessionTmpDir");
    }

    REDEMPTION_UNREACHABLE();
}
