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


#pragma once

#include "core/RDP/x224.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/share.hpp"
#include "core/RDP/sec.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/out_per_bstream.hpp"

namespace RDP {

// 2.2.11.2 Client Refresh Rect PDU
// ================================
// The Refresh Rect PDU allows the client to request that the server redraw
//  one or more rectangles of the session screen area. The client can use it
//  to repaint sections of the client window that were obscured by other
//  windowed applications. Server support for this PDU is indicated in the
//  General Capability Set (section 2.2.7.1.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          tpktHeader                           |
// +-----------------------------------------------+---------------+
// |                    x224Data                   |    mcsSDrq    |
// |                                               |   (variable)  |
// +-----------------------------------------------+---------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   securityHeader (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 refreshRectPduData (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//  section 13.7.

// mcsSDrq (variable): Variable-length PER-encoded MCS Domain PDU
//  (DomainMCSPDU) which encapsulates an MCS Send Data Request structure
//  (SDrq, choice 25 from DomainMCSPDU), as specified in [T125] section 11.32
//  (the ASN.1 structure definitions are given [T125] in section 7, parts 7
//  and 10). The userData field of the MCS Send Data Request contains a
//  Security Header and a Refresh Rect PDU Data (section 2.2.11.2.1)
//  structure.

// securityHeader (variable): Optional security header. The presence and
//  format of the security header depends on the Encryption Level and
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).
//  If the Encryption Level selected by the server is greater than
//  ENCRYPTION_LEVEL_NONE (0) and the Encryption Method selected by the
//  server is greater than ENCRYPTION_METHOD_NONE (0), then this field MUST
//  contain one of the following headers:

//  * Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//    Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
//    ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
//    (0x00000002).

//  * FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
//    selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010).

// If the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE
//  (0) and the Encryption Method selected by the server is
//  ENCRYPTION_METHOD_NONE (0), then this header MUST NOT be included in the
//  PDU.

// refreshRectPduData (variable): The actual contents of the Refresh Rect
//  PDU, as specified in section 2.2.11.2.1.

// 2.2.11.2.1 Refresh Rect PDU Data (TS_REFRESH_RECT_PDU)
// ======================================================
// The TS_REFRESH_RECT_PDU structure contains the contents of the Refresh
//  Rect PDU, which is a Share Data Header (section 2.2.8.1.1.1.2) and two
//  fields.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        shareDataHeader                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+---------------+---------------+
// |              ...              | numberOfAreas |  pad3Octects  |
// +-------------------------------+---------------+---------------+
// |              ...              |   areasToRefresh (variable)   |
// +-------------------------------+---------------+---------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+

// shareDataHeader (18 bytes): A Share Data Header containing information
//  about the packet. The type subfield of the pduType field of the Share
//  Control Header (section 2.2.8.1.1.1.1) MUST be set to PDUTYPE_DATAPDU
//  (7). The pduType2 field of the Share Data Header MUST be set to
//  PDUTYPE2_REFRESH_RECT (33).

// numberOfAreas (1 byte): An 8-bit, unsigned integer. The number of
//  Inclusive Rectangle (section 2.2.11.1) structures in the areasToRefresh
//  field.

// pad3Octects (3 bytes): A 3-element array of 8-bit, unsigned integer
//  values. Padding. Values in this field MUST be ignored.

// areasToRefresh (variable): An array of TS_RECTANGLE16 structures (variable
//  number of bytes). Array of screen area Inclusive Rectangles to redraw.
//  The number of rectangles is given by the numberOfAreas field.

struct RefreshRectPDU {
    StaticOutStream<65536> buffer_stream;

    ShareData sdata;

    uint16_t userId;

    int            encryptionLevel;
    CryptContext & encrypt;

    uint8_t  area_count;
    uint32_t offset_area_count;

    RefreshRectPDU(uint32_t shareId,
                   uint16_t userId,
                   int encryptionLevel,
                   CryptContext & encrypt) :
    sdata(buffer_stream),
    userId(userId),
    encryptionLevel(encryptionLevel),
    encrypt(encrypt),
    area_count(0),
    offset_area_count(0) {
        this->sdata.emit_begin(PDUTYPE2_REFRESH_RECT,
                               shareId,
                               RDP::STREAM_MED);

        this->offset_area_count = this->buffer_stream.get_offset();
        this->buffer_stream.out_clear_bytes(1); /* number of areas, set later */

        this->buffer_stream.out_clear_bytes(3); /* pad */
    }

    void addInclusiveRect(uint16_t left,
                          uint16_t top,
                          uint16_t right,
                          uint16_t bottom) {
        buffer_stream.out_uint16_le(left);
        buffer_stream.out_uint16_le(top);
        buffer_stream.out_uint16_le(right);
        buffer_stream.out_uint16_le(bottom);

        area_count++;
    }

    void emit(OutTransport trans) /* TODO const*/ {
        this->buffer_stream.set_out_uint8(this->area_count,
                                          this->offset_area_count);

        this->sdata.emit_end();

        write_packets(
            trans,
            [this](StreamSize<65536+256>, OutStream & sctrl_header) {
                ShareControl_Send(sctrl_header,
                                  PDUTYPE_DATAPDU,
                                  this->userId + GCC::MCS_USERCHANNEL_BASE,
                                  this->buffer_stream.get_offset());

                sctrl_header.out_copy_bytes(this->buffer_stream.get_data(),
                                            this->buffer_stream.get_offset());
            },
            [this](StreamSize<256>, OutStream & sec_header, uint8_t * pkt_data, std::size_t pkt_size) {
                SEC::Sec_Send sec(sec_header,
                                  pkt_data,
                                  pkt_size,
                                  0,
                                  this->encrypt,
                                  this->encryptionLevel);
                (void)sec;
            },
            [this](StreamSize<256>, OutStream & mcs_header, std::size_t pkt_size) {
                MCS::SendDataRequest_Send mcs(static_cast<OutPerStream&>(mcs_header),
                                              this->userId,
                                              GCC::MCS_GLOBAL_CHANNEL,
                                              1,
                                              3,
                                              pkt_size,
                                              MCS::PER_ENCODING);
                (void)mcs;
            },
            [](StreamSize<256>, OutStream &x224_header, std::size_t pkt_size) {
                X224::DT_TPDU_Send(x224_header, pkt_size);
            }
        );
    }
};  // struct RefreshRectPDU

}   // namespace RDP

