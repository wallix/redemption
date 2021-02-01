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
#include "utils/sugar/bytes_view.hpp"
#include "transport/file_transport.hpp"

#include <memory>


// "MFCW"
constexpr uint32_t WABCRYPTOFILE_MAGIC = 0x4D464357;
constexpr uint32_t WABCRYPTOFILE_EOF_MAGIC = 0x5743464D;
constexpr uint32_t WABCRYPTOFILE_VERSION = 0x00000001;


constexpr std::size_t CRYPTO_BUFFER_SIZE = 4096 * 4;

class Random;

class InCryptoTransport : public Transport //, public PartialIO
{
public:
    enum class EncryptionMode { Auto, Encrypted, NotEncrypted };

    explicit InCryptoTransport(CryptoContext & cctx, EncryptionMode encryption_mode) noexcept;

    ~InCryptoTransport();

    [[nodiscard]] bool is_encrypted() const;

    [[nodiscard]] bool is_open() const;

    struct HASH {
        uint8_t hash[MD_HASH::DIGEST_LENGTH];
    };

    // is noexcept
    [[nodiscard]] static bool read_qhash(
        const char * pathname, uint8_t const (&hmac_key)[HMAC_KEY_LENGTH], HASH& result_hash);
    // is noexcept
    [[nodiscard]] static bool read_fhash(
        const char * pathname, uint8_t const (&hmac_key)[HMAC_KEY_LENGTH], HASH& result_hash);

    void open(const char * const pathname, bytes_view derivator);

    // derivator implicitly basename(pathname)
    void open(const char * const pathname);

    void close();

    bool is_eof() noexcept;

    void disable_log_decrypt(bool disable = true) noexcept; /*NOLINT*/

private:
    // this perform atomic read, partial read will result in exception
    void raw_read(uint8_t buffer[], const size_t len);

    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;


    int fd;
    bool eof;
    uint64_t file_len;
    uint64_t current_len;

    CryptoContext & cctx;
    char clear_data[CRYPTO_BUFFER_SIZE];  // contains either raw data from unencrypted file
                                          // or already decrypted/decompressed data
    uint32_t clear_pos;                   // current position in clear_data buf
    uint32_t raw_size;                    // the unciphered/uncompressed data available in buffer

    DecryptContext ectx;
    unsigned int   MAX_CIPHERED_SIZE;     // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    EncryptionMode encryption_mode;
    bool encrypted;

    struct EncryptedBufferHandle
    {
        void allocate(std::size_t n);

        uint8_t* raw_buffer(std::size_t encrypted_len);

        uint8_t* decrypted_buffer(std::size_t encrypted_len);

    private:
        std::unique_ptr<uint8_t[]> full_buf;
        std::size_t size = 0;
    };
    EncryptedBufferHandle enc_buffer_handle;
};


struct ocrypto : noncopyable
{
    struct Result {
        bytes_view buf;
        std::size_t consumed;
    };

    using HashArray = uint8_t[MD_HASH::DIGEST_LENGTH];

    ocrypto(CryptoContext & cctx, Random & rnd);

    ~ocrypto();

    Result open(bytes_view derivator);

    ocrypto::Result close(HashArray & qhash, HashArray & fhash);

    ocrypto::Result write(bytes_view data);

private:
    EncryptContext ectx;
    SslHMAC_Sha256_Delayed hm;              // full hash context
    SslHMAC_Sha256_Delayed hm4k;             // quick hash context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint64_t       file_size;               // the current file size
    uint8_t header_buf[40];
    uint8_t result_buffer[65536] = {};
    char           buf[CRYPTO_BUFFER_SIZE]; //

    CryptoContext & cctx;
    Random & rnd;

    /* Flush procedure (compression, encryption)
     * Return 0 on success, negatif on error
     */
    void flush(uint8_t * buffer, size_t buflen, size_t & towrite);

    void update_hmac(bytes_view buf);
};


class OutCryptoTransport : public Transport
{
public:
    using HashArray = ocrypto::HashArray;

    explicit OutCryptoTransport(
        CryptoContext & cctx, Random & rnd,
        std::function<void(const Error & error)> notify_error
    ) noexcept;

    [[nodiscard]] const char * get_finalname() const noexcept { return this->finalname; }

    ~OutCryptoTransport();

    // TODO: CGR: I want to remove that from Transport API
    bool disconnect() override;

    [[nodiscard]] bool is_open() const;

    void open(const char * const finalname, const char * const hash_filename, int groupid, uint32_t file_permissions, bytes_view derivator);

    // derivator implicitly basename(finalname)
    void open(const char * finalname, const char * const hash_filename, int groupid, uint32_t file_permissions);

    void close(HashArray & qhash, HashArray & fhash);

    void create_hash_file(HashArray const & qhash, HashArray const & fhash);

    void do_send(const uint8_t * data, size_t len) override;

    bool cancel();

private:
    ocrypto encrypter;
    OutFileTransport out_file;
    char tmpname[2048];
    char finalname[2048];
    std::string hash_filename;
    CryptoContext & cctx;
    Random & rnd;
    int groupid;
    std::vector<uint8_t> derivator;
};


enum class EncryptionSchemeTypeResult
{
    Error = -1,
    NoEncrypted = 0,
    OldScheme,
    NewScheme,
};

EncryptionSchemeTypeResult get_encryption_scheme_type(
    CryptoContext & cctx, const char * filename, bytes_view derivator, Error * err);

/// \attention if result is \c EncryptionSchemeTypeResult::OldScheme, the CryptoContext::old_encryption_scheme must be set to 1 and the file reopen because some data are lost
EncryptionSchemeTypeResult open_if_possible_and_get_encryption_scheme_type(
    InCryptoTransport & in, const char * filename, bytes_view derivator, Error * err);
