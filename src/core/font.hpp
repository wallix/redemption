/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <memory>
#include <utility>

#include <cassert>
#include <cstring>

#include "utils/bitfu.hpp"

struct FontCharView
{
    int8_t offsetx = 0;
    int8_t offsety = 0;
    uint8_t incby = 0;
    uint8_t width = 0;
    uint8_t height = 0;
    uint8_t const* data = nullptr;

    explicit operator bool () const noexcept
    {
        return bool(this->data);
    }

    [[nodiscard]] uint16_t datasize() const noexcept
    {
        return align4(nbbytes(this->width) * this->height);
    }

    [[nodiscard]] uint8_t const* data_ptr() const noexcept
    {
        return this->data;
    }
};


struct RDPFontChar
{
    int16_t offsetx = 0;
    int16_t offsety = 0;
    int16_t incby = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    std::unique_ptr<uint8_t[]> data; // PERF mini_vector<32> or allocator

    RDPFontChar(std::unique_ptr<uint8_t[]> data, int16_t offsetx, int16_t offsety, uint16_t width, uint16_t height, int16_t incby)
        : offsetx{offsetx}
        , offsety{offsety}
        , incby{incby}
        , width{width}
        , height{height}
        , data{std::move(data)}
    {
    }

    RDPFontChar(int16_t offsetx, int16_t offsety, uint16_t width, uint16_t height, int16_t incby)
        : offsetx{offsetx}
        , offsety{offsety}
        , incby{incby}
        , width{width}
        , height{height}
        , data{std::make_unique<uint8_t[]>(this->datasize())}
    {
    }

    explicit RDPFontChar(FontCharView const& font_char_view)
        : offsetx{font_char_view.offsetx}
        , offsety{font_char_view.offsety}
        , incby{font_char_view.incby}
        , width{font_char_view.width}
        , height{font_char_view.height}
        , data{std::make_unique<uint8_t[]>(font_char_view.datasize())}
    {
        assert(font_char_view.datasize() == this->datasize());
        memcpy(this->data.get(), font_char_view.data, font_char_view.datasize());
    }

    RDPFontChar() = default;

    RDPFontChar(RDPFontChar && other) = default;
    RDPFontChar(RDPFontChar const & other) = delete;
    RDPFontChar & operator=(RDPFontChar &&) = default;
    RDPFontChar & operator=(RDPFontChar const &) = delete;

    void * operator new (size_t) = delete;

    [[nodiscard]] RDPFontChar clone() const
    {
        auto ptr = std::make_unique<uint8_t[]>(this->datasize());
        memcpy(ptr.get(), this->data.get(), this->datasize());
        return RDPFontChar(std::move(ptr), this->offsetx, this->offsety, this->width, this->height, this->incby);
    }

    explicit operator bool () const noexcept
    {
        return bool(this->data);
    }

    [[nodiscard]] uint16_t datasize() const noexcept
    {
        return align4(nbbytes(this->width) * this->height);
    }

    [[nodiscard]] uint8_t const* data_ptr() const noexcept
    {
        return this->data.get();
    }
}; // END STRUCT - FontChar

/* compare the two font items returns true if they match */
template<class FontChar1, class FontChar2>
bool font_item_equal(FontChar1 const& font_char1, FontChar2 const& font_char2) noexcept
{
    return (font_char1.offsetx == font_char2.offsetx)
        && (font_char1.offsety == font_char2.offsety)
        && (font_char1.width == font_char2.width)
        && (font_char1.height == font_char2.height)
        && (0 == memcmp(font_char1.data_ptr(), font_char2.data_ptr(), font_char2.datasize()));
}


// template<class FontChar1>
// void show(FontChar1 const& fc)
// {
//     uint8_t fc_bit_mask = 128;
//     const uint8_t * fc_data = fc.data_ptr();
//     const bool skip_padding_pixel = (fc.width % 8);
//
//     for (unsigned y = 0; y < fc.height; y++) {
//         for (unsigned x = 0; x < fc.width; x++) {
//             if (fc_bit_mask & (*fc_data)) {
//                 printf("X");
//             }
//             else {
//                 printf(".");
//             }
//
//             fc_bit_mask >>= 1;
//             if (!fc_bit_mask)
//             {
//                 fc_data++;
//                 fc_bit_mask = 128;
//             }
//         }
//
//         if (skip_padding_pixel) {
//             fc_data++;
//             fc_bit_mask = 128;
//             printf("_");
//         }
//         printf("\n");
//     }
//     printf("");
// }


struct Font
{
    Font() = default;

    Font(FontCharView const* font_items,
         uint32_t nb_contiguous_item,
         uint32_t nb_random_item,
         uint16_t max_height,
         uint32_t const* unicode_values,
         FontCharView unknown_item)
    : font_items(font_items)
    , nb_contiguous_item(nb_contiguous_item)
    , nb_random_item(nb_random_item)
    , max_height_(max_height)
    , unicode_values(unicode_values)
    , unknown_item(unknown_item)
    {}

    uint16_t max_height() const noexcept
    {
        return this->max_height_;
    }

    FontCharView const & unknown_glyph() const noexcept
    {
        return this->unknown_item;
    }

    struct [[nodiscard]] FontCharElement
    {
        FontCharView const& view;
        bool is_valid;
    };

    FontCharElement item(uint32_t unicode) const noexcept
    {
        if (unicode >= 32u) {
            if (unicode - 32u < nb_contiguous_item) {
                return {this->font_items[unicode - 32u], true};
            }
            return this->get_higher_item(unicode);
        }
        return {this->unknown_item, false};
    }

private:
    FontCharElement get_higher_item(uint32_t unicode) const noexcept;

    FontCharView const* font_items {};
    uint32_t nb_contiguous_item {};
    uint32_t nb_random_item {};
    uint16_t max_height_ {};
    uint32_t const* unicode_values {};
    FontCharView unknown_item {};
};


struct FontData
{
    FontData() = default;

    /// \param file_path  path to the font definition file (*.rbf)
    explicit FontData(char const * file_path);

    bool is_loaded() const noexcept
    {
        return this->nb_contiguous_item || this->nb_random_item;
    }

    Font font() const noexcept
    {
        return Font(
            font_items.get(),
            nb_contiguous_item,
            nb_random_item,
            max_height_,
            unicode_values.get(),
            unknown_item
        );
    }

private:
    std::unique_ptr<FontCharView[]> font_items;
    uint32_t nb_contiguous_item = 0;
    uint32_t nb_random_item = 0;
    uint16_t max_height_ = 0;
    std::unique_ptr<uint32_t[]> unicode_values;
    std::unique_ptr<uint8_t[]> data_glyphs;
    FontCharView unknown_item;
};
