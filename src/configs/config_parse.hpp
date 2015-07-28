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
#include <limits>
#include <string>

#include "theme.hpp"
#include "config_types.hpp"
#include "basefield.hpp"
#include "parser.hpp"
#include "defines.hpp"
#include "fileutils.hpp"
#include "dynamic_buffer.hpp"

#include <cstdio>

namespace configs {

inline void parse(unsigned & x, char const * value) { x = ulong_from_cstr(value); }
inline void parse(int & x, char const * value) { x = long_from_cstr(value); }
inline void parse(bool & x, char const * value) { x = bool_from_cstr(value); }

inline void parse(std::string & x, char const * value) { x = value; }

inline void parse(Level & x, char const * value) { x = level_from_cstr(value); }
inline void parse(ColorDepth & x, char const * value) { x = color_depth_from_cstr(value); }

template<std::size_t N, class Copier, bool NullableString>
void parse(StaticStringBase<N, Copier, NullableString> & x, char const * value) { x = value; }

template<std::size_t N>
void parse(StaticKeyString<N> & key, char const * value) {
    if (strlen(value) >= N * 2) {
        char   hexval[3] = { 0 };
        char * end;
        for (std::size_t i = 0; i < sizeof(key); i++) {
            memcpy(hexval, value + i * 2, 2);

            key.data()[i] = strtol(hexval, &end, 16);
        }
    }
}

template<class T, T Min, T Max, T Default>
void parse(Range<T, Min, Max, Default> & x, char const * value) { x = long_from_cstr(value); }

inline void parse(Theme & theme, char const * value) {
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


inline int copy_val(unsigned x, char * buff, std::size_t n) { return snprintf(buff, n, "%u", x); }
inline int copy_val(int x, char * buff, std::size_t n) { return snprintf(buff, n, "%d", x); }
inline int copy_val(bool x, char * buff, std::size_t n) {
    if (n > 1) {
        *buff++ = x ? '1' :'0';
        return 1;
    }
    return -1;
}

inline int copy_val(std::string const & x, char * buff, std::size_t n) {
    if (x.size() < n) {
        memcpy(buff, x.data(), x.size());
    }
    return int(x.size());
}

inline int copy_val(Level x, char * buff, std::size_t n) {
    return snprintf(buff, n, "%s", cstr_from_level(x));
}
inline int copy_val(ColorDepth x, char * buff, std::size_t n) {
    return snprintf(buff, n, "%s", cstr_from_color_depth(x));
}

template<std::size_t N, class Copier, bool NullableString>
int copy_val(StaticStringBase<N, Copier, NullableString> const & x, char * buff, std::size_t n) {
    return snprintf(buff, n, "%s", x.c_str());
}

template<std::size_t N>
int copy_val(StaticKeyString<N> const & key, char * buff, std::size_t n) {
    if (N * 2 < n) {
        auto first = key.c_str();
        auto last = key.c_str() + sizeof(key);
        for (; first != last; ++buff, ++first) {
            auto x = unsigned(*first) & 0xf;
            *buff = x < 10 ? ('0' + x) : ('A' + x - 10);
            ++buff;
            *buff = x < 10 ? ('0' + x) : ('A' + x - 10);
        }
    }
    return int(N*2);
}

template<int Min, int Max, int Default>
int copy_val(Range<int, Min, Max, Default> const & x, char * buff, std::size_t n) {
    return snprintf(buff, n, "%d", int(x));
}

template<unsigned Min, unsigned Max, unsigned Default>
int copy_val(Range<unsigned, Min, Max, Default> const & x, char * buff, std::size_t n) {
    return snprintf(buff, n, "%u", unsigned(x));
}

inline int copy_val(Theme const & , char * , std::size_t ) {
    assert(false);
    return 0;
}

inline char const * c_str(DynamicBuffer& s, unsigned x) {
    s.reserve(std::numeric_limits<unsigned>::digits10 + 2);
    snprintf(s.get(), s.size(), "%u", x);
    return s.get();
}

inline char const * c_str(DynamicBuffer& s, int x) {
    s.reserve(std::numeric_limits<unsigned>::digits10 + 2);
    snprintf(s.get(), s.size(), "%d", x);
    return s.get();
}

inline char const * c_str(DynamicBuffer& s, bool x) {
    s.reserve(32);
    s[0] = x ? '1' : '0';
    s[1] = 0;
    return s.get();
}

inline char const * c_str(DynamicBuffer&, std::string const & x) {
    return x.c_str();
}

inline char const * c_str(DynamicBuffer& s, Level x) {
    auto cstr = cstr_from_level(x);
    s.reserve(32);
    s[s.size()-1] = 0;
    strncpy(s.get(), cstr, s.size()-1);
    return s.get();
}

inline char const * c_str(DynamicBuffer& s, ColorDepth x, char const * value) {
    auto cstr = cstr_from_color_depth(x);
    s.reserve(32);
    s[s.size()-1] = 0;
    strncpy(s.get(), cstr, s.size()-1);
    return s.get();
}

inline char const * c_str(DynamicBuffer& s, Theme const & theme) {
    assert(false);
    return "";
}

}

#endif
