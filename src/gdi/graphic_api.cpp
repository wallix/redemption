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

#include "gdi/graphic_api.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/utf.hpp"

namespace
{
    template<class Getc>
    void textmetrics_impl(
        const Font & font, const char * unicode_text, int & width, int & height, Getc getc)
    {
        UTF8toUnicodeIterator unicode_iter(unicode_text);
        FontCharView const* font_item = nullptr;
        for (; uint32_t c = getc(unicode_iter); ++unicode_iter) {
            font_item = &font.glyph_or_unknown(c);
            width += font_item->offsetx + font_item->incby;
        }
        height = font.max_height();
    }

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

            auto ws = (unicode_text - end_word) * space_w;

            WordInfo winfo{font, unicode_text};
            if (max_width >= w + winfo.w + ws) {
                w += winfo.w + ws;
                unicode_text = winfo.p;
            }
            else {
                new_line(start, end_word, w);
                goto _start; /* NOLINT */
            }
        }
    }

    gdi::NullGraphic gd;
} // namespace

namespace gdi
{

gdi::GraphicApi & null_gd() noexcept
{
    return gd;
}

TextMetrics::TextMetrics(const Font & font, const char * unicode_text)
{
    textmetrics_impl(
        font, unicode_text, this->width, this->height,
        [](UTF8toUnicodeIterator & it){ return *it; });
}

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
            pline->width = w;
            ++pline;
            memcpy(s, p, e-p);
            s += e-p;
            memset(s, 0, 4);
            s += 4;
        });
}

// TODO implementation of the server_draw_text function below is a small subset of possibilities text can be packed (detecting duplicated strings). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)
// TODO: is it still used ? If yes move it somewhere else. Method from internal mods ?
void server_draw_text(
    GraphicApi & drawable, Font const & font,
    int16_t x, int16_t y, const char * text,
    RDPColor fgcolor, RDPColor bgcolor,
    ColorCtx color_ctx,
    Rect clip
) {
    // BUG TODO static not const is a bad idea
    static GlyphCache mod_glyph_cache;

    UTF8toUnicodeIterator unicode_iter(text);

    int16_t endx = clip.eright();

    if (*unicode_iter && x <= clip.x) {
        do {
            const uint32_t charnum = *unicode_iter;

            FontCharView const* font_item = font.glyph_at(charnum);
            if (!font_item) {
                LOG(LOG_WARNING, "server_draw_text() - character not defined >0x%02x<", charnum);
                font_item = &font.unknown_glyph();
            }

            auto nextx = x + font_item->offsetx + font_item->incby;
            if (nextx > clip.x) {
                break;
            }

            x = nextx;
            ++unicode_iter;
        } while (*unicode_iter);
    }

    while (*unicode_iter) {
        int total_width = 0;
        uint8_t data[256];
        data[1] = 0;
        auto data_begin = std::begin(data);
        const auto data_end = std::end(data)-2;

        const int cacheId = 7;
        while (data_begin != data_end && *unicode_iter && x+total_width <= endx) {
            const uint32_t charnum = *unicode_iter;
            ++unicode_iter;

            int cacheIndex = 0;
            FontCharView const* font_item = font.glyph_at(charnum);
            if (!font_item) {
                LOG(LOG_WARNING, "server_draw_text() - character not defined >0x%02x<", charnum);
                font_item = &font.unknown_glyph();
            }

            const GlyphCache::t_glyph_cache_result cache_result =
                mod_glyph_cache.add_glyph(*font_item, cacheId, cacheIndex);
            (void)cache_result; // supress warning

            *data_begin++ = cacheIndex;
            *data_begin++ += font_item->offsetx;
            data_begin[1] = font_item->incby;
            total_width += font_item->offsetx + font_item->incby;
        }

        Rect bk(x, y, total_width + 2, font.max_height());

        RDPGlyphIndex glyphindex(
            cacheId,            // cache_id
            0x03,               // fl_accel
            0x0,                // ui_charinc
            1,                  // f_op_redundant,
            fgcolor,            // BackColor (text color)
            bgcolor,            // ForeColor (color of the opaque rectangle)
            bk,                 // bk
            bk,                 // op
            // brush
            RDPBrush(0, 0, 3, 0xaa,
                byte_ptr_cast("\xaa\x55\xaa\x55\xaa\x55\xaa\x55")),
            x,                  // glyph_x
            y,                  // glyph_y
            data_begin - data,  // data_len in bytes
            data                // data
        );

        drawable.draw(glyphindex, clip, color_ctx, mod_glyph_cache);

        if (x+total_width <= endx) {
            break;
        }
        x += total_width - 1;
    }
}

} // namespace gdi
