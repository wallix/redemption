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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "RDPOrdersCommon.hpp"

namespace RDP {

// [MS-RDPEGDI] - 2.2.2.2.1.1.1.1 Coord Field (COORD_FIELD)
// ========================================================
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

// [MS-RDPEGDI] - 2.2.2.2.1.1.2.8 MultiScrBlt (MULTI_SCRBLT_ORDER)
// ===============================================================
// The MultiScrBlt Primary Drawing Order is used to perform multiple
//  bit-block transfers from source regions to destination regions of the
//  screen.

// Encoding order number: 17 (0x11)
// Negotiation order number: 17 (0x11)
// Number of fields: 9
// Number of field encoding bytes: 2
// Maximum encoded field length: 399 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      nLeftRect (variable)     |      nTopRect (variable)      |
// +-------------------------------+-------------------------------+
// |       nWidth (variable)       |       nHeight (variable)      |
// +---------------+---------------+---------------+---------------+
// |      bRop     |        nXSrc (variable)       |     nYSrc     |
// |   (optional)  |                               |   (variable)  |
// +---------------+---------------+---------------+---------------+
// |      ...      | nDeltaEntries |   CodedDeltaList (variable)   |
// |               |   (optional)  |                               |
// +---------------+---------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// nLeftRect (variable): The left coordinate of the destination rectangle
//  specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): The top coordinate of the destination rectangle
//  specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nWidth (variable): The width of the destination rectangle specified by
//  using a Coord Field (see section 2.2.2.2.1.1.1.1).

// nHeight (variable): The height of the destination rectangle specified by
//  using a Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): The index of the ternary raster operation to perform (see
//  section 2.2.2.2.1.1.1.7). The resultant ROP3 operation MUST only depend
//  on the destination and source bits. (There MUST NOT be any dependence on
//  pattern bits.)

// nXSrc (variable): The x-coordinate of the source rectangle specified by
//  using a Coord Field (section 2.2.2.2.1.1.1.1).

// nYSrc (variable): The y-coordinate of the source rectangle specified by
//  using a Coord Field (section 2.2.2.2.1.1.1.1).

// nDeltaEntries (1 byte): An 8-bit, unsigned integer. The number of
//  bounding rectangles described by the CodedDeltaList field.

// CodedDeltaList (variable): A Two-Byte Header Variable Field (section
//  2.2.2.2.1.1.1.3) structure that encapsulates a Delta-Encoded Rectangles
//  (section 2.2.2.2.1.1.1.5) structure that contains bounding rectangles to
//  use when rendering the order. The number of rectangles described by the
//  Delta-Encoded Rectangles structure is specified by the nDeltaEntries
//  field.

class RDPMultiScrBlt {
public:
    Rect       rect {}; //nLeftRect, nTopRect, nWidth, nHeight
    uint8_t    bRop {};
    int16_t    nXSrc {};
    int16_t    nYSrc {};
    uint8_t    nDeltaEntries {};

    DeltaEncodedRectangle deltaEncodedRectangles[45] {};

    static uint8_t id(void)
    {
        return RDP::MULTISCRBLT;
    }

    RDPMultiScrBlt() = default;

    RDPMultiScrBlt( const Rect _rect, uint8_t bRop, int16_t nXSrc, int16_t nYSrc, uint8_t nDeltaEntries
                  , InStream & deltaEncodedRectangles)
    : rect(_rect)
    , bRop(bRop)
    , nXSrc(nXSrc)
    , nYSrc(nYSrc)
    , nDeltaEntries(nDeltaEntries) {
        ::memset(this->deltaEncodedRectangles, 0, sizeof(this->deltaEncodedRectangles));
        for (int i = 0; i < this->nDeltaEntries; i++) {
            this->deltaEncodedRectangles[i].leftDelta = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].topDelta  = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].width     = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].height    = deltaEncodedRectangles.in_sint16_le();
        }
    }

    RDPMultiScrBlt & operator=(const RDPMultiScrBlt & other) = default;

    void emit( OutStream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon
             , const RDPMultiScrBlt & oldcmd) const {
        RDPPrimaryOrderHeader header(RDP::STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // MultiScrBlt fields bytes (2 byte)
        // =================================
        // 0x0001: nLeftRect
        // 0x0002: nTopRect
        // 0x0004: nWidth
        // 0x0008: nHeight
        // 0x0010: bRop
        // 0x0020: nXSrc
        // 0x0040: nYSrc
        // 0x0080: nDeltaEntries
        // 0x0100: CodedDeltaList

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we have bounds.
        //  Can't see the rationale and rdesktop don't do it by the book.
        //  Behavior should be checked with server and clients from
        //  Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft            != 0                   ) * 0x0001
                      | (dr.dtop             != 0                   ) * 0x0002
                      | (dr.dwidth           != 0                   ) * 0x0004
                      | (dr.dheight          != 0                   ) * 0x0008

                      | (this->bRop          != oldcmd.bRop         ) * 0x0010
                      | (this->nXSrc         != oldcmd.nXSrc        ) * 0x0020
                      | (this->nYSrc         != oldcmd.nYSrc        ) * 0x0040

                      | (this->nDeltaEntries != oldcmd.nDeltaEntries) * 0x0080
                      | (
                         (this->nDeltaEntries != oldcmd.nDeltaEntries) ||
                         memcmp(this->deltaEncodedRectangles, oldcmd.deltaEncodedRectangles,
                                this->nDeltaEntries * sizeof(RDP::DeltaEncodedRectangle))
                                                                    ) * 0x0100
                      ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x0001, this->rect, oldcmd.rect);

        if (header.fields & 0x0010) {
            stream.out_uint8(this->bRop);
        }

        header.emit_coord(stream, 0x0020, this->nXSrc, oldcmd.nXSrc);
        header.emit_coord(stream, 0x0040, this->nYSrc, oldcmd.nYSrc);

        if (header.fields & 0x0080) { stream.out_uint8(this->nDeltaEntries); }

        if (header.fields & 0x0100) {
            uint32_t offset_cbData = stream.get_offset();
            stream.out_clear_bytes(2);
            uint8_t * zeroBit = stream.get_current();
            stream.out_clear_bytes((this->nDeltaEntries + 1) / 2);
            *zeroBit = 0;

            for (uint8_t i = 0, m2 = 0; i < this->nDeltaEntries; i++, m2++) {
                if (m2 == 2) {
                    m2 = 0;
                }

                if (i && !m2) {
                    *(++zeroBit) = 0;
                }

                if (!this->deltaEncodedRectangles[i].leftDelta) {
                    *zeroBit |= (1 << (7 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].leftDelta);
                }

                if (!this->deltaEncodedRectangles[i].topDelta) {
                    *zeroBit |= (1 << (6 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].topDelta);
                }

                if (!this->deltaEncodedRectangles[i].width) {
                    *zeroBit |= (1 << (5 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].width);
                }

                if (!this->deltaEncodedRectangles[i].height) {
                    *zeroBit |= (1 << (4 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].height);
                }
            }

            stream.set_out_uint16_le(stream.get_offset() - offset_cbData - 2, offset_cbData);
        }
    }   // void emit( OutStream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon, const RDPMultiScrBlt & oldcmd) const

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header) {
        //LOG(LOG_INFO, "RDPMultiScrBlt::receive: header fields=0x%02X", header.fields);

        header.receive_rect(stream, 0x0001, this->rect);

        if (header.fields & 0x0010) {
            this->bRop = stream.in_uint8();
        }

        header.receive_coord(stream, 0x0020, this->nXSrc);
        header.receive_coord(stream, 0x0040, this->nYSrc);


        if (header.fields & 0x0080) {
            this->nDeltaEntries = stream.in_uint8();
        }

        if (header.fields & 0x0100) {
            uint16_t cbData = stream.in_uint16_le();
            //LOG(LOG_INFO, "cbData=%d", cbData);

            InStream rgbData(stream.get_current(), cbData);
            stream.in_skip_bytes(cbData);
            //hexdump_d(rgbData.get_current(), rgbData.get_capacity());

            uint8_t zeroBitsSize = ((this->nDeltaEntries + 1) / 2);
            //LOG(LOG_INFO, "zeroBitsSize=%d", zeroBitsSize);

            InStream zeroBits(rgbData.get_current(), zeroBitsSize);
            rgbData.in_skip_bytes(zeroBitsSize);

            uint8_t zeroBit = 0;

            for (uint8_t i = 0, m2 = 0; i < this->nDeltaEntries; i++, m2++) {
                if (m2 == 2) {
                    m2 = 0;
                }

                if (!m2) {
                    zeroBit = zeroBits.in_uint8();
                    //LOG(LOG_INFO, "0x%02X", zeroBit);
                }

                this->deltaEncodedRectangles[i].leftDelta = (!(zeroBit & 0x80) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].topDelta  = (!(zeroBit & 0x40) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].width     = (!(zeroBit & 0x20) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].height    = (!(zeroBit & 0x10) ? rgbData.in_DEP() : 0);

                //LOG(LOG_INFO, "RDPMultiScrBlt::receive: delta rectangle=(%d, %d, %d, %d)",
                //    this->deltaEncodedRectangles[i].leftDelta, this->deltaEncodedRectangles[i].topDelta,
                //    this->deltaEncodedRectangles[i].width, this->deltaEncodedRectangles[i].height);

                zeroBit <<= 4;
            }
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        size_t lg = 0;
        lg += common.str(buffer + lg, sz - lg, true);
        lg += snprintf( buffer + lg, sz - lg
                      , "MultiSrcBlt(nLeftRect=%d nTopRect=%d nWidth=%d nHeight=%d bRop=0x%02X "
                        "nXSrc=%d nXSrc=%d "
                        "nDeltaEntries=%d "
                        "CodedDeltaList=("
                      , this->rect.x, this->rect.y, this->rect.cx, this->rect.cy
                      , unsigned(this->bRop), this->nXSrc, this->nYSrc, this->nDeltaEntries);
        for (uint8_t i = 0; i < this->nDeltaEntries; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sz - lg, " ");
            }
            lg += snprintf(buffer + lg, sz - lg, "(%d, %d, %d, %d)", this->deltaEncodedRectangles[i].leftDelta,
                this->deltaEncodedRectangles[i].topDelta, this->deltaEncodedRectangles[i].width,
                this->deltaEncodedRectangles[i].height);
        }
        lg += snprintf(buffer + lg, sz - lg, "))");
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }

    void print(const Rect clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        printf("%s", buffer);
    }
};  // class RDPMultiScrBlt

}   // namespace RDP

