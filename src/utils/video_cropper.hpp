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

#include "core/RDP/RDPDrawable.hpp"

class VideoCropper
{
public:
    static constexpr const unsigned int bytes_per_pixel = 3;

public:
    virtual ~VideoCropper() = default;

    virtual unsigned width() const = 0;

    virtual unsigned height() const = 0;

    virtual const uint8_t* data() const = 0;

    virtual uint8_t* first_pixel() const = 0;

    virtual unsigned int rowsize() const = 0;

    virtual size_t pix_len() const = 0;

    virtual void prepare_video_frame() = 0;
};

class DummyVideoCropper : public VideoCropper
{
private:
    const unsigned int in_width;
    const unsigned int in_height;

    const unsigned int in_rowsize;

    const uint8_t* in_bmpdata;

public:
    DummyVideoCropper(unsigned int in_width, unsigned int in_height,
        const uint8_t* in_bmpdata)
    : in_width(in_width)
    , in_height(in_height)
    , in_rowsize(in_width * VideoCropper::bytes_per_pixel)
    , in_bmpdata(in_bmpdata) {}

    unsigned width() const override {
        return this->in_width;
    }

    unsigned height() const override {
        return this->in_height;
    }

    const uint8_t* data() const override {
        return this->in_bmpdata;
    }

    uint8_t* first_pixel() const override {
        return const_cast<uint8_t*>(this->in_bmpdata);
    }

    unsigned int rowsize() const override {
        return this->in_rowsize;
    }

    size_t pix_len() const override {
        return this->in_rowsize * this->in_height;
    }

    void prepare_video_frame() override {}
};

class EffectiveVideoCropper : public VideoCropper
{
private:
    const unsigned int in_width;
    const unsigned int in_height;

    const unsigned int in_rowsize;

    const uint8_t* in_bmpdata;

    const unsigned int x;
    const unsigned int y;

    const unsigned int out_width;
    const unsigned int out_height;

    const unsigned int out_rowsize;

    std::unique_ptr<uint8_t[]> out_bmpdata;

    const uint8_t* in_bmpdata_effective;

public:
    EffectiveVideoCropper(unsigned int in_width, unsigned int in_height,
        const uint8_t* in_bmpdata, unsigned int x, unsigned int y,
        unsigned int out_width, unsigned int out_height)
    : in_width(in_width)
    , in_height(in_height)
    , in_rowsize(in_width * VideoCropper::bytes_per_pixel)
    , in_bmpdata(in_bmpdata)
    , x(x)
    , y(y)
    , out_width(out_width)
    , out_height(out_height)
    , out_rowsize(this->out_width * VideoCropper::bytes_per_pixel)
    , out_bmpdata(
          std::make_unique<uint8_t[]>(this->out_rowsize * out_height))
    , in_bmpdata_effective(
          this->in_bmpdata +
          this->y * this->in_rowsize +
          this->x * VideoCropper::bytes_per_pixel) {
        LOG(LOG_INFO, "out_width=%u out_height=%u", out_width, out_height);
    }

    unsigned width() const override {
        return this->out_width;
    }

    unsigned height() const override {
        return this->out_height;
    }

    const uint8_t* data() const override {
        return this->out_bmpdata.get();
    }

    uint8_t* first_pixel() const override {
        return this->out_bmpdata.get();
    }

    unsigned int rowsize() const override {
        return this->out_rowsize;
    }

    size_t pix_len() const override {
        return this->out_rowsize * this->out_height;
    }

    void prepare_video_frame() override {
        const uint8_t* in_bmpdata_tmp  = this->in_bmpdata_effective;
              uint8_t* out_bmpdata_tmp = this->out_bmpdata.get();

        for (unsigned int i = 0; i < this->out_height; ++i) {
            ::memcpy(out_bmpdata_tmp, in_bmpdata_tmp, this->out_rowsize);

            in_bmpdata_tmp  += this->in_rowsize;
            out_bmpdata_tmp += this->out_rowsize;
        }
    }
};