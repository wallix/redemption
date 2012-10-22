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


//BOOST_AUTO_TEST_CASE(TestCryptFileOpenSSL)
//{
//    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//    unsigned char iv[] = {1,2,3,4,5,6,7,8};

//    const char * mystr = "123456";

//    unsigned char pDecryptedStr[1024] = {0};

//    int fd = open("/tmp/encrypt.txt", O_WRONLY|O_CREAT, 0644);
//    BOOST_REQUIRE(fd != 0);
//    {
//        OutFileTransport out_file(fd);
//        OutCipherTransport trans(&out_file);
//        trans.start(CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
//        trans.send(mystr, strlen(mystr));
//        trans.stop();
//    }
//    close(fd);

//    fd = open("/tmp/encrypt.txt", O_RDONLY);
//    BOOST_REQUIRE(fd != 0);
//    std::size_t size_buf = 4;
//    {
//        InFileTransport in_file(fd);
//        InCipherTransport trans(&in_file);
//        trans.start(CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
//        unsigned char * p = pDecryptedStr;
//        try {
//            trans.recv(&p, size_buf);
//        }
//        catch (const Error & e) {
//            printf("error=%u\n", e.id);
//        };
        
//        BOOST_REQUIRE_EQUAL(p - pDecryptedStr, 4);
//        BOOST_REQUIRE(
//            pDecryptedStr[0] == '1'
//            && pDecryptedStr[1] == '2'
//            && pDecryptedStr[2] == '3'
//            && pDecryptedStr[3] == '4'
//        );
//        trans.recv(&p, 2);
//        BOOST_REQUIRE_EQUAL(p - pDecryptedStr, 6);
//        BOOST_REQUIRE(
//            pDecryptedStr[0] == '1'
//            && pDecryptedStr[1] == '2'
//            && pDecryptedStr[2] == '3'
//            && pDecryptedStr[3] == '4'
//            && pDecryptedStr[4] == '5'
//            && pDecryptedStr[5] == '6'
//        );
//        try {
//            trans.recv(&p, size_buf); ///\note recv() throw an exeption
//            BOOST_REQUIRE(false);
//        } catch (Error& err) {
//            //throw std::runtime_error("normal error");
//        }
//        trans.stop();
//    }
//    close(fd);

//    unlink("/tmp/encrypt.txt");
//}

//BOOST_AUTO_TEST_CASE(TestCryptWRMFileOpenSSL)
//{
//    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//    unsigned char iv[] = {1,2,3,4,5,6,7,8};

//    unlink("/tmp/encrypt.wrm");
//    int fd = open("/tmp/encrypt.wrm", O_WRONLY|O_CREAT, 0644);
//    Rect clip(0,0, 800,600);
//    BOOST_REQUIRE(fd != 0);
//    try {
//        OutFileTransport out_file(fd);
//        OutCipherTransport trans(&out_file);
//        RaiiOutCipherTransport raii(trans, CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
//        BStream stream(65536);
//        timeval now;
//        gettimeofday(&now, NULL);
//        {
//            GraphicsToFile gtof(&trans, &stream, 0, 24, 8192, 768, 8192, 3072, 8192, 12288, now);
//            gtof.draw(RDPOpaqueRect(Rect(300,150,100,200), RED), clip);
//            gtof.draw(RDPOpaqueRect(Rect(52,50,540,150), YELLOW), clip);
//            gtof.draw(RDPOpaqueRect(Rect(500,400,50,18), GREEN), clip);
//            gtof.flush();
//            close(fd);
//        }
//    } catch (Error & e){
//        printf("got error e=%u\n", e.id);
//    };
////    fd = open("/tmp/encrypt.wrm", O_RDONLY);
//    BOOST_REQUIRE(fd != 0);

//    InFileTransport in_file(fd, false);
//    InCipherTransport trans_in(&in_file);
//    RaiiInCipherTransport raii2(trans_in, CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
//    StaticCapture pngcap(800,600,"/tmp/decrypt.png", true);
//    FileToGraphic unserializer(&trans_in, now, &pngcap, Rect(0,0,800,600));

//    char message[1024];

//    unserializer.next();
//    if (!check_sig(pngcap.drawable, message,
//        "\x2d\x26\x43\x36\x49\xe3\x03\xd2\xb7\xf9"
//        "\x76\x8a\x60\xbe\x38\xc2\x76\xf4\xb8\x28")){
//        BOOST_REQUIRE_MESSAGE(false, message);
//    }
//    unserializer.next();
//    if (!check_sig(pngcap.drawable, message,
//        "\x50\xda\x3f\xef\x0b\x53\x74\x2c\xa5\x4f"
//        "\x07\x5f\xcd\x77\xa0\xa6\x60\x7d\xc7\x1a")){
//        BOOST_REQUIRE_MESSAGE(false, message);
//    }
//    unserializer.next();
//    if (!check_sig(pngcap.drawable, message,
//        "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
//        "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42")){
//        BOOST_REQUIRE_MESSAGE(false, message);
//    }
//    BOOST_CHECK_EQUAL(unserializer.next(), false);
//    close(fd);
//    unlink("/tmp/encrypt.wrm");
//}

//BOOST_AUTO_TEST_CASE(TestCaptureWithOpenSSL)
//{

//    const EVP_CIPHER * cipher_mode = CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC);
//    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//    unsigned char iv[] = {1,2,3,4,5,6,7,8};

//    Rect clip(0,0, 800,600);
//    {
//        timeval now;
//        gettimeofday(&now, NULL);
//        NativeCapture cap(now, 800, 600, "/tmp/encrypt-cap", 0, CipherMode::BLOWFISH_CBC, key, iv);
//        timeval tm = {0,0};
//        cap.send_time_start(tm);
//        BOOST_REQUIRE(true);
//        cap.draw(RDPOpaqueRect(Rect(300,150,100,200), RED), clip);
//        BOOST_REQUIRE(true);
//        cap.draw(RDPOpaqueRect(Rect(52,50,540,150), YELLOW), clip);
//        BOOST_REQUIRE(true);
//        cap.draw(RDPOpaqueRect(Rect(500,400,50,18), GREEN), clip);
//        BOOST_REQUIRE(true);
//        //cap.recorder.flush();
//        Bitmap bitmap(FIXTURES_PATH "/logo-truncated-16x16.bmp");
//        BOOST_REQUIRE(true);
//        cap.draw(RDPMemBlt(0, Rect(0,0,bitmap.cx, bitmap.cy), 0xcc, 0, 0, 0), clip, bitmap);
//        BOOST_REQUIRE(true);
//        cap.recorder.flush();
//    }

//    std::string filename_mwrm = filename_to_pid_filename("/tmp/encrypt-cap.mwrm");
//    {
//        timeval now;
//        gettimeofday(&now, NULL);

//        HexadecimalKeyOption in_crypt_key;
//        in_crypt_key.parse("000102030405060708090A0B0C0D0E0F");
//        HexadecimalIVOption in_crypt_iv;
//        in_crypt_iv.parse("0102030405060708");
//        range_time_point range;
//        std::string path("/tmp");
//        std::string sfilename("/tmp/encrypt-cap.mwrm");

//        WRMRecorder recorder(now, cipher_mode, in_crypt_key, in_crypt_iv, InputType::META_TYPE, path, false, false, false, range, sfilename, 0);
//        const char * filename = sfilename.c_str();


//        if (!recorder.cipher_trans.start(recorder.cipher_mode, key, iv, 0))
//        {
//            recorder.cipher_mode = 0;
//            // false
//        }
//        else {
//            recorder.cipher_key = key;
//            recorder.cipher_iv = iv;
//            recorder.reader.trans = &recorder.cipher_trans;
//            recorder.trans.diff_size_is_error = false;
//        }
//        
//        if (!recorder.reader.load_data(filename_mwrm.c_str())){
//            throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errno);
//        }
//        if (recorder.reader.data_meta.files.empty()){
//            throw Error(ERR_RECORDER_META_REFERENCE_WRM);
//        }
//        if (recorder.reader.data_meta.crypt_mode && !recorder.cipher_mode){
//            throw Error(ERR_RECORDER_FILE_CRYPTED);
//        }
//        
//        filename = recorder.reader.data_meta.files[0].wrm_filename.c_str();
//        
//        if (recorder.only_filename)
//        {
//            const char * tmp = strrchr(filename + strlen(filename), '/');
//            if (tmp){
//                filename = tmp+1;
//            }
//        }
//        if (recorder.base_path_len){
//            recorder.path.erase(recorder.base_path_len);
//            recorder.path += filename;
//            filename = recorder.path.c_str();
//        }
//        
//        LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
//        int fd = ::open(filename, O_RDONLY);
//        if (-1 == fd){
//            LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
//           throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
//        }
//        recorder.trans.fd = fd;
//        
//        ++recorder.idx_file;
//        if (recorder.reader.selected_next_order() 
//        && recorder.reader.chunk_type == WRMChunk::META_FILE){
//            recorder.reader.stream.p = recorder.reader.stream.end;
//            recorder.reader.remaining_order_count = 0;
//        }
//        
//        /*WRMRecorder recorder(filename_mwrm, "",
//                             CipherMode::BLOWFISH_CBC, key, iv);*/
//        StaticCapture pngcap(800,600,"/tmp/decrypt-cap", true);
//        recorder.reader.consumer = &pngcap;
//        BOOST_REQUIRE(true);

//        char message[1024];
//        bool b;

//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, true);
//        BOOST_REQUIRE_EQUAL(recorder.reader.chunk_type, 1008);
//        recorder.interpret_order();
//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, true);
//        BOOST_REQUIRE_EQUAL(recorder.reader.chunk_type, 0);
//        recorder.interpret_order();
//        if (!check_sig(pngcap.drawable, message,
//            "\x2d\x26\x43\x36\x49\xe3\x03\xd2\xb7\xf9"
//            "\x76\x8a\x60\xbe\x38\xc2\x76\xf4\xb8\x28"))
//        {
//            BOOST_REQUIRE_MESSAGE(false, message);
//        }
//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, true);
//        recorder.interpret_order();
//        if (!check_sig(pngcap.drawable, message,
//            "\x50\xda\x3f\xef\x0b\x53\x74\x2c\xa5\x4f"
//            "\x07\x5f\xcd\x77\xa0\xa6\x60\x7d\xc7\x1a"))
//        {
//            BOOST_REQUIRE_MESSAGE(false, message);
//        }
//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, true);
//        recorder.interpret_order();
//        if (!check_sig(pngcap.drawable, message,
//            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
//            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
//        {
//            BOOST_REQUIRE_MESSAGE(false, message);
//        }
//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, true);
//        recorder.interpret_order();
//        if (!check_sig(pngcap.drawable, message,
//            "\xab\xd2\xed\x13\xa5\x3e\xc6\x85\xb0\x1a"
//            "\x2a\x68\xb0\x1d\xc1\x74\x20\x83\x48\x42"))
//        {
//            BOOST_REQUIRE_MESSAGE(false, message);
//        }
//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, true);
//        recorder.interpret_order();
//        if (!check_sig(pngcap.drawable, message,
//            "\x0f\x3e\xa2\x25\x2c\x2a\x77\x1e\xdf\x26"
//            "\x95\x54\x59\xd3\x33\xfb\x1b\x97\xed\xc3"))
//        {
//            BOOST_REQUIRE_MESSAGE(false, message);
//        }

//        b = recorder.reader.selected_next_order();
//        BOOST_REQUIRE_EQUAL(b, false);
//    }

//    unlink(filename_mwrm.c_str());
//    filename_mwrm.erase(filename_mwrm.size() - 4, 1);
//    unlink(FilenameIncrementalGenerator(filename_mwrm).next().c_str());
//}

//BOOST_AUTO_TEST_CASE(TestWrmWithOpenSSL)
//{
//        timeval now;
//        gettimeofday(&now, NULL);

//        const EVP_CIPHER * cipher_mode = CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC);
//        HexadecimalKeyOption in_crypt_key;
//        in_crypt_key.parse("000102030405060708090A0B0C0D0E0F");
//        HexadecimalIVOption in_crypt_iv;
//        in_crypt_iv.parse("0102030405060708");
//        range_time_point range;
//        std::string path(FIXTURES_PATH);
//        std::string sfilename(FIXTURES_PATH "/test_w2008_2-880.mwrm");

//        WRMRecorder recorder(now, cipher_mode, in_crypt_key, in_crypt_iv, InputType::META_TYPE, path, false, false, false, range, sfilename, 0);

//        unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//        unsigned char iv[] = {1,2,3,4,5,6,7,8};
//        NativeCapture cap(now, 800, 600, "/tmp/wrm-encrypt", 0, CipherMode::BLOWFISH_CBC, key, iv);

//        cap.send_time_start(now);
//        BOOST_CHECK(true);
//        recorder.reader.consumer = &cap;

//        while (recorder.reader.selected_next_order()){
//            if (recorder.reader.chunk_type == WRMChunk::TIMESTAMP){
//                recorder.reader.stream.p = recorder.reader.stream.end;
//                recorder.reader.remaining_order_count = 0;
//                cap.recorder.timestamp();
//                continue;
//            }
//            recorder.interpret_order();
//            BOOST_CHECK(true);
//        }
//        cap.flush();
//        BOOST_CHECK(true);

//        std::string sxxx = make_pid_filename_generator("/tmp/wrm-encrypt.mwrm")();
//        std::string path2 = std::string("/tmp");
//        const char * xxx = sxxx.c_str();        
//        printf("xxx=%s\n", xxx);
////        WRMRecorder recorder(now, xxx, "", CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);

//        WRMRecorder recorder2(now, cipher_mode, in_crypt_key, in_crypt_iv, InputType::META_TYPE, path2, false, false, false, range, sxxx, 0);

//        StaticCapture cap2(800, 600, "/tmp/wrm-encrypt", true);
//        recorder2.reader.consumer = &cap;

//        while (recorder2.reader.selected_next_order()){
//            if (recorder2.reader.chunk_type == WRMChunk::TIMESTAMP){
//                recorder2.reader.stream.p = recorder.reader.stream.end;
//                recorder2.reader.remaining_order_count = 0;
//                continue;
//            }
//            recorder2.reader.interpret_order();
//            BOOST_CHECK(true);
//        }

//        char message[1024];
//        if (!check_sig(cap2.drawable, message,
//            "\xd0\x8a\xe3\x69\x7c\x88\x91\xf8\xc4\xf5"
//            "\xd8\x90\xaa\xaa\xec\x13\xd0\xde\x1c\xe1")){
//            BOOST_REQUIRE_MESSAGE(false, message);
//        }
//}
