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

#include <istream>
#include <fstream>

#include "utils/log.hpp"
#include "sugar/noncopyable.hpp"

struct ConfigurationHolder : private noncopyable
{
    virtual void set_value(const char * section, const char * key, const char * value) = 0;

    virtual ~ConfigurationHolder() = default;
};


struct ConfigurationLoader {
    ConfigurationLoader() = default;

    explicit ConfigurationLoader(ConfigurationHolder &) {
    }

    ConfigurationLoader(ConfigurationHolder & configuration_holder, const char * filename) {
        this->cparse(configuration_holder, filename);
    }

    ConfigurationLoader(ConfigurationHolder & configuration_holder, std::string const & filename) {
        this->cparse(configuration_holder, filename.c_str());
    }

    ConfigurationLoader(ConfigurationHolder & configuration_holder, std::istream & Inifile_stream) {
        this->cparse(configuration_holder, Inifile_stream);
    }

    void cparse(ConfigurationHolder & configuration_holder, const char * filename) {
        std::ifstream inifile(filename);

        this->cparse(configuration_holder, inifile);
    }

    void cparse(ConfigurationHolder & configuration_holder, std::string const & filename) {
        this->cparse(configuration_holder, filename.c_str());
    }

    void cparse(ConfigurationHolder & configuration_holder, std::istream & ifs) {
        const size_t maxlen = 1024;
        char line[maxlen];
        char context[512] = { 0 };
        bool truncated = false;
        while (ifs.good()) {
            ifs.getline(line, maxlen);
            if (ifs.fail() && ifs.gcount() == (maxlen-1)) {
                if (!truncated) {
                    LOG(LOG_INFO, "Line too long in configuration file");
                    hexdump(line, maxlen-1);
                }
                ifs.clear();
                truncated = true;
                continue;
            }
            if (truncated) {
                truncated = false;
                continue;
            }
            char * tmp_line = line;
            while (isspace(*tmp_line)) tmp_line++;
            if (*tmp_line == '#') continue;
            char * last_char_ptr = tmp_line + strlen(tmp_line) - 1;
            while ((last_char_ptr >= tmp_line) && isspace(*last_char_ptr)) last_char_ptr--;
            if (last_char_ptr < tmp_line) continue;
            *(last_char_ptr + 1) = '\0';
            //LOG(LOG_INFO, "Line='%s'", tmp_line);
            this->parseline(configuration_holder, tmp_line, context);
        };
    }

    void parseline( ConfigurationHolder & configuration_holder
                  , const char * line, char * context) {
        const char * startkey = line;
        for (; *startkey ; startkey++) {
            if (!isspace(*startkey)) {
                if (*startkey == '[') {
                    const char * startcontext = startkey + 1;
                    while (*startcontext && isspace(*startcontext)) startcontext++;
                    const char * endcontext = strchr(startcontext, ']');
                    if (endcontext && !(*(endcontext + 1))) {
                        while ((endcontext - 1 > startcontext) && isspace(*(endcontext - 1))) endcontext--;
                        if (endcontext - 1 > startcontext) {
                            memcpy(context, startcontext, endcontext - startcontext);
                            context[endcontext - startcontext] = 0;
                        }
                    }
                    return;
                }
                break;
            }
        }

        const char * endkey = strchr(startkey, '=');
        if (endkey && endkey != startkey) {
            const char * sep = endkey;
            char key[512];
            char value[512];
            for (--endkey; endkey >= startkey ; endkey--) {
                if (!isspace(*endkey)) {
                    // TODO RZ: Possible buffer overflow if length of key is larger than 128 bytes
                    memcpy(key, startkey, endkey - startkey + 1);
                    key[endkey - startkey + 1] = 0;

                    const char * startvalue = sep + 1;
                    for ( ; *startvalue ; startvalue++) {
                        if (!isspace(*startvalue)) {
                            break;
                        }
                    }
                    const char * endvalue = startvalue + strlen(startvalue);
                    while ((endvalue > startvalue) && isspace(*(endvalue - 1))) endvalue--;
                    if (endvalue >= startvalue) {
                        memcpy(value, startvalue, endvalue - startvalue);
                        value[endvalue - startvalue] = 0;
                        //LOG(LOG_INFO, "context='%s' key='%s' value='%s'", context, key, value);
                        configuration_holder.set_value(context, key, value);
                    }
                    break;
                }
            }
        }
    }   // void parseline( const char * line, const char * context
        //               , ConfigurationHolder & configuration_holder)
};  // struct ConfigurationLoader

