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

#include "gdi/multiline_text_metrics.hpp"
#include "core/font.hpp"
#include "utils/utf.hpp"

namespace
{
    struct WordInfo
    {
        int w = 0;
        uint8_t const* p;

        WordInfo(const Font& font, uint8_t const* p)
        {
            for (;;) {
                switch (*p) {
                    case ' ':
                    case '\n':
                    case '\0':
                        this->p = p;
                        return;
                    default:
                        UTF8toUnicodeIterator iter(p);
                        FontCharView const* font_item = &font.glyph_or_unknown(*iter);
                        w += font_item->offsetx + font_item->incby;
                        p = iter.pos();
                }
            }
        }
    };

    template<class NewLine>
    void multi_textmetrics_impl(
        const Font& font, uint8_t const* unicode_text,
        const int max_width, const int space_w,
        NewLine new_line
    ) {
        _start:

        int w = 0;

        // left spaces
        auto* start = unicode_text;
        for (;;) {
            switch (*unicode_text) {
                case ' ':
                    ++unicode_text;
                    continue;

                case '\n':
                    new_line(start, start, 0);
                    ++unicode_text;
                    goto _start; /* NOLINT */

                case '\0':
                    return;

                default:;
            }
            break;
        }

        w = (unicode_text - start) * space_w;

        // first word
        {
            WordInfo winfo{font, unicode_text};
            if (max_width < w + winfo.w) {
                if (w) {
                    new_line(start, start, 0);
                    start = unicode_text;
                    w = 0;
                }

                // word too long
                if (max_width < winfo.w) {
                    for (;;) {
                        switch (*unicode_text) {
                            case ' ':
                                goto _words; /* NOLINT */

                            case '\n':
                                if (w) {
                                    new_line(start, unicode_text, w);
                                }
                                ++unicode_text;
                                goto _start; /* NOLINT */

                            case '\0':
                                if (w) {
                                    new_line(start, unicode_text, w);
                                }
                                return ;

                            default:
                                UTF8toUnicodeIterator iter(unicode_text);
                                FontCharView const* font_item = &font.glyph_or_unknown(*iter);
                                int cw = font_item->offsetx + font_item->incby;
                                if (max_width < w + cw) {
                                    if (w) {
                                        new_line(start, unicode_text, w);
                                        w = 0;
                                    }
                                    else {
                                        new_line(start, iter.pos(), cw);
                                        unicode_text = iter.pos();
                                    }
                                    start = unicode_text;
                                    continue;
                                }
                                w += cw;
                                unicode_text = iter.pos();
                        }
                    }
                }
            }

            w += winfo.w;
            unicode_text = winfo.p;
        }

        _words:

        for (;;) {
            auto* end_word = unicode_text;

            for (;;) {
                switch (*unicode_text) {
                    case ' ':
                        ++unicode_text;
                        continue;

                    case '\n':
                        new_line(start, end_word, w);
                        ++unicode_text;
                        goto _start; /* NOLINT */

                    case '\0':
                        new_line(start, end_word, w);
                        return;

                    default:;
                }
                break;
            }

            WordInfo winfo{font, unicode_text};
            if (max_width >= w + winfo.w) {
                w += winfo.w;
                unicode_text = winfo.p;
            }
            else {
                new_line(start, end_word, w);
                goto _start; /* NOLINT */
            }
        }
    }
} // namespace

namespace gdi
{

MultiLineTextMetrics::MultiLineTextMetrics(
    const Font& font, const char* unicode_text, unsigned max_width)
{
    FontCharView const* font_item = &font.glyph_or_unknown(' ');
    const int space_w = font_item->offsetx + font_item->incby;

    uint8_t const* p = byte_ptr(unicode_text).as_u8p();

    int nb_line = 0;
    int nb_byte = 0;
    multi_textmetrics_impl(font, p, int(max_width), space_w,
        [&](auto* p, auto* e, int /*w*/){
            ++nb_line;
            nb_byte += e - p;
        });

    this->size_ = nb_line;
    this->lines_.reset(new Line[nb_line /* NOLINT */
        // char buffer
        + (nb_line * 4 + nb_byte) / sizeof(Line) + sizeof(Line)]);
    Line* pline = this->lines_.get();
    char* s = reinterpret_cast<char*>(pline + nb_line); /* NOLINT */

    multi_textmetrics_impl(font, p, int(max_width), space_w,
        [&](auto* p, auto* e, int w){
            pline->text = s;
            pline->cx = w;
            ++pline;
            memcpy(s, p, e-p);
            s += e-p;
            memset(s, 0, 4);
            s += 4;
        });
}

} // namespace gdi
