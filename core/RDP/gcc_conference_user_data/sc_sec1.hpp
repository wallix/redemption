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

   MCS Connect Initial PDU with GCC Conference User Data


*/

#if !defined(__CORE_RDP_GCC_CONFERENCE_USER_DATA_SC_SEC1_HPP__)
#define __CORE_RDP_GCC_CONFERENCE_USER_DATA_SC_SEC1_HPP__

#include "ssl_calls.hpp"
#include "genrandom.hpp"

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

// serverRandomLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of
// the serverRandom field. If the encryptionMethod and encryptionLevel fields
// are both set to 0 then the contents of this field MUST be ignored and the
// serverRandom field MUST NOT be present. Otherwise, this field MUST be set to
// 32 bytes.

// serverCertLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
//  serverCertificate field. If the encryptionMethod and encryptionLevel fields
//  are both set to 0 then the contents of this field MUST be ignored and the
// serverCertificate field MUST NOT be present.

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

enum {

    SEC_TAG_PUBKEY    = 0x0006,
    SEC_TAG_KEYSIG    = 0x0008,

    SEC_RSA_MAGIC     = 0x31415352, /* RSA1 */
};


struct SCSecurityGccUserData {
    uint16_t userDataType;
    uint16_t length;

    enum {
        ENCRYPTION_METHOD_NONE   = 0x00000000,
        ENCRYPTION_METHOD_40BIT  = 0x00000001,
        ENCRYPTION_METHOD_128BIT = 0x00000002,
        ENCRYPTION_METHOD_56BIT  = 0x00000008,
        ENCRYPTION_METHOD_FIPS   = 0x00000010,
    };
    uint32_t encryptionMethod;

    enum {
        ENCRYPTION_LEVEL_NONE              = 0x00000000,
        ENCRYPTION_LEVEL_LOW               = 0x00000001,
        ENCRYPTION_LEVEL_CLIENT_COMPATIBLE = 0x00000002,
        ENCRYPTION_LEVEL_HIGH              = 0x00000003,
        ENCRYPTION_LEVEL_FIPS              = 0x00000004,
    };
    uint32_t encryptionLevel;
    uint32_t serverRandomLen;
    uint32_t serverCertLen;
    uint8_t * serverRandom;

    enum {
        CERT_CHAIN_VERSION_1 = 0x00000001,
        CERT_CHAIN_VERSION_2 = 0x00000002,
    };

    struct {
        uint32_t dwVersion;
//        uint32_t certChainVersion;
//        bool t;

        union {
            struct ServerProprietaryCertificate {
                // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
                //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).
                uint32_t dwSigAlgId;

                // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
                //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).
                uint32_t dwKeyAlgId;

                // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
                //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
                //  (0x0006).
                uint16_t wPublicKeyBlobType;

                // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
                //  of the PublicKeyBlob field.
                uint16_t wPublicKeyBlobLen;

                // PublicKeyBlob (variable): Variable-length server public key bytes, formatted
                //  using the Rivest-Shamir-Adleman (RSA) Public Key structure (section
                //  2.2.1.4.3.1.1.1). The length in bytes is given by the wPublicKeyBlobLen
                //  field.
                uint8_t * PublicKeyBlob;

                // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
                //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
                //  (0x0008).
                uint16_t wSignatureBlobType;

                // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
                //  of the SignatureKeyBlob field.
                uint16_t wSignatureBlobLen;

                // SignatureBlob (variable): Variable-length signature of the certificate
                // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
                // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.
                uint8_t * wSignatureBlob;

            } * proprietary;
            struct X509Certificate {
                uint8_t * blob;
            } * x509;
        } certData;
    } * serverCertificate;

    SCSecurityGccUserData()
    : userDataType(SC_SECURITY)
    , length(12)
    , encryptionMethod(0)
    , encryptionLevel(0)
    , serverRandomLen(0)
    , serverCertLen(0)
    {
    }


    void emit(Stream & stream)
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
    }

    void recv(Stream & stream, uint16_t length)
    {
        this->length = length;
    }

    void log(const char * msg)
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data SC_SECURITY (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "sc_security::encryptionMethod = %u", this->encryptionMethod);
        LOG(LOG_INFO, "sc_security::encryptionLevel  = %u", this->encryptionLevel);
        LOG(LOG_INFO, "sc_security::serverRandomLen  = %u", this->serverRandomLen);
        LOG(LOG_INFO, "sc_security::serverCertLen    = %u", this->serverCertLen);

    }
};

static inline void parse_mcs_data_sc_security(Stream & cr_stream,
                                              uint32_t & encryptionMethod,
                                              uint8_t (& serverRandom)[SEC_RANDOM_SIZE],
                                              uint8_t (& modulus)[SEC_MAX_MODULUS_SIZE],
                                              uint8_t (& exponent)[SEC_EXPONENT_SIZE],
                                              uint32_t & server_public_key_len,
                                              uint8_t (& client_crypt_random)[512],
                                              int & encryptionLevel,
                                              Random * gen)
{
    LOG(LOG_INFO, "SC_SECURITY");

    encryptionMethod = cr_stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
    LOG(LOG_INFO, "encryptionMethod = %u", encryptionMethod);
    encryptionLevel = cr_stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
    LOG(LOG_INFO, "encryptionLevel = %u", encryptionLevel);

    if (encryptionLevel == 0 && encryptionMethod == 0) { /* no encryption */
        LOG(LOG_INFO, "No encryption");
        return;
    }

    ssllib ssl;

// serverRandomLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of
// the serverRandom field. If the encryptionMethod and encryptionLevel fields
// are both set to 0 then the contents of this field MUST be ignored and the
// serverRandom field MUST NOT be present. Otherwise, this field MUST be set to
// 32 bytes.
    uint32_t serverRandomLen = cr_stream.in_uint32_le();
    LOG(LOG_INFO, "serverRandomLen = %u", serverRandomLen);

    if (serverRandomLen != SEC_RANDOM_SIZE) {
        LOG(LOG_ERR, "parse_crypt_info_error: serverRandomLen %d, expected %d", serverRandomLen, SEC_RANDOM_SIZE);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RANDOM_LEN);
    }

// serverCertLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
//  serverCertificate field. If the encryptionMethod and encryptionLevel fields
//  are both set to 0 then the contents of this field MUST be ignored and the
// serverCertificate field MUST NOT be present.
    uint32_t serverCertLen = cr_stream.in_uint32_le();
    LOG(LOG_INFO, "serverCertLen = %u", serverCertLen);

// serverRandom (variable): The variable-length server random value used to
// derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
// given by the serverRandomLen field. If the encryptionMethod and
// encryptionLevel fields are both set to 0 then this field MUST NOT be present.

    cr_stream.in_copy_bytes(serverRandom, serverRandomLen);

// serverCertificate (variable): The variable-length certificate containing the
//  server's public key information. The length in bytes is given by the
// serverCertLen field. If the encryptionMethod and encryptionLevel fields are
// both set to 0 then this field MUST NOT be present.

    /* RSA info */
    uint8_t * end = cr_stream.p + serverCertLen;
    if (end > cr_stream.end) {
        LOG(LOG_ERR,
            "serverCertLen outside of buffer (%u bytes, remains: %u)", serverCertLen, cr_stream.end - cr_stream.p);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RSA_LEN);
    }

    uint32_t dwVersion = cr_stream.in_uint32_le(); /* 1 = RDP4-style, 0x80000002 = X.509 */
    LOG(LOG_INFO, "dwVersion = %x", dwVersion);
    if (dwVersion & SCSecurityGccUserData::CERT_CHAIN_VERSION_1) {
        LOG(LOG_DEBUG, "We're going for the RDP4-style encryption");
        // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
        //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).
        uint32_t dwSigAlgId = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "dwSigAlgId = %u", dwSigAlgId);

        // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
        //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).
        uint32_t dwKeyAlgId = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "dwKeyAlgId = %u", dwKeyAlgId);

        LOG(LOG_DEBUG, "ReceivingPublic key, RDP4-style");
        // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
        //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
        //  (0x0006).
        TODO("put assertion to check type and throw and error if not as expected");
        uint16_t wPublicKeyBlobType = cr_stream.in_uint16_le();
        LOG(LOG_DEBUG, "wPublicKeyBlobType = %u", wPublicKeyBlobType);

        // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
        //  of the PublicKeyBlob field.
        uint16_t wPublicKeyBlobLen = cr_stream.in_uint16_le();
        LOG(LOG_DEBUG, "wPublicKeyBlobLen = %u", wPublicKeyBlobLen);

        uint8_t * next_tag = cr_stream.p + wPublicKeyBlobLen;

        // PublicKeyBlob (variable): Variable-length server public key bytes, formatted
        //  using the Rivest-Shamir-Adleman (RSA) Public Key structure (section
        //  2.2.1.4.3.1.1.1). The length in bytes is given by the wPublicKeyBlobLen
        //  field.


        uint32_t magic = cr_stream.in_uint32_le();
        if (magic != SEC_RSA_MAGIC) {
            LOG(LOG_WARNING, "RSA magic 0x%x", magic);
            throw Error(ERR_SEC_PARSE_PUB_KEY_MAGIC_NOT_OK);
        }
        server_public_key_len = cr_stream.in_uint32_le() - SEC_PADDING_SIZE;

        if ((server_public_key_len < SEC_MODULUS_SIZE)
        ||  (server_public_key_len > SEC_MAX_MODULUS_SIZE)) {
            LOG(LOG_WARNING, "Bad server public key size (%u bits)", server_public_key_len * 8);
            throw Error(ERR_SEC_PARSE_PUB_KEY_MODUL_NOT_OK);
        }
        cr_stream.in_skip_bytes(8); /* modulus_bits, unknown */

        cr_stream.in_copy_bytes(exponent, SEC_EXPONENT_SIZE);
        cr_stream.in_copy_bytes(modulus, server_public_key_len);
        cr_stream.in_skip_bytes(SEC_PADDING_SIZE);
        LOG(LOG_DEBUG, "Got Public key, RDP4-style");

        // This should not be necessary as previous field if fully decoded
        cr_stream.p = next_tag;

        LOG(LOG_DEBUG, "Receiving key sig RDP4-style");
        // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
        //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
        //  (0x0008).
        TODO("put assertion to check type and throw and error if not as expected");
        uint16_t wSignatureBlobType = cr_stream.in_uint16_le();
        LOG(LOG_DEBUG, "wSignatureBlobType = %u", wSignatureBlobType);

        // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
        //  of the SignatureKeyBlob field.
        uint16_t wSignatureBlobLen = cr_stream.in_uint16_le();

        // SignatureBlob (variable): Variable-length signature of the certificate
        // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
        // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.
        cr_stream.in_skip_bytes(wSignatureBlobLen);
        LOG(LOG_DEBUG, "Got key sig RDP4-style");
    }
    else {
        LOG(LOG_DEBUG, "We're going for the RDP5-style encryption");
        uint32_t certcount = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certcount = %u", certcount);

        if (certcount < 2){
            LOG(LOG_DEBUG, "Server didn't send enough X509 certificates");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK);
        }
        for (; certcount > 2; certcount--){
            /* ignore all the certificates between the root and the signing CA */
            LOG(LOG_WARNING, " Ignored certs left: %d", certcount);
            uint32_t ignorelen = cr_stream.in_uint32_le();
            LOG(LOG_WARNING, "Ignored Certificate length is %d", ignorelen);
            X509 *ignorecert = ssl.ssl_cert_read(cr_stream.p, ignorelen);
            cr_stream.in_skip_bytes(ignorelen);
            if (ignorecert == NULL){
                LOG(LOG_WARNING,
                    "got a bad cert: this will probably screw up"
                    " the rest of the communication");
            }
            LOG(LOG_WARNING, "cert #%d (ignored)", certcount);
        }

        /* Do da funky X.509 stuffy

       "How did I find out about this?  I looked up and saw a
       bright light and when I came to I had a scar on my forehead
       and knew about X.500"
       - Peter Gutman in a early version of
       http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
       */

        /* Loading CA_Certificate from server*/
        uint32_t cacert_len = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "CA Certificate length is %d", cacert_len);
        X509 *cacert = ssl.ssl_cert_read(cr_stream.p, cacert_len);
        cr_stream.in_skip_bytes(cacert_len);
        if (NULL == cacert){
            LOG(LOG_DEBUG, "Couldn't load CA Certificate from server");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL);
        }

        /* Loading Certificate from server*/
        uint32_t cert_len = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certificate length is %d", cert_len);
        X509 *server_cert = ssl.ssl_cert_read(cr_stream.p, cert_len);
        cr_stream.in_skip_bytes(cert_len);
        if (NULL == server_cert){
            ssl.ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Couldn't load Certificate from server");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_LOADED);
        }

        /* Matching certificates */
        if (!ssl.ssl_certs_ok(server_cert, cacert)){
            ssl.ssl_cert_free(server_cert);
            ssl.ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Security error CA Certificate invalid");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_MATCH);
        }
        ssl.ssl_cert_free(cacert);
        cr_stream.in_skip_bytes(16); /* Padding */

        RSA *server_public_key = ssl.ssl_cert_to_rkey(server_cert, server_public_key_len);
        LOG(LOG_DEBUG, "Server public key length=%u", (unsigned)server_public_key_len);

        if (NULL == server_public_key){
            LOG(LOG_DEBUG, "Didn't parse X509 correctly");
            ssl.ssl_cert_free(server_cert);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);

        }
        ssl.ssl_cert_free(server_cert);

        LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d",
            server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);

        if ((server_public_key_len < SEC_MODULUS_SIZE) ||
            (server_public_key_len > SEC_MAX_MODULUS_SIZE)){
            LOG(LOG_WARNING, "Bad server public key size (%u bits)", server_public_key_len * 8);
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
        }

        if ((BN_num_bytes(server_public_key->e) > SEC_EXPONENT_SIZE) 
        ||  (BN_num_bytes(server_public_key->n) > SEC_MAX_MODULUS_SIZE)){
            LOG(LOG_WARNING, "Problem extracting RSA exponent, modulus");
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK);
        }
        int len_e = BN_bn2bin(server_public_key->e, (unsigned char*)exponent);
        reverseit(exponent, len_e);
        int len_n = BN_bn2bin(server_public_key->n, (unsigned char*)modulus);
        reverseit(modulus, len_n);

        ssl.rkey_free(server_public_key);
        TODO(" find a way to correctly dispose of garbage at end of buffer")
        /* There's some garbage here we don't care about */
    }
}


//02 0c ec 00 -> TS_UD_HEADER::type = SC_SECURITY, length = 236

//02 00 00 00 -> TS_UD_SC_SEC1::encryptionMethod = 128BIT_ENCRYPTION_FLAG
//02 00 00 00 -> TS_UD_SC_SEC1::encryptionLevel = TS_ENCRYPTION_LEVEL_CLIENT_COMPATIBLE
//20 00 00 00 -> TS_UD_SC_SEC1::serverRandomLen = 32 bytes
//b8 00 00 00 -> TS_UD_SC_SEC1::serverCertLen = 184 bytes

static inline void front_out_gcc_conference_user_data_sc_sec1(Stream & stream,
                                                int encryptionLevel,
                                                uint8_t (&serverRandom)[32],
                                                int rc4_key_size,
                                                uint8_t (&pub_mod)[512],
                                                uint8_t (&pri_exp)[512],
                                                Random * gen)
{
    /*
       For now rsa_keys are not in a configuration file any more, but as we were not changing keys
       the values have been embedded in code and the key generator file removed from source code.

       It will be put back at some later time using a clean parser/writer module and sll calls
       coherent with the remaining of ReDemPtion code. For reference to historical key generator code
       look for utils/keygen.cpp in old repository code.

       references for RSA Keys:

       http://www.securiteam.com/windowsntfocus/5EP010KG0G.html

    */

    uint8_t rsa_keys_pub_exp[4] = {
        0x01,0x00,0x01,0x00
    };
    uint8_t rsa_keys_pub_mod[64] = {
        0x67, 0xab, 0x0e, 0x6a, 0x9f, 0xd6, 0x2b, 0xa3, 0x32, 0x2f, 0x41, 0xd1, 0xce, 0xee, 0x61, 0xc3,
        0x76, 0x0b, 0x26, 0x11, 0x70, 0x48, 0x8a, 0x8d, 0x23, 0x81, 0x95, 0xa0, 0x39, 0xf7, 0x5b, 0xaa,
        0x3e, 0xf1, 0xed, 0xb8, 0xc4, 0xee, 0xce, 0x5f, 0x6a, 0xf5, 0x43, 0xce, 0x5f, 0x60, 0xca, 0x6c,
        0x06, 0x75, 0xae, 0xc0, 0xd6, 0xa4, 0x0c, 0x92, 0xa4, 0xc6, 0x75, 0xea, 0x64, 0xb2, 0x50, 0x5b
    };
    uint8_t rsa_keys_pub_sig[64] = {
        0x6a, 0x41, 0xb1, 0x43, 0xcf, 0x47, 0x6f, 0xf1, 0xe6, 0xcc, 0xa1, 0x72, 0x97, 0xd9, 0xe1, 0x85,
        0x15, 0xb3, 0xc2, 0x39, 0xa0, 0xa6, 0x26, 0x1a, 0xb6, 0x49, 0x01, 0xfa, 0xa6, 0xda, 0x60, 0xd7,
        0x45, 0xf7, 0x2c, 0xee, 0xe4, 0x8e, 0x64, 0x2e, 0x37, 0x49, 0xf0, 0x4c, 0x94, 0x6f, 0x08, 0xf5,
        0x63, 0x4c, 0x56, 0x29, 0x55, 0x5a, 0x63, 0x41, 0x2c, 0x20, 0x65, 0x95, 0x99, 0xb1, 0x15, 0x7c
    };
    uint8_t rsa_keys_pri_exp[64] = {
        0x41, 0x93, 0x05, 0xB1, 0xF4, 0x38, 0xFC, 0x47, 0x88, 0xC4, 0x7F, 0x83, 0x8C, 0xEC, 0x90, 0xDA,
        0x0C, 0x8A, 0xB5, 0xAE, 0x61, 0x32, 0x72, 0xF5, 0x2B, 0xD1, 0x7B, 0x5F, 0x44, 0xC0, 0x7C, 0xBD,
        0x8A, 0x35, 0xFA, 0xAE, 0x30, 0xF6, 0xC4, 0x6B, 0x55, 0xA7, 0x65, 0xEF, 0xF4, 0xB2, 0xAB, 0x18,
        0x4E, 0xAA, 0xE6, 0xDC, 0x71, 0x17, 0x3B, 0x4C, 0xC2, 0x15, 0x4C, 0xF7, 0x81, 0xBB, 0xF0, 0x03
    };
    gen->random(serverRandom, 32);

    uint8_t pub_sig[512];

    memcpy(pub_mod, rsa_keys_pub_mod, 64);
    memcpy(pub_sig, rsa_keys_pub_sig, 64);
    memcpy(pri_exp, rsa_keys_pri_exp, 64);

    stream.out_uint16_le(SC_SECURITY);
    stream.out_uint16_le(236); // length, including tag and length fields
    stream.out_uint32_le(rc4_key_size); // key len 1 = 40 bit 2 = 128 bit
    // crypt level 1 = low 2 = medium, 3 = high
    stream.out_uint32_le(encryptionLevel);

    stream.out_uint32_le(32);  // random len
    stream.out_uint32_le(184); // len of rsa info(certificate)
    stream.out_copy_bytes(serverRandom, 32);
    /* here to end is certificate */
    /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ */
    /* TermService\Parameters\Certificate */
    stream.out_uint32_le(1);
    stream.out_uint32_le(1);
    stream.out_uint32_le(1);

    // 96 bytes long of sec_tag pubkey
    stream.out_uint16_le(SEC_TAG_PUBKEY);
    stream.out_uint16_le(92); // length
    stream.out_uint32_le(SEC_RSA_MAGIC);
    stream.out_uint32_le(72); /* 72 bytes modulus len */
    stream.out_uint32_be(0x00020000);
    stream.out_uint32_be(0x3f000000);
    stream.out_copy_bytes(rsa_keys_pub_exp, 4); /* pub exp */
    stream.out_copy_bytes(pub_mod, 64); /* pub mod */
    stream.out_clear_bytes(8); /* pad */

    // 76 bytes long of sec_tag_pub_sig
    stream.out_uint16_le(SEC_TAG_KEYSIG);
    stream.out_uint16_le(72); /* len */
    stream.out_copy_bytes(pub_sig, 64); /* pub sig */
    stream.out_clear_bytes(8); /* pad */
    /* end certificate */
}

#endif
