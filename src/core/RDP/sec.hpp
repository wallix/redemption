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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common sec layer at core module
*/


#pragma once

#include <cassert>
#include <cstdint>
#include <cinttypes>

#include "core/RDP/share.hpp"
#include "system/ssl_sha1.hpp"
#include "utils/crypto/cryptcontext.hpp"

namespace SEC
{

// 2.2.8.1.1.2 Security Headers
// =============================
// 2.2.8.1.1.2.1 Basic (TS_SECURITY_HEADER)
// ----------------------------------------
// The TS_SECURITY_HEADER structure is used to store security flags.

// flags (2 bytes): A 16-bit, unsigned integer. Security flags.

// +--------------------------------+------------------------------------------+
// | 0x0001 SEC_EXCHANGE_PKT        | Indicates that the packet is a Security  |
// |                                | Exchange PDU (section 2.2.1.10). This    |
// |                                | packet type is sent from client to server|
// |                                | only. The client only sends this packet  |
// |                                | if it will be encrypting further         |
// |                                | communication and Standard RDP Security  |
// |                                | mechanisms (section 5.3) are in effect.  |
// +--------------------------------+------------------------------------------+
// | 0x0002 SEC_TRANSPORT_REQ       | Indicates that the packet is an          |
// |                                | Inititiate Multitransport Request PDU    |
// |                                | (section 2.2.15.1).                      |
// |                                |                                          |
// |                                | This flag MUST NOT be present if the PDU |
// |                                | containing the security header is not    |
// |                                | being sent on the MCS message channel.   |
// |                                | The ID of the message channel is         |
// |                                | specified in the Server Message Channel  |
// |                                | Data (section 2.2.1.4.5).                |
// +--------------------------------+------------------------------------------+
// | 0x0004 RDP_SEC_TRANSPORT_RSP   | Indicates that the packet is an          |
// |                                | Inititiate Multitransport Error PDU      |
// |                                | (section 2.2.15.2).                      |
// |                                |                                          |
// |                                | This flag MUST NOT be present if the PDU |
// |                                | containing the security header is not    |
// |                                | being sent on the MCS message channel.   |
// |                                | The ID of the message channel is         |
// |                                | specified in the Server Message Channel  |
// |                                | Data (section 2.2.1.4.5).                |
// +--------------------------------+------------------------------------------+
// | 0x0008 SEC_ENCRYPT             | Indicates that the packet is encrypted.  |
// +--------------------------------+------------------------------------------+
// | 0x0010 SEC_RESET_SEQNO         | This flag is not processed by any RDP    |
// |                                | clients or servers and MUST be ignored.  |
// +--------------------------------+------------------------------------------+
// | 0x0020 SEC_IGNORE_SEQNO        | This flag is not processed by any RDP    |
// |                                | clients or servers and MUST be ignored.  |
// +--------------------------------+------------------------------------------+
// | 0x0040 SEC_INFO_PKT            | Indicates that the packet is a Client    |
// |                                | Info PDU (section 2.2.1.11). This packet |
// |                                | type is sent from client to server only. |
// |                                | If Standard RDP Security mechanisms are  |
// |                                | in effect, then this packet MUST also be |
// |                                | encrypted.                               |
// +--------------------------------+------------------------------------------+
// | 0x0080 SEC_LICENSE_PKT         | Indicates that the packet is a Licensing |
// |                                | PDU (section 2.2.1.12).                  |
// +--------------------------------+------------------------------------------+
// | 0x0200 SEC_LICENSE_ENCRYPT_CS  | Indicates to the client that the server  |
// |                                | is capable of processing encrypted       |
// |                                | licensing packets. It is sent by the     |
// |                                | server together with any licensing PDUs  |
// |                                | (section 2.2.1.12).                      |
// +--------------------------------+------------------------------------------+
// | 0x0200 SEC_LICENSE_ENCRYPT_SC  | Indicates to the server that the client  |
// |                                | is capable of processing encrypted       |
// |                                | licensing packets. It is sent by the     |
// |                                | client together with the SEC_EXCHANGE_PKT|
// |                                | flag when sending a Security Exchange PDU|
// |                                | (section 2.2.1.10).                      |
// +--------------------------------+------------------------------------------+
// | 0x0400 SEC_REDIRECTION_PKT     | Indicates that the packet is a Standard  |
// |                                | Security Server Redirection PDU (section |
// |                                | 2.2.13.2.1) and that the PDU is          |
// |                                | encrypted.                               |
// +--------------------------------+------------------------------------------+
// | 0x0800 SEC_SECURE_CHECKSUM     | Indicates that the MAC for the PDU was   |
// |                                | generated using the "salted MAC          |
// |                                | generation" technique (see section       |
// |                                | 5.3.6.1.1). If this flag is not present, |
// |                                | then the standard technique was used     |
// |                                | (sections 2.2.8.1.1.2.2 and              |
// |                                | 2.2.8.1.1.2.3).                          |
// +--------------------------------+------------------------------------------+
// | 0x1000 SEC_AUTODETECT_REQ      | Indicates that the autoDetectReqData     |
// |                                | field is present. This flag MUST NOT be  |
// |                                | present if the PDU containing the        |
// |                                | security header is being sent from client|
// |                                | to server.                               |
// |                                | This flag MUST NOT be present if the PDU |
// |                                | containing the security header is not    |
// |                                | being sent on the MCS message channel.   |
// |                                | The ID of the message channel is         |
// |                                | specified in the Server Message Channel  |
// |                                | Data (section 2.2.1.4.5).                |
// +--------------------------------+------------------------------------------+
// | 0x2000 SEC_AUTODETECT_RSP      | Indicates that the packet is an          |
// |                                | Auto-Detect Response PDU (2.2.14.2).     |
// |                                | This flag MUST NOT be present if the PDU |
// |                                | containing the security header is not    |
// |                                | being sent on the MCS message channel.   |
// |                                | The ID of the message channel is         |
// |                                | specified in the Server Message Channel  |
// |                                | Data (section 2.2.1.4.5).                |
// +--------------------------------+------------------------------------------+
// | 0x8000 SEC_FLAGSHI_VALID       | Indicates that the flagsHi field contains|
// |                                | valid data. If this flag is not set, then|
// |                                | the contents of the flagsHi field MUST be|
// |                                | ignored.                                 |
// +--------------------------------+------------------------------------------+

enum {
    SEC_EXCHANGE_PKT       = 0x0001,
    SEC_TRANSPORT_REQ      = 0x0002,
    RDP_SEC_TRANSPORT_RSP  = 0x0004,
    SEC_ENCRYPT            = 0x0008,
    SEC_RESET_SEQNO        = 0x0010,
    SEC_IGNORE_SEQNO       = 0x0020,
    SEC_INFO_PKT           = 0x0040,
    SEC_LICENSE_PKT        = 0x0080,
    SEC_LICENSE_ENCRYPT_CS = 0x0200,
    SEC_LICENSE_ENCRYPT_SC = 0x0200,
    SEC_REDIRECTION_PKT    = 0x0400,
    SEC_SECURE_CHECKSUM    = 0x0800,
    SEC_AUTODETECT_REQ     = 0x1000,
    SEC_AUTODETECT_RSP     = 0x2000,
    SEC_FLAGSHI_VALID      = 0x8000
};



// flagsHi (2 bytes): A 16-bit, unsigned integer. This field is reserved for
// future RDP needs. It is currently unused and all values are ignored. This
// field MUST contain valid data only if the SEC_FLAGSHI_VALID bit (0x8000) is
// set in the flags field. If this bit is not set, the flagsHi field is
// uninitialized and MAY contain random data.

// 2.2.8.1.1.2.2 Non-FIPS (TS_SECURITY_HEADER1)
// --------------------------------------------
// The TS_SECURITY_HEADER1 structure extends the Basic Security Header (section
// 2.2.8.1.1.2.1) and is used to store a 64-bit Message Authentication Code.

// basicSecurityHeader (4 bytes): Basic Security Header, as specified in section
//  2.2.8.1.1.2.1.

// dataSignature (8 bytes): A 64-bit Message Authentication Code generated by
// using one of the techniques described in section 5.3.6.1.

// 2.2.8.1.1.2.3 FIPS (TS_SECURITY_HEADER2)
// ----------------------------------------

// The TS_SECURITY_HEADER2 structure extends the Basic Security Header (section
// 2.2.8.1.1.2.1) and is used to store padding information and a 64-bit Message
// Authentication Code.

// basicSecurityHeader (4 bytes): Basic Security Header, as specified in section
//  2.2.8.1.1.2.1.

// length (2 bytes): A 16-bit, unsigned integer. The length of the FIPS security
//  header. This field MUST be set to 0x0010 (16 bytes).

// version (1 byte): An 8-bit, unsigned integer. The version of the FIPS header.
//  This field SHOULD be set to TSFIPS_VERSION1 (0x01).

// padlen (1 byte): An 8-bit, unsigned integer. The number of padding bytes of
// padding appended to the end of the packet prior to encryption to make sure
// that the data to be encrypted is a multiple of the 3DES block size (that is,
// a multiple of 8 because the block size is 64 bits).

// dataSignature (8 bytes): A 64-bit Message Authentication Code generated by
// using the techniques specified in section 5.3.6.2.

// 5 Security
// ==========
// 5.1 Security Considerations for Implementers
// --------------------------------------------
// See sections 5.3 through 5.5 for complete details of RDP security considerations.

// 5.2 Index of Security Parameters
// --------------------------------
// None.

// 5.3 Standard RDP Security
// -------------------------
// 5.3.1 Encryption Levels
// ~~~~~~~~~~~~~~~~~~~~~~~

// Standard RDP Security (section 5.3) supports four levels of encryption: Low,
// Client Compatible, High, and FIPS Compliant. The required Encryption Level is
// configured on the server.

// 1. Low: All data sent from the client to the server is protected by encryption
// based on the maximum key strength supported by the client.

// 2. Client Compatible: All data sent between the client and the server is
// protected by encryption based on the maximum key strength supported by the
// client.

// 3. High: All data sent between the client and server is protected by
// encryption based on the server's maximum key strength.

// 4. FIPS: All data sent between the client and server is protected using
// Federal Information Processing Standard 140-1 validated encryption methods.

// 5.3.2 Negotiating the Cryptographic Configuration
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Clients advertise their cryptographic support (for use with Standard RDP
// Security mechanisms, as described in sections 5.3.3 to 5.3.8) in the  Client
// Security Data (section 2.2.1.3.3), sent to the server as part of the Basic
// Settings Exchange phase of the RDP Connection Sequence (see section 1.3.1.1).
// Upon receiving the client data the server will determine the cryptographic
// configuration to use for the session based on its configured Encryption Level
// and then send this selection to the client in the Server Security Data
// (section 2.2.1.4.3), as part of the Basic Settings Exchange phase.
//  The client will use this information to configure its cryptographic modules.

// Client                                                                Server
//   |                                                                     |
//   |---------> Client Security Data: Supported Encryption Method ------->|
//   |                                                                     |
//   |                                                                     |
//   |                                                                     |
//   |           Server Security Data :                                    |
//   | <----Selected Encryption Method and Encryption Level <--------------|
//   |                                                                     |
//   |                                                                     |
//     Figure 7: Determining the cryptographic configuration for a session

// The Encryption Method and Encryption Level (see section 5.3.1) are closely
// related. If the Encryption Level is zero, then the Encryption Method is zero
// (the converse is also true). This means that if no encryption is being used
// for the session (an Encryption Level of zero), there is no Encryption Method
// being applied to the data. If the Encryption Level is greater than zero
// (encryption is in force for at least client-to-server traffic) then the
// Encryption Method is greater than zero (the converse is also true). This
// means that if encryption is in force for the session, then an Encryption
// Method must be defined which specifies how to encrypt the data. Furthermore,
// if the Encryption Level is set to FIPS, then the Encryption Method selects
// only FIPS-compatible methods.

// If the server determines that no encryption is necessary for the session, it
// can send the client a value of zero for the selected Encryption Method and
// Encryption Level. In this scenario the Security Commencement phase of the
// connection sequence (see section 5.4.2.3) is not executed, with the result
// that the client does not send the Security Exchange PDU (section 2.2.1.10).
// This PDU can be dropped because the Client Random (see section 5.3.4) is
// redundant, since no security keys need to be generated. Furthermore, because
// no security measures are in effect, the Security Header (see section 5.3.8)
// will not be include " with any data sent on the wire, except for the Client
// Info (see section 3.2.5.3.11) and licensing PDUs (see [MS-RDPELE]), which
// always contain the Security Header (see section 2.2.9.1.1.2). To protect the
// confidentiality of client-to-server user data, an RDP server must ensure that
// the negotiated Encryption Level is always greater than zero when using
// Standard RDP Security mechanisms.

// 5.3.2.1 Cryptographic Negotiation Failures
// ++++++++++++++++++++++++++++++++++++++++++

// The Encryption Method selected by the server (section 5.3.2) is based on the
// Encryption Methods supported by the client (section 2.2.1.3.3), the
// Encryption Methods supported by the server and the configured Encryption
// Level (section 5.3.1) of the server. The negotiation of the cryptographic
// parameters for a connection must fail if the server is not able to select an
// Encryption Method to send to the client (section 2.2.1.4.3).

// - Low and Client Compatible: Cryptographic configuration must fail if the
// server does not support the highest Encryption Method advertised by the
// client (for example, the server supports 40-bit and 56-bit encryption while
// the client only supports 40-bit, 56-bit and 128-bit encryption).

// - High: Cryptographic configuration must fail if the client does not support
// the highest Encryption Method supported by the server (for example, the
// server supports 40-bit, 56-bit and 128-bit encryption while the client only
// supports 40-bit and 56-bit encryption).

//- FIPS: Cryptographic configuration must fail if the client does not support
// FIPS 140-1 validated encryption methods.

// If the server is not able to select an Encryption Method to send to the
// client, then the network connection must be closed.

// 5.3.3 Server Certificates
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// 5.3.3.1 Proprietary Certificates
// ++++++++++++++++++++++++++++++++

// Proprietary Certificates are used exclusively by servers that have not
// received an X.509 certificate from a Domain or Enterprise License Server.
// Every server creates a public/private key pair and then generates and stores
// a Proprietary Certificate containing the public key at least once at system
// start-up time. The certificate is only generated when one does not already
// exist.

// The server sends the Proprietary Certificate to the client in the Server
// Security Data (section 2.2.1.4.3) during the Basic Settings Exchange phase
// of the RDP Connection Sequence (see section 1.3.1.1). The Proprietary
// Certificate structure is detailed in section 2.2.1.4.3.1.1.

// 5.3.3.1.1 Terminal Services Signing Key
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The modulus, private exponent, and public exponent of the 512-bit Terminal
// Services asymmetric key used for signing Proprietary Certificates with the
// RSA algorithm are detailed as follows.

// 64-byte Modulus (n):

// 0x3d, 0x3a, 0x5e, 0xbd, 0x72, 0x43, 0x3e, 0xc9,
// 0x4d, 0xbb, 0xc1, 0x1e, 0x4a, 0xba, 0x5f, 0xcb,
// 0x3e, 0x88, 0x20, 0x87, 0xef, 0xf5, 0xc1, 0xe2,
// 0xd7, 0xb7, 0x6b, 0x9a, 0xf2, 0x52, 0x45, 0x95,
// 0xce, 0x63, 0x65, 0x6b, 0x58, 0x3a, 0xfe, 0xef,
// 0x7c, 0xe7, 0xbf, 0xfe, 0x3d, 0xf6, 0x5c, 0x7d,
// 0x6c, 0x5e, 0x06, 0x09, 0x1a, 0xf5, 0x61, 0xbb,
// 0x20, 0x93, 0x09, 0x5f, 0x05, 0x6d, 0xea, 0x87

// 64-byte Private Exponent (d):

// 0x87, 0xa7, 0x19, 0x32, 0xda, 0x11, 0x87, 0x55,
// 0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xf8,
// 0x24, 0x3e, 0xe6, 0xfa, 0xe9, 0x67, 0x49, 0x94,
// 0xcf, 0x92, 0xcc, 0x33, 0x99, 0xe8, 0x08, 0x60,
// 0x17, 0x9a, 0x12, 0x9f, 0x24, 0xdd, 0xb1, 0x24,
// 0x99, 0xc7, 0x3a, 0xb8, 0x0a, 0x7b, 0x0d, 0xdd,
// 0x35, 0x07, 0x79, 0x17, 0x0b, 0x51, 0x9b, 0xb3,
// 0xc7, 0x10, 0x01, 0x13, 0xe7, 0x3f, 0xf3, 0x5f

// 4-byte Public Exponent (e):

// 0x5b, 0x7b, 0x88, 0xc0

// The enumerated integers are in little-endian byte order. The public key is
// the pair (e, n), while the private key is the pair (d, n).

// 5.3.3.1.2 Signing a Proprietary Certificate
// +++++++++++++++++++++++++++++++++++++++++++

// The Proprietary Certificate is signed by using RSA to encrypt the hash of the
// first six fields with the Terminal Services private signing key (specified in
// section 5.3.3.1.1) and then appending the result to the end of the
// certificate. Mathematically the signing operation is formulated as follows:

// s = m^d mod n

// Where

// s = signature;
// m = hash of first six fields of certificate
// d = private exponent
// n = modulus

// The structure of the Proprietary Certificate is detailed in section
// 2.2.1.4.3.1.1. The structure of the public key embedded in the certificate
// is described in 2.2.1.4.3.1.1.1. An example of public key bytes (in
// little-endian format) follows.

// 0x52 0x53 0x41 0x31: magic (0x31415352)
// 0x48 0x00 0x00 0x00: keylen (72 bytes)
// 0x00 0x02 0x00 0x00: bitlen (512 bits)
// 0x3f 0x00 0x00 0x00: datalen (63 bytes)
// 0x01 0x00 0x01 0x00: pubExp (0x00010001)

// 0xaf 0xfe 0x36 0xf2 0xc5 0xa1 0x44 0x2e
// 0x47 0xc1 0x31 0xa7 0xdb 0xc6 0x67 0x02
// 0x64 0x71 0x5c 0x00 0xc9 0xb6 0xb3 0x04
// 0xd0 0x89 0x9f 0xe7 0x6b 0x24 0xe8 0xe8
// 0xe5 0x2d 0x0b 0x13 0xa9 0x0c 0x6d 0x4d
// 0x91 0x5e 0xe8 0xf6 0xb3 0x17 0x17 0xe3
// 0x9f 0xc5 0x4d 0x4a 0xba 0xfa 0xb9 0x2a
// 0x1b 0xfb 0x10 0xdd 0x91 0x8c 0x60 0xb7: modulus

// A 128-bit MD5 hash over the first six fields of the proprietary certificate
// (which are all in little-endian format) appears as follows.

// PublicKeyBlob = wBlobType + wBlobLen + PublicKeyBytes
// hash = MD5(dwVersion + dwSigAlgID + dwKeyAlgID + PublicKeyBlob)

// Because the Terminal Services private signing key has a 64-byte modulus, the
// maximum number of bytes that can be encoded by using the key is 63 (the size
// of the modulus, in bytes, minus 1). An array of 63 bytes is created and
// initialized as follows.

// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x10

// The 128-bit MD5 hash is copied into the first 16 bytes of the array. For
// example, assume that the generated hash is as follows.

// 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
// 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f

// The byte array will appear as follows after copying in the 16 bytes of the
// MD5 hash.

// 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
// 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f
// 0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x01

// The 63-byte array is then treated as an unsigned little-endian integer and
// signed with the Terminal Services private key by using RSA. The resultant
// signature must be in little-endian format before appending it to the
// Proprietary Certificate structure. The final structure of the certificate
// must conform to the specification in section 2.2.1.4.3.1.1. This means that
// fields 7 through to 9 will be the signature BLOB type, the number of bytes in
// the signature and the actual signature bytes respectively. The BLOB type and
// number of bytes must be in little-endian format.

// Example Java source code that shows how to use a private 64-byte asymmetric
// key to sign an array of 63 bytes using RSA is presented in section 4.9. The
// code also shows how to use the associated public key to verify the signature.

// 5.3.3.1.3 Validating a Proprietary Certificate
// ++++++++++++++++++++++++++++++++++++++++++++++

// Verification of the Proprietary Certificate signature is carried out by
// decrypting the signature with the Terminal Services public signing key and
// then verifying that this result is the same as the MD5 hash of the first six
// fields of the certificate.

// m = s^e mod n

// Where

// m = decrypted signature
// s = signature
// e = public exponent
// n = modulus

// The structure of the Proprietary Certificate is detailed in section
// 2.2.1.4.3.1.1. A 128-bit MD5 hash over the first six fields (which are all
// little-endian integers of varying lengths) appears as follows.

// PublicKeyBlob = wBlobType + wBlobLen + PublicKeyBytes
// hash = MD5(dwVersion + dwSigAlgID + dwKeyAlgID + PublicKeyBlob)

// Next, the actual signature bytes are decrypted with the Terminal Services
// public key using RSA by treating the signature bytes as an unsigned
// little-endian integer. If performed correctly, the decryption operation will
// produce a 63-byte integer value. When represented in little-endian format,
// this integer value must conform to the following specification.

// The 17th byte is 0x00.
// The 18th through 62nd bytes are each 0xFF.
// The 63rd byte is 0x01.

// 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
// 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f
// 0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
// 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x01

// The first 16 bytes of the decrypted signature are then compared to the hash
// that was generated over the Proprietary Certificate, and if they match, the
// signature has been successfully verified.

// Example Java source code that shows how to use a private 64-byte asymmetric
// key to sign an array of 63 bytes by using RSA is presented in section 4.9.
// The code also shows how to use the associated public key to verify the
// signature.

// 5.3.8 Packet Layout in the I/O Data Stream
// ==========================================

// The usage of Standard RDP Security mechanisms (see section 5.3) results in a
// security header being present in all packets following the Security Exchange
// PDU (section 2.2.1.10) when encryption is in force. Connection sequence PDUs
// following the RDP Security Commencement phase of the RDP Connection Sequence
// (see section 1.3.1.1) and Slow-Path packets have the same general wire format.

// Unencrypted: TPKT Header
// Unencrypted: X224 Data Header
// Unencrypted: MCS Header (Send Data Request or SQend Data Indication)
// Unencrypted: Security Header
//   Encrypted: Data

// Figure 9: Slow-Path packet layout

// The Security Header essentially contains flags and a MAC signature taken over
// the encrypted data (see section 5.3.6 for details on the MAC generation). In
// FIPS scenarios, the header also include " the number of padding bytes appended
// to the data.

// Fast-Path packets are more compact and formatted differently, but the
// essential contents of the Security Header are still present. For non-FIPS
// scenarios, the packet layout is as follows.

// Unencrypted: Fast-Path Header
// Unencrypted: Length
// Unencrypted: MAC Signature
//   Encrypted: Data

// Figure 10: Non-FIPS Fast-Path packet layout

// And in FIPS Fast-Path scenarios the packet layout is as follows.

// Unencrypted: Fast-Path Header
// Unencrypted: Length
// Unencrypted: FIPS Information
// Unencrypted: MAC Signature
//   Encrypted: Data

// Figure 11: FIPS Fast-Path packet layout

// If no encryption is in effect, the Selected Encryption Method and Encryption
// Level (see section 5.3.1) returned to the client is zero. The Security Header
// will not be include " with any data sent on the wire, except for the Client
// Info (section 2.2.1.11) and licensing PDUs (for an example of a licensing PDU
// see section 2.2.1.12), which always contain the Security Header.

// See sections 2.2.8.1 and 2.2.9.1 for more details on Slow and Fast-Path
// packet formats and the structure of the Security Header in both of these
// scenarios.

// 2.2.1.10 Client Security Exchange PDU
// =====================================

// The Security Exchange PDU is an optional RDP Connection Sequence PDU that is sent from client to
// server during the RDP Security Commencement phase of the RDP Connection Sequence (see section
// 1.3.1.1 for an overview of the RDP Connection Sequence phases). It is sent after all of the
// requested MCS Channel Join Confirm PDUs (section 2.2.1.9) have been received.

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.
//
// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.
//
// mcsSDrq (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU) which
//  encapsulates an MCS Send Data Request structure (SDrq, choice 25 from DomainMCSPDU),
//  as specified in [T125] section 11.32 (the ASN.1 structure definitions are given in [T125]
//  section 7, parts 7 and 10). The userData field of the MCS Send Data Request contains a
//  Security Exchange PDU Data (section 2.2.1.10.1) structure.

// securityExchangePduData (variable): The actual contents of the Security Exchange PDU, as
//  specified in section 2.2.1.10.1.

// 2.2.1.10.1 Security Exchange PDU Data (TS_SECURITY_PACKET)
// ==========================================================

// The TS_SECURITY_PACKET structure contains the encrypted client random value which is used
// together with the server random (see section 2.2.1.4.3) to derive session keys to secure the
// connection (see sections 5.3.4 and 5.3.5).

// basicSecurityHeader (4 bytes): A Basic Security Header (section 2.2.8.1.1.2.1). The flags
// field of the security header MUST contain the SEC_EXCHANGE_PKT flag (0x0001).

// length (4 bytes): A 32-bit, unsigned integer. The size in bytes of the buffer containing the
//  encrypted client random value, not including the header length.

// encryptedClientRandom (variable): The client random value encrypted with the public key of
// the server (see section 5.3.4).

// 5.3.4 Client and Server Random Values
// =====================================
// The client and server both generate a 32-byte random value using a cryptographically-safe
// pseudorandom number generator.
// The server sends the random value that it generated (along with its public key embedded in a
// certificate) to the client in the Server Security Data (section 2.2.1.4.3) during the Basic Settings
// Exchange phase of the RDP Connection Sequence (see section 1.3.1.1).

// If RDP Standard Security mechanisms (section 5.3) are being used, the client sends its random
// value to the server (encrypted with the server's public key) in the Security Exchange PDU (section
// 2.2.1.10) as part of the RDP Security Commencement phase of the RDP Connection Sequence (see
// section 1.3.1.1).

//           Client                                                          Server
//             |                                                                |
//             | <----- Server Security Data: Server Random and Certificate ----|
//             |                                                                |
//             |                                                                |
//             | ------ Security Exchange PDU: Client Random -------------------|
//             |        (encrypted with server public key)                      |
//             Figure 8: Client and server random value exchange

// The two random values are used by the client and server to generate session keys to secure the
// connection.

// 5.3.4.1 Encrypting Client Random
// =============================
// The client random is encrypted by the client with the server's public key (obtained from the Server
// Security Data (section 2.2.1.4.3)) using RSA. Mathematically the encryption operation is formulated
// as follows.

//         c = r^e mod n

//  Where

//         c = encrypted client random
//         r = unencrypted client random
//         e = public exponent
//         n = modulus

// The client random value must be interpreted as an unsigned little-endian integer value when
// performing the encryption. The resultant encrypted client random must be copied into a zeroed-out
// buffer, which is of size:

//  (bitlen / 8) + 8

// For example, if the public key of the server is 512 bits long, then the zeroed-out buffer must be 72
// bytes. This value can also be obtained from the keylen field in the public key structure (see section
// 2.2.1.4.3.1.1.1). The buffer is sent to the server in the Security Exchange PDU (section 2.2.1.10).

// Example Java source code that shows how to use a public 64-byte asymmetric key to encrypt a 32-
// byte client random using RSA is presented in section 4.8. The code also shows how to use the
// associated private key to decrypt the ciphertext.

    struct SecExchangePacket_Recv
    {
        uint32_t basicSecurityHeader;
        uint32_t length;
        InStream payload;

        explicit SecExchangePacket_Recv(InStream & stream)
            : basicSecurityHeader([&stream](){
                const unsigned expected = 8; /* basicSecurityHeader(4) + length(4) */
                if (!stream.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated SEC_EXCHANGE_PKT, expected=%u remains %zu",
                       expected, stream.in_remain());
                    throw Error(ERR_SEC);
                }

                uint32_t basicSecurityHeader = stream.in_uint32_le() & 0xFFFF;
                if (!(basicSecurityHeader & SEC_EXCHANGE_PKT)) {
                    LOG(LOG_ERR, "Expecting SEC_EXCHANGE_PKT, got (%x)", basicSecurityHeader);
                    throw Error(ERR_SEC);
                }
                return basicSecurityHeader;
            }())
            , length([&stream](){
                uint32_t length = stream.in_uint32_le();
                if (length != stream.in_remain()){
                    LOG(LOG_ERR, "Bad SEC_EXCHANGE_PKT length, header say length=%" PRIu32 " available=%zu", length, stream.in_remain());
                }
                return length;
            }())
            , payload(stream.get_current(), stream.in_remain() - 8)
        {
            if (this->payload.get_capacity() != 64){
                LOG(LOG_INFO, "Expecting SEC_EXCHANGE_PKT crypt length=64, got %zu", this->payload.get_capacity());
                throw Error(ERR_SEC_EXPECTING_512_BITS_CLIENT_RANDOM);
            }
            // Skip payload and 8 bytes trailing padding
            stream.in_skip_bytes(this->payload.get_capacity() + 8);
        }
    };

    struct SecExchangePacket_Send
    {
        SecExchangePacket_Send(OutStream & stream, const uint8_t * client_encrypted_key, size_t keylen_in_bytes){
            stream.out_uint32_le(SEC::SEC_EXCHANGE_PKT);
            stream.out_uint32_le(keylen_in_bytes + 8);
            stream.out_copy_bytes(client_encrypted_key, keylen_in_bytes);
            const uint8_t null[8] = {};
            stream.out_copy_bytes(null, 8);
        }
    };

    struct SecInfoPacket_Recv
    {
        uint32_t basicSecurityHeader;
        InStream signature;
        InStream payload;

        SecInfoPacket_Recv(InStream & stream, CryptContext & crypt)
        : basicSecurityHeader([&stream](){
            const unsigned expected = 12; /* basicSecurityHeader(4) + signature(8) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated SEC_INFO_PKT, expected=%u remains %zu",
                   expected, stream.in_remain());
                throw Error(ERR_SEC);
            }
            uint32_t basicSecurityHeader = stream.in_uint32_le() & 0xFFFF;
            if (0 == (basicSecurityHeader & SEC::SEC_INFO_PKT)){
                LOG(LOG_INFO, "SEC_INFO_PKT expected, got %x", basicSecurityHeader);
            }
            if (0 == (basicSecurityHeader & SEC::SEC_ENCRYPT)){
                LOG(LOG_INFO, "SEC_ENCRYPT expected, got %x", basicSecurityHeader);
            }
            return basicSecurityHeader;
        }())
        , signature(stream.get_current(), 8)
        , payload([&stream, this, &crypt](){
            stream.in_skip_bytes(this->signature.get_capacity());
            crypt.decrypt(const_cast<uint8_t*>(stream.get_current()), stream.in_remain());
            return InStream(stream.get_current(), stream.in_remain());
        }())
        // Body of constructor
        {
            // TODO We should not decrypt inplace, it's a bad idea for optimisations
            // TODO Signature should be checked
            stream.in_skip_bytes(this->payload.get_capacity());
        }
    };


    class SecSpecialPacket_Recv
    {
        public:
        uint32_t flags;

        InStream payload;

        SecSpecialPacket_Recv(InStream & stream, CryptContext & crypt, uint32_t encryptionLevel)
            : flags([&stream](){
                const unsigned need = 4; /* flags(4) */
                if (!stream.in_check_rem(need)){
                    LOG(LOG_ERR, "flags expected: need=%u remains=%zu", need, stream.in_remain());
                    throw Error(ERR_SEC);
                }
                return stream.in_uint32_le();
            }())
            //, signature() => we should also check signature
            , payload([&stream, this, encryptionLevel, &crypt](){
                if (encryptionLevel > 0 && this->flags & SEC::SEC_ENCRYPT){
                    if (encryptionLevel == 0){
                        LOG(LOG_ERR, "RDP Packet headers says packet is encrypted, but RDP encryption is disabled");
                        throw Error(ERR_SEC);
                    }
                    const unsigned need = 8; /* signature(8) */
                    if (!stream.in_check_rem(need)){
                        LOG(LOG_ERR, "signature expected: need=%u remains=%zu", need, stream.in_remain());
                        throw Error(ERR_SEC);
                    }

                    // TODO we should check signature
                    stream.in_skip_bytes(8); /* signature */
                    //if (this->verbose >= 0x200){
                    //    LOG(LOG_INFO, "Receiving encrypted TPDU");
                    //    hexdump_c(stream.get_current(), stream.in_remain());
                    //}
                    crypt.decrypt(const_cast<uint8_t*>(stream.get_current()), stream.in_remain());
                    //if (this->verbose >= 0x80){
                    //    LOG(LOG_INFO, "Decrypted %zu bytes", stream.in_remain());
                    //    hexdump_c(stream.get_current(), stream.in_remain());
                    //}
                }
                return InStream(stream.get_current(), stream.in_remain());
            }())
        // Constructor
        {
            stream.in_skip_bytes(this->payload.get_capacity());
        }
    };


    class Sec_Recv
    {
        public:
        uint32_t flags;
        InStream payload;
        Sec_Recv(InStream & stream, CryptContext & crypt, uint32_t encryptionLevel)
            : flags([&stream, encryptionLevel, &crypt](){
                uint32_t flags = 0;
                if (encryptionLevel){
                    const unsigned need = 4; /* flags(4) */
                    if (!stream.in_check_rem(need))
                    {
                        LOG(LOG_ERR, "flags expected: need=%u remains=%zu",
                            need, stream.in_remain());
                        throw Error(ERR_SEC);
                    }
                    flags = stream.in_uint32_le();
                    if (flags & SEC::SEC_ENCRYPT){
                        const unsigned need = 8; /* signature(8) */
                        if (!stream.in_check_rem(need))
                        {
                            LOG(LOG_ERR, "signature expected: need=%u remains=%zu",
                                need, stream.in_remain());
                            throw Error(ERR_SEC);
                        }

                        // TODO shouldn't we check signature ?
                        stream.in_skip_bytes(8); /* signature */
                        //if (this->verbose >= 0x200){
                        //    LOG(LOG_INFO, "Receiving encrypted TPDU");
                        //    hexdump_c(stream.get_current(), stream.in_remain());
                        //}
                        crypt.decrypt(const_cast<uint8_t *>(stream.get_current()), stream.in_remain());
                        //if (this->verbose >= 0x80){
                        //    LOG(LOG_INFO, "Decrypted %zu bytes", stream.get_capacity());
                        //    hexdump_c(stream.get_current(), stream.in_remain());
                        //}
                    }
                }
                return flags;
            }())
            , payload(stream.get_current(), stream.in_remain())
        // Constructor Body
        {
            stream.in_skip_bytes(this->payload.get_capacity());
        }
    };

    struct Sec_Send
    {
        Sec_Send(OutStream & stream, uint8_t * data, size_t len, uint32_t flags, CryptContext & crypt, uint32_t encryptionLevel){
            flags |= encryptionLevel?SEC_ENCRYPT:0;
            if (flags) {
                stream.out_uint32_le(flags);
            }
            if (flags & SEC_ENCRYPT){
                size_t const sig_sz = 8;
                auto & signature = reinterpret_cast<uint8_t(&)[sig_sz]>(*stream.get_current()); /*NOLINT*/
                crypt.sign(data, len, signature);
                stream.out_skip_bytes(sig_sz);
                crypt.decrypt(data, len);
            }
        }
    };

    struct write_sec_send_fn
    {
        uint32_t flags;
        CryptContext & encrypt;
        int encryption_level;

        void operator()(StreamSize<256> /*unused*/, OutStream & sec_header, uint8_t * packet_data, std::size_t packet_size) const {
            SEC::Sec_Send sec(sec_header, packet_data, packet_size, this->flags, this->encrypt, this->encryption_level);
            (void)sec;
        }
    };

    // 5.1.3 Generating the Licensing Encryption and MAC Salt Keys
    // ===========================================================

    // Both the client and the server use the licensing encryption key when necessary to encrypt and decrypt
    // licensing message data. Both the client and the server use the method described in this section to generate
    // the licensing encryption key. The key generating procedure is described as follows. Note that the "+" symbol
    // is used in the following procedure to represent concatenation of the keys.

    // The client and server random values and the decrypted premaster secret are first used to generate a 384-bit
    // master secret, as follows. Note that SHA-1 hash is used.

    // SaltedHash(S, I) = MD5(S + SHA-1 (I + S + ClientRandom + ServerRandom))
    // PreMasterHash(I) = SaltedHash(PremasterSecret, I)
    // MasterSecret = PreMasterHash('A') + PreMasterHash('BB') + PreMasterHash('CCC')

    // A 384-bit SessionKeyBlob is generated.

    // SaltedHash2(S, I) = MD5(S + SHA-1 (I + S + ServerRandom + ClientRandom))
    // MasterHash(I) = SaltedHash2(MasterSecret, I)
    // SessionKeyBlob = MasterHash('A') + MasterHash('BB')  + MasterHash('CCC')

    // The first 128 bits of the SessionKeyBlob are used to generate the MAC salt key.

    // MAC-salt-key = First128Bits(SessionKeyBlob)
    // The MAC salt key is used to generate the MAC checksum that the recipient uses to check
    // the integrity of the licensing message.

    // The licensing encryption key is derived from the SessionKeyBlob.
    // Note that the "+" symbol is used in the following procedure to represent concatenation of the keys.

    // FinalHash(K) = MD5(K + ClientRandom + ServerRandom)
    // LicensingEncryptionKey = FinalHash(Second128Bits(SessionKeyBlob))

    struct SessionKey
    {
        uint8_t blob0[SslMd5::DIGEST_LENGTH];
        uint8_t blob1[SslMd5::DIGEST_LENGTH];
        uint8_t blob2[SslMd5::DIGEST_LENGTH];
        uint8_t licensingEncryptionKey[SslMd5::DIGEST_LENGTH];

        SessionKey(const uint8_t * pre_master_secret, const uint8_t * client_random, const uint8_t (&server_random)[SEC_RANDOM_SIZE])
        {
            const size_t pre_master_secret_size = SslMd5::DIGEST_LENGTH * 3;
            const size_t client_random_size = SEC_RANDOM_SIZE;
            const size_t server_random_size = SEC_RANDOM_SIZE;
            uint8_t master_secret[SslMd5::DIGEST_LENGTH * 3];
            const size_t master_secret_size = sizeof(master_secret);
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t A[1] = {0x41U}; // "A"
                sha1.update(A, sizeof(A));
                sha1.update(pre_master_secret, pre_master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(pre_master_secret, pre_master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.unchecked_final(master_secret);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t BB[2] = {0x42U, 0x42U}; // "BB"
                sha1.update(BB, sizeof(BB));
                sha1.update(pre_master_secret, pre_master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(pre_master_secret, pre_master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.unchecked_final(master_secret + 16);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t CCC[3] = {0x43U, 0x43U, 0x43U}; // "CCC"
                sha1.update(CCC, sizeof(CCC));
                sha1.update(pre_master_secret, pre_master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(pre_master_secret, pre_master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.unchecked_final(master_secret + 32);
            }

            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t A[1] = {0x41U}; // "A"
                sha1.update(A, sizeof(A));
                sha1.update(master_secret, master_secret_size);
                sha1.update(server_random, server_random_size);
                sha1.update(client_random, client_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(master_secret, master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.final(this->blob0);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t BB[2] = {0x42U, 0x42U}; // "BB"
                sha1.update(BB, sizeof(BB));
                sha1.update(master_secret, master_secret_size);
                sha1.update(server_random, server_random_size);
                sha1.update(client_random, client_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(master_secret, master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.final(this->blob1);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t CCC[3] = {0x43U, 0x43U, 0x43U}; // "CCC"
                sha1.update(CCC, sizeof(CCC));
                sha1.update(master_secret, master_secret_size);
                sha1.update(server_random, server_random_size);
                sha1.update(client_random, client_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(master_secret, master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.final(this->blob2);
            }

            {
                SslMd5 md5;
                md5.update(this->blob1, sizeof(this->blob1));
                md5.update(client_random, client_random_size);
                md5.update(server_random, server_random_size);
                md5.final(this->licensingEncryptionKey);
            }
        }

        const uint8_t * get_MAC_salt_key()
        {
            return this->blob0;
        }

        const uint8_t * get_LicensingEncryptionKey()
        {
            return this->licensingEncryptionKey;
        }
    };



// 5.3.4 Client and Server Random Values
// =====================================

// The client and server both generate a 32-byte random value using a cryptographically-safe
// pseudorandom number generator.

// The server sends the random value that it generated (along with its public key embedded in
// a certificate) to the client in the Server Security Data (section 2.2.1.4.3) during the
// Basic Settings Exchange phase of the RDP Connection Sequence (see section 1.3.1.1).

// If RDP Standard Security mechanisms (section 5.3) are being used, the client sends its random
// value to the server (encrypted with the server's public key) in the Security Exchange PDU
// (section 2.2.1.10) as part of the RDP Security Commencement phase of the RDP Connection Sequence
// (see section 1.3.1.1).


// 5.3.5 Initial Session Key Generation
// ====================================

// RDP uses three symmetric session keys derived from the client and server random values
// (see section 5.3.4). Client-to-server traffic is encrypted with one of these keys (known as
//  the client's encryption key and server's decryption key), server-to-client traffic with another
// (known as the server's encryption key and client's decryption key) and the final key is used to
// generate a MAC over the data to help ensure its integrity. The generated keys are 40, 56,
// or 128 bits in length.


// 5.3.5.1 Non-FIPS
// ================

// The client and server random values are used to create a 384-bit Pre-Master Secret by concatenating
// the first 192 bits of the Client Random with the first 192 bits of the Server Random.

//    PreMasterSecret = First192Bits(ClientRandom) + First192Bits(ServerRandom)

// A 384-bit Master Secret is generated using the Pre-Master Secret, the client and server random values,
// and the MD5 hash and SHA-1 hash functions.

//    MasterSecret = PreMasterHash(0x41) + PreMasterHash(0x4242) + PreMasterHash(0x434343)

// Here, the PreMasterHash function is defined as follows.

//    PreMasterHash(I) = SaltedHash(PremasterSecret, I)

// The SaltedHash function is defined as follows.

//    SaltedHash(S, I) = MD5(S + SHA(I + S + ClientRandom + ServerRandom))

// A 384-bit session key blob is generated as follows.

//    SessionKeyBlob = MasterHash(0x58) + MasterHash(0x5959) + MasterHash(0x5A5A5A)

// Here, the MasterHash function is defined as follows.

//    MasterHash(I) = SaltedHash(MasterSecret, I)

// From the session key blob the actual session keys which will be used are derived.
// Both client and server extract the same key data for generating MAC signatures.

//    MACKey128 = First128Bits(SessionKeyBlob)

// The initial encryption and decryption keys are generated next (these keys are updated
// at a later point in the protocol, per section 5.3.6.1). The server generates its encryption
// and decryption keys as follows.

//    InitialServerEncryptKey128 = FinalHash(Second128Bits(SessionKeyBlob))
//    InitialServerDecryptKey128 = FinalHash(Third128Bits(SessionKeyBlob))

// Here, the FinalHash function is defined as follows.

//    FinalHash(K) = MD5(K + ClientRandom + ServerRandom)

// The client constructs its initial decryption key with the bytes that the server uses to
// construct its initial encryption key. Similarly, the client forms its initial encryption
// key with the bytes that the server uses to form its initial decryption key.

//    InitialClientDecryptKey128 = FinalHash(Second128Bits(SessionKeyBlob))
//    InitialClientEncryptKey128 = FinalHash(Third128Bits(SessionKeyBlob))

// This means that the client will use its encryption key to encrypt data and the server will
// use its decryption key to decrypt the same data. Similarly, the server will use its encryption
// key to encrypt data and the client will use its decryption key to decrypt the same data.
// In effect, there are two streams of data (client-to-server and server-to-client) encrypted
// with different session keys which are updated at different intervals.

// To reduce the entropy of the keys to either 40 or 56 bits, the 128-bit client and server keys
// are salted appropriately to produce 64-bit versions with the required strength. The salt values
// to reduce key entropy are shown in the following table:

// +-----------------------+-------------+------------------+----------------+
// | Negotiated key length | Salt length | Salt values      | RC4 key length |
// +-----------------------+-------------+------------------+----------------+
// |           40 bits     |  3 bytes    | 0xD1, 0x26, 0x9E |  8 bytes       |
// +-----------------------+-------------+------------------+----------------+
// |           56 bits     |  1 byte     | 0xD1             |  8 bytes       |
// +-----------------------+-------------+------------------+----------------+
// |          128 bits     |  0 byte     |  N/A             | 16 bytes       |
// +-----------------------+-------------+------------------+----------------+

// Using the salt values, the 40-bit keys are generated as follows.

// MACKey40 = 0xD1269E + Last40Bits(First64Bits(MACKey128))

// InitialServerEncryptKey40 = 0xD1269E + Last40Bits(First64Bits(InitialServerEncryptKey128))
// InitialServerDecryptKey40 = 0xD1269E + Last40Bits(First64Bits(InitialServerDecryptKey128))

// InitialClientEncryptKey40 = 0xD1269E + Last40Bits(First64Bits(InitialClientEncryptKey128))
// InitialClientDecryptKey40 = 0xD1269E + Last40Bits(First64Bits(InitialClientDecryptKey128))

// The 56-bit keys are generated as follows.

// MACKey56 = 0xD1 + Last56Bits(First64Bits(MACKey128))

// InitialServerEncryptKey56 = 0xD1 + Last56Bits(First64Bits(InitialServerEncryptKey128))
// InitialServerDecryptKey56 = 0xD1 + Last56Bits(First64Bits(InitialServerDecryptKey128))

// InitialClientEncryptKey56 = 0xD1 + Last56Bits(First64Bits(InitialClientEncryptKey128))
// InitialClientDecryptKey56 = 0xD1 + Last56Bits(First64Bits(InitialClientDecryptKey128))

// After any necessary salting has been applied, the generated encryption and decryption keys
// are used to initialize RC-4 substitution tables which can then be used to encrypt and decrypt
// data.

// At the end of this process the client and server will each possess three symmetric keys
// to use with the RC4 stream cipher: a MAC key, an encryption key, and a decryption key.
// The MAC key is used to initialize the RC4 substitution table that is used to generate
// Message Authentication Codes, the encryption key is used to initialize the RC4 substitution
// table that is used to perform encryption, and the decryption key is used to initialize the
// RC4 substitution table that is used to perform decryption (for more information on RC4
// substitution table initialization, see [SCHNEIER] section 17.1).




// Detailed sample : Computing message hashing keys
// ================================================

// INPUT
// -----

// ClientRandom (32 bytes):
//    f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78 f3 c5

// ServerRandom (32 bytes):
//    7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74 85 7e

// PROCESS
// ========

//PreMasterSecret = First192Bits(ClientRandom) + First192Bits(ServerRandom)
//    f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e 23 97 87 b6 91 65 21 bb
//    7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16 10 bd

// MasterSecret = PreMasterHash( 0x41 'A' ) + PreMasterHash( 0x4242 'BB' ) + PreMasterHash( 0x434343 'CCC' )

// PreMasterHash( 'A' )
// ~~~~~~~~~~~~~~~~~~~~
//    'A' (0x41) + PreMasterSecret + ClientRandom + ServerRandom:
//        41 f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87
//        5e 23 97 87 b6 91 65 21 bb 7a fb 29 5b 7e 19 74
//        67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16 10
//        bd f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87
//        5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78 f3
//        c5 7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c
//        6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74 85
//        7e

//    Calculate SHA1 Hash of above:
//        14 cf 3a 70 31 ee cb bb a8 9b 7b f3 0b b1 02 c3 3a 55 67 be

//    PreMasterSecret + SHA1Hash:
//        f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e 23 97 87 b6 91 65 21 bb
//        7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16 10 bd
//        14 cf 3a 70 31 ee cb bb a8 9b 7b f3 0b b1 02 c3 3a 55 67 be

//    MD5Hash of the above:

//        03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22 a9 f8

// PreMasterHash( 'BB' )
// ~~~~~~~~~~~~~~~~~~~~~
//    'BB' (0x4242) + PreMasterSecret + ClientRandom + ServerRandom:
//        42 42 f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32
//        87 5e 23 97 87 b6 91 65 21 bb 7a fb 29 5b 7e 19
//        74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16
//        10 bd f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32
//        87 5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78
//        f3 c5 7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf
//        8c 6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74
//        85 7e

//    Calculate SHA1 Hash of above:
//        99 3a 3c e5 2a 5c 49 3f 49 b0 16 3a 40 b3 40 0d 97 ce d4 9e

//    PreMasterSecret + SHA1Hash:
//        f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e 23 97 87 b6 91 65 21 bb
//        7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16 10 bd
//        99 3a 3c e5 2a 5c 49 3f 49 b0 16 3a 40 b3 40 0d 97 ce d4 9e

//    MD5Hash of the above:
//        c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d 9d c9


// PreMasterHash( 'CCC' )
// ~~~~~~~~~~~~~~~~~~~~~~

//    'CCC' (0x434343) + PreMasterSecret + ClientRandom + ServerRandom:
//        43 43 43 f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2
//        32 87 5e 23 97 87 b6 91 65 21 bb 7a fb 29 5b 7e
//        19 74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82
//        16 10 bd f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2
//        32 87 5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d
//        78 f3 c5 7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80
//        bf 8c 6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48
//        74 85 7e

//    Calculate SHA1 Hash of above:
//        40 6f 23 e7 da 94 1a 5f 29 ec 11 a6 38 ad b7 91 13 2c 03 e6

//    PreMasterSecret + SHA1Hash:
//        f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e 23 97 87 b6 91 65 21 bb
//        7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c e0 16 4b 03 82 16 10 bd
//        40 6f 23 e7 da 94 1a 5f 29 ec 11 a6 38 ad b7 91 13 2c 03 e6

//    MD5Hash of the above:
//        7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36 14 b4

// MasterSecret = PreMasterHash( 0x41 'A' ) + PreMasterHash( 0x4242 'BB' ) + PreMasterHash( 0x434343 'CCC' ):
//    03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22 a9 f8
//    c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d 9d c9
//    7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36 14 b4

// SessionKeyBlob = MasterHash( 0x58 'X' ) + MasterHash( 0x5959 'YY' ) + MasterHash( 0x5A5A5A 'ZZZ' )

// MasterHash( 'X' )
// ~~~~~~~~~~~~~~~~~~~
//    'X' (0x58) + MasterSecret + ClientRandom + ServerRandom:
//        58 03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22 a9
//        f8 c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d 9d
//        c9 7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36 14
//        b4 f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87
//        5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78 f3
//        c5 7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c
//        6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74 85
//        7e

//    Calculate SHA1 Hash of above:
//        ac e2 c1 6c f8 95 38 17 fd de c2 cf a6 c1 32 78 f3 8b 7d 25

//    MasterSecret + SHA1Hash:
//        03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22 a9 f8
//        c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d 9d c9
//        7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36 14 b4
//        ac e2 c1 6c f8 95 38 17 fd de c2 cf a6 c1 32 78
//        f3 8b 7d 25

//    MD5Hash of the above:
//        15 0e 4f 19 89 ae be 30 5f 08 ab 26 4a 2d 66 26


// MasterHash( 'YY' )
// ~~~~~~~~~~~~~~~~~~~

//    'YY' (0x5959) + MasterSecret + ClientRandom + ServerRandom:
//        59 59 03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22
//        a9 f8 c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d
//        9d c9 7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36
//        14 b4 f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32
//        87 5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78
//        f3 c5 7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf
//        8c 6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74
//        85 7e

//    Calculate SHA1 Hash of above:
//        9f a9 d4 fd 6f 76 98 52 f5 10 22 4d 57 94 a5 a8 3b 57 3e 71

//    MasterSecret + SHA1Hash:
//        03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22 a9 f8
//        c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d 9d c9
//        7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36 14 b4
//        9f a9 d4 fd 6f 76 98 52 f5 10 22 4d 57 94 a5 a8
//        3b 57 3e 71

//    MD5Hash of the above:
//        ee d7 8c 87 7f c5 bf 60 46 35 63 f4 ea 86 76 fa


// MasterHash( 'ZZZ' )
// ~~~~~~~~~~~~~~~~~~~
//    'ZZZ' (0x5A5A5A) + MasterSecret + ClientRandom + ServerRandom:
//        5a 5a 5a 03 75 89 f4 30 6c 14 35 69 f5 66 db 28
//        22 a9 f8 c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23
//        6d 9d c9 7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13
//        36 14 b4 f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2
//        32 87 5e 23 97 87 b6 91 65 21 bb c6 b0 86 81 1d
//        78 f3 c5 7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80
//        bf 8c 6c e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48
//        74 85 7e

//    Calculate SHA1 Hash of above:
//        2d 6b d9 ed fa b8 7f 95 e0 07 45 1f a2 f4 04 b8 4a aa 55 ce

//    MasterSecret + SHA1Hash:
//        03 75 89 f4 30 6c 14 35 69 f5 66 db 28 22 a9 f8
//        c7 3a ab 7a 66 0f 8e 65 76 af aa 56 23 6d 9d c9
//        7b bc 03 05 4a c9 5e 2a 76 4c 65 3a 13 36 14 b4
//        2d 6b d9 ed fa b8 7f 95 e0 07 45 1f a2 f4 04 b8
//        4a aa 55 ce

//    MD5Hash of the above:
//        f1 4d f1 d4 34 f1 d8 ea 2c d9 4a af 9a 05 84 f3

// SessionKeyBlob = MasterHash( 0x58 'X' ) + MasterHash( 0x5959 'YY' ) + MasterHash( 0x5A5A5A 'ZZZ' ):
//    15 0e 4f 19 89 ae be 30 5f 08 ab 26 4a 2d 66 26
//    ee d7 8c 87 7f c5 bf 60 46 35 63 f4 ea 86 76 fa
//    f1 4d f1 d4 34 f1 d8 ea 2c d9 4a af 9a 05 84 f3

// MACKey128 = First128Bits( SessionKeyBlob ):
//    15 0e 4f 19 89 ae be 30 5f 08 ab 26 4a 2d 66 26

// InitialServerEncryptKey128 = FinalHash(Second128Bits(SessionKeyBlob)):
//    Second128Bits(SessionKeyBlob) + ClientRandom + ServerRandom:
//        ee d7 8c 87 7f c5 bf 60 46 35 63 f4 ea 86 76 fa
//        f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e
//        23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78 f3 c5
//        7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c
//        e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74 85 7e

//    MD5Hash of the above:
//        59 8d b3 6b 64 bd 07 75 85 8a f6 28 5f ba 94 30


// InitialServerDecryptKey128 = FinalHash(Third128Bits(SessionKeyBlob)):
//    Third128Bits(SessionKeyBlob) + ClientRandom + ServerRandom:
//        f1 4d f1 d4 34 f1 d8 ea 2c d9 4a af 9a 05 84 f3
//        f1 0b 35 78 df 9e cc 37 9c 4e 65 68 f2 32 87 5e
//        23 97 87 b6 91 65 21 bb c6 b0 86 81 1d 78 f3 c5
//        7a fb 29 5b 7e 19 74 67 ae 88 a1 ad 80 bf 8c 6c
//        e0 16 4b 03 82 16 10 bd a2 8c 85 5f 48 74 85 7e

//    MD5Hash of the above:
//        cd 8c 18 db d4 34 bd 2f 9e 9a 3d b4 ee 11 af 92

// OUTPUT
// ------

// MACKey:
//    15 0e 4f 19 89 ae be 30 5f 08 ab 26 4a 2d 66 26

// ServerEncryption/ClientDecryption:
//    59 8d b3 6b 64 bd 07 75 85 8a f6 28 5f ba 94 30

// ServerDecryption/ClientEncryption:
//    cd 8c 18 db d4 34 bd 2f 9e 9a 3d b4 ee 11 af 92

    struct KeyBlock
    {
        uint8_t blob0[SslMd5::DIGEST_LENGTH];
        uint8_t blob1[SslMd5::DIGEST_LENGTH];
        uint8_t blob2[SslMd5::DIGEST_LENGTH];
        uint8_t key1[SslMd5::DIGEST_LENGTH];
        uint8_t key2[SslMd5::DIGEST_LENGTH];

        KeyBlock(const uint8_t * client_random, const uint8_t (&server_random)[SEC_RANDOM_SIZE])
        {
            const size_t client_random_size = SEC_RANDOM_SIZE;
            const size_t server_random_size = SEC_RANDOM_SIZE;
            uint8_t pre_master_secret[SslMd5::DIGEST_LENGTH * 3];
            const int pre_master_secret_size = sizeof(pre_master_secret);
            // Construct pre-master secret (session key)
            // we get 24 bytes on 32 from
            // client_random and server_random
            static_assert(SEC_RANDOM_SIZE == 32 );
            memcpy(pre_master_secret, client_random, 24);
            memcpy(pre_master_secret + 24, server_random, 24);

            uint8_t master_secret[SslMd5::DIGEST_LENGTH*3];
            const int master_secret_size = sizeof(master_secret);
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t A[1] = {0x41U}; // "A"
                sha1.update(A, sizeof(A));
                sha1.update(pre_master_secret, pre_master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(pre_master_secret, pre_master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.unchecked_final(master_secret);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t BB[2] = {0x42U, 0x42U}; // "BB"
                sha1.update(BB, sizeof(BB));
                sha1.update(pre_master_secret, pre_master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(pre_master_secret, pre_master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.unchecked_final(master_secret + 16);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t CCC[3] = {0x43U, 0x43U, 0x43U}; // "CCC"
                sha1.update(CCC, sizeof(CCC));
                sha1.update(pre_master_secret, pre_master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(pre_master_secret, pre_master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.unchecked_final(master_secret + 32);
            }

            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t X[1] = {0x58U}; // "X"
                sha1.update(X, sizeof(X));
                sha1.update(master_secret, master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(master_secret, master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.final(this->blob0);
            }
            {
                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t YY[2] = {0x59U, 0x59U}; // "YY"
                sha1.update(YY, sizeof(YY));
                sha1.update(master_secret, master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(master_secret, master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.final(this->blob1);
            }
            {

                uint8_t shasig[SslSha1::DIGEST_LENGTH];
                SslSha1 sha1;
                uint8_t ZZZ[3] = {0x5AU, 0x5AU, 0x5AU}; // "ZZZ"
                sha1.update(ZZZ, sizeof(ZZZ));
                sha1.update(master_secret, master_secret_size);
                sha1.update(client_random, client_random_size);
                sha1.update(server_random, server_random_size);
                sha1.final(shasig);

                SslMd5 md5;
                md5.update(master_secret, master_secret_size);
                md5.update(shasig, sizeof(shasig));
                md5.final(this->blob2);
            }

            {
                SslMd5 md5;
                md5.update(this->blob1, sizeof(this->blob1));
                md5.update(client_random, client_random_size);
                md5.update(server_random, server_random_size);
                md5.final(this->key1);
            }
            {
                SslMd5 md5;
                md5.update(this->blob2, sizeof(this->blob2));
                md5.update(client_random, client_random_size);
                md5.update(server_random, server_random_size);
                md5.final(this->key2);
            }
        }
    };


// 5.3.5.2 FIPS
// ============

// The client and server random values are used to generate temporary 160-bit initial
// encryption and decryption keys by using the SHA-1 hash function. The client generates
// the following:

// ClientEncryptKeyT = SHA(Last128Bits(ClientRandom) + Last128Bits(ServerRandom))
// ClientDecryptKeyT = SHA(First128Bits(ClientRandom) + First128Bits(ServerRandom))

// The server generates the following:

// ServerDecryptKeyT = SHA(Last128Bits(ClientRandom) + Last128Bits(ServerRandom))
// ServerEncryptKeyT= SHA(First128Bits(ClientRandom) + First128Bits(ServerRandom))

// Each of these four keys are then expanded to be 168 bits in length by copying the
// first 8 bits of each key to the rear of the key:

// ClientEncryptKey = ClientEncryptKeyT + First8Bits(ClientEncryptKeyT)
// ClientDecryptKey = ClientDecryptKeyT + First8Bits(ClientDecryptKeyT)

// ServerDecryptKey = ServerDecryptKeyT + First8Bits(ServerDecryptKeyT)
// ServerEncryptKey= ServerEncryptKeyT + First8Bits(ServerEncryptKeyT)

// After expansion to 168 bits, each key is then expanded to be 192 bits in length by
// adding a zero-bit to every group of seven bits using the following algorithm:

//    Reverse every byte in the key.
//    Insert a zero-bit bit after every seventh bit.
//    Reverse every byte.

// The following example (which only shows the first 5 bytes of a 21-byte key) demonstrates
// how a 168-bit key is expanded to 192 bits in size. Assume that the key is:

// 0xD1 0x5E 0xC4 0x7E 0xDA ...

// In binary this is:
//   11010001 01011110 11000100 01111110 11011010 ...
// Reversing each byte yields:
//   10001011 01111010 00100011 01111110 01011011 ...
// Adding a zero-bit after each group of seven bits results in the following values:
//   10001010 10111100 10001000 01101110 11100100 ...
// Finally, reversing each of the bytes yields:
//   01010001 00111101 00010001 01110110 00100111 ...
// In hexadecimal this is:
//   0x51 0x3D 0x11 0x76 0x27 ...

// Once each key has been expanded to 192 bits in size, the final step is to alter the
// least significant bit in each byte so that the entire byte has odd parity. Applying
// this transformation to the bytes in the previous example yields:
//    01010001 00111101 00010000 01110110 00100110 ...

// In hexadecimal this is:
//    0x51 0x3D 0x10 0x76 0x26 ...

// After producing the client and server encryption and decryption keys, the shared key
// to be used with SHA-1 hash to produce a Hash-Based Message Authentication Code (HMAC)
// (see [RFC2104]) is computed by the client as follows:

// HMACKey = SHA(ClientDecryptKeyT + ClientEncryptKeyT)

// The server performs the same computation with the same data (the client encryption
// and server decryption keys are identical, while the server encryption and
// client decryption keys are identical).

// HMACKey = SHA(ServerEncryptKeyT + ServerDecryptKeyT)

// At the end of this process the client and server will each possess three symmetric keys
// to use with the Triple DES block cipher: an HMAC key, a encryption key, and a decryption key.


// 5.3.6 Encrypting and Decrypting the I/O Data Stream

// If the Encryption Level (see section 5.4.1) of the server is greater than zero,
// then encryption will always be in effect. At a minimum, all client-to-server traffic
// (except for licensing PDUs which have optional encryption) will be encrypted and a MAC
// will be appended to the data to ensure transmission integrity.

// The table which follows summarizes the possible encryption and MAC generation scenarios
// based on the Encryption Method and Encryption Level selected by the server (the Encryption
// Method values are described in section 2.2.1.4.3, while the Encryption Levels are described
// in 5.4.1) as part of the cryptographic negotiation described in section 5.3.2:

//+--------------------------+----------------------------+-------------------------+-----------------------+
//|Selected Encryption Level | Selected Encryption Method |   Data Encryption       |   MAC Generation      |
//+--------------------------+----------------------------+-------------------------+-----------------------+
//|         None (0)         |          None (0x00)       |       None              |       None            |
//+--------------------------+----------------------------+-------------------------+-----------------------+
//|         Low (1)          |        40-Bit (0x01)       |   Client-to-server      | Client-to-server      |
//|                          |        56-Bit (0x08)       | traffic only using RC4  | traffic only using    |
//|                          |       128-Bit (0x02)       |                         |  MD5 and SHA-1        |
//+--------------------------+----------------------------+-------------------------+-----------------------+
//|  Client Compatible (2)   |        40-Bit (0x01)       |  Client-to-server and   | Client-to-server and  |
//|                          |        56-Bit (0x08)       | server_to_client        | server_to_client      |
//|                          |       128-Bit (0x02)       | traffic  using RC4      | traffic using MD5 and |
//|                          |                            |                         | SHA-1.                |
//+--------------------------+----------------------------+-------------------------+-----------------------+
//|  High (3)                |       128-Bit (0x02)       |  Client-to-server and   | Client-to-server and  |
//|                          |                            | server_to_client        | server_to_client      |
//|                          |                            | traffic using RC4       | traffic using MD5 and |
//|                          |                            |                         | SHA-1.                |
//+--------------------------+----------------------------+-------------------------+-----------------------+
//|  FIPS (4)                |       FIPS (0x10)          | Client-to-server and    | Client-to-server and  |
//|                          |                            | server_to_client traffic| server_to_client      |
//|                          |                            | using Triple DES        | traffic using SHA-1   |
//+--------------------------+----------------------------+-------------------------+-----------------------+

// 5.3.6.1 Non-FIPS
// ================

// The client and server follow the same series of steps to encrypt a block of data. First,
// a MAC value is generated over the unencrypted data.

// Pad1 = 0x36 repeated 40 times to give 320 bits
// Pad2 = 0x5C repeated 48 times to give 384 bits

// SHAComponent = SHA(MACKeyN + Pad1 + DataLength + Data)
// MACSignature = First64Bits(MD5(MACKeyN + Pad2 + SHAComponent))

// MACKeyN is either MACKey40, MACKey56 or MACKey128, depending on the negotiated key strength.

// DataLength is the size of the data to encrypt in bytes, expressed as a little-endian 32-bit integer.
// Data is the information to be encrypted. The first 8 bytes of the generated MD5 hash are used
// as an 8-byte MAC value to send on the wire.

// Next, the data block is encrypted with RC4 using the current client or server encryption
// substitution table. The encrypted data is appended to the 8-byte MAC value in the network packet.

// Decryption involves a reverse ordering of the previous steps. First, the data is decrypted using
// the current RC4 decryption substitution table. Then, a 16-byte MAC value is generated over the
// decrypted data, and the first 8 bytes of this MAC are compared to the 8-byte MAC value that was
// sent over the wire. If the MAC values do not match, an appropriate error is generated and the
// connection is dropped.

// 5.3.6.1.1 Salted MAC Generation
// ===============================

// The MAC value may be generated by salting the data to be hashed with the current encryption
// count. For example, assume that 42 packets  have already been encrypted. When the next packet
// is encrypted the value 42 is added to the SHA component of the MAC signature. The addition
// of the encryption count can be expressed as follows.

// SHAComponent = SHA(MACKeyN + Pad1 + DataLength + Data + EncryptionCount)
// MACSignature = First64Bits(MD5(MACKeyN + Pad2 + SHAComponent))

// EncryptionCount is the cumulative encryption count, indicating how many encryptions have been
// carried out. It is expressed as a little-endian 32-bit integer. The descriptions for DataLength,
// Data, and MacKeyN are the same as in section 5.3.6.1.

// The use of the salted MAC is dictated by capability flags in the General Capability Set
// (section 2.2.7.1.1), sent by both client and server during the Capability Exchange phase
// of the RDP Connection Sequence (see section 1.3.1.1). In addition, the presence of a
// salted MAC is indicated by the presence of the SEC_SECURE_CHECKSUM flag in the Security Header
// flags field (see section 5.3.8).

// 5.3.6.2 FIPS
// ============

// Prior to performing an encryption or decryption operation, the cryptographic modules used to
// implement Triple DES must be configured with the following Initialization Vector.
// {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF}

// The 160-bit MAC signature key is used to key the HMAC function (see [RFC2104]), which uses
// SHA-1 as the iterative hash function.

// MACSignature = First64Bits(HMAC(HMACKey, Data + EncryptionCount))

// EncryptionCount is the cumulative encryption count, indicating how many encryptions
// have been carried out. It is expressed as a little-endian 32-bit integer. The description
// for Data is the same as in section 5.3.6.1.

// Encryption of the data and construction of the network packet to transmit is similar to
// section 5.3.6.1. The main difference is that Triple DES (in cipher block chaining (CBC) mode)
// is used. Because DES is a block cipher, the data to be encrypted must be padded to be a multiple
// of the block size (8 bytes). The FIPS Security Header (see sections 2.2.8.1 and 2.2.9.1)
// has an extra field to record the number of padding bytes which were appended to the data prior
// to encryption to ensure that upon decryption these bytes are not included as part of the data.

// 5.3.7 Session Key Updates
// =========================

// During the course of a session, the symmetric encryption and decryption keys may need
// to be refreshed.

// 5.3.7.1 Non-FIPS
// ================

// The encryption and the decryption keys are updated after 4,096 packets have been
// sent or received.

// Generating an updated session key requires:

//    The initial session keys (generated as described in section 5.3.5).

//    The current session keys (if no update has been performed, the current
// and initial session keys will be identical).

//    Knowledge of the RC4 key length (computed using Table 1 and the negotiated key length).

// The following sequence of steps shows how updated client and server encryption keys are
// generated (the same steps are used to update the client and server decryption keys).
// The following padding constants are used.

// Pad1 = 0x36 repeated 40 times to give 320 bits
// Pad2 = 0x5C repeated 48 times to give 384 bits

// If the negotiated key strength is 128-bit, then the full 128 bits of the initial
// and current encryption key will be used.

// InitialEncryptKey = InitialEncryptKey128
// CurrentEncryptKey = CurrentEncryptKey128

// If the negotiated key strength is 40-bit or 56-bit, then the first 64 bits of the initial
// and current encryption keys will be used.

// InitialEncryptKey = First64Bits(InitialEncryptKeyN)
// CurrentEncryptKey = First64Bits(CurrentEncryptKeyN)

// InitialEncryptKeyN is either InitialEncryptKey40 or InitialEncryptKey56, depending
// on the negotiated key strength, while CurrentEncryptKeyN is either CurrentEncryptKey40
// or CurrentEncryptKey56, depending on the negotiated key strength.

// The initial and current keys are concatenated and hashed together with padding to form
// a temporary key as follows.

// SHAComponent = SHA(InitialEncryptKey + Pad1 + CurrentEncryptKey)
// TempKey128 = MD5(InitialEncryptKey + Pad2 + SHAComponent)

// If the key strength is 128 bits, then the temporary key (TempKey128) is used
// to reinitialize the associated RC4 substitution table. (For more information on
// RC4 substitution table initialization, see [SCHNEIER] section 17.1.)

// S-TableEncrypt = InitRC4(TempKey128)

// RC4 is then used to encrypt TempKey128 to obtain the new 128-bit encryption key.

// NewEncryptKey128 = RC4(TempKey128, S-TableEncrypt)

// Finally, the associated RC4 substitution table is reinitialized with the new encryption
// key (NewEncryptKey128), which can then be used to encrypt a further 4,096 packets.

// S-Table = InitRC4(NewEncryptKey128)

// If 40-bit or 56-bit keys are being used, then the first 64 bits of the temporary key
// (TempKey128) are used to reinitialize the associated RC4 substitution table.

// TempKey64 = First64Bits(TempKey128)
// S-TableEncrypt = InitRC4(TempKey64)

// RC4 is then used to encrypt these 64 bits, and the first few bytes are salted
// according to the key strength to derive a new 40-bit or 56-bit encryption key
// (see section 5.3.5.1 for details on how to perform the salting operation).

// PreSaltKey = RC4(TempKey64, S-TableEncrypt)

// NewEncryptKey40 = 0xD1269E + Last40Bits(PreSaltKey)
// NewEncryptKey56 = 0xD1 + Last56Bits(PreSaltKey)

// Finally, the new 40-bit or 56-bit encryption key (NewEncryptKey40 or NewEncryptKey56)
// is used to reinitialize the associated RC4 substitution table.

// 5.3.7.2 FIPS
// ============
// No session key updates take place for the duration of a connection if Standard RDP Security
// mechanisms (section 5.3) are being used with a FIPS Encryption Level.

// 5.3.8 Packet Layout in the I/O Data Stream
// ==========================================

// The usage of Standard RDP Security mechanisms (see section 5.3) results in a security header
// being present in all packets following the Security Exchange PDU (section 2.2.1.10) when
// encryption is in force. Connection sequence PDUs following the RDP Security Commencement
// phase of the RDP Connection Sequence (see section 1.3.1.1) and slow-path packets have the
//  same general wire format.

// +----------+------------------+---------------------------+------------+------+
// | TPKT-HDR | X224 Data Header | MCS Header (SDRQ or SDIN) | Sec Header | Data |
// +----------+------------------+---------------------------+------------+------+
// Figure 9: Slow-path packet layout


// The Security Header essentially contains flags and a MAC signature taken over the encrypted
// data (see section 5.3.6 for details on the MAC generation). In FIPS scenarios, the header also
// includes the number of padding bytes appended to the data.

// Fast-path packets are more compact and formatted differently, but the essential contents
// of the Security Header are still present. For non-FIPS scenarios, the packet layout is as follows.

// +------------------+---------+---------------+------+
// | Fast-Path Header | Length  | MAC Signature | Data |
// +------------------+---------+---------------+------+
// Figure 10: Non-FIPS fast-path packet layout

// And in FIPS fast-path scenarios the packet layout is as follows.

// +------------------+---------+------------------+---------------+------+
// | Fast-Path Header | Length  | FIPS Information | MAC Signature | Data |
// +------------------+---------+------------------+---------------+------+
// Figure 11: FIPS fast-path packet layout

// If no encryption is in effect, the Selected Encryption Method and Encryption Level (see section 5.3.1)
// returned to the client is zero. The Security Header will not be included with any data sent on the wire,
// except for the Client Info (section 2.2.1.11) and licensing PDUs (for an example of a licensing PDU see
// section 2.2.1.12), which always contain the Security Header.

// See sections 2.2.8.1 and 2.2.9.1 for more details on slow and fast-path packet formats and the structure
// of the Security Header in both of these scenarios.


// 5.5 Automatic Reconnection
// ==========================

// The Automatic Reconnection feature allows a client to reconnect to an existing session
// (after a short-term network failure has occurred) without having to resend the user's
// credentials to the server. A connection which employs Automatic Reconnection proceeds as follows:

// The user logs in to a new or existing session. As soon as the user has been authenticated,
// a Server Auto-Reconnect Packet (section 2.2.4.2) is generated by the server and sent to the
// client in the Save Session Info PDU (section 2.2.10.1). The Auto-Reconnect Packet (also called
// the auto-reconnect cookie) contains a 16-byte cryptographically secure random number (called the
// auto-reconnect random) and the ID of the session to which the user has connected.

// The client receives the cookie and stores it in memory, never allowing programmatic access to it.

// In the case of a disconnection due to a network error, the client attempts to reconnect to the
// server by trying to reconnect continuously or for a predetermined number of times. Once it has
// connected, the client and server may exchange large random numbers (the client and server random
// specified in section 5.3.4). If Enhanced RDP Security (section 5.4) is in effect, no client random
// is sent to the server (see section 5.3.2).

// The client derives a 16-byte security verifier from the random number contained in the auto-reconnect
// cookie received in Step 2. This security verifier is wrapped in a Client Auto-Reconnect Packet (section
// 2.2.4.3) and sent to the server as part of the extended information (see section 2.2.1.11.1.1.1) of the
// Client Info PDU (section 2.2.1.11).

// The auto-reconnect random is used to key the HMAC function (see [RFC2104]), which uses MD5 as the
// iterative hash function. The security verifier is derived by applying the HMAC to the client random
// received in Step 3.

// SecurityVerifier = HMAC(AutoReconnectRandom, ClientRandom)

// The one-way HMAC transformation prevents an unauthenticated server from obtaining the original
// auto-reconnect random and replaying it for the purpose of connecting to the user's existing session.

// When Enhanced RDP Security is in effect the client random value is not generated (see section 5.3.2).
// In this case, for the purpose of generating the security verifier, the client random is assumed to be
// an array of 16 zero bytes, that is, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }. This effectively
// means that the derived security verifier will always have the same value when carrying out auto-reconnect
// under the Enhanced RDP Security. Hence, care must be taken to authenticate the identity of the server
// to which the client is reconnecting, ensuring that the identity has not changed in the period between
// connections.

// When the server receives the Client Auto-Reconnect Packet, it looks up the auto-reconnect random for the
// session and computes the security verifier using the client random (the same calculation executed by the
// client). If the security verifier value which the client transmitted matches the one computed by the
// server, the client is granted access. At this point, the server has confirmed that the client requesting
// auto-reconnection was the last one connected to the session in question.

// If the check in Step 5 passes, then the client is automatically reconnected to the desired session;
// otherwise the client must obtain the user's credentials to regain access to the session on the remote
// server.

// The auto-reconnect cookie associated with a given session is flushed and regenerated whenever a client
// connects to the session or the session is reset. This ensures that if a different client connects to the
// session, then any previous clients which were connected can no longer use the auto-reconnect mechanism
// to connect. Furthermore, the server invalidates and updates the cookie at hourly intervals, sending the
// new cookie to the client in the Save Session Info PDU.

} // namespace SEC

