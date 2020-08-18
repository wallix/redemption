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
   Author(s): David Fort

*/

#include "dsm.hpp"
#include "vnc.hpp"

#include "utils/log.hpp"
#include "core/error.hpp"

#define SCOPED_EVP_CONTEXT(TYPENAME, T, INIT, DESTROY) \
    struct TYPENAME { \
        TYPENAME() : ctx(INIT()) { } \
        ~TYPENAME() { DESTROY(ctx); } \
        T *ctx; \
    }

SCOPED_EVP_CONTEXT(ScopedEvpMd, EVP_MD_CTX, EVP_MD_CTX_new, EVP_MD_CTX_free);
SCOPED_EVP_CONTEXT(ScopedEvpCipher, EVP_CIPHER_CTX, EVP_CIPHER_CTX_new, EVP_CIPHER_CTX_free);

static void cipherDelete(EVP_CIPHER_CTX **pctx) {
    EVP_CIPHER_CTX *ctx = *pctx;
    if (ctx) {
        EVP_CIPHER_CTX_cleanup(ctx);
        EVP_CIPHER_CTX_free(ctx);
        *pctx = nullptr;
    }
}

uint32_t UltraDSM::checkBestSupportedFlags(uint32_t dwFlags) {
    if (dwFlags & svncKey448) {
        if (dwFlags & svncCipherBlowfish){
            return svncKey448 | svncCipherBlowfish;
        }
    }
    if (dwFlags & svncKey256) {
        if (dwFlags & svncCipher3AESOFB){
            return svncKey256 | svncCipher3AESOFB;
        }

        if (dwFlags & svncCipherAESCFB){
            return svncKey256 | svncCipherAESCFB;
        }

        if (dwFlags & svncCipherAES){
            return svncKey256 | svncCipherAES;
        }

        if (dwFlags & svncCipherBlowfish){
            return svncKey256 | svncCipherBlowfish;
        }

        if (dwFlags & svncCipherARC4){
            return svncKey256 | svncCipherARC4;
        }
    }
    if (dwFlags & svncKey192) {
        if (dwFlags & svncCipher3AESOFB){
            return svncKey192 | svncCipher3AESOFB;
        }

        if (dwFlags & svncCipherAESCFB){
            return svncKey192 | svncCipherAESCFB;
        }

        if (dwFlags & svncCipherAES){
            return svncKey192 | svncCipherAES;
        }

        if (dwFlags & svncCipherBlowfish){
            return svncKey192 | svncCipherBlowfish;
        }

        if (dwFlags & svncCipherARC4){
            return svncKey192 | svncCipherARC4;
        }
    }
    if (dwFlags & svncKey128) {
        if (dwFlags & svncCipher3AESOFB){
            return svncKey128 | svncCipher3AESOFB;
        }

        if (dwFlags & svncCipherAESCFB){
            return svncKey128 | svncCipherAESCFB;
        }

        if (dwFlags & svncCipherAES){
            return svncKey128 | svncCipherAES;
        }

        if (dwFlags & svncCipherBlowfish){
            return svncKey128 | svncCipherBlowfish;
        }

#ifndef OPENSSL_NO_IDEA
        if (dwFlags & svncCipherIDEA){
            return svncKey128 | svncCipherIDEA;
        }
#endif

        if (dwFlags & svncCipherCAST5){
            return svncKey128 | svncCipherCAST5;
        }

        if (dwFlags & svncCipherARC4){
            return svncKey128 | svncCipherARC4;
        }
    }
    if (dwFlags & svncKey56) {
        if (dwFlags & svncCipherBlowfish){
            return svncKey56 | svncCipherBlowfish;
        }

        if (dwFlags & svncCipherCAST5){
            return svncKey56 | svncCipherCAST5;
        }

        if (dwFlags & svncCipherARC4){
            return svncKey56 | svncCipherARC4;
        }
    }

    return 0;
}

const EVP_CIPHER* UltraDSM::getCipher(uint32_t dwFlags, int &nKeyLength) {
    nKeyLength = 0;

    if (dwFlags & svncCipher3AESOFB) {
        if (dwFlags & svncKey256) {
            nKeyLength = 256 / 8;
            return EVP_aes_256_cfb8();
        } else if (dwFlags & svncKey192) {
            nKeyLength = 192 / 8;
            return EVP_aes_192_cfb8();
        } else if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_aes_128_cfb8();
        } else {
            return nullptr;
        }
    } else if (dwFlags & svncCipherAESCFB) {
        if (dwFlags & svncKey256) {
            nKeyLength = 256 / 8;
            return EVP_aes_256_cfb8();
        } else if (dwFlags & svncKey192) {
            nKeyLength = 192 / 8;
            return EVP_aes_192_cfb8();
        } else if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_aes_128_cfb8();
        } else {
            LOG(LOG_ERR, "Invalid keysize for cipher (flags 0x%08x).", dwFlags);
            return nullptr;
        }
    } else if (dwFlags & svncCipherAES) {
        if (dwFlags & svncKey256) {
            nKeyLength = 256 / 8;
            return EVP_aes_256_ofb();
        } else if (dwFlags & svncKey192) {
            nKeyLength = 192 / 8;
            return EVP_aes_192_ofb();
        } else if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_aes_128_ofb();
        } else {
            LOG(LOG_ERR, "Invalid keysize for cipher (flags 0x%08x).", dwFlags);
            return nullptr;
        }
    } else if (dwFlags & svncCipherBlowfish) {
        if (dwFlags & svncKey448) {
            nKeyLength = 448 / 8;
            return EVP_bf_ofb();
        } else if (dwFlags & svncKey256) {
            nKeyLength = 256 / 8;
            return EVP_bf_ofb();
        } else if (dwFlags & svncKey192) {
            nKeyLength = 192 / 8;
            return EVP_bf_ofb();
        } else if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_bf_ofb();
        } else {
            LOG(LOG_ERR, "Invalid keysize for cipher (flags 0x%08x).", dwFlags);
            return nullptr;
        }
#ifndef OPENSSL_NO_IDEA
    } else if (dwFlags & svncCipherIDEA) {
        if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_idea_ofb();
        } else {
            LOG(LOG_ERR, "Invalid keysize for cipher (flags 0x%08x).", dwFlags);
            return NULL;
        }
#endif
    } else if (dwFlags & svncCipherCAST5) {
        if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_cast5_ofb();
        } else if (dwFlags & svncKey56) {
            nKeyLength = 56 / 8;
            return EVP_cast5_ofb();
        } else {
            LOG(LOG_ERR, "Invalid keysize for cipher (flags 0x%08x).", dwFlags);
            return nullptr;
        }
    } else if (dwFlags & svncCipherARC4) {
        if (dwFlags & svncKey256) {
            nKeyLength = 256 / 8;
            return EVP_rc4();
        } else if (dwFlags & svncKey192) {
            nKeyLength = 192 / 8;
            return EVP_rc4();
        } else if (dwFlags & svncKey128) {
            nKeyLength = 128 / 8;
            return EVP_rc4();
        } else if (dwFlags & svncKey56) {
            nKeyLength = 56 / 8;
            return EVP_rc4();
        } else {
            LOG(LOG_ERR, "Invalid keysize for cipher (flags 0x%08x).", dwFlags);
            return nullptr;
        }
    } else {
        LOG(LOG_ERR, "Invalid cipher (flags 0x%08x).", dwFlags);
        return nullptr;
    }
}

static EVP_CIPHER_CTX *initCipher(const EVP_CIPHER* cipher, bool encrypt, int keyLen) {
    EVP_CIPHER_CTX *ret = EVP_CIPHER_CTX_new();

    EVP_CipherInit(ret, cipher, nullptr, nullptr, encrypt);
    EVP_CIPHER_CTX_set_key_length(ret, keyLen);
    return ret;
}


UltraDSM::UltraDSM(char *password)
    : m_state(DSM_WAITING_CHALLENGE)
    , m_password(password)
    , m_rsa(nullptr)
    , m_nRSASize(0)
    , m_challengeFlags(0)
    , m_responseFlags(0)
    , m_contextVS1(nullptr)
    , m_contextSV1(nullptr)
    , m_contextVS2(nullptr)
    , m_contextSV2(nullptr)
    , m_contextVS3(nullptr)
    , m_contextSV3(nullptr)
    , m_bTriple(false)
{
}

UltraDSM::~UltraDSM() {
    reset();
}

void UltraDSM::reset() {
    if (m_rsa) {
        RSA_free(m_rsa);
        m_rsa = nullptr;
    }

    cipherDelete(&m_contextVS1);
    cipherDelete(&m_contextSV1);

    cipherDelete(&m_contextVS2);
    cipherDelete(&m_contextSV2);

    cipherDelete(&m_contextVS3);
    cipherDelete(&m_contextSV3);
}

bool UltraDSM::handleChallenge(InStream &instream, uint16_t &challengeLen, uint8_t &passphraseused) {
    if (!instream.in_check_rem(2)){
        return false;
    }

    challengeLen = instream.in_uint16_le();
    if (!instream.in_check_rem(challengeLen + 1)){
        return false;
    }

    InStream s({instream.get_current(), challengeLen});
    instream.in_skip_bytes(challengeLen);

    passphraseused = instream.in_uint8();

    if (!s.in_check_rem(1 + 1 + 4)) {
        LOG(LOG_ERR, "Invalid challengeLen");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    uint8_t pluginId = s.in_uint8();
    if (pluginId != 1) {
        LOG(LOG_ERR, "Invalid pluginId 0x%.2x", pluginId);
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    uint8_t pluginVersion = s.in_uint8();
    if (pluginVersion > 3) {
        LOG(LOG_ERR, "Invalid pluginVersion %d", pluginVersion);
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    uint32_t serverIdentificationLen = s.in_uint32_le();
    if (!s.in_check_rem(serverIdentificationLen)) {
        LOG(LOG_ERR, "Invalid challengeLen for serverIdentification");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }
    /*bytes_view serverIdentification =*/ s.in_skip_bytes(serverIdentificationLen);

    if (!s.in_check_rem(4 + 2)) {
        LOG(LOG_ERR, "Invalid challengeLen for challenge and wClientAuthPublicKeyIdentifierLength");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    m_challengeFlags = s.in_uint32_le();
    // bool passPhraseRequired = (m_challengeFlags & svncOverridePassphrase);

    uint16_t clientAuthPublicKeyIdentifierLen = s.in_uint16_le();
    if (!s.in_check_rem(clientAuthPublicKeyIdentifierLen)) {
        LOG(LOG_ERR, "Invalid challengeLen for clientAuthPublicKeyIdentifierLen");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    /*const uint8_t *clientAuthPublicKeyIdentifier = s.get_current();*/

    if (clientAuthPublicKeyIdentifierLen > 0) {
        // handle clientAuthPublicKeyIdentifier
        //TODO: s.in_copy_bytes(target, clientAuthPublicKeyIdentifierLen);
        s.in_skip_bytes(clientAuthPublicKeyIdentifierLen);
    }

    // SALT
    if (!s.in_check_rem(8)) {
        LOG(LOG_ERR, "Invalid challengeLen for salt");
        //throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    const uint8_t *salt = s.get_current();
    s.in_skip_bytes(8);

    ScopedEvpMd digestContext;
    EVP_DigestInit(digestContext.ctx, EVP_sha1());
    EVP_DigestUpdate(digestContext.ctx, salt, 8);

    ScopedEvpCipher cipherContext;
    int keySize = 0;
    unsigned int messageDigestLength = 0;

    const EVP_CIPHER* cipher = nullptr;
    if (m_challengeFlags & svncLowKey) {
        cipher = EVP_bf_ofb();
        keySize = 56 / 8;
        EVP_CipherInit(cipherContext.ctx, cipher, nullptr, nullptr, 0);
        EVP_CIPHER_CTX_set_key_length(cipherContext.ctx, keySize);
    } else {
        cipher = EVP_aes_256_ofb();
        EVP_CipherInit(cipherContext.ctx, cipher, nullptr, nullptr, 0);
        keySize = EVP_CIPHER_key_length(cipher);
    }

    int nIVLength = EVP_CIPHER_iv_length(cipher);

    if (!s.in_check_rem(nIVLength + 2)) {
        LOG(LOG_ERR, "Invalid challengeLen for IV and publicKeyLength");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }
    const uint8_t *blobInitialKeyIV = s.get_current();
    s.in_skip_bytes(nIVLength);

     static uint8_t g_DefaultPassword[] = {
        0x69, 0xF4, 0xA4, 0x7C, 0xF8, 0xF1, 0xA6, 0x11, 0xC1, 0x05, 0x81, 0xC4,
        0x95, 0x49, 0xAF, 0x4E, 0xB9, 0x55, 0x22, 0x69, 0x2F, 0x68, 0x32, 0xF4,
        0xD5, 0x64, 0x5D, 0xF5, 0xE2, 0x37, 0x02, 0x70
    };

    uint8_t *blobInitialKey = new uint8_t[keySize]();
    size_t passwdLen;
    char *passwd;

#if 0
    if (this->m_password && strlen(this->m_password) && false) {
        passwd = this->m_password;
        passwdLen = strlen(this->m_password);
    } else {
#endif
    passwd = char_ptr_cast(g_DefaultPassword);
    passwdLen = sizeof(g_DefaultPassword);


    if (m_challengeFlags & svncNewKey) {
        PKCS5_PBKDF2_HMAC_SHA1(passwd, passwdLen, salt, 8, 0x1001, keySize, blobInitialKey);
    } else {
        EVP_BytesToKey(cipher, EVP_sha1(), salt, reinterpret_cast<uint8_t*>(passwd), passwdLen, 11, blobInitialKey, nullptr);
    }

    uint8_t blobMessageDigest[EVP_MAX_MD_SIZE];

    EVP_CipherInit_ex(cipherContext.ctx, nullptr, nullptr, blobInitialKey, blobInitialKeyIV, 0);

    uint16_t publicKeyLength = s.in_uint16_le();
    if (!s.in_check_rem(publicKeyLength)) {
        LOG(LOG_ERR, "Invalid challengeLen publicKeyLen=%d", publicKeyLength);
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    const uint8_t *publicKeyRaw = s.get_current();
    s.in_skip_bytes(publicKeyLength);

    uint8_t *m_pPublicKeyRaw = new uint8_t[publicKeyLength];
    int m_nPublicKeyRawLength;
    EVP_CipherUpdate(cipherContext.ctx, m_pPublicKeyRaw, &m_nPublicKeyRawLength, publicKeyRaw, publicKeyLength);

    EVP_DigestUpdate(digestContext.ctx, m_pPublicKeyRaw, m_nPublicKeyRawLength);
    EVP_DigestFinal(digestContext.ctx, blobMessageDigest, &messageDigestLength);

    const uint8_t *packetHash = s.get_current();
    if (memcmp(blobMessageDigest, packetHash, EVP_MD_size(EVP_sha1())) != 0) {
        LOG(LOG_ERR, "Invalid handshake hash");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }

    m_rsa = d2i_RSAPublicKey(nullptr, const_cast<const uint8_t **>(&m_pPublicKeyRaw), m_nPublicKeyRawLength);
    if (m_rsa == nullptr) {
        LOG(LOG_ERR, "Failed to load public key.");
        throw Error(ERR_VNC_CONNECTION_ERROR);
    }
    m_nRSASize = RSA_size(m_rsa);

    LOG(LOG_DEBUG, "Handshake packet, passphraseUsed=%d", passphraseused);
    return true;
}


bool UltraDSM::getResponse(OutStream &out) {
        //bool bExpectChallenge = false;

        if (!m_rsa) {
            LOG(LOG_ERR, "Public key unavailable.");
            return false;
        }

        //m_bIsViewer = true;

        m_responseFlags = checkBestSupportedFlags(m_challengeFlags);
        if (m_responseFlags == 0) {
            LOG(LOG_ERR, "Invalid response flags.");
            return false;
        }

        int nKeyLength = 0;
        const EVP_CIPHER* pCipher = getCipher(m_responseFlags, nKeyLength);
        if (!pCipher) {
            LOG(LOG_ERR, "No available cipher.");
            return false;
        }

        m_contextSV1 = initCipher(pCipher, false, nKeyLength);
        m_contextVS1 = initCipher(pCipher, true, nKeyLength);

        if (!m_contextSV1 || !m_contextVS1) {
            LOG(LOG_ERR, "Failed to create key 1.");
            // throw Error(ERR_VNC_CONNECTION_ERROR);
        }

        if (m_responseFlags & svncCipher3AESOFB) {
            // encrypt/decrypt swapped for key 2
            m_contextSV2 = initCipher(pCipher, true, nKeyLength);
            m_contextVS2 = initCipher(pCipher, false, nKeyLength);

            m_contextSV3 = initCipher(pCipher, false, nKeyLength);
            m_contextVS3 = initCipher(pCipher, true, nKeyLength);
        }

        int nKeyDataLength = nKeyLength * 2;

        if (m_challengeFlags & svncNewKey) {
            nKeyDataLength = RSA_size(m_rsa) - 12;
        }

        BufMaker<0x1000> blobKeysBuffer;
        writable_u8_array_view blobKeys = blobKeysBuffer.dyn_array(nKeyDataLength);

        m_bTriple = false;

        if (m_challengeFlags & svncNewKey) {
            RAND_bytes(blobKeys.data(), nKeyDataLength);

            if (m_responseFlags & svncCipher3AESOFB) {
                m_bTriple = true;

                BufMaker<0x1000> blobKeySV1; // nKeyLength
                BufMaker<0x1000> blobKeyVS1;
                BufMaker<0x1000> blobKeySV2;
                BufMaker<0x1000> blobKeyVS2;
                BufMaker<0x1000> blobKeySV3;
                BufMaker<0x1000> blobKeyVS3;

                int nIVLength = EVP_CIPHER_CTX_iv_length(m_contextSV1);

                LOG(LOG_DEBUG, "Using 3AES NewKey; IV %d, data %zu\r\n", nIVLength, blobKeys.size());

                int nSourceLength = (blobKeys.size() - nIVLength) / 6;

                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (nSourceLength * 0), nSourceLength,
                        nullptr, 0, 0x1001, nKeyLength, blobKeySV1.dyn_array(nKeyLength).data());
                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (nSourceLength * 1), nSourceLength,
                        nullptr, 0, 0x1001, nKeyLength, blobKeyVS1.dyn_array(nKeyLength).data());

                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (nSourceLength * 2), nSourceLength,
                        nullptr, 0, 0x1001, nKeyLength, blobKeySV2.dyn_array(nKeyLength).data());
                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (nSourceLength * 3), nSourceLength,
                        nullptr, 0, 0x1001, nKeyLength, blobKeyVS2.dyn_array(nKeyLength).data());

                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (nSourceLength * 4), nSourceLength,
                        nullptr, 0, 0x1001, nKeyLength, blobKeySV3.dyn_array(nKeyLength).data());
                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (nSourceLength * 5), nSourceLength,
                        nullptr, 0, 0x1001, nKeyLength, blobKeyVS3.dyn_array(nKeyLength).data());

                const uint8_t* pIV = blobKeys.data() + (blobKeys.size() - nIVLength);

                EVP_CipherInit_ex(m_contextSV1, nullptr, nullptr, blobKeySV1.dyn_array(nKeyLength).data(), pIV, 0);
                EVP_CipherInit_ex(m_contextVS1, nullptr, nullptr, blobKeyVS1.dyn_array(nKeyLength).data(), pIV, 1);

                EVP_CipherInit_ex(m_contextSV2, nullptr, nullptr, blobKeySV2.dyn_array(nKeyLength).data(), pIV, 1); // swapped for 2
                EVP_CipherInit_ex(m_contextVS2, nullptr, nullptr, blobKeyVS2.dyn_array(nKeyLength).data(), pIV, 0);

                EVP_CipherInit_ex(m_contextSV3, nullptr, nullptr, blobKeySV3.dyn_array(nKeyLength).data(), pIV, 0);
                EVP_CipherInit_ex(m_contextVS3, nullptr, nullptr, blobKeyVS3.dyn_array(nKeyLength).data(), pIV, 1);
            } else {
                BufMaker<0x10000> blobKeySVBuf;
                writable_bytes_view blobKeySV = blobKeySVBuf.dyn_array(nKeyLength);
                BufMaker<0x10000> blobKeyVSBuf;
                writable_bytes_view blobKeyVS = blobKeyVSBuf.dyn_array(nKeyLength);

                int nIVLength = EVP_CIPHER_CTX_iv_length(m_contextSV1);

                LOG(LOG_DEBUG, "Using NewKey; IV %d, data %zu\r\n", nIVLength, blobKeys.size());

                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()), ((blobKeys.size() - nIVLength) / 2),
                        nullptr, 0, 0x1001, nKeyLength, blobKeySV.as_u8p());
                PKCS5_PBKDF2_HMAC_SHA1(char_ptr_cast(blobKeys.data()) + (blobKeys.size() - nIVLength) / 2, (blobKeys.size() - nIVLength) / 2,
                        nullptr, 0, 0x1001, nKeyLength, blobKeyVS.as_u8p());

                const uint8_t* pIV = blobKeys.data() + (blobKeys.size() - nIVLength);

                EVP_CipherInit_ex(m_contextSV1, nullptr, nullptr, blobKeySV.data(), pIV, 0);
                EVP_CipherInit_ex(m_contextVS1, nullptr, nullptr, blobKeyVS.data(), pIV, 1);
            }
        } else {
            uint8_t *pKeySV = blobKeys.data();
            uint8_t *pKeyVS = pKeySV + nKeyLength;

            EVP_CIPHER_CTX_rand_key(m_contextSV1, pKeySV);
            EVP_CIPHER_CTX_rand_key(m_contextVS1, pKeyVS);

            EVP_CipherInit_ex(m_contextSV1, nullptr, nullptr, pKeySV, nullptr, 0);
            EVP_CipherInit_ex(m_contextVS1, nullptr, nullptr, pKeyVS, nullptr, 1);
        }

        BufMaker<0x1000> blobEncryptedKeysBufferBuf;
        writable_buffer_view blobEncryptedKeysBuffer = blobEncryptedKeysBufferBuf.dyn_array(RSA_size(m_rsa));

        int nEncryptedSize = RSA_public_encrypt(blobKeys.size(), blobKeys.data(), blobEncryptedKeysBuffer.as_u8p(), m_rsa, RSA_PKCS1_PADDING);
        if (nEncryptedSize == -1) {
            LOG(LOG_ERR, "Failed to encrypt symmetric keys.");
            return false;
        }


        BufMaker<0x1000> blobClientAuthSigBuf;
        writable_buffer_view blobClientAuthSig;
        unsigned int nClientAuthSigLength = 0;
        if (m_challengeFlags & svncClientAuthRequired) {
#if 0
            // Sign with the Client Authentication private key
            RSA* rsaClientAuth = LoadClientAuthPrivateKey(m_szClientAuthPublicKeyIdentifier);

            if (rsaClientAuth) {
                m_nClientAuthPublicKeyRSASize = RSA_size(rsaClientAuth);
                unsigned int messageDigestLength = 0;
                AutoBlob<> blobMessageDigest(EVP_MAX_MD_SIZE);

                EVP_MD_CTX digestContext;
                EVP_DigestInit(&digestContext, EVP_sha1());
                EVP_DigestUpdate(&digestContext, m_pPublicKeyRaw, m_nPublicKeyRawLength);
                EVP_DigestUpdate(&digestContext, blobKeys, nKeyDataLength);
                EVP_DigestFinal(&digestContext, blobMessageDigest, &messageDigestLength);

                blobClientAuthSig.Alloc(RSA_size(rsaClientAuth));
                RSA_sign(NID_sha1, blobMessageDigest, messageDigestLength, blobClientAuthSig, &nClientAuthSigLength, rsaClientAuth);

                RSA_free(rsaClientAuth);
            } else {
                m_nClientAuthPublicKeyRSASize = 0;
                SetLastErrorString("Client authentication private key unavailable.");
            }
#endif
        }

        uint16_t wEncryptedSize = nEncryptedSize;
        uint16_t wClientAuthSigLength = nClientAuthSigLength;

        int nResponseLength = sizeof(m_responseFlags) + sizeof(wEncryptedSize) + nEncryptedSize + sizeof(wClientAuthSigLength) + nClientAuthSigLength;

        out.out_uint32_le(m_responseFlags);
        out.out_uint16_le(wEncryptedSize);
        out.out_copy_bytes(blobEncryptedKeysBuffer.data(), nEncryptedSize);
        out.out_uint16_le(wClientAuthSigLength);
        out.out_copy_bytes(blobClientAuthSig.data(), nClientAuthSigLength);

        if (m_responseFlags & svncCipherARC4) {
            BufMaker<0x1000> blobFlotsamBuf;
            writable_bytes_view blobFlotsam = blobFlotsamBuf.dyn_array(RC4_DROP_BYTES);
            BufMaker<0x1000> blobJetsamBuf;
            writable_bytes_view blobJetsam = blobJetsamBuf.dyn_array(RC4_DROP_BYTES);

            int nDummyByteCount = 0;
            EVP_CipherUpdate(m_contextSV1, blobFlotsam.as_u8p(), &nDummyByteCount, blobJetsam.data(), RC4_DROP_BYTES);
            EVP_CipherUpdate(m_contextVS1, blobFlotsam.as_u8p(), &nDummyByteCount, blobJetsam.data(), RC4_DROP_BYTES);
        }


        LOG(LOG_ERR, "GetResponse OK.");
        return true;
}


bool UltraDSM::encrypt(byte_ptr buffer, size_t len, writable_bytes_view & out) {
    int nEncryptedLength = 0;

    if (m_bTriple) {
#if 0
        EVP_CipherUpdate(&m_ContextVS1, pOutputBuffer, &nEncryptedLength, pDataBuffer, nDataLen);
        EVP_CipherUpdate(&m_ContextVS2, pTempBuffer, &nEncryptedLength2, pOutputBuffer, nEncryptedLength);
        EVP_CipherUpdate(&m_ContextVS3, pOutputBuffer, &nEncryptedLength3, pTempBuffer, nEncryptedLength2);
#endif
    } else {
        EVP_CipherUpdate(m_contextVS1, out.as_u8p(), &nEncryptedLength, buffer.as_u8p(), len);
    }

    return true;
}

bool UltraDSM::decrypt(const uint8_t *buffer, size_t len, writable_bytes_view out) {
    int nEncryptedLength;
    EVP_CipherUpdate(m_contextSV1, out.as_u8p(), &nEncryptedLength, buffer, len);
    return true;
}
