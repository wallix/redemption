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
#define BOOST_TEST_MODULE TestWRMRecorde
#include <boost/test/auto_unit_test.hpp>

#include <unistd.h>

// #define LOGPRINT

// #include <iostream>
#include "wrm_recorder.hpp"
#include "transport.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "capture.hpp"
#include "RDP/RDPGraphicDevice.hpp"

#include "unlink.hpp"

BOOST_AUTO_TEST_CASE(TestWrmToMultiWRM)
{
    BOOST_CHECK(1);
    WRMRecorder recorder(FIXTURES_PATH "/test_w2008_2-5446.mwrm", FIXTURES_PATH);

    BOOST_CHECK_EQUAL(800, recorder.meta().width);
    BOOST_CHECK_EQUAL(600, recorder.meta().height);
    /*BOOST_CHECK_EQUAL(24, recorder.meta.bpp);*/

    uint breakpoint = 0;
    {
        Capture consumer(recorder.meta().width, recorder.meta().height,
                        "/tmp/replay_part", 0, 0, false);

        recorder.consumer(&consumer);

        uint n = 0;
        uint ntime = 0;
        //uint nb_break = 0;
        BOOST_REQUIRE(1);

        while (recorder.selected_next_order())
        {
            //std::cout << recorder.chunk_type() << std::endl;
            //BOOST_CHECK(recorder.chunk_type() != WRMChunk::BREAKPOINT);
            if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
            {
                ++ntime;
                consumer.timestamp(recorder.reader.stream.in_uint64_be());
                --recorder.reader.remaining_order_count;
            }
            else
            {
                BOOST_REQUIRE(1);
                recorder.interpret_order();
                BOOST_REQUIRE(1);
                if (50 < ++n/* && recorder.remaining_order_count() == 0*/)
                {
                    n = 0;
                    //std::cout << "breakpoint start\n";
                    ++breakpoint;
                    consumer.breakpoint();
                    //std::cout << "breakpoint stop\n";
                    //if (++nb_break == 3)
                    //return ; ///WARNING
                }
                BOOST_REQUIRE(1);
            }
        }
        //consumer.breakpoint();
        BOOST_CHECK_EQUAL(333, ntime);
    }
    BOOST_REQUIRE(1);

    unlink_wrm("/tmp/replay_part", breakpoint+1);
}

/*BOOST_AUTO_TEST_CASE(TestMultiWRMToPng)
{
    char filename[50];
    sprintf(filename, "/tmp/replay_part-%u-0.wrm", getpid());
    BOOST_CHECK(1);
    WRMRecorder recorder(filename);
    BOOST_CHECK_EQUAL(800, recorder.meta.width);
    BOOST_CHECK_EQUAL(600, recorder.meta.height);
    BOOST_CHECK_EQUAL(24, recorder.meta.bpp);

    StaticCapture consumer(recorder.meta.width, recorder.meta.height,
                           "/tmp/test_wrm_recorder_to_png", 0, 0);
    BOOST_CHECK(1);

    recorder.consumer(&consumer);
    bool is_chunk_time = true;
    BOOST_CHECK(1);

    uint n = 0;
    //std::cout << "TestMultiWRMToPng start\n";

    while (recorder.selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP){
            is_chunk_time = true;
            //std::cout << "timestamp\n";
            recorder.remaining_order_count() = 0;
            ++n;
        } else {
            //if (consumer.framenb == 39){
            //    std::cout << recorder.chunk_type() << '\n';
            //}
            BOOST_CHECK(1);
            if (is_chunk_time)
            {
                BOOST_CHECK(1);
                consumer.flush();
                is_chunk_time = false;
            }
            //bool is_breakpoint = recorder.chunk_type() == WRMChunk::BREAKPOINT;
            //bool is_nextfile = recorder.chunk_type() == WRMChunk::NEXT_FILE;
            //if (is_breakpoint){
            //    bzero(consumer.drawable.data, consumer.drawable.pix_len);
            //}
            //if (is_nextfile)
            //    std::cout << "NEXT_FILE" << std::endl;
            //if (is_breakpoint)
            //    std::cout << "BREAKPOINT" << std::endl;
            recorder.interpret_order();
            //if (is_breakpoint)
            //    std::cout << "BREAKPOINT DONE" << std::endl;
            //if (is_nextfile)
            //    std::cout << "NEXT_FILE DONE" << std::endl;
            BOOST_CHECK(1);
        }
        BOOST_CHECK(1);
    }
    BOOST_CHECK(1);
    consumer.flush();

    BOOST_CHECK(328 == n);
}*/

void TestMultiWRMToPng_random_file(uint nfile, uint numtest, uint totalframe, const uint8_t * sigdata = 0, bool realloc_consumer = false)
{
    char filename[50];
    sprintf(filename, "/tmp/replay_part-%u-%u.wrm", getpid(), nfile);
    BOOST_CHECK(1);
    WRMRecorder* recorder = new WRMRecorder(filename);
    BOOST_CHECK_EQUAL(800, recorder->meta().width);
    BOOST_CHECK_EQUAL(600, recorder->meta().height);
    /*BOOST_CHECK_EQUAL(24, recorder->meta.bpp);*/

    char filename_consumer[50];
    int nframe = 0;
    sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png%u-%d", numtest, nframe);
    StaticCapture *consumer = new StaticCapture(recorder->meta().width,
                                                recorder->meta().height,
                                                filename_consumer,
                                                0, 0);
    BOOST_CHECK(1);

    recorder->consumer(consumer);
    recorder->redraw_consumer(consumer);
    bool is_chunk_time = true;
    BOOST_CHECK(1);

    uint n = 0;

    while (recorder->selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder->chunk_type() == WRMChunk::TIMESTAMP){
            is_chunk_time = true;
            recorder->remaining_order_count() = 0;
            ++n;
        } else if (recorder->chunk_type() == WRMChunk::NEXT_FILE) {
            BOOST_CHECK(1);
            size_t len = recorder->reader.stream.in_uint32_le();
            recorder->reader.stream.in_copy_bytes((uint8_t*)filename, len);
            BOOST_CHECK(1);
            filename[len] = 0;
            BOOST_CHECK(1);
            delete recorder;
            BOOST_CHECK(1);
            recorder = new WRMRecorder(filename);
            if (realloc_consumer)
            {
                delete consumer;
                sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png%u-%d", numtest, ++nframe);
                consumer = new StaticCapture(recorder->meta().width,
                                             recorder->meta().height,
                                             filename_consumer,
                                             0, 0);
            }
            recorder->consumer(consumer);
            recorder->redraw_consumer(consumer);
            BOOST_CHECK(1);
        } else {
            BOOST_CHECK(1);
            if (is_chunk_time)
            {
                BOOST_CHECK(1);
                consumer->flush();
                is_chunk_time = false;
            }
            recorder->interpret_order();
            BOOST_CHECK(1);
        }
        BOOST_CHECK(1);
    }
    BOOST_CHECK(1);
    consumer->flush();

    SSL_SHA1 sha1;
    ssllib ssl;
    ssl.sha1_init(&sha1);
    uint16_t rowsize = consumer->drawable.width;
    for (size_t y = 0; y < consumer->drawable.height; y++){
        ssl.sha1_update(&sha1, consumer->drawable.data + y * rowsize, rowsize);
    }
    uint8_t sig[20];
    ssl.sha1_final(&sha1, sig);
    if (!sigdata || memcmp(sig, sigdata, 20)){
        char message[200];
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
        if (sigdata)
            BOOST_CHECK_MESSAGE(false, message);
        else
            puts(message);
    }

    BOOST_CHECK_EQUAL(n, totalframe);
    delete recorder;
    delete consumer;
    //std::cout << n << '\n';
}

/*BOOST_AUTO_TEST_CASE(TestMultiWRMToPng2)
{
    TestMultiWRMToPng_random_file(0, 2, 328, 0);
}*/

//BOOST_AUTO_TEST_CASE(TestMultiWRMToPng3)
//{
//    TestMultiWRMToPng_random_file(0, 3, 328,
//                                  (const uint8_t*)
//                                  "\xf3\x72\x33\x18\x5a\x8b\xd8\xf1\x46\xfd"
//                                  "\x69\x48\x48\x9c\xe7\x9b\x4e\x59\x1b\xa1",
//                                  true);
//}

//BOOST_AUTO_TEST_CASE(TestMultiWRMToPng4)
//{
//    TestMultiWRMToPng_random_file(60, 4, 313,
//                                  (const uint8_t*)
//                                  "\xf3\x72\x33\x18\x5a\x8b\xd8\xf1\x46\xfd"
//                                  "\x69\x48\x48\x9c\xe7\x9b\x4e\x59\x1b\xa1",
//                                  true);
//}
