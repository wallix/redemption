/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/font.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/stream.hpp"
#include "utils/log.hpp"

#include <algorithm>
#include <string_view>

#include <cerrno>
#include <cstring>

namespace
{
    // height aligned of 4 bytes
    constexpr uint8_t unknown_glyph_data[12]{
        0b01111'000,
        0b10001'000,
        0b00001'000,
        0b00011'000,
        0b00110'000,
        0b00100'000,
        0b00100'000,
        0b00000'000,
        0b00100'000,
        0b00100'000,
    };

    constexpr FontCharView default_unknown{2, 0, 7, 5, 10, unknown_glyph_data};

    constexpr uint32_t contiguous_unicode_max = 0xD7FC;
} // anonymous namespace

REDEMPTION_NOINLINE
Font::FontCharElement Font::get_higher_item(uint32_t unicode) const noexcept
{
    auto* p = this->unicode_values;
    auto* end = p + this->nb_random_item;
    auto* p2 = std::lower_bound(p, end, unicode);
    if (p2 != end && *p2 == unicode) {
        auto* item = this->font_items + this->nb_contiguous_item + (p2 - p);
        return {*item, true};
    }
    return {unknown_item, false};
}

/*
- the RBF2 file always begins by the label "RBF2"
- Police global informations are :
    * version (u32)
    * name (u8[32]) (ex : Deja Vu Sans)
    * fontsize (u16)
    * fontstyle (u16) (always '1')
    * max ascent (u16)
    * max descent (u16)
    * number of glyph (u32)
    * unicode max (u32)
    * total data len: sum of aligned_of_4(glyph_data_len) (u32)
    * replacement glyph (assume uni < CONTIGUOUS_LIMIT)
    * glyph in range [CHARSET_START..CHARSET_END]

- Individual glyph informations are :
    ? when uni < CONTIGUOUS_LIMIT
      * has_glyph (u8 = 1 or 0)
    ? when has_glyph = 1 or when uni < CONTIGUOUS_LIMIT
      ? when uni >= CONTIGUOUS_LIMIT
        * unicode value (u32)
      * offsetx (u8)
      * offsety (u8)
      * incby (u8)
      * cx (s8)
      * cy (s8)
      * data (the bitmap representing the sketch of the glyph, one bit by pixel,
              0 for background, 1 for foreground)
*/
FontData::FontData(char const * file_path)
  : max_height_(default_unknown.offsety + default_unknown.height + 1)
  , unknown_item(default_unknown)
{
    LOG(LOG_INFO, "Reading font file %s", file_path);

    unique_fd const ufd{open(file_path, O_RDONLY)};
    if (!ufd) {
        LOG(LOG_ERR, "Font: can't open font file [%s] for reading: %s",
            file_path, strerror(errno));
        return ;
    }

    size_t const stream_buf_sz = 32 * 1024;
    char stream_buf[stream_buf_sz];
    InStream stream({stream_buf, 0});

    auto prepare_stream = [&](size_t const len, uint32_t index) -> bool {
        assert(len < stream_buf_sz / 2);
        size_t const remaining = stream.in_remain();
        if (remaining < len) {
            memmove(stream_buf, stream.get_current(), remaining);
            ssize_t n = ::read(ufd.fd(), stream_buf + remaining, stream_buf_sz - remaining);
            ssize_t newlen = n + static_cast<ssize_t>(remaining);
            if (REDEMPTION_UNLIKELY(newlen < static_cast<ssize_t>(len))) {
                if (n == 0) {
                    return false;
                }
                if (n < 0) {
                    LOG(LOG_ERR, "Font: error reading font file [%s]: %s",
                        file_path, strerror(errno));
                    return false;
                }
                LOG(LOG_WARNING, "Font: file %s defines glyphs up to %u, file looks broken",
                    file_path, index);
                return false;
            }
            stream = InStream({stream_buf, static_cast<std::size_t>(newlen)});
        }
        return true;
    };


    uint32_t number_of_glyph;
    uint32_t unicode_max;
    uint32_t total_data_len;

    // Read header
    {
        size_t const header_size = 4 + 4 + 32 + 2 + 2 + 2 + 4 + 4 + 4;

        if (!prepare_stream(header_size, -1u)) {
            return ;
        }

        auto magic_number = stream.in_skip_bytes(4).as_chars();
        if (magic_number.as<std::string_view>() != std::string_view("RBF2")) {
            LOG(LOG_ERR, "Font: error reading font file [%s]: bad magic number ('%.*s', expected 'RBF2')",
                file_path, 4, magic_number.data());
            return ;
        }

        auto version = stream.in_uint32_le();
        auto fontname = stream.in_skip_bytes(32);
        auto fontsize = stream.in_uint16_le();
        auto fontstyle = stream.in_uint16_le();
        auto max_ascent = stream.in_uint16_le();
        auto max_descent = stream.in_uint16_le();
        number_of_glyph = stream.in_uint32_le();
        unicode_max = stream.in_uint32_le();
        total_data_len = stream.in_uint32_le();

        this->max_height_ = max_ascent + max_descent;

        LOG(LOG_INFO, "Font: version: %u  name: '%.*s'  size: %u  style: %u  max_ascent: %u  max_descent: %u  nbglyph: %u  unicode_max = %u  total_data: %u",
            version, int(fontname.size()), fontname.data(), fontsize, fontstyle,
            max_ascent, max_descent, number_of_glyph, unicode_max, total_data_len);
    }

    if (total_data_len > 32 * 1024 * 1024) {
        LOG(LOG_ERR, "Font: error reading font file [%s]: total size of glyphs too large (%u)",
            file_path, total_data_len);
        return ;
    }

    if (number_of_glyph == 0) {
        return ;
    }

    this->data_glyphs = std::make_unique<uint8_t[]>(total_data_len);
    this->font_items = std::make_unique<FontCharView[]>(number_of_glyph);
    auto* font_char_p = this->font_items.get();

    auto* data = this->data_glyphs.get();
    memset(data, 0, total_data_len);
    auto usable_data_len = total_data_len;

    auto extract_glyph = [&](uint32_t index) {
        auto const offsetx = stream.in_sint8();
        auto const offsety = stream.in_sint8();
        auto const incby = stream.in_uint8();
        auto const cx = stream.in_uint8();
        auto const cy = stream.in_uint8();

        uint16_t const glyph_data_size = nbbytes(cx) * cy;
        uint16_t const data_size = align4(glyph_data_size);

        // LOG(LOG_DEBUG,
        //     "index: 0x%x  offsetx: %d  offsety: %d  incby: %u  cx: %u  cy: %u  data_size: %d",
        //     index, offsetx, offsety, incby, cx, cy, data_size);

        if (REDEMPTION_UNLIKELY(data_size > usable_data_len)) {
            LOG(LOG_ERR, "Font: error reading font file [%s at glyph %u]:"
                " width(%u)*height(%u) too large (total_data_len = %u)",
                file_path, index, cx, cy, total_data_len);
            return false;
        }

        if (REDEMPTION_UNLIKELY(!prepare_stream(glyph_data_size, index))) {
            return false;
        }

        stream.in_copy_bytes(data, glyph_data_size);
        *font_char_p++ = FontCharView{offsetx, offsety, incby, cx, cy, data};
        data += data_size;
        usable_data_len -= data_size;

        return true;
    };

    auto extract_contigous_glyph = [&](uint32_t index) {
        if (REDEMPTION_UNLIKELY(!prepare_stream(1, index))) {
            return false;
        }

        if (!stream.in_uint8()) {
            // LOG(LOG_DEBUG, "index: 0x%x  NonPrintable", index);
            *font_char_p++ = this->unknown_item;
            return true;
        }

        size_t const description_size = 1 + 1 + 1 + 1 + 1;
        if (REDEMPTION_UNLIKELY(!prepare_stream(description_size, index))) {
            return false;
        }

        return extract_glyph(index);
    };

    uint32_t index = 0;

    // extract replacement char
    bool status = extract_contigous_glyph(-1u);
    if (this->font_items.get() != font_char_p) {
        this->unknown_item = this->font_items[0];
        --number_of_glyph;
        --font_char_p;
    }

    if (status) {
        auto contiguous_end = std::max(std::min(unicode_max, contiguous_unicode_max), uint32_t(32));
        auto index_end = std::min(number_of_glyph, contiguous_end - uint32_t(32));
        for (; index < index_end; ++index) {
            status = extract_contigous_glyph(index);
            if (REDEMPTION_UNLIKELY(!status)) {
                break;
            }
            ++this->nb_contiguous_item;
        }
    }

    if (status) {
        this->unicode_values = std::make_unique<uint32_t[]>(number_of_glyph - index);
        auto* unicode_p = this->unicode_values.get();
        uint32_t previous_unicode32 = 0;
        for (; index < number_of_glyph; ++index) {
            size_t const description_size = 4 + 1 + 1 + 1 + 1 + 1;
            status = prepare_stream(description_size, index);
            if (REDEMPTION_UNLIKELY(!status)) {
                break;
            }

            auto unicode32 = stream.in_uint32_le();

            if (REDEMPTION_UNLIKELY(previous_unicode32 >= unicode32)) {
                LOG(LOG_ERR, "Font: error reading font file [%s]: unicode values are unsorted"
                    "(previous: %u  current: %u)",
                    file_path, previous_unicode32, unicode32);
                status = false;
                break;
            }

            status = extract_glyph(index);
            if (REDEMPTION_UNLIKELY(!status)) {
                break;
            }

            *unicode_p++ = unicode32;
            ++this->nb_random_item;
        }
    }

    if (!status && (stream.in_remain() || 0 != ::read(ufd.fd(), stream_buf, 1))) {
        LOG(LOG_ERR, "Font: error reading font file [%s]: Bad number of glyph", file_path);
    }
}
