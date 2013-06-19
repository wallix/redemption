/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#define BOOST_TEST_MODULE TestWrmCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "capture.hpp"

BOOST_AUTO_TEST_CASE(TestSplittedCapture)
{
    Inifile ini;
    const int groupid = 0;
    {
        // Timestamps are applied only when flushing
        struct timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);
        
        ini.video.frame_interval = 100; // one timestamp every second
        ini.video.break_interval = 3;   // one WRM file every 5 seconds

        ini.video.png_limit = 10; // one snapshot by second
        ini.video.png_interval = 10; // one snapshot by second

        ini.video.capture_wrm = true;
        ini.video.capture_png = true;
        ini.globals.enable_file_encryption = false;
        Capture capture(now, scr.cx, scr.cy, "./", "./", "/tmp/", "capture", false, ini);

        capture.draw(RDPOpaqueRect(scr, GREEN), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.flush(); // to close last wrm
        
        BOOST_CHECK_EQUAL((unsigned)3051, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 0));
        sq_outfilename_unlink(&(capture.png_trans->seq), 0);
        BOOST_CHECK_EQUAL((unsigned)3082, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 1));
        sq_outfilename_unlink(&(capture.png_trans->seq), 1);
        BOOST_CHECK_EQUAL((unsigned)3102, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 2));
        sq_outfilename_unlink(&(capture.png_trans->seq), 2);
        // 3109
        BOOST_CHECK_EQUAL((unsigned)3108, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 3));
        sq_outfilename_unlink(&(capture.png_trans->seq), 3);
        BOOST_CHECK_EQUAL((unsigned)3136, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 4));
        sq_outfilename_unlink(&(capture.png_trans->seq), 4);
        BOOST_CHECK_EQUAL((unsigned)3145, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 5));
        sq_outfilename_unlink(&(capture.png_trans->seq), 5);
        // 3176
        BOOST_CHECK_EQUAL((unsigned)3177, (unsigned)sq_outfilename_filesize(&(capture.png_trans->seq), 6));
        sq_outfilename_unlink(&(capture.png_trans->seq), 6);

        SQ wrm_seq;
        sq_init_outfilename(&wrm_seq, SQF_PATH_FILE_PID_COUNT_EXTENSION, "./", "capture", ".wrm", groupid);        
        BOOST_CHECK_EQUAL((unsigned)1622, (unsigned)sq_outfilename_filesize(&wrm_seq, 0));
        sq_outfilename_unlink(&wrm_seq, 0);
        // Mem3Blt save state = 34 bytes
        BOOST_CHECK_EQUAL((unsigned)3392 + 34, (unsigned)sq_outfilename_filesize(&wrm_seq, 1));
        sq_outfilename_unlink(&wrm_seq, 1);
        // Mem3Blt save state = 34 bytes
        BOOST_CHECK_EQUAL((unsigned)3371 + 34, (unsigned)sq_outfilename_filesize(&wrm_seq, 2));
        sq_outfilename_unlink(&wrm_seq, 2);
        // The destruction of capture object will finalize the metafile content
    }
    SQ meta_seq;
    sq_init_outfilename(&meta_seq, SQF_PATH_FILE_PID_EXTENSION, "./", "capture", ".mwrm", groupid);
    BOOST_CHECK_EQUAL((unsigned)125, (unsigned)sq_outfilename_filesize(&meta_seq, 0));
    sq_outfilename_unlink(&meta_seq, 0);

    if (ini.globals.enable_file_encryption){
        sq_init_outfilename(&meta_seq, SQF_PATH_FILE_PID_EXTENSION, "/tmp/", "capture", ".mwrm", groupid);
        BOOST_CHECK_EQUAL((unsigned)32, (unsigned)sq_outfilename_filesize(&meta_seq, 0));
        sq_outfilename_unlink(&meta_seq, 0);
    }
}
