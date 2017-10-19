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

#include "utils/bitmap_private_data.hpp"
#include "utils/bitmap_from_rle.hpp"

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
        REDASSERT(data_size);
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

void decompress60(
  Bitmap::PrivateData::Data & bmp_data,
  uint16_t src_cx, uint16_t src_cy, const uint8_t *data, size_t data_size)
{
    //LOG(LOG_INFO, "bmp decompress60: cx=%u cy=%u data_size=%u", src_cx, src_cy, data_size);
  REDASSERT((bmp_data.bpp() == 24) || (bmp_data.bpp() == 32));
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
    const uint16_t cx = bmp_data.cx();
    const uint16_t cy = bmp_data.cy();
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
    REDASSERT(!data_size);
    uint8_t * r     = red_plane;
    uint8_t * g     = green_plane;
    uint8_t * b     = blue_plane;
    uint8_t * pixel = bmp_data.get();
    uint8_t   bpp   = nbbytes(bmp_data.bpp());
    for (uint16_t y = 0; y < cy; y++) {
        for (uint16_t x = 0; x < cx; x++) {
            uint32_t color = (0xFFu << 24) | ((*r++) << 16) | ((*g++) << 8) | (*b++);
            ::out_bytes_le(pixel, bpp, color);
            pixel += bpp;
        }
    }
    //LOG(LOG_INFO, "bmp decompress60: done");
}


template<int depth>
struct RLEDecompressorImpl
{
  static constexpr std::integral_constant<uint8_t, nbbytes(depth)> Bpp = {};

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

void decompress_(
  Bitmap::PrivateData::Data & bmp_data,
  const uint8_t* input, uint16_t src_cx, size_t size)
{
  const uint16_t dst_cx = bmp_data.cx();
    uint8_t* pmin = bmp_data.get();
    uint8_t* pmax = pmin + bmp_data.bmp_size();
    const size_t line_size = bmp_data.line_size();
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
        while (count > 0) {
            if(out >= pmax) {
                LOG(LOG_WARNING, "Decompressed bitmap too large. Dying.");
                throw Error(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE);
            }
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
    }
    return;
}

};

void decompress(
  Bitmap::PrivateData::Data & bmp_data,
  const uint8_t* input, uint16_t src_cx, uint16_t src_cy, size_t size)
{
    (void)src_cy;
    switch (bmp_data.bpp()) {
        case 8 : return RLEDecompressorImpl<8>{}.decompress_(bmp_data, input, src_cx, size);
        case 15: return RLEDecompressorImpl<15>{}.decompress_(bmp_data, input, src_cx, size);
        case 16: return RLEDecompressorImpl<16>{}.decompress_(bmp_data, input, src_cx, size);
        default: return RLEDecompressorImpl<24>{}.decompress_(bmp_data, input, src_cx, size);
    }
}

}

Bitmap bitmap_from_rle(
    uint8_t session_color_depth, uint8_t bpp, const BGRPalette * palette,
    uint16_t cx, uint16_t cy, const uint8_t * data, const size_t size,
    bool compressed);
