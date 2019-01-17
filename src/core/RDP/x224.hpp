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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   X224 class 0 packets management, complies with
   RFC 1006, ISO transport services on top of the TCP
*/


#pragma once

#include "transport/transport.hpp"
#include "utils/stream.hpp"
#include "utils/log.hpp"
#include "core/error.hpp"
#include "core/RDP/fastpath.hpp"
#include "utils/parse.hpp"
#include "utils/hexdump.hpp"

#include <cstdint>
#include <cinttypes>

//##############################################################################
namespace X224
//##############################################################################
{
    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.
    // -------------------------------------------------------------------------
    // Packet header to delimit data units in an octet stream
    // [ITU-T X.224] expects information to be transmitted and received in
    // discrete units termed network service data units (NSDUs), which can be an
    // arbitrary sequence of octets. Although other classes of the transport
    // protocol may combine more than one TPDU inside a single NSDU, X.224 class
    // 0 does not use this facility. Hence, in the context of T.123 protocol
    // stacks, a TPDU may be identified with its underlying NSDU.
    // A fundamental difference between the network service expected by
    // [ITU-T X.224] and an octet stream transfer service, as characterized in
    // clause 7.6, is that the latter conveys a continuous sequence of octets
    // with no explicit boundaries between related groups of octets.
    // This clause specifies a distinct protocol layer to repair the discrepancy
    // and meet the needs of [ITU-T X.224]. It defines a simple packet format
    // whose purpose is to delimit TPDUs. Each packet, termed a TPKT, is a unit
    // composed of a whole integral number of octets, of variable length.
    // A TPKT consists of two parts: a packet header, followed by a TPDU. The
    // format of the packet header is constant, independent of the type of TPDU.
    // The packet header consists of four octets as shown in Figure 11.

    // Figure 11 – Format of the TPKT packet header
    //
    //           -----------------------------
    //           |         0000 0011         | 1
    //           -----------------------------
    //           |         Reserved 2        | 2
    //           -----------------------------
    //           | Most significant octet    | 3
    //           |    of TPKT length         |
    //           -----------------------------
    //           | least significant octet   | 4
    //           |       of TPKT length      |
    //           -----------------------------
    //           :         TPDU              : 5-?
    //           - - - - - - - - - - - - - - -

    // Octet 1 is a version number, with binary value 0000 0011.
    // Octet 2 is reserved for further study.
    // Octets 3 and 4 are the unsigned 16-bit binary encoding of the TPKT
    //   length. This is the length of the entire packet in octets, including
    //   both the packet header and the TPDU.


    // Since an X.224 TPDU occupies at least 3 octets, the minimum value of TPKT
    // length is 7. The maximum value is 65535. This permits a maximum TPDU size
    // of 65531 octets.

    // NOTE – This description of the TPKT protocol layer agrees with RFC 1006,
    // ISO transport services on top of the TCP.
    // -------------------------------------------------------------------------

    // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
    //                     section 13.7.
    // -------------------------------------------------------------------------
    // 13.7 Data (DT) TPDU
    // 13.7.1 Structure
    // Depending on the class and the option the DT-TPDU shall have one of the
    // following structures:

    // a) Normal format for classes 0 and 1:
    //   1         2             3        4           ...            end
    // ------------------------------------------------------------------
    // |    |      |      |            |
    // | LI |   DT y=ROA  |  TPDU-NR   |         User Data
    // |    |  1111 000y  |  and EOT   |
    // ------------------------------------------------------------------

    // b) Normal format for classes 2, 3 and 4:
    //   1         2         3      4       5    ...   6   p        p+1     end
    // -------------------------------------------------------------------------
    // |    |      |     |      |      |            |            |
    // | LI |   DT y=ROA |   DST-REF   |  TPDU-NR   |  variable  |   User Data
    // |    |  1111 000y |             |  and EOT   |    part    |
    // -------------------------------------------------------------------------

    // b) Extended format for use in classes 2, 3 and 4 when selected during
    //    connection stablishment:
    //   1         2         3      4    5, 6, 7, 8    9     p      p+1     end
    // -------------------------------------------------------------------------
    // |    |      |     |      |      |            |            |
    // | LI |   DT y=ROA |   DST-REF   |  TPDU-NR   |  variable  |   User Data
    // |    |  1111 000y |             |  and EOT   |    part    |
    // -------------------------------------------------------------------------

    // 13.7.2 LI : Length indicator field (defined in 13.2.1)
    // The field is contained in the first octet of the TPDUs. The length is
    // indicated by a binary number, with a maximum value of 254 (1111 1110).
    // The length indicated shall be the header length in octets including
    // parameters, but excluding the length indicator field anduni user data, if
    // any. The value 255 (1111 1111) is reserved for possible extensions.
    // If the length indicated exceeds or is equal to the size of the NS-user
    // data which is present, this is a protocol error.

    // 13.7.3 Fixed part
    // -----------------
    // The fixed part shall contain:
    // a) DT – Data transfer code: bits 8 to 5 shall be set to 1111.
    //    Bits 4 to 2 shall be set to zero.

    // b) ROA – Request of acknowledgement mark: If the request acknowledgement
    //  procedures has not been agreed during connection establishment, bit 1
    //  shall be set to 0 in all DT-TPDUs.
    //  When the request acknowledgement procedure has been agreed during
    //  connection establishment, bit 1 (ROA) is used to request acknowledgement
    //  in classes 1, 3, and 4. When set to one, ROA indicates that the sending
    // transport entity requests an acknowledgement from the receiving transport
    // entity. Otherwise ROA is set to zero.

    // c) DST-REF – See 13.4.3.

    // d) EOT – When set to ONE, it indicates that the current DT-TPDU is the
    // last data unit of a complete DT-TPDU sequence (end of TSDU). EOT is bit 8
    // of octet 3 in class 0 and 1 and bit 8 of octet 5 for classes 2, 3 and 4.

    // e) TPDU-NR – TPDU send sequence number (zero in class 0). May take any
    // value in class 2 without explicit flow control. TPDU-NR is bits 7 to 1 of
    // octet 3 for classes 0 and 1, bits 7 to 1 of octet 5 for normal formats in
    // classes 2, 3 and 4 and bits 7 to 1 of octet 5 together with octets 6, 7
    // and 8 for extended format.

    // NOTE – Depending on the class, the fixed part of the DT-TPDU uses the
    // following octets:
    // classes 0 and 1: octets 2 to 3;
    // classes 2, 3, 4 (normal format): octets 2 to 5;
    // classes 2, 3, 4 (extended format): octets 2 to 8.

    // 13.7.4 Variable part
    // --------------------
    // The variable part shall contain the checksum parameter if the condition
    // defined in 13.2.3.1 applies.
    // If the use of non-blocking expedited data transfer service is negotiated
    // (class 4 only), the variable part shall contain the ED-TPDU-NR for the
    // first DT-TPDU created from a T-DATA request subsequent to the T-EXPEDITED
    // DATA request.
    //               Parameter code: 1001 0000
    //               Parameter length: 2 (normal format)
    //                                 4 (extended format)
    // Parameter value: The ED-TPDU-NR of the ED-TPDU created from the
    // T-EXPEDITED DATA request immediately before the T-DATA request that this
    // DT-TPDU is created from.

    // NOTE – In the case of the normal format, a length of two octets is
    // necessary (when one octet would suffice to express a
    // modulo 2**7 arithmetic number) to ensure that the implicit rule that an
    // even (resp. odd) LI always corresponds to the normal (resp.extended)
    // format is not broken.

    // 13.7.5 User data field
    // ----------------------
    // This field contains data of the TSDU being transmitted.
    // NOTE – The length of this field is limited to the negotiated TPDU size
    // for this transport connection minus 3 octets in classes 0 and 1, and
    // minus 5 octets (normal header format) or 8 octets (extended header
    // format) in the other classes. The variable part, if present, may further
    // reduce the size of the user data field.

    enum : uint8_t {
        CR_TPDU = 0xE0, // Connection Request 1110 xxxx
        CC_TPDU = 0xD0, // Connection Confirm 1101 xxxx
        DR_TPDU = 0x80, // Disconnect Request 1000 0000
        DT_TPDU = 0xF0, // Data               1111 0000 (no ROA = No Ack)
        ER_TPDU = 0x70  // TPDU Error         0111 0000
    };

// TPDU shall contain in the following order:
// a) The Header, comprising:
// - The Length Indicator (LI) field
// - the fixed part
// - the variable part, if present
// b) The Data field if present

// Length Indicator field
// ----------------------

// The field is contained in first octet of the TPDUs. The length is indicated
// by a binary number, with a maximum value of 254. The length indicated shall
// be the header length in octets including parameters, but excluding the length
// indicator field and user data if any. The vaue of 255 is reserved for future
// extensions.

// If the length indicated exceed or is equal to the size of data which is
// actually present, this is a protocol error.

// Fixed Part
// ----------

// The fixed part contains frequently occuring parameters includint the code of
// the TPDU. The length and the structure of the fixd part are defined by the
// TPDU code and in certain cases by the protocol class and the format in use
// (normal or extended). If any of the parameters of the fixed part have and
// invalid value, or if the fixed part cannot be contained withing the header
// (as defined by LI), this is a protocol error.

// TPDU code
// ---------

// This field contains the TPDU code and is contained in octet 2 of the header.
// It is used to define the structure of the remaining header. In the cases we
// care about (class 0) this field is a full octet except for CR_TPDU and
// CC_TPDU. In these two cases the low nibble is used to signal the CDT (initial
// credit allocation).

// Variable Part
// -------------

// The size of the variable part, used to define the less frequently used
// paameters, is LI minus the size of the fixed part.

// Each parameter in the variable part is of the form :
// - parameter code on 1 byte (allowed parameter codes are from 64 and above).
// - parameter length indication
// - parameter value (of the size given by parameter length indication, may be
// empty in which case parameter length indication is zero).

// Parameter codes not defined in x224 are protocol errors, except for CR_TPDU
// in which they should be ignored.

// See docs/X224_class0_cheat_sheet.txt for supported packets format details


    enum {
        TPKT_HEADER_LEN = 4
    };

    // Error codes for DR_TPDU
    enum {
        REASON_NOT_SPECIFIED        = 0,
        REASON_CONGESTION           = 1,
        REASON_SESSION_NOT_ATTACHED = 2,
        REASON_ADDRESS_UNKNOWN      = 3
    };

    // Error codes in ER_TPDU
    enum {
        // REASON_NOT_SPECIFIED         = 0,
        REASON_INVALID_PARAMETER_CODE   = 1,
        REASON_INVALID_TPDU_TYPE        = 2,
        REASON_INVALID_PARAMETER_VALUE  = 3
    };

    enum {
        EOT_MORE_DATA       = 0x00,
        EOT_EOT             = 0x80
    };

    enum {
        RDP_NEG_NONE    = 0,
        RDP_NEG_REQ     = 1,
        RDP_NEG_RSP     = 2,
        RDP_NEG_FAILURE = 3
    };

    enum {
        RESTRICTED_ADMIN_MODE_REQUIRED = 0x01,
        CORRELATION_INFO_PRESENT = 0x08
    };

    enum {
        EXTENDED_CLIENT_DATA_SUPPORTED  = 0x01,
        DYNVC_GFX_PROTOCOL_SUPPORTED    = 0x02,
        NEGRSP_FLAG_RESERVED            = 0x04,
        RESTRICTED_ADMIN_MODE_SUPPORTED = 0x08
    };

    enum {
        PROTOCOL_RDP    = 0,
        PROTOCOL_TLS    = 1,
        PROTOCOL_HYBRID = 2,
        PROTOCOL_RDSTLS = 4,
        PROTOCOL_HYBRID_EX = 8
    };

    enum {
        SSL_REQUIRED_BY_SERVER                = 0x00000001,
        SSL_NOT_ALLOWED_BY_SERVER             = 0x00000002,
        SSL_CERT_NOT_ON_SERVER                = 0x00000003,
        INCONSISTENT_FLAGS                    = 0x00000004,
        HYBRID_REQUIRED_BY_SERVER             = 0x00000005,
        SSL_WITH_USER_AUTH_REQUIRED_BY_SERVER = 0x00000006
    };

    // Factory read full TPDU, other layers are just using what was read by factory
    // Also decode enough data to know the type of packet we are dealing with (TPDU type or fastpath)
    struct RecvFactory {
        int    type;
        size_t length;
        bool   fast_path;

        RecvFactory(Transport & t, uint8_t ** end, size_t bufsize)
                : type(0)
                , length(0)
                , fast_path(false) {
            size_t nbbytes = 0;
            Parse data(*end);
            // TODO We should have less calls to read, one to get length, the other to get data, other short packets are error

            t.recv_boom(*end, 1);
            *end += 1;

            nbbytes++;
            uint8_t tpkt_version = data.in_uint8();
            int action = tpkt_version & 0x03;

            if (action == FastPath::FASTPATH_OUTPUT_ACTION_FASTPATH) {
                this->fast_path = true;

                t.recv_boom(*end, 1);
                *end += 1;
                nbbytes++;

                uint16_t lg = data.in_uint8();
                if (lg & 0x80){
                    t.recv_boom(*end, 1);
                    *end += 1;
                    nbbytes++;
                    uint8_t byte = data.in_uint8();
                    lg = (lg & 0x7F) << 8 | byte;
                }

                this->length = lg;
                if (bufsize < this->length){
                    LOG(LOG_ERR, "Buffer too small to read data need=%zu available=%zu",
                        this->length, bufsize );
                    throw Error(ERR_X224);
                };
                t.recv_boom(*end, this->length - nbbytes);
                *end += this->length - nbbytes;

                return;
            }

            if (action == FastPath::FASTPATH_OUTPUT_ACTION_X224) {
                /* 4 bytes */
                t.recv_boom(*end, X224::TPKT_HEADER_LEN - nbbytes);
                *end += X224::TPKT_HEADER_LEN - nbbytes;
                nbbytes = X224::TPKT_HEADER_LEN;
                data.in_skip_bytes(1);
                uint16_t tpkt_len = data.in_uint16_be();
                if (tpkt_len < 6) {
                    LOG(LOG_ERR, "Bad X224 header, length too short (length = %u)", tpkt_len);
                    throw Error(ERR_X224);
                }
                this->length = tpkt_len;
                if (bufsize < this->length){
                    LOG(LOG_ERR, "Buffer too small to read data need=%zu available=%zu",
                        this->length, bufsize );
                    throw Error(ERR_X224);
                }
                t.recv_boom(*end, tpkt_len - nbbytes );
                *end += tpkt_len - nbbytes;
                data.in_skip_bytes(1);
                uint8_t tpdu_type = data.in_uint8();
                switch (uint8_t(tpdu_type & 0xF0)) {
                case X224::CR_TPDU: // Connection Request 1110 xxxx
                case X224::CC_TPDU: // Connection Confirm 1101 xxxx
                case X224::DR_TPDU: // Disconnect Request 1000 0000
                case X224::DT_TPDU: // Data               1111 0000 (no ROA = No Ack)
                case X224::ER_TPDU: // TPDU Error         0111 0000
                    this->type = tpdu_type & 0xF0;

                break;
                default:
                    this->type = 0;
                    LOG(LOG_ERR, "Bad X224 header, unknown TPDU type (code = %u)", tpdu_type);
                    throw Error(ERR_X224);
                }
            }
            else {
                LOG(LOG_ERR, "Bad X224 header, unknown TPKT version (%.2x)", tpkt_version);
                throw Error(ERR_X224);
            }
        }

    };

    // ################################### COMMON CODE #################################
    struct Recv
    {
        struct Tpkt
        {
            uint8_t version;
            uint16_t len;
        } tpkt;

        explicit Recv(InStream & stream)
        {
            uint16_t length = stream.get_capacity();
            if (length < 4){
                LOG(LOG_ERR, "Truncated TPKT: stream=%u", length);
            }

            // TPKT
            this->tpkt.version = stream.in_uint8();
            stream.in_skip_bytes(1);
            this->tpkt.len = stream.in_uint16_be();
            if (length < this->tpkt.len){
                LOG(LOG_ERR, "Truncated TPKT: stream=%u tpkt=%u",
                    length, this->tpkt.len);
            }
        }
    };


    // ################################ END OF COMMON CODE #################################

    // 2.2.1.1 Client X.224 Connection Request PDU
    // ===========================================

    // The X.224 Connection Request PDU is an RDP Connection Sequence PDU sent from
    // client to server during the Connection Initiation phase (see section 1.3.1.1).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Crq (7 bytes): An X.224 Class 0 Connection Request transport protocol
    // data unit (TPDU), as specified in [X224] section 13.3.

    // Class 0 x224 TPDU
    // -----------------
    //                                                    +--------+
    //                     +----+-----+---------+---------+ CLASS  |
    //                     | LI |     | DST-REF | SRC-REF | OPTION |
    //            +--------+----+-----+---------+---------+--------+
    //            | OFFSET | 4  |  5  |  6   7  |  8   9  |   10   |
    // +----------+--------+----+-----+---------+---------+--------+
    // | Connection Request|    |     |         |         |        |
    // | CR_TPDU 1110 xxxx | 06 |  E0 |  00  00 |  00  00 |   00   |
    // +-------------------+----+-----+---------+---------+--------+

    // routingToken (variable): An optional and variable-length routing token
    // (used for load balancing) terminated by a carriage-return (CR) and line-feed
    // (LF) ANSI sequence. For more information about Terminal Server load balancing
    // and the routing token format, see [MSFT-SDLBTS]. The length of the routing
    // token and CR+LF sequence is included in the X.224 Connection Request Length
    // Indicator field. If this field is present, then the cookie field MUST NOT be
    //  present.

    //cookie (variable): An optional and variable-length ANSI text string terminated
    // by a carriage-return (CR) and line-feed (LF) ANSI sequence. This text string
    // MUST be "Cookie: mstshash=IDENTIFIER", where IDENTIFIER is an ANSI string
    //(an example cookie string is shown in section 4.1.1). The length of the entire
    // cookie string and CR+LF sequence is included in the X.224 Connection Request
    // Length Indicator field. This field MUST NOT be present if the routingToken
    // field is present.

    // rdpNegData (8 bytes): An optional RDP Negotiation Request (section 2.2.1.1.1)
    // structure. The length of this negotiation structure is include " in the X.224
    // Connection Request Length Indicator field.

    // 2.2.1.1.1 RDP Negotiation Request (RDP_NEG_REQ)
    // ===============================================

    // The RDP Negotiation Request structure is used by a client to advertise the
    // security protocols which it supports.

    // type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
    // field MUST be set to 0x01 (TYPE_RDP_NEG_REQ) to indicate that the packet is
    // a Negotiation Request.

    // flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
    // are currently no defined flags so the field MUST be set to 0x00.

    // length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size.
    // This field MUST be set to 0x0008 (8 bytes).

    // requestedProtocols (4 bytes): A 32-bit, unsigned integer. Flags indicating
    // the supported security protocols.

    // +-------------------------------+----------------------------------------------+
    // | 0x00000000 PROTOCOL_RDP       | Standard RDP Security (section 5.3).         |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000001 PROTOCOL_SSL       | TLS 1.0, 1.1 or 1.2 (section 5.4.5.1).       |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000002 PROTOCOL_HYBRID    | Credential Security Support Provider protocol|
    // |                               | (CredSSP) (section 5.4.5.2). If this flag is |
    // |                               | set, then the PROTOCOL_SSL (0x00000001)      |
    // |                               | SHOULD also be set because Transport Layer   |
    // |                               | Security (TLS) is a subset of CredSSP.       |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000004 PROTOCOL_RDSTLS    | RDSTLS protocol (section 5.4.5.3).           |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000008 PROTOCOL_HYBRID_EX | Credential Security Support Provider protocol|
    // |                               | (CredSSP) (section 5.4.5.2) coupled with the |
    // |                               | Early User Authorization Result PDU (section |
    // |                               | 2.2.10.2). If this flag is set, then the     |
    // |                               | PROTOCOL_HYBRID (0x00000002) flag SHOULD     |
    // |                               | also be set. For more information on the     |
    // |                               | sequencing of the CredSSP messages and the   |
    // |                               | Early User Authorization Result PDU, see     |
    // |                               | sections 5.4.2.1 and 5.4.2.2.CredSSP         |
    // |                               | (section 5.4.5.2)                            |
    // +-------------------------------+----------------------------------------------+

    // 2.2.1.1.2 RDP Correlation Info (RDP_NEG_CORRELATION_INFO)
    // =========================================================
    // The RDP Correlation Info structure is used by a client to propagate connection
    // correlation information to the server. This information allows diagnostic tools
    // on the server to track and monitor a specific connection as it is handled by
    // Terminal Services components.

    // type (1 byte): An 8-bit, unsigned integer that indicates the packet type.
    // This field MUST be set to 0x06 (TYPE_RDP_CORRELATION_INFO).

    // flags (1 byte): An 8-bit, unsigned integer that contains protocol flags.
    // There are currently no defined flags, so this field MUST be set to 0x00.

    // length (2 bytes): A 16-bit, unsigned integer that specifies the packet size.
    // This field MUST be set to 0x0024 (36 bytes).

    // correlationId (16 bytes): An array of sixteen 8-bit, unsigned integers that
    // specifies a unique identifier to associate with the connection.

    // reserved (16 bytes): An array of sixteen 8-bit, unsigned i


    // 3.3.5.3.1 Processing X.224 Connection Request PDU
    // =================================================

    // The structure and fields of the X.224 Connection Request PDU are specified
    // in section 2.2.1.1.

    // The embedded length fields within the tpktHeader field ([T123] section 8)
    // MUST be examined for consistency with the received data. If there is any
    // discrepancy, the connection SHOULD be dropped. Other reasons for dropping
    // the connection include:

    // The length of the X.224 Connection Request PDU is less than 11 bytes.

    // The X.224 Connection Request PDU is not Class 0 ([X224] section 13.7).

    // The Destination reference, Source reference, and Class and options fields
    // within the x224Crq field SHOULD be ignored.

    // If the optional routingToken field exists, it MUST be ignored because the
    // routing token is intended to be inspected and parsed by external networking
    // hardware along the connection path (for more information about load balancing
    // of Remote Desktop sessions and the routing token format, see [MSFT-SDLBTS]
    // "Load-Balanced Configurations", "Revectoring Clients", and "Routing Token Format").

    // If the optional cookie field is present, it MUST be ignored.

    // If both the routingToken and cookie fields are present, the server SHOULD
    //  continue with the connection. Since the server does not process either
    //  the routingToken or cookie fields, a client violation of the protocol
    //  specification in section 2.2.1.1 is not an issue. However, including
    //  both the routingToken and the cookie fields will most likely result
    //  in problems when the X.224 Connection Request is inspected and parsed
    // by networking hardware that is used for load balancing Remote Desktop sessions.

    // If the rdpNegData field is not present, it is assumed that the client does
    // not support Enhanced RDP Security (section 5.4) and negotiation data MUST NOT
    // be sent to the client as part of the X.224 Connection Confirm PDU (section 2.2.1.2).
    // If the rdpNegData field is present, it is parsed to check that it contains an RDP
    // Negotiation Request structure, as specified in section 2.2.1.1.1. If this is the case,
    // the flags describing the supported security protocols in the requestedProtocols field
    // are saved in the Received Client Data store (section 3.3.1.1).

    // Once the X.224 Connection Request PDU has been processed successfully, the server MUST
    // send the X.224 Connection Confirm PDU to the client (section 3.3.5.3.2) and update the
    // Connection Start Time store (section 3.3.1.12).

    struct CR_TPDU_Recv : public Recv
    {
        struct CR_Header
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint16_t src_ref;
            uint8_t class_option;
        } tpdu_hdr;

        size_t _header_size;

        size_t cookie_len = 0;
        char cookie[1024];

        uint8_t rdp_neg_type = 0;
        uint8_t rdp_neg_flags = 0;
        uint16_t rdp_neg_length = 0;
        uint32_t rdp_neg_requestedProtocols = 0;

        uint8_t rdp_cinfo_type = 0;
        uint8_t rdp_cinfo_flags = 0;
        uint16_t rdp_cinfo_length = 0;
        uint8_t rdp_cinfo_correlationid[16]{};

        CR_TPDU_Recv(InStream & stream, bool bogus_neg_req, uint32_t verbose = 0)
        : Recv(stream)
        , tpdu_hdr([&]()
            {
                /* LI(1) + code(1) + dst_ref(2) + src_ref(2) + class_option(1) */
                if (!stream.in_check_rem(7)){
                    LOG(LOG_ERR, "Truncated TPDU header: expected=7 remains=%zu", stream.in_remain());
                    throw Error(ERR_X224);
                }

                uint8_t LI = stream.in_uint8();
                uint8_t code = stream.in_uint8();

                if (!(code == X224::CR_TPDU)){
                    LOG(LOG_ERR, "Unexpected TPDU opcode, expected CR_TPDU, got %u", code);
                    throw Error(ERR_X224);
                }

                uint16_t dst_ref = stream.in_uint16_le();
                uint16_t src_ref = stream.in_uint16_le();
                uint8_t class_option = stream.in_uint8();

                return CR_Header{LI, code, dst_ref, src_ref, class_option};
            }())
        , _header_size(X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1)
        {
            if (stream.get_capacity() < this->_header_size){
                LOG(LOG_ERR, "Truncated CR TPDU header: expected %zu, got %zu",
                    this->_header_size, stream.get_capacity());
                throw Error(ERR_X224);
            }

            // TODO CGR: we should fix the code here to support routingtoken (or we may have some troubles with load balancing RDP hardware

            // extended negotiation header
            this->cookie[0] = 0;

            // TODO We should have some reading function in stream to read this
            uint8_t const * end_of_header = stream.get_data() + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            for (uint8_t const * p = stream.get_current() + 1; p < end_of_header ; p++){
                if (p[-1] == 0x0D && p[0] == 0x0A){
                    this->cookie_len = p - (stream.get_data() + 11) + 1;
                    // cookie can't be larger than header (HEADER_LEN + LI + 1 = 230)
                    memcpy(this->cookie, stream.get_data() + 11, this->cookie_len);
                    this->cookie[this->cookie_len] = 0;
                    if (verbose){
                        LOG(LOG_INFO, "cookie: %s [%.2x][%.2x]",
                            this->cookie,
                            unsigned(this->cookie[this->cookie_len-2]),
                            unsigned(this->cookie[this->cookie_len-1]));
                    }
                    break;
                }
            }
            stream.in_skip_bytes(this->cookie_len);

            // 2.2.1.1.1 RDP Negotiation Request (RDP_NEG_REQ)
            if (end_of_header - stream.get_current() >= 8){
                if (verbose){
                    LOG(LOG_INFO, "Found RDP Negotiation Request Structure");
                }
                this->rdp_neg_type = stream.in_uint8();
                this->rdp_neg_flags = stream.in_uint8();
                this->rdp_neg_length = stream.in_uint16_le();
                this->rdp_neg_requestedProtocols = stream.in_uint32_le();

                if (bogus_neg_req){
                    // for broken clients like jrdp
                    stream.in_skip_bytes(end_of_header - stream.get_current());
                }
                else {

                    if (this->rdp_neg_type != X224::RDP_NEG_REQ){
                        LOG(LOG_INFO, "X224:RDP_NEG_REQ Expected LI=%u %x %x %x %x",
                            this->tpdu_hdr.LI, this->rdp_neg_type, this->rdp_neg_flags, this->rdp_neg_length, this->rdp_neg_requestedProtocols);
                        throw Error(ERR_X224);
                    }

                    if (this->rdp_neg_requestedProtocols & X224::PROTOCOL_RDP){
                        LOG(LOG_INFO, "CR Recv: PROTOCOL RDP");
                    }
                    if (this->rdp_neg_requestedProtocols & X224::PROTOCOL_TLS){
                        LOG(LOG_INFO, "CR Recv: PROTOCOL TLS");
                    }
                    if (this->rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID){
                        LOG(LOG_INFO, "CR Recv: PROTOCOL HYBRID");
                    }
                    if (this->rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID_EX){
                        LOG(LOG_INFO, "CR Recv: PROTOCOL HYBRID EX");
                    }
                    if (this->rdp_neg_requestedProtocols
                    & ~(X224::PROTOCOL_RDP
                       |X224::PROTOCOL_TLS
                       |X224::PROTOCOL_HYBRID
                       |X224::PROTOCOL_HYBRID_EX)){
                        LOG(LOG_INFO, "CR Recv: Unknown protocol flags %x", this->rdp_neg_requestedProtocols);
                    }
                }
            }
            // 2.2.1.1.2 RDP Correlation Info (RDP_NEG_CORRELATION_INFO)
            if (this->rdp_neg_flags & CORRELATION_INFO_PRESENT) {
                this->rdp_cinfo_type = stream.in_uint8();
                this->rdp_cinfo_flags = stream.in_uint8();
                this->rdp_cinfo_length = stream.in_uint16_le();
                stream.in_copy_bytes(this->rdp_cinfo_correlationid, 16);
                stream.in_skip_bytes(16);
                hexdump_c(this->rdp_cinfo_correlationid, 16);
            }
                hexdump_c(stream.get_data(), stream.get_capacity());
            if (end_of_header != stream.get_current()){
                LOG(LOG_ERR, "CR TPDU header should be terminated, got trailing data %ld", end_of_header - stream.get_current());
                hexdump_c(stream.get_data(), stream.get_capacity());
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();
        }
    }; // END CLASS CR_TPDU_Recv


    struct CR_TPDU_Send
    {
        CR_TPDU_Send(OutStream & stream, const char * cookie, uint8_t rdp_neg_type, uint8_t rdp_neg_flags, uint32_t rdp_neg_requestedProtocols)
        {

            stream.out_uint8(0x03); // version 3
            stream.out_uint8(0x00);
            size_t cookie_len = strlen(cookie);
            stream.out_uint16_be(11 + cookie_len + (rdp_neg_type?8:0));
            stream.out_uint8(11 + cookie_len + (rdp_neg_type?8:0) - 5);

            stream.out_uint8(X224::CR_TPDU); // CR_TPDU code
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint16_be(0x0000); // SRC-REF
            stream.out_uint8(0x00); // CLASS OPTION

            if (cookie_len){
                stream.out_copy_bytes(cookie, cookie_len);
            }
            if (rdp_neg_type){
                stream.out_uint8(rdp_neg_type);
                stream.out_uint8(rdp_neg_flags);
                stream.out_uint16_le(8);
                stream.out_uint32_le(rdp_neg_requestedProtocols);
            }
        }
    };

    // 2.2.1.2 Server X.224 Connection Confirm PDU
    // ===========================================

    // The X.224 Connection Confirm PDU is an RDP Connection Sequence PDU sent from
    // server to client during the Connection Initiation phase (see section
    // 1.3.1.1). It is sent as a response to the X.224 Connection Request PDU
    // (section 2.2.1.1).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Ccf (7 bytes): An X.224 Class 0 Connection Confirm TPDU, as specified in
    // [X224] section 13.4.

    //    Class 0 x224 TPDU
    //    -----------------
    //                                                       +--------+
    //                        +----+-----+---------+---------+ CLASS  |
    //                        | LI |     | DST-REF | SRC-REF | OPTION |
    //               +--------+----+-----+---------+---------+--------+
    //               | OFFSET | 4  |  5  |  6   7  |  8   9  |   10   |
    //    +----------+--------+----+-----+---------+---------+--------+
    //    | Connection Confirm|    |     |         |         |        |
    //    | CC_TPDU 1101 xxxx | 06 |  D0 |  00  00 |  00  00 |   00   |
    //    +-------------------+----+-----+---------+---------+--------+


    // rdpNegData (8 bytes): Optional RDP Negotiation Response (section 2.2.1.2.1)
    // structure or an optional RDP Negotiation Failure (section 2.2.1.2.2)
    // structure. The length of the negotiation structure is include " in the X.224
    // Connection Confirm Length Indicator field.

    // 2.2.1.2.1 RDP Negotiation Response (RDP_NEG_RSP)
    // ================================================

    // The RDP Negotiation Response structure is used by a server to inform the
    // client of the security protocol which it has selected to use for the
    // connection.

    // type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
    // field MUST be set to 0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is
    // a Negotiation Response.

    // flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

    // +-------------------------------------+---------------------------------------+
    // | 0x01 EXTENDED_CLIENT_DATA_SUPPORTED | The server supports Extended Client   |
    // |                                     | Data Blocks in the GCC Conference     |
    // |                                     | Create Request user data (section     |
    // |                                     | 2.2.1.3).                             |
    // +-------------------------------------+---------------------------------------+
    // | 0x02 DYNVC_GFX_PROTOCOL_SUPPORTED   | The server supports the Graphics      |
    // |                                     | Pipeline Extension Protocol described |
    // |                                     | in [MS-RDPEGFX] sections 1, 2, and 3. |
    // +-------------------------------------+---------------------------------------+
    // | 0x04 RDP_NEGRSP_RESERVED            | An unused flag that is reserved for   |
    // |                                     | future use.                           |
    // +-------------------------------------+---------------------------------------+
    // | 0x08 RESTRICTED_ADMIN_MODE_SUPPORTED| Indicates that the server supports    |
    // |                                     | credential-less logon over CredSSP    |
    // |                                     | (also known as "restricted admin      |
    // |                                     | mode") and it is acceptable for the   |
    // |                                     | client to send empty credentials in   |
    // |                                     | the TSPasswordCreds structure defined |
    // |                                     | in [MS-CSSP] section 2.2.1.2.1.<3>    |
    // +-------------------------------------+---------------------------------------+
    // | 0x10 REDIRECTED_AUTHENTICATION_MODE_SUPPORTED | Indicates that the server   |
    // |                                     +---------+ supports credential-less    |
    // |                                     | logon over CredSSP with credential    |
    // |                                     | redirection (also known as "Remote    |
    // |                                     | Credential Guard"). The client can    |
    // |                                     | send a redirected logon buffer in the |
    // |                                     | TSRemoteGuardCreds structure defined  |
    // |                                     | in [MS-CSSP] section 2.2.1.2.3.       |
    // +-------------------------------------+---------------------------------------+

    // length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
    //   field MUST be set to 0x0008 (8 bytes)

    // selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the
    //  selected security protocol.

    // +-------------------------------+----------------------------------------------+
    // | 0x00000000 PROTOCOL_RDP       | Standard RDP Security (section 5.3)          |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000001 PROTOCOL_SSL       | TLS 1.0, 1.1 or 1.2 (section 5.4.5.1)                    |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000002 PROTOCOL_HYBRID    | CredSSP (section 5.4.5.2)                    |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000004 PROTOCOL_RDSTLS    | RDSTLS protocol (section 5.4.5.3).           |
    // +-------------------------------+----------------------------------------------+
    // | 0x00000008 PROTOCOL_HYBRID_EX | Credential Security Support Provider protocol|
    // |                               | (CredSSP) (section 5.4.5.2) coupled with the |
    // |                               | Early User Authorization Result PDU (section |
    // |                               | 2.2.10.2). If this flag is set, then the     |
    // |                               | PROTOCOL_HYBRID (0x00000002) flag SHOULD     |
    // |                               | also be set. For more information on the     |
    // |                               | sequencing of the CredSSP messages and the   |
    // |                               | Early User Authorization Result PDU, see     |
    // |                               | sections 5.4.2.1 and 5.4.2.2.CredSSP         |
    // |                               | (section 5.4.5.2)                            |
    // +-------------------------------+----------------------------------------------+

    // 2.2.1.2.2 RDP Negotiation Failure (RDP_NEG_FAILURE)
    // ===================================================

    // The RDP Negotiation Failure structure is used by a server to inform the
    // client of a failure that has occurred while preparing security for the
    // connection.

    // type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
    // field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the packet
    // is a Negotiation Failure.

    // flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
    // are currently no defined flags so the field MUST be set to 0x00.

    // length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
    // field MUST be set to 0x0008 (8 bytes).

    // failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
    // failure code.

    // +--------------------------------------+------------------------------------+
    // | 0x00000001 SSL_REQUIRED_BY_SERVER    | The server requires that the       |
    // |                                      | client support Enhanced RDP        |
    // |                                      | Security (section 5.4) with either |
    // |                                      | TLS 1.0 (section 5.4.5.1) or       |
    // |                                      | CredSSP (section 5.4.5.2). If only |
    // |                                      | CredSSP was requested then the     |
    // |                                      | server only supports TLS.          |
    // +--------------------------------------+------------------------------------+
    // | 0x00000002 SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only   |
    // |                                      | use Standard RDP Security          |
    // |                                      | mechanisms (section 5.3) and does  |
    // |                                      | not support any External Security  |
    // |                                      | Protocols (section 5.4.5).         |
    // +--------------------------------------+------------------------------------+
    // | 0x00000003 SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid|
    // |                                      | authentication certificate and     |
    // |                                      | cannot initialize the External     |
    // |                                      | Security Protocol Provider         |
    // |                                      | (section 5.4.5).                   |
    // +--------------------------------------+------------------------------------+
    // | 0x00000004 INCONSISTENT_FLAGS        | The list of requested security     |
    // |                                      | protocols is not consistent with   |
    // |                                      | the current security protocol in   |
    // |                                      | effect. This error is only possible|
    // |                                      | when the Direct Approach (see      |
    // |                                      | sections 5.4.2.2 and 1.3.1.2) is   |
    // |                                      | used and an External Security      |
    // |                                      | Protocol (section 5.4.5) is already|
    // |                                      | being used.                        |
    // +--------------------------------------+------------------------------------+
    // | 0x00000005 HYBRID_REQUIRED_BY_SERVER | The server requires that the client|
    // |                                      | support Enhanced RDP Security      |
    // |                                      | (section 5.4) with CredSSP (section|
    // |                                      | 5.4.5.2).                          |
    // +--------------------------------------+------------------------------------+
    // | 0x00000006                            | Credential Security Support       |
    // | SSL_WITH_USER_AUTH_REQUIRED_BY_SERVER | The server requires that the      |
    // |                                       | client support Enhanced RDP       |
    // |                                       | Security (section 5.4) with TLS   |
    // |                                       | 1.0, 1.1 or 1.2 (section 5.4.5.1) |
    // |                                       | and certificate-based client      |
    // |                                       | authentication.<2>                |
    // +---------------------------------------+-----------------------------------+

    enum {
        CC_TPDU_EXTENDED_CLIENT_DATA_SUPPORTED    = 1,
        CC_TPDU_DYNVC_GFX_PROTOCOL_SUPPORTED      = 2,
        CC_TPDU_RDP_NEGRSP_RESERVED               = 4
    };

    struct CC_TPDU_Recv : public Recv
    {
        struct CC_Header
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint16_t src_ref;
            uint8_t class_option;
        } tpdu_hdr;

        size_t _header_size;

        uint8_t rdp_neg_type;
        uint8_t rdp_neg_flags;
        uint16_t rdp_neg_length;
        uint32_t rdp_neg_code; // selected_protocol or failure_code

        explicit CC_TPDU_Recv(InStream & stream, uint32_t verbose = 0)
            : Recv(stream)
            , tpdu_hdr([&]()
            {
                /* LI(1) + code(1) + dst_ref(2) + src_ref(2) + class_option(1) */
                if (!stream.in_check_rem(7)){
                    LOG(LOG_ERR, "Truncated TPDU header: expected=7 remains=%zu", stream.in_remain());
                    throw Error(ERR_X224);
                }
                uint8_t LI = stream.in_uint8();
                uint8_t code = stream.in_uint8();

                if (!(code == X224::CC_TPDU)){
                    LOG(LOG_ERR, "Unexpected TPDU opcode, expected CC_TPDU, got %u", code);
                    throw Error(ERR_X224);
                }

                uint16_t dst_ref = stream.in_uint16_le();
                uint16_t src_ref = stream.in_uint16_le();
                uint8_t class_option = stream.in_uint8();

                return CC_Header{LI, code, dst_ref, src_ref, class_option};
            }())
            , _header_size(X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1)
        {
            unsigned expected = X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI;

            if (stream.get_capacity() < expected){
                LOG(LOG_ERR, "Truncated CC TPDU header: expected %u, got %zu",
                    expected, stream.get_capacity());
                throw Error(ERR_X224);
            }

            // extended negotiation header
            this->rdp_neg_type = 0;
            this->rdp_neg_flags = 0;
            this->rdp_neg_length = 0;
            this->rdp_neg_code = 0;

            uint8_t const * end_of_header = stream.get_data() + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            /* rdp_neg_type(1) + rdp_neg_flags(1) + rdp_neg_length(2) + rdp_neg_code(4) */
            if (stream.in_remain() >= 8){
                this->rdp_neg_type = stream.in_uint8();

                if ((this->rdp_neg_type != X224::RDP_NEG_FAILURE)
                &&  (this->rdp_neg_type != X224::RDP_NEG_RSP)){
                    this->rdp_neg_flags = stream.in_uint8();
                    this->rdp_neg_length = stream.in_uint16_le();
                    this->rdp_neg_code = stream.in_uint32_le();
                    LOG(LOG_ERR, "X224:RDP_NEG_RSP or X224:RDP_NEG_FAILURE Expected, got LI=%u %x %x %x %x",
                        this->tpdu_hdr.LI,
                        this->rdp_neg_type,
                        this->rdp_neg_flags,
                        this->rdp_neg_length,
                        this->rdp_neg_code);
                    throw Error(ERR_X224);
                }

                if (verbose){
                    LOG(LOG_INFO, "Found RDP Negotiation %s Structure",
                        (this->rdp_neg_type == X224::RDP_NEG_RSP)?"Response":"Failure");
                }
                //NEGTYPE=2 NEGFLAGS=0 NEGLENGTH=8 NEGCODE=1
                this->rdp_neg_flags = stream.in_uint8();
                this->rdp_neg_length = stream.in_uint16_le();
                this->rdp_neg_code = stream.in_uint32_le();

                LOG(LOG_INFO, "NEG_RSP_TYPE=%u NEG_RSP_FLAGS=%u NEG_RSP_LENGTH=%u NEG_RSP_SELECTED_PROTOCOL=%u",
                    this->rdp_neg_type, this->rdp_neg_flags, this->rdp_neg_length, this->rdp_neg_code);

                switch (this->rdp_neg_type){
                case X224::RDP_NEG_RSP:
                    switch (this->rdp_neg_code){
                        case X224::PROTOCOL_RDP:
                            LOG(LOG_INFO, "CC Recv: PROTOCOL RDP");
                            break;
                        case X224::PROTOCOL_TLS:
                            LOG(LOG_INFO, "CC Recv: PROTOCOL TLS");
                            break;
                        case X224::PROTOCOL_HYBRID:
                            LOG(LOG_INFO, "CC Recv: PROTOCOL HYBRID");
                            break;
                        case X224::PROTOCOL_HYBRID_EX:
                            LOG(LOG_INFO, "CC Recv: PROTOCOL HYBRID EX");
                            break;
                        default:
                            LOG(LOG_INFO, "CC Recv: Unknown protocol code %u", this->rdp_neg_code);
                            break;
                    }
                    break;
                case X224::RDP_NEG_FAILURE:
                    switch (this->rdp_neg_code){
                        case X224::SSL_REQUIRED_BY_SERVER:
                            LOG(LOG_INFO, "SSL_REQUIRED_BY_SERVER");
                            break;
                        case X224::SSL_NOT_ALLOWED_BY_SERVER:
                            LOG(LOG_INFO, "SSL_NOT_ALLOWED_BY_SERVER");
                            break;
                        case X224::SSL_CERT_NOT_ON_SERVER:
                            LOG(LOG_INFO, "SSL_CERT_NOT_ON_SERVER");
                            break;
                        case X224::INCONSISTENT_FLAGS:
                            LOG(LOG_INFO, "INCONSISTENT_FLAGS");
                            break;
                        case X224::HYBRID_REQUIRED_BY_SERVER:
                            LOG(LOG_INFO, "HYBRID_REQUIRED_BY_SERVER");
                            break;
                        default:
                            LOG(LOG_INFO, "Unknown failure code %u", this->rdp_neg_code);
                            break;
                    }
                    break;
                default:
                    break;
                }
            }

            if (end_of_header != stream.get_current()){
                LOG(LOG_ERR, "CC TPDU header should be terminated, got trailing data %ld", end_of_header - stream.get_current());
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();

        }

        void throw_error() {
            if (this->rdp_neg_type == X224::RDP_NEG_FAILURE) {
                switch (this->rdp_neg_code){
                case X224::SSL_REQUIRED_BY_SERVER:
                    LOG(LOG_INFO, "SSL_REQUIRED_BY_SERVER");
                    throw Error(ERR_NEGO_SSL_REQUIRED_BY_SERVER);
                case X224::SSL_NOT_ALLOWED_BY_SERVER:
                    LOG(LOG_INFO, "SSL_NOT_ALLOWED_BY_SERVER");
                    throw Error(ERR_NEGO_SSL_NOT_ALLOWED_BY_SERVER);
                case X224::SSL_CERT_NOT_ON_SERVER:
                    LOG(LOG_INFO, "SSL_CERT_NOT_ON_SERVER");
                    throw Error(ERR_NEGO_SSL_CERT_NOT_ON_SERVER);
                case X224::INCONSISTENT_FLAGS:
                    LOG(LOG_INFO, "INCONSISTENT_FLAGS");
                    throw Error(ERR_NEGO_INCONSISTENT_FLAGS);
                case X224::HYBRID_REQUIRED_BY_SERVER:
                    LOG(LOG_INFO, "HYBRID_REQUIRED_BY_SERVER");
                    throw Error(ERR_NEGO_HYBRID_REQUIRED_BY_SERVER);
                default:
                    LOG(LOG_INFO, "Unknown failure code %u", this->rdp_neg_code);
                    break;
                }
            }
        }
    }; // END CLASS CC_TPDU_Recv


    struct CC_TPDU_Send
    {

        CC_TPDU_Send(OutStream & stream, uint8_t rdp_neg_type, uint8_t rdp_neg_flags, uint32_t rdp_neg_code)
        {
            stream.out_uint8(0x03); // version 3
            stream.out_uint8(0x00);
            stream.out_uint16_be(11 + (rdp_neg_type?8:0));
            stream.out_uint8(11 + (rdp_neg_type?8:0) - 5);

            stream.out_uint8(X224::CC_TPDU); // CC_TPDU code
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint16_be(0x0000); // SRC-REF
            stream.out_uint8(0x00); // CLASS OPTION

            if (rdp_neg_type){
                switch (rdp_neg_type){
                case X224::RDP_NEG_RSP:
                    switch (rdp_neg_code){
                    case X224::PROTOCOL_RDP:
                        LOG(LOG_INFO, "CC Send: PROTOCOL RDP");
                    break;
                    case X224::PROTOCOL_TLS:
                        LOG(LOG_INFO, "CC Send: PROTOCOL TLS");
                    break;
                    case X224::PROTOCOL_HYBRID:
                        LOG(LOG_INFO, "CC Send: PROTOCOL HYBRID");
                    break;
                    case X224::PROTOCOL_HYBRID_EX:
                        LOG(LOG_INFO, "CC Send: PROTOCOL HYBRID EX");
                    break;
                    default:
                        LOG(LOG_INFO, "CC Send: Unknown protocol code %u", rdp_neg_code);
                    break;
                    }
                break;
                case X224::RDP_NEG_FAILURE:
                    switch (rdp_neg_code){
                    case X224::SSL_REQUIRED_BY_SERVER:
                        LOG(LOG_INFO, "SSL_REQUIRED_BY_SERVER");
                    break;
                    case X224::SSL_NOT_ALLOWED_BY_SERVER:
                        LOG(LOG_INFO, "SSL_NOT_ALLOWED_BY_SERVER");
                    break;
                    case X224::SSL_CERT_NOT_ON_SERVER:
                        LOG(LOG_INFO, "SSL_CERT_NOT_ON_SERVER");
                    break;
                    case X224::INCONSISTENT_FLAGS:
                        LOG(LOG_INFO, "INCONSISTENT_FLAGS");
                    break;
                    case X224::HYBRID_REQUIRED_BY_SERVER:
                        LOG(LOG_INFO, "HYBRID_REQUIRED_BY_SERVER");
                        break;
                    default:
                        LOG(LOG_INFO, "Unknown failure code %u", rdp_neg_code);
                        break;
                    }
                break;
                default:
                    LOG(LOG_INFO, "Unknown negociation response code %u", rdp_neg_code);
                break;
                }

                stream.out_uint8(rdp_neg_type);
                stream.out_uint8(rdp_neg_flags);
                stream.out_uint16_le(8);
                stream.out_uint32_le(rdp_neg_code);
            }

        }
    };


    //    Class 0 x224 TPDU
    //    -----------------

    //                        +----+-----+---------+---------+---------------------------+
    //                        | LI |     | DST-REF | SRC-REF | REASON                    |
    //               +--------+----+-----+---------+---------+---------------------------+
    //               | OFFSET | 4  |  5  |  6   7  |  8   9  |    10                     |
    //    +-------------------+----+-----+---------+---------+---------------------------+
    //    | Disconnect Request|    |     |         |         | 00 = NOT SPECIFIED        |
    //    | DR_TPDU 1000 0000 | 06 |  80 |  00  00 |  00  00 | 01 = CONGESTION           |
    //    |                   |    |     |         |         | 02 = SESSION NOT ATTACHED |
    //    |                   |    |     |         |         | 03 = ADDRESS UNKNOWN      |
    //    +-------------------+----+-----+---------+---------+---------------------------+

    struct DR_TPDU_Recv : public Recv
    {
        struct DR_Header
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint16_t src_ref;
            uint8_t reason;
        } tpdu_hdr;

        size_t _header_size;

        explicit DR_TPDU_Recv(InStream & stream)
            : Recv(stream)
            , tpdu_hdr([&]()
            {
                /* LI(1) + code(1) + dst_ref(2) + src_ref(2) + reason(1) */
                if (!stream.in_check_rem(7)){
                    LOG(LOG_ERR, "Truncated TPDU header: expected=7 remains=%zu", stream.in_remain());
                    throw Error(ERR_X224);
                }

                uint8_t LI = stream.in_uint8();
                uint8_t code = stream.in_uint8();

                if (!(code == X224::DR_TPDU)){
                    LOG(LOG_ERR, "Unexpected TPDU opcode, expected DR_TPDU, got %u", code);
                    throw Error(ERR_X224);
                }

                uint16_t dst_ref = stream.in_uint16_le();
                uint16_t src_ref = stream.in_uint16_le();
                uint8_t reason = stream.in_uint8();
                return DR_Header{LI, code, dst_ref, src_ref, reason};
            }())
            , _header_size(X224::TPKT_HEADER_LEN + 1 + this->tpdu_hdr.LI)
        {
            uint8_t const * end_of_header = stream.get_data() + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            if (end_of_header != stream.get_current()){
                LOG(LOG_ERR, "DR TPDU header should be terminated, got trailing data %ld", end_of_header - stream.get_current());
                throw Error(ERR_X224);
            }
        }
    }; // END CLASS DR_TPDU_Recv


    struct DR_TPDU_Send
    {
        DR_TPDU_Send(OutStream & stream, uint8_t reason)
        {
            stream.out_uint8(0x03); // version 3
            stream.out_uint8(0x00);
            stream.out_uint16_be(11); // packet len, inclusive
            stream.out_uint8(11 - 5); // LI

            stream.out_uint8(X224::DR_TPDU);
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint16_be(0x0000); // SRC-REF
            stream.out_uint8(reason);
        }
    };


    //    Class 0 x224 TPDU
    //    -----------------

    //                                                                             +----------------------------------------------------+
    //                                                                             |           Variable Part                            |
    //                        +----+-----+---------+-------------------------------+-----------+-----+----------------------------------+
    //                        |    |     |         |                               | Invalid   |     | Rejected TPDU Header up to octet |
    //                        | LI |     | DST-REF |         REJECT CAUSE          |  TPDU     | VL  | which caused rejection           |
    //                        |    |     |         |                               |           |     | (mandatory in class 0)           |
    //               +--------+----+-----+---------+-------------------------------+-----------+-----+----------------------------------+
    //               | OFFSET | 4  |  5  |  6   7  |                8              |    9      |  10 | 11 ...                           |
    //    +-------------------+----+-----+---------+---------+---------------------+-----------+-----+----------------------------------+
    //    | TPDU Error        |    |     |         |  00 = Reason not specified    |           |     |                                  |
    //    | ER_TPDU 0111 0000 | 04 |  70 |  00  00 |  01 = Invalid parameter code  |    C1     |  v  | ?? ?? ?? ?? ??                   |
    //    |                   |+2+v|     |         |  02 = Invalid TPDU type       | 1100 0001 |     | ~~~~~~~~~~~~~~                   |
    //    |                   |    |     |         |  03 = Invalid Parameter Value |           |     |    v bytes                       |
    //    +-------------------+----+-----+---------+-------------------------------+-----------+-----+----------------------------------+

    struct ER_TPDU_Recv  : public Recv
    {
        struct ER_Header
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint8_t reject_cause;
            uint8_t invalid_tpdu_var;
            uint8_t invalid_tpdu_vl;
            uint8_t invalid[256];

            explicit ER_Header(InStream & stream)
            {
                /* LI(1) + code(1) + dst_ref(2) + reject_cause(1) */
                if (!stream.in_check_rem(5)){
                    LOG(LOG_ERR, "Truncated TPDU header: expected=5 remains=%zu", stream.in_remain());
                    throw Error(ERR_X224);
                }

                // TPDU
                this->LI = stream.in_uint8();
                this->code = stream.in_uint8();

                if (!(this->code == X224::ER_TPDU)){
                    LOG(LOG_ERR, "Unexpected TPDU opcode, expected ER_TPDU, got %u", this->code);
                    throw Error(ERR_X224);
                }

                this->dst_ref = stream.in_uint16_le();
                this->reject_cause = stream.in_uint8();

                unsigned expected = X224::TPKT_HEADER_LEN + this->LI;
                if (stream.get_capacity() < expected){
                    LOG(LOG_ERR, "Truncated ER TPDU header: expected %u, got %zu",
                        expected, stream.get_capacity());
                    throw Error(ERR_X224);
                }

                uint8_t const * end_of_header = stream.get_data() + X224::TPKT_HEADER_LEN + this->LI + 1;
                if (end_of_header - stream.get_current() >= 2){
                    this->invalid_tpdu_var = stream.in_uint8();
                    if (this->invalid_tpdu_var != 0xC1){
                        LOG(LOG_ERR, "Unexpected ER TPDU, variable code, expected C1 (invalid TPDU details), got %x",
                            this->invalid_tpdu_var);
                        throw Error(ERR_X224);
                    }
                    this->invalid_tpdu_vl = stream.in_uint8();
                    if (this->invalid_tpdu_vl > this->LI - 6){
                        LOG(LOG_ERR, "Invalid TPDU details too large, max=%d got %x",
                            this->LI - 6, this->invalid_tpdu_vl);
                        throw Error(ERR_X224);
                    }

                    if (!stream.in_check_rem(this->invalid_tpdu_vl)){
                        LOG(LOG_ERR, "Truncated ER TPDU invalid: expected %u, got %zu",
                            this->invalid_tpdu_vl, stream.get_capacity());
                        throw Error(ERR_X224);
                    }

                    stream.in_copy_bytes(this->invalid, this->invalid_tpdu_vl);
                    if (this->LI - 6 - this->invalid_tpdu_vl != 0){
                        LOG(LOG_ERR, "Trailing variable data in ER_TPDU, %d bytes",
                            this->LI - 6 - this->invalid_tpdu_vl);
                        throw Error(ERR_X224);
                    }

                }
                const size_t _header_size = X224::TPKT_HEADER_LEN + 1 + this->LI;
                if (_header_size != stream.get_offset()){
                    LOG(LOG_ERR, "ER TPDU header should be terminated, got trailing data %zu",
                        stream.get_offset() - _header_size);
                    throw Error(ERR_X224);
                }
            }
        } tpdu_hdr;

        size_t _header_size;

        explicit ER_TPDU_Recv(InStream & stream)
            : Recv(stream)
            , tpdu_hdr(stream)
            , _header_size(X224::TPKT_HEADER_LEN + 1 + this->tpdu_hdr.LI)
        {
        }
    }; // END CLASS ER_TPDU_Recv

    struct ER_TPDU_Send
    {
        ER_TPDU_Send(OutStream & stream, uint8_t cause, uint8_t vl, uint8_t * invalid)
        {
            stream.out_uint8(0x03); // version 3
            stream.out_uint8(0x00);
            stream.out_uint16_be(11 + vl); // packet len, inclusive
            stream.out_uint8(11 + vl - 5); // LI

            stream.out_uint8(X224::ER_TPDU);
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint8(cause);

            stream.out_uint8(0xC1);
            stream.out_uint8(vl);
            stream.out_copy_bytes(invalid, vl);
        }
    };


    //    Class 0 x224 TPDU
    //    -----------------

    //                        +----+-----+----------------+
    //                        | LI |     | EOT            |
    //               +--------+----+-----+----------------+
    //               | OFFSET | 4  |  5  |  6             |
    //    +----------+--------+----+-----+----------------+
    //    | Data              |    |     | 80 = EOT       |
    //    | DT_TPDU 1111 0000 | 02 |  F0 | 00 = MORE DATA |
    //    +-------------------+----+-----+----------------+

    struct DT_TPDU_Recv : public Recv
    {
        struct DT_Header
        {
            uint8_t LI;
            uint8_t code;
            uint8_t eot;
        } tpdu_hdr;

        size_t _header_size;
        InStream payload;

        explicit DT_TPDU_Recv(InStream & stream)
        : Recv(stream)
        , tpdu_hdr([&](){
            if (!stream.in_check_rem(3)){
                LOG(LOG_ERR, "Truncated TPDU header: expected=3 remains=%zu", stream.in_remain());
                throw Error(ERR_X224);
            }

            uint8_t LI = stream.in_uint8();
            uint8_t  code = stream.in_uint8();
            if (!(code == X224::DT_TPDU)){
                LOG(LOG_ERR, "Unexpected TPDU opcode, expected DT_TPDU, got %u", code);
                throw Error(ERR_X224);
            }

            uint8_t eot = stream.in_uint8();
            if (eot != EOT_EOT){
                LOG(LOG_ERR, "DT TPDU should say EOT, got=%x", eot);
                throw Error(ERR_X224);
            }
            return DT_Header{LI, code, eot};
        }())
        , _header_size([&](){
            const size_t header_size = X224::TPKT_HEADER_LEN + 1 + this->tpdu_hdr.LI;
            if (header_size != stream.get_offset()){
                LOG(LOG_ERR, "DT TPDU header should be terminated, got trailing data %zu",
                    stream.get_offset() - header_size);
                throw Error(ERR_X224);
            }
            return header_size;
        }())
        , payload([&](){
            size_t payload_size = stream.get_capacity() - this->_header_size;
            if (!stream.in_check_rem(payload_size)){
                LOG(LOG_ERR, "Truncated DT TPDU data: expected=%zu remains=%zu", payload_size, stream.in_remain());
                throw Error(ERR_X224);
            }
            return InStream(stream.get_data() + this->_header_size, payload_size);
        }())
        // ========= Constructor's body ============
        {
            stream.in_skip_bytes(stream.get_capacity() - this->_header_size);
        }
    }; // END CLASS DT_TPDU_Recv

    struct DT_TPDU_Send
    {
        DT_TPDU_Send(OutStream & stream, size_t payload_len)
        {
            stream.out_uint8(0x03); // version 3
            stream.out_uint8(0x00);
            stream.out_uint16_be(7 + payload_len); // packet len, inclusive
            stream.out_uint8(7 - 5); // LI

            stream.out_uint8(X224::DT_TPDU);
            stream.out_uint8(X224::EOT_EOT);
        }
    };


    struct write_x224_dt_tpdu_fn
    {
        void operator()(StreamSize<7> /*unused*/ /*unused*/, OutStream & x224_header, std::size_t sz) const {
            X224::DT_TPDU_Send(x224_header, sz);
        }
    };
} // end namespace X224

