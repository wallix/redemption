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

#include "gdi/image_frame_api.hpp"
#include "utils/image_view.hpp"
#include "utils/sugar/cast.hpp"

#include <memory>
#include <cstring>


class VideoCropper : public gdi::ImageFrameApi
{
public:
    VideoCropper(ImageFrameApi& image_frame, Rect cropper) noexcept
    : image_frame(image_frame)
    , in_rect(cropper)
    {
        auto img = this->image_frame.prepare_image_frame();
        cropper = this->in_rect.intersect(img.width(), img.height());
        unsigned rowsize = cropper.cx * underlying_cast(img.bytes_per_pixel());
        this->alloc_buffer(rowsize * cropper.cy);
    }

    void set_crop(Rect cropper) noexcept
    {
        this->in_rect = cropper;
    }

    Rect get_rect() noexcept
    {
        return this->in_rect;
    }

    WritableImageView prepare_image_frame() override
    {
        auto img = this->image_frame.prepare_image_frame();

        if (this->in_rect.cy == img.width() && this->in_rect.cx == img.height()) {
            return img;
        }

        // image not modified, returns previous image
        const unsigned remote_last_update_index =
            this->image_frame.get_last_update_index();
        if (remote_last_update_index == this->last_update_index) {
            assert(this->previous_image_view.data());
            return this->previous_image_view;
        }
        this->last_update_index = remote_last_update_index;

        Rect cropper = this->in_rect.intersect(img.width(), img.height());
        unsigned rowsize = cropper.cx * underlying_cast(img.bytes_per_pixel());
        uint8_t* out_bmpdata_tmp = this->alloc_buffer(rowsize * cropper.cy);

        uint8_t const* in_bmpdata_tmp
          = img.data()
          + cropper.y * img.line_size()
          + cropper.x * underlying_cast(img.bytes_per_pixel());

        for (uint16_t i = 0; i < cropper.cy; ++i) {
            std::memcpy(out_bmpdata_tmp, in_bmpdata_tmp, rowsize);

            in_bmpdata_tmp  += img.line_size();
            out_bmpdata_tmp += rowsize;
        }

        this->previous_image_view = WritableImageView{
            this->out_bmpdata.get(),
            cropper.cx,
            cropper.cy,
            rowsize,
            img.bytes_per_pixel(),
            img.storage_type()
        };

        return this->previous_image_view;
    }

    unsigned int get_last_update_index() const noexcept override
    {
        return this->last_update_index + 1u;
    }

private:
    uint8_t* alloc_buffer(std::size_t len)
    {
        if (this->max_buffer_len < len) {
            this->out_bmpdata = std::make_unique<uint8_t[]>(len);
            this->max_buffer_len = len;
        }
        return this->out_bmpdata.get();
    }

private:
    gdi::ImageFrameApi& image_frame;
    std::unique_ptr<uint8_t[]> out_bmpdata;
    WritableImageView previous_image_view = WritableImageView::create_null_view();
    Rect in_rect;
    unsigned max_buffer_len = 0;
    unsigned int last_update_index = 0;
};
