#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <memory>

// this is to silent warning as Python.h will redefine this constant
#undef _XOPEN_SOURCE
// this is to silent warning as Python.h will redefine this constant
#undef _POSIX_C_SOURCE
#include "Python.h"
#include <structmember.h>
typedef PyObject * __attribute__((__may_alias__)) AlPyObject;
#include <algorithm>
#include <unistd.h>
#include <genrandom.hpp>
#include <new>


#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#include <snappy-c.h>

#include "fdbuf.hpp"

#include "transport/cryptofile.hpp"

namespace transfil {

    class decrypt_filter3
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       state;                   // enum crypto_file_state
        unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

    public:
        decrypt_filter3() = default;
        //: pos(0)
        //, raw_size(0)
        //, state(0)
        //, MAX_CIPHERED_SIZE(0)
        //{}

        int open(io::posix::fdbuf & src, unsigned char * trace_key)
        {
            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));

            this->pos = 0;
            this->raw_size = 0;
            this->state = 0;
            const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
            this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

            unsigned char tmp_buf[40];

            if (src.read(tmp_buf, 40) < 40) {
                return err < 0 ? err :-1;
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

        ssize_t read(io::posix::fdbuf & src, void * data, size_t len)
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
                    if (src.read(tmp_buf, 4) < 4) {
                        return err < 0 ? err : -1;
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

                            if (src.read(ciphered_buf, ciphered_buf_size) < ciphered_buf_size) {
                                return err < 0 ? err :-1;
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
        ssize_t raw_read(io::posix::fdbuf & src, void * data, size_t len)
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

}

namespace transfil {

    class encrypt_filter3
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        EVP_MD_CTX     hctx;                    // hash context
        EVP_MD_CTX     hctx4k;                  // hash context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       file_size;               // the current file size

    public:
        encrypt_filter3() = default;
        //: pos(0)
        //, raw_size(0)
        //, file_size(0)
        //{}

        template<class Sink>
        int open(Sink & snk, const unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
        {
            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));
            ::memset(&this->hctx, 0, sizeof(this->hctx));
            ::memset(&this->hctx4k, 0, sizeof(this->hctx4k));
            this->pos = 0;
            this->raw_size = 0;
            this->file_size = 0;

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
            if (::EVP_EncryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
                return -1;
            }

            // MD stuff
            const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
            if (!md) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
                return -1;
            }

            ::EVP_MD_CTX_init(&this->hctx);
            ::EVP_MD_CTX_init(&this->hctx4k);
            if (::EVP_DigestInit_ex(&this->hctx, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
                return -1;
            }
            if (::EVP_DigestInit_ex(&this->hctx4k, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize 4k MD hash context!\n", ::getpid());
                return -1;
            }

            // HMAC: key^ipad
            const int     blocksize = ::EVP_MD_block_size(md);
            unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
            {
                if (key_buf == nullptr) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc!\n", ::getpid());
                    return -1;
                }
                const std::unique_ptr<unsigned char[]> auto_free(key_buf);
                ::memset(key_buf, 0, blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if ( ! ::MD_HASH_FUNC(static_cast<unsigned char *>(cctx->get_hmac_key()), CRYPTO_KEY_LENGTH, keyhash)) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, cctx->get_hmac_key(), CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++) {
                    key_buf[idx] = key_buf[idx] ^ 0x36;
                }
                if (::EVP_DigestUpdate(&this->hctx, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&this->hctx4k, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                    return -1;
                }
            }

            // update context with previously written data
            char tmp_buf[40];
            tmp_buf[0] = WABCRYPTOFILE_MAGIC & 0xFF;
            tmp_buf[1] = (WABCRYPTOFILE_MAGIC >> 8) & 0xFF;
            tmp_buf[2] = (WABCRYPTOFILE_MAGIC >> 16) & 0xFF;
            tmp_buf[3] = (WABCRYPTOFILE_MAGIC >> 24) & 0xFF;
            tmp_buf[4] = WABCRYPTOFILE_VERSION & 0xFF;
            tmp_buf[5] = (WABCRYPTOFILE_VERSION >> 8) & 0xFF;
            tmp_buf[6] = (WABCRYPTOFILE_VERSION >> 16) & 0xFF;
            tmp_buf[7] = (WABCRYPTOFILE_VERSION >> 24) & 0xFF;
            ::memcpy(tmp_buf + 8, iv, 32);

            // TODO: if I suceeded writing a broken file, wouldn't it be better to remove it ?
            if (const ssize_t write_ret = this->raw_write(snk, tmp_buf, 40)){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: write error! error=%s\n", ::getpid(), ::strerror(errno));
                return write_ret;
            }
            // update file_size
            this->file_size += 40;

            return this->xmd_update(tmp_buf, 40);
        }

        template<class Sink>
        ssize_t write(Sink & snk, const void * data, size_t len)
        {
            unsigned int remaining_size = len;
            while (remaining_size > 0) {
                // Check how much we can append into buffer
                unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->pos, remaining_size);
                // Append and update pos pointer
                ::memcpy(this->buf + this->pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
                this->pos += available_size;
                // If buffer is full, flush it to disk
                if (this->pos == CRYPTO_BUFFER_SIZE) {
                    if (this->flush(snk)) {
                        return -1;
                    }
                }
                remaining_size -= available_size;
            }
            // Update raw size counter
            this->raw_size += len;
            return len;
        }

        /* Flush procedure (compression, encryption, effective file writing)
         * Return 0 on success, negatif on error
         */
        template<class Sink>
        int flush(Sink & snk)
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
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with status code INVALID_INPUT!\n", getpid());
                    return -1;
                case SNAPPY_BUFFER_TOO_SMALL:
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                    return -1;
                default:
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with unknown status code (%d)!\n", getpid(), status);
                    return -1;
            }

            // Encrypt
            unsigned char ciphered_buf[4 + 65536];
            //char ciphered_buf[ciphered_buf_sz];
            uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
            {
                const unsigned char * src_buf = reinterpret_cast<unsigned char*>(compressed_buf);
                if (this->xaes_encrypt(src_buf, compressed_buf_sz, ciphered_buf + 4, &ciphered_buf_sz)) {
                    return -1;
                }
            }

            ciphered_buf[0] = ciphered_buf_sz & 0xFF;
            ciphered_buf[1] = (ciphered_buf_sz >> 8) & 0xFF;
            ciphered_buf[2] = (ciphered_buf_sz >> 16) & 0xFF;
            ciphered_buf[3] = (ciphered_buf_sz >> 24) & 0xFF;

            ciphered_buf_sz += 4;

            if (const ssize_t err = this->raw_write(snk, ciphered_buf, ciphered_buf_sz)) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
                return err;
            }
            if (-1 == this->xmd_update(&ciphered_buf, ciphered_buf_sz)) {
                return -1;
            }
            this->file_size += ciphered_buf_sz;

            // Reset buffer
            this->pos = 0;
            return 0;
        }

        template<class Sink>
        int close(Sink & snk, unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
        {
            int result = this->flush(snk);

            const uint32_t eof_magic = WABCRYPTOFILE_EOF_MAGIC;
            unsigned char tmp_buf[8] = {
                eof_magic & 0xFF,
                (eof_magic >> 8) & 0xFF,
                (eof_magic >> 16) & 0xFF,
                (eof_magic >> 24) & 0xFF,
                uint8_t(this->raw_size & 0xFF),
                uint8_t((this->raw_size >> 8) & 0xFF),
                uint8_t((this->raw_size >> 16) & 0xFF),
                uint8_t((this->raw_size >> 24) & 0xFF),
            };

            int write_ret1 = this->raw_write(snk, tmp_buf, 8);
            if (write_ret1){
                // TOOD: actual error code could help
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            }
            this->file_size += 8;

            this->xmd_update(tmp_buf, 8);

            if (hash) {
                unsigned char tmp_hash[MD_HASH_LENGTH << 1];
                if (::EVP_DigestFinal_ex(&this->hctx4k, tmp_hash, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[0] = '\0';
                }
                if (::EVP_DigestFinal_ex(&this->hctx, tmp_hash + MD_HASH_LENGTH, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[MD_HASH_LENGTH] = '\0';
                }
                // HMAC: MD(key^opad + MD(key^ipad))
                const EVP_MD *md = ::EVP_get_digestbyname(MD_HASH_NAME);
                if (!md) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find MD message digest\n", ::getpid());
                    return -1;
                }
                const int     blocksize = ::EVP_MD_block_size(md);
                unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
                if (key_buf == nullptr) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc\n", ::getpid());
                    return -1;
                }
                const std::unique_ptr<unsigned char[]> auto_free(key_buf);
                ::memset(key_buf, '\0', blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if ( ! ::MD_HASH_FUNC(static_cast<const unsigned char *>(hmac_key), CRYPTO_KEY_LENGTH, keyhash)) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, hmac_key, CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++) {
                    key_buf[idx] = key_buf[idx] ^ 0x5c;
                }

                EVP_MD_CTX mdctx;
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash, MD_HASH_LENGTH) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[0] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1){
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash + MD_HASH_LENGTH, MD_HASH_LENGTH) != 1){
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash + MD_HASH_LENGTH, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[MD_HASH_LENGTH] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
            }

            return result;
        }

    private:
        ///\return 0 if success, otherwise a negatif number
        template<class Sink>
        ssize_t raw_write(Sink & snk, void * data, size_t len)
        {
            ssize_t err = snk.write(data, len);
            return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
        }

        /* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
         * Return 0 on success, negative value on error
         */
        int xaes_encrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
        {
            int safe_size = *dst_sz;
            int remaining_size = 0;

            /* allows reusing of ectx for multiple encryption cycles */
            if (EVP_EncryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare encryption context!\n", getpid());
                return -1;
            }
            if (EVP_EncryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could encrypt data!\n", getpid());
                return -1;
            }
            if (EVP_EncryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish encryption!\n", getpid());
                return -1;
            }
            *dst_sz = safe_size + remaining_size;
            return 0;
        }

        /* Update hash context with new data.
         * Returns 0 on success, -1 on error
         */
        int xmd_update(const void * src_buf, uint32_t src_sz)
        {
            if (::EVP_DigestUpdate(&this->hctx, src_buf, src_sz) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                return -1;
            }
            if (this->file_size < 4096) {
                size_t remaining_size = 4096 - this->file_size;
                size_t hashable_size = MIN(remaining_size, src_sz);
                if (::EVP_DigestUpdate(&this->hctx4k, src_buf, hashable_size) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                    return -1;
                }
            }
            return 0;
        }
    };
}



struct crypto_file_read
{
  transfil::decrypt_filter3 decrypt;
  io::posix::fdbuf file;
  crypto_file_read(int fd) : file(fd) {}
};

struct crypto_file_write
{
  transfil::encrypt_filter3 encrypt;
  io::posix::fdbuf file;
  crypto_file_write(int fd) : file(fd) {}
};

enum crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

extern "C" {

int gl_read_nb_files = 0;
struct crypto_file_read * gl_file_store_read[1024];
int gl_write_nb_files = 0;
struct crypto_file_write * gl_file_store_write[1024];

unsigned char iv[32] = {0}; //  not used for reading
}


struct crypto_file
{
    enum crypto_type type;
    int idx;
    crypto_file(): type(CRYPTO_DECRYPT_TYPE), idx(-1) {}


    crypto_file(crypto_type t, int fd)
    : type(t)
    , idx(-1)
    {
        switch (t){
        case CRYPTO_DECRYPT_TYPE:
        {
            auto cf = new crypto_file_read(fd);
            int idx = 0;
            for (; idx < gl_read_nb_files ; idx++){
                if (gl_file_store_read[idx] == nullptr){
                    break;
                }
            }
            gl_read_nb_files += 1;
            gl_file_store_read[idx] = cf;
            this->idx = idx;
        }
        break;
        case CRYPTO_ENCRYPT_TYPE:
        {
            auto cf = new crypto_file_write(fd);
            int idx = 0;
            for (; idx < gl_write_nb_files ; idx++){
                if (gl_file_store_write[idx] == nullptr){
                    break;
                }
            }
            gl_write_nb_files += 1;
            gl_file_store_write[idx] = cf;
            this->idx = idx;
        }
        break;
        }
    }
};

extern "C" {
int gl_nb_files = 0;
struct crypto_file gl_file_store[1024];
}


extern "C" {
    UdevRandom * get_rnd();
    CryptoContext * get_cctx();
}

/* File format V1:  ([...] represent an uint32_t)
 *
 * Header:
 *  [WABCRYPTOFILE_MAGIC][FILE_VERSION][Crypto IV]
 *
 * Chunk:
 *  [ciphered chunk size (size it takes on disk)][data]
 *
 * Footer:
 *  [WABCRYPTOFILE_EOF_MAGIC][raw_file_size]
 *
 */


UdevRandom * get_rnd(){
    static UdevRandom * rnd = nullptr;
    if (rnd == nullptr){
        rnd = new UdevRandom;
    }
    return rnd;
}

Inifile * get_ini(){
    static Inifile * ini = nullptr;
    if (ini == nullptr){
        ini = new Inifile;
        ini->set<cfg::crypto::key0>(cstr_array_view(
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"));
    }
    return ini;
}


CryptoContext * get_cctx()
{
    static CryptoContext * cctx = nullptr;
    if (cctx == nullptr){
        cctx = new CryptoContext(*get_rnd(), *get_ini(), 1);
    }
    return cctx;
}

extern "C" {

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} redcryptofile_NoddyObject;

// This union is work around for 
typedef union {
    PyTypeObject pto;
    PyObject po;
} t_PyTyOb;


#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
t_PyTyOb redcryptofile_NoddyType = {
    PyObject_HEAD_INIT(nullptr)
    0,                         /*ob_size*/
    "redcryptofile.Noddy",     /*tp_name*/
    sizeof(redcryptofile_NoddyObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    nullptr,                   /*tp_dealloc*/
    nullptr,                   /*tp_print*/
    nullptr,                   /*tp_getattr*/
    nullptr,                   /*tp_setattr*/
    nullptr,                   /*tp_compare*/
    nullptr,                   /*tp_repr*/
    nullptr,                   /*tp_as_number*/
    nullptr,                   /*tp_as_sequence*/
    nullptr,                   /*tp_as_mapping*/
    nullptr,                   /*tp_hash */
    nullptr,                   /*tp_call*/
    nullptr,                   /*tp_str*/
    nullptr,                   /*tp_getattro*/
    nullptr,                   /*tp_setattro*/
    nullptr,                   /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Noddy objects",           /* tp_doc */
    nullptr,                   /* tp_traverse */
    nullptr,                   /* tp_clear */
    nullptr,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    nullptr,                   /* tp_iter */
    nullptr,                   /* tp_iternext */
    nullptr,                   /* tp_methods */
    nullptr,                   /* tp_members */
    nullptr,                   /* tp_getset */
    nullptr,                   /* tp_base */
    nullptr,                   /* tp_dict */
    nullptr,                   /* tp_descr_get */
    nullptr,                   /* tp_descr_set */
    0,                         /* tp_dictoffset */
    nullptr,                   /* tp_init */
    nullptr,                   /* tp_alloc */
    nullptr,                   /* tp_new */
    nullptr,                   /* tp_free */
    nullptr,                   /* tp_is_gc */
    nullptr,                   /* tp_bases */
    nullptr,                   /* tp_mro */
    nullptr,                   /* tp_cache */
    nullptr,                   /* tp_subclasses */
    nullptr,                   /* tp_weaklist */
    nullptr,                   /* tp_del */
};
#pragma GCC diagnostic pop

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    UdevRandom * rnd;
} PyORandom;

static void Random_dealloc(PyORandom* self) {
    printf("Random dealloc\n");
    delete self->rnd;
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *Random_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    printf("Random new\n");
    PyORandom *self = (PyORandom *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->rnd = new UdevRandom;
    }
    return (PyObject *)self;
}

static int Random_init(PyORandom *self, PyObject *args, PyObject *kwds)
{
    printf("Random init\n");
    if (self != nullptr) {
        if (self->rnd == nullptr){
            self->rnd = new UdevRandom;
        }
    }
    return 0;
}

static PyObject *
Random_rand(PyORandom* self)
{
    printf("Random rand\n");
    long val = (long)self->rnd->rand64();
    PyObject * result = PyInt_FromLong(val);
    return result;
}

static PyMemberDef Random_members[] = {
    {nullptr, 0, 0, 0, nullptr}
};


static PyMethodDef Random_methods[] = {
    {"rand", (PyCFunction)Random_rand, METH_NOARGS, "Return a new random int"},
    {nullptr, nullptr, 0, nullptr}
  /* Sentinel */
};

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
t_PyTyOb PyTyRandom = {
    PyObject_HEAD_INIT(nullptr)
    0,                         /*ob_size*/
    "redcryptofile.Random",    /*tp_name*/
    sizeof(PyORandom), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Random_dealloc,/*tp_dealloc*/
    nullptr,                   /*tp_print*/
    nullptr,                   /*tp_getattr*/
    nullptr,                   /*tp_setattr*/
    nullptr,                   /*tp_compare*/
    nullptr,                   /*tp_repr*/
    nullptr,                   /*tp_as_number*/
    nullptr,                   /*tp_as_sequence*/
    nullptr,                   /*tp_as_mapping*/
    nullptr,                   /*tp_hash */
    nullptr,                   /*tp_call*/
    nullptr,                   /*tp_str*/
    nullptr,                   /*tp_getattro*/
    nullptr,                   /*tp_setattro*/
    nullptr,                   /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT| Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Random objects",          /* tp_doc */
    nullptr,                   /* tp_traverse */
    nullptr,                   /* tp_clear */
    nullptr,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    nullptr,                   /* tp_iter */
    nullptr,                   /* tp_iternext */
    Random_methods,            /* tp_methods */
    Random_members,            /* tp_members */
    nullptr,                   /* tp_getset */
    nullptr,                   /* tp_base */
    nullptr,                   /* tp_dict */
    nullptr,                   /* tp_descr_get */
    nullptr,                   /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Random_init,     /* tp_init */
    nullptr,                   /* tp_alloc */
    Random_new,                /* tp_new */
    nullptr,                   /* tp_free */
    nullptr,                   /* tp_is_gc */
    nullptr,                   /* tp_bases */
    nullptr,                   /* tp_mro */
    nullptr,                   /* tp_cache */
    nullptr,                   /* tp_subclasses */
    nullptr,                   /* tp_weaklist */
    nullptr,                   /* tp_del */
};
#pragma GCC diagnostic pop

static PyObject *python_redcryptofile_open(PyObject* self, PyObject* args)
{
    char *path = nullptr;
    char *omode = nullptr;
    if (!PyArg_ParseTuple(args, "ss", &path, &omode)){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }
    unsigned char derivator[DERIVATOR_LENGTH];
    get_cctx()->get_derivator(path, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    
    unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
    memcpy(tmp_derivation, derivator, DERIVATOR_LENGTH);
    memcpy(tmp_derivation + DERIVATOR_LENGTH, get_cctx()->get_crypto_key(), CRYPTO_KEY_LENGTH);
    SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated);
    memcpy(trace_key, derivated, HMAC_KEY_LENGTH);

    if (omode[0] == 'r') {
        int system_fd = open(path, O_RDONLY, 0600);
        if (system_fd == -1){
            printf("failed opening=%s\n", path);
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
        }

        auto result = crypto_file(CRYPTO_DECRYPT_TYPE, system_fd);
        auto cfr = gl_file_store_read[result.idx];
        cfr->decrypt.open(cfr->file, trace_key);

        int fd = 0;
        for (; fd < gl_nb_files ; fd++){
            if (gl_file_store[fd].idx == -1){
                break;
            }
        }
        gl_nb_files += 1;
        gl_file_store[fd] = result;
        return Py_BuildValue("i", fd);

    } else if (omode[0] == 'w') {
        unsigned i = 0;
        for (i = 0; i < sizeof(iv) ; i++){ iv[i] = i; }

        int system_fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0600);
        if (system_fd == -1){
            printf("failed opening=%s\n", path);
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
        }

        auto result = crypto_file(CRYPTO_ENCRYPT_TYPE, system_fd);
        auto cfw = gl_file_store_write[result.idx];
        cfw->encrypt.open(cfw->file, trace_key, get_cctx(), iv);

        int fd = 0;
        for (; fd < gl_nb_files ; fd++){
            if (gl_file_store[fd].idx == -1){
                break;
            }
        }
        gl_nb_files += 1;
        gl_file_store[fd] = result;
        return Py_BuildValue("i", fd);
    } else {
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    return Py_BuildValue("i", -1);
}

static PyObject *python_redcryptofile_flush(PyObject* self, PyObject* args)
{
    int fd = 0;
    if (!PyArg_ParseTuple(args, "i", &fd)){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }
    if (fd >= gl_nb_files){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }
    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type == CRYPTO_ENCRYPT_TYPE){
        auto & cfw = gl_file_store_write[cf.idx];
        result = cfw->encrypt.flush(cfw->file);
    }
    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_close(PyObject* self, PyObject* args)
{
    int fd = 0;
    unsigned char hash[MD_HASH_LENGTH<<1];
    char hash_digest[(MD_HASH_LENGTH*4)+1];

    if (!PyArg_ParseTuple(args, "i", &fd)){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    if (fd >= static_cast<int>(sizeof(gl_file_store)/sizeof(gl_file_store[0]))){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    auto & cf = gl_file_store[fd];
    if (cf.idx == -1){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    int result = 0;
    switch (cf.type){
    case CRYPTO_DECRYPT_TYPE:
    {
        auto cfr = gl_file_store_read[cf.idx];
        gl_file_store_read[cf.idx] = nullptr;
        gl_read_nb_files--;
        delete cfr;
    }
    break;
    case CRYPTO_ENCRYPT_TYPE:
    {
        auto cfw = gl_file_store_write[cf.idx];
        gl_file_store_write[cf.idx] = nullptr;
        gl_write_nb_files--;
        result = cfw->encrypt.close(cfw->file, hash, get_cctx()->get_hmac_key());
        delete cfw;
    }
    break;
    }

    cf.idx = -1;
    gl_nb_files--;

    // Crazy API: return error as integer or HASH as string... change that
    if (result){
        return Py_BuildValue("i", result);
    }

    int idx = 0;
    for (idx = 0; idx < MD_HASH_LENGTH; idx++) {
        sprintf(hash_digest + idx * 2, "%02x", hash[idx]);
    }
    for (idx = MD_HASH_LENGTH; idx < (MD_HASH_LENGTH*2); idx++) {
        sprintf(hash_digest + idx * 2, "%02x", hash[idx]);
    }
    hash_digest[MD_HASH_LENGTH*4] = 0;
    return Py_BuildValue("s", hash_digest);
}

static PyObject *python_redcryptofile_write(PyObject* self, PyObject* args)
{
    int fd;
    PyObject *python_buf;
    if (!PyArg_ParseTuple(args, "iS", &fd, &python_buf)){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    int buf_len = PyString_Size(python_buf);
    if (buf_len > 2147483647 || buf_len < 0){
        return Py_BuildValue("i", -1);
    }
    char *buf = PyString_AsString(python_buf);

    if (fd >= gl_nb_files){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type == CRYPTO_ENCRYPT_TYPE){
        auto & cfw = gl_file_store_write[cf.idx];
        result = cfw->encrypt.write(cfw->file, buf, buf_len);
    }

    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_read(PyObject* self, PyObject* args)
{
    int fd;
    int buf_len;

    if (!PyArg_ParseTuple(args, "ii", &fd, &buf_len))
        return nullptr;
    if (buf_len > 2147483647 || buf_len <= 0){
        return Py_BuildValue("i", -1);
    }

    if (fd >= gl_nb_files){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }

    std::unique_ptr<char[]> buf(new char[buf_len]);

    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type ==  CRYPTO_DECRYPT_TYPE) {
        auto & cfr = gl_file_store_read[cf.idx];
        result = cfr->decrypt.read(cfr->file, buf.get(), buf_len);
    }
    if (result < 0){
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
        Py_RETURN_NONE;
#pragma GCC diagnostic pop
    }
    return PyString_FromStringAndSize(buf.get(), result);
}

static PyMethodDef redcryptoFileMethods[] = {
    {"open", python_redcryptofile_open, METH_VARARGS, ""},
    {"flush", python_redcryptofile_flush, METH_VARARGS, ""},
    {"close", python_redcryptofile_close, METH_VARARGS, ""},
    {"write", python_redcryptofile_write, METH_VARARGS, ""},
    {"read", python_redcryptofile_read, METH_VARARGS, ""},
    {nullptr, nullptr, 0, nullptr}
};

PyMODINIT_FUNC 
initredcryptofile(void)
{
    PyObject* module = Py_InitModule3("redcryptofile", redcryptoFileMethods,
                           "redcryptofile module");


    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };

    uint8_t trace_key[32] = {};
    CryptoContext * cctx = get_cctx();
    
    unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
    memcpy(tmp_derivation, HASH_DERIVATOR, DERIVATOR_LENGTH);
    memcpy(tmp_derivation + DERIVATOR_LENGTH, cctx->get_crypto_key(), CRYPTO_KEY_LENGTH);
    SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated);
    memcpy(trace_key, derivated, HMAC_KEY_LENGTH);
    
    get_ini()->set<cfg::crypto::key1>(trace_key);
    OpenSSL_add_all_digests();

    size_t idx = 0;
    for (; idx < sizeof(gl_file_store)/sizeof(gl_file_store[0]);idx++)
    {
        gl_file_store[idx].idx = -1;
    }
    size_t idxr = 0;
    for (; idxr < sizeof(gl_file_store_read)/sizeof(gl_file_store_read[0]);idxr++)
    {
        gl_file_store_read[idxr] = nullptr;
    }
    size_t idxw = 0;
    for (; idxw < sizeof(gl_file_store_write)/sizeof(gl_file_store_write[0]);idxw++)
    {
        gl_file_store_write[idxw] = nullptr;
    }

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wuseless-cast"
    redcryptofile_NoddyType.pto.tp_new = PyType_GenericNew;
    if (PyType_Ready(&redcryptofile_NoddyType.pto) == 0){
        Py_INCREF(&redcryptofile_NoddyType.po);
        PyModule_AddObject(module, "Noddy", &redcryptofile_NoddyType.po);
    }

//    PyTyRandom.pto.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyTyRandom.pto) == 0){
        Py_INCREF(&PyTyRandom.po);
        PyModule_AddObject(module, "Random", &PyTyRandom.po);
    }
#pragma GCC diagnostic pop
}

}
