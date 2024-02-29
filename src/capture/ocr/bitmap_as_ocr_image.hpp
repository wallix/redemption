/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/bitmap_from_file.hpp"
#include "utils/sugar/cast.hpp"
#include "ppocr/image/coordinate.hpp"

#include <cassert>

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

    unsigned width() const { return img.cx(); }
    unsigned height() const { return img.cy(); }

    struct Color
    {
        const uint8_t * c;

        uint8_t blue()  const { return c[0]; }
        uint8_t green() const { return c[1]; }
        uint8_t red()   const { return c[2]; }
    };

    using value_type = Color;

    Color operator[](ppocr::Index p) const
    {
        auto pos = (img.cy() - p.y()) * img.line_size() - img.line_size() + p.x() * 3;
        assert(pos < img.bmp_size());
        return {img.data() + pos};
    }
};

} // namespace ocr
