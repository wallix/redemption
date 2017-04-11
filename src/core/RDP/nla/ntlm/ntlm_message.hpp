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

// [MS-NLMP]

// NTLM Message
// =============================================
// The NTLM Authentication Protocol consists of three message types
// used during authentication and one message type used for message
// integrity after authentication has occurred.

// The authentication messages:
// NEGOTIATE_MESSAGE
// CHALLENGE_MESSAGE
// AUTHENTICATE_MESSAGE
// are variable-length messages containing a fixed-length header and
// a variable-sized message payload. The fixed-length header always starts
// as shown in the following table with a Signature and MessageType field.
// Depending on the MessageType field, the message may have other
// message-dependent fixed-length fields. The fixed-length fields are then
// followed by a variable-length message payload.

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
// |              MessageDependantFields (Variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                       Payload (Variable)                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+


// Signature (8 bytes):  An 8-byte character array that MUST contain the
//  ASCII string ('N', 'T', 'L', 'M', 'S', 'S', 'P', '\0').
// MessageType (4 bytes):  The MessageType field MUST take one of the
//  values from the following list:

// +------------------+------------------------------------------+
// | Value            | Meaning                                  |
// +------------------+------------------------------------------+
// | NtlmNegotiate    | The message is a NEGOTIATE_MESSAGE       |
// | 0x00000001       |                                          |
// +------------------+------------------------------------------+
// | NtlmChallenge    | The message is a CHALLENGE_MESSAGE       |
// | 0x00000002       |                                          |
// +------------------+------------------------------------------+
// | NtlmAuthenticate | The message is a AUTHENTICATE_MESSAGE    |
// | 0x00000003       |                                          |
// +------------------+------------------------------------------+


// MessageDependentFields (variable):  The NTLM message contents, as
//  specified in section 2.2.1.
// payload (variable):  The payload data contains a message-dependent
//  number of individual payload messages. This payload data is referenced
//  by byte offsets located in the MessageDependentFields.


enum NtlmMessageType {
    NtlmNegotiate = 0x00000001,
    NtlmChallenge = 0x00000002,
    NtlmAuthenticate = 0x00000003
};
static const uint8_t NTLM_MESSAGE_SIGNATURE[] = "NTLMSSP\0";
struct NTLMMessage final {
    uint8_t signature[8];      /* 8 Bytes */
    NtlmMessageType msgType;   /* 4 Bytes */

    explicit NTLMMessage(NtlmMessageType msgType)
    : signature()
    , msgType(msgType)
    {
        memcpy(this->signature, NTLM_MESSAGE_SIGNATURE, 8);
        // signature[0] = 'N';
        // signature[1] = 'T';
        // signature[2] = 'L';
        // signature[3] = 'M';
        // signature[4] = 'S';
        // signature[5] = 'S';
        // signature[6] = 'P';
        // signature[7] = '\0';
    }

    void emit(OutStream & stream) const {
        stream.out_copy_bytes(this->signature, 8);
        stream.out_uint32_le(this->msgType);
    }

    bool recv(InStream & stream) {
        bool res = true;
        uint8_t received_sig[8];
        stream.in_copy_bytes(received_sig, 8);
        res &= (!memcmp(this->signature, received_sig, 8));
        uint32_t type = stream.in_uint32_le();
        res &= (static_cast<uint32_t>(this->msgType) == type);
        return res;
    }

};

// 2.2.2.10   VERSION
// ===================================================
// The VERSION structure contains Windows version information that SHOULD be ignored. This
//  structure is used for debugging purposes only and its value does not affect NTLM message
//  processing. It is present in the NEGOTIATE_MESSAGE, CHALLENGE_MESSAGE, and
//  AUTHENTICATE_MESSAGE messages only if NTLMSSP_NEGOTIATE_VERSION is negotiated.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | Product Major | Product Minor |         ProductBuild          |
// |    Version    |    Version    |                               |
// +---------------+---------------+---------------+---------------+
// |                   Reserved                    | NTLM Revision |
// |                                               |    Current    |
// +---------------+---------------+---------------+---------------+
//   ProductMajorVersion (1 byte):  An 8-bit unsigned integer that contains the major version
//    number of the Windows operating system in use. This field SHOULD contain one of the
//    following values:
// +------------------------+------------------------------------------+
// | Value                  | Meaning                                  |
// +------------------------+------------------------------------------+
// | WINDOW_MAJOR_VERSION_5 | The major version of the Windows         |
// | 0x05                   | operating system is 0x05                 |
// +------------------------+------------------------------------------+
// | WINDOW_MAJOR_VERSION_6 | The major version of the Windows         |
// | 0x06                   | operating system is 0x06                 |
// +------------------------+------------------------------------------+

//   ProductMinorVersion (1 byte):  An 8-bit unsigned integer that contains the minor version
//    number of the Windows operating system in use. This field SHOULD contain one of the
//    following values:
// +------------------------+------------------------------------------+
// | Value                  | Meaning                                  |
// +------------------------+------------------------------------------+
// | WINDOW_MINOR_VERSION_0 | The minor version of the Windows         |
// | 0x00                   | operating system is 0x00                 |
// +------------------------+------------------------------------------+
// | WINDOW_MINOR_VERSION_1 | The minor version of the Windows         |
// | 0x01                   | operating system is 0x01                 |
// +------------------------+------------------------------------------+
// | WINDOW_MINOR_VERSION_2 | The minor version of the Windows         |
// | 0x02                   | operating system is 0x02                 |
// +------------------------+------------------------------------------+

//   ProductBuild (2 bytes):  A 16-bit unsigned integer that contains the build number
//    of the Windows operating system in use. This field SHOULD be set to a 16-bit
//    quantity that identifie the operating system build number.
//   Reserved (3 bytes):  A 24-bit data area that SHOULD be set to zero and MUST be ignored by
//    the recipient.
//   NTLMRevisionCurrent (1 byte):  An 8-bit unsigned integer that contains a value indicating th
//    current revision of the NTLMSSP in use. This field SHOULD contain the following value:
// +------------------------+------------------------------------------+
// | Value                  | Meaning                                  |
// +------------------------+------------------------------------------+
// | NTLMSSP_REVISION_W2K3  | Version 15 of the NTLMSSP is in use      |
// | 0x0F                   |                                          |
// +------------------------+------------------------------------------+

#define WINDOWS_MINOR_VERSION_0          0x00
#define WINDOWS_MINOR_VERSION_1          0x01
#define WINDOWS_MINOR_VERSION_2          0x02

#define WINDOWS_MAJOR_VERSION_5          0x05
#define WINDOWS_MAJOR_VERSION_6          0x06

#define NTLMSSP_REVISION_W2K3            0x0F

struct NtlmVersion {
    uint8_t ProductMajorVersion;   /* 1 Byte */
    uint8_t ProductMinorVersion;   /* 1 Byte */
    uint16_t ProductBuild;         /* 2 Bytes */
    /* 3 Bytes Reserved */
    uint8_t NtlmRevisionCurrent;   /* 1 Byte */

    bool ignore_version;

    NtlmVersion()
        : ProductMajorVersion(WINDOWS_MAJOR_VERSION_6)
        , ProductMinorVersion(WINDOWS_MINOR_VERSION_2)
        , ProductBuild(0x0000)
        , NtlmRevisionCurrent(NTLMSSP_REVISION_W2K3)
        , ignore_version(true)
    {
    }

    void ignore_version_info() {
        this->ignore_version = true;
    }

    void ntlm_get_version_info() {
        this->ignore_version = false;
        this->ProductMajorVersion = WINDOWS_MAJOR_VERSION_5;
        this->ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
	this->ProductBuild        = 2600;
        this->NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
    }

    void emit(OutStream & stream) const {
        if (this->ignore_version) {
            return;
        }
        stream.out_uint8(this->ProductMajorVersion);
        stream.out_uint8(this->ProductMinorVersion);
        stream.out_uint16_le(this->ProductBuild);
        stream.out_clear_bytes(3);
        stream.out_uint8(this->NtlmRevisionCurrent);
    }

    void recv(InStream & stream) {
        this->ignore_version = false;
        this->ProductMajorVersion = stream.in_uint8();
        this->ProductMinorVersion = stream.in_uint8();
        this->ProductBuild = stream.in_uint16_le();
        stream.in_skip_bytes(3);
        this->NtlmRevisionCurrent = stream.in_uint8();
    }

    void log() const {
        LOG(LOG_INFO, "VERSION = {");
        LOG(LOG_INFO, "\tProductMajorVersion: %d", this->ProductMajorVersion);
        LOG(LOG_INFO, "\tProductMinorVersion: %d", this->ProductMinorVersion);
        LOG(LOG_INFO, "\tProductBuild: %d", this->ProductBuild);
        LOG(LOG_INFO, "\tNTLMRevisionCurrent: 0x%02X", this->NtlmRevisionCurrent);
        LOG(LOG_INFO, "}\n");
    }
};

// 2.2.2.5 NEGOTIATE
// ===================================================
// During NTLM authentication, each of the following flags is a possible value of
//  the NegotiateFlags field of the NEGOTIATE_MESSAGE, CHALLENGE_MESSAGE, and
//  AUTHENTICATE_MESSAGE, unless otherwise noted. These flags define client or
//  server NTLM capabilities supported by the sender.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |W|V|U|r|r|r|T|r|S|R|r|Q|P|r|O|N|M|r|L|K|J|r|H|r|G|F|E|D|r|C|B|A|
// | | | |1|2|3| |4| | |5| | |6| | | |7| | | |8| |9| | | | |1| | | |
// | | | | | | | | | | | | | | | | | | | | | | | | | | | | |0| | | |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// W (1 bit):  If set, requests 56-bit encryption. If the client sends
//  NTLMSSP_NEGOTIATE_SEAL or NTLMSSP_NEGOTIATE_SIGN with NTLMSSP_NEGOTIATE_56
//  to the server in the NEGOTIATE_MESSAGE, the server MUST return NTLMSSP_NEGOTIATE_56
//  to the client in the CHALLENGE_MESSAGE. Otherwise it is ignored.
//  If both NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128 are requested and supported
//  by the client and server, NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128 will both
//  be returned to the client.
//  Clients and servers that set NTLMSSP_NEGOTIATE_SEAL SHOULD set  NTLMSSP_NEGOTIATE_56
//  if it is supported. An alternate name for this field is  NTLMSSP_NEGOTIATE_56.

// V (1 bit):  If set, requests an explicit key exchange. This capability SHOULD
// be used because it improves security for message integrity or confidentiality.
// See sections 3.2.5.1.2, 3.2.5.2.1, and 3.2.5.2.2 for details. An alternate name
// for this field is NTLMSSP_NEGOTIATE_KEY_EXCH.

// U (1 bit):  If set, requests 128-bit session key negotiation. An alternate name
//  for this field is NTLMSSP_NEGOTIATE_128. If the client sends NTLMSSP_NEGOTIATE_128
//  to the server in the NEGOTIATE_MESSAGE, the server MUST return NTLMSSP_NEGOTIATE_128
//  to the client in the CHALLENGE_MESSAGE only if the client sets NTLMSSP_NEGOTIATE_SEAL or
//  NTLMSSP_NEGOTIATE_SIGN. Otherwise it is ignored. If both NTLMSSP_NEGOTIATE_56 and
//  NTLMSSP_NEGOTIATE_128 are requested and supported by the client and server,
//  NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128 will both be returned to the client.
//  Clients and servers that set NTLMSSP_NEGOTIATE_SEAL SHOULD set
//  NTLMSSP_NEGOTIATE_128 if it is supported. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_128.

// r1 (1 bit):  This bit is unused and MUST be zero.
// r2 (1 bit):  This bit is unused and MUST be zero.
// r3 (1 bit):  This bit is unused and MUST be zero.

// T (1 bit):  If set, requests the protocol version number. The data corresponding
//  to this flag is provided in the Version field of the NEGOTIATE_MESSAGE,
//  the CHALLENGE_MESSAGE, and the AUTHENTICATE_MESSAGE.<22> An alternate name for
//   this field is NTLMSSP_NEGOTIATE_VERSION.

// r4 (1 bit):  This bit is unused and MUST be zero.

// S (1 bit):  If set, indicates that the TargetInfo fields in the CHALLENGE_MESSAGE
//  (section 2.2.1.2) are populated. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_TARGET_INFO.

// R (1 bit):  If set, requests the usage of the LMOWF (section 3.3).
//  An alternate name for this field is NTLMSSP_REQUEST_NON_NT_SESSION_KEY.

// r5 (1 bit):  This bit is unused and MUST be zero.

// Q (1 bit):  If set, requests an identify level token. An alternate name for this
//  field is NTLMSSP_NEGOTIATE_IDENTIFY.

// P (1 bit):  If set, requests usage of the NTLM v2 session security.
//  NTLM v2 session security is a misnomer because it is not NTLM v2. It is NTLM v1
//  using the extended session security that is also in NTLM v2.
//  NTLMSSP_NEGOTIATE_LM_KEY and NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY
//  are mutually exclusive. If both NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY
//  and NTLMSSP_NEGOTIATE_LM_KEY are requested, NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY
//  alone MUST be returned to the client. NTLM v2 authentication session key generation
//  MUST be supported by both the client and the DC in order to be used, and extended
//  session security signing and sealing requires support from the client and the server
//  in order to be used. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY.

// r6 (1 bit):  This bit is unused and MUST be zero.

// O (1 bit):  If set, TargetName MUST be a server name. The data corresponding to this flag is
//  provided by the server in the TargetName field of the CHALLENGE_MESSAGE. If this bit is
//  set, then NTLMSSP_TARGET_TYPE_DOMAIN MUST NOT be set. This flag MUST be ignored in
//  the NEGOTIATE_MESSAGE and the AUTHENTICATE_MESSAGE. An alternate name for this field
//  is NTLMSSP_TARGET_TYPE_SERVER.

// N (1 bit):  If set, TargetName MUST be a domain name. The data corresponding to this flag is
//  provided by the server in the TargetName field of the CHALLENGE_MESSAGE. If set, then
//  NTLMSSP_TARGET_TYPE_SERVER MUST NOT be set. This flag MUST be ignored in the
//  NEGOTIATE_MESSAGE and the AUTHENTICATE_MESSAGE. An alternate name for this field is
//  NTLMSSP_TARGET_TYPE_DOMAIN.

// M (1 bit):  If set, requests the presence of a signature block on all messages.
//  NTLMSSP_NEGOTIATE_ALWAYS_SIGN MUST be set in the NEGOTIATE_MESSAGE to the
//  server and the CHALLENGE_MESSAGE to the client. NTLMSSP_NEGOTIATE_ALWAYS_SIGN is
//  overridden by NTLMSSP_NEGOTIATE_SIGN and NTLMSSP_NEGOTIATE_SEAL, if they are
//  supported. An alternate name for this field is NTLMSSP_NEGOTIATE_ALWAYS_SIGN.

// r7 (1 bit):  This bit is unused and MUST be zero.

// L (1 bit):  This flag indicates whether the Workstation field is present. If this flag is
//  not set, the Workstation field MUST be ignored. If this flag is set, the length field of
//  the Workstation field specifies whether the workstation name is nonempty or not.
//  An alternate name for this field is NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED.

// K (1 bit):  If set, the domain name is provided (section 2.2.1.1).<25> An alternate name for
//  this field is NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED.

// J (1 bit):  If set, the connection SHOULD be anonymous.<26>

// r8 (1 bit):  This bit is unused and SHOULD be zero.<27>

// H (1 bit):  If set, requests usage of the NTLM v1 session security protocol.
//  NTLMSSP_NEGOTIATE_NTLM MUST be set in the NEGOTIATE_MESSAGE to the server and the
//  CHALLENGE_MESSAGE to the client. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_NTLM.

// r9 (1 bit):  This bit is unused and MUST be zero.

// G (1 bit):  If set, requests LAN Manager (LM) session key computation.
//  NTLMSSP_NEGOTIATE_LM_KEY and NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY
//  are mutually exclusive. If both NTLMSSP_NEGOTIATE_LM_KEY and
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY are requested,
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY alone MUST be returned to the client.
//  NTLM v2 authentication session key generation MUST be supported by both the client and the
//  DC in order to be used, and extended session security signing and sealing requires support
//  from the client and the server to be used. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_LM_KEY.

// F (1 bit):  If set, requests connectionless authentication. If NTLMSSP_NEGOTIATE_DATAGRAM is
//  set, then NTLMSSP_NEGOTIATE_KEY_EXCH MUST always be set in the
//  AUTHENTICATE_MESSAGE to the server and the CHALLENGE_MESSAGE to the client. An
//  alternate name for this field is NTLMSSP_NEGOTIATE_DATAGRAM.

// E (1 bit):  If set, requests session key negotiation for message confidentiality.
//  If the client sends NTLMSSP_NEGOTIATE_SEAL to the server in the NEGOTIATE_MESSAGE,
//  the server MUST return NTLMSSP_NEGOTIATE_SEAL to the client in the CHALLENGE_MESSAGE.
//  Clients and servers that set NTLMSSP_NEGOTIATE_SEAL SHOULD always set NTLMSSP_NEGOTIATE_56
//  and NTLMSSP_NEGOTIATE_128, if they are supported. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_SEAL.

// D (1 bit):  If set, requests session key negotiation for message signatures.
//  If the client sends NTLMSSP_NEGOTIATE_SIGN to the server in the NEGOTIATE_MESSAGE,
//   the server MUST return NTLMSSP_NEGOTIATE_SIGN to the client in the CHALLENGE_MESSAGE.
//   An alternate name for this field is NTLMSSP_NEGOTIATE_SIGN.

// r10 (1 bit):  This bit is unused and MUST be zero.

// C (1 bit):  If set, a TargetName field of the CHALLENGE_MESSAGE (section 2.2.1.2) MUST be
//  supplied. An alternate name for this field is NTLMSSP_REQUEST_TARGET.

// B (1 bit):  If set, requests OEM character set encoding. An alternate name for this field is
//  NTLM_NEGOTIATE_OEM. See bit A for details.

// A (1 bit):  If set, requests Unicode character set encoding. An alternate name for this
//  field is NTLMSSP_NEGOTIATE_UNICODE.
//  The A and B bits are evaluated together as follows:
//  - A==1: The choice of character set encoding MUST be Unicode.
//  - A==0 and B==1: The choice of character set encoding MUST be OEM.
//  - A==0 and B==0: The protocol MUST return SEC_E_INVALID_TOKEN.

#define NTLMSSP_NEGOTIATE_56					0x80000000 /* W   (0) */
#define NTLMSSP_NEGOTIATE_KEY_EXCH				0x40000000 /* V   (1) */
#define NTLMSSP_NEGOTIATE_128					0x20000000 /* U   (2) */
#define NTLMSSP_RESERVED1					0x10000000 /* r1  (3) */
#define NTLMSSP_RESERVED2					0x08000000 /* r2  (4) */
#define NTLMSSP_RESERVED3					0x04000000 /* r3  (5) */
#define NTLMSSP_NEGOTIATE_VERSION				0x02000000 /* T   (6) */
#define NTLMSSP_RESERVED4					0x01000000 /* r4  (7) */
#define NTLMSSP_NEGOTIATE_TARGET_INFO				0x00800000 /* S   (8) */
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY			0x00400000 /* R   (9) */
#define NTLMSSP_RESERVED5					0x00200000 /* r5  (10) */
#define NTLMSSP_NEGOTIATE_IDENTIFY				0x00100000 /* Q   (11) */
#define NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY		0x00080000 /* P   (12) */
#define NTLMSSP_RESERVED6					0x00040000 /* r6  (13) */
#define NTLMSSP_TARGET_TYPE_SERVER				0x00020000 /* O   (14) */
#define NTLMSSP_TARGET_TYPE_DOMAIN				0x00010000 /* N   (15) */
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN				0x00008000 /* M   (16) */
#define NTLMSSP_RESERVED7					0x00004000 /* r7  (17) */
#define NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED			0x00002000 /* L   (18) */
#define NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED			0x00001000 /* K   (19) */
#define NTLMSSP_NEGOTIATE_ANONYMOUS				0x00000800 /* J   (20) */
#define NTLMSSP_RESERVED8					0x00000400 /* r8  (21) */
#define NTLMSSP_NEGOTIATE_NTLM					0x00000200 /* H   (22) */
#define NTLMSSP_RESERVED9					0x00000100 /* r9  (23) */
#define NTLMSSP_NEGOTIATE_LM_KEY				0x00000080 /* G   (24) */
#define NTLMSSP_NEGOTIATE_DATAGRAM				0x00000040 /* F   (25) */
#define NTLMSSP_NEGOTIATE_SEAL					0x00000020 /* E   (26) */
#define NTLMSSP_NEGOTIATE_SIGN					0x00000010 /* D   (27) */
#define NTLMSSP_RESERVED10					0x00000008 /* r10 (28) */
#define NTLMSSP_REQUEST_TARGET					0x00000004 /* C   (29) */
#define NTLMSSP_NEGOTIATE_OEM					0x00000002 /* B   (30) */
#define NTLMSSP_NEGOTIATE_UNICODE				0x00000001 /* A   (31) */

static const char* const NTLM_NEGOTIATE_STRINGS[] ={
    "NTLMSSP_NEGOTIATE_56",
    "NTLMSSP_NEGOTIATE_KEY_EXCH",
    "NTLMSSP_NEGOTIATE_128",
    "NTLMSSP_RESERVED1",
    "NTLMSSP_RESERVED2",
    "NTLMSSP_RESERVED3",
    "NTLMSSP_NEGOTIATE_VERSION",
    "NTLMSSP_RESERVED4",
    "NTLMSSP_NEGOTIATE_TARGET_INFO",
    "NTLMSSP_REQUEST_NON_NT_SESSION_KEY",
    "NTLMSSP_RESERVED5",
    "NTLMSSP_NEGOTIATE_IDENTIFY",
    "NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY",
    "NTLMSSP_RESERVED6",
    "NTLMSSP_TARGET_TYPE_SERVER",
    "NTLMSSP_TARGET_TYPE_DOMAIN",
    "NTLMSSP_NEGOTIATE_ALWAYS_SIGN",
    "NTLMSSP_RESERVED7",
    "NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED",
    "NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED",
    "NTLMSSP_NEGOTIATE_ANONYMOUS",
    "NTLMSSP_RESERVED8",
    "NTLMSSP_NEGOTIATE_NTLM",
    "NTLMSSP_RESERVED9",
    "NTLMSSP_NEGOTIATE_LM_KEY",
    "NTLMSSP_NEGOTIATE_DATAGRAM",
    "NTLMSSP_NEGOTIATE_SEAL",
    "NTLMSSP_NEGOTIATE_SIGN",
    "NTLMSSP_RESERVED10",
    "NTLMSSP_REQUEST_TARGET",
    "NTLMSSP_NEGOTIATE_OEM",
    "NTLMSSP_NEGOTIATE_UNICODE"
};

struct NtlmNegotiateFlags {
    uint32_t flags;          /* 4 Bytes */
    NtlmNegotiateFlags()
        : flags(0)
    {
    }

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->flags);
    }

    void recv(InStream & stream) {
        this->flags = stream.in_uint32_le();
    }

    void log() const
    {
        int i;
        const char* str;

        LOG(LOG_INFO, "negotiateFlags \"0x%08X\"{", this->flags);

        for (i = 31; i >= 0; i--) {
            if ((this->flags >> i) & 1) {
                str = NTLM_NEGOTIATE_STRINGS[(31 - i)];
                LOG(LOG_INFO, "\t%s (%d),", str, (31 - i));
            }
        }

        LOG(LOG_INFO, "}");
    }
};

struct NtlmField {
    uint16_t len;           /* 2 Bytes */
    uint16_t maxLen;        /* 2 Bytes */
    uint32_t bufferOffset;  /* 4 Bytes */
    struct Buffer {
        std::unique_ptr<uint8_t[]> dynbuf;
        uint8_t buf[65535];
        std::size_t sz_buf;
        OutStream ostream;
        std::size_t in_sz;

        Buffer()
        : sz_buf(sizeof(this->buf))
        , ostream(this->buf)
        , in_sz(0)
        {}

        Buffer(Buffer const &) = delete;

        void init(std::size_t sz) {
            auto p = this->ostream.get_data();
            if (sz > this->sz_buf) {
                p = this->buf;
                if (sz > sizeof(this->buf)) {
                    p = new uint8_t[sz];
                    this->dynbuf.reset(p);
                    this->sz_buf = sz;
                }
            }
            this->in_sz = 0;
            this->ostream = OutStream(p, sz);
        }

        void mark_end() {
            this->in_sz = this->ostream.get_offset();
        }

        uint8_t * get_data() {
            return this->ostream.get_data();
        }

        uint8_t const * get_data() const {
            return this->ostream.get_data();
        }

        std::size_t size() const {
            return this->in_sz;
        }

        InStream in_stream() const {
            return InStream(this->ostream.get_data(), this->in_sz);
        }

        void reset() {
            this->ostream.rewind();
            this->in_sz = 0;
        }
    } buffer;

    NtlmField()
        : len(0)
        , maxLen(0)
        , bufferOffset(0)
    {
    }

    ~NtlmField() {}

    unsigned int emit(OutStream & stream, unsigned int currentOffset) /* TODO const*/ {
        this->len = this->buffer.size();
        this->maxLen = this->len;
        this->bufferOffset = currentOffset;
        // currentOffset += this->len;

        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->maxLen);
        stream.out_uint32_le(this->bufferOffset);
        return this->len;
    }

    void recv(InStream & stream) {
        this->len = stream.in_uint16_le();
        this->maxLen = stream.in_uint16_le();
        this->bufferOffset = stream.in_uint32_le();
    }

    void read_payload(InStream & stream, uint8_t const * pBegin) {
        if (this->len > 0) {
            uint8_t const * pEnd = pBegin + this->bufferOffset + this->len;
            if (pEnd > stream.get_current()) {
                if (pEnd > stream.get_data_end()) {
                    LOG(LOG_ERR, "INVALID stream read");
                    return;
                }
                stream.in_skip_bytes(pEnd - stream.get_current());;
            }
            this->buffer.init(this->len);
            this->buffer.ostream.out_copy_bytes(pBegin + this->bufferOffset, this->len);
            this->buffer.mark_end();
            this->buffer.ostream.rewind();
        }
    }

    void write_payload(OutStream & stream) const {
        if (this->len > 0) {
            stream.out_copy_bytes(this->buffer.get_data(), this->len);
        }
    }

};


// 2.2.2.9   NTLMSSP_MESSAGE_SIGNATURE
// ===================================================

// The NTLMSSP_MESSAGE_SIGNATURE structure (section 3.4.4), specifies the signature
//  block used for application message integrity and confidentiality. This structure
//  is then passed back to the application, which embeds it within the application
//  protocol messages, along with the NTLM-encrypted or integrity-protected application
//  message data.

// This structure MUST take one of the two following forms, depending on whether the
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY flag is negotiated:
//  - NTLMSSP_MESSAGE_SIGNATURE
//  - NTLMSSP_MESSAGE_SIGNATURE for Extended Session Security

// 2.2.2.9.1   NTLMSSP_MESSAGE_SIGNATURE
// ====================================================
// This version of the NTLMSSP_MESSAGE_SIGNATURE structure MUST be used when the
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY flag is not negotiated.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                            Version                            |
// +---------------+---------------+---------------+---------------+
// |                           RandomPad                           |
// +---------------+---------------+---------------+---------------+
// |                           CheckSum                            |
// +---------------+---------------+---------------+---------------+
// |                            SeqNum                             |
// +---------------+---------------+---------------+---------------+
// Version (4 bytes):  A 32-bit unsigned integer that contains the signature version.
//  This field MUST be 0x00000001.
// RandomPad (4 bytes):  A 4-byte array that contains the random pad for the message.
// Checksum (4 bytes):  A 4-byte array that contains the checksum for the message.
// SeqNum (4 bytes):  A 32-bit unsigned integer that contains the NTLM sequence number for
//  this application message.

struct NTLMSSPMessageSignature {
    uint32_t Version;
    uint8_t  RandomPad[4];
    uint8_t  CheckSum[4];
    uint32_t SeqNum;
};


// 2.2.2.9.2   NTLMSSP_MESSAGE_SIGNATURE for Extended Session Security
// ====================================================================
// This version of the NTLMSSP_MESSAGE_SIGNATURE structure MUST be used when the
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY flag is negotiated.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                            Version                            |
// +---------------+---------------+---------------+---------------+
// |                           CheckSum                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                            SeqNum                             |
// +---------------+---------------+---------------+---------------+

// Version (4 bytes):  A 32-bit unsigned integer that contains the signature version.
//  This field MUST be 0x00000001.
// Checksum (8 bytes):  An 8-byte array that contains the checksum for the message.
// SeqNum (4 bytes):  A 32-bit unsigned integer that contains the NTLM sequence number
// for this application message.
struct NTLMSSPMessageSignatureESS {
    uint32_t Version;
    uint8_t  CheckSum[8];
    uint32_t SeqNum;
};


