/*
 This program *is free software; you can redistribute it and/or modify
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

#include "recorder/wrm_recorder_option.hpp"
#include "staticcapture.hpp"
#include "transport.hpp"
#include "check_sig.hpp"

#include <iostream>
BOOST_AUTO_TEST_CASE(TestWrmFileToPng)
{
    BOOST_CHECK(true);
    timeval now;
    gettimeofday(&now, NULL);

    HexadecimalKeyOption in_crypt_key;
    HexadecimalIVOption in_crypt_iv;
    range_time_point range;
    std::string path(FIXTURES_PATH);
    std::string filename(FIXTURES_PATH "/test_w2008_2-880.mwrm");

    InFileTransport trans(-1);
    RDPUnserializer reader(&trans, now, 0, Rect());

    WRMRecorder recorder(now, trans, reader, InputType::META_TYPE, path, false, false, false, range, filename, 0);
    BOOST_CHECK(true);
    DataMetaFile& meta = recorder.reader.data_meta;
    BOOST_CHECK_EQUAL(800, meta.width);
    BOOST_CHECK_EQUAL(600, meta.height);

    StaticCapture consumer(meta.width, meta.height, "/tmp/test_replay_to_png", true);

    bool is_chunk_time = false;
    uint count_img = 0;

    recorder.reader.consumer = &consumer;
    while (recorder.reader.selected_next_order())
    {
        if (recorder.reader.chunk_type == WRMChunk::TIMESTAMP){
            is_chunk_time = true;
            recorder.reader.stream.p = recorder.reader.stream.end;
            recorder.reader.remaining_order_count = 0;

        } else {
            recorder.interpret_order();
            if (is_chunk_time)
            {
                //consumer.dump_png();
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
