/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__CORE_CIPHER_HPP__)
#define __CORE_CIPHER_HPP__

#include <cstddef>
#include <openssl/evp.h>
#include <string.h>

struct CipherInfo {
private:
    unsigned char _key[EVP_MAX_KEY_LENGTH];
    unsigned char _iv[EVP_MAX_IV_LENGTH];
    const EVP_CIPHER * _context;

public:
    CipherInfo(EVP_CIPHER * context = 0)
    : _key()
    , _iv()
    , _context(context)
    {}

    CipherInfo(unsigned char * key, std::size_t len_key,
               unsigned char * iv, std::size_t len_iv,
               EVP_CIPHER * context = 0)
    : _key()
    , _iv()
    , _context(context)
    {
        this->set_key(key, len_key);
        this->set_iv(iv, len_iv);
    }

    void set_key(unsigned char * key, std::size_t len_key)
    {
        memcpy(this->_key, key, len_key);
    }

    void set_iv(unsigned char * iv, std::size_t len_iv)
    {
        memcpy(this->_iv, iv, len_iv);
    }

    void set_key_zero_after(std::size_t pos)
    {
        bzero(this->_key + pos, this->key_max_size() - pos);
    }

    void set_iv_zero_after(std::size_t pos)
    {
        bzero(this->_iv + pos, this->iv_max_size() - pos);
    }

    void set_full_key(unsigned char * key, std::size_t len_key)
    {
        this->set_key(key, len_key);
        this->set_key_zero_after(len_key);
    }

    void set_full_iv(unsigned char * iv, std::size_t len_iv)
    {
        this->set_iv(iv, len_iv);
        this->set_iv_zero_after(len_iv);
    }

    void set_context(const EVP_CIPHER * context)
    {
        this->_context = context;
    }

    unsigned char * key()
    {
        return this->_key;
    }

    unsigned char * iv()
    {
        return this->_iv;
    }

    const unsigned char * key() const
    {
        return this->_key;
    }

    const unsigned char * iv() const
    {
        return this->_iv;
    }

    const EVP_CIPHER * context() const
    {
        return this->_context;
    }

    std::size_t key_max_size() const
    {
        return sizeof(this->_key);
    }

    std::size_t iv_max_size() const
    {
        return sizeof(this->_iv);
    }

    std::size_t key_len() const
    {
        return EVP_CIPHER_key_length(this->_context);
    }

    std::size_t iv_len() const
    {
        return EVP_CIPHER_iv_length(this->_context);
    }
};

struct CipherCryptData {
    unsigned char * data;
    unsigned char * pbuf;

    CipherCryptData(unsigned char * data)
    : data(data)
    , pbuf(data)
    {}

    std::size_t size() const
    { return this->pbuf - this->data; }

    bool empty() const
    { return this->pbuf == this->data; }

    void reset()
    {
        this->pbuf = this->data;
    }

    void reset(unsigned char * data)
    {
        this->data = data;
        this->pbuf = data;
    }
};

typedef int (*CipherCryptInit)(
    EVP_CIPHER_CTX* ctx,
    const EVP_CIPHER* cipher,
    ENGINE* impl,
    const unsigned char* key,
    const unsigned char* iv
);
typedef int (*CipherCryptUpdate)(
    EVP_CIPHER_CTX* ctx,
    unsigned char* out,
    int* outl,
    const unsigned char* in,
    int inl
);
typedef int (*CipherCryptFinal)(
    EVP_CIPHER_CTX* ctx,
    unsigned char* out,
    int* outl
);

struct CipherMode
{
    enum enum_t
    {
        NO_MODE,

        BLOWFISH_CBC,
        BLOWFISH_ECB,
        BLOWFISH_OFB,

        CAST5_CBC,
        CAST5_CFB,
        CAST5_ECB,
        CAST5_OFB,

        DES_CBC,
        DES_CFB,
        DES_OFB,
        DES_ECB,

        DES_EDE_CBC,
        DES_EDE_ECB,
        DES_EDE_OFB,
        DES_EDE_CFB_64,

        DES_EDE3_CBC,
        DES_EDE3_ECB,
        DES_EDE3_CFB_1,
        DES_EDE3_CFB_8,
        DES_EDE3_CFB_64,
        DES_EDE3_OFB,

        RC2_CBC,
        RC2_CFB,
        RC2_ECB,
        RC2_OFB,
        RC2_64_CBC,
        RC2_40_CBC,

        RC4,
        RC4_40,

        AES_128_CBC,
        AES_128_CFB,
        AES_128_CFB1,
        AES_128_CFB8,
        AES_128_ECB,
        AES_128_OFB,

        AES_192_CBC,
        AES_192_CFB,
        AES_192_CFB1,
        AES_192_CFB8,
        AES_192_ECB,
        AES_192_OFB,

        AES_256_CBC,
        AES_256_CFB,
        AES_256_CFB1,
        AES_256_CFB8,
        AES_256_ECB,
        AES_256_OFB,
    };

    static const EVP_CIPHER * to_evp_cipher(enum_t e)
    {
        switch (e)
        {
            case BLOWFISH_CBC:  return EVP_bf_cbc();
            case BLOWFISH_ECB:  return EVP_bf_ecb();
            case BLOWFISH_OFB:  return EVP_bf_ofb();

            case CAST5_CBC: return EVP_cast5_cbc();
            case CAST5_CFB: return EVP_cast5_cfb64();
            case CAST5_ECB: return EVP_cast5_ecb();
            case CAST5_OFB: return EVP_cast5_ofb();

            case DES_CBC:   return EVP_des_cbc();
            case DES_CFB:   return EVP_des_cfb1();
            case DES_OFB:   return EVP_des_ofb();
            case DES_ECB:   return EVP_des_ecb();

            case DES_EDE_CBC:   return EVP_des_ede_cbc();
            case DES_EDE_ECB:   return EVP_des_ede_ecb();
            case DES_EDE_OFB:   return EVP_des_ede_ofb();
            case DES_EDE_CFB_64:return EVP_des_ede_cfb64();

            case DES_EDE3_CBC:   return EVP_des_ede3_cbc();
            case DES_EDE3_ECB:   return EVP_des_ede3_ecb();
            case DES_EDE3_CFB_1: return EVP_des_ede3_cfb1();
            case DES_EDE3_CFB_8: return EVP_des_ede3_cfb8();
            case DES_EDE3_CFB_64:return EVP_des_ede3_cfb64();
            case DES_EDE3_OFB:   return EVP_des_ede3_ofb();

            case RC2_CBC:   return EVP_rc2_cbc();
            case RC2_CFB:   return EVP_rc2_cfb();
            case RC2_ECB:   return EVP_rc2_ecb();
            case RC2_OFB:   return EVP_rc2_ofb();
            case RC2_64_CBC:return EVP_rc2_64_cbc();
            case RC2_40_CBC:return EVP_rc2_40_cbc();

            case RC4:   return EVP_rc4();
            case RC4_40:return EVP_rc4_40();

            case AES_128_CBC:   return EVP_aes_128_cbc();
            case AES_128_CFB:   return EVP_aes_128_cfb();
            case AES_128_CFB1:  return EVP_aes_128_cfb1();
            case AES_128_CFB8:  return EVP_aes_128_cfb8();
            case AES_128_ECB:   return EVP_aes_128_ecb();
            case AES_128_OFB:   return EVP_aes_128_ofb();

            case AES_192_CBC:   return EVP_aes_192_cbc();
            case AES_192_CFB:   return EVP_aes_192_cfb();
            case AES_192_CFB1:  return EVP_aes_192_cfb1();
            case AES_192_CFB8:  return EVP_aes_192_cfb8();
            case AES_192_ECB:   return EVP_aes_192_ecb();
            case AES_192_OFB:   return EVP_aes_192_ofb();

            case AES_256_CBC:   return EVP_aes_256_cbc();
            case AES_256_CFB:   return EVP_aes_256_cfb();
            case AES_256_CFB1:  return EVP_aes_256_cfb1();
            case AES_256_CFB8:  return EVP_aes_256_cfb8();
            case AES_256_ECB:   return EVP_aes_256_ecb();
            case AES_256_OFB:   return EVP_aes_256_ofb();


            default:
                return 0;
        }
    }
};

class CipherCrypt
{
    EVP_CIPHER_CTX _ctx;
    CipherCryptData* _data;

    CipherCryptInit _init;
    CipherCryptUpdate _update;
    CipherCryptFinal _final;

public:
    struct EncryptConstruct {};
    struct DecryptConstruct {};

    CipherCrypt(CipherCryptData* data = 0)
    : _ctx()
    , _data(data)
    {
        EVP_CIPHER_CTX_init(&this->_ctx);
    }

    CipherCrypt(const EncryptConstruct&, CipherCryptData* data = 0)
    : _ctx()
    , _data(data)
    , _init(&EVP_EncryptInit_ex)
    , _update(&EVP_EncryptUpdate)
    , _final(&EVP_EncryptFinal_ex)
    {
        EVP_CIPHER_CTX_init(&this->_ctx);
    }

    CipherCrypt(const DecryptConstruct&, CipherCryptData* data = 0)
    : _ctx()
    , _data(data)
    , _init(&EVP_DecryptInit_ex)
    , _update(&EVP_DecryptUpdate)
    , _final(&EVP_DecryptFinal_ex)
    {
        EVP_CIPHER_CTX_init(&this->_ctx);
    }

    ~CipherCrypt()
    {
        this->clean();
    }

    const EVP_CIPHER_CTX& ctx() const
    {
        return this->_ctx;
    }

    void clean()
    {
        EVP_CIPHER_CTX_cleanup(&this->_ctx);
    }

    void init(const EncryptConstruct&)
    {
        _init = &EVP_EncryptInit_ex;
        _update = &EVP_EncryptUpdate;
        _final = &EVP_EncryptFinal_ex;
    }

    void init(const DecryptConstruct&)
    {
        _init = &EVP_DecryptInit_ex;
        _update = &EVP_DecryptUpdate;
        _final = &EVP_DecryptFinal_ex;
    }

    void init_encrypt()
    {
        this->init(EncryptConstruct());
    }

    void init_decrypt()
    {
        this->init(DecryptConstruct());
    }

    CipherCryptData* data() const
    {
        return this->_data;
    }

    bool start(const EVP_CIPHER * mode,
               const unsigned char* key = 0, const unsigned char* iv = 0,
               ENGINE* impl = 0)
    {
        return this->_init(&this->_ctx, mode, impl, key, iv);
    }

    bool start(CipherCryptData* data, const EVP_CIPHER * mode,
               const unsigned char* key = 0, const unsigned char* iv = 0, ENGINE* impl = 0)
    {
        this->_data = data;
        return this->start(mode, key, iv, impl);
    }

    bool update(const unsigned char* in, std::size_t size)
    {
        int tmplen;
        if (!this->_update(&this->_ctx, this->_data->pbuf, &tmplen, in, size))
        {
            return false;
        }
        this->_data->pbuf += tmplen;
        return true;
    }

    bool stop()
    {
        int tmplen;
        if (!this->_final(&this->_ctx, this->_data->pbuf, &tmplen))
        {
            return false;
        }
        this->_data->pbuf += tmplen;
        return true;
    }
};

#endif