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

// #define LOGPRINT

#include <iostream>
#include "wrm_recorder.hpp"
#include "transport.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "RDP/RDPGraphicDevice.hpp"

BOOST_AUTO_TEST_CASE(TestWRMRecordeToMultiWRM)
{
    BOOST_CHECK(1);
    WRMRecorder recorder(FIXTURES_PATH "/replay2.wrm");
    BOOST_CHECK_EQUAL(800, recorder.meta.width);
    BOOST_CHECK_EQUAL(600, recorder.meta.height);
    BOOST_CHECK_EQUAL(24, recorder.meta.bpp);

    NativeCapture consumer(recorder.meta.width, recorder.meta.height,
                           "/tmp/replay_part");

    recorder.consumer(&consumer);

    uint n = 0;
    uint ntime = 0;
    BOOST_CHECK(1);

    while (recorder.selected_next_order())
    {
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP || recorder.chunk_type() == WRMChunk::OLD_TIMESTAMP)
        {
            ++ntime;
            consumer.recorder.timestamp();
            recorder.remaining_order_count() = 0;
        }
        else
        {
            BOOST_CHECK(1);
            recorder.interpret_order();
            BOOST_CHECK(1);
            if (50 < ++n && recorder.remaining_order_count() == 0)
            {
                n = 0;
                consumer.breakpoint();
            }
            BOOST_CHECK(1);
        }
    }
    BOOST_CHECK(328 == ntime);
}

BOOST_AUTO_TEST_CASE(TestMultiWRMRecordeToPng)
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

    while (recorder.selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP || recorder.chunk_type() == WRMChunk::OLD_TIMESTAMP){
            is_chunk_time = true;
            recorder.remaining_order_count() = 0;
            ++n;
        } else {
            BOOST_CHECK(1);
            if (is_chunk_time)
            {
                BOOST_CHECK(1);
                consumer.flush();
                is_chunk_time = false;
            }
            recorder.interpret_order();
            BOOST_CHECK(1);
        }
        BOOST_CHECK(1);
    }
    BOOST_CHECK(1);
    consumer.flush();

    BOOST_CHECK(328 == n);
}

BOOST_AUTO_TEST_CASE(TestMultiWRMRecordeToPng2)
{
    char filename[50];
    sprintf(filename, "/tmp/replay_part-%u-0.wrm", getpid());
    BOOST_CHECK(1);
    WRMRecorder* recorder = new WRMRecorder(filename);
    BOOST_CHECK_EQUAL(800, recorder->meta.width);
    BOOST_CHECK_EQUAL(600, recorder->meta.height);
    BOOST_CHECK_EQUAL(24, recorder->meta.bpp);

    StaticCapture consumer(recorder->meta.width, recorder->meta.height,
                           "/tmp/test_wrm_recorder_to_png2", 0, 0);
    BOOST_CHECK(1);

    recorder->consumer(&consumer);
    bool is_chunk_time = true;
    BOOST_CHECK(1);

    uint n = 0;

    while (recorder->selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder->chunk_type() == WRMChunk::TIMESTAMP || recorder->chunk_type() == WRMChunk::OLD_TIMESTAMP){
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
            recorder->consumer(&consumer);
            BOOST_CHECK(1);
        } else {
            BOOST_CHECK(1);
            if (is_chunk_time)
            {
                BOOST_CHECK(1);
                consumer.flush();
                is_chunk_time = false;
            }
            recorder->interpret_order();
            BOOST_CHECK(1);
        }
        BOOST_CHECK(1);
    }
    BOOST_CHECK(1);
    consumer.flush();

    BOOST_CHECK(328 == n);
    delete recorder;
    //std::cout << n << '\n';
}

BOOST_AUTO_TEST_CASE(TestMultiWRMRecordeToPng3)
{
    char filename[50];
    sprintf(filename, "/tmp/replay_part-%u-0.wrm", getpid());
    BOOST_CHECK(1);
    WRMRecorder* recorder = new WRMRecorder(filename);
    BOOST_CHECK_EQUAL(800, recorder->meta.width);
    BOOST_CHECK_EQUAL(600, recorder->meta.height);
    BOOST_CHECK_EQUAL(24, recorder->meta.bpp);

    char filename_consumer[50];
    int nframe = 0;
    sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png3-%d", nframe);
    StaticCapture *consumer = new StaticCapture(recorder->meta.width,
                                                recorder->meta.height,
                                                filename_consumer,
                                                0, 0);
    BOOST_CHECK(1);

    recorder->consumer(consumer);
    bool is_chunk_time = true;
    BOOST_CHECK(1);

    uint n = 0;

    while (recorder->selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder->chunk_type() == WRMChunk::TIMESTAMP || recorder->chunk_type() == WRMChunk::OLD_TIMESTAMP){
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
            delete consumer;
            sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png3-%d", ++nframe);
            consumer = new StaticCapture(recorder->meta.width,
                                         recorder->meta.height,
                                         filename_consumer,
                                         0, 0);
            recorder->consumer(consumer);
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

    BOOST_CHECK(328 == n);
    delete recorder;
    delete consumer;
    //std::cout << n << '\n';
}

BOOST_AUTO_TEST_CASE(TestMultiWRMRecordeToPng4)
{
    char filename[50];
    sprintf(filename, "/tmp/replay_part-%u-5.wrm", getpid());
    BOOST_CHECK(1);
    WRMRecorder* recorder = new WRMRecorder(filename);
    BOOST_CHECK_EQUAL(800, recorder->meta.width);
    BOOST_CHECK_EQUAL(600, recorder->meta.height);
    BOOST_CHECK_EQUAL(24, recorder->meta.bpp);

    char filename_consumer[50];
    int nframe = 0;
    sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png4-%d", nframe);
    StaticCapture *consumer = new StaticCapture(recorder->meta.width,
                                                recorder->meta.height,
                                                filename_consumer,
                                                0, 0);
    BOOST_CHECK(1);

    recorder->consumer(consumer);
    if (recorder->reader.chunk_type != WRMChunk::META_INFO)
        recorder->interpret_order();
    bool is_chunk_time = true;
    BOOST_CHECK(1);

    uint n = 0;

    while (recorder->selected_next_order())
    {
        BOOST_CHECK(1);
        if (recorder->chunk_type() == WRMChunk::TIMESTAMP || recorder->chunk_type() == WRMChunk::OLD_TIMESTAMP){
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
            delete consumer;
            sprintf(filename_consumer, "/tmp/test_wrm_recorder_to_png4-%d", ++nframe);
            consumer = new StaticCapture(recorder->meta.width,
                                         recorder->meta.height,
                                         filename_consumer,
                                         0, 0);
            recorder->consumer(consumer);
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

    //BOOST_CHECK(323 == n);
    delete recorder;
    delete consumer;
    //std::cout << n << '\n';
}