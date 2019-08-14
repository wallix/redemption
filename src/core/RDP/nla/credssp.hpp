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

#include <cstring>

#include "core/error.hpp"
#include "utils/utf.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/stream.hpp"
#include "core/RDP/nla/ntlm/ntlm_message.hpp"

// BER Encoding Cheat Sheet
// ========================

// BER INTEGER
// -----------
// Only the 4 first patterns are canonical,
// next 4 are using non packed integer length encoding (pretty inefficient)
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 01 XX] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 02 HH LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 03 HH MM LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 04 HH MM LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 01 XX] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 02 HH LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 03 HH MM LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 04 HH MM LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 01 XX] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 02 HH LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 03 HH MM LL] 
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 04 HH MM LL] 

// BER OCTET STRING
// ----------------
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ ll ] [ XX .. XX ]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ 81 LL ] [ XX .. XX]  
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ 82 LH LL ] [ XX .. XX]  
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ 83 LH LM LL ] [ XX .. XX]  
// ll = length of string <= 127
// LL = length of string <= 255
// LHLL = length of string <= 65535
// LHLMLL = length of string 24 bits (unlikely, in credssp context it's probably an error)

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


    inline void backward_push_ber_len(std::vector<uint8_t> & v, uint32_t len)
    {
        v.push_back(uint8_t(len));
        if (len > 0xFF){
            v.push_back(uint8_t(len >> 8));
            v.push_back(0x82);
        } else if (len > 0x7F){
            v.push_back(0x81);
        }
    }

    inline void backward_push_tagged_field_header(std::vector<uint8_t> & v, uint32_t payload_size, uint8_t tag)
    {
       backward_push_ber_len(v, payload_size);
       v.push_back(CLASS_CTXT|PC_CONSTRUCT|tag);
    }

    inline void backward_push_octet_string_field_header(std::vector<uint8_t> & v, uint32_t payload_size)
    {
       backward_push_ber_len(v, payload_size);
       v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING);
    }

    inline void backward_push_sequence_tag_field_header(std::vector<uint8_t> & v, uint32_t payload_size)
    {
        backward_push_ber_len(v, payload_size);
        v.push_back(CLASS_UNIV|PC_CONSTRUCT|TAG_SEQUENCE);
    }

    // for values < 0x80
    inline void backward_push_small_integer_field(std::vector<uint8_t> & v, uint8_t value)
    {
        v.push_back(value);
        v.push_back(1); // length
        v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER);
    }

    inline void backward_push_integer_field(std::vector<uint8_t> & v, uint32_t value)
    {
        if (value < 0x80) {
            v.push_back(value);
            v.push_back(1); // length
        }
        else if (value <  0x8000) {
            v.push_back(value);
            v.push_back(value >> 8);
            v.push_back(2); // length
        }
        else if (value <  0x800000) {
            v.push_back(value);
            v.push_back(value >> 8);
            v.push_back(value >> 16);
            v.push_back(3); // length
        }
        else {
            v.push_back(value);
            v.push_back(value >> 8);
            v.push_back(value >> 16);
            v.push_back(value >> 24);
            v.push_back(4); // length
        }
        v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER);
    }


    inline std::vector<uint8_t> mkOptionalNegoTokensHeader(uint32_t payload_size)
    {
        std::vector<uint8_t> head;

        if (payload_size > 0) {
            backward_push_octet_string_field_header(head, payload_size);
            backward_push_tagged_field_header(head, payload_size + head.size(), 0);
            backward_push_sequence_tag_field_header(head, payload_size + head.size());
            backward_push_sequence_tag_field_header(head, payload_size + head.size());
            backward_push_tagged_field_header(head, payload_size + head.size(), 1);
            std::reverse(head.begin(), head.end());
        }
        return head;
    }

    inline std::vector<uint8_t> mkMandatoryOctetStringFieldHeader(uint32_t payload_size, uint8_t tag)
    {
        std::vector<uint8_t> head;
        backward_push_octet_string_field_header(head, payload_size);
        backward_push_tagged_field_header(head, payload_size + head.size(), tag);
        std::reverse(head.begin(), head.end());
        return head;
    }

    inline std::vector<uint8_t> mkOptionalOctetStringFieldHeader(uint32_t payload_size, uint8_t tag)
    {
        if (payload_size > 0) {
            return mkMandatoryOctetStringFieldHeader(payload_size, tag);
        }
        return {};
    }

    inline std::vector<uint8_t> mkSmallIntegerField(uint8_t value, uint8_t tag)
    {
        std::vector<uint8_t> field;
        backward_push_small_integer_field(field, value);
        backward_push_tagged_field_header(field, field.size(), tag);
        std::reverse(field.begin(), field.end());
        return field;
    }

    inline std::vector<uint8_t> mkIntegerField(uint32_t value, uint8_t tag)
    {
        std::vector<uint8_t> field;
        backward_push_integer_field(field, value);
        backward_push_tagged_field_header(field, field.size(), tag);
        std::reverse(field.begin(), field.end());
        return field;
    }

    inline std::vector<uint8_t> mkTSRequestSequenceHeader(uint32_t payload_size)
    {
        std::vector<uint8_t> head;
        backward_push_sequence_tag_field_header(head, payload_size);
        std::reverse(head.begin(), head.end());
        return head;
    }

    // TO trash after cleanup until end of namespace

    inline PC ber_pc(bool _pc) {
        return (_pc ? PC_CONSTRUCT : PC_PRIMITIVE);
    }

    // ==========================
    //   LENGTH
    // ==========================
    inline int _ber_sizeof_length(int length) {
        return (length <= 0x7F)?1:(length <= 0xFF)?2:3;
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
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t tag_byte = s.peek_uint8();
        // LOG(LOG_INFO, "read_contextual_tag read: %x", byte);
        if (tag_byte != (CLASS_CTXT | ber_pc(pc) | (TAG_MASK & tag))) { /*NOLINT*/
            return false;
        }
        s.in_skip_bytes(1);
        // read length
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t byte = s.in_uint8();
        if (byte & 0x80) {
            if (!s.in_check_rem(byte & 0x7F)) {
                return false;
            }
            if (byte == 0x81) {
                length = s.in_uint8();
                return true;
            }
            if (byte == 0x82) {
                length = s.in_uint16_be();
                return true;
            }
            return false;
        }
        length = byte;
        return true;
    }

    inline int write_contextual_tag(OutStream & s, uint8_t tag, int length, bool pc) {
        s.out_uint8(CLASS_CTXT | ber_pc(pc) | (TAG_MASK & tag)); /*NOLINT*/
        switch (_ber_sizeof_length(length)){
        case 1:
            s.out_uint8(length);
            return 2;
        case 2:
            s.out_uint8(0x81);
            s.out_uint8(length);
            return 3;
        default:
            break;
        }
        s.out_uint8(0x82);
        s.out_uint16_be(length);
        return 4;
    }

    inline int sizeof_contextual_tag(int length) {
        return 1 + _ber_sizeof_length(length);
    }

    // ==========================
    //   SEQUENCE TAG
    // ==========================
    inline bool read_sequence_tag(InStream & s, int & length) {
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t tag_byte = s.in_uint8();

        if (tag_byte != (CLASS_UNIV | PC_CONSTRUCT | TAG_SEQUENCE_OF)) { /*NOLINT*/
            return false;
        }
        // read length
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t byte = s.in_uint8();
        if (byte & 0x80) {
            if (!s.in_check_rem(byte & 0x7F)) {
                return false;
            }
            if (byte == 0x81) {
                length = s.in_uint8();
                return true;
            }
            if (byte == 0x82) {
                length = s.in_uint16_be();
                return true;
            }
            return false;
        }
        length = byte;
        return true;
    }

    inline int write_sequence_tag(OutStream & s, int length) {
        s.out_uint8(CLASS_UNIV | PC_CONSTRUCT | (TAG_MASK & TAG_SEQUENCE)); /*NOLINT*/
        switch (_ber_sizeof_length(length)){
        case 1:
            s.out_uint8(length);
            return 2;
        case 2:
            s.out_uint8(0x81);
            s.out_uint8(length);
            return 3;
        default:
            break;
        }
        s.out_uint8(0x82);
        s.out_uint16_be(length);
        return 4;
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
    //    s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_ENUMERATED));
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
        s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_OCTET_STRING));
        size += 1;
        switch (_ber_sizeof_length(length)){
        case 1:
            s.out_uint8(length);
            size += 1;
            break;
        case 2:
            s.out_uint8(0x81);
            s.out_uint8(length);
            size += 2;
            break;
        default:
            s.out_uint8(0x82);
            s.out_uint16_be(length);
            size += 3;
            break;
        }
        s.out_copy_bytes(oct_str, length);
        size += length;
        return size;
    }


    inline bool read_octet_string_tag(InStream & s, int & length) {
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t tag_byte = s.in_uint8();
        if  (tag_byte != (CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_OCTET_STRING))){
            return false;
        }
        // read length
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t byte = s.in_uint8();
        if (byte & 0x80) {
            if (!s.in_check_rem(byte & 0x7F)) {
                return false;
            }
            if (byte == 0x81) {
                length = s.in_uint8();
                return true;
            }
            if (byte == 0x82) {
                length = s.in_uint16_be();
                return true;
            }
            return false;
        }
        length = byte;
        return true;
    }

    inline int write_octet_string_tag(OutStream & s, int length) {
        s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_OCTET_STRING));
        switch (_ber_sizeof_length(length)){
        case 1:
            s.out_uint8(length);
            break;
        case 2:
            s.out_uint8(0x81);
            s.out_uint8(length);
            break;
        default:
            s.out_uint8(0x82);
            s.out_uint16_be(length);
            break;
        }
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
        auto size = write_contextual_tag(stream, context, sizeof_octet_string(length), true);
        stream.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_OCTET_STRING));
        size += 1;
        switch (_ber_sizeof_length(length)){
        case 1:
            stream.out_uint8(length);
            size += 1;
            break;
        case 2:
            stream.out_uint8(0x81);
            stream.out_uint8(length);
            size += 2;
            break;
        default:
            stream.out_uint8(0x82);
            stream.out_uint16_be(length);
            size += 3;
            break;
        }
        stream.out_copy_bytes(value, length);
        size += length;
        return size;
    }

    // ==========================
    //   GENERAL STRING
    // ==========================
    //int write_general_string(OutStream & s, const uint8_t * oct_str, int length) {
    //    int size = 0;
    //    s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_GENERAL_STRING));
    //    size += 1;
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
    //    s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_GENERAL_STRING));
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
    //    s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_BOOLEAN));
    //    write_length(s, 1);
    //    s.out_uint8(value ? 0xFF : 0);
    //}

    // ==========================
    //   INTEGER
    // ==========================
    
    inline bool read_integer(InStream & s, uint32_t & value) {
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t tag_byte = s.in_uint8();
        if  (tag_byte != (CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_INTEGER))){
            return false;
        }
        // read length of following value
        if (!s.in_check_rem(1)) {
            return false;
        }
        uint8_t byte = s.in_uint8();
        switch (byte) {
        case 0:
            return false;
        case 0x82:
            if (!s.in_check_rem(2)) {
                return false;
            }
            byte = s.in_uint16_be();
            break;
        case 0x81:
            if (!s.in_check_rem(1)) {
                return false;
            }
            byte = s.in_uint8();
            break;
        default:
            break;
        }
        // Now bytes contains length of integer value
        switch (byte) {
        case 4:
            if (!s.in_check_rem(4)) {
                return false;
            }
            value = s.in_uint32_be();
            break;
        case 3:
            if (!s.in_check_rem(3)) {
                return false;
            }
            value = s.in_uint24_be();
            break;
        case 2:
            if (!s.in_check_rem(2)) {
                return false;
            }
            value = s.in_uint16_be();
            break;
        case 1:
            if (!s.in_check_rem(1)) {
                return false;
            }
            value = s.in_uint8();
            break;
        default:
            return false;
        }
        return true;
    }

    inline int write_integer(OutStream & s, uint32_t value)
    {
        s.out_uint8(CLASS_UNIV | PC_PRIMITIVE | (TAG_MASK & TAG_INTEGER));
        if (value <  0x80) {
            s.out_uint8(1); // length
            s.out_uint8(value);
            return 3;
        }
        if (value <  0x8000) {
            s.out_uint8(2); // length
            s.out_uint16_be(value);
            return 4;
        }
        if (value <  0x800000) {
            s.out_uint8(3); // length
            s.out_uint8(value >> 16);
            s.out_uint16_be(value & 0xFFFF);
            return 5;
        }
        if (value <  0x80000000) {
            s.out_uint8(4); // length
            s.out_uint32_be(value);
            return 6;
        }
        return 0;
    }

    inline int sizeof_integer(uint32_t value) {
        return (value < 0x80)?3:(value < 0x8000)?4:(value < 0x800000)?5:(value < 0x80000000)?6:0;
    }
} // namespace BER


enum class PasswordCallback
{
    Error, Ok, Wait
};

namespace credssp {
    enum class State { Err, Cont, Finish, };
}


enum SecIdFlag {
    SEC_WINNT_AUTH_IDENTITY_ANSI = 0x1,
    SEC_WINNT_AUTH_IDENTITY_UNICODE = 0x2
};


enum SEC_STATUS {
    SEC_E_OK                   = 0x00000000,
    SEC_E_INSUFFICIENT_MEMORY  = 0x80090300,
    SEC_E_INVALID_HANDLE       = 0x80090301,
    SEC_E_UNSUPPORTED_FUNCTION = 0x80090302,
    SEC_E_TARGET_UNKNOWN       = 0x80090303,
    SEC_E_INTERNAL_ERROR       = 0x80090304,
    SEC_E_SECPKG_NOT_FOUND     = 0x80090305,
    SEC_E_NOT_OWNER            = 0x80090306,
    SEC_E_CANNOT_INSTALL       = 0x80090307,
    SEC_E_INVALID_TOKEN        = 0x80090308,
    SEC_E_CANNOT_PACK          = 0x80090309,
    SEC_E_QOP_NOT_SUPPORTED    = 0x8009030A,
    SEC_E_NO_IMPERSONATION     = 0x8009030B,
    SEC_E_LOGON_DENIED         = 0x8009030C,
    SEC_E_UNKNOWN_CREDENTIALS  = 0x8009030D,
    SEC_E_NO_CREDENTIALS       = 0x8009030E,
    SEC_E_MESSAGE_ALTERED      = 0x8009030F,
    SEC_E_OUT_OF_SEQUENCE      = 0x80090310,
    SEC_E_NO_AUTHENTICATING_AUTHORITY = 0x80090311,
    SEC_E_BAD_PKGID            = 0x80090316,
    SEC_E_CONTEXT_EXPIRED      = 0x80090317,
    SEC_E_INCOMPLETE_MESSAGE   = 0x80090318,
    SEC_E_INCOMPLETE_CREDENTIALS = 0x80090320,
    SEC_E_BUFFER_TOO_SMALL     = 0x80090321,
    SEC_E_WRONG_PRINCIPAL      = 0x80090322,
    SEC_E_TIME_SKEW            = 0x80090324,
    SEC_E_UNTRUSTED_ROOT       = 0x80090325,
    SEC_E_ILLEGAL_MESSAGE      = 0x80090326,
    SEC_E_CERT_UNKNOWN         = 0x80090327,
    SEC_E_CERT_EXPIRED         = 0x80090328,
    SEC_E_ENCRYPT_FAILURE      = 0x80090329,
    SEC_E_DECRYPT_FAILURE      = 0x80090330,
    SEC_E_ALGORITHM_MISMATCH   = 0x80090331,
    SEC_E_SECURITY_QOS_FAILED  = 0x80090332,
    SEC_E_UNFINISHED_CONTEXT_DELETED = 0x80090333,
    SEC_E_NO_TGT_REPLY         = 0x80090334,
    SEC_E_NO_IP_ADDRESSES      = 0x80090335,
    SEC_E_WRONG_CREDENTIAL_HANDLE = 0x80090336,
    SEC_E_CRYPTO_SYSTEM_INVALID   = 0x80090337,
    SEC_E_MAX_REFERRALS_EXCEEDED  = 0x80090338,
    SEC_E_MUST_BE_KDC          = 0x80090339,
    SEC_E_STRONG_CRYPTO_NOT_SUPPORTED = 0x8009033A,
    SEC_E_TOO_MANY_PRINCIPALS  = 0x8009033B,
    SEC_E_NO_PA_DATA           = 0x8009033C,
    SEC_E_PKINIT_NAME_MISMATCH = 0x8009033D,
    SEC_E_SMARTCARD_LOGON_REQUIRED = 0x8009033E,
    SEC_E_SHUTDOWN_IN_PROGRESS = 0x8009033F,
    SEC_E_KDC_INVALID_REQUEST  = 0x80090340,
    SEC_E_KDC_UNABLE_TO_REFER  = 0x80090341,
    SEC_E_KDC_UNKNOWN_ETYPE    = 0x80090342,
    SEC_E_UNSUPPORTED_PREAUTH  = 0x80090343,
    SEC_E_DELEGATION_REQUIRED  = 0x80090345,
    SEC_E_BAD_BINDINGS         = 0x80090346,
    SEC_E_MULTIPLE_ACCOUNTS    = 0x80090347,
    SEC_E_NO_KERB_KEY          = 0x80090348,
    SEC_E_CERT_WRONG_USAGE     = 0x80090349,
    SEC_E_DOWNGRADE_DETECTED   = 0x80090350,
    SEC_E_SMARTCARD_CERT_REVOKED = 0x80090351,
    SEC_E_ISSUING_CA_UNTRUSTED = 0x80090352,
    SEC_E_REVOCATION_OFFLINE_C = 0x80090353,
    SEC_E_PKINIT_CLIENT_FAILURE = 0x80090354,
    SEC_E_SMARTCARD_CERT_EXPIRED = 0x80090355,
    SEC_E_NO_S4U_PROT_SUPPORT  = 0x80090356,
    SEC_E_CROSSREALM_DELEGATION_FAILURE = 0x80090357,
    SEC_E_REVOCATION_OFFLINE_KDC = 0x80090358,
    SEC_E_ISSUING_CA_UNTRUSTED_KDC = 0x80090359,
    SEC_E_KDC_CERT_EXPIRED     = 0x8009035A,
    SEC_E_KDC_CERT_REVOKED     = 0x8009035B,
    SEC_E_INVALID_PARAMETER    = 0x8009035D,
    SEC_E_DELEGATION_POLICY    = 0x8009035E,
    SEC_E_POLICY_NLTM_ONLY     = 0x8009035F,
    SEC_E_NO_CONTEXT           = 0x80090361,
    SEC_E_PKU2U_CERT_FAILURE   = 0x80090362,
    SEC_E_MUTUAL_AUTH_FAILED   = 0x80090363,

    SEC_I_CONTINUE_NEEDED      = 0x00090312,
    SEC_I_COMPLETE_NEEDED      = 0x00090313,
    SEC_I_COMPLETE_AND_CONTINUE = 0x00090314,
    SEC_I_LOCAL_LOGON          = 0x00090315,
    SEC_I_CONTEXT_EXPIRED      = 0x00090317,
    SEC_I_INCOMPLETE_CREDENTIALS = 0x00090320,
    SEC_I_RENEGOTIATE          = 0x00090321,
    SEC_I_NO_LSA_CONTEXT       = 0x00090323,
    SEC_I_SIGNATURE_NEEDED     = 0x0009035C,
    SEC_I_NO_RENEGOTIATION     = 0x00090360
};

/* InitializeSecurityContext Flags */
enum ISC_REQ {
    ISC_REQ_DELEGATE = 0x00000001,
    ISC_REQ_MUTUAL_AUTH = 0x00000002,
    ISC_REQ_REPLAY_DETECT = 0x00000004,
    ISC_REQ_SEQUENCE_DETECT = 0x00000008,
    ISC_REQ_CONFIDENTIALITY = 0x00000010,
    ISC_REQ_USE_SESSION_KEY = 0x00000020,
    ISC_REQ_PROMPT_FOR_CREDS = 0x00000040,
    ISC_REQ_USE_SUPPLIED_CREDS = 0x00000080,
    ISC_REQ_ALLOCATE_MEMORY = 0x00000100,
    ISC_REQ_USE_DCE_STYLE = 0x00000200,
    ISC_REQ_DATAGRAM = 0x00000400,
    ISC_REQ_CONNECTION = 0x00000800,
    ISC_REQ_CALL_LEVEL = 0x00001000,
    ISC_REQ_FRAGMENT_SUPPLIED = 0x00002000,
    ISC_REQ_EXTENDED_ERROR = 0x00004000,
    ISC_REQ_STREAM = 0x00008000,
    ISC_REQ_INTEGRITY = 0x00010000,
    ISC_REQ_IDENTIFY = 0x00020000,
    ISC_REQ_NULL_SESSION = 0x00040000,
    ISC_REQ_MANUAL_CRED_VALIDATION = 0x00080000,
    ISC_REQ_RESERVED1 = 0x00100000,
    ISC_REQ_FRAGMENT_TO_FIT = 0x00200000,
    ISC_REQ_FORWARD_CREDENTIALS = 0x00400000,
    ISC_REQ_NO_INTEGRITY = 0x00800000,
    ISC_REQ_USE_HTTP_STYLE = 0x01000000
};

/* AcceptSecurityContext Flags */
enum ASC_REQ {
    ASC_REQ_DELEGATE = 0x00000001,
    ASC_REQ_MUTUAL_AUTH = 0x00000002,
    ASC_REQ_REPLAY_DETECT = 0x00000004,
    ASC_REQ_SEQUENCE_DETECT = 0x00000008,
    ASC_REQ_CONFIDENTIALITY = 0x00000010,
    ASC_REQ_USE_SESSION_KEY = 0x00000020,
    ASC_REQ_ALLOCATE_MEMORY = 0x00000100,
    ASC_REQ_USE_DCE_STYLE = 0x00000200,
    ASC_REQ_DATAGRAM = 0x00000400,
    ASC_REQ_CONNECTION = 0x00000800,
    ASC_REQ_CALL_LEVEL = 0x00001000,
    ASC_REQ_EXTENDED_ERROR = 0x00008000,
    ASC_REQ_STREAM = 0x00010000,
    ASC_REQ_INTEGRITY = 0x00020000,
    ASC_REQ_LICENSING = 0x00040000,
    ASC_REQ_IDENTIFY = 0x00080000,
    ASC_REQ_ALLOW_NULL_SESSION = 0x00100000,
    ASC_REQ_ALLOW_NON_USER_LOGONS = 0x00200000,
    ASC_REQ_ALLOW_CONTEXT_REPLAY = 0x00400000,
    ASC_REQ_FRAGMENT_TO_FIT = 0x00800000,
    ASC_REQ_FRAGMENT_SUPPLIED = 0x00002000,
    ASC_REQ_NO_TOKEN = 0x01000000,
    ASC_REQ_PROXY_BINDINGS = 0x04000000,
    ASC_REQ_ALLOW_MISSING_BINDINGS = 0x10000000
};

enum ASC_RET {
    ASC_RET_DELEGATE = 0x00000001,
    ASC_RET_MUTUAL_AUTH = 0x00000002,
    ASC_RET_REPLAY_DETECT = 0x00000004,
    ASC_RET_SEQUENCE_DETECT = 0x00000008,
    ASC_RET_CONFIDENTIALITY = 0x00000010,
    ASC_RET_USE_SESSION_KEY = 0x00000020,
    ASC_RET_ALLOCATED_MEMORY = 0x00000100,
    ASC_RET_USED_DCE_STYLE = 0x00000200,
    ASC_RET_DATAGRAM = 0x00000400,
    ASC_RET_CONNECTION = 0x00000800,
    ASC_RET_CALL_LEVEL = 0x00002000,
    ASC_RET_THIRD_LEG_FAILED = 0x00004000,
    ASC_RET_EXTENDED_ERROR = 0x00008000,
    ASC_RET_STREAM = 0x00010000,
    ASC_RET_INTEGRITY = 0x00020000,
    ASC_RET_LICENSING = 0x00040000,
    ASC_RET_IDENTIFY = 0x00080000,
    ASC_RET_NULL_SESSION = 0x00100000,
    ASC_RET_ALLOW_NON_USER_LOGONS = 0x00200000,
    ASC_RET_FRAGMENT_ONLY = 0x00800000,
    ASC_RET_NO_TOKEN = 0x01000000,
    ASC_RET_NO_PROXY_BINDINGS = 0x04000000,
    ASC_RET_MISSING_BINDINGS = 0x10000000
};

enum CredentialUse {
    SECPKG_CRED_INBOUND = 0x00000001,
    SECPKG_CRED_OUTBOUND = 0x00000002,
    SECPKG_CRED_BOTH = 0x00000003,
    SECPKG_CRED_AUTOLOGON_RESTRICTED = 0x00000010,
    SECPKG_CRED_PROCESS_POLICY_ONLY = 0x00000020
};


/**
 * TSRequest ::= SEQUENCE {
 *     version    [0] INTEGER,
 *     negoTokens [1] NegoData OPTIONAL,
 *     authInfo   [2] OCTET STRING OPTIONAL,
 *     pubKeyAuth [3] OCTET STRING OPTIONAL,
 *     errorCode  [4] INTEGER OPTIONAL,
 *     clientNonce[5] OCTET STRING OPTIONAL
 * }
 *
 * NegoData ::= SEQUENCE OF NegoDataItem
 *
 * NegoDataItem ::= SEQUENCE {
 *     negoToken [0] OCTET STRING
 * }
 *
 * TSCredentials ::= SEQUENCE {
 *     credType    [0] INTEGER,
 *     credentials [1] OCTET STRING
 * }
 *
 * TSPasswordCreds ::= SEQUENCE {
 *     domainName  [0] OCTET STRING,
 *     userName    [1] OCTET STRING,
 *     password    [2] OCTET STRING
 * }
 *
 * TSSmartCardCreds ::= SEQUENCE {
 *     pin        [0] OCTET STRING,
 *     cspData    [1] TSCspDataDetail,
 *     userHint   [2] OCTET STRING OPTIONAL,
 *     domainHint [3] OCTET STRING OPTIONAL
 * }
 *
 * TSCspDataDetail ::= SEQUENCE {
 *     keySpec       [0] INTEGER,
 *     cardName      [1] OCTET STRING OPTIONAL,
 *     readerName    [2] OCTET STRING OPTIONAL,
 *     containerName [3] OCTET STRING OPTIONAL,
 *     cspName       [4] OCTET STRING OPTIONAL
 * }
 *
 */


struct ClientNonce {
    static const int CLIENT_NONCE_LENGTH = 32;
    bool initialized = false;
    uint8_t data[CLIENT_NONCE_LENGTH] = {};

    static inline int sizeof_client_nonce(int length) {
        length = BER::sizeof_octet_string(length);
        length += BER::sizeof_contextual_tag(length);
        return length;
    }

    ClientNonce() {}

    bool isset() {
        return this->initialized;
    }

    void reset()
    {
        this->initialized = false;
    }

    int ber_length(int use_version)
    {
        return (use_version >= 5 && this->initialized)
            ? sizeof_client_nonce(CLIENT_NONCE_LENGTH)
            : 0;
    }

    void ber_write(int version, OutStream & stream)
    {
        if (version >= 5 && this->initialized) {
            // LOG(LOG_INFO, "Credssp: TSCredentials::emit() clientNonce");
            BER::write_sequence_octet_string(stream, 5, this->data, CLIENT_NONCE_LENGTH);
        }
    }

    int ber_read(int version, int & length, InStream & stream)
    {
        if (version >= 5 && BER::read_contextual_tag(stream, 5, length, true)) {
            // LOG(LOG_INFO, "Credssp TSCredentials::recv() CLIENTNONCE");
            if(!BER::read_octet_string_tag(stream, length)){
                return -1;
            }
            this->initialized = true;
            if (length != CLIENT_NONCE_LENGTH){
                LOG(LOG_ERR, "Truncated client nonce");
                return -1;
            }
            stream.in_copy_bytes(this->data, CLIENT_NONCE_LENGTH);
            this->initialized = true;
        }
        return 0;
    }
};

// 2.2.1 TSRequest
// ===============

// The TSRequest structure is the top-most structure used by the CredSSP client and 
// CredSSP server. It contains the SPNEGO tokens and MAY<6> contain Kerberos/NTLM 
// messages that are passed between the client and server, and either the public 
// key authentication messages that are used to bind to the TLS session or the 
// client credentials that are delegated to the server. The TSRequest message is 
// always sent over the TLS-encrypted channel between the client and server in a 
// CredSSP Protocol exchange (see step 1 in section 3.1.5).

//     TSRequest ::= SEQUENCE {
//             version    [0] INTEGER,
//             negoTokens [1] NegoData  OPTIONAL,
//             authInfo   [2] OCTET STRING OPTIONAL,
//             pubKeyAuth [3] OCTET STRING OPTIONAL,
//             errorCode  [4] INTEGER OPTIONAL,
//             clientNonce [5] OCTET STRING OPTIONAL
//     }

// version: This field specifies the supported version of the CredSSP Protocol. 
// Valid values for this field are 2, 3, 4, 5, and 6.<7> If the version received is 
// greater than the implementation understands, treat the peer as one that is 
// compatible with the version of the CredSSP Protocol that the implementation 
// understands.

// negoTokens: A NegoData structure, as defined in section 2.2.1.1, that contains 
// the SPNEGO tokens or Kerberos/NTLM messages that are passed between the client 
// and server.

// * NegoData ::= SEQUENCE OF NegoDataItem
// *
// * NegoDataItem ::= SEQUENCE {
// *     negoToken [0] OCTET STRING
// * }

// authInfo: A TSCredentials structure, as defined in section 2.2.1.2, that 
// contains the user's credentials that are delegated to the server. The authInfo 
// field MUST be encrypted under the encryption key that is negotiated under the 
// SPNEGO package. The authInfo field carries the message signature and then the 
// encrypted data.

// * TSCredentials ::= SEQUENCE {
// *     credType    [0] INTEGER,
// *     credentials [1] OCTET STRING
// * }


// pubKeyAuth: This field is used to assure that the public key that is used by the 
// server during the TLS handshake belongs to the target server and not to a 
// man-in-the-middle. This TLS session-binding is specified in section 3.1.5. After 
// the client completes the SPNEGO phase of the CredSSP Protocol, it uses 
// GSS_WrapEx() for the negotiated protocol to encrypt the server's public key. 
// With version 4 or lower, the pubKeyAuth field carries the message signature and 
// then the encrypted public key to the server. In response, the server uses the 
// pubKeyAuth field to transmit to the client a modified version of the public key 
// (as specified in section 3.1.5) that is encrypted under the encryption key that 
// is negotiated under SPNEGO. In version 5 or higher, this field stores a computed 
// hash of the public key.<8>

// errorCode: If the negotiated protocol version is 3, 4, or 6, and the SPNEGO 
// exchange fails on the server, this field SHOULD<9> be used to send the NTSTATUS 
// failure code ([MS-ERREF] section 2.3) to the client so that it knows what failed 
// and be able to display a descriptive error to the user.

// clientNonce: A 32-byte array of cryptographically random bytes used to provide 
// sufficient entropy during hash computation. This value is only used in version 5 
// or higher of this protocol.




struct TSRequest final {
    /* TSRequest */

    /* [0] version */
    uint32_t version;
    uint32_t use_version;

    /* [1] negoTokens (NegoData) */
    std::vector<uint8_t> negoTokens;
    // BStream negoTokens;
    /* [2] authInfo (OCTET STRING) */
    std::vector<uint8_t> authInfo;
    // BStream authInfo;
    /* [3] pubKeyAuth (OCTET STRING) */
    std::vector<uint8_t> pubKeyAuth;
    // BStream pubKeyAuth;
    /* [4] errorCode (INTEGER OPTIONAL) */
//    uint32_t error_code{0};
    /* [5] clientNonce (OCTET STRING OPTIONAL) */
    ClientNonce clientNonce;

    TSRequest(uint32_t version = 6): version(version), use_version(this->version)
//    , error_code(0)
    {
    }
};


inline void emitTSRequest(OutStream & stream, TSRequest & self, uint32_t error_code)
{
    auto * begin = stream.get_current();

    if ((self.version == 3 || self.version == 4 || self.version >= 6)
    && error_code != 0) {
        /* [0] version */
        auto ber_version_field = BER::mkSmallIntegerField(self.version, 0);
        /* [4] errorCode (INTEGER) */
        auto ber_error_code_field = BER::mkIntegerField(error_code, 4);

        /* TSRequest */
        size_t ts_request_length = ber_version_field.size()+ber_error_code_field.size();
        auto ber_ts_request_header = BER::mkTSRequestSequenceHeader(ts_request_length);

        /* TSRequest */
        stream.out_copy_bytes(ber_ts_request_header);
        // version    [0] INTEGER,
        stream.out_copy_bytes(ber_version_field);
        // errorCode  [4] INTEGER OPTIONAL
        stream.out_copy_bytes(ber_error_code_field);

        auto * end = stream.get_current();

        LOG(LOG_INFO, "TSRequest hexdump ---------------------------------");
        LOG(LOG_INFO, "TSRequest ts_request_header -----------------------");
        hexdump_c(ber_ts_request_header);
        LOG(LOG_INFO, "TSRequest version_field ---------------------------");
        hexdump_c(ber_version_field);
        LOG(LOG_INFO, "TSRequest error_code field ------------------------");
        hexdump_c(ber_error_code_field);
        LOG(LOG_INFO, "TSRequest full dump--------------------------------");
        hexdump_c({begin, size_t(end-begin)});
        LOG(LOG_INFO, "TSRequest hexdump -DONE----------------------------");

        return;
    }
    
    // version    [0] INTEGER,
    auto ber_version_field = BER::mkSmallIntegerField(self.version, 0);
    // negoTokens [1] NegoData OPTIONAL
    auto ber_nego_tokens_header = BER::mkOptionalNegoTokensHeader(self.negoTokens.size());
    // authInfo   [2] OCTET STRING OPTIONAL
    auto ber_auth_info_header = BER::mkOptionalOctetStringFieldHeader(self.authInfo.size(), 2);
    // pubKeyAuth [3] OCTET STRING OPTIONAL
    auto ber_pub_key_auth_header = BER::mkOptionalOctetStringFieldHeader(self.pubKeyAuth.size(), 3);
    // clientNonce[5] OCTET STRING OPTIONAL
    auto ber_nonce_header = BER::mkOptionalOctetStringFieldHeader(sizeof(self.clientNonce.data), 5);

    /* TSRequest */
    size_t ts_request_length = ber_version_field.size()
          + ber_nego_tokens_header.size()
          + self.negoTokens.size()
          + ber_auth_info_header.size()
          + self.authInfo.size()
          + ber_pub_key_auth_header.size()
          + self.pubKeyAuth.size()
          + (self.version >= 5 && self.clientNonce.initialized)
            *(ber_nonce_header.size()+sizeof(self.clientNonce.data));

    auto ber_ts_request_header = BER::mkTSRequestSequenceHeader(ts_request_length);

    /* TSRequest */
    stream.out_copy_bytes(ber_ts_request_header);
    // version    [0] INTEGER,
    stream.out_copy_bytes(ber_version_field);
    // negoTokens [1] NegoData OPTIONAL
    stream.out_copy_bytes(ber_nego_tokens_header);
    stream.out_copy_bytes(self.negoTokens);
    // authInfo   [2] OCTET STRING OPTIONAL
    stream.out_copy_bytes(ber_auth_info_header);
    stream.out_copy_bytes(self.authInfo);
    // pubKeyAuth [3] OCTET STRING OPTIONAL
    stream.out_copy_bytes(ber_pub_key_auth_header);
    stream.out_copy_bytes(self.pubKeyAuth);
    // errorCode  [4] INTEGER OPTIONAL
    // clientNonce[5] OCTET STRING OPTIONAL
    if (self.version >= 5 && self.clientNonce.initialized){
        stream.out_copy_bytes(ber_nonce_header);
        stream.out_copy_bytes({self.clientNonce.data, sizeof(self.clientNonce.data)});
    }
    auto * end = stream.get_current();
    
    LOG(LOG_INFO, "TSRequest hexdump ---------------------------------");
    LOG(LOG_INFO, "TSRequest ts_request_header -----------------------");
    hexdump_c(ber_ts_request_header);
    LOG(LOG_INFO, "TSRequest version_field ---------------------------");
    hexdump_c(ber_version_field);
    LOG(LOG_INFO, "TSRequest nego_tokens_header ----------------------");
    hexdump_c(ber_nego_tokens_header);
    LOG(LOG_INFO, "TSRequest auth_info_header ------------------------");
    hexdump_c(ber_auth_info_header);
    LOG(LOG_INFO, "TSRequest pub_key_auth_header ---------------------");
    hexdump_c(ber_pub_key_auth_header);
    LOG(LOG_INFO, "TSRequest nonce -----------------------------------");
    if (self.version >= 5 && self.clientNonce.initialized){
        hexdump_c(ber_nonce_header);
    }
    LOG(LOG_INFO, "TSRequest full dump--------------------------------");
    hexdump_c({begin, size_t(end-begin)});
    LOG(LOG_INFO, "TSRequest hexdump -DONE----------------------------");

}

// TODO: use exceptions instead of error_code for returning errors
inline TSRequest recvTSRequest(InStream & stream, uint32_t & error_code, uint32_t version = 6) 
{
    TSRequest self(version);
    int length;
    uint32_t remote_version;

    /* TSRequest */
    if(!BER::read_sequence_tag(stream, length) ||
       !BER::read_contextual_tag(stream, 0, length, true) ||
       !BER::read_integer(stream, remote_version)) {
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }
    LOG(LOG_INFO, "Credssp TSCredentials::recv() Remote Version %u", remote_version);

    if (remote_version < self.use_version) {
        self.use_version = remote_version;
    }
    LOG(LOG_INFO, "Credssp TSCredentials::recv() Negotiated version %u", self.use_version);

    /* [1] negoTokens (NegoData) */
    if (BER::read_contextual_tag(stream, 1, length, true))        {
        // LOG(LOG_INFO, "Credssp TSCredentials::recv() NEGOTOKENS");

        if (!BER::read_sequence_tag(stream, length) || /* SEQUENCE OF NegoDataItem */ /*NOLINT(misc-redundant-expression)*/
            !BER::read_sequence_tag(stream, length) || /* NegoDataItem */
            !BER::read_contextual_tag(stream, 0, length, true) || /* [0] negoToken */
            !BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
            !stream.in_check_rem(length)) {
            throw Error(ERR_CREDSSP_TS_REQUEST);
        }

        self.negoTokens = std::vector<uint8_t>(length);
        stream.in_copy_bytes(self.negoTokens.data(), length);
    }

    /* [2] authInfo (OCTET STRING) */
    if (BER::read_contextual_tag(stream, 2, length, true)) {
        // LOG(LOG_INFO, "Credssp TSCredentials::recv() AUTHINFO");
        if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
           !stream.in_check_rem(length)) {
            throw Error(ERR_CREDSSP_TS_REQUEST);
        }

        self.authInfo = std::vector<uint8_t>(length);
        stream.in_copy_bytes(self.authInfo.data(), length);
    }

    /* [3] pubKeyAuth (OCTET STRING) */
    if (BER::read_contextual_tag(stream, 3, length, true)) {
        // LOG(LOG_INFO, "Credssp TSCredentials::recv() PUBKEYAUTH");
        if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
           !stream.in_check_rem(length)) {
            throw Error(ERR_CREDSSP_TS_REQUEST);
        }
        self.pubKeyAuth = std::vector<uint8_t>(length);
        stream.in_copy_bytes(self.pubKeyAuth.data(), length);
    }
    /* [4] errorCode (INTEGER) */
    if (remote_version >= 3
        && remote_version != 5
        && BER::read_contextual_tag(stream, 4, length, true)) {
        LOG(LOG_INFO, "Credssp TSCredentials::recv() ErrorCode");
        if (!BER::read_integer(stream, error_code)) {
            throw Error(ERR_CREDSSP_TS_REQUEST);
        }
        LOG(LOG_INFO, "Credssp TSCredentials::recv() "
            "ErrorCode = %x, Facility = %x, Code = %x",
            error_code,
            (error_code >> 16) & 0x7FF,
            (error_code & 0xFFFF)
        );
    }
    /* [5] clientNonce (OCTET STRING) */
    if (self.clientNonce.ber_read(remote_version, length, stream) == -1){
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }
    // return 0;
    return self;
}


/*
 * TSPasswordCreds ::= SEQUENCE {
 *     domainName  [0] OCTET STRING,
 *     userName    [1] OCTET STRING,
 *     password    [2] OCTET STRING
 * }
 */
struct TSPasswordCreds {
    uint8_t domainName[256];
    size_t domainName_length{0};
    uint8_t userName[256];
    size_t userName_length{0};
    uint8_t password[256];
    size_t password_length{0};

    TSPasswordCreds() = default;

    TSPasswordCreds(const uint8_t * domain, size_t domain_length, const uint8_t * user, size_t user_length, const uint8_t * pass, size_t pass_length) {
        this->domainName_length = (domain_length < sizeof(this->domainName))
            ? domain_length
            : sizeof(this->domainName);
        memcpy(this->domainName, domain, this->domainName_length);

        this->userName_length = (user_length < sizeof(this->userName))
            ? user_length
            : sizeof(this->userName);
        memcpy(this->userName, user, this->userName_length);

        this->password_length = (pass_length < sizeof(this->password))
            ? pass_length
            : sizeof(this->password);
        memcpy(this->password, pass, this->password_length);
    }

    // TSPasswordCreds(InStream & stream) {
    //     this->recv(stream);
    // }


    int ber_sizeof() const {
        int length = 0;
        // TO COMPLETE
        length += BER::sizeof_sequence_octet_string(domainName_length);
        length += BER::sizeof_sequence_octet_string(userName_length);
        length += BER::sizeof_sequence_octet_string(password_length);
        return length;
    }
    
    int emit(OutStream & stream) const {
        int size = 0;
        int innerSize = this->ber_sizeof();

        // /* TSPasswordCreds (SEQUENCE) */
        auto * begin = stream.get_current();
        size += BER::write_sequence_tag(stream, innerSize);
        auto * end = stream.get_current();
        LOG(LOG_INFO, "TSPasswordCreds(old) ts_password_creds_header ----------");
        hexdump_c({begin,size_t(end-begin)});
        

        // /* [0] domainName (OCTET STRING) */
        {
        auto * begin = stream.get_current();
        size += BER::write_sequence_octet_string(stream, 0, this->domainName,
                                                 this->domainName_length);
        auto * end = stream.get_current();
        LOG(LOG_INFO, "TSPasswordCreds(old) domain name header ----------------------");
        hexdump_c({begin,size_t(end-begin)});
        }

        // /* [1] userName (OCTET STRING) */
        {
        auto * begin = stream.get_current();
        size += BER::write_sequence_octet_string(stream, 1, this->userName,
                                                 this->userName_length);
        auto * end = stream.get_current();
        LOG(LOG_INFO, "TSPasswordCreds(old) user name header ------------------------");
        hexdump_c({begin,size_t(end-begin)});
        }

        // /* [2] password (OCTET STRING) */
        {
        auto * begin = stream.get_current();
        size += BER::write_sequence_octet_string(stream, 2, this->password,
                                                 this->password_length);
        auto * end = stream.get_current();
        LOG(LOG_INFO, "TSPasswordCreds(old) password header -------------------------");
        hexdump_c({begin,size_t(end-begin)});
        }

        end = stream.get_current();
        LOG(LOG_INFO, "TSPasswordCreds(old) full dump -------------------------");
        hexdump_c({begin,size_t(end-begin)});
        LOG(LOG_INFO, "TSPasswordCreds(old) DONE ------------------------------");
        return size;
    }

    void recv(InStream & stream) {
        int length = 0;
        /* TSPasswordCreds (SEQUENCE) */
        BER::read_sequence_tag(stream, length);

        /* [0] domainName (OCTET STRING) */
        BER::read_contextual_tag(stream, 0, length, true);
        BER::read_octet_string_tag(stream, length);

        this->domainName_length = length;
        stream.in_copy_bytes(this->domainName, length);

        /* [1] userName (OCTET STRING) */
        BER::read_contextual_tag(stream, 1, length, true);
        BER::read_octet_string_tag(stream, length);

        this->userName_length = length;
        stream.in_copy_bytes(this->userName, length);

        /* [2] password (OCTET STRING) */
        BER::read_contextual_tag(stream, 2, length, true);
        BER::read_octet_string_tag(stream, length);

        this->password_length = length;
        stream.in_copy_bytes(this->password, length);

    }
};


// emitTSPasswordCreds

inline size_t emitTSPasswordCreds(OutStream & stream, const TSPasswordCreds & self)
{
    auto * begin = stream.get_current();

    // [0] domainName (OCTET STRING)
    auto ber_domain_name_header = BER::mkMandatoryOctetStringFieldHeader(self.domainName_length, 0);
    // [1] userName (OCTET STRING)
    auto ber_user_name_header = BER::mkMandatoryOctetStringFieldHeader(self.userName_length, 1);
    // [2] password (OCTET STRING)
    auto ber_password_header = BER::mkMandatoryOctetStringFieldHeader(self.password_length, 2);

    // TSPasswordCreds (SEQUENCE)
    size_t ts_password_creds_length = ber_domain_name_header.size()+self.domainName_length
                             + ber_user_name_header.size()+self.userName_length
                             + ber_password_header.size()+self.password_length;

    auto ber_ts_password_creds_header = BER::mkTSRequestSequenceHeader(ts_password_creds_length);

    // TSPasswordCreds (SEQUENCE)
    stream.out_copy_bytes(ber_ts_password_creds_header);
    // [0] domainName (OCTET STRING)
    stream.out_copy_bytes(ber_domain_name_header);
    stream.out_copy_bytes(self.domainName, self.domainName_length);
    // [1] userName (OCTET STRING)
    stream.out_copy_bytes(ber_user_name_header);
    stream.out_copy_bytes(self.userName, self.userName_length);
    // [2] password (OCTET STRING)
    stream.out_copy_bytes(ber_password_header);
    stream.out_copy_bytes(self.password, self.password_length);

    auto * end = stream.get_current();
    
    LOG(LOG_INFO, "TSPasswordCreds hexdump ---------------------------");
    LOG(LOG_INFO, "TSPasswordCreds ts_password_creds_header ----------");
    hexdump_c(ber_ts_password_creds_header);
    LOG(LOG_INFO, "TSPasswordCreds domain name header ----------------------");
    hexdump_c(ber_domain_name_header);
    hexdump_c({self.domainName, self.domainName_length});
    LOG(LOG_INFO, "TSPasswordCreds user name header ------------------------");
    hexdump_c(ber_user_name_header);
    hexdump_c({self.userName, self.userName_length});
    LOG(LOG_INFO, "TSPasswordCreds password header -------------------------");
    hexdump_c(ber_password_header);
    hexdump_c({self.password, self.password_length});
    LOG(LOG_INFO, "TSPasswordCreds full dump--------------------------------");
    hexdump_c({begin, size_t(end-begin)});
    LOG(LOG_INFO, "TSPasswordCreds hexdump -DONE----------------------------");
    
    return size_t(end-begin);
}



namespace CredSSP {


    inline int sizeof_octet_string_seq(int length) {
        length = BER::sizeof_octet_string(length);
        length += BER::sizeof_contextual_tag(length);
        return length;
    }
}  // namespace CredSSP


/* TSCspDataDetail ::= SEQUENCE {
 *     keySpec       [0] INTEGER,
 *     cardName      [1] OCTET STRING OPTIONAL,
 *     readerName    [2] OCTET STRING OPTIONAL,
 *     containerName [3] OCTET STRING OPTIONAL,
 *     cspName       [4] OCTET STRING OPTIONAL
 * }
 *
 */
struct TSCspDataDetail {
    uint32_t keySpec{0};
    uint8_t cardName[256]{};
    size_t cardName_length{0};
    uint8_t readerName[256]{};
    size_t readerName_length{0};
    uint8_t containerName[256]{};
    size_t containerName_length{0};
    uint8_t cspName[256]{};
    size_t cspName_length{0};

    TSCspDataDetail()

    = default;

    TSCspDataDetail(uint32_t keySpec, uint8_t * cardName, size_t cardName_length,
                    uint8_t * readerName, size_t readerName_length,
                    uint8_t * containerName, size_t containerName_length,
                    uint8_t * cspName, size_t cspName_length)
        : keySpec(keySpec)
        ,
         cardName_length(cardName_length)
        ,
         readerName_length(readerName_length)
        ,
         containerName_length(containerName_length)
        ,
         cspName_length(cspName_length)
    {
        this->cardName_length = (cardName_length < sizeof(this->cardName))
            ? cardName_length
            : sizeof(this->cardName);
        memcpy(this->cardName, cardName, this->cardName_length);

        this->readerName_length = (readerName_length < sizeof(this->readerName))
            ? readerName_length
            : sizeof(this->readerName);
        memcpy(this->readerName, readerName, this->readerName_length);

        this->containerName_length = (containerName_length < sizeof(this->containerName))
            ? containerName_length
            : sizeof(this->containerName);
        memcpy(this->containerName, containerName, this->containerName_length);

        this->cspName_length = (cspName_length < sizeof(this->cspName))
            ? cspName_length
            : sizeof(this->cspName);
        memcpy(this->cspName, cspName, this->cspName_length);

    }

    int ber_sizeof() const {
        int length = 0;
        length += BER::sizeof_contextual_tag(BER::sizeof_integer(this->keySpec));
        length += BER::sizeof_integer(this->keySpec);
        length += (this->cardName_length > 0) ?
            CredSSP::sizeof_octet_string_seq(this->cardName_length) : 0;
        length += (this->readerName_length > 0) ?
            CredSSP::sizeof_octet_string_seq(this->readerName_length) : 0;
        length += (this->containerName_length > 0) ?
            CredSSP::sizeof_octet_string_seq(this->containerName_length) : 0;
        length += (this->cspName_length > 0) ?
            CredSSP::sizeof_octet_string_seq(this->cspName_length) : 0;
        return length;
    }

    int emit(OutStream & stream) const {
        int length = 0;
        int size = 0;
        int innerSize = this->ber_sizeof();

        // /* TSCspDataDetail (SEQUENCE) */

        size += BER::write_sequence_tag(stream, innerSize);

        /* [0] keySpec */
        size += BER::write_contextual_tag(stream, 0, BER::sizeof_integer(this->keySpec), true);
        size += BER::write_integer(stream, this->keySpec);

        /* [1] cardName (OCTET STRING OPTIONAL) */
        if (this->cardName_length > 0) {
            // LOG(LOG_INFO, "Credssp: TSCspDataDetail::emit() cardName");
            length = CredSSP::sizeof_octet_string_seq(this->cardName_length);
            size += length;
            length -= BER::write_sequence_octet_string(stream, 1,
                                                       this->cardName,
                                                       this->cardName_length);
            assert(length == 0);
            (void)length;
        }
        /* [2] readerName (OCTET STRING OPTIONAL) */
        if (this->readerName_length > 0) {
            // LOG(LOG_INFO, "Credssp: TSCspDataDetail::emit() readerName");
            length = CredSSP::sizeof_octet_string_seq(this->readerName_length);
            size += length;
            length -= BER::write_sequence_octet_string(stream, 2,
                                                       this->readerName,
                                                       this->readerName_length);
            assert(length == 0);
            (void)length;
        }
        /* [3] containerName (OCTET STRING OPTIONAL) */
        if (this->containerName_length > 0) {
            // LOG(LOG_INFO, "Credssp: TSCspDataDetail::emit() containerName");
            length = CredSSP::sizeof_octet_string_seq(this->containerName_length);
            size += length;
            length -= BER::write_sequence_octet_string(stream, 3,
                                                       this->containerName,
                                                       this->containerName_length);
            assert(length == 0);
            (void)length;
        }
        /* [4] cspName (OCTET STRING OPTIONAL) */
        if (this->cspName_length > 0) {
            // LOG(LOG_INFO, "Credssp: TSCspDataDetail::emit() cspName");
            length = CredSSP::sizeof_octet_string_seq(this->cspName_length);
            size += length;
            length -= BER::write_sequence_octet_string(stream, 4,
                                                       this->cspName,
                                                       this->cspName_length);
            assert(length == 0);
            (void)length;
        }
        return size;
    }

    int recv(InStream & stream) {
        int length = 0;
        /* TSCspDataDetail ::= SEQUENCE */
        /* TSSmartCardCreds (SEQUENCE) */
        BER::read_sequence_tag(stream, length);


        /* [0] keySpec (INTEGER) */
        BER::read_contextual_tag(stream, 0, length, true);
        BER::read_integer(stream, this->keySpec);

        /* [1] cardName (OCTET STRING OPTIONAL) */
        if (BER::read_contextual_tag(stream, 1, length, true)) {
            // LOG(LOG_INFO, "Credssp TSCspDataDetail::recv() : cardName");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->cardName_length = length;
            stream.in_copy_bytes(this->cardName, length);
        }
        /* [2] readerName (OCTET STRING OPTIONAL) */
        if (BER::read_contextual_tag(stream, 2, length, true)) {
            // LOG(LOG_INFO, "Credssp TSCspDataDetail::recv() : readerName");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->readerName_length = length;
            stream.in_copy_bytes(this->readerName, length);
        }
        /* [3] containerName (OCTET STRING OPTIONAL) */
        if (BER::read_contextual_tag(stream, 3, length, true)) {
            // LOG(LOG_INFO, "Credssp TSCspDataDetail::recv() : containerName");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->containerName_length = length;
            stream.in_copy_bytes(this->containerName, length);
        }
        /* [4] cspName (OCTET STRING OPTIONAL) */
        if (BER::read_contextual_tag(stream, 4, length, true)) {
            // LOG(LOG_INFO, "Credssp TSCspDataDetail::recv() : cspName");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->cspName_length = length;
            stream.in_copy_bytes(this->cspName, length);
        }
        return 0;

    }
};
/*
 * TSSmartCardCreds ::= SEQUENCE {
 *     pin        [0] OCTET STRING,
 *     cspData    [1] TSCspDataDetail,
 *     userHint   [2] OCTET STRING OPTIONAL,
 *     domainHint [3] OCTET STRING OPTIONAL
 * }
 */

struct TSSmartCardCreds {
    uint8_t pin[256]{};
    size_t pin_length{0};
    TSCspDataDetail cspData;
    uint8_t userHint[256]{};
    size_t userHint_length{0};
    uint8_t domainHint[256]{};
    size_t domainHint_length{0};

    TSSmartCardCreds() = default;

    TSSmartCardCreds(uint8_t * pin, size_t pin_length,
                     uint8_t * userHint, size_t userHint_length,
                     uint8_t * domainHint, size_t domainHint_length)
    {
        this->pin_length = (pin_length < sizeof(this->pin))
            ? pin_length
            : sizeof(this->pin);
        memcpy(this->pin, pin, this->pin_length);

        this->userHint_length = (userHint_length < sizeof(this->userHint))
            ? userHint_length
            : sizeof(this->userHint);
        memcpy(this->userHint, userHint, this->userHint_length);

        this->domainHint_length = (domainHint_length < sizeof(this->domainHint))
            ? domainHint_length
            : sizeof(this->domainHint);
        memcpy(this->domainHint, domainHint, this->domainHint_length);
    }

    void set_cspdatadetail(uint32_t keySpec, uint8_t * cardName, size_t cardName_length,
                           uint8_t * readerName, size_t readerName_length,
                           uint8_t * containerName, size_t containerName_length,
                           uint8_t * cspName, size_t cspName_length) {
        this->cspData = TSCspDataDetail(keySpec, cardName, cardName_length,
                           readerName, readerName_length,
                           containerName, containerName_length,
                           cspName, cspName_length);
    }

    int ber_sizeof() const {
        int length = 0;
        length += CredSSP::sizeof_octet_string_seq(this->pin_length);
        length += BER::sizeof_contextual_tag(BER::sizeof_sequence(this->cspData.ber_sizeof()));
        length += BER::sizeof_sequence(this->cspData.ber_sizeof());
        length += (this->userHint_length > 0) ?
            CredSSP::sizeof_octet_string_seq(this->userHint_length) : 0;
        length += (this->domainHint_length > 0) ?
            CredSSP::sizeof_octet_string_seq(this->domainHint_length) : 0;
        return length;
    }

    int emit(OutStream & stream) const {
        int size = 0;
        int length;
        int innerSize = this->ber_sizeof();
        int cspDataSize = 0;

        /* TSCredentials (SEQUENCE) */
        size += BER::write_sequence_tag(stream, innerSize);

        /* [0] pin (OCTET STRING) */
        size += BER::write_sequence_octet_string(stream, 0, this->pin,
                                         this->pin_length);

        /* [1] cspData (OCTET STRING) */

        cspDataSize = BER::sizeof_sequence(this->cspData.ber_sizeof());
        size += BER::write_contextual_tag(stream, 1, cspDataSize, true);
        size += this->cspData.emit(stream);

        /* [2] userHint (OCTET STRING OPTIONAL) */
        if (this->userHint_length > 0) {
            // LOG(LOG_INFO, "Credssp: TSSmartCard::emit() userHint");
            length = CredSSP::sizeof_octet_string_seq(this->userHint_length);
            size += length;
            length -= BER::write_sequence_octet_string(stream, 2,
                                                       this->userHint,
                                                       this->userHint_length);
            assert(length == 0);
            (void)length;
        }

        /* [3] domainHint (OCTET STRING OPTIONAL) */
        if (this->domainHint_length > 0) {
            // LOG(LOG_INFO, "Credssp: TSSmartCard::emit() domainHint");
            length = CredSSP::sizeof_octet_string_seq(this->domainHint_length);
            size += length;
            length -= BER::write_sequence_octet_string(stream, 3,
                                                       this->domainHint,
                                                       this->domainHint_length);
            assert(length == 0);
            (void)length;
        }

        return size;
    }

    int recv(InStream & stream) {
        int length = 0;
        /* TSSmartCardCreds (SEQUENCE) */
        BER::read_sequence_tag(stream, length);

        /* [0] pin (OCTET STRING) */
        BER::read_contextual_tag(stream, 0, length, true);
        BER::read_octet_string_tag(stream, length);

        this->pin_length = length;
        stream.in_copy_bytes(this->pin, length);

        /* [1] cspData (TSCspDataDetail) */
        BER::read_contextual_tag(stream, 1, length, true);
        this->cspData.recv(stream);

        /* [2] userHint (OCTET STRING) */
        if (BER::read_contextual_tag(stream, 2, length, true)) {
            // LOG(LOG_INFO, "Credssp TSSmartCardCreds::recv() : userHint");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->userHint_length = length;
            stream.in_copy_bytes(this->userHint, length);
        }

        /* [3] domainHint (OCTET STRING) */
        if (BER::read_contextual_tag(stream, 3, length, true)) {
            // LOG(LOG_INFO, "Credssp TSSmartCardCreds::recv() : domainHint");
            if(!BER::read_octet_string_tag(stream, length) || /* OCTET STRING */
               !stream.in_check_rem(length)) {
                return -1;
            }

            this->domainHint_length = length;
            stream.in_copy_bytes(this->domainHint, length);
        }

        return 0;
    }
};




/*
 * TSCredentials ::= SEQUENCE {
 *     credType    [0] INTEGER,
 *     credentials [1] OCTET STRING
 * }
 */
struct TSCredentials
{
    uint32_t credType{1};
    TSPasswordCreds passCreds;
    TSSmartCardCreds smartcardCreds;
    // For now, TSCredentials can only contains TSPasswordCreds (not TSSmartCardCreds)

    TSCredentials() = default;

    TSCredentials(const uint8_t * domain, size_t domain_length, const uint8_t * user, size_t user_length, const uint8_t * pass, size_t pass_length)
        : credType(1)
        , passCreds(domain, domain_length,
                    user, user_length,
                    pass, pass_length)
    {

    }

    TSCredentials(uint8_t * pin, size_t pin_length,
                  uint8_t * userHint, size_t userHint_length,
                  uint8_t * domainHint, size_t domainHint_length,
                  uint32_t keySpec, uint8_t * cardName, size_t cardName_length,
                  uint8_t * readerName, size_t readerName_length,
                  uint8_t * containerName, size_t containerName_length,
                  uint8_t * cspName, size_t cspName_length)
        : credType(2)
        , smartcardCreds(pin, pin_length,
                         userHint, userHint_length,
                         domainHint, domainHint_length)
    {
        this->smartcardCreds.set_cspdatadetail(keySpec, cardName, cardName_length,
                                               readerName, readerName_length,
                                               containerName, containerName_length,
                                               cspName, cspName_length);

    }

    void set_smartcard(uint8_t * pin, size_t pin_length,
                       uint8_t * userHint, size_t userHint_length,
                       uint8_t * domainHint, size_t domainHint_length,
                       uint32_t keySpec, uint8_t * cardName, size_t cardName_length,
                       uint8_t * readerName, size_t readerName_length,
                       uint8_t * containerName, size_t containerName_length,
                       uint8_t * cspName, size_t cspName_length) {
        this->credType = 2;
        this->smartcardCreds = TSSmartCardCreds(pin, pin_length,
                                                userHint, userHint_length,
                                                domainHint, domainHint_length);
        this->smartcardCreds.set_cspdatadetail(keySpec, cardName, cardName_length,
                                               readerName, readerName_length,
                                               containerName, containerName_length,
                                               cspName, cspName_length);
    }

//    void set_credentials(const uint8_t * domain, int domain_length, const uint8_t * user,
//                         int user_length, const uint8_t * pass, int pass_length) {
//        this->passCreds = TSPasswordCreds(domain, domain_length, user, user_length, pass, pass_length);
//    }

    void set_credentials_from_av(cbytes_view domain_av, cbytes_view user_av, cbytes_view password_av) {
        this->passCreds = TSPasswordCreds(domain_av.data(), domain_av.size(),
                                          user_av.data(), user_av.size(),
                                          password_av.data(), password_av.size());
    }

    int ber_sizeof() const {
        int size = 0;
        size += BER::sizeof_integer(this->credType);
        size += BER::sizeof_contextual_tag(BER::sizeof_integer(this->credType));
        if (this->credType == 2) {
            size += BER::sizeof_sequence_octet_string(BER::sizeof_sequence(this->smartcardCreds.ber_sizeof()));
        } else {
            size += BER::sizeof_sequence_octet_string(BER::sizeof_sequence(this->passCreds.ber_sizeof()));
        }
        return size;
    }

    int emit(OutStream & ts_credentials) const {
        // ts_credentials is the authInfo Stream field of TSRequest before it is sent
        // ts_credentials will not be encrypted and should be encrypted after calling emit
        int size = 0;

        int innerSize = this->ber_sizeof();
        int credsSize;

        /* TSCredentials (SEQUENCE) */
        size += BER::write_sequence_tag(ts_credentials, innerSize);

        /* [0] credType (INTEGER) */
        size += BER::write_contextual_tag(ts_credentials, 0, BER::sizeof_integer(this->credType), true);
        size += BER::write_integer(ts_credentials, this->credType);

        /* [1] credentials (OCTET STRING) */

        credsSize = (this->credType == 2) ?
            BER::sizeof_sequence(this->smartcardCreds.ber_sizeof()) :
            BER::sizeof_sequence(this->passCreds.ber_sizeof());

        size += BER::write_contextual_tag(ts_credentials, 1, BER::sizeof_octet_string(credsSize), true);
        size += BER::write_octet_string_tag(ts_credentials, credsSize);

        if (this->credType == 1){
            StaticOutStream<20000> stream;
            auto pass_cred_size = emitTSPasswordCreds(ts_credentials, this->passCreds);
            auto * begin = stream.get_current();
            auto passcred_size = this->passCreds.emit(stream);
            auto * end = stream.get_current();
            LOG(LOG_INFO, "TSPasswordCreds hexdump (old)----------------------");
            LOG(LOG_INFO, "TSRequest full dump--------------------------------");
            hexdump_c({begin, size_t(end-begin)});
            LOG(LOG_INFO, "TSRequest hexdump -DONE----------------------------");

//            exit(0);
            size += pass_cred_size;
        }
        else {
            size += this->smartcardCreds.emit(ts_credentials);
        }

        return size;
    }


    void recv(InStream & ts_credentials) {
        // ts_credentials is decrypted and should be decrypted before calling recv
        int length;
        int creds_length;

        /* TSCredentials (SEQUENCE) */
        BER::read_sequence_tag(ts_credentials, length);

        /* [0] credType (INTEGER) */
        BER::read_contextual_tag(ts_credentials, 0, length, true);
        BER::read_integer(ts_credentials, this->credType);

        /* [1] credentials (OCTET STRING) */
        BER::read_contextual_tag(ts_credentials, 1, length, true);
        BER::read_octet_string_tag(ts_credentials, creds_length);

        if (this->credType == 2) {
            this->smartcardCreds.recv(ts_credentials);
        } else {
            this->passCreds.recv(ts_credentials);
        }
    }
};


// struct TSCspDataDetail : public TSCredentials {
//     int keySpec;
//     uint8_t cardName[256];
//     uint8_t readerName[256];
//     uint8_t containerName[256];
//     uint8_t cspName[256];

// };
// struct TSSmartCardCreds : public TSCredentials {
//     uint8_t pin[256];
//     TSCspDataDetail capData;
//     uint8_t userHint[256];
//     uint8_t domainHint[256];

// };
