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

   RDP Capabilities :

*/

#if !defined(__RDP_CAPABILITIES_POINTER_HPP__)
#define __RDP_CAPABILITIES_POINTER_HPP__

#include "constants.hpp"


// 2.2.7.1.5 Pointer Capability Set (TS_POINTER_CAPABILITYSET)

// The TS_POINTER_CAPABILITYSET structure advertises pointer cache sizes and
// flags and is based on the capability set specified in [T128] section 8.2.11.
// This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
// capability set. This field MUST be set to CAPSTYPE_POINTER (8).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
// of the capability data, including the size of the capabilitySetType and
// lengthCapability fields.

// colorPointerFlag (2 bytes): A 16-bit, unsigned integer. Indicates support for
// color pointers. Since RDP supports monochrome cursors by using Color Pointer
// Updates and New Pointer Updates (sections 2.2.9.1.1.4.4 and 2.2.9.1.1.4.5
// respectively), the value of this field is ignored and is always assumed to be
// TRUE (at a minimum the Color Pointer Update MUST be supported by an RDP
// client).

// +---------------+-----------------------------------------+
// |   Value       |            Meaning                      |
// +---------------+-----------------------------------------+
// | 0x0000 FALSE  | Monochrome mouse cursors are supported. |
// +---------------+-----------------------------------------+
// | 0x0001 TRUE   | Color mouse cursors are supported.      |
// +---------------+-----------------------------------------+

// colorPointerCacheSize (2 bytes): A 16-bit, unsigned integer. The number of
// available slots in the 24 bpp color pointer cache used to store data received
// in the Color Pointer Update (section 2.2.9.1.1.4.4).

// pointerCacheSize (2 bytes): A 16-bit, unsigned integer. The number of
// available slots in the pointer cache used to store pointer data of arbitrary
// bit depth received in the New Pointer Update (section 2.2.9.1.1.4.5).

// If the value contained in this field is zero or the Pointer Capability Set
// sent from the client does not include this field, the server will not use
// the New Pointer Update.


static inline void out_pointer_caps(Stream & stream)
{
    LOG(LOG_INFO, "Sending Pointer caps to server");

    stream.out_uint16_le(RDP_CAPSET_POINTER);
    stream.out_uint16_le(8); // total length of caps
    stream.out_uint16_le(1); /* colorPointerFlag */
    stream.out_uint16_le(20); /* colorPointerCacheSize */
}

static inline void front_out_pointer_caps(Stream & stream)
{
    stream.out_uint16_le(RDP_CAPSET_POINTER);
    stream.out_uint16_le(RDP_CAPLEN_POINTER);
    stream.out_uint16_le(1); /* Colour pointer */
    stream.out_uint16_le(0x19); /* Cache size */
    stream.out_uint16_le(0x19); /* Cache size */
}

#endif
