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

#ifndef _REDEMPTION_CORE_RDP_SEC_HPP_
#define _REDEMPTION_CORE_RDP_SEC_HPP_

#include <assert.h>
#include <stdint.h>

#include "RDP/rdp.hpp"
#include "client_info.hpp"
#include "constants.hpp"


TODO(" ssl calls introduce some dependency on ssl system library  injecting it in the sec object would be better.")
#include "ssl_calls.hpp"

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
    SEC_FLAGSHI_VALID      = 0x8000,
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
        SubStream payload;

        SecExchangePacket_Recv(Stream & stream, uint16_t available_len) 
            : payload(stream)
        {
            const unsigned expected = 8; /* basicSecurityHeader(4) + length(4) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated SEC_EXCHANGE_PKT, expected=%u remains %u",
                   expected, stream.in_remain());
                throw Error(ERR_SEC);
            }

            this->basicSecurityHeader = stream.in_uint32_le() & 0xFFFF;

            if (!(this->basicSecurityHeader & SEC_EXCHANGE_PKT)) {
                LOG(LOG_ERR, "Expecting SEC_EXCHANGE_PKT, got (%x)", this->basicSecurityHeader);
                throw Error(ERR_SEC);
            }
            uint32_t length = stream.in_uint32_le();
            if (length + 8 != available_len){
                LOG(LOG_ERR, "Bad SEC_EXCHANGE_PKT length, header say length=%u available=%u", length, available_len-8);
            }
            this->payload.resize(stream, stream.in_remain());
            if (this->payload.size() != 64 + 8){
                LOG(LOG_INFO, "Expecting SEC_EXCHANGE_PKT crypt length=64, got %u", this->payload.size()-8);
                throw Error(ERR_SEC_EXPECTING_512_BITS_CLIENT_RANDOM);
            }
        }
    };

    struct SecExchangePacket_Send
    {
        SecExchangePacket_Send(Stream & stream, const uint8_t * client_encrypted_key, size_t keylen_in_bytes){
            stream.out_uint32_le(SEC::SEC_EXCHANGE_PKT);
            stream.out_uint32_le(keylen_in_bytes + 8);
            stream.out_copy_bytes(client_encrypted_key, keylen_in_bytes);
            const uint8_t null[8] = {};
            stream.out_copy_bytes(null, 8);
            stream.mark_end();
        }
    };

    struct SecInfoPacket_Recv
    {
        uint32_t basicSecurityHeader;
        SubStream payload;

        SecInfoPacket_Recv(Stream & stream, uint16_t available_len, CryptContext & crypt)
        : payload(stream, 4 + 8)
        {
            const unsigned expected = 12; /* basicSecurityHeader(4) + signature(8) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated SEC_INFO_PKT, expected=%u remains %u",
                   expected, stream.in_remain());
                throw Error(ERR_SEC);
            }

            this->basicSecurityHeader = stream.in_uint32_le() & 0xFFFF;
            if (0 == (this->basicSecurityHeader & SEC::SEC_INFO_PKT)){
                LOG(LOG_INFO, "SEC_INFO_PKT expected, got %x", this->basicSecurityHeader);
            }
            if (0 == (this->basicSecurityHeader & SEC::SEC_ENCRYPT)){
                LOG(LOG_INFO, "SEC_ENCRYPT expected, got %x", this->basicSecurityHeader);
            }

            // skip signature
            stream.in_skip_bytes(8);
            this->payload.resize(stream, stream.in_remain());
            crypt.decrypt(this->payload);
        }
    };


    class SecSpecialPacket_Recv
    {
        public:
        uint32_t flags;
        SubStream payload;
        uint32_t verbose;
        SecSpecialPacket_Recv(Stream & stream, CryptContext & crypt, uint32_t encryptionLevel) 
            : flags(0), payload(stream), verbose(0)
        {
            const unsigned need = 4; /* flags(4) */
            if (!stream.in_check_rem(need)){
                LOG(LOG_ERR, "flags expected: need=%u remains=%u", need, stream.in_remain());
                throw Error(ERR_SEC);
            }
            this->flags = stream.in_uint32_le();

            if (encryptionLevel > 0 && this->flags & SEC::SEC_ENCRYPT){
                if (encryptionLevel == 0){
                    LOG(LOG_ERR, "RDP Packet headers says packet is encrypted, but RDP encryption is disabled");
                    throw Error(ERR_SEC);
                }
                const unsigned need = 8; /* signature(8) */
                if (!stream.in_check_rem(need)){
                    LOG(LOG_ERR, "signature expected: need=%u remains=%u", need, stream.in_remain());
                    throw Error(ERR_SEC);
                }

                TODO("we should check signature")
                stream.in_skip_bytes(8); /* signature */
                this->payload.resize(stream, stream.in_remain());
                if (this->verbose >= 0x200){
                    LOG(LOG_INFO, "Receiving encrypted TPDU");
                    hexdump_c((char*)payload.data, payload.size());
                }
                crypt.decrypt(this->payload);
                if (this->verbose >= 0x80){
                    LOG(LOG_INFO, "Decrypted %u bytes", payload.size());
                    hexdump_c((char*)payload.data, payload.size());
                }
            }
            else{
                TODO("find a better name instead of resize: the function resize is used to set payload to remaining data in stream (start at stream.p, size in_remain)"
                     "maybe something like resize(stream, stream.get_offset(), stream.in_remain())")
                this->payload.resize(stream, stream.in_remain());
            }
        }
    };


    class Sec_Recv
    {
        public:
        uint32_t flags;
        SubStream payload;
        uint32_t verbose;
        Sec_Recv(Stream & stream, CryptContext & crypt, uint32_t encryptionLevel) 
            : flags(0), payload(stream), verbose(0)
        {
            if (encryptionLevel){
                const unsigned need = 4; /* flags(4) */
                if (!stream.in_check_rem(need))
                {
                    LOG(LOG_ERR, "flags expected: need=%u remains=%u",
                        need, stream.in_remain());
                    throw Error(ERR_SEC);
                }
                this->flags = stream.in_uint32_le();
                if (this->flags & SEC::SEC_ENCRYPT){
                    
                    const unsigned need = 8; /* signature(8) */
                    if (!stream.in_check_rem(need))
                    {
                        LOG(LOG_ERR, "signature expected: need=%u remains=%u",
                            need, stream.in_remain());
                        throw Error(ERR_SEC);
                    }

                    TODO("shouldn't we check signature ?")
                    stream.in_skip_bytes(8); /* signature */
                    this->payload.resize(stream, stream.in_remain());
                    if (this->verbose >= 0x200){
                        LOG(LOG_INFO, "Receiving encrypted TPDU");
                        hexdump_c((char*)payload.data, payload.size());
                    }
                    crypt.decrypt(this->payload);
                    if (this->verbose >= 0x80){
                        LOG(LOG_INFO, "Decrypted %u bytes", payload.size());
                        hexdump_c((char*)payload.data, payload.size());
                    }
                }
            }
        }
    };

    struct Sec_Send
    {
        Sec_Send(Stream & stream, Stream & data, uint32_t flags, CryptContext & crypt, uint32_t encryptionLevel){
            flags |= encryptionLevel?SEC_ENCRYPT:0;
            if (flags) {
                stream.out_uint32_le(flags);
            }
            if (flags & SEC_ENCRYPT){
                SubStream signature(stream, stream.get_offset(), 8);

                // <signature> is an output parameter
                crypt.sign(signature, data);
                stream.p += 8;
                crypt.decrypt(data);
            }
            stream.mark_end();
        }
    };
};

#endif
