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

#include "utils/image_view.hpp"

#include <cstdint>
#include <ctime>


class TimestampTracer
{
public:
    explicit TimestampTracer() noexcept;

    void trace(WritableImageView image_view, const tm & now) noexcept;
    void clear(WritableImageView image_view) noexcept;

private:
    enum {
        char_width  = 7,
        char_height = 12,

        ts_max_length = 32,

        ts_width = ts_max_length * char_width,
        ts_height = char_height,
        size_str_timestamp = ts_max_length + 1
    };

    uint8_t timestamp_save[ts_width * ts_height * /*DrawableImplPrivate::bytes_per_pixel*/4];
    uint8_t timestamp_data[ts_width * ts_height * /*DrawableImplPrivate::bytes_per_pixel*/4];
    char previous_timestamp[size_str_timestamp];
    int previous_timestamp_length;
};
