/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE TestOutCryptoTransport
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT
#include "utils/log.hpp"
#include "transport/out_crypto_transport.hpp"
#include <cstring>

#include "test_only/lcg_random.hpp"
#include "utils/parse.hpp"

inline void init_keys(CryptoContext & cctx)
{
    cctx.set_master_key(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    ));
    cctx.set_hmac_key(cstr_array_view(
         "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
         "\x49\x21\x57\x87\x47\x74\x08\x8a"
         "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
         "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
    ));
}

// TODO should be a subclass of InCryptoTransport
class read_encrypted
{
public:
    CryptoContext & cctx;
    char clear_data[CRYPTO_BUFFER_SIZE];  // contains either raw data from unencrypted file
                                          // or already decrypted/decompressed data
    uint32_t clear_pos;                   // current position in clear_data buf
    uint32_t raw_size;                    // the unciphered/uncompressed data available in buffer

    EVP_CIPHER_CTX ectx;                  // [en|de]cryption context
    uint32_t state;                       // enum crypto_file_state
    unsigned int   MAX_CIPHERED_SIZE;     // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    int encryption; // encryption: 0: auto, 1: encrypted, 2: not encrypted
    bool encrypted;
    uint8_t * cdata;
    size_t cdata_size;
    size_t coffset;

public:
    explicit read_encrypted(CryptoContext & cctx, int encryption, uint8_t * cdata, size_t cdata_size)
    : cctx(cctx)
    , clear_data{}
    , clear_pos(0)
    , raw_size(0)
    , state(0)
    , MAX_CIPHERED_SIZE(0)
    , encryption(encryption)
    , encrypted(false)
    , cdata(cdata)
    , cdata_size(cdata_size)
    , coffset(0)
    {
    }

    ~read_encrypted()
    {
        EVP_CIPHER_CTX_cleanup(&this->ectx);
    }

    int open(uint8_t * derivator, size_t derivator_len)
    {
        size_t base_len = derivator_len;
        const uint8_t * base = derivator;

        ::memset(this->clear_data, 0, sizeof(this->clear_data));
        ::memset(&this->ectx, 0, sizeof(this->ectx));
        this->clear_pos = 0;
        this->raw_size = 0;
        this->state = 0;

        const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
        this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        // todo: we could read in clear_data, that would avoid some copying
        uint8_t data[40];
        size_t avail = 0;
        ::memcpy(&data[avail], &this->cdata[this->coffset], 40);
        avail += 40; this->coffset += 40;
        const uint32_t magic = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        this->encrypted = (magic == WABCRYPTOFILE_MAGIC);

        // Encrypted/Compressed file header (40 bytes)
        // -------------------------------------------
        // MAGIC: 4 bytes
        // 0x57 0x43 0x46 0x4D (WCFM)
        // VERSION: 4 bytes
        // 0x01 0x00 0x00 0x00
        // IV: 32 bytes
        // (random)


        Parse p(data+4);
        // check version
        {
            const uint32_t version = p.in_uint32_le();
            if (version > WABCRYPTOFILE_VERSION) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                    ::getpid(), version, WABCRYPTOFILE_VERSION);
                errno = EINVAL;
                return -1;
            }
        }

        // TODO: replace p.p with some array view of 32 bytes ?
        const uint8_t * const iv = p.p;
        const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
        const uint8_t salt[]  = { 0x39, 0x30, 0x00, 0x00, 0x31, 0xd4, 0x00, 0x00 };
        const int          nrounds = 5;
        unsigned char      key[32];

        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        cctx.get_derived_key(trace_key, {base, base_len});

        int evp_bytes_to_key_res = ::EVP_BytesToKey(cipher, ::EVP_sha1(), salt,
                           trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
        if (32 != evp_bytes_to_key_res){
            // TODO: add true error management
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
            errno = EINVAL;
            return -1;
        }

        ::EVP_CIPHER_CTX_init(&this->ectx);
        if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
            // TODO: add error management
            errno = EINVAL;
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
            return -1;
        }
        return 0;
    }

    ssize_t read(char * data, size_t len)
    {
        if (this->encrypted){
            if (this->state & CF_EOF) {
                return 0;
            }

            unsigned int requested_size = len;

            while (requested_size > 0) {
                // Check how much we have already decoded
                if (!this->raw_size) {
                    uint8_t hlen[4] = {};
                    ::memcpy(&hlen[0], &this->cdata[this->coffset], 4);
                    this->coffset += 4;

                    Parse p(hlen);
                    uint32_t ciphered_buf_size = p.in_uint32_le();
                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->state = CF_EOF;
                        this->clear_pos = 0;
                        this->raw_size = 0;
                        break;
                    }

                    if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        return -1;
                    }

                    uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;

                    //char ciphered_buf[ciphered_buf_size];
                    unsigned char ciphered_buf[65536];
                    //char compressed_buf[compressed_buf_size];
                    unsigned char compressed_buf[65536];
                    ::memcpy(&ciphered_buf[0], &this->cdata[this->coffset], ciphered_buf_size);
                    this->coffset += ciphered_buf_size;

                    int safe_size = compressed_buf_size;
                    int remaining_size = 0;

                    /* allows reusing of ectx for multiple encryption cycles */
                    if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                        return -1;
                    }
                    if (EVP_DecryptUpdate(&this->ectx, compressed_buf, &safe_size, ciphered_buf, ciphered_buf_size) != 1){
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                        return -1;
                    }
                    if (EVP_DecryptFinal_ex(&this->ectx, compressed_buf + safe_size, &remaining_size) != 1){
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                        return -1;
                    }
                    compressed_buf_size = safe_size + remaining_size;

                    size_t chunk_size = CRYPTO_BUFFER_SIZE;
                    const snappy_status status = snappy_uncompress(
                            reinterpret_cast<const char *>(compressed_buf),
                            compressed_buf_size, this->clear_data, &chunk_size);

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

                    this->clear_pos = 0;
                    // When reading, raw_size represent the current chunk size
                    this->raw_size = chunk_size;

                    if (!this->raw_size) { // end of file reached
                        break;
                    }
                }
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->raw_size - this->clear_pos;
                // Check how much we can copy
                unsigned int copiable_size = std::min(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(&data[len - requested_size], this->clear_data + this->clear_pos, copiable_size);
                this->clear_pos      += copiable_size;
                requested_size -= copiable_size;
                // Check if we reach the end
                if (this->raw_size == this->clear_pos) {
                    this->raw_size = 0;
                }
            }
            return len - requested_size;
        }
        else {
            unsigned int requested_size = len;
            if (this->raw_size){
                unsigned int remaining_size = this->raw_size - this->clear_pos;
                // Check how much we can copy
                unsigned int copiable_size = std::min(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(&data[len - requested_size], this->clear_data + this->clear_pos, copiable_size);
                this->clear_pos      += copiable_size;
                requested_size -= copiable_size;
                if (this->raw_size == this->clear_pos) {
                    this->raw_size = 0;
                    this->clear_pos = 0;
                }
                // if we have data in buffer, returning it is OK
                return len - requested_size;
            }
            // for non encrypted file, returning partial read is OK
            ::memcpy(&data[0], &this->cdata[this->coffset], len);
            this->coffset += len;
        }
        // TODO: should never be reached
        return -1;
    }
};


RED_AUTO_TEST_CASE(TestEncryption1)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[8192];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(true, true, cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    RED_CHECK_EQUAL(res.buf.size(), 40);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.
    ocrypto::Result res2 = encrypter.write(reinterpret_cast<const uint8_t*>("toto"), 4);
    memcpy(result + offset, res2.buf.data(), res2.buf.size());
    offset += res2.buf.size();
    RED_CHECK_EQUAL(res2.buf.size(), 0);
    RED_CHECK_EQUAL(res2.consumed, 4);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char fhash[MD_HASH::DIGEST_LENGTH];
    unsigned char qhash[MD_HASH::DIGEST_LENGTH];
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 28);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }

    uint8_t expected_result[68] =  { 'W', 'C', 'F', 'M', // Magic
                                       1, 0, 0, 0,       // Version
                                  // iv
                                  0xb8, 0x6c, 0xda, 0xa6, 0xf0, 0xf6, 0x30, 0x8d,
                                  0xa8, 0x16, 0xa6, 0x6e, 0xe0, 0xc3, 0xe5, 0xcc,
                                  0x98, 0x76, 0xdd, 0xf5, 0xd0, 0x26, 0x74, 0x5f,
                                  0x88, 0x4c, 0xc2, 0x50, 0xc0, 0xdf, 0xc9, 0x50,
                                  // Data
                                  0x10, 0x00, 0x00, 0x00,
                                  0x26, 0xf6, 0x39, 0x17, 0x14, 0x45, 0x7e, 0x3b,
                                  0xfa, 0xfc, 0x11, 0x8a, 0xc0, 0x92, 0xf7, 0x53,
                                  'M', 'F', 'C', 'W',    // EOF Magic
                                  0x04, 0x00, 0x00, 0x00 // Total Length of decrypted data
                                  };
    RED_CHECK_MEM_AA(make_array_view(result, 68), expected_result);

    auto expected_hash = cstr_array_view(
        "\x29\x5c\x52\xcd\xf6\x99\x92\xc3"
        "\xfe\x2f\x05\x90\x0b\x62\x92\xdd"
        "\x12\x31\x2d\x3e\x1d\x17\xd3\xfd"
        "\x8e\x9c\x3b\x52\xcd\x1d\xf7\x29");
    RED_CHECK_MEM_AA(qhash, expected_hash);
    RED_CHECK_MEM_AA(fhash, expected_hash);

}

RED_AUTO_TEST_CASE(TestEncryption2)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[8192];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(true, true, cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    RED_CHECK_EQUAL(res.buf.size(), 40);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.
    {
        ocrypto::Result res2 = encrypter.write(reinterpret_cast<const uint8_t*>("to"), 2);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 2);
    }
    // This test is very similar to Encryption1, but we are performing 2 writes
    {
        ocrypto::Result res2 = encrypter.write(reinterpret_cast<const uint8_t*>("to"), 2);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 2);
    }
    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char qhash[MD_HASH::DIGEST_LENGTH];
    unsigned char fhash[MD_HASH::DIGEST_LENGTH];
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 28);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }

    uint8_t expected_result[68] =  { 'W', 'C', 'F', 'M', // Magic
                                       1, 0, 0, 0,       // Version
                                  // iv
                                  0xb8, 0x6c, 0xda, 0xa6, 0xf0, 0xf6, 0x30, 0x8d,
                                  0xa8, 0x16, 0xa6, 0x6e, 0xe0, 0xc3, 0xe5, 0xcc,
                                  0x98, 0x76, 0xdd, 0xf5, 0xd0, 0x26, 0x74, 0x5f,
                                  0x88, 0x4c, 0xc2, 0x50, 0xc0, 0xdf, 0xc9, 0x50,
                                  // Data
                                  0x10, 0x00, 0x00, 0x00,
                                  0x26, 0xf6, 0x39, 0x17, 0x14, 0x45, 0x7e, 0x3b,
                                  0xfa, 0xfc, 0x11, 0x8a, 0xc0, 0x92, 0xf7, 0x53,
                                  'M', 'F', 'C', 'W',    // EOF Magic
                                  0x04, 0x00, 0x00, 0x00 // Total Length of decrypted data
                                  };
    RED_CHECK_MEM_AA(make_array_view(result, 68), expected_result);

    auto expected_hash = cstr_array_view(
        "\x29\x5c\x52\xcd\xf6\x99\x92\xc3"
        "\xfe\x2f\x05\x90\x0b\x62\x92\xdd"
        "\x12\x31\x2d\x3e\x1d\x17\xd3\xfd"
        "\x8e\x9c\x3b\x52\xcd\x1d\xf7\x29");
    RED_CHECK_MEM_AA(qhash, expected_hash);
    RED_CHECK_MEM_AA(fhash, expected_hash);

    char clear[8192] = {};
    read_encrypted decrypter(cctx, 1, result, offset);
    decrypter.open(derivator, sizeof(derivator));

    size_t res2 = decrypter.read(clear, sizeof(clear));
    RED_CHECK_EQUAL(res2, 4);
    RED_CHECK_MEM_C(make_array_view(clear, 4), "toto");

}



// This sample was generated using udevrandom on Linux
static uint8_t randomSample[8192] = {
#include "fixtures/randomdata.hpp"
};

RED_AUTO_TEST_CASE(TestEncryptionLarge1)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(true, true, cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    RED_CHECK_EQUAL(res.buf.size(), 40);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
    }

    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 8612);
    }

    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, randomSample, sizeof(randomSample));
//    offset += towrite;
//    RED_CHECK_EQUAL(towrite, 8612);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char qhash[MD_HASH::DIGEST_LENGTH];
    unsigned char fhash[MD_HASH::DIGEST_LENGTH];
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 8);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }
    RED_CHECK_EQUAL(offset, 8660);

    char clear[sizeof(randomSample)] = {};
    read_encrypted decrypter(cctx, 1, result, offset);
    decrypter.open(derivator, sizeof(derivator));

    size_t res2 = decrypter.read(clear, sizeof(clear));
    RED_CHECK_EQUAL(res2, sizeof(randomSample));
    RED_CHECK_MEM_AA(clear, randomSample);

    auto expected_qhash = cstr_array_view(
        "\x88\x80\x2e\x37\x08\xca\x43\x30\xed\xd2\x72\x27\x2d\x05\x5d\xee"
        "\x01\x71\x4a\x12\xa5\xd9\x72\x84\xec\x0e\xd5\xaa\x47\x9e\xc3\xc2");
    auto expected_fhash = cstr_array_view(
        "\x62\x96\xe9\xa2\x20\x4f\x39\x21\x06\x4d\x1a\xcf\xf8\x6e\x34\x9c"
        "\xd6\xae\x6c\x44\xd4\x55\x57\xd5\x29\x04\xde\x58\x7f\x1d\x0b\x35");

    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);

    unsigned char fhash2[MD_HASH::DIGEST_LENGTH];

    SslHMAC_Sha256_Delayed hmac;
    hmac.init(cctx.get_hmac_key(), MD_HASH::DIGEST_LENGTH);
    hmac.update(result, offset);
    hmac.final(fhash2);

    RED_CHECK_MEM_AA(fhash2, expected_fhash);

    unsigned char qhash2[MD_HASH::DIGEST_LENGTH];

    SslHMAC_Sha256_Delayed hmac2;
    hmac2.init(cctx.get_hmac_key(), MD_HASH::DIGEST_LENGTH);
    hmac2.update(result, 4096);
    hmac2.final(qhash2);

    RED_CHECK_MEM_AA(qhash2, expected_qhash);
}

RED_AUTO_TEST_CASE(TestEncryptionLargeNoEncryptionChecksum)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(false, true, cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    RED_CHECK_EQUAL(res.buf.size(), 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }

    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }

    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, randomSample, sizeof(randomSample));
//    offset += towrite;
//    RED_CHECK_EQUAL(towrite, 8612);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char qhash[MD_HASH::DIGEST_LENGTH] {};
    unsigned char fhash[MD_HASH::DIGEST_LENGTH] {};
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }
    RED_CHECK_EQUAL(offset, sizeof(randomSample)*2);

    auto expected_qhash = cstr_array_view(
        "\x73\xe8\x21\x3a\x8f\xa3\x61\x0e\x0f\xfe\x14\x28\xff\xcd\x1d\x97"
        "\x7f\xc8\xe8\x90\x44\xfc\x4f\x75\xf7\x6c\xa3\x5b\x0d\x2e\x14\x80");
    auto expected_fhash = cstr_array_view(
        "\x07\xa7\xe7\x14\x9b\xf7\xeb\x34\x57\xdc\xce\x07\x5c\x62\x61\x34"
        "\x51\x42\x7d\xe0\x0f\xbe\xda\x53\x11\x08\x75\x31\x40\xc5\x50\xe8");

    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);

    uint8_t qhash2[MD_HASH::DIGEST_LENGTH] {};
    uint8_t fhash2[MD_HASH::DIGEST_LENGTH] {};

    SslHMAC_Sha256_Delayed hmac;
    hmac.init(cctx.get_hmac_key(), MD_HASH::DIGEST_LENGTH);
    hmac.update(randomSample, sizeof(randomSample));
    hmac.update(randomSample, sizeof(randomSample));
    hmac.final(fhash2);

    SslHMAC_Sha256_Delayed quick_hmac;
    quick_hmac.init(cctx.get_hmac_key(), MD_HASH::DIGEST_LENGTH);
    quick_hmac.update(randomSample, 4096);
    quick_hmac.final(qhash2);

    RED_CHECK_MEM_AA(fhash2, expected_fhash);
    // "\x73\xe8\x21\x3a\x8f\xa3\x61\x0e\x0f\xfe\x14\x28\xff\xcd\x1d\x97\x7f\xc8\xe8\x90\x44\xfc\x4f\x75\xf7\x6c\xa3\x5b\x0d\x2e\x14\x80"
    RED_CHECK_MEM_AA(qhash2, expected_qhash);
}

RED_AUTO_TEST_CASE(TestEncryptionLargeNoEncryption)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(false, false, cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    RED_CHECK_EQUAL(res.buf.size(), 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }

    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }


    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, randomSample, sizeof(randomSample));
//    offset += towrite;
//    RED_CHECK_EQUAL(towrite, 8612);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    uint8_t expected_qhash[MD_HASH::DIGEST_LENGTH] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    uint8_t expected_fhash[MD_HASH::DIGEST_LENGTH] = {
        0x10, 0x01, 0x12, 0x03, 0x14, 0x05, 0x16, 0x07, 0x18, 0x09,0x1A, 0x0B, 0x1C, 0x0D, 0x1E, 0x0F,
        0x10, 0x01, 0x12, 0x03, 0x14, 0x05, 0x16, 0x07, 0x18, 0x09,0x1A, 0x0B, 0x1C, 0x0D, 0x0E, 0x0F
    };

    uint8_t qhash[MD_HASH::DIGEST_LENGTH];
    ::memcpy(qhash, expected_qhash, MD_HASH::DIGEST_LENGTH);
    uint8_t fhash[MD_HASH::DIGEST_LENGTH];
    ::memcpy(fhash, expected_fhash, MD_HASH::DIGEST_LENGTH);
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }
    RED_CHECK_EQUAL(offset, sizeof(randomSample)*2);

    // Check qhash and fhash are left unchanged if no checksum is enabled
    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);
}

RED_AUTO_TEST_CASE(TestEncryptionSmallNoEncryptionChecksum)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(false, true, cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    RED_CHECK_EQUAL(res.buf.size(), 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a small block of data
    {
        uint8_t data[5] = {1, 2, 3, 4, 5};
        ocrypto::Result res2 = encrypter.write(data, sizeof(data));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 5);
    }

    // Let's send a small block of data
    {
        uint8_t data[5] = {1, 2, 3, 4, 5};
        ocrypto::Result res2 = encrypter.write(data, sizeof(data));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 5);
    }

    RED_CHECK_EQUAL(offset, 10);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    uint8_t qhash[MD_HASH::DIGEST_LENGTH] {};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH] {};
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }

    auto expected_qhash = cstr_array_view(
        "\x3b\x79\xd5\x76\x98\x66\x4f\xe1\xdd\xd4\x90\x5b\xa5\x56\x6a\xa3"
        "\x14\x45\x5e\xf3\x8c\x04\xc4\xc4\x49\x6b\x00\xd4\x5e\x82\x13\x68");
    auto expected_fhash = cstr_array_view(
        "\x3b\x79\xd5\x76\x98\x66\x4f\xe1\xdd\xd4\x90\x5b\xa5\x56\x6a\xa3\x14"
        "\x45\x5e\xf3\x8c\x04\xc4\xc4\x49\x6b\x00\xd4\x5e\x82\x13\x68");

    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);

    uint8_t qhash2[MD_HASH::DIGEST_LENGTH] {};
    uint8_t fhash2[MD_HASH::DIGEST_LENGTH] {};

    SslHMAC_Sha256_Delayed hmac;
    hmac.init(cctx.get_hmac_key(), MD_HASH::DIGEST_LENGTH);
    uint8_t data[5] = {1, 2, 3, 4, 5};
    hmac.update(data, sizeof(data));
    hmac.update(data, sizeof(data));
    hmac.final(fhash2);

    SslHMAC_Sha256_Delayed quick_hmac;
    quick_hmac.init(cctx.get_hmac_key(), MD_HASH::DIGEST_LENGTH);
    quick_hmac.update(data, sizeof(data));
    quick_hmac.update(data, sizeof(data));
    quick_hmac.final(qhash2);

    RED_CHECK_MEM_AA(fhash2, expected_fhash);
    RED_CHECK_MEM_AA(qhash2, expected_qhash);
}


RED_AUTO_TEST_CASE(TestOutCryptoTransport_encryption_checksum_oneshot)
{
    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    const char * finalname = "./encrypted.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, 0);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, and again, and so on.", 31);
        ct.close(qhash, fhash);
    }

    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming
    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}

struct TestCryptoCtx
{
    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    TestCryptoCtx(bool with_encryption, bool with_checksum)
    {
        LCGRandom rnd(0);
        CryptoContext cctx;
        init_keys(cctx);

        const char * finalname = "./encrypted.txt";
        char tmpname[256];
        {
            OutCryptoTransport ct(with_encryption, with_checksum, cctx, rnd);
            ct.open(finalname, 0);
            ::strcpy(tmpname, ct.get_tmp());
            ct.send("We write, ", 10);
            ct.send("and again, ", 11);
            ct.send("and so on.", 10);
            ct.close(qhash, fhash);
        }

        RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming
        RED_CHECK(::unlink(finalname) == 0); // finalname exists
    }
};


RED_AUTO_TEST_CASE(TestOutCryptoTransport)
{
    TestCryptoCtx enc_check    (true,  true);
    TestCryptoCtx noenc_check  (false, true);
    TestCryptoCtx noenc_nocheck(false, false);
    TestCryptoCtx enc_nocheck  (true,  false);

    RED_CHECK_MEM_AA(enc_check.fhash, enc_check.qhash);
    RED_CHECK_MEM_AC(
        enc_check.qhash,
        "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce"
        "\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5"
    );

    RED_CHECK_MEM_AA(noenc_check.fhash, noenc_check.qhash);
    RED_CHECK_MEM_AC(
        noenc_check.qhash,
        "\xc5\x28\xb4\x74\x84\x3d\x8b\x14\xcf\x5b\xf4\x3a\x9c\x04\x9a\xf3"
        "\x23\x9f\xac\x56\x4d\x86\xb4\x32\x90\x69\xb5\xe1\x45\xd0\x76\x9b"
    );

    RED_CHECK_MEM_AA(noenc_nocheck.fhash, noenc_nocheck.qhash);
    RED_CHECK_MEM_AC(
        noenc_nocheck.qhash,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    );

    RED_CHECK_MEM_AA(enc_nocheck.fhash, enc_nocheck.qhash);
    RED_CHECK_MEM_AC(
        enc_nocheck.qhash,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    );
}

RED_AUTO_TEST_CASE(TestOutCryptoTransportBigFile)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    const char * finalname = "./encrypted.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, 0);
        ::strcpy(tmpname, ct.get_tmp());
        char buf[200000]{};
        ct.send(buf, sizeof(buf));
        ct.close(qhash, fhash);
    }

    RED_CHECK_MEM_AC(
        qhash,
        "\x39\xcd\x15\x84\x07\x35\x55\xf3\x9b\x45\xc7\xb2\xdd\x06\xa1\x0f"
        "\xd0\x9d\x44\xdd\xcd\x40\x49\x74\x14\xec\x72\x59\xa9\x7b\x7f\x81"
    );

    RED_CHECK_MEM_AC(
        fhash,
        "\xc2\x55\x50\xf3\xcd\x56\xf3\xb9\x26\x37\x06\x9a\x3b\xb1\x26\xd6"
        "\x84\xfd\x6c\xac\x15\xc1\x76\x92\x2f\x16\xc0\xe3\x19\xce\xd0\xe4"
    );

    RED_CHECK(fhash[0] != qhash[0]);


    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming
    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}

RED_AUTO_TEST_CASE(TestOutCryptoTransportAutoClose)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);
    char tmpname[128];
    const char * finalname = "./encrypted.txt";
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, 0);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, and again, and so on.", 31);
    }
    // if there is no explicit close we can't get hash values
    // but the file is correctly closed and ressources freed
    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming
    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}

RED_AUTO_TEST_CASE(TestOutCryptoTransportMultipleFiles)
{
    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);
    char tmpname1[128];
    char tmpname2[128];
    const char * finalname1 = "./encrypted001.txt";
    const char * finalname2 = "./encrypted002.txt";
    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
    {
        OutCryptoTransport ct(true, true, cctx, rnd);

        ct.open(finalname1, 0);
        ::strcpy(tmpname1, ct.get_tmp());
        ct.send("We write, and again, and so on.", 31);
        ct.close(qhash, fhash);

        ct.open(finalname2, 0);
        ::strcpy(tmpname2, ct.get_tmp());
        ct.send("We write, and again, and so on.", 31);
        ct.close(qhash, fhash);
    }
    RED_CHECK(::unlink(tmpname1) == -1); // already removed while renaming
    RED_CHECK(::unlink(finalname1) == 0); // finalname exists
    RED_CHECK(::unlink(tmpname2) == -1); // already removed while renaming
    RED_CHECK(::unlink(finalname2) == 0); // finalname exists
}
