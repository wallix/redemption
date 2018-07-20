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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   out_per_bstream object, formatted buffering for entities expecting per format
*/


#pragma once

#include "utils/log.hpp"

#include "utils/stream.hpp"

// =========================================================================
// PER encoding rules support methods
// =========================================================================

inline void out_per_length(OutStream& out, uint16_t length)
{
    out.out_2BUE(length);
}

inline void out_per_choice(OutStream& out, uint8_t choice)
{
    out.out_uint8(choice);
}

inline void out_per_selection(OutStream& out, uint8_t selection)
{
    out.out_uint8(selection);
}

inline void out_per_number_of_sets(OutStream& out, uint8_t number)
{
    out.out_uint8(number);
}

inline void out_per_padding(OutStream& out, uint8_t length)
{
    out.out_clear_bytes(length);
}

inline void out_per_integer(OutStream& out, uint32_t v)
{
    uint8_t length = (v & 0xFFFF0000)?4:(v & 0xFF00)?2:1;
    out.out_uint8(length);
    switch (length){
    case 4:
        out.out_uint32_be(v);
        break;
    case 2:
        out.out_uint16_be(static_cast<uint16_t>(v));
        break;
    default:
        out.out_uint8(static_cast<uint8_t>(v));
        break;
    }
}

inline void out_per_object_identifier(OutStream& out, const uint8_t * oid)
{
    const uint8_t t12 = (oid[0] << 4) & (oid[1] & 0x0F);
    out_per_length(out, 5); // length
    out.out_uint8(t12);    // first two tuples
    out.out_uint8(oid[2]); // tuple 3
    out.out_uint8(oid[3]); // tuple 4
    out.out_uint8(oid[4]); // tuple 5
    out.out_uint8(oid[5]); // tuple 6
}

//    16 Encoding the octetstring type
//    ================================
//    NOTE – Octet strings of fixed length less than or equal to two octets are
//    not octet-aligned. All other octet strings are octet-aligned in the ALIGNED
//    variant. Fixed length octet strings encode with no length octets if they
//    are shorter than 64K. For unconstrained octet strings the length is
//    explicitly encoded (with fragmentation if necessary).

//    16.1 PER-visible constraints can only constrain the length of the octetstring.

//    16.2 Let the maximum number of octets in the octetstring (as determined by
//    PER-visible constraints on the length) be "ub" and the minimum number of
//    octets be "lb". If there is no finite maximum we say that "ub" is unset.
//    If there is no constraint on the minimum then "lb" has the value zero.
//    Let the length of the actual octetstring value to be encoded be "n" octets.

//    16.3 If there is a PER-visible size constraint and an extension marker is
//    present in it, a single bit shall be added to the field-list in a bit-field
//    of length one. The bit shall be set to 1 if the length of this encoding is
//    not within the range of the extension root, and zero otherwise. In the
//    former case 16.8 shall be invoked to add the length as a semi-constrained
//    whole number to the field-list, followed by the octetstring value. In the
//    latter case the length and value shall be encoded as if the extension
//    marker is not present.

//    16.4 If an extension marker is not present in the constraint specification
//    of the octetstring type, then 16.5 to 16.8 apply.

//    16.5 If the octetstring is constrained to be of zero length ("ub" equals
//    zero), then it shall not be encoded (no additions to the field-list),
//    completing the procedures of this clause.

//    16.6 If all values of the octetstring are constrained to be of the same
//    length ("ub" equals "lb") and that length is less than or equal to two
//    octets, the octetstring shall be placed in a bit-field with a number of
//    bits equal to the constrained length "ub" multiplied by eight which shall
//    be appended to the field-list with no length determinant, completing the
//    procedures of this clause.

//    16.7 If all values of the octetstring are constrained to be of the same
//    length ("ub" equals "lb") and that length is greater than two octets but
//    less than 64K, then the octetstring shall be placed in a bit-field
//    (octet-aligned in the ALIGNED variant) with the constrained length "ub"
//    octets which shall be appended to the field-list with no length determinant,
//    completing the procedures of this clause.

//    16.8 If 16.5 to 16.7 do not apply, the octetstring shall be placed in a
//    bit-field (octet-aligned in the ALIGNED variant) of length "n" octets and
//    the procedures of 10.9 shall be invoked to add this bit-field (octet-aligned
//    in the ALIGNED variant) of "n" octets to the field-list, preceded by a
//    length determinant equal to "n" octets as a constrained whole number if "ub"
//    is set, and as a semi-constrained whole number if "ub" is unset. "lb" is
//    as determined above.

//    NOTE – The fragmentation procedures may apply after 16K, 32K, 48K, or 64K octets.

inline void out_per_octet_string(OutStream& out, const uint8_t * oct_str, uint32_t length, uint32_t min)
{
    if (length >= min){
        out_per_length(out, length - min);
        out.out_copy_bytes(oct_str, length);
    }
    else {
        // TODO Check this length, looks dubious
        out_per_length(out, min);
        out.out_copy_bytes(oct_str, length);
        out.out_clear_bytes(min-length);
    }
}
