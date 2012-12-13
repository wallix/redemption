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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   X224 class 0 packets management, complies with
   RFC 1006, ISO transport services on top of the TCP

*/

#if !defined(__CORE_RDP_X224_HPP__)
#define __CORE_RDP_X224_HPP__

#include <stdint.h>
#include "transport.hpp"
#include "stream.hpp"
#include "log.hpp"
#include "error.hpp"

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

    enum {
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
        REASON_ADDRESS_UNKNOWN      = 3,
    };

    // Error codes in ER_TPDU
    enum {
        // REASON_NOT_SPECIFIED         = 0,
        REASON_INVALID_PARAMETER_CODE   = 1,
        REASON_INVALID_TPDU_TYPE        = 2,
        REASON_INVALID_PARAMETER_VALUE  = 3,
    };

    enum {
        EOT_MORE_DATA       = 0x00,
        EOT_EOT             = 0x80
    };

    enum {
        RDP_NEG_NONE    = 0,
        RDP_NEG_REQ     = 1,
        RDP_NEG_RESP    = 2,
        RDP_NEG_FAILURE = 3
    };

    enum {
        RDP_NEG_PROTOCOL_RDP    = 0,
        RDP_NEG_PROTOCOL_TLS    = 1,
        RDP_NEG_PROTOCOL_HYBRID = 2,
    };

    enum {
        SSL_REQUIRED_BY_SERVER    = 0x00000001,
        SSL_NOT_ALLOWED_BY_SERVER = 0x00000002,
        SSL_CERT_NOT_ON_SERVER    = 0x00000003,
        INCONSISTENT_FLAGS        = 0x00000004,
        HYBRID_REQUIRED_BY_SERVER = 0x00000005,
    };

    // Factory just read enough data to know the type of packet we are dealing with
    TODO("This could be TPKT transport layer, hence X224 could follow the same profile as others layers functions");
    struct RecvFactory
    {
        int type;
        size_t length;

        RecvFactory(Transport & t, Stream & stream)
        {
            t.recv((char**)(&(stream.end)), X224::TPKT_HEADER_LEN);
            uint8_t tpkt_version = stream.in_uint8();
            if (tpkt_version != 3) {
                LOG(LOG_ERR, "Tpkt type 3 slow-path PDU expected (version = %u)", tpkt_version);
                throw Error(ERR_X224);
            }
            stream.in_skip_bytes(1);
            uint16_t tpkt_len = stream.in_uint16_be();
            t.recv((char**)(&(stream.end)), 2);
            if (tpkt_len < 6){
                LOG(LOG_ERR, "Bad X224 header, length too short (length = %u)", tpkt_len);
                throw Error(ERR_X224);
            }
            this->length = tpkt_len;
            stream.in_skip_bytes(1);
            uint8_t tpdu_type = stream.in_uint8();
            switch (tpdu_type & 0xF0){
            case X224::CR_TPDU: // Connection Request 1110 xxxx
            case X224::CC_TPDU: // Connection Confirm 1101 xxxx
            case X224::DR_TPDU: // Disconnect Request 1000 0000
            case X224::DT_TPDU: // Data               1111 0000 (no ROA = No Ack)
            case X224::ER_TPDU:  // TPDU Error        0111 0000
                this->type = tpdu_type & 0xF0;
            break;
            default:
                this->type = 0;
                LOG(LOG_ERR, "Bad X224 header, unknown TPDU type (code = %u)", tpdu_type);
                throw Error(ERR_X224);
            break;
            }
        }
    };

    // ################################### COMMON CODE #################################
    struct Recv
    {
        size_t _header_size;

        struct Tpkt
        {
            uint8_t version;
            uint16_t len;
        } tpkt;

        Recv(Transport & t, Stream & stream) : _header_size(4)
        {
            uint16_t length = stream.size();
            if (length < 4){
                t.recv((char**)(&(stream.end)), 4 - length);
            }
            stream.p = stream.data;

            // TPKT
            this->tpkt.version = stream.in_uint8();
            stream.in_skip_bytes(1);
            this->tpkt.len = stream.in_uint16_be();
            if (stream.size() < this->tpkt.len){
                t.recv((char**)(&(stream.end)), this->tpkt.len - stream.size());
            }
        }
    };

    struct Send
    {
        enum {
            OFFSET_TPKT_LEN = 2,
            OFFSET_LI       = 4
        };

        void header(Stream & stream) {
            stream.out_uint8(0x03); // version 3
            stream.out_uint8(0x00);
            stream.out_skip_bytes(3);
        }

        void trailer(Stream & stream) {
            stream.set_out_uint16_be(stream.p - stream.data, OFFSET_TPKT_LEN);
            stream.set_out_uint8(stream.p - stream.data - 5, OFFSET_LI);
            stream.mark_end();
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
    // structure. The length of this negotiation structure is included in the X.224
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

    // +----------------------------+----------------------------------------------+
    // | 0x00000000 PROTOCOL_RDP    | Standard RDP Security (section 5.3).         |
    // +----------------------------+----------------------------------------------+
    // | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1).                   |
    // +----------------------------+----------------------------------------------+
    // | 0x00000002 PROTOCOL_HYBRID | Credential Security Support Provider protocol|
    // |                            | (CredSSP) (section 5.4.5.2). If this flag is |
    // |                            | set, then the PROTOCOL_SSL (0x00000001)      |
    // |                            | SHOULD also be set because Transport Layer   |
    // |                            | Security (TLS) is a subset of CredSSP.       |
    // +----------------------------+----------------------------------------------+


    struct CR_TPDU_Recv : public Recv
    {
        struct TPDUHeader
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint16_t src_ref;
            uint8_t class_option;
        } tpdu_hdr;

        size_t cookie_len;
        char cookie[1024];

        uint8_t rdp_neg_type;
        uint8_t rdp_neg_flags;
        uint16_t rdp_neg_length;
        uint32_t rdp_neg_code;

        CR_TPDU_Recv(Transport & t, Stream & stream, uint32_t verbose = 0)
        : Recv(t, stream)
        {
            this->tpdu_hdr.LI = stream.in_uint8();
            this->tpdu_hdr.code = stream.in_uint8();

            if (!this->tpdu_hdr.code == X224::CR_TPDU){
                LOG(LOG_ERR, "Unexpected TPDU opcode, expected CR_TPDU, got %u",
                    this->tpdu_hdr.code);
                throw Error(ERR_X224);
            }

            this->tpdu_hdr.dst_ref = stream.in_uint16_le();
            this->tpdu_hdr.src_ref = stream.in_uint16_le();
            this->tpdu_hdr.class_option = stream.in_uint8();

            // extended negotiation header
            this->cookie_len = 0;
            this->cookie[0] = 0;
            this->rdp_neg_type = 0;

            uint8_t * end_of_header = stream.data + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            for (uint8_t * p = stream.p + 1; p < end_of_header ; p++){
                if (p[-1] == 0x0D &&  p[0]  == 0x0A){
                    this->cookie_len = p - (stream.data + 11) + 1;
                    if (cookie_len > 1023){
                        LOG(LOG_ERR, "Bad Connection Request X224 header, cookie too large (length = %u)", cookie_len);
                        throw Error(ERR_X224);
                    }
                    memcpy(this->cookie, stream.data + 11, this->cookie_len);
                    this->cookie[this->cookie_len] = 0;
                    if (verbose){
                        LOG(LOG_INFO, "cookie: %s", this->cookie);
                    }
                    break;
                }
            }
            stream.p += this->cookie_len;

            if (end_of_header - stream.p >= 8){
                if (verbose){
                    LOG(LOG_INFO, "Found RDP Negotiation Request Structure");
                }
                this->rdp_neg_type = stream.in_uint8();
                this->rdp_neg_flags = stream.in_uint8();
                this->rdp_neg_length = stream.in_uint16_le();
                this->rdp_neg_code = stream.in_uint32_le();

                if (this->rdp_neg_type != X224::RDP_NEG_REQ){
                    LOG(LOG_INFO, "X224:RDP_NEG_REQ Expected LI=%u %x %x %x %x",
                        this->tpdu_hdr.LI, this->rdp_neg_type, this->rdp_neg_flags, this->rdp_neg_length, this->rdp_neg_code);
                    throw Error(ERR_X224);
                }

                switch (this->rdp_neg_code){
                    case X224::RDP_NEG_PROTOCOL_RDP:
                        LOG(LOG_INFO, "PROTOCOL RDP");
                        break;
                    case X224::RDP_NEG_PROTOCOL_TLS:
                        LOG(LOG_INFO, "PROTOCOL TLS 1.0");
                        break;
                    case X224::RDP_NEG_PROTOCOL_HYBRID:
                        LOG(LOG_INFO, "PROTOCOL HYBRID");
                        break;
                }
            }

            if (end_of_header != stream.p){
                LOG(LOG_ERR, "CR TPDU header should be terminated, got trailing data %u", end_of_header - stream.p);
                hexdump_c(stream.data, stream.size());
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();
        }
    }; // END CLASS CR_TPDU_Recv


    struct CR_TPDU_Send : public Send
    {
        CR_TPDU_Send(Stream & stream, const char * cookie, uint8_t rdp_neg_type, uint8_t rdp_neg_flags, uint32_t rdp_neg_code)
        {
            this->header(stream);

            stream.out_uint8(X224::CR_TPDU); // CR_TPDU code
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint16_be(0x0000); // SRC-REF
            stream.out_uint8(0x00); // CLASS OPTION

            size_t cookie_len = strlen(cookie);
            if (cookie_len){
                stream.out_copy_bytes(cookie, cookie_len);
            }
            if (rdp_neg_type){
                stream.out_uint8(rdp_neg_type);
                stream.out_uint8(rdp_neg_flags);
                stream.out_uint16_le(8);
                stream.out_uint32_le(rdp_neg_code);
            }

            this->trailer(stream);

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
    // structure. The length of the negotiation structure is included in the X.224
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

    // +-------------------------------------+-------------------------------------+
    // | 0x01 EXTENDED_CLIENT_DATA_SUPPORTED | The server supports Extended Client |
    // |                                     | Data Blocks in the GCC Conference   |
    // |                                     | Create Request user data (section   |
    // |                                     | 2.2.1.3).                           |
    // +-------------------------------------+-------------------------------------+

    // length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This field MUST be set to 0x0008 (8 bytes)

    // selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the selected security protocol.

    // +----------------------------+----------------------------------------------+
    // | 0x00000000 PROTOCOL_RDP    | Standard RDP Security (section 5.3)          |
    // +----------------------------+----------------------------------------------+
    // | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
    // +----------------------------+----------------------------------------------+
    // | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
    // +----------------------------+----------------------------------------------+

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


    struct CC_TPDU_Recv : public Recv
    {
        struct TPDUHeader
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint16_t src_ref;
            uint8_t class_option;
        } tpdu_hdr;

        uint8_t rdp_neg_type;
        uint8_t rdp_neg_flags;
        uint16_t rdp_neg_length;
        uint32_t rdp_neg_code; // selected_protocol or failure_code

        CC_TPDU_Recv(Transport & t, Stream & stream, uint32_t verbose = 0)
        : Recv(t, stream)
        {
            this->tpdu_hdr.LI = stream.in_uint8();
            this->tpdu_hdr.code = stream.in_uint8();

            if (!this->tpdu_hdr.code == X224::CC_TPDU){
                LOG(LOG_ERR, "Unexpected TPDU opcode, expected CC_TPDU, got %u",
                    this->tpdu_hdr.code);
                throw Error(ERR_X224);
            }

            this->tpdu_hdr.dst_ref = stream.in_uint16_le();
            this->tpdu_hdr.src_ref = stream.in_uint16_le();
            this->tpdu_hdr.class_option = stream.in_uint8();

            // extended negotiation header
            this->rdp_neg_type = 0;
            this->rdp_neg_flags = 0;
            this->rdp_neg_length = 0;
            this->rdp_neg_code = 0;

            uint8_t * end_of_header = stream.data + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            if (stream.end - stream.p >= 8){
                this->rdp_neg_type = stream.in_uint8();

                if ((this->rdp_neg_type != X224::RDP_NEG_FAILURE)
                &&  (this->rdp_neg_type != X224::RDP_NEG_RESP)){
                    this->rdp_neg_flags = stream.in_uint8();
                    this->rdp_neg_length = stream.in_uint16_le();
                    this->rdp_neg_code = stream.in_uint32_le();
                    LOG(LOG_ERR, "X224:RDP_NEG_RESP or X224:RDP_NEG_FAILURE Expected, got LI=%u %x %x %x %x",
                        this->tpdu_hdr.LI,
                        this->rdp_neg_type,
                        this->rdp_neg_flags,
                        this->rdp_neg_length,
                        this->rdp_neg_code);
                    throw Error(ERR_X224);
                }

                if (verbose){
                    LOG(LOG_INFO, "Found RDP Negotiation %s Structure",
                        (this->rdp_neg_type == X224::RDP_NEG_RESP)?"Response":"Failure");
                }

                this->rdp_neg_flags = stream.in_uint8();
                this->rdp_neg_length = stream.in_uint16_le();
                this->rdp_neg_code = stream.in_uint32_le();

                switch (this->rdp_neg_type){
                case X224::RDP_NEG_RESP:
                    switch (this->rdp_neg_code){
                        case X224::RDP_NEG_PROTOCOL_RDP:
                            LOG(LOG_INFO, "PROTOCOL RDP");
                            break;
                        case X224::RDP_NEG_PROTOCOL_TLS:
                            LOG(LOG_INFO, "PROTOCOL TLS 1.0");
                            break;
                        case X224::RDP_NEG_PROTOCOL_HYBRID:
                            LOG(LOG_INFO, "PROTOCOL HYBRID");
                            break;
                        default:
                            LOG(LOG_INFO, "Unknown protocol code %u", this->rdp_neg_code);
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
            if (end_of_header != stream.p){
                LOG(LOG_ERR, "CC TPDU header should be tertminated, got trailing data %u", end_of_header - stream.p);
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();
        }
    }; // END CLASS CC_TPDU_Recv

    struct CC_TPDU_Send : public Send
    {
        CC_TPDU_Send(Stream & stream, uint8_t rdp_neg_type, uint8_t rdp_neg_flags, uint32_t rdp_neg_code)
        {
            this->header(stream);

            stream.out_uint8(X224::CC_TPDU); // CC_TPDU code
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint16_be(0x0000); // SRC-REF
            stream.out_uint8(0x00); // CLASS OPTION

            if (rdp_neg_type){
                stream.out_uint8(rdp_neg_type);
                stream.out_uint8(rdp_neg_flags);
                stream.out_uint16_le(8);
                stream.out_uint32_le(rdp_neg_code);
            }

            this->trailer(stream);
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
        struct TPDUHeader
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint16_t src_ref;
            uint8_t reason;
        } tpdu_hdr;

        // CONSTRUCTOR
        //==============================================================================
        DR_TPDU_Recv(Transport & t, Stream & stream)
        //==============================================================================
        : Recv(t, stream)
        {
            this->tpdu_hdr.LI = stream.in_uint8();
            this->tpdu_hdr.code = stream.in_uint8();

            if (!this->tpdu_hdr.code == X224::DR_TPDU){
                LOG(LOG_ERR, "Unexpected TPDU opcode, expected DR_TPDU, got %u",
                    this->tpdu_hdr.code);
                throw Error(ERR_X224);
            }

            this->tpdu_hdr.dst_ref = stream.in_uint16_le();
            this->tpdu_hdr.src_ref = stream.in_uint16_le();
            this->tpdu_hdr.reason = stream.in_uint8();

            uint8_t * end_of_header = stream.data + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            if (end_of_header != stream.p){
                LOG(LOG_ERR, "DR TPDU header should be tertminated, got trailing data %u", end_of_header - stream.p);
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();
        }
    }; // END CLASS DR_TPDU_Recv


    struct DR_TPDU_Send : public Send
    {
         DR_TPDU_Send( Stream & stream, uint8_t reason)
        {
            this->header(stream);

            stream.out_uint8(X224::DR_TPDU);
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint16_be(0x0000); // SRC-REF
            stream.out_uint8(reason);

            this->trailer(stream);
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
        struct TPDUHeader
        {
            uint8_t LI;
            uint8_t code;

            uint16_t dst_ref;
            uint8_t reject_cause;
            uint8_t invalid_tpdu_var;
            uint8_t invalid_tpdu_vl;
            uint8_t invalid[256];
        } tpdu_hdr;

        ER_TPDU_Recv(Transport & t, Stream & stream)
        : Recv(t, stream)
        {
            // TPDU
            this->tpdu_hdr.LI = stream.in_uint8();
            this->tpdu_hdr.code = stream.in_uint8();

            if (!this->tpdu_hdr.code == X224::ER_TPDU){
                LOG(LOG_ERR, "Unexpected TPDU opcode, expected ER_TPDU, got %u",
                    this->tpdu_hdr.code);
                throw Error(ERR_X224);
            }

            this->tpdu_hdr.dst_ref = stream.in_uint16_le();
            this->tpdu_hdr.reject_cause = stream.in_uint8();

            uint8_t * end_of_header = stream.data + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            if (end_of_header - stream.p >= 2){
                this->tpdu_hdr.invalid_tpdu_var = stream.in_uint8();
                if (this->tpdu_hdr.invalid_tpdu_var != 0xC1){
                    LOG(LOG_ERR, "Unexpected ER TPDU, variable code, expected C1 (invalid TPDU details), got %x",
                        this->tpdu_hdr.invalid_tpdu_var);
                    throw Error(ERR_X224);
                }
                this->tpdu_hdr.invalid_tpdu_vl = stream.in_uint8();
                if (this->tpdu_hdr.invalid_tpdu_vl > this->tpdu_hdr.LI - 6){
                    LOG(LOG_ERR, "Invalid TPDU details too large, max=%u got %x",
                        this->tpdu_hdr.LI - 6, this->tpdu_hdr.invalid_tpdu_vl);
                    throw Error(ERR_X224);
                }
                stream.in_copy_bytes(this->tpdu_hdr.invalid, this->tpdu_hdr.invalid_tpdu_vl);
                if (this->tpdu_hdr.LI - 6 - this->tpdu_hdr.invalid_tpdu_vl != 0){
                    LOG(LOG_ERR, "Trailing variable data in ER_TPDU, %u bytes",
                        this->tpdu_hdr.LI - 6 - this->tpdu_hdr.invalid_tpdu_vl);
                    throw Error(ERR_X224);
                }

            }
            if (end_of_header != stream.p){
                LOG(LOG_ERR, "ER TPDU header should be terminated, got trailing data %u", end_of_header - stream.p);
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();
        }
    }; // END CLASS ER_TPDU_Recv

    struct ER_TPDU_Send : public Send
    {
        ER_TPDU_Send(Stream & stream, uint8_t cause, uint8_t vl, uint8_t * invalid)
        {
            this->header(stream);

            stream.out_uint8(X224::ER_TPDU);
            stream.out_uint16_be(0x0000); // DST-REF
            stream.out_uint8(cause);

            stream.out_uint8(0xC1);
            stream.out_uint8(vl);
            stream.out_copy_bytes(invalid, vl);

            this->trailer(stream);
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
        size_t payload_size;
        SubStream payload;

        struct TPDUHeader
        {
            uint8_t LI;
            uint8_t code;
            uint8_t eot;
        } tpdu_hdr;

        enum {
            EOT_MORE_DATA        = 0x00,
            EOT_EOT             = 0x80
        };

        DT_TPDU_Recv(Transport & t, Stream & stream)
        : Recv(t, stream)
        , payload(stream, 0)
        {
            this->tpdu_hdr.LI = stream.in_uint8();

            this->tpdu_hdr.code = stream.in_uint8();
            if (!this->tpdu_hdr.code == X224::DT_TPDU){
                LOG(LOG_ERR, "Unexpected TPDU opcode, expected DT_TPDU, got %u",
                    this->tpdu_hdr.code);
                throw Error(ERR_X224);
            }

            this->tpdu_hdr.eot = stream.in_uint8();
            if (this->tpdu_hdr.eot != EOT_EOT){
                LOG(LOG_ERR, "DT TPDU should say EOT, got=%x", this->tpdu_hdr.eot);
                throw Error(ERR_X224);
            }

            uint8_t * end_of_header = stream.data + X224::TPKT_HEADER_LEN + this->tpdu_hdr.LI + 1;
            if (end_of_header != stream.p){
                LOG(LOG_ERR, "DT TPDU header should be tertminated, got trailing data %u", end_of_header - stream.p);
                throw Error(ERR_X224);
            }
            this->_header_size = stream.get_offset();
            this->payload_size = stream.size() - this->_header_size;

            TODO("Factorize this")
            this->payload.data = this->payload.p = stream.p;
            this->payload.capacity = this->payload_size;
            this->payload.end = this->payload.data + this->payload_size;
        }
    }; // END CLASS DT_TPDU_Recv

    struct DT_TPDU_Send : public Send
    {
        DT_TPDU_Send(Stream & stream, size_t payload_len)
        {
            this->header(stream);

            stream.out_uint8(X224::DT_TPDU);
            stream.out_uint8(X224::EOT_EOT);

            stream.set_out_uint16_be(7 + payload_len, Send::OFFSET_TPKT_LEN);
            stream.set_out_uint8(2, Send::OFFSET_LI);
            stream.mark_end();
        }
    };

}; // end namespace X224

#endif
