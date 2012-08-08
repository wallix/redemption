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

        DES_CBC,
        DES_ECB,
        DES_EDE,

        RC2_CBC,
        RC2_ECB,

        RC4,
        RC4_40
    };

    static const EVP_CIPHER * to_evp_cipher(enum_t e)
    {
        switch (e)
        {
            case BLOWFISH_CBC:
                return EVP_bf_cbc();
            case BLOWFISH_ECB:
                return EVP_bf_ecb();
            case BLOWFISH_OFB:
                return EVP_bf_ofb();

            case DES_CBC:
                return EVP_des_cbc();
            case DES_ECB:
                return EVP_des_ecb();
            case DES_EDE:
                return EVP_des_ede();

            case RC2_CBC:
                return EVP_rc2_cbc();
            case RC2_ECB:
                return EVP_rc2_ecb();

            case RC4:
                return EVP_rc4();
            case RC4_40:
                return EVP_rc4_40();
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

    CipherCryptData* data() const { return this->_data; }

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