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

#include "utils/tm_to_chars.hpp"
#include "utils/timestamp_tracer.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <cstring>

namespace
{
    struct Ch12x7ToPosTable
    {
        constexpr Ch12x7ToPosTable()
        {
            for (unsigned i = 0; i < 256; ++i) {
                posTable[i] = ('0' <= i && i <= '9') ? int(i - '0')
                            : ('A' <= i && i <= 'Z') ? int(i - 'A' + 14)
                            : i == '-'  ? 10
                            : i == ':'  ? 11
                            : i == 0x07 ? 13
                            : 12;
            }
        }

        int posTable[256] {};
    };

    constexpr Ch12x7ToPosTable ch12x7ToPosTable;

    constexpr const char * digits =
    "       "
    "       "
    "  XXX  "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    "  XXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    "  XX   "
    " X X   "
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    " XXXXX "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXX   "
    "    X  "
    "    X  "
    "   X   "
    "  X    "
    " X     "
    " XXXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXX  "
    "    X  "
    "    X  "
    "  XX   "
    "    X  "
    "    X  "
    " XXX   "
    "       "
    "       "
    "       "

    "       "
    "       "
    "    X  "
    "   XX  "
    "  X X  "
    " X  X  "
    " XXXXX "
    "    X  "
    "    X  "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXX  "
    " X     "
    " X     "
    " XXX   "
    "    X  "
    "    X  "
    " XXX   "
    "       "
    "       "
    "       "

    "       "
    "       "
    "   XXX "
    "  X    "
    " X     "
    " X XX  "
    " XX  X "
    " X   X "
    "  XXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXXX "
    "    X  "
    "    X  "
    "   X   "
    "  X    "
    "  X    "
    " X     "
    "       "
    "       "
    "       "

    "       "
    "       "
    "  XXX  "
    " X   X "
    " X  X  "
    "  XXX  "
    " X   X "
    " X   X "
    "  XXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    "  XXX  "
    " X   X "
    " X   X "
    "  XXXX "
    "     X "
    "    X  "
    " XXX   "
    "       "
    "       "
    "       "

    "       "
    "       "
    "       "
    "       "
    "       "
    " XXXXX "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "

    "       "
    "       "
    "       "
    "  XX   "
    "  XX   "
    "       "
    "  XX   "
    "  XX   "
    "       "
    "       "
    "       "
    "       "

    "       "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "
    "       "

    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"
    "XXXXXXX"

    "       "
    "       "
    "   X   "
    "  X X  "
    "  X X  "
    " X   X "
    " XXXXX "
    " X   X "
    "X     X"
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXX  "
    " X   X "
    " X   X "
    " XXXX  "
    " X   X "
    " X   X "
    " XXXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    "  XXXX "
    " X     "
    "X      "
    "X      "
    "X      "
    " X     "
    "  XXXX "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXX  "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    " XXXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXXX "
    " X     "
    " X     "
    " XXXX  "
    " X     "
    " X     "
    " XXXXX "
    "       "
    "       "
    "       "

    "       "
    "       "
    " XXXXX "
    " X     "
    " X     "
    " XXXX  "
    " X     "
    " X     "
    " X     "
    "       "
    "       "
    "       "

    "       "   // G
    "       "
    "  XXXX "
    " X     "
    "X      "
    "X   XX "
    "X    X "
    " X   X "
    "  XXXX "
    "       "
    "       "
    "       "

    "       "   // H
    "       "
    " X   X "
    " X   X "
    " X   X "
    " XXXXX "
    " X   X "
    " X   X "
    " X   X "
    "       "
    "       "
    "       "

    "       "   // I
    "       "
    " XXXXX "
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    " XXXXX "
    "       "
    "       "
    "       "

    "       "   // J
    "       "
    "  XXX  "
    "    X  "
    "    X  "
    "    X  "
    "    X  "
    "    X  "
    " XXX   "
    "       "
    "       "
    "       "

    "       "   // K
    "       "
    " X   X "
    " X  X  "
    " X X   "
    " XX    "
    " X X   "
    " X  X  "
    " X   X "
    "       "
    "       "
    "       "

    "       "   // L
    "       "
    " X     "
    " X     "
    " X     "
    " X     "
    " X     "
    " X     "
    " XXXXX "
    "       "
    "       "
    "       "

    "       "   // M
    "       "
    "XX  XX "
    "XX  XX "
    "XX X X "
    "X XX X "
    "X XX X "
    "X X  X "
    "X    X "
    "       "
    "       "
    "       "

    "       "   // N
    "       "
    " X   X "
    " XX  X "
    " XXX X "
    " X X X "
    " X  XX "
    " X  XX "
    " X   X "
    "       "
    "       "
    "       "

    "       "   // O
    "       "
    " XXXX  "
    "X    X "
    "X    X "
    "X    X "
    "X    X "
    "X    X "
    " XXXX  "
    "       "
    "       "
    "       "

    "       "   // P
    "       "
    " XXXX  "
    " X   X "
    " X   X "
    " XXXX  "
    " X     "
    " X     "
    " X     "
    "       "
    "       "
    "       "

    "       "   // Q
    "       "
    " XXXX  "
    "X    X "
    "X    X "
    "X    X "
    "X    X "
    "X    X "
    " XXXX  "
    "    XX "
    "     XX"
    "       "

    "       "   // R
    "       "
    "XXXX   "
    "X   X  "
    "X   X  "
    "XXXX   "
    "X  X   "
    "X   X  "
    "X    X "
    "       "
    "       "
    "       "

    "       "   // S
    "       "
    "  XXXX "
    " X     "
    " X     "
    "  XXX  "
    "     X "
    "     X "
    " XXXX  "
    "       "
    "       "
    "       "

    "       "   // T
    "       "
    "XXXXXXX"
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    "       "
    "       "
    "       "

    "       "
    "       "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    " X   X "
    "  XXX  "
    "       "
    "       "
    "       "

    "       "
    "       "
    "X     X"
    " X   X "
    " X   X "
    " X   X "
    "  X X  "
    "  X X  "
    "   X   "
    "       "
    "       "
    "       "

    "       "
    "       "
    "X     X"
    "X  X  X"
    "X  X  X"
    " XX X X"
    " XX XX "
    " X  XX "
    " X   X "
    "       "
    "       "
    "       "

    "       "
    "       "
    "X     X"
    " X   X "
    "  X X  "
    "   X   "
    "  X X  "
    " X   X "
    "X     X"
    "       "
    "       "
    "       "

    "       "
    "       "
    "X     X"
    " X   X "
    "  X X  "
    "   X   "
    "   X   "
    "   X   "
    "   X   "
    "       "
    "       "
    "       "

    "       "
    "       "
    "XXXXXX "
    "     X "
    "    X  "
    "   X   "
    "  X    "
    " X     "
    "XXXXXX "
    "       "
    "       "
    "       "
    ;
} // anonymous namespace

TimestampTracer::TimestampTracer() noexcept
{
    memset(this->timestamp_data, 0xFF, sizeof(this->timestamp_data));
    memset(this->previous_timestamp, 0x07, sizeof(this->previous_timestamp));
}

void TimestampTracer::trace(WritableImageView image_view, const tm & now) noexcept
{
    unsigned const bytes_per_pixel = safe_int(image_view.bytes_per_pixel());

    auto draw_12x7_digits = [&, this](unsigned lg_message, const char * message) noexcept {
        auto posch_12x7 = [](char ch) noexcept {
            return char_width * char_height
                 * ch12x7ToPosTable.posTable[static_cast<unsigned char>(ch)];
        };

        for (unsigned i = 0; i < lg_message; ++i) {
            char newch = message[i];
            char oldch = this->previous_timestamp[i];

            if (newch != oldch) {
                const char * pnewch = digits + posch_12x7(newch);
                const char * poldch = digits + posch_12x7(oldch);

                unsigned br_pix = 0;
                unsigned br_pixindex = i * (char_width * bytes_per_pixel);

                for (unsigned y = 0; y < char_height
                  ; ++y, br_pix += char_width
                  , br_pixindex += ts_width * bytes_per_pixel
                ) {
                    for (unsigned x = 0; x < char_width; ++x) {
                        unsigned pix = br_pix + x;
                        if (pnewch[pix] != poldch[pix]) {
                            uint8_t pixcolorcomponent = (pnewch[pix] == 'X') ? 0xFF : 0;
                            unsigned pixindex = br_pixindex + x * bytes_per_pixel;
                            memset(&this->timestamp_data[pixindex], pixcolorcomponent,
                                   bytes_per_pixel);
                        }
                    }
                }
            }
        }
    };

    const char* timezone = (daylight ? tzname[1] : tzname[0]);
    char rawdate[ts_max_length] {};
    using date_format = dateformats::YYYY_mm_dd_HH_MM_SS;
    static_assert(date_format::output_length + 10 < ts_max_length);
    char* date_p = date_format::to_chars(rawdate, now);
    *date_p++ = ' ';
    auto prefix_len = std::size_t(date_p - rawdate);
    auto timezone_len = strnlen(timezone, ts_max_length - prefix_len);
    memcpy(date_p, timezone, timezone_len);
    const int timestamp_length = int(prefix_len + timezone_len);
    draw_12x7_digits(ts_max_length, rawdate);
    memcpy(this->previous_timestamp, rawdate, ts_max_length);
    this->previous_timestamp_length = timestamp_length;

    uint8_t * tsave = this->timestamp_save;
    uint8_t * buf = image_view.mutable_data();
    size_t const width = image_view.width();
    size_t const height = image_view.height();
    size_t const rowsize = checked_int(image_view.line_size());
    size_t const n = checked_cast<size_t>(timestamp_length) * char_width * bytes_per_pixel;
    size_t const cp_n = std::min<size_t>(n, width * bytes_per_pixel);
    size_t const ny = std::min<size_t>(ts_height, height);
    for (size_t y = 0; y < ny ; ++y, buf += rowsize, tsave += n) {
        memcpy(tsave, buf, cp_n);
        memcpy(buf, this->timestamp_data + y * ts_width * bytes_per_pixel, cp_n);
    }
}

void TimestampTracer::clear(WritableImageView image_view) noexcept
{
    size_t const width = image_view.width();
    size_t const height = image_view.height();
    size_t const rowsize = checked_int(image_view.line_size());
    size_t const bytes_per_pixel = safe_int(image_view.bytes_per_pixel());
    uint8_t * buf = image_view.mutable_data();
    uint8_t const * tsave = this->timestamp_save;
    size_t const n = checked_cast<size_t>(this->previous_timestamp_length) * char_width * bytes_per_pixel;
    size_t const cp_n = std::min<size_t>(n, width);
    size_t const ny = std::min<size_t>(ts_height, height);
    for (size_t y = 0; y < ny; ++y, buf += rowsize, tsave += n) {
        memcpy(buf, tsave, cp_n);
    }
}
