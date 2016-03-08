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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestInMetaSequenceTransport
#include <boost/test/auto_unit_test.hpp>

//#define LOGNULL
#define LOGPRINT

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#include "transport/out_meta_sequence_transport.hpp"
#include "transport/sequence_generator.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "error.hpp"

    class ifile_buf
    {
    public:
        CryptoContext * cctx;
        int cfb_file_fd;
        char           cfb_decrypt_buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX cfb_decrypt_ectx;                    // [en|de]cryption context
        uint32_t       cfb_decrypt_pos;                     // current position in buf
        uint32_t       cfb_decrypt_raw_size;                // the unciphered/uncompressed file size
        uint32_t       cfb_decrypt_state;                   // enum crypto_file_state
        unsigned int   cfb_decrypt_MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        int encryption;

        int cfb_decrypt_decrypt_open(unsigned char * trace_key)
        {
            ::memset(this->cfb_decrypt_buf, 0, sizeof(this->cfb_decrypt_buf));
            ::memset(&this->cfb_decrypt_ectx, 0, sizeof(this->cfb_decrypt_ectx));

            this->cfb_decrypt_pos = 0;
            this->cfb_decrypt_raw_size = 0;
            this->cfb_decrypt_state = 0;
            const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
            this->cfb_decrypt_MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

            unsigned char tmp_buf[40];
            const ssize_t err = this->cfb_file_read(tmp_buf, 40);
            if (err != 40) {
                return err < 0 ? err : -1;
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

            ::EVP_CIPHER_CTX_init(&this->cfb_decrypt_ectx);
            if(::EVP_DecryptInit_ex(&this->cfb_decrypt_ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                return -1;
            }

            return 0;
        }

        ssize_t cfb_decrypt_decrypt_read(void * data, size_t len)
        {
            if (this->cfb_decrypt_state & CF_EOF) {
                //printf("cf EOF\n");
                return 0;
            }

            unsigned int requested_size = len;

            while (requested_size > 0) {
                // Check how much we have decoded
                if (!this->cfb_decrypt_raw_size) {
                    // Buffer is empty. Read a chunk from file
                    /*
                     i f (-1 == ::do_chunk_read*(this)) {
                         return -1;
                }
                */
                    // TODO: avoid reading size directly into an integer, performance enhancement is minimal
                    // and it's not portable because of endianness issue => read in a buffer and decode by hand
                    unsigned char tmp_buf[4] = {};
                    const int err = this->cfb_file_read(tmp_buf, 4);
                    if (err != 4) {
                        return err < 0 ? err : -1;
                    }

                    uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);

                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->cfb_decrypt_state |= CF_EOF;
                        this->cfb_decrypt_pos = 0;
                        this->cfb_decrypt_raw_size = 0;
                    }
                    else {
                        if (ciphered_buf_size > this->cfb_decrypt_MAX_CIPHERED_SIZE) {
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                            return -1;
                        }
                        else {
                            uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                            //char ciphered_buf[ciphered_buf_size];
                            unsigned char ciphered_buf[65536];
                            //char compressed_buf[compressed_buf_size];
                            unsigned char compressed_buf[65536];

                            ssize_t err = this->cfb_file_read(
                                                    ciphered_buf,
                                                    ciphered_buf_size);

                            if (err != ssize_t(len)){
                                return err < 0 ? err : -1;
                            }

                            if (this->cfb_decrypt_xaes_decrypt(ciphered_buf,
                                            ciphered_buf_size,
                                            compressed_buf,
                                            &compressed_buf_size)) {
                                return -1;
                            }

                            size_t chunk_size = CRYPTO_BUFFER_SIZE;
                            const snappy_status status = snappy_uncompress(
                                    reinterpret_cast<char *>(compressed_buf),
                                    compressed_buf_size, this->cfb_decrypt_buf, &chunk_size);

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

                            this->cfb_decrypt_pos = 0;
                            // When reading, raw_size represent the current chunk size
                            this->cfb_decrypt_raw_size = chunk_size;
                        }
                    }

                    // TODO: check that
                    if (!this->cfb_decrypt_raw_size) { // end of file reached
                        break;
                    }
                }
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->cfb_decrypt_raw_size - this->cfb_decrypt_pos;
                // Check how much we can copy
                unsigned int copiable_size = MIN(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(static_cast<char*>(data) + (len - requested_size), this->cfb_decrypt_buf + this->cfb_decrypt_pos, copiable_size);
                this->cfb_decrypt_pos      += copiable_size;
                requested_size -= copiable_size;
                // Check if we reach the end
                if (this->cfb_decrypt_raw_size == this->cfb_decrypt_pos) {
                    this->cfb_decrypt_raw_size = 0;
                }
            }
            return len - requested_size;
        }

    private:

        int cfb_decrypt_xaes_decrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
        {
            int safe_size = *dst_sz;
            int remaining_size = 0;

            /* allows reusing of ectx for multiple encryption cycles */
            if (EVP_DecryptInit_ex(&this->cfb_decrypt_ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                return -1;
            }
            if (EVP_DecryptUpdate(&this->cfb_decrypt_ectx, dst_buf, &safe_size, src_buf, src_sz) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                return -1;
            }
            if (EVP_DecryptFinal_ex(&this->cfb_decrypt_ectx, dst_buf + safe_size, &remaining_size) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                return -1;
            }
            *dst_sz = safe_size + remaining_size;
            return 0;
        }

        int cfb_file_open(const char * filename, mode_t /*mode*/)
        {
            TODO("see why mode is ignored even if it's provided as a parameter?");
            this->cfb_file_close();
            this->cfb_file_fd = ::open(filename, O_RDONLY);
            return this->cfb_file_fd;
        }

        int cfb_file_close()
        {
            if (this->is_open()) {
                const int ret = ::close(this->cfb_file_fd);
                this->cfb_file_fd = -1;
                return ret;
            }
            return 0;
        }

        bool cfb_file_is_open() const noexcept
        { return -1 != this->cfb_file_fd; }

        ssize_t cfb_file_read(void * data, size_t len)
        {
            TODO("this is blocking read, add support for timeout reading");
            TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
            size_t remaining_len = len;
            while (remaining_len) {
                ssize_t ret = ::read(this->cfb_file_fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                if (ret < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Error should still be there next time we try to read
                    if (remaining_len != len){
                        return len - remaining_len;
                    }
                    return ret;
                }
                // We must exit loop or we will enter infinite loop
                if (ret == 0){
                    break;
                }
                remaining_len -= ret;
            }
            return len - remaining_len;
        }


    public:
        explicit ifile_buf(CryptoContext * cctx, int encryption)
        : cctx(cctx)
        , cfb_file_fd(-1)
        , encryption(encryption)
        {}

        ~ifile_buf()
        {
            this->cfb_file_close();
        }

        int open(const char * filename, mode_t mode = 0600)
        {
            if (this->encryption){

                int err = this->cfb_file_open(filename, mode);
                if (err < 0) {
                    return err;
                }

                unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
                size_t base_len = 0;
                const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));
                this->cctx->get_derived_key(trace_key, base, base_len);

                return this->cfb_decrypt_decrypt_open(trace_key);
            }
            else {
                return this->cfb_file_open(filename, mode);
            }
        }

        ssize_t read(void * data, size_t len)
        {
            if (this->encryption){
                return this->cfb_decrypt_decrypt_read(data, len);
            }
            else {
                return this->cfb_file_read(data, len);
            }
        }

        int close()
        {
            return this->cfb_file_close();
        }

        bool is_open() const noexcept
        {
            return this->cfb_file_is_open();
        }
    };

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr),
        "./tests/fixtures/sample", ".mwrm", 0, 0);
    char buffer[10000];
    char * pbuffer = buffer;
    size_t total = 0;
    try {
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv(&pbuffer, sizeof(buffer));
            total += pbuffer - buffer;
        }
    } catch (const Error & e) {
        BOOST_CHECK_EQUAL(ERR_TRANSPORT_NO_MORE_DATA, e.id);
        total += pbuffer - buffer;
    };
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(1471394 + 444578 + 290245, total);
}

BOOST_AUTO_TEST_CASE(TestMetav2)
{
    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));


    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    ifile_buf ifile(&cctx, 0);

    ifile.open("./tests/fixtures/sample_v2.mwrm");

    struct ReaderBuf
    {
        ifile_buf & buf;

        ssize_t reader_read(char * buf, size_t len) const {
            return this->buf.read(buf, len);
        }
    };

    detail::ReaderLine<ReaderBuf> reader({ifile});
    auto meta_header = detail::read_meta_headers(reader);
    BOOST_REQUIRE_EQUAL(meta_header.version, 2);
    BOOST_CHECK_EQUAL(meta_header.has_checksum, false);

    detail::MetaLine meta_line;
    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
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

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 9);
    BOOST_CHECK_EQUAL(meta_line.mode, 8);
    BOOST_CHECK_EQUAL(meta_line.uid, 10);
    BOOST_CHECK_EQUAL(meta_line.gid, 11);
    BOOST_CHECK_EQUAL(meta_line.dev, 12);
    BOOST_CHECK_EQUAL(meta_line.ino, 13);
    BOOST_CHECK_EQUAL(meta_line.mtime, 14);
    BOOST_CHECK_EQUAL(meta_line.ctime, 15);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304870);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304930);

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 16);
    BOOST_CHECK_EQUAL(meta_line.mode, 17);
    BOOST_CHECK_EQUAL(meta_line.uid, 18);
    BOOST_CHECK_EQUAL(meta_line.gid, 19);
    BOOST_CHECK_EQUAL(meta_line.dev, 20);
    BOOST_CHECK_EQUAL(meta_line.ino, 21);
    BOOST_CHECK_EQUAL(meta_line.mtime, 22);
    BOOST_CHECK_EQUAL(meta_line.ctime, 23);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304930);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304990);

    BOOST_CHECK_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), ERR_TRANSPORT_NO_MORE_DATA);
}

BOOST_AUTO_TEST_CASE(TestMetav2sum)
{
    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));


    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    ifile_buf ifile(&cctx, 0);
    ifile.open("./tests/fixtures/sample_v2_checksum.mwrm");

    struct ReaderBuf
    {
        ifile_buf & buf;

        ssize_t reader_read(char * buf, size_t len) const {
            return this->buf.read(buf, len);
        }
    };

    detail::ReaderLine<ReaderBuf> reader({ifile});
    auto meta_header = detail::read_meta_headers(reader);
    BOOST_REQUIRE_EQUAL(meta_header.version, 2);
    BOOST_CHECK_EQUAL(meta_header.has_checksum, true);

    using std::begin;
    using std::end;

    struct is_char {
        unsigned char c;
        bool operator()(unsigned char c) const {
            return this->c == c;
        }
    };

    detail::MetaLine meta_line;
    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
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
    BOOST_CHECK(std::all_of(begin(meta_line.hash1), end(meta_line.hash1), is_char{0xaa}));
    BOOST_CHECK(std::all_of(begin(meta_line.hash2), end(meta_line.hash2), is_char{0xbb}));

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 9);
    BOOST_CHECK_EQUAL(meta_line.mode, 8);
    BOOST_CHECK_EQUAL(meta_line.uid, 10);
    BOOST_CHECK_EQUAL(meta_line.gid, 11);
    BOOST_CHECK_EQUAL(meta_line.dev, 12);
    BOOST_CHECK_EQUAL(meta_line.ino, 13);
    BOOST_CHECK_EQUAL(meta_line.mtime, 14);
    BOOST_CHECK_EQUAL(meta_line.ctime, 15);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304870);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304930);
    BOOST_CHECK(std::all_of(begin(meta_line.hash1), end(meta_line.hash1), is_char{0xcc}));
    BOOST_CHECK(std::all_of(begin(meta_line.hash2), end(meta_line.hash2), is_char{0xdd}));

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 16);
    BOOST_CHECK_EQUAL(meta_line.mode, 17);
    BOOST_CHECK_EQUAL(meta_line.uid, 18);
    BOOST_CHECK_EQUAL(meta_line.gid, 19);
    BOOST_CHECK_EQUAL(meta_line.dev, 20);
    BOOST_CHECK_EQUAL(meta_line.ino, 21);
    BOOST_CHECK_EQUAL(meta_line.mtime, 22);
    BOOST_CHECK_EQUAL(meta_line.ctime, 23);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304930);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304990);
    BOOST_CHECK(std::all_of(begin(meta_line.hash1), end(meta_line.hash1), is_char{0xee}));
    BOOST_CHECK(std::all_of(begin(meta_line.hash2), end(meta_line.hash2), is_char{0xff}));

    BOOST_CHECK_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), ERR_TRANSPORT_NO_MORE_DATA);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_v2)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample_v2", ".mwrm", 0, 0);
    char buffer[10000];
    char * pbuffer = buffer;
    size_t total = 0;
    try {
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv(&pbuffer, sizeof(buffer));
            total += pbuffer - buffer;
        }
    } catch (const Error & e) {
        BOOST_CHECK_EQUAL(ERR_TRANSPORT_NO_MORE_DATA, e.id);
        total += pbuffer - buffer;
    };
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(1471394 + 444578 + 290245, total);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "./tests/fixtures/sample0.wrm 1352304810 1352304870\n",
//        "./tests/fixtures/sample1.wrm 1352304870 1352304930\n",
//        "./tests/fixtures/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm", 0, 0);
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        try {
            mwrm_trans.next();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL(ERR_TRANSPORT_NO_MORE_DATA, e.id);
        };
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm", 0, 0);

    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2_RIO)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "./tests/fixtures/sample0.wrm 1352304810 1352304870\n",
//        "./tests/fixtures/sample1.wrm 1352304870 1352304930\n",
//        "./tests/fixtures/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    try {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm", 0, 0);
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        try {
            mwrm_trans.next();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL(ERR_TRANSPORT_NO_MORE_DATA, e.id);
        };

    } catch(const Error & e) {
        BOOST_CHECK(false);
    };
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM3)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "/var/rdpproxy/recorded/sample0.wrm 1352304810 1352304870\n",
//        "/var/rdpproxy/recorded/sample1.wrm 1352304870 1352304930\n",
//        "/var/rdpproxy/recorded/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content

    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/moved_sample", ".mwrm", 0, 0);
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        try {
            mwrm_trans.next();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL(ERR_TRANSPORT_NO_MORE_DATA, e.id);
        };
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/moved_sample", ".mwrm", 0, 0);

    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

BOOST_AUTO_TEST_CASE(TestCryptoInmetaSequenceTransport)
{
    OpenSSL_add_all_digests();

    // cleanup of possible previous test files
    {
        const char * file[] = {"/tmp/TESTOFS.mwrm", "TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
            ::unlink(file[i]);
        }
    }

    BOOST_CHECK(true);

    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));


    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    BOOST_CHECK(true);

    {
        struct timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        const int groupid = 0;
        CryptoOutMetaSequenceTransport crypto_trans(&cctx, "", "/tmp/", "TESTOFS", tv, 800, 600, groupid,
                                                    nullptr, 0, FilenameGenerator::PATH_FILE_COUNT_EXTENSION);
        crypto_trans.send("AAAAX", 5);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        crypto_trans.next();
        crypto_trans.send("BBBBXCCCCX", 10);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        BOOST_CHECK(true);
    }

    {
        InMetaSequenceTransport crypto_trans(&cctx, "TESTOFS", ".mwrm", 1, 0);

        char buffer[1024] = {};
        char * bob = buffer;
        char ** pbuffer = &bob;

        BOOST_CHECK(true);

        try {
            crypto_trans.recv(pbuffer, 15);
        } catch (Error & e){
            BOOST_CHECK(false);
        };

        BOOST_CHECK(true);

        BOOST_CHECK_EQUAL(15, *pbuffer - buffer);

        if (0 != memcmp(buffer, "AAAAXBBBBXCCCCX", 15)){
            BOOST_CHECK_EQUAL(0, buffer[15]); // this one should not have changed
            buffer[15] = 0;
            BOOST_CHECK(true);
            LOG(LOG_ERR, "expected \"AAAAXBBBBXCCCCX\" got \"%s\"", buffer);
            BOOST_CHECK(false);
        }

        BOOST_CHECK(true);
    }

    const char * file[] = {
        "/tmp/TESTOFS.mwrm", // hash
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
        if (::unlink(file[i])){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(CryptoTestInMetaSequenceTransport2)
{

    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));

    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    try {
        InMetaSequenceTransport(&cctx, "TESTOFSXXX", ".mwrm", 1, 0);
        BOOST_CHECK(false); // check open fails if file does not exist
    } catch (Error & e) {
        if (e.id != ERR_TRANSPORT_OPEN_FAILED) {
            BOOST_CHECK(false); // check open fails if file does not exist
        }
    }
}


