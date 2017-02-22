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

#include "rvt/text_rendering.hpp"

#include "rvt/character.hpp"
#include "rvt/screen.hpp"

#include "rvt/ucs.hpp"
#include "rvt/utf8_decoder.hpp"

#include <functional> // std::cref

namespace rvt {

struct Buf
{
    char buf[4096];
    char * s = buf;

    void push_ucs(ucs4_char ucs)
    {
        switch (ucs) {
            case '\\': assert(remaining() >= 2); *s++ = '\\'; *s++ = '\\'; break;
            case '"': assert(remaining() >= 2); *s++ = '\\'; *s++ = '"'; break;
            default : assert(remaining() >= 4); s += unsafe_ucs4_to_utf8(ucs, s); break;
        }
    }

    void push_character(Character const & ch, const rvt::ExtendedCharTable & extended_char_table, std::size_t max_size, std::string & out)
    {
        if (ch.isRealCharacter) {
            if (ch.is_extended()) {
                auto ucs_array = extended_char_table[ch.character];
                while (ucs_array.size() * 4u + max_size >= this->remaining()) {
                    auto const offset = this->remaining() / 4u;
                    this->push_ucs_array(ucs_array.first(offset));
                    ucs_array = ucs_array.subarray(offset);
                    this->flush(out);
                }
                this->push_ucs_array(ucs_array);
            }
            else {
                this->push_ucs(ch.character);
            }
        }
        else {
            this->push_c(' ');
        }
    }

    void push_ucs_array(ucs4_carray_view ucs_array)
    {
        for (ucs4_char ucs : ucs_array) {
            push_ucs(ucs);
        }
    }

    template<std::size_t n>
    void push_s(char const (&a)[n])
    {
        assert(remaining() >= n);
        for (std::size_t i = 0; i < n-1; ++i) {
            *s++ = a[i];
        }
    }

    void push_c(ucs4_char c) = delete; // unused
    void push_c(char c)
    {
        assert(remaining() >= 1);
        *s++ = c;
    }

    #ifndef NDEBUG
    ~Buf() {
        assert(std::uncaught_exception() || s == buf);
    }
    #endif

    void flush(std::string & out)
    {
        out.append(buf, s - buf);
        s = buf;
    }

    std::size_t remaining() const { return static_cast<std::size_t>(std::end(buf) - s); }
};

// format = "{
//      lines: %d,
//      columns: %d,
//      title: %s,
//      style: {$render $foreground $background},
//      props: "[0-7]*"
//      data: [ $line... ]
// }"
// $line = "[ {" $render? $foreground? $background? "s: %s } ]"
// $render = "r: %d
//      flags:
//      1 -> bold
//      2 -> italic
//      4 -> underline
//      8 -> blink
// $foreground = "f: $color"
// $background = "b: $color"
// $color = %d
//      decimal rgb
std::string json_rendering(
    array_view<ucs4_char const> title,
    Screen const & screen,
    ColorTableView palette
) {
    auto color2int = [](rvt::Color const & color){
        return (color.red() << 16) | (color.green() << 8) |  (color.blue() << 0);
    };

    std::string out;
    out.reserve(screen.getLines() * screen.getColumns() * 7);
    constexpr std::size_t max_size_by_loop = 111; // approximate

    Buf buf;
    buf.s += std::sprintf(buf.s, R"({"lines":%d,"columns":%d,"title":")", screen.getLines(), screen.getColumns());
    buf.push_ucs_array(title);
    buf.s += std::sprintf(buf.s, R"(","style":{"r":0,"f":%d,"b":%d},"data":[)", color2int(palette[0]), color2int(palette[1]));

    if (!screen.getColumns() || !screen.getLines()) {
        buf.push_s("]}");
        buf.flush(out);
        return out;
    }

    using CharacterRef = std::reference_wrapper<rvt::Character const>;
    rvt::Character const default_ch; // Default format
    CharacterRef previous_ch = std::cref(default_ch);

    for (auto const & line : screen.getScreenLines()) {
        buf.push_s("[[{");
        if (buf.remaining() <= max_size_by_loop) {
            buf.flush(out);
        }

        bool is_s_enable = false;
        for (rvt::Character const & ch : line) {
            if (buf.remaining() <= max_size_by_loop) {
                buf.flush(out);
            }

            constexpr auto rendition_flags
              = rvt::Rendition::Bold
              | rvt::Rendition::Italic
              | rvt::Rendition::Underline
              | rvt::Rendition::Blink;
            bool const is_same_bg = ch.backgroundColor == previous_ch.get().backgroundColor;
            bool const is_same_fg = ch.foregroundColor == previous_ch.get().foregroundColor;
            bool const is_same_rendition
              = (ch.rendition & rendition_flags) == (previous_ch.get().rendition & rendition_flags);
            bool const is_same_format = is_same_bg & is_same_fg & is_same_rendition;
            if (!is_same_format) {
                if (is_s_enable) {
                    buf.push_s("\"},{");
                }
                if (!is_same_rendition) {
                    int const r = (0
                        | (bool(ch.rendition & rvt::Rendition::Bold)      ? 1 : 0)
                        | (bool(ch.rendition & rvt::Rendition::Italic)    ? 2 : 0)
                        | (bool(ch.rendition & rvt::Rendition::Underline) ? 4 : 0)
                        | (bool(ch.rendition & rvt::Rendition::Blink)     ? 8 : 0)
                    );
                    buf.s += std::sprintf(buf.s, R"("r":%d,)", r);
                }

                if (!is_same_fg) {
                    buf.s += std::sprintf(buf.s, R"("f":%d,)", color2int(ch.foregroundColor.color(palette)));
                }
                if (!is_same_bg) {
                    buf.s += std::sprintf(buf.s, R"("b":%d,)", color2int(ch.backgroundColor.color(palette)));
                }

                is_s_enable = false;
            }

            if (!is_s_enable) {
                is_s_enable = true;
                buf.push_s(R"("s":")");
            }

            buf.push_character(ch, screen.extendedCharTable(), max_size_by_loop, out);

            previous_ch = ch;
        }

        if (is_s_enable) {
            buf.push_c('"');
        }
        buf.push_s("}]],");
    }
    --buf.s;
    buf.push_s("]}");

    buf.flush(out);
    return out;
}

std::string ansi_rendering(
    array_view<ucs4_char const> title,
    Screen const & screen,
    ColorTableView palette
) {
    auto write_color = [palette](Buf & buf, char cmd, rvt::CharacterColor const & ch_color) {
        auto color = ch_color.color(palette);
        buf.s += std::sprintf(buf.s, ";%c8;2;%d;%d;%d", cmd, color.red()+0, color.green()+0, color.blue()+0);
    };

    std::string out;
    out.reserve(screen.getLines() * screen.getColumns() * 4);

    Buf buf;
    buf.s += std::sprintf(buf.s, "\033]%*s\a", int(title.size()), title.data());

    using CharacterRef = std::reference_wrapper<rvt::Character const>;
    rvt::Character const default_ch; // Default format
    CharacterRef previous_ch = std::cref(default_ch);
    constexpr std::ptrdiff_t max_size_by_loop = 80; // approximate

    for (auto const & line : screen.getScreenLines()) {
        for (rvt::Character const & ch : line) {
            if (!ch.isRealCharacter) {
                continue;
            }

            if (buf.remaining() >= max_size_by_loop) {
                buf.flush(out);
            }

            bool const is_same_bg = ch.backgroundColor == previous_ch.get().backgroundColor;
            bool const is_same_fg = ch.foregroundColor == previous_ch.get().foregroundColor;
            bool const is_same_rendition = ch.rendition == previous_ch.get().rendition;
            bool const is_same_format = is_same_bg & is_same_fg & is_same_rendition;
            if (!is_same_format) {
                buf.push_s("\033[0");
                if (!is_same_format) {
                    auto const r = ch.rendition;
                    if (bool(r & rvt::Rendition::Bold))     { buf.push_s(";1"); }
                    if (bool(r & rvt::Rendition::Italic))   { buf.push_s(";3"); }
                    if (bool(r & rvt::Rendition::Underline)){ buf.push_s(";4"); }
                    if (bool(r & rvt::Rendition::Blink))    { buf.push_s(";5"); }
                    if (bool(r & rvt::Rendition::Reverse))  { buf.push_s(";6"); }
                }
                if (!is_same_fg) write_color(buf, '3', ch.foregroundColor);
                if (!is_same_bg) write_color(buf, '4', ch.backgroundColor);
                buf.push_c('m');
            }

            buf.push_character(ch, screen.extendedCharTable(), max_size_by_loop, out);

            previous_ch = ch;
        }
        buf.push_c('\n');
    }

    buf.flush(out);
    return out;
}

}
