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
#define BOOST_TEST_MODULE TestBreakpoint
#include <boost/test/auto_unit_test.hpp>
#include <boost/lexical_cast.hpp>

#define LOGPRINT

#include "capture.hpp"
#include "transport.hpp"
#include "recorder/wrm_recorder_option.hpp"

#include "unlink.hpp"
#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestBreakpoint)
{
    const char * filename_base = "/tmp/test_breakpoint";

    BOOST_CHECK(1);
    {
        int w = 1024, h = 912;
        struct timeval now;
        gettimeofday(&now, NULL);
        Capture cap(now, w, h, filename_base, 0, 0);
        cap.start(now);
        Rect clip(0, 0, w, h);
        cap.draw(RDPOpaqueRect(Rect(10,844,500,42), RED), clip);
        BOOST_CHECK(1);
        cap.breakpoint();
        BOOST_CHECK(1);
        cap.draw(RDPOpaqueRect(Rect(777,110,144,188), GREEN), clip);
        cap.breakpoint();
        BOOST_CHECK(1);
        cap.draw(RDPOpaqueRect(Rect(200,400,60,60), BLUE), clip);
        cap.timestamp();
        BOOST_CHECK(1);
    }

    WRMRecorder recorder;
    BOOST_CHECK(1);
    std::string mwrm_filename = filename_base;
    mwrm_filename += '-';
    mwrm_filename += boost::lexical_cast<std::string>(getpid());
    mwrm_filename += ".mwrm";
    recorder.open_meta_followed_wrm(mwrm_filename.c_str());
    StaticCapture consumer(recorder.meta().width,
                           recorder.meta().height,
                           "/tmp/test.png",
                           recorder.meta().width,
                           recorder.meta().height,
                           true);
    recorder.consumer(&consumer);

    const uint16_t next_file_id   = WRMChunk::NEXT_FILE_ID;
    const uint16_t meta_file      = WRMChunk::META_FILE;
    const uint16_t breakpoint     = WRMChunk::BREAKPOINT;
    const uint16_t timestamp      = WRMChunk::TIMESTAMP;
    const uint16_t time_start     = WRMChunk::TIME_START;

    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), time_start);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), 0);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), timestamp);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), next_file_id);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), meta_file);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), breakpoint);
    recorder.interpret_order();

    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), time_start);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), 0);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), timestamp);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), next_file_id);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), meta_file);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), breakpoint);
    recorder.interpret_order();

    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), time_start);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), 0);
    recorder.interpret_order();
    BOOST_REQUIRE(recorder.reader.selected_next_order());
    BOOST_REQUIRE_EQUAL(recorder.chunk_type(), timestamp);
    recorder.interpret_order();

    BOOST_REQUIRE(false == recorder.reader.selected_next_order());


    char message[1024];
    if (!check_sig(consumer.drawable, message,
        "\x10\xa4\xc3\xe2\x18\x1e\x00\x51\xcc\x9b"
        "\x09\xaf\xf3\x20\xb5\xb2\xba\xb7\x38\x21")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    unlink_full_wrm("/tmp/test_breakpoint", 3);
}
