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

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wrm_recorder.hpp"
#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "GraphicToFile.hpp"
#include "check_sig.hpp"

#include "cipher.hpp"
#include "cipher_transport.hpp"

#include "filename_generator.hpp"

// BlowFish
#define MY_CIPHER_MODE EVP_bf_cbc()   // Blowfish CBC mode
//#define MY_CIPHER_MODE EVP_bf_ecb()   // Blowfish ECB mode

// DES
//#define MY_CIPHER_MODE EVP_des_cbc()    // DES CBC mode
//#define MY_CIPHER_MODE EVP_des_ecb()    // DES ECB mode
//#define MY_CIPHER_MODE EVP_des_ede()    // DES EDE mode
///des_ede3 = error … @{
//#define MY_CIPHER_MODE EVP_des_ede3_cbc()
//#define MY_CIPHER_MODE EVP_des_ede3_ofb()
//#define MY_CIPHER_MODE EVP_des_ede3_cfb()
//#define MY_CIPHER_MODE EVP_des_ede3()   // DES EDE3 mode
///@}

// RC2
//#define MY_CIPHER_MODE EVP_rc2_cbc()    // RC2 CBC mode
//#define MY_CIPHER_MODE EVP_rc2_ecb()    // RC2 ECB mode

// RC4
//#define MY_CIPHER_MODE EVP_rc4()      // RC4 mode
//#define MY_CIPHER_MODE EVP_rc4_40()   // RC4 40 mode

void cipher_crypt_test_run(CipherCrypt& cipher_crypt,
                           const unsigned char* data, std::size_t len,
                           const unsigned char* key = 0,
                           const unsigned char* iv = 0)
{
    if (len)
    {
        BOOST_REQUIRE(true);
        cipher_crypt.start(MY_CIPHER_MODE, key, iv);
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

    std::cout << "EVP_MAX_BLOCK_LENGTH: " << EVP_MAX_BLOCK_LENGTH << '\n';

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
        BOOST_REQUIRE(true);
        outlen = crypt_data.size();
        std::cout << "After encrypted: " << pEncryptedStr << std::endl;
        std::cout << "size: " << outlen << std::endl;
    }
    crypt_data.reset(pDecryptedStr);
    {
        BOOST_REQUIRE(true);
        cipher_crypt.init_decrypt();
        BOOST_REQUIRE(true);
        cipher_crypt_test_run(cipher_crypt,
                              pEncryptedStr, outlen,
                              key, iv);
        BOOST_REQUIRE(true);
        std::cout << "After decrypted: " << pDecryptedStr << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(TestCryptFileOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    //const char * mystr = "1234567890";
    const char * mystr = "123456";

    unsigned char pDecryptedStr[1024] = {0};

    std::cout << "EVP_MAX_BLOCK_LENGTH: " << EVP_MAX_BLOCK_LENGTH << '\n';

    int fd = open("/tmp/encrypt.txt", O_WRONLY|O_CREAT, 0644);
    BOOST_REQUIRE(fd != 0);
    {
        OutFileTransport out_file(fd);
        OutCipherTransport trans(&out_file);
        RaiiOutCipherTransport raii(trans, MY_CIPHER_MODE, key, iv);
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
        RaiiInCipherTransport raii(trans, MY_CIPHER_MODE, key, iv);
        BOOST_REQUIRE(true);
        unsigned char * p = pDecryptedStr;
        trans.recv(&p, size_buf);
        BOOST_REQUIRE(true);
        std::cout << "size: " << p - pDecryptedStr << std::endl;
        std::cout << "After decrypted: " << pDecryptedStr << std::endl;
        trans.recv(&p, 2);
        BOOST_REQUIRE(true);
        std::cout << "size: " << p - pDecryptedStr << std::endl;
        std::cout << "After decrypted: " << pDecryptedStr << std::endl;
        try {
            trans.recv(&p, size_buf);
            BOOST_REQUIRE(true);
            std::cout << "size: " << p - pDecryptedStr << std::endl;
            std::cout << "After decrypted: " << pDecryptedStr << std::endl;
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
        RaiiOutCipherTransport raii(trans, MY_CIPHER_MODE, key, iv);
        BStream stream(65536);
        GraphicsToFile gtof(&trans, &stream, 0, 24,
                            8192, 768, 8192, 3072, 8192, 12288);
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
        RaiiInCipherTransport raii(trans, MY_CIPHER_MODE, key, iv);
        StaticCapture pngcap(800,600,"/tmp/decrypt.png");
        RDPUnserializer unserializer(&trans, &pngcap, Rect(0,0,800,600));
        BOOST_REQUIRE(true);

        char message[1024];

        unserializer.next();
        if (!check_sig(pngcap.drawable, message,
            "\x2d\x26\x43\x36\x49\xe3\x03\xd2\xb7\xf9"
            "\x76\x8a\x60\xbe\x38\xc2\x76\xf4\xb8\x28"))
        {
            BOOST_CHECK_MESSAGE(false, message);
        }
        BOOST_REQUIRE(true);
        unserializer.next();
        if (!check_sig(pngcap.drawable, message,
            "\x50\xda\x3f\xef\x0b\x53\x74\x2c\xa5\x4f"
            "\x07\x5f\xcd\x77\xa0\xa6\x60\x7d\xc7\x1a"))
        {
            BOOST_CHECK_MESSAGE(false, message);
        }
        BOOST_REQUIRE(true);
        unserializer.next();
        if (!check_sig(pngcap.drawable, message,
            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
        {
            BOOST_CHECK_MESSAGE(false, message);
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
        NativeCapture cap(800, 600, "/tmp/encrypt-cap.wrm", 0,
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
        cap.flush();
    }

    try
    {
        WRMRecorder recorder(filename_to_pid_filename(
            "/tmp/encrypt-cap.wrm.mwrm"),
            "",
            CipherMode::BLOWFISH_CBC, key, iv);
        StaticCapture pngcap(800,600,"/tmp/decrypt-cap.png");
        recorder.consumer(&pngcap);
        BOOST_REQUIRE(true);

        char message[1024];

        BOOST_CHECK_MESSAGE(recorder.selected_next_order(), true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\x2d\x26\x43\x36\x49\xe3\x03\xd2\xb7\xf9"
            "\x76\x8a\x60\xbe\x38\xc2\x76\xf4\xb8\x28"))
        {
            BOOST_CHECK_MESSAGE(false, message);
        }
        BOOST_CHECK_MESSAGE(recorder.selected_next_order(), true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\x50\xda\x3f\xef\x0b\x53\x74\x2c\xa5\x4f"
            "\x07\x5f\xcd\x77\xa0\xa6\x60\x7d\xc7\x1a"))
        {
            BOOST_CHECK_MESSAGE(false, message);
        }
        BOOST_CHECK_MESSAGE(recorder.selected_next_order(), true);
        recorder.interpret_order();
        if (!check_sig(pngcap.drawable, message,
            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
        {
            BOOST_CHECK_MESSAGE(false, message);
        }
        BOOST_CHECK_MESSAGE(recorder.selected_next_order(), false);
    } catch (Error& e)
    {
        std::cerr << e.id << std::endl;
        throw ;
    }

//     unlink("/tmp/encrypt.wrm");
}