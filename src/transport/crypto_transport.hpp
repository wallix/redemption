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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan, Clément Moroldo
 */


#pragma once

#include "capture/cryptofile.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/transport.hpp"
#include "utils/fileutils.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "utils/parse.hpp"
#include "utils/sugar/byte.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <memory>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


class InCryptoTransport : public Transport //, public PartialIO
{
public:
    enum class EncryptionMode { Auto, Encrypted, NotEncrypted };

private:
    int fd;
    bool eof;
    size_t file_len;
    size_t current_len;

    CryptoContext & cctx;
    char clear_data[CRYPTO_BUFFER_SIZE];  // contains either raw data from unencrypted file
                                          // or already decrypted/decompressed data
    uint32_t clear_pos;                   // current position in clear_data buf
    uint32_t raw_size;                    // the unciphered/uncompressed data available in buffer

    DecryptContext ectx;
    // TODO: state to remove ? Seems to duplicate eof flag
    uint32_t state;                       // enum crypto_file_state
    unsigned int   MAX_CIPHERED_SIZE;     // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    EncryptionMode encryption_mode;
    bool encrypted;

public:
    explicit InCryptoTransport(CryptoContext & cctx, EncryptionMode encryption_mode) noexcept
    : fd(-1)
    , eof(true)
    , file_len(0)
    , current_len(0)
    , cctx(cctx)
    , clear_data{}
    , clear_pos(0)
    , raw_size(0)
    , state(0)
    , MAX_CIPHERED_SIZE(0)
    , encryption_mode(encryption_mode)
    , encrypted(false)
    {
    }

    ~InCryptoTransport()
    {
        if (this->is_open()){
            ::close(this->fd);
        }
    }

    bool is_encrypted() const
    {
        return this->encrypted;
    }

    bool is_open() const
    {
        return this->fd != -1;
    }

    struct HASH {
        uint8_t hash[MD_HASH::DIGEST_LENGTH];
    };

    const HASH qhash(const char * pathname)
    {
        SslHMAC_Sha256 hm4k(this->cctx.get_hmac_key(), HMAC_KEY_LENGTH);

        {
            int fd = ::open(pathname, O_RDONLY);
            if (fd < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            unique_fd auto_close(fd);

            constexpr std::size_t buffer_size = 4096;
            uint8_t buffer[buffer_size];
            size_t total_length = 0;
            do {
                size_t const remaining_size = buffer_size - total_length;
                ssize_t res = ::read(fd, buffer, remaining_size);
                if (res == 0) { break; }
                if (res < 0 && errno == EINTR) { continue; }
                if (res < 0) { throw Error(ERR_TRANSPORT_READ_FAILED, errno); }
                hm4k.update(buffer, res);
                total_length += res;
            } while (total_length != buffer_size);
        }

        HASH qhash;
        hm4k.final(qhash.hash);
        return qhash;
    }

    const HASH fhash(const char * pathname)
    {
        SslHMAC_Sha256 hm(this->cctx.get_hmac_key(), HMAC_KEY_LENGTH);

        {
            int fd = ::open(pathname, O_RDONLY);
            if (fd < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            unique_fd auto_close(fd);

            uint8_t buffer[4096];
            do {
                ssize_t res = ::read(fd, &buffer[0], sizeof(buffer));
                if (res == 0) { break; }
                if (res < 0 && errno == EINTR) { continue; }
                if (res < 0) { throw Error(ERR_TRANSPORT_READ_FAILED, errno); }
                hm.update(buffer, res);
            } while (1);
        }

        HASH fhash;
        hm.final(fhash.hash);
        return fhash;
    }

//    void hash(const char * pathname)
//    {
//        this->open(pathname);
//    }

    size_t partial_read(uint8_t * buffer, size_t len) __attribute__ ((warn_unused_result))
    {
        return this->do_partial_read(buffer, len);
    }

    size_t partial_read(char * buffer, size_t len) __attribute__ ((warn_unused_result))
    {
        return this->do_partial_read(reinterpret_cast<uint8_t*>(buffer), len);
    }

    void open(const char * const pathname, const_byte_array derivator)
    {
        if (this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        this->fd = ::open(pathname, O_RDONLY);
        if (this->fd < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        this->eof = false;
        this->current_len = 0;

        ::memset(this->clear_data, 0, sizeof(this->clear_data));

        this->clear_pos = 0;
        this->raw_size = 0;
        this->state = 0;

        const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
        this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        // todo: we could read in clear_data, that would avoid some copying
        uint8_t data[40];
        size_t avail = 0;
        while (avail != 40) {
            ssize_t ret = ::read(this->fd, &data[avail], 40-avail);
            if (ret < 0 && errno == EINTR){
                continue;
            }
            if (ret <= 0){
                // Either read error or EOF: in both cases we are in trouble
                if (ret == 0) {
                    // if we have less than magic followed by encryption header
                    // then our file is necessarilly not encrypted.
                    this->encrypted = false;
                    // encryption requested but no encryption
                    if (this->encryption_mode == EncryptionMode::Encrypted){
                        this->close();
                        throw Error(ERR_TRANSPORT_READ_FAILED);
                    }
                    // copy what we have, it's not encrypted
                    this->raw_size = avail;
                    this->clear_pos = 0;
                    ::memcpy(this->clear_data, data, avail);
                    this->file_len = this->get_file_len(pathname);
                    return;
                }
                this->close();
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            avail += ret;
        }

        if (this->encryption_mode == EncryptionMode::NotEncrypted){
            // copy what we have, it's not encrypted, don't care about magic
            this->raw_size = 40;
            this->clear_pos = 0;
            ::memcpy(this->clear_data, data, 40);
            this->file_len = this->get_file_len(pathname);
            return;
        }

        // Encrypted/Compressed file header (40 bytes)
        // -------------------------------------------
        // MAGIC: 4 bytes
        // 0x57 0x43 0x46 0x4D (WCFM)
        // VERSION: 4 bytes
        // 0x01 0x00 0x00 0x00
        // IV: 32 bytes
        // (random)

        Parse p(data);
        {
            const int magic = p.in_uint32_le();
            if (magic != WABCRYPTOFILE_MAGIC) {
                this->encrypted = false;
                // encryption requested but no encryption
                if (this->encryption_mode == EncryptionMode::Encrypted){
                    this->close();
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }
                // Auto: rely on magic. Copy what we have, it's not encrypted
                this->raw_size = 40;
                this->clear_pos = 0;
                ::memcpy(this->clear_data, data, 40);
                this->file_len = this->get_file_len(pathname);
                return;
            }
        }
        this->encrypted = true;
        // check version
        {
            const uint32_t version = p.in_uint32_le();
            if (version > WABCRYPTOFILE_VERSION) {
                // Unsupported version
                this->close();
                LOG(LOG_INFO, "unsupported_version");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
        }

        // Read File trailer, check for magic trailer and size
        off64_t off_here = lseek64(this->fd, 0, SEEK_CUR);
        lseek64(this->fd, -8, SEEK_END);
        uint8_t trail[8] = {};
        this->raw_read(trail, 8);
        Parse pt1(&trail[0]);
        if (pt1.in_uint32_be() != WABCRYPTOFILE_MAGIC){
            // truncated file
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        Parse pt2(&trail[4]);
        this->file_len = pt2.in_uint32_le();
        lseek64(this->fd, off_here, SEEK_SET);
        // the fd is back where we was

        // TODO: replace p.with some array view of 32 bytes ?
        const uint8_t * const iv = p.p;
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        cctx.get_derived_key(trace_key, derivator);

        if (!this->ectx.init(trace_key, iv)) {
            this->close();
            throw Error(ERR_SSL_CALL_FAILED);
        }
    }

    // derivator implicitly basename(pathname)
    void open(const char * const pathname)
    {
        size_t base_len = 0;
        const char * base = basename_len(pathname, base_len);
        this->open(pathname, {base, base_len});
    }

    void close()
    {
        if (!this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        ::close(this->fd);
        this->fd = -1;
        this->eof = true;
    }

    bool is_eof() {
        return this->eof;
    }

private:
    // this perform atomic read, partial read will result in exception
    void raw_read(uint8_t buffer[], const size_t len)
    {
        size_t rlen = len;
        while (rlen) {
            ssize_t ret = ::read(this->fd, &buffer[len - rlen], rlen);
            if (ret <= 0){ // unexpected EOF or error
                if (ret != 0 && errno == EINTR){
                    continue;
                }
                this->close();
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            rlen -= ret;
        }
    }

    size_t do_partial_read(uint8_t * buffer, size_t len)
    {
        if (this->eof){
            return 0;
        }
        if (this->encrypted){
            if (this->state & CF_EOF) {
                return 0;
            }

            // If we do not have any clear data available read some
            if (!this->raw_size) {

                // Read a full ciphered block at once
                uint8_t hlen[4] = {};
                this->raw_read(hlen, 4);

                Parse p(hlen);
                uint32_t enc_len = p.in_uint32_le();
                if (enc_len == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                    this->state = CF_EOF;
                    this->clear_pos = 0;
                    this->raw_size = 0;
                    this->eof = true;
                    return 0;
                }
                if (enc_len > this->MAX_CIPHERED_SIZE) { // integrity error
                    this->close();
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }

                // PERF allocation in loop
                std::unique_ptr<uint8_t []> enc_buf(new uint8_t[enc_len]);
                this->raw_read(&enc_buf[0], enc_len);

                // PERF allocation in loop
                std::unique_ptr<uint8_t []> pack_buf(new uint8_t[enc_len + AES_BLOCK_SIZE]);
                size_t pack_buf_size = this->ectx.decrypt(&enc_buf[0], enc_len, &pack_buf[0]);

                size_t chunk_size = CRYPTO_BUFFER_SIZE;
                const snappy_status status = snappy_uncompress(
                        reinterpret_cast<char *>(&pack_buf[0]),
                        pack_buf_size, this->clear_data, &chunk_size);

                switch (status)
                {
                    case SNAPPY_OK:
                        break;
                    case SNAPPY_INVALID_INPUT:
                    case SNAPPY_BUFFER_TOO_SMALL:
                    default:
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }

                this->clear_pos = 0;
                // When reading, raw_size represent the current chunk size
                this->raw_size = chunk_size;

            } // raw_size

            // Check how much we can copy
            std::size_t const copiable_size = std::min(
                len, static_cast<std::size_t>(this->raw_size - this->clear_pos)
            );
            // Copy buffer to caller
            ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], copiable_size);
            this->clear_pos += copiable_size;
            this->current_len += copiable_size;
            if (this->file_len <= this->current_len) {
                this->eof = true;
            }
            // Check if we reach the end
            if (this->raw_size == this->clear_pos) {
                this->raw_size = 0;
            }
            return copiable_size;
        }
        else {
            if (this->raw_size - this->clear_pos > len){
                ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], len);
                this->clear_pos += len;
                this->current_len += len;
                if (this->file_len <= this->current_len) {
                    this->eof = true;
                }
                return len;
            }
            size_t remaining_len = len;
            if (this->raw_size - this->clear_pos > 0){
                ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], this->raw_size - this->clear_pos);
                remaining_len -= this->raw_size - this->clear_pos;
                this->raw_size = 0;
                this->clear_pos = 0;
            }
            while(remaining_len){
                ssize_t const res = ::read(this->fd, &buffer[len - remaining_len], remaining_len);
                if (res <= 0){
                    if (res == 0) {
                        this->eof = true;
                        break;
                    }
                    if (errno == EINTR){
                        continue;
                    }
                    throw Error(ERR_TRANSPORT_READ_FAILED, res);
                }
                remaining_len -= res;
            };

            this->current_len += len;
            if (this->file_len <= this->current_len) {
                this->eof = true;
            }
            return len - remaining_len;
        }
    }

    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        size_t res;
        size_t total = 0;
        size_t remaining_len = len;
        while ((res = do_partial_read(buffer, remaining_len)) && res != remaining_len) {
            total += res;
            buffer += res;
            remaining_len -= res;
        }
        total += res;
        if (res != 0 && total != len) {
            throw Error(ERR_TRANSPORT_READ_FAILED, 0);
        }
        return total == len ? Read::Ok : Read::Eof;
    }

    std::size_t get_file_len(char const * pathname)
    {
        struct stat sb;
        if (int err = ::stat(pathname, &sb)) {
            LOG(LOG_ERR, "crypto: stat error %d", err);
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        return sb.st_size;
    }
};


struct ocrypto : noncopyable
{
    struct Result {
        const_byte_array buf;
        std::size_t consumed;
    };

private:
    EncryptContext ectx;
    SslHMAC_Sha256_Delayed hm;              // full hash context
    SslHMAC_Sha256_Delayed hm4k;             // quick hash context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       file_size;               // the current file size
    uint8_t header_buf[40];
    uint8_t result_buffer[65536] = {};
    char           buf[CRYPTO_BUFFER_SIZE]; //

    CryptoContext & cctx;
    Random & rnd;
    bool encryption;
    bool checksum;

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
        ciphered_buf_sz = this->ectx.encrypt(
            reinterpret_cast<unsigned char*>(compressed_buf), compressed_buf_sz,
            ciphered_buf + 4, ciphered_buf_sz
        );

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

    Result open(const_byte_array derivator)
    {
        this->file_size = 0;
        this->pos = 0;
        if (this->checksum) {
            this->hm.init(this->cctx.get_hmac_key(), HMAC_KEY_LENGTH);
            this->hm4k.init(this->cctx.get_hmac_key(), HMAC_KEY_LENGTH);
        }

        if (this->encryption) {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            this->cctx.get_derived_key(trace_key, derivator);
            unsigned char iv[32];
            this->rnd.random(iv, 32);

            ::memset(this->buf, 0, sizeof(this->buf));
            this->pos = 0;
            this->raw_size = 0;

            if (!this->ectx.init(trace_key, iv)) {
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

            // this->ectx.deinit();

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
        if (len > buflen - 1000) { // 1000: magic enough for header, actual value is smaller
            len = buflen;
        }
        // Check how much we can append into buffer
        size_t available_size = CRYPTO_BUFFER_SIZE - this->pos;
        if (available_size > len) {
            available_size = len;
        }
        // Append and update pos pointer
        ::memcpy(this->buf + this->pos, &data[0], available_size);
        this->pos += available_size;
        // If buffer is full, flush it to disk
        size_t towrite = 0;
        if (this->pos == CRYPTO_BUFFER_SIZE) {
            this->flush(this->result_buffer, buflen, towrite);
        }
        // Update raw size counter
        this->raw_size += available_size;
        return {{this->result_buffer, towrite}, available_size};
    }
};

struct OutBufferHashLineCtx
{
    using ull = unsigned long long;
    using ll = long long;

    // 8Ko for a filename with expanded slash should be enough
    // or we will truncate filename at buffersize
    static const std::size_t tmp_filename_size = 8192;
    char mes[
        tmp_filename_size +
        (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
        (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
        (MD_HASH::DIGEST_LENGTH*2 + 1) * 2 + 1 +
        2
    ];
    std::size_t len = 0;

    void write_filename(char const * filename)
    {
        for (size_t i = 0; (filename[i]) && (this->len < tmp_filename_size-2) ; i++){
            switch (filename[i]){
            case '\\':
            case ' ':
                this->mes[this->len++] = '\\';
                REDEMPTION_CXX_FALLTHROUGH;
            default:
                this->mes[this->len++] = filename[i];
            break;
            }
        }
    }

    void write_stat(struct stat & stat)
    {
        this->len += std::sprintf(
            this->mes + this->len,
            " %lld %llu %lld %lld %llu %lld %lld %lld",
            ll(stat.st_size),
            ull(stat.st_mode),
            ll(stat.st_uid),
            ll(stat.st_gid),
            ull(stat.st_dev),
            ll(stat.st_ino),
            ll(stat.st_mtim.tv_sec),
            ll(stat.st_ctim.tv_sec)
        );
    }

    void write_start_and_stop(time_t start, time_t stop)
    {
        this->len += std::sprintf(
            this->mes + this->len,
            " %lld %lld",
            ll(start),
            ll(stop+1)
        );
    }

    void write_hashs(
        uint8_t const (&qhash)[MD_HASH::DIGEST_LENGTH],
        uint8_t const (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        char * p = this->mes + this->len;

        auto hexdump = [&p](uint8_t const (&hash)[MD_HASH::DIGEST_LENGTH]) {
            *p++ = ' ';                // 1 octet
            for (unsigned c : hash) {
                sprintf(p, "%02x", c); // 64 octets (hash)
                p += 2;
            }
        };
        hexdump(qhash);
        hexdump(fhash);

        this->len = p - this->mes;
    }

    void write_newline()
    {
        this->mes[this->len++] = '\n';
    }
};

class OutCryptoTransport : public Transport
{
    ocrypto encrypter;
    OutFileTransport out_file;
    char tmpname[2048];
    char finalname[2048];
    std::string hash_filename;
    bool with_encryption;
    bool with_checksum;
    CryptoContext & cctx;
    Random & rnd;
    Fstat & fstat;
    int groupid;
    std::vector<uint8_t> derivator;

public:
    explicit OutCryptoTransport(
        bool with_encryption, bool with_checksum,
        CryptoContext & cctx, Random & rnd, Fstat & fstat,
        ReportError report_error = ReportError()
    ) noexcept
    : encrypter(with_encryption, with_checksum, cctx, rnd)
    , out_file(invalid_fd(), std::move(report_error))
    , with_encryption(with_encryption)
    , with_checksum(with_checksum)
    , cctx(cctx)
    , rnd(rnd)
    , fstat(fstat)
    {
        this->tmpname[0] = 0;
        this->finalname[0] = 0;
    }

    const char * get_tmp() const
    {
        return &this->tmpname[0];
    }

    ReportError & get_report_error()
    {
        return this->out_file.get_report_error();
    }

    ~OutCryptoTransport()
    {
        if (not this->is_open()) {
            return;
        }
        try {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
            uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
            this->close(qhash, fhash);
            if (this->with_checksum){
                char mes[MD_HASH::DIGEST_LENGTH*4+1+128]{};
                char * p = mes;
                p+= sprintf(mes, "Encrypted transport implicitly closed, hash checksums dropped :");
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
    }

    // TODO: CGR: I want to remove that from Transport API
    bool disconnect() override
    {
        return 0;
    }

    bool is_open() const
    {
        return this->out_file.is_open();
    }

    void open(const char * const finalname, const char * const hash_filename, int groupid, const_byte_array derivator)
    {
        // This should avoid double open, we do not want that
        if (this->is_open()){
            LOG(LOG_ERR, "OutCryptoTransport::open (double open error) %s", finalname);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        // also ensure pathes are not to long, we will copy them in the object
        if (strlen(finalname) >= 2047-15){
            LOG(LOG_ERR, "OutCryptoTransport::open finalname oversize");
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        // basic compare filename
        if (0 == strcmp(finalname, hash_filename)){
            LOG(LOG_ERR, "OutCryptoTransport::open finalname and hash_filename are same");
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        snprintf(this->tmpname, sizeof(this->tmpname), "%sred-XXXXXX.tmp", finalname);
        this->out_file.open(unique_fd(::mkostemps(this->tmpname, 4, O_WRONLY | O_CREAT)));
        if (not this->is_open()){
            int const err = errno;
            LOG(LOG_ERR, "OutCryptoTransport::open : open failed (%s -> %s)", this->tmpname, finalname);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
        }

        if (chmod(this->tmpname, groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
            int const err = errno;
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                , this->tmpname
                , groupid ? "u+r, g+r" : "u+r"
                , strerror(err), err);
            LOG(LOG_INFO, "OutCryptoTransport::open : chmod failed (%s -> %s)", this->tmpname, finalname);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
        }

        strcpy(this->finalname, finalname);
        this->hash_filename = hash_filename;
        this->derivator.assign(derivator.begin(), derivator.end());
        this->groupid = groupid;

        ocrypto::Result res = this->encrypter.open(derivator);
        this->out_file.send(res.buf.data(), res.buf.size());
    }

    // derivator implicitly basename(finalname)
    void open(const char * finalname, const char * const hash_filename, int groupid)
    {
        size_t base_len = 0;
        const char * base = basename_len(finalname, base_len);
        this->open(finalname, hash_filename, groupid, {base, base_len});
    }

    void close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        // This should avoid double closes, we do not want that
        if (!this->out_file.is_open()){
            LOG(LOG_ERR, "OutCryptoTransport::close error (double close error)");
            throw Error(ERR_TRANSPORT_CLOSED);
        }
        const ocrypto::Result res = this->encrypter.close(qhash, fhash);
        this->out_file.send(res.buf.data(), res.buf.size());
        if (this->tmpname[0] != 0){
            if (::rename(this->tmpname, this->finalname) < 0) {
                int const err = errno;
                LOG(LOG_ERR, "OutCryptoTransport::close Renaming file \"%s\" -> \"%s\" failed, errno=%u : %s\n"
                   , this->tmpname, this->finalname, err, strerror(err));
                this->out_file.close();
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
            this->tmpname[0] = 0;
        }
        this->out_file.close();
        this->create_hash_file(qhash, fhash);
    }

    void create_hash_file(
        uint8_t const (&qhash)[MD_HASH::DIGEST_LENGTH],
        uint8_t const (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        ocrypto hash_encrypter(this->with_encryption, this->with_checksum, this->cctx, this->rnd);
        OutFileTransport hash_out_file(unique_fd(::open(
            this->hash_filename.c_str(),
            O_WRONLY | O_CREAT,
            this->groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR)));
        if (!hash_out_file.is_open()){
            int const err = errno;
            LOG(LOG_ERR, "OutCryptoTransport::open: open failed hash file %s: %s", this->hash_filename, strerror(err));
            throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
        }

        struct stat stat;
        int err = this->fstat.stat(this->finalname, stat);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                this->hash_filename, err);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        // open
        {
            const ocrypto::Result res = hash_encrypter.open(this->derivator);
            hash_out_file.send(res.buf.data(), res.buf.size());
        }

        constexpr char header[] = "v2\n\n\n";
        this->send_data(cbyte_ptr(header), sizeof(header)-1, hash_encrypter, hash_out_file);

        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};

        canonical_path(
            this->finalname,
            path, sizeof(path),
            basename, sizeof(basename),
            extension, sizeof(extension)
        );

        OutBufferHashLineCtx buf;
        buf.write_filename(basename);
        buf.write_filename(extension);
        buf.write_stat(stat);
        if (this->with_checksum) {
            buf.write_hashs(qhash, fhash);
        }
        buf.write_newline();

        this->send_data(cbyte_ptr(buf.mes), buf.len, hash_encrypter, hash_out_file);

        // close
        {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH];
            uint8_t fhash[MD_HASH::DIGEST_LENGTH];
            const ocrypto::Result res = hash_encrypter.close(qhash, fhash);
            hash_out_file.send(res.buf.data(), res.buf.size());
            hash_out_file.close();
        }
    }

    void do_send(const uint8_t * data, size_t len) override
    {
        if (not this->out_file.is_open()){
            LOG(LOG_ERR, "OutCryptoTransport::do_send failed: file not opened (%s->%s)", this->tmpname, this->finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        this->send_data(data, len, this->encrypter, this->out_file);
    }

    inline static void send_data(
        const uint8_t * data, size_t len,
        ocrypto & encrypter, OutFileTransport & out_file)
    {
        auto to_send = len;
        while (to_send > 0) {
            const ocrypto::Result res = encrypter.write(data, to_send);
            out_file.send(res.buf.data(), res.buf.size());
            to_send -= res.consumed;
            data += res.consumed;
        }
    }
};
