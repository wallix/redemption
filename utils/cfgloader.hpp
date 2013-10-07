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

#ifndef _REDEMPTION_UTILS_CFG_LOADER_HPP_
#define _REDEMPTION_UTILS_CFG_LOADER_HPP_

#include <istream>

#include "log.hpp"

static inline unsigned ulong_from_cstr(const char * str)
{ // 10 = 10, 0x10 = 16
    if ((*str == '0') && (*(str + 1) == 'x')){
        return strtol(str + 2, 0, 16);
    }

    return atol(str);
}

static inline signed _long_from_cstr(const char * str)
{
    return atol(str);
}

static inline bool bool_from_cstr(const char * str)
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

static inline unsigned level_from_cstr(const char * str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
    if (0 == strcasecmp("medium", str)) { res = 1; }
    else if (0 == strcasecmp("high",   str)) { res = 2; }
    return res;
}

static inline unsigned logtype_from_cstr(const char * str)
{ // null = 0, print = 1, syslog = 2, file = 3, encryptedfile = 4
    unsigned res = 0;
    if (0 == strcasecmp("print",         str)) { res = 1; }
    else if (0 == strcasecmp("syslog",        str)) { res = 2; }
    else if (0 == strcasecmp("file",          str)) { res = 3; }
    else if (0 == strcasecmp("encryptedfile", str)) { res = 4; }
    return res;
}

struct ConfigurationHolder {
    virtual void set_value(const char * section, const char * key, const char * value) = 0;
};


struct ConfigurationLoader {
    ConfigurationLoader(ConfigurationHolder & configuration_holder) {
        std::stringstream oss("");

        this->cparse(configuration_holder, oss);
    }

    ConfigurationLoader(ConfigurationHolder & configuration_holder, const char * filename) {
        std::stringstream oss("");

        this->cparse(configuration_holder, filename);
    }

    ConfigurationLoader(ConfigurationHolder & configuration_holder, istream & Inifile_stream) {
        this->cparse(configuration_holder, Inifile_stream);
    }

    void cparse(ConfigurationHolder & configuration_holder, const char * filename) {
        ifstream inifile(filename);

        this->cparse(configuration_holder, inifile);
    }

    void cparse(ConfigurationHolder & configuration_holder, istream & ifs) {
        const size_t maxlen = 256;
        char line[maxlen];
        char context[128] = { 0 };
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
            this->parseline(configuration_holder, line, context);
        };
    }

    void parseline( ConfigurationHolder & configuration_holder
                  , const char * line, char * context) {
        char key[128];
        char value[128];

        const char * startkey = line;
        for (; *startkey ; startkey++) {
            if (!isspace(*startkey)) {
                if (*startkey == '[') {
                    const char * startcontext = startkey + 1;
                    const char * endcontext = strchr(startcontext, ']');
                    if (endcontext) {
                        memcpy(context, startcontext, endcontext - startcontext);
                        context[endcontext - startcontext] = 0;
                    }
                    return;
                }
                break;
            }
        }

        const char * endkey = strchr(startkey, '=');
        if (endkey && endkey != startkey) {
            const char * sep = endkey;
            for (--endkey; endkey >= startkey ; endkey--) {
                if (!isspace(*endkey)) {
                    TODO("RZ: Possible buffer overflow if length of key is larger than 128 bytes");
                    memcpy(key, startkey, endkey - startkey + 1);
                    key[endkey - startkey + 1] = 0;

                    const char * startvalue = sep + 1;
                    for ( ; *startvalue ; startvalue++) {
                        if (!isspace(*startvalue)) {
                            break;
                        }
                    }
                    const char * endvalue;
                    /*
                      for (endvalue = startvalue; *endvalue ; endvalue++) {
                      TODO("RZ: Support space in value")
                      if (isspace(*endvalue) || *endvalue == '#'){
                      break;
                      }
                      }
                      TODO("RZ: Possible buffer overflow if length of value is larger than 128 bytes")
                      memcpy(value, startvalue, endvalue - startvalue + 1);
                      value[endvalue - startvalue + 1] = 0;
                    */
                    char *curvalue = value;
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        if (isspace(*endvalue) || *endvalue == '#') {
                            break;
                        }
                        else if ((*endvalue == '\\') && *(endvalue + 1)) {
                            if (endvalue > startvalue) {
                                memcpy(curvalue, startvalue, endvalue - startvalue);
                                curvalue += (endvalue - startvalue);
                            }

                            endvalue++;

                            *curvalue++ = *endvalue;

                            startvalue = endvalue + 1;
                        }
                    }
                    if (endvalue > startvalue) {
                        memcpy(curvalue, startvalue, endvalue - startvalue);
                        curvalue += (endvalue - startvalue);
                    }
                    *curvalue = 0;

                    configuration_holder.set_value(context, key, value);
                    break;
                }
            }
        }
    }   // void parseline( const char * line, const char * context
        //               , ConfigurationHolder & configuration_holder)
};  // struct ConfigurationLoader

#endif  // #ifndef _REDEMPTION_UTILS_CFG_LOADER_HPP_
