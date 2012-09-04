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
    if (dwVersion & GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
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
        if (magic != GCC::UserData::SCSecurity::SEC_RSA_MAGIC) {
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


#endif
