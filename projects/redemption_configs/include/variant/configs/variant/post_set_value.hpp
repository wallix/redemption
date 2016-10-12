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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/theme.hpp"
#include "utils/fileutils.hpp"
#include "core/defines.hpp"

namespace configs
{
    inline void post_set_value(VariablesConfiguration & vars, ::cfg::internal_mod::theme const & cfg_value)
    {
        Theme & theme = static_cast<cfg::theme&>(vars).value;

        auto & str = cfg_value.value;
        LOG(LOG_INFO, "LOAD_THEME: %s", str.c_str());

        {
            char theme_path[1024] = {};
            snprintf(theme_path, 1024, CFG_PATH "/themes/%s/" THEME_INI, str.c_str());
            theme_path[sizeof(theme_path) - 1] = 0;

            ConfigurationLoader theme_load;
            theme_load.cparse(theme, theme_path);
        }

        if (theme.global.logo) {
            char logo_path[1024] = {};
            snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_PNG, str.c_str());
            logo_path[sizeof(logo_path) - 1] = 0;
            if (!file_exist(logo_path)) {
                snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_BMP, str.c_str());
                logo_path[sizeof(logo_path) - 1] = 0;
                if (!file_exist(logo_path)) {
                    theme.global.logo = false;
                    return;
                }
            }
            theme.set_logo_path(logo_path);
        }
    }
}
