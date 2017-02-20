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
            default : assert(remaining() >= 4); s += ucs4_to_utf8(ucs, s); break;
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

static int color2int(rvt::Color const & color)
{ return (color.red() << 16) | (color.green() << 8) |  (color.blue() << 0); }

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
                    int const r = static_cast<int>(0
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

            if (ch.isRealCharacter) {
                if (ch.is_extended()) {
                    auto ucs_array = screen.extendedCharTable()[ch.character];
                    while (ucs_array.size() * 4u + max_size_by_loop >= buf.remaining()) {
                        auto const offset = buf.remaining() / 4u;
                        buf.push_ucs_array(ucs_array.first(offset));
                        ucs_array = ucs_array.subarray(offset);
                        buf.flush(out);
                    }
                    buf.push_ucs_array(ucs_array);
                }
                else {
                    buf.push_ucs(ch.character);
                }
            }
            else {
                buf.push_c(' ');
            }

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

}
