/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
h
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Vnc encoder module for Cursor Pseudo Encoding
*/

#pragma once

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"

// 7.7.2   Cursor Pseudo-encoding
// ------------------------------

// A client which requests the Cursor pseudo-encoding is
// declaring that it is capable of drawing a mouse cursor
// locally. This can significantly improve perceived performance
// over slow links.

// The server sets the cursor shape by sending a pseudo-rectangle
// with the Cursor pseudo-encoding as part of an update.

// x, y : The pseudo-rectangle's x-position and y-position
// indicate the hotspot of the cursor,

// cx, cy : width and height indicate the width and height of
// the cursor in pixels.

// The data consists of width * height pixel values followed by
// a bitmask.

// PIXEL array : width * height * bytesPerPixel
// bitmask     : floor((width + 7) / 8) * height

// The bitmask consists of left-to-right, top-to-bottom
// scanlines, where each scanline is padded to a whole number of
// bytes. Within each byte the most significant bit represents
// the leftmost pixel, with a 1-bit meaning the corresponding
// pixel in the cursor is valid.

namespace VNC {
    namespace Encoder {
        class Cursor : public EncoderApi {
            const uint8_t bpp;
            const uint8_t Bpp;
            const size_t x;
            size_t y;
            const size_t cx;
            size_t cy;
            int red_shift;
            int red_max;
            int green_shift;
            int green_max;
            int blue_shift;
            int blue_max;

        public:
            VNCVerbose verbose;

            Cursor(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy,
                   int red_shift, int red_max, int green_shift, int green_max, int blue_shift, int blue_max, 
                   VNCVerbose verbose) 
                : bpp(bpp), Bpp(Bpp), x(x), y(y), cx(cx), cy(cy),
                  red_shift(red_shift), red_max(red_max), 
                  green_shift(green_shift), green_max(green_max), 
                  blue_shift(blue_shift), blue_max(blue_max)
                , verbose(verbose)
            {
            }
            
            virtual ~Cursor(){}
            
            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            EncoderState consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
                // TODO see why we get these empty rects ?
                if (this->cx <= 0 && this->cy <= 0) {
                    LOG(LOG_INFO, "empty rect %s", Rect(this->x, this->y, this->cx, this->cy));
                }

                const int sz_pixel_array = this->cx * this->cy * this->Bpp;
                const int sz_bitmask = nbbytes(this->cx) * this->cy;

                if (sz_pixel_array + sz_bitmask > 65536)
                {
                    LOG(LOG_ERR,
                        "VNC Encoding: Cursor, data buffer too small (65536 < %d)",
                        sz_pixel_array + sz_bitmask);
                    throw Error(ERR_BUFFER_TOO_SMALL);
                }

                if (buf.remaining() < sz_pixel_array + sz_bitmask)
                {
                    return EncoderState::NeedMoreData; 
                }

                auto cursor_buf = buf.av(sz_pixel_array + sz_bitmask).data();
                const uint8_t * vnc_pointer_data = cursor_buf;
                const uint8_t * vnc_pointer_mask = cursor_buf + sz_pixel_array;

                Pointer cursor;
//                LOG(LOG_INFO, "Cursor x=%zu y=%zu", x, y);
                Pointer::Hotspot hotspot(x, y);
                cursor.set_hotspot(hotspot);
                // cursor.bpp = 24;
                Pointer::CursorSize dimensions(32, 32);
                cursor.set_dimensions(dimensions);
                // a VNC pointer of 1x1 size is not visible, so a default minimal pointer (dot pointer) is provided instead
                if (this->cx == 0 || this->cy == 0) {
                    buf.advance(sz_pixel_array + sz_bitmask);
                    return EncoderState::Exit; 
                }
                else if (this->cx == 1 && this->cy == 1) {
                    // TODO Appearence of this 1x1 cursor looks broken, check what we actually get
                    memset(cursor.data, 0, sizeof(cursor.data));
                    cursor.data[2883] = 0xFF;
                    cursor.data[2884] = 0xFF;
                    cursor.data[2885] = 0xFF;
                    memset(cursor.mask, 0xFF, sizeof(cursor.mask));
                    cursor.mask[116] = 0x1F;
                    cursor.mask[120] = 0x1F;
                    cursor.mask[124] = 0x1F;
                }
                else {
                    // clear target cursor mask
                    for (size_t tmpy = 0; tmpy < 32; tmpy++) {
                        for (size_t mask_x = 0; mask_x < nbbytes(32); mask_x++) {
                            cursor.mask[tmpy*nbbytes(32) + mask_x] = 0xFF;
                        }
                    }
                    // TODO The code below is likely to explain the yellow pointer: we ask for 16 bits for VNC, but we work with cursor as if it were 24 bits. We should use decode primitives and reencode it appropriately. Cursor has the right shape because the mask used is 1 bit per pixel arrays
                    // copy vnc pointer and mask to rdp pointer and mask

                    for (int yy = 0; size_t(yy) < this->cy; yy++) {
                        for (int xx = 0 ; size_t(xx) < this->cx ; xx++){
                            if (vnc_pointer_mask[yy * nbbytes(this->cx) + xx / 8 ] & (0x80 >> (xx&7))){
                                if ((yy < 32) && (xx < 32)){
                                    cursor.mask[(31-yy) * nbbytes(32) + (xx / 8)] &= ~(0x80 >> (xx&7));
                                    int pixel = 0;
                                    for (int tt = 0 ; tt < this->Bpp; tt++){
                                        pixel += vnc_pointer_data[(yy * this->cx + xx) * this->Bpp + tt] << (8 * tt);
                                    }
                                    // TODO temporary: force black cursor
                                    int red   = (pixel >> this->red_shift) & this->red_max;
                                    int green = (pixel >> this->green_shift) & this->green_max;
                                    int blue  = (pixel >> this->blue_shift) & this->blue_max;
                                    cursor.data[((31-yy) * 32 + xx) * 3 + 0] = (red << 3) | (red >> 2);
                                    cursor.data[((31-yy) * 32 + xx) * 3 + 1] = (green << 2) | (green >> 4);
                                    cursor.data[((31-yy) * 32 + xx) * 3 + 2] = (blue << 3) | (blue >> 2);
                                }
                            }
                        }
                    }
                    /* keep these in 32x32, vnc cursor can be alot bigger */
                    /* (anyway hotspot is usually 0, 0)                   */
                    //if (x > 31) { x = 31; }
                    //if (y > 31) { y = 31; }
                }
                cursor.update_bw();
                // TODO we should manage cursors bigger then 32 x 32  this is not an RDP protocol limitation
                drawable.begin_update();
                drawable.set_pointer(cursor);
                drawable.end_update();

                buf.advance(sz_pixel_array + sz_bitmask);
                return EncoderState::Exit; 
            }
        };
    } // namespace encoder
} // namespace VNC
