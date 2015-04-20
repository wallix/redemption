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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for Lightweight UTF library

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestProgramOption
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#include "program_options.hpp"

namespace po = program_options;

BOOST_AUTO_TEST_CASE(TestUTF8Len_2)
{
    int i = 0;
    po::options_description desc({
        {'v', "verbose", &i, "verbosity"},
        {'g', "blah blah"},
        {'u', "blah blah"}
    });

    {
        char const * av[] {
            "", "--verbose", "3", "-u"
        };
        auto map = po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);

        BOOST_CHECK_EQUAL(map.count("verbose"), 1);
        BOOST_CHECK_EQUAL(map.count('v'), 1);
        BOOST_CHECK_EQUAL(map.count('g'), 0);
        BOOST_CHECK_EQUAL(map.count('u'), 1);
        BOOST_CHECK_EQUAL(i, 3);
    }

    {
        char const * av[] {
            "", "-gv5"
        };
        auto map = po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);

        BOOST_CHECK_EQUAL(map.count("verbose"), 1);
        BOOST_CHECK_EQUAL(map.count('v'), 1);
        BOOST_CHECK_EQUAL(map.count('g'), 1);
        BOOST_CHECK_EQUAL(map.count('u'), 0);
        BOOST_CHECK_EQUAL(i, 5);
    }

    {
        char const * av[] {
            "", "-ug"
        };
        auto map = po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);

        BOOST_CHECK_EQUAL(map.count("verbose"), 0);
        BOOST_CHECK_EQUAL(map.count('v'), 0);
        BOOST_CHECK_EQUAL(map.count('g'), 1);
        BOOST_CHECK_EQUAL(map.count('u'), 1);
    }

    {
        char const * av[] {
            "", "-v"
        };

        try {
            po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);
        }
        catch (po::invalid_command_line_syntax &)
        {}
        catch (...)
        { BOOST_MESSAGE("invalid exception"); }
    }

    {
        char const * av[] {
            "", "-v22222222222222222222222222222222222222222222222222222222222222222222"
        };

        try {
            po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);
        }
        catch (po::invalid_option_value &)
        {}
        catch (...)
        { BOOST_MESSAGE("invalid exception"); }
    }

    {
        char const * av[] {
            "", "-"
        };

        try {
            po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);
        }
        catch (po::unknow_option&)
        {}
        catch (...)
        { BOOST_MESSAGE("invalid exception"); }
    }

    {
        char const * av[] {
            "", "-"
        };

        try {
            po::parse_command_line(sizeof(av)/sizeof(av[0]), const_cast<char**>(av), desc);
        }
        catch (po::invalid_command_line_syntax&)
        {}
        catch (...)
        { BOOST_MESSAGE("invalid exception"); }
    }
}

