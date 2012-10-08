/*
 * T his program *is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCryptOpenSSL
#include <boost/test/auto_unit_test.hpp>
#include <boost/lexical_cast.hpp>

#define LOGPRINT

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "recorder/wrm_recorder_option.hpp"
#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "GraphicToFile.hpp"
#include "check_sig.hpp"

#include "cipher.hpp"
#include "cipher_transport.hpp"

#include "filename_generator.hpp"

const EVP_CIPHER* cipher_mode()
{
    return CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC);
}

void cipher_crypt_test_run(CipherCrypt& cipher_crypt,
                           const unsigned char* data, std::size_t len,
                           const unsigned char* key = 0,
                           const unsigned char* iv = 0)
{
    if (len)
    {
        BOOST_REQUIRE(true);
        cipher_crypt.start(cipher_mode(), key, iv);
        BOOST_REQUIRE(true);
        std::size_t p = 0;
        for (std::size_t n = 2; n <= len; p = n, n += 2)
        {
            cipher_crypt.update(data + p, n-p);
        }
        BOOST_REQUIRE(true);
        if (p != len)
            cipher_crypt.update(data + p, len - p);
        BOOST_REQUIRE(true);
        cipher_crypt.stop();
    }
}

BOOST_AUTO_TEST_CASE(TestCryptOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    //const char * mystr = "1234567890";
    const char * mystr = "123456";

    unsigned char pEncryptedStr[1024] = {0};
    unsigned char pDecryptedStr[1024] = {0};
    std::size_t outlen;
    BOOST_REQUIRE(true);
    CipherCryptData crypt_data(pEncryptedStr);
    BOOST_REQUIRE(true);
    CipherCrypt cipher_crypt(&crypt_data);
    BOOST_REQUIRE(true);
    {
        cipher_crypt.init_encrypt();
        BOOST_REQUIRE(true);
        cipher_crypt_test_run(cipher_crypt,
                              (const unsigned char *)mystr,
                              strlen(mystr),
                              key, iv);
        outlen = crypt_data.size();
        BOOST_REQUIRE_EQUAL(outlen, 8);
        const unsigned char * tmp = (const unsigned char *)"\x17\xf9\x7b\xe7\xfb\x8b\xfa\xd6";
        BOOST_REQUIRE(
            pEncryptedStr[0] == tmp[0]
            && pEncryptedStr[1] == tmp[1]
            && pEncryptedStr[2] == tmp[2]
            && pEncryptedStr[3] == tmp[3]
            && pEncryptedStr[4] == tmp[4]
            && pEncryptedStr[5] == tmp[5]
            && pEncryptedStr[6] == tmp[6]
            && pEncryptedStr[7] == tmp[7]
        );
    }
    crypt_data.reset(pDecryptedStr);
    {
        BOOST_REQUIRE(true);
        cipher_crypt.init_decrypt();
        BOOST_REQUIRE(true);
        cipher_crypt_test_run(cipher_crypt,
                              pEncryptedStr, outlen,
                              key, iv);
        BOOST_REQUIRE_EQUAL(crypt_data.size(), 6);
        const unsigned char * tmp = (const unsigned char *)mystr;
        BOOST_REQUIRE(
            pDecryptedStr[0] == tmp[0]
            && pDecryptedStr[1] == tmp[1]
            && pDecryptedStr[2] == tmp[2]
            && pDecryptedStr[3] == tmp[3]
            && pDecryptedStr[4] == tmp[4]
            && pDecryptedStr[5] == tmp[5]
        );
    }
}

BOOST_AUTO_TEST_CASE(TestCryptFileOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    //const char * mystr = "1234567890";
    const char * mystr = "123456";

    unsigned char pDecryptedStr[1024] = {0};

    int fd = open("/tmp/encrypt.txt", O_WRONLY|O_CREAT, 0644);
    BOOST_REQUIRE(fd != 0);
    {
        OutFileTransport out_file(fd);
        OutCipherTransport trans(&out_file);
        RaiiOutCipherTransport raii(trans, cipher_mode(), key, iv);
        BOOST_REQUIRE(true);
        trans.send(mystr, strlen(mystr));
        BOOST_REQUIRE(true);
    }
    close(fd);

    fd = open("/tmp/encrypt.txt", O_RDONLY);
    BOOST_REQUIRE(fd != 0);
    std::size_t size_buf = 4;
    {
        InFileTransport in_file(fd, false);
        InCipherTransport trans(&in_file);
        RaiiInCipherTransport raii(trans, cipher_mode(), key, iv);
        BOOST_REQUIRE(true);
        unsigned char * p = pDecryptedStr;
        trans.recv(&p, size_buf);
        BOOST_REQUIRE_EQUAL(p - pDecryptedStr, 4);
        BOOST_REQUIRE(
            pDecryptedStr[0] == '1'
            && pDecryptedStr[1] == '2'
            && pDecryptedStr[2] == '3'
            && pDecryptedStr[3] == '4'
        );
        trans.recv(&p, 2);
        BOOST_REQUIRE_EQUAL(p - pDecryptedStr, 6);
        BOOST_REQUIRE(
            pDecryptedStr[0] == '1'
            && pDecryptedStr[1] == '2'
            && pDecryptedStr[2] == '3'
            && pDecryptedStr[3] == '4'
            && pDecryptedStr[4] == '5'
            && pDecryptedStr[5] == '6'
        );
        try {
            trans.recv(&p, size_buf); ///\note recv() throw an exeption
            BOOST_REQUIRE(false);
        } catch (Error& err) {
            //throw std::runtime_error("normal error");
        }
    }
    close(fd);

    unlink("/tmp/encrypt.txt");
}

BOOST_AUTO_TEST_CASE(TestCryptWRMFileOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    int fd = open("/tmp/encrypt.wrm", O_WRONLY|O_CREAT, 0644);
    Rect clip(0,0, 800,600);
    BOOST_REQUIRE(fd != 0);
    {
        OutFileTransport out_file(fd);
        OutCipherTransport trans(&out_file);
        RaiiOutCipherTransport raii(trans, cipher_mode(), key, iv);
        BStream stream(65536);
        timeval now;
        gettimeofday(&now, NULL);
        GraphicsToFile gtof(&trans, &stream, 0, 24,
                            8192, 768, 8192, 3072, 8192, 12288, now);
        BOOST_REQUIRE(true);
        gtof.draw(RDPOpaqueRect(Rect(300,150,100,200), RED), clip);
        BOOST_REQUIRE(true);
        gtof.draw(RDPOpaqueRect(Rect(52,50,540,150), YELLOW), clip);
        BOOST_REQUIRE(true);
        gtof.draw(RDPOpaqueRect(Rect(500,400,50,18), GREEN), clip);
        BOOST_REQUIRE(true);
        gtof.flush();
    }
    close(fd);

    fd = open("/tmp/encrypt.wrm", O_RDONLY);
    BOOST_REQUIRE(fd != 0);
    {
        InFileTransport in_file(fd, false);
        InCipherTransport trans(&in_file);
        RaiiInCipherTransport raii(trans, cipher_mode(), key, iv);
        timeval now;
        gettimeofday(&now, NULL);
        StaticCapture pngcap(800,600,"/tmp/decrypt.png", true);
        RDPUnserializer unserializer(&trans, now, &pngcap, Rect(0,0,800,600));

        char message[1024];

        BOOST_REQUIRE(true);
        unserializer.next();
        if (!check_sig(pngcap.drawable, message,
            "\x2d\x26\x43\x36\x49\xe3\x03\xd2\xb7\xf9"
            "\x76\x8a\x60\xbe\x38\xc2\x76\xf4\xb8\x28"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        BOOST_REQUIRE(true);
        unserializer.next();
        if (!check_sig(pngcap.drawable, message,
            "\x50\xda\x3f\xef\x0b\x53\x74\x2c\xa5\x4f"
            "\x07\x5f\xcd\x77\xa0\xa6\x60\x7d\xc7\x1a"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        BOOST_REQUIRE(true);
        unserializer.next();
        if (!check_sig(pngcap.drawable, message,
            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        BOOST_CHECK_EQUAL(unserializer.next(), false);
    }
    close(fd);
    unlink("/tmp/encrypt.wrm");
}

BOOST_AUTO_TEST_CASE(TestCaptureWithOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    Rect clip(0,0, 800,600);
    {
        timeval now;
        gettimeofday(&now, NULL);
        NativeCapture cap(now, 800, 600, "/tmp/encrypt-cap", 0,
                          CipherMode::BLOWFISH_CBC, key, iv);
        timeval tm = {0,0};
        cap.send_time_start(tm);
        BOOST_REQUIRE(true);
        cap.draw(RDPOpaqueRect(Rect(300,150,100,200), RED), clip);
        BOOST_REQUIRE(true);
        cap.draw(RDPOpaqueRect(Rect(52,50,540,150), YELLOW), clip);
        BOOST_REQUIRE(true);
        cap.draw(RDPOpaqueRect(Rect(500,400,50,18), GREEN), clip);
        BOOST_REQUIRE(true);
        //cap.recorder.flush();
        Bitmap bitmap(FIXTURES_PATH "/logo-truncated-16x16.bmp");
        BOOST_REQUIRE(true);
        cap.draw(RDPMemBlt(0, Rect(0,0,bitmap.cx, bitmap.cy), 0xcc, 0, 0, 0), clip, bitmap);
        BOOST_REQUIRE(true);
        cap.recorder.flush();
    }

    std::string filename_mwrm = filename_to_pid_filename(
        "/tmp/encrypt-cap.mwrm");
    {
        timeval now;
        gettimeofday(&now, NULL);
        WRMRecorder recorder(now);
        recorder.init_cipher(CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
        recorder.open_meta_followed_wrm(filename_mwrm.c_str());
        /*WRMRecorder recorder(filename_mwrm, "",
                             CipherMode::BLOWFISH_CBC, key, iv);*/
        StaticCapture pngcap(800,600,"/tmp/decrypt-cap", true);
        recorder.consumer(&pngcap);
        BOOST_REQUIRE(true);

        char message[1024];
        bool b;

        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, true);
        BOOST_REQUIRE_EQUAL(recorder.chunk_type(), 1008);
        recorder.interpret_order();
        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, true);
        BOOST_REQUIRE_EQUAL(recorder.chunk_type(), 0);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\x2d\x26\x43\x36\x49\xe3\x03\xd2\xb7\xf9"
            "\x76\x8a\x60\xbe\x38\xc2\x76\xf4\xb8\x28"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\x50\xda\x3f\xef\x0b\x53\x74\x2c\xa5\x4f"
            "\x07\x5f\xcd\x77\xa0\xa6\x60\x7d\xc7\x1a"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\x0f\x3e\xa2\x25\x2c\x2a\x77\x1e\xdf\x26"
            "\x95\x54\x59\xd3\x33\xfb\x1b\x97\xed\xc3"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }

        b = recorder.reader.selected_next_order();
        BOOST_REQUIRE_EQUAL(b, false);
    }

    unlink(filename_mwrm.c_str());
    filename_mwrm.erase(filename_mwrm.size() - 4, 1);
    unlink(FilenameIncrementalGenerator(filename_mwrm).next().c_str());
}

BOOST_AUTO_TEST_CASE(TestWrmWithOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    BOOST_CHECK(true);
    {
        timeval now;
        gettimeofday(&now, NULL);
        WRMRecorder recorder(now, FIXTURES_PATH "/test_w2008_2-880.mwrm", FIXTURES_PATH);
        BOOST_CHECK(true);
        NativeCapture cap(now, 800, 600, "/tmp/wrm-encrypt", 0,
                          CipherMode::BLOWFISH_CBC, key, iv);
        gettimeofday(&now, 0);
        cap.send_time_start(now);
        BOOST_CHECK(true);
        recorder.consumer(&cap);

        while (recorder.reader.selected_next_order())
        {
            if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
            {
                recorder.reader.stream.p = recorder.reader.stream.end;
                recorder.reader.remaining_order_count = 0;
                cap.recorder.timestamp();
                continue;
            }
            recorder.interpret_order();
            BOOST_CHECK(true);
        }
        cap.flush();
    }
    BOOST_CHECK(true);
    {
        timeval now;
        gettimeofday(&now, NULL);
        WRMRecorder recorder(now, make_pid_filename_generator("/tmp/wrm-encrypt.mwrm")().c_str(), "",
                             CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
        BOOST_CHECK(true);
        StaticCapture cap(800, 600, "/tmp/wrm-encrypt", true);
        BOOST_CHECK(true);
        recorder.consumer(&cap);

        while (recorder.reader.selected_next_order())
        {
            if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
            {
                recorder.reader.stream.p = recorder.reader.stream.end;
                recorder.reader.remaining_order_count = 0;
                continue;
            }
            recorder.reader.interpret_order();
            BOOST_CHECK(true);
        }

        char message[1024];
        if (!check_sig(cap.drawable, message,
            "\xd0\x8a\xe3\x69\x7c\x88\x91\xf8\xc4\xf5"
            "\xd8\x90\xaa\xaa\xec\x13\xd0\xde\x1c\xe1"))
        {
            BOOST_REQUIRE_MESSAGE(false, message);
        }
    }
}
