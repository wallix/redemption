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

#include "sec_utils.hpp"
#include "rsa_keys.hpp"
TODO(" ssl calls introduce some dependency on ssl system library  injecting it in the sec object would be better.")
#include "ssl_calls.hpp"

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
// certChainVersion (31 bits): A 31-bit field. The certificate version.

// +---------------------------------+-----------------------------------------+
// | 0x00000001 CERT_CHAIN_VERSION_1 | The certificate contained in the        |
// |                                 | certData field is a Server Proprietary  |
// |                                 | Certificate (section 2.2.1.4.3.1.1).    |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 CERT_CHAIN_VERSION_2 | The certificate contained in the        |
// |                                 | certData field is an X.509 Certificate  |
// |                                 | (see section 5.3.3.2).                  |
// +---------------------------------+-----------------------------------------+

// t (1 bit): A 1-bit field. Indicates whether the certificate contained in the
//  certData field has been permanently or temporarily issued to the server.

// 0 The certificate has been permanently issued to the server.
// 1 The certificate has been temporarily issued to the server.

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


TODO("Move crypto related utility methods to sec module (probably extract more sec generic code from parse_mcs_data_sc_security")

inline static void rdp_sec_generate_keys(CryptContext & encrypt, CryptContext & decrypt, uint8_t (& sign_key)[16], uint8_t *client_random, uint8_t *server_random, uint32_t rc4_key_size)
{
    uint8_t pre_master_secret[48];
    uint8_t master_secret[48];
    uint8_t key_block[48];

    /* Construct pre-master secret (session key) */
    memcpy(pre_master_secret, client_random, 24);
    memcpy(pre_master_secret + 24, server_random, 24);

    /* Generate master secret and then key material */
    sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 'A');
    sec_hash_48(key_block, master_secret, client_random, server_random, 'X');

    /* First 16 bytes of key material is MAC secret */
    memcpy(sign_key, key_block, 16);

    /* Generate export keys from next two blocks of 16 bytes */
    sec_hash_16(decrypt.key, &key_block[16], client_random, server_random);
    sec_hash_16(encrypt.key, &key_block[32], client_random, server_random);

    if (rc4_key_size == 1) {
        // LOG(LOG_DEBUG, "40-bit encryption enabled\n");
        sec_make_40bit(sign_key);
        sec_make_40bit(decrypt.key);
        sec_make_40bit(encrypt.key);
        decrypt.rc4_key_len = 8;
        encrypt.rc4_key_len = 8;
    }
    else {
        //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled\n", rc4_key_size);
        decrypt.rc4_key_len = 16;
        encrypt.rc4_key_len = 16;
    }

    /* Save initial RC4 keys as update keys */
    memcpy(decrypt.update_key, decrypt.key, 16);
    memcpy(encrypt.update_key, encrypt.key, 16);

    ssllib ssl;

    ssl.rc4_set_key(decrypt.rc4_info, decrypt.key, decrypt.rc4_key_len);
    ssl.rc4_set_key(encrypt.rc4_info, encrypt.key, encrypt.rc4_key_len);
}


static inline void recv_sec_tag_sig(Stream & stream, uint16_t len)
{
    stream.skip_uint8(len);
    /* Parse a public key structure */
    TODO("is padding always 8 bytes long ? may signature length change ? Check in documentation")
    TODO("we should check the signature is ok (using other provided parameters). This is not yet done today. Signature is just dropped")
}


static inline void recv_sec_tag_pubkey(Stream & stream, uint32_t & server_public_key_len, uint8_t* modulus, uint8_t* exponent)
{
    /* Parse a public key structure */
    uint32_t magic = stream.in_uint32_le();
    if (magic != SEC_RSA_MAGIC) {
        LOG(LOG_WARNING, "RSA magic 0x%x\n", magic);
        throw Error(ERR_SEC_PARSE_PUB_KEY_MAGIC_NOT_OK);
    }
    server_public_key_len = stream.in_uint32_le() - SEC_PADDING_SIZE;

    if ((server_public_key_len < SEC_MODULUS_SIZE)
    ||  (server_public_key_len > SEC_MAX_MODULUS_SIZE)) {
        LOG(LOG_WARNING, "Bad server public key size (%u bits)\n", server_public_key_len * 8);
        throw Error(ERR_SEC_PARSE_PUB_KEY_MODUL_NOT_OK);
    }
    stream.skip_uint8(8); /* modulus_bits, unknown */
    memcpy(exponent, stream.in_uint8p(SEC_EXPONENT_SIZE), SEC_EXPONENT_SIZE);
    memcpy(modulus, stream.in_uint8p(server_public_key_len), server_public_key_len);
    stream.skip_uint8(SEC_PADDING_SIZE);

    if (stream.p > stream.end){
        throw Error(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM);
    }
    LOG(LOG_DEBUG, "Got Public key, RDP4-style\n");
}


static inline void parse_mcs_data_sc_security(Stream & cr_stream,
                                              CryptContext & encrypt,
                                              CryptContext & decrypt,
                                              uint32_t & server_public_key_len,
                                              uint8_t (& client_crypt_random)[512],
                                              int & crypt_level)
{
    LOG(LOG_INFO, "SC_SECURITY\n");

    uint8_t server_random[SEC_RANDOM_SIZE];
    uint8_t client_random[SEC_RANDOM_SIZE];
    uint8_t modulus[SEC_MAX_MODULUS_SIZE];
    uint8_t exponent[SEC_EXPONENT_SIZE];
    uint32_t rc4_key_size;

    ssllib ssl;

    memset(modulus, 0, sizeof(modulus));
    memset(exponent, 0, sizeof(exponent));
    memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));
    memset(server_random, 0, SEC_RANDOM_SIZE);

    uint16_t length;
    uint8_t* end;

    rc4_key_size = cr_stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
    LOG(LOG_INFO, "rc4_key_size = %u", rc4_key_size);
    crypt_level = cr_stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
    LOG(LOG_INFO, "crypt_level = %u", crypt_level);

    if (crypt_level == 0) { /* no encryption */
        LOG(LOG_INFO, "No encryption");
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_ENCRYPTION_REQUIRED);
    }
    uint32_t random_len = cr_stream.in_uint32_le();
    uint32_t rsa_info_len = cr_stream.in_uint32_le();

    LOG(LOG_INFO, "random_len = %u", random_len);
    LOG(LOG_INFO, "rsa_info_len = %u", rsa_info_len);

// serverRandom (variable): The variable-length server random value used to
// derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
// given by the serverRandomLen field. If the encryptionMethod and
// encryptionLevel fields are both set to 0 then this field MUST NOT be present.

    if (random_len != SEC_RANDOM_SIZE) {
        LOG(LOG_ERR,
            "parse_crypt_info_error: random len %d, expected %d\n",
            random_len, SEC_RANDOM_SIZE);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RANDOM_LEN);
    }
    memcpy(server_random, cr_stream.in_uint8p(random_len), random_len);

// serverCertificate (variable): The variable-length certificate containing the
//  server's public key information. The length in bytes is given by the
// serverCertLen field. If the encryptionMethod and encryptionLevel fields are
// both set to 0 then this field MUST NOT be present.

    /* RSA info */
    end = cr_stream.p + rsa_info_len;
    if (end > cr_stream.end) {
        LOG(LOG_ERR,
            "rsa_info_len outside of buffer %u remains: %u", rsa_info_len, cr_stream.end - cr_stream.p);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RSA_LEN);
    }

    uint32_t flags = cr_stream.in_uint32_le(); /* 1 = RDP4-style, 0x80000002 = X.509 */
    LOG(LOG_INFO, "crypt flags %x\n", flags);
    if (flags & 1) {

        LOG(LOG_DEBUG, "We're going for the RDP4-style encryption\n");
        cr_stream.skip_uint8(8); /* unknown */

        while (cr_stream.p < end) {
            uint16_t tag = cr_stream.in_uint16_le();
            length = cr_stream.in_uint16_le();
            TODO(" this should not be necessary any more as received tag are fully decoded (but we should check length does not lead accessing data out of buffer)")
            uint8_t * next_tag = cr_stream.p + length;

            switch (tag) {
            case SEC_TAG_PUBKEY:
                LOG(LOG_DEBUG, "ReceivingPublic key, RDP4-style\n");
                recv_sec_tag_pubkey(cr_stream, server_public_key_len, modulus, exponent);
                LOG(LOG_DEBUG, "Got Public key, RDP4-style");
            break;
            case SEC_TAG_KEYSIG:
                LOG(LOG_DEBUG, "Receiving key sig RDP4-style\n");
                recv_sec_tag_sig(cr_stream, length);
                LOG(LOG_DEBUG, "Got key sig RDP4-style\n");
                break;
            default:
                LOG(LOG_DEBUG, "unimplemented: crypt tag 0x%x\n", tag);
                throw Error(ERR_SEC_PARSE_CRYPT_INFO_UNIMPLEMENTED_TAG);
                break;
            }
            cr_stream.p = next_tag;
        }
    }
    else {
        LOG(LOG_DEBUG, "We're going for the RDP5-style encryption\n");
        uint32_t certcount = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certcount = %u\n", certcount);

        if (certcount < 2){
            LOG(LOG_DEBUG, "Server didn't send enough X509 certificates\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK);
        }
        for (; certcount > 2; certcount--){
            /* ignore all the certificates between the root and the signing CA */
            LOG(LOG_WARNING, " Ignored certs left: %d\n", certcount);
            uint32_t ignorelen = cr_stream.in_uint32_le();
            LOG(LOG_WARNING, "Ignored Certificate length is %d\n", ignorelen);
            SSL_CERT *ignorecert = ssl_cert_read(cr_stream.p, ignorelen);
            cr_stream.skip_uint8(ignorelen);
            if (ignorecert == NULL){
                LOG(LOG_WARNING,
                    "got a bad cert: this will probably screw up"
                    " the rest of the communication\n");
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
        LOG(LOG_DEBUG, "CA Certificate length is %d\n", cacert_len);
        SSL_CERT *cacert = ssl_cert_read(cr_stream.p, cacert_len);
        cr_stream.skip_uint8(cacert_len);
        if (NULL == cacert){
            LOG(LOG_DEBUG, "Couldn't load CA Certificate from server\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL);
        }

        /* Loading Certificate from server*/
        uint32_t cert_len = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certificate length is %d\n", cert_len);
        SSL_CERT *server_cert = ssl_cert_read(cr_stream.p, cert_len);
        cr_stream.skip_uint8(cert_len);
        if (NULL == server_cert){
            ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Couldn't load Certificate from server\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_LOADED);
        }

        /* Matching certificates */
        if (!ssl_certs_ok(server_cert, cacert)){
            ssl_cert_free(server_cert);
            ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Security error CA Certificate invalid\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_MATCH);
        }
        ssl_cert_free(cacert);
        cr_stream.skip_uint8(16); /* Padding */
        SSL_RKEY *server_public_key = ssl_cert_to_rkey(server_cert, server_public_key_len);
        LOG(LOG_DEBUG, "Server public key length=%u\n", (unsigned)server_public_key_len);

        if (NULL == server_public_key){
            LOG(LOG_DEBUG, "Didn't parse X509 correctly\n");
            ssl_cert_free(server_cert);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);

        }
        ssl_cert_free(server_cert);

        LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d\n", server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);
        if ((server_public_key_len < SEC_MODULUS_SIZE) ||
            (server_public_key_len > SEC_MAX_MODULUS_SIZE)){
            LOG(LOG_DEBUG, "Bad server public key size (%u bits)\n",
                server_public_key_len * 8);
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
        }
        if (ssl_rkey_get_exp_mod(server_public_key, exponent, SEC_EXPONENT_SIZE,
            modulus, SEC_MAX_MODULUS_SIZE) != 0){
            LOG(LOG_DEBUG, "Problem extracting RSA exponent, modulus");
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK);
        }
        ssl.rkey_free(server_public_key);
        TODO(" find a way to correctly dispose of garbage at end of buffer")
        /* There's some garbage here we don't care about */
    }

    /* Generate a client random, and determine encryption keys */
    memset(client_random, 0x44, SEC_RANDOM_SIZE);
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fd = open("/dev/random", O_RDONLY);
    }
    if (fd != -1) {
        if (read(fd, client_random, SEC_RANDOM_SIZE) != SEC_RANDOM_SIZE) {
            LOG(LOG_WARNING, "random source failed to provide random data\n");
        }
        close(fd);
    }
    else {
        LOG(LOG_WARNING, "random source failed to provide random data : couldn't open device\n");
    }

    memset(client_random, 0, SEC_RANDOM_SIZE);

    ssl.rsa_encrypt(client_crypt_random, client_random, SEC_RANDOM_SIZE, server_public_key_len, modulus, exponent);
    rdp_sec_generate_keys(encrypt, decrypt, encrypt.sign_key, client_random, server_random, rc4_key_size);
}

static inline void send_sec_tag_sig(Stream & stream, const uint8_t (&pub_sig)[512])
{
    stream.out_uint16_le(SEC_TAG_KEYSIG);
    stream.out_uint16_le(72); /* len */
    stream.out_copy_bytes(pub_sig, 64); /* pub sig */
    stream.out_clear_bytes(8); /* pad */
}

static inline void send_sec_tag_pubkey(Stream & stream, const uint8_t (&pub_exp)[4], const uint8_t (&pub_mod)[512])
{
    stream.out_uint16_le(SEC_TAG_PUBKEY);
    stream.out_uint16_le(92); // length
    stream.out_uint32_le(SEC_RSA_MAGIC);
    stream.out_uint32_le(72); /* 72 bytes modulus len */
    stream.out_uint32_be(0x00020000);
    stream.out_uint32_be(0x3f000000);
    stream.out_copy_bytes(pub_exp, 4); /* pub exp */
    stream.out_copy_bytes(pub_mod, 64); /* pub mod */
    stream.out_clear_bytes(8); /* pad */
}

//02 0c ec 00 -> TS_UD_HEADER::type = SC_SECURITY, length = 236

//02 00 00 00 -> TS_UD_SC_SEC1::encryptionMethod = 128BIT_ENCRYPTION_FLAG
//02 00 00 00 -> TS_UD_SC_SEC1::encryptionLevel = TS_ENCRYPTION_LEVEL_CLIENT_COMPATIBLE
//20 00 00 00 -> TS_UD_SC_SEC1::serverRandomLen = 32 bytes
//b8 00 00 00 -> TS_UD_SC_SEC1::serverCertLen = 184 bytes

static inline void front_out_gcc_conference_user_data_sc_sec1(Stream & stream,
                                                int crypt_level,
                                                uint8_t (&server_random)[32],
                                                int rc4_key_size,
                                                uint8_t (&pub_mod)[512],
                                                uint8_t (&pri_exp)[512])
{

    TODO("Should we not keep rsa_keys somewhere. We just seem to forget them.")
    Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);
    memset(server_random, 0x44, 32);
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fd = open("/dev/random", O_RDONLY);
    }
    if (fd != -1) {
        if (read(fd, server_random, 32) != 32) {
        }
        close(fd);
    }

    uint8_t pub_sig[512];

    memcpy(pub_mod, rsa_keys.pub_mod, 64);
    memcpy(pub_sig, rsa_keys.pub_sig, 64);
    memcpy(pri_exp, rsa_keys.pri_exp, 64);

    stream.out_uint16_le(SC_SECURITY);
    stream.out_uint16_le(236); // length, including tag and length fields
    stream.out_uint32_le(rc4_key_size); // key len 1 = 40 bit 2 = 128 bit
    // crypt level 1 = low 2 = medium, 3 = high
    stream.out_uint32_le(crypt_level);

    stream.out_uint32_le(32);  // random len
    stream.out_uint32_le(184); // len of rsa info(certificate)
    stream.out_copy_bytes(server_random, 32);
    /* here to end is certificate */
    /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ */
    /* TermService\Parameters\Certificate */
    stream.out_uint32_le(1);
    stream.out_uint32_le(1);
    stream.out_uint32_le(1);

    // 96 bytes long of sec_tag pubkey
    send_sec_tag_pubkey(stream, rsa_keys.pub_exp, pub_mod);
    // 76 bytes long of sec_tag_pub_sig
    send_sec_tag_sig(stream, pub_sig);
    /* end certificate */
}

#endif
