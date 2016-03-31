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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_UTILS_TEXT_METRICS_HPP
#define REDEMPTION_UTILS_TEXT_METRICS_HPP

#include "utils/utf.hpp"
#include "font.hpp"

template<class NotFoundCallback>
void text_metrics(const Font & font, const char * text, int & width, int & height, NotFoundCallback not_found_callback)
{
    height = 0;
    width = 0;
    UTF8toUnicodeIterator unicode_iter(text);
    if (*unicode_iter) {
        for (; uint32_t c = *unicode_iter; ++unicode_iter) {
            const FontChar & font_item = [&, c]() -> const FontChar & {
                if (!font.glyph_defined(c) || !font.font_items[c]) {
                    not_found_callback(c);
                    return font.font_items[unsigned('?')];
                }
                return font.font_items[c];
            }();
            width += font_item.incby;
            //width += font_item.width + 2;
            height = std::max(height, font_item.height);
        }
        //width -= 1;
    }
}

inline void text_metrics(const Font & font, const char * text, int & width, int & height)
{
    text_metrics(font, text, width, height, [](uint32_t){});
}

#endif
