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

#ifndef _REDEMPTION_CORE_RDP_PROTOCOL_HPP_
#define _REDEMPTION_CORE_RDP_PROTOCOL_HPP_

namespace RDP {

// 2.2.2.1 Orders Update (TS_UPDATE_ORDERS_PDU_DATA)
// =================================================

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

// 2.2.2.2 Fast-Path Orders Update (TS_FP_UPDATE_ORDERS)
// =====================================================

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

    OrdersUpdate_Recv(Stream & stream, bool fast_path) {
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

// 2.2.2.2.1 Drawing Order (DRAWING_ORDER)
// =======================================
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

// +--------------+-------+
// | Name         | Value |
// +--------------+-------+
// | TS_STANDARD  | 0x01  |
// +--------------+-------+
// | TS_SECONDARY | 0x02  |
// +--------------+-------+

//  orderSpecificData (variable): Variable-length data specific to the drawing
//   order class and the drawing order itself.

struct DrawingOrder_RecvFactory {
    uint8_t control_flags;

    DrawingOrder_RecvFactory(Stream & stream) {
        this->control_flags = stream.in_uint8();
    }
};

}   // namespace RDP

#endif  // #ifndef _REDEMPTION_CORE_RDP_PROTOCOL_HPP_