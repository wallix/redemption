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
 *   Copyright (C) Wallix 2010-2017
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include "transport/transport.hpp"
#include "utils/genrandom.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/iter.hpp"
#include "capture/cryptofile.hpp"

struct ocrypto
{
    struct Result {
        const_bytes_array buf;
        std::size_t consumed;
    };

private:
    EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
    SslHMAC_Sha256_Delayed hm;              // full hash context
    SslHMAC_Sha256_Delayed hm4k;             // quick hash context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       file_size;               // the current file size
    uint8_t header_buf[40];
    uint8_t result_buffer[32768] = {};
    char           buf[CRYPTO_BUFFER_SIZE]; //

    CryptoContext & cctx;
    Random & rnd;
    bool encryption;
    bool checksum;

    /* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
     * Return 0 on success, negative value on error
     */
    void xaes_encrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
    {
        int safe_size = *dst_sz;
        int remaining_size = 0;

        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_EncryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
            throw Error(ERR_SSL_CALL_FAILED);
        }
        if (EVP_EncryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1) {
            throw Error(ERR_SSL_CALL_FAILED);
        }
        if (EVP_EncryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
            throw Error(ERR_SSL_CALL_FAILED);
        }
        *dst_sz = safe_size + remaining_size;
    }

    /* Flush procedure (compression, encryption)
     * Return 0 on success, negatif on error
     */
    void flush(uint8_t * buffer, size_t buflen, size_t & towrite)
    {
        // No data to flush
        if (!this->pos) {
            return;
        }
        // Compress
        // TODO: check this
        char compressed_buf[65536];
        size_t compressed_buf_sz = ::snappy_max_compressed_length(this->pos);
        snappy_status status = snappy_compress(this->buf, this->pos, compressed_buf, &compressed_buf_sz);

        switch (status)
        {
            case SNAPPY_OK:
                break;
            case SNAPPY_INVALID_INPUT:
                throw Error(ERR_CRYPTO_SNAPPY_COMPRESSION_INVALID_INPUT);
            case SNAPPY_BUFFER_TOO_SMALL:
                throw Error(ERR_CRYPTO_SNAPPY_BUFFER_TOO_SMALL);
        }

        // Encrypt
        unsigned char ciphered_buf[4 + 65536];
        uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
        this->xaes_encrypt(reinterpret_cast<unsigned char*>(compressed_buf),
                           compressed_buf_sz,
                           ciphered_buf + 4, &ciphered_buf_sz);

        ciphered_buf[0] = ciphered_buf_sz & 0xFF;
        ciphered_buf[1] = (ciphered_buf_sz >> 8) & 0xFF;
        ciphered_buf[2] = (ciphered_buf_sz >> 16) & 0xFF;
        ciphered_buf[3] = (ciphered_buf_sz >> 24) & 0xFF;

        ciphered_buf_sz += 4;
        if (ciphered_buf_sz > buflen){
            throw Error(ERR_CRYPTO_BUFFER_TOO_SMALL);
        }
        ::memcpy(buffer, ciphered_buf, ciphered_buf_sz);
        towrite += ciphered_buf_sz;

        this->update_hmac(&ciphered_buf[0], ciphered_buf_sz);

        // Reset buffer
        this->pos = 0;
    }

    void update_hmac(uint8_t const * buf, size_t len)
    {
        if (this->checksum){
            this->hm.update(buf, len);
            if (this->file_size < 4096) {
                size_t remaining_size = 4096 - this->file_size;
                this->hm4k.update(buf, std::min(remaining_size, len));
            }
            this->file_size += len;
        }
    }

public:
    ocrypto(bool encryption, bool checksum, CryptoContext & cctx, Random & rnd)
        : cctx(cctx)
        , rnd(rnd)
        , encryption(encryption)
        , checksum(checksum)
    {
    }

    ~ocrypto() = default;

    Result open(const uint8_t * derivator, size_t derivator_len)
    {
        this->file_size = 0;
        this->pos = 0;
        if (this->checksum) {
            this->hm.init(this->cctx.get_hmac_key(), CRYPTO_KEY_LENGTH);
            this->hm4k.init(this->cctx.get_hmac_key(), CRYPTO_KEY_LENGTH);
        }

        if (this->encryption) {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            this->cctx.get_derived_key(trace_key, derivator, derivator_len);
            unsigned char iv[32];
            this->rnd.random(iv, 32);

            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));
            this->pos = 0;
            this->raw_size = 0;

            const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
            const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
            const int          nrounds = 5;
            unsigned char      key[32];
            const int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), reinterpret_cast<const unsigned char *>(salt),
                                           trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
            if (i != 32) {
                throw Error(ERR_SSL_CALL_FAILED);
            }

            ::EVP_CIPHER_CTX_init(&this->ectx);
            if (::EVP_EncryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                throw Error(ERR_SSL_CALL_FAILED);
            }

            // update context with previously written data
            this->header_buf[0] = 'W';
            this->header_buf[1] = 'C';
            this->header_buf[2] = 'F';
            this->header_buf[3] = 'M';
            this->header_buf[4] = WABCRYPTOFILE_VERSION & 0xFF;
            this->header_buf[5] = (WABCRYPTOFILE_VERSION >> 8) & 0xFF;
            this->header_buf[6] = (WABCRYPTOFILE_VERSION >> 16) & 0xFF;
            this->header_buf[7] = (WABCRYPTOFILE_VERSION >> 24) & 0xFF;
            ::memcpy(this->header_buf + 8, iv, 32);
            this->update_hmac(&this->header_buf[0], 40);
            return Result{{this->header_buf, 40u}, 0u};
        }
        else {
            return Result{{this->header_buf, 0u}, 0u};
        }
    }

    ocrypto::Result close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        size_t towrite = 0;
        if (this->encryption) {
            size_t buflen = sizeof(this->result_buffer);
            this->flush(this->result_buffer, buflen, towrite);

            unsigned char tmp_buf[8] = {
                'M','F','C','W',
                uint8_t(this->raw_size & 0xFF),
                uint8_t((this->raw_size >> 8) & 0xFF),
                uint8_t((this->raw_size >> 16) & 0xFF),
                uint8_t((this->raw_size >> 24) & 0xFF),
            };

            if (towrite + 8 > buflen){
                throw Error(ERR_CRYPTO_BUFFER_TOO_SMALL);
            }
            ::memcpy(this->result_buffer + towrite, tmp_buf, 8);
            towrite += 8;

            this->update_hmac(tmp_buf, 8);
        }

        if (this->checksum) {
            this->hm.final(fhash);
            this->hm4k.final(qhash);

        }
        return Result{{this->result_buffer, towrite}, 0u};

    }

    ocrypto::Result write(const uint8_t * data, size_t len)
    {
        if (!this->encryption) {
            this->update_hmac(data, len);
            return Result{{data, len}, len};
        }

        size_t buflen = sizeof(this->result_buffer);
        size_t towrite = 0;
        unsigned int remaining_size = len;
        while (remaining_size > 0) {
            // Check how much we can append into buffer
            unsigned int available_size = std::min(unsigned(CRYPTO_BUFFER_SIZE - this->pos), remaining_size);
            // Append and update pos pointer
            ::memcpy(this->buf + this->pos, data + (len - remaining_size), available_size);
            this->pos += available_size;
            // If buffer is full, flush it to disk
            if (this->pos == CRYPTO_BUFFER_SIZE) {
                size_t tmp_towrite = 0;
                this->flush(this->result_buffer + towrite, buflen - towrite, tmp_towrite);
                towrite += tmp_towrite;
            }
            remaining_size -= available_size;
        }
        // Update raw size counter
        this->raw_size += len;
        return {{this->result_buffer, towrite}, len};
    }
};


class OutCryptoTransport : public Transport
{
    bool with_checksum;
    ocrypto encrypter;
    char tmpname[2048];
    char finalname[2048];
    int fd;
public:
    explicit OutCryptoTransport(bool with_encryption,
                                bool with_checksum,
                                CryptoContext & cctx,
                                Random & rnd) noexcept
        : with_checksum(with_checksum)
        , encrypter(with_encryption, with_checksum, cctx, rnd)
        , fd(-1)
    {
//        LOG(LOG_INFO, "OutCryptoTransport::OutCryptoTransport()");
        this->tmpname[0] = 0;
        this->finalname[0] = 0;
//        LOG(LOG_INFO, "OutCryptoTransport::OutCryptoTransport() done");
    }

    const char * get_tmp(){
        return &this->tmpname[0];
    }

    ~OutCryptoTransport() {
//        LOG(LOG_INFO, "OutCryptoTransport::~OutCryptoTransport()");
        if (this->fd == -1){
            return;
        }
        try {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
            uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
            this->close(qhash, fhash);
            if (this->with_checksum){
                char mes[MD_HASH::DIGEST_LENGTH*4+1+128]{};
                char * p = mes;
                p+= sprintf(mes, "Encrypted transport implicitely closed, hash checksums dropped :");
                auto hexdump = [&p](uint8_t (&hash)[MD_HASH::DIGEST_LENGTH]) {
                    *p++ = ' ';                // 1 octet
                    for (unsigned c : hash) {
                        sprintf(p, "%02x", c); // 64 octets (hash)
                        p += 2;
                    }
                };
                hexdump(qhash);
                hexdump(fhash);
                *p++ = 0;
                LOG(LOG_INFO, "%s", mes);
            }
        }
        catch (Error const & e){
            LOG(LOG_INFO, "Exception raised in ~OutCryptoTransport %d", e.id);
        }
//        LOG(LOG_INFO, "OutCryptoTransport::~OutCryptoTransport() done");
    }

    // TODO: CGR: I want to remove that from Transport API
    bool disconnect() override {
        return 0;
    }

    bool is_open()
    {
        return this->fd != -1;
    }

    void open(const char * finalname, int groupid)
    {
//        LOG(LOG_INFO, "OutCryptoTransport::open()");

        // This should avoid double open, we do not want that
        if (this->fd != -1){
            LOG(LOG_ERR, "OutCryptoTransport::open (double open error) %s", finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        // also ensure pathes are not to long, we will copy them in the object
        if (strlen(finalname) >= 2047-15){
            LOG(LOG_ERR, "OutCryptoTransport::open finalname oversize");
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        snprintf(this->tmpname, sizeof(this->tmpname), "%sred-XXXXXX.tmp", finalname);
        this->fd = ::mkostemps(this->tmpname, 4, O_WRONLY | O_CREAT);
        if (this->fd == -1){
            LOG(LOG_ERR, "OutCryptoTransport::open : open failed (%s -> %s)", this->tmpname, finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        if (chmod(this->tmpname, groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                , this->tmpname
                , groupid ? "u+r, g+r" : "u+r"
                , strerror(errno), errno);
            LOG(LOG_INFO, "OutCryptoTransport::open : chmod failed (%s -> %s)", this->tmpname, finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        strcpy(this->finalname, finalname);
        size_t derivator_len = 0;
        const uint8_t * derivator = reinterpret_cast<const uint8_t *>(basename_len(finalname, derivator_len));

        ocrypto::Result res = this->encrypter.open(derivator, derivator_len);
        this->raw_write(res.buf.data(), res.buf.size());
//        LOG(LOG_INFO, "OutCryptoTransport::open() done");
    }

    void close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
//        LOG(LOG_INFO, "OutCryptoTransport::close()");
        // This should avoid double closes, we do not want that
        if (this->fd == -1){
            LOG(LOG_ERR, "OutCryptoTransport::close error (double close error)");
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        const ocrypto::Result res = this->encrypter.close(qhash, fhash);
        this->raw_write(res.buf.data(), res.buf.size());
        if (this->tmpname[0] != 0){
            if (::rename(this->tmpname, this->finalname) < 0) {
                LOG(LOG_ERR, "OutCryptoTransport::close Renaming file \"%s\" -> \"%s\" failed, errno=%u : %s\n"
                   , this->tmpname, this->finalname, errno, strerror(errno));
                ::close(this->fd);
                this->fd = -1;
                throw Error(ERR_TRANSPORT_WRITE_FAILED);
            }
            this->tmpname[0] = 0;
        }
        ::close(this->fd);
        this->fd = -1;
//        LOG(LOG_INFO, "OutCryptoTransport::close() done");
    }

private:
    void do_send(const uint8_t * data, size_t len) override
    {
        if (this->fd == -1){
            LOG(LOG_ERR, "OutCryptoTransport::do_send failed: file not opened (%s->%s)", this->tmpname, this->finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        const ocrypto::Result res = this->encrypter.write(data, len);
        this->raw_write(res.buf.data(), res.buf.size());
    }

    void raw_write(const uint8_t * data, size_t len)
    {
        size_t total_sent = 0;
        while (len > total_sent) {
            ssize_t ret = ::write(this->fd, &data[total_sent], len-total_sent);
            if (ret <= 0){
                if (errno == EINTR){
                    continue;
                }
                throw Error(ERR_TRANSPORT_WRITE_FAILED);
            }
            total_sent += ret;
        }
    }

};
