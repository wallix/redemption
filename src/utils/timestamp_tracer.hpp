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

#include "utils/sugar/numerics/safe_conversions.hpp"
#include "gdi/image_frame_api.hpp"

#include <cstring>
#include <ctime>


class TimestampTracer {
    enum {
        char_width  = 7,
        char_height = 12
    };

    enum {
        ts_max_length = 32
    };

    enum {
        ts_width = ts_max_length * char_width,
        ts_height = char_height,
        size_str_timestamp = ts_max_length + 1
    };

    uint8_t timestamp_save[ts_width * ts_height * /*DrawableImplPrivate::bytes_per_pixel*/4];
    uint8_t timestamp_data[ts_width * ts_height * /*DrawableImplPrivate::bytes_per_pixel*/4];
    char previous_timestamp[size_str_timestamp];
    uint8_t previous_timestamp_length;

    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel;
    uint8_t*     pixels;
    unsigned int rowsize;

public:
    explicit TimestampTracer(const MutableImageDataView & image_view)
    : previous_timestamp_length(0)
    , width(image_view.width())
    , height(image_view.height())
    , bytes_per_pixel(safe_int(image_view.bytes_per_pixel()))
    , pixels(image_view.mutable_data())
    , rowsize(image_view.line_size()) {
        memset(this->timestamp_data, 0xFF, sizeof(this->timestamp_data));
        memset(this->previous_timestamp, 0x07, sizeof(this->previous_timestamp));
    }

private:
    int _posch_12x7(char ch) {
        return char_width * char_height *
        ( ('0' <= ch && ch <= '9') ? ch - '0'
        : ('A' <= ch && ch <= 'Z') ? ch - 'A' + 14
        : ch == '-'   ? 10
        : ch == ':'   ? 11
        : ch == 0x07  ? 13
        :               12);
    }

    void draw_12x7_digits(uint8_t * rgbpixbuf, unsigned width, unsigned lg_message,
            const char * message, const char * old_message) {
        const char * digits =
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
        for (size_t i = 0; i < lg_message; ++i) {
            char newch = message[i];
            char oldch = old_message[i];

            if (newch != oldch) {
                const char * pnewch = digits + this->_posch_12x7(newch);
                const char * poldch = digits + this->_posch_12x7(oldch);

                unsigned br_pix = 0;
                unsigned br_pixindex = i * (char_width * this->bytes_per_pixel);

                for (size_t y = 0; y < char_height; ++y, br_pix += char_width, br_pixindex += width * this->bytes_per_pixel) {
                    for (size_t x = 0; x <  char_width; ++x) {
                        unsigned pix = br_pix + x;
                        if (pnewch[pix] != poldch[pix]) {
                            uint8_t pixcolorcomponent = (pnewch[pix] == 'X') ? 0xFF : 0;
                            unsigned pixindex = br_pixindex + x * this->bytes_per_pixel;
                            memset(&rgbpixbuf[pixindex], pixcolorcomponent, this->bytes_per_pixel);
                        }
                    }
                }
            }
        }
    }

public:
    void trace(const tm & now)
    {
        const char* timezone = (daylight ? tzname[1] : tzname[0]);
        const uint8_t timestamp_length = 20 + strlen(timezone);
        char rawdate[size_str_timestamp] {};
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wformat-truncation")
        snprintf(rawdate, timestamp_length + 1, "%4d-%02d-%02d %02d:%02d:%02d %s",
            now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
            now.tm_hour, now.tm_min, now.tm_sec, timezone);
        REDEMPTION_DIAGNOSTIC_POP
        this->draw_12x7_digits(this->timestamp_data, ts_width, size_str_timestamp - 1, rawdate,
            this->previous_timestamp);
        memcpy(this->previous_timestamp, rawdate, size_str_timestamp);
        this->previous_timestamp_length = timestamp_length;

        uint8_t * tsave = this->timestamp_save;
        uint8_t * buf = this->pixels;
        const size_t n = timestamp_length * char_width * this->bytes_per_pixel;
        const size_t cp_n = std::min<size_t>(n, this->width * this->bytes_per_pixel);
        const size_t ny = std::min<size_t>(ts_height, this->height);
        for (size_t y = 0; y < ny ; ++y, buf += this->rowsize, tsave += n) {
            memcpy(tsave, buf, cp_n);
            memcpy(buf, this->timestamp_data + y * ts_width * this->bytes_per_pixel, cp_n);
        }
    }

    void clear()
    {
        const uint8_t * tsave = this->timestamp_save;
        uint8_t * buf = this->pixels;
        const size_t n = this->previous_timestamp_length * char_width * this->bytes_per_pixel;
        const size_t cp_n = std::min<size_t>(n, this->width);
        const size_t ny = std::min<size_t>(ts_height, this->height);
        for (size_t y = 0; y < ny; ++y, buf += this->rowsize, tsave += n) {
            memcpy(buf, tsave, cp_n);
        }
    }
};
