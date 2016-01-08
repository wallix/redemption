#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCryptoMetaTransport
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH
#define LOGNULL

#include "crypto_in_meta_sequence_transport.hpp"
#include "crypto_out_meta_sequence_transport.hpp"
#include "sequence_generator.hpp"

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

    Inifile ini;
    ini.set_value("crypto", "key0", 
                  "\x00\x01\x02\x03\x04\x05\x06\x07"
                  "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                  "\x10\x11\x12\x13\x14\x15\x16\x17"
                  "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");
    ini.set_value("crypto", "key1", "12345678901234567890123456789012");

    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);
//    memset(&cctx, 0, sizeof(cctx));
    cctx.set_crypto_key(
       "\x00\x01\x02\x03\x04\x05\x06\x07"
       "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
       "\x10\x11\x12\x13\x14\x15\x16\x17"
       "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");

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
    }

    {
        CryptoInMetaSequenceTransport crypto_trans(&cctx, "TESTOFS", ".mwrm");

        char buffer[1024] = {};
        char * bob = buffer;
        char ** pbuffer = &bob;
        crypto_trans.recv(pbuffer, 15);
        BOOST_CHECK_EQUAL(15, *pbuffer - buffer);

        if (0 != memcmp(buffer, "AAAAXBBBBXCCCCX", 15)){
            BOOST_CHECK_EQUAL(0, buffer[15]); // this one should not have changed
            buffer[15] = 0;
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
    ini.set_value("crypto", "key0", 
                  "\x00\x01\x02\x03\x04\x05\x06\x07"
                  "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                  "\x10\x11\x12\x13\x14\x15\x16\x17"
                  "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");
    ini.set_value("crypto", "key1", "12345678901234567890123456789012");

    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);
    cctx.set_crypto_key(
       "\x00\x01\x02\x03\x04\x05\x06\x07"
       "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
       "\x10\x11\x12\x13\x14\x15\x16\x17"
       "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");

    try {
        CryptoInMetaSequenceTransport(&cctx, "TESTOFSXXX", ".mwrm");
        BOOST_CHECK(false); // check open fails if file does not exist
    } catch (Error & e) {
        if (e.id != ERR_TRANSPORT_OPEN_FAILED) {
            BOOST_CHECK(false); // check open fails if file does not exist
        }
    }
}
