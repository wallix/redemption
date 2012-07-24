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
#define BOOST_TEST_MODULE TestFileToPng
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT

#include "meta_file.hpp"

#include <iostream>
#include <sstream>

BOOST_AUTO_TEST_CASE(TestMetaFile)
{
    DataMetaFile data;

    data.width = 800;
    data.height = 600;
    data.files.push_back(DataFile(
        "khifeza.wrm",
        "",
        13857638,
        46687687
    ));
    data.files.push_back(DataFile(
        "ldehgmzg.wrm",
        "cceds.png",
        153857638,
        46767
    ));
    data.files.push_back(DataFile(
        "ezpiuze.wrm",
        "jz.png",
        138576385,
        2
    ));

    std::stringstream ss;
    ss << data;

    DataMetaFile data2;
    ss >> data2;

    BOOST_REQUIRE_EQUAL(data.files.size(), data2.files.size());
    BOOST_REQUIRE(data.files == data2.files);

    ss.clear();
    ss.str("800 600\n"
    "khifeza.wrm, \n"
    "ldehgmzg.wrm,cceds.png 46767\n"
    "ezpiuze.wrm,jz.png\n");
    ss >> data;
    data2.files[0].start_sec = 0;
    data2.files[0].start_usec = 0;
    data2.files[1].start_sec = 46767;
    data2.files[1].start_usec = 0;
    data2.files[2].start_sec = 0;
    data2.files[2].start_usec = 0;
    BOOST_REQUIRE_EQUAL(data.files.size(), data2.files.size());
    BOOST_REQUIRE(data.files == data2.files);
}
