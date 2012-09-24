
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

#define LOGPRINT

#include "wrm_recorder.hpp"
#include "staticcapture.hpp"
#include "transport.hpp"

// #include "unlink.hpp"

#include "check_sig.hpp"

/*void file_to_png(const char* filename, uint16_t w, uint16_t h, uint16_t bpp, const char * shasig)
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
    reader.selected_next_order();
    BOOST_REQUIRE(reader.chunk_type == WRMChunk::META_FILE);
    reader.interpret_order();
    BOOST_CHECK(1);
    DataMetaFile& meta = reader.data_meta;
    BOOST_CHECK_EQUAL(w, meta.width);
    BOOST_CHECK_EQUAL(h, meta.height);
    StaticCapture consumer(meta.width, meta.height,
                           "/tmp/test_file_to_png.png", 0, 0);
    reader.consumer = &consumer;
    while (reader.next())
        ;
    //consumer.flush();

    char message[1024];
    if (!check_sig(consumer.drawable, message, shasig)){
        BOOST_CHECK_MESSAGE(false, message);
    }
    ::close(fd);

    TODO("if boost::unit_test::error_count() == 0");
    unlink_wrm(filename, 1);
}

BOOST_AUTO_TEST_CASE(TestFileWithoutMetaToPng)
{
    Rect clip(0, 0, 800, 600);
    {
        NativeCapture cap(clip.cx, clip.cy,
                          "/tmp/test_file_without_meta_to_png");
        cap.draw(RDPOpaqueRect(Rect(10,500,500,42), RED), clip);
        cap.draw(RDPOpaqueRect(Rect(600,110,144,188), GREEN), clip);
        cap.draw(RDPOpaqueRect(Rect(200,400,60,60), BLUE), clip);
        struct timeval now;
        gettimeofday(&now, NULL);
        cap.recorder.timestamp(now);
    }
    file_to_png("/tmp/test_file_without_meta_to_png", clip.cx, clip.cy, 24,
                "\x1c\xf2\xcc\xf1\x37\x2d\x51\xde\x41\x9e"
                "\xee\x36\xa3\xa2\x20\x99\x04\x1e\xb7\xba");
    unlink_mwrm_and_wrm("/tmp/test_file_without_meta_to_png", 1);
}*/
#include <iostream>
BOOST_AUTO_TEST_CASE(TestWrmFileToPng)
{
    BOOST_CHECK(true);
    WRMRecorder reader(FIXTURES_PATH "/test_w2008_2-880.mwrm", FIXTURES_PATH);
    BOOST_CHECK(true);
    DataMetaFile& meta = reader.meta();
    BOOST_CHECK_EQUAL(reader.is_meta_chunk(), true);
    BOOST_CHECK_EQUAL(800, meta.width);
    BOOST_CHECK_EQUAL(600, meta.height);

    std::cout << "cipher is active: " << reader.cipher_is_active() << std::endl;

    StaticCapture consumer(meta.width, meta.height, "/tmp/test_replay_to_png", NULL, NULL, 10);

    bool is_chunk_time = false;
    uint count_img = 0;

    reader.consumer(&consumer);
    while (reader.selected_next_order())
    {
        if (reader.chunk_type() == WRMChunk::TIMESTAMP){
            is_chunk_time = true;
            reader.ignore_chunks();
        } else {
            reader.interpret_order();
            if (is_chunk_time)
            {
                consumer.dump_png();
                ++count_img;
                is_chunk_time = false;
            }
        }
    }
    //consumer.dump_png();
    ++count_img;
    BOOST_CHECK_EQUAL(count_img, 7);
    char mess[1024];
    if (!check_sig(consumer.drawable, mess,
        "\xd0\x8a\xe3\x69\x7c\x88\x91\xf8\xc4\xf5"
        "\xd8\x90\xaa\xaa\xec\x13\xd0\xde\x1c\xe1"))
    {
        BOOST_CHECK_MESSAGE(false, mess);
    }
    //unlink_png("/tmp/test_replay_to_png", count_img);
}
