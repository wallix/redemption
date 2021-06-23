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
#include "core/RDP/nla/ntlm_message.hpp"

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
    enum {
        CLASS_UNIV            = 0x00,
        CLASS_APPL            = 0x40,
        CLASS_CTXT            = 0x80,
        CLASS_PRIV            = 0xC0,
        CLASS_MASK            = 0xC0,
        PC_MASK               = 0x20,
        PC_CONSTRUCT          = 0x20,
        PC_PRIMITIVE          = 0x00,
        TAG_MASK              = 0x1F,
        TAG_BOOLEAN           = 0x01,
        TAG_INTEGER           = 0x02,
        TAG_BIT_STRING        = 0x03,
        TAG_OCTET_STRING      = 0x04,
        TAG_OBJECT_IDENTIFIER = 0x06,
        TAG_ENUMERATED        = 0x0A,
        TAG_SEQUENCE          = 0x10,
        TAG_SEQUENCE_OF       = 0x10,
        TAG_GENERAL_STRING    = 0x1B,
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

    inline void backward_push_octet_string_header(std::vector<uint8_t> & v, uint32_t payload_size)
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
    inline void backward_push_small_integer(std::vector<uint8_t> & v, uint8_t value)
    {
        v.push_back(value);
        v.push_back(1); // length
        v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER);
    }

    inline void backward_push_15bits_integer(std::vector<uint8_t> & v, uint16_t value)
    {
        v.push_back(value);
        v.push_back(value >> 8);
        v.push_back(2); // length
        v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER);
    }

    inline void backward_push_23bits_integer(std::vector<uint8_t> & v, uint32_t value)
    {
        v.push_back(value);
        v.push_back(value >> 8);
        v.push_back(value >> 16);
        v.push_back(3); // length
        v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER);
    }

    inline void backward_push_32bits_integer(std::vector<uint8_t> & v, uint32_t value)
    {
        v.push_back(value);
        v.push_back(value >> 8);
        v.push_back(value >> 16);
        v.push_back(value >> 24);
        v.push_back(4); // length
        v.push_back(CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER);
    }

    inline void backward_push_integer(std::vector<uint8_t> & v, uint32_t value)
    {
        if (value < 0x80) {
            backward_push_small_integer(v, uint8_t(value));
        }
        else if (value <  0x8000) {
            backward_push_15bits_integer(v, uint16_t(value));
        }
        else if (value <  0x800000) {
            backward_push_23bits_integer(v, value);
        }
        else {
            backward_push_32bits_integer(v, value);
        }
    }


    inline std::vector<uint8_t> mkOptionalNegoTokensHeader(uint32_t payload_size)
    {
        std::vector<uint8_t> head;

        if (payload_size > 0) {
            backward_push_octet_string_header(head, payload_size);
            backward_push_tagged_field_header(head, payload_size + head.size(), 0);
            backward_push_sequence_tag_field_header(head, payload_size + head.size());
            backward_push_sequence_tag_field_header(head, payload_size + head.size());
            backward_push_tagged_field_header(head, payload_size + head.size(), 1);
            std::reverse(head.begin(), head.end());
        }
        return head;
    }

    inline std::vector<uint8_t> mkContextualFieldHeader(uint32_t payload_size, uint8_t tag)
    {
        std::vector<uint8_t> head;
        backward_push_tagged_field_header(head, payload_size, tag);
        std::reverse(head.begin(), head.end());
        return head;
    }


    inline std::vector<uint8_t> mkOctetStringHeader(uint32_t payload_size)
    {
        std::vector<uint8_t> head;
        backward_push_octet_string_header(head, payload_size);
        std::reverse(head.begin(), head.end());
        return head;
    }


    inline std::vector<uint8_t> mkMandatoryOctetStringFieldHeader(uint32_t payload_size, uint8_t tag)
    {
        std::vector<uint8_t> head;
        backward_push_octet_string_header(head, payload_size);
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

    inline std::vector<uint8_t> mkSmallInteger(uint32_t value)
    {
        std::vector<uint8_t> field;
        backward_push_small_integer(field, value);
        std::reverse(field.begin(), field.end());
        return field;
    }

    inline std::vector<uint8_t> mkSmallIntegerField(uint8_t value, uint8_t tag)
    {
        std::vector<uint8_t> field;
        backward_push_small_integer(field, value);
        backward_push_tagged_field_header(field, field.size(), tag);
        std::reverse(field.begin(), field.end());
        return field;
    }

    inline std::vector<uint8_t> mkInteger(uint32_t value)
    {
        std::vector<uint8_t> field;
        backward_push_integer(field, value);
        std::reverse(field.begin(), field.end());
        return field;
    }

    inline std::vector<uint8_t> mkIntegerField(uint32_t value, uint8_t tag)
    {
        std::vector<uint8_t> field;
        backward_push_integer(field, value);
        backward_push_tagged_field_header(field, field.size(), tag);
        std::reverse(field.begin(), field.end());
        return field;
    }

    inline std::vector<uint8_t> mkSequenceHeader(uint32_t payload_size)
    {
        std::vector<uint8_t> head;
        backward_push_sequence_tag_field_header(head, payload_size);
        std::reverse(head.begin(), head.end());
        return head;
    }

    inline bool check_ber_ctxt_tag(bytes_view s, uint8_t tag)
    {
        if (s.empty()) {
            return false;
        }
        return s[0] == (CLASS_CTXT|PC_CONSTRUCT|tag);
    }

    inline std::pair<size_t, bytes_view> pop_length(bytes_view s, const char * message, error_type eid) {
        // read length
        if (s.empty()) {
            LOG(LOG_ERR, "%s: Ber parse error", message);
            throw Error(eid);
        }
        size_t length = s[0];
        if (s[0] > 0x80) {
            switch (length){
            case 0x81:
                if (s.size() < 2) {
                    LOG(LOG_ERR, "%s: Ber parse error", message);
                    throw Error(eid);
                }
                length = s[1];
                if (s.size() < 2 + length){
                    LOG(LOG_ERR, "%s: Ber Not enough data", message);
                    throw Error(eid);
                }
                return {length, s.drop_front(2)};
            case 0x82:
                if (s.size() < 3) {
                    LOG(LOG_ERR, "%s: Ber parse error", message);
                    throw Error(eid);
                }
                length = (s[2] | (s[1] << 8)); // uint16_be()
                if (s.size() < 3 + length){
                    LOG(LOG_ERR, "%s: Ber Not enough data", message);
                    throw Error(eid);
                }
                return {length, s.drop_front(3)};
            default:
                LOG(LOG_ERR, "%s: Ber parse error", message);
                throw Error(eid);
            }
        }
        if (s.size() < 1 + length){
            LOG(LOG_ERR, "%s: Ber Not enough data", message);
            throw Error(eid);
        }
        return {length, s.drop_front(1)};
    }

    inline bytes_view pop_check_tag(bytes_view s, uint8_t tag, const char * message, error_type eid)
    {
        if (s.empty()) {
            LOG(LOG_ERR, "%s: Ber data truncated", message);
            throw Error(eid);
        }
        uint8_t tag_byte = s[0];
        if (tag_byte != tag) { /*NOLINT*/
            LOG(LOG_ERR, "%s: Ber unexpected tag", message);
            throw Error(eid);
        }
        return s.drop_front(1);
    }

    inline std::pair<size_t, bytes_view> pop_tag_length(bytes_view s, uint8_t tag, const char * message, error_type eid)
    {
        return pop_length(pop_check_tag(s, tag, message, eid), message, eid);
    }

    inline std::pair<int, bytes_view> pop_integer(bytes_view s, const char * message, error_type eid)
    {
        auto [byte, queue] = pop_tag_length(s, CLASS_UNIV | PC_PRIMITIVE | TAG_INTEGER, message, eid);

        if ((byte < 1) || (byte > 4)){
            LOG(LOG_ERR, "%s: Ber unexpected integer length %zu", message, byte);
            throw Error(eid);
        }
        // Now bytes contains length of integer value
        if (queue.size() < byte) {
            LOG(LOG_ERR, "%s: Ber integer data truncated %zu", message, byte);
            throw Error(eid);
        }
        InStream in_s(queue);
        switch (byte) {
        default:
            break;
        case 3:
            return {in_s.in_uint24_be(), queue.drop_front(3)};
        case 2:
            return {in_s.in_uint16_be(), queue.drop_front(2)};
        case 1:
            return {in_s.in_uint8(), queue.drop_front(1)};
        }
        return {in_s.in_uint32_be(), queue.drop_front(4)};
    }

    inline std::pair<int, bytes_view> pop_integer_field(bytes_view s, uint8_t tag, const char * message, error_type eid)
    {
        auto [length, queue] = pop_tag_length(s, CLASS_CTXT|PC_CONSTRUCT|tag, "TS Request", ERR_CREDSSP_TS_REQUEST);
        if (queue.size() < length) {
            LOG(LOG_ERR, "%s: Ber tagged integer field truncated", message);
            throw Error(eid);
        }
        return pop_integer(queue, message, eid);
    }

    inline std::vector<uint8_t> read_mandatory_octet_string(InStream & stream, uint8_t tag, const char * message, error_type eid)
    {
        auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), CLASS_CTXT|PC_CONSTRUCT|tag, message, eid);
        stream.in_skip_bytes(stream.in_remain()-queue1.size());
        (void)length1;

        auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING, message, eid);
        stream.in_skip_bytes(stream.in_remain()-queue2.size());

        auto av = stream.in_skip_bytes(length2);
        return {av.data(), av.data()+av.size()};
    }

    inline std::vector<uint8_t> read_optional_octet_string(InStream & stream, uint8_t tag, const char * message, error_type eid)
    {
        if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), tag)) {
            return read_mandatory_octet_string(stream, tag, message, eid);
        }
        return {};
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

static const int CLIENT_NONCE_LENGTH = 32;

struct ClientNonce {
    bool initialized = false;
    std::vector<uint8_t> clientNonce;

    ClientNonce() : clientNonce{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        {}

    bool isset() const {
        return this->initialized;
    }

    void reset()
    {
        this->initialized = false;
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
    uint32_t error_code{0};
    /* [5] clientNonce (OCTET STRING OPTIONAL) */
    ClientNonce clientNonce;

    TSRequest(uint32_t version = 6): version(version), use_version(this->version)
    {
    }
};


inline std::vector<uint8_t> emitTSRequest(uint32_t version,
                                          bytes_view negoTokens,
                                          bytes_view authInfo,
                                          bytes_view pubKeyAuth,
                                          uint32_t error_code,
                                          bytes_view clientNonce,
                                          bool nonce_initialized,
                                          bool verbose)
{


    // version    [0] INTEGER,
    auto ber_version_field = BER::mkSmallIntegerField(version, 0);
    // negoTokens [1] NegoData OPTIONAL
    auto ber_nego_tokens_header = BER::mkOptionalNegoTokensHeader(negoTokens.size());
    // authInfo   [2] OCTET STRING OPTIONAL
    auto ber_auth_info_header = BER::mkOptionalOctetStringFieldHeader(authInfo.size(), 2);
    // pubKeyAuth [3] OCTET STRING OPTIONAL
    auto ber_pub_key_auth_header = BER::mkOptionalOctetStringFieldHeader(pubKeyAuth.size(), 3);
    // [4] errorCode (INTEGER) OPTIONAL
    std::vector<uint8_t> ber_error_code_field;
    if ((version == 3 || version == 4 || version >= 6) && error_code != 0) {
        ber_error_code_field = BER::mkIntegerField(error_code, 4);
    }
    // clientNonce[5] OCTET STRING OPTIONAL
    auto ber_nonce_header = BER::mkOptionalOctetStringFieldHeader((version >= 5 && nonce_initialized)?CLIENT_NONCE_LENGTH:0,5);

    /* TSRequest */
    size_t ts_request_length = ber_version_field.size()
          + ber_nego_tokens_header.size()  + negoTokens.size()
          + ber_auth_info_header.size()    + authInfo.size()
          + ber_pub_key_auth_header.size() + pubKeyAuth.size()
          + ber_error_code_field.size()
          + (version >= 5 && nonce_initialized)*(ber_nonce_header.size()+clientNonce.size());

    auto ber_ts_request_header = BER::mkSequenceHeader(ts_request_length);

    std::vector<uint8_t> result = std::vector<uint8_t>{}
           << ber_ts_request_header
           << ber_version_field
           << ber_nego_tokens_header << negoTokens
           << ber_auth_info_header << authInfo
           << ber_pub_key_auth_header << pubKeyAuth
           << ber_error_code_field;

    if (version >= 5 && nonce_initialized){
       result << ber_nonce_header << clientNonce;
    }

    if (verbose) {
        LOG(LOG_INFO, "TSRequest hexdump ---------------------------------");
        LOG(LOG_INFO, "TSRequest version %u ------------------------------", version);
        LOG(LOG_INFO, "TSRequest negoTokens ------------------------------");
        hexdump_d(negoTokens);
        LOG(LOG_INFO, "TSRequest authInfo --------------------------------");
        hexdump_d(authInfo);
        LOG(LOG_INFO, "TSRequest pubkeyAuth ------------------------------");
        hexdump_d(pubKeyAuth);
        LOG(LOG_INFO, "TSRequest error_code %u ---------------------------", error_code);
        LOG(LOG_INFO, "TSRequest clientNonce -----------------------------");
        hexdump_d(clientNonce);

        LOG(LOG_INFO, "TSRequest ts_request_header --(request length = %u)-", unsigned(ts_request_length));
        hexdump_d(ber_ts_request_header);
        LOG(LOG_INFO, "TSRequest version_field ---------------------------");
        hexdump_d(ber_version_field);
        LOG(LOG_INFO, "TSRequest nego_tokens_header ----------------------");
        hexdump_d(ber_nego_tokens_header);
        LOG(LOG_INFO, "TSRequest auth_info_header ------------------------");
        hexdump_d(ber_auth_info_header);
        LOG(LOG_INFO, "TSRequest pub_key_auth_header ---------------------");
        hexdump_d(ber_pub_key_auth_header);
        LOG(LOG_INFO, "TSRequest error_code field ------------------------");
        hexdump_d(ber_error_code_field);
        LOG(LOG_INFO, "TSRequest nonce -----------------------------------");
        hexdump_d(ber_nonce_header);
        LOG(LOG_INFO, "emit TSRequest full dump--------------------------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emit TSRequest hexdump -DONE----------------------------");
    }
    return result;
}

inline TSRequest recvTSRequest(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recv TSRequest full dump++++++++++++++++++++++++++++++++");
        hexdump_d(data);
        LOG(LOG_INFO, "recv TSRequest hexdump - START PARSING DATA+++++++++++++");
    }

    InStream stream(data);
    TSRequest self(6);

    /* TSRequest */
    auto [length, queue] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TS Request",    ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue.size());
    (void)length;

    // version    [0] INTEGER,
    auto [value8, queue8] = BER::pop_integer_field(stream.remaining_bytes(), 0, "TS Request [0]", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue8.size());
    self.use_version = value8;
    if (verbose) {
        LOG(LOG_INFO, "Credssp recvTSRequest() Remote Version %u", self.use_version);
    }

    // [1] negoTokens (NegoData) OPTIONAL
    if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), 1)) {
        stream.in_skip_bytes(1);
        auto [len0, queue0] = BER::pop_length(stream.remaining_bytes(), "TS Request [1] negoTokens", ERR_CREDSSP_TS_REQUEST);
        stream.in_skip_bytes(stream.in_remain()-queue0.size());
        (void)len0;

        // * NegoData ::= SEQUENCE OF NegoDataItem
        // *
        // * NegoDataItem ::= SEQUENCE {
        // *     negoToken [0] OCTET STRING
        // * }

        // NegoData ::= SEQUENCE OF NegoDataItem
        auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TS Request [1] negoTokens NegoData", ERR_CREDSSP_TS_REQUEST);
        stream.in_skip_bytes(stream.in_remain()-queue1.size());
        (void)length1;

        // NegoDataItem ::= SEQUENCE {
        auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TS Request [1] negoTokens NegoData NegoDataItem", ERR_CREDSSP_TS_REQUEST);
        stream.in_skip_bytes(stream.in_remain()-queue2.size());
        (void)length2;

        self.negoTokens = BER::read_mandatory_octet_string(stream, 0, "TS Request [1] negoTokens NegoData NegoDataItem [0] negoToken", ERR_CREDSSP_TS_REQUEST);
    }

    self.authInfo   = BER::read_optional_octet_string(stream, 2, "TSRequest [2] authInfo", ERR_CREDSSP_TS_REQUEST);
    self.pubKeyAuth = BER::read_optional_octet_string(stream, 3, "TSRequest [2] pubKeyAuth", ERR_CREDSSP_TS_REQUEST);

    /* [4] errorCode (INTEGER) */
    if (self.use_version >= 3 && self.use_version != 5){
        if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), 4)){
            auto [value, queue] = BER::pop_integer_field(stream.remaining_bytes(), 4, "TS Request [4] errorCode", ERR_CREDSSP_TS_REQUEST);
            stream.in_skip_bytes(stream.in_remain()-queue.size());
            self.error_code = value;
            LOG(LOG_INFO, "Credssp recvTSCredentials() "
                "ErrorCode = %x, Facility = %x, Code = %x",
                self.error_code,
                (self.error_code >> 16) & 0x7FF,
                (self.error_code & 0xFFFF)
            );
        }
    }

    /* [5] clientNonce (OCTET STRING) */
    if (self.use_version >= 5){
        self.clientNonce.clientNonce = BER::read_optional_octet_string(stream, 5, "TSRequest [5] clientNonce", ERR_CREDSSP_TS_REQUEST);
        if (not self.clientNonce.clientNonce.empty()){
            self.clientNonce.initialized = true;
        }
    }
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
    std::vector<uint8_t> domainName;
    std::vector<uint8_t> userName;
    std::vector<uint8_t> password;
};


inline TSPasswordCreds recvTSPasswordCreds(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recvTSPasswordCreds full dump--------------------------------");
        hexdump_d(data);
        LOG(LOG_INFO, "recvTSPasswordCreds hexdump - START PARSING DATA-------------");
    }

    InStream stream(data); // check all is consumed
    TSPasswordCreds self;
    /* TSPasswordCreds (SEQUENCE) */
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSPasswordCreds", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    /* [0] domainName (OCTET STRING) */
    auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|0, "TSPasswordCreds::domainName", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    (void)length2;

    auto [length3, queue3] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSPasswordCreds::domainName", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue3.size());

    self.domainName.resize(length3);
    stream.in_copy_bytes(self.domainName.data(), self.domainName.size());

    /* [1] userName (OCTET STRING) */
    auto [length4, queue4] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|1, "TSPasswordCreds::userName", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue4.size());
    (void)length4;

    auto [length5, queue5] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSPasswordCreds::userName", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue5.size());

    self.userName.resize(length5);
    stream.in_copy_bytes(self.userName.data(), self.userName.size());

    /* [2] password (OCTET STRING) */
    auto [length6, queue6] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|2, "TSPasswordCreds::password", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue6.size());
    (void)length6;

    auto [length7, queue7] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSPasswordCreds::password", ERR_CREDSSP_TS_REQUEST);

    stream.in_skip_bytes(stream.in_remain()-queue7.size());

    self.password.resize(length7);
    stream.in_copy_bytes(self.password.data(), self.password.size());

    return self;
}

inline std::vector<uint8_t> emitTSPasswordCreds(bytes_view domain, bytes_view user, bytes_view password, bool verbose)
{
    // [0] domainName (OCTET STRING)
    auto ber_domain_name_header = BER::mkMandatoryOctetStringFieldHeader(domain.size(), 0);
    // [1] userName (OCTET STRING)
    auto ber_user_name_header = BER::mkMandatoryOctetStringFieldHeader(user.size(), 1);
    // [2] password (OCTET STRING)
    auto ber_password_header = BER::mkMandatoryOctetStringFieldHeader(password.size(), 2);

    // TSPasswordCreds (SEQUENCE)
    size_t ts_password_creds_length = ber_domain_name_header.size()+ domain.size()
                             + ber_user_name_header.size()+user.size()
                             + ber_password_header.size()+password.size();

    auto ber_ts_password_creds_header = BER::mkSequenceHeader(uint32_t(ts_password_creds_length));

    std::vector<uint8_t> result = std::move(ber_ts_password_creds_header);
    result << ber_domain_name_header << domain
           << ber_user_name_header << user
           << ber_password_header << password;

    if (verbose) {
        LOG(LOG_INFO, "emitPasswordsCreds full dump ------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emitPasswordsCreds hexdump done ---------");
    }

    return result;
}


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
    std::vector<uint8_t> cardName;
    std::vector<uint8_t> readerName;
    std::vector<uint8_t> containerName;
    std::vector<uint8_t> cspName;
};


inline TSCspDataDetail recvTSCspDataDetail(bytes_view data)
{
    InStream stream(data);
    TSCspDataDetail self;
    // TSCspDataDetail ::= SEQUENCE
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSCspDataDetail Sequence", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [value2, queue2] = BER::pop_integer_field(queue1, 0, "TSCspDataDetail [0] keySpec", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    self.keySpec       = value2;

    self.cardName      = BER::read_optional_octet_string(stream, 1, "TSCspDataDetail [1] cardName", ERR_CREDSSP_TS_REQUEST);
    self.readerName    = BER::read_optional_octet_string(stream, 2, "TSCspDataDetail [2] readerName", ERR_CREDSSP_TS_REQUEST);
    self.containerName = BER::read_optional_octet_string(stream, 3, "TSCspDataDetail [3] containerName", ERR_CREDSSP_TS_REQUEST);
    self.cspName       = BER::read_optional_octet_string(stream, 4, "TSCspDataDetail [4] cspName", ERR_CREDSSP_TS_REQUEST);
    return self;
}


inline std::vector<uint8_t> emitTSCspDataDetail(uint32_t keySpec,
                                                bytes_view cardName,
                                                bytes_view readerName,
                                                bytes_view containerName,
                                                bytes_view cspName)
{
    auto ber_keySpec_Field        = BER::mkIntegerField(keySpec, 0);
    auto ber_cardName_Header      = BER::mkOptionalOctetStringFieldHeader(cardName.size(), 1);
    auto ber_readerName_Header    = BER::mkOptionalOctetStringFieldHeader(readerName.size(), 2);
    auto ber_containerName_Header = BER::mkOptionalOctetStringFieldHeader(containerName.size(), 3);
    auto ber_cspName_Header       = BER::mkOptionalOctetStringFieldHeader(cspName.size(), 4);

    auto innerSize = ber_keySpec_Field.size()
                  + ber_cardName_Header.size() + cardName.size()
                  + ber_readerName_Header.size() + readerName.size()
                  + ber_containerName_Header.size() + containerName.size()
                  + ber_cspName_Header.size() + cspName.size();

    // TSCspDataDetail (SEQUENCE)
    auto sequence_header = BER::mkSequenceHeader(innerSize);

    std::vector<uint8_t> result = std::move(sequence_header);
    result << ber_keySpec_Field
           << ber_cardName_Header      << cardName
           << ber_readerName_Header    << readerName
           << ber_containerName_Header << containerName
           << ber_cspName_Header       << cspName;

    return result;
}


/*
 * TSSmartCardCreds ::= SEQUENCE {
 *     pin        [0] OCTET STRING,
 *     cspData    [1] TSCspDataDetail,
 *     userHint   [2] OCTET STRING OPTIONAL,
 *     domainHint [3] OCTET STRING OPTIONAL
 * }
 */

struct TSSmartCardCreds {
    std::vector<uint8_t> pin;
    TSCspDataDetail cspData;
    std::vector<uint8_t> userHint;
    std::vector<uint8_t> domainHint;
};

inline TSSmartCardCreds recvTSSmartCardCreds(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recvTSSmartCardCreds full dump--------------------------------");
        hexdump_d(data);
        LOG(LOG_INFO, "recvTSSmartCardCreds hexdump - START PARSING DATA-------------");
    }

    InStream stream(data);
    TSSmartCardCreds self;
    /* TSSmartCardCreds (SEQUENCE) */
    auto [length0, queue0] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSSmartCardCreds", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue0.size());
    (void)length0;

    /* [0] pin (OCTET STRING) */
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|0, "TSSmartCardCreds [0] pin", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSSmartCardCreds [0] pin", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());

    self.pin.resize(length2);
    stream.in_copy_bytes(self.pin);

    /* [1] cspData (TSCspDataDetail) */
    {
        auto [tSCspDataDetailLength, queue3] =  BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|1, "TSSmartCardCreds [1] cspData", ERR_CREDSSP_TS_REQUEST);
        stream.in_skip_bytes(stream.in_remain()-queue3.size());

        bytes_view data = stream.in_skip_bytes(tSCspDataDetailLength);
        self.cspData = recvTSCspDataDetail(data);
    }

    self.userHint   = BER::read_optional_octet_string(stream, 2, "TSSmartCardCreds [2] userHint", ERR_CREDSSP_TS_REQUEST);
    self.domainHint = BER::read_optional_octet_string(stream, 3, "TSSmartCardCreds [3] domainHint", ERR_CREDSSP_TS_REQUEST);
    return self;
}

inline std::vector<uint8_t> emitTSSmartCardCreds(
                  buffer_view pin, buffer_view userHint, bytes_view domainHint,
                  uint32_t keySpec,
                  bytes_view cardName,
                  bytes_view readerName,
                  bytes_view containerName,
                  bytes_view cspName)
{
    // [0] pin (OCTET STRING)
    auto ber_pin_header = BER::mkMandatoryOctetStringFieldHeader(pin.size(), 0);

    // [1] cspData (TSCspDataDetail)
    auto ber_TSCspDataDetail = emitTSCspDataDetail(keySpec, cardName, readerName, containerName, cspName);
    auto ber_CspDataDetail_header = BER::mkContextualFieldHeader(ber_TSCspDataDetail.size(), 1);

    /* [2] userHint (OCTET STRING OPTIONAL) */
    auto ber_userHint_header = BER::mkOptionalOctetStringFieldHeader(userHint.size(), 2);

    /* [3] domainHint (OCTET STRING OPTIONAL) */
    auto ber_domainHint_header = BER::mkOptionalOctetStringFieldHeader(domainHint.size(), 3);

    /* TSCredentials (SEQUENCE) */
    size_t ts_smartcards_creds_length = ber_pin_header.size() + pin.size()
                  + ber_CspDataDetail_header.size() + ber_TSCspDataDetail.size()
                  + ber_userHint_header.size() + userHint.size()
                  + ber_domainHint_header.size() + domainHint.size()
                  ;

    auto ber_ts_smartcards_creds_header = BER::mkSequenceHeader(ts_smartcards_creds_length);

    std::vector<uint8_t> result = std::move(ber_ts_smartcards_creds_header);
    result << ber_pin_header           << pin
           << ber_CspDataDetail_header << ber_TSCspDataDetail
           << ber_userHint_header      << userHint
           << ber_domainHint_header    << domainHint;

    return result;
}


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
    // For now, TSCredentials only contains TSPasswordCreds (not TSSmartCardCreds)
};

inline TSCredentials recvTSCredentials(bytes_view data, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "recvTSCredentials full dump--------------------------------");
        hexdump_d(data);
        LOG(LOG_INFO, "recvTSCredentials hexdump - START PARSING DATA-------------");
    }
    InStream stream(data);
    TSCredentials self;
    // stream is decrypted and should be decrypted before calling recv

    // TSCredentials (SEQUENCE)
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_CONSTRUCT| BER::TAG_SEQUENCE_OF, "TSCredentials", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    // [0] credType (INTEGER)
    auto [value2, queue2] = BER::pop_integer_field(stream.remaining_bytes(), 0, "TSCredentials [0] credType ", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    self.credType = value2;


    // [1] credentials (OCTET STRING)
    auto [length3, queue3] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_CTXT|BER::PC_CONSTRUCT|1, "TSCredentials", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue3.size());
    (void)length3;

    auto [creds_length, queue4] = BER::pop_tag_length(stream.remaining_bytes(), BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING, "TSSmartCardCreds [3] domainHint", ERR_CREDSSP_TS_REQUEST);
    stream.in_skip_bytes(stream.in_remain()-queue4.size());


    if (self.credType == 2) {
        bytes_view data = stream.in_skip_bytes(creds_length);
        self.smartcardCreds = recvTSSmartCardCreds(data, verbose);
    } else {
        bytes_view data = stream.in_skip_bytes(creds_length);
        self.passCreds = recvTSPasswordCreds(data, verbose);
    }
    return self;
}

inline std::vector<uint8_t> emitTSCredentialsPassword(bytes_view domainName, bytes_view userName, bytes_view password, bool verbose)
{
    // [0] credType (INTEGER) : 1 means password
    auto ber_credtype_field = BER::mkSmallIntegerField(1, 0);

    // [1] credentials (OCTET STRING)
    std::vector<uint8_t> ber_credentials = emitTSPasswordCreds(domainName, userName, password, verbose);
    auto ber_credentials_header = BER::mkMandatoryOctetStringFieldHeader(ber_credentials.size(), 1);

    // TSCredentials (SEQUENCE)
    auto inner_size = ber_credtype_field.size() + ber_credentials_header.size() + ber_credentials.size();
    auto sequence_header = BER::mkSequenceHeader(inner_size);

    std::vector<uint8_t> result = std::move(sequence_header);
    result << ber_credtype_field
           << ber_credentials_header << ber_credentials;

    if (verbose) {
        LOG(LOG_INFO, "emitTSCredentialsPassword full dump ------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emitTSCredentialsPassword hexdump done----------");
    }

    return result;
}

inline std::vector<uint8_t> emitTSCredentialsSmartCard(
                  buffer_view pin, buffer_view userHint, bytes_view domainHint,
                  uint32_t keySpec,
                  bytes_view cardName,
                  bytes_view readerName,
                  bytes_view containerName,
                  bytes_view cspName,
                  bool verbose)
{
    // [0] credType (INTEGER): 2 means SmartCard
    auto ber_credtype_field = BER::mkSmallIntegerField(2, 0);

    // [1] credentials (OCTET STRING)
    std::vector<uint8_t> ber_credentials = emitTSSmartCardCreds(pin, userHint, domainHint, keySpec, cardName, readerName, containerName, cspName);
    auto ber_credentials_header = BER::mkMandatoryOctetStringFieldHeader(ber_credentials.size(), 1);

    // TSCredentials (SEQUENCE)
    auto inner_size = ber_credtype_field.size() + ber_credentials_header.size() + ber_credentials.size();
    auto sequence_header = BER::mkSequenceHeader(inner_size);

    std::vector<uint8_t> result = std::move(sequence_header);
    result << ber_credtype_field
           << ber_credentials_header << ber_credentials;

    if (verbose) {
        LOG(LOG_INFO, "emitTSCredentialsSmartCard full dump ------------");
        hexdump_d(result);
        LOG(LOG_INFO, "emitTSCredentialsSmartCard hexdump done----------");
    }

    return result;
}
