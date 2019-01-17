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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities :

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

// 2.2.7.1.2    Bitmap Capability Set (TS_BITMAP_CAPABILITYSET)
// ============================================================

//  The TS_BITMAP_CAPABILITYSET structure is used to advertise bitmap-oriented
//    characteristics and is based on the capability set specified in [T128]
// section 8.2.4. This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//   capability set. This field MUST be set to CAPSTYPE_BITMAP (2).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//   of the capability data, including the size of the capabilitySetType and
//   lengthCapability fields.

// preferredBitsPerPixel (2 bytes): A 16-bit, unsigned integer. Color depth of
//   the remote session. In RDP 4.0 and 5.0, this field MUST be set to 8 (even
//   for a 16-color session).

// receive1BitPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   the client can receive 1 bpp. This field is ignored and SHOULD be set to
//   TRUE (0x0001).

// receive4BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   the client can receive 4 bpp. This field is ignored and SHOULD be set to
//   TRUE (0x0001).

// receive8BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//    the client can receive 8 bpp. This field is ignored and SHOULD be set to
//    TRUE (0x0001).

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The width of the desktop
//   in the remote session.

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The height of the
//   desktop in the remote session.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//   field are ignored.

// desktopResizeFlag (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   desktop resizing is supported.
//   0x0000 FALSE  Desktop resizing is not supported.
//   0x0001 TRUE   Desktop resizing is supported.
//   If a desktop resize occurs, the server will deactivate the session (see
//   section 1.3.1.3), and on session reactivation will specify the new desktop
//   size in the desktopWidth and desktopHeight fields in the Bitmap Capability
//   Set, along with a value of TRUE for the desktopResizeFlag field. The client
//   should check these sizes and, if different from the previous desktop size,
//   resize any windows to support this size.

// bitmapCompressionFlag (2 bytes): A 16-bit, unsigned integer. Indicates
//   whether the client supports bitmap compression. RDP requires bitmap
//   compression and hence this field MUST be set to TRUE (0x0001). If it is not
//   set to TRUE, the server MUST NOT continue with the connection.

// highColorFlags (1 byte): An 8-bit, unsigned integer. Client support for
//   16 bpp color modes. This field is ignored and SHOULD be set to 0.

// drawingFlags (1 byte): An 8-bit, unsigned integer. Flags describing support
//   for 32 bpp bitmaps.

// +----------------------------------------+----------------------------------+
// | 0x02 DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY | Indicates support for lossy      |
// |                                        | compression of 32 bpp bitmaps by |
// |                                        | reducing color-fidelity on a     |
// |                                        | per-pixel basis.                 |
// +----------------------------------------+----------------------------------+
// | 0x04 DRAW_ALLOW_COLOR_SUBSAMPLING      | Indicates support for chroma     |
// |                                        | subsampling when compressing     |
// |                                        | 32 bpp bitmaps.                  |
// +----------------------------------------+----------------------------------+
// | 0x08 DRAW_ALLOW_SKIP_ALPHA             | Indicates that the client        |
// |                                        | supports the removal of the      |
// |                                        | alpha-channel when compressing   |
// |                                        | 32 bpp bitmaps. In this case the |
// |                                        | alpha is assumed to be 0xFF,     |
// |                                        | meaning the bitmap is opaque.    |
// |                                        | Compression of 32 bpp bitmaps is |
// |                                        | specified in [MS-RDPEGDI]        |
// |                                        | section 3.1.9.                   |
// +----------------------------------------+----------------------------------+

// multipleRectangleSupport (2 bytes): A 16-bit, unsigned integer. Indicates
//   whether the client supports the use of multiple bitmap rectangles. RDP
//   requires the use of multiple bitmap rectangles and hence this field MUST be
//   set to TRUE (0x0001). If it is not set to TRUE, the server MUST NOT
//   continue with the connection.

// pad2octetsB (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//   field are ignored.

enum {
    DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY = 0x02,
    DRAW_ALLOW_COLOR_SUBSAMPLING = 0x04,
    DRAW_ALLOW_SKIP_ALPHA = 0x08
};

enum {
    CAPLEN_BITMAP = 28
};

struct BitmapCaps : public Capability {
    uint16_t preferredBitsPerPixel{8};
    uint16_t receive1BitPerPixel{1};
    uint16_t receive4BitsPerPixel{1};
    uint16_t receive8BitsPerPixel{1};
    uint16_t desktopWidth{0};
    uint16_t desktopHeight{0};
    uint16_t pad2octets{0};
    uint16_t desktopResizeFlag{1};
    uint16_t bitmapCompressionFlag{1};
    uint8_t  highColorFlags{0};
    uint8_t  drawingFlags{0};
    uint16_t multipleRectangleSupport{1};
    uint16_t pad2octetsB{0};
    BitmapCaps()
    : Capability(CAPSTYPE_BITMAP, CAPLEN_BITMAP)
     
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->preferredBitsPerPixel);
        stream.out_uint16_le(this->receive1BitPerPixel);
        stream.out_uint16_le(this->receive4BitsPerPixel);
        stream.out_uint16_le(this->receive8BitsPerPixel);
        stream.out_uint16_le(this->desktopWidth);
        stream.out_uint16_le(this->desktopHeight);
        stream.out_uint16_le(this->pad2octets);
        stream.out_uint16_le(this->desktopResizeFlag);
        stream.out_uint16_le(this->bitmapCompressionFlag);
        stream.out_uint8(this->highColorFlags);
        stream.out_uint8(this->drawingFlags);
        stream.out_uint16_le(this->multipleRectangleSupport);
        stream.out_uint16_le(this->pad2octetsB);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;

        /* preferredBitsPerPixel(2) + receive1BitPerPixel(2) + receive4BitsPerPixel(2) + receive8BitsPerPixel(2) +
         * desktopWidth(2) + desktopHeight(2) + pad2octets(2) + desktopResizeFlag(2) + bitmapCompressionFlag(2) +
         * highColorFlags(1) + drawingFlags(1) + multipleRectangleSupport(2) + pad2octetsB(2)
         */
        const unsigned expected = 24;
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated BitmapCaps, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->preferredBitsPerPixel = stream.in_uint16_le();
        this->receive1BitPerPixel = stream.in_uint16_le();
        this->receive4BitsPerPixel = stream.in_uint16_le();
        this->receive8BitsPerPixel = stream.in_uint16_le();
        this->desktopWidth = stream.in_uint16_le();
        this->desktopHeight = stream.in_uint16_le();
        this->pad2octets = stream.in_uint16_le();
        this->desktopResizeFlag = stream.in_uint16_le();
        this->bitmapCompressionFlag = stream.in_uint16_le();
        this->highColorFlags = stream.in_uint8();
        this->drawingFlags = stream.in_uint8();
        this->multipleRectangleSupport = stream.in_uint16_le();
        this->pad2octetsB = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s Bitmap caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     Bitmap caps::preferredBitsPerPixel %u", this->preferredBitsPerPixel);
        LOG(LOG_INFO, "     Bitmap caps::receive1BitPerPixel %u", this->receive1BitPerPixel);
        LOG(LOG_INFO, "     Bitmap caps::receive4BitsPerPixel %u", this->receive4BitsPerPixel);
        LOG(LOG_INFO, "     Bitmap caps::receive8BitsPerPixel %u", this->receive8BitsPerPixel);
        LOG(LOG_INFO, "     Bitmap caps::desktopWidth %u", this->desktopWidth);
        LOG(LOG_INFO, "     Bitmap caps::desktopHeight %u", this->desktopHeight);
        LOG(LOG_INFO, "     Bitmap caps::pad2octets %u", this->pad2octets);
        LOG(LOG_INFO, "     Bitmap caps::desktopResizeFlag %u (%s)",
            this->desktopResizeFlag, this->desktopResizeFlag?"yes":"no");
        LOG(LOG_INFO, "     Bitmap caps::bitmapCompressionFlag %u %s",
            this->bitmapCompressionFlag, this->bitmapCompressionFlag?"yes":"no");
        LOG(LOG_INFO, "     Bitmap caps::highColorFlags %u", this->highColorFlags);
        LOG(LOG_INFO, "     Bitmap caps::drawingFlags %u", this->drawingFlags);
        LOG(LOG_INFO, "     Bitmap caps::drawingFlags:DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY %s",
            (this->drawingFlags & DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY)?"yes":"no");
        LOG(LOG_INFO, "     Bitmap caps::drawingFlags:DRAW_ALLOW_COLOR_SUBSAMPLING %s",
            (this->drawingFlags & DRAW_ALLOW_COLOR_SUBSAMPLING)?"yes":"no");
        LOG(LOG_INFO, "     Bitmap caps::drawingFlags:DRAW_ALLOW_SKIP_ALPHA %s",
            (this->drawingFlags & DRAW_ALLOW_SKIP_ALPHA)?"yes":"no");
        LOG(LOG_INFO, "     Bitmap caps::multipleRectangleSupport %u", this->multipleRectangleSupport);
        LOG(LOG_INFO, "     Bitmap caps::pad2octetsB %u", this->pad2octetsB);
    }

    void dump(FILE * f) const
    {
       fprintf(f, "[Bitmap Capability Set]\n");

       fprintf(f, "preferredBitsPerPixel=%u\n",    unsigned(this->preferredBitsPerPixel));
       fprintf(f, "receive1BitPerPixel=%u\n",      unsigned(this->receive1BitPerPixel));
       fprintf(f, "receive4BitsPerPixel=%u\n",     unsigned(this->receive4BitsPerPixel));
       fprintf(f, "receive8BitsPerPixel=%u\n",     unsigned(this->receive8BitsPerPixel));
       fprintf(f, "desktopWidth=%u\n",             unsigned(this->desktopWidth));
       fprintf(f, "desktopHeight=%u\n",            unsigned(this->desktopHeight));
       fprintf(f, "desktopResizeFlag=%u\n",        unsigned(this->desktopResizeFlag));
       fprintf(f, "bitmapCompressionFlag=%u\n",    unsigned(this->bitmapCompressionFlag));
       fprintf(f, "highColorFlags=%u\n",           unsigned(this->highColorFlags));
       fprintf(f, "drawingFlags=%u\n",             unsigned(this->drawingFlags));
       fprintf(f, "multipleRectangleSupport=%u\n\n", unsigned(this->multipleRectangleSupport));
    }
};
