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

#if !defined(__X224_HPP__)
#define __X224_HPP__

#include <stdint.h>
#include "transport.hpp"
#include "stream.hpp"
#include "log.hpp"
#include "error.hpp"


struct X224Packet
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
    // parameters, but excluding the length indicator field and user data, if
    // any. The value 255 (1111 1111) is reserved for possible extensions.
    // If the length indicated exceeds or is equal to the size of the NS-user
    // data which is present, this is a protocol error.

    // 13.7.3 Fixed part
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

    // 13.7.4
    // Variable part
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

    // 13.7.5
    // User data field
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

// 2.2.1.2   Server X.224 Connection Confirm PDU
// =============================================

//  The X.224 Connection Confirm PDU is an RDP Connection Sequence PDU sent from
//  server to client during the Connection Initiation phase (see section
//  1.3.1.1). It is sent as a response to the X.224 Connection Request PDU
//  (section 2.2.1.1).

//tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//x224Ccf (7 bytes): An X.224 Class 0 Connection Confirm TPDU, as specified in [X224] section
//   13.4.

//rdpNegData (8 bytes): Optional RDP Negotiation Response (section 2.2.1.2.1) structure or an
//   optional RDP Negotiation Failure (section 2.2.1.2.2) structure. The length of the negotiation
//   structure is included in the X.224 Connection Confirm Length Indicator field.


    enum {
        TPKT_HEADER_LEN = 4
    };

};

struct X224In : public X224Packet
{
    struct Tpkt {
        Tpkt(uint8_t version, uint16_t len) : version(version), len(len) {}
        uint8_t version;
        uint16_t len;
    } tpkt;

    struct TPDUHeader {
        TPDUHeader(uint8_t LI, uint8_t code)
            : LI(LI), code(code), reason(0), eot(0), reject_cause(0) {}
        uint8_t LI;
        uint8_t code;
        uint8_t reason;
        uint8_t eot;
        uint8_t reject_cause;
    } tpdu_hdr;

    Stream & stream;


    X224In(Transport * t, Stream & stream)
        : tpkt(0,0), tpdu_hdr(0, 0), stream(stream)
    // Receive a X224 TPDU from the wires
    {
        if (!stream.has_room(TPKT_HEADER_LEN)){
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }
        t->recv((char**)(&(stream.end)), TPKT_HEADER_LEN);
        this->tpkt.version = stream.in_uint8();
        if (3 != this->tpkt.version) {
            throw Error(ERR_ISO_RECV_MSG_VER_NOT_3);
        }
        stream.skip_uint8(1);
        this->tpkt.len = stream.in_uint16_be();
        const size_t payload_len = this->tpkt.len - TPKT_HEADER_LEN;
        if (!stream.has_room(payload_len)){
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }

        t->recv((char**)(&(stream.end)), payload_len);
        this->tpdu_hdr.LI = stream.in_uint8();
        this->tpdu_hdr.code = stream.in_uint8() & 0xF0;
        switch (this->tpdu_hdr.code){
            case DT_TPDU:
                this->tpdu_hdr.eot = stream.in_uint8();
                stream.skip_uint8(this->tpdu_hdr.LI-2);
            break;
            case DR_TPDU:
                stream.skip_uint8(4);
                this->tpdu_hdr.reason = stream.in_uint8();
                stream.skip_uint8(this->tpdu_hdr.LI-6);
            break;
            case ER_TPDU:
                stream.skip_uint8(2);
                this->tpdu_hdr.reject_cause = stream.in_uint8();
                stream.skip_uint8(this->tpdu_hdr.LI-4);
            break;
            default:
                // just skip remaining TPDU header content
                stream.skip_uint8(this->tpdu_hdr.LI-1);
        }
    }
};

struct X224Out : public X224Packet
{
    Stream & stream;

    X224Out(uint8_t tpdutype, Stream & stream) : stream(stream)
    // Prepare a X224 TPDU in buffer for writing
    {
        switch (tpdutype){
            case CR_TPDU: // Connection Request 1110 xxxx
                // we can write the header, there must not be any data afterward
                // tpkt
                stream.out_uint8(0x03); // version 3
                stream.out_uint8(0x00);
                stream.out_uint8(0x00); // 11 bytes
                stream.out_uint8(0x0B); //
                // CR_TPDU
                stream.out_uint8(0x06); // LI
                stream.out_uint8(CR_TPDU); // CR_TPDU code
                stream.out_uint8(0x00); // DST-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // SRC-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // CLASS OPTION
            break;
            case CC_TPDU: // Connection Confirm 1101 xxxx
                // we can write the header, there must not be any data
                // tpkt
                stream.out_uint8(0x03); // version 3
                stream.out_uint8(0x00);
                stream.out_uint8(0x00); // 11 bytes
                stream.out_uint8(0x0B); //
                // CR_TPDU
                stream.out_uint8(0x06); // LI
                stream.out_uint8(CC_TPDU); // CC_TPDU code
                stream.out_uint8(0x00); // DST-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // SRC-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // CLASS OPTION
            break;
            case DR_TPDU: // Disconnect Request 1000 0000
                // we can write the header, there must not be any data
                // tpkt
                stream.out_uint8(0x03); // version 3
                stream.out_uint8(0x00);
                stream.out_uint8(0x00); // 11 bytes
                stream.out_uint8(0x0B); //
                // CR_TPDU
                stream.out_uint8(0x06); // LI
                stream.out_uint8(DR_TPDU); // DR_TPDU code
                stream.out_uint8(0x00); // DST-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // SRC-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // CLASS OPTION
            break;
            case DT_TPDU: // Data               1111 0000 (no ROA = No Ack)
                // we can't write the full header yet,
                // we will know the length later
                // tpkt
                stream.out_uint8(0x03); // version 3
                stream.out_uint8(0x00);
                stream.out_uint8(0x00); // length still unknown
                stream.out_uint8(0x00); //
                // DT_TPDU
                stream.out_uint8(0x02); // LI
                stream.out_uint8(DT_TPDU); // DT_TPDU code
                stream.out_uint8(0x80); // EOT
            break;
            case ER_TPDU:  // TPDU Error         0111 0000
                // we can write the header, there must not be any data
                // tpkt
                stream.out_uint8(0x03); // version 3
                stream.out_uint8(0x00);
                stream.out_uint8(0x00); // 11 bytes
                stream.out_uint8(0x0B); //
                // ER_TPDU
                stream.out_uint8(0x06); // LI
                stream.out_uint8(ER_TPDU); // ER_TPDU code
                stream.out_uint8(0x00); // DST-REF
                stream.out_uint8(0x00); //
                stream.out_uint8(0x00); // Reject Cause : Unspecified
                stream.out_uint8(0xC1); // Invalid TPDU Code
                stream.out_uint8(0x00); // Parameter Length 0
            break;
            default:
                LOG(LOG_ERR, "Trying to send unknown TPDU Type %u", tpdutype);
                throw Error(ERR_X224_SENDING_UNKNOWN_PDU_TYPE, tpdutype);
        }
    }

    void extend_tpdu_hdr()
    // include user data from end of tpdu header to stream.p inside tpdu header.
    // Not at all part of x224, but RDP uses it to transmit username!!!
    // appending a string "Cookie: mstshash=username\r\n" to tpdu header.
    {
        this->stream.set_out_uint8(this->stream.p-this->stream.data-5, 4); // LI
    }

    void end()
    // This function update header informations of TPDU before it is sent
    // on the wires.
    {
        stream.mark_end();
        size_t len = stream.end - stream.data;
        stream.set_out_uint8(len >> 8, 2);
        stream.set_out_uint8(len & 0xFF, 3);
//        LOG(LOG_INFO, "2) [%.2X %.2X %.2X %.2X] [%.2X %.2X %.2X]", stream.data[0], stream.data[1], stream.data[2], stream.data[3], stream.data[4], stream.data[5], stream.data[6], stream.data[7]);
    }

    void send(Transport * t)
    {
//        LOG(LOG_INFO, "3) [%.2X %.2X %.2X %.2X] [%.2X %.2X %.2X]", stream.data[0], stream.data[1], stream.data[2], stream.data[3], stream.data[4], stream.data[5], stream.data[6], stream.data[7]);

        t->send((char*)stream.data, stream.end - stream.data);
        stream.p = stream.data;
    }
};

#endif
