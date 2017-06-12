/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "sugar/noncopyable.hpp"

#include <istream>
#include <fstream>

#include <cstring>


struct ConfigurationHolder : private noncopyable
{
    virtual void set_value(const char * section, const char * key, const char * value) = 0;

    virtual ~ConfigurationHolder() = default;
};

bool configuration_load(ConfigurationHolder & configuration_holder, std::istream & inifile_stream);

inline
bool configuration_load(ConfigurationHolder & configuration_holder, const char * filename)
{
    std::ifstream inifile(filename);
    return configuration_load(configuration_holder, inifile);
}

inline
bool configuration_load(ConfigurationHolder & configuration_holder, std::string const & filename)
{
    return configuration_load(configuration_holder, filename.c_str());
}

inline
bool configuration_load(ConfigurationHolder && configuration_holder, std::istream & inifile_stream)
{
    return configuration_load(configuration_holder, inifile_stream);
}

inline
bool configuration_load(ConfigurationHolder && configuration_holder, const char * filename)
{
    return configuration_load(configuration_holder, filename);
}

inline
bool configuration_load(ConfigurationHolder && configuration_holder, std::string const & filename)
{
    return configuration_load(configuration_holder, filename);
}

inline
bool configuration_load(ConfigurationHolder & configuration_holder, std::istream & ifs)
{
    const size_t maxlen = 1024;
    char line[maxlen];
    char context[512]; context[0] = 0;
    char new_key[maxlen];
    char new_value[maxlen];
    bool truncated = false;
    unsigned num_line = 0;
    bool has_err = false;

    while (ifs.good()) {
        ++num_line;
        ifs.getline(line, maxlen);
        if (ifs.fail() && ifs.gcount() == maxlen-1) {
            if (!truncated) {
                LOG(LOG_INFO, "Line too long in configuration file at line %u", num_line);
                hexdump(line, maxlen-1);
                has_err = true;
            }
            ifs.clear();
            truncated = true;
            continue;
        }
        if (truncated) {
            truncated = false;
            continue;
        }

        auto const len = ifs.gcount() - 1;
        if (len <= 0) continue;
        char * last_char_ptr = line + len;
        if (*last_char_ptr) ++last_char_ptr; // line without new line char
        char * first_char_line = ltrim(line, last_char_ptr);
        if (*first_char_line == '#') continue;
        last_char_ptr = rtrim(first_char_line, last_char_ptr);

        array_view_const_char const line {first_char_line, last_char_ptr};
        auto err_msg = [&configuration_holder, &new_key, &new_value, &context, &line]() -> char const *
        {
            if (line.empty()) return nullptr;

            if (line.front() == '[') {
                if (line.back() != ']') return "missing ']'";
                if (line.size() <= 2) return "Empty section";

                auto new_context = trim(line.begin()+1, line.end()-1);
                if (new_context.empty()) return "Empty section";
                if (new_context.size() >= sizeof(context)) return "Section too long";

                memcpy(context, new_context.begin(), new_context.size());
                context[new_context.size()] = 0;
            }
            else {
                const char * endkey = std::find(line.begin(), line.end(), '=');
                if (endkey == line.end()) return "Bad line format";

                array_view_const_char const key (line.begin(), rtrim(line.begin(), endkey));
                array_view_const_char const value (ltrim(endkey+1, line.end()), line.end());
                if (key.empty()) return "Empty Key";

                memcpy(new_key, key.begin(), key.size()); new_key[key.size()] = 0;
                memcpy(new_value, value.begin(), value.size()); new_value[value.size()] = 0;
                configuration_holder.set_value(context, new_key, new_value);
            }

            return nullptr;
        }();

        if (err_msg) {
            LOG(LOG_INFO, "%s in configuration file at line %u", err_msg, num_line);
            hexdump(line.data(), line.size());
            has_err = true;
        }
    }

    return !has_err && ifs.eof();
}
