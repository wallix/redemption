/*
   This program is free software; you can redistribute it and/or modify
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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestStaticCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "transport.hpp"
#include "outfilenametransport.hpp"
#include "staticcapture.hpp"
#include "OSDCapture.hpp"
#include "RDP/caches/bmpcache.hpp"


BOOST_AUTO_TEST_CASE(TestOneRedScreen)
{
    Rect screen_rect(0, 0, 800, 600);
    const int groupid = 0;
    OutFilenameTransport trans(SQF_PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test", ".png", groupid);

    struct timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;

    Inifile ini;
    ini.video.png_limit = -1;
    ini.video.png_interval = 0;
    RDPDrawable drawable(800, 600);
    StaticCapture consumer(now, trans, &(trans.seq), 800, 600, false, ini, drawable.drawable);

    consumer.set_pointer_display();

    bool ignore_frame_in_timeval = false;

    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), RED);
    drawable.draw(cmd, screen_rect);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

    {
        Drawable tmp_drawable(50,50);
        Rect tmp_drawable_rect(0,0,50,50);
        tmp_drawable.white_color(tmp_drawable_rect);
        OSDCapture osd(drawable, drawable.drawable, tmp_drawable, 120, 120);
        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

        RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), GREEN);
        osd.draw(cmd1, screen_rect);
        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);
    }

    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

    RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), BLUE);
    drawable.draw(cmd1, screen_rect);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

    rio_clear(&trans.rio); // ensure file is closed to have accurate values for size

    BOOST_CHECK_EQUAL(3083, sq_outfilename_filesize(&(trans.seq), 1));
    BOOST_CHECK_EQUAL(3100, sq_outfilename_filesize(&(trans.seq), 2));
    BOOST_CHECK_EQUAL(3082, sq_outfilename_filesize(&(trans.seq), 3));
    sq_outfilename_unlink(&(trans.seq), 0);
    sq_outfilename_unlink(&(trans.seq), 1);
    sq_outfilename_unlink(&(trans.seq), 2);
    sq_outfilename_unlink(&(trans.seq), 3);
    sq_outfilename_unlink(&(trans.seq), 4);
}

