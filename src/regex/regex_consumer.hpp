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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
 */


#pragma once

#include <ostream>
#include <cstdint>
#include <cstdio>

namespace re {

    using std::size_t;
    using char_int = uint32_t;

    struct utf8_char
    {
        explicit utf8_char(char_int c)
        : uc(c)
        {}

        char_int uc;
    };

    inline std::ostream& operator<<(std::ostream& os, utf8_char utf8_c)
    {
        char c[] = {
            char((utf8_c.uc & 0xFF000000) >> 24),
            char((utf8_c.uc & 0x00FF0000) >> 16),
            char((utf8_c.uc & 0x0000FF00) >> 8),
            char((utf8_c.uc & 0x000000FF)),
        };
        if (c[0]) {
            return os.write(c, 4);
        }
        else if (c[1]) {
            return os.write(c+1, 3);
        }
        else if (c[2]) {
            return os.write(c+2, 2);
        }
        else {
            os.write(c+3, 1);
        }
        return os;
    }

    inline std::string& operator+=(std::string& str, utf8_char utf8_c)
    {
        char c[] = {
            char((utf8_c.uc & 0xFF000000) >> 24),
            char((utf8_c.uc & 0x00FF0000) >> 16),
            char((utf8_c.uc & 0x0000FF00) >> 8),
            char((utf8_c.uc & 0x000000FF)),
        };
        if (c[0]) {
            str += c[0];
            str += c[1];
            str += c[2];
        }
        else if (c[1]) {
            str += c[1];
            str += c[2];
        }
        else if (c[2]) {
            str += c[2];
        }
        str += c[3];
        return str;
    }

    class utf8_consumer
    {
    public:
        explicit utf8_consumer(const char * str)
        : s(reinterpret_cast<const unsigned char *>(str))
        {}

        char_int bumpc()
        {
            char_int c = *this->s;
            if (!c) {
                return c;
            }
            ++this->s;
            if (*this->s >> 6 == 2) {
                c <<= 8;
                c |= *this->s;
                ++this->s;
                if (*this->s >> 6 == 2) {
                    c <<= 8;
                    c |= *this->s;
                    ++this->s;
                    if (*this->s >> 6 == 2) {
                        c <<= 8;
                        c |= *this->s;
                        ++this->s;
                    }
                }
            }
            return c;
        }

        size_t length() const
        {
            size_t len = 0;
            const unsigned char * p = this->s;
            while (*p) {
                if ((*++p >> 6 == 2) && (*++p >> 6 == 2) && (*++p >> 6 == 2)) {
                    ++p;
                }
                ++len;
            }
            return len;
        }

        char_int getc() const
        {
            return utf8_consumer(this->str()).bumpc();
        }

        bool valid() const
        {
            return *this->s;
        }

        const char * str() const
        {
            return reinterpret_cast<const char *>(s);
        }

        void str(const char * str)
        {
            s = reinterpret_cast<const unsigned char *>(str);
        }

        const unsigned char * s;
    };

    inline bool utf8_contains(const char * str, char_int c)
    {
        utf8_consumer consumer(str);
        while (consumer.valid()) {
            if (consumer.bumpc() == c) {
                return true;
            }
        }
        return false;
    }

}  // namespace re

