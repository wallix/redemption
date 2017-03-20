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
#include "system/redemption_unit_tests.hpp"
#include "check_mem.hpp"

//#define LOGNULL
#define LOGPRINT

#include "capture/cryptofile.hpp"
#include "utils/parse.hpp"



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
        const int version = p.in_uint32_le();
        if (version > WABCRYPTOFILE_VERSION) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                ::getpid(), version, WABCRYPTOFILE_VERSION);
            errno = EINVAL;
            return -1;
        }

        // TODO: replace p.p with some array view of 32 bytes ?
        const uint8_t * const iv = p.p;
        const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
        const uint8_t salt[]  = { 0x39, 0x30, 0x00, 0x00, 0x31, 0xd4, 0x00, 0x00 };
        const int          nrounds = 5;
        unsigned char      key[32];

        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        cctx.get_derived_key(trace_key, base, base_len);

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
                            reinterpret_cast<char *>(compressed_buf),
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


BOOST_AUTO_TEST_CASE(TestDerivationOfHmacKeyFromCryptoKey)
{
    unsigned char expected_master_key[] {
        0x61, 0x1f, 0xd4, 0xcd, 0xe5, 0x95, 0xb7, 0xfd,
        0xa6, 0x50, 0x38, 0xfc, 0xd8, 0x86, 0x51, 0x4f,
        0x59, 0x7e, 0x8e, 0x90, 0x81, 0xf6, 0xf4, 0x48,
        0x9c, 0x77, 0x41, 0x51, 0x0f, 0x53, 0x0e, 0xe8,
    };

    unsigned char expected_hmac_key[] {
        0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
        0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
        0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
        0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c,
    };

    CryptoContext cctx;
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

    BOOST_CHECK(0 == memcmp(expected_master_key, cctx.get_master_key(), 32));
    BOOST_CHECK(0 == memcmp(expected_hmac_key, cctx.get_hmac_key(), 32));
}


BOOST_AUTO_TEST_CASE(TestEncryption1)
{
    OpenSSL_add_all_digests();
    
    LCGRandom rnd(0);
    CryptoContext cctx;
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
    

    uint8_t result[8192];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(derivator, sizeof(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    BOOST_CHECK_EQUAL(res.buf.size(), 40);
    BOOST_CHECK_EQUAL(res.err_code, 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.
    ocrypto::Result res2 = encrypter.write("toto", 4);
    memcpy(result + offset, res2.buf.data(), res2.buf.size());
    offset += res2.buf.size();
    BOOST_CHECK_EQUAL(res2.buf.size(), 0);
    BOOST_CHECK_EQUAL(res2.consumed, 4);
    BOOST_CHECK_EQUAL(res2.err_code, 0);
    
    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char hash[MD_HASH_LENGTH << 1];
    {
        ocrypto::Result res2 = encrypter.close(hash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 28);
        BOOST_CHECK_EQUAL(res2.consumed, 0);
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
    CHECK_MEM(result, 68, expected_result);

    char expected_hash[MD_HASH_LENGTH+1] = 
                                  "\x29\x5c\x52\xcd\xf6\x99\x92\xc3"
                                  "\xfe\x2f\x05\x90\x0b\x62\x92\xdd"
                                  "\x12\x31\x2d\x3e\x1d\x17\xd3\xfd"
                                  "\x8e\x9c\x3b\x52\xcd\x1d\xf7\x29";
    CHECK_MEM(hash, MD_HASH_LENGTH, expected_hash);
    CHECK_MEM(hash + MD_HASH_LENGTH, MD_HASH_LENGTH, expected_hash);

}

BOOST_AUTO_TEST_CASE(TestEncryption2)
{
    OpenSSL_add_all_digests();
    
    LCGRandom rnd(0);
    CryptoContext cctx;
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
    

    uint8_t result[8192];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(derivator, sizeof(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    BOOST_CHECK_EQUAL(res.buf.size(), 40);
    BOOST_CHECK_EQUAL(res.err_code, 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.
    {
        ocrypto::Result res2 = encrypter.write("toto", 2);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 0);
        BOOST_CHECK_EQUAL(res2.consumed, 2);
        BOOST_CHECK_EQUAL(res2.err_code, 0);
    }
    // This test is very similar to Encryption1, but we are performing 2 writes
    {
        ocrypto::Result res2 = encrypter.write("toto", 2);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 0);
        BOOST_CHECK_EQUAL(res2.consumed, 2);
        BOOST_CHECK_EQUAL(res2.err_code, 0);
    }    
    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char hash[MD_HASH_LENGTH << 1];
    {
        ocrypto::Result res2 = encrypter.close(hash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 28);
        BOOST_CHECK_EQUAL(res2.consumed, 0);
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
    CHECK_MEM(result, 68, expected_result);

    char expected_hash[MD_HASH_LENGTH+1] = 
                                  "\x29\x5c\x52\xcd\xf6\x99\x92\xc3"
                                  "\xfe\x2f\x05\x90\x0b\x62\x92\xdd"
                                  "\x12\x31\x2d\x3e\x1d\x17\xd3\xfd"
                                  "\x8e\x9c\x3b\x52\xcd\x1d\xf7\x29";
    CHECK_MEM(hash, MD_HASH_LENGTH, expected_hash);
    CHECK_MEM(hash + MD_HASH_LENGTH, MD_HASH_LENGTH, expected_hash);

    char clear[8192] = {};
    read_encrypted decrypter(cctx, 1, result, offset);
    decrypter.open(derivator, sizeof(derivator));
    
    size_t res2 = decrypter.read(clear, sizeof(clear));
    BOOST_CHECK_EQUAL(res2, 4);
    CHECK_MEM(clear, 4, "toto");

}



// This sample was generated using udevrandom on Linux
uint8_t randomSample[8192] = {
#include "fixtures/randomdata.hpp"
};

BOOST_AUTO_TEST_CASE(TestEncryptionLarge1)
{
    OpenSSL_add_all_digests();
    
    LCGRandom rnd(0);
    CryptoContext cctx;
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
    

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(derivator, sizeof(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    BOOST_CHECK_EQUAL(res.buf.size(), 40);
    BOOST_CHECK_EQUAL(res.err_code, 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data 
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 0);
        BOOST_CHECK_EQUAL(res2.err_code, 0);
    }

    {
        ocrypto::Result res2 = encrypter.write(randomSample, sizeof(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 8612);
        BOOST_CHECK_EQUAL(res2.err_code, 0);
    }

    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, randomSample, sizeof(randomSample));
//    offset += towrite;
//    BOOST_CHECK_EQUAL(towrite, 8612);
    
    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char hash[MD_HASH_LENGTH << 1];
    {
        ocrypto::Result res2 = encrypter.close(hash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        BOOST_CHECK_EQUAL(res2.buf.size(), 8);
        BOOST_CHECK_EQUAL(res2.consumed, 0);
    }
    BOOST_CHECK_EQUAL(offset, 8660);

//    CHECK_MEM(result, offset, tmp);


    char clear[sizeof(randomSample)] = {};
    read_encrypted decrypter(cctx, 1, result, offset);
    decrypter.open(derivator, sizeof(derivator));
    
    size_t res2 = decrypter.read(clear, sizeof(clear));
    BOOST_CHECK_EQUAL(res2, sizeof(randomSample));
    CHECK_MEM(clear, sizeof(randomSample), randomSample);

    char expected_hash1[MD_HASH_LENGTH+1] = "\x88\x80\x2e\x37\x08\xca\x43\x30\xed\xd2\x72\x27\x2d\x05\x5d\xee\x01\x71\x4a\x12\xa5\xd9\x72\x84\xec\x0e\xd5\xaa\x47\x9e\xc3\xc2";
    char expected_hash2[MD_HASH_LENGTH+1] = "\x62\x96\xe9\xa2\x20\x4f\x39\x21\x06\x4d\x1a\xcf\xf8\x6e\x34\x9c\xd6\xae\x6c\x44\xd4\x55\x57\xd5\x29\x04\xde\x58\x7f\x1d\x0b\x35";

    CHECK_MEM(hash, MD_HASH_LENGTH, expected_hash1);
    CHECK_MEM(hash + MD_HASH_LENGTH, MD_HASH_LENGTH, expected_hash2);
}

