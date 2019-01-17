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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to Mcs PDU coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestSec
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"
#include "test_only/transport/test_transport.hpp"
#include "core/RDP/sec.hpp"

RED_AUTO_TEST_CASE(TestSend_SecExchangePacket)
{

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
    size_t length = sizeof(sec_pkt) - 1;
    StaticOutStream<1024> stream;
    SEC::SecExchangePacket_Send sec(stream, client_encrypted_key, sizeof(client_encrypted_key));

    RED_CHECK_EQUAL(0, memcmp(sec_pkt, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestReceive_SecExchangePacket)
{
    const char sec_pkt[] =
        "\x01\x00\x00\x00" // 0x00000001 = SEC_EXCHANGE_PKT
        "\x48\x00\x00\x00" // 0x00000048 = 72 (64 bytes key + 8 bytes padding)
        "\xca\xe7\xdf\x85\x01\x42\x02\x47\x28\xfc\x11\x97\x85\xa3\xf9\x40"
        "\x73\x97\x33\x2d\x9e\xe4\x0c\x8e\xe0\x97\xfc\x17\x24\x4e\x35\x33"
        "\xe0\x4e\x7d\xdc\x12\x1d\x41\xf1\xd8\x17\x86\x0e\x79\x9b\x4f\x44"
        "\xb2\x82\xf0\x93\x17\xf8\x59\xc9\x7b\xba\x2a\x22\x59\x45\xa7\x3a"
        "\x00\x00\x00\x00\x00\x00\x00\x00" // Padding
    ;
    size_t length = sizeof(sec_pkt)-1;
    GeneratorTransport t(sec_pkt, length);

    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    SEC::SecExchangePacket_Recv sec(stream);
    RED_CHECK_EQUAL(static_cast<uint32_t>(SEC::SEC_EXCHANGE_PKT), sec.basicSecurityHeader);
    RED_CHECK_EQUAL(length - 16, sec.payload.get_capacity());
    RED_CHECK_EQUAL(64, sec.payload.get_capacity());
    // We won't compare padding
    RED_CHECK_EQUAL(0, memcmp(sec_pkt+8, sec.payload.get_data(), sec.payload.get_capacity()));
}

RED_AUTO_TEST_CASE(TestReceive_SecInfoPacket)
{
    const char sec_pkt[] =
        "\x48\x00\x00\x00\xf6\xc8\x5c\xd6\xe4\x2e\xd3\x88\x66\x93\x36\x57"
        "\x73\x09\x8b\xf8\xa7\xdb\x68\xf6\xaf\x75\x3a\x1a\x74\x6b\x56\xe0"
        "\x5e\x28\xd4\x04\x22\x77\x25\x85\x69\xb1\x43\xfa\x85\x74\x9e\xa1"
        "\x3d\xa9\xf7\x83\x93\xf1\xe5\x62\xa9\x68\x97\x07\xcd\x62\xfc\xa0"
        "\x67\xff\xfa\x2a\x5a\x81\xb5\xc3\x3d\x5c\x29\x09\xc4\xad\x23\x45"
        "\x90\x96\x47\x44\xb4\xbc\x8f\x82\x57\xfa\x7c\xec\x50\xd9\xba\x66"
        "\x97\xf9\x9b\x74\x20\xee\x59\x9d\x5a\xf7\x35\x63\xce\xd7\x4e\x6d"
        "\x7d\xaf\xfd\xbd\xdf\xb0\xa9\xa7\x87\xd2\x10\x28\x7d\x24\x12\x06"
        "\x6c\xec\x9d\x22\x8f\x01\xad\xe5\x04\x41\x01\x99\xb9\x42\x9b\x2a"
        "\xe7\xcd\xf8\x1c\x77\xb8\x34\xf3\xd1\xe9\xb5\x73\x7c\x3a\x6e\x90"
        "\x7a\x25\x7e\x8e\x52\xea\x7a\xc1\x20\xd2\x36\xc9\x4f\x51\x42\x56"
        "\x5b\xfb\xac\x2a\x6b\x82\xda\x99\x27\xe0\x5e\xd6\xde\x5b\x10\x4e"
        "\x5e\x36\x14\x38\xb4\x16\x39\x01\xcd\x7b\xee\x11\x0d\x1e\x1b\x3c"
        "\x20\x9a\x8f\x5a\xb3\x7f\x02\x96\x5d\x29\x6b\x4f\xa0\x06\x73\xae"
        "\x92\x56\xdf\x60\xf3\xc1\x3c\x81\x96\x96\x3f\x02\x10\x00\x86\x86"
        "\xb3\x74\x1c\x43\x86\x73\x2e\xab\x7b\xf5\x56\x59\xa1\x82\xf8\x40"
        "\x60\xe9\xd6\xa0\xf0\x01\x3a\x74\x11\x52\xc7\xfa\xbb\x03\x33\x4a"
        "\xef\x83\x26\xf4\x38\x02\xef\x06\x9b\x7b\xc1\xb1\xc6\xb3\x8f\xba"
        "\x6e\x1a\xe4\x3a\xf4\xb3\x4d\xa6\xc6\x33\x0c\x87\x2f\x6c\xe8\x92"
        "\x03\xde\x60\xf8\x56\xe6\x8d\x36\xf6\x19\xfd\x19\xb7\xd5\x55\x5e"
        "\x8e\x83"
    ;
    size_t length = sizeof(sec_pkt) - 1;
    GeneratorTransport t(sec_pkt, length);

    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);

    CryptContext decrypt;
    decrypt.encryptionMethod = 1;
    memcpy(decrypt.key, "\xd1\x26\x9e\x63\xec\x51\x65\x1d\x89\x5c\x5a\x2a\x29\xef\x08\x4c", 16);
    memcpy(decrypt.update_key, decrypt.key, 16);

    decrypt.rc4.set_key({decrypt.key, (decrypt.encryptionMethod==1)?8u:16u});

    SEC::SecInfoPacket_Recv sec(stream, decrypt);

    const char expected[] =
        /* 0000 */ "\x0c\x04\x0c\x04\xb3\x47\x03\x00\x00\x00\x02\x00\x00\x00\x00\x00" //.....G..........
        /* 0010 */ "\x00\x00\x00\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00" //....x...........
        /* 0020 */ "\x18\x00\x31\x00\x30\x00\x2e\x00\x31\x00\x30\x00\x2e\x00\x34\x00" //..1.0...1.0...4.
        /* 0030 */ "\x2e\x00\x31\x00\x36\x00\x30\x00\x00\x00\x40\x00\x43\x00\x3a\x00" //..1.6.0...@.C.:.
        /* 0040 */ "\x5c\x00\x57\x00\x69\x00\x6e\x00\x64\x00\x6f\x00\x77\x00\x73\x00" //\.W.i.n.d.o.w.s.
        /* 0050 */ "\x5c\x00\x73\x00\x79\x00\x73\x00\x74\x00\x65\x00\x6d\x00\x33\x00" //\.s.y.s.t.e.m.3.
        /* 0060 */ "\x32\x00\x5c\x00\x6d\x00\x73\x00\x74\x00\x73\x00\x63\x00\x61\x00" //2.\.m.s.t.s.c.a.
        /* 0070 */ "\x78\x00\x2e\x00\x64\x00\x6c\x00\x6c\x00\x00\x00\xc4\xff\xff\xff" //x...d.l.l.......
        /* 0080 */ "\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c\x00\x20\x00\x4d\x00" //P.a.r.i.s.,. .M.
        /* 0090 */ "\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x00\x00\x00\x00\x00\x00" //a.d.r.i.d.......
        /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        /* 00c0 */ "\x00\x00\x0a\x00\x00\x00\x05\x00\x03\x00\x00\x00\x00\x00\x00\x00" //................
        /* 00d0 */ "\x00\x00\x00\x00\x50\x00\x61\x00\x72\x00\x69\x00\x73\x00\x2c\x00" //....P.a.r.i.s.,.
        /* 00e0 */ "\x20\x00\x4d\x00\x61\x00\x64\x00\x72\x00\x69\x00\x64\x00\x20\x00" // .M.a.d.r.i.d. .
        /* 00f0 */ "\x28\x00\x68\x00\x65\x00\x75\x00\x72\x00\x65\x00\x20\x00\x64\x00" //(.h.e.u.r.e. .d.
        /* 0100 */ "\x19\x20\xe9\x00\x74\x00\xe9\x00\x29\x00\x00\x00\x00\x00\x00\x00" //. ..t...).......
        /* 0110 */ "\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x05\x00\x02\x00\x00\x00" //................
        /* 0120 */ "\x00\x00\x00\x00\xc4\xff\xff\xff\x01\x00\x00\x00\x07\x00\x00\x00" //................
        /* 0130 */ "\x00\x00\x64\x00\x00\x00"
        ;
    RED_CHECK_EQUAL(sizeof(expected)-1, sec.payload.get_capacity());
    RED_CHECK_EQUAL(0, memcmp(expected, sec.payload.get_data(), sizeof(expected)-1));
}


//// 2.2.1.12 Server License Error PDU - Valid Client
//// =============================================

//// The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
//// from server to client during the Licensing phase of the RDP Connection
//// Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
//// phases). This licensing PDU indicates that the server will not issue the
//// client a license to store and that the Licensing Phase has ended successfully.

//// This is one possible licensing PDU that may be sent during the Licensing
//// Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible licensing
//// PDUs).

//// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

//// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU) which encapsulates an MCS Send Data Indication structure (SDin, choice 26 from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure definitions are given in [T125] section 7, parts 7 and 10). The userData field of the MCS Send Data Indication contains a Security Header and a Valid Client License Data (section 2.2.1.12.1) structure.

//// securityHeader (variable): Security header. The format of the security header depends on the Encryption Level and Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).

//// This field MUST contain one of the following headers:

//// - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008) flag.

//// - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001), ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

//// - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

//// If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2), ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the licensing PDU is not encrypted), then the field MUST contain a Basic Security Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not set on the Security Exchange PDU (section 2.2.1.10).

//// The SEC_LICENSE_ENCRYPT_CS (0x0200) and SEC_LICENSE_ENCRYPT_SC (0x0200) flags
//// are used to communicate whether encryption should be applied to the licensing PDUs (see
//// [MS-RDPBCGR] section 2.2.8.1.1.2.1).

//// The flags field of the security header MUST contain the SEC_LICENSE_PKT (0x0080) flag (see
//// [MS-RDPBCGR] section 2.2.8.1.1.2.1) for all the licensing messages.

//// LicensingMessage (variable): A variable-length licensing message whose structure depends
//// on the value of the bMsgType field in the preamble structure. The following table lists
//// possible values for bMsgType and the associated licensing message (this table also appears
//// in [MS-RDPBCGR] section 2.2.1.12.1.1).

//// Sent by server:
//// ---------------

//// +-------------------------+----------------------------------------------------------------------------------------+
//// | 0x01 LICENSE_REQUEST    | Indicates a License Request PDU ([MS-RDPELE] section 2.2.2.1) and the LicensingMessage |
//// |                         | contains a Server License Request.                                                     |
//// +-------------------------+----------------------------------------------------------------------------------------+
//// | 0x02 PLATFORM_CHALLENGE |Indicates a Platform Challenge PDU ([MS-RDPELE] section 2.2.2.4) and the                |
//// |                         | LicensingMessage contains a Server Platform Challenge.                                 |
//// +-------------------------+----------------------------------------------------------------------------------------+
//// | 0x03 NEW_LICENSE        | Indicates a New License PDU ([MS-RDPELE] section 2.2.2.7) and the LicensingMessage     |
//// |                         | contains a Server New License structure.                                               |
//// +-------------------------+----------------------------------------------------------------------------------------+
//// | 0x04 UPGRADE_LICENSE    | Indicates an Upgrade License PDU ([MS-RDPELE] section 2.2.2.6), and the                |
//// |                         | LicensingMessage contains a Server Upgrade License structure.                          |
//// +-------------------------+----------------------------------------------------------------------------------------+
////
//// Sent by client:
//// ---------------
//// +--------------------------+----------------------------------------------------------------------------------------+
//// | 0x12 LICENSE_INFO        | Indicates a License Information PDU ([MS-RDPELE] section 2.2.2.3).                     |
//// +--------------------------+----------------------------------------------------------------------------------------+
//// | 0x13 NEW_LICENSE_REQUEST | Indicates a New License Request PDU ([MS-RDPELE] section 2.2.2.2).                     |
//// +--------------------------+-------+--------------------------------------------------------------------------------+
//// | 0x15 PLATFORM_CHALLENGE_RESPONSE | Indicates a Platform Challenge Response PDU ([MS-RDPELE] section 2.2.2.5).     |
//// +----------------------------------+--------------------------------------------------------------------------------+

//// Sent by either client or server:
//// --------------------------------
//// +----------------------------------+--------------------------------------------------------------------------------+
//// | 0xFF ERROR_ALERT                 | Indicates a Licensing Error Message PDU (section 2.2.1.12.1.3).                |
//// +----------------------------------+--------------------------------------------------------------------------------+

//// flags (1 byte): An 8-bit unsigned integer. License preamble flags.
//// 0x0F LicenseProtocolVersionMask The license protocol version. See the discussion which follows this table for more information.
//// 0x80 EXTENDED_ERROR_MSG_SUPPORTED Indicates that extended error information using the License Error Message (section 2.2.1.12.1.3) is supported.

//// The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version. The following are possible version values.

//// wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in bytes of the licensing packet (including the size of the preamble).


////RED_AUTO_TEST_CASE(TestSend_SecLicensePacket)
////{
////    const char sec_pkt[] =
////    "\x80\x00\x00\x00" // SEC::SEC_LICENSE_PKT
////    "\x01"             // LICENSE_REQUEST
////    "\x02"             // PREAMBLE_VERSION_2_0 (RDP 4.0)
////    "\x3e\x01"         // wMsgSize = 318 including preamble
////    // 32 bytes Server Random
////    "\x7b\x3c\x31\xa6\xae\xe8\x74\xf6\xb4\xa5\x03\x90\xe7\xc2\xc7\x39"
////    "\xba\x53\x1c\x30\x54\x6e\x90\x05\xd0\x05\xce\x44\x18\x91\x83\x81"
////    // ProductInfo::dwVersion (4 bytes): A 32-bit unsigned integer that contains the license version information.
////    // The high-order word contains the major version of the operating system on which the terminal
////    // server is running, while the low-order word contains the minor version.<6>
////    "\x00\x00\x04\x00"
////    // ProductInfo::cbCompanyName (4 bytes): An unsigned 32-bit integer that contains the number of bytes in
////    // the pbCompanyName field, including the terminating null character. This value MUST be
////    // greater than zero.
////    "\x2c\x00\x00\x00" // len = 44
////    // ProductInfo::pbCompanyName (variable): Contains a null-terminated Unicode string that specifies the
////    // company name.<7>
////    "\x4d\x00\x69\x00\x63\x00\x72\x00\x6f\x00\x73\x00\x6f\x00\x66\x00" //M.i.c.r.o.s.o.f.
////    "\x74\x00\x20\x00\x43\x00\x6f\x00\x72\x00\x70\x00\x6f\x00\x72\x00" //t. .C.o.r.p.o.r.
////    "\x61\x00\x74\x00\x69\x00\x6f\x00\x6e\x00\x00\x00"                 //a.t.i.o.n...
////    // ProductInfo::cbProductId (4 bytes): An unsigned 32-bit integer that contains the number of bytes in the
////    // pbProductId field, including the terminating null character. This value MUST be greater than
////    // zero.
////    "\x08\x00\x00\x00" // len = 8
////    // ProductInfo::pbProductId (variable): Contains a null-terminated Unicode string that identifies the type of
////    // the license that is required by the terminal server. It MAY have the following string value.
////    // "A02" Per device or per user license
////    "\x32\x00\x33\x00\x36\x00\x00\x00" //2.3.6...

////    // KeyExchangeList (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
////    //  2.2.1.12.1.2) of type BB_KEY_EXCHG_ALG_BLOB (0x000D). This BLOB contains the list of 32-
////    //  bit unsigned integers specifying key exchange algorithms that the server supports. The
////    //  terminal server supports only one key exchange algorithm as of now, so the BLOB contains
////    //  the following value.

////    // 0x00000001 KEY_EXCHANGE_ALG_RSA Indicates RSA key exchange algorithm with a 512-bit asymmetric key.<3>

////    "\x0d\x00"         // KeyExchangeList::wBlobType = BB_KEY_EXCHG_ALG_BLOB (0x000D)
////    "\x04\x00"         // KeyExchangeList::wBlobLen = 4
////    "\x01\x00\x00\x00" // KEY_EXCHANGE_ALG_RSA

////    // ServerCertificate (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
////    //  2.2.1.12.1.2) of type BB_CERTIFICATE_BLOB (0x0003). This BLOB contains the terminal
////    //  server certificate (see section 2.2.1.4). The terminal server can choose not to send the
////    //  certificate by setting the wblobLen field in the Licensing Binary BLOB structure to 0. If
////    //  encryption is in effect and is already protecting RDP traffic, the licensing protocol MAY<4>
////    //  choose not to send the server certificate (for RDP security measures, see [MS-RDPBCGR]
////    //  sections 5.3 and 5.4). If the licensing protocol chooses not to send the server certificate, then
////    //  the client uses the public key obtained from the server certificate sent as part of Server
////    //  Security Data in the Server MCS Connect Response PDU (see [MS-RDPBCGR] section 2.2.1.4).

////    "\x03\x00" // ServerCertificate::wBlobType = BB_CERTIFICATE_BLOB (0x0003)
////    "\xb8\x00" // ServerCertificate::wBlobLen = 184
////    "\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x06\x00\x5c\x00"
////    "\x52\x53\x41\x31\x48\x00\x00\x00\x00\x02\x00\x00\x3f\x00\x00\x00"
////    "\x01\x00\x01\x00\x01\xc7\xc9\xf7\x8e\x5a\x38\xe4\x29\xc3\x00\x95"
////    "\x2d\xdd\x4c\x3e\x50\x45\x0b\x0d\x9e\x2a\x5d\x18\x63\x64\xc4\x2c"
////    "\xf7\x8f\x29\xd5\x3f\xc5\x35\x22\x34\xff\xad\x3a\xe6\xe3\x95\x06"
////    "\xae\x55\x82\xe3\xc8\xc7\xb4\xa8\x47\xc8\x50\x71\x74\x29\x53\x89"
////    "\x6d\x9c\xed\x70\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x48\x00"
////    "\xa8\xf4\x31\xb9\xab\x4b\xe6\xb4\xf4\x39\x89\xd6\xb1\xda\xf6\x1e"
////    "\xec\xb1\xf0\x54\x3b\x5e\x3e\x6a\x71\xb4\xf7\x75\xc8\x16\x2f\x24"
////    "\x00\xde\xe9\x82\x99\x5f\x33\x0b\xa9\xa6\x94\xaf\xcb\x11\xc3\xf2"
////    "\xdb\x09\x42\x68\x29\x56\x58\x01\x56\xdb\x59\x03\x69\xdb\x7d\x37"
////    "\x00\x00\x00\x00\x00\x00\x00\x00"

////    // ScopeList (variable): A variable-length Scope List structure that contains a list of entities that
////    //  issued the client license. This list is used by the client in conjunction with ProductInfo to
////    //  search for an appropriate license in its license store.<5>
////    "\x01\x00\x00\x00" // ScopeList::ScopeCount = 1
////    "\x0e\x00"         // ScopeArray[0]::wBlobType = BB_SCOPE_BLOB
////    "\x0e\x00"         // // ScopeArray[0]::wBlobLen = 14
////    "\x6d\x69\x63\x72\x6f\x73\x6f\x66\x74\x2e\x63\x6f\x6d\x00" //....microsoft.com.
////    ;

////    BStream stream(2048);
////    size_t datalen = sizeof(sec_pkt) - 1;
////    memcpy(stream.data, sec_pkt, datalen);
////    stream.end = stream.data + datalen;
////    RED_CHECK_EQUAL(datalen, stream.size());
////    RED_CHECK_EQUAL((uint32_t)322, stream.size());

////    CryptContext decrypt;
////    SEC::Sec_Recv sec(stream, decrypt, 0, 0);
////    RED_CHECK_EQUAL((uint32_t)SEC::SEC_LICENSE_PKT, sec.flags);
////    RED_CHECK_EQUAL((uint32_t)(datalen - 4), sec.payload.size());
////    RED_CHECK_EQUAL((uint32_t)318, sec.payload.size());
////    RED_CHECK_EQUAL(0, memcmp(sec.payload.data, sec_pkt + 4, 318));


//////    size_t length = sizeof(sec_pkt);
//////    SEC::SecLicenseRequest_Send sec(stream);

//////    RED_CHECK_EQUAL(0, memcmp(sec_pkt, stream.data, length));
////}

