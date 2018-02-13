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
             const uint8_t bpp;
             const uint8_t Bpp;
             const Rect r;
             Rect tile;

//             enum class HextileState {
//                 Header,
//                 Data,
//                 Exit
//             } state;
        
        public:
             VNCVerbose verbose;

            Hextile(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, VNCVerbose verbose)
                 : bpp(bpp), Bpp(Bpp), r(x, y, cx, cy)
//                 , state(HextileState::Header)
                 , verbose(verbose)
            {
                this->tile = Rect(this->r.x, this->r.y, std::min<size_t>(this->r.cx, 16), std::min<size_t>(this->r.y, 16));
            }
            
            virtual ~Hextile(){}

//#define PIXEL_T rdr::CONCAT2E(U,BPP)
//#define READ_PIXEL CONCAT2E(readOpaque,BPP)
//#define HEXTILE_DECODE CONCAT2E(hextileDecode,BPP)

//static void HEXTILE_DECODE (const Rect& r, rdr::InStream* is,
//                            const PixelFormat& pf,
//                            ModifiablePixelBuffer* pb)
//{
//  Rect t;
//  PIXEL_T bg = 0;
//  PIXEL_T fg = 0;
//  PIXEL_T buf[16 * 16];

//  for (t.tl.y = r.tl.y; t.tl.y < r.br.y; t.tl.y += 16) {

//    t.br.y = __rfbmin(r.br.y, t.tl.y + 16);

//    for (t.tl.x = r.tl.x; t.tl.x < r.br.x; t.tl.x += 16) {

//      t.br.x = __rfbmin(r.br.x, t.tl.x + 16);

//      int tileType = is->readU8();

//      if (tileType & hextileRaw) {
//        is->readBytes(buf, t.area() * (BPP/8));
//        pb->imageRect(pf, t, buf);
//        continue;
//      }

//      if (tileType & hextileBgSpecified)
//        bg = is->READ_PIXEL();

//      int len = t.area();
//      PIXEL_T* ptr = buf;
//      while (len-- > 0) *ptr++ = bg;

//      if (tileType & hextileFgSpecified)
//        fg = is->READ_PIXEL();

//      if (tileType & hextileAnySubrects) {
//        int nSubrects = is->readU8();

//        for (int i = 0; i < nSubrects; i++) {

//          if (tileType & hextileSubrectsColoured)
//            fg = is->READ_PIXEL();

//          int xy = is->readU8();
//          int wh = is->readU8();

//          int x = ((xy >> 4) & 15);
//          int y = (xy & 15);
//          int w = ((wh >> 4) & 15) + 1;
//          int h = (wh & 15) + 1;
//          if (x + w > 16 || y + h > 16) {
//            throw rfb::Exception("HEXTILE_DECODE: Hextile out of bounds");
//          }
//          PIXEL_T* ptr = buf + y * t.width() + x;
//          int rowAdd = t.width() - w;
//          while (h-- > 0) {
//            int len = w;
//            while (len-- > 0) *ptr++ = fg;
//            ptr += rowAdd;
//          }
//        }
//      }
//      pb->imageRect(pf, t, buf);
//    }
//  }
//}

//#undef PIXEL_T
//#undef READ_PIXEL
//#undef HEXTILE_DECODE
            
            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            EncoderState consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
//                  for (t.tl.y = r.tl.y; t.tl.y < r.br.y; t.tl.y += 16) {

//                    t.br.y = __rfbmin(r.br.y, t.tl.y + 16);

//                    for (t.tl.x = r.tl.x; t.tl.x < r.br.x; t.tl.x += 16) {
//                      rdr::U8 tileType;

//                      t.br.x = __rfbmin(r.br.x, t.tl.x + 16);

//                      tileType = is->readU8();
//                      os->writeU8(tileType);

//                      if (tileType & hextileRaw) {
//                        os->copyBytes(is, t.area() * this->Bpp);
//                        continue;
//                      }

//                      if (tileType & hextileBgSpecified)
//                        os->copyBytes(is, this->Bpp);

//                      if (tileType & hextileFgSpecified)
//                        os->copyBytes(is, bytesPerPixel);

//                      if (tileType & hextileAnySubrects) {
//                        rdr::U8 nSubrects;

//                        nSubrects = is->readU8();
//                        os->writeU8(nSubrects);

//                        if (tileType & hextileSubrectsColoured)
//                          os->copyBytes(is, nSubrects * (this->Bpp + 2));
//                        else
//                          os->copyBytes(is, nSubrects * 2);
//                      }
//                    }
//                  }            
                return EncoderState::Exit; // finished decoding
            }
            
        };    
    } // namespace encoder
} // namespace VNC
