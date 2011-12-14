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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to widget.cpp file
   Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBitmap
#include <boost/test/auto_unit_test.hpp>

#include "../mod/internal/internal_mod.hpp"
#include "../mod/internal/widget.hpp"
#include "colors.hpp"
#include <sstream>
#include <iostream>
#include <string>
#include <string.h>

#include "front.hpp"

BOOST_AUTO_TEST_CASE(TestCreateWidget)
{
//    cout << "Test Creating Screen object";
    TODO(" passing in a null front is not a good idea  define a test front")
    Widget *screen = new widget_screen((GraphicalContext*)0, 20, 10, 24);
    BOOST_CHECK_EQUAL(20, screen->rect.cx);
    BOOST_CHECK_EQUAL(10, screen->rect.cy);
    BOOST_CHECK_EQUAL((int)WND_TYPE_SCREEN, screen->type);
    delete screen;
}

BOOST_AUTO_TEST_CASE(TestCreateWidgetScreen)
{
//    cout << "Test Creating Screen object";
    TODO(" passing in a null front is not a good idea  define a test front")
    widget_screen *screen = new widget_screen((GraphicalContext*)0, 20, 10, 24);
    BOOST_CHECK_EQUAL(20, screen->rect.cx);
    BOOST_CHECK_EQUAL(10, screen->rect.cy);
    BOOST_CHECK_EQUAL(24, screen->bpp);
    BOOST_CHECK_EQUAL((int)WND_TYPE_SCREEN, screen->type);
    delete screen;
}
