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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_UTILS_CONFIG_TYPES_PARSE_HPP
#define REDEMPTION_SRC_UTILS_CONFIG_TYPES_PARSE_HPP

#include <type_traits>

#include "theme.hpp"
#include "types.hpp"
#include "basefield.hpp"
#include "parser.hpp"
#include "defines.hpp"
#include "fileutils.hpp"

#include <cstdio>

namespace configs {

void parse(unsigned & x, char const * value) { x = ulong_from_cstr(value); }
void parse(int & x, char const * value) { x = long_from_cstr(value); }
void parse(bool & x, char const * value) { x = bool_from_cstr(value); }

void parse(std::string & x, char const * value) { x = value; }

void parse(Level & x, char const * value) { x = level_from_cstr(value); }
void parse(ColorDepth & x, char const * value) { x = color_depth_from_cstr(value); }
void parse(CaptureFlags & x, char const * value) { x = static_cast<CaptureFlags>(ulong_from_cstr(value)); }

template<std::size_t N, class Copier, bool NullableString>
void parse(StaticStringBase<N, Copier, NullableString> & x, char const * value) { x = value; }

template<std::size_t N>
void parse(StaticKeyString<N> & key, char const * value) {
    if (strlen(value) >= sizeof(key) * 2) {
        char   hexval[3] = { 0 };
        char * end;
        for (std::size_t i = 0; i < sizeof(key); i++) {
            memcpy(hexval, value + i * 2, 2);

            key.data()[i] = strtol(hexval, &end, 16);
        }
    }
}

void parse(BoolField & x, char const * value) { x.set_from_cstr(value); }
void parse(UnsignedField & x, char const * value) { x.set_from_cstr(value); }
void parse(SignedField & x, char const * value) { x.set_from_cstr(value); }
void parse(StringField & x, char const * value) { x.set_from_cstr(value); }

template<class T, T Min, T Max, T Default>
void parse(Range<T, Min, Max, Default> & x, char const * value) { x = long_from_cstr(value); }

template<class Enum>
void parse(FlagsField<Enum> & x, char const * value) { x.set_from_cstr(value); }

template<class Enum, class Traits>
void parse(EnumField<Enum, Traits> & x, char const * value) { x.set_from_cstr(value); }

void parse(Theme & theme, char const * value) {
    if (value && *value) {
        LOG(LOG_INFO, "LOAD_THEME: %s", value);
        char theme_path[1024] = {};
        snprintf(theme_path, 1024, CFG_PATH "/themes/%s/" THEME_INI, value);
        theme_path[sizeof(theme_path) - 1] = 0;
        ConfigurationLoader theme_load(theme, theme_path);
        if (theme.global.logo) {
            char logo_path[1024] = {};
            snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_PNG, value);
            logo_path[sizeof(logo_path) - 1] = 0;
            if (!file_exist(logo_path)) {
                snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_BMP,
                        value);
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

}

#endif
