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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Raphael ZHOU
*/

#pragma once

#include <cstdint>

#include "utils/bitfu.hpp"

class BGRPalette;

struct ConstImageDataView
{
protected:
    using size_t = std::size_t;

public:
    enum class BitsPerPixel : uint8_t {};
    enum class BytesPerPixel : uint8_t {};

    enum class Storage : bool { BottomToTop, TopToBottom };

    explicit ConstImageDataView(
        uint8_t const * data,
        uint16_t width,
        uint16_t height,
        size_t line_size,
        BytesPerPixel bytes_per_pixel,
        Storage storage,
        BGRPalette const * palette = nullptr
    ) noexcept
    : data_(data)
    , rowsize_(line_size)
    , width_(width)
    , height_(height)
    , bits_per_pixel_(static_cast<uint8_t>(bytes_per_pixel) * 8)
    , bytes_per_pixel_(static_cast<uint8_t>(bytes_per_pixel))
    , storage_(storage)
    , palette_(palette)
    {}

    explicit ConstImageDataView(
        uint8_t const * data,
        uint16_t width,
        uint16_t height,
        size_t line_size,
        BitsPerPixel bits_per_pixel,
        Storage storage,
        BGRPalette const * palette = nullptr
    ) noexcept
    : data_(data)
    , rowsize_(line_size)
    , width_(width)
    , height_(height)
    , bits_per_pixel_(static_cast<uint8_t>(bits_per_pixel))
    , bytes_per_pixel_(nbbytes(static_cast<uint8_t>(bits_per_pixel)))
    , storage_(storage)
    , palette_(palette)
    {}

    uint8_t const * data()      const noexcept { return this->data_; }
    uint16_t width()            const noexcept { return this->width_; }
    uint16_t height()           const noexcept { return this->height_; }
    uint8_t bytes_per_pixel()   const noexcept { return this->bytes_per_pixel_; }
    uint8_t bits_per_pixel()    const noexcept { return this->bits_per_pixel_; }
    size_t size()               const noexcept { return this->width_ * this->height_; }
    size_t line_size()          const noexcept { return this->rowsize_; }
    size_t pix_len()            const noexcept { return this->rowsize_ * this->height_; }
    Storage storage_type()      const noexcept { return this->storage_; }
    BGRPalette const& palette() const noexcept { return *this->palette_; }

    const uint8_t * end_data() const noexcept
    { return this->data_ + this->height_ * this->rowsize_; }

    const uint8_t * data(uint16_t x, uint16_t y) const noexcept
    { return this->data_ + this->offset(x, y); }

    size_t offset(uint16_t x, uint16_t y) const noexcept
    { return y * this->rowsize_ + x * this->bytes_per_pixel_; }

private:
    uint8_t const * data_;
    size_t rowsize_;
    uint16_t width_;
    uint16_t height_;
    uint8_t bits_per_pixel_;
    uint8_t bytes_per_pixel_;
    Storage storage_;
    BGRPalette const * palette_;
};

struct MutableImageDataView : ConstImageDataView
{
    explicit MutableImageDataView(
        uint8_t * data,
        uint16_t width,
        uint16_t height,
        size_t line_size,
        BytesPerPixel bytes_per_pixel,
        Storage storage,
        BGRPalette const * palette = nullptr
    ) noexcept
    : ConstImageDataView(data, width, height, line_size, bytes_per_pixel, storage, palette)
    {}

    explicit MutableImageDataView(
        uint8_t * data,
        uint16_t width,
        uint16_t height,
        size_t line_size,
        BitsPerPixel bits_per_pixel,
        Storage storage,
        BGRPalette const * palette = nullptr
    ) noexcept
    : ConstImageDataView(data, width, height, line_size, bits_per_pixel, storage, palette)
    {}

    uint8_t * mutable_data() const noexcept { return const_cast<uint8_t*>(this->data()); }
};
