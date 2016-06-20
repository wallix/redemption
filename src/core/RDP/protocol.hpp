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

#include "utils/stream.hpp"
#include "utils/colors.hpp"
#include "capabilities/common.hpp"
#include <cassert>

namespace RDP {

// [MS-RDPEGDI] - 2.2.2.1 Orders Update (TS_UPDATE_ORDERS_PDU_DATA)
// ================================================================

// The TS_UPDATE_ORDERS_PDU_DATA structure contains primary, secondary, and
//  alternate secondary drawing orders aligned on byte boundaries. This
//  structure conforms to the layout of a Slow Path Graphics Update (see
//  [MS-RDPBCGR] section 2.2.9.1.1.3.1) and is encapsulated within a Graphics
//  Update PDU (see [MS-RDPBCGR] section 2.2.9.1.1.3.1.1).

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
// +-------------------------------+-------------------------------+
// |              ...              |           updateType          |
// +-------------------------------+-------------------------------+
// |          pad2OctetsA          |          numberOrders         |
// +-------------------------------+-------------------------------+
// |          pad2OctetsB          |      orderData (variable)     |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// shareDataHeader (18 bytes): Share Data Header (see [MS-RDPBCGR], section
//  2.2.8.1.1.1.2) containing information about the packet. The type subfield
//  of the pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
//  MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//  Header MUST be set to PDUTYPE2_UPDATE (2).

// updateType (2 bytes): A 16-bit, unsigned integer. The field contains the
//  graphics update type. This field MUST be set to UPDATETYPE_ORDERS
//  (0x0000).

// pad2OctetsA (2 bytes): A 16-bit, unsigned integer used as a padding field.
//  Values in this field are arbitrary and MUST be ignored.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//  Order (section 2.2.2.1.1) structures contained in the orderData field.

// pad2OctetsB (2 bytes): A 16-bit, unsigned integer used as a padding field.
//  Values in this field are arbitrary and MUST be ignored.

// orderData (variable): A variable-sized array of Drawing Order (section
//  2.2.2.1.1) structures packed on byte boundaries. Each structure contains
//  a primary, secondary, or alternate secondary drawing order. The
//  controlFlags field of the Drawing Order identifies the type of drawing
//  order.

// [MS-RDPEGDI] - 2.2.2.2 Fast-Path Orders Update (TS_FP_UPDATE_ORDERS)
// ====================================================================

// The TS_FP_UPDATE_ORDERS structure contains primary, secondary, and
//  alternate secondary drawing orders aligned on byte boundaries. This
//  structure conforms to the layout of a Fast-Path Update (see [MS-RDPBCGR]
//  section 2.2.9.1.2.1) and is encapsulated within a Fast-Path Update PDU
//  (see [MS-RDPBCGR] section 2.2.9.1.2.1.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  updateHeader |  compression  |              size             |
// |               |     Flags     |                               |
// |               |   (optional)  |                               |
// +---------------+---------------+-------------------------------+
// |          numberOrders         |      orderData (variable)     |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// updateHeader (1 byte): An 8-bit, unsigned integer. The format of this field
//  is the same as the updateHeader byte field described in the Fast-Path
//  Update structure (see [MS-RDPBCGR] section 2.2.9.1.2.1). The updateCode
//  bitfield (4 bits in size) MUST be set to FASTPATH_UPDATETYPE_ORDERS (0x0).

// compressionFlags (1 byte): An 8-bit, unsigned integer. The format of this
//  optional field (as well as the possible values) is the same as the
//  compressionFlags field described in the Fast-Path Update structure
//  specified in [MS-RDPBCGR] section 2.2.9.1.2.1.

// size (2 bytes): A 16-bit, unsigned integer. The format of this field (as
//  well as the possible values) is the same as the size field described in
//  the Fast-Path Update structure specified in [MS-RDPBCGR] section
//  2.2.9.1.2.1.

// numberOrders (2 bytes): A 16-bit, unsigned integer. The number of Drawing
//  Order (section 2.2.2.1.1) structures contained in the orderData field.

// orderData (variable): A variable-sized array of Drawing Order (section
//  2.2.2.1.1) structures packed on byte boundaries. Each structure contains a
//  primary, secondary, or alternate secondary drawing order. The controlFlags
//  field of the Drawing Order identifies the type of drawing order.

struct OrdersUpdate_Recv {
    uint16_t number_orders;

    OrdersUpdate_Recv(InStream & stream, bool fast_path) {
        if (fast_path) {
            this->number_orders = stream.in_uint16_le();
        }
        else {
            stream.in_skip_bytes(2);    /* pad2OctetsA */
            this->number_orders = stream.in_uint16_le();
            stream.in_skip_bytes(2);    /* pad2OctetsB */
        }
    }
};

// [MS-RDPEGDI] - 2.2.2.2.1 Drawing Order (DRAWING_ORDER)
// ======================================================
// The DRAWING_ORDER structure is used to describe and encapsulate a single
//  primary, secondary, or alternate secondary drawing order sent from server
//  to client. All drawing orders conform to this basic structure (see
//  sections 2.2.2.2.1.1.2, 2.2.2.2.1.2.1.1, and 2.2.2.2.1.3.1.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  controlFlags |          orderSpecificData (variable)         |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// controlFlags (1 byte): An 8-bit, unsigned integer. A control byte that
//  identifies the class of the drawing order.
//
//  If the TS_STANDARD (0x01) flag is set, the order is a primary drawing
//   order. If both the TS_STANDARD (0x01) and TS_SECONDARY (0x02) flags are
//   set, the order is a secondary drawing order. Finally, if only the
//   TS_SECONDARY (0x02) flag is set, the order is an alternate secondary
//   drawing order.
//
//  More flags MAY be present, depending on the drawing order class. The flags
//   listed are common to all three classes of drawing orders.

//  +--------------+-------+
//  | Name         | Value |
//  +--------------+-------+
//  | TS_STANDARD  | 0x01  |
//  +--------------+-------+
//  | TS_SECONDARY | 0x02  |
//  +--------------+-------+

//  orderSpecificData (variable): Variable-length data specific to the drawing
//   order class and the drawing order itself.

struct DrawingOrder_RecvFactory {
    uint8_t control_flags;

    explicit DrawingOrder_RecvFactory(InStream & stream) {
        this->control_flags = stream.in_uint8();
    }
};

// [MS-RDPBCGR] - 2.2.9.1.1.3.1.1.1 Palette Update Data
//  (TS_UPDATE_PALETTE_DATA)
// ====================================================

// The TS_UPDATE_PALETTE_DATA encapsulates the palette data that defines a
//  Palette Update (section 2.2.9.1.1.3.1.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           updateType          |           pad2Octets          |
// +-------------------------------+-------------------------------+
// |                          numberColors                         |
// +---------------------------------------------------------------+
// |                   paletteEntries (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// updateType (2 bytes): A 16-bit, unsigned integer. The update type. This
//  field MUST be set to UPDATETYPE_PALETTE (0x0002).

// pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//  field MUST be ignored.

// numberColors (4 bytes): A 32-bit, unsigned integer. The number of RGB
//  triplets in the paletteData field. This field MUST be set to 256 (the
//  number of entries in an 8 bpp palette).

// paletteEntries (variable): An array of palette entries in RGB triplet
//  format (section 2.2.9.1.1.3.1.1.2) packed on byte boundaries. The number
//  of triplet entries is given by the numberColors field.

// [MS-RDPBCGR] - 2.2.9.1.1.3.1.1.2 RGB Palette Entry (TS_PALETTE_ENTRY)
// =====================================================================

// The TS_PALETTE_ENTRY structure is used to express the red, green, and blue
//  components necessary to reproduce a color in the additive RGB space.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      red      |     green     |      blue     |
// +---------------+---------------+---------------+

// red (1 byte): An 8-bit, unsigned integer. The red RGB color component.

// green (1 byte): An 8-bit, unsigned integer. The green RGB color component.

// blue (1 byte): An 8-bit, unsigned integer. The blue RGB color component.

struct UpdatePaletteData_Recv {
    UpdatePaletteData_Recv(InStream & stream, bool fast_path, BGRPalette & palette) {
        if (fast_path) {
            stream.in_skip_bytes(2);    // updateType(2)
        }

        stream.in_skip_bytes(2);    // pad2Octets(2)

        uint32_t numberColors = stream.in_uint32_le();
        assert(numberColors == 256);

        for (uint32_t i = 0; i < numberColors; i++) {
            uint8_t const r = stream.in_uint8();
            uint8_t const g = stream.in_uint8();
            uint8_t const b = stream.in_uint8();
            palette.set_color(i, (r << 16) | (g << 8) | b);
        }
    }
};

// [MS-RDPBCGR] - 2.2.1.13.2 Client Confirm Active PDU
// ===================================================

// The Confirm Active PDU is an RDP Connection Sequence PDU sent from client
//  to server during the Capabilities Exchange phase of the RDP Connection
//  Sequence (see section 1.3.1.1 for an overview of the RDP Connection
//  Sequence phases). It is sent as a response to the Demand Active PDU
//  (section 2.2.1.13.1). Once the Confirm Active PDU has been sent, the
//  client can start sending input PDUs (see section 2.2.8) to the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           tpktHeader                          |
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
// |                confirmActivePduData (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//  section 13.7.

// mcsSDrq (variable): Variable-length PER-encoded MCS Domain PDU
//  (DomainMCSPDU) which encapsulates an MCS Send Data Request structure
//  (SDrq, choice 25 from DomainMCSPDU), as specified in [T125] section 11.32
//  (the ASN.1 structure definitions are given in [T125] section 7, parts 7
//  and 10). The userData field of the MCS Send Data Request contains a
//  Security Header and a Confirm Active PDU Data (section 2.2.1.13.2)
//  structure.

// securityHeader (variable): Optional security header. The presence and
//  format of the security header depends on the Encryption Level and
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).
//  If the Encryption Level selected by the server is greater than
//  ENCRYPTION_LEVEL_NONE (0) and the Encryption Method selected by the server
//  is greater than ENCRYPTION_METHOD_NONE (0) then this field MUST contain
//  one of the following headers:

//  * Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//    Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
//    ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
//    (0x00000002).

//  * FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
//    selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010).

// If the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE (0)
//  and the Encryption Method selected by the server is ENCRYPTION_METHOD_NONE
//  (0), then this header MUST NOT be included in the PDU.

// confirmActivePduData (variable): The contents of the Confirm Active PDU, as
//  specified in section 2.2.1.13.2.1.

// [MS-RDPBCGR] - 2.2.1.13.2.1 Confirm Active PDU Data
//  (TS_CONFIRM_ACTIVE_PDU)
// ===================================================

// The TS_CONFIRM_ACTIVE_PDU structure is a standard T.128 Confirm Active PDU
//  (see [T128] section 8.4.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       shareControlHeader                      |
// +-------------------------------+-------------------------------+
// |              ...              |            shareId            |
// +-------------------------------+-------------------------------+
// |              ...              |          originatorId         |
// +-------------------------------+-------------------------------+
// |     lengthSourceDescriptor    |   lengthCombinedCapabilities  |
// +-------------------------------+-------------------------------+
// |                  sourceDescriptor (variable)                  |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |       numberCapabilities      |           pad2Octets          |
// +-------------------------------+-------------------------------+
// |                   capabilitySets (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
//  containing information about the packet. The type subfield of the pduType
//  field of the Share Control Header MUST be set to PDUTYPE_CONFIRMACTIVEPDU
//  (3).

// shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the
//  packet (see [T128] section 8.4.2 for more information regarding share
//  IDs).

// originatorId (2 bytes): A 16-bit, unsigned integer. The identifier of the
//  packet originator. This field MUST be set to the server channel ID
//  (0x03EA).

// lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in
//  bytes of the sourceDescriptor field.

// lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The
//  combined size in bytes of the numberCapabilities, pad2Octets and
//  capabilitySets fields.

// sourceDescriptor (variable): A variable-length array of bytes containing a
//  source descriptor (see [T128] section 8.4.1 for more information regarding
//  source descriptors).

// numberCapabilities (2 bytes): A 16-bit, unsigned integer. Number of
//  capability sets included in the Confirm Active PDU.

// pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//  field MUST be ignored.

// capabilitySets (variable): An array of Capability Set (section
//  2.2.1.13.1.1.1) structures. The number of capability sets is specified by
//  the numberCapabilities field.

struct ConfirmActivePDU_Send {
    OutStream & payload;
    uint16_t   offset_lengthCombinedCapabilities;
    uint16_t   offset_numberCapabilities;
    uint16_t   numberCapabilities;
    uint16_t   offset_capabilitySets;

    explicit ConfirmActivePDU_Send(OutStream & stream)
        : payload(stream)
        , offset_lengthCombinedCapabilities(0)
        , offset_numberCapabilities(0)
        , numberCapabilities(0)
        , offset_capabilitySets(0) {
    }

    void emit_begin(uint32_t shareId) {
        const char * sourceDescriptor       = "MSTSC";
        uint16_t     lengthSourceDescriptor = ::strlen(sourceDescriptor);

        // Payload

        // shareId (4 bytes): A 32-bit, unsigned integer. The share identifier
        //  for the packet (see [T128] section 8.4.2 for more information
        //  regarding share IDs).
        payload.out_uint32_le(shareId);

        // riginatorId (2 bytes): A 16-bit, unsigned integer. The identifier
        //  of the packet originator. This field MUST be set to the server
        //  channel ID (0x03EA).
        payload.out_uint16_le(0x03EA);

        // lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The
        //  size in bytes of the sourceDescriptor field.
        payload.out_uint16_le(lengthSourceDescriptor);

        // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer.
        //  The combined size in bytes of the numberCapabilities, pad2Octets
        //  and capabilitySets fields.
        this->offset_lengthCombinedCapabilities = payload.get_offset();
        payload.out_uint16_le(0);

        // sourceDescriptor (variable): A variable-length array of bytes
        //  containing a source descriptor (see [T128] section 8.4.1 for more
        //  information regarding source descriptors).
        payload.out_copy_bytes(sourceDescriptor, lengthSourceDescriptor);

        // numberCapabilities (2 bytes): A 16-bit, unsigned integer. Number of
        //  capability sets included in the Confirm Active PDU.
        this->offset_numberCapabilities = payload.get_offset();
        this->numberCapabilities        = 0;
        payload.out_uint16_le(0);

        // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values
        //  in this field MUST be ignored.
        payload.out_clear_bytes(2);

        this->offset_capabilitySets = payload.get_offset();
    }

    void emit_capability_set(Capability & capability) {
        capability.emit(this->payload);
        this->numberCapabilities++;
    }

    void emit_end() {
        uint16_t lengthCombinedCapabilities;

        // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer.
        //  The combined size in bytes of the numberCapabilities, pad2Octets
        //  and capabilitySets fields.
        lengthCombinedCapabilities =
              2 // numberCapabilities(2)
            + 2 // pad2Octets(2)
            + payload.get_offset() - this->offset_capabilitySets;
        this->payload.set_out_uint16_le( lengthCombinedCapabilities
                                       , this->offset_lengthCombinedCapabilities);

        // numberCapabilities (2 bytes): A 16-bit, unsigned integer. Number of
        //  capability sets included in the Confirm Active PDU.
        this->payload.set_out_uint16_le(this->numberCapabilities, this->offset_numberCapabilities);
    }
};  // struct ConfirmActivePDU_Send

// [MS-RDPBCGR] - 2.2.6.1 Virtual Channel PDU
// ==========================================

// The Virtual Channel PDU is sent from client to server or from server to
//  client and is used to transport data between static virtual channel
//  endpoints.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           tpktHeader                          |
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
// |                        channelPduHeader                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 virtualChannelData (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//  section 13.7.

// mcsPdu (variable): If the PDU is being sent from client to server, this
//  field MUST contain a variable-length, PER-encoded MCS Domain PDU
//  (DomainMCSPDU) which encapsulates an MCS Send Data Request structure
//  (SDrq, choice 25 from DomainMCSPDU), as specified in [T125] section 11.32
//  (the ASN.1 structure definition is given in [T125] section 7, parts 7 and
//  10). The userData field of the MCS Send Data Request contains a Security
//  Header and the static virtual channel data.
//
//  If the PDU is being sent from server to client, this field MUST contain a
//   variable-length, PER-encoded MCS Domain PDU (DomainMCSPDU) which
//   encapsulates an MCS Send Data Indication structure (SDin, choice 26 from
//   DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure
//   definition is given in [T125] section 7, parts 7 and 10). The userData
//   field of the MCS Send Data Indication contains a Security Header and the
//   static virtual channel data.

// securityHeader (variable): Optional security header. The presence and
//  format of the security header depends on the Encryption Level and
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).
//  If the Encryption Level selected by the server is greater than
//  ENCRYPTION_LEVEL_NONE (0) and the Encryption Method selected by the
//  server is greater than ENCRYPTION_METHOD_NONE (0), then this field MUST
//  contain one of the security headers described in section 2.2.8.1.1.2.
//
//  If the PDU is being sent from client to server:
//
//  * The securityHeader field MUST contain a Non-FIPS Security Header
//    (section 2.2.8.1.1.2.2) if the Encryption Method selected by the server
//    is ENCRYPTION_METHOD_40BIT (0x00000001), ENCRYPTION_METHOD_56BIT
//    (0x00000008), or ENCRYPTION_METHOD_128BIT (0x00000002).
//
//  If the PDU is being sent from server to client:
//
//  * The securityHeader field MUST contain a Basic Security Header (section
//    2.2.8.1.1.2.1) if the Encryption Level selected by the server is
//    ENCRYPTION_LEVEL_LOW (1).
//
//  * The securityHeader field MUST contain a Non-FIPS Security Header
//    (section 2.2.8.1.1.2.2) if the Encryption Method selected by the server
//    is ENCRYPTION_METHOD_40BIT (0x00000001), ENCRYPTION_METHOD_56BIT
//    (0x00000008), or ENCRYPTION_METHOD_128BIT (0x00000002).
//
//  If the Encryption Method selected by the server is ENCRYPTION_METHOD_FIPS
//   (0x00000010) the securityHeader field MUST contain a FIPS Security Header
//   (section 2.2.8.1.1.2.3).
//
//  If the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE
//   (0) and the Encryption Method selected by the server is
//   ENCRYPTION_METHOD_NONE (0), then this header MUST NOT be included in the
//   PDU.

// channelPduHeader (8 bytes): A Channel PDU Header (section 2.2.6.1.1)
//  structure, which contains control flags and describes the size of the
//  opaque channel data.

// virtualChannelData (variable): Variable-length data to be processed by the
//  static virtual channel protocol handler. This field MUST NOT be larger
//  than CHANNEL_CHUNK_LENGTH (1600) bytes in size unless the maximum virtual
//  channel chunk size is specified in the optional VCChunkSize field of the
//  Virtual Channel Capability Set (section 2.2.7.1.10).

// [MS-RDPBCGR] - 2.2.6.1.1 Channel PDU Header (CHANNEL_PDU_HEADER)
// ================================================================

// The CHANNEL_PDU_HEADER MUST precede all opaque static virtual channel
//  traffic chunks transmitted via RDP between a client and server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             length                            |
// +---------------------------------------------------------------+
// |                             flags                             |
// +---------------------------------------------------------------+

// length (4 bytes): A 32-bit, unsigned integer. The total length in bytes of
//  the uncompressed channel data, excluding this header. The data can span
//  multiple Virtual Channel PDUs and the individual chunks will need to be
//  reassembled in that case (section 3.1.5.2.2).

// flags (4 bytes): A 32-bit, unsigned integer. The channel control flags.

//  +----------------------------+---------------------------------------------+
//  | Flag                       | Meaning                                     |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_FLAG_FIRST         | Indicates that the chunk is the first in a  |
//  | 0x00000001                 | sequence.                                   |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_FLAG_LAST          | Indicates that the chunk is the last in a   |
//  | 0x00000002                 | sequence.                                   |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_FLAG_SHOW_PROTOCOL | The Channel PDU Header MUST be visible to   |
//  | 0x00000010                 | the application endpoint (see section       |
//  |                            | 2.2.1.3.4.1).                               |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_FLAG_SUSPEND       | All virtual channel traffic MUST be         |
//  | 0x00000020 suspended.      | This flag is only valid in server-to-client |
//  |                            | virtual channel traffic. It MUST be ignored |
//  |                            | in client-to-server data.                   |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_FLAG_RESUME        | All virtual channel traffic MUST be         |
//  | 0x00000040                 | resumed. This flag is only valid in         |
//  |                            | server-to-client virtual channel traffic.   |
//  |                            | It MUST be ignored in client-to-server      |
//  |                            | data.                                       |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_PACKET_COMPRESSED  | The virtual channel data is compressed.     |
//  | 0x00200000                 | This flag is equivalent to MPPC bit C (for  |
//  |                            | more information see [RFC2118] section      |
//  |                            | 3.1).                                       |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_PACKET_AT_FRONT    | The decompressed packet MUST be placed at   |
//  | 0x00400000                 | the beginning of the history buffer. This   |
//  |                            | flag is equivalent to MPPC bit B (for more  |
//  |                            | information see [RFC2118] section 3.1).     |
//  +----------------------------+---------------------------------------------+
//  | CHANNEL_PACKET_FLUSHED     | The decompressor MUST reinitialize the      |
//  | 0x00800000                 | history buffer (by filling it with zeros)   |
//  |                            | and reset the HistoryOffset to zero. After  |
//  |                            | it has been reinitialized, the entire       |
//  |                            | history buffer is immediately regarded as   |
//  |                            | valid. This flag is equivalent to MPPC bit  |
//  |                            | A (for more information see [RFC2118]       |
//  |                            | section 3.1). If the                        |
//  |                            | CHANNEL_PACKET_COMPRESSED (0x00200000) flag |
//  |                            | is also present, then the                   |
//  |                            | CHANNEL_PACKET_FLUSHED flag MUST be         |
//  |                            | processed first.                            |
//  +----------------------------+---------------------------------------------+
//  | CompressionTypeMask        | Indicates the compression package which was |
//  | 0x000F0000                 | used to compress the data. See the          |
//  |                            | discussion which follows this table for a   |
//  |                            | list of compression packages.               |
//  +----------------------------+---------------------------------------------+

//  If neither the CHANNEL_FLAG_FIRST (0x00000001) nor the CHANNEL_FLAG_LAST
//   (0x00000002) flag is present, the chunk is from the middle of a
//   sequence.

//  Instructions specifying how to set the compression flags can be found in
//   section 3.1.8.2.1.

//  Possible compression types are as follows.

//  +-------------------------+------------------------------------------------+
//  | Value                   | Meaning                                        |
//  +-------------------------+------------------------------------------------+
//  | PACKET_COMPR_TYPE_8K    | RDP 4.0 bulk compression (see section          |
//  | 0x0                     | 3.1.8.4.1).                                    |
//  +-------------------------+------------------------------------------------+
//  | PACKET_COMPR_TYPE_64K   | RDP 5.0 bulk compression (see section          |
//  | 0x1                     | 3.1.8.4.2).                                    |
//  +-------------------------+------------------------------------------------+
//  | PACKET_COMPR_TYPE_RDP6  | RDP 6.0 bulk compression (see [MS-RDPEGDI]     |
//  | 0x2                     | section 3.1.8.1).                              |
//  +-------------------------+------------------------------------------------+
//  | PACKET_COMPR_TYPE_RDP61 | RDP 6.1 bulk compression (see [MS-RDPEGDI]     |
//  | 0x3                     | section 3.1.8.2).                              |
//  +-------------------------+------------------------------------------------+

//  Instructions detailing how to compress a data stream are listed in
//   section 3.1.8.2, while decompression of a data stream is described in
//   section 3.1.8.3.

}   // namespace RDP

