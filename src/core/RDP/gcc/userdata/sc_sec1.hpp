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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/error.hpp"
#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/crypto/ssl_lib.hpp"
#include "utils/stream.hpp"
#include "utils/log.hpp"

#include <cinttypes>

#ifndef __EMSCRIPTEN__
# include <openssl/x509.h>
#endif

namespace GCC { namespace UserData {

// 2.2.1.4.3 Server Security Data (TS_UD_SC_SEC1)
// ==============================================

// The TS_UD_SC_SEC1 data block returns negotiated security-related information
// to the client. See section 5.3.2 for a detailed discussion of how this
// information is used.

// header (4 bytes): GCC user data block header, as specified in User Data
// Header (section 2.2.1.3.1). The User Data Header type field MUST be set to
// SC_SECURITY (0x0C02).

// encryptionMethod (4 bytes): A 32-bit, unsigned integer. The selected
// cryptographic method to use for the session. When Enhanced RDP Security
// (section 5.4) is being used, this field MUST be set to ENCRYPTION_METHOD_NONE
// (0).

// +-------------------------------------+-------------------------------------+
// | 0x00000000 ENCRYPTION_METHOD_NONE   | No encryption or Message            |
// |                                     | Authentication Codes (MACs) will be |
// |                                     | used.                               |
// +-------------------------------------+-------------------------------------+
// | 0x00000001 ENCRYPTION_METHOD_40BIT  | 40-bit session keys will be used to |
// |                                     | encrypt data (with RC4) and generate|
// |                                     | MACs.                               |
// +-------------------------------------+-------------------------------------+
// | 0x00000002 ENCRYPTION_METHOD_128BIT | 128-bit session keys will be used   |
// |                                     | to encrypt data (with RC4) and      |
// |                                     | generate MACs.                      |
// +-------------------------------------+-------------------------------------+
// | 0x00000008 ENCRYPTION_METHOD_56BIT  | 56-bit session keys will be used to |
// |                                     | encrypt data (with RC4) and generate|
// |                                     | MACs.                               |
// +-------------------------------------+-------------------------------------+
// | 0x00000010 ENCRYPTION_METHOD_FIPS   | All encryption and Message          |
// |                                     | Authentication Code                 |
// |                                     | generation routines will            |
// |                                     | be FIPS 140-1 compliant.            |
// +-------------------------------------+-------------------------------------+

// encryptionLevel (4 bytes): A 32-bit unsigned integer. It describes the
//  encryption behavior to use for the session. When Enhanced RDP Security
//  (section 5.4) is being used, this field MUST be set to ENCRYPTION_LEVEL_NONE
//  (0).

// +------------------------------------+------------+
// | ENCRYPTION_LEVEL_NONE              | 0x00000000 |
// +------------------------------------+------------+
// | ENCRYPTION_LEVEL_LOW               | 0x00000001 |
// +------------------------------------+------------+
// | ENCRYPTION_LEVEL_CLIENT_COMPATIBLE | 0x00000002 |
// +------------------------------------+------------+
// | ENCRYPTION_LEVEL_HIGH              | 0x00000003 |
// +------------------------------------+------------+
// | ENCRYPTION_LEVEL_FIPS              | 0x00000004 |
// +------------------------------------+------------+

// See section 5.3.1 for a description of each of the low, client-compatible,
// high, and FIPS encryption levels.

// serverRandomLen (4 bytes): An optional 32-bit, unsigned integer that specifies
// the size in bytes of the serverRandom field. If the encryptionMethod and
// encryptionLevel fields are both set to zero, then this field MUST NOT be present
// and the length of the serverRandom field MUST be zero. If either the
// encryptionMethod or encryptionLevel field is non-zero, this field MUST be set
// to 0x00000020 (32 bytes serverRandom).

// serverCertLen (4 bytes): An optional 32-bit, unsigned integer that specifies
// the size in bytes of the serverCertificate field. If the encryptionMethod and
// encryptionLevel fields are both set to zero, then this field MUST NOT be present
// and the length of the serverCertificate field MUST be zero.

// serverRandom (variable): The variable-length server random value used to
// derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
// given by the serverRandomLen field. If the encryptionMethod and
// encryptionLevel fields are both set to 0 then this field MUST NOT be present.

// serverCertificate (variable): The variable-length certificate containing the
//  server's public key information. The length in bytes is given by the
// serverCertLen field. If the encryptionMethod and encryptionLevel fields are
// both set to 0 then this field MUST NOT be present.

// 2.2.1.4.3.1 Server Certificate (SERVER_CERTIFICATE)
// ===================================================

// The SERVER_CERTIFICATE structure describes the generic server certificate
// structure to which all server certificates present in the Server Security
// Data (section 2.2.1.4.3) conform.

// dwVersion (4 bytes): A 32-bit, unsigned integer.
// dwVersion::certChainVersion (31 bits): A 31-bit field. The certificate version.

// +---------------------------------+-----------------------------------------+
// | 0x00000001 CERT_CHAIN_VERSION_1 | The certificate contained in the        |
// |                                 | certData field is a Server Proprietary  |
// |                                 | Certificate (section 2.2.1.4.3.1.1).    |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 CERT_CHAIN_VERSION_2 | The certificate contained in the        |
// |                                 | certData field is an X.509 Certificate  |
// |                                 | (see section 5.3.3.2).                  |
// +---------------------------------+-----------------------------------------+

// dwVersion::t (1 bit): A 1-bit field. Indicates whether the certificate contained in the
//  certData field has been permanently or temporarily issued to the server.

// +---+----------------------------------------------------------------------+
// | 0 | The certificate has been permanently issued to the server.           |
// +---+----------------------------+-----------------------------------------+
// | 1 | The certificate has been temporarily issued to the server.           |
// +---+----------------------------+-----------------------------------------+

// certData (variable): Certificate data. The format of this certificate data is
//  determined by the dwVersion field.

// 2.2.1.4.3.1.1 Server Proprietary Certificate (PROPRIETARYSERVERCERTIFICATE)
// ===========================================================================

// The PROPRIETARYSERVERCERTIFICATE structure describes a signed certificate
// containing the server's public key and conforming to the structure of a
// Server Certificate (section 2.2.1.4.3.1). For a detailed description of
// Proprietary Certificates, see section 5.3.3.1.

// dwVersion (4 bytes): A 32-bit, unsigned integer. The certificate version
//  number. This field MUST be set to CERT_CHAIN_VERSION_1 (0x00000001).

// dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
//  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).

// dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
//  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).

// wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
//  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
//  (0x0006).

// wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
//  of the PublicKeyBlob field.

// PublicKeyBlob (variable): Variable-length server public key bytes, formatted
//  using the Rivest-Shamir-Adleman (RSA) Public Key structure (section
//  2.2.1.4.3.1.1.1). The length in bytes is given by the wPublicKeyBlobLen
//  field.

// wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
//  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
//  (0x0008).

// wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
//  of the SignatureKeyBlob field.

// SignatureBlob (variable): Variable-length signature of the certificate
// created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
// 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.

// 2.2.1.4.3.1.1.1 RSA Public Key (RSA_PUBLIC_KEY)
// ===============================================
// The structure used to describe a public key in a Proprietary Certificate
// (section 2.2.1.4.3.1.1).

// magic (4 bytes): A 32-bit, unsigned integer. The sentinel value. This field
//  MUST be set to 0x31415352.

// keylen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
//  modulus field. This value is directly related to the bitlen field and MUST
//  be ((bitlen / 8) + 8) bytes.

// bitlen (4 bytes): A 32-bit, unsigned integer. The number of bits in the
//  public key modulus.

// datalen (4 bytes): A 32-bit, unsigned integer. The maximum number of bytes
//  that can be encoded using the public key.

// pubExp (4 bytes): A 32-bit, unsigned integer. The public exponent of the
//  public key.

// modulus (variable): A variable-length array of bytes containing the public
//  key modulus. The length in bytes of this field is given by the keylen field.
//  The modulus field contains all (bitlen / 8) bytes of the public key modulus
//  and 8 bytes of zero padding (which MUST follow after the modulus bytes).

// 5.3.3.1.1 Terminal Services Signing Key
// =======================================
// The modulus, private exponent, and public exponent of the 512-bit Terminal Services asymmetric
// key used for signing Proprietary Certificates with the RSA algorithm are detailed as follows.

// 64 byte Modulus (n)

//        0x3d, 0x3a, 0x5e, 0xbd, 0x72, 0x43, 0x3e, 0xc9,
//        0x4d, 0xbb, 0xc1, 0x1e, 0x4a, 0xba, 0x5f, 0xcb,
//        0x3e, 0x88, 0x20, 0x87, 0xef, 0xf5, 0xc1, 0xe2,
//        0xd7, 0xb7, 0x6b, 0x9a, 0xf2, 0x52, 0x45, 0x95,
//        0xce, 0x63, 0x65, 0x6b, 0x58, 0x3a, 0xfe, 0xef,
//        0x7c, 0xe7, 0xbf, 0xfe, 0x3d, 0xf6, 0x5c, 0x7d,
//        0x6c, 0x5e, 0x06, 0x09, 0x1a, 0xf5, 0x61, 0xbb,
//        0x20, 0x93, 0x09, 0x5f, 0x05, 0x6d, 0xea, 0x87

// 64-byte Private Exponent (d) :

//        0x87, 0xa7, 0x19, 0x32, 0xda, 0x11, 0x87, 0x55,
//        0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xf8,
//        0x24, 0x3e, 0xe6, 0xfa, 0xe9, 0x67, 0x49, 0x94,
//        0xcf, 0x92, 0xcc, 0x33, 0x99, 0xe8, 0x08, 0x60,
//        0x17, 0x9a, 0x12, 0x9f, 0x24, 0xdd, 0xb1, 0x24,
//        0x99, 0xc7, 0x3a, 0xb8, 0x0a, 0x7b, 0x0d, 0xdd,
//        0x35, 0x07, 0x79, 0x17, 0x0b, 0x51, 0x9b, 0xb3,
//        0xc7, 0x10, 0x01, 0x13, 0xe7, 0x3f, 0xf3, 0x5f

//  4-byte Public Exponent (e):

//        0x5b, 0x7b, 0x88, 0xc0

/// The enumerated integers are in little-endian byte order. The public
// key is the pair (e, n), while the private key is the pair (d, n)

// 5.3.3.1.2 Signing a Proprietary Certificate
// ========================================

// The Proprietary Certificate is signed by using RSA to encrypt the hash
// of the first six fields with the Terminal Services private signing key
// (specified in section 5.3.3.1.1) and then appending the result to the end
// of the certificate. Mathematically the signing operation is formulated as follows:

//    s = m^d mod n

// Where

//    s = signature
//    m = hash of first six fields of certificate
//    d = private exponent
//    n = modulus

// The structure of the Proprietary Certificate is detailed in section 2.2.1.4.3.1.1.
// The structure of the public key embedded in the certificate is described in 2.2.1.4.3.1.1.1.
// An example of public key bytes (in little-endian format) follows.

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

//      PublicKeyBlob = wBlobType + wBlobLen + PublicKeyBytes
//      hash = MD5(dwVersion + dwSigAlgID + dwKeyAlgID + PublicKeyBlob)

// Because the Terminal Services private signing key has a 64-byte modulus, the
// maximum number of bytes that can be encoded by using the key is 63 (the size
// of the modulus, in bytes, minus 1). An array of 63 bytes is created and
//initialized as follows.

// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0x00 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0x01

// The 128-bit MD5 hash is copied into the first 16 bytes of the array.
// For example, assume that the generated hash is as follows.

// 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
// 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f

// The byte array will appear as follows after copying in the 16 bytes of the MD5 hash.

// 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
// 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f
// 0x00 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
// 0xff 0xff 0xff 0xff 0xff 0xff 0x01

// The 63-byte array is then treated as an unsigned little-endian integer and signed
// with the Terminal Services private key by using RSA. The resultant signature must
// be in little-endian format before appending it to the Proprietary Certificate
// structure. The final structure of the certificate must conform to the specification
// in section 2.2.1.4.3.1.1. This means that fields 7 through to 9 will be the signature
// BLOB type, the number of bytes in the signature and the actual signature bytes respectively.
// The BLOB type and number of bytes must be in little-endian format.

// Example Java source code that shows how to use a private 64-byte asymmetric key to sign an
// array of 63 bytes using RSA is presented in section 4.9. The code also shows how to use the
// associated public key to verify the signature.

struct SCSecurity {
    uint16_t userDataType{SC_SECURITY};
    uint16_t length{236};

    // TODO use enum class
    uint32_t encryptionMethod{0};

    enum {
        ENCRYPTION_LEVEL_NONE              = 0x00000000,
        ENCRYPTION_LEVEL_LOW               = 0x00000001,
        ENCRYPTION_LEVEL_CLIENT_COMPATIBLE = 0x00000002,
        ENCRYPTION_LEVEL_HIGH              = 0x00000003,
        ENCRYPTION_LEVEL_FIPS              = 0x00000004
    };
    // TODO use enum class
    uint32_t encryptionLevel{0};  // crypt level 0 = none, 1 = low 2 = medium, 3 = high
    uint32_t serverRandomLen{SEC_RANDOM_SIZE};
    uint8_t serverRandom[SEC_RANDOM_SIZE];

    uint32_t serverCertLen{184};

    uint8_t pri_exp[64];
    uint8_t pub_sig[64];

    enum { CERT_CHAIN_VERSION_1 = 0x00000001
         , CERT_CHAIN_VERSION_2 = 0x00000002
    };

    enum { SIGNATURE_ALG_RSA = 1
         , KEY_EXCHANGE_ALG_RSA = 1
         , BB_RSA_KEY_BLOB = 0x0006
         , BB_RSA_SIGNATURE_BLOB = 0x0008
         , RSA_MAGIC     = 0x31415352 /* RSA1 */
    };

    // really proprietaryCertificate and X509Certificate should be some union (sum) controlled by dwVersion
    // anyway, it's not really usefull here to bother about small lost space
    // (real alternative would be to dynamically allocate memory, buth memory allocation also has it's costs)

    uint32_t dwVersion{CERT_CHAIN_VERSION_1};
    bool temporary{false};

    struct ServerProprietaryCertificate {
        // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
        //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).
        uint32_t dwSigAlgId{SIGNATURE_ALG_RSA};

        // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
        //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).
        uint32_t dwKeyAlgId{KEY_EXCHANGE_ALG_RSA};

        // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
        //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
        //  (0x0006).
        uint16_t wPublicKeyBlobType{BB_RSA_KEY_BLOB};

        // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
        //  of the PublicKeyBlob field.
        uint16_t wPublicKeyBlobLen{92};

        // PublicKeyBlob (variable): Variable-length server public key bytes, formatted
        //  using the Rivest-Shamir-Adleman (RSA) Public Key structure (section
        //  2.2.1.4.3.1.1.1). The length in bytes is given by the wPublicKeyBlobLen
        //  field.
        struct PublicKeyBlob {
            // 2.2.1.4.3.1.1.1 RSA Public Key (RSA_PUBLIC_KEY)
            // ===============================================
            // The structure used to describe a public key in a Proprietary Certificate
            // (section 2.2.1.4.3.1.1).

            // magic (4 bytes): A 32-bit, unsigned integer. The sentinel value. This field
            //  MUST be set to 0x31415352.
            uint32_t magic{RSA_MAGIC};

            // keylen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
            //  modulus field. This value is directly related to the bitlen field and MUST
            //  be ((bitlen / 8) + 8) bytes.
            uint32_t keylen{72};

            // bitlen (4 bytes): A 32-bit, unsigned integer. The number of bits in the
            //  public key modulus.
            uint32_t bitlen{512};

            // datalen (4 bytes): A 32-bit, unsigned integer. The maximum number of bytes
            //  that can be encoded using the public key.
            // This value is directly related to the bitlen field and MUST be ((bitlen / 8) - 1) bytes.
            uint32_t datalen{63};

            // pubExp (4 bytes): A 32-bit, unsigned integer. The public exponent of the
            //  public key.
            uint8_t pubExp[4];

            // modulus (variable): A variable-length array of bytes containing the public
            //  key modulus. The length in bytes of this field is given by the keylen field.
            //  The modulus field contains all (bitlen / 8) bytes of the public key modulus
            //  and 8 bytes of zero padding (which MUST follow after the modulus bytes).
            uint8_t modulus[/*72*/264];

            PublicKeyBlob()

            = default;
        } RSAPK;

        // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
        //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
        //  (0x0008).
        uint16_t wSignatureBlobType{BB_RSA_SIGNATURE_BLOB};

        // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
        //  of the SignatureKeyBlob field.
        uint16_t wSignatureBlobLen{72};

        // SignatureBlob (variable): Variable-length signature of the certificate
        // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
        // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.
        uint8_t wSignatureBlob[72];

        ServerProprietaryCertificate()
        = default;
    } proprietaryCertificate;

#ifndef __EMSCRIPTEN__
    struct X509CertificateChain {
        uint32_t certCount {};
        X509 * certs[32] {}; // a chain of at most 32 certificates, should be enough

        ~X509CertificateChain()
        {
            for (auto&& cert: this->certs){
                if (cert){
                    X509_free(cert);
                    cert = nullptr;
                }
            }
        }
    } x509 {};
#endif

    SCSecurity() = default;

    SCSecurity(SCSecurity const &) = delete;
    SCSecurity & operator = (SCSecurity const &) = delete;

    ~SCSecurity() = default;

    void emit(OutStream & stream) /* TODO const*/
    {
        stream.out_uint16_le(SC_SECURITY);

        if ((this->encryptionMethod == 0) && (this->encryptionLevel == 0)){
            this->length = 12;
            this->serverRandomLen = 0;
            this->encryptionLevel = 0;
            stream.out_uint16_le(this->length); // length, including tag and length fields
            stream.out_uint32_le(this->encryptionMethod); // encryptionMethod
            stream.out_uint32_le(this->encryptionLevel); // encryptionLevel
        }
        else {
            stream.out_uint16_le(this->length); // length, including tag and length fields
            stream.out_uint32_le(this->encryptionMethod); // key len 1 = 40 bit 2 = 128 bit
            stream.out_uint32_le(this->encryptionLevel);
            stream.out_uint32_le(this->serverRandomLen);  // random len
            stream.out_uint32_le(this->serverCertLen); // len of rsa info(certificate)
            stream.out_copy_bytes(this->serverRandom, this->serverRandomLen);

            // --------------------------------------------------------------
            /* here to end is certificate */
            /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\TermService\Parameters\Certificate */
            stream.out_uint32_le(this->dwVersion|(this->temporary << 31));

            if ((this->dwVersion & 0x7FFFFFFF) == CERT_CHAIN_VERSION_1){
                stream.out_uint32_le(this->proprietaryCertificate.dwSigAlgId);
                stream.out_uint32_le(this->proprietaryCertificate.dwKeyAlgId);

                stream.out_uint16_le(this->proprietaryCertificate.wPublicKeyBlobType);
                stream.out_uint16_le(this->proprietaryCertificate.wPublicKeyBlobLen);
                stream.out_uint32_le(this->proprietaryCertificate.RSAPK.magic);
                stream.out_uint32_le(this->proprietaryCertificate.RSAPK.keylen);
                stream.out_uint32_le(this->proprietaryCertificate.RSAPK.bitlen);
                stream.out_uint32_le(this->proprietaryCertificate.RSAPK.datalen);
                stream.out_copy_bytes(this->proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
                stream.out_copy_bytes(this->proprietaryCertificate.RSAPK.modulus,
                                      /*SEC_MODULUS_SIZE*/this->proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE);
                stream.out_clear_bytes(SEC_PADDING_SIZE);

                stream.out_uint16_le(this->proprietaryCertificate.wSignatureBlobType);
                stream.out_uint16_le(this->proprietaryCertificate.wSignatureBlobLen); /* len */
                stream.out_copy_bytes(this->proprietaryCertificate.wSignatureBlob, 64); /* pub sig */
                stream.out_clear_bytes(8); /* pad */
            }
            else {
                // send chain of certificates
            }
            /* end certificate */
            // --------------------------------------------------------------
        }
    }

    void recv(InStream & stream)
    {
        if (!stream.in_check_rem(4)){
            LOG(LOG_ERR, "SC_SECURITY short header");
            throw Error(ERR_GCC);
        }
        this->userDataType = stream.in_uint16_le();
        this->length = stream.in_uint16_le();
        if ((this->length <= 4) || !stream.in_check_rem(this->length - 4)){
            LOG(LOG_ERR, "SC_SECURITY bad header length");
            throw Error(ERR_GCC);
        }
        this->encryptionMethod = stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
        this->encryptionLevel = stream.in_uint32_le();  /* 1 = low, 2 = medium, 3 = high */

        if ((this->encryptionMethod == 0) && (this->encryptionLevel == 0)){
            this->serverRandomLen = 0;
            this->encryptionLevel = 0;
        }
        if (((this->encryptionLevel == 0) || (this->encryptionMethod == 0))
        && (this->length != 12)) {
            LOG(LOG_ERR, "SC_SECURITY bad header length, no encryption length=%d", this->length);
            throw Error(ERR_GCC);
        }
        if (this->length == 12) {
            if ((this->encryptionLevel != 0)||(this->encryptionMethod != 0)){
                LOG(LOG_ERR, "SC_SECURITY short header with encription method=%u level=%u",
                    this->encryptionMethod, this->encryptionLevel);
                throw Error(ERR_GCC);
            }
            return;
        }

        if ((this->encryptionLevel == 0) || (encryptionMethod == 0)){
            LOG(LOG_ERR, "SC_SECURITY encryption header but no encryption setted : method=%u level=%u",
                this->encryptionMethod, this->encryptionLevel);
            throw Error(ERR_GCC);
        }

        // serverRandomLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of
        // the serverRandom field. If the encryptionMethod and encryptionLevel fields
        // are both set to 0 then the contents of this field MUST be ignored and the
        // serverRandom field MUST NOT be present. Otherwise, this field MUST be set to
        // 32 bytes.
        this->serverRandomLen = stream.in_uint32_le();

        // serverCertLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
        //  serverCertificate field. If the encryptionMethod and encryptionLevel fields
        //  are both set to 0 then the contents of this field MUST be ignored and the
        // serverCertificate field MUST NOT be present.

        this->serverCertLen = stream.in_uint32_le();

        if (this->serverRandomLen != SEC_RANDOM_SIZE) {
            LOG(LOG_ERR, "SCSecutity recv: serverRandomLen %u, expected %u",
                 this->serverRandomLen, SEC_RANDOM_SIZE);
            throw Error(ERR_GCC);
        }

        if (!stream.in_check_rem(this->serverCertLen)) {
            LOG(LOG_ERR, "SCSecutity recv: serverCertLen %" PRIu32 ", not enough data available (%zu)",
                 this->serverCertLen, stream.in_remain());
            throw Error(ERR_GCC);
        }
        // serverRandom (variable): The variable-length server random value used to
        // derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
        // given by the serverRandomLen field. If the encryptionMethod and
        // encryptionLevel fields are both set to 0 then this field MUST NOT be present.

        stream.in_copy_bytes(this->serverRandom, this->serverRandomLen);
        uint32_t certType = stream.in_uint32_le();
        this->dwVersion = certType & 0x7FFFFFFF;
        this->temporary = 0 != (certType & 0x80000000);
        if (this->dwVersion == CERT_CHAIN_VERSION_1){
            // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
            //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).
            this->proprietaryCertificate.dwSigAlgId = stream.in_uint32_le();

            // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
            //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).
            this->proprietaryCertificate.dwKeyAlgId = stream.in_uint32_le();

            // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
            //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
            //  (0x0006).
            this->proprietaryCertificate.wPublicKeyBlobType = stream.in_uint16_le();

            // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
            //  of the PublicKeyBlob field.
            this->proprietaryCertificate.wPublicKeyBlobLen = stream.in_uint16_le();

            if ((this->proprietaryCertificate.wPublicKeyBlobLen != 92) &&
                (this->proprietaryCertificate.wPublicKeyBlobLen != 284)) {
                LOG(LOG_ERR, "Unsupported RSA Key blob len in certificate %u (expected 92 or 284)",
                    this->proprietaryCertificate.wPublicKeyBlobLen);
                throw Error(ERR_GCC);
            }
            LOG(LOG_INFO, "RSA Key blob len in certificate is %u", this->proprietaryCertificate.wPublicKeyBlobLen);

            this->proprietaryCertificate.RSAPK.magic = stream.in_uint32_le();
            if (this->proprietaryCertificate.RSAPK.magic != RSA_MAGIC) {
                    LOG(LOG_ERR, "Bad RSA magic 0x%x", this->proprietaryCertificate.RSAPK.magic);
                    throw Error(ERR_GCC);
            }
            this->proprietaryCertificate.RSAPK.keylen = stream.in_uint32_le();
            if ((this->proprietaryCertificate.RSAPK.keylen != 72) &&
                (this->proprietaryCertificate.RSAPK.keylen != 264)) {
                LOG(LOG_WARNING, "Bad server public key len in certificate %u (expected 72 or 264)",
                    this->proprietaryCertificate.RSAPK.keylen);
                throw Error(ERR_GCC);
            }
            this->proprietaryCertificate.RSAPK.bitlen = stream.in_uint32_le();
            this->proprietaryCertificate.RSAPK.datalen = stream.in_uint32_le();
            stream.in_copy_bytes(this->proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
            stream.in_copy_bytes(this->proprietaryCertificate.RSAPK.modulus,
                                 /*SEC_MODULUS_SIZE + SEC_PADDING_SIZE*/this->proprietaryCertificate.RSAPK.keylen);


            // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
            //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
            //  (0x0008).
            this->proprietaryCertificate.wSignatureBlobType = stream.in_uint16_le();
            if (this->proprietaryCertificate.wSignatureBlobType != BB_RSA_SIGNATURE_BLOB){
                LOG(LOG_ERR, "RSA Signature blob expected, got %x",
                    this->proprietaryCertificate.wSignatureBlobType);
                throw Error(ERR_GCC);
            }

            // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
            //  of the SignatureKeyBlob field.
            this->proprietaryCertificate.wSignatureBlobLen = stream.in_uint16_le();

            // SignatureBlob (variable): Variable-length signature of the certificate
            // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
            // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.
            if (this->proprietaryCertificate.wSignatureBlobLen != 72){
                LOG(LOG_ERR, "RSA Signature blob len too large in certificate %u (expected 72)",
                    this->proprietaryCertificate.wSignatureBlobLen);
                throw Error(ERR_GCC);
            }
            stream.in_copy_bytes(this->proprietaryCertificate.wSignatureBlob, 64 + SEC_PADDING_SIZE);
        }
        else {
#ifndef __EMSCRIPTEN__
            this->x509.certCount = stream.in_uint32_le();
            if (this->x509.certCount > 32){
                LOG(LOG_ERR, "More than 32 certificates (count=%u), this is probably an attack",
                    this->x509.certCount);
                throw Error(ERR_GCC);
            }

            for (size_t i = 0; i < this->x509.certCount ; i++){
                auto const len = stream.in_uint32_le();
                if (this->x509.certs[i]) {
                    X509_free(this->x509.certs[i]);
                }
                auto p = stream.get_current();
                this->x509.certs[i] = d2i_X509(nullptr, &p, len);
                stream.in_skip_bytes(p - stream.get_current());
            }
            stream.in_skip_bytes(16); /* Padding */
#else
            LOG(LOG_ERR, "SCSecurity recv X509 certificate not implemented");
            throw Error(ERR_GCC);
#endif
        }
    }

    void log(const char * msg) const
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data SC_SECURITY (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "sc_security::encryptionMethod = %u", this->encryptionMethod);
        LOG(LOG_INFO, "sc_security::encryptionLevel  = %u", this->encryptionLevel);
        if (this->length == 12) { return; }
        LOG(LOG_INFO, "sc_security::serverRandomLen  = %u", this->serverRandomLen);
        LOG(LOG_INFO, "sc_security::serverCertLen    = %u", this->serverCertLen);
        LOG(LOG_INFO, "sc_security::dwVersion = %x", this->dwVersion);
        LOG(LOG_INFO, "sc_security::temporary = %s", this->temporary?"true":"false");
        if (this->dwVersion == GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
            LOG(LOG_INFO, "sc_security::RDP4-style encryption");
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::dwSigAlgId = %u", this->proprietaryCertificate.dwSigAlgId);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::dwKeyAlgId = %u", this->proprietaryCertificate.dwKeyAlgId);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::wPublicKeyBlobType = %u",
                 this->proprietaryCertificate.wPublicKeyBlobType);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::wPublicKeyBlobLen = %u",
                this->proprietaryCertificate.wPublicKeyBlobLen);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::magic = %u",
                this->proprietaryCertificate.RSAPK.magic);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::keylen = %u",
                this->proprietaryCertificate.RSAPK.keylen);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::bitlen = %u",
                this->proprietaryCertificate.RSAPK.bitlen);
            LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::datalen = %u",
                this->proprietaryCertificate.RSAPK.datalen);
        }
        else {
            LOG(LOG_INFO, "sc_security::RDP5-style encryption");
        }
    }

    static const char * get_encryptionLevel_name(uint32_t encryptionLevel) {
        switch (encryptionLevel) {
            case GCC::UserData::SCSecurity::ENCRYPTION_LEVEL_NONE:              return "ENCRYPTION_LEVEL_NONE";
            case GCC::UserData::SCSecurity::ENCRYPTION_LEVEL_LOW:               return "ENCRYPTION_LEVEL_LOW";
            case GCC::UserData::SCSecurity::ENCRYPTION_LEVEL_CLIENT_COMPATIBLE: return "ENCRYPTION_LEVEL_CLIENT_COMPATIBLE";
            case GCC::UserData::SCSecurity::ENCRYPTION_LEVEL_HIGH:              return "ENCRYPTION_LEVEL_HIGH";
            case GCC::UserData::SCSecurity::ENCRYPTION_LEVEL_FIPS:              return "PAKID_CORE_DEVICE_REPLY";
        }

        return "<unknown>";
    }

    enum : uint32_t {
        ENCRYPTION_METHOD_NONE   = 0x00000000
      , ENCRYPTION_METHOD_40BIT  = 0x00000001
      , ENCRYPTION_METHOD_128BIT = 0x00000002
      , ENCRYPTION_METHOD_56BIT  = 0x00000008
      , ENCRYPTION_METHOD_FIPS   = 0x00000010
    };

    static const char * get_encryptionMethod_name(uint32_t encryptionMethod) {
        switch (encryptionMethod) {
            case GCC::UserData::SCSecurity::ENCRYPTION_METHOD_NONE:   return "ENCRYPTION_METHOD_NONE";
            case GCC::UserData::SCSecurity::ENCRYPTION_METHOD_40BIT:  return "ENCRYPTION_METHOD_40BIT";
            case GCC::UserData::SCSecurity::ENCRYPTION_METHOD_128BIT: return "ENCRYPTION_METHOD_128BIT";
            case GCC::UserData::SCSecurity::ENCRYPTION_METHOD_56BIT:  return "ENCRYPTION_METHOD_56BIT";
            case GCC::UserData::SCSecurity::ENCRYPTION_METHOD_FIPS:   return "ENCRYPTION_METHOD_FIPS";
        }

        return "<unknown>";
    }

};

} // namespace UserData
} // namespace GCC
