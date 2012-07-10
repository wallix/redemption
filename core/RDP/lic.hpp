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

   Licence Management

*/

#if !defined(__CORE_RDP_LIC_HPP__)
#define __CORE_RDP_LIC_HPP__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sec_utils.hpp"
#include "RDP/sec.hpp"

enum {
    LICENCE_TOKEN_SIZE             = 10,
    LICENCE_HWID_SIZE              = 20,
    LICENCE_SIGNATURE_SIZE         = 16,
};

enum {
    LICENSE_REQUEST             = 0x01,
    PLATFORM_CHALLENGE          = 0x02,
    NEW_LICENSE                 = 0x03,
    UPGRADE_LICENSE             = 0x04,
    LICENSE_INFO                = 0x12,
    NEW_LICENSE_REQUEST         = 0x13,
    PLATFORM_CHALLENGE_RESPONSE = 0x15,
    ERROR_ALERT                 = 0xff
};


enum {
    KEY_EXCHANGE_ALG_RSA        = 0x01,
};

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

// The flags field of the security header MUST contain the SEC_LICENSE_PKT (0x0080) flag (see Basic (TS_SECURITY_HEADER)).

// validClientLicenseData (variable): The actual contents of the License Error (Valid Client) PDU, as specified in section 2.2.1.12.1.

// 2.2.1.12.1 Valid Client License Data (LICENSE_VALID_CLIENT_DATA)
// The LICENSE_VALID_CLIENT_DATA structure contains information which indicates that the server will not issue the client a license to store and that the Licensing Phase has ended successfully.

// preamble (4 bytes): Licensing Preamble (section 2.2.1.12.1.1) structure containing header information. The bMsgType field of the preamble structure MUST be set to ERROR_ALERT (0xFF).

// validClientMessage (variable): A Licensing Error Message (section 2.2.1.12.1.3) structure.

// The dwErrorCode field of the error message structure MUST be set to STATUS_VALID_CLIENT (0x00000007) and the dwStateTransition field MUST be set to ST_NO_TRANSITION (0x00000002). The bbErrorInfo field MUST contain an empty binary large object (BLOB) of type BB_ERROR_BLOB (0x0004).

// 2.2.1.12.1.1 Licensing Preamble (LICENSE_PREAMBLE)
// ==================================================
// The LICENSE_PREAMBLE structure precedes every licensing packet sent on the wire.

// bMsgType (1 byte): An 8-bit, unsigned integer. A type of the licensing
// packet. For more details about the different licensing packets, see
// [MS-RDPELE] section 2.2.2.

// Sent by server:
// ---------------
// 0x01 LICENSE_REQUEST Indicates a License Request PDU ([MS-RDPELE] section 2.2.2.1).
// 0x02 PLATFORM_CHALLENGE Indicates a Platform Challenge PDU ([MS-RDPELE] section 2.2.2.4).
// 0x03 NEW_LICENSE Indicates a New License PDU ([MS-RDPELE] section 2.2.2.7).
// 0x04 UPGRADE_LICENSE Indicates an Upgrade License PDU ([MS-RDPELE] section 2.2.2.6).
//
// Sent by client:
// ---------------
// 0x12 LICENSE_INFO Indicates a License Information PDU ([MS-RDPELE] section 2.2.2.3).
// 0x13 NEW_LICENSE_REQUEST Indicates a New License Request PDU ([MS-RDPELE] section 2.2.2.2).
// 0x15 PLATFORM_CHALLENGE_RESPONSE Indicates a Platform Challenge Response PDU ([MS-RDPELE] section 2.2.2.5).

// Sent by either client or server:
// --------------------------------
// 0xFF ERROR_ALERT Indicates a Licensing Error Message PDU (section 2.2.1.12.1.3).

// flags (1 byte): An 8-bit unsigned integer. License preamble flags.
// 0x0F LicenseProtocolVersionMask The license protocol version. See the discussion which follows this table for more information.
// 0x80 EXTENDED_ERROR_MSG_SUPPORTED Indicates that extended error information using the License Error Message (section 2.2.1.12.1.3) is supported.

// The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version. The following are possible version values.

// wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in bytes of the licensing packet (including the size of the preamble).

// 2.2.2.1 Server License Request (SERVER_LICENSE_REQUEST)
// =======================================================

// See MS-RDPELE for details



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

enum {
    BB_DATA_BLOB                = 0x0001,
    BB_RANDOM_BLOB              = 0x0002,
    BB_CERTIFICATE_BLOB         = 0x0003,
    BB_ERROR_BLOB               = 0x0004,
    BB_ENCRYPTED_DATA_BLOB      = 0x0009,
    BB_KEY_EXCHG_ALG_BLOB       = 0x000D,
    BB_SCOPE_BLOB               = 0x000E,
    BB_CLIENT_USER_NAME_BLOB    = 0x000F,
    BB_CLIENT_MACHINE_NAME_BLOB = 0x0010,
};

static inline void send_lic_initial(Transport * trans, int userid) throw (Error)
{

    Stream stream(32768);
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_SendDataIndication, userid, MCS_GLOBAL_CHANNEL);

    stream.out_uint32_le(SEC_LICENSE_PKT);
    stream.out_uint8(LICENSE_REQUEST);
    stream.out_uint8(2); // preamble flags : PREAMBLE_VERSION_2_0 (RDP 4.0)
    stream.out_uint16_le(318); // wMsgSize = 318 including preamble

    /* some compilers need unsigned char to avoid warnings */
    static uint8_t lic1[314] = {
        // SEC_RANDOM ?
        0x7b, 0x3c, 0x31, 0xa6, 0xae, 0xe8, 0x74, 0xf6,
        0xb4, 0xa5, 0x03, 0x90, 0xe7, 0xc2, 0xc7, 0x39,
        0xba, 0x53, 0x1c, 0x30, 0x54, 0x6e, 0x90, 0x05,
        0xd0, 0x05, 0xce, 0x44, 0x18, 0x91, 0x83, 0x81,
        //
        0x00, 0x00, 0x04, 0x00, 0x2c, 0x00, 0x00, 0x00,
        0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00,
        0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00,
        0x74, 0x00, 0x20, 0x00, 0x43, 0x00, 0x6f, 0x00,
        0x72, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x72, 0x00,
        0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00,
        0x6e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x32, 0x00, 0x33, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x0d, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x03, 0x00, 0xb8, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x5c, 0x00, 0x52, 0x53, 0x41, 0x31,
        0x48, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x3f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x01, 0xc7, 0xc9, 0xf7, 0x8e, 0x5a, 0x38, 0xe4,
        0x29, 0xc3, 0x00, 0x95, 0x2d, 0xdd, 0x4c, 0x3e,
        0x50, 0x45, 0x0b, 0x0d, 0x9e, 0x2a, 0x5d, 0x18,
        0x63, 0x64, 0xc4, 0x2c, 0xf7, 0x8f, 0x29, 0xd5,
        0x3f, 0xc5, 0x35, 0x22, 0x34, 0xff, 0xad, 0x3a,
        0xe6, 0xe3, 0x95, 0x06, 0xae, 0x55, 0x82, 0xe3,
        0xc8, 0xc7, 0xb4, 0xa8, 0x47, 0xc8, 0x50, 0x71,
        0x74, 0x29, 0x53, 0x89, 0x6d, 0x9c, 0xed, 0x70,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x48, 0x00, 0xa8, 0xf4, 0x31, 0xb9,
        0xab, 0x4b, 0xe6, 0xb4, 0xf4, 0x39, 0x89, 0xd6,
        0xb1, 0xda, 0xf6, 0x1e, 0xec, 0xb1, 0xf0, 0x54,
        0x3b, 0x5e, 0x3e, 0x6a, 0x71, 0xb4, 0xf7, 0x75,
        0xc8, 0x16, 0x2f, 0x24, 0x00, 0xde, 0xe9, 0x82,
        0x99, 0x5f, 0x33, 0x0b, 0xa9, 0xa6, 0x94, 0xaf,
        0xcb, 0x11, 0xc3, 0xf2, 0xdb, 0x09, 0x42, 0x68,
        0x29, 0x56, 0x58, 0x01, 0x56, 0xdb, 0x59, 0x03,
        0x69, 0xdb, 0x7d, 0x37, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x0e, 0x00, 0x0e, 0x00, 0x6d, 0x69, 0x63, 0x72,
        0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f,
        0x6d, 0x00
   };

    stream.out_copy_bytes((char*)lic1, 314);

//    Stream stream(32768);
//    X224Out tpdu(X224::DT_TPDU, stream);
//    McsOut sdin_out(stream, MCSPDU_SendDataIndication, userid, MCS_GLOBAL_CHANNEL);
//    stream.out_copy_bytes((char*)lic1, 322);

    mcs.emit_end();
    x224.emit_end();
    trans->send(x224.header(), x224.size());
}


static inline void send_lic_response(Transport * trans, int userid) throw (Error)
{
    /* some compilers need unsigned char to avoid warnings */
    static uint8_t lic2[20] = { 0x80, 0x00, 0x10, 0x00, 0xff, 0x02, 0x10, 0x00,
                             0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                             0x28, 0x14, 0x00, 0x00
                           };

    Stream stream(32768);
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_SendDataIndication, userid, MCS_GLOBAL_CHANNEL);
    stream.out_copy_bytes((char*)lic2, 20);
    mcs.emit_end();
    x224.emit_end();
    trans->send(x224.header(), x224.size());
}

static inline void send_media_lic_response(Transport * trans, int userid) throw (Error)
{
    /* mce */
    /* some compilers need unsigned char to avoid warnings */
    static uint8_t lic3[20] = { 0x80, 0x02, 0x10, 0x00, 0xff, 0x03, 0x10, 0x00,
                             0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                             0xf3, 0x99, 0x00, 0x00
                             };

    Stream stream(32768);
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_SendDataIndication, userid, MCS_GLOBAL_CHANNEL);
    stream.out_copy_bytes((char*)lic3, 20);
    mcs.emit_end();
    x224.emit_end();
    trans->send(x224.header(), x224.size());
}


struct RdpLicence {
    uint8_t licence_key[16];
    uint8_t licence_sign_key[16];
    int licence_issued;
    uint8_t * licence_data;
    size_t licence_size;

    RdpLicence(const char * hostname) : licence_issued(0), licence_size(0) {
        memset(this->licence_key, 0, 16);
        memset(this->licence_sign_key, 0, 16);
        TODO(" licence loading should be done before creating protocol layers")
        struct stat st;
        char path[256];
        sprintf(path, LICENCE_PATH "/licence.%s", hostname);
        int fd = open(path, O_RDONLY);
        if (fd != -1 && fstat(fd, &st) != 0){
            this->licence_data = (uint8_t *)malloc(this->licence_size);
            TODO(" check error code here")
            if (this->licence_data){
                close(fd);
                return;
            }
            if (((int)this->licence_size) != read(fd, this->licence_data, this->licence_size)){
                close(fd);
                return;
            }
            close(fd);
        }
    }

    void rdp_lic_process_authreq(Transport * trans, Stream & stream, const char * hostname, int userid, int licence_issued, CryptContext & encrypt, int use_rdp5)
    {
        ssllib ssl;

        const uint8_t* in_token;
        uint8_t out_token[LICENCE_TOKEN_SIZE];
        uint8_t decrypt_token[LICENCE_TOKEN_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];
        uint8_t crypt_hwid[LICENCE_HWID_SIZE];
        uint8_t out_sig[LICENCE_SIGNATURE_SIZE];

        in_token = 0;
        /* Parse incoming packet and save the encrypted token */
        stream.in_skip_bytes(6); /* unknown: f8 3d 15 00 04 f6 */

        int tokenlen = stream.in_uint16_le();
        if (tokenlen != LICENCE_TOKEN_SIZE) {
            LOG(LOG_ERR, "token len = %d, expected %d", tokenlen, LICENCE_TOKEN_SIZE);
        }
        else{
            in_token = stream.in_uint8p(tokenlen);
            stream.in_uint8p(LICENCE_SIGNATURE_SIZE); // in_sig
            stream.check_end();
        }

        memcpy(out_token, in_token, LICENCE_TOKEN_SIZE);
        /* Decrypt the token. It should read TEST in Unicode. */
        SSL_RC4 crypt_key;
        ssl.rc4_set_key(crypt_key, this->licence_key, 16);
        memcpy(decrypt_token, in_token, LICENCE_TOKEN_SIZE);
        ssl.rc4_crypt(crypt_key, decrypt_token, decrypt_token, LICENCE_TOKEN_SIZE);

        hexdump((const char*)decrypt_token, LICENCE_TOKEN_SIZE);
        /* Generate a signature for a buffer of token and HWID */
        buf_out_uint32(hwid, 2);
        memcpy(hwid + 4, hostname, LICENCE_HWID_SIZE - 4);
//        memcpy(hwid, "\x00\x00\x00\x00\x73\x19\x46\x88\x47\xd7\xb1\xae\xe4\x0d\xbf\x5d\xd9\x63\xc9\x99", LICENCE_HWID_SIZE);

        hexdump((const char*)hwid, LICENCE_HWID_SIZE);

        uint8_t sealed_buffer[LICENCE_TOKEN_SIZE + LICENCE_HWID_SIZE];
        memcpy(sealed_buffer, decrypt_token, LICENCE_TOKEN_SIZE);
        memcpy(sealed_buffer + LICENCE_TOKEN_SIZE, hwid, LICENCE_HWID_SIZE);
        sec_sign(out_sig, 16, this->licence_sign_key, 16, sealed_buffer, sizeof(sealed_buffer));

        /* Now encrypt the HWID */
        ssl.rc4_set_key(crypt_key, this->licence_key, 16);
        memcpy(crypt_hwid, hwid, LICENCE_HWID_SIZE);
        ssl.rc4_crypt(crypt_key, crypt_hwid, crypt_hwid, LICENCE_HWID_SIZE);

        rdp_lic_send_authresp(trans, out_token, crypt_hwid, out_sig, userid, licence_issued, encrypt, use_rdp5);
    }

    // 2.2.2.5 Client Platform Challenge Response (CLIENT_PLATFORM_CHALLENGE_RESPONSE)
    // ===============================================================================

    // The Client Platform Challenge Response packet is sent by the client in response
    // to the Server Platform Challenge (section 2.2.2.4) message.

    // EncryptedPlatformChallengeResponse (variable): A LICENSE_BINARY_BLOB<14>
    // structure (as specified in [MS-RDPBCGR] section 2.2.1.12.1.2) of wBlobType
    // BB_ENCRYPTED_DATA_BLOB (0x0009). This BLOB contains the encrypted Platform
    // Challenge Response Data (section 2.2.2.5.1) generated by the client and is
    // encrypted with the licensing encryption key (see section 5.1.3), using RC4
    // (for instructions on how to perform the encryption, see section 5.1.4).

    // 2.2.2.5.1 Platform Challenge Response Data (PLATFORM_CHALLENGE_RESPONSE_DATA)
    // ------------------------------------------------------------------------------

    // The Platform Challenge Response Data packet contains information pertaining
    // to the client's license handling capabilities and the Client Platform Challenge
    // data sent by the server in the Server Platform Challenge.

    // wVersion (2 bytes): A 16-bit unsigned integer that contains the platform
    // challenge version. This field MUST be set to 0x0100.

    // wClientType (2 bytes): A 16-bit unsigned integer that represents the
    // operating system type of the client and MAY contain one of following values.<15>

    // +-------------------------------------+--------------------------------+
    // | 0x0100 WIN32_PLATFORMCHALLENGE_TYPE | Win32 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0200 WIN16_PLATFORMCHALLENGE_TYPE | Win16 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0300 WINCE_PLATFORMCHALLENGE_TYPE | WinCE Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0xFF00 OTHER_PLATFORMCHALLENGE_TYPE | Other Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+

    // wLicenseDetailLevel (2 bytes): A 16-bit unsigned integer. This field
    // represents the capability of the client to handle license data. RDP
    // version 5.0 and later clients SHOULD advertise support for large (6.5 KB
    // or higher) licenses by setting the detail level to LICENSE_DETAIL_DETAIL
    // (0x0003). The following table lists valid values for this field.

    // +--------------------------------+---------------------------------------+
    // | 0x0001 LICENSE_DETAIL_SIMPLE   | License Detail Simple (client license |
    // |                                | certificate and license server        |
    // |                                | certificate without issuer details).  |
    // +--------------------------------+---------------------------------------+
    // | 0x0002 LICENSE_DETAIL_MODERATE | License Detail Moderate (client       |
    // |                                | license certificate chain up to       |
    // |                                | license server's certificate issuer). |
    // +--------------------------------+---------------------------------------+
    // | 0x0003 LICENSE_DETAIL_DETAIL   | License Detail Detail (client license |
    // |                                | certificate chain up to root          |
    // |                                | certificate).                         |
    // +--------------------------------+---------------------------------------+

    // cbChallenge (2 bytes): A 16-bit unsigned integer that indicates the number
    // of bytes of binary data contained in the pbChallenge field.

    // pbChallenge (variable): Contains the decrypted Client Platform Challenge
    // data sent by the server in the Server Platform Challenge message.

    // ---------------- End of Platform Challenge Data ------------------------

    // EncryptedHWID (variable): A LICENSE_BINARY_BLOB structure (as specified in
    // [MS-RDPBCGR] section 2.2.1.12.1.2) of wBlobType BB_ENCRYPTED_DATA_BLOB (0x0009).
    // This BLOB contains the encrypted Client Hardware Identification (section 2.2.2.3.1)
    // and is encrypted with the licensing encryption key (see section 5.1.3) using RC4
    // (for instructions on how to perform the encryption, see section 5.1.4).

    // MACData (16 bytes): An array of 16 bytes containing an MD5 digest (MAC)
    // generated over the decrypted Client Hardware Identification and Platform
    // Challenge Response Data. For instructions on how to generate this message
    // digest, see section 5.1.6; for a description of how the server uses the
    // MACData field to verify the integrity of the Client Hardware Identification
    // and the Platform Challenge Response Data, see section 3.1.5.1.


    void rdp_lic_send_authresp(Transport * trans, uint8_t* token, uint8_t* crypt_hwid, uint8_t* signature, int userid, int licence_issued, CryptContext & encrypt, int use_rdp5)
    {
        int length = 58;

        Stream stream(32768);
        X224 x224(stream);
        x224.emit_begin(X224::DT_TPDU);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataRequest, userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, encrypt);
        sec.emit_begin( SEC_LICENSE_PKT );

        stream.out_uint8(PLATFORM_CHALLENGE_RESPONSE);
        stream.out_uint8(use_rdp5?3:2); /* version */
        stream.out_uint16_le(length);


        // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
        // the binary information. If wBlobLen is set to 0, then the contents
        // of this field SHOULD be ignored.
        stream.out_uint16_le(BB_DATA_BLOB);

        // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of
        // the binary information in the blobData field. If wBlobLen is set to 0,
        // then the blobData field is not included in the Licensing Binary BLOB
        // structure and the contents of the wBlobType field SHOULD be ignored.
        stream.out_uint16_le(LICENCE_TOKEN_SIZE);
        stream.out_copy_bytes(token, LICENCE_TOKEN_SIZE);

        // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
        // the binary information. If wBlobLen is set to 0, then the contents
        // of this field SHOULD be ignored.
        stream.out_uint16_le(BB_DATA_BLOB);

        // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of
        // the binary information in the blobData field. If wBlobLen is set to 0,
        // then the blobData field is not included in the Licensing Binary BLOB
        // structure and the contents of the wBlobType field SHOULD be ignored.
        stream.out_uint16_le(LICENCE_HWID_SIZE);
        stream.out_copy_bytes(crypt_hwid, LICENCE_HWID_SIZE);

        stream.out_copy_bytes(signature, LICENCE_SIGNATURE_SIZE);

        sec.emit_end();
        mcs.emit_end();
        x224.emit_end();

        trans->send(x224.header(), x224.size());
    }


    void rdp_lic_process_demand(Transport * trans, Stream & stream, const char * hostname, const char * username, int userid, const int licence_issued, CryptContext & encrypt, int crypt_level, int use_rdp5)
    {
        LOG(LOG_DEBUG, "rdp_lic_process_demand");

        uint8_t null_data[SEC_MODULUS_SIZE];
        uint8_t signature[LICENCE_SIGNATURE_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];

        /* Retrieve the server random from the incoming packet */
        const uint8_t * server_random = stream.in_uint8p(SEC_RANDOM_SIZE);

        // RDP licence generate key
        {
            /* We currently use null client keys. This is a bit naughty but, hey,
               the security of licence negotiation isn't exactly paramount. */
            memset(null_data, 0, sizeof(null_data));
            uint8_t* client_random = null_data;
            uint8_t* pre_master_secret = null_data;
            uint8_t master_secret[48];
            uint8_t key_block[48];

            /* Generate master secret and then key material */
            sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 65);
            sec_hash_48(key_block, master_secret, server_random, client_random, 65);
            /* Store first 16 bytes of session key as MAC secret */
            memcpy(this->licence_sign_key, key_block, 16);
            /* Generate RC4 key from next 16 bytes */
            sec_hash_16(this->licence_key, key_block + 16, client_random, server_random);
        }

        if (this->licence_size > 0) {
            buf_out_uint32(hwid, 2);
            memcpy(hwid + 4, hostname, LICENCE_HWID_SIZE - 4);

            /* Generate a signature for the HWID buffer */
            sec_sign(signature, 16, this->licence_sign_key, 16, hwid, sizeof(hwid));
            /* Now encrypt the HWID */
            ssllib ssl;

            SSL_RC4 crypt_key;
            ssl.rc4_set_key(crypt_key, this->licence_key, 16);
            ssl.rc4_crypt(crypt_key, hwid, hwid, sizeof(hwid));

            this->rdp_lic_present(trans, null_data, null_data,
                                  this->licence_data,
                                  this->licence_size,
                                  hwid, signature, userid, licence_issued, use_rdp5);
        }
        else {
            this->rdp_lic_send_request(trans, null_data, null_data, hostname, username, userid, licence_issued, encrypt, crypt_level, use_rdp5);
        }
    }

    int rdp_lic_process_issue(Stream & stream, const char * hostname, int & licence_issued, int use_rdp5)
    {

        stream.in_skip_bytes(2); /* 3d 45 - unknown */
        int length = stream.in_uint16_le();
        if (!stream.check_rem(length)) {
            return 0; // 0 = not connected, this case is probably worse and should cause a disconnection
        }
        ssllib ssl;
        SSL_RC4 crypt_key;
        ssl.rc4_set_key(crypt_key, this->licence_key, 16);
        ssl.rc4_crypt(crypt_key, stream.p, stream.p, length);
        int check = stream.in_uint16_le();
        if (check != 0) {
            return 0; // 0 = not connected, is it enough ?
        }


        licence_issued = 1;

        stream.in_skip_bytes(2); /* pad */
        /* advance to fourth string */
        length = 0;
        for (int i = 0; i < 4; i++) {
            stream.in_skip_bytes(length);
            length = stream.in_uint32_le();
            if (!stream.check_rem(length)) {
                // remaining data after licence
                return 0; // 0 = not connected, this case is probably worse and should cause a disconnection
            }
        }

        this->rdp_save_licence(stream.p, length, hostname);
        return 1;
    }

    TODO(" this is not supported yet  but using rdp_save_licence we would keep a local copy of the licence of a remote server thus avoiding to ask it every time we connect. Anyway the use of files to stoe licences should be abstracted.")
    void rdp_save_licence(uint8_t *data, int length, const char * hostname)
    {
        if ((mkdir(LICENCE_PATH, 0700) == -1))
        {
            if (errno != EEXIST){
                LOG(LOG_ERR, "Error creating path to store licence file %s [%s]", LICENCE_PATH, strerror(errno));
                return;
            }
        }

        char path[256];
        char tmppath[256];

        /* TODO: verify if location that we've stablished is right or not */
        sprintf(path, LICENCE_PATH "/licence.%s", hostname);

        /* write licence to licence.hostname.new and after rename to licence.hostname */
        strcpy(tmppath, path);
        strcat(tmppath, ".new");

        int fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC, 0600);

        if (fd == -1){
            LOG(LOG_ERR, "Error creating licence file %s [%s]", tmppath, strerror(errno));
            return;
        }

        if (write(fd, data, length) != length){
            LOG(LOG_ERR, "Error writing data to licence file %s [%s]", tmppath, strerror(errno));
            unlink(tmppath);
        }
        else if (rename(tmppath, path) == -1){
            LOG(LOG_ERR, "Error renaming licence file %s -> %s [%s]", tmppath, path, strerror(errno));
            unlink(tmppath);
        }
        close(fd);
    }

    // 2.2.2.2 Client New License Request (CLIENT_NEW_LICENSE_REQUEST)
    // ===============================================================
    // The Client New License Request packet is sent to a server when the client
    // cannot find a license matching the product information provided in the
    // Server License Request message. This message is interpreted as a new
    // license request by the server, and the server SHOULD attempt to issue
    // a new license to the client on receipt of this message.

    // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
    // indicates the key exchange algorithm chosen by the client. It MUST be set
    // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
    // exchange with a 512-bit asymmetric key.<9>

    // PlatformId (4 bytes): A 32-bit unsigned integer. This field is composed
    // of two identifiers: the operating system identifier and the independent
    // software vendor (ISV) identifier. The platform ID is composed of the
    // logical OR of these two values.

    // The most significant byte of the PlatformId field contains the operating
    // system version of the client.<10>

    // The second most significant byte of the PlatformId field identifies the
    // ISV that provided the client image.<11>

    // The remaining two bytes in the PlatformId field are used by the ISV to
    // identify the build number of the operating system.<12>

    // ClientRandom (32 bytes): A 32-byte random number generated by the client
    // using a cryptographically secure pseudo-random number generator. The
    // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
    // the data in the EncryptedPreMasterSecret field, are used to generate
    // licensing encryption keys (see section 5.1.3). These keys are used to
    // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

    // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
    // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
    // This BLOB contains an encrypted 48-byte random number. For instructions
    // on how to encrypt this random number, see section 5.1.2.1.

    // ClientUserName (variable): A Licensing Binary BLOB structure (see
    // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_USER_NAME_BLOB
    // (0x000F). This BLOB contains the client user name string in
    // null-terminated ANSI character set format and is used along with the
    // ClientMachineName BLOB to keep track of licenses issued to clients.

    // ClientMachineName (variable): A Licensing Binary BLOB structure (see
    // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_MACHINE_NAME_BLOB
    // (0x0010). This BLOB contains the client machine name string in
    // null-terminated ANSI character set format and is used along with the
    // ClientUserName BLOB to keep track of licenses issued to clients.

    void rdp_lic_send_request(Transport * trans, uint8_t* client_random, uint8_t* rsa_data, const char * hostname, const char * username, int userid, int licence_issued, CryptContext & encrypt, int crypt_level, int use_rdp5)
    {
        LOG(LOG_INFO, "rdp_lic_send_request");

        int userlen = strlen(username) + 1;
        int hostlen = strlen(hostname) + 1;
        int length = 128 + userlen + hostlen;

        Stream stream(32768);
        X224 x224(stream);
        x224.emit_begin(X224::DT_TPDU);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataRequest, userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, encrypt);
        sec.emit_begin( SEC_LICENSE_PKT );

        stream.out_uint8(NEW_LICENSE_REQUEST);
        stream.out_uint8(use_rdp5?3:2);
        stream.out_uint16_le(length);

        // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
        // indicates the key exchange algorithm chosen by the client. It MUST be set
        // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
        // exchange with a 512-bit asymmetric key.<9>

        stream.out_uint32_le(KEY_EXCHANGE_ALG_RSA);

        // PlatformId (4 bytes): A 32-bit unsigned integer. This field is composed
        // of two identifiers: the operating system identifier and the independent
        // software vendor (ISV) identifier. The platform ID is composed of the
        // logical OR of these two values.

        // The most significant byte of the PlatformId field contains the operating
        // system version of the client.<10>

        // The second most significant byte of the PlatformId field identifies the
        // ISV that provided the client image.<11>

        // The remaining two bytes in the PlatformId field are used by the ISV to
        // identify the build number of the operating system.<12>

        stream.out_uint32_le(0);

        // ClientRandom (32 bytes): A 32-byte random number generated by the client
        // using a cryptographically secure pseudo-random number generator. The
        // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
        // the data in the EncryptedPreMasterSecret field, are used to generate
        // licensing encryption keys (see section 5.1.3). These keys are used to
        // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

        stream.out_copy_bytes(client_random, SEC_RANDOM_SIZE);

        // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
        // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
        // This BLOB contains an encrypted 48-byte random number. For instructions
        // on how to encrypt this random number, see section 5.1.2.1.

        // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
        // --------------------------------------------------------
        // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
        // length binary licensing data.

        // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
        // the binary information. If wBlobLen is set to 0, then the contents
        // of this field SHOULD be ignored.

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

        // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the binary information in the blobData field. If wBlobLen is set to 0, then the blobData field is not included in the Licensing Binary BLOB structure and the contents of the wBlobType field SHOULD be ignored.

        // blobData (variable): Variable-length binary data. The size of this data in bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field is not included in the Licensing Binary BLOB structure.

        stream.out_uint16_le(BB_RANDOM_BLOB);
        stream.out_uint16_le((SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
        stream.out_copy_bytes(rsa_data, SEC_MODULUS_SIZE);
        stream.out_clear_bytes(SEC_PADDING_SIZE);

        // ClientUserName (variable): A Licensing Binary BLOB structure (see
        // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_USER_NAME_BLOB
        // (0x000F). This BLOB contains the client user name string in
        // null-terminated ANSI character set format and is used along with the
        // ClientMachineName BLOB to keep track of licenses issued to clients.

        stream.out_uint16_le(LICENCE_TAG_USER);
        stream.out_uint16_le(userlen);
        stream.out_copy_bytes(username, userlen);

        // ClientMachineName (variable): A Licensing Binary BLOB structure (see
        // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_MACHINE_NAME_BLOB
        // (0x0010). This BLOB contains the client machine name string in
        // null-terminated ANSI character set format and is used along with the
        // ClientUserName BLOB to keep track of licenses issued to clients.

        stream.out_uint16_le(LICENCE_TAG_HOST);
        stream.out_uint16_le(hostlen);
        stream.out_copy_bytes(hostname, hostlen);

        sec.emit_end();
        mcs.emit_end();
        x224.emit_end();

        trans->send(x224.header(), x224.size());
    }

    void rdp_lic_present(Transport * trans, uint8_t* client_random, uint8_t* rsa_data,
                uint8_t* licence_data, int licence_size, uint8_t* hwid,
                uint8_t* signature, int userid, const int licence_issued, int use_rdp5)
    {
        Stream stream(32768);
        X224 x224(stream);
        x224.emit_begin(X224::DT_TPDU);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataRequest, userid, MCS_GLOBAL_CHANNEL);

        int length = 16 + SEC_RANDOM_SIZE + SEC_MODULUS_SIZE + SEC_PADDING_SIZE +
                 licence_size + LICENCE_HWID_SIZE + LICENCE_SIGNATURE_SIZE;

        stream.out_uint8(LICENSE_INFO);
        stream.out_uint8(use_rdp5?3:2); /* version */
        stream.out_uint16_le(length);
        stream.out_uint32_le(1);
        stream.out_uint16_le(0);
        stream.out_uint16_le(0x0201);
        stream.out_copy_bytes(client_random, SEC_RANDOM_SIZE);
        stream.out_uint16_le(0);
        stream.out_uint16_le(SEC_MODULUS_SIZE + SEC_PADDING_SIZE);
        stream.out_copy_bytes(rsa_data, SEC_MODULUS_SIZE);
        stream.out_clear_bytes( SEC_PADDING_SIZE);
        stream.out_uint16_le(1);
        stream.out_uint16_le(licence_size);
        stream.out_copy_bytes(licence_data, licence_size);
        stream.out_uint16_le(1);
        stream.out_uint16_le(LICENCE_HWID_SIZE);
        stream.out_copy_bytes(hwid, LICENCE_HWID_SIZE);
        stream.out_copy_bytes(signature, LICENCE_SIGNATURE_SIZE);

        mcs.emit_end();
        x224.emit_end();
        trans->send(x224.header(), x224.size());
    }

};

// GLOSSARY
// ========

// clearing house: A Microsoft central authority for activating a license server and registering client access licenses (CALs).

// client access license (CAL): A license required by a client user or device for accessing a terminal server configured in Application Server mode.

// client license: See client access license (CAL).

// grace period: The duration of time during which a terminal server allows clients to connect without requiring a CAL. The grace period ends either when the duration is complete or when the terminal server receives the first permanent license from the license server.

// license encryption key: A shared symmetric key generated by both the server and client that is used to encrypt licensing message data.

// license server: A server that issues CALs.

// license server certificate: An X.509 certificate used for signing CALs.

// license store: A client-side database that stores CALs issued by a terminal server.

// MD5 digest: A 128-bit message hash value generated as output by the MD5 Message-Digest algorithm. See [RFC1321].

// Message Authentication Code (MAC): A generated value used to verify the integrity of a received licensing message.

// object identifier (OID): A number that uniquely identifies an object class or attribute in a system. See [MSDN-OBJID].

// permanent license: A CAL issued to authenticated clients.

// personal terminal server: In general context, refers to a client SKU target machine that hosts remote desktop sessions. From a terminal service licensing perspective, the behavior of a personal terminal server is similar to that of a terminal server in remote administration mode. Thus any behavioral reference to a personal terminal server in this document essentially implies that the particular behavior is valid for a terminal server in remote administration mode as well. The term personal terminal server is therefore used to encompass all connections where either the end point is a client SKU operating system or is a terminal server running in remote administration mode.

// premaster secret: A 48-byte random number used in license encryption key generation.

// remote administration mode: A terminal server may function in remote administration mode if either the terminal services role is not installed on the machine or the client used to invoke the session has enabled the /admin switch.<1>

// Remote Desktop client: A device that connects to a terminal server and renders the user interface through which a user interacts with a remote session.

// session encryption key: A shared key used for confidential exchange of data between the client and the server.

// temporary license: A type of CAL issued by a terminal server to a client in situations in which a permanent license is not available.

// terminal server: A server that hosts Remote Desktop sessions and enables interaction with each of these sessions on a connected client device. A reference to terminal server in this document generally implies a terminal server in app-server mode.

// terminal server certificate: A certificate that should be used to authenticate a terminal server.

// MAY, SHOULD, MUST, SHOULD NOT, MUST NOT: These terms (in all caps) are used as specified in [RFC2119]. All statements of optional behavior use either MAY, SHOULD, or SHOULD NOT.

// 3.2.5.5 Processing Client Platform Challenge Responses
// ===================================================

// When a server receives the Client Platform Challenge Response message, it decrypts the EncryptedPlatformChallengeResponse and EncryptedHWID fields in the message using the license encryption key generated while processing earlier licensing messages.

// The server MUST then generate the MAC checksum over the decrypted Platform Challenge Response Data packet and decrypted Client Hardware Identification packet, and MUST compare it with the received MAC checksum to verify the data integrity. Handling invalid MACs (section 3.2.5.9) is specified in section 3.2.5.9.

// The following cases can result:

// Case 1: A Client License Information message was received earlier by the server, and the CAL (LicenseInfo BLOB) in the message required an upgrade.

// If the license server cannot be contacted to upgrade the license, and the old license is still valid, the terminal server sends the Server Upgrade License message and returns the old license to the client.

// Case 2: If either of the following conditions occurs:

//    The CAL (LicenseInfo BLOB) received in the Client License Information message required an upgrade, and the license server cannot be contacted to upgrade the CAL and the old license is not valid.

// Or:

//    A Client New License Request message was received earlier, and the license server cannot be contacted to issue a new CAL.

// In this case, if the server's grace period has not been exceeded, the server responds as if the client presented a valid license by sending a license error message with an error code of STATUS_VALID_CLIENT (0x00000007) and a state transition code of ST_NO_TRANSITION (0x00000002), ending the licensing protocol.

// If the server's grace period has been exceeded, it sends a license error message with error code ERR_NO_LICENSE_SERVER (0x00000006) and a state transition of ST_TOTAL_ABORT (0x00000001). The licensing protocol is aborted.

// Case 3: If either of the following conditions occurs:

//    The CAL (LicenseInfo BLOB) received in the Client License Information message required an upgrade, and the license server is available to upgrade the CAL, but it cannot upgrade the license and the old license is not valid.

// Or:

//    A Client New License Request message was received earlier, and the license server is available to issue a new CAL, but the server was not able to issue a new license.

// In this case, if the grace period has not been exceeded, the server responds as if the client presented a valid license by sending a license error message with an error code of STATUS_VALID_CLIENT (0x00000007) and a state transition code of ST_NO_TRANSITION (0x00000002), ending the licensing protocol.

// If the server's grace period has been exceeded, it sends a license error message with an error code of ERR_INVALID_CLIENT (0x00000008) and a state transition of ST_TOTAL_ABORT (0x00000001). The licensing protocol is aborted.

// Case 4: The CAL (LicenseInfo BLOB) in the Client License Information message received by the server required an upgrade, and the CAL is valid and the license server available, but the license server cannot upgrade the license. In this case, the terminal server sends the Server Upgrade License message and returns the old license to the client.

// Case 5: A Client License Information message was received earlier by the server; the CAL (LicenseInfo BLOB) in the message required an upgrade; the license server can be contacted; and the old license is successfully upgraded. In this case, the terminal server returns the upgraded CAL in a Server Upgrade License message.

// Case 6: A Client New License Request message was received earlier, the license server was contacted, and it issued a new license. In this case, the terminal server sends the new license to the client in a Server New License message.

        // wVersion (2 bytes): A 16-bit unsigned integer that contains the platform
        // challenge version. This field MUST be set to 0x0100.

        // wClientType (2 bytes): A 16-bit unsigned integer that represents the
        // operating system type of the client and MAY contain one of following values.<15>

    // +-------------------------------------+--------------------------------+
    // | 0x0100 WIN32_PLATFORMCHALLENGE_TYPE | Win32 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0200 WIN16_PLATFORMCHALLENGE_TYPE | Win16 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0300 WINCE_PLATFORMCHALLENGE_TYPE | WinCE Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0xFF00 OTHER_PLATFORMCHALLENGE_TYPE | Other Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+

    // wLicenseDetailLevel (2 bytes): A 16-bit unsigned integer. This field
    // represents the capability of the client to handle license data. RDP
    // version 5.0 and later clients SHOULD advertise support for large (6.5 KB
    // or higher) licenses by setting the detail level to LICENSE_DETAIL_DETAIL
    // (0x0003). The following table lists valid values for this field.


#endif
