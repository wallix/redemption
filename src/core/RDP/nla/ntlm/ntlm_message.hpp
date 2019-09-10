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

#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/serialize.hpp"
#include "utils/hexdump.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "system/ssl_md5.hpp"
#include "system/ssl_rc4.hpp"
#include "system/ssl_md4.hpp"
#include "system/ssl_sha256.hpp"

#include <numeric>

static inline std::vector<uint8_t> UTF16_to_upper(array_view_const_u8 name)
{
    std::vector<uint8_t> result(name.data(), name.data()+name.size());
    ::UTF16Upper(result.data(), result.size());
    return result;
}

using array_challenge = std::array<uint8_t, 8>;

using array_md4 = std::array<uint8_t, SslMd4::DIGEST_LENGTH>;
static inline array_md4 Md4(array_view_const_u8 data)
{
    array_md4 result;
    SslMd4 md4;
    md4.update(data);
    md4.unchecked_final(result.data());
    return result;
}

using array_md5 = std::array<uint8_t, SslMd5::DIGEST_LENGTH>;
static inline array_md5 Rc4Key(array_view_const_u8 key, array_view_const_u8 plaintext)
{
    array_md5 cyphertext;
    SslRC4 rc4;
    rc4.set_key(key);
    rc4.crypt(plaintext.size(), plaintext.data(), cyphertext.data());
    return cyphertext;
}

static inline std::vector<uint8_t> Rc4CryptVector(SslRC4 &rc4, array_view_const_u8 plaintext)
{
    std::vector<uint8_t> cyphertext(plaintext.size());
    rc4.crypt(plaintext.size(), plaintext.data(), cyphertext.data());
    return cyphertext;
}

template<std::size_t N>
static inline std::array<uint8_t, N> Rc4Crypt(SslRC4 &rc4, array_view_const_u8 plaintext)
{
    std::array<uint8_t, N> cyphertext;
    rc4.crypt(N, plaintext.data(), cyphertext.data());
    return cyphertext;
}

static inline array_md5 Md5(array_view_const_u8 data)
{
    array_md5 result;
    SslMd5 md5;
    md5.update(data);
    md5.unchecked_final(result.data());
    return result;
}

static inline array_md5 Md5(array_view_const_u8 data1, bytes_view data2)
{
    array_md5 result;
    SslMd5 md5;
    md5.update(data1);
    md5.update(data2);
    md5.unchecked_final(result.data());
    return result;
}


static inline array_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data)
{
    array_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data);
    hmac_md5.unchecked_final(result.data());
    return result;
}

static inline array_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data1, array_view_const_u8 data2)
{
    array_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data1);
    hmac_md5.update(data2);
    hmac_md5.unchecked_final(result.data());
    return result;
}

static inline array_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data1, array_view_const_u8 data2, array_view_const_u8 data3)
{
    array_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data1);
    hmac_md5.update(data2);
    hmac_md5.update(data3);
    hmac_md5.unchecked_final(result.data());
    return result;
}

using array_sha256 = std::array<uint8_t, SslSha256::DIGEST_LENGTH>;
static inline array_sha256 Sha256(bytes_view data1, array_view_const_u8 data2, array_view_const_u8 data3)
{
    array_sha256 result;
    SslSha256 sha256;
    sha256.update(data1);
    sha256.update(data2);
    sha256.update(data3);
    sha256.unchecked_final(result.data());
    return result;
}


// 2.2.2.1   AV_PAIR
// ==================================
// The AV_PAIR structure defines an attribute/value pair. Sequences of AV_PAIR
//  structures are used in the CHALLENGE_MESSAGE (section 2.2.1.2) and
//  AUTHENTICATE_MESSAGE (section 2.2.1.3) messages.

//  Although the following figure suggests that the most significant bit (MSB) of AvId
//  is aligned with the MSB of a 32-bit word, an AV_PAIR can be aligned on any byte
//  boundary and can be 4+N bytes long for arbitrary N (N = the contents of AvLen).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |              AvId             |             AvLen             |
// +---------------+---------------+---------------+---------------+
// |                         Value (variable)                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+

// AvId (2 bytes):  A 16-bit unsigned integer that defines the information type in the
//  Value field. The contents of this field MUST be one of the values from the following
//  table.The corresponding Value field in this AV_PAIR MUST contain the information
//  specified in the description of that AvId.
// +----------------------+-------------------------------------------------------------+
// | Value                | Meaning                                                     |
// +----------------------+-------------------------------------------------------------+
// | MsvAvEOL             | Indicates that this is the last AV_PAIR in the list. AvLen  |
// | 0x0000               |  MUST be 0. This type of information MUST be present in     |
// |                      |  the AV pair list.                                          |
// +----------------------+-------------------------------------------------------------+
// | MsvAvNbComputerName  | The server's NetBIOS computer name. The name MUST be in     |
// | 0x0001               |  Unicode, and is not null-terminated. This type of          |
// |                      |  information MUST be present in the AV_pair list.           |
// +----------------------+-------------------------------------------------------------+
// | MsvAvNbDomainName    | The server's NetBIOS domain name. The name MUST be in       |
// | 0x0002               |  Unicode, and is not null-terminated. This type of          |
// |                      |  information MUST be present in the AV_pair list.           |
// +----------------------+-------------------------------------------------------------+
// | MsvAvDnsComputerName | The fully qualified domain name (FQDN (1)) of the computer. |
// | 0x0003               |  The name MUST be in Unicode, and is not null-terminated.   |
// +----------------------+-------------------------------------------------------------+
// | MsvAvDnsDomainName   | The FQDN (2) of the domain. The name MUST be in Unicode,    |
// | 0x0004               |  and is not null-terminated.                                |
// +----------------------+-------------------------------------------------------------+
// | MsvAvDnsTreeName     | The FQDN (2) of the forest. The name MUST be in Unicode,    |
// | 0x0005               |  and is not null-terminated.                                |
// +----------------------+-------------------------------------------------------------+
// | MsvAvFlags           | A 32-bit value indicating server or client configuration.   |
// | 0x0006               | 0x00000001: indicates to the client that the account        |
// |                      |  authentication is constrained.                             |
// |                      | 0x00000002: indicates that the client is providing message  |
// |                      |  integrity in the MIC field (section 2.2.1.3) in the        |
// |                      |  AUTHENTICATE_MESSAGE.                                      |
// |                      | 0x00000004: indicates that the client is providing a        |
// |                      |  target SPN generated from an untrusted source.             |
// +----------------------+-------------------------------------------------------------+
// | MsvAvTimeStamp       | A FILETIME structure ([MS-DTYP] section 2.3.3) in little    |
// | 0x0007               |  -endian byte order that contains the server local time.    |
// +----------------------+-------------------------------------------------------------+
// | MsvAvSingleHost      | A Single_Host_Data (section 2.2.2.2) structure. The Value   |
// | 0x0008               |  field contains a platform-specific blob, as well as a      |
// |                      |  MachineID created at computer startup to identify the      |
// |                      |  calling machine.                                           |
// +----------------------+-------------------------------------------------------------+
// | MsvAvTargetName      | The SPN of the target server. The name MUST be in Unicode   |
// | 0x0009               |  and is not null-terminated.                                |
// +----------------------+-------------------------------------------------------------+
// | MsvChannelBindings   | A channel bindings hash. The Value field contains an MD5    |
// | 0x000A               |  hash ([RFC4121] section 4.1.1.2) of a                      |
// |                      |  gss_channel_bindings_struct ([RFC2744] section 3.11).      |
// |                      |  An all-zero value of the hash is used to indicate absence  |
// |                      |  of channel bindings.                                       |
// +----------------------+-------------------------------------------------------------+


// AvLen (2 bytes):  A 16-bit unsigned integer that defines the length, in bytes, of
//  the Value field.
// Value (variable):  A variable-length byte-array that contains the value defined for
//  this AV pair entry. The contents of this field depend on the type expressed in the
//  AvId field. The available types and resulting format and contents of this field are
//  specified in the table within the AvId field description in this topic.
enum NTLM_AV_ID : uint16_t {
    MsvAvEOL             = 0x0000,
    MsvAvNbComputerName  = 0x01,
    MsvAvNbDomainName    = 0x02,
    MsvAvDnsComputerName = 0x03,
    MsvAvDnsDomainName   = 0x04,
    MsvAvDnsTreeName     = 0x05,
    MsvAvFlags           = 0x06,
    MsvAvTimestamp       = 0x07,
    MsvAvSingleHost      = 0x08,
    MsvAvTargetName      = 0x09,
    MsvChannelBindings   = 0x0A,
    AV_ID_MAX
};

struct AvPair {
    NTLM_AV_ID id;
    std::vector<uint8_t> data;
};

using NtlmAvPairList = std::vector<AvPair>;

enum NtlmState {
    NTLM_STATE_INITIAL,
    NTLM_STATE_NEGOTIATE,
    NTLM_STATE_CHALLENGE,
    NTLM_STATE_AUTHENTICATE,
    NTLM_STATE_WAIT_PASSWORD,
    NTLM_STATE_FINAL
};

// TODO: constants below are still globals,
// better to move them in the scope of functions/objects using them
//const char* NTLM_PACKAGE_NAME = "NTLM";
// const char Ntlm_Name[] = "NTLM";NtlmAuthenticate
// const char Ntlm_Comment[] = "NTLM Security Package";
// const SecPkgInfo NTLM_SecPkgInfo = {
//     0x00082B37,             // fCapabilities
//     1,                      // wVersion
//     0x000A,                 // wRPCID
//     0x00000B48,             // cbMaxToken
//     Ntlm_Name,              // Name
//     Ntlm_Comment            // Comment
// };

// SecPkgContext_Sizes ContextSizes;
// ContextSizes.cbMaxToken = 2010;
// ContextSizes.cbMaxSignature = 16;
// ContextSizes.cbBlockSize = 0;
// ContextSizes.cbSecurityTrailer = 16;

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


enum NtlmMessageType : uint32_t {
    NtlmNegotiate = 0x00000001,
    NtlmChallenge = 0x00000002,
    NtlmAuthenticate = 0x00000003
};

static constexpr uint8_t NTLM_MESSAGE_SIGNATURE[8] = "NTLMSSP";

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

enum ProductMinorVersion : uint8_t
{
    WINDOWS_MINOR_VERSION_0 = 0x00,
    WINDOWS_MINOR_VERSION_1 = 0x01,
    WINDOWS_MINOR_VERSION_2 = 0x02,
};

enum ProductMajorVersion : uint8_t
{
    WINDOWS_MAJOR_VERSION_5 = 0x05,
    WINDOWS_MAJOR_VERSION_6 = 0x06
};

enum NTLMRevisionCurrent : uint8_t
{
    NTLMSSP_REVISION_W2K3 = 0x0F
};

struct NtlmVersion {
    uint8_t ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
    uint8_t ProductMinorVersion = WINDOWS_MINOR_VERSION_2;
    uint16_t ProductBuild = 0;
    /* 3 Bytes Reserved */
    uint8_t NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;

//    bool ignore_version{true};
};

inline void LogNtlmVersion(const NtlmVersion & self) {
    LOG(LOG_INFO, "VERSION = {");
    LOG(LOG_INFO, "\tProductMajorVersion: %d", self.ProductMajorVersion);
    LOG(LOG_INFO, "\tProductMinorVersion: %d", self.ProductMinorVersion);
    LOG(LOG_INFO, "\tProductBuild: %d", self.ProductBuild);
    LOG(LOG_INFO, "\tNTLMRevisionCurrent: 0x%02X", self.NtlmRevisionCurrent);
    LOG(LOG_INFO, "}");
}

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

enum {

// NTLMSSP_NEGOTIATE_56: W (1 bit):  If set, requests 56-bit encryption. If the client sends
//  NTLMSSP_NEGOTIATE_SEAL or NTLMSSP_NEGOTIATE_SIGN with NTLMSSP_NEGOTIATE_56
//  to the server in the NEGOTIATE_MESSAGE, the server MUST return NTLMSSP_NEGOTIATE_56
//  to the client in the CHALLENGE_MESSAGE. Otherwise it is ignored.
//  If both NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128 are requested and supported
//  by the client and server, NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128 will both
//  be returned to the client.
//  Clients and servers that set NTLMSSP_NEGOTIATE_SEAL SHOULD set NTLMSSP_NEGOTIATE_56
//  if it is supported.

    NTLMSSP_NEGOTIATE_56 = 0x80000000, /* W   (0) */

// NTLMSSP_NEGOTIATE_KEY_EXCH: V (1 bit):  If set, requests an explicit key exchange. 
// This capability SHOULD be used because it improves security for message integrity 
// or confidentiality. See sections 3.2.5.1.2, 3.2.5.2.1, and 3.2.5.2.2 for details.

    NTLMSSP_NEGOTIATE_KEY_EXCH = 0x40000000, /* V   (1) */

// NTLMSSP_NEGOTIATE_128: U (1 bit):  If set, requests 128-bit session key negotiation.
//  If the client sends NTLMSSP_NEGOTIATE_128 to the server in the NEGOTIATE_MESSAGE,
//  the server MUST return NTLMSSP_NEGOTIATE_128 to the client in the CHALLENGE_MESSAGE
//  only if the client sets NTLMSSP_NEGOTIATE_SEAL or NTLMSSP_NEGOTIATE_SIGN. Otherwise
// it is ignored. If both NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128 are requested
// and supported by the client and server, NTLMSSP_NEGOTIATE_56 and NTLMSSP_NEGOTIATE_128
// will both be returned to the client. Clients and servers that set NTLMSSP_NEGOTIATE_SEAL
// SHOULD set NTLMSSP_NEGOTIATE_128 if it is supported.

    NTLMSSP_NEGOTIATE_128 = 0x20000000, /* U   (2) */

// r1 (1 bit):  This bit is unused and MUST be zero.
    NTLMSSP_RESERVED1 = 0x10000000, /* r1  (3) */

// r2 (1 bit):  This bit is unused and MUST be zero.
    NTLMSSP_RESERVED2 = 0x08000000, /* r2  (4) */

// r3 (1 bit):  This bit is unused and MUST be zero.
    NTLMSSP_RESERVED3 = 0x04000000, /* r3  (5) */

// NTLMSSP_NEGOTIATE_VERSION: T (1 bit):  If set, requests the protocol version 
// number. The data corresponding to this flag is provided in the Version field
// of the NEGOTIATE_MESSAGE, the CHALLENGE_MESSAGE, and the AUTHENTICATE_MESSAGE.<22>

    NTLMSSP_NEGOTIATE_VERSION = 0x02000000, /* T   (6) */

// r4 (1 bit):  This bit is unused and MUST be zero.

    NTLMSSP_RESERVED4 = 0x01000000, /* r4  (7) */

// S (1 bit):  If set, indicates that the TargetInfo fields in the CHALLENGE_MESSAGE
//  (section 2.2.1.2) are populated. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_TARGET_INFO.

    NTLMSSP_NEGOTIATE_TARGET_INFO = 0x00800000, /* S   (8) */

// R (1 bit):  If set, requests the usage of the LMOWF (section 3.3).
//  An alternate name for this field is NTLMSSP_REQUEST_NON_NT_SESSION_KEY.

    NTLMSSP_REQUEST_NON_NT_SESSION_KEY = 0x00400000, /* R   (9) */

// r5 (1 bit):  This bit is unused and MUST be zero.

    NTLMSSP_RESERVED5 = 0x00200000, /* r5  (10) */

// Q (1 bit):  If set, requests an identify level token. An alternate name for this
//  field is NTLMSSP_NEGOTIATE_IDENTIFY.

    NTLMSSP_NEGOTIATE_IDENTIFY = 0x00100000, /* Q   (11) */

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

    NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY = 0x00080000, /* P   (12) */

// r6 (1 bit):  This bit is unused and MUST be zero.

    NTLMSSP_RESERVED6 = 0x00040000, /* r6  (13) */

// O (1 bit):  If set, TargetName MUST be a server name. The data corresponding to this flag is
//  provided by the server in the TargetName field of the CHALLENGE_MESSAGE. If this bit is
//  set, then NTLMSSP_TARGET_TYPE_DOMAIN MUST NOT be set. This flag MUST be ignored in
//  the NEGOTIATE_MESSAGE and the AUTHENTICATE_MESSAGE. An alternate name for this field
//  is NTLMSSP_TARGET_TYPE_SERVER.

    NTLMSSP_TARGET_TYPE_SERVER = 0x00020000, /* O   (14) */

// N (1 bit):  If set, TargetName MUST be a domain name. The data corresponding to this flag is
//  provided by the server in the TargetName field of the CHALLENGE_MESSAGE. If set, then
//  NTLMSSP_TARGET_TYPE_SERVER MUST NOT be set. This flag MUST be ignored in the
//  NEGOTIATE_MESSAGE and the AUTHENTICATE_MESSAGE. An alternate name for this field is
//  NTLMSSP_TARGET_TYPE_DOMAIN.

    NTLMSSP_TARGET_TYPE_DOMAIN = 0x00010000, /* N   (15) */

// M (1 bit):  If set, requests the presence of a signature block on all messages.
//  NTLMSSP_NEGOTIATE_ALWAYS_SIGN MUST be set in the NEGOTIATE_MESSAGE to the
//  server and the CHALLENGE_MESSAGE to the client. NTLMSSP_NEGOTIATE_ALWAYS_SIGN is
//  overridden by NTLMSSP_NEGOTIATE_SIGN and NTLMSSP_NEGOTIATE_SEAL, if they are
//  supported. An alternate name for this field is NTLMSSP_NEGOTIATE_ALWAYS_SIGN.

    NTLMSSP_NEGOTIATE_ALWAYS_SIGN = 0x00008000, /* M   (16) */

// r7 (1 bit):  This bit is unused and MUST be zero.

    NTLMSSP_RESERVED7 =  0x00004000, /* r7  (17) */

// L (1 bit):  This flag indicates whether the Workstation field is present. If this flag is
//  not set, the Workstation field MUST be ignored. If this flag is set, the length field of
//  the Workstation field specifies whether the workstation name is nonempty or not.
//  An alternate name for this field is NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED.

    NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED = 0x00002000, /* L   (18) */

// K (1 bit):  If set, the domain name is provided (section 2.2.1.1).<25> An alternate name for
//  this field is NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED.

    NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED = 0x00001000, /* K   (19) */

// J (1 bit):  If set, the connection SHOULD be anonymous.<26>

    NTLMSSP_NEGOTIATE_ANONYMOUS = 0x00000800, /* J   (20) */

// r8 (1 bit):  This bit is unused and SHOULD be zero.<27>

    NTLMSSP_RESERVED8 = 0x00000400, /* r8  (21) */

// H (1 bit):  If set, requests usage of the NTLM v1 session security protocol.
//  NTLMSSP_NEGOTIATE_NTLM MUST be set in the NEGOTIATE_MESSAGE to the server and the
//  CHALLENGE_MESSAGE to the client. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_NTLM.

    NTLMSSP_NEGOTIATE_NTLM = 0x00000200, /* H   (22) */

// r9 (1 bit):  This bit is unused and MUST be zero.

    NTLMSSP_RESERVED9 = 0x00000100, /* r9  (23) */

// G (1 bit):  If set, requests LAN Manager (LM) session key computation.
//  NTLMSSP_NEGOTIATE_LM_KEY and NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY
//  are mutually exclusive. If both NTLMSSP_NEGOTIATE_LM_KEY and
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY are requested,
//  NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY alone MUST be returned to the client.
//  NTLM v2 authentication session key generation MUST be supported by both the client and the
//  DC in order to be used, and extended session security signing and sealing requires support
//  from the client and the server to be used. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_LM_KEY.

    NTLMSSP_NEGOTIATE_LM_KEY = 0x00000080, /* G   (24) */

// F (1 bit):  If set, requests connectionless authentication. If NTLMSSP_NEGOTIATE_DATAGRAM is
//  set, then NTLMSSP_NEGOTIATE_KEY_EXCH MUST always be set in the
//  AUTHENTICATE_MESSAGE to the server and the CHALLENGE_MESSAGE to the client. An
//  alternate name for this field is NTLMSSP_NEGOTIATE_DATAGRAM.

    NTLMSSP_NEGOTIATE_DATAGRAM = 0x00000040, /* F   (25) */

// E (1 bit):  If set, requests session key negotiation for message confidentiality.
//  If the client sends NTLMSSP_NEGOTIATE_SEAL to the server in the NEGOTIATE_MESSAGE,
//  the server MUST return NTLMSSP_NEGOTIATE_SEAL to the client in the CHALLENGE_MESSAGE.
//  Clients and servers that set NTLMSSP_NEGOTIATE_SEAL SHOULD always set NTLMSSP_NEGOTIATE_56
//  and NTLMSSP_NEGOTIATE_128, if they are supported. An alternate name for this field is
//  NTLMSSP_NEGOTIATE_SEAL.

    NTLMSSP_NEGOTIATE_SEAL = 0x00000020, /* E   (26) */

// D (1 bit):  If set, requests session key negotiation for message signatures.
//  If the client sends NTLMSSP_NEGOTIATE_SIGN to the server in the NEGOTIATE_MESSAGE,
//   the server MUST return NTLMSSP_NEGOTIATE_SIGN to the client in the CHALLENGE_MESSAGE.
//   An alternate name for this field is NTLMSSP_NEGOTIATE_SIGN.

    NTLMSSP_NEGOTIATE_SIGN = 0x00000010, /* D   (27) */

// r10 (1 bit):  This bit is unused and MUST be zero.

    NTLMSSP_RESERVED10 = 0x00000008, /* r10 (28) */

// C (1 bit):  If set, a TargetName field of the CHALLENGE_MESSAGE (section 2.2.1.2) MUST be
//  supplied. An alternate name for this field is NTLMSSP_REQUEST_TARGET.

    NTLMSSP_REQUEST_TARGET = 0x00000004, /* C   (29) */

// B (1 bit):  If set, requests OEM character set encoding. An alternate name for this field is
//  NTLM_NEGOTIATE_OEM. See bit A for details.

    NTLMSSP_NEGOTIATE_OEM = 0x00000002, /* B   (30) */

// A (1 bit):  If set, requests Unicode character set encoding. An alternate name for this
//  field is NTLMSSP_NEGOTIATE_UNICODE.
//  The A and B bits are evaluated together as follows:
//  - A==1: The choice of character set encoding MUST be Unicode.
//  - A==0 and B==1: The choice of character set encoding MUST be OEM.
//  - A==0 and B==0: The protocol MUST return SEC_E_INVALID_TOKEN.

    NTLMSSP_NEGOTIATE_UNICODE = 0x00000001, /* A   (31) */
};

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
    uint32_t flags{0};          /* 4 Bytes */
    NtlmNegotiateFlags() = default;
};

struct NtlmField {
//    uint16_t len{0};         /* 2 Bytes */ // becomes size of the vector
//    uint16_t maxLen{0};      /* 2 Bytes */ // dropped
    uint32_t bufferOffset{0};  /* 4 Bytes */
    std::vector<uint8_t> buffer;

    NtlmField() = default;
};

inline void logNtlmField(const char * name, const NtlmField & self) {
    LOG(LOG_DEBUG, "Field %s, len: %lu, maxlen: %lu, offset: %u",
        name, self.buffer.size(), self.buffer.size(), self.bufferOffset);
    hexdump_d(self.buffer);
}

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


// 2.2.2.4   LMv2_RESPONSE
// =================================================
// The LMv2_RESPONSE structure defines the NTLM v2 authentication LmChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is used only when NTLM v2 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      ChallengeFromClient                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (16 bytes):  A 16-byte array of unsigned char that contains the client's LM
//   challenge-response. This is the portion of the LmChallengeResponse field to which the
//   HMAC_MD5 algorithm has been applied, as defined in section 3.3.2. Specifically, Response
//   corresponds to the result of applying the HMAC_MD5 algorithm, using the key
//   ResponseKeyLM, to a message consisting of the concatenation of the ResponseKeyLM,
//   ServerChallenge and ClientChallenge.
//  ChallengeFromClient (8 bytes):  An 8-byte array of unsigned char that contains the client's
//   ClientChallenge, as defined in section 3.1.5.1.2.

inline array_view_const_u8 lmv2_response(std::vector<uint8_t> & buffer)
{
    return {buffer.data(), 16};
}

inline array_view_const_u8 lmv2_client_challenge(std::vector<uint8_t> & buffer)
{
    return {buffer.data() + 16, 8};
}

inline array_md5 compute_LMv2_Response(array_view_const_u8 responseKeyLM, array_view_const_u8 serverChallenge, array_view_const_u8 clientChallenge)
{
    return ::HmacMd5(responseKeyLM, serverChallenge, clientChallenge);
}


// [MS-NLMP]
// 2.2.1.3   AUTHENTICATE_MESSAGE
// ===================================================
// The AUTHENTICATE_MESSAGE defines an NTLM authenticate message that is sent from the client
//  to the server after the CHALLENGE_MESSAGE (section 2.2.1.2) is processed by the client.

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
// |                   LmChallengeResponseFields                   |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                   NtChallengeResponseFields                   |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                        DomainNameFields                       |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                         UserNameFields                        |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                        WorkstationFields                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                EncryptedRandomSessionKeyFields                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                         NegotiateFlags                        |
// +---------------+---------------+---------------+---------------+
// |                            Version                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              MIC                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
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
//  type. This field MUST be set to 0x00000003.

// LmChallengeResponseFields (8 bytes):  If the client chooses not to send an
//  LmChallengeResponse to the server:
//  - LmChallengeResponseLen and LmChallengeResponseMaxLen MUST be set to zero on
//     transmission.
//  - LmChallengeResponseBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the LmChallengeResponse would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    LmChallengeResponseLen     |   LmChallengeResponseMaxLen   |
// +---------------+---------------+---------------+---------------+
// |                LmChallengeResponseBufferOffset                |
// +---------------+---------------+---------------+---------------+
//    LmChallengeResponseLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of LmChallengeResponse in Payload.
//    LmChallengeResponseMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of LmChallengeResponseLen and MUST be ignored on receipt.
//    LmChallengeResponseBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     LmChallengeResponse in Payload.

// NtChallengeResponseFields (8 bytes):  If the client chooses not to send an
//  NtChallengeResponse to the server:
//  - NtChallengeResponseLen and NtChallengeResponseMaxLen MUST be set to zero on
//     transmission.
//  - NtChallengeResponseBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the NtChallengeResponse would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    NtChallengeResponseLen     |   NtChallengeResponseMaxLen   |
// +---------------+---------------+---------------+---------------+
// |                NtChallengeResponseBufferOffset                |
// +---------------+---------------+---------------+---------------+
//    NtChallengeResponseLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of NtChallengeResponse in Payload.
//    NtChallengeResponseMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of NtChallengeResponseLen and MUST be ignored on receipt.
//    NtChallengeResponseBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     NtChallengeResponse in Payload.

// DomainNameFields (8 bytes):  If the client chooses not to send an
//  DomainName to the server:
//  - DomainNameLen and DomainNameMaxLen MUST be set to zero on transmission.
//  - DomainNameBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the DomainName would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         DomainNameLen         |        DomainNameMaxLen       |
// +---------------+---------------+---------------+---------------+
// |                    DomainNameBufferOffset                     |
// +---------------+---------------+---------------+---------------+
//    DomainNameLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of DomainName in Payload, not including a nullptr terminator.
//    DomainNameMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of DomainNameLen and MUST be ignored on receipt.
//    DomainNameBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     DomainName in Payload.
//     If DomainName is a Unicode string, the values of DomainNameBufferOffset
//     and DomainNameLen MUST be multiples of 2.

// UserNameFields (8 bytes):  If the client chooses not to send an
//  UserName to the server:
//  - UserNameLen and UserNameMaxLen MUST be set to zero on transmission.
//  - UserNameBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the UserName would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          UserNameLen          |         UserNameMaxLen        |
// +---------------+---------------+---------------+---------------+
// |                     UserNameBufferOffset                      |
// +---------------+---------------+---------------+---------------+
//    UserNameLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of UserName in Payload, not including a nullptr terminator.
//    UserNameMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of UserNameLen and MUST be ignored on receipt.
//    UserNameBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     UserName in Payload.
//     If UserName is a Unicode string, the values of UserNameBufferOffset
//     and UserNameLen MUST be multiples of 2.

// WorkstationFields (8 bytes):  If the client chooses not to send an
//  Workstation to the server:
//  - WorkstationLen and WorkstationMaxLen MUST be set to zero on transmission.
//  - WorkstationBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the Workstation would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        WorkstationLen         |        WorkstationMaxLen      |
// +---------------+---------------+---------------+---------------+
// |                    WorkstationBufferOffset                    |
// +---------------+---------------+---------------+---------------+
//    WorkstationLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of Workstation in Payload, not including a nullptr terminator.
//    WorkstationMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of WorkstationLen and MUST be ignored on receipt.
//    WorkstationBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     Workstation in Payload.
//     If Workstation is a Unicode string, the values of WorkstationBufferOffset
//     and WorkstationLen MUST be multiples of 2.

// EncryptedRandomSessionKeyFields (8 bytes):  If the NTLMSSP_NEGOTIATE_KEY_EXCH flag
//  is not set in NegotiateFlags, indicating that no EncryptedRandomSessionKey is supplied:
//  - EncryptedRandomSessionKeyLen and EncryptedRandomSessionKeyMaxLen
//     SHOULD be set to zero on transmission.
//  - EncryptedRandomSessionKeyBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the EncryptedRandomSessionKey
//     would be in Payload if it was present.
//  - EncryptedRandomSessionKeyLen, EncryptedRandomSessionKeyMaxLen and
//     EncryptedRandomSessionKeyBufferOffset MUST be ignored on receipt.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  EncryptedRandomSessionKeyLen |EncryptedRandomSessionKeyMaxLen|
// +---------------+---------------+---------------+---------------+
// |               EncryptedRandomSessionBufferOffset              |
// +---------------+---------------+---------------+---------------+
//    EncryptedRandomSessionKeyLen (2 bytes):  A 16-bit unsigned integer that defines
//     the size, in bytes, of EncryptedRandomSessionKey in Payload.
//    EncryptedRandomSessionKeyMaxLen (2 bytes):  A 16-bit unsigned integer that
//     SHOULD be set to the value of EncryptedRandomSessionKeyLen and MUST be
//     ignored on receipt.
//    EncryptedRandomSessionKeyBufferOffset (4 bytes):  A 32-bit unsigned integer that
//     defines the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     EncryptedRandomSessionKey in Payload.

// NegotiateFlags (4 bytes):  In connectionless mode, a NEGOTIATE structure that contains
//  a set of bit flags (section 2.2.2.5) and represents the conclusion of negotiation.
//  the choices the client has made from the options the server offered in the
//  CHALLENGE_MESSAGE. In connection-oriented mode, a NEGOTIATE structure that contains
//  the set of bit flags (section 2.2.2.5) negotiated in the previous messages.

// Version (8 bytes):  A VERSION structure (section 2.2.2.10) that is present only when
//  the NTLMSSP_NEGOTIATE_VERSION flag is set in the NegotiateFlags field. This structure
//  is used for debugging purposes only. In normal protocol messages, it is ignored and
//  does not affect the NTLM message processing.

// MIC (16 bytes):  The message integrity for the NTLM NEGOTIATE_MESSAGE,
//  CHALLENGE_MESSAGE, and AUTHENTICATE_MESSAGE.

// Payload (variable):  A byte array that contains the data referred to by the
//  LmChallengeResponseBufferOffset, NtChallengeResponseBufferOffset,
//  DomainNameBufferOffset, UserNameBufferOffset, WorkstationBufferOffset, and
//  EncryptedRandomSessionKeyBufferOffset message fields. Payload data can be present
//  in any order within the Payload field, with variable-length padding before or after
//  the data. The data that can be present in the Payload field of this message,
//  in no particular order, are:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 LmChallengeResponse (Variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                 NtChallengeResponse (Variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      DomainName (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                       UserName (Variable)                     |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                     Workstation (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |              EncryptedRandomSessionKey (Variable)             |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  LmChallengeResponse (variable):  An LM_RESPONSE or LMv2_RESPONSE structure that
//   contains the computed LM response to the challenge. If NTLM v2 authentication is
//   configured, LmChallengeResponse MUST be an LMv2_RESPONSE structure (section
//   2.2.2.4). Otherwise, it MUST be an LM_RESPONSE structure (section 2.2.2.3).
//  NtChallengeResponse (variable):  An NTLM_RESPONSE or NTLMv2_RESPONSE
//   structure that contains the computed NT response to the challenge. If NTLM v2
//   authentication is configured, NtChallengeResponse MUST be an NTLMv2_RESPONSE
//   (section 2.2.2.8). Otherwise, it MUST be an NTLM_RESPONSE structure (section
//   2.2.2.6).
//  DomainName (variable):  The domain or computer name hosting the user account.
//   DomainName MUST be encoded in the negotiated character set.
//  UserName (variable):  The name of the user to be authenticated. UserName MUST be
//   encoded in the negotiated character set.
//  Workstation (variable):  The name of the computer to which the user is logged on.
//   Workstation MUST be encoded in the negotiated character set.
//  EncryptedRandomSessionKey (variable):  The client's encrypted random session key.
//   EncryptedRandomSessionKey and its usage are defined in sections 3.1.5 and 3.2.5.


struct NTLMAuthenticateMessage {
    NtlmField LmChallengeResponse;        /* 8 Bytes */
    NtlmField NtChallengeResponse;        /* 8 Bytes */
    NtlmField DomainName;                 /* 8 Bytes */
    NtlmField UserName;                   /* 8 Bytes */
    NtlmField Workstation;                /* 8 Bytes */
    NtlmField EncryptedRandomSessionKey;  /* 8 Bytes */
    NtlmNegotiateFlags negoFlags;         /* 4 Bytes */
    NtlmVersion version;                  /* 8 Bytes */
    uint8_t MIC[16]{};                      /* 16 Bytes */
    bool has_mic{true};
    uint32_t PayloadOffset;
    std::vector<uint8_t> message_bytes_dump;

    NTLMAuthenticateMessage()
        : PayloadOffset(12+8+8+8+8+8+8+4+8)
    {
        memset(this->MIC, 0x00, 16);
    }
    
    std::vector<uint8_t> get_bytes() 
    {
        return this->message_bytes_dump;
    }
    
    array_md5 NTOWFv2(array_view_const_u8 hash) {
        auto userup = UTF16_to_upper(this->UserName.buffer);
        return HmacMd5(hash, userup, this->DomainName.buffer);
    }
    
    array_md5 compute_session_base_key(array_view_const_u8 hash) {
        array_view_u8 NtProofStr{this->NtChallengeResponse.buffer.data(), 16};
        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain), NtProofStr)
        return HmacMd5(this->NTOWFv2(hash), NtProofStr);
    }

    bool check_nt_response_from_authenticate(array_view_const_u8 hash, array_view_const_u8 server_challenge) {
        auto & AuthNtResponse = this->NtChallengeResponse.buffer;
        array_md5 NtProofStr = HmacMd5(this->NTOWFv2(hash), server_challenge, {AuthNtResponse.data()+16, AuthNtResponse.size()-16});

        array_md5 NtProofStr_from_msg;
        memcpy(NtProofStr_from_msg.data(), AuthNtResponse.data(), NtProofStr_from_msg.size()); 

        return NtProofStr_from_msg == NtProofStr;
    }

    bool check_lm_response_from_authenticate(array_view_const_u8 hash, array_view_const_u8 server_challenge) 
    {
        if (this->LmChallengeResponse.buffer.size() != 24) {
            return false;
        }
        auto a = compute_LMv2_Response(this->NTOWFv2(hash), server_challenge,
                         lmv2_client_challenge(this->LmChallengeResponse.buffer));

        auto b = lmv2_response(this->LmChallengeResponse.buffer);
        return are_buffer_equal(a, b);
    }
    
    array_md5 get_exported_session_key(array_view_const_u8 sessionBaseKey)
    {
        return Rc4Key(sessionBaseKey, this->EncryptedRandomSessionKey.buffer); 
    }
};

inline void logNtlmFlags(uint32_t flags)
{
    LOG(LOG_INFO, "negotiateFlags \"0x%08X\"{", flags);

    for (int i = 0; i < 32; i++) {
        if ((flags >> (31-i)) & 1) {
            const char* str = NTLM_NEGOTIATE_STRINGS[i];
            LOG(LOG_INFO, "    |%s, // (%d)", str, (31-i));
        }
    }
    LOG(LOG_INFO, "}");
}

inline void logNTLMAuthenticateMessage(NTLMAuthenticateMessage & self) 
{
    logNtlmField("LmChallengeResponse", self.LmChallengeResponse);
    logNtlmField("NtChallengeResponse", self.NtChallengeResponse);
    logNtlmField("DomainName", self.DomainName);
    logNtlmField("UserName", self.UserName);
    logNtlmField("Workstation", self.Workstation);
    logNtlmField("EncryptedRandomSessionKey", self.EncryptedRandomSessionKey);
    
    logNtlmFlags(self.negoFlags.flags);
    
    LogNtlmVersion(self.version);
    LOG(LOG_DEBUG, "MIC");
    hexdump_d(self.MIC, 16);
}

inline std::vector<uint8_t> emitNTLMAuthenticateMessageNew(uint32_t negoFlags, 
                                        bytes_view LmChallengeResponse,
                                        bytes_view NtChallengeResponse,
                                        bytes_view DomainName,
                                        bytes_view UserName,
                                        bytes_view Workstation,
                                        bytes_view EncryptedRandomSessionKey,
                                        bytes_view MIC, bool has_mic, size_t & mic_offset)
{
    uint32_t payloadOffset = 12+8+8+8+8+8+8+4
                            +8*bool(negoFlags & NTLMSSP_NEGOTIATE_VERSION)
                            +16*has_mic;

    size_t message_size = payloadOffset 
                        + LmChallengeResponse.size()
                        + NtChallengeResponse.size()
                        + DomainName.size()
                        + UserName.size()
                        + Workstation.size()
                        + EncryptedRandomSessionKey.size()
                        ;
    if (has_mic) {
        mic_offset = payloadOffset-16;
    }

    std::vector<uint8_t> result(message_size);
    OutStream stream(result);

    stream.out_copy_bytes("NTLMSSP\0"_av);
    stream.out_uint32_le(NtlmAuthenticate);

    struct TmpNtlmField {
        uint16_t offset;
        bytes_view f;
    };

    std::array<TmpNtlmField, 6> l{{
         {0, LmChallengeResponse},
         {0, NtChallengeResponse},
         {0, DomainName},
         {0, UserName},
         {0, Workstation},
         {0, EncryptedRandomSessionKey}}};

    for (auto field: l){
        stream.out_uint16_le(field.f.size());
        stream.out_uint16_le(field.f.size());
        stream.out_uint32_le(payloadOffset);
        payloadOffset += field.f.size();
        field.offset = payloadOffset;
    }

// Check that when reading buffer
//    if (self.LmChallengeResponse.buffer.size() != 24) {
//        // This is some message format error
//    }

    stream.out_uint32_le(negoFlags);
    if (negoFlags & NTLMSSP_NEGOTIATE_VERSION) {
        stream.out_uint8(WINDOWS_MAJOR_VERSION_6);
        stream.out_uint8(WINDOWS_MINOR_VERSION_1);
        stream.out_uint16_le(7601);
        stream.out_clear_bytes(3);
        stream.out_uint8(NTLMSSP_REVISION_W2K3);
    }


    if (has_mic) {
        stream.out_clear_bytes(16);
    }

    // PAYLOAD
    for (auto field: l){
        stream.out_copy_bytes(field.f);
    }

    return result;
}

inline void emitNTLMAuthenticateMessage(OutStream & stream, uint32_t negoFlags, 
                                        bytes_view LmChallengeResponse,
                                        bytes_view NtChallengeResponse,
                                        bytes_view DomainName,
                                        bytes_view UserName,
                                        bytes_view Workstation,
                                        bytes_view EncryptedRandomSessionKey,
                                        bytes_view MIC, bool has_mic, bool ignore_mic)
//                                         NTLMAuthenticateMessage & self, bool ignore_mic)
{
    uint32_t payloadOffset = 12+8+8+8+8+8+8+4
                            +8*bool(negoFlags & NTLMSSP_NEGOTIATE_VERSION)
                            +16*has_mic;

    stream.out_copy_bytes("NTLMSSP\0"_av);
    stream.out_uint32_le(NtlmAuthenticate);

    struct TmpNtlmField {
        uint16_t offset;
        bytes_view f;
    };

    std::array<TmpNtlmField, 6> l{{
         {0, LmChallengeResponse},
         {0, NtChallengeResponse},
         {0, DomainName},
         {0, UserName},
         {0, Workstation},
         {0, EncryptedRandomSessionKey}}};

    for (auto field: l){
        stream.out_uint16_le(field.f.size());
        stream.out_uint16_le(field.f.size());
        stream.out_uint32_le(payloadOffset);
        payloadOffset += field.f.size();
        field.offset = payloadOffset;
    }

// Check that when reading buffer
//    if (self.LmChallengeResponse.buffer.size() != 24) {
//        // This is some message format error
//    }

    stream.out_uint32_le(negoFlags);
    if (negoFlags & NTLMSSP_NEGOTIATE_VERSION) {
        stream.out_uint8(WINDOWS_MAJOR_VERSION_6);
        stream.out_uint8(WINDOWS_MINOR_VERSION_1);
        stream.out_uint16_le(7601);
        stream.out_clear_bytes(3);
        stream.out_uint8(NTLMSSP_REVISION_W2K3);
    }

    if (has_mic) {
        if (ignore_mic) {
            stream.out_clear_bytes(16);
        }
        else {
            stream.out_copy_bytes(MIC);
        }
    }

    // PAYLOAD
    for (auto field: l){
        stream.out_copy_bytes(field.f);
    }

//    LOG(LOG_INFO, "NTLM Message Authenticate Dump (Sent)");
//    hexdump_d(stream.get_bytes());
}


inline void recvNTLMAuthenticateMessage(InStream & stream, NTLMAuthenticateMessage & self) {
//    LOG(LOG_INFO, "NTLM Message Authenticate Dump (Recv)");
//    hexdump_d(stream.remaining_bytes());

    uint8_t const * pBegin = stream.get_current();
    
    // Read Message Header
    constexpr auto sig_len = sizeof(NTLM_MESSAGE_SIGNATURE);
    uint8_t received_sig[sig_len];
    stream.in_copy_bytes(received_sig, sig_len);
    uint32_t type = stream.in_uint32_le();
    if (NtlmAuthenticate != type){
        LOG(LOG_ERR, "INVALID MSG RECEIVED expected NtlmAuthenticate (0003), got type: %u", type);
    }
    if (0 != memcmp(NTLM_MESSAGE_SIGNATURE, received_sig, sig_len)){
        LOG(LOG_ERR, "INVALID MSG RECEIVED BAD SIGNATURE (NtlmAuthenticate)");
    }

    // Read Ntlm Fields Headers
    struct TmpNtlmField {
        uint16_t len;
        NtlmField * f;
    };

    std::array<TmpNtlmField, 6> l{{
        {0, &self.LmChallengeResponse},
        {0, &self.NtChallengeResponse},
        {0, &self.DomainName},
        {0, &self.UserName},
        {0, &self.Workstation},
        {0, &self.EncryptedRandomSessionKey}}};

    uint32_t min_offset = stream.get_current()+stream.in_remain() - pBegin;
    for (auto & tmp: l){
        tmp.len = stream.in_uint16_le();
        auto max_len = stream.in_uint16_le();
        (void)max_len; // TODO: we should check that max_len is equal to len
        auto offset = stream.in_uint32_le();
        min_offset = std::min(min_offset, offset);
        tmp.f->bufferOffset = offset;
    }

    // Read Flags
    self.negoFlags.flags = stream.in_uint32_le();
    if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
        self.version.ProductMajorVersion = static_cast<::ProductMajorVersion>(stream.in_uint8());
        self.version.ProductMinorVersion = static_cast<::ProductMinorVersion>(stream.in_uint8());
        self.version.ProductBuild = stream.in_uint16_le();
        stream.in_skip_bytes(3);
        self.version.NtlmRevisionCurrent = static_cast<::NTLMRevisionCurrent>(stream.in_uint8());
    }

    // Read Mic is there is a gap between headers and payload
    // TODO: don't we have a flag to know if we should read MIC ?
    self.has_mic = pBegin + min_offset > stream.get_current();
    if (self.has_mic){
        stream.in_copy_bytes(self.MIC, 16);
    }

    // PAYLOAD
    // Ensure payload is available
    auto maxp = std::accumulate(l.begin(), l.end(), 0, 
        [](size_t a, const TmpNtlmField tmp) {
             return std::max(a, size_t(tmp.f->bufferOffset + tmp.len));
    });
    if (pBegin + maxp > stream.get_current()) {
        stream.in_skip_bytes(pBegin + maxp - stream.get_current());
    }

    // Actually read payload data
    for(auto & tmp: l){
        tmp.f->buffer.assign(pBegin + tmp.f->bufferOffset, 
                             pBegin + tmp.f->bufferOffset + tmp.len);
    }

    if (self.has_mic){
        // Store message bytes for later reference
        std::vector<uint8_t> v;
        constexpr std::size_t null_data_sz = 16;
        uint8_t const null_data[null_data_sz]{0u};
        push_back_array(v, {stream.get_data(), stream.get_data()+12+8+8+8+8+8+8+4+8});
        push_back_array(v, {null_data, 16});
        push_back_array(v, {stream.get_data() + 12+8+8+8+8+8+8+4+8 + 16, stream.get_offset() - (12+8+8+8+8+8+8+4+8 + 16)});
        self.message_bytes_dump = v;
    }
}

// 2.2.2.3   LM_RESPONSE
// ====================================
// The LM_RESPONSE structure defines the NTLM v1 authentication LmChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is used only when NTLM v1 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (24 bytes):  A 24-byte array of unsigned char that contains the client's
//   LmChallengeResponse as defined in section 3.3.1.

struct LM_Response {
    uint8_t response[24];
};




// 2.2.2.6   NTLM v1 Response: NTLM_RESPONSE
// ===============================================
// The NTLM_RESPONSE structure defines the NTLM v1 authentication NtChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is only used when NTLM v1 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (24 bytes):  A 24-byte array of unsigned char that contains the client's
//   NtChallengeResponse (section 3.3.1).
struct NTLM_Response {
    uint8_t response[32];
};


// 2.2.2.7   NTLM v2: NTLMv2_CLIENT_CHALLENGE
// ===============================================
// The NTLMv2_CLIENT_CHALLENGE structure defines the client challenge in the
// AUTHENTICATE_MESSAGE. This structure is used only when NTLM v2 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   RespType    |   HiRespType  |          Reserved1            |
// +---------------+---------------+---------------+---------------+
// |                           Reserved2                           |
// +---------------+---------------+---------------+---------------+
// |                           TimeStamp                           |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      ChallengeFromClient                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                           Reserved3                           |
// +---------------+---------------+---------------+---------------+
// |                       AvPairs (variable)                      |
// +---------------+---------------+---------------+---------------+
//  RespType (1 byte):  An 8-bit unsigned char that contains the current version of the challenge
//   response type. This field MUST be 0x01.
//  HiRespType (1 byte):  An 8-bit unsigned char that contains the maximum supported version of
//   the challenge response type. This field MUST be 0x01.
//  Reserved1 (2 bytes):  A 16-bit unsigned integer that SHOULD be 0x0000 and MUST be ignored
//   on receipt.
//  Reserved2 (4 bytes):  A 32-bit unsigned integer that SHOULD be 0x00000000 and MUST be
//   ignored on receipt.
//  TimeStamp (8 bytes):  A 64-bit unsigned integer that contains the current system time,
//   represented as the number of 100 nanosecond ticks elapsed since midnight of January 1,
//   1601 (UTC).
//  ChallengeFromClient (8 bytes):  An 8-byte array of unsigned char that contains the client's
//   ClientChallenge (section 3.1.5.1.2).
//  Reserved3 (4 bytes):  A 32-bit unsigned integer that SHOULD be 0x00000000 and MUST be
//   ignored on receipt.
//  AvPairs (variable):  A byte array that contains a sequence of AV_PAIR structures (section
//   2.2.2.1). The sequence contains the server-naming context and is terminated by an AV_PAIR
//   structure with an AvId field of MsvAvEOL.


struct NTLMv2_Client_Challenge {
    uint8_t  RespType;              // MUST BE 0x01
    uint8_t  HiRespType;            // MUST BE 0x01
    uint16_t Reserved1;             // MUST BE 0x00
    uint32_t Reserved2;             // MUST BE 0x00
    uint8_t  Timestamp[8]{};          // Current system time
    array_challenge ClientChallenge;    // Client Challenge
    uint32_t Reserved3;             // MUST BE 0x00
    NtlmAvPairList AvPairList;
    uint32_t Reserved4;             // MUST BE 0x00

    NTLMv2_Client_Challenge() = default;
};

inline void EmitNTLMv2_Client_Challenge(OutStream & stream, NTLMv2_Client_Challenge & self)
{
    self.RespType = 0x01;
    self.HiRespType = 0x01;
    stream.out_uint8(self.RespType);
    stream.out_uint8(self.HiRespType);
    stream.out_clear_bytes(2);
    stream.out_clear_bytes(4);
    stream.out_copy_bytes(self.Timestamp, 8);
    stream.out_copy_bytes(self.ClientChallenge);
    stream.out_clear_bytes(4);

    for (auto & avp: self.AvPairList) {
        stream.out_uint16_le(avp.id);
        stream.out_uint16_le(avp.data.size());
        stream.out_copy_bytes(avp.data);
    }
    stream.out_uint16_le(MsvAvEOL);
    stream.out_uint16_le(0);

    stream.out_clear_bytes(4);
}

inline void RecvNTLMv2_Client_Challenge(InStream & stream, NTLMv2_Client_Challenge & self)
{
    // size_t size;
    self.RespType = stream.in_uint8();
    self.HiRespType = stream.in_uint8();
    stream.in_skip_bytes(2);
    stream.in_skip_bytes(4);
    stream.in_copy_bytes(self.Timestamp, 8);
    stream.in_copy_bytes(self.ClientChallenge);
    stream.in_skip_bytes(4);

    for (std::size_t i = 0; i < AV_ID_MAX; ++i) {
        auto id = stream.in_uint16_le();
        auto length = stream.in_uint16_le();
        if (id == MsvAvEOL) {
            // ASSUME last element is MsvAvEOL
            stream.in_skip_bytes(length);
            break;
        }
        self.AvPairList.push_back({static_cast<NTLM_AV_ID>(id), stream.in_copy_bytes_as_vector(length)});
    }

    stream.in_skip_bytes(4);
}


// 2.2.2.8   NTLM2 V2 Response: NTLMv2_RESPONSE
// ==================================================
// The NTLMv2_RESPONSE structure defines the NTLMv2 authentication NtChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is used only when NTLMv2 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                 ChallengeFromClient (variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (16 bytes):  A 16-byte array of unsigned char that contains the client's NT
//  challenge-response as defined in section 3.3.2. Response corresponds to the NTProofStr
//   variable from section 3.3.2.
//  ChallengeFromClient (variable):  A variable-length byte array that contains the
//   ClientChallenge as defined in section 3.3.2. ChallengeFromClient corresponds to the
//   temp variable from section 3.3.2.

struct NTLMv2_Response {
    uint8_t Response[16]{};
    NTLMv2_Client_Challenge Challenge;

    NTLMv2_Response() = default;

    void emit(OutStream & stream) /* TODO const*/ {
        stream.out_copy_bytes(this->Response, 16);
        EmitNTLMv2_Client_Challenge(stream, this->Challenge);
    }

    void recv(InStream & stream) {
        stream.in_copy_bytes(this->Response, 16);
        RecvNTLMv2_Client_Challenge(stream, this->Challenge);
    }

};

// [MS-NLMP]

// 2.2.1.2   CHALLENGE_MESSAGE
// ======================================================================
// The CHALLENGE_MESSAGE defines an NTLM challenge message that is sent from
//  the server to the client. The CHALLENGE_MESSAGE is used by the server to
//  challenge the client to prove its identity.
//  For connection-oriented requests, the CHALLENGE_MESSAGE generated by the
//  server is in response to the NEGOTIATE_MESSAGE (section 2.2.1.1) from the client.

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
// |                        TargetNameFields                       |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                         NegotiateFlags                        |
// +---------------+---------------+---------------+---------------+
// |                         ServerChallenge                       |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                           Reserved                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                        TargetInfoFields                       |
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
//  type. This field MUST be set to 0x00000002.

// TargetNameFields (8 bytes):  If the NTLMSSP_REQUEST_TARGET flag is not set in
//  NegotiateFlags, indicating that no TargetName is required:
//  - TargetNameLen and TargetNameMaxLen SHOULD be set to zero on transmission.
//  - TargetNameBufferOffset field SHOULD be set to the offset from the beginning of the
//     CHALLENGE_MESSAGE to where the TargetName would be in Payload if it were present.
//  - TargetNameLen, TargetNameMaxLen, and TargetNameBufferOffset MUST be ignored
//     on receipt.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        TargetNameLen          |       TargetNameMaxLen        |
// +---------------+---------------+---------------+---------------+
// |                     TargetNameBufferOffset                    |
// +---------------+---------------+---------------+---------------+
//    TargetNameLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of TargetName in Payload.
//    TargetNameMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be set
//     to the value of TargetNameLen and MUST be ignored on receipt.
//    TargetNameBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the CHALLENGE_MESSAGE to
//     TargetName in Payload.
//     If TargetName is a Unicode string, the values of TargetNameBufferOffset and
//     TargetNameLen MUST be multiples of 2.

// NegotiateFlags (4 bytes):  A NEGOTIATE structure that contains a set of bit flags,
//  as defined in section 2.2.2.5. The server sets flags to indicate options it supports or,
//  if thre has been a NEGOTIATE_MESSAGE (section 2.2.1.1), the choices it has made from
//   the options offered by the client.

// ServerChallenge (8 bytes):  A 64-bit value that contains the NTLM challenge.
//  The challenge is a 64-bit nonce. The processing of the ServerChallenge is specified
//  in sections 3.1.5 and 3.2.5.
// Reserved (8 bytes):  An 8-byte array whose elements MUST be zero when sent and MUST be
//  ignored on receipt.

// TargetInfoFields (8 bytes):  If the NTLMSSP_NEGOTIATE_TARGET_INFO flag of
//  NegotiateFlags is clear, indicating that no TargetInfo is required:
//  - TargetInfoLen and TargetInfoMaxLen SHOULD be set to zero on transmission.
//  - TargetInfoBufferOffset field SHOULD be set to the offset from the beginning of the
//    CHALLENGE_MESSAGE to where the TargetInfo would be in Payload if it were present.
//  - TargetInfoLen, TargetInfoMaxLen, and TargetInfoBufferOffset MUST be ignored on receipt.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        TargetInfoLen          |       TargetInfoMaxLen        |
// +---------------+---------------+---------------+---------------+
// |                     TargetInfoBufferOffset                    |
// +---------------+---------------+---------------+---------------+
//    TargetInfoLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of TargetInfo in Payload.
//    TargetInfoMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be set
//     to the value of TargetInfoLen and MUST be ignored on receipt.
//    TargetInfoBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the CHALLENGE_MESSAGE to
//     TargetInfo in Payload.

// Version (8 bytes):  A VERSION structure (as defined in section 2.2.2.10) that
//  is present only when the NTLMSSP_NEGOTIATE_VERSION flag is set in the
//  NegotiateFlags field. This structure is used for debugging purposes only.
//  In normal (non-debugging) protocol messages, it is ignored and does not affect
//  the NTLM message processing.


// Payload (variable):  A byte-array that contains the data referred to by the
//   TargetNameBufferOffset and TargetInfoBufferOffset message fields. Payload data
//   can be present in any order within the Payload field, with variable-length padding
//   before or after the data. The data that can be present in the Payload field of
//   this message, in no particular order, are:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      TargetName (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      TargetInfo (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//    TargetName (variable):  If TargetNameLen does not equal 0x0000, TargetName
//     MUST be a byte-array that contains the name of the server authentication realm,
//     and MUST be expressed in the negotiated character set. A server that is a
//     member of a domain returns the domain of which it is a member, and a server
//     that is not a member of a domain returns the server name.
//    TargetInfo (variable):  If TargetInfoLen does not equal 0x0000, TargetInfo
//     MUST be a byte array that contains a sequence of AV_PAIR structures.
//     The AV_PAIR structure is defined in section 2.2.2.1. The length of each
//     AV_PAIR is determined by its AvLen field (plus 4 bytes).

//     Note  An AV_PAIR structure can start on any byte alignment and the sequence of
//      AV_PAIRs has no padding between structures.
//     The sequence MUST be terminated by an AV_PAIR structure with an AvId field of
//      MsvAvEOL. The total length of the TargetInfo byte array is the sum of the lengths,
//      in bytes, of the AV_PAIR structures it contains.
//     Note  If a TargetInfo AV_PAIR Value is textual, it MUST be encoded in Unicode
//      irrespective of what character set was negotiated (section 2.2.2.1).


struct NTLMChallengeMessage
{
public:
    NtlmField TargetName;               /* 8 Bytes */
    NtlmNegotiateFlags negoFlags;       /* 4 Bytes */
    array_challenge serverChallenge;    /* 8 Bytes */
                                        /* 8 Bytes reserved */
    NtlmField TargetInfo;               /* 8 Bytes */
    NtlmVersion version;                /* 8 Bytes */
    NtlmAvPairList AvPairList;          // used to build TargetInfo payload
    std::vector<uint8_t> raw_bytes;
};


inline void EmitNTLMChallengeMessage(OutStream & stream, NTLMChallengeMessage & self)
{
        if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
            self.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            self.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            self.version.ProductBuild        = 7601;
            self.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
        }

        stream.out_copy_bytes(NTLM_MESSAGE_SIGNATURE, sizeof(NTLM_MESSAGE_SIGNATURE));
        stream.out_uint32_le(NtlmChallenge);

        uint32_t payloadOffset = 12+8+4+8+8+8 + 8*bool(self.negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION);

        stream.out_uint16_le(self.TargetName.buffer.size());
        stream.out_uint16_le(self.TargetName.buffer.size());
        stream.out_uint32_le(payloadOffset);
        payloadOffset += self.TargetName.buffer.size();
        
        stream.out_uint32_le(self.negoFlags.flags);
        stream.out_copy_bytes(self.serverChallenge);
        stream.out_clear_bytes(8);

        self.TargetInfo.buffer.clear();
        for (auto & avp: self.AvPairList) {
            push_back_array(self.TargetInfo.buffer, out_uint16_le(avp.id));
            push_back_array(self.TargetInfo.buffer, buffer_view(out_uint16_le(avp.data.size())));
            push_back_array(self.TargetInfo.buffer, avp.data);
        }
        push_back_array(self.TargetInfo.buffer, buffer_view(out_uint16_le(MsvAvEOL)));
        push_back_array(self.TargetInfo.buffer, buffer_view(out_uint16_le(0)));

        stream.out_uint16_le(self.TargetInfo.buffer.size());
        stream.out_uint16_le(self.TargetInfo.buffer.size());
        stream.out_uint32_le(payloadOffset);

        if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
            stream.out_uint8(self.version.ProductMajorVersion);
            stream.out_uint8(self.version.ProductMinorVersion);
            stream.out_uint16_le(self.version.ProductBuild);
            stream.out_clear_bytes(3);
            stream.out_uint8(self.version.NtlmRevisionCurrent);
        }
        // PAYLOAD
        stream.out_copy_bytes(self.TargetName.buffer);
        stream.out_copy_bytes(self.TargetInfo.buffer);
        
//        LOG(LOG_INFO, "NTLM Message Challenge Dump (Sent)");
//        hexdump_d(stream.get_bytes());
}


inline NTLMChallengeMessage recvNTLMChallengeMessage(bytes_view av)
{
    InStream stream(av);
    NTLMChallengeMessage self;
    self.raw_bytes.assign(av.begin(),av.end());

//    LOG(LOG_INFO, "NTLM Message Challenge Dump (Recv)");
//    hexdump_d(stream.remaining_bytes());

    uint8_t const * pBegin = stream.get_current();

    constexpr auto sig_len = sizeof(NTLM_MESSAGE_SIGNATURE);
    uint8_t received_sig[sig_len];
    stream.in_copy_bytes(received_sig, sig_len);
    uint32_t type = stream.in_uint32_le();
    (void)type;

    // Add Checks and throw Error if necessary

//    if (type != NtlmChallenge){
//        LOG(LOG_ERR, "INVALID MSG RECEIVED type: %u", type);
//    }
//    if (0 != memcmp(NTLM_MESSAGE_SIGNATURE, received_sig, sig_len)){
//        LOG(LOG_ERR, "INVALID MSG RECEIVED bad signature");
//    }
    
    uint16_t TargetName_len = stream.in_uint16_le();
    uint16_t TargetName_maxlen = stream.in_uint16_le();
    (void)TargetName_maxlen; // we should check it's the same as len
    self.TargetName.bufferOffset = stream.in_uint32_le();

    self.negoFlags.flags = stream.in_uint32_le();
    stream.in_copy_bytes(self.serverChallenge);
    // self.serverChallenge = stream.in_uint64_le();
    stream.in_skip_bytes(8);

    uint16_t TargetInfo_len = stream.in_uint16_le();
    uint16_t TargetInfo_maxlen = stream.in_uint16_le();
    (void)TargetInfo_maxlen; // we should check it's the same as len
    self.TargetInfo.bufferOffset = stream.in_uint32_le();

    if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
        self.version.ProductMajorVersion = static_cast<::ProductMajorVersion>(stream.in_uint8());
        self.version.ProductMinorVersion = static_cast<::ProductMinorVersion>(stream.in_uint8());
        self.version.ProductBuild = stream.in_uint16_le();
        stream.in_skip_bytes(3);
        self.version.NtlmRevisionCurrent = static_cast<::NTLMRevisionCurrent>(stream.in_uint8());
    }

    // PAYLOAD
    auto maxp = std::max(size_t(self.TargetName.bufferOffset + TargetName_len),
                         size_t(self.TargetInfo.bufferOffset + TargetInfo_len));
    
    if (pBegin + maxp > stream.get_current()) {
        stream.in_skip_bytes(pBegin + maxp - stream.get_current());
    }

    self.TargetName.buffer.assign(pBegin + self.TargetName.bufferOffset, 
                         pBegin + self.TargetName.bufferOffset + TargetName_len);

    self.TargetInfo.buffer.assign(pBegin + self.TargetInfo.bufferOffset, 
                         pBegin + self.TargetInfo.bufferOffset + TargetInfo_len);
    
    InStream in_stream(self.TargetInfo.buffer);
    
    for (std::size_t i = 0; i < AV_ID_MAX; ++i) {
        auto id = in_stream.in_uint16_le();
        auto length = in_stream.in_uint16_le();
        if (id == MsvAvEOL) {
            // ASSUME last element is MsvAvEOL
            in_stream.in_skip_bytes(length);
            break;
        }
        auto v = in_stream.in_copy_bytes_as_vector(length);
        self.AvPairList.push_back({static_cast<NTLM_AV_ID>(id), v});
    }
    return self;
}

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


struct NTLMNegotiateMessage
{
    NtlmNegotiateFlags negoFlags; /* 4 Bytes */
    NtlmField DomainName;         /* 8 Bytes */
    NtlmField Workstation;        /* 8 Bytes */
    NtlmVersion version;          /* 8 Bytes */
    
    std::vector<uint8_t> raw_bytes;
};


inline void logNTLMNegotiateMessage(const NTLMNegotiateMessage & message)
{
    LOG(LOG_INFO, "NTLMSSP: Negotiate");

    logNtlmField("DomainName", message.DomainName);
    logNtlmField("Workstation", message.Workstation);
    
    logNtlmFlags(message.negoFlags.flags);
    
    LogNtlmVersion(message.version);
}

inline std::vector<uint8_t> emitNTLMNegotiateMessage()
{
    std::vector<uint8_t> DomainName;
    std::vector<uint8_t> Workstation;

    const uint32_t payloadOffset = 8+  // message signature 
                             4+  // MessageType = NtlmNegotiate
                             4+  // negoFlags
                             8+  // DomainName field header
                             8+  // Workstation field header
                             8   // Negotiate Version
                             ;
    std::vector<uint8_t> message(payloadOffset+DomainName.size()+Workstation.size());
    
    OutStream stream(message);

    stream.out_copy_bytes("NTLMSSP\0"_av);
    stream.out_uint32_le(NtlmNegotiate);
    stream.out_uint32_le(
          NTLMSSP_NEGOTIATE_LM_KEY
        | NTLMSSP_NEGOTIATE_OEM
        | NTLMSSP_NEGOTIATE_56
        | NTLMSSP_NEGOTIATE_SEAL
        | NTLMSSP_NEGOTIATE_KEY_EXCH
        | NTLMSSP_NEGOTIATE_128
        | NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
        | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
        | NTLMSSP_NEGOTIATE_NTLM
        | NTLMSSP_NEGOTIATE_SIGN
        | NTLMSSP_REQUEST_TARGET
        | NTLMSSP_NEGOTIATE_UNICODE
        | NTLMSSP_NEGOTIATE_VERSION
        );
    stream.out_uint16_le(DomainName.size());
    stream.out_uint16_le(DomainName.size());
    stream.out_uint32_le(payloadOffset);

    stream.out_uint16_le(Workstation.size());
    stream.out_uint16_le(Workstation.size());
    stream.out_uint32_le(payloadOffset + DomainName.size());

    // Negotiate Version    
    stream.out_uint8(WINDOWS_MAJOR_VERSION_6);
    stream.out_uint8(WINDOWS_MINOR_VERSION_1);
    stream.out_uint16_le(7601);
    stream.out_clear_bytes(3);
    stream.out_uint8(NTLMSSP_REVISION_W2K3);

    // PAYLOAD
    stream.out_copy_bytes(DomainName);
    stream.out_copy_bytes(Workstation);
    
//    LOG(LOG_INFO, "NTLM Message Negotiate Dump (Sent)");
//    hexdump_d(stream.get_bytes());
    
    return message;
}

inline NTLMNegotiateMessage recvNTLMNegotiateMessage(bytes_view av)
{
    LOG(LOG_INFO, "recvNTLMNegotiateMessage full dump--------------------------------");
    hexdump_c(av);
    LOG(LOG_INFO, "recvNTLMNegotiateMessage hexdump end - START PARSING DATA-------------");

    InStream stream(av);
    NTLMNegotiateMessage self;
    self.raw_bytes.assign(av.begin(),av.end());
//    LOG(LOG_INFO, "NTLM Message Negotiate Dump (Recv)");
//    hexdump_c(stream.remaining_bytes());
    uint8_t const * pBegin = stream.get_current();

    auto signature = stream.view_bytes(sizeof(NTLM_MESSAGE_SIGNATURE));
    stream.in_skip_bytes(signature.size());
    // to check NTLM_MESSAGE_SIGNATURE
    //    LOG(LOG_ERR, "INVALID MSG RECEIVED bad signature");

    uint32_t type = stream.in_uint32_le();
    (void)type;
    // to check type == NtlmNegotiate
    //    LOG(LOG_ERR, "INVALID MSG RECEIVED NtlmNegotiate (0001) expected, got type: %u", type);

    self.negoFlags.flags = stream.in_uint32_le();
    
    uint16_t DomainName_len = stream.in_uint16_le();
    uint16_t DomainName_maxlen = stream.in_uint16_le();
    (void)DomainName_maxlen; // ensure it's identical to len
    // to check: DomainName_len == DomainName_maxlen
    if (not (self.negoFlags.flags & NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED)) {
        DomainName_maxlen = DomainName_len = 0;
    }
    self.DomainName.bufferOffset = stream.in_uint32_le();
    // to check: bufferOffset is inside stream, bufferOffset+len is inside stream


    uint16_t Workstation_len = stream.in_uint16_le();
    uint16_t Workstation_maxlen = stream.in_uint16_le();
    (void)Workstation_maxlen; // ensure it's identical to len
    // to check: DomainName_len == DomainName_maxlen
    if (not (self.negoFlags.flags & NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED)) {
        Workstation_maxlen = Workstation_len = 0;
    }
    self.Workstation.bufferOffset = stream.in_uint32_le();
    // to check: bufferOffset is inside stream, bufferOffset+len is inside stream
    

    if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
        self.version.ProductMajorVersion = static_cast<::ProductMajorVersion>(stream.in_uint8());
        self.version.ProductMinorVersion = static_cast<::ProductMinorVersion>(stream.in_uint8());
        self.version.ProductBuild = stream.in_uint16_le();
        stream.in_skip_bytes(3);
        self.version.NtlmRevisionCurrent = static_cast<::NTLMRevisionCurrent>(stream.in_uint8());
    }

    // PAYLOAD
    if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED) {
        self.DomainName.buffer.assign(pBegin + self.DomainName.bufferOffset, 
                         pBegin + self.DomainName.bufferOffset + DomainName_len);
    }

    if (self.negoFlags.flags & NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED) {
        self.Workstation.buffer.assign(pBegin + self.Workstation.bufferOffset, 
                             pBegin + self.Workstation.bufferOffset + Workstation_len);
    }
    // Consume Payload
    auto maxp = std::max(size_t(self.DomainName.bufferOffset + DomainName_len),
                         size_t(self.Workstation.bufferOffset + Workstation_len));
    
    if (pBegin + maxp > stream.get_current()) {
        stream.in_skip_bytes(pBegin + maxp - stream.get_current());
    }
    return self;
}


