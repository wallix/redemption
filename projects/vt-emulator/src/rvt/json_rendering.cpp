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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#include "rvt/json_rendering.hpp"

#include "rvt/character.hpp"
#include "rvt/screen.hpp"

#include "utils/utf.hpp"

#include <ostream>
#include <functional> // std::cref

namespace rvt {

// format = "{ lines: %d, columns: %d, screen: [ $line... ] }"
// $line = "[ {" $render? $foreground? $background? "s: %s } ]"
// $render = "r: 'b'? 'i'? 'u'? 'l'?"
//      b -> bold
//      i -> italic
//      u -> underline
//      l -> blink
// $foreground = "f: $color"
// $background = "b: $color"
// $color = %d
//      decimal rgb
void json_rendering(Screen const & screen, ColorTableView palette, std::ostream & out)
{
    char buf[4096];
    auto s = buf;
    s += std::sprintf(s, R"({"lines":%d,"columns":%d,"screen":[)", screen.getLines(), screen.getColumns());

    if (!screen.getColumns() || !screen.getLines()) {
        *s++ = ']';
        *s++ = '}';
        out.write(buf, s - buf);
        return ;
    }

    auto color2int = [palette](rvt::CharacterColor const & ch_color) {
        auto const color = ch_color.color(palette);
        return (color.red() << 16) | (color.green() << 8) |  (color.blue() << 0);
    };

    using CharacterRef = std::reference_wrapper<rvt::Character const>;
    rvt::Character const default_ch; // Default format
    CharacterRef previous_ch = std::cref(default_ch);
    constexpr std::ptrdiff_t max_size_by_loop = 80; // approximate

    for (auto const & line : screen.getScreenLines()) {
        *s++ = '[';
        *s++ = '[';
        *s++ = '{';
        *s++ = '"';
        *s++ = 's';
        *s++ = '"';
        *s++ = ':';
        *s++ = '"';
        for (rvt::Character const & ch : line) {
            if (!ch.isRealCharacter) {
                continue;
            }

            if (std::end(buf) - s >= max_size_by_loop) {
                out.write(buf, s - buf);
                s = buf;
            }

            bool const is_same_bg = ch.backgroundColor == previous_ch.get().backgroundColor;
            bool const is_same_fg = ch.foregroundColor == previous_ch.get().foregroundColor;
            bool const is_same_rendition = ch.rendition == previous_ch.get().rendition;
            bool const is_same_format = is_same_bg & is_same_fg & is_same_rendition;
            if (!is_same_format) {
                *s++ = '"';
                *s++ = '}';
                *s++ = ',';
                *s++ = '{';
                if (!is_same_rendition) {
                    *s++ = '"';
                    *s++ = 'r';
                    *s++ = '"';
                    *s++ = ':';
                    *s++ = '"';
                    auto const r = ch.rendition;
                    if (bool(r & rvt::Rendition::Bold))     { *s++ = 'b'; }
                    if (bool(r & rvt::Rendition::Italic))   { *s++ = 'i'; }
                    if (bool(r & rvt::Rendition::Underline)){ *s++ = 'u'; }
                    if (bool(r & rvt::Rendition::Blink))    { *s++ = 'l'; }
                    *s++ = '"';
                    *s++ = ',';
                }

                if (!is_same_fg) { s += std::sprintf(s, R"("f":%d,)", color2int(ch.foregroundColor)); }
                if (!is_same_bg) { s += std::sprintf(s, R"("b":%d,)", color2int(ch.backgroundColor)); }

                *s++ = '"';
                *s++ = 's';
                *s++ = '"';
                *s++ = ':';
                *s++ = '"';
            }

            if (ch.is_extended()) {
                for (rvt::ucs4_char ucs : screen.extendedCharTable()[ch.character]) {
                    s += ucs4_to_utf8(ucs, s);
                }
            }
            else {
                s += ucs4_to_utf8(ch.character, s);
            }

            previous_ch = ch;
        }
        *s++ = '"';
        *s++ = '}';
        *s++ = ']';
        *s++ = ']';
        *s++ = ',';
    }
    --s;
    *s++ = ']';
    *s++ = '}';

    out.write(buf, s - buf);
}

}
