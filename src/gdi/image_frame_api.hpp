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
#include <cstddef>


namespace gdi {

struct ConstImageDataView
{
protected:
    using size_t = std::size_t;

public:
    explicit ConstImageDataView(
        uint8_t const * data,
        uint16_t width,
        uint16_t height,
        size_t rowsize,
        uint8_t bytes_per_pixel
    ) noexcept
    : data_(data)
    , rowsize_(rowsize)
    , width_(width)
    , height_(height)
    , bytes_per_pixel_(bytes_per_pixel)
    {}

    uint8_t const * data() const noexcept  { return this->data_; }
    uint16_t width() const noexcept { return this->width_; }
    uint16_t height() const noexcept { return this->height_; }
    size_t bytes_per_pixel() const noexcept { return this->bytes_per_pixel_; }
    size_t size() const noexcept { return this->width_ * this->height_; }
    size_t rowsize() const noexcept { return this->rowsize_; }
    size_t pix_len() const noexcept { return this->rowsize_ * this->height_; }

    const uint8_t * end_data() const noexcept
    { return this->data_ + this->height_ * this->rowsize_; }

    const uint8_t * data(uint16_t x, uint16_t y) const noexcept
    { return this->data_ + this->offset(x, y); }

    size_t offset(uint16_t x, uint16_t y) const noexcept
    { return (y * this->width_ + x) * this->bytes_per_pixel_; }

private:
    uint8_t const * data_;
    size_t rowsize_;
    uint16_t width_;
    uint16_t height_;
    uint8_t bytes_per_pixel_;
};

struct ImageDataView : ConstImageDataView
{
    explicit ImageDataView(
        uint8_t * data,
        uint16_t width,
        uint16_t height,
        size_t rowsize,
        uint8_t bytes_per_pixel
    ) noexcept
    : ConstImageDataView(data, width, height, rowsize, bytes_per_pixel)
    {}

    uint8_t * first_pixel() const noexcept { return const_cast<uint8_t*>(this->data()); }
};

struct ImageFrameApi
{
    using ImageView = ImageDataView;
    using ConstImageView = ConstImageDataView;

    virtual ~ImageFrameApi() = default;

    virtual ImageView get_mutable_image_view() = 0;
    virtual ConstImageView get_image_view() const = 0;

    virtual void prepare_image_frame() = 0;

    virtual unsigned int get_last_update_index() const = 0;

//    virtual void reset_to_origin() = 0;

    virtual void reset(unsigned int x, unsigned int y,
               unsigned int out_width, unsigned int out_height) = 0;
};

inline ConstImageDataView get_image_view(ImageFrameApi const & image_frame)
{
    return image_frame.get_image_view();
}

inline ImageDataView get_mutable_image_view(ImageFrameApi & image_frame)
{
    return image_frame.get_mutable_image_view();
}

}
