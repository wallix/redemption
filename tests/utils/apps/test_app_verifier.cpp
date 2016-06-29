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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAppVerifier
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGPRINT
//#define LOGNULL

#include <fcntl.h>

#include <iostream>

#include "system/ssl_calls.hpp"
#include "utils/apps/app_verifier.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include "utils/genrandom.hpp"

#include <new>

#include "utils/fdbuf.hpp"
#include "utils/sugar/local_fd.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/cryptofile.hpp"

#ifdef IN_IDE_PARSER
#define FIXTURES_PATH ""
#endif

#ifdef HASH_LEN
#undef HASH_LEN
#endif  // #ifdef HASH_LEN
#define HASH_LEN 64

BOOST_AUTO_TEST_CASE(TestReverseIterators)
{

    // Show how to extract filename even if it cvontains spaces
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

BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash)
{
    const std::string test_mwrm_path = "./";
    const std::string test_file_name = "TestCheckFileHash";

    /************************
    * Manage encryption key *
    ************************/
    Inifile ini;
    ini.set<cfg::crypto::key0>(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    );
    ini.set<cfg::crypto::key1>("12345678901234567890123456789012");
    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

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
    uint8_t tmp[SHA256_DIGEST_LENGTH];
    sha256.final(tmp, SHA256_DIGEST_LENGTH);
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
    if (system_fd == -1){
        printf("failed opening=%s\n", full_test_file_name.c_str());
        BOOST_CHECK(false);
    }

    struct crypto_file
    {
      transfil::encrypt_filter encrypt;
      io::posix::fdbuf file;

      crypto_file(int fd)
      : file(fd)
      {}
    } * cf_struct = new (std::nothrow) crypto_file(system_fd);

    if (cf_struct) {
        if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, &cctx, iv)) {
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
        FileChecker check(test_full_mwrm_filename);
        check.check_hash_sha256(cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()), hash, HASH_LEN / 2, true);
        BOOST_CHECK_EQUAL(false, check.failed);
    }

    {
        FileChecker check(test_full_mwrm_filename);
        check.check_hash_sha256(cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()),
                                hash + (HASH_LEN / 2), HASH_LEN / 2, false);
        BOOST_CHECK_EQUAL(false, check.failed);
    }

    unlink(full_test_file_name.c_str());
}   /* BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash) */

// python tools/verifier.py -i toto@10.10.43.13\,Administrateur@QA@cible\,20160218-183009\,wab-5-0-0.yourdomain\,7335.mwrm --hash-path tests/fixtures/verifier/hash/ --mwrm-path tests/fixtures/verifier/recorded/ --verbose 10

extern "C" {
    int hmac_fn(char * buffer)
    {
        uint8_t hmac_key[32] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        memcpy(buffer, hmac_key, 32);
        return 0;
    }

    int trace_fn(char * base, int len, char * buffer)
    {
        // in real uses actual trace_key is derived from base and some master key
        uint8_t trace_key[32] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2
         };
        memcpy(buffer, trace_key, 32);
        return 0;
    }
}

BOOST_AUTO_TEST_CASE(TestVerifierEncryptedData)
{
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        char const * argv[] = {
            "verifier.py",
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
        try {
            res = app_verifier(ini,
                argc, argv
              , "ReDemPtion VERifier " VERSION ".\n"
                "Copyright (C) Wallix 2010-2016.\n"
                "Christophe Grosjean, Raphael Zhou."
              , cctx);
            if (res == 0){
                printf("verify ok\n");
            }
            else {
                printf("verify failed\n");
            }
        } catch (const Error & e) {
            printf("verify failed: with id=%d\n", e.id);
        }
        BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestVerifierClearData)
{
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        char const * argv[] {
            "verifier.py",
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


        int res = -1;
        try {
            res = app_verifier(ini,
                argc, argv
              , "ReDemPtion VERifier " VERSION ".\n"
                "Copyright (C) Wallix 2010-2016.\n"
                "Christophe Grosjean, Raphael Zhou."
              , cctx);
            if (res == 0){
                printf("verify ok\n");
            }
            else {
                printf("verify failed\n");
            }
        } catch (const Error & e) {
            printf("verify failed: with id=%d\n", e.id);
        }
        BOOST_CHECK_EQUAL(0, res);
}



struct MetaHeaderXXX {
    unsigned version;
    //unsigned witdh;
    //unsigned height;
    bool has_checksum;
};

struct ifile_read_API
{
    // ifile is a thin API layer over system open/read/close
    // it means open/read/close mimicks system open/read/close
    // (except fd is wrapped in an object)

    int fd;
    ifile_read() : fd(-1) {}
    // We choose to define an open function to mimick system behavior
    // instead of opening through constructor. This allows to manage
    // explicit error management depending on return code.
    virtual int open(const char * s);
    // read can either return the number of bytes asked or less.
    // That the exact number of bytes is returned is never 
    // guaranteed and checking that is at caller's responsibility
    // if some error occurs the return is -1 and the error code
    // is in errno, like for system calls.
    // returning 0 means EOF
    virtual int read(char * buf, size_t len);
    // close beside calling the system call must also ensure it sets fd to 1
    // this is to avoid performing close twice when called explicitely
    // as it is also performed by destructor (in most cases there will be
    // no reason for calling close explicitly).
    virtual void close();
    ~ifile_read(){
        if (this->fd != -1){
            this->close();
        }
    }
};

struct ifile_read : public ifile_read_API
{
    int open(const char * s)
    {
        this->fd = ::open(s, O_RDONLY);
        return this->fd;
    }
    int read(char * buf, size_t len)
    {
        return ::read(this->fd, buf, len);
    }
    void close()
    {
        ::close(fd);
        this->fd = -1;
    }
};

class MwrmReaderXXX
{
    public:
    MetaHeaderXXX header;
    
    private:
    char buf[1024];
    char * eof;
    char * eol;
    char * cur;
    ifile_read_API & ibuf;


    long long int get_ll(char * & cur, char * eol, char sep, int err)
    {
        char * pos = std::find(cur, eol, sep);
        if (pos == eol || (pos - cur < 1)){
            throw Error(err);
        }
        char * pend = nullptr;
        long long int res = strtoll(cur, &pend, 10);
        if (pend != pos){
            throw Error(err);
        }
        cur = pos + 1;
        return res;
    }

    void in_copy_bytes(uint8_t * hash, int len, char * & cur, char * eol, int err)
    {
        if (eol - cur < len){
            throw Error(err);
        }
        memcpy(hash, cur, len);
        cur += len;
    }

    void in_hex256(uint8_t * hash, int len, char * & cur, char * eol, char sep, int exc)
    {
        int err = 0;
        char * pos = std::find(cur, eol, sep);
        if (pos == eol || (pos - cur != 2*len)){
            throw Error(exc);
        }
        for (int i = 0 ; i < len ; i++){
            hash[i] = (chex_to_int(cur[i*2u], err)*16)
                     + chex_to_int(cur[i*2u+1], err);
        }
        if (err){
            throw Error(err);
        }
        cur = pos + 1;
    }

public:
    MwrmReaderXXX(ifile_read_API & reader_buf) noexcept
    : buf{}
    , eof(buf)
    , eol(buf)
    , cur(buf)
    , ibuf(reader_buf)
    {
        memset(this->buf, 0, sizeof(this->buf));
        printf("MwrmReader\n");
    }

    int read_meta_file2(MetaLine2 & meta_line) 
    {
        try {
            if (this->header.version == 1) {
                this->read_meta_file_v1(meta_line);
                return 0;
            }
            else {
                this->read_meta_file_v2(meta_line);
                return 0;
            }
        }
        catch(...){
            return 1;
        };
    }

    void read_meta_headers()
    {
        this->next_line(); // v2
        this->header.version = (this->cur[0] == 'v')?2:1;
        if (this->header.version == 2) {
            this->next_line(); // 800 600
            this->next_line(); // checksum or nochecksum
        }
        this->header.has_checksum = (header.version > 1) 
                                 && (this->cur[0] == 'c');
        // else v1
        this->next_line(); // blank
        this->next_line(); // blank
    }

    void read_meta_file_v1(MetaLine2 & meta_line)
    {
        this->next_line();
        size_t len = this->eol - this->cur;

        // Line format "fffff sssss eeeee hhhhh HHHHH"
        //                               ^  ^  ^  ^
        //                               |  |  |  |
        //                               |hash1|  |
        //                               |     |  |
        //                           space3    |hash2
        //                                     |
        //                                   space4
        //
        // filename(1 or >) + space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
        //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

        typedef std::reverse_iterator<char*> reverse_iterator;
        reverse_iterator first(this->cur);
        reverse_iterator space(this->cur+len);                
        for(int i = 0; i < 2 + 2*this->header.has_checksum; i++){
            space = std::find(space, first, ' ');                
            space++;
        }
        int path_len = first-space;
        this->in_copy_bytes(reinterpret_cast<uint8_t*>(meta_line.filename), path_len, this->cur, this->eol, ERR_TRANSPORT_READ_FAILED);
        this->cur++;
        meta_line.filename[path_len] = 0;

        // st_start_time + space
        meta_line.start_time = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_stop_time + space
        meta_line.stop_time = this->get_ll(cur, eol, this->header.has_checksum?' ':'\n',
                                           ERR_TRANSPORT_READ_FAILED);
        if (this->header.has_checksum){
            // HASH1 + space
            this->in_hex256(meta_line.hash1, MD_HASH_LENGTH, cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            // HASH1 + CR
            this->in_hex256(meta_line.hash2, MD_HASH_LENGTH, cur, eol, '\n', ERR_TRANSPORT_READ_FAILED);
        }

        // TODO: check the whole line has been consumed (or it's an error)
        this->cur = this->eol;
    }

    void read_meta_file_v2(MetaLine2 & meta_line) 
    {
        BOOST_CHECK(true);
        this->next_line();

        size_t len = this->eol - this->cur;

        BOOST_CHECK(true);

        // Line format "fffff
        // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
        // sssss eeeee hhhhh HHHHH"
        //            ^  ^  ^  ^
        //            |  |  |  |
        //            |hash1|  |
        //            |     |  |
        //        space11   |hash2
        //                  |
        //                space12
        //
        // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
        //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
        //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

        typedef std::reverse_iterator<char*> reverse_iterator;
        reverse_iterator first(this->cur);
        reverse_iterator space(this->cur+len);                
        for(int i = 0; i < 10 + 2*this->header.has_checksum; i++){
            space = std::find(space, first, ' ');                
            space++;
        }
        int path_len = first-space;
        this->in_copy_bytes(reinterpret_cast<uint8_t*>(meta_line.filename), path_len, this->cur, this->eol, ERR_TRANSPORT_READ_FAILED);
        this->cur++;
        meta_line.filename[path_len] = 0;

        // st_size + space
        meta_line.size = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_mode + space
        meta_line.mode = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_uid + space
        meta_line.uid = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_gid + space
        meta_line.gid = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_dev + space
        meta_line.dev = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_ino + space
        meta_line.ino = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_mtime + space
        meta_line.mtime = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_ctime + space
        meta_line.ctime = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);

        // st_start_time + space
        meta_line.start_time = this->get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        // st_stop_time + space
        meta_line.stop_time = this->get_ll(cur, eol, this->header.has_checksum?' ':'\n',
                                           ERR_TRANSPORT_READ_FAILED);
        if (this->header.has_checksum){
            // HASH1 + space
            this->in_hex256(meta_line.hash1, MD_HASH_LENGTH, cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            // HASH1 + CR
            this->in_hex256(meta_line.hash2, MD_HASH_LENGTH, cur, eol, '\n', ERR_TRANSPORT_READ_FAILED);
        }

        // TODO: check the whole line has been consumed (or it's an error)
        this->cur = this->eol;
    }

    void next_line()
    {
        this->cur = this->eol;
        while (this->cur == this->eof) // empty buffer
        {
            ssize_t ret = this->ibuf.read(this->buf, sizeof(this->buf)-1);
            if (ret < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            if (ret == 0) {
                break;
            }
            this->cur = this->buf;
            this->eof = this->buf + ret;
            this->eof[0] = 0;
        }
        char * pos = std::find(this->cur, this->eof, '\n');
        while (pos == this->eof){ // read and append to buffer
            size_t len = -(this->eof-this->cur);
            if (len >= sizeof(buf)-1){
                // if the buffer can't hold at least one line, 
                // there is some problem behind
                // if a line were available we should have found \n
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            ssize_t ret = this->ibuf.read(this->eof, sizeof(this->buf)-1-len);
            if (ret < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            if (ret == 0) {
                break;
            }
            this->eof += ret;
            this->eof[0] = 0;
            pos = std::find(this->cur, this->eof, '\n');
        }
        this->eol = (pos == this->eof)?this->eof:pos+1; // set eol after \n (start of next line)
    }
};

BOOST_AUTO_TEST_CASE(ReadClearHeaderV2)
{
    ifile_read fd;
    fd.open(FIXTURES_PATH "/verifier/recorded/v2_nochecksum_nocrypt.mwrm");
    MwrmReaderXXX reader(fd);
    
    reader.read_meta_headers();
    BOOST_CHECK(reader.header.version == 2);
    BOOST_CHECK(!reader.header.has_checksum);
    
    MetaLine2 meta_line;
    reader.read_meta_file_v2(meta_line);
    BOOST_CHECK(0 == strcmp(meta_line.filename,
                        "/var/wab/recorded/rdp/"
                        "toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,"
                        "wab-5-0-0.yourdomain,7681-000000.wrm"));
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
    ifile_read fd;
    fd.open(FIXTURES_PATH "/verifier/recorded/v1_nochecksum_nocrypt.mwrm");
    MwrmReaderXXX reader(fd);
    
    reader.read_meta_headers();
    BOOST_CHECK(reader.header.version == 1);
    BOOST_CHECK(!reader.header.has_checksum);
    
    MetaLine2 meta_line;
    reader.read_meta_file_v1(meta_line);
//    BOOST_CHECK(0 == strcmp(meta_line.filename,
//                        "/var/wab/recorded/rdp/"
//                        "toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,"
//                        "wab-5-0-0.yourdomain,7681-000000.wrm"));
//    BOOST_CHECK_EQUAL(meta_line.size, 181826); 
//    BOOST_CHECK_EQUAL(meta_line.mode, 33056);
//    BOOST_CHECK_EQUAL(meta_line.uid, 1001);
//    BOOST_CHECK_EQUAL(meta_line.gid, 1001);
//    BOOST_CHECK_EQUAL(meta_line.dev, 65030);
//    BOOST_CHECK_EQUAL(meta_line.ino, 81);
//    BOOST_CHECK_EQUAL(meta_line.mtime, 1455816421);
//    BOOST_CHECK_EQUAL(meta_line.ctime, 1455816421);
//    BOOST_CHECK_EQUAL(meta_line.start_time, 1455815820);
//    BOOST_CHECK_EQUAL(meta_line.stop_time, 1455816422);
}

BOOST_AUTO_TEST_CASE(ReadClearHeaderV2Checksum)
{
    ifile_read fd;
    fd.open(FIXTURES_PATH "/sample_v2_checksum.mwrm");
    MwrmReaderXXX reader(fd);
    
    reader.read_meta_headers();
    BOOST_CHECK(reader.header.version == 2);
    BOOST_CHECK(reader.header.has_checksum);
    
    MetaLine2 meta_line;
    reader.read_meta_file_v2(meta_line);
    BOOST_CHECK(true);
    BOOST_CHECK(0 == strcmp(meta_line.filename, "./tests/fixtures/sample0.wrm"));
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

