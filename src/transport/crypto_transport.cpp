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

#include "transport/crypto_transport.hpp"
#include "capture/cryptofile.hpp"
#include "transport/mwrm_reader.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/transport.hpp"
#include "utils/fileutils.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "utils/parse.hpp"
#include "utils/sugar/byte_ptr.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <memory>

#include <snappy-c.h>
#include <openssl/aes.h>


namespace
{
    std::size_t get_file_len(Fstat & fstat, char const * pathname)
    {
        struct stat sb;
        if (int err = fstat.stat(pathname, sb)) {
            if (err == -1 && errno != 0) {
                err = errno;
            }
            LOG(LOG_ERR, "crypto: stat error %d", err);
            throw Error(ERR_TRANSPORT_READ_FAILED, err);
        }
        return sb.st_size;
    }
} // namespace


void InCryptoTransport::EncryptedBufferHandle::allocate(std::size_t n)
{
    assert(!this->size || n == this->size);
    // [enc_buf] [dec_buf]
    this->full_buf = std::make_unique<uint8_t[]>(n + n + AES_BLOCK_SIZE);
    this->size = n;
}

uint8_t* InCryptoTransport::EncryptedBufferHandle::raw_buffer(std::size_t encrypted_len)
{
    (void)encrypted_len;
    assert(this->full_buf);
    assert(encrypted_len <= this->size);
    return this->full_buf.get();
}

uint8_t* InCryptoTransport::EncryptedBufferHandle::decrypted_buffer(std::size_t encrypted_len)
{
    assert(this->full_buf);
    assert(encrypted_len <= this->size);
    return this->full_buf.get() + encrypted_len;
}

InCryptoTransport::InCryptoTransport(
    CryptoContext & cctx, EncryptionMode encryption_mode, Fstat & fstat) noexcept
: fd(-1)
, eof(true)
, file_len(0)
, current_len(0)
, cctx(cctx)
, clear_data{}
, clear_pos(0)
, raw_size(0)
, MAX_CIPHERED_SIZE(0)
, encryption_mode(encryption_mode)
, encrypted(false)
, fstat(fstat)
{
}

InCryptoTransport::~InCryptoTransport()
{
    if (this->is_open()){
        ::close(this->fd);
    }
}

bool InCryptoTransport::is_encrypted() const
{
    return this->encrypted;
}

bool InCryptoTransport::is_open() const
{
    return this->fd != -1;
}

const InCryptoTransport::HASH InCryptoTransport::qhash(const char * pathname)
{
    SslHMAC_Sha256 hm4k(make_array_view(this->cctx.get_hmac_key()));

    {
        int fd = ::open(pathname, O_RDONLY);
        if (fd < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
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
            hm4k.update({buffer, size_t(res)});
            total_length += res;
        } while (total_length != buffer_size);
    }

    HASH qhash;
    hm4k.final(qhash.hash);
    return qhash;
}

const InCryptoTransport::HASH InCryptoTransport::fhash(const char * pathname)
{
    SslHMAC_Sha256 hm(make_array_view(this->cctx.get_hmac_key()));

    {
        int fd = ::open(pathname, O_RDONLY);
        if (fd < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }
        unique_fd auto_close(fd);

        uint8_t buffer[4096];
        do {
            ssize_t res = ::read(fd, &buffer[0], sizeof(buffer));
            if (res == 0) { break; }
            if (res < 0 && errno == EINTR) { continue; }
            if (res < 0) { throw Error(ERR_TRANSPORT_READ_FAILED, errno); }
            hm.update({buffer, size_t(res)});
        } while (true);
    }

    HASH fhash;
    hm.final(fhash.hash);
    return fhash;
}

void InCryptoTransport::open(const char * const pathname, const_bytes_view derivator)
{
    if (this->is_open()){
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    this->fd = ::open(pathname, O_RDONLY);
    if (this->fd < 0) {
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    this->eof = false;
    this->current_len = 0;

    ::memset(this->clear_data, 0, sizeof(this->clear_data));

    this->clear_pos = 0;
    this->raw_size = 0;

    const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
    this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    this->enc_buffer_handle.allocate(this->MAX_CIPHERED_SIZE);

    // todo: we could read in clear_data, that would avoid some copying
    uint8_t data[40];
    size_t avail = 0;
    while (avail != 40) {
        const ssize_t ret = ::read(this->fd, &data[avail], 40-avail);
        if (ret < 0 && errno == EINTR){
            continue;
        }
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
            this->file_len = get_file_len(this->fstat, pathname);

            return;
        }
        if (ret <= 0) {
            int const err = errno;
            this->close();
            throw Error(ERR_TRANSPORT_READ_FAILED, err);
        }
        avail += ret;
    }

    if (this->encryption_mode == EncryptionMode::NotEncrypted){
        // copy what we have, it's not encrypted, don't care about magic
        this->raw_size = 40;
        this->clear_pos = 0;
        ::memcpy(this->clear_data, data, 40);
        this->file_len = get_file_len(this->fstat, pathname);
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
            this->file_len = get_file_len(this->fstat, pathname);
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
void InCryptoTransport::open(const char * const pathname)
{
    size_t base_len = 0;
    const char * base = basename_len(pathname, base_len);
    this->open(pathname, {base, base_len});
}

void InCryptoTransport::close()
{
    if (!this->is_open()){
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
    ::close(this->fd);
    this->fd = -1;
    this->eof = true;
}

bool InCryptoTransport::is_eof() noexcept
{
    return this->eof;
}

void InCryptoTransport::disable_log_decrypt(bool disable) noexcept
{
    this->ectx.disable_log_decrypt(disable);
}

// this perform atomic read, partial read will result in exception
void InCryptoTransport::raw_read(uint8_t buffer[], const size_t len)
{
    size_t rlen = len;
    while (rlen) {
        ssize_t ret = ::read(this->fd, &buffer[len - rlen], rlen);
        if (ret <= 0){ // unexpected EOF or error
            if (ret != 0 && errno == EINTR){
                continue;
            }
            int const err = errno;
            this->close();
            throw Error(ERR_TRANSPORT_READ_FAILED, err);
        }
        rlen -= ret;
    }
}

size_t InCryptoTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    if (this->eof){
        return 0;
    }
    if (this->encrypted){
        // If we do not have any clear data available read some
        if (!this->raw_size) {

            // Read a full ciphered block at once
            uint8_t hlen[4] = {};
            this->raw_read(hlen, 4);

            const uint32_t enc_len = Parse(hlen).in_uint32_le();
            if (enc_len == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                this->clear_pos = 0;
                this->raw_size = 0;
                this->eof = true;
                return 0;
            }
            if (enc_len > this->MAX_CIPHERED_SIZE) { // integrity error
                this->close();
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }

            auto * raw_buf = this->enc_buffer_handle.raw_buffer(enc_len);
            auto * dec_buf = this->enc_buffer_handle.decrypted_buffer(enc_len);

            this->raw_read(raw_buf, enc_len);
            const size_t pack_buf_size = this->ectx.decrypt(raw_buf, enc_len, dec_buf);

            size_t chunk_size = CRYPTO_BUFFER_SIZE;
            const snappy_status status = snappy_uncompress(
                    char_ptr_cast(dec_buf),
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
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
        remaining_len -= res;
    };

    this->current_len += len;
    if (this->file_len <= this->current_len) {
        this->eof = true;
    }
    return len - remaining_len;
}

InCryptoTransport::Read InCryptoTransport::do_atomic_read(uint8_t * buffer, size_t len)
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
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
    return total == len ? Read::Ok : Read::Eof;
}


/* Flush procedure (compression, encryption)
 * Return 0 on success, negatif on error
 */
void ocrypto::flush(uint8_t * buffer, size_t buflen, size_t & towrite)
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
    uint8_t ciphered_buf[4 + 65536];
    uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
    ciphered_buf_sz = this->ectx.encrypt(
        byte_ptr_cast(compressed_buf), compressed_buf_sz,
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

    this->update_hmac({&ciphered_buf[0], ciphered_buf_sz});

    // Reset buffer
    this->pos = 0;
}

void ocrypto::update_hmac(const_bytes_view buf)
{
    if (this->cctx.get_with_checksum()){
        this->hm.update(buf);
        if (this->file_size < 4096) {
            size_t remaining_size = 4096 - this->file_size;
            this->hm4k.update(buf.first(std::min(remaining_size, buf.size())));
        }
        this->file_size += buf.size();
    }
}

ocrypto::ocrypto(CryptoContext & cctx, Random & rnd)
    : cctx(cctx)
    , rnd(rnd)
{
}

ocrypto::~ocrypto() = default;

ocrypto::Result ocrypto::open(const_bytes_view derivator)
{
    this->file_size = 0;
    this->pos = 0;
    if (this->cctx.get_with_checksum()) {
        this->hm.init(make_array_view(this->cctx.get_hmac_key()));
        this->hm4k.init(make_array_view(this->cctx.get_hmac_key()));
    }

    if (this->cctx.get_with_encryption()) {
        uint8_t trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        this->cctx.get_derived_key(trace_key, derivator);
        uint8_t iv[32];
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
        this->update_hmac({&this->header_buf[0], 40});
        return Result{{this->header_buf, 40u}, 0u};
    }
    return Result{{this->header_buf, 0u}, 0u};
}

ocrypto::Result ocrypto::close(
    uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH],
    uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
{
    size_t towrite = 0;
    if (this->cctx.get_with_encryption()) {
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

        this->update_hmac(make_array_view(tmp_buf));
    }

    if (this->cctx.get_with_checksum()) {
        this->hm.final(fhash);
        this->hm4k.final(qhash);

    }
    return Result{{this->result_buffer, towrite}, 0u};

}

ocrypto::Result ocrypto::write(const_bytes_view data)
{
    if (!this->cctx.get_with_encryption()) {
        this->update_hmac(data);
        return Result{data, data.size()};
    }

    size_t buflen = sizeof(this->result_buffer);
    if (data.size() > buflen - 1000) { // 1000: magic enough for header, actual value is smaller
        data = data.first(buflen);
    }
    // Check how much we can append into buffer
    size_t available_size = CRYPTO_BUFFER_SIZE - this->pos;
    if (available_size > data.size()) {
        available_size = data.size();
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


OutCryptoTransport::OutCryptoTransport(
    CryptoContext & cctx, Random & rnd, Fstat & fstat,
    ReportError report_error
) noexcept
: encrypter(cctx, rnd)
, out_file(invalid_fd(), std::move(report_error))
, cctx(cctx)
, rnd(rnd)
, fstat(fstat)
{
    this->tmpname[0] = 0;
    this->finalname[0] = 0;
}

const char * OutCryptoTransport::get_tmp() const
{
    return &this->tmpname[0];
}

ReportError & OutCryptoTransport::get_report_error()
{
    return this->out_file.get_report_error();
}

OutCryptoTransport::~OutCryptoTransport()
{
    if (not this->is_open()) {
        return;
    }
    try {
        uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
        uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
        this->close(qhash, fhash);
        if (this->cctx.get_with_checksum()){
            char mes[MD_HASH::DIGEST_LENGTH*4+1+128]{};
            char * p = mes;
            p += sprintf(mes, "Encrypted transport implicitly closed, hash checksums dropped :");
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
        LOG(LOG_INFO, "Exception raised in ~OutCryptoTransport %u", e.id);
    }
}

// TODO: CGR: I want to remove that from Transport API
bool OutCryptoTransport::disconnect()
{
    return false;
}

bool OutCryptoTransport::is_open() const
{
    return this->out_file.is_open();
}

void OutCryptoTransport::open(const char * const finalname, const char * const hash_filename, int groupid, const_bytes_view derivator)
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
        LOG(LOG_ERR, "OutCryptoTransport::open : open failed (%s -> %s): %s", this->tmpname, finalname, strerror(errno));
        throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
    }

    if (chmod(this->tmpname, groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
        int const err = errno;
        LOG( LOG_ERR, "can't set file %s mod to %s : %s [%d]"
            , this->tmpname
            , groupid ? "u+r, g+r" : "u+r"
            , strerror(err), err);
        LOG(LOG_INFO, "OutCryptoTransport::open : chmod failed (%s -> %s)", this->tmpname, finalname);
        throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
    }

    if (!utils::strbcpy(this->finalname, finalname)) {
        LOG(LOG_ERR, "OutCryptoTransport::open finalname too long");
        throw Error(ERR_TRANSPORT_OPEN_FAILED);
    }
    this->hash_filename = hash_filename;
    this->derivator.assign(derivator.begin(), derivator.end());
    this->groupid = groupid;

    ocrypto::Result res = this->encrypter.open(derivator);
    this->out_file.send(res.buf.data(), res.buf.size());
}

// derivator implicitly basename(finalname)
void OutCryptoTransport::open(const char * finalname, const char * const hash_filename, int groupid)
{
    size_t base_len = 0;
    const char * base = basename_len(finalname, base_len);
    this->open(finalname, hash_filename, groupid, {base, base_len});
}

void OutCryptoTransport::close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
{
    // Force hash result if no checksum asked
    if (!this->cctx.get_with_checksum()){
        memset(qhash, 0xFF, sizeof(qhash));
        memset(fhash, 0xFF, sizeof(fhash));
    }
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
            LOG(LOG_ERR, "OutCryptoTransport::close Renaming file \"%s\" -> \"%s\" failed, errno=%d : %s\n"
                , this->tmpname, this->finalname, err, strerror(err));
            this->out_file.close();
            throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
        }
        this->tmpname[0] = 0;
    }
    this->out_file.close();
    this->create_hash_file(qhash, fhash);
}

namespace
{
    void send_data(
        cbyte_ptr data_, size_t len,
        ocrypto & encrypter, OutFileTransport & out_file)
    {
        const uint8_t * data = data_.to_u8p();
        auto to_send = len;
        while (to_send > 0) {
            const ocrypto::Result res = encrypter.write({data, to_send});
            out_file.send(res.buf.data(), res.buf.size());
            to_send -= res.consumed;
            data += res.consumed;
        }
    }
} // namespace

void OutCryptoTransport::create_hash_file(
    uint8_t const (&qhash)[MD_HASH::DIGEST_LENGTH],
    uint8_t const (&fhash)[MD_HASH::DIGEST_LENGTH])
{
    ocrypto hash_encrypter(this->cctx, this->rnd);
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
    if (0 != this->fstat.stat(this->finalname, stat)) {
        int const err = errno;
        LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
            this->hash_filename, err);
        throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
    }

    // open
    {
        const ocrypto::Result res = hash_encrypter.open(this->derivator);
        hash_out_file.send(res.buf.data(), res.buf.size());
    }

    // write
    {
        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};

        canonical_path(
            this->finalname,
            path, sizeof(path),
            basename, sizeof(basename),
            extension, sizeof(extension)
        );

        strncat(basename, extension, sizeof(basename) - strlen(basename) - 1);
        utils::back(basename) = '\0';

        MwrmWriterBuf hash_file_buf;
        hash_file_buf.write_hash_file(basename, stat, this->cctx.get_with_checksum(), qhash, fhash);
        auto buf = hash_file_buf.buffer();
        send_data(buf.data(), buf.size(), hash_encrypter, hash_out_file);
    }

    // close
    {
        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];
        const ocrypto::Result res = hash_encrypter.close(qhash, fhash);
        hash_out_file.send(res.buf.data(), res.buf.size());
        hash_out_file.close();
    }
}

void OutCryptoTransport::do_send(const uint8_t * data, size_t len)
{
    if (not this->out_file.is_open()){
        LOG(LOG_ERR, "OutCryptoTransport::do_send failed: file not opened (%s->%s)", this->tmpname, this->finalname);
        throw Error(ERR_TRANSPORT_WRITE_FAILED);
    }
    send_data(data, len, this->encrypter, this->out_file);
}


EncryptionSchemeTypeResult open_if_possible_and_get_encryption_scheme_type(
    InCryptoTransport & in, const char * filename, const_bytes_view derivator, Error * err)
{
    try {
        if (derivator.data()) {
            in.open(filename, derivator);
        }
        else {
            in.open(filename);
        }

        if (not in.is_encrypted()) {
            return EncryptionSchemeTypeResult::NoEncrypted;
        }

        in.disable_log_decrypt();
        char mem[1];
        auto len = in.partial_read(mem, 0);
        (void)len;
    }
    catch(Error const& e) {
        in.disable_log_decrypt(false);
        if (e.id == ERR_SSL_CALL_FAILED) {
            if (in.is_open()) {
                in.close();
            }
            return EncryptionSchemeTypeResult::OldScheme;
        }
        if (err) {
            *err = Error{e.id, (e.errnum ? e.errnum : errno)};
        }
        return EncryptionSchemeTypeResult::Error;
    }

    in.disable_log_decrypt(false);
    return EncryptionSchemeTypeResult::NewScheme;
}

EncryptionSchemeTypeResult get_encryption_scheme_type(
    CryptoContext & cctx, const char * filename, const_bytes_view derivator, Error * err)
{
    Fstat fstat;
    InCryptoTransport in_test(cctx, InCryptoTransport::EncryptionMode::Auto, fstat);
    return open_if_possible_and_get_encryption_scheme_type(in_test, filename, derivator, err);
}
