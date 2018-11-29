/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010
Author(s): Christophe Grosjean, Jonathan Poelen
*/

#include "utils/hexdump.hpp"
#include "utils/log.hpp"

#include <cstdio> // std::sprintf
#include <cstring> // strlen

namespace {

// $line_prefix "%.4x" $sep_page_values ($value_prefix "%.2x" $value_suffix) $sep_value_chars "%*c" $prefix_chars ("%c")
void hexdump_impl(
    const unsigned char * data, size_t size, unsigned line_length,
    char const * line_prefix, char const * sep_page_values,
    char const * value_prefix, char const * value_suffix,
    char const * sep_value_chars, char const * prefix_chars)
{
    char buffer[2048];
    size_t const sep_len = strlen(value_prefix) + strlen(value_suffix) + 2;
    for (size_t j = 0; j < size; j += line_length){
        char * line = buffer;
        line += std::sprintf(line, "%s%.4x%s",
            line_prefix, static_cast<unsigned>(j), sep_page_values);

        size_t i;

        for (i = 0; i < line_length && j+i < size; i++){
            line += std::sprintf(line, "%s%.2x%s",
                value_prefix, static_cast<unsigned>(data[j+i]), value_suffix);
        }

        line += std::sprintf(line, "%s", sep_value_chars);
        if (i < line_length){
            line += std::sprintf(line, "%*c", static_cast<int>((line_length-i)*sep_len), ' ');
        }
        line += std::sprintf(line, "%s", prefix_chars);

        for (i = 0; i < line_length && j+i < size; i++){
            unsigned char tmp = data[j+i];
            if (tmp < ' ' || tmp > '~' || tmp == '\\'){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
        }
    }
}

} // namespace


void hexdump(const_byte_ptr data, size_t size, unsigned line_length)
{
    // %.4x %x %x ... %c%c..
    hexdump_impl(data.to_u8p(), size, line_length, "", " ", "", " ", "", "");
}

void hexdump_av(const_bytes_view data, unsigned line_length)
{
    hexdump(data.to_u8p(), data.size(), line_length);
}

// also available for 16 bits items arrays, size in bytes and must be even
void hexdump16_d(const uint16_t * data, size_t size)
{
    // /* %.4x */ 0x%x 0x%x ... // %c%c..
    hexdump_impl(reinterpret_cast<const uint8_t *>(data), size, 16, "/* ", " */ ", "0x", ", ", "", " // "); /* NOLINT */

}


void hexdump_d(const_byte_ptr data, size_t size, unsigned line_length)
{
    // /* %.4x */ 0x%x 0x%x ... // %c%c..
    hexdump_impl(data.to_u8p(), size, line_length, "/* ", " */ ", "0x", ", ", "", " // ");
}

void hexdump_av_d(const_bytes_view data, unsigned line_length)
{
    hexdump_d(data.to_u8p(), data.size(), line_length);
}


void hexdump_c(const_byte_ptr data, size_t size, unsigned line_length)
{
    // /* %.4x */ "\x%x\x%x ..." // %c%c..
    hexdump_impl(data.to_u8p(), size, line_length, "/* ", " */ \"", "\\x", "", "\"", " // ");
}

void hexdump_av_c(const_bytes_view data, unsigned line_length)
{
    hexdump_c(data.to_u8p(), data.size(), line_length);
}
