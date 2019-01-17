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

#include "capture/ocr/locale/latin_to_cyrillic.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/utf.hpp"


namespace
{
    char const * ascii_table_to_cyrillic[] {
        /* 0 */ nullptr,
        /* 1 */ nullptr,
        /* 2 */ nullptr,
        /* 3 */ nullptr,
        /* 4 */ nullptr,
        /* 5 */ nullptr,
        /* 6 */ nullptr,
        /* 7 */ nullptr,
        /* 8 */ nullptr,
        /* 9 */ nullptr,
        /* 10 */ nullptr,
        /* 11 */ nullptr,
        /* 12 */ nullptr,
        /* 13 */ nullptr,
        /* 14 */ nullptr,
        /* 15 */ nullptr,
        /* 16 */ nullptr,
        /* 17 */ nullptr,
        /* 18 */ nullptr,
        /* 19 */ nullptr,
        /* 20 */ nullptr,
        /* 21 */ nullptr,
        /* 22 */ nullptr,
        /* 23 */ nullptr,
        /* 24 */ nullptr,
        /* 25 */ nullptr,
        /* 26 */ nullptr,
        /* 27 */ nullptr,
        /* 28 */ nullptr,
        /* 29 */ nullptr,
        /* 30 */ nullptr,
        /* 31 */ nullptr,
        /* 32 */ nullptr,
        /* 33   ! */ nullptr,
        /* 34   " */ nullptr,
        /* 35   # */ nullptr,
        /* 36   $ */ nullptr,
        /* 37   % */ nullptr,
        /* 38   & */ nullptr,
        /* 39   ' */ nullptr,
        /* 40   ( */ nullptr,
        /* 41   ) */ nullptr,
        /* 42   * */ nullptr,
        /* 43   + */ nullptr,
        /* 44   , */ nullptr,
        /* 45   - */ nullptr,
        /* 46   . */ nullptr,
        /* 47   / */ nullptr,
        /* 48   0 */ nullptr,
        /* 49   1 */ nullptr,
        /* 50   2 */ nullptr,
        /* 51   3 */ nullptr,
        /* 52   4 */ nullptr,
        /* 53   5 */ nullptr,
        /* 54   6 */ nullptr,
        /* 55   7 */ nullptr,
        /* 56   8 */ nullptr,
        /* 57   9 */ nullptr,
        /* 58   : */ nullptr,
        /* 59   ; */ nullptr,
        /* 60   < */ nullptr,
        /* 61   = */ nullptr,
        /* 62   > */ nullptr,
        /* 63   ? */ nullptr,
        /* 64   @ */ nullptr,
        /* 65   A */ "\u0410", // "А"
        /* 66   B */ nullptr,
        /* 67   C */ "\u0421", // "С"
        /* 68   D */ nullptr,
        /* 69   E */ "\u0415", // "Е"
        /* 70   F */ nullptr,
        /* 71   G */ nullptr,
        /* 72   H */ "\u041D", // "Н"
        /* 73   I */ "\u0406", // "І"
        /* 74   J */ nullptr,
        /* 75   K */ "\u041A", // "К"
        /* 76   L */ nullptr,
        /* 77   M */ "\u041C", // "М"
        /* 78   N */ nullptr,
        /* 79   O */ "\u041E", // "О"
        /* 80   P */ "\u0420", // "Р"
        /* 81   Q */ nullptr,
        /* 82   R */ nullptr,
        /* 83   S */ nullptr,
        /* 84   T */ "\u0422", // "Т"
        /* 85   U */ nullptr,
        /* 86   V */ nullptr,
        /* 87   W */ nullptr,
        /* 88   X */ nullptr,
        /* 89   Y */ nullptr,
        /* 90   Z */ nullptr,
        /* 91   [ */ nullptr,
        /* 92   \ */ nullptr,
        /* 93   ] */ nullptr,
        /* 94   ^ */ nullptr,
        /* 95   _ */ nullptr,
        /* 96   ` */ nullptr,
        /* 97   a */ "\u0430", // "а"
        /* 98   b */ nullptr,
        /* 99   c */ "\u0441", // "с"
        /* 100  d */ nullptr,
        /* 101  e */ "\u0435", // "е"
        /* 102  f */ nullptr,
        /* 103  g */ nullptr,
        /* 104  h */ nullptr,
        /* 105  i */ "\u0456", // "і"
        /* 106  j */ nullptr,
        /* 107  k */ nullptr,
        /* 108  l */ nullptr,
        /* 109  m */ nullptr,
        /* 110  n */ nullptr,
        /* 111  o */ "\u043E", // "о"
        /* 112  p */ "\u0440", // "р"
        /* 113  q */ nullptr,
        /* 114  r */ nullptr,
        /* 115  s */ nullptr,
        /* 116  t */ nullptr,
        /* 117  u */ nullptr,
        /* 118  v */ nullptr,
        /* 119  w */ nullptr,
        /* 120  x */ nullptr,
        /* 121  y */ "\u0432", // "у"
        /* 122  z */ nullptr,
    };

    bool is_latin_alpha(uint32_t c) noexcept
    { return (97 <= c && c <= 122) || (65 <= c && c <= 90); }

    bool is_cyrillic_alpha(uint32_t c) noexcept
    { return ((1040 <= c && c <= 1071) || (1072 <= c && c <= 1103)); }
} // namespace


void ocr::locale::latin_to_cyrillic_context::latin_to_cyrillic(std::string& out)
{
    using std::size_t;
    const char * str = out.c_str();

    auto beg = str;
    auto e = str;
    this->out_tmp.clear();
    UTF8toUnicodeIterator utf8_it(str);

    auto uc = *utf8_it;
    while (uc) {
        if (uc ==  ' ') {
            do {
                e = char_ptr_cast(utf8_it.pos());
                uc = *++utf8_it;
            } while (uc == ' ');
        }

        if (!uc) {
            break;
        }

        UTF8toUnicodeIterator utf8_it_start = utf8_it;

        size_t latin = 0;
        size_t latin_cyrillic = 0;
        size_t cyrillic = 0;
        size_t unknown = 0;

        do {
            if (is_latin_alpha(uc)) {
                ++latin;
                if (ascii_table_to_cyrillic[uc]) {
                    ++cyrillic;
                    ++latin_cyrillic;
                }
            }
            else if (is_cyrillic_alpha(uc)) {
                ++cyrillic;
            }
            else if (uc > 127 || uc == '?') {
                ++unknown;
            }
        } while ((uc = *++utf8_it) && uc != ' ');

        if (unknown < (latin + cyrillic - latin_cyrillic * 2) / 2 && cyrillic / 2 >= latin) {
            for (; utf8_it_start.pos() != utf8_it.pos(); ++utf8_it_start) {
                auto uc = *utf8_it_start;
                if (uc < sizeof(ascii_table_to_cyrillic) / sizeof(ascii_table_to_cyrillic[0])
                && ascii_table_to_cyrillic[uc]
                ) {
                    this->out_tmp.append(beg, e);
                    beg = char_ptr_cast(utf8_it_start.pos());
                    this->out_tmp += ascii_table_to_cyrillic[uc];
                }
                e = char_ptr_cast(utf8_it_start.pos());
            }
        }

        e = char_ptr_cast(utf8_it.pos());
    }

    if (!this->out_tmp.empty()) {
        this->out_tmp.append(beg, e-1);
        std::swap(out, this->out_tmp);
    }
}
