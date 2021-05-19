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

#include "utils/cfgloader.hpp"
#include "utils/log.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <memory>
#include <cerrno>
#include <cstring>

#include <sys/stat.h>
#include <unistd.h>


namespace
{
    bool configuration_load_from_string(
        ConfigurationHolder & configuration_holder,
        char const * filename, char * p)
    {
        bool has_error = false;

        int line_count = 1;
        char const * pos_line = p;

        char cstr_empty[1]{};
        char * section = cstr_empty;
        char * key;
        char * value;
        char * end_key;
        char * end_value;
        char * end_section;

        auto next_line = [&]{
            ++line_count;
            pos_line = p;
        };

        auto set_error = [&](char const* mess){
            LOG(LOG_ERR, "%s:%d:%d: %s", filename, line_count, int(p - pos_line), mess);
            has_error = true;
        };

        for (;;) {
            loop:
            switch (*p) {
                case '\0': return !has_error;
                case '\n': case '\r': ++p; continue;

                // comment
                case '#': ++p; goto consume_line;

                // empty line / left trim
                case ' ':
                case '\t':
                    for (;;) switch (*++p) /* NOLINT */ {
                        case ' ': case '\t': continue;
                        case '\n': case '\r': next_line(); ++p; goto loop;
                        case '\0': return !has_error;
                        default: goto loop;
                    }

                // section
                case '[' :
                    for (;;) switch (*++p) /* NOLINT */ {
                        // left trim
                        case ' ': case '\t': continue;
                        case '\n':
                        case '\r':
                            next_line();
                            [[fallthrough]];
                        case ']':
                            ++p;
                            [[fallthrough]];
                        case '\0':
                            set_error("empty section");
                            goto loop;

                        default:
                            section = p;
                        insection:
                            for (;;) switch (*++p) /* NOLINT */ {
                                default: continue;

                                case '\0':
                                case '\n':
                                case '\r':
                                    set_error("']' not found, assume new section");
                                    if (*p == '\0') {
                                        return !has_error;
                                    }
                                    next_line();
                                    [[fallthrough]];
                                case ']':
                                    end_section = p;
                                    *p = '\0';
                                    goto assign_section;

                                case ' ':
                                case '\t':
                                    // right trim
                                    end_section = p;
                                    for (;;) switch (*++p) /* NOLINT */ {
                                        case ' ': case '\t': continue;
                                        default: goto insection;
                                        case ']':
                                            *end_section = '\0';
                                            goto assign_section;
                                    }
                            }

                        assign_section:
                            configuration_holder.set_section(
                                zstring_view::from_null_terminated({section, end_section})
                            );
                            *end_section = '\0';
                            ++p;
                            goto loop;
                    }

                // value
                default:
                    key = p;
                    for (;;) switch (*++p) /* NOLINT */ {
                        default: continue;
                        // right trim key
                        case ' ': case '\t':
                            end_key = p;
                            *p = '\0';
                            for (;;) switch (*++p) /* NOLINT */ {
                                case ' ': case '\t': continue;
                                case '=': goto set_value;
                                default: goto set_value_error;
                            }

                        case '=':
                            end_key = p;
                            *p = '\0';

                        set_value:
                            if (not *key) {
                                set_error("empty key");
                                goto consume_line;
                            }

                            for (;;) switch (*++p) /* NOLINT */ {
                                // left trim
                                case ' ': case '\t': continue;
                                case '\n':
                                case '\r':
                                    value = p;
                                    end_value = p;
                                    *p = '\0';
                                    goto assign_value_and_next_line;
                                case '\0':
                                    value = p;
                                    end_value = p;
                                    *p = '\0';
                                    goto assign_value;

                                default:
                                    value = p;
                                invalue:
                                    for (;;) switch (*++p) /* NOLINT */ {
                                        default: continue;
                                        case '\n':
                                        case '\r':
                                            end_value = p;
                                            *p = '\0';
                                            goto assign_value_and_next_line;
                                        case '\0':
                                            end_value = p;
                                            *p = '\0';
                                            goto assign_value;

                                        case ' ':
                                        case '\t':
                                            // right trim
                                            end_value = p;
                                            for (;;) switch (*++p) /* NOLINT */ {
                                                case ' ': case '\t': continue;
                                                default: goto invalue;
                                                case '\n':
                                                case '\r':
                                                    *end_value = '\0';
                                                    goto assign_value_and_next_line;
                                                case '\0':
                                                    *end_value = '\0';
                                                    goto assign_value;
                                            }
                                    }

                                assign_value_and_next_line:
                                    next_line();
                                    ++p;
                                assign_value:
                                    configuration_holder.set_value(
                                        zstring_view::from_null_terminated({key, end_key}),
                                        zstring_view::from_null_terminated({value, end_value})
                                    );
                                    goto loop;
                            }

                        case '\n':
                        case '\r':
                        case '\0':
                        set_value_error:
                            set_error("invalid syntax, expected '=' ; this line is ignored");
                            goto consume_line;
                    }
            }

            consume_line:
                for (;;) switch (*p) /*NOLINT*/ {
                    default: ++p; continue;
                    case '\n': case '\r': next_line(); ++p; goto loop;
                    case '\0': return !has_error;
                }
        }
    }
}

bool configuration_load(
    ConfigurationHolder & configuration_holder, char const* filename)
{
    struct stat st;
    if (-1 == stat(filename, &st)){
        return false;
    }

    if (st.st_size > 1024*1024) {
        LOG(LOG_ERR, "%s: file too large", filename);
        return false;
    }

    if (st.st_size == 0) {
        return true;
    }

    std::size_t len = std::size_t(st.st_size);
    std::unique_ptr<char[]> buf(new char[len + 1u]); /* NOLINT */

    // read file
    {
        unique_fd fd(filename);
        if (not fd) {
            LOG(LOG_ERR, "%s: %s", filename, strerror(errno));
            return false;
        }

        char* p = buf.get();
        std::size_t remaning = len;
        ssize_t r;
        while ((r = read(fd.fd(), p, remaning)) > 0) {
            remaning -= std::size_t(r);
            p += r;
        }

        if (r == -1) {
            LOG(LOG_ERR, "%s: %s", filename, strerror(errno));
            return false;
        }

        *p = '\0';
    }

    return configuration_load_from_string(configuration_holder, filename, buf.get());
}
