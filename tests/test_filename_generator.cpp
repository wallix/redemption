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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFilenameGenerator
#include <boost/test/auto_unit_test.hpp>

#include "filename_generator.hpp"

BOOST_AUTO_TEST_CASE(TestFilenameGenerator)
{
    {
        FilenameGenerator generator("plop");

        BOOST_CHECK_EQUAL(generator(), "plop");
        BOOST_CHECK_EQUAL(generator(), "plop");
        BOOST_CHECK_EQUAL(generator(), "plop");
    }

    {
        FilenameIncrementalGenerator generator("plop");

        BOOST_CHECK_EQUAL(generator(), "plop0");
        BOOST_CHECK_EQUAL(generator(), "plop1");
        BOOST_CHECK_EQUAL(generator(), "plop2");
    }

    {
        FilenameIncrementalGenerator generator("plop.png");

        BOOST_CHECK_EQUAL(generator(), "plop-0.png");
        BOOST_CHECK_EQUAL(generator(), "plop-1.png");
        BOOST_CHECK_EQUAL(generator(), "plop-2.png");
    }

    {
        FilenameIncrementalGenerator generator("plop-", ".png", 5);

        BOOST_CHECK_EQUAL(generator(), "plop-5.png");
        BOOST_CHECK_EQUAL(generator(), "plop-6.png");
        BOOST_CHECK_EQUAL(generator(), "plop-7.png");
    }

    {
        std::string filename("plop-");
        filename += boost::lexical_cast<std::string>(getpid());
        BOOST_CHECK_EQUAL(filename_to_pid_filename("plop.png"), filename+".png");
        BOOST_CHECK_EQUAL(filename_to_pid_filename("plop"), filename);
    }

    {
        FilenameGenerator generator = make_pid_filename_generator("plop.png");

        std::string filename("plop-");
        filename += boost::lexical_cast<std::string>(getpid());
        filename += ".png";
        BOOST_CHECK_EQUAL(generator(), filename);
        BOOST_CHECK_EQUAL(generator(), filename);
    }

    {
        FilenameGenerator generator = make_pid_filename_generator("plop-", ".png");

        std::string filename("plop-");
        filename += boost::lexical_cast<std::string>(getpid());
        filename += ".png";
        BOOST_CHECK_EQUAL(generator(), filename);
        BOOST_CHECK_EQUAL(generator(), filename);
    }

    {
        FilenameIncrementalGenerator generator = make_pid_filename_incremental_generator("plop.png", 5);

        std::string basename("plop-");
        basename += boost::lexical_cast<std::string>(getpid());
        BOOST_CHECK_EQUAL(generator(), basename + "-5.png");
        BOOST_CHECK_EQUAL(generator(), basename + "-6.png");
        BOOST_CHECK_EQUAL(generator(), basename + "-7.png");
    }
}