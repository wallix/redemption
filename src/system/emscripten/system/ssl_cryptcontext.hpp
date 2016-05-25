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
   Copyright (C) Wallix 2010-2016
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan

*/

#pragma once

#include "system/ssl_calls.hpp"
#include "system/ssl_lib.hpp"
#include "system/ssl_sign.hpp"


enum {
    ENCRYPTION_METHOD_NONE   = 0x00000000,
    ENCRYPTION_METHOD_40BIT  = 0x00000001,
    ENCRYPTION_METHOD_128BIT = 0x00000002,
    ENCRYPTION_METHOD_56BIT  = 0x00000008,
    ENCRYPTION_METHOD_FIPS   = 0x00000010
};


struct CryptContext
{
    int use_count;
    uint8_t sign_key[MD5_DIGEST_LENGTH];
    uint8_t key[MD5_DIGEST_LENGTH];
    uint8_t update_key[MD5_DIGEST_LENGTH];
    SslRC4 rc4;

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
    uint32_t encryptionMethod;

    CryptContext()
        : use_count(0)
        , encryptionMethod(0)
    {
        memset(this->sign_key, 0, MD5_DIGEST_LENGTH);
        memset(this->key, 0, MD5_DIGEST_LENGTH);
        memset(this->update_key, 0, MD5_DIGEST_LENGTH);
    }

    void generate_key(uint8_t * keyblob, uint32_t encryptionMethod)
    {
        // 16-byte transformation used to generate export keys (6.2.2).
        this->encryptionMethod = encryptionMethod;
        memcpy(this->key, keyblob, 16);

        if (encryptionMethod == 1) {
            // 40 bits encryption
            ssllib ssl;
            ssl.sec_make_40bit(this->key);
            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(this->key, 8);
        }
        else {
            // 128 bits encryption

            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(this->key, 16);
        }
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t * data, size_t data_size)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            size_t keylen = (this->encryptionMethod==1)?8:16;

            Sign sign(this->update_key, keylen);
            sign.update(this->key, keylen);
            sign.final(this->key, sizeof(key));

            this->rc4.set_key(this->key, keylen);

            // size, in, out
            this->rc4.crypt(keylen, this->key, this->key);

            if (this->encryptionMethod == 1){
                ssl.sec_make_40bit(this->key);
            }
            this->rc4.set_key(this->key, keylen);
            this->use_count = 0;
        }
        // size, in, out
        this->rc4.crypt(data_size, data, data);
        this->use_count++;
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t const * data, size_t data_size, uint8_t * out_data)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            size_t keylen = (this->encryptionMethod==1)?8:16;

            Sign sign(this->update_key, keylen);
            sign.update(this->key, keylen);
            sign.final(this->key, sizeof(key));

            this->rc4.set_key(this->key, keylen);

            // size, in, out
            this->rc4.crypt(keylen, this->key, this->key);

            if (this->encryptionMethod == 1){
                ssl.sec_make_40bit(this->key);
            }
            this->rc4.set_key(this->key, keylen);
            this->use_count = 0;
        }
        // size, in, out
        this->rc4.crypt(data_size, data, out_data);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(const uint8_t * data, size_t data_size, uint8_t (&signature)[8])
    {
        uint8_t lenhdr[] = {
            static_cast<uint8_t>(data_size & 0xff),
            static_cast<uint8_t>((data_size >> 8) & 0xff),
            static_cast<uint8_t>((data_size >> 16) & 0xff),
            static_cast<uint8_t>((data_size >> 24) & 0xff)
        };

        Sign sign(this->sign_key, (this->encryptionMethod==1)?8:16);
        sign.update(lenhdr, sizeof(lenhdr));
        sign.update(data, data_size);
        sign.final(signature, 8);
    }
};


/*****************************************************************************/

