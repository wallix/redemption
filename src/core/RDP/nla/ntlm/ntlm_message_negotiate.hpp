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

#include "core/RDP/nla/ntlm/ntlm_message.hpp"
#include "utils/log.hpp"

// [MS-NLMP]

// 2.2.1.1 NEGOTIATE_MESSAGE
// ============================================================
// The NEGOTIATE_MESSAGE defines an NTLM Negotiate message that is sent from
// the client to the server. This message allows the client to specify its
//  supported NTLM options to the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Signature                           |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                          MessageType                          |
// +---------------+---------------+---------------+---------------+
// |                         NegotiateFlags                        |
// +---------------+---------------+---------------+---------------+
// |                        DomainNameFields                       |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                        WorkstationFields                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                            Version                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                       Payload (Variable)                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+

// Signature (8 bytes):  An 8-byte character array that MUST contain the
//  ASCII string ('N', 'T', 'L', 'M', 'S', 'S', 'P', '\0').

// MessageType (4 bytes):  A 32-bit unsigned integer that indicates the message
//  type. This field MUST be set to 0x00000001.

// NegotiateFlags (4 bytes):  A NEGOTIATE structure that contains a set of bit flags,
//  as defined in section 2.2.2.5. The client sets flags to indicate options it supports.

// DomainNameFields (8 bytes):  If the NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED flag is
//  not set in NegotiateFlags, indicating that no DomainName is supplied in Payload:
//  - DomainNameLen and DomainNameMaxLen fields SHOULD be set to zero.
//  - DomainNameBufferOffset field SHOULD be set to the offset from the beginning of the
//     NEGOTIATE_MESSAGE to where the DomainName would be in Payload if it was present.
//  - DomainNameLen, DomainNameMaxLen, and DomainNameBufferOffset MUST be ignored
//     on receipt.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        DomainNameLen          |       DomainNameMaxLen        |
// +---------------+---------------+---------------+---------------+
// |                     DomainNameBufferOffset                    |
// +---------------+---------------+---------------+---------------+
//    DomainNameLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of DomainName in Payload.
//    DomainNameMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be set
//     to the value of DomainNameLen and MUST be ignored on receipt.
//    DomainNameBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the NEGOTIATE_MESSAGE to
//     DomainName in Payload.

// WorkstationFields (8 bytes):  If the NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED
//  flag is not set in NegotiateFlags, indicating that no WorkstationName is
//  supplied in Payload:
//  - WorkstationLen and WorkstationMaxLen fields SHOULD be set to zero.
//  - WorkstationBufferOffset field SHOULD be set to the offset from the beginning
//     of the NEGOTIATE_MESSAGE to where the WorkstationName would be in Payload
//     if it was present.
//  - WorkstationLen, WorkstationMaxLen, and WorkstationBufferOffset MUST be
//     ignored on receipt.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         WorkstatonLen         |       WorkstationMaxLen       |
// +---------------+---------------+---------------+---------------+
// |                     WorkstationBufferOffset                   |
// +---------------+---------------+---------------+---------------+
//    WorkstationLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of WorkStationName in Payload.
//    WorkstationMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be set
//     to the value of WorkstationLen and MUST be ignored on receipt.
//    WorkstationBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the NEGOTIATE_MESSAGE to
//     WorkstationName in Payload.

// Version (8 bytes):  A VERSION structure (as defined in section 2.2.2.10) that
//  is present only when the NTLMSSP_NEGOTIATE_VERSION flag is set in the
//  NegotiateFlags field. This structure is used for debugging purposes only.
//  In normal (non-debugging) protocol messages, it is ignored and does not affect
//  the NTLM message processing.

// Payload (variable):  A byte-array that contains the data referred to by the
//   DomainNameBufferOffset and WorkstationBufferOffset message fields. Payload data
//   can be present in any order within the Payload field, with variable-length padding
//   before or after the data. The data that can be present in the Payload field of
//   this message, in no particular order, are:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      DomainName (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                    WorkstationName (Variable)                 |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//    DomainName (variable):  If DomainNameLen does not equal 0x0000, DomainName
//     MUST be a byte-array that contains the name of the client authentication domain
//     that MUST be encoded using the OEM character set. Otherwise, this data is
//     not present.
//    WorkstationName (variable):  If WorkstationLen does not equal 0x0000,
//     WorkstationName MUST be a byte array that contains the name of the client
//     machine that MUST be encoded using the OEM character set. Otherwise, this data
//     is not present.




class NTLMNegotiateMessage
{
    NTLMMessage message;

public:
    NtlmNegotiateFlags negoFlags; /* 4 Bytes */
    NtlmField DomainName;         /* 8 Bytes */
    NtlmField Workstation;        /* 8 Bytes */
    NtlmVersion version;          /* 8 Bytes */
private:
    uint32_t PayloadOffset;

public:
    NTLMNegotiateMessage()
        : message(NtlmNegotiate)
        , PayloadOffset(12+4+8+8+8)
    {
    }

    void emit(OutStream & stream) /* TODO const*/ {
        uint32_t currentOffset = this->PayloadOffset;
        if (this->version.ignore_version) {
            currentOffset -= 8;
        }
        this->message.emit(stream);
        this->negoFlags.emit(stream);
        currentOffset += this->DomainName.emit(stream, currentOffset);
        currentOffset += this->Workstation.emit(stream, currentOffset);
        (void)currentOffset;
        this->version.emit(stream);

        // PAYLOAD
        this->DomainName.write_payload(stream);
        this->Workstation.write_payload(stream);
    }

    void recv(InStream & stream) {
        uint8_t const * pBegin = stream.get_current();
        bool res;
        res = this->message.recv(stream);
        if (!res) {
            LOG(LOG_ERR, "INVALID MSG RECEIVED type: %u", this->message.msgType);
        }
        this->negoFlags.recv(stream);
        this->DomainName.recv(stream);
        this->Workstation.recv(stream);
        if (this->negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
            this->version.recv(stream);
        }
        // PAYLOAD
        this->DomainName.read_payload(stream, pBegin);
        this->Workstation.read_payload(stream, pBegin);
    }


    //bool check_negotiate_flag_received() {
    //    uint32_t flags = this->negoFlags.flags;
    //    if (!((flags & NTLMSSP_REQUEST_TARGET) &&
    //          (flags & NTLMSSP_NEGOTIATE_NTLM) &&
    //          (flags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN) &&
    //          (flags & NTLMSSP_NEGOTIATE_UNICODE))) {
    //        return false;
    //    }
    //    return true;
    //}

};

