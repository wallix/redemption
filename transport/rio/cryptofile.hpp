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
   Copyright (C) Wallix 2010-2014
   Author(s): Christophe Grosjean
*/

#ifndef WABCRYPTOFILE_HPP
#define WABCRYPTOFILE_HPP

#include <errno.h>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

extern "C" {
#include "cryptofile.h"
}

/*****************************************************************************************************
 *                             System access wrapper                                                 *
 *****************************************************************************************************/
static inline int dev_urandom_read(unsigned char *buf, int sz) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        printf("[CRYPTO_ERROR][%d]: /dev/urandom!\n", getpid());
        return -1;
    }
    ssize_t read_ret = read(fd, buf, sz);
    if (read_ret == -1) {
        printf("[CRYPTO_ERROR][%d]: read /dev/urandom! error=%s\n", getpid(), strerror(errno));
        int close_ret = close(fd);
        if (close_ret == -1) {
            printf("[CRYPTO_ERROR][%d]: close /dev/urandom! error=%s\n", getpid(), strerror(errno));
        }
        return -1;
    }
    int close_ret = close(fd);
    if (close_ret == -1) {
        printf("[CRYPTO_ERROR][%d]: close /dev/urandom! error=%s\n", getpid(), strerror(errno));
    }
    return sz;
}

/* Decrypt src_buf into dst_buf. Update dst_sz with decrypted output size
 * Return 0 on success, negative value on error
 */
static inline int xaes_decrypt(EVP_CIPHER_CTX *ectx, char *src_buf, uint32_t src_sz, char *dst_buf, uint32_t *dst_sz) {
    int safe_size = *dst_sz;
    int remaining_size = 0;

    /* allows reusing of ectx for multiple encryption cycles */
    if (EVP_DecryptInit_ex(ectx, NULL, NULL, NULL, NULL) != 1){
        printf("[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
        return -1;
    }
    if (EVP_DecryptUpdate(ectx, (unsigned char *)dst_buf, &safe_size, (unsigned char *)src_buf, src_sz) != 1){
        printf("[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
        return -1;
    }
    if (EVP_DecryptFinal_ex(ectx, (unsigned char *)dst_buf + safe_size, &remaining_size) != 1){
        printf("[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
        return -1;
    }
    *dst_sz = safe_size + remaining_size;
    return 0;
}

/* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
 * Return 0 on success, negative value on error
 */
static inline int xaes_encrypt(EVP_CIPHER_CTX *ectx, char *src_buf, uint32_t src_sz, char *dst_buf, uint32_t *dst_sz) {
    int safe_size = *dst_sz;
    int remaining_size = 0;

    /* allows reusing of ectx for multiple encryption cycles */
    if (EVP_EncryptInit_ex(ectx, NULL, NULL, NULL, NULL) != 1){
        printf("[CRYPTO_ERROR][%d]: Could not prepare encryption context!\n", getpid());
        return -1;
    }
    if (EVP_EncryptUpdate(ectx, (unsigned char *)dst_buf, &safe_size, (unsigned char *)src_buf, src_sz) != 1){
        printf("[CRYPTO_ERROR][%d]: Could encrypt data!\n", getpid());
        return -1;
    }
    if (EVP_EncryptFinal_ex(ectx, (unsigned char *)dst_buf + safe_size, &remaining_size) != 1){
        printf("[CRYPTO_ERROR][%d]: Could not finish encryption!\n", getpid());
        return -1;
    }
    *dst_sz = safe_size + remaining_size;
    return 0;
}

static inline bool raw_read(int fd, uint8_t * buffer, size_t requested)
{
    const size_t initial_requested = requested;
    while (requested > 0){
        int read_ret = read(fd, buffer + initial_requested - requested, requested);
        if (read_ret == -1) {
            if ((errno == EINTR)||(errno == EAGAIN)){
                continue;
            }
            printf("[CRYPTO_ERROR][%d]: read error : %s!\n", getpid(), strerror(errno));
            return false;
        }
        if (read_ret == 0) {
            printf("[CRYPTO_ERROR][%d]: read error : unexpected EOF!\n", getpid());
            return false;
        }
        requested -= read_ret;
    }

    return true;
}

struct crypto_file {
private:
    int            fd;                      // system file descriptor
    int            oflag;                   // file output flag (O_RDONLY or O_WRONLY)
    char           buf[CRYPTO_BUFFER_SIZE]; //
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       file_size;               // the current file size
    uint32_t       state;                   // enum crypto_file_state
    int            version;                 // file format version
    unsigned int   MAX_COMPRESSED_SIZE;     // = snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
    unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
    EVP_MD_CTX     hctx;                    // hash context
    EVP_MD_CTX     hctx4k;                  // hash context

public:
    crypto_file()
        : fd(-1)
        , oflag(0)
        , pos(0)
        , raw_size(0)
        , file_size(0)
        , state(0)
        , version(0)
        , MAX_COMPRESSED_SIZE(0)
        , MAX_CIPHERED_SIZE(0)
    {
        ::memset(this->buf, 0, sizeof(this->buf));
        ::memset(&this->ectx, 0, sizeof(this->ectx));
        ::memset(&this->hctx, 0, sizeof(this->hctx));
        ::memset(&this->hctx4k, 0, sizeof(this->hctx4k));
    }

    // Opening a crypto file for reading.
    int open_read_init(int systemfd, unsigned char * trace_key, struct CryptoContext * cctx)
    {
        this->MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
        this->MAX_CIPHERED_SIZE   = this->MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
        this->fd                  = systemfd;
        this->oflag               = O_RDONLY;

        unsigned char tmp_buf[40];

        if (!::raw_read(this->fd, tmp_buf, 40))
            return -1;

        // Check magic
        uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
        if (magic != WABCRYPTOFILE_MAGIC) {
            ::printf("[CRYPTO_ERROR][%d]: Wrong file type %04x != %04x\n", ::getpid(), magic, WABCRYPTOFILE_MAGIC);
            return -1;
        }
        this->version = tmp_buf[4] + (tmp_buf[5] << 8) + (tmp_buf[6] << 16) + (tmp_buf[7] << 24);
        if (this->version > WABCRYPTOFILE_VERSION) {
            ::printf("[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n", ::getpid(), this->version, WABCRYPTOFILE_VERSION);
            return -1;
        }

        const EVP_CIPHER * cipher   = ::EVP_aes_256_cbc();
        unsigned int       salt[]   = { 12345, 54321 };    // suspicious, to check...
        int                nrounds  = 5;
        unsigned char      key[32];
        int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), (unsigned char *)salt, trace_key, CRYPTO_KEY_LENGTH, nrounds, key, NULL);
        if (i != 32) {
            ::printf("[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
            return -1;
        }

        unsigned char iv[32];
        memcpy(iv, tmp_buf + 8, 32);

        ::EVP_CIPHER_CTX_init(&this->ectx);
        if (::EVP_DecryptInit_ex(&this->ectx, cipher, NULL, key, iv) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not initialize decrypion context\n", ::getpid());
            return -1;
        }
        return 0;
    }

    // Opening a crypto file for writing.
    int open_write_init(int systemfd, unsigned char * trace_key, struct CryptoContext * cctx, const unsigned char * iv)
    {
        this->MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
        this->MAX_CIPHERED_SIZE   = this->MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
        this->fd                  = systemfd;
        this->oflag               = O_WRONLY;

        const EVP_CIPHER * cipher   = EVP_aes_256_cbc();
        unsigned int       salt[]   = {12345, 54321};    // suspicious, to check...
        int                nrounds  = 5;
        unsigned char      key[32];
        int i = EVP_BytesToKey(cipher, EVP_sha1(), (unsigned char *)salt, trace_key, CRYPTO_KEY_LENGTH, nrounds, key, NULL);
        if (i != 32) {
            ::printf("[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong!\n", ::getpid());
            return -1;
        }

        ::EVP_CIPHER_CTX_init(&this->ectx);
        if (::EVP_EncryptInit_ex(&this->ectx, cipher, NULL, key, iv) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not initialize encryption context!\n", ::getpid());
            return -1;
        }

        // MD stuff
        const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
        if (!md) {
            ::printf("[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
            return -1;
        }

        ::EVP_MD_CTX_init(&this->hctx);
        ::EVP_MD_CTX_init(&this->hctx4k);
        if (::EVP_DigestInit_ex(&this->hctx, md, NULL) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestInit_ex(&this->hctx4k, md, NULL) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not initialize 4k MD hash context!\n", ::getpid());
            return -1;
        }

        // HMAC: key^ipad
        int             blocksize = ::EVP_MD_block_size(md);
        unsigned char * key_buf   = (unsigned char *)::calloc(blocksize, 1);
        if (key_buf == NULL) {
            ::printf("[CRYPTO_ERROR][%d]: malloc!\n", ::getpid());
            return -1;
        }
        ::memset(key_buf, 0, blocksize);
        if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
            unsigned char keyhash[MD_HASH_LENGTH];
            if (::MD_HASH_FUNC((unsigned char *)cctx->hmac_key, CRYPTO_KEY_LENGTH, keyhash) == (void *)0) {
                ::printf("[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                ::free(key_buf);
                return -1;
            }
            ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
        }
        else {
            ::memcpy(key_buf, cctx->hmac_key, CRYPTO_KEY_LENGTH);
        }
        for (int idx = 0; idx <  blocksize; idx++) {
            key_buf[idx] = key_buf[idx] ^ 0x36;
        }
        if (::EVP_DigestUpdate(&this->hctx, key_buf, blocksize) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
            ::free(key_buf);
            return -1;
        }
        if (::EVP_DigestUpdate(&this->hctx4k, key_buf, blocksize) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
            ::free(key_buf);
            return -1;
        }
        ::free(key_buf);

        // update context with previously written data
        {
            char tmp_buf[40] = {};
            tmp_buf[0] = WABCRYPTOFILE_MAGIC & 0xFF;
            tmp_buf[1] = (WABCRYPTOFILE_MAGIC >> 8) & 0xFF;
            tmp_buf[2] = (WABCRYPTOFILE_MAGIC >> 16) & 0xFF;
            tmp_buf[3] = (WABCRYPTOFILE_MAGIC >> 24) & 0xFF;
            tmp_buf[4] = WABCRYPTOFILE_VERSION & 0xFF;
            tmp_buf[5] = (WABCRYPTOFILE_VERSION >> 8) & 0xFF;
            tmp_buf[6] = (WABCRYPTOFILE_VERSION >> 16) & 0xFF;
            tmp_buf[7] = (WABCRYPTOFILE_VERSION >> 24) & 0xFF;
            ::memcpy(tmp_buf + 8, iv, 32);

            // TODO: Add write loop
            ssize_t write_ret = ::write(this->fd, tmp_buf, 40);
            // TODO: if I suceeded writing a broken file, wouldn't it be better to remove it ?
            if (write_ret != 40){
                printf("[CRYPTO_ERROR][%d]: write error! error=%s\n", ::getpid(), ::strerror(errno));
                return -1;
            }
            // update file_size
            this->file_size += 40;

            this->xmd_update(tmp_buf, 40);
        }
        return 0;
    }

    /* Flush procedure (compression, encryption, effective file writing)
     * Return 0 on success, -1 on error
     */
    int flush()
    {
        // No data to flush
        if (!this->pos) {
            return 0;
        }

        // Compress
        // TODO: check this
        char compressed_buf[65536];
        //char compressed_buf[compressed_buf_sz];
        size_t compressed_buf_sz = ::snappy_max_compressed_length(this->pos);
        snappy_status status = snappy_compress(this->buf, this->pos, compressed_buf, &compressed_buf_sz);

        switch (status)
        {
        case SNAPPY_OK:
        break;
        case SNAPPY_INVALID_INPUT:
            printf("[CRYPTO_ERROR][%d]: Snappy compression failed with status code INVALID_INPUT!\n", getpid());
            return -1;
        break;
        case SNAPPY_BUFFER_TOO_SMALL:
            printf("[CRYPTO_ERROR][%d]: Snappy compression failed with status code BUFFER_TOO_SMALL!\n", getpid());
            return -1;
        break;
        default:
            printf("[CRYPTO_ERROR][%d]: Snappy compression failed with unknown status code (%d)!\n", getpid(), status);
            return -1;
        break;
        }

        // Encrypt
        char ciphered_buf[65536];
        //char ciphered_buf[ciphered_buf_sz];
        uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
        if (::xaes_encrypt(&this->ectx, compressed_buf, compressed_buf_sz, ciphered_buf, &ciphered_buf_sz)) {
            return -1;
        }

        // TODO: merge tmp_sz and ciphered_buf and perform only one write

        char tmp_sz[4] = {};
        tmp_sz[0] = ciphered_buf_sz & 0xFF;
        tmp_sz[1] = (ciphered_buf_sz >> 8) & 0xFF;
        tmp_sz[2] = (ciphered_buf_sz >> 16) & 0xFF;
        tmp_sz[3] = (ciphered_buf_sz >> 24) & 0xFF;
        int write_ret1 = ::write(this->fd, tmp_sz, 4);
        if (write_ret1 == -1) {
            ::printf("[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
        }
        // TODO: check errors
        this->xmd_update((char *)&ciphered_buf_sz, 4);
        this->file_size += 4;

        int write_ret2 = ::write(this->fd, ciphered_buf, ciphered_buf_sz);
        if (write_ret2 == -1) {
            ::printf("[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
        }
        // TODO: check errors
        this->xmd_update(ciphered_buf, ciphered_buf_sz);
        this->file_size += ciphered_buf_sz;

        // Reset buffer
        this->pos = 0;
        return 0;
    }

    /* The actual read method. Read chunks until we reach requested size.
     * Return the actual size read into buf, -1 on error
     */
    int read(char * buf, unsigned int buf_size)
    {
        if (this->state & CF_EOF) {
            //printf("cf EOF\n");
            return 0;
        }

        unsigned int requested_size = buf_size;

        while (requested_size > 0) {
            // Check how much we have decoded
            if (!this->raw_size) {
                // Buffer is empty. Read a chunk from file
/*
                if (-1 == ::do_chunk_read(this)) {
                    return -1;
                }
*/
                // TODO: avoid reading size directly into an integer, performance enhancement is minimal
                // and it's not portable because of endianness issue => read in a buffer and decode by hand
                unsigned char tmp_buf[4] = {};
                if (!::raw_read(this->fd, tmp_buf, 4)) {
                    return -1;
                }

                uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);

                if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                    this->state |= CF_EOF;
                    this->pos = 0;
                    this->raw_size = 0;
                }
                else {
                    if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                        ::printf("[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        return -1;
                    } else {
                        uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                        //char ciphered_buf[ciphered_buf_size];
                        char ciphered_buf[65536];
                        //char compressed_buf[compressed_buf_size];
                        char compressed_buf[65536];

                        if (!::raw_read(this->fd, reinterpret_cast<uint8_t *>(ciphered_buf), ciphered_buf_size)) {
                            return -1;
                        }

                        if (::xaes_decrypt(&this->ectx, ciphered_buf, ciphered_buf_size, compressed_buf, &compressed_buf_size)) {
                            return -1;
                        }

                        size_t chunk_size = CRYPTO_BUFFER_SIZE;
                        snappy_status status = snappy_uncompress(compressed_buf, compressed_buf_size, this->buf, &chunk_size);

                        switch (status)
                        {
                        case SNAPPY_OK:
                        break;
                        case SNAPPY_INVALID_INPUT:
                            printf("[CRYPTO_ERROR][%d]: Snappy decompression failed with status code INVALID_INPUT!\n", getpid());
                            return -1;
                        break;
                        case SNAPPY_BUFFER_TOO_SMALL:
                            printf("[CRYPTO_ERROR][%d]: Snappy decompression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                            return -1;
                        break;
                        default:
                            printf("[CRYPTO_ERROR][%d]: Snappy decompression failed with unknown status code (%d)!\n", getpid(), status);
                            return -1;
                        break;
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
            ::memcpy(buf + (buf_size - requested_size), this->buf + this->pos, copiable_size);
            this->pos      += copiable_size;
            requested_size -= copiable_size;
            // Check if we reach the end
            if (this->raw_size == this->pos) {
                this->raw_size = 0;
            }
        }
        return buf_size - requested_size;
    }

    /* Actually appends data to crypto_file buffer, flush if buffer gets full
     * Return the written size, -1 on error
     */
    int write(const char * buf, unsigned int size)
    {
        unsigned int remaining_size = size;
        while (remaining_size > 0) {
            // Check how much we can append into buffer
            unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->pos, remaining_size);
            // Append and update pos pointer
            ::memcpy(this->buf + this->pos, buf + (size - remaining_size), available_size);
            this->pos += available_size;
            // If buffer is full, flush it to disk
            if (this->pos == (CRYPTO_BUFFER_SIZE)) {
                if (this->flush()) {
                    return -1;
                }
            }
            remaining_size -= available_size;
        }
        // Update raw size counter
        this->raw_size += size;
        return size;
    }

    int close(unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key)
    {
        int result = 0;

        if (this->oflag & O_WRONLY) {
            this->flush();

            char     tmp_buf[8] = {};
            uint32_t eof_magic  = WABCRYPTOFILE_EOF_MAGIC;
            tmp_buf[0] = eof_magic & 0xFF;
            tmp_buf[1] = (eof_magic >> 8) & 0xFF;
            tmp_buf[2] = (eof_magic >> 16) & 0xFF;
            tmp_buf[3] = (eof_magic >> 24) & 0xFF;
            tmp_buf[4] = this->raw_size & 0xFF;
            tmp_buf[5] = (this->raw_size >> 8) & 0xFF;
            tmp_buf[6] = (this->raw_size >> 16) & 0xFF;
            tmp_buf[7] = (this->raw_size >> 24) & 0xFF;

            int write_ret1 = ::write(this->fd, tmp_buf, 8);
            if (write_ret1 == -1){
                // TOOD: actual error code could help
                ::printf("[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            }
            this->file_size += 8;

            this->xmd_update(tmp_buf, 8);

            if (hash) {
                unsigned char tmp_hash[MD_HASH_LENGTH << 1];
                if (::EVP_DigestFinal_ex(&this->hctx4k, tmp_hash, NULL) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[0] = '\0';
                }
                if (::EVP_DigestFinal_ex(&this->hctx, tmp_hash + MD_HASH_LENGTH, NULL) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[MD_HASH_LENGTH] = '\0';
                }
                // HMAC: MD(key^opad + MD(key^ipad))
                const EVP_MD *md = ::EVP_get_digestbyname(MD_HASH_NAME);
                if (!md) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not find MD message digest\n", ::getpid());
                    return -1;
                }
                int blocksize = ::EVP_MD_block_size(md);
                unsigned char * key_buf = (unsigned char *)::calloc(blocksize, 1);
                if (key_buf == NULL) {
                    ::printf("[CRYPTO_ERROR][%d]: malloc\n", ::getpid());
                    return -1;
                }
                ::memset(key_buf, '\0', blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if (::MD_HASH_FUNC((unsigned char *)hmac_key, CRYPTO_KEY_LENGTH, keyhash) == (void *)0) {
                        ::printf("[CRYPTO_ERROR][%d]: Could not hash crypto key\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, hmac_key, CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++)
                    key_buf[idx] = key_buf[idx] ^ 0x5c;

                EVP_MD_CTX mdctx;
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, NULL) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash, MD_HASH_LENGTH) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash, NULL) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[0] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, NULL) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1){
                    ::printf("[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash + MD_HASH_LENGTH, MD_HASH_LENGTH) != 1){
                    ::printf("[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash + MD_HASH_LENGTH, NULL) != 1) {
                    ::printf("[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[MD_HASH_LENGTH] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
                ::free(key_buf);
            }
        }
        // TODO: the actual closing of the file should be done by caller (as open is now done by caller)
        ::close(this->fd);
        this->fd = -1;

        return result;
    }

private:
    /* Update hash context with new data.
     * Returns 0 on success, -1 on error
     */
    int xmd_update(const char * src_buf, uint32_t src_sz) {
        if (::EVP_DigestUpdate(&this->hctx, src_buf, src_sz) != 1) {
            ::printf("[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
            return -1;
        }
        if (this->file_size < 4096) {
            size_t remaining_size = 4096 - this->file_size;
            size_t hashable_size = MIN(remaining_size, src_sz);
            if (::EVP_DigestUpdate(&this->hctx4k, src_buf, hashable_size) != 1) {
                ::printf("[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                return -1;
            }
        }
        return 0;
    }
};

#endif
