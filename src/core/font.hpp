/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
              Raphael Zhou, Jonathan Poelen
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Font header file
*/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include <cstring>

#include "utils/bitfu.hpp"
#include "utils/sugar/not_null_ptr.hpp"


struct FontCharView
{
    int16_t offsetx = 0;
    int16_t offsety = 0;
    int16_t incby = 0;
    int16_t right = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t const* data = nullptr;

    FontCharView(
        int16_t offsetx, int16_t offsety, int16_t incby, int16_t right,
        uint16_t width, uint16_t height, uint8_t const* data) noexcept
    : offsetx{offsetx}
    , offsety{offsety}
    , incby{incby}
    , right{right}
    , width{width}
    , height{height}
    , data{data}
    {}

    FontCharView() = default;

    explicit operator bool () const noexcept {
        return bool(this->incby);
    }

    uint16_t datasize() const noexcept
    {
        return align4(nbbytes(this->width) * this->height);
    }

    /* compare the two font items returns true if they match */
    bool item_compare(FontCharView const & glyph) const noexcept
    {
        return glyph
            && (this->offsetx == glyph.offsetx)
            && (this->offsety == glyph.offsety)
            && (this->width == glyph.width)
            && (this->height == glyph.height)
            && (this->right == glyph.right)
            && (0 == memcmp(this->data, glyph.data, glyph.datasize()));
    }

    //void show() {
    //          uint8_t   fc_bit_mask        = 128;
    //    const uint8_t * fc_data            = this->data.get();
    //    const bool      skip_padding_pixel = (this->width % 8);
    //
    //    for (int y = 0; y < this->height; y++)
    //    {
    //        for (int x = 0; x < this->width; x++)
    //        {
    //            if (fc_bit_mask & (*fc_data)) {
    //                printf("X");
    //            }
    //            else {
    //                printf(".");
    //            }
    //
    //            fc_bit_mask >>= 1;
    //            if (!fc_bit_mask)
    //            {
    //                fc_data++;
    //                fc_bit_mask = 128;
    //            }
    //        }
    //
    //        if (skip_padding_pixel) {
    //            fc_data++;
    //            fc_bit_mask = 128;
    //            printf("_");
    //        }
    //        printf("\n");
    //    }
    //    printf("
    //}
}; // END STRUCT - FontCharView


struct FontChar
{
    int16_t offsetx = 0;
    int16_t offsety = 0;
    int16_t incby = 0;
    int16_t right = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    std::unique_ptr<uint8_t[]> data; // PERF mini_vector<32>

    // TODO data really aligned ?
    FontChar(std::unique_ptr<uint8_t[]> data, int16_t offsetx, int16_t offsety, uint16_t width, uint16_t height, int16_t incby)
        : offsetx{offsetx}
        , offsety{offsety}
        , incby{incby}
        , width{width}
        , height{height}
        , data{std::move(data)}
    {
    }

    FontChar(int16_t offsetx, int16_t offsety, uint16_t width, uint16_t height, int16_t incby)
        : offsetx{offsetx}
        , offsety{offsety}
        , incby{incby}
        , width{width}
        , height{height}
        , data{std::make_unique<uint8_t[]>(this->datasize())}
    {
    }

    explicit FontChar(FontCharView const& font_char_view)
        : offsetx{font_char_view.offsetx}
        , offsety{font_char_view.offsety}
        , incby{font_char_view.incby}
        , right{font_char_view.right}
        , width{font_char_view.width}
        , height{font_char_view.height}
        , data{std::make_unique<uint8_t[]>(font_char_view.datasize())}
    {
        assert(font_char_view.datasize() == this->datasize());
        memcpy(this->data.get(), font_char_view.data, font_char_view.datasize());
    }

    FontChar() = default;

    FontChar(FontChar && other) = default;
    FontChar(FontChar const & other) = delete;
    FontChar & operator=(FontChar &&) = default;
    FontChar & operator=(FontChar const &) = delete;

    FontCharView to_view() const noexcept
    {
        return FontCharView{
            this->offsetx, this->offsety, this->incby, this->right,
            this->width, this->height, this->data.get()
        };
    }

    void * operator new (size_t) = delete;

    FontChar clone() const {
        auto ptr = std::make_unique<uint8_t[]>(this->datasize());
        memcpy(ptr.get(), this->data.get(), this->datasize());
        return FontChar(std::move(ptr), this->offsetx, this->offsety, this->width, this->height, this->incby);
    }

    explicit operator bool () const noexcept {
        return bool(this->incby);
    }

    uint16_t datasize() const noexcept
    {
        return align4(nbbytes(this->width) * this->height);
    }

    /* compare the two font items returns true if they match */
    bool item_compare(FontChar const & glyph) const noexcept
    {
        return glyph
            && (this->offsetx == glyph.offsetx)
            && (this->offsety == glyph.offsety)
            && (this->width == glyph.width)
            && (this->height == glyph.height)
            && (0 == memcmp(this->data.get(), glyph.data.get(), glyph.datasize()));
    }

    //void show() {
    //          uint8_t   fc_bit_mask        = 128;
    //    const uint8_t * fc_data            = this->data.get();
    //    const bool      skip_padding_pixel = (this->width % 8);
    //
    //    for (int y = 0; y < this->height; y++)
    //    {
    //        for (int x = 0; x < this->width; x++)
    //        {
    //            if (fc_bit_mask & (*fc_data)) {
    //                printf("X");
    //            }
    //            else {
    //                printf(".");
    //            }
    //
    //            fc_bit_mask >>= 1;
    //            if (!fc_bit_mask)
    //            {
    //                fc_data++;
    //                fc_bit_mask = 128;
    //            }
    //        }
    //
    //        if (skip_padding_pixel) {
    //            fc_data++;
    //            fc_bit_mask = 128;
    //            printf("_");
    //        }
    //        printf("\n");
    //    }
    //    printf("
    //}
}; // END STRUCT - FontChar


// TODO NUM_GLYPHS is misleading it's actually number of glyph in font. Using it to set size of a static array is quite dangerous as we shouldn't have to change code whenever we change font file.


/*
  The fv1 files contain
  Font File Header (just one)
    FNT1       4 bytes
    Font Name  32 bytes
    Font Size  2 bytes
    Font Style 2 bytes
    Pad        8 bytes
  Font Data (repeat for each glyph)
    Width      2 bytes
    Height     2 bytes
    Baseline   2 bytes
    Offset     2 bytes
    Incby      2 bytes
    Pad        6 bytes
    Glyph Data var, see FONT_DATASIZE macro
*/

struct Font
{
    Font() = default;

    /// \param file_path  path to the font definition file (*.rbf)
    explicit Font(const char * file_path, bool spark_view_specific_glyph_width = false)
    : spark_view_specific_glyph_width_(spark_view_specific_glyph_width)
    {
        this->load_from_file(file_path);
        this->font_items.shrink_to_fit();
        if (auto item = this->glyph_at('?')) {
            this->unknown_item = *item;
        }
    }

    explicit Font(std::string const & file_path, bool spark_view_specific_glyph_width = false)
    : Font(file_path.c_str(), spark_view_specific_glyph_width)
    {}

    bool is_loaded() const
    {
        return this->font_items.size();
    }

    uint16_t size() const noexcept {
        return this->size_;
    }

    uint16_t max_height() const noexcept {
        return this->max_height_;
    }

    uint16_t style() const noexcept {
        return this->style_;
    }

    char const * name() const noexcept {
        return this->name_;
    }

    bool spark_view_specific_glyph_width() const noexcept {
        return this->spark_view_specific_glyph_width_;
    }

    bool glyph_defined(uint32_t charnum) const
    {
        return (size_t(charnum - 32u) < this->font_items.size())
            && bool(this->font_items[charnum - 32u]);
    }

    FontCharView const & glyph_or_unknown(uint32_t charnum) const
    {
        return this->glyph_defined(charnum)
             ? this->font_items[charnum - 32u]
             : this->unknown_glyph();
    }

    FontCharView const * glyph_at(uint32_t charnum) const
    {
        return this->glyph_defined(charnum)
             ? &this->font_items[charnum - 32u]
             : nullptr;
    }

    FontCharView const & unknown_glyph() const noexcept
    {
        return this->unknown_item;
    }

private:
    static FontCharView default_unknown_glyph() noexcept;

    void load_from_file(const char * file_path);

    std::unique_ptr<uint8_t[]> data_glyphs;
    std::vector<FontCharView> font_items;
    FontCharView unknown_item = default_unknown_glyph();
    uint16_t size_ = 0;
    uint16_t max_height_ = 0;
    uint16_t style_ = 0;
    char name_[32] {};

    bool spark_view_specific_glyph_width_ = false;
};
