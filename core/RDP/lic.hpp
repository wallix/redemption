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

#include "RDP/sec.hpp"

namespace LIC 
{
    enum {
        LICENSE_TOKEN_SIZE             = 10,
        LICENSE_HWID_SIZE              = 20,
        LICENSE_SIGNATURE_SIZE         = 16,
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
        LICENSE_TAG_USER            = 0x000f,
        LICENSE_TAG_HOST            = 0x0010,
    };

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






struct RdpLicence {
    uint8_t license_key[16];
    uint8_t license_sign_key[16];
    int license_issued;
    uint8_t * license_data;
    size_t license_size;

    RdpLicence(const char * hostname) : license_issued(0), license_size(0) {
        memset(this->license_key, 0, 16);
        memset(this->license_sign_key, 0, 16);
        TODO(" licence loading should be done before creating protocol layers")
        struct stat st;
        char path[256];
        sprintf(path, LICENSE_PATH "/licence.%s", hostname);
        int fd = open(path, O_RDONLY);
        if (fd != -1 && fstat(fd, &st) != 0){
            this->license_data = (uint8_t *)malloc(this->license_size);
            TODO(" check error code here")
            if (this->license_data){
                close(fd);
                return;
            }
            if (((int)this->license_size) != read(fd, this->license_data, this->license_size)){
                close(fd);
                return;
            }
            close(fd);
        }
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
