/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/bitmap_from_file.hpp"

namespace ocr
{

class BitmapAsOcrImage
{
    Bitmap img;

public:
    explicit BitmapAsOcrImage(char const* filename)
    : img(BitsPerPixel::BitsPP24, bitmap_from_file_impl(filename, BGRColor()))
    {}

    bool is_valid() const { return img.is_valid(); }

    unsigned width() const { return img.cx() + 2; /* add a border */ }
    unsigned height() const { return img.cy() + 2; /* add a border */ }

    struct Color
    {
        const uint8_t * c;

        uint8_t blue()  const { return c[0]; }
        uint8_t green() const { return c[1]; }
        uint8_t red()   const { return c[2]; }
    };

    using value_type = Color;

    Color operator()(unsigned row, unsigned col) const
    {
        // when border -> black color
        if (row > 0 && col > 0 && col < width() - 1 && row < height() - 1) {
            --row;
            --col;
            return {img.data() + (img.cy() - row) * img.line_size() + col * 3};
        }
        return Color{reinterpret_cast<uint8_t const*>("\0\0\0")};
    }
};

} // namespace ocr
