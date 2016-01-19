/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TESTS_TRANSPORT_TEST_REQUEST_FULL_CLEANING_CPP
#define REDEMPTION_TESTS_TRANSPORT_TEST_REQUEST_FULL_CLEANING_CPP


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCountTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "fileutils.hpp"

#include "out_meta_sequence_transport.hpp"

BOOST_AUTO_TEST_CASE(TestRequestFullCleaning)
{
    unlink("./xxx-000000.wrm");
    unlink("./xxx-000001.wrm");
    unlink("./xxx.mwrm");

    timeval now;
    now.tv_sec = 1352304810;
    now.tv_usec = 0;
    const int groupid = 0;
    OutMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), "./", "./hash-", "xxx", now, 800, 600, groupid, nullptr, 0,
                                       FilenameGenerator::PATH_FILE_COUNT_EXTENSION);
    wrm_trans.send("AAAAX", 5);
    wrm_trans.send("BBBBX", 5);
    wrm_trans.next();
    wrm_trans.send("CCCCX", 5);

    const FilenameGenerator * sqgen = wrm_trans.seqgen();

    BOOST_CHECK(-1 != filesize(sqgen->get(0)));
    BOOST_CHECK(-1 != filesize(sqgen->get(1)));
    BOOST_CHECK(-1 != filesize("./xxx.mwrm"));

    wrm_trans.request_full_cleaning();

    BOOST_CHECK_EQUAL(-1, filesize(sqgen->get(0)));
    BOOST_CHECK_EQUAL(-1, filesize(sqgen->get(1)));
    BOOST_CHECK_EQUAL(-1, filesize("./xxx.mwrm"));
}

#endif
