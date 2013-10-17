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

#ifndef REDEMPTION_REGEX_CONSUMER_HPP
#define REDEMPTION_REGEX_CONSUMER_HPP

#include <ostream>
#include <stdint.h>

namespace re {

    using std::size_t;
    typedef uint32_t char_int;

    struct utf_char
    {
        utf_char(char_int c)
        : utfc(c)
        {}

        char_int utfc;
    };

    inline std::ostream& operator<<(std::ostream& os, utf_char utf_c)
    {
        char c[] = {
            char((utf_c.utfc & 0XFF000000) >> 24),
            char((utf_c.utfc & 0X00FF0000) >> 16),
            char((utf_c.utfc & 0X0000FF00) >> 8),
            char((utf_c.utfc & 0X000000FF)),
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

    inline std::string& operator+=(std::string& str, utf_char utf_c)
    {
        char c[] = {
            char((utf_c.utfc & 0XFF000000) >> 24),
            char((utf_c.utfc & 0X00FF0000) >> 16),
            char((utf_c.utfc & 0X0000FF00) >> 8),
            char((utf_c.utfc & 0X000000FF)),
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

    class utf_consumer
    {
    public:
        utf_consumer(const char * str)
        : s(reinterpret_cast<const unsigned char *>(str))
        {}

        char_int bumpc()
        {
            char_int c = *this->s;
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

        char_int getc() const
        {
            return utf_consumer(this->str()).bumpc();
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

    inline bool utf_contains(const char * str, char_int c)
    {
        utf_consumer consumer(str);
        while (consumer.valid()) {
            if (consumer.bumpc() == c) {
                return true;
            }
        }
        return false;
    }

}

#endif
