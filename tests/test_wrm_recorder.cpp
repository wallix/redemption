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
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWRMRecorder
#include <boost/test/auto_unit_test.hpp>

#include <unistd.h>

#define LOGPRINT

// #include <iostream>
#include "recorder/wrm_recorder_option.hpp"
#include "transport.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "capture.hpp"
#include "RDP/RDPGraphicDevice.hpp"

#include "unlink.hpp"
#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestWrmToMultiWRM)
{
    BOOST_CHECK(1);
    timeval now;
    gettimeofday(&now, NULL);

    HexadecimalKeyOption in_crypt_key;
    HexadecimalIVOption in_crypt_iv;
    range_time_point range;
    std::string path(FIXTURES_PATH);
    std::string filename(FIXTURES_PATH "/test_w2008_2-880.mwrm");

    WRMRecorder recorder(now, 0, in_crypt_key, in_crypt_iv, InputType::META_TYPE, path, false, false, false, range, filename, 0);

    BOOST_CHECK_EQUAL(800, recorder.reader.data_meta.width);
    BOOST_CHECK_EQUAL(600, recorder.reader.data_meta.height);
    /*BOOST_CHECK_EQUAL(24, recorder.meta.bpp);*/

    uint breakpoint = 0;
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        Capture consumer(now, recorder.reader.data_meta.width, recorder.reader.data_meta.height,
                        "/tmp/replay_part", 0, 0, false);

        recorder.reader.consumer = &consumer;

        uint n = 0;
        uint ntime = 0;
        //uint nb_break = 0;
        BOOST_REQUIRE(1);

        while (recorder.reader.selected_next_order())
        {
            //std::cout << recorder.reader.chunk_type << std::endl;
            //BOOST_CHECK(recorder.reader.chunk_type != WRMChunk::BREAKPOINT);
            if (recorder.reader.chunk_type == WRMChunk::TIMESTAMP)
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
                if (50 < ++n/* && recorder.reader.remaining_order_count == 0*/)
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
        BOOST_CHECK_EQUAL(17, ntime);
    }
    BOOST_REQUIRE(1);

    unlink_full_wrm("/tmp/replay_part", breakpoint+1);
}

void TestMultiWRMToPng_random_file(uint nfile, uint numtest, uint totalframe, const char * sigdata = 0, bool realloc_consumer = false)
{
    BOOST_CHECK(1);
    timeval now;
    gettimeofday(&now, NULL);   

    HexadecimalKeyOption in_crypt_key;
    HexadecimalIVOption in_crypt_iv;
    range_time_point range;
    std::string path("/tmp");
    char cfilename[50];
    sprintf(cfilename, "/tmp/replay_part-%u-%u.wrm", getpid(), nfile);
    std::string filename(cfilename);

    WRMRecorder recorder(now, 0, in_crypt_key, in_crypt_iv, InputType::META_TYPE, path, false, false, false, range, filename, 0);

    BOOST_CHECK_EQUAL(800, recorder.reader.data_meta.width);
    BOOST_CHECK_EQUAL(600, recorder.reader.data_meta.height);
    /*BOOST_CHECK_EQUAL(24, recorder.meta.bpp);*/

    char filename_consumer[50];
    int nframe = 0;
    sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png%u-%d", numtest, nframe);
    StaticCapture *consumer = new StaticCapture(recorder.reader.data_meta.width,
                                                recorder.reader.data_meta.height,
                                                filename_consumer,
                                                true);
    BOOST_CHECK(1);

    recorder.reader.consumer = consumer;
    recorder.redrawable = &consumer->drawable;
    bool is_chunk_time = true;
    BOOST_CHECK(1);

    uint n = 0;

    while (recorder.reader.selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder.reader.chunk_type == WRMChunk::TIMESTAMP){
            is_chunk_time = true;
            recorder.reader.remaining_order_count = 0;
            ++n;
        } else if (recorder.reader.chunk_type == WRMChunk::NEXT_FILE_ID) {
            BOOST_CHECK(1);
            std::size_t n = recorder.reader.stream.in_uint32_le();
            std::string wrm_filename = recorder.reader.data_meta.files[n].wrm_filename;
            BOOST_CHECK(1);
            BOOST_CHECK(1);
            timeval now;
            gettimeofday(&now, NULL);

            HexadecimalKeyOption in_crypt_key;
            HexadecimalIVOption in_crypt_iv;
            range_time_point range;
            std::string path("/tmp");

            WRMRecorder recorder(now, 0, in_crypt_key, in_crypt_iv, InputType::META_TYPE, path, false, false, false, range, wrm_filename, 0);

            if (realloc_consumer)
            {
                delete consumer;
                sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png%u-%d", numtest, ++nframe);
                consumer = new StaticCapture(recorder.reader.data_meta.width,
                                             recorder.reader.data_meta.height,
                                             filename_consumer,
                                             true);
            }
            recorder.reader.consumer = consumer;
            recorder.redrawable = &consumer->drawable;
            BOOST_CHECK(1);
        } else {
            BOOST_CHECK(1);
            if (is_chunk_time)
            {
                BOOST_CHECK(1);
                consumer->flush();
                is_chunk_time = false;
            }
            recorder.interpret_order();
            BOOST_CHECK(1);
        }
        BOOST_CHECK(1);
    }
    BOOST_CHECK(1);
    consumer->flush();

    if (sigdata)
    {
        char message[1024];
        if (!check_sig(consumer->drawable, message, sigdata)){
            BOOST_CHECK_MESSAGE(false, message);
        }
    }

    BOOST_CHECK_EQUAL(n, totalframe);
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

#include "recorder/wrm_recorder_option.hpp"

BOOST_AUTO_TEST_CASE(TestHexadecimalOption)
{
    HexadecimalOption<20> hop;
    bool b = hop.parse("0506070809000A");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(hop.size, 7);
    BOOST_REQUIRE_EQUAL(hop.data[0], 0x05);
    BOOST_REQUIRE_EQUAL(hop.data[1], 0x06);
    BOOST_REQUIRE_EQUAL(hop.data[2], 0x07);
    BOOST_REQUIRE_EQUAL(hop.data[3], 0x08);
    BOOST_REQUIRE_EQUAL(hop.data[4], 0x09);
    BOOST_REQUIRE_EQUAL(hop.data[5], 0x00);
    BOOST_REQUIRE_EQUAL(hop.data[6], 0x0A);
    BOOST_REQUIRE_EQUAL(hop.data[7], 0);
    BOOST_REQUIRE_EQUAL(hop.data[8], 0);
    BOOST_REQUIRE_EQUAL(hop.data[9], 0);
    BOOST_REQUIRE_EQUAL(hop.data[10], 0);
    BOOST_REQUIRE_EQUAL(hop.data[11], 0);
    BOOST_REQUIRE_EQUAL(hop.data[12], 0);
    BOOST_REQUIRE_EQUAL(hop.data[13], 0);
    BOOST_REQUIRE_EQUAL(hop.data[14], 0);
    BOOST_REQUIRE_EQUAL(hop.data[15], 0);
    BOOST_REQUIRE_EQUAL(hop.data[16], 0);
    BOOST_REQUIRE_EQUAL(hop.data[17], 0);
    BOOST_REQUIRE_EQUAL(hop.data[18], 0);
    BOOST_REQUIRE_EQUAL(hop.data[19], 0);

    b = hop.parse("0506070809000a5");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(hop.size, 8);
    BOOST_REQUIRE_EQUAL(hop.data[0], 0x05);
    BOOST_REQUIRE_EQUAL(hop.data[1], 0x06);
    BOOST_REQUIRE_EQUAL(hop.data[2], 0x07);
    BOOST_REQUIRE_EQUAL(hop.data[3], 0x08);
    BOOST_REQUIRE_EQUAL(hop.data[4], 0x09);
    BOOST_REQUIRE_EQUAL(hop.data[5], 0x00);
    BOOST_REQUIRE_EQUAL(hop.data[6], 0x0A);
    BOOST_REQUIRE_EQUAL(hop.data[7], 0x50);
    BOOST_REQUIRE_EQUAL(hop.data[8], 0);
    BOOST_REQUIRE_EQUAL(hop.data[9], 0);
    BOOST_REQUIRE_EQUAL(hop.data[10], 0);
    BOOST_REQUIRE_EQUAL(hop.data[11], 0);
    BOOST_REQUIRE_EQUAL(hop.data[12], 0);
    BOOST_REQUIRE_EQUAL(hop.data[13], 0);
    BOOST_REQUIRE_EQUAL(hop.data[14], 0);
    BOOST_REQUIRE_EQUAL(hop.data[15], 0);
    BOOST_REQUIRE_EQUAL(hop.data[16], 0);
    BOOST_REQUIRE_EQUAL(hop.data[17], 0);
    BOOST_REQUIRE_EQUAL(hop.data[18], 0);
    BOOST_REQUIRE_EQUAL(hop.data[19], 0);

    BOOST_REQUIRE(!HexadecimalOption<5>().parse("0506070809000A"));
}
