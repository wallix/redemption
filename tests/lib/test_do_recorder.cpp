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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "lib/do_recorder.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/algostring.hpp"
#include "transport/crypto_transport.hpp"
#include "test_only/ostream_buffered.hpp"

#include <iostream>

namespace
{

#ifndef REDEMPTION_NO_FFMPEG
struct Codec
{
    char const* name;
    char const* options;
};
constexpr char const* mp4_options{"profile=baseline preset=ultrafast b=80000"};
#endif

// E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE
uint8_t hmac_key[] = {
    0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
    0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
    0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
    0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };

int trace_fn(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme)
{
    // in real uses actual trace_key is derived from base and some master key
    (void)base;
    (void)len;
    (void)oldscheme;
    // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
    uint8_t trace_key[] = {
        0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
        0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
        0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
        0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2 };
    static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH );
    memcpy(buffer, trace_key, sizeof(trace_key));
    return 0;
}

} // anonymous namespace

#define TEST_DO_MAIN(argv, res_result, hmac_key, trace_fn, output, output_error) do { \
    int argc = sizeof(argv)/sizeof(char*);                                            \
    tu::ostream_buffered cout_buf;                                                    \
    tu::ostream_buffered cerr_buf(std::cerr);                                         \
    int res = do_main(argc, argv, hmac_key, trace_fn);                                \
    EVP_cleanup();                                                                    \
    RED_CHECK(cout_buf.str() == output);                                              \
    RED_CHECK(cerr_buf.str() == output_error);                                        \
    RED_TEST(res_result == res);                                                      \
} while (0)

// tests/fixtures/verifier/recorded/toto@10.10.43.13\,Administrateur@QA@cible\,20160218-181658\,wab-5-0-0.yourdomain\,7681.mwrm

// python tools/decrypter.py -i tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm -o decrypted.out

using EncryptionMode = InCryptoTransport::EncryptionMode;

RED_AUTO_TEST_CASE_WD(TestDecrypterEncryptedData, wd)
{
    auto output = wd.add_file("decrypted.out");
    char const * argv[] {
        "decrypter.py",
        "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        "-o",
            output.c_str(),
        "--verbose",
            "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Input file is encrypted.\nOutput file is \"", output, "\".\ndecrypt ok\n"),
        ""_av);
}

RED_AUTO_TEST_CASE_WD(TestDecrypterClearData, wd)
{
    auto output = wd.dirname().string() + "decrypted.2.out";
    char const * argv[] {
        "decrypter.py",
        "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
                "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "-o",
            output.c_str(),
        "--verbose",
            "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, "\".\nInput file is not encrypted.\n"),
        ""_av);
}

// python tools/verifier.py -i toto@10.10.43.13\,Administrateur@QA@cible\,20160218-183009\,wab-5-0-0.yourdomain\,7335.mwrm --hash-path tests/fixtures/verifier/hash/ --mwrm-path tests/fixtures/verifier/recorded/ --verbose 10


template<class Exception>
bool is_except( Exception const &  /*unused*/) { return true; }

RED_AUTO_TEST_CASE(TestVerifierFileNotFound)
{
    char const * argv[] = {
        "verifier.py",
        "redver",
        "-i", "asdfgfsghsdhds.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/bad",
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, -1, hmac_key, trace_fn,
        ""_av, "Input file error: No such file or directory\n"_av);
}

RED_AUTO_TEST_CASE(TestVerifierEncryptedDataFailure)
{
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

    TEST_DO_MAIN(argv, 1, hmac_key, trace_fn,
        "Input file is encrypted.\nverify failed\n"_av,
        "Error checking file \"" FIXTURES_PATH "/verifier/recorded/bad/"
            "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335-000000.wrm"
            "\" (invalid checksum)\n\n"_av);
}

RED_AUTO_TEST_CASE(TestVerifierEncryptedData)
{
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

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        "Input file is encrypted.\nNo error detected during the data verification.\n\nverify ok\n"_av,
        ""_av);
}

RED_AUTO_TEST_CASE(TestVerifierClearData)
{
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
            "10"
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        "No error detected during the data verification.\n\nverify ok\n"_av, ""_av);
}

namespace
{
    uint8_t hmac_key_2016[32] = {
        0x56 , 0xdd , 0xb2 , 0x92 , 0x47 , 0xbe , 0x4b , 0x89 ,
        0x1f , 0x12 , 0x62 , 0x39 , 0x0f , 0x10 , 0xb9 , 0x8e ,
        0xac , 0xff , 0xbc , 0x8a , 0x8f , 0x71 , 0xfb , 0x21 ,
        0x07 , 0x7d , 0xef , 0x9c , 0xb3 , 0x5f , 0xf9 , 0x7b ,
        };
}

inline int trace_20161025_fn(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme)
{
    struct {
        chars_view base;
        unsigned scheme;
        uint8_t derived_key[32];
    } keys[] = {
        {
            "cgrosjean@10.10.43.13,proxyuser@local@win2008,20161025-213153,wab-4-2-4.yourdomain,3243.mwrm"_av,
            0,
            {
                0x63, 0xfc, 0x3a, 0x0a, 0x32, 0x36, 0x41, 0x8a,
                0x7f, 0xaa, 0x8d, 0x88, 0xbb, 0x33, 0x73, 0x34,
                0x6a, 0xdb, 0xa9, 0x42, 0x96, 0xbb, 0xcd, 0x06,
                0xbe, 0xf8, 0xc4, 0x07, 0x8b, 0x0a, 0x80, 0xc4
            }
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@local@win2008,20161201-163203,wab-4-2-4.yourdomain,1046.mwrm"_av,
            0,
            {
                0xdc, 0x55, 0x98, 0xe3, 0x7f, 0x90, 0xa5, 0x94,
                0x60, 0x89, 0x1f, 0x95, 0x81, 0x00, 0xf8, 0xaf,
                0x73, 0xe1, 0x3d, 0x22, 0xe3, 0x6c, 0x40, 0x59,
                0x93, 0x05, 0xab, 0xf6, 0x16, 0xd2, 0x3a, 0xdb
            }
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm"_av,
            0,
            {
                0x8f, 0x17, 0x01, 0xd8, 0x87, 0xd7, 0xa1, 0x1b,
                0x40, 0x02, 0x68, 0x8d, 0xe4, 0x22, 0x2c, 0x42,
                0xe1, 0x30, 0x8e, 0x37, 0xfa, 0x2c, 0xfa, 0xef,
                0x0e, 0x40, 0x87, 0xf1, 0x57, 0x94, 0x42, 0x96
            }
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm"_av,
            1,
            {
                0xa8, 0x6e, 0x1c, 0x63, 0xe1, 0xa6, 0xfd, 0xed,
                0x2f, 0x73, 0x17, 0xca, 0x97, 0xad, 0x48, 0x07,
                0x99, 0xf5, 0xcf, 0x84, 0xad, 0x9f, 0x4a, 0x16,
                0x66, 0x38, 0x09, 0xb7, 0x74, 0xe0, 0x58, 0x34
            },
        },
        {
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560-000000.wrm"_av,
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
        && (k.base.size() == static_cast<size_t>(len))
        && (strncmp(k.base.data(), char_ptr_cast(base), static_cast<size_t>(len)) == 0))
        {
            memcpy(buffer, k.derived_key, 32);
            //hexdump_d(buffer, 32);
            return 0;
        }
    }
    RED_ERROR(base << " not found");
    return 0;
}



RED_AUTO_TEST_CASE_WD(TestDecrypterEncrypted, wd)
{
    auto output = wd.add_file("out0.txt");
    char const * argv[] {
        "decrypter.py", "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "cgrosjean@10.10.43.13,proxyuser@local@win2008,"
            "20161025-213153,wab-4-2-4.yourdomain,3243.mwrm",
        // "--hash-path", FIXTURES_PATH "/verifier/hash/",
        // "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o",
            output.c_str(),
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        str_concat("Input file is encrypted.\nOutput file is \"", output, "\".\ndecrypt ok\n"),
        ""_av);
}

RED_AUTO_TEST_CASE_WD(TestDecrypterEncrypted1, wd)
{
    auto output = wd.add_file("out8.txt");
    char const * argv[] {
        "decrypter.py", "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "cgrosjean@10.10.43.13,proxyuser@local@win2008,"
            "20161201-163203,wab-4-2-4.yourdomain,1046.mwrm",
        "-o",
            output.c_str(),
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        str_concat("Input file is encrypted.\nOutput file is \"", output, "\".\ndecrypt ok\n"),
        ""_av);
}

RED_AUTO_TEST_CASE_WD(TestDecrypterMigratedEncrypted, wd)
{
    // verifier.py redver -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm --hash-path ./tests/fixtures/verifier/hash --mwrm-path ./tests/fixtures/verifier/recorded/ --verbose 10

    auto output = wd.add_file("out8.txt");
    char const * argv[] {
        "decrypter.py", "reddec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm",
        // "--hash-path", FIXTURES_PATH "/verifier/hash/",
        // "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o",
            output.c_str(),
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        str_concat("Input file is encrypted.\nOutput file is \"", output, "\".\ndecrypt ok\n"),
        ""_av);
}

RED_AUTO_TEST_CASE_WD(TestDecrypterMigratedEncrypted2, wd)
{
    // verifier.py redver -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm --hash-path ./tests/fixtures/verifier/hash --mwrm-path ./tests/fixtures/verifier/recorded/ --verbose 10

    auto output = wd.add_file("out2.txt");
    char const * argv[] {
        "decrypter.py", "reddec",
        "-i", FIXTURES_PATH "/verifier/recorded/" "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm",
        // "--hash-path", FIXTURES_PATH "/verifier/hash/",
        // "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o",
            output.c_str(),
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        str_concat("Input file is encrypted.\nOutput file is \"", output, "\".\ndecrypt ok\n"),
        ""_av);
}

RED_AUTO_TEST_CASE(TestVerifierMigratedEncrypted)
{
    // verifier.py redver -i cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm --hash-path ./tests/fixtures/verifier/hash --mwrm-path ./tests/fixtures/verifier/recorded/ --verbose 10

    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        "Input file is encrypted.\nNo error detected during the data verification.\n\nverify ok\n"_av,
        ""_av);
}

// "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-134039,wab-4-2-4.yourdomain,4714.mwrm".
// verify as 1

RED_AUTO_TEST_CASE(TestVerifier4714)
{
    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-134039,wab-4-2-4.yourdomain,4714.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, -1, hmac_key_2016, trace_20161025_fn,
        ""_av, "Input file error: No such file or directory\n"_av);
}


//python -O /opt/wab/bin/verifier.py -i cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-164758,wab-4-2-4.yourdomain,7192.mwrm
//Input file is "/var/wab/recorded/rdp/cgrosjean@10.10.43.13,proxyadmin@win2008,20161025-164758,wab-4-2-4.yourdomain,7192.mwrm".
//Input file is unencrypted (no hash).
//verify ok (1)

RED_AUTO_TEST_CASE(TestVerifier7192)
{
    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@win2008,20161025"
            "-164758,wab-4-2-4.yourdomain,7192.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 1, hmac_key_2016, trace_20161025_fn,
        "verify failed\n"_av,
            "Cannot read hash file: \"" FIXTURES_PATH "/verifier/hash/"
            "cgrosjean@10.10.43.13,proxyadmin@win2008,20161025"
            "-164758,wab-4-2-4.yourdomain,7192.mwrm"
            "\"\n\n"_av);
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

RED_AUTO_TEST_CASE(TestVerifier1914MigratedNocryptHasChecksum)
{
    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026-131957,wab-4-2-4.yourdomain,1914.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        "No error detected during the data verification.\n\nverify ok\n"_av, ""_av);
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

RED_AUTO_TEST_CASE(TestVerifier9904NocryptNochecksumV2Statinfo)
{
    char const * argv[] {
        "verifier.py", "redver",
        "-i", "cgrosjean@10.10.43.13,proxyadmin@local@win2008,20161026"
            "-132156,wab-4-2-4.yourdomain,9904.mwrm",
        "--hash-path", FIXTURES_PATH "/verifier/hash/",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "--verbose", "10",
    };

    TEST_DO_MAIN(argv, 0, hmac_key_2016, trace_20161025_fn,
        "No error detected during the data verification.\n\nverify ok\n"_av,
        ""_av);
}

#ifndef REDEMPTION_NO_FFMPEG
RED_AUTO_TEST_CASE_WD(TestAppRecorderChunk, wd)
{
    auto output = wd.dirname().string() + "recorder-chunk";
    char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "--mwrm-path", FIXTURES_PATH "/verifier/recorded/",
        "-o",
            output.c_str(),
        "--chunk",
        "--video-codec", "mp4",
        "-D", mp4_options,
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, ".mwrm\".\n\n"), ""_av);

    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-000000.png"), 26981);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-000001.png"), 27536);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-000000.mp4"), 11'000'000 +- 10_percent);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-000001.mp4"), 110'000 +- 20_percent);
    RED_CHECK_FILE_CONTENTS(wd.add_file("recorder-chunk.pgs"), R"js({"percentage":100,"eta":0,"videos":1})js"_av);
    RED_CHECK_FILE_CONTENTS(wd.add_file("recorder-chunk.meta"), "2016-02-18 18:27:01 + (break)\n"_av);
}
#endif

RED_AUTO_TEST_CASE(TestClearTargetFiles)
{
    WorkingDirectory wd;

    struct Data
    {
        std::string name;
        WorkingFileBase path;
        std::array<bool, 5> sizes;
    };

    auto create_file = [&wd](std::string filename, auto... exists){
        filename[filename.find_last_of('_')] = '.';
        auto f = wd.add_file(filename);
        RED_TEST(::close(::open(f.c_str(), 0777, O_RDONLY | O_CREAT)) == 0);
        return Data{std::move(filename), std::move(f), {bool(exists)...}};
    };
#define F(filename, ...) create_file(#filename, __VA_ARGS__)
    const std::array prefixes{"", "ddd", "toto", "titititi", "tititi"};
    const std::array datas{
        F(toto_mwrm,          1,   1,     1,      1,          1),
        F(toto_0_wrm,         1,   1,     1,      1,          1),
        F(toto_1_wrm,         1,   1,     1,      1,          1),
        F(toto_0_flv,         1,   1,     0,      0,          0),
        F(toto_1_flv,         1,   1,     0,      0,          0),
        F(toto_meta,          1,   1,     0,      0,          0),
        F(toto_0_png,         1,   1,     0,      0,          0),
        F(toto_1_png,         1,   1,     0,      0,          0),
        F(tititi_mwrm,        1,   1,     1,      1,          1),
        F(tititi_0_wrm,       1,   1,     1,      1,          1),
        F(tititi_1_wrm,       1,   1,     1,      1,          1),
        F(tititi_0_flv,       1,   1,     1,      1,          0),
        F(tititi_1_flv,       1,   1,     1,      1,          0),
        F(tititi_meta,        1,   1,     1,      1,          0),
        F(tititi_0_png,       1,   1,     1,      1,          0),
        F(tititi_1_png,       1,   1,     1,      1,          0)
    };
#undef F

    for (std::size_t i = 0; i < prefixes.size(); ++i)
    {
        if (prefixes[i][0])
        {
            clear_files_flv_meta_png(wd.dirname(), prefixes[i]);
        }

        RED_TEST_CONTEXT("i = " << i)
        {
            for (Data const& data : datas)
            {
                if (!data.sizes[i] && data.sizes[i-1])
                {
                    wd.remove_file(data.name);
                }
            }
            RED_CHECK_WORKSPACE(wd);
        }
    }
}

#ifndef REDEMPTION_NO_FFMPEG

RED_AUTO_TEST_CASE_WD(TestAppRecorderChunkMeta, wd)
{
    auto output = wd.dirname().string() + "recorder-chunk-meta";
    char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/kpd_input.mwrm",
        "--config-file",
            FIXTURES_PATH "/disable_kbd_input_in_meta.ini",
        "--mwrm-path", FIXTURES_PATH,
        "-o",
            output.c_str(),
        "--chunk",
        "--video-codec", "mp4",
        "-D", mp4_options,
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, ".mwrm\".\n\n"), ""_av);

    RED_CHECK_FILE_CONTENTS(wd.add_file("recorder-chunk-meta.meta"), "2018-07-10 13:51:55 + type=\"TITLE_BAR\" data=\"Invite de commandes\"\n"_av);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-meta-000000.mp4"), 385000 +- 7_percent);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-meta-000000.png"), 15353 /*+- 0_v*/);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-meta-000001.mp4"), 570000 +- 10_percent);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-chunk-meta-000001.png"), 40151 /*+- 0_v*/);
    RED_TEST_FILE_CONTENTS(wd.add_file("recorder-chunk-meta.pgs"),
        "{\"percentage\":100,\"eta\":0,\"videos\":2}"_av);
}

RED_AUTO_TEST_CASE_WD(TestAppRecorderResize, wd)
{
    auto output = wd.dirname().string() + "recorder-resize-0";
    char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/resizing-capture-0.mwrm",
        "--mwrm-path", FIXTURES_PATH,
        "-o",
            output.c_str(),
        "--chunk",
        "--video-codec", "mp4",
        "--ignore-file-size",
        "-D", mp4_options,
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, ".mwrm\".\n\n"), ""_av);

    RED_TEST_FILE_SIZE(wd.add_file("recorder-resize-0-000000.mp4"), 24536 +- 200_v);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-resize-0-000000.png"), 3972);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-resize-0.meta"),       0);
    RED_TEST_FILE_CONTENTS(wd.add_file("recorder-resize-0.pgs"),
        "{\"percentage\":100,\"eta\":0,\"videos\":1}"_av);
}

RED_AUTO_TEST_CASE_WD(TestAppRecorderResize1, wd)
{
    auto output = wd.dirname().string() + "recorder-resize-1";
    char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/resizing-capture-1.mwrm",
        "--mwrm-path", FIXTURES_PATH,
        "-o",
            output.c_str(),
        "--chunk",
        "--video-codec", "mp4",
        "--ignore-file-size",
        "-D", mp4_options,
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, ".mwrm\".\n\n"), ""_av);

    RED_TEST_FILE_SIZE(wd.add_file("recorder-resize-1-000000.mp4"), 21265 +- 200_v);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-resize-1-000000.png"), 3080);
    RED_TEST_FILE_SIZE(wd.add_file("recorder-resize-1.meta"),       0);
    RED_TEST_FILE_CONTENTS(wd.add_file("recorder-resize-1.pgs"),
        "{\"percentage\":100,\"eta\":0,\"videos\":1}"_av);
}

RED_AUTO_TEST_CASE_WD(TestMetaCapture, wd)
{
    auto output = wd.dirname().string() + "test_capture.mwrm";
    char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/meta_message.mwrm",
        "--mwrm-path",
            FIXTURES_PATH "/verifier/recorded/",
        "-o", output.c_str(),
        "--chunk",
        "--video-codec", "mp4",
        "-D", "profile=baseline preset=ultrafast flags=+qscale b=80000",
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, "\".\n\n"), ""_av);

    RED_CHECK_FILE_CONTENTS(wd.add_file("test_capture.meta"),
        "1970-01-01 01:16:50 - type=\"NEW_PROCESS\" command_line=\"def\"\n"
        "1970-01-01 01:16:51 - type=\"COMPLETED_PROCESS\" command_line=\"def\"\n"
        "1970-01-01 01:16:53 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
        "1970-01-01 01:16:55 - type=\"COMPLETED_PROCESS\" command_line=\"abc\"\n"
        "1970-01-01 01:16:55 - type=\"KBD_INPUT\" data=\"Wallix\"\n"_av);

    RED_TEST_FILE_SIZE(wd.add_file("test_capture-000000.mp4"), 5881 +- 200_v);
    RED_TEST_FILE_SIZE(wd.add_file("test_capture-000000.png"), 244);
    RED_TEST_FILE_CONTENTS(wd.add_file("test_capture.pgs"),
        "{\"percentage\":100,\"eta\":0,\"videos\":1}"_av);
}

// same as previous test, but without KBD_INPUT
RED_AUTO_TEST_CASE_WD(TestMetaCaptureDisableKbdInput, wd)
{
    auto output = wd.dirname().string() + "test_capture.mwrm";
    char const * argv[] {
        "recorder.py",
        "redrec",
        "-i",
            FIXTURES_PATH "/verifier/recorded/meta_message.mwrm",
        "--mwrm-path",
            FIXTURES_PATH "/verifier/recorded/",
        "--config-file",
            FIXTURES_PATH "/disable_kbd_input_in_meta.ini",
        "-o", output.c_str(),
        "--chunk",
        "--video-codec", "mp4",
        "-D", "profile=baseline preset=ultrafast flags=+qscale b=80000",
    };

    TEST_DO_MAIN(argv, 0, hmac_key, trace_fn,
        str_concat("Output file is \"", output, "\".\n\n"), ""_av);

    RED_CHECK_FILE_CONTENTS(wd.add_file("test_capture.meta"),
        "1970-01-01 01:16:50 - type=\"NEW_PROCESS\" command_line=\"def\"\n"
        "1970-01-01 01:16:51 - type=\"COMPLETED_PROCESS\" command_line=\"def\"\n"
        "1970-01-01 01:16:53 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
        "1970-01-01 01:16:55 - type=\"COMPLETED_PROCESS\" command_line=\"abc\"\n"_av);

    RED_TEST_FILE_SIZE(wd.add_file("test_capture-000000.mp4"), 5881 +- 200_v);
    RED_TEST_FILE_SIZE(wd.add_file("test_capture-000000.png"), 244);
    RED_TEST_FILE_CONTENTS(wd.add_file("test_capture.pgs"),
        "{\"percentage\":100,\"eta\":0,\"videos\":1}"_av);
}
#endif
