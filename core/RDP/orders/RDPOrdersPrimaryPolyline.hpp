/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYPOLYLINE_HPP_
#define _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYPOLYLINE_HPP_

// 2.2.2.2.1.1.1.1 Coord Field (COORD_FIELD)
// =========================================
// The COORD_FIELD structure is used to describe a value in the range -32768
//  to 32767.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     signedValue (variable)    |
// +-------------------------------+

// signedValue (variable): A signed, 1-byte or 2-byte value that describes a
//  coordinate in the range -32768 to 32767.

//  When the controlFlags field (see section 2.2.2.2.1.1.2) of the primary
//   drawing order that contains the COORD_FIELD structure has the
//   TS_DELTA_COORDINATES flag (0x10) set, the signedValue field MUST contain
//   a signed 1-byte value. If the TS_DELTA_COORDINATES flag is not set, the
//   signedValue field MUST contain a 2-byte signed value.

//  The 1-byte format contains a signed delta from the previous value of the
//   Coord field. To obtain the new value of the field, the decoder MUST
//   increment the previous value of the field by the signed delta to produce
//   the current value. The 2-byte format is simply the full value of the
//   field that MUST replace the previous value.

// 2.2.2.2.1.1.1.2 One-Byte Header Variable Field (VARIABLE1_FIELD)
// ================================================================
// TheVARIABLE1_FIELD structure is used to encode a variable-length byte-
//  stream that will hold a maximum of 255 bytes. This structure is always
//  situated at the end of an order.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     cbData    |               rgbData (variable)              |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cbData (1 byte): An 8-bit, unsigned integer. The number of bytes present
//  in the rgbData field.

// rgbData (variable): Variable-length, binary data. The size of this data,
//  in bytes, is given by the cbData field.

// 2.2.2.2.1.1.1.4 Delta-Encoded Points (DELTA_PTS_FIELD)
// ======================================================
// The DELTA_PTS_FIELD structure is used to encode a series of points. Each
//  point is expressed as an X and Y delta from the previous point in the
//  series (the first X and Y deltas are relative to a base point that MUST
//  be included in the order that contains the DELTA_PTS_FIELD structure).
//  The number of points is order-dependent and is not specified by any field
//  within the DELTA_PTS_FIELD structure. Instead, a separate field within
//  the order that contains the DELTA_PTS_FIELD structure MUST be used to
//  specify the number of points (this field SHOULD<1> be placed immediately
//  before the DELTA_PTS_FIELD structure in the order encoding).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      zeroBits (variable)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 deltaEncodedPoints (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// zeroBits (variable): A variable-length byte field. The zeroBits field is
//  used to indicate the absence of an X or Y delta value for a specific
//  point in the series. The size in bytes of the zeroBits field is given by
//  ceil(NumPoints / 4) where NumPoints is the number of points being
//  encoded. Each point in the series requires two zero-bits (four points per
//  byte) to indicate whether an X or Y delta value is zero (and not
//  present), starting with the most significant bits, so that for the first
//  point the X-zero flag is set at (zeroBits[0] & 0x80), and the Y-zero flag
//  is set at (zeroBits[0] & 0x40).

// deltaEncodedPoints (variable): A variable-length byte field. The
//  deltaEncodedPoints field contains a series of (X, Y) pairs, each pair
//  specifying the delta from the previous pair in the series (the first pair
//  in the series contains a delta from a pre-established coordinate).

//  The presence of the X and Y delta values for a given pair in the series
//   is dictated by the individual bits of the zeroBits field. If the zero
//   bit is set for a given X or Y component, its value is unchanged from the
//   previous X or Y component in the series (a delta of zero), and no data
//   is provided. If the zero bit is not set for a given X or Y component,
//   the delta value it represents is encoded in a packed signed format:

//   * If the high bit (0x80) is not set in the first encoding byte, the
//     field is 1 byte long and is encoded as a signed delta in the lower 7
//     bits of the byte.

//   * If the high bit of the first encoding byte is set, the lower 7 bits of
//     the first byte and the 8 bits of the next byte are concatenated (the
//     first byte containing the high-order bits) to create a 15-bit signed
//     delta value.

// 2.2.2.2.1.1.2.18 Polyline (POLYLINE_ORDER)
// ==========================================
// The Polyline Primary Drawing Order encodes a solid color polyline
//  consisting of two or more vertices connected by straight lines.

//  Encoding order number: 22 (0x16)
//  Negotiation order number: 22 (0x16)
//  Number of fields: 7
//  Number of field encoding bytes: 1
//  Maximum encoded field length: 148 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       xStart (variable)       |       yStart (variable)       |
// +---------------+---------------+---------------+---------------+
// |     bRop2     |   BrushCacheEntry (optional)  |    PenColor   |
// |   (optional)  |                               |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |              ...              |NumDeltaEntries| CodedDeltaList|
// |                               |   (optional)  |   (variable)  |
// +-------------------------------+---------------+---------------+
// |              ...              |
// +-------------------------------+

// xStart (variable): The x-coordinate of the starting point of the polygon
//  path specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// yStart (variable): The y-coordinate of the starting point of the polygon
//  path specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// bRop2 (1 byte): The binary raster operation to perform (see section
//  2.2.2.2.1.1.1.6).

// BrushCacheEntry (2 bytes): A 16-bit unsigned integer. The brush cache
//  entry. This field is unused (as only solid color polylines are drawn) and
//  SHOULD<3> be set to 0x0000.

// PenColor (3 bytes): The foreground color described by using a Generic
//  Color (section 2.2.2.2.1.1.1.8) structure.

// NumDeltaEntries (1 byte): An 8-bit, unsigned integer. The number of points
//  along the polyline path described by the CodedDeltaList field.

// CodedDeltaList (variable): A One-Byte Header Variable Field (section
//  2.2.2.2.1.1.1.2) structure that encapsulates a Delta-Encoded Points
//  (section 2.2.2.2.1.1.1.4) structure that contains the points along the
//  polyline path. The number of points described by the Delta-Encoded Points
//  structure is specified by the NumDeltaEntries field.

class RDPPolyline {
public:
    int16_t  xStart;
    int16_t  yStart;
    uint8_t  bRop2;
    uint16_t BrushCacheEntry;
    uint32_t PenColor;
    uint8_t  NumDeltaEntries;

    struct {
        int16_t xDelta;
        int16_t yDelta;
    } deltaPoints [128];

    static const uint8_t id(void) {
        return RDP::POLYLINE;
    }

    RDPPolyline()
    : xStart(0)
    , yStart(0)
    , bRop2(0x0)
    , BrushCacheEntry(0x0000)
    , PenColor(0x00000000)
    , NumDeltaEntries(0) {
        ::memset(this->deltaPoints, 0, sizeof(this->deltaPoints));
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header) {
        // LOG(LOG_INFO, "RDPPolyline::receive: header fields=0x%02X", header.fields);

        header.receive_coord(stream, 0x0001, this->xStart);
        header.receive_coord(stream, 0x0002, this->yStart);
        if (header.fields & 0x0004) {
            this->bRop2  = stream.in_uint8();
        }
        if (header.fields & 0x0008) {
            this->BrushCacheEntry = stream.in_uint16_le();
        }

        if (header.fields & 0x0010) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->PenColor = r + (g << 8) + (b << 16);
        }

        if (header.fields & 0x0020) {
            this->NumDeltaEntries = stream.in_uint8();
        }

        if (header.fields & 0x0040) {
            uint8_t cbData = stream.in_uint8();
            // LOG(LOG_INFO, "cbData=%d", cbData);

            SubStream rgbData(stream, stream.get_offset(), cbData);
            stream.in_skip_bytes(cbData);
            // hexdump_d(rgbData.p, rgbData.size());

            uint8_t zeroBitsSize = ((this->NumDeltaEntries + 3) / 4);
            // LOG(LOG_INFO, "zeroBitsSize=%d", zeroBitsSize);

            SubStream zeroBits(rgbData, rgbData.get_offset(), zeroBitsSize);
            rgbData.in_skip_bytes(zeroBitsSize);

            uint8_t zeroBit = 0;
            int16_t xDelta;
            int16_t yDelta;

            for (uint8_t i = 0; i < this->NumDeltaEntries; i++) {
                if (!(i % 4)) {
                    zeroBit = zeroBits.in_uint8();
                    // LOG(LOG_INFO, "0x%02X", zeroBit);
                }
                if (!(zeroBit & 0x80)) {
                    xDelta = rgbData.in_uint8();
                    if (xDelta & 0x80)
                    {
                        xDelta = ((xDelta & 0x7F) << 8) + rgbData.in_uint8();
                        if (xDelta & 0x4000)
                            xDelta = - ((~(xDelta - 1)) & 0x7FFF);
                    }
                    else
                    {
                        if (xDelta & 0x40)
                            xDelta = - ((~(xDelta - 1)) & 0x7F);
                    }
                }
                else {
                    xDelta = 0;
                }
                if (!(zeroBit & 0x40)) {
                    yDelta = rgbData.in_uint8();
                    if (yDelta & 0x80)
                    {
                        yDelta = ((yDelta & 0x7F) << 8) + rgbData.in_uint8();
                        if (yDelta & 0x4000)
                            yDelta = - ((~(yDelta - 1)) & 0x7FFF);
                    }
                    else
                    {
                        if (yDelta & 0x40)
                            yDelta = - ((~(yDelta - 1)) & 0x7F);
                    }
                }
                else {
                    yDelta = 0;
                }

                // LOG(LOG_INFO, "RDPPolyline::receive: delta point=%d, %d", xDelta, yDelta);

                this->deltaPoints[i].xDelta = xDelta;
                this->deltaPoints[i].yDelta = yDelta;

                zeroBit <<= 2;
            }
        }
    }   // receive

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        size_t lg = 0;
        lg += common.str(buffer + lg, sz - lg, true);
        lg += snprintf(buffer + lg, sz - lg,
            "polyline(xStart=%d xStart=%d bRop2=0x%02X BrushCacheEntry=%d PenColor=%.6x "
                "NumDeltaEntries=%d DeltaEntries=(",
            this->xStart, this->yStart, this->bRop2, this->BrushCacheEntry, this->PenColor, this->NumDeltaEntries);
        for (uint8_t i = 0; i < this->NumDeltaEntries; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sz - lg, " ");
            }
            lg += snprintf(buffer + lg, sz - lg, "(%d, %d)", this->deltaPoints[i].xDelta, this->deltaPoints[i].yDelta);
        }
        lg += snprintf(buffer + lg, sz - lg, "))");
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect & clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }

    void print(const Rect & clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        printf("%s", buffer);
    }
};  // class RDPPolyLine

#endif  // #ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYPOLYLINE_HPP_
