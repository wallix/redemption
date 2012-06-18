/*
 T his program *is free software; you can redistribute it and/or modify
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
 Copyright (C) Wallix 2010-2012
 Author(s): Christophe Grosjean, Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFileToPng
#include <boost/test/auto_unit_test.hpp>

#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "transport.hpp"
#include "meta_wrm.hpp"

#include "unlink.hpp"

bool check_sig(Drawable & data, char * message, const char * shasig)
{
    SSL_SHA1 sha1;
    uint8_t sig[20];
    ssllib ssl;
    ssl.sha1_init(&sha1);
    for (size_t y = 0; y < (size_t)data.height; y++){
        ssl.sha1_update(&sha1, data.data + y * data.rowsize, data.rowsize);
    }
    ssl.sha1_final(&sha1, sig);

    if (memcmp(shasig, sig, 20)){
        sprintf(message, "Expected signature: \""
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
        sig[ 0], sig[ 1], sig[ 2], sig[ 3],
        sig[ 4], sig[ 5], sig[ 6], sig[ 7],
        sig[ 8], sig[ 9], sig[10], sig[11],
        sig[12], sig[13], sig[14], sig[15],
        sig[16], sig[17], sig[18], sig[19]);
        return false;
    }
    return true;
}

void file_to_png(const char* filename, uint16_t w, uint16_t h, uint16_t bpp, const char * shasig)
{
    int fd;
    {
        char tmppath[1024] = {};
        sprintf(tmppath, "%s-%u-0.wrm", filename, getpid());
        fd = ::open(tmppath, O_RDONLY);
        if (fd == -1)
        {
            BOOST_CHECK_MESSAGE(false, "fd == -1");
            return ;
        }
    }
    InFileTransport in_trans(fd);
    RDPUnserializer reader(&in_trans, 0, Rect());
    BOOST_CHECK(1);
    MetaWRM meta(reader);
    BOOST_CHECK_EQUAL(w, meta.width);
    BOOST_CHECK_EQUAL(h, meta.height);
    BOOST_CHECK_EQUAL(bpp, meta.bpp);
    reader.screen_rect.cx = meta.width;
    reader.screen_rect.cy = meta.height;
    StaticCapture consumer(meta.width, meta.height,
                           "/tmp/test_file_to_png.png", 0, 0);
    reader.consumer = &consumer;
    while (reader.next())
        ;
    consumer.flush();

    char message[1024];
    if (!check_sig(consumer.drawable, message, shasig)){
        BOOST_CHECK_MESSAGE(false, message);
    }
    ::close(fd);

    TODO("if boost::unit_test::error_count() == 0");
    unlink_wrm(filename, 1);
    unlink_png("/tmp/test_file_to_png.png", 1);
}

BOOST_AUTO_TEST_CASE(TestFileToPng)
{
    {
        //MetaWRM meta(800, 600, 24);
        MetaWRM meta(1024, 912, 16);
        NativeCapture cap(meta.width, meta.height,
                          "/tmp/test_file_to_png");
        BOOST_CHECK_EQUAL(cap.recorder.stream.p - cap.recorder.stream.data, 8);
        meta.emit(cap.recorder);
        Rect clip(0, 0, meta.width, meta.height);
        cap.draw(RDPOpaqueRect(Rect(10,844,500,42), RED), clip);
        cap.draw(RDPOpaqueRect(Rect(777,110,144,188), GREEN), clip);
        cap.draw(RDPOpaqueRect(Rect(200,400,60,60), BLUE), clip);
        struct timeval now;
        gettimeofday(&now, NULL);
        cap.recorder.timestamp(now);
    }
    file_to_png("/tmp/test_file_to_png", 1024, 912, 16,
                     "\x10\xa4\xc3\xe2\x18\x1e\x00\x51\xcc\x9b"
                     "\x09\xaf\xf3\x20\xb5\xb2\xba\xb7\x38\x21");
}

BOOST_AUTO_TEST_CASE(TestFileWithoutMetaToPng)
{
    {
        Rect clip(0, 0, 800, 600);
        NativeCapture cap(clip.cx, clip.cy,
                          "/tmp/test_file_without_meta_to_png");
        cap.draw(RDPOpaqueRect(Rect(10,500,500,42), RED), clip);
        cap.draw(RDPOpaqueRect(Rect(600,110,144,188), GREEN), clip);
        cap.draw(RDPOpaqueRect(Rect(200,400,60,60), BLUE), clip);
        struct timeval now;
        gettimeofday(&now, NULL);
        cap.recorder.timestamp(now);
    }
    file_to_png("/tmp/test_file_without_meta_to_png", 800, 600, 24,
                     "\x1c\xf2\xcc\xf1\x37\x2d\x51\xde\x41\x9e"
                     "\xee\x36\xa3\xa2\x20\x99\x04\x1e\xb7\xba");
}

BOOST_AUTO_TEST_CASE(TestWrmFileToPng)
{
    int fd = ::open(FIXTURES_PATH "/replay2.wrm", O_RDONLY);
    if (fd == -1)
    {
        BOOST_CHECK_MESSAGE(false, "fd == -1");
        return ;
    }
    InFileTransport in_trans(fd);
    RDPUnserializer reader(&in_trans, 0, Rect());
    BOOST_CHECK(1);
    MetaWRM meta(reader);
    reader.screen_rect.cx = meta.width;
    reader.screen_rect.cy = meta.height;
    BOOST_CHECK_EQUAL(800, meta.width);
    BOOST_CHECK_EQUAL(600, meta.height);
    BOOST_CHECK_EQUAL(24, meta.bpp);

    StaticCapture consumer(meta.width, meta.height,
                           "/tmp/test_replay_to_png", 0, 0);

    bool is_chunk_time = false;
    uint count_img = 1;

    reader.consumer = &consumer;
    while (reader.selected_next_order())
    {
        if (reader.chunk_type == WRMChunk::TIMESTAMP || reader.chunk_type == WRMChunk::OLD_TIMESTAMP){
            is_chunk_time = true;
            reader.remaining_order_count = 0;
        } else {
            reader.interpret_order();
            if (is_chunk_time)
            {
                consumer.flush();
                ++count_img;
                is_chunk_time = false;
            }
        }
    }
    TODO("if boost::unit_test::error_count() == 0");
    unlink_png("/tmp/test_replay_to_png", count_img);
}
