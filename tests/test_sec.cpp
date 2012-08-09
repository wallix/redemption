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
   Author(s): Christophe Grosjean

   Unit test to Mcs PDU coder/decoder
   Using lib boost functions for testing
*/
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSec
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "RDP/sec.hpp"

BOOST_AUTO_TEST_CASE(TestSend_SecExchangePacket)
{
    BStream stream(1024);

    const char sec_pkt[] =
        "\x01\x00\x00\x00" // 0x00000001 = SEC_EXCHANGE_PKT
        "\x48\x00\x00\x00" // 0x00000048 = 72 (64 bytes key + 8 bytes padding)
        "\xca\xe7\xdf\x85\x01\x42\x02\x47\x28\xfc\x11\x97\x85\xa3\xf9\x40"
        "\x73\x97\x33\x2d\x9e\xe4\x0c\x8e\xe0\x97\xfc\x17\x24\x4e\x35\x33"
        "\xe0\x4e\x7d\xdc\x12\x1d\x41\xf1\xd8\x17\x86\x0e\x79\x9b\x4f\x44"
        "\xb2\x82\xf0\x93\x17\xf8\x59\xc9\x7b\xba\x2a\x22\x59\x45\xa7\x3a"
        "\x00\x00\x00\x00\x00\x00\x00\x00" // Padding
    ;

    uint8_t client_encrypted_key[] = {
        0xca, 0xe7, 0xdf, 0x85, 0x01, 0x42, 0x02, 0x47, 0x28, 0xfc, 0x11, 0x97, 0x85, 0xa3, 0xf9, 0x40,
        0x73, 0x97, 0x33, 0x2d, 0x9e, 0xe4, 0x0c, 0x8e, 0xe0, 0x97, 0xfc, 0x17, 0x24, 0x4e, 0x35, 0x33,
        0xe0, 0x4e, 0x7d, 0xdc, 0x12, 0x1d, 0x41, 0xf1, 0xd8, 0x17, 0x86, 0x0e, 0x79, 0x9b, 0x4f, 0x44,
        0xb2, 0x82, 0xf0, 0x93, 0x17, 0xf8, 0x59, 0xc9, 0x7b, 0xba, 0x2a, 0x22, 0x59, 0x45, 0xa7, 0x3a
        };
    size_t length = sizeof(sec_pkt);
    SEC::SecExchangePacket_Send sec(stream, client_encrypted_key, 64);

    BOOST_CHECK_EQUAL(0, memcmp(sec_pkt, stream.data, length));
}

BOOST_AUTO_TEST_CASE(TestReceive_SecExchangePacket)
{
    BStream stream(1024);

    const char sec_pkt[] =
        "\x01\x00\x00\x00" // 0x00000001 = SEC_EXCHANGE_PKT
        "\x48\x00\x00\x00" // 0x00000048 = 72 (64 bytes key + 8 bytes padding)
        "\xca\xe7\xdf\x85\x01\x42\x02\x47\x28\xfc\x11\x97\x85\xa3\xf9\x40"
        "\x73\x97\x33\x2d\x9e\xe4\x0c\x8e\xe0\x97\xfc\x17\x24\x4e\x35\x33"
        "\xe0\x4e\x7d\xdc\x12\x1d\x41\xf1\xd8\x17\x86\x0e\x79\x9b\x4f\x44"
        "\xb2\x82\xf0\x93\x17\xf8\x59\xc9\x7b\xba\x2a\x22\x59\x45\xa7\x3a"
        "\x00\x00\x00\x00\x00\x00\x00\x00" // Padding
    ;
    size_t length = sizeof(sec_pkt);
    GeneratorTransport t(sec_pkt, length);
    t.recv(&stream.end, length);

    SEC::SecExchangePacket_Recv sec(stream, length);
    BOOST_CHECK_EQUAL((uint32_t)SEC::SEC_EXCHANGE_PKT, sec.basicSecurityHeader);
    BOOST_CHECK_EQUAL(72, sec.length);
    BOOST_CHECK_EQUAL(0, memcmp(sec_pkt+8, sec.client_crypt_random, sec.length));
}



// 2.2.1.12 Server License Error PDU - Valid Client
// =============================================

// The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
// from server to client during the Licensing phase of the RDP Connection
// Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
// phases). This licensing PDU indicates that the server will not issue the
// client a license to store and that the Licensing Phase has ended successfully.

// This is one possible licensing PDU that may be sent during the Licensing
// Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible licensing
// PDUs).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU) which encapsulates an MCS Send Data Indication structure (SDin, choice 26 from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure definitions are given in [T125] section 7, parts 7 and 10). The userData field of the MCS Send Data Indication contains a Security Header and a Valid Client License Data (section 2.2.1.12.1) structure.

// securityHeader (variable): Security header. The format of the security header depends on the Encryption Level and Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).

// This field MUST contain one of the following headers:

// - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008) flag.

// - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001), ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

// - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

// If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2), ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the licensing PDU is not encrypted), then the field MUST contain a Basic Security Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not set on the Security Exchange PDU (section 2.2.1.10).

// The SEC_LICENSE_ENCRYPT_CS (0x0200) and SEC_LICENSE_ENCRYPT_SC (0x0200) flags
// are used to communicate whether encryption should be applied to the licensing PDUs (see
// [MS-RDPBCGR] section 2.2.8.1.1.2.1).

// The flags field of the security header MUST contain the SEC_LICENSE_PKT (0x0080) flag (see
// [MS-RDPBCGR] section 2.2.8.1.1.2.1) for all the licensing messages.

// LicensingMessage (variable): A variable-length licensing message whose structure depends
// on the value of the bMsgType field in the preamble structure. The following table lists
// possible values for bMsgType and the associated licensing message (this table also appears
// in [MS-RDPBCGR] section 2.2.1.12.1.1).

// Sent by server:
// ---------------

// +-------------------------+----------------------------------------------------------------------------------------+
// | 0x01 LICENSE_REQUEST    | Indicates a License Request PDU ([MS-RDPELE] section 2.2.2.1) and the LicensingMessage |
// |                         | contains a Server License Request.                                                     |
// +-------------------------+----------------------------------------------------------------------------------------+
// | 0x02 PLATFORM_CHALLENGE |Indicates a Platform Challenge PDU ([MS-RDPELE] section 2.2.2.4) and the                |
// |                         | LicensingMessage contains a Server Platform Challenge.                                 |
// +-------------------------+----------------------------------------------------------------------------------------+
// | 0x03 NEW_LICENSE        | Indicates a New License PDU ([MS-RDPELE] section 2.2.2.7) and the LicensingMessage     |
// |                         | contains a Server New License structure.                                               |
// +-------------------------+----------------------------------------------------------------------------------------+
// | 0x04 UPGRADE_LICENSE    | Indicates an Upgrade License PDU ([MS-RDPELE] section 2.2.2.6), and the                |
// |                         | LicensingMessage contains a Server Upgrade License structure.                          |
// +-------------------------+----------------------------------------------------------------------------------------+
//
// Sent by client:
// ---------------
// +--------------------------+----------------------------------------------------------------------------------------+
// | 0x12 LICENSE_INFO        | Indicates a License Information PDU ([MS-RDPELE] section 2.2.2.3).                     |
// +--------------------------+----------------------------------------------------------------------------------------+
// | 0x13 NEW_LICENSE_REQUEST | Indicates a New License Request PDU ([MS-RDPELE] section 2.2.2.2).                     |
// +--------------------------+-------+--------------------------------------------------------------------------------+
// | 0x15 PLATFORM_CHALLENGE_RESPONSE | Indicates a Platform Challenge Response PDU ([MS-RDPELE] section 2.2.2.5).     |
// +----------------------------------+--------------------------------------------------------------------------------+

// Sent by either client or server:
// --------------------------------
// +----------------------------------+--------------------------------------------------------------------------------+
// | 0xFF ERROR_ALERT                 | Indicates a Licensing Error Message PDU (section 2.2.1.12.1.3).                |
// +----------------------------------+--------------------------------------------------------------------------------+

// flags (1 byte): An 8-bit unsigned integer. License preamble flags.
// 0x0F LicenseProtocolVersionMask The license protocol version. See the discussion which follows this table for more information.
// 0x80 EXTENDED_ERROR_MSG_SUPPORTED Indicates that extended error information using the License Error Message (section 2.2.1.12.1.3) is supported.

// The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version. The following are possible version values.

// wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in bytes of the licensing packet (including the size of the preamble).

// 2.2.2.1 Server License Request (SERVER_LICENSE_REQUEST)
// =======================================================

// The Server License Request packet is sent to the client to initiate the RDP licensing handshake.

// ServerRandom (32 bytes): A 32-byte array containing a random number. This random
//  number is created using a cryptographically secure pseudo-random number generator and is
//  used to generate licensing encryption keys (see section 5.1.3). These keys are used to
//  encrypt licensing data in subsequent licensing messages (see sections 5.1.4 and 5.1.5).

// ProductInfo (variable): A variable-length Product Information structure. This structure
//  contains the details of the product license required for connecting to the terminal server.

// 2.2.2.1.1 Product Information (PRODUCT_INFO)
// ============================================
// The Product Information packet contains the details of the product license that is required for
// connecting to the terminal server. The client uses this structure together with the scope list to
// search for and identify an appropriate license in its license store. Depending on the outcome of the
// search, the client sends a Client New License Request (section 2.2.2.2), Client License Information
// packet (section 2.2.2.3), or license error message (section 2.2.2.7.1) to the server.

// ProductInfo::dwVersion (4 bytes): A 32-bit unsigned integer that contains the license version information.
// The high-order word contains the major version of the operating system on which the terminal
// server is running, while the low-order word contains the minor version.<6>

// ProductInfo::cbCompanyName (4 bytes): An unsigned 32-bit integer that contains the number of bytes in
// the pbCompanyName field, including the terminating null character. This value MUST be
// greater than zero.

// ProductInfo::pbCompanyName (variable): Contains a null-terminated Unicode string that specifies the
// company name.<7>

// ProductInfo::cbProductId (4 bytes): An unsigned 32-bit integer that contains the number of bytes in the
// pbProductId field, including the terminating null character. This value MUST be greater than
// zero.

// ProductInfo::pbProductId (variable): Contains a null-terminated Unicode string that identifies the type of
// the license that is required by the terminal server. It MAY have the following string value. 
// "A02" Per device or per user license
// ------------------------------------------------------------------

// KeyExchangeList (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
//  2.2.1.12.1.2) of type BB_KEY_EXCHG_ALG_BLOB (0x000D). This BLOB contains the list of 32-
//  bit unsigned integers specifying key exchange algorithms that the server supports. The
//  terminal server supports only one key exchange algorithm as of now, so the BLOB contains
//  the following value.

// 0x00000001 KEY_EXCHANGE_ALG_RSA Indicates RSA key exchange algorithm with a 512-bit asymmetric key.<3>

// ServerCertificate (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
//  2.2.1.12.1.2) of type BB_CERTIFICATE_BLOB (0x0003). This BLOB contains the terminal
//  server certificate (see section 2.2.1.4). The terminal server can choose not to send the
//  certificate by setting the wblobLen field in the Licensing Binary BLOB structure to 0. If
//  encryption is in effect and is already protecting RDP traffic, the licensing protocol MAY<4>
//  choose not to send the server certificate (for RDP security measures, see [MS-RDPBCGR]
//  sections 5.3 and 5.4). If the licensing protocol chooses not to send the server certificate, then
//  the client uses the public key obtained from the server certificate sent as part of Server
//  Security Data in the Server MCS Connect Response PDU (see [MS-RDPBCGR] section 2.2.1.4).

// ScopeList (variable): A variable-length Scope List structure that contains a list of entities that
//  issued the client license. This list is used by the client in conjunction with ProductInfo to
//  search for an appropriate license in its license store.<5>

// 2.2.2.1.2 Scope List (SCOPE_LIST)
// =================================
// The Scope List packet contains a list of entities that issued a client license. The client uses the name
// of the issuers in the Scope structures of this list in conjunction with the Product Information
// structure to search the license store for a matching client license.

// ScopeList::ScopeCount (4 bytes): A 32-bit unsigned integer containing the number of elements in the ScopeArray field.

// ScopeList::ScopeArray (variable): An array of Scope structures containing ScopeCount elements. <8>

// 2.2.2.1.2.1 Scope (SCOPE)
// =========================
// The Scope packet contains the name of an entity that issued a client license.

// Scope (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section 2.2.1.12.1.2)
// of type BB_SCOPE_BLOB (0x000E). This BLOB contains the name of a license issuer in null-
// terminated ANSI characters, as specified in [ISO/IEC-8859-1], string format, with an
// implementation-specific valid code page.

// ------------------------------------------------------------------

// +------------------------------------+-------------------------------------+
// | 0x0001 BB_DATA_BLOB                | Used by License Information PDU and |
// |                                    | Platform Challenge Response PDU     |
// |                                    | ([MS-RDPELE] sections 2.2.2.3 and   |
// |                                    | 2.2.2.5).                           |
// +------------------------------------+-------------------------------------+
// | 0x0002 BB_RANDOM_BLOB              | Used by License Information PDU and |
// |                                    | New License Request PDU ([MS-RDPELE]|
// |                                    | sections 2.2.2.3 and 2.2.2.2).      |
// +------------------------------------+-------------------------------------+
// | 0x0003 BB_CERTIFICATE_BLOB         | Used by License Request PDU         |
// |                                    | ([MS-RDPELE] section 2.2.2.1).      |
// +------------------------------------+-------------------------------------+
// | 0x0004 BB_ERROR_BLOB               | Used by License Error PDU (section  |
// |                                    | 2.2.1.12).                          |
// +------------------------------------+-------------------------------------+
// | 0x0009 BB_ENCRYPTED_DATA_BLOB      | Used by Platform Challenge Response |
// |                                    | PDU and Upgrade License PDU         |
// |                                    | ([MS-RDPELE] sections 2.2.2.5 and   |
// |                                    | 2.2.2.6).                           |
// +------------------------------------+-------------------------------------+
// | 0x000D BB_KEY_EXCHG_ALG_BLOB       | Used by License Request PDU         |
// |                                    | ([MS-RDPELE] section 2.2.2.1).      |
// +------------------------------------+-------------------------------------+
// | 0x000E BB_SCOPE_BLOB               | Used by License Request PDU         |
// |                                    | ([MS-RDPELE] section 2.2.2.1).      |
// +------------------------------------+-------------------------------------+
// | 0x000F BB_CLIENT_USER_NAME_BLOB    | Used by New License Request PDU     |
// |                                    | ([MS-RDPELE] section 2.2.2.2).      |
// +------------------------------------+-------------------------------------+
// | 0x0010 BB_CLIENT_MACHINE_NAME_BLOB | Used by New License Request PDU     |
// |                                    | ([MS-RDPELE] section 2.2.2.2).      |
// +------------------------------------+-------------------------------------+

BOOST_AUTO_TEST_CASE(TestSend_SecLicensePacket)
{
    BStream stream(1024);

    const char sec_pkt[] = 
    "\x80\x00\x00\x00" // SEC::SEC_LICENSE_PKT
    "\x01"             // LICENSE_REQUEST
    "\x02"             // PREAMBLE_VERSION_2_0 (RDP 4.0)
    "\x3e\x01"         // wMsgSize = 318 including preamble
    // 32 bytes Server Random
    "\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4\xa5\x03\x90\xe7\xc2\xc7\x39"
    "\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0\x05\xce\x44\x18\x91\x83\x81"
    // ProductInfo::dwVersion (4 bytes): A 32-bit unsigned integer that contains the license version information.
    // The high-order word contains the major version of the operating system on which the terminal
    // server is running, while the low-order word contains the minor version.<6>
    "\x00\x00\x04\x00"
    // ProductInfo::cbCompanyName (4 bytes): An unsigned 32-bit integer that contains the number of bytes in
    // the pbCompanyName field, including the terminating null character. This value MUST be
    // greater than zero.
    "\x2c\x00\x00\x00" // len = 44
    // ProductInfo::pbCompanyName (variable): Contains a null-terminated Unicode string that specifies the
    // company name.<7>
    "\x4d\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00" //M.i.c.r.o.s.o.f.
    "\x74\x00\x20\x00\x43\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00" //t. .C.o.r.p.o.r.
    "\x61\x00\x74\x00\x69\x00\x6f\x00\x6e\x00\x00\x00"                 //a.t.i.o.n...
    // ProductInfo::cbProductId (4 bytes): An unsigned 32-bit integer that contains the number of bytes in the
    // pbProductId field, including the terminating null character. This value MUST be greater than
    // zero.
    "\x08\x00\x00\x00" // len = 8
    // ProductInfo::pbProductId (variable): Contains a null-terminated Unicode string that identifies the type of
    // the license that is required by the terminal server. It MAY have the following string value. 
    // "A02" Per device or per user license
    "\x32\x00\x33\x00\x36\x00\x00\x00" //2.3.6...

    // KeyExchangeList (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
    //  2.2.1.12.1.2) of type BB_KEY_EXCHG_ALG_BLOB (0x000D). This BLOB contains the list of 32-
    //  bit unsigned integers specifying key exchange algorithms that the server supports. The
    //  terminal server supports only one key exchange algorithm as of now, so the BLOB contains
    //  the following value.

    // 0x00000001 KEY_EXCHANGE_ALG_RSA Indicates RSA key exchange algorithm with a 512-bit asymmetric key.<3>

    "\x0d\x00"         // KeyExchangeList::wBlobType = BB_KEY_EXCHG_ALG_BLOB (0x000D)
    "\x04\x00"         // KeyExchangeList::wBlobLen = 4
    "\x01\x00\x00\x00" // KEY_EXCHANGE_ALG_RSA

    // ServerCertificate (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
    //  2.2.1.12.1.2) of type BB_CERTIFICATE_BLOB (0x0003). This BLOB contains the terminal
    //  server certificate (see section 2.2.1.4). The terminal server can choose not to send the
    //  certificate by setting the wblobLen field in the Licensing Binary BLOB structure to 0. If
    //  encryption is in effect and is already protecting RDP traffic, the licensing protocol MAY<4>
    //  choose not to send the server certificate (for RDP security measures, see [MS-RDPBCGR]
    //  sections 5.3 and 5.4). If the licensing protocol chooses not to send the server certificate, then
    //  the client uses the public key obtained from the server certificate sent as part of Server
    //  Security Data in the Server MCS Connect Response PDU (see [MS-RDPBCGR] section 2.2.1.4).

    "\x03\x00" // ServerCertificate::wBlobType = BB_CERTIFICATE_BLOB (0x0003)
    "\xb8\x00" // ServerCertificate::wBlobLen = 184
    "\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00\x5c\x00"
    "\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f\x00\x00\x00"
    "\x01\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29\xc3\x00\x95"
    "\x2d\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63\x64\xc4\x2c"
    "\xf7\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6\xe3\x95\x06"
    "\xae\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74\x29\x53\x89"
    "\x6d\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x48\x00"
    "\xa8\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1\xda\xf6\x1e"
    "\xec\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24"
    "\x00\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2"
    "\xdb\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37"
    "\x00\x00\x00\x00\x00\x00\x00\x00"

    // ScopeList (variable): A variable-length Scope List structure that contains a list of entities that
    //  issued the client license. This list is used by the client in conjunction with ProductInfo to
    //  search for an appropriate license in its license store.<5>
    "\x01\x00\x00\x00" // ScopeList::ScopeCount = 1
    "\x0e\x00"         // ScopeArray[0]::wBlobType = BB_SCOPE_BLOB
    "\x0e\x00"         // // ScopeArray[0]::wBlobLen = 14
    "\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00" //....microsoft.com.
    ;

    size_t length = sizeof(sec_pkt);
//    SEC::SecLicenseRequest_Send sec(stream);

//    BOOST_CHECK_EQUAL(0, memcmp(sec_pkt, stream.data, length));
}

