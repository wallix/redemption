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
#include "parser.hpp"
#include "defines.hpp"
#include "fileutils.hpp"
#include "config_c_str_buf.hpp"

#include <cstdio>

namespace configs {

inline void parse(unsigned & x, char const * value) { x = ulong_from_cstr(value); }
inline void parse(int & x, char const * value) { x = long_from_cstr(value); }
inline void parse(bool & x, char const * value) { x = bool_from_cstr(value); }

inline void parse(std::string & x, char const * value) { x = value; }

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
    if (x) {
        if (n > 4) {
            *buff++ = 'T';
            *buff++ = 'r';
            *buff++ = 'u';
            *buff++ = 'e';
            return 4;
        }
    }
    else {
        if (n > 5) {
            *buff++ = 'F';
            *buff++ = 'a';
            *buff++ = 'l';
            *buff++ = 's';
            *buff++ = 'e';
            return 5;
        }
    }
    return -1;
}

inline int copy_val(std::string const & x, char * buff, std::size_t n) {
    if (x.size() < n) {
        memcpy(buff, x.data(), x.size());
    }
    return int(x.size());
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

inline char const * c_str(CStrBuf<unsigned>& s, unsigned x) {
    snprintf(s.get(), s.size(), "%u", x);
    return s.get();
}

inline char const * c_str(CStrBuf<int>& s, int x) {
    snprintf(s.get(), s.size(), "%d", x);
    return s.get();
}

inline char const * c_str(CStrBuf<bool>&, bool x) {
    return x ? "True" : "False";
}

inline char const * c_str(CStrBuf<std::string>&, std::string const & x) {
    return x.c_str();
}

}

#endif
