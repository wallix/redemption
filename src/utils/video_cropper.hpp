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
#include "utils/bitfu.hpp"
#include "utils/sugar/not_null_ptr.hpp"

#include <memory>
#include <cstring>


class VideoCropper : public gdi::ImageFrameApi
{
private:
    static constexpr const unsigned int bytes_per_pixel = 3;

    not_null_ptr<gdi::ImageFrameApi> image_frame_api_ptr;

    unsigned int in_width;
    unsigned int in_height;

    unsigned int in_rowsize;

    const uint8_t* in_bmpdata;

    unsigned int x;
    unsigned int y;

    unsigned int out_width;
    unsigned int out_height;

    unsigned int out_rowsize;

    std::unique_ptr<uint8_t[]> out_bmpdata;

    const uint8_t* in_bmpdata_effective;

    unsigned int last_update_index = 0;

    VideoCropper(
        ImageFrameApi& imageFrameApi, ImageView const & image_view,
        /* TODO x, y, out_width, out_height -> Rect*/
        unsigned int x, unsigned int y,
        unsigned int out_width, unsigned int out_height)
    : image_frame_api_ptr(&imageFrameApi)
    , in_width(image_view.width())
    , in_height(image_view.height())
    , in_rowsize(image_view.width() * VideoCropper::bytes_per_pixel) /* TODO image_view.rowsize() ? */
    , in_bmpdata(image_view.data())
    , x(x)
    , y(y)
    , out_width(out_width)
    , out_height(out_height)
    , out_rowsize(this->out_width * VideoCropper::bytes_per_pixel)
    , in_bmpdata_effective(
          this->in_bmpdata +
          this->y * this->in_rowsize +
          this->x * VideoCropper::bytes_per_pixel) {
        if ((this->out_width != this->in_width) ||
            (this->out_height != this->in_height)) {
            this->out_bmpdata = std::make_unique<uint8_t[]>(this->out_rowsize * out_height);
        }
    }

public:
    VideoCropper(
        ImageFrameApi& imageFrameApi,
        /* TODO x, y, out_width, out_height -> Rect*/
        unsigned int x, unsigned int y,
        unsigned int out_width, unsigned int out_height)
    : VideoCropper(imageFrameApi, imageFrameApi.get_mutable_image_view(), x, y, out_width, out_height)
    {}

    void resize(ImageFrameApi& imageFrameApi) {
        this->image_frame_api_ptr = &imageFrameApi;

        ImageView const & image_view = imageFrameApi.get_mutable_image_view();

        this->in_width   = image_view.width();
        this->in_height  = image_view.height();
        this->in_rowsize = image_view.width() * VideoCropper::bytes_per_pixel;  /* TODO image_view.rowsize() ? */
        this->in_bmpdata = image_view.data();

        Rect rect = Rect(this->x, this->y, this->out_width, this->out_height).intersect(this->in_width, this->in_height);

        this->reset(rect.x, rect.y, rect.cx, rect.cy);

        this->in_bmpdata_effective =
            this->in_bmpdata +
              this->y * this->in_rowsize +
              this->x * VideoCropper::bytes_per_pixel;
    }

private:
    template<class ImgView>
    ImgView create_image_view() const
    {
        uint8_t * data = this->out_bmpdata.get();

        if (this->out_width == this->in_width && this->out_height == this->in_height) {
            data = const_cast<uint8_t*>(this->in_bmpdata); /*NOLINT*/
        }

        return ImgView{
            data,
            static_cast<uint16_t>(this->out_width),
            static_cast<uint16_t>(this->out_height),
            this->out_rowsize,
            ConstImageDataView::BytesPerPixel(this->bytes_per_pixel),
            ConstImageDataView::Storage::TopToBottom
        };
    }

public:
    ImageView get_mutable_image_view() override
    {
        return this->create_image_view<ImageView>();
    }

    ConstImageView get_image_view() const override
    {
        return this->create_image_view<ConstImageView>();
    }

    void prepare_image_frame() override {
        if ((this->out_width == this->in_width) &&
            (this->out_height == this->in_height)) {
            return;
        }

        const unsigned int remote_last_update_index =
            this->image_frame_api_ptr->get_last_update_index();
        if (remote_last_update_index == this->last_update_index) {
            return;
        }
        this->last_update_index = remote_last_update_index;

        const uint8_t* in_bmpdata_tmp  = this->in_bmpdata_effective;
              uint8_t* out_bmpdata_tmp = this->out_bmpdata.get();

        for (unsigned int i = 0; i < this->out_height; ++i) {
            ::memcpy(out_bmpdata_tmp, in_bmpdata_tmp, this->out_rowsize);

            in_bmpdata_tmp  += this->in_rowsize;
            out_bmpdata_tmp += this->out_rowsize;
        }
    }

    unsigned int get_last_update_index() const noexcept override {
        return this->last_update_index;
    }

    using gdi::ImageFrameApi::reset;

    // returns true if size of image frame has changed
    /* TODO x, y, out_width, out_height -> Rect*/
    bool reset(unsigned int x, unsigned int y,
               unsigned int out_width, unsigned int out_height) override {
        unsigned int const old_out_rowsize = this->out_rowsize;
        unsigned int const old_out_height  = this->out_height;

        bool result = false;

        this->x = x;
        this->y = y;
        this->out_width = out_width;
        this->out_height = out_height;
        this->out_rowsize = this->out_width * VideoCropper::bytes_per_pixel;
        if ((this->out_width != this->in_width) ||
            (this->out_height != this->in_height)) {
            if (((old_out_rowsize * old_out_height) < (this->out_rowsize * this->out_height)) || !this->out_bmpdata) {
                this->out_bmpdata = std::make_unique<uint8_t[]>(this->out_rowsize * this->out_height);

                result = true;
            }
            else if (old_out_rowsize != this->out_rowsize) {
                result = true;
            }
        }
        else {
            this->out_bmpdata = nullptr;
        }

        this->in_bmpdata_effective =
            this->in_bmpdata +
            this->y * this->in_rowsize +
            this->x * VideoCropper::bytes_per_pixel;

        return result;
    }

    Rect get_rect() const override {
        return Rect(this->x, this->y, this->out_width, this->out_height);
    }
};
