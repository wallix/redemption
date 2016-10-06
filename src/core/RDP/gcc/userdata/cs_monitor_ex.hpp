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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/RDP/gcc/data_block_type.hpp"
#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

namespace GCC { namespace UserData {
// 2.2.1.3.9 Client Monitor Extended Data (TS_UD_CS_MONITOR_EX)
// ============================================================

// The TS_UD_CS_MONITOR_EX packet describes extended attributes of the client-side display
// monitor layout defined by the Client Monitor Data block (section 2.2.1.3.6). This packet
// is an Extended Client Data Block and MUST NOT be sent to a server which does not advertise
// support for Extended Client Data Blocks by using the EXTENDED_CLIENT_DATA_SUPPORTED flag
// (0x00000001) as described in section 2.2.1.2.1.

//    header (4 bytes): A GCC user data block header, as specified in User Data Header
// (section 2.2.1.3.1). The User Data Header type field MUST be set to CS_MONITOR_EX (0xC008).

//    flags (4 bytes): A 32-bit, unsigned integer. This field is unused and reserved for
// future use. It MUST be set to zero.

//    monitorAttributeSize (4 bytes): A 32-bit, unsigned integer. The size, in bytes, of a
// single element in the monitorAttributesArray field. This field MUST be set to 20 bytes,
// which is the size of the Monitor Attributes structure (section 2.2.1.3.9.1).

//    monitorCount (4 bytes): A 32-bit, unsigned integer. The number of elements in the
// monitorAttributesArray field. This value MUST be the same as the monitorCount field
// specified in the Client Monitor Data (section 2.2.1.3.6 block (section).

//    monitorAttributesArray (variable): A variable-length array containing a series of
// TS_MONITOR_ATTRIBUTES structures (section 2.2.1.3.9.1) which describe extended attributes
// of each display monitor specified in the Client Monitor Data block. The number of
// TS_MONITOR_ATTRIBUTES structures is specified by the monitorCount field.


}} // namespaces
