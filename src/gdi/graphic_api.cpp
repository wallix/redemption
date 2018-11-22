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
#include "utils/sugar/splitter.hpp"

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
} // namespace

namespace gdi {

TextMetrics::TextMetrics(const Font & font, const char * unicode_text)
{
    textmetrics_impl(
        font, unicode_text, this->width, this->height,
        [](UTF8toUnicodeIterator & it){ return *it; });
}

inline std::string & operator += (std::string & s, range<char const*> r)
{
    s.insert(s.end(), r.begin(), r.end());
    return s;
}

MultiLineTextMetrics::MultiLineTextMetrics(const Font& font, const char* unicode_text, unsigned int line_spacing,
    int max_width, std::string& out_multiline_string_ref)
{
    out_multiline_string_ref.clear();

    int number_of_lines = 1;

    TextMetrics const tt(font, " ");

    int height_max = tt.height;
    const int white_space_width = tt.width;

    int cumulative_width = 0;

    for (auto parameter : get_line(unicode_text, ' ')) {
        if (parameter.empty()) {
            continue;
        }

        const int part_width = [&]{
            int w = 0;
            int h = 0;
            textmetrics_impl(
                font, parameter.begin(), w, h,
                [&](UTF8toUnicodeIterator & it){
                    return it.pos() <= byte_ptr_cast(parameter.end())
                        ? *it : 0u;
                });

            height_max = std::max(height_max, h);

            return w;
        }();

        if (cumulative_width) {
            auto to_av = [](range<char const*> r) { return make_array_view(r.begin(), r.end()); };

            if (cumulative_width + white_space_width + part_width > max_width) {
                str_append(out_multiline_string_ref, "<br>", to_av(parameter));

                cumulative_width = part_width;

                this->width = std::max(this->width, cumulative_width);

                number_of_lines++;
            }
            else {
                str_append(out_multiline_string_ref, ' ', to_av(parameter));

                cumulative_width += (white_space_width + part_width);

                this->width = std::max(this->width, cumulative_width);
            }
        }
        else {
            out_multiline_string_ref += parameter;

            cumulative_width = part_width;

            this->width = std::max(this->width, cumulative_width);
        }
    }

    this->height = height_max * number_of_lines + line_spacing * (number_of_lines - 1);
}

MultiLineTextMetricsEx::MultiLineTextMetricsEx(const Font& font, const char* unicode_text, unsigned int line_spacing,
    int max_width, std::string& out_multiline_string_ref)
{
    out_multiline_string_ref.clear();

    const char   delimiter[]      = "<br>";
    const size_t delimiter_length = sizeof(delimiter) - 1;

    std::string s(unicode_text);
    std::string temp_str;

    auto get_mltm = [this, &temp_str, &font, max_width, &out_multiline_string_ref, delimiter, line_spacing]
            (const char *s) {
        MultiLineTextMetrics mltm(font, s, line_spacing, max_width, temp_str);

        if (!out_multiline_string_ref.empty()) {
            out_multiline_string_ref += delimiter;
        }
        out_multiline_string_ref += temp_str;

        if (this->height) {
            this->height += line_spacing;
        }

        this->width   = std::max(this->width, mltm.width);
        this->height += mltm.height;
    };

    auto start = 0;
    auto end = s.find(delimiter);
    while (end != std::string::npos)
    {
        get_mltm(s.substr(start, end - start).c_str());

        start = end + delimiter_length;
        end = s.find(delimiter, start);
    }

    get_mltm(s.substr(start, end).c_str());
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

    while (*unicode_iter) {
        int total_width = 0;
        uint8_t data[256];
        data[1] = 0;
        auto data_begin = std::begin(data);
        const auto data_end = std::end(data)-2;

        const int cacheId = 7;
        FontCharView const* font_item = nullptr;
        while (data_begin != data_end && *unicode_iter) {
            const uint32_t charnum = *unicode_iter;
            ++unicode_iter;

            int cacheIndex = 0;
            font_item = font.glyph_at(charnum);
            if (!font_item) {
                LOG(LOG_WARNING, "server_draw_text() - character not defined >0x%02x<", charnum);
                font_item = &font.unknown_glyph();
            }

            // TODO avoid passing parameters by reference to get results
            const GlyphCache::t_glyph_cache_result cache_result =
                mod_glyph_cache.add_glyph(FontChar(*font_item), cacheId, cacheIndex);
            (void)cache_result; // supress warning

            *data_begin++ = cacheIndex;
            *data_begin++ += font_item->offsetx;
            data_begin[1] = font_item->incby;
            total_width += font_item->offsetx + font_item->incby;
        }

        const Rect bk(x, y, total_width+1, font.max_height());

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

        x += total_width - 1;

        drawable.draw(glyphindex, clip, color_ctx, mod_glyph_cache);
    }
}

} // namespace gdi
