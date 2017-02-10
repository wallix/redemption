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

#include "rvt/ansi_rendering.hpp"

#include "rvt/character.hpp"
#include "rvt/screen.hpp"

#include "utils/utf.hpp"

#include <ostream>
#include <functional> // std::cref

namespace rvt {

void ansi_rendering(Screen const & screen, ColorTableView palette, std::ostream & out)
{
    auto write_color = [&out, palette](char * s, char cmd, rvt::CharacterColor const & ch_color) {
        auto color = ch_color.color(palette);
        return s + std::sprintf(s, ";%c8;2;%d;%d;%d", cmd, color.red()+0, color.green()+0, color.blue()+0);
    };

    using CharacterRef = std::reference_wrapper<rvt::Character const>;
    rvt::Character const default_ch; // Default format
    CharacterRef previous_ch = std::cref(default_ch);
    constexpr std::ptrdiff_t description_ch_max_size = 80; // approximate
    char buf[4096];
    auto s = buf;
    for (auto const & line : screen.getScreenLines()) {
        for (rvt::Character const & ch : line) {
            if (!ch.isRealCharacter) {
                continue;
            }

            if (std::end(buf) - s >= description_ch_max_size) {
                out.write(buf, s - buf);
                s = buf;
            }

            bool const is_same_bg = ch.backgroundColor == previous_ch.get().backgroundColor;
            bool const is_same_fg = ch.foregroundColor == previous_ch.get().foregroundColor;
            bool const is_same_rendition = ch.rendition == previous_ch.get().rendition;
            bool const is_same_format = is_same_bg & is_same_fg & is_same_rendition;
            if (!is_same_format) {
                *s++ = '\033';
                *s++ = '[';
                *s++ = '0';
                if (!is_same_format) {
                    auto const r = ch.rendition;
                    if (bool(r & rvt::Rendition::Bold))     { *s++ = ';'; *s++ = '1'; }
                    if (bool(r & rvt::Rendition::Italic))   { *s++ = ';'; *s++ = '3'; }
                    if (bool(r & rvt::Rendition::Underline)){ *s++ = ';'; *s++ = '4'; }
                    if (bool(r & rvt::Rendition::Blink))    { *s++ = ';'; *s++ = '5'; }
                    if (bool(r & rvt::Rendition::Reverse))  { *s++ = ';'; *s++ = '6'; }
                }
                if (!is_same_fg) s = write_color(s, '3', ch.foregroundColor);
                if (!is_same_bg) s = write_color(s, '4', ch.backgroundColor);
                *s++ = 'm';
                out.write(buf, s - buf);
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
        *s++ = '\n';
    }

    if (s != buf) {
        out.write(buf, s - buf);
    }
}

}
