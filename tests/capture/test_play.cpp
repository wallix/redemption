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
#define BOOST_TEST_MODULE TestWrmCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include <sys/time.h>
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"

//BOOST_AUTO_TEST_CASE(TestTranscodeWRM)
//{
//    const char * input_filename = "./tests/fixtures/capture.wrm";
//    InByFilenameTransport in_wrm_trans(input_filename);
//    FileToGraphic player(&in_wrm_trans);

//    Inifile ini;
//    ini.globals.debug.primary_orders = 127;
//    ini.globals.debug.secondary_orders = 127;
//    ini.globals.png_limit = 3;
//    ini.globals.png_interval = 1;

//    FileSequence png_sequence("path file pid count extension", "./", "testxxx", "png");
//    OutByFilenameSequenceTransport out_png_trans(png_sequence);
//    StaticCapture png_recorder(
//        player.replay_now,
//        out_png_trans,
//        png_sequence,
//        player.screen_rect.cx,
//        player.screen_rect.cy);
//        
//    png_recorder.update_config(ini);
//    player.add_consumer(&png_recorder);

//    FileSequence wrm_sequence("path file pid count extension", "./", "testxxx", "wrm");
//    OutByFilenameSequenceTransport out_wrm_trans(wrm_sequence);
//    ini.globals.frame_interval = 10;
//    ini.globals.break_interval = 10;
//    
//    BmpCache bmp_cache(
//        player.bmp_cache->bpp,
//        player.bmp_cache->small_entries,
//        player.bmp_cache->small_size,
//        player.bmp_cache->medium_entries,
//        player.bmp_cache->medium_size,
//        player.bmp_cache->big_entries,
//        player.bmp_cache->big_size); 

//    NativeCapture wrm_recorder(
//        player.replay_now,
//        out_wrm_trans,
//        player.screen_rect.cx,
//        player.screen_rect.cy,
//        bmp_cache,
//        ini);

//    wrm_recorder.update_config(ini);
//    player.add_consumer(&wrm_recorder);

//    player.play();
//}

BOOST_AUTO_TEST_CASE(TestBogusWRM)
{
//    const char * input_filename = "./bogus.wrm";
//    InByFilenameTransport in_wrm_trans(input_filename);
//    FileToGraphic player(&in_wrm_trans);

//    Inifile ini;
//    ini.globals.debug.primary_orders = 127;
//    ini.globals.debug.secondary_orders = 127;

//    FileSequence png_sequence("path file pid count extension", "./", "testxxx", "png");
//    OutByFilenameSequenceTransport out_png_trans(png_sequence);
//    ImageCapture png_recorder(out_png_trans, player.screen_rect.cx, player.screen_rect.cy);
//        
//    png_recorder.update_config(ini);
//    player.add_consumer(&png_recorder);


////    FileSequence wrm_sequence("path file pid count extension", "./", "bogbogbog", "wrm");
////    OutByFilenameSequenceTransport out_wrm_trans(wrm_sequence);
////    ini.globals.frame_interval = 10;
////    ini.globals.break_interval = 10;
////    
////    BmpCache bmp_cache(
////        player.bmp_cache->bpp,
////        player.bmp_cache->small_entries,
////        player.bmp_cache->small_size,
////        player.bmp_cache->medium_entries,
////        player.bmp_cache->medium_size,
////        player.bmp_cache->big_entries,
////        player.bmp_cache->big_size); 

////    NativeCapture wrm_recorder(
////        player.replay_now,
////        out_wrm_trans,
////        player.screen_rect.cx,
////        player.screen_rect.cy,
////        bmp_cache, ini);

////    wrm_recorder.update_config(ini);
////    player.add_consumer(&wrm_recorder);

////    BOOST_CHECK_EQUAL((unsigned)1352220800, (unsigned)player.playtime_now.tv_sec);
//    png_recorder.flush();
//    player.play();
////    out_png_trans.next();
//    png_recorder.flush();
////    BOOST_CHECK_EQUAL((unsigned)1352220800, (unsigned)player.playtime_now.tv_sec);
//   

}
