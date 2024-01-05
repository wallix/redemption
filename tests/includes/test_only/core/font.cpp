/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/core/font.hpp"
#include "core/font.hpp"

namespace
{
    Font font;
    Font deja_vu_14;
    FontData deja_vu_14_data;
}

Font const& global_font()
{
    return font;
}

Font const& global_font_deja_vu_14()
{
    if (!deja_vu_14_data.is_loaded()) {
        deja_vu_14_data = FontData(FIXTURES_PATH "/dejavu_14.rbf2");
        deja_vu_14 = deja_vu_14_data.font();
    }
    return deja_vu_14;
}
