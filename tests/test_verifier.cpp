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
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of Verifier module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGPRINT

#include <fcntl.h>

#include <iostream>

#include "ssl_calls.hpp"
#include "utils/apps/app_verifier.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include <genrandom.hpp>

#include <new>

#include "fdbuf.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/cryptofile.hpp"


class decrypt_filter
{
    char           buf[CRYPTO_BUFFER_SIZE]; //
    EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       state;                   // enum crypto_file_state
    unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

public:
    decrypt_filter() = default;
    //: pos(0)
    //, raw_size(0)
    //, state(0)
    //, MAX_CIPHERED_SIZE(0)
    //{}

    template<class Source>
    int decrypt_open(Source & src, unsigned char * trace_key)
    {
        ::memset(this->buf, 0, sizeof(this->buf));
        ::memset(&this->ectx, 0, sizeof(this->ectx));

        this->pos = 0;
        this->raw_size = 0;
        this->state = 0;
        const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
        this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        unsigned char tmp_buf[40];

        if (const ssize_t err = this->raw_read(src, tmp_buf, 40)) {
            return err;
        }

        // Check magic
        const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
        if (magic != WABCRYPTOFILE_MAGIC) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Wrong file type %04x != %04x\n",
                ::getpid(), magic, WABCRYPTOFILE_MAGIC);
            return -1;
        }
        const int version = tmp_buf[4] + (tmp_buf[5] << 8) + (tmp_buf[6] << 16) + (tmp_buf[7] << 24);
        if (version > WABCRYPTOFILE_VERSION) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                ::getpid(), version, WABCRYPTOFILE_VERSION);
            return -1;
        }

        unsigned char * const iv = tmp_buf + 8;

        const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
        const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
        const int          nrounds = 5;
        unsigned char      key[32];
        const int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), reinterpret_cast<const unsigned char *>(salt),
                                       trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
        if (i != 32) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
            return -1;
        }

        ::EVP_CIPHER_CTX_init(&this->ectx);
        if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
            return -1;
        }

        return 0;
    }

    template<class Source>
    ssize_t decrypt_read(Source & src, void * data, size_t len)
    {
        if (this->state & CF_EOF) {
            //printf("cf EOF\n");
            return 0;
        }

        unsigned int requested_size = len;

        while (requested_size > 0) {
            // Check how much we have decoded
            if (!this->raw_size) {
                // Buffer is empty. Read a chunk from file
                /*
                 i f (-1 == ::do_chunk_read*(this)) {
                     return -1;
            }
            */
                // TODO: avoid reading size directly into an integer, performance enhancement is minimal
                // and it's not portable because of endianness issue => read in a buffer and decode by hand
                unsigned char tmp_buf[4] = {};
                if (const int err = this->raw_read(src, tmp_buf, 4)) {
                    return err;
                }

                uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);

                if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                    this->state |= CF_EOF;
                    this->pos = 0;
                    this->raw_size = 0;
                }
                else {
                    if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        return -1;
                    }
                    else {
                        uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                        //char ciphered_buf[ciphered_buf_size];
                        unsigned char ciphered_buf[65536];
                        //char compressed_buf[compressed_buf_size];
                        unsigned char compressed_buf[65536];

                        if (const ssize_t err = this->raw_read(src, ciphered_buf, ciphered_buf_size)) {
                            return err;
                        }

                        if (this->xaes_decrypt(ciphered_buf, ciphered_buf_size, compressed_buf, &compressed_buf_size)) {
                            return -1;
                        }

                        size_t chunk_size = CRYPTO_BUFFER_SIZE;
                        const snappy_status status = snappy_uncompress(reinterpret_cast<char *>(compressed_buf),
                                                                       compressed_buf_size, this->buf, &chunk_size);

                        switch (status)
                        {
                            case SNAPPY_OK:
                                break;
                            case SNAPPY_INVALID_INPUT:
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code INVALID_INPUT!\n", getpid());
                                return -1;
                            case SNAPPY_BUFFER_TOO_SMALL:
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                                return -1;
                            default:
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with unknown status code (%d)!\n", getpid(), status);
                                return -1;
                        }

                        this->pos = 0;
                        // When reading, raw_size represent the current chunk size
                        this->raw_size = chunk_size;
                    }
                }

                // TODO: check that
                if (!this->raw_size) { // end of file reached
                    break;
                }
            }
            // remaining_size is the amount of data available in decoded buffer
            unsigned int remaining_size = this->raw_size - this->pos;
            // Check how much we can copy
            unsigned int copiable_size = MIN(remaining_size, requested_size);
            // Copy buffer to caller
            ::memcpy(static_cast<char*>(data) + (len - requested_size), this->buf + this->pos, copiable_size);
            this->pos      += copiable_size;
            requested_size -= copiable_size;
            // Check if we reach the end
            if (this->raw_size == this->pos) {
                this->raw_size = 0;
            }
        }
        return len - requested_size;
    }

private:
    ///\return 0 if success, otherwise a negatif number
    template<class Source>
    ssize_t raw_read(Source & src, void * data, size_t len)
    {
        ssize_t err = src.read(data, len);
        return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
    }

    int xaes_decrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
    {
        int safe_size = *dst_sz;
        int remaining_size = 0;

        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
            return -1;
        }
        if (EVP_DecryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
            return -1;
        }
        if (EVP_DecryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
            return -1;
        }
        *dst_sz = safe_size + remaining_size;
        return 0;
    }
};


extern "C" {

struct crypto_file;

/**********************************************
 *                Public API                  *
 **********************************************/

enum Priv_crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

class Priv_crypto_type_base
{
    Priv_crypto_type type;

public:
    Priv_crypto_type_base(Priv_crypto_type t)
    : type(t)
    {}

    bool is_encrypt() const
    { return CRYPTO_ENCRYPT_TYPE == type; }

    bool is_decrypt() const
    { return CRYPTO_DECRYPT_TYPE == type; }
};



struct Priv_crypto_file_decrypt
: Priv_crypto_type_base
{
  decrypt_filter decrypt;
  io::posix::fdbuf file;

  Priv_crypto_file_decrypt(int fd)
  : Priv_crypto_type_base(CRYPTO_DECRYPT_TYPE)
  , file(fd)
  {}
};

struct Priv_crypto_file_encrypt
: Priv_crypto_type_base
{
  transfil::encrypt_filter encrypt;
  io::posix::fdbuf file;

  Priv_crypto_file_encrypt(int fd)
  : Priv_crypto_type_base(CRYPTO_ENCRYPT_TYPE)
  , file(fd)
  {}
};


crypto_file * crypto_open_read(int systemfd, unsigned char * trace_key,  CryptoContext * cctx)
{
    (void)cctx;
    Priv_crypto_file_decrypt * cf_struct = new (std::nothrow) Priv_crypto_file_decrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->decrypt.decrypt_open(cf_struct->file, trace_key)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<crypto_file*>(cf_struct);
}

crypto_file * crypto_open_write(int systemfd, unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
{
    Priv_crypto_file_encrypt * cf_struct = new (std::nothrow) Priv_crypto_file_encrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, cctx, iv)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<crypto_file*>(cf_struct);
}

/* Flush procedure (compression, encryption, effective file writing)
 * Return 0 on success, -1 on error
 */
int crypto_flush(crypto_file * cf)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        return -1;
    }
    Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.flush(cf_struct->file);
}

/* The actual read method. Read chunks until we reach requested size.
 * Return the actual size read into buf, -1 on error
 */
int crypto_read(crypto_file * cf, char * buf, unsigned int buf_size)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        Priv_crypto_file_decrypt * cf_struct = reinterpret_cast<Priv_crypto_file_decrypt*>(cf);
        return cf_struct->decrypt.decrypt_read(cf_struct->file, buf, buf_size);
    }
    return -1;
}

/* Actually appends data to crypto_file buffer, flush if buffer gets full
 * Return the written size, -1 on error
 */
int crypto_write(crypto_file *cf, const char * buf, unsigned int size)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        return -1;
    }
    Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.write(cf_struct->file, buf, size);
}

int crypto_close(crypto_file *cf, unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key)
{
    int nResult = 0;

    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        Priv_crypto_file_decrypt * cf_struct = reinterpret_cast<Priv_crypto_file_decrypt*>(cf);
        delete cf_struct;
    }
    else {
        Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
        nResult = cf_struct->encrypt.close(cf_struct->file, hash, hmac_key);
        delete cf_struct;
    }

    return nResult;
}

} // extern "C"

#ifdef HASH_LEN
#undef HASH_LEN
#endif  // #ifdef HASH_LEN
#define HASH_LEN 64

int libc_read(int fd, char *buf, unsigned int count)
{
   return read(fd, buf, count);
}   /* int libc_read(int fd, char *buf, unsigned int count) */

BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash)
{
    const char * test_file_name = "./TestCheckFileHash";

    /************************
    * Manage encryption key *
    ************************/
    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));
    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini, 1);

    uint8_t hmac_key[32] = {};

    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };
    BOOST_CHECK(0 == cctx.compute_hmac(hmac_key, HASH_DERIVATOR));
    OpenSSL_add_all_digests();

    // Any iv key would do, we are checking round trip
    unsigned char iv[32] = {
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
    };

    unsigned char derivator[DERIVATOR_LENGTH];
    cctx.get_derivator(test_file_name, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    if (cctx.compute_hmac(trace_key, derivator) == -1){
        BOOST_CHECK(false);
    }

    int system_fd = open(test_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (system_fd == -1){
        printf("failed opening=%s\n", test_file_name);
        BOOST_CHECK(false);
    }

    crypto_file * cf_struct = crypto_open_write(system_fd, trace_key, &cctx, iv);
    if (!cf_struct){
        close(system_fd);
    }

    BOOST_CHECK(cf_struct);

    const char    * data     = "Indentation Settings determine the size of the tab stops, "
                               "and control whether the tab key should insert tabs or spaces.";
    const size_t    data_len = strlen(data);
    int             res;
    unsigned char   hash[HASH_LEN];


    for (int i = 0; i < 256; i ++) {
        res = crypto_write(cf_struct, const_cast<char *>(data), data_len);

        BOOST_CHECK_EQUAL(data_len, res);
    }

    res = crypto_close(cf_struct, hash, cctx.get_hmac_key());

    BOOST_CHECK_EQUAL(0, res);

    BOOST_CHECK_EQUAL(true, check_file_hash_sha256(test_file_name, cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()),
                                                   hash, HASH_LEN / 2, 4096));
    BOOST_CHECK_EQUAL(true, check_file_hash_sha256(test_file_name, cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()),
                                                   hash + (HASH_LEN / 2), HASH_LEN / 2, 0));

    unlink(test_file_name);
}   /* BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash) */
