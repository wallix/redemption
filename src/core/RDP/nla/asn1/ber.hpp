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
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#pragma once

#include "utils/stream.hpp"

namespace BER {
    enum CLASS {
        CLASS_MASK = 0xC0,
        CLASS_UNIV = 0x00,
        CLASS_APPL = 0x40,
        CLASS_CTXT = 0x80,
        CLASS_PRIV = 0xC0
    };

    enum PC {
        PC_MASK      = 0x20,
        PC_PRIMITIVE = 0x00,
        PC_CONSTRUCT = 0x20
    };

    enum TAG {
        TAG_MASK              = 0x1F,
        TAG_BOOLEAN           = 0x01,
        TAG_INTEGER           = 0x02,
        TAG_BIT_STRING        = 0x03,
        TAG_OCTET_STRING      = 0x04,
        TAG_OBJECT_IDENTIFIER = 0x06,
        TAG_ENUMERATED        = 0x0A,
        TAG_SEQUENCE          = 0x10,
        TAG_SEQUENCE_OF       = 0x10,
        TAG_GENERAL_STRING    = 0x1B
    };

    inline PC ber_pc(bool _pc) {
        return (_pc ? PC_CONSTRUCT : PC_PRIMITIVE);
    }

    // ==========================
    //   LENGTH
    // ==========================
    inline bool read_length(InStream & s, int & length) {
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t byte = s.in_uint8();
        if (byte & 0x80) {
            byte &= ~(0x80);
            if (!s.in_check_rem(byte)) {
                return false;
            }
            if (byte == 1) {
                length = s.in_uint8();
            }
            else if (byte == 2) {
                length = s.in_uint16_be();
            }
            else {
                return false;
            }
        }
        else {
            length = byte;
        }
        return true;
    }

    inline int write_length(OutStream & s, int length) {
        int res = 1;
        if (length > 0xFF) {
            s.out_uint8(0x82);
            s.out_uint16_be(length);
            res = 3;
        }
        else if (length > 0x7F) {
            s.out_uint8(0x81);
            s.out_uint8(length);
            res = 2;
        }
        else {
            s.out_uint8(length);
        }
        return res;
    }

    inline int _ber_sizeof_length(int length) {
        int res = 1;
        if (length > 0xFF) {
            res = 3;
        }
        else if (length > 0x7F) {
            res = 2;
        }
        return res;
    }


    // ==========================
    //   UNIVERSAL TAG
    // ==========================
    inline bool read_universal_tag(InStream & s, uint8_t tag, bool pc) {
        uint8_t byte;
        if (!s.in_check_rem(1)) {
            return false;
        }
        byte = s.in_uint8();
        return byte == (CLASS_UNIV | ber_pc(pc) | (TAG_MASK & tag)); /*NOLINT*/
    }

    inline int write_universal_tag(OutStream & s, uint8_t tag, bool pc) {
        s.out_uint8(CLASS_UNIV | ber_pc(pc) | (TAG_MASK & tag)); /*NOLINT*/
        return 1;
    }

    // ==========================
    //   APPLICATION TAG
    // ==========================
    //bool read_application_tag(InStream & s, uint8_t tag, int & length) {
    //    uint8_t byte;
    //    if (tag > 30) {
    //        if (!s.in_check_rem(1))
    //            return false;
    //        byte = s.in_uint8();
    //
    //        if (byte != (CLASS_APPL | PC_CONSTRUCT | TAG_MASK))
    //            return false;
    //
    //        if (!s.in_check_rem(1))
    //            return false;
    //        byte = s.in_uint8();
    //
    //        if (byte != tag)
    //            return false;
    //
    //        return read_length(s, length);
    //    }
    //    else {
    //        if (!s.in_check_rem(1))
    //            return false;
    //        byte = s.in_uint8();
    //
    //        if (byte != (CLASS_APPL | PC_CONSTRUCT | (TAG_MASK & tag)))
    //            return false;
    //        return read_length(s, length);
    //    }
    //    return true;
    //}

    //void write_application_tag(OutStream & s, uint8_t tag, int length) {
    //    if (tag > 30) {
    //        s.out_uint8(CLASS_APPL | PC_CONSTRUCT | TAG_MASK);
    //        s.out_uint8(tag);
    //        write_length(s, length);
    //    }
    //    else {
    //        s.out_uint8(CLASS_APPL | PC_CONSTRUCT | (TAG_MASK & tag));
    //        write_length(s, length);
    //    }
    //}

    // ==========================
    //   CONTEXTUAL TAG
    // ==========================
    inline bool read_contextual_tag(InStream & s, uint8_t tag, int & length, bool pc) {
        uint8_t byte;
        if (!s.in_check_rem(1)) {
            return false;
        }
        byte = s.peek_uint8();
        // LOG(LOG_INFO, "read_contextual_tag read: %x\n", byte);
        if (byte != (CLASS_CTXT | ber_pc(pc) | (TAG_MASK & tag))) { /*NOLINT*/
            return false;
        }
        s.in_skip_bytes(1);
        return read_length(s, length);
    }

    inline int write_contextual_tag(OutStream & s, uint8_t tag, int length, bool pc) {
        s.out_uint8(CLASS_CTXT | ber_pc(pc) | (TAG_MASK & tag)); /*NOLINT*/
        return 1 + write_length(s, length);
    }

    inline int sizeof_contextual_tag(int length) {
        return 1 + _ber_sizeof_length(length);
    }

    // ==========================
    //   SEQUENCE TAG
    // ==========================
    inline bool read_sequence_tag(InStream & s, int & length) {
        uint8_t byte;
        if (!s.in_check_rem(1)) {
            return false;
        }
        byte = s.in_uint8();

        if (byte != (CLASS_UNIV | PC_CONSTRUCT | TAG_SEQUENCE_OF)) { /*NOLINT*/
            return false;
        }

        return read_length(s, length);

    }

    inline int write_sequence_tag(OutStream & s, int length) {
        s.out_uint8(CLASS_UNIV | PC_CONSTRUCT | (TAG_MASK & TAG_SEQUENCE)); /*NOLINT*/
        return 1 + write_length(s, length);
    }

    inline int sizeof_sequence(int length) {
        return 1 + _ber_sizeof_length(length) + length;
    }

    inline int sizeof_sequence_tag(int length) {
        return 1 + _ber_sizeof_length(length);
    }

    // ==========================
    //   ENUMERATED
    // ==========================
    //bool read_enumerated(InStream & s, uint8_t & enumerated, uint8_t count) {
    //    int length;
    //    if (!read_universal_tag(s, TAG_ENUMERATED, false) ||
    //        !read_length(s, length)) {
    //        return false;
    //    }
    //    if (length != 1 || !s.in_check_rem(1)) {
    //        return false;
    //    }
    //    enumerated = s.in_uint8();
    //
    //    if (enumerated + 1 > count) {
    //        return false;
    //    }
    //    return true;
    //}

    //void write_enumerated(OutStream & s, uint8_t enumerated, uint8_t count) {
    //    write_universal_tag(s, TAG_ENUMERATED, false);
    //    write_length(s, 1);
    //    s.out_uint8(enumerated);
    //}

    // ==========================
    //   BIT STRING
    // ==========================
    // bool read_bit_string(InStream & s, int & length, uint8_t & padding) {
    //     if (!read_universal_tag(s, TAG_BIT_STRING, false) ||
    //         !read_length(s, length)) {
    //         return false;
    //     }
    //     if (!s.in_check_rem(1)) {
    //         return false;
    //     }
    //     padding = s.in_uint8();
    //     return true;
    // }

    // ==========================
    //   OCTET STRING
    // ==========================
    inline int write_octet_string(OutStream & s, const uint8_t * oct_str, int length) {
        int size = 0;
        size += write_universal_tag(s, TAG_OCTET_STRING, false);
        size += write_length(s, length);
        s.out_copy_bytes(oct_str, length);
        size += length;
        return size;
    }


    inline bool read_octet_string_tag(InStream & s, int & length) {
        return read_universal_tag(s, TAG_OCTET_STRING, false)
            && read_length(s, length);
    }

    inline int write_octet_string_tag(OutStream & s, int length) {
        write_universal_tag(s, TAG_OCTET_STRING, false);
        write_length(s, length);
        return 1 + _ber_sizeof_length(length);
    }

    inline int sizeof_octet_string(int length) {
        return 1 + _ber_sizeof_length(length) + length;
    }
    inline int sizeof_sequence_octet_string(int length) {
        return sizeof_contextual_tag(sizeof_octet_string(length))
            + sizeof_octet_string(length);
    }

    inline int write_sequence_octet_string(OutStream & stream, uint8_t context,
                                        const uint8_t * value, int length) {
        return write_contextual_tag(stream, context, sizeof_octet_string(length), true)
            + write_octet_string(stream, value, length);
    }

    // ==========================
    //   GENERAL STRING
    // ==========================
    //int write_general_string(OutStream & s, const uint8_t * oct_str, int length) {
    //    int size = 0;
    //    size += write_universal_tag(s, TAG_GENERAL_STRING, false);
    //    size += write_length(s, length);
    //    s.out_copy_bytes(oct_str, length);
    //    size += length;
    //    return size;
    //}
    //bool read_general_string_tag(InStream & s, int & length) {
    //    return read_universal_tag(s, TAG_GENERAL_STRING, false)
    //        && read_length(s, length);
    //}
    //int write_general_string_tag(OutStream & s, int length) {
    //    write_universal_tag(s, TAG_GENERAL_STRING, false);
    //    write_length(s, length);
    //    return 1 + _ber_sizeof_length(length);
    //}
    //int sizeof_general_string(int length) {
    //    return 1 + _ber_sizeof_length(length) + length;
    //}
    // ==========================
    //   BOOL
    // ==========================
    //bool read_bool(InStream & s, bool & value) {
    //    int length;
    //    if (!read_universal_tag(s, TAG_BOOLEAN, false) ||
    //        !read_length(s, length)) {
    //        return false;
    //    }
    //
    //    if (length != 1 || !s.in_check_rem(1)) {
    //        return false;
    //    }
    //    uint8_t v = s.in_uint8();
    //    value = (v ? true : false);
    //    return true;
    //}

    //void write_bool(OutStream & s, bool value) {
    //    write_universal_tag(s, TAG_BOOLEAN, false);
    //    write_length(s, 1);
    //    s.out_uint8(value ? 0xFF : 0);
    //}

    // ==========================
    //   INTEGER
    // ==========================
    inline bool read_integer(InStream & s, uint32_t & value) {
        int length;
        if (!read_universal_tag(s, TAG_INTEGER, false) ||
            !read_length(s, length) ||
            !s.in_check_rem(1)) {
            return false;
        }
        // if (value == nullptr) {
        //     s.in_skip_bytes(length);
        // }

        if (length == 1) {
            value = s.in_uint8();
        }
        else if (length == 2) {
            value = s.in_uint16_be();
        }
        else if (length == 3) {
            uint8_t byte = s.in_uint8();
            value = s.in_uint16_be();
            value += (byte << 16);
        }
        else if (length == 4) {
            value = s.in_uint32_be();
        }
        else {
            return false;
        }
        return true;
    }

    inline int write_integer(OutStream & s, uint32_t value)
    {
        if (value <  0x80) {
            write_universal_tag(s, TAG_INTEGER, false);
            write_length(s, 1);
            s.out_uint8(value);
            return 3;
        }
        if (value <  0x8000) {
            write_universal_tag(s, TAG_INTEGER, false);
            write_length(s, 2);
            s.out_uint16_be(value);
            return 4;
        }
        if (value <  0x800000) {
            write_universal_tag(s, TAG_INTEGER, false);
            write_length(s, 3);
            s.out_uint8(value >> 16);
            s.out_uint16_be(value & 0xFFFF);
            return 5;
        }
        if (value <  0x80000000) {
            write_universal_tag(s, TAG_INTEGER, false);
            write_length(s, 4);
            s.out_uint32_be(value);
            return 6;
        }

        return 0;
    }

    inline int sizeof_integer(uint32_t value) {
        if (value < 0x80) {
            return 3;
        }
        if (value < 0x8000) {
            return 4;
        }
        if (value < 0x800000) {
            return 5;
        }
        if (value < 0x80000000) {
            return 6;
        }
        return 0;
    }

    //bool read_integer_length(InStream & s, int & length)
    //{
    //    return read_universal_tag(s, TAG_INTEGER, false)
    //        && read_length(s, length);
    //}
} // namespace BER
