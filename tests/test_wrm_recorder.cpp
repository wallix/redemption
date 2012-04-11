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

#include "wrm_recorder.hpp"
#include "transport.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"

BOOST_AUTO_TEST_CASE(TestWRMRecordeToMultiWRM)
{
    BOOST_CHECK(1);
    WRMRecorder recorder("/tmp/replay.wrm");
    BOOST_CHECK_EQUAL(800, recorder.meta.width);
    BOOST_CHECK_EQUAL(600, recorder.meta.height);
    BOOST_CHECK_EQUAL(24, recorder.meta.bpp);

    BGRPalette palette;
    if (recorder.meta.bpp == 8)
        init_palette332(palette);
    NativeCapture consumer(recorder.meta.width, recorder.meta.height,
                          recorder.meta.bpp, palette,
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
            if (200 > ++n && recorder.remaining_order_count() == 0)
            {
                n = 0;
                consumer.breakpoint();
            }
            BOOST_CHECK(1);
        }
    }
    std::cout << ntime << '\n';
    BOOST_CHECK(1);
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

    BGRPalette palette;
    if (recorder.meta.bpp == 8)
        init_palette332(palette);
    StaticCapture consumer(recorder.meta.width, recorder.meta.height,
                           recorder.meta.bpp, palette,
                           "/tmp/test_wrm_recorder_to_png", 0, 0);
    BOOST_CHECK(1);

    recorder.consumer(&consumer);
    bool is_chunk_time = false;
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
            if (recorder.chunk_type() == WRMChunk::NEXT_FILE){
                std::cout << "WRMChunk::NEXT_FILE\n";
                recorder.interpret_order();
                std::cout << "NEXT_FILE done\n";
            }
            else if (recorder.chunk_type() == WRMChunk::BREAKPOINT){
                std::cout << "WRMChunk::BREAKPOINT\n";
                recorder.interpret_order();
                std::cout << "BREAKPOINT done\n";
            }
            else
            {
                if (is_chunk_time)
                {
                    BOOST_CHECK(1);
                    consumer.flush();
                    is_chunk_time = false;
                }
                recorder.interpret_order();
            }
            BOOST_CHECK(1);
        }
        BOOST_CHECK(1);
    }
    BOOST_CHECK(1);
    consumer.flush();
    BOOST_CHECK(1);

    std::cout << n << '\n';
}