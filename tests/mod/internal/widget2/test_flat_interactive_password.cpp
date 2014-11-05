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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFlatInteractivePassword
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#include "internal/widget2/flat_interactive_password.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatInteractivePassword)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    FlatInteractivePassword interactive(drawable, 800, 600, parent, notifier,
                                        0, 0, colors, "Authentication Required",
                                        "login", "user1", "password");
    // ask to widget to redraw at it's current position
    interactive.rdp_input_invalidate(interactive.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "interactive_pass.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
                   "\xcd\x0a\xf2\xa7\xeb\xcb\x10\xc7\x45\x42"
                   "\xb8\x9c\xa7\x41\xe7\xda\x3f\xec\x52\xa8"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


}
