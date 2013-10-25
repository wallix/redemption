/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConfDescriptor
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "confdescriptor.hpp"

BOOST_AUTO_TEST_CASE(TestConfigFromFile)
{
    GeneralCaps       generalcaps;
    GeneralCapsLoader generalcaps_loader;

    ConfigurationLoader cfg_loader(generalcaps_loader, FIXTURES_PATH "/capsset.ini");

    BOOST_CHECK_EQUAL(generalcaps.extraFlags, 1025);
}