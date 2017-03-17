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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDoRecorder
#include "system/redemption_unit_tests.hpp"

#include <check_mem.hpp>

#define LOGPRINT
//#define LOGNULL
#include "utils/log.hpp"

#include "main/do_recorder.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fstream>
#include <sstream>
#include "utils/fileutils.hpp"
#include "capture/capture.hpp"

#ifdef HASH_LEN
#undef HASH_LEN
#endif  // #ifdef HASH_LEN
#define HASH_LEN 64

extern "C" {
    inline int hmac_fn(char * buffer)
    {
        // E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE
        uint8_t hmac_key[SslSha256::DIGEST_LENGTH] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        memcpy(buffer, hmac_key, SslSha256::DIGEST_LENGTH);
        return 0;
    }

    inline int trace_fn(char * base, int len, char * buffer, unsigned oldscheme)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
        uint8_t trace_key[SslSha256::DIGEST_LENGTH] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2
         };
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }
}

// tests/fixtures/verifier/recorded/toto@10.10.43.13\,Administrateur@QA@cible\,20160218-181658\,wab-5-0-0.yourdomain\,7681.mwrm

// python tools/decrypter.py -i tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm -o decrypted.out

BOOST_AUTO_TEST_CASE(TestDecrypterEncryptedData)
{
   LOG(LOG_INFO, "=================== TestDecrypterEncryptedData =============");
    char const * argv[] {
        "decrypter.py",
        "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        "-o",
            "./decrypted.out",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(0, unlink("./decrypted.out"));
    BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestDecrypterClearData)
{
   LOG(LOG_INFO, "=================== TestDecrypterClearData =============");
    char const * argv[] {
        "decrypter.py",
        "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
                "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "-o",
            "decrypted.2.out",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(0, res);
}


BOOST_AUTO_TEST_CASE(TestReverseIterators)
{
   LOG(LOG_INFO, "=================== TestReverseIterators =============");

    // Show how to extract filename even if it contains spaces
    // the idea is that the final fields are fixed, henceforth
    // we can skip these fields by looking for spaces from end of line
    // once filename is found we can manage other fields as usual.
    // no need to search backward.

    char line[256] = {};
    const char * str = "ff fff sssss eeeee hhhhh HHHHH\n";
    int len = strlen(str);
    memcpy(line, str, len+1);
    typedef std::reverse_iterator<char*> reverse_iterator;

    reverse_iterator first(line);
    reverse_iterator last(line + len);

    reverse_iterator space4 = std::find(last, first, ' ');
    space4++;
    reverse_iterator space3 = std::find(space4, first, ' ');
    space3++;
    reverse_iterator space2 = std::find(space3, first, ' ');
    space2++;
    reverse_iterator space1 = std::find(space2, first, ' ');
    space1++;
    int filename_len = first-space1;

    char filename[128];
    memcpy(filename, line, filename_len);

    BOOST_CHECK(0 == memcmp("ff fff", filename, filename_len));

}


BOOST_AUTO_TEST_CASE(TestLineReader)
{
   LOG(LOG_INFO, "=================== TestLineReader =============");
    char const * filename = "/tmp/test_app_verifier_s.txt";

    std::ofstream(filename) <<
        "abcd\n"
        "efghi\n"
        "jklmno\n"
    ;

    {
        ifile_read ifile;
        ifile.open(filename);
        LineReader line_reader(ifile);
        BOOST_CHECK(line_reader.next_line());
        BOOST_CHECK_EQUAL(5, line_reader.get_buf().size());
        BOOST_CHECK(line_reader.next_line());
        BOOST_CHECK_EQUAL(6, line_reader.get_buf().size());
        BOOST_CHECK(line_reader.next_line());
        BOOST_CHECK_EQUAL(7, line_reader.get_buf().size());
        BOOST_CHECK(not line_reader.next_line());
    }

    std::size_t const big_line_len = LineReader::line_max - LineReader::line_max / 4;
    {
        std::string s(big_line_len, 'a');
        std::ofstream(filename) << s << '\n' << s << '\n';
    }

    {
        ifile_read ifile;
        ifile.open(filename);
        LineReader line_reader(ifile);
        BOOST_CHECK(line_reader.next_line());
        BOOST_CHECK_EQUAL(big_line_len+1, line_reader.get_buf().size());
        BOOST_CHECK(line_reader.next_line());
        BOOST_CHECK_EQUAL(big_line_len+1, line_reader.get_buf().size());
        BOOST_CHECK(!line_reader.next_line());
    }

    std::ofstream(filename) << std::string(LineReader::line_max + 20, 'a');

    {
        ifile_read ifile;
        ifile.open(filename);
        LineReader line_reader(ifile);
        BOOST_CHECK_EXCEPTION(line_reader.next_line(), Error, [](Error const & e) {
            return e.id == ERR_TRANSPORT_READ_FAILED && e.errnum == 0;
        });
    }

    remove(filename);
}

BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash)
{
   LOG(LOG_INFO, "=================== TestVerifierCheckFileHash =============");
    const std::string test_mwrm_path = "./";
    const std::string test_file_name = "TestCheckFileHash";

    /************************
    * Manage encryption key *
    ************************/

    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

    uint8_t hmac_key[32] = {};

    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };
    unsigned char tmp_derivation1[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated2[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
    memcpy(tmp_derivation1, HASH_DERIVATOR, DERIVATOR_LENGTH);
    memcpy(tmp_derivation1 + DERIVATOR_LENGTH, cctx.get_master_key(), CRYPTO_KEY_LENGTH);
    SHA256(tmp_derivation1, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated2);
    memcpy(hmac_key, derivated2, HMAC_KEY_LENGTH);

    OpenSSL_add_all_digests();

    // Any iv key would do, we are checking round trip
    unsigned char iv[32] = {
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
    };

    unsigned char derivator[DERIVATOR_LENGTH];

    size_t len = 0;
    const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(test_file_name.c_str(), len));
    SslSha256 sha256;
    sha256.update(base, len);
    uint8_t tmp[SslSha256::DIGEST_LENGTH];
    sha256.final(tmp);
    memcpy(derivator, tmp, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    unsigned char tmp_derivation2[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated1[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
    memcpy(tmp_derivation2, derivator, DERIVATOR_LENGTH);
    memcpy(tmp_derivation2 + DERIVATOR_LENGTH, cctx.get_master_key(), CRYPTO_KEY_LENGTH);
    SHA256(tmp_derivation2, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated1);
    memcpy(trace_key, derivated1, HMAC_KEY_LENGTH);

    std::string full_test_file_name = test_mwrm_path + test_file_name;
    int system_fd = open(full_test_file_name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0600);
    BOOST_REQUIRE_MESSAGE(system_fd != -1, "failed opening=" << full_test_file_name);

    struct crypto_file
    {
          iofdbuf file;
          class encrypt_filter_iofdbuf
          {
            char           buf[CRYPTO_BUFFER_SIZE]; //
            EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
            EVP_MD_CTX     hctx;                    // hash context
            EVP_MD_CTX     hctx4k;                  // hash context
            uint32_t       pos;                     // current position in buf
            uint32_t       raw_size;                // the unciphered/uncompressed file size
            uint32_t       file_size;               // the current file size

            iofdbuf & snk;

        public:
            encrypt_filter_iofdbuf(iofdbuf & snk) : snk(snk) {}
            encrypt_filter_iofdbuf() = delete;
            //: pos(0)
            //, raw_size(0)
            //, file_size(0)
            //{}

            int open(iofdbuf & snk, const unsigned char * trace_key, CryptoContext & cctx, const unsigned char * iv)         {
            
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
                        if ( ! ::MD_HASH_FUNC(cctx.get_hmac_key(), CRYPTO_KEY_LENGTH, keyhash)) {
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                            return -1;
                        }
                        ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                    }
                    else {
                        ::memcpy(key_buf, cctx.get_hmac_key(), CRYPTO_KEY_LENGTH);
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
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: write error! write_ret=%d error=%s\n", ::getpid(), int(write_ret), ::strerror(errno));
                    return write_ret;
                }
                // update file_size
                this->file_size += 40;

                return this->xmd_update(tmp_buf, 40);
            }

            ssize_t write(iofdbuf & snk, const void * data, size_t len)
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
            int flush(iofdbuf & snk)
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

            int close(iofdbuf & snk, unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
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
                        if ( ! ::MD_HASH_FUNC(hmac_key, CRYPTO_KEY_LENGTH, keyhash)) {
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
            ssize_t raw_write(iofdbuf & snk, void * data, size_t len)
            {
                ssize_t err = snk.write(data, len);
                return err < 0 ? -1 : 0;
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
      } encrypt;

      crypto_file(int fd) : encrypt(this->file)
      {
        this->file.iofdbuf_fd = fd;
      }
    } * cf_struct = new (std::nothrow) crypto_file(system_fd);

    if (cf_struct) {
        if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, cctx, iv)) {
            LOG(LOG_INFO, "encrypt.open failed for cf_struct %s", full_test_file_name.c_str());
            delete cf_struct;
            cf_struct = nullptr;
            close(system_fd);
        }
    }

    BOOST_CHECK(cf_struct);

    const char    * data     = "Indentation Settings determine the size of the tab stops, "
                               "and control whether the tab key should insert tabs or spaces.";
    const size_t    data_len = strlen(data);
    int             res;
    unsigned char   hash[HASH_LEN];


    for (int i = 0; i < 256; i ++) {
        res = cf_struct->encrypt.write(cf_struct->file, const_cast<char *>(data), data_len);
        BOOST_CHECK_EQUAL(data_len, res);
    }

    res = cf_struct->encrypt.close(cf_struct->file, hash, cctx.get_hmac_key());
    delete cf_struct;

    BOOST_CHECK_EQUAL(0, res);

    std::string const test_full_mwrm_filename = test_mwrm_path + test_file_name;

    {
        uint8_t tmp_hash[SHA256_DIGEST_LENGTH]={};
        int res = file_start_hmac_sha256(test_full_mwrm_filename.c_str(),
                         cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()),
                         QUICK_CHECK_LENGTH, tmp_hash);
        BOOST_CHECK_EQUAL(res, 0);
        BOOST_CHECK(0 == memcmp(hash, tmp_hash, SHA256_DIGEST_LENGTH));
    }

    {
        uint8_t tmp_hash[SHA256_DIGEST_LENGTH]={};
        int res = file_start_hmac_sha256(test_full_mwrm_filename.c_str(),
                         cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()),
                         0, tmp_hash);
        BOOST_CHECK_EQUAL(res, 0);
        BOOST_CHECK(0 == memcmp(hash + (HASH_LEN / 2), tmp_hash, SHA256_DIGEST_LENGTH));
    }

    unlink(full_test_file_name.c_str());
}   /* BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash) */

// python tools/verifier.py -i toto@10.10.43.13\,Administrateur@QA@cible\,20160218-183009\,wab-5-0-0.yourdomain\,7335.mwrm --hash-path tests/fixtures/verifier/hash/ --mwrm-path tests/fixtures/verifier/recorded/ --verbose 10



template<class Exception>
bool is_except( Exception const & ) { return true; }

BOOST_AUTO_TEST_CASE(TestVerifierFileNotFound)
{
   LOG(LOG_INFO, "=================== TestVerifierFileNotFound =============");
    char const * argv[] = {
        "verifier.py",
        "redver",
        "-i", "asdfgfsghsdhds.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/bad",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(res, -1);
}

BOOST_AUTO_TEST_CASE(TestVerifierEncryptedDataFailure)
{
   LOG(LOG_INFO, "=================== TestVerifierEncryptedDataFailure =============");
    char const * argv[] = {
        "verifier.py",
        "redver",
        "-i",
            "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        "--hash-path",
            FIXTURES_PATH "/verifier/hash",
        "--mwrm-path",
            FIXTURES_PATH "/verifier/recorded/bad",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(1, res);
}

BOOST_AUTO_TEST_CASE(TestVerifierEncryptedData)
{
   LOG(LOG_INFO, "=================== TestVerifierEncryptedData =============");
    char const * argv[] = {
        "verifier.py",
        "redver",
        "-i",
            "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        "--hash-path",
            FIXTURES_PATH "/verifier/hash",
        "--mwrm-path",
            FIXTURES_PATH "/verifier/recorded",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestVerifierClearData)
{
   LOG(LOG_INFO, "=================== TestVerifierClearData =============");
    char const * argv[] {
        "verifier.py",
        "redver",
        "-i",
            "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "--hash-path",
            FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path",
            FIXTURES_PATH "/verifier/recorded/",
        "--verbose",
            "10",
        "--ignore-stat-info"
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(0, res);
}


BOOST_AUTO_TEST_CASE(TestVerifierUpdateData)
{
    LOG(LOG_INFO, "=================== TestVerifierUpdateData =============");
//    Inifile ini;
//    ini.set<cfg::debug::config>(false);
//    CryptoContext cctx;
//    LOG(LOG_INFO, "set_get_hmac_key");
//    cctx.set_get_hmac_key_cb(hmac_fn);
//    cctx.set_get_trace_key_cb(trace_fn);
//    LOG(LOG_INFO, "set_get_hmac_key done");

#define MWRM_FILENAME "toto@10.10.43.13,Administrateur@QA@cible" \
    ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm"
#define WRM_FILENAME "toto@10.10.43.13,Administrateur@QA@cible" \
    ",20160218-181658,wab-5-0-0.yourdomain,7681-000000.wrm"
#define TMP_VERIFIER "/tmp/app_verifier_test"

    char const * tmp_recorded_mwrm = TMP_VERIFIER "/recorded/" MWRM_FILENAME;
    char const * tmp_recorded_wrm = TMP_VERIFIER "/recorded/" WRM_FILENAME;
    char const * tmp_hash_mwrm = TMP_VERIFIER "/hash/" MWRM_FILENAME;

    mkdir(TMP_VERIFIER, 0777);
    mkdir(TMP_VERIFIER "/hash", 0777);
    mkdir(TMP_VERIFIER "/recorded", 0777);
    std::ofstream(tmp_hash_mwrm, std::ios::trunc)
      << std::ifstream(FIXTURES_PATH "/verifier/hash/" MWRM_FILENAME).rdbuf();
    std::ofstream(tmp_recorded_mwrm, std::ios::trunc)
      << std::ifstream(FIXTURES_PATH "/verifier/recorded/" MWRM_FILENAME).rdbuf();
    std::ofstream(tmp_recorded_wrm, std::ios::trunc | std::ios::binary)
      << std::ifstream(FIXTURES_PATH "/verifier/recorded/" WRM_FILENAME).rdbuf();

    auto str_stat = [](char const * filename){
        std::string s;
        struct stat64 stat;
        ::stat64(filename, &stat);
        s += std::to_string(stat.st_size);
        s += ' ';
        s += std::to_string(stat.st_mode);
        s += ' ';
        s += std::to_string(stat.st_uid);
        s += ' ';
        s += std::to_string(stat.st_gid);
        s += ' ';
        s += std::to_string(stat.st_dev);
        s += ' ';
        s += std::to_string(stat.st_ino);
        s += ' ';
        s += std::to_string(stat.st_mtime);
        s += ' ';
        s += std::to_string(stat.st_ctime);
        return s;
    };

    std::string mwrm_hash_contents = "v2\n\n\n" MWRM_FILENAME " " + str_stat(tmp_recorded_mwrm) + "\n";
    std::string mwrm_recorded_contents = "v2\n800 600\nnochecksum\n\n\n/var/wab/recorded/rdp/"
        WRM_FILENAME " " + str_stat(tmp_recorded_wrm) + " 1455815820 1455816422\n";

    auto get_file_contents = [](char const * filename){
      std::ostringstream out;
      out << std::ifstream(filename).rdbuf();
      return out.str();
    };

    BOOST_CHECK_NE(get_file_contents(tmp_hash_mwrm), mwrm_hash_contents);
    BOOST_CHECK_NE(get_file_contents(tmp_recorded_mwrm), mwrm_recorded_contents);

    char const * argv[] {
        "verifier.py",
        "redver",
        "-i",
            MWRM_FILENAME,
        "--hash-path",
            TMP_VERIFIER "/hash/",
        "--mwrm-path",
            TMP_VERIFIER "/recorded/",
        "--verbose",
            "10",
        "--update-stat-info"
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(0, res);

    mwrm_hash_contents = "v2\n\n\n" MWRM_FILENAME " " + str_stat(tmp_recorded_mwrm) + "\n";
    mwrm_recorded_contents = "v2\n800 600\nnochecksum\n\n\n/var/wab/recorded/rdp/"
        WRM_FILENAME " " + str_stat(tmp_recorded_wrm) + " 1455815820 1455816422\n";

    BOOST_CHECK_EQUAL(get_file_contents(tmp_hash_mwrm), mwrm_hash_contents);
    BOOST_CHECK_EQUAL(get_file_contents(tmp_recorded_mwrm), mwrm_recorded_contents);

    remove(tmp_hash_mwrm);
    remove(tmp_recorded_mwrm);
    remove(tmp_recorded_wrm);

#undef TMP_VERIFIER
#undef WRM_FILENAME
#undef MWRM_FILENAME
}

BOOST_AUTO_TEST_CASE(TestVerifierClearDataStatFailed)
{
    LOG(LOG_INFO, "=================== TestVerifierClearDataStatFailed =============");
    char const * argv[] {
        "verifier.py",
        "redver",
        "-i",
            "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "--hash-path",
            FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path",
            FIXTURES_PATH "/verifier/recorded/",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_fn, trace_fn));
    BOOST_CHECK_EQUAL(1, res);
}

BOOST_AUTO_TEST_CASE(ReadClearHeaderV2)
{
    LOG(LOG_INFO, "=================== ReadClearHeaderV2 =============");
    ifile_read fd;
    fd.open(FIXTURES_PATH "/verifier/recorded/v2_nochecksum_nocrypt.mwrm");
    MwrmReader reader(fd);

    reader.read_meta_headers(false);
    BOOST_CHECK(reader.header.version == 2);
    BOOST_CHECK(!reader.header.has_checksum);

    MetaLine2 meta_line;
    BOOST_CHECK(reader.read_meta_file(meta_line));
    BOOST_CHECK_EQUAL(meta_line.filename,
                        "/var/wab/recorded/rdp/"
                        "toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,"
                        "wab-5-0-0.yourdomain,7681-000000.wrm");
    BOOST_CHECK_EQUAL(meta_line.size, 181826);
    BOOST_CHECK_EQUAL(meta_line.mode, 33056);
    BOOST_CHECK_EQUAL(meta_line.uid, 1001);
    BOOST_CHECK_EQUAL(meta_line.gid, 1001);
    BOOST_CHECK_EQUAL(meta_line.dev, 65030);
    BOOST_CHECK_EQUAL(meta_line.ino, 81);
    BOOST_CHECK_EQUAL(meta_line.mtime, 1455816421);
    BOOST_CHECK_EQUAL(meta_line.ctime, 1455816421);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1455815820);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1455816422);
}

BOOST_AUTO_TEST_CASE(ReadClearHeaderV1)
{
    LOG(LOG_INFO, "=================== (ReadClearHeaderV1 =============");
    ifile_read fd;
    fd.open(FIXTURES_PATH "/verifier/recorded/v1_nochecksum_nocrypt.mwrm");
    MwrmReader reader(fd);

    reader.read_meta_headers(false);
    BOOST_CHECK(reader.header.version == 1);
    BOOST_CHECK(!reader.header.has_checksum);

    MetaLine2 meta_line;
    reader.read_meta_file(meta_line);
    BOOST_CHECK_EQUAL(meta_line.filename,
                        "/var/wab/recorded/rdp/"
                        "toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,"
                        "wab-5-0-0.yourdomain,7681-000000.wrm");
    BOOST_CHECK_EQUAL(meta_line.size, 0);
    BOOST_CHECK_EQUAL(meta_line.mode, 0);
    BOOST_CHECK_EQUAL(meta_line.uid, 0);
    BOOST_CHECK_EQUAL(meta_line.gid, 0);
    BOOST_CHECK_EQUAL(meta_line.dev, 0);
    BOOST_CHECK_EQUAL(meta_line.ino, 0);
    BOOST_CHECK_EQUAL(meta_line.mtime, 0);
    BOOST_CHECK_EQUAL(meta_line.ctime, 0);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1455815820);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1455816422);

    BOOST_CHECK(0 == memcmp(meta_line.hash1,
                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
                        32));
    BOOST_CHECK(0 == memcmp(meta_line.hash2,
                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
                        32));

}

BOOST_AUTO_TEST_CASE(ReadClearHeaderV2Checksum)
{
    LOG(LOG_INFO, "=================== ReadClearHeaderV2Checksum =============");
    ifile_read fd;
    fd.open(FIXTURES_PATH "/sample_v2_checksum.mwrm");
    MwrmReader reader(fd);

    reader.read_meta_headers(false);
    BOOST_CHECK(reader.header.version == 2);
    BOOST_CHECK(reader.header.has_checksum);

    MetaLine2 meta_line;
    reader.read_meta_file(meta_line);
    BOOST_CHECK(true);
    BOOST_CHECK_EQUAL(meta_line.filename, "./tests/fixtures/sample0.wrm");
    BOOST_CHECK_EQUAL(meta_line.size, 1);
    BOOST_CHECK_EQUAL(meta_line.mode, 2);
    BOOST_CHECK_EQUAL(meta_line.uid, 3);
    BOOST_CHECK_EQUAL(meta_line.gid, 4);
    BOOST_CHECK_EQUAL(meta_line.dev, 5);
    BOOST_CHECK_EQUAL(meta_line.ino, 6);
    BOOST_CHECK_EQUAL(meta_line.mtime, 7);
    BOOST_CHECK_EQUAL(meta_line.ctime, 8);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304810);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304870);
    BOOST_CHECK(0 == memcmp(meta_line.hash1,
                        "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
                        "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA",
                        32));
    BOOST_CHECK(0 == memcmp(meta_line.hash2,
                        "\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB"
                        "\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB",
                        32));
}

BOOST_AUTO_TEST_CASE(ReadEncryptedHeaderV2Checksum)
{
    LOG(LOG_INFO, "=================== ReadEncryptedHeaderV2Checksum =============");

    CryptoContext cctx;
    cctx.set_get_hmac_key_cb(hmac_fn);
    cctx.set_get_trace_key_cb(trace_fn);

    ifile_read_encrypted fd(cctx, 1);
    fd.open(FIXTURES_PATH
        "/verifier/recorded/"
        "toto@10.10.43.13,Administrateur@QA@cible,"
        "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm");

    MwrmReader reader(fd);

    reader.read_meta_headers(true);
    BOOST_CHECK(reader.header.version == 2);
    BOOST_CHECK(reader.header.has_checksum);

    MetaLine2 meta_line;
    BOOST_CHECK(reader.read_meta_file(meta_line));

    {
        std::string expected("/var/wab/recorded/rdp"
                             "/toto@10.10.43.13,Administrateur@QA@cible,"
                             "20160218-183009,wab-5-0-0.yourdomain,7335-000000.wrm");
        BOOST_REQUIRE_EQUAL(meta_line.filename, expected);
    }
    BOOST_CHECK_EQUAL(meta_line.size, 163032);
    BOOST_CHECK_EQUAL(meta_line.mode, 33056);
    BOOST_CHECK_EQUAL(meta_line.uid, 1001);
    BOOST_CHECK_EQUAL(meta_line.gid, 1001);
    BOOST_CHECK_EQUAL(meta_line.dev, 65030);
    BOOST_CHECK_EQUAL(meta_line.ino, 89);
    BOOST_CHECK_EQUAL(meta_line.mtime, 1455816632);
    BOOST_CHECK_EQUAL(meta_line.ctime, 1455816632);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1455816611);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1455816633);
    CHECK_MEM(meta_line.hash1, 32,
      "\x05\x6c\x10\xb7\xbd\x80\xa8\x72\x87\x33\x6d\xee\x6e\x43\x1d\x81"
      "\x56\x06\xa1\xf9\xf0\xe6\x37\x12\x07\x22\xe3\x0c\x2c\x8c\xd7\x77");
    CHECK_MEM(meta_line.hash2, 32,
      "\xf3\xc5\x36\x2b\xc3\x47\xf8\xb4\x4a\x1d\x91\x63\xdd\x68\xed\x99"
      "\xc1\xed\x58\xc2\xd3\x28\xd1\xa9\x4a\x07\x7d\x76\x58\xca\x66\x7c");

    BOOST_CHECK(!reader.read_meta_file(meta_line));
}

inline int hmac_2016_fn(char * buffer)
{

    uint8_t hmac_key[32] = {
        0x56 , 0xdd , 0xb2 , 0x92 , 0x47 , 0xbe , 0x4b , 0x89 ,
        0x1f , 0x12 , 0x62 , 0x39 , 0x0f , 0x10 , 0xb9 , 0x8e ,
        0xac , 0xff , 0xbc , 0x8a , 0x8f , 0x71 , 0xfb , 0x21 ,
        0x07 , 0x7d , 0xef , 0x9c , 0xb3 , 0x5f , 0xf9 , 0x7b ,
        };
    memcpy(buffer, hmac_key, 32);
    return 0;
}

inline int trace_20161025_fn(char * base, int len, char * buffer, unsigned oldscheme)
{
//    LOG(LOG_INFO, "\n\ntrace_20161025_fn(%*s,%d,oldscheme=%d)->\n", len, base, len, oldscheme);

    (void)base;
    (void)len;

    struct {
        std::string base;
        unsigned scheme;
        uint8_t derived_key[32];
    } keys[] = {
        {
            "cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm",
            0,
            {
                0x63, 0xfc, 0x3a, 0x0a, 0x32, 0x36, 0x41, 0x8a,
                0x7f, 0xaa, 0x8d, 0x88, 0xbb, 0x33, 0x73, 0x34,
                0x6a, 0xdb, 0xa9, 0x42, 0x96, 0xbb, 0xcd, 0x06,
                0xbe, 0xf8, 0xc4, 0x07, 0x8b, 0x0a, 0x80, 0xc4
            }
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm",
            0,
            {
                0xdc, 0x55, 0x98, 0xe3, 0x7f, 0x90, 0xa5, 0x94,
                0x60, 0x89, 0x1f, 0x95, 0x81, 0x00, 0xf8, 0xaf,
                0x73, 0xe1, 0x3d, 0x22, 0xe3, 0x6c, 0x40, 0x59,
                0x93, 0x05, 0xab, 0xf6, 0x16, 0xd2, 0x3a, 0xdb
            }
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm",
            0,
            {
                0x8f, 0x17, 0x01, 0xd8, 0x87, 0xd7, 0xa1, 0x1b,
                0x40, 0x02, 0x68, 0x8d, 0xe4, 0x22, 0x2c, 0x42,
                0xe1, 0x30, 0x8e, 0x37, 0xfa, 0x2c, 0xfa, 0xef,
                0x0e, 0x40, 0x87, 0xf1, 0x57, 0x94, 0x42, 0x96
            }
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm",
            1,
            {
                0xa8, 0x6e, 0x1c, 0x63, 0xe1, 0xa6, 0xfd, 0xed,
                0x2f, 0x73, 0x17, 0xca, 0x97, 0xad, 0x48, 0x07,
                0x99, 0xf5, 0xcf, 0x84, 0xad, 0x9f, 0x4a, 0x16,
                0x66, 0x38, 0x09, 0xb7, 0x74, 0xe0, 0x58, 0x34
            },
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560-000000.wrm",
            1,
            {
                0xfc, 0x06, 0xf3, 0x0f, 0xc8, 0x3d, 0x16, 0x9f,
                0xa1, 0x64, 0xb8, 0xca, 0x0f, 0xf3, 0x85, 0xf0,
                0x22, 0x09, 0xaf, 0xfc, 0x0c, 0xe0, 0x76, 0x13,
                0x46, 0x62, 0xff, 0x55, 0xcb, 0x41, 0x87, 0x6a
            }
        }
    };

    for (auto & k: keys){
        if ((k.scheme == oldscheme)
        && (k.base.length() == static_cast<size_t>(len))
        && (strncmp(k.base.c_str(), base, static_cast<size_t>(len)) == 0))
        {
            if (oldscheme){
                LOG(LOG_INFO, "old key (derived from main master)");
            }
            else {
                LOG(LOG_INFO, "new key (derived master to use as master)");
            }
            memcpy(buffer, k.derived_key, 32);
//            hexdump_d(buffer, 32);
            return 0;
        }
    }
    memset(buffer, 0, 32);
    LOG(LOG_INFO, "key not found for base=%*s", len, base);
    hexdump_d(buffer, 32);
    return 0;
}



BOOST_AUTO_TEST_CASE(TestDecrypterEncrypted)
{
    LOG(LOG_INFO, "=================== TestDecrypterEncrypted =============");
    char const * argv[] {
        "decrypter.py", "reddec",
        "-i", FIXTURES_PATH "/verifier/recorded/"
        "cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm",
//        "--hash-path", FIXTURES_PATH "/verifier/hash/",
//        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o", "/tmp/out0.txt",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestDecrypterEncrypted1)
{
    LOG(LOG_INFO, "=================== TestDecrypterEncrypted1 =============");

    char const * argv[] {
        "decrypter.py", "reddec",
        "-i", FIXTURES_PATH "/verifier/recorded/"
        "cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm",
        "-o", "/tmp/out8.txt",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestDecrypterMigratedEncrypted)
{
    LOG(LOG_INFO, "=================== TestDecrypterMigratedEncrypted =============");
    // verifier.py redver -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm --hash-path ./tests/fixtures/verifier/hash --mwrm-path ./tests/fixtures/verifier/recorded/ --verbose 10


    char const * argv[] {
        "decrypter.py", "reddec",
        "-i", FIXTURES_PATH "/verifier/recorded/" "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm",
//        "--hash-path", FIXTURES_PATH "/verifier/hash/",
//        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o", "/tmp/out.txt",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestDecrypterMigratedEncrypted2)
{
    LOG(LOG_INFO, "=================== TestDecrypterMigratedEncrypted =============");
    // verifier.py redver -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm --hash-path ./tests/fixtures/verifier/hash --mwrm-path ./tests/fixtures/verifier/recorded/ --verbose 10


    char const * argv[] {
        "decrypter.py", "reddec",
        "-i", FIXTURES_PATH "/verifier/recorded/" "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm",
//        "--hash-path", FIXTURES_PATH "/verifier/hash/",
//        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o", "/tmp/out2.txt",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}


BOOST_AUTO_TEST_CASE(TestVerifierMigratedEncrypted)
{
    LOG(LOG_INFO, "=================== TestVerifierMigratedEncrypted =============");
    // verifier.py redver -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm --hash-path ./tests/fixtures/verifier/hash --mwrm-path ./tests/fixtures/verifier/recorded/ --verbose 10


    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(1, res);
}

// "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-134039,wab-4-2-4.yourdomain,4714.mwrm".
// verify as 1

BOOST_AUTO_TEST_CASE(TestVerifier4714)
{
    LOG(LOG_INFO, "=================== TestVerifier4714 =============");

    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-134039,wab-4-2-4.yourdomain,4714.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(-1, res);
}


//python -O /opt/wab/bin/verifier.py -i cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-164758,wab-4-2-4.yourdomain,7192.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-164758,wab-4-2-4.yourdomain,7192.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

BOOST_AUTO_TEST_CASE(TestVerifier7192)
{
    LOG(LOG_INFO, "=================== TestVerifier7192 =============");

    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-164758,wab-4-2-4.yourdomain,7192.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}


//python -O /opt/wab/bin/verifier.py -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-165619,wab-4-2-4.yourdomain,2510.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-165619,wab-4-2-4.yourdomain,2510.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

BOOST_AUTO_TEST_CASE(TestVerifier2510)
{
    LOG(LOG_INFO, "=================== TestVerifier2510 =============");

    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-165619,wab-4-2-4.yourdomain,2510.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}


//python -O /opt/wab/bin/verifier.py -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-181703,wab-4-2-4.yourdomain,6759.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-181703,wab-4-2-4.yourdomain,6759.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

//python -O /opt/wab/bin/verifier.py -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-184533,wab-4-2-4.yourdomain,1359.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-184533,wab-4-2-4.yourdomain,1359.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

//python -O /opt/wab/bin/verifier.py -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-191724,wab-4-2-4.yourdomain,6734.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-191724,wab-4-2-4.yourdomain,6734.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-191826,wab-4-2-4.yourdomain,9485.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-191826,wab-4-2-4.yourdomain,9485.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-191826,wab-4-2-4.yourdomain,9485.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-191826,wab-4-2-4.yourdomain,9485.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm".
//Input file is encrypted.
//verifier: -- get_trace_key_cb:::::(new scheme)
//verifier: -- /* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x50\xaf\x99\x03\x00\x00\x00\x00" //........P.......
//verifier: -- /* 0010 */ "\x80\x9a\x8a\x68\x77\x6f\x00\x00\xd6\xd6\x63\x68\x77\x6f\x00\x00" //...hwo....chwo..
//verifier: -- derivator used:::::(new scheme)
//verifier: -- /* 0000 */ "\x63\x67\x72\x6f\x73\x6a\x65\x61\x6e\x40\x31\x30\x2e\x31\x30\x2e" //cgrosjean@10.10.
//verifier: -- /* 0010 */ "\x34\x33\x2e\x31\x33\x2c\x70\x72\x6f\x78\x79\x75\x73\x65\x72\x40" //43.13,proxyuser@
//verifier: -- /* 0020 */ "\x77\x69\x6e\x32\x30\x30\x38\x2c\x32\x30\x31\x36\x31\x30\x32\x35" //win2008,20161025
//verifier: -- /* 0030 */ "\x2d\x31\x39\x32\x33\x30\x34\x2c\x77\x61\x62\x2d\x34\x2d\x32\x2d" //-192304,wab-4-2-
//verifier: -- /* 0040 */ "\x34\x2e\x79\x6f\x75\x72\x64\x6f\x6d\x61\x69\x6e\x2c\x35\x35\x36" //4.yourdomain,556
//verifier: -- /* 0050 */ "\x30\x2e\x6d\x77\x72\x6d"                                         //0.mwrm
//verifier: ERR (16506/16506) -- [CRYPTO_ERROR][16506]: Could not finish decryption!
//verifier: -- get_trace_key_cb:::::(old scheme)
//verifier: -- /* 0000 */ "\xa8\x6e\x1c\x63\xe1\xa6\xfd\xed\x2f\x73\x17\xca\x97\xad\x48\x07" //.n.c..../s....H.
//verifier: -- /* 0010 */ "\x99\xf5\xcf\x84\xad\x9f\x4a\x16\x66\x38\x09\xb7\x74\xe0\x58\x34" //......J.f8..t.X4
//verifier: -- derivator used:::::(old scheme)
//verifier: -- /* 0000 */ "\x63\x67\x72\x6f\x73\x6a\x65\x61\x6e\x40\x31\x30\x2e\x31\x30\x2e" //cgrosjean@10.10.
//verifier: -- /* 0010 */ "\x34\x33\x2e\x31\x33\x2c\x70\x72\x6f\x78\x79\x75\x73\x65\x72\x40" //43.13,proxyuser@
//verifier: -- /* 0020 */ "\x77\x69\x6e\x32\x30\x30\x38\x2c\x32\x30\x31\x36\x31\x30\x32\x35" //win2008,20161025
//verifier: -- /* 0030 */ "\x2d\x31\x39\x32\x33\x30\x34\x2c\x77\x61\x62\x2d\x34\x2d\x32\x2d" //-192304,wab-4-2-
//verifier: -- /* 0040 */ "\x34\x2e\x79\x6f\x75\x72\x64\x6f\x6d\x61\x69\x6e\x2c\x35\x35\x36" //4.yourdomain,556
//verifier: -- /* 0050 */ "\x30\x2e\x6d\x77\x72\x6d"                                         //0.mwrm
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm".
//verifier: -- get_trace_key_cb:::::(old scheme)
//verifier: -- /* 0000 */ "\xa8\x6e\x1c\x63\xe1\xa6\xfd\xed\x2f\x73\x17\xca\x97\xad\x48\x07" //.n.c..../s....H.
//verifier: -- /* 0010 */ "\x99\xf5\xcf\x84\xad\x9f\x4a\x16\x66\x38\x09\xb7\x74\xe0\x58\x34" //......J.f8..t.X4
//verifier: -- derivator used:::::(old scheme)
//verifier: -- /* 0000 */ "\x63\x67\x72\x6f\x73\x6a\x65\x61\x6e\x40\x31\x30\x2e\x31\x30\x2e" //cgrosjean@10.10.
//verifier: -- /* 0010 */ "\x34\x33\x2e\x31\x33\x2c\x70\x72\x6f\x78\x79\x75\x73\x65\x72\x40" //43.13,proxyuser@
//verifier: -- /* 0020 */ "\x77\x69\x6e\x32\x30\x30\x38\x2c\x32\x30\x31\x36\x31\x30\x32\x35" //win2008,20161025
//verifier: -- /* 0030 */ "\x2d\x31\x39\x32\x33\x30\x34\x2c\x77\x61\x62\x2d\x34\x2d\x32\x2d" //-192304,wab-4-2-
//verifier: -- /* 0040 */ "\x34\x2e\x79\x6f\x75\x72\x64\x6f\x6d\x61\x69\x6e\x2c\x35\x35\x36" //4.yourdomain,556
//verifier: -- /* 0050 */ "\x30\x2e\x6d\x77\x72\x6d"                                         //0.mwrm
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//verifier: -- get_trace_key_cb:::::(old scheme)
//verifier: -- /* 0000 */ "\xa8\x6e\x1c\x63\xe1\xa6\xfd\xed\x2f\x73\x17\xca\x97\xad\x48\x07" //.n.c..../s....H.
//verifier: -- /* 0010 */ "\x99\xf5\xcf\x84\xad\x9f\x4a\x16\x66\x38\x09\xb7\x74\xe0\x58\x34" //......J.f8..t.X4
//verifier: -- derivator used:::::(old scheme)
//verifier: -- /* 0000 */ "\x63\x67\x72\x6f\x73\x6a\x65\x61\x6e\x40\x31\x30\x2e\x31\x30\x2e" //cgrosjean@10.10.
//verifier: -- /* 0010 */ "\x34\x33\x2e\x31\x33\x2c\x70\x72\x6f\x78\x79\x75\x73\x65\x72\x40" //43.13,proxyuser@
//verifier: -- /* 0020 */ "\x77\x69\x6e\x32\x30\x30\x38\x2c\x32\x30\x31\x36\x31\x30\x32\x35" //win2008,20161025
//verifier: -- /* 0030 */ "\x2d\x31\x39\x32\x33\x30\x34\x2c\x77\x61\x62\x2d\x34\x2d\x32\x2d" //-192304,wab-4-2-
//verifier: -- /* 0040 */ "\x34\x2e\x79\x6f\x75\x72\x64\x6f\x6d\x61\x69\x6e\x2c\x35\x35\x36" //4.yourdomain,556
//verifier: -- /* 0050 */ "\x30\x2e\x6d\x77\x72\x6d"                                         //0.mwrm
//No error detected during the data verification.

//verify ok (4)


//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-164758,wab-4-2-4.yourdomain,7192.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-191826,wab-4-2-4.yourdomain,9485.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161026-132131,wab-4-2-4.yourdomain,9904.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-165619,wab-4-2-4.yourdomain,2510.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-181703,wab-4-2-4.yourdomain,6759.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-184533,wab-4-2-4.yourdomain,1359.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-191724,wab-4-2-4.yourdomain,6734.mwrm
//tests/fixtures/verifier/recorded/cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm
//tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,wab-5-0-0.yourdomain,7681.mwrm
//tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm
//tests/fixtures/verifier/recorded/v1_nochecksum_nocrypt.mwrm
//tests/fixtures/verifier/recorded/v2_nochecksum_nocrypt.mwrm

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm".
//Input file is encrypted.
//verifier: -- get_trace_key_cb:::::(new scheme)
//verifier: -- /* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\xd0\xd3\x71\x01\x00\x00\x00\x00" //..........q.....
//verifier: -- /* 0010 */ "\x80\xea\x97\x41\xfa\x72\x00\x00\xd6\x26\x71\x41\xfa\x72\x00\x00" //...A.r...&qA.r..
//verifier: -- derivator used:::::(new scheme)
//verifier: -- /* 0000 */ "\x63\x67\x72\x6f\x73\x6a\x65\x61\x6e\x40\x31\x30\x2e\x31\x30\x2e" //cgrosjean@10.10.
//verifier: -- /* 0010 */ "\x34\x33\x2e\x31\x33\x2c\x70\x72\x6f\x78\x79\x75\x73\x65\x72\x40" //43.13,proxyuser@
//verifier: -- /* 0020 */ "\x6c\x6f\x63\x61\x6c\x40\x77\x69\x6e\x32\x30\x30\x38\x2c\x32\x30" //local@win2008,20
//verifier: -- /* 0030 */ "\x31\x36\x31\x30\x32\x35\x2d\x32\x31\x33\x31\x35\x33\x2c\x77\x61" //161025-213153,wa
//verifier: -- /* 0040 */ "\x62\x2d\x34\x2d\x32\x2d\x34\x2e\x79\x6f\x75\x72\x64\x6f\x6d\x61" //b-4-2-4.yourdoma
//verifier: -- /* 0050 */ "\x69\x6e\x2c\x33\x32\x34\x33\x2e\x6d\x77\x72\x6d"                 //in,3243.mwrm
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (4)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-131957,wab-4-2-4.yourdomain,1914.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-131957,wab-4-2-4.yourdomain,1914.mwrm".
//Input file is unencrypted.
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-131957,wab-4-2-4.yourdomain,1914.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (4)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161026-132131,wab-4-2-4.yourdomain,9904.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161026-132131,wab-4-2-4.yourdomain,9904.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161026-132131,wab-4-2-4.yourdomain,9904.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//File "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161026-132131,wab-4-2-4.yourdomain,9904.mwrm" is invalid!

//verify failed

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161115-160802,wab-4-2-4.yourdomain,9484.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161115-160802,wab-4-2-4.yourdomain,9484.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161115-160802,wab-4-2-4.yourdomain,9484.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161115-160933,wab-4-2-4.yourdomain,5749.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161115-160933,wab-4-2-4.yourdomain,5749.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161115-160933,wab-4-2-4.yourdomain,5749.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161031,wab-4-2-4.yourdomain,1101.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161031,wab-4-2-4.yourdomain,1101.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161031,wab-4-2-4.yourdomain,1101.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161323,wab-4-2-4.yourdomain,7569.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161323,wab-4-2-4.yourdomain,7569.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161323,wab-4-2-4.yourdomain,7569.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

// python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161654,wab-4-2-4.yourdomain,6669.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161654,wab-4-2-4.yourdomain,6669.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-161654,wab-4-2-4.yourdomain,6669.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

// python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-163245,wab-4-2-4.yourdomain,7749.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-163245,wab-4-2-4.yourdomain,7749.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161115-163245,wab-4-2-4.yourdomain,7749.mwrm".
//verifier: INFO -- get_hmac_key_cb:::::
//verifier: INFO -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: INFO -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161201-155259,wab-4-2-4.yourdomain,8299.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161201-155259,wab-4-2-4.yourdomain,8299.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161201-155259,wab-4-2-4.yourdomain,8299.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (3)

//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm".
//Input file is encrypted.
//verifier: -- get_trace_key_cb:::::(new scheme)
//verifier: -- /* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\xd0\xda\xa8\x03\x00\x00\x00\x00" //................
//verifier: -- /* 0010 */ "\x80\x5a\xa5\xbe\x80\x77\x00\x00\xd6\x96\x7e\xbe\x80\x77\x00\x00" //.Z...w....~..w..
//verifier: -- derivator used:::::(new scheme)
//verifier: -- /* 0000 */ "\x63\x67\x72\x6f\x73\x6a\x65\x61\x6e\x40\x31\x30\x2e\x31\x30\x2e" //cgrosjean@10.10.
//verifier: -- /* 0010 */ "\x34\x33\x2e\x31\x33\x2c\x70\x72\x6f\x78\x79\x75\x73\x65\x72\x40" //43.13,proxyuser@
//verifier: -- /* 0020 */ "\x6c\x6f\x63\x61\x6c\x40\x77\x69\x6e\x32\x30\x30\x38\x2c\x32\x30" //local@win2008,20
//verifier: -- /* 0030 */ "\x31\x36\x31\x32\x30\x31\x2d\x31\x36\x33\x32\x30\x33\x2c\x77\x61" //161201-163203,wa
//verifier: -- /* 0040 */ "\x62\x2d\x34\x2d\x32\x2d\x34\x2e\x79\x6f\x75\x72\x64\x6f\x6d\x61" //b-4-2-4.yourdoma
//verifier: -- /* 0050 */ "\x69\x6e\x2c\x31\x30\x34\x36\x2e\x6d\x77\x72\x6d"                 //in,1046.mwrm
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//No error detected during the data verification.

//verify ok (4)

BOOST_AUTO_TEST_CASE(TestVerifier1914MigratedNocryptHasChecksum)
{
    LOG(LOG_INFO, "=================== TestVerifier1914MigratedNocryptHasChecksum =============");

    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-131957,wab-4-2-4.yourdomain,1914.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(0, res);
}

// cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-132156,wab-4-2-4.yourdomain,9904.mwrm
//python -O /opt/wab/bin/verifier.py -i /var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-132156,wab-4-2-4.yourdomain,9904.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-132156,wab-4-2-4.yourdomain,9904.mwrm".
//Input file is unencrypted.
//Input file don't include checksum
//hash file path: "/var/wab/hash/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-132156,wab-4-2-4.yourdomain,9904.mwrm".
//verifier: -- get_hmac_key_cb:::::
//verifier: -- /* 0000 */ "\x56\xdd\xb2\x92\x47\xbe\x4b\x89\x1f\x12\x62\x39\x0f\x10\xb9\x8e" //V...G.K...b9....
//verifier: -- /* 0010 */ "\xac\xff\xbc\x8a\x8f\x71\xfb\x21\x07\x7d\xef\x9c\xb3\x5f\xf9\x7b" //.....q.!.}..._.{
//File "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-132156,wab-4-2-4.yourdomain,9904.mwrm" is invalid!

//verify failed

BOOST_AUTO_TEST_CASE(TestVerifier9904NocryptNochecksumV2Statinfo)
{
    LOG(LOG_INFO, "=================== TestVerifier9904NocryptNochecksumStatinfo =============");

    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-132156,wab-4-2-4.yourdomain,9904.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    BOOST_CHECK_EQUAL(true, true);

    int res = -1;
    BOOST_CHECK_NO_THROW(res = do_main(argc, argv, hmac_2016_fn, trace_20161025_fn));
    BOOST_CHECK_EQUAL(1, res);
}

BOOST_AUTO_TEST_CASE(TestAppRecorder)
{
   LOG(LOG_INFO, "=================== TestAppRecorder =============");
     char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o",
            "/tmp/recorder.1.flva",
        "--flv",
        "--full",
        "--flvbreakinterval", "500",
        "--video-codec", "flv"
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = do_main(argc, argv, hmac_fn, trace_fn);
    BOOST_CHECK_EQUAL(0, res);

    const char * filename;
    filename = "/tmp/recorder.1-000000.flv";
    BOOST_CHECK_EQUAL(6008654, filesize(filename));
    ::unlink(filename);
    filename = "/tmp/recorder.1-000001.flv";
    BOOST_CHECK_EQUAL(717756, filesize(filename));
    ::unlink(filename);
    filename = "/tmp/recorder.1.flv";
    BOOST_CHECK_EQUAL(6724388, filesize(filename));
    ::unlink(filename);
}
