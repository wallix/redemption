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

   Vnc encoder module for Hextile bitmaps
*/

#pragma once

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"

//    7.6.5   Hextile Encoding

//    Hextile is a variation on the RRE idea. Rectangles are split up into 16x16 tiles,
// allowing the dimensions of the subrectangles to be specified in 4 bits each, 16 bits in total.
// The rectangle is split into tiles starting at the top left going in left-to-right,
// top-to-bottom order. The encoded contents of the tiles simply follow one another in the
// predetermined order. If the width of the whole rectangle is not an exact multiple of 16
// then the width of the last tile in each row will be correspondingly smaller. Similarly
// if the height of the whole rectangle is not an exact multiple of 16 then the height of
// each tile in the final row will also be smaller.

//    Each tile is either encoded as raw pixel data, or as a variation on RRE. Each tile has a
// background pixel value, as before. The background pixel value does not need to be explicitly
// specified for a given tile if it is the same as the background of the previous tile. However
// the background pixel value may not be carried over if the previous tile was raw. If all of the
// subrectangles of a tile have the same pixel value, this can be specified once as a foreground
// pixel value for the whole tile. As with the background, the foreground pixel value can be left
// unspecified, meaning it is carried over from the previous tile. The foreground pixel value may
// not be carried over if the previous tile was raw or had the SubrectsColored bit set. It may,
// however, be carried over from a previous tile with the AnySubrects bit clear, as long as that
// tile itself carried over a valid foreground from its previous tile.

//    So the data consists of each tile encoded in order. Each tile begins with a subencoding
// type byte, which is a mask made up of a number of bits:

//    No. of bytes      Type
//    1                  U8      

//    subencoding-mask:
//      1   Raw
//      2   BackgroundSpecified
//      4   ForegroundSpecified
//      8   AnySubrects
//      16  SubrectsColoured

//    If the Raw bit is set then the other bits are irrelevant; 
// width * height pixel values follow (where width and height are the width
// and height of the tile). 

// Otherwise the other bits in the mask are as follows:

//    BackgroundSpecified

//        If set, a pixel value follows which specifies the background colour for this tile:
//        No. of bytes      Type          Description
//        bytesPerPixel     PIXEL       background-pixel-value

//        The first non-raw tile in a rectangle must have this bit set. 
// If this bit isn't set then the background is the same as the last tile.

//    ForegroundSpecified

//        If set, a pixel value follows which specifies the foreground colour
//        to be used for all subrectangles in this tile:

//        No. of bytes      Type        Description
//        bytesPerPixel     PIXEL   foreground-pixel-value

//        If this bit is set then the SubrectsColoured bit must be zero
.
//    AnySubrects

//        If set, a single byte follows giving the number of subrectangles following:
//        No. of bytes      Type        Description
//        1                 U8      number-of-subrectangles

//        If not set, there are no subrectangles (i.e. the whole tile is just solid
//        background colour).

//    SubrectsColoured

//        If set then each subrectangle is preceded by a pixel value giving the colour of
//        that subrectangle, so a subrectangle is:

//        No. of bytes      Type        Description
//        bytesPerPixel     PIXEL   subrect-pixel-value
//        1                 U8      x-and-y-position
//        1                 U8      width-and-height

//        If not set, all subrectangles are the same colour, the foreground colour; if the
// ForegroundSpecified bit wasn't set then the foreground is the same as the last tile.

// A subrectangle is:
//        No. of bytes 	Type 	Description
//        1 	U8 	x-and-y-position
//        1 	U8 	width-and-height

//    The position and size of each subrectangle is specified in two bytes,
// x-and-y-position and width-and-height. The most-significant four bits
// of x-and-y-position specify the X position, the least-significant specify
// the Y position. The most-significant four bits of width-and-height specify
// the width minus one, the least-significant specify the height minus one.

namespace VNC {
    namespace Encoder {
        class Hextile : public EncoderApi {
//             const uint8_t bpp;
//             const uint8_t Bpp;
//             const size_t x;
//             size_t y;
//             const size_t cx;
//             size_t cy;

//             enum class HextileState {
//                 Header,
//                 Data,
//                 Exit
//             } state;
        
        public:
//             VNCVerbose verbose;

            Hextile(uint8_t /*bpp*/, uint8_t /*Bpp*/, size_t /*x*/, size_t /*y*/, size_t /*cx*/, size_t /*cy*/, VNCVerbose /*verbose*/)
//             Hextile(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, VNCVerbose verbose)
//                 : bpp(bpp), Bpp(Bpp), x(x), y(y), cx(cx), cy(cy)
//                 , state(HextileState::Header)
//                 , verbose(verbose)
            {
            }
            
            virtual ~Hextile(){}
            
            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            EncoderState consume(Buf64k & /*buf*/, gdi::GraphicApi & /*drawable*/) override
            {
                return EncoderState::Exit; // finished decoding
            }
            
            public:
                void draw_tile(Rect rect, const uint8_t * raw, gdi::GraphicApi & drawable)
                {
                    const uint16_t TILE_CX = 32;
                    const uint16_t TILE_CY = 32;

                    for (int y = 0; y < rect.cy ; y += TILE_CY) {
                        uint16_t cy = std::min(TILE_CY, uint16_t(rect.cy - y));

                        for (int x = 0; x < rect.cx ; x += TILE_CX) {
                            uint16_t cx = std::min(TILE_CX, uint16_t(rect.cx - x));

                            const Rect src_tile(x, y, cx, cy);
                            // TODO: fix here magic number 16 is vnc.bpp
                            const Bitmap tiled_bmp(raw, rect.cx, rect.cy, 16, src_tile);
                            const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                            const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                            drawable.draw(cmd2, dst_tile, tiled_bmp);
                        }
                    }
                }
        };    
    } // namespace encoder
} // namespace VNC
