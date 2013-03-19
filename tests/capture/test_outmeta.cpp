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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   Unit test for OutByFilenameSequenceWithMetaTransport

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOutByFilenameSequenceWithMetaTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "transport.hpp"
#include "outmetatransport.hpp"

BOOST_AUTO_TEST_CASE(TestOutmetaTransport)
{
//    FileSequence meta_sequence("path file pid extension", path, basename, "mwrm");
//    FileSequence wrm_sequence("path file pid count extension", path, basename, "wrm");

//    OutByFilenameSequenceWithMetaTransport wrm_trans(meta_sequence, now, width, height, wrm_sequence);
    {
        FileSequence * wrm_sequence = NULL;
        timeval now;
        now.tv_sec = 1352304810;
        now.tv_usec = 0;
        OutmetaTransport wrm_trans("./", "xxx", now, 800, 600, &wrm_sequence);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    char meta_path[1024];
    char file1[1024];
    char file2[1024];
    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
    BOOST_CHECK_EQUAL(103, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));

    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));   
}

//BOOST_AUTO_TEST_CASE(TestOutmetaTransport2)
//{
////    FileSequence meta_sequence("path file pid extension", path, basename, "mwrm");
////    FileSequence wrm_sequence("path file pid count extension", path, basename, "wrm");

////    OutByFilenameSequenceWithMetaTransport wrm_trans(meta_sequence, now, width, height, wrm_sequence);
//    {
//        FileSequence * wrm_sequence = NULL;
//        timeval now;
//        now.tv_sec = 1352304810;
//        now.tv_usec = 0;
//        OutmetaTransport2 wrm_trans("./", "xxx", now, 800, 600, &wrm_sequence);
//        wrm_trans.send("AAAAX", 5);
//        wrm_trans.send("BBBBX", 5);
//        wrm_trans.next();
//        wrm_trans.send("CCCCX", 5);
//    } // brackets necessary to force closing sequence

//    char meta_path[1024];
//    char file1[1024];
//    char file2[1024];
//    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
//    BOOST_CHECK_EQUAL(103, filesize(meta_path));
//    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));

//    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
//    BOOST_CHECK_EQUAL(10, filesize(file1));
//    BOOST_CHECK_EQUAL(0, ::unlink(file1));

//    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
//    BOOST_CHECK_EQUAL(5, filesize(file2));
//    BOOST_CHECK_EQUAL(0, ::unlink(file2));   
//}
