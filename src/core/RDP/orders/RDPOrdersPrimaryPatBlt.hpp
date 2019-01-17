/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder : Primary Order Pat Blt

*/


#pragma once

#include "core/RDP/orders/RDPOrdersCommon.hpp"

// 2.2.2.2.1.1.2.3 PatBlt (PATBLT_ORDER)
// =====================================

// The PatBlt Primary Drawing Order is used to paint a rectangle by using a
// specified brush and three-way raster operation.

// Encoding order number: 1 (0x01)
// Negotiation order number: 1 (0x01)
// Number of fields: 12
// Number of field encoding bytes: 2
// Maximum encoded field length: 26 bytes

// nLeftRect (variable): Left coordinate of the destination rectangle specified
//   using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): Top coordinate of the destination rectangle specified
//   using a Coord Field (section 2.2.2.2.1.1.1.1).

// nWidth (variable): Width of the destination rectangle specified using a
//   Coord Field (section 2.2.2.2.1.1.1.1).

// nHeight (variable): Height of the destination rectangle specified using a
//   Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): Index of the ternary raster operation to perform (see section
//   2.2.2.2.1.1.1.7).

// As far as I understand this order, there is not source bitmap involved in the
// definition of patblt, hence only orders using dest and brush only should be
// allowed.

// ie:

// +------+-------------------------------+
// | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
// |      | RPN: 0                        |
// +------+-------------------------------+
// | 0x05 | ROP: 0x000500A9               |
// |      | RPN: DPon                     |
// +------+-------------------------------+
// | 0x0F | ROP: 0x000F0001               |
// |      | RPN: Pn                       |
// +------+-------------------------------+
// | 0x50 | ROP: 0x00500325               |
// |      | RPN: PDna                     |
// +------+-------------------------------+
// | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
// |      | RPN: Dn                       |
// +------+-------------------------------+
// | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
// |      | RPN: DPx                      |
// +------+-------------------------------+
// | 0x5F | ROP: 0x005F00E9               |
// |      | RPN: DPan                     |
// +------+-------------------------------+
// | 0xA0 | ROP: 0x00A000C9               |
// |      | RPN: DPa                      |
// +------+-------------------------------+
// | 0xA5 | ROP: 0x00A50065               |
// |      | RPN: PDxn                     |
// +------+-------------------------------+
// | 0xAA | ROP: 0x00AA0029               |
// |      | RPN: D                        |
// +------+-------------------------------+
// | 0xAF | ROP: 0x00AF0229               |
// |      | RPN: DPno                     |
// +------+-------------------------------+
// | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
// |      | RPN: P                        |
// +------+-------------------------------+
// | 0xF5 | ROP: 0x00F50225               |
// |      | RPN: PDno                     |
// +------+-------------------------------+
// | 0xFA | ROP: 0x00FA0089               |
// |      | RPN: DPo                      |
// +------+-------------------------------+
// | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
// |      | RPN: 1                        |
// +------+-------------------------------+

// BackColor (3 bytes): Background color described using a Generic Color
//   (section 2.2.2.2.1.1.1.8) structure.

// ForeColor (3 bytes): Foreground color described using a Generic Color
//   (section 2.2.2.2.1.1.1.8) structure.

// BrushOrgX (1 byte): An 8-bit, signed integer. The x-coordinate of the point
//   where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushOrgY (1 byte): An 8-bit, signed integer. The y-coordinate of the point
//   where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushStyle (1 byte): An 8-bit, unsigned integer. The style of the brush used
//   in the drawing operation. If the TS_CACHED_BRUSH (0x80) flag is set in the
//   most significant bit of the BrushStyle field, a brush that was previously
//   cached using the Cache Bitmap Secondary Order (see section 2.2.2.2.1.2.7)
//   MUST be used. In this case, the BrushHatch field MUST contain the index of
//   the Brush Cache entry that holds the selected brush to use, and the low
//   nibble of the BrushStyle field MUST contain an identifier describing the
//   color depth of the cached brush.

// +---------------+------------------------------+
// | 0x1 BMF_1BPP  | 1 bit per pixel              |
// +---------------+------------------------------+
// | 0x3 BMF_8BPP  | 8 bits per pixel             |
// +---------------+------------------------------+
// | 0x4 BMF_16BPP | 15 or 16 bits per pixel      |
// +---------------+------------------------------+
// | 0x5 BMF_24BPP | 24 bits per pixel            |
// +---------------+------------------------------+

// If the TS_CACHED_BRUSH (0x80) flag is not set in the most significant bit of
//   the BrushStyle field, an 8-by-8-pixel brush MUST be used, and one of the
//   following style identifiers MUST be present in the field.

// +-----------------+---------------------------------------------------------+
// | 0x00 BS_SOLID   | Solid color brush. The BrushHatch field SHOULD be set   |
// |                 | to 0.                                                   |
// +-----------------+---------------------------------------------------------+
// | 0x01 BS_NULL    | Hollow brush. The BrushHatch field SHOULD be set to 0.  |
// +-----------------+---------------------------------------------------------+
// | 0x02 BS_HATCHED | Hatched brush. The hatch pattern MUST be described by   |
// |                 | the BrushHatch field (there are six possible hatch      |
// |                 | patterns).                                              |
// +-----------------+---------------------------------------------------------+
// | 0x03 BS_PATTERN | Pattern brush. The pixel pattern MUST be described by   |
// |                 | the BrushExtra and BrushHatch fields.                   |
// +-----------------+---------------------------------------------------------+

// BrushHatch (1 byte): An 8-bit, unsigned integer. Holds a brush hatch
//   identifier or a Brush Cache index, depending on the contents of the
//   BrushStyle field.

//   If the TS_CACHED_BRUSH (0x80) flag is set in the most significant bit of
//   the BrushStyle field, the BrushHatch value MUST contain the index of the
//   Brush Cache entry that holds the selected brush to use.

//   If the BrushStyle field is set to BS_SOLID (0x00) or BS_NULL (0x01), the
//   BrushHatch field SHOULD be set to 0.

//   If the BrushStyle field is set to BS_HATCHED (0x02), the BrushHatch field
//   MUST contain one of the following hatch constants.

// +--------------------+--------------------------+
// | 0x00 HS_HORIZONTAL | Horizontal hatching. --- |
// +--------------------+--------------------------+
// | 0x01 HS_VERTICAL   | Vertical hatching.   ||| |
// +--------------------+--------------------------+
// | 0x02 HS_FDIAGONAL  | Diagonal hatching.   \\\ |
// +--------------------+--------------------------+
// | 0x03 HS_BDIAGONAL  | Diagonal hatching.   /// |
// +--------------------+--------------------------+
// | 0x04 HS_CROSS      | Cross-hatching.      +++ |
// +--------------------+--------------------------+
// | 0x05 HS_DIAGCROSS  | Cross-hatching.      xxx |
// +--------------------+--------------------------+

// If the BrushStyle field is set to BS_PATTERN (0x03), the BrushHatch field
//   MUST encode the pixel pattern present in the bottom row of the 8x8 pattern
//   brush (the pixel patterns in the top seven rows MUST be encoded in the
//   BrushExtra field). For example, if the bottom row of the pattern brush
//   contains an alternating series of black and white pixels, BrushHatch will
//   contain either 0xAA or 0x55.

// BrushExtra (7 bytes): A byte array of length 7. BrushExtra contains an array
//   of bitmap bits that encodes the pixel pattern present in the top seven rows
//   of the 8x8 pattern brush. The pixel pattern present in the bottom row is
//   encoded in the BrushHatch field. The BrushExtra field is only present if
//   the BrushStyle is set to BS_PATTERN (0x03). The rows are encoded in reverse
//   order, that is, the pixels in the penultimate row are encoded in the first
//   byte, and the pixels in the top row are encoded in the seventh byte. For
//   example, a 45-degree downward sloping left-to-right line would be encoded
//   in BrushExtra as { 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 } with
//   BrushHatch containing the value 0x01 (the bottom row).

// 2.2.2.2.1.1.1.8 Generic Color (TS_COLOR)
// ========================================
// The TS_COLOR structure holds a 3-byte RGB color triplet (the red, green, and
// blue components necessary to reproduce a color in the additive RGB space) or
// a 1-byte palette index.

// RedOrPaletteIndex (1 byte): An 8-bit, unsigned integer. RedOrPaletteIndex is
// used as a palette index for 16-color and 256-color palettized color schemes.
// If the RGB color scheme is in effect, this field contains the red RGB
// component. To determine whether a palettized or RGB color scheme is in
// effect, the client MUST examine the preferredBitsPerPixel field of the Bitmap
// Capability Set ([MS-RDPBCGR] section 2.2.7.1.2). If preferredBitsPerPixel is
// less than or equal to 8, then a palettized color scheme is in effect;
// otherwise, an RGB color scheme is in effect.

// Green (1 byte): An 8-bit, unsigned integer. Green contains the green RGB
// color component.

// Blue (1 byte): An 8-bit, unsigned integer. Blue contains the blue RGB color
// component.

// 2.2.2.2.1.1.1.7 Ternary Raster Operation Index (ROP3_OPERATION_INDEX)
// --------------------------------------------------------------------

// The ROP3_OPERATION_INDEX structure is used to define how the bits in a source
// bitmap, destination bitmap, and a selected brush or pen are combined by using
// Boolean operators.

// rop3Index (1 byte): An 8-bit, unsigned integer. This field contains an index
// of a raster operation code that describes a Boolean operation, in Reverse
// Polish Notation, to perform on the bits in a source bitmap (S), destination
// bitmap (D), and selected brush or pen (P). This operation is a combination
// of the AND (a), OR (o), NOT (n), and XOR (x) Boolean operators.

// +------+-------------------------------+
// | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
// |      | RPN: 0                        |
// +------+-------------------------------+
// | 0x01 | ROP: 0x00010289               |
// |      | RPN: DPSoon      !(D|P|S)     |
// +------+-------------------------------+
// | 0x02 | ROP: 0x00020C89               |
// |      | RPN: DPSona      (!(P|S)&D)   |
// +------+-------------------------------+
// | 0x03 | ROP: 0x000300AA               |
// |      | RPN: PSon        !(P|S)       |
// +------+-------------------------------+
// | 0x04 | ROP: 0x00040C88               |
// |      | RPN: SDPona                   |
// +------+-------------------------------+
// | 0x05 | ROP: 0x000500A9               |
// |      | RPN: DPon                     |
// +------+-------------------------------+
// | 0x06 | ROP: 0x00060865               |
// |      | RPN: PDSxnon                  |
// +------+-------------------------------+
// | 0x07 | ROP: 0x000702C5               |
// |      | RPN: PDSaon                   |
// +------+-------------------------------+
// | 0x08 | ROP: 0x00080F08               |
// |      | RPN: SDPnaa                   |
// +------+-------------------------------+
// | 0x09 | ROP: 0x00090245               |
// |      | RPN: PDSxon                   |
// +------+-------------------------------+
// | 0x0A | ROP: 0x000A0329               |
// |      | RPN: DPna                     |
// +------+-------------------------------+
// | 0x0B | ROP: 0x000B0B2A               |
// |      | RPN: PSDnaon                  |
// +------+-------------------------------+
// | 0x0C | ROP: 0x000C0324               |
// |      | RPN: SPna                     |
// +------+-------------------------------+
// | 0x0D | ROP: 0x000D0B25               |
// |      | RPN: PDSnaon                  |
// +------+-------------------------------+
// | 0x0E | ROP: 0x000E08A5               |
// |      | RPN: PDSonon                  |
// +------+-------------------------------+
// | 0x0F | ROP: 0x000F0001               |
// |      | RPN: Pn                       |
// +------+-------------------------------+
// | 0x10 | ROP: 0x00100C85               |
// |      | RPN: PDSona                   |
// +------+-------------------------------+
// | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
// |      | RPN: DSon                     |
// +------+-------------------------------+
// | 0x12 | ROP: 0x00120868               |
// |      | RPN: SDPxnon                  |
// +------+-------------------------------+
// | 0x13 | ROP: 001302C8                 |
// |      | RPN: SDPaon                   |
// +------+-------------------------------+
// | 0x14 | ROP: 0x0140869                |
// |      | RPN: DPSxnon                  |
// +------+-------------------------------+
// | 0x15 | ROP: 0x001502C9               |
// |      | RPN: DPSaon                   |
// +------+-------------------------------+
// | 0x16 | ROP: 0x00165CCA               |
// |      | RPN: PSDPSanaxx               |
// +------+-------------------------------+
// | 0x17 | ROP: 0x00171D54               |
// |      | RPN: SSPxDSxaxn               |
// +------+-------------------------------+
// | 0x18 | ROP: 0x00180D59               |
// |      | RPN: SPxPDxa                  |
// +------+-------------------------------+
// | 0x19 | ROP: 0x00191CC8               |
// |      | RPN: SDPSanaxn                |
// +------+-------------------------------+
// | 0x1A | ROP: 0x001A06C5               |
// |      | RPN: PDSPaox                  |
// +------+-------------------------------+
// | 0x1B | ROP: 0x001B0768               |
// |      | RPN: SDPSxaxn                 |
// +------+-------------------------------+
// | 0x1C | ROP: 0x001C06CA               |
// |      | RPN: PSDPaox                  |
// +------+-------------------------------+
// | 0x1D | ROP: 0x001D0766               |
// |      | RPN: DSPDxaxn                 |
// +------+-------------------------------+
// | 0x1E | ROP: 0x001E01A5               |
// |      | RPN: PDSox                    |
// +------+-------------------------------+
// | 0x1F | ROP: 0x001F0385               |
// |      | RPN: PDSoan                   |
// +------+-------------------------------+
// | 0x20 | ROP: 0x00200F09               |
// |      | RPN: DPSnaa                   |
// +------+-------------------------------+
// | 0x21 | ROP: 0x00210248               |
// |      | RPN: SDPxon                   |
// +------+-------------------------------+
// | 0x22 | ROP: 0x00220326               |
// |      | RPN: DSna                     |
// +------+-------------------------------+
// | 0x23 | ROP: 0x00230B24               |
// |      | RPN: SPDnaon                  |
// +------+-------------------------------+
// | 0x24 | ROP: 0x00240D55               |
// |      | RPN: SPxDSxa                  |
// +------+-------------------------------+
// | 0x25 | ROP: 0x00251CC5               |
// |      | RPN: PDSPanaxn                |
// +------+-------------------------------+
// | 0x26 | ROP: 0x002606C8               |
// |      | RPN: SDPSaox                  |
// +------+-------------------------------+
// | 0x27 | ROP: 0x00271868               |
// |      | RPN: SDPSxnox                 |
// +------+-------------------------------+
// | 0x28 | ROP: 0x00280369               |
// |      | RPN: DPSxa                    |
// +------+-------------------------------+
// | 0x29 | ROP: 0x002916CA               |
// |      | RPN: PSDPSaoxxn               |
// +------+-------------------------------+
// | 0x2A | ROP: 0x002A0CC9               |
// |      | RPN: DPSana                   |
// +------+-------------------------------+
// | 0x2B | ROP: 0x002B1D58               |
// |      | RPN: SSPxPDxaxn               |
// +------+-------------------------------+
// | 0x2C | ROP: 0x002C0784               |
// |      | RPN: SPDSoax                  |
// +------+-------------------------------+
// | 0x2D | ROP: 0x002D060A               |
// |      | RPN: PSDnox                   |
// +------+-------------------------------+
// | 0x2E | ROP: 0x002E064A               |
// |      | RPN: PSDPxox                  |
// +------+-------------------------------+
// | 0x2F | ROP: 0x002F0E2A               |
// |      | RPN: PSDnoan                  |
// +------+-------------------------------+
// | 0x30 | ROP: 0x0030032A               |
// |      | RPN: PSna                     |
// +------+-------------------------------+
// | 0x31 | ROP: 0x00310B28               |
// |      | RPN: SDPnaon                  |
// +------+-------------------------------+
// | 0x32 | ROP: 0x00320688               |
// |      | RPN: SDPSoox                  |
// +------+-------------------------------+
// | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
// |      | RPN: Sn                       |
// +------+-------------------------------+
// | 0x34 | ROP: 0x003406C4               |
// |      | RPN: SPDSaox                  |
// +------+-------------------------------+
// | 0x35 | ROP: 0x00351864               |
// |      | RPN: SPDSxnox                 |
// +------+-------------------------------+
// | 0x36 | ROP: 0x003601A8               |
// |      | RPN: SDPox                    |
// +------+-------------------------------+
// | 0x37 | ROP: 0x00370388               |
// |      | RPN: SDPoan                   |
// +------+-------------------------------+
// | 0x38 | ROP: 0x0038078A               |
// |      | RPN: PSDPoax                  |
// +------+-------------------------------+
// | 0x39 | ROP: 0x00390604               |
// |      | RPN: SPDnox                   |
// +------+-------------------------------+
// | 0x3A | ROP: 0x003A0644               |
// |      | RPN: SPDSxox                  |
// +------+-------------------------------+
// | 0x3B | ROP: 0x003B0E24               |
// |      | RPN: SPDnoan                  |
// +------+-------------------------------+
// | 0x3C | ROP: 0x003C004A               |
// |      | RPN: PSx                      |
// +------+-------------------------------+
// | 0x3D | ROP: 0x003D18A4               |
// |      | RPN: SPDSonox                 |
// +------+-------------------------------+
// | 0x3E | ROP: 0x003E1B24               |
// |      | RPN: SPDSnaox                 |
// +------+-------------------------------+
// | 0x3F | ROP: 0x003F00EA               |
// |      | RPN: PSan                     |
// +------+-------------------------------+
// | 0x40 | ROP: 0x00400F0A               |
// |      | RPN: PSDnaa                   |
// +------+-------------------------------+
// | 0x41 | ROP: 0x00410249               |
// |      | RPN: DPSxon                   |
// +------+-------------------------------+
// | 0x42 | ROP: 0x00420D5D               |
// |      | RPN: SDxPDxa                  |
// +------+-------------------------------+
// | 0x43 | ROP: 0x00431CC4               |
// |      | RPN: SPDSanaxn                |
// +------+-------------------------------+
// | 0x44 | ROP: 0x00440328 (SRCERASE)    |
// |      | RPN: SDna                     |
// +------+-------------------------------+
// | 0x45 | ROP: 0x00450B29               |
// |      | RPN: DPSnaon                  |
// +------+-------------------------------+
// | 0x46 | ROP: 0x004606C6               |
// |      | RPN: DSPDaox                  |
// +------+-------------------------------+
// | 0x47 | ROP: 0x0047076A               |
// |      | RPN: PSDPxaxn                 |
// +------+-------------------------------+
// | 0x48 | ROP: 0x00480368               |
// |      | RPN: SDPxa                    |
// +------+-------------------------------+
// | 0x49 | ROP: 0x004916C5               |
// |      | RPN: PDSPDaoxxn               |
// +------+-------------------------------+
// | 0x4A | ROP: 0x004A0789               |
// |      | RPN: DPSDoax                  |
// +------+-------------------------------+
// | 0x4B | ROP: 0x004B0605               |
// |      | RPN: PDSnox                   |
// +------+-------------------------------+
// | 0x4C | ROP: 0x004C0CC8               |
// |      | RPN: SDPana                   |
// +------+-------------------------------+
// | 0x4D | ROP: 0x004D1954               |
// |      | RPN: SSPxDSxoxn               |
// +------+-------------------------------+
// | 0x4E | ROP: 0x004E0645               |
// |      | RPN: PDSPxox                  |
// +------+-------------------------------+
// | 0x4F | ROP: 0x004F0E25               |
// |      | RPN: PDSnoan                  |
// +------+-------------------------------+
// | 0x50 | ROP: 0x00500325               |
// |      | RPN: PDna                     |
// +------+-------------------------------+
// | 0x51 | ROP: 0x00510B26               |
// |      | RPN: DSPnaon                  |
// +------+-------------------------------+
// | 0x52 | ROP: 0x005206C9               |
// |      | RPN: DPSDaox                  |
// +------+-------------------------------+
// | 0x53 | ROP: 0x00530764               |
// |      | RPN: SPDSxaxn                 |
// +------+-------------------------------+
// | 0x54 | ROP: 0x005408A9               |
// |      | RPN: DPSonon                  |
// +------+-------------------------------+
// | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
// |      | RPN: Dn                       |
// +------+-------------------------------+
// | 0x56 | ROP: 0x005601A9               |
// |      | RPN: DPSox                    |
// +------+-------------------------------+
// | 0x57 | ROP: 0x00570389               |
// |      | RPN: DPSoan                   |
// +------+-------------------------------+
// | 0x58 | ROP: 0x00580785               |
// |      | RPN: PDSPoax                  |
// +------+-------------------------------+
// | 0x59 | ROP: 0x00590609               |
// |      | RPN: DPSnox                   |
// +------+-------------------------------+
// | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
// |      | RPN: DPx                      |
// +------+-------------------------------+
// | 0x5B | ROP: 0x005B18A9               |
// |      | RPN: DPSDonox                 |
// +------+-------------------------------+
// | 0x5C | ROP: 0x005C0649               |
// |      | RPN: DPSDxox                  |
// +------+-------------------------------+
// | 0x5D | ROP: 0x005D0E29               |
// |      | RPN: DPSnoan                  |
// +------+-------------------------------+
// | 0x5E | ROP: 0x005E1B29               |
// |      | RPN: DPSDnaox                 |
// +------+-------------------------------+
// | 0x5F | ROP: 0x005F00E9               |
// |      | RPN: DPan                     |
// +------+-------------------------------+
// | 0x60 | ROP: 0x00600365               |
// |      | RPN: PDSxa                    |
// +------+-------------------------------+
// | 0x61 | ROP: 0x006116C6               |
// |      | RPN: DSPDSaoxxn               |
// +------+-------------------------------+
// | 0x62 | ROP: 0x00620786               |
// |      | RPN: DSPDoax                  |
// +------+-------------------------------+
// | 0x63 | ROP: 0x00630608               |
// |      | RPN: SDPnox                   |
// +------+-------------------------------+
// | 0x64 | ROP: 0x00640788               |
// |      | RPN: SDPSoax                  |
// +------+-------------------------------+
// | 0x65 | ROP: 0x00650606               |
// |      | RPN: DSPnox                   |
// +------+-------------------------------+
// | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
// |      | RPN: DSx                      |
// +------+-------------------------------+
// | 0x67 | ROP: 0x006718A8               |
// |      | RPN: SDPSonox                 |
// +------+-------------------------------+
// | 0x68 | ROP: 0x006858A6               |
// |      | RPN: DSPDSonoxxn              |
// +------+-------------------------------+
// | 0x69 | ROP: 0x00690145               |
// |      | RPN: PDSxxn                   |
// +------+-------------------------------+
// | 0x6A | ROP: 0x006A01E9               |
// |      | RPN: DPSax                    |
// +------+-------------------------------+
// | 0x6B | ROP: 0x006B178A               |
// |      | RPN: PSDPSoaxxn               |
// +------+-------------------------------+
// | 0x6C | ROP: 0x006C01E8               |
// |      | RPN: SDPax                    |
// +------+-------------------------------+
// | 0x6D | ROP: 0x006D1785               |
// |      | RPN: PDSPDoaxxn               |
// +------+-------------------------------+
// | 0x6E | ROP: 0x006E1E28               |
// |      | RPN: SDPSnoax                 |
// +------+-------------------------------+
// | 0x6F | ROP: 0x006F0C65               |
// |      | RPN: PDSxnan                  |
// +------+-------------------------------+
// | 0x70 | ROP: 0x00700CC5               |
// |      | RPN: PDSana                   |
// +------+-------------------------------+
// | 0x71 | ROP: 0x00711D5C               |
// |      | RPN: SSDxPDxaxn               |
// +------+-------------------------------+
// | 0x72 | ROP: 0x00720648               |
// |      | RPN: SDPSxox                  |
// +------+-------------------------------+
// | 0x73 | ROP: 0x00730E28               |
// |      | RPN: SDPnoan                  |
// +------+-------------------------------+
// | 0x74 | ROP: 0x00740646               |
// |      | RPN: DSPDxox                  |
// +------+-------------------------------+
// | 0x75 | ROP: 0x00750E26               |
// |      | RPN: DSPnoan                  |
// +------+-------------------------------+
// | 0x76 | ROP: 0x00761B28               |
// |      | RPN: SDPSnaox                 |
// +------+-------------------------------+
// | 0x77 | ROP: 0x007700E6               |
// |      | RPN: DSan                     |
// +------+-------------------------------+
// | 0x78 | ROP: 0x007801E5               |
// |      | RPN: PDSax                    |
// +------+-------------------------------+
// | 0x79 | ROP: 0x00791786               |
// |      | RPN: DSPDSoaxxn               |
// +------+-------------------------------+
// | 0x7A | ROP: 0x007A1E29               |
// |      | RPN: DPSDnoax                 |
// +------+-------------------------------+
// | 0x7B | ROP: 0x007B0C68               |
// |      | RPN: SDPxnan                  |
// +------+-------------------------------+
// | 0x7C | ROP: 0x007C1E24               |
// |      | RPN: SPDSnoax                 |
// +------+-------------------------------+
// | 0x7D | ROP: 0x007D0C69               |
// |      | RPN: DPSxnan                  |
// +------+-------------------------------+
// | 0x7E | ROP: 0x007E0955               |
// |      | RPN: SPxDSxo                  |
// +------+-------------------------------+
// | 0x7F | ROP: 0x007F03C9               |
// |      | RPN: DPSaan                   |
// +------+-------------------------------+
// | 0x80 | ROP: 0x008003E9               |
// |      | RPN: DPSaa                    |
// +------+-------------------------------+
// | 0x81 | ROP: 0x00810975               |
// |      | RPN: SPxDSxon                 |
// +------+-------------------------------+
// | 0x82 | ROP: 0x00820C49               |
// |      | RPN: DPSxna                   |
// +------+-------------------------------+
// | 0x83 | ROP: 0x00831E04               |
// |      | RPN: SPDSnoaxn                |
// +------+-------------------------------+
// | 0x84 | ROP: 0x00840C48               |
// |      | RPN: SDPxna                   |
// +------+-------------------------------+
// | 0x85 | ROP: 0x00851E05               |
// |      | RPN: PDSPnoaxn                |
// +------+-------------------------------+
// | 0x86 | ROP: 0x008617A6               |
// |      | RPN: DSPDSoaxx                |
// +------+-------------------------------+
// | 0x87 | ROP: 0x008701C5               |
// |      | RPN: PDSaxn                   |
// +------+-------------------------------+
// | 0x88 | ROP: 0x008800C6 (SRCAND)      |
// |      | RPN: DSa                      |
// +------+-------------------------------+
// | 0x89 | ROP: 0x00891B08               |
// |      | RPN: SDPSnaoxn                |
// +------+-------------------------------+
// | 0x8A | ROP: 0x008A0E06               |
// |      | RPN: DSPnoa                   |
// +------+-------------------------------+
// | 0x8B | ROP: 0x008B0666               |
// |      | RPN: DSPDxoxn                 |
// +------+-------------------------------+
// | 0x8C | ROP: 0x008C0E08               |
// |      | RPN: SDPnoa                   |
// +------+-------------------------------+
// | 0x8D | ROP: 0x008D0668               |
// |      | RPN: SDPSxoxn                 |
// +------+-------------------------------+
// | 0x8E | ROP: 0x008E1D7C               |
// |      | RPN: SSDxPDxax                |
// +------+-------------------------------+
// | 0x8F | ROP: 0x008F0CE5               |
// |      | RPN: PDSanan                  |
// +------+-------------------------------+
// | 0x90 | ROP: 0x00900C45               |
// |      | RPN: PDSxna                   |
// +------+-------------------------------+
// | 0x91 | ROP: 0x00911E08               |
// |      | RPN: SDPSnoaxn                |
// +------+-------------------------------+
// | 0x92 | ROP: 0x009217A9               |
// |      | RPN: DPSDPoaxx                |
// +------+-------------------------------+
// | 0x93 | ROP: 0x009301C4               |
// |      | RPN: SPDaxn                   |
// +------+-------------------------------+
// | 0x94 | ROP: 0x009417AA               |
// |      | RPN: PSDPSoaxx                |
// +------+-------------------------------+
// | 0x95 | ROP: 0x009501C9               |
// |      | RPN: DPSaxn                   |
// +------+-------------------------------+
// | 0x96 | ROP: 0x00960169               |
// |      | RPN: DPSxx                    |
// +------+-------------------------------+
// | 0x97 | ROP: 0x0097588A               |
// |      | RPN: PSDPSonoxx               |
// +------+-------------------------------+
// | 0x98 | ROP: 0x00981888               |
// |      | RPN: SDPSonoxn                |
// +------+-------------------------------+
// | 0x99 | ROP: 0x00990066               |
// |      | RPN: DSxn                     |
// +------+-------------------------------+
// | 0x9A | ROP: 0x009A0709               |
// |      | RPN: DPSnax                   |
// +------+-------------------------------+
// | 0x9B | ROP: 0x009B07A8               |
// |      | RPN: SDPSoaxn                 |
// +------+-------------------------------+
// | 0x9C | ROP: 0x009C0704               |
// |      | RPN: SPDnax                   |
// +------+-------------------------------+
// | 0x9D | ROP: 0x009D07A6               |
// |      | RPN: DSPDoaxn                 |
// +------+-------------------------------+
// | 0x9E | ROP: 0x009E16E6               |
// |      | RPN: DSPDSaoxx                |
// +------+-------------------------------+
// | 0x9F | ROP: 0x009F0345               |
// |      | RPN: PDSxan                   |
// +------+-------------------------------+
// | 0xA0 | ROP: 0x00A000C9               |
// |      | RPN: DPa                      |
// +------+-------------------------------+
// | 0xA1 | ROP: 0x00A11B05               |
// |      | RPN: PDSPnaoxn                |
// +------+-------------------------------+
// | 0xA2 | ROP: 0x00A20E09               |
// |      | RPN: DPSnoa                   |
// +------+-------------------------------+
// | 0xA3 | ROP: 0x00A30669               |
// |      | RPN: DPSDxoxn                 |
// +------+-------------------------------+
// | 0xA4 | ROP: 0x00A41885               |
// |      | RPN: PDSPonoxn                |
// +------+-------------------------------+
// | 0xA5 | ROP: 0x00A50065               |
// |      | RPN: PDxn                     |
// +------+-------------------------------+
// | 0xA6 | ROP: 0x00A60706               |
// |      | RPN: DSPnax                   |
// +------+-------------------------------+
// | 0xA7 | ROP: 0x00A707A5               |
// |      | RPN: PDSPoaxn                 |
// +------+-------------------------------+
// | 0xA8 | ROP: 0x00A803A9               |
// |      | RPN: DPSoa                    |
// +------+-------------------------------+
// | 0xA9 | ROP: 0x00A90189               |
// |      | RPN: DPSoxn                   |
// +------+-------------------------------+
// | 0xAA | ROP: 0x00AA0029               |
// |      | RPN: D                        |
// +------+-------------------------------+
// | 0xAB | ROP: 0x00AB0889               |
// |      | RPN: DPSono                   |
// +------+-------------------------------+
// | 0xAC | ROP: 0x00AC0744               |
// |      | RPN: SPDSxax                  |
// +------+-------------------------------+
// | 0xAD | ROP: 0x00AD06E9               |
// |      | RPN: DPSDaoxn                 |
// +------+-------------------------------+
// | 0xAE | ROP: 0x00AE0B06               |
// |      | RPN: DSPnao                   |
// +------+-------------------------------+
// | 0xAF | ROP: 0x00AF0229               |
// |      | RPN: DPno                     |
// +------+-------------------------------+
// | 0xB0 | ROP: 0x00B00E05               |
// |      | RPN: PDSnoa                   |
// +------+-------------------------------+
// | 0xB1 | ROP: 0x00B10665               |
// |      | RPN: PDSPxoxn                 |
// +------+-------------------------------+
// | 0xB2 | ROP: 0x00B21974               |
// |      | RPN: SSPxDSxox                |
// +------+-------------------------------+
// | 0xB3 | ROP: 0x00B30CE8               |
// |      | RPN: SDPanan                  |
// +------+-------------------------------+
// | 0xB4 | ROP: 0x00B4070A               |
// |      | RPN: PSDnax                   |
// +------+-------------------------------+
// | 0xB5 | ROP: 0x00B507A9               |
// |      | RPN: DPSDoaxn                 |
// +------+-------------------------------+
// | 0xB6 | ROP: 0x00B616E9               |
// |      | RPN: DPSDPaoxx                |
// +------+-------------------------------+
// | 0xB7 | ROP: 0x00B70348               |
// |      | RPN: SDPxan                   |
// +------+-------------------------------+
// | 0xB8 | ROP: 0x00B8074A               |
// |      | RPN: PSDPxax                  |
// +------+-------------------------------+
// | 0xB9 | ROP: 0x00B906E6               |
// |      | RPN: DSPDaoxn                 |
// +------+-------------------------------+
// | 0xBA | ROP: 0x00BA0B09               |
// |      | RPN: DPSnao                   |
// +------+-------------------------------+
// | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
// |      | RPN: DSno                     |
// +------+-------------------------------+
// | 0xBC | ROP: 0x00BC1CE4               |
// |      | RPN: SPDSanax                 |
// +------+-------------------------------+
// | 0xBD | ROP: 0x00BD0D7D               |
// |      | RPN: SDxPDxan                 |
// +------+-------------------------------+
// | 0xBE | ROP: 0x00BE0269               |
// |      | RPN: DPSxo                    |
// +------+-------------------------------+
// | 0xBF | ROP: 0x00BF08C9               |
// |      | RPN: DPSano                   |
// +------+-------------------------------+
// | 0xC0 | ROP: 0x00C000CA (MERGECOPY)   |
// |      | RPN: PSa                      |
// +------+-------------------------------+
// | 0xC1 | ROP: 0x00C11B04               |
// |      | RPN: SPDSnaoxn                |
// +------+-------------------------------+
// | 0xC2 | ROP: 0x00C21884               |
// |      | RPN: SPDSonoxn                |
// +------+-------------------------------+
// | 0xC3 | ROP: 0x00C3006A               |
// |      | RPN: PSxn                     |
// +------+-------------------------------+
// | 0xC4 | ROP: 0x00C40E04               |
// |      | RPN: SPDnoa                   |
// +------+-------------------------------+
// | 0xC5 | ROP: 0x00C50664               |
// |      | RPN: SPDSxoxn                 |
// +------+-------------------------------+
// | 0xC6 | ROP: 0x00C60708               |
// |      | RPN: SDPnax                   |
// +------+-------------------------------+
// | 0xC7 | ROP: 0x00C707AA               |
// |      | RPN: PSDPoaxn                 |
// +------+-------------------------------+
// | 0xC8 | ROP: 0x00C803A8               |
// |      | RPN: SDPoa                    |
// +------+-------------------------------+
// | 0xC9 | ROP: 0x00C90184               |
// |      | RPN: SPDoxn                   |
// +------+-------------------------------+
// | 0xCA | ROP: 0x00CA0749               |
// |      | RPN: DPSDxax                  |
// +------+-------------------------------+
// | 0xCB | ROP: 0x00CB06E4               |
// |      | RPN: SPDSaoxn                 |
// +------+-------------------------------+
// | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
// |      | RPN: S                        |
// +------+-------------------------------+
// | 0xCD | ROP: 0x00CD0888               |
// |      | RPN: SDPono                   |
// +------+-------------------------------+
// | 0xCE | ROP: 0x00CE0B08               |
// |      | RPN: SDPnao                   |
// +------+-------------------------------+
// | 0xCF | ROP: 0x00CF0224               |
// |      | RPN: SPno                     |
// +------+-------------------------------+
// | 0xD0 | ROP: 0x00D00E0A               |
// |      | RPN: PSDnoa                   |
// +------+-------------------------------+
// | 0xD1 | ROP: 0x00D1066A               |
// |      | RPN: PSDPxoxn                 |
// +------+-------------------------------+
// | 0xD2 | ROP: 0x00D20705               |
// |      | RPN: PDSnax                   |
// +------+-------------------------------+
// | 0xD3 | ROP: 0x00D307A4               |
// |      | RPN: SPDSoaxn                 |
// +------+-------------------------------+
// | 0xD4 | ROP: 0x00D41D78               |
// |      | RPN: SSPxPDxax                |
// +------+-------------------------------+
// | 0xD5 | ROP: 0x00D50CE9               |
// |      | RPN: DPSanan                  |
// +------+-------------------------------+
// | 0xD6 | ROP: 0x00D616EA               |
// |      | RPN: PSDPSaoxx                |
// +------+-------------------------------+
// | 0xD7 | ROP: 0x00D70349               |
// |      | RPN: DPSxan                   |
// +------+-------------------------------+
// | 0xD8 | ROP: 0x00D80745               |
// |      | RPN: PDSPxax                  |
// +------+-------------------------------+
// | 0xD9 | ROP: 0x00D906E8               |
// |      | RPN: SDPSaoxn                 |
// +------+-------------------------------+
// | 0xDA | ROP: 0x00DA1CE9               |
// |      | RPN: DPSDanax                 |
// +------+-------------------------------+
// | 0xDB | ROP: 0x00DB0D75               |
// |      | RPN: SPxDSxan                 |
// +------+-------------------------------+
// | 0xDC | ROP: 0x00DC0B04               |
// |      | RPN: SPDnao                   |
// +------+-------------------------------+
// | 0xDD | ROP: 0x00DD0228               |
// |      | RPN: SDno                     |
// +------+-------------------------------+
// | 0xDE | ROP: 0x00DE0268               |
// |      | RPN: SDPxo                    |
// +------+-------------------------------+
// | 0xDF | ROP: 0x00DF08C8               |
// |      | RPN: SDPano                   |
// +------+-------------------------------+
// | 0xE0 | ROP: 0x00E003A5               |
// |      | RPN: PDSoa                    |
// +------+-------------------------------+
// | 0xE1 | ROP: 0x00E10185               |
// |      | RPN: PDSoxn                   |
// +------+-------------------------------+
// | 0xE2 | ROP: 0x00E20746               |
// |      | RPN: DSPDxax                  |
// +------+-------------------------------+
// | 0xE3 | ROP: 0x00E306EA               |
// |      | RPN: PSDPaoxn                 |
// +------+-------------------------------+
// | 0xE4 | ROP: 0x00E40748               |
// |      | RPN: SDPSxax                  |
// +------+-------------------------------+
// | 0xE5 | ROP: 0x00E506E5               |
// |      | RPN: PDSPaoxn                 |
// +------+-------------------------------+
// | 0xE6 | ROP: 0x00E61CE8               |
// |      | RPN: SDPSanax                 |
// +------+-------------------------------+
// | 0xE7 | ROP: 0x00E70D79               |
// |      | RPN: SPxPDxan                 |
// +------+-------------------------------+
// | 0xE8 | ROP: 0x00E81D74               |
// |      | RPN: SSPxDSxax                |
// +------+-------------------------------+
// | 0xE9 | ROP: 0x00E95CE6               |
// |      | RPN: DSPDSanaxxn              |
// +------+-------------------------------+
// | 0xEA | ROP: 0x00EA02E9               |
// |      | RPN: DPSao                    |
// +------+-------------------------------+
// | 0xEB | ROP: 0x00EB0849               |
// |      | RPN: DPSxno                   |
// +------+-------------------------------+
// | 0xEC | ROP: 0x00EC02E8               |
// |      | RPN: SDPao                    |
// +------+-------------------------------+
// | 0xED | ROP: 0x00ED0848               |
// |      | RPN: SDPxno                   |
// +------+-------------------------------+
// | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
// |      | RPN: DSo                      |
// +------+-------------------------------+
// | 0xEF | ROP: 0x00EF0A08               |
// |      | RPN: SDPnoo                   |
// +------+-------------------------------+
// | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
// |      | RPN: P                        |
// +------+-------------------------------+
// | 0xF1 | ROP: 0x00F10885               |
// |      | RPN: PDSono                   |
// +------+-------------------------------+
// | 0xF2 | ROP: 0x00F20B05               |
// |      | RPN: PDSnao                   |
// +------+-------------------------------+
// | 0xF3 | ROP: 0x00F3022A               |
// |      | RPN: PSno                     |
// +------+-------------------------------+
// | 0xF4 | ROP: 0x00F40B0A               |
// |      | RPN: PSDnao                   |
// +------+-------------------------------+
// | 0xF5 | ROP: 0x00F50225               |
// |      | RPN: PDno                     |
// +------+-------------------------------+
// | 0xF6 | ROP: 0x00F60265               |
// |      | RPN: PDSxo                    |
// +------+-------------------------------+
// | 0xF7 | ROP: 0x00F708C5               |
// |      | RPN: PDSano                   |
// +------+-------------------------------+
// | 0xF8 | ROP: 0x00F802E5               |
// |      | RPN: PDSao                    |
// +------+-------------------------------+
// | 0xF9 | ROP: 0x00F90845               |
// |      | RPN: PDSxno                   |
// +------+-------------------------------+
// | 0xFA | ROP: 0x00FA0089               |
// |      | RPN: DPo                      |
// +------+-------------------------------+
// | 0xFB | ROP: 0x00FB0A09 (PATPAINT)    |
// |      | RPN: DPSnoo                   |
// +------+-------------------------------+
// | 0xFC | ROP: 0x00FC008A               |
// |      | RPN: PSo                      |
// +------+-------------------------------+
// | 0xFD | ROP: 0x00FD0A0A               |
// |      | RPN: PSDnoo                   |
// +------+-------------------------------+
// | 0xFE | ROP: 0x00FE02A9               |
// |      | RPN: DPSoo                    |
// +------+-------------------------------+
// | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
// |      | RPN: 1                        |
// +------+-------------------------------+

// For example, by using the previous table, it can be determined that the
// raster operation at index 0xEE (SRCPAINT) replaces the values of the pixels
// in the destination bitmap with a combination of pixel values of the
// destination and source bitmaps.

// For more information about ternary raster operations, see [MSDN-TRO] and
// [WGFX] section 11.1.


class RDPPatBlt {
    public:
    Rect rect;
    uint8_t rop;
    RDPColor back_color;
    RDPColor fore_color;
    RDPBrush brush;

    static uint8_t id()
    {
        return RDP::PATBLT;
    }

    RDPPatBlt(const Rect rect, uint8_t rop,
              RDPColor back_color, RDPColor fore_color,
              const RDPBrush & brush) :
        rect(rect),
        rop(rop),
        back_color(back_color),
        fore_color(fore_color),
        brush(brush)
        {
        }

    bool operator==(const RDPPatBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->back_color == other.back_color)
             && (this->fore_color == other.fore_color)
             && (this->brush.org_x == other.brush.org_x)
             && (this->brush.org_y == other.brush.org_y)
             && (this->brush.style == other.brush.style)
             && (this->brush.hatch == other.brush.hatch)
             && (   (this->brush.style != 0x03)
                 || (0 == memcmp(this->brush.extra, other.brush.extra, 7)))
             ;
    }

    void emit(OutStream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPPatBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // PATBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte
        // 0x20: Back color (3 bytes)
        // 0x40: Fore color (3 bytes)
        // 0x80: Brush Org X (1 byte)

        // 0x0100: Brush Org Y (1 byte)
        // 0x0200: Brush style (1 byte)
        // 0x0400: Brush Hatch (1 byte)

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft   != 0) * 0x01
                       | (dr.dtop     != 0) * 0x02
                       | (dr.dwidth   != 0) * 0x04
                       | (dr.dheight  != 0) * 0x08
                       | (this->rop         != oldcmd.rop        ) *  0x10
                       | (this->back_color  != oldcmd.back_color ) *  0x20
                       | (this->fore_color  != oldcmd.fore_color ) *  0x40

                       | (this->brush.org_x != oldcmd.brush.org_x) *  0x80
                       | (this->brush.org_y != oldcmd.brush.org_y) * 0x100
                       | (this->brush.style != oldcmd.brush.style) * 0x200
                       | (this->brush.hatch != oldcmd.brush.hatch) * 0x400
                       | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0) * 0x800
                       ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10) {
            stream.out_uint8(this->rop);
        }
        if (header.fields & 0x20) {
            emit_rdp_color(stream, this->back_color);
        }
        if (header.fields & 0x40) {
            emit_rdp_color(stream, this->fore_color);
        }

        header.emit_brush(stream, 0x80, this->brush, oldcmd.brush);
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }
        if (header.fields & 0x20) {
            receive_rdp_color(stream, this->back_color);
        }
        if (header.fields & 0x40) {
            receive_rdp_color(stream, this->fore_color);
        }

        header.receive_brush(stream, 0x080, this->brush);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz, common.clip.contains(this->rect));

        if (this->brush.style == 0x3){
            lg += snprintf(
                buffer+lg,
                sz-lg,
                "patblt(rect(%d,%d,%d,%d) rop=%x "
                    "back_color=%x fore_color=%x "
                    "brush.org_x=%d brush.org_y=%d "
                    "brush.style=%d brush.hatch=%d brush.extra=[%.2x %.2x %.2x %.2x %.2x %.2x %.2x])\n",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                unsigned(this->rop),
                this->back_color.as_bgr().to_u32(),
                this->fore_color.as_bgr().to_u32(),
                this->brush.org_x,
                this->brush.org_y,
                this->brush.style,
                this->brush.hatch,
                unsigned(this->brush.extra[0]),
                unsigned(this->brush.extra[1]),
                unsigned(this->brush.extra[2]),
                unsigned(this->brush.extra[3]),
                unsigned(this->brush.extra[4]),
                unsigned(this->brush.extra[5]),
                unsigned(this->brush.extra[6])
                );
        }
        else {
            lg += snprintf(
                buffer+lg,
                sz-lg,
                "patblt(rect(%d,%d,%d,%d) rop=%x "
                    "back_color=%x fore_color=%x "
                    "brush.org_x=%d brush.org_y=%d "
                    "brush.style=%d brush.hatch=%d)\n",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                unsigned(this->rop),
                this->back_color.as_bgr().to_u32(),
                this->fore_color.as_bgr().to_u32(),
                this->brush.org_x,
                this->brush.org_y,
                this->brush.style,
                this->brush.hatch
                );
        }
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect clip) const {
        char buffer[1024];
        this->str(buffer, 1024, RDPOrderCommon(RDP::PATBLT, clip));
        LOG(level, "%s", buffer);
    }

    void move(int offset_x, int offset_y) {
        this->rect = this->rect.offset(offset_x, offset_y);
    }
};
