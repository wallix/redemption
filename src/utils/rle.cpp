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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Martin Potier,
              Meng Tan, Clement Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   This file implement the bitmap items data structure
   including RDP RLE compression and decompression algorithms

   It also features storage and color versionning of the bitmap
   returning a pointer on a table, corresponding to the required
   color model.
*/

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/rle.hpp"
#include "utils/bitmap_private_data.hpp" // aux_::bitmap_data_allocator
#include "utils/stream.hpp"
#include "utils/image_data_view.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"


using std::size_t; /*NOLINT*/

namespace {
// [MS-RDPEGDI] 2.2.2.5.1 RDP 6.0 Bitmap Compressed Bitmap Stream
//  (RDP6_BITMAP_STREAM)
// ==============================================================

// The RDP6_BITMAP_STREAM structure contains a stream of bitmap data
//  compressed using RDP 6.0 Bitmap Compression techniques (section 3.1.9).
//  Depending on the compression techniques employed, the bitmap data is
//  represented using the AYCoCg or ARGB color space (section 3.1.9.1.2).

// Compressed bitmap data is sent encapsulated in a Bitmap Update
//  ([MS-RDPBCGR] section 2.2.9.1.1.3.1.2), Fast-Path Bitmap Update
//  ([MS-RDPBCGR] section 2.2.9.1.2.1.2 ), Cache Bitmap -Revision 1 (section
//  2.2.2.2.1.2.2) Secondary Drawing Orders, or Cache Bitmap - Revision 2
//  (section 2.2.2.2.1.2.3) Secondary Drawing Orders. In all of these cases,
//  the data is encapsulated inside a Bitmap Data structure ([MS-RDPBCGR]
//  section 2.2.9.1.1.3.1.2.2).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  FormatHeader |             AlphaPlane (variable)             |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   LumaOrRedPlane (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |              OrangeChromaOrGreenPlane (variable)              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |               GreenChromaOrBluePlane (variable)               |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------+-----------------------------------------------+
// | Pad (optional)|
// +---------------+

// FormatHeader (1 byte): An 8-bit, unsigned integer. This field contains a
//  1-byte, bit-packed update header as follows.

//  The format of the update header byte is described by the following
//   bitmask diagram.

//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
//  |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | CLL |C|R|N|Res|
//  |     |S|L|A|erv|
//  |     | |E| | ed|
//  +-----+-+-+-+---+

//   CLL (3 bits): A 3-bit, unsigned integer field that indicates the Color
//    Loss Level (section 3.1.9.1.4). If CLL is set to 0, the color space
//    used is ARGB. Otherwise, CLL MUST be in the range 1 to 7 (inclusive),
//    and the color space used is AYCoCg.

//   CS (1 bit): A 1-bit field that indicates whether chroma subsampling is
//    being used (section 3.1.9.1.3). If CS is equal to 1, chroma subsampling
//    is being used, and the CLL field MUST be greater than 0, as chroma
//    subsampling applies only to the AYCoCg color space.

//   RLE (1 bit): A 1-bit field. If RLE is equal to 1, RDP 6.0 RLE is used to
//    compress the color planes (section 3.1.9.2). If not, RLE is equal to 0,
//    and the color plane is sent uncompressed.

//   NA (1 bit): A 1-bit field. Indicates if an alpha plane is present. If NA
//    is equal to 1, there is no alpha plane. The values of the alpha plane
//    are then assumed to be 0xFF (fully opaque), and the bitmap data
//    contains only three color planes. If NA is equal to 0, the alpha plane
//    is sent as the first color plane.

//   Reserved (2 bits): A 2-bit, unsigned integer field. Reserved for future
//    use.

// AlphaPlane (variable): A variable-length field that contains the alpha
//  plane. If the RLE subfield in the FormatHeader indicates that all of the
//  color planes are RLE compressed (section 3.1.9.2), this field contains an
//  RDP 6.0 RLE Segments (section 2.2.2.5.1.1) structure. Otherwise, it
//  contains the raw bytes of the color plane.

// LumaOrRedPlane (variable): A variable-length field that contains the luma
//  plane (AYCoCg color space) or the red plane (ARGB color space). If the
//  CLL subfield of the FormatHeader is greater than 0, the AYCoCg color
//  space MUST be used. Otherwise, the ARGB color space MUST be used.

//  If the RLE subfield in the FormatHeader indicates that all of the color
//   planes are RLE compressed (section 3.1.9.2), this field contains an RDP
//   6.0 RLE Segments (section 2.2.2.5.1.1) structure. Otherwise, it contains
//   the raw bytes of the color plane.

// OrangeChromaOrGreenPlane (variable): A variable-length field that contains
//  the orange chroma plane (AYCoCg color space) or the green plane (ARGB
//  color space). If the CLL subfield of the FormatHeader is greater than 0,
//  the AYCoCg color space MUST be used. Otherwise, the ARGB color space MUST
//  be used.

//  If the RLE subfield in the FormatHeader indicates that all of the color
//   planes are RLE compressed (section 3.1.9.2), this field contains an RDP
//   6.0 RLE Segments (section 2.2.2.5.1.1) structure. Otherwise, it contains
//   the raw bytes of the color plane.

//  Depending on the values of the CLL and CS subfields of the FormatHeader
//   (in the case of the AYCoCg color space), the orange chroma plane may
//   have been transformed by color loss reduction (section 3.1.9.1.4) and
//   chroma subsampling (section 3.1.9.1.3).

// GreenChromaOrBluePlane (variable): A variable-length field that contains
//  the green chroma plane (AYCoCg color space) or the blue plane (ARGB color
//  space). If the CLL subfield of the FormatHeader is greater than 0, the
//  AYCoCg color space MUST be used. Otherwise, the ARGB color space MUST be
//  used.

//  If the RLE subfield in the FormatHeader indicates that all of the color
//   planes are RLE compressed (section 3.1.9.2), this field contains an RDP
//   6.0 RLE Segments (section 2.2.2.5.1.1) structure. Otherwise, it contains
//   the raw bytes of the color plane.

//  Depending on the values of the CLL and CS subfields of the FormatHeader
//   (in the case of the AYCoCg color space), the green chroma plane may have
//   been transformed by color loss reduction (section 3.1.9.1.4) and chroma
//   subsampling (section 3.1.9.1.3).

// Pad (1 byte): An 8-bit, unsigned integer containing padding values that
//  MUST be ignored. This optional field is only present if the RLE subfield
//  of the FormatHeader field is zero.

void decompress_color_plane(
    uint16_t src_cx, uint16_t src_cy, const uint8_t *& data,
    size_t & data_size, uint16_t cx, uint8_t * color_plane)
{
    uint32_t   size        = sizeof(uint8_t) * src_cx * src_cy;
    uint16_t   line_size   = src_cx;
    uint8_t  * line_start  = color_plane;
    uint8_t  * write_point = line_start;
    while (size) {
        //LOG(LOG_INFO, "size=%u data_size=%u", size, data_size);
        assert(data_size);
        uint8_t controlByte = *data++;
        data_size--;
        uint8_t nRunLength =  (controlByte & 0x0F);
        uint8_t cRawBytes  = ((controlByte & 0xF0) >> 4);
        //LOG(LOG_INFO, "    nRunLength=%d cRawBytes=%d", nRunLength, cRawBytes);
        if (nRunLength == 1) {
            nRunLength = 16 + cRawBytes;
            cRawBytes  = 0;
        }
        else if (nRunLength == 2) {
            if (cRawBytes == 15) {
                nRunLength = 47;
                cRawBytes  = 0;
            }
            else {
                nRunLength = 32 + cRawBytes;
                cRawBytes  = 0;
            }
        }
        //LOG(LOG_INFO, "(1) nRunLength=%d cRawBytes=%d", nRunLength, cRawBytes);
        if (cRawBytes) {
            ::memcpy(write_point, data, cRawBytes);
            write_point += cRawBytes;
            data        += cRawBytes;
            data_size   -= cRawBytes;
            line_size   -= cRawBytes;
            size        -= cRawBytes;
        }
        if (nRunLength) {
            ::memset(write_point, ((line_size == src_cx) ? 0 : *(write_point - 1)), nRunLength);
            write_point += nRunLength;
            line_size   -= nRunLength;
            size        -= nRunLength;
        }
        if (!line_size) {
            line_size  =  src_cx;
            line_start += cx;
        }
    }
    for (uint8_t * ypos_begin = color_plane + cx, * ypos_end = color_plane + cx * src_cy;
         ypos_begin < ypos_end; ypos_begin += cx) {
        for (uint8_t * xpos_begin = ypos_begin, * xpos_end = xpos_begin + src_cx; xpos_begin < xpos_end; xpos_begin++) {
            uint8_t delta = *xpos_begin;
            //LOG(LOG_INFO, "delta=%d", delta);
            if ((delta % 2) == 0) {
                delta >>= 1;
                //LOG(LOG_INFO, "delta(o)=%02x", delta);
            }
            else {
                delta = 255 - ((delta - 1) >> 1);
                //LOG(LOG_INFO, "delta(e)=%02x", delta);
            }
            *xpos_begin = static_cast<uint8_t>(*(xpos_begin - cx) + delta);
            //LOG(LOG_INFO, "delta(1)=%d", (*xpos_begin));
        }
    }
}

void in_copy_color_plan(
    uint16_t src_cx, uint16_t src_cy, const uint8_t * & data,
    size_t & data_size, uint16_t cx, uint8_t * color_plane)
{
    uint8_t * line_start  = color_plane;
    for (uint16_t y = 0; y < src_cy; y++) {
        ::memcpy(line_start, data, src_cx);
        data       += src_cx;
        data_size  -= src_cx;
        line_start += cx;
    }
}

template<int BitsPerPixel>
struct RLEDecompressorImpl
{
  static constexpr std::integral_constant<uint8_t, nbbytes(BitsPerPixel)> Bpp {};

enum {
    FLAG_NONE = 0,
    FLAG_FILL = 1,
    FLAG_MIX  = 2,
    FLAG_FOM  = 3,
    FLAG_MIX_SET = 6,
    FLAG_FOM_SET = 7,
    FLAG_COLOR = 8,
    FLAG_BICOLOR = 9
};


unsigned get_pixel(const uint8_t * const p)
{
    return in_uint32_from_nb_bytes_le(Bpp, p);
}

unsigned get_pixel_above(std::size_t line_size, const uint8_t * pmin, const uint8_t * const p)
{
    return ((p - line_size) < pmin)
    ? 0
    : get_pixel(p - line_size);
}

unsigned get_color_count(const uint8_t * pmax, const uint8_t * p, unsigned color)
{
    unsigned acc = 0;
    while (p < pmax && this->get_pixel(p) == color){
        acc++;
        p = p + Bpp;
    }
    return acc;
}

unsigned get_bicolor_count(const uint8_t * pmax, const uint8_t * p, unsigned color1, unsigned color2)
{
    unsigned acc = 0;
    while ((p < pmax)
        && (color1 == this->get_pixel(p))
        && (p + Bpp < pmax)
        && (color2 == this->get_pixel(p + Bpp))) {
            acc = acc + 2;
            p = p + 2 * Bpp;
    }
    return acc;
}

unsigned get_fill_count(std::size_t line_size, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p)
{
    unsigned acc = 0;
    while  (p + Bpp <= pmax) {
        unsigned pixel = this->get_pixel(p);
        unsigned ypixel = this->get_pixel_above(line_size, pmin, p);
        if (ypixel != pixel){
            break;
        }
        p += Bpp;
        acc += 1;
    }
    return acc;
}

unsigned get_mix_count(std::size_t line_size, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground)
{
    unsigned acc = 0;
    while (p + Bpp <= pmax){
        if (this->get_pixel_above(line_size, pmin, p) ^ foreground ^ this->get_pixel(p)){
            break;
        }
        p += Bpp;
        acc += 1;
    }
    return acc;
}

unsigned get_fom_count(std::size_t line_size, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground, bool fill)
{
    unsigned acc = 0;
    while (true){
        unsigned count = 0;
        while  (p + Bpp <= pmax) {
            unsigned pixel = this->get_pixel(p);
            unsigned ypixel = this->get_pixel_above(line_size, pmin, p);
            if (ypixel ^ pixel ^ (fill?0:foreground)){
                break;
            }
            p += Bpp;
            count += 1;
            if (count >= 9) {
                return acc;
            }
        }
        if (!count){
            break;
        }
        acc += count;
        fill ^= true;
    }
    return acc;
}

void get_fom_masks(std::size_t line_size, const uint8_t * pmin, const uint8_t * p, uint8_t * mask, const unsigned count)
{
    unsigned i = 0;
    for (i = 0; i < count; i += 8)
    {
        mask[i>>3] = 0;
    }
    for (i = 0 ; i < count; i++, p += Bpp)
    {
        if (get_pixel(p) != get_pixel_above(line_size, pmin, p)){
            mask[i>>3] |= static_cast<uint8_t>(0x01 << (i & 7));
        }
    }
}

unsigned get_fom_count_set(std::size_t line_size, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned & foreground, unsigned & flags)
{
    // flags : 1 = fill, 2 = MIX, 3 = (1+2) = FOM
    flags = FLAG_FILL;
    unsigned fill_count = this->get_fill_count(line_size, pmin, pmax, p);
    if (fill_count) {
        if (fill_count < 8) {
            unsigned fom_count = this->get_fom_count(line_size, pmin, pmax, p + fill_count * Bpp, foreground, false);
            if (fom_count){
                flags = FLAG_FOM;
                fill_count += fom_count;
            }
        }
        return fill_count;
    }
    // fill_count and mix_count can't match at the same time.
    // this would mean that foreground is black, and we will never set
    // it to black, as it's useless because fill_count allready does that.
    // Hence it's ok to check them independently.
    if  (p + Bpp <= pmax) {
        flags = FLAG_MIX;
        // if there is a pixel we are always able to mix (at worse we will set foreground ourself)
        foreground = this->get_pixel_above(line_size, pmin, p) ^ this->get_pixel(p);
        unsigned mix_count = 1 + this->get_mix_count(line_size, pmin, pmax, p + Bpp, foreground);
        if (mix_count < 8) {
            unsigned fom_count = 0;
            fom_count = this->get_fom_count(line_size, pmin, pmax, p + mix_count * Bpp, foreground, true);
            if (fom_count){
                flags = FLAG_FOM;
                mix_count += fom_count;
            }
        }
        return mix_count;
    }
    flags = FLAG_NONE;
    return 0;
}

// Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
void decompress_(
  MutableImageDataView const & image,
  const uint8_t* input, uint16_t src_cx, size_t size, size_t* RM18446_adjusted_size)
{
    // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
    if (RM18446_adjusted_size) {
        *RM18446_adjusted_size = 0;
    }

    // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
    const uint8_t * const RM18446_input_saved = input;

    const uint16_t dst_cx = image.width();
    uint8_t* pmin = image.mutable_data();
    uint8_t* pmax = pmin + image.pix_len();
    const size_t line_size = image.line_size();
    uint16_t out_x_count = 0;
    unsigned yprev = 0;
    uint8_t* out = pmin;
    const uint8_t* end = input + size;
    unsigned color1 = 0;
    unsigned color2 = 0;
    unsigned mix = 0xFFFFFFFF;
    unsigned mask = 0;
    unsigned fom_mask = 0;
    unsigned count = 0;
    int bicolor = 0;
    enum {
        FILL    = 0,
        MIX     = 1,
        FOM     = 2,
        COLOR   = 3,
        COPY    = 4,
        MIX_SET = 6,
        FOM_SET = 7,
        BICOLOR = 8,
        SPECIAL_FGBG_1 = 9,
        SPECIAL_FGBG_2 = 10,
        WHITE = 13,
        BLACK = 14
    };
    uint8_t opcode;
    uint8_t lastopcode = 0xFF;
    while (input < end) {
        // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
        const uint8_t* RM18446_input_good = input;

        // Read RLE operators, handle short and long forms
        uint8_t code = input[0]; input++;
        switch (code >> 4) {
        case 0xf:
            switch (code){
                case 0xFD:
                    opcode = WHITE;
                    count = 1;
                break;
                case 0xFE:
                    opcode = BLACK;
                    count = 1;
                break;
                case 0xFA:
                    opcode = SPECIAL_FGBG_2;
                    count = 8;
                break;
                case 0xF9:
                    opcode = SPECIAL_FGBG_1;
                    count = 8;
                break;
                case 0xF8:
                    opcode = code & 0xf;
                    assert(opcode != 11 && opcode != 12 && opcode != 15);
                    count = input[0]|(input[1] << 8);
                    count += count;
                    input += 2;
                break;
                default:
                    opcode = code & 0xf;
                    assert(opcode != 11 && opcode != 12 && opcode != 15);
                    count = input[0]|(input[1] << 8);
                    input += 2;
                    // Opcodes 0xFB, 0xFC, 0xFF are some unknown orders of length 1 ?
                break;
            }
        break;
        case 0x0e: // Bicolor, short form (1 or 2 bytes)
            opcode = BICOLOR;
            count = code & 0xf;
            if (!count){
                count = input[0] + 16; input++;
            }
            count += count;
            break;
        case 0x0d:  // FOM SET, short form  (1 or 2 bytes)
            opcode = FOM_SET;
            count = code & 0x0F;
            if (count){
                count <<= 3;
            }
            else {
                count = input[0] + 1; input++;
            }
        break;
        case 0x05:
        case 0x04:  // FOM, short form  (1 or 2 bytes)
            opcode = FOM;
            count = code & 0x1F;
            if (count){
                count <<= 3;
            }
            else {
                count = input[0] + 1; input++;
            }
        break;
        case 0x0c: // MIX SET, short form (1 or 2 bytes)
            opcode = MIX_SET;
            count = code & 0x0f;
            if (!count){
                count = input[0] + 16; input++;
            }
        break;
        default:
            opcode = static_cast<uint8_t>(code >> 5); // FILL, MIX, FOM, COLOR, COPY
            count = code & 0x1f;
            if (!count){
                count = input[0] + 32; input++;
            }
            assert(opcode < 5);
            break;
        }
        /* Read preliminary data */
        switch (opcode) {
        case FOM:
            mask = 1;
            fom_mask = input[0]; input++;
        break;
        case SPECIAL_FGBG_1:
            mask = 1;
            fom_mask = 3;
        break;
        case SPECIAL_FGBG_2:
            mask = 1;
            fom_mask = 5;
        break;
        case BICOLOR:
            bicolor = 0;
            color1 = this->get_pixel(input);
            input += Bpp;
            color2 = this->get_pixel(input);
            input += Bpp;
            break;
        case COLOR:
            color2 = this->get_pixel(input);
            input += Bpp;
            break;
        case MIX_SET:
            mix = this->get_pixel(input);
            input += Bpp;
        break;
        case FOM_SET:
            mix = this->get_pixel(input);
            input += Bpp;
            mask = 1;
            fom_mask = input[0]; input++;
            break;
        default: // for FILL, MIX or COPY nothing to do here
            break;
        }
        // MAGIC MIX of one pixel to comply with crap in Bitmap RLE compression
        if ((opcode == FILL)
        && (opcode == lastopcode)
        && (out != pmin + line_size)){
            yprev = (out - line_size < pmin) ? 0 : this->get_pixel(out - line_size);
            out_bytes_le(out, Bpp, yprev ^ mix);
            count--;
            out += Bpp;
            out_x_count += 1;
            if (out_x_count == dst_cx){
                memset(out, 0, (dst_cx - src_cx) * Bpp);
                out_x_count = 0;
            }
        }
        lastopcode = opcode;
        //LOG(LOG_INFO, "%s %u", this->get_opcode(opcode), count);
        /* Output body */

        // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
        bool RM18446_processing_in_progress = false;

        while (count > 0) {
            if(out >= pmax) {
                LOG(LOG_WARNING, "Decompressed bitmap too large. Dying.");

                // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
                if (RM18446_adjusted_size && !RM18446_processing_in_progress && (out == pmax)) {
                    *RM18446_adjusted_size = RM18446_input_good - RM18446_input_saved;
                    return;
                }

                throw Error(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE);
            }

            // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
            RM18446_processing_in_progress = true;

            yprev = (out - line_size < pmin) ? 0 : this->get_pixel(out - line_size);
            switch (opcode) {
            case FILL:
                out_bytes_le(out, Bpp, yprev);
                break;
            case MIX_SET:
            case MIX:
                out_bytes_le(out, Bpp, yprev ^ mix);
                break;
            case FOM_SET:
            case FOM:
                if (mask == 0x100){
                    mask = 1;
                    fom_mask = input[0]; input++;
                }
                REDEMPTION_CXX_FALLTHROUGH;
            case SPECIAL_FGBG_1:
            case SPECIAL_FGBG_2:
                if (mask & fom_mask){
                    out_bytes_le(out, Bpp, yprev ^ mix);
                }
                else {
                    out_bytes_le(out, Bpp, yprev);
                }
                mask <<= 1;
                break;
            case COLOR:
                out_bytes_le(out, Bpp, color2);
                break;
            case COPY:
                out_bytes_le(out, Bpp, this->get_pixel(input));
                input += Bpp;
                break;
            case BICOLOR:
                if (bicolor) {
                    out_bytes_le(out, Bpp, color2);
                    bicolor = 0;
                }
                else {
                    out_bytes_le(out, Bpp, color1);
                    bicolor = 1;
                }
            break;
            case WHITE:
                out_bytes_le(out, Bpp, 0xFFFFFFFF);
            break;
            case BLACK:
                out_bytes_le(out, Bpp, 0);
            break;
            default:
                assert(false);
                break;
            }
            count--;
            out += Bpp;
            out_x_count += 1;
            if (out_x_count == dst_cx){
                memset(out, 0, (dst_cx - src_cx) * Bpp);
                out_x_count = 0;
            }
        }

        if(out == pmax) {
            // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
            if(RM18446_adjusted_size) {
                *RM18446_adjusted_size = input - RM18446_input_saved;
            }

            return;
        }
    }

    // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
    if (RM18446_adjusted_size && (out == pmax)) {
        *RM18446_adjusted_size = size;
    }
}

void compress_(ConstImageDataView const & image, OutStream & outbuffer)
{
    struct RLE_OutStream {
        OutStream & stream;
        explicit RLE_OutStream(OutStream & outbuffer)
        : stream(outbuffer)
        {}
        // =========================================================================
        // Helper methods for RDP RLE bitmap compression support
        // =========================================================================
        void out_count(const int in_count, const int mask){
            if (in_count < 32) {
                this->stream.out_uint8(static_cast<uint8_t>((mask << 5) | in_count));
            }
            else if (in_count < 256 + 32){
                this->stream.out_uint8(static_cast<uint8_t>(mask << 5));
                this->stream.out_uint8(static_cast<uint8_t>(in_count - 32));
            }
            else {
                this->stream.out_uint8(static_cast<uint8_t>(0xf0 | mask));
                this->stream.out_uint16_le(in_count);
            }
        }
        // Background Run Orders
        // ~~~~~~~~~~~~~~~~~~~~~
        // A Background Run Order encodes a run of pixels where each pixel in the
        // run matches the uncompressed pixel on the previous scanline. If there is
        // no previous scanline then each pixel in the run MUST be black.
        // When encountering back-to-back background runs, the decompressor MUST
        // write a one-pixel foreground run to the destination buffer before
        // processing the second background run if both runs occur on the first
        // scanline or after the first scanline (if the first run is on the first
        // scanline, and the second run is on the second scanline, then a one-pixel
        // foreground run MUST NOT be written to the destination buffer). This
        // one-pixel foreground run is counted in the length of the run.
        // The run length encodes the number of pixels in the run. There is no data
        // associated with Background Run Orders.
        // +-----------------------+-----------------------------------------------+
        // | 0x0 REGULAR_BG_RUN    | The compression order encodes a regular-form  |
        // |                       | background run. The run length is stored in   |
        // |                       | the five low-order bits of  the order header  |
        // |                       | byte. If this value is zero, then the run     |
        // |                       | length is encoded in the byte following the   |
        // |                       | order header and MUST be incremented by 32 to |
        // |                       | give the final value.                         |
        // +-----------------------+-----------------------------------------------+
        // | 0xF0 MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA     |
        // |                       | background run. The run length is stored in   |
        // |                       | the two bytes following the order header      |
        // |                       | (in little-endian format).                    |
        // +-----------------------+-----------------------------------------------+
        void out_fill_count(const int in_count)
        {
            this->out_count(in_count, 0x00);
        }
        // Foreground Run Orders
        // ~~~~~~~~~~~~~~~~~~~~~
        // A Foreground Run Order encodes a run of pixels where each pixel in the
        // run matches the uncompressed pixel on the previous scanline XOR’ed with
        // the current foreground color. If there is no previous scanline, then
        // each pixel in the run MUST be set to the current foreground color (the
        // initial foreground color is white).
        // The run length encodes the number of pixels in the run.
        // If the order is a "set" variant, then in addition to encoding a run of
        // pixels, the order also encodes a new foreground color (in little-endian
        // format) in the bytes following the optional run length. The current
        // foreground color MUST be updated with the new value before writing
        // the run to the destination buffer.
        // +---------------------------+-------------------------------------------+
        // | 0x1 REGULAR_FG_RUN        | The compression order encodes a           |
        // |                           | regular-form foreground run. The run      |
        // |                           | length is stored in the five low-order    |
        // |                           | bits of the order header byte. If this    |
        // |                           | value is zero, then the run length is     |
        // |                           | encoded in the byte following the order   |
        // |                           | header and MUST be incremented by 32 to   |
        // |                           | give the final value.                     |
        // +---------------------------+-------------------------------------------+
        // | 0xF1 MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA |
        // |                           | foreground run. The run length is stored  |
        // |                           | in the two bytes following the order      |
        // |                           | header (in little-endian format).         |
        // +---------------------------+-------------------------------------------+
        // | 0xC LITE_SET_FG_FG_RUN    | The compression order encodes a "set"     |
        // |                           | variant lite-form foreground run. The run |
        // |                           | length is stored in the four low-order    |
        // |                           | bits of the order header byte. If this    |
        // |                           | value is zero, then the run length is     |
        // |                           | encoded in the byte following the order   |
        // |                           | header and MUST be incremented by 16 to   |
        // |                           | give the final value.                     |
        // +---------------------------+-------------------------------------------+
        // | 0xF6 MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set"     |
        // |                           | variant MEGA_MEGA foreground run. The run |
        // |                           | length is stored in the two bytes         |
        // |                           | following the order header (in            |
        // |                           | little-endian format).                    |
        // +---------------------------+-------------------------------------------+
        void out_mix_count(const int in_count)
        {
            this->out_count(in_count, 0x01);
        }
        void out_mix_count_set(const int in_count, unsigned new_foreground)
        {
            const uint8_t mask = 0x06;
            if (in_count < 16) {
                this->stream.out_uint8(static_cast<uint8_t>(0xc0 | in_count));
            }
            else if (in_count < 256 + 16){
                this->stream.out_uint8(0xc0);
                this->stream.out_uint8(static_cast<uint8_t>(in_count - 16));
            }
            else {
                this->stream.out_uint8(0xf0 | mask);
                this->stream.out_uint16_le(in_count);
            }
            this->stream.out_bytes_le(Bpp, new_foreground);
        }
        // Foreground / Background Image Orders
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // A Foreground/Background Image Order encodes a binary image where each
        // pixel in the image that is not on the first scanline fulfils exactly one
        // of the following two properties:
        // (a) The pixel matches the uncompressed pixel on the previous scanline
        // XOR'ed with the current foreground color.
        // (b) The pixel matches the uncompressed pixel on the previous scanline.
        // If the pixel is on the first scanline then it fulfils exactly one of the
        // following two properties:
        // (c) The pixel is the current foreground color.
        // (d) The pixel is black.
        // The binary image is encoded as a sequence of byte-sized bitmasks which
        // follow the optional run length (the last bitmask in the sequence can be
        // smaller than one byte in size). If the order is a "set" variant then the
        // bitmasks MUST follow the bytes which specify the new foreground color.
        // Each bit in the encoded bitmask sequence represents one pixel in the
        // image. A bit that has a value of 1 represents a pixel that fulfils
        // either property (a) or (c), while a bit that has a value of 0 represents
        // a pixel that fulfils either property (b) or (d). The individual bitmasks
        // MUST each be processed from the low-order bit to the high-order bit.
        // The run length encodes the number of pixels in the run.
        // If the order is a "set" variant, then in addition to encoding a binary
        // image, the order also encodes a new foreground color (in little-endian
        // format) in the bytes following the optional run length. The current
        // foreground color MUST be updated with the new value before writing
        // the run to the destination buffer.
        // +--------------------------------+--------------------------------------+
        // | 0x2 REGULAR_FGBG_IMAGE         | The compression order encodes a      |
        // |                                | regular-form foreground/background   |
        // |                                | image. The run length is encoded in  |
        // |                                | the five low-order bits of the order |
        // |                                | header byte and MUST be multiplied   |
        // |                                | by 8 to give the final value. If     |
        // |                                | this value is zero, then the run     |
        // |                                | length is encoded in the byte        |
        // |                                | following the order header and MUST  |
        // |                                | be incremented by 1 to give the      |
        // |                                | final value.                         |
        // +--------------------------------+--------------------------------------+
        // | 0xF2 MEGA_MEGA_FGBG_IMAGE      | The compression order encodes a      |
        // |                                | MEGA_MEGA foreground/background      |
        // |                                | image. The run length is stored in   |
        // |                                | the two bytes following the order    |
        // |                                | header (in little-endian format).    |
        // +--------------------------------+--------------------------------------+
        // | 0xD LITE_SET_FG_FGBG_IMAGE     | The compression order encodes a      |
        // |                                | "set" variant lite-form              |
        // |                                | foreground/background image. The run |
        // |                                | length is encoded in the four        |
        // |                                | low-order bits of the order header   |
        // |                                | byte and MUST be multiplied by 8 to  |
        // |                                | give the final value. If this value  |
        // |                                | is zero, then the run length is      |
        // |                                | encoded in the byte following the    |
        // |                                | order header and MUST be incremented |
        // |                                | by 1 to give the final value.        |
        // +--------------------------------+--------------------------------------+
        // | 0xF7 MEGA_MEGA_SET_FGBG_IMAGE  | The compression order encodes a      |
        // |                                | "set" variant MEGA_MEGA              |
        // |                                | foreground/background image. The run |
        // |                                | length is stored in the two bytes    |
        // |                                | following the order header (in       |
        // |                                | little-endian format).               |
        // +-----------------------------------------------------------------------+
        void out_fom_count(const int in_count)
        {
            if (in_count < 256){
                if (in_count & 7){
                    this->stream.out_uint8(0x40);
                    this->stream.out_uint8(static_cast<uint8_t>(in_count - 1));
                }
                else{
                    this->stream.out_uint8(static_cast<uint8_t>(0x40 | (in_count >> 3)));
                }
            }
            else{
                this->stream.out_uint8(0xf2);
                this->stream.out_uint16_le(in_count);
            }
        }
        void out_fom_sequence(const int count, const uint8_t * masks) {
            this->out_fom_count(count);
            this->stream.out_copy_bytes(masks, nbbytes_large(count));
        }
        void out_fom_count_set(const int in_count)
        {
            if (in_count < 256){
                if (in_count & 0x87){
                    this->stream.out_uint8(0xD0);
                    this->stream.out_uint8(static_cast<uint8_t>(in_count - 1));
                }
                else{
                    this->stream.out_uint8(static_cast<uint8_t>(0xD0 | (in_count >> 3)));
                }
            }
            else{
                this->stream.out_uint8(0xf7);
                this->stream.out_uint16_le(in_count);
            }
        }
        void out_fom_sequence_set(const int count,
                                  const unsigned foreground, const uint8_t * masks) {
            this->out_fom_count_set(count);
            this->stream.out_bytes_le(Bpp, foreground);
            this->stream.out_copy_bytes(masks, nbbytes_large(count));
        }
        // Color Run Orders
        // ~~~~~~~~~~~~~~~~
        // A Color Run Order encodes a run of pixels where each pixel is the same
        // color. The color is encoded (in little-endian format) in the bytes
        // following the optional run length.
        // The run length encodes the number of pixels in the run.
        // +--------------------------+--------------------------------------------+
        // | 0x3 REGULAR_COLOR_RUN    | The compression order encodes a            |
        // |                          | regular-form color run. The run length is  |
        // |                          | stored in the five low-order bits of the   |
        // |                          | order header byte. If this value is zero,  |
        // |                          | then the run length is encoded in the byte |
        // |                          | following the order header and MUST be     |
        // |                          | incremented by 32 to give the final value. |
        // +--------------------------+--------------------------------------------+
        // | 0xF3 MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA  |
        // |                          | color run. The run length is stored in the |
        // |                          | two bytes following the order header (in   |
        // |                          | little-endian format).                     |
        // +--------------------------+--------------------------------------------+
        void out_color_sequence(const int count, const uint32_t color)
        {
            this->out_color_count(count);
            this->stream.out_bytes_le(Bpp, color);
        }
        void out_color_count(const int in_count)
        {
            this->out_count(in_count, 0x03);
        }
        // Color Image Orders
        // ~~~~~~~~~~~~~~~~~~
        // A Color Image Order encodes a run of uncompressed pixels.
        // The run length encodes the number of pixels in the run. So, to compute
        // the actual number of bytes which follow the optional run length, the run
        // length MUST be multiplied by the color depth (in bits-per-pixel) of the
        // bitmap data.
        // +-----------------------------+-----------------------------------------+
        // | 0x4 REGULAR_COLOR_IMAGE     | The compression order encodes a         |
        // |                             | regular-form color image. The run       |
        // |                             | length is stored in the five low-order  |
        // |                             | bits of the order header byte. If this  |
        // |                             | value is zero, then the run length is   |
        // |                             | encoded in the byte following the order |
        // |                             | header and MUST be incremented by 32 to |
        // |                             | give the final value.                   |
        // +-----------------------------+-----------------------------------------+
        // | 0xF4 MEGA_MEGA_COLOR_IMAGE  | The compression order encodes a         |
        // |                             | MEGA_MEGA color image. The run length   |
        // |                             | is stored in the two bytes following    |
        // |                             | the order header (in little-endian      |
        // |                             | format).                                |
        // +-----------------------------+-----------------------------------------+
        void out_copy_sequence(const int count, const uint8_t * data)
        {
            this->out_copy_count(count);
            this->stream.out_copy_bytes(data, count * Bpp);
        }
        void out_copy_count(const int in_count)
        {
            this->out_count(in_count, 0x04);
        }
        // Dithered Run Orders
        // ~~~~~~~~~~~~~~~~~~~
        // A Dithered Run Order encodes a run of pixels which is composed of two
        // alternating colors. The two colors are encoded (in little-endian format)
        // in the bytes following the optional run length.
        // The run length encodes the number of pixel-pairs in the run (not pixels).
        // +-----------------------------+-----------------------------------------+
        // | 0xE LITE_DITHERED_RUN       | The compression order encodes a         |
        // |                             | lite-form dithered run. The run length  |
        // |                             | is stored in the four low-order bits of |
        // |                             | the order header byte. If this value is |
        // |                             | zero, then the run length is encoded in |
        // |                             | the byte following the order header and |
        // |                             | MUST be incremented by 16 to give the   |
        // |                             | final value.                            |
        // +-----------------------------+-----------------------------------------+
        // | 0xF8 MEGA_MEGA_DITHERED_RUN | The compression order encodes a         |
        // |                             | MEGA_MEGA dithered run. The run length  |
        // |                             | is stored in the two bytes following    |
        // |                             | the order header (in little-endian      |
        // |                             | format).                                |
        // +-----------------------------+-----------------------------------------+
        void out_bicolor_sequence(const int count,
                                  const unsigned color1, const unsigned color2)
        {
            this->out_bicolor_count(count);
            this->stream.out_bytes_le(Bpp, color1);
            this->stream.out_bytes_le(Bpp, color2);
        }
        void out_bicolor_count(const int in_count)
        {
            const uint8_t mask = 0x08;
            if (in_count / 2 < 16){
                this->stream.out_uint8(static_cast<uint8_t>(0xe0 | (in_count / 2)));
            }
            else if (in_count / 2 < 256 + 16){
                this->stream.out_uint8(static_cast<uint8_t>(0xe0));
                this->stream.out_uint8(static_cast<uint8_t>(in_count / 2 - 16));
            }
            else{
                this->stream.out_uint8(0xf0 | mask);
                this->stream.out_uint16_le(in_count / 2);
            }
        }
    } out(outbuffer);
    const uint8_t * pmin = image.data();
    const uint8_t * p = pmin;
    // white with the right length : either 0xFF or 0xFFFF or 0xFFFFFF
    unsigned foreground = ~(-1u << (Bpp*8));
    unsigned new_foreground = foreground;
    unsigned flags = 0;
    uint8_t masks[512];
    unsigned copy_count = 0;
    const uint8_t * pmax = nullptr;
    uint32_t color = 0;
    uint32_t color2 = 0;
    const size_t bmp_size = image.pix_len();
    const size_t align4_cx_bpp = align4(image.width() * Bpp);
    for (int part = 0 ; part < 2 ; part++){
        // As far as I can see the specs of bitmap RLE compressor is crap here
        // Fill orders between first scanline and all others must be splitted
        // (or on windows RDP clients black pixels are inserted at beginning of line,
        // on rdesktop this corner case works just fine)...
        // but if the first scanline contains two successive FILL or
        // if all the remaining scanlines contains two consecutive fill
        // orders, a magic MIX pixel is inserted between fills.
        // This explains the surprising loop above and the test below.pp
        if (part){
            pmax = pmin + bmp_size;
        }
        else {
            pmax = pmin + align4_cx_bpp;
        }
        while (p < pmax)
        {
            uint32_t fom_count = this->get_fom_count_set(image.line_size(), pmin, pmax, p, new_foreground, flags);
            uint32_t color_count = 0;
            uint32_t bicolor_count = 0;
            if (p + Bpp < pmax){
                color = this->get_pixel(p);
                color2 = this->get_pixel(p + Bpp);
                if (color == color2){
                    color_count = this->get_color_count(pmax, p, color);
                }
                else {
                    bicolor_count = this->get_bicolor_count(pmax, p, color, color2);
                }
            }
            const unsigned fom_cost = 1                            // header
                + (foreground != new_foreground) * Bpp             // set
                + (flags == FLAG_FOM) * nbbytes_large(fom_count);  // mask
            const unsigned copy_fom_cost = 1 * (copy_count == 0) + fom_count * Bpp;     // pixels
            const unsigned color_cost = 1 + Bpp;
            const unsigned bicolor_cost = 1 + 2*Bpp;
            if ((fom_count >= color_count || (color_count == 0))
            && ((fom_count >= bicolor_count) || (bicolor_count < 4))
            && fom_cost < copy_fom_cost) {
                switch (flags){
                    case FLAG_FOM:
                        if (nbbytes_large(fom_count) > sizeof(masks)) {
                            fom_count = sizeof(masks) * 8;
                        }
                        this->get_fom_masks(image.line_size(), pmin, p, masks, fom_count);
                        if (new_foreground != foreground){
                            flags = FLAG_FOM_SET;
                        }
                    break;
                    case FLAG_MIX:
                        if (new_foreground != foreground){
                            flags = FLAG_MIX_SET;
                        }
                    break;
                    default:
                    break;
                }
            }
            else {
                unsigned copy_color_cost = (copy_count == 0) + color_count * Bpp;       // copy + pixels
                unsigned copy_bicolor_cost = (copy_count == 0) + bicolor_count * Bpp;   // copy + pixels
                if ((color_cost < copy_color_cost) && (color_count > 0)){
                    flags = FLAG_COLOR;
                }
                else if ((bicolor_cost < copy_bicolor_cost) && (bicolor_count > 0)){
                    flags = FLAG_BICOLOR;
                }
                else {
                    flags = FLAG_NONE;
                    copy_count++;
                }
            }
            if (flags && copy_count > 0){
                out.out_copy_sequence(copy_count, p - copy_count * Bpp);
                copy_count = 0;
            }
            switch (flags){
                case FLAG_BICOLOR:
                    out.out_bicolor_sequence(bicolor_count, color, color2);
                    p+= bicolor_count * Bpp;
                break;
                case FLAG_COLOR:
                    out.out_color_sequence(color_count, color);
                    p+= color_count * Bpp;
                break;
                case FLAG_FOM_SET:
                    if (nbbytes_large(fom_count) > sizeof(masks)) {
                        fom_count = sizeof(masks) * 8;
                    }
                    out.out_fom_sequence_set(fom_count, new_foreground, masks);
                    foreground = new_foreground;
                    p+= fom_count * Bpp;
                break;
                case FLAG_MIX_SET:
                    out.out_mix_count_set(fom_count, new_foreground);
                    foreground = new_foreground;
                    p+= fom_count * Bpp;
                break;
                case FLAG_FOM:
                    if (nbbytes_large(fom_count) > sizeof(masks)) {
                        fom_count = sizeof(masks) * 8;
                    }
                    out.out_fom_sequence(fom_count, masks);
                    p+= fom_count * Bpp;
                break;
                case FLAG_MIX:
                    out.out_mix_count(fom_count);
                    p+= fom_count * Bpp;
                break;
                case FLAG_FILL:
                    out.out_fill_count(fom_count);
                    p+= fom_count * Bpp;
                break;
                default: // copy, but wait until next good sequence before actual sending
                    p += Bpp;
                break;
            }
        }
        if (copy_count > 0){
            out.out_copy_sequence(copy_count, p - copy_count * Bpp);
            copy_count = 0;
        }
    }
}

};

void get_run(
    const uint8_t * data, uint16_t data_size, uint8_t last_raw, uint32_t & run_length,
    uint32_t & raw_bytes)
{
    const uint8_t * data_save = data;
    run_length = 0;
    raw_bytes  = 0;
    while (data_size) {
        raw_bytes++;
        data_size--;
        //LOG(LOG_INFO, "row_value=%c", *data);
        uint8_t last_raw_value = *(data++);
        while (data_size && (*data == last_raw_value)) {
            run_length++;
            data_size--;
            data++;
        }
        /*LOG(LOG_INFO, "run=%c", *data)*/;
        if (run_length >= 3) {
            break;
        }
        raw_bytes += run_length;
        run_length = 0;
    }
    if ((raw_bytes == 1) && run_length && (*data_save == last_raw)) {
        // [MS-RDPEGDI] Previous base value assumed to be 0.
        raw_bytes = 0;
        run_length++;
    }
}

void compress_color_plane(uint16_t cx, uint16_t cy, OutStream & outbuffer, uint8_t * color_plane)
{
    //LOG(LOG_INFO, "compress_color_plane: cx=%u cy=%u", cx, cy);
    //hexdump_d(color_plane, cx * cy);
    uint16_t plane_line_size = cx * sizeof(uint8_t);
    // Converts to delta values.
    for (uint8_t * ypos_rbegin = color_plane + (cy - 1) * plane_line_size, * ypos_rend = color_plane;
         ypos_rbegin != ypos_rend; ypos_rbegin -= plane_line_size) {
        //LOG(LOG_INFO, "Line");
        for (uint8_t * xpos_begin = ypos_rbegin, * xpos_end = xpos_begin + plane_line_size;
             xpos_begin != xpos_end; xpos_begin += sizeof(uint8_t)) {
            //LOG(LOG_INFO, "delta=%d", *xpos_begin);
            int8_t delta = (
                  (  static_cast<int16_t>(*xpos_begin                    )
                   - static_cast<int16_t>(*(xpos_begin - plane_line_size)))
                & 0xFF
            );
            //LOG(LOG_INFO, "delta(1)=%d", delta);
            if (delta >= 0) {
                delta <<= 1;
            }
            else {
                delta = (((~delta + 1) << 1) - 1) & 0xFF;
                //LOG(LOG_INFO, "delta(2)=%d", delta);
            }
            *xpos_begin = static_cast<uint8_t>(delta);
        }
    }
    //LOG(LOG_INFO, "After delta conversion");
    //hexdump_d(color_plane, cx * cy);
    for (const uint8_t * ypos_begin = color_plane, * ypos_end = color_plane + cy * plane_line_size;
         ypos_begin != ypos_end; ypos_begin += plane_line_size) {
        uint16_t data_size = plane_line_size;
        //LOG(LOG_INFO, "Line");
        uint8_t  last_raw  = 0;
        for (const uint8_t * xpos = ypos_begin; data_size; ) {
            uint32_t run_length;
            uint32_t raw_bytes;
            get_run(xpos, data_size, last_raw, run_length, raw_bytes);
            //LOG(LOG_INFO, "run_length=%u raw_bytes=%u", run_length, raw_bytes);
            while (run_length || raw_bytes) {
                if ((run_length > 0) && (run_length < 3)) {
                    break;
                }
                if (!raw_bytes) {
                    if (run_length > 47) {
                        outbuffer.out_uint8((15                << 4) | 2         ); // Control byte
                        //LOG(LOG_INFO, "controlByte: (15, 2); rawValues: <none>");
                        xpos        += 47;
                        data_size   -= 47;
                        run_length  -= 47;
                    }
                    else if (run_length > 31) {
                        outbuffer.out_uint8(((run_length - 32) << 4) | 2         ); // Control byte
                        //LOG(LOG_INFO, "controlByte(1): (%d, 2); rawValues: <none>", run_length - 32);
                        xpos        += run_length;
                        data_size   -= run_length;
                        run_length  =  0;
                    }
                    else if (run_length > 15) {
                        outbuffer.out_uint8(((run_length - 16) << 4) | 1         ); // Control byte
                        //LOG(LOG_INFO, "controlByte(2): (%d, 1); rawValues: <none>", run_length - 16);
                        xpos        += run_length;
                        data_size   -= run_length;
                        run_length  =  0;
                    }
                    else {
                        outbuffer.out_uint8((0                 << 4) | run_length); // Control byte
                        //LOG(LOG_INFO, "controlByte(3): (0, %d); rawValues: <none>", run_length);
                        assert(!run_length || (run_length > 2));
                        xpos        += run_length;
                        data_size   -= run_length;
                        run_length  =  0;
                    }
                }
                else if (raw_bytes > 15) {
                    uint8_t rb[16];
                    memset(rb, 0, sizeof(rb));
                    memcpy(rb, xpos, 15);
                    outbuffer.out_uint8((15 << 4) | 0); // Control byte
                    //LOG(LOG_INFO, "controlByte(6): (15, 0); rawValues: %s", rb);
                    //hexdump_d(rb, 15);
                    outbuffer.out_copy_bytes(xpos, 15);
                    xpos        += 15;
                    data_size   -= 15;
                    raw_bytes   -= 15;
                }
                else/* if (raw_bytes < 16)*/ {
                    uint8_t rb[16];
                    memset(rb, 0, sizeof(rb));
                    memcpy(rb, xpos, raw_bytes);
                    if (run_length > 15) {
                        outbuffer.out_uint8((raw_bytes << 4) | 15        ); // Control byte
                        //LOG(LOG_INFO, "controlByte(4): (%d, 15); rawValues: %s", raw_bytes, rb);
                        //hexdump_d(rb, raw_bytes);
                        outbuffer.out_copy_bytes(xpos, raw_bytes);
                        xpos        += raw_bytes + 15;
                        data_size   -= raw_bytes + 15;
                        run_length  -= 15;
                        raw_bytes   =  0;
                    }
                    else {
                        outbuffer.out_uint8((raw_bytes << 4) | run_length); // Control byte
                        //LOG(LOG_INFO, "controlByte(5): (%d, %d); rawValues: %s", raw_bytes, run_length, rb);
                        //hexdump_d(rb, raw_bytes);
                        assert(!run_length || (run_length > 2));
                        outbuffer.out_copy_bytes(xpos, raw_bytes);
                        xpos        += raw_bytes + run_length;
                        data_size   -= raw_bytes + run_length;
                        run_length  = 0;
                        raw_bytes   = 0;
                    }
                }
            }
            last_raw = *(xpos - 1);
        }
    }
    //LOG(LOG_INFO, "compress_color_plane: exit");
}

} // namespace

void rle_compress60(ConstImageDataView const & image, OutStream & outbuffer)
{
    //LOG(LOG_INFO, "bmp compress60");
    assert(image.bits_per_pixel() == BitsPerPixel{24}
        || image.bits_per_pixel() == BitsPerPixel{32});
    const uint16_t cx = image.width();
    const uint16_t cy = image.height();
    const uint32_t color_plane_size = sizeof(uint8_t) * cx * cy;
    // PERF allocation is unnecessary
    struct Mem {
        void * p; ~Mem() { aux_::bitmap_data_allocator.dealloc(p); }
    } mem { aux_::bitmap_data_allocator.alloc(color_plane_size * 3) };
    uint8_t * mem_color   = static_cast<uint8_t *>(mem.p);
    uint8_t * red_plane   = mem_color + color_plane_size * 0;
    uint8_t * green_plane = mem_color + color_plane_size * 1;
    uint8_t * blue_plane  = mem_color + color_plane_size * 2;
    const uint8_t   byte_per_color = safe_int(image.bytes_per_pixel());
    const uint8_t * data = image.data();
    uint8_t * pixel_over_red_plane   = red_plane;
    uint8_t * pixel_over_green_plane = green_plane;
    uint8_t * pixel_over_blue_plane  = blue_plane;
    for (uint16_t y = 0; y < cy; y++) {
        for (uint16_t x = 0; x < cx; x++) {
            uint32_t pixel = in_uint32_from_nb_bytes_le(byte_per_color, data);
            uint8_t b =  ( pixel        & 0xFF);
            uint8_t g =  ((pixel >> 8 ) & 0xFF);
            uint8_t r =  ((pixel >> 16) & 0xFF);
            uint8_t a =  ((pixel >> 24) & 0xFF);
            (void)a;
            *(pixel_over_red_plane++)   = r;
            *(pixel_over_green_plane++) = g;
            *(pixel_over_blue_plane++)  = b;
            data += byte_per_color;
        }
    }
    /*
    assert(outbuffer.has_room(1 + color_plane_size * 3));
    outbuffer.out_uint8(
          (1 << 5)  // No alpha plane
        );
    outbuffer.out_copy_bytes(red_plane,   color_plane_size);
    outbuffer.out_copy_bytes(green_plane, color_plane_size);
    outbuffer.out_copy_bytes(blue_plane,  color_plane_size);
    outbuffer.out_uint8(0);
    */
    outbuffer.out_uint8(
          (1 << 5)  // No alpha plane
        | (1 << 4)  // RLE
        );
    compress_color_plane(cx, cy, outbuffer, red_plane);
    compress_color_plane(cx, cy, outbuffer, green_plane);
    compress_color_plane(cx, cy, outbuffer, blue_plane);
    //LOG(LOG_INFO, "data_compressedsize=%u", this->data_compressedsize);
    //LOG(LOG_INFO, "bmp compress60: done");
}

void rle_decompress60(
  MutableImageDataView const & image,
  uint16_t src_cx, uint16_t src_cy, const uint8_t *data, size_t data_size)
{
    //LOG(LOG_INFO, "bmp decompress60: cx=%u cy=%u data_size=%u", src_cx, src_cy, data_size);
  assert((image.bits_per_pixel() == BitsPerPixel{24})
      || (image.bits_per_pixel() == BitsPerPixel{32}));
    //LOG(LOG_INFO, "data_size=%u src_cx=%u src_cy=%u", data_size, src_cx, src_cy);
    //hexdump_d(data, data_size);
    uint8_t FormatHeader = *data++;
    data_size--;
    uint8_t color_loss_level   =   (FormatHeader & 0x07);
    bool    chroma_subsampling = (((FormatHeader & 0x08) >> 3) == 1);
    bool    rle                = (((FormatHeader & 0x10) >> 4) == 1);
    bool    no_alpha_plane     = (((FormatHeader & 0x20) >> 5) == 1);
    //LOG(LOG_INFO, "FormatHeader=0x%02X color_loss_level=%d chroma_subsampling=%s rle=%s no_alpha_plane=%s",
    //    FormatHeader, color_loss_level, (chroma_subsampling ? "yes" : "no"), (rle ? "yes" : "no"),
    //    (no_alpha_plane ? "yes" : "no"));
    if (color_loss_level || chroma_subsampling) {
        LOG(LOG_INFO, "Unsupported compression options %d", color_loss_level & (chroma_subsampling << 3));
        return;
    }
    const uint16_t cx = image.width();
    const uint16_t cy = image.height();
    const uint32_t color_plane_size = sizeof(uint8_t) * cx * cy;
    struct Mem {
        void * p; ~Mem() { aux_::bitmap_data_allocator.dealloc(p); }
    } mem { aux_::bitmap_data_allocator.alloc(color_plane_size * 3) };
    uint8_t * mem_color   = static_cast<uint8_t *>(mem.p);
    uint8_t * red_plane   = mem_color + color_plane_size * 0;
    uint8_t * green_plane = mem_color + color_plane_size * 1;
    uint8_t * blue_plane  = mem_color + color_plane_size * 2;
    if (rle) {
        if (!no_alpha_plane) {
            decompress_color_plane(src_cx, src_cy, data, data_size, cx, red_plane);
        }
        decompress_color_plane(src_cx, src_cy, data, data_size, cx, red_plane);
        decompress_color_plane(src_cx, src_cy, data, data_size, cx, green_plane);
        decompress_color_plane(src_cx, src_cy, data, data_size, cx, blue_plane);
    }
    else {
        if (!no_alpha_plane) {
            const uint32_t size = sizeof(uint8_t) * src_cx * src_cy;
            data      += size;
            data_size -= size;
        }
        in_copy_color_plan(src_cx, src_cy, data, data_size, cx, red_plane);
        in_copy_color_plan(src_cx, src_cy, data, data_size, cx, green_plane);
        in_copy_color_plan(src_cx, src_cy, data, data_size, cx, blue_plane);
        data_size--;    // Pad
    }
    //LOG(LOG_INFO, "data_size=%u", data_size);
    assert(!data_size);
    uint8_t * r     = red_plane;
    uint8_t * g     = green_plane;
    uint8_t * b     = blue_plane;
    uint8_t * pixel = image.mutable_data();
    uint8_t   bpp   = safe_int(image.bytes_per_pixel());
    for (uint16_t y = 0; y < cy; y++) {
        for (uint16_t x = 0; x < cx; x++) {
            uint32_t color = (0xFFu << 24) | ((*r++) << 16) | ((*g++) << 8) | (*b++);
            ::out_bytes_le(pixel, bpp, color);
            pixel += bpp;
        }
    }
    //LOG(LOG_INFO, "bmp decompress60: done");
}

// Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
void rle_decompress(
    MutableImageDataView const & image,
    const uint8_t* input, uint16_t src_cx, uint16_t src_cy, size_t size, size_t* RM18446_adjusted_size)
{
    (void)src_cy;
    switch (image.bits_per_pixel()) {
        // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
        case BitsPerPixel{8 }: return RLEDecompressorImpl< 8>{}.decompress_(image, input, src_cx, size, RM18446_adjusted_size);
        case BitsPerPixel{15}: return RLEDecompressorImpl<15>{}.decompress_(image, input, src_cx, size, RM18446_adjusted_size);
        case BitsPerPixel{16}: return RLEDecompressorImpl<16>{}.decompress_(image, input, src_cx, size, RM18446_adjusted_size);
        default:               return RLEDecompressorImpl<24>{}.decompress_(image, input, src_cx, size, RM18446_adjusted_size);
    }
}

// TODO simplify and enhance compression using 1 pixel orders BLACK or WHITE.
void rle_compress(ConstImageDataView const & image, OutStream & outbuffer)
{
    switch (image.bits_per_pixel()) {
        case BitsPerPixel{8 }: return RLEDecompressorImpl< 8>{}.compress_(image, outbuffer);
        case BitsPerPixel{15}: return RLEDecompressorImpl<15>{}.compress_(image, outbuffer);
        case BitsPerPixel{16}: return RLEDecompressorImpl<16>{}.compress_(image, outbuffer);
        default:               return RLEDecompressorImpl<24>{}.compress_(image, outbuffer);
    }
}
