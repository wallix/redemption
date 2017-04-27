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

#define RED_TEST_MODULE TestInCryptoTransport
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT
#include "utils/log.hpp"
#include "transport/out_crypto_transport.hpp"
#include "transport/in_crypto_transport.hpp"
#include <cstring>
#include "test_only/lcg_random.hpp"

using Read = Transport::Read;

RED_AUTO_TEST_CASE(TestInCryptoTransportClearText)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransport");
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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    const char * finalname = "./clear.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(false, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }

    auto expected_hash = cstr_array_view(
        "\xc5\x28\xb4\x74\x84\x3d\x8b\x14\xcf\x5b\xf4\x3a\x9c\x04\x9a\xf3"
        "\x23\x9f\xac\x56\x4d\x86\xb4\x32\x90\x69\xb5\xe1\x45\xd0\x76\x9b");

    RED_CHECK_MEM_AA(qhash, expected_hash);
    RED_CHECK_MEM_AA(fhash, expected_hash);

    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    {
        char buffer[40];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, 30));
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[30], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[31], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        ct.close();
        RED_CHECK_MEM_AC(make_array_view(buffer, 31), "We write, and again, and so on.");
        // close followed by open
        ct.open(finalname);
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, 30));
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[30], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[31], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        ct.close();
        RED_CHECK_MEM_AC(make_array_view(buffer, 31), "We write, and again, and so on.");
    }

    RED_CHECK(::unlink(finalname) == 0);
}


// This sample was generated using udevrandom on Linux
static uint8_t randomSample[8192] = {
#include "fixtures/randomdata.hpp"
};

RED_AUTO_TEST_CASE(TestInCryptoTransportBigCrypted)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransportBigCrypted");

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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    const char * finalname = "./encrypted.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send(randomSample, sizeof(randomSample));
        ct.close(qhash, fhash);
    }

    auto expected_qhash = cstr_array_view(
        "\x04\x52\x16\x50\xdb\x48\xe6\x70\x36\x3c\x68\xa9\xcd\xdb\xeb\x60"
        "\xf9\x25\x83\xbc\x0d\x2e\x09\x3f\xf2\xc9\x37\x5d\xa6\x9d\x7a\xf0");

    auto expected_fhash = cstr_array_view(
        "\xa8\x7c\x51\x79\xe2\xcc\x2c\xe3\x51\x64\x40\xc0\xb0\xbd\xa8\x99"
        "\xcc\x46\xac\x42\x3f\x22\x0f\x64\x50\xbb\xbb\x7c\x45\xb8\x1c\xc4"
        );

    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);

    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    {
        char buffer[sizeof(randomSample)];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK_EQUAL(ct.is_encrypted(), true);
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, sizeof(buffer)-10));
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[sizeof(buffer)-10], 10));
        RED_CHECK_EQUAL(true, ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[sizeof(buffer)], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        ct.close();
        RED_CHECK_MEM_AA(make_array_view(buffer, sizeof(buffer)),
                         make_array_view(randomSample, sizeof(randomSample)));
    }
    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}

RED_AUTO_TEST_CASE(TestInCryptoTransportCrypted)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransportCrypted");
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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    const char * finalname = "./encrypted.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }

    auto expected_hash = cstr_array_view(
        "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce"
        "\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5");

    RED_CHECK_MEM_AA(qhash, expected_hash);
    RED_CHECK_MEM_AA(fhash, expected_hash);

    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    RED_CHECK_NO_THROW([&]{
        char buffer[40];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK_EQUAL(ct.is_encrypted(), true);
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, 30));
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[30], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[30], 1));
        ct.close();
        RED_CHECK_MEM_AC(make_array_view(buffer, 31), "We write, and again, and so on.");
    }());

    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}

static char clearSample[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas posuere, libero nec auctor auctor, leo sapien consequat nulla, non condimentum diam mauris eget ante. Nam in commodo elit, vel pulvinar risus. Sed commodo nisl aliquet nisl varius, non mollis ex sodales. Duis eu pellentesque odio, ac tincidunt libero. Aenean quam risus, lacinia non interdum ac, interdum ut neque. Morbi eget justo posuere, commodo erat et, elementum nisl. Nunc in tincidunt libero. Sed gravida purus in dolor iaculis dictum. Vivamus vel dolor eu purus scelerisque convallis lacinia vitae lacus. Nulla facilisi. Aliquam urna leo, facilisis id ipsum ac, rhoncus vulputate nibh. Nulla vitae aliquet lectus, ac finibus nulla. Suspendisse ut risus porttitor, rhoncus massa id, feugiat felis.\n\n"

"Suspendisse molestie erat et nibh efficitur varius. Nulla nec eros pulvinar, condimentum odio a, fermentum lorem. Vivamus sit amet sapien pulvinar, vehicula purus pellentesque, lobortis nisl. Curabitur orci metus, ornare a lobortis vitae, aliquam at risus. Phasellus tristique lacus quis vestibulum lacinia. Aenean interdum erat non odio semper, vitae ultricies ipsum mollis. Integer dapibus ultrices eros, elementum posuere massa sollicitudin sit amet. Phasellus quis diam imperdiet, tempor tortor eget, ultrices purus. Suspendisse semper at justo nec pharetra. Sed est velit, egestas at justo vitae, varius mattis massa. Donec efficitur orci et elementum consectetur. In malesuada molestie quam mattis egestas. Nulla dictum massa a metus feugiat, sed maximus metus aliquam. Phasellus sem lacus, dictum vel vestibulum cursus, elementum id dui.\n\n"

"Nulla sed ullamcorper massa, dignissim porttitor nunc. Nulla facilisi. Aliquam tincidunt, nulla fringilla tincidunt tincidunt, nulla magna accumsan velit, nec maximus elit risus sed justo. Donec tincidunt eget est vel ultricies. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nunc semper, nulla a eleifend feugiat, ex enim mattis felis, ut dapibus mi massa ac libero. Sed sit amet dignissim nisl, eget fermentum ex. In hac habitasse platea dictumst. Pellentesque tempor nisi quis laoreet euismod. Proin viverra nisi justo, id finibus massa bibendum id. Donec sit amet congue ex. Suspendisse eu nisi ligula. Aliquam mattis orci pellentesque elit laoreet interdum. Integer tincidunt sem nec lacus sagittis varius.\n\n"

"Nunc at sem in odio interdum placerat. Integer pulvinar, velit vitae euismod volutpat, turpis elit lobortis nibh, in ornare lorem velit vel orci. Integer sem odio, aliquam id vulputate non, dapibus sed est. Vivamus porta sapien leo, vulputate lobortis lacus commodo non. Aliquam fermentum maximus convallis. Sed vestibulum, massa et vestibulum interdum, tortor leo placerat quam, eu blandit neque mi sit amet elit. Etiam consequat dapibus augue sed ultrices. Proin eget massa id massa cursus mollis. Aenean non quam iaculis, consectetur justo rutrum, ullamcorper velit. Donec blandit leo quis sapien vehicula egestas. Fusce dignissim erat in nisi blandit sollicitudin et vitae neque. Nulla vitae dolor dolor. Donec scelerisque velit at purus auctor feugiat. Ut urna ante, lobortis ut placerat nec, scelerisque sit amet dui. Sed scelerisque, lorem et gravida tristique, elit nisl pretium eros, et hendrerit neque neque quis turpis.\n\n"

"Vestibulum id arcu in neque accumsan posuere. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut at tincidunt elit. Sed risus quam, rhoncus vitae venenatis interdum, auctor ut lectus. Nulla sit amet arcu sit amet dolor gravida cursus. Nulla nibh lacus, egestas sed lacus at, interdum auctor risus. Maecenas at ultrices orci. Praesent lacinia erat vitae sem posuere cursus. Ut commodo lorem in venenatis feugiat. Suspendisse semper neque vel sapien congue finibus. Donec eget massa imperdiet, posuere elit vitae, consectetur est. Integer dictum finibus mauris. Proin tempor ex eget enim fermentum, scelerisque aliquet enim vestibulum. Fusce justo felis, feugiat ac lobortis ut, euismod non felis.\n\n";

RED_AUTO_TEST_CASE(TestInCryptoTransportBigClear)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransportBigClear");

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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};


    const char * finalname = "./clear.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(false, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send(clearSample, sizeof(clearSample));
        ct.close(qhash, fhash);
    }

    auto expected_qhash = cstr_array_view(
        "\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"
        );

    auto expected_fhash = cstr_array_view("\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"
        );

    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);

    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    {
        char buffer[sizeof(clearSample)];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK_EQUAL(ct.is_encrypted(), false);
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, sizeof(buffer)-10));
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[sizeof(buffer)-10], 10));
        RED_CHECK_EQUAL(true, ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[sizeof(buffer)], 1));
        RED_CHECK_EQUAL(true, ct.is_eof());
        ct.close();
        RED_CHECK_MEM_AA(make_array_view(buffer, sizeof(buffer)),
                         make_array_view(clearSample, sizeof(clearSample)));
    }
    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}

RED_AUTO_TEST_CASE(TestInCryptoTransportBigClearPartialRead)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransportBigClearPartial");

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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};


    const char * finalname = "./clear.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(false, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send(clearSample, sizeof(clearSample));
        ct.close(qhash, fhash);
    }

    auto expected_qhash = cstr_array_view(
        "\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"
        );

    auto expected_fhash = cstr_array_view("\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"
        );

    RED_CHECK_MEM_AA(qhash, expected_qhash);
    RED_CHECK_MEM_AA(fhash, expected_fhash);

    RED_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    {
        char buffer[sizeof(clearSample)];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK_EQUAL(ct.is_encrypted(), false);
        RED_CHECK_EQUAL(false, ct.is_eof());
        RED_CHECK_EQUAL(20, ct.partial_read(buffer, 20));
        RED_CHECK_EQUAL(100, ct.partial_read(&buffer[20], 100));
        // At end of file partial_read should return what it can
        RED_CHECK_EQUAL(sizeof(buffer)-120, ct.partial_read(&buffer[120], sizeof(buffer)));


        ct.close();
        RED_CHECK_MEM_AA(make_array_view(buffer, sizeof(buffer)),
                         make_array_view(clearSample, sizeof(clearSample)));
    }
    RED_CHECK(::unlink(finalname) == 0); // finalname exists
}
