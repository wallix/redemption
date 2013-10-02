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
#define BOOST_TEST_MODULE TestFlatLogin
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#include "internal/widget2/flat_login.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int id = 0;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test1", false, id, "rec", "rec");

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x42\xd5\x14\x43\x4d\xbd\xc1\x21\xc7\x2b"
        "\xee\x9b\x25\xb9\x94\x21\x1e\x9d\x62\x53"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test2");

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x62\x4e\x13\x69\x10\x15\x4c\xdd\x7f\x2a"
        "\x86\x2d\x08\x42\x00\xc3\xa0\xf8\x81\xd3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin3)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test3");

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa6\x4c\xe8\xbb\x18\x5c\x91\xb3\x06\x20"
        "\x26\x67\x40\xcf\x11\xdc\xad\xca\x4d\x2e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin4)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test4");

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb0\x72\x79\xa3\xa5\xba\x02\xd9\x10\xa7"
        "\x6a\x55\x7e\xca\x5d\xb0\x1f\xc6\xab\xcb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin5)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test5");

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x9a\x05\xed\x85\x44\xd9\xb6\xac\x35\xba"
        "\xca\x4c\x38\xa4\x3c\x35\x09\xe3\xb2\x1e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin6)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6");

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x61\x63\x76\x6c\xf2\xab\x72\x06\x8d\x49"
        "\x6f\x4c\xa3\x9d\x95\xd6\x76\x3e\x80\x27"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x94\x4c\x47\xf5\x74\x1f\x42\x1b\x4a\xf2"
        "\x0e\xf3\x5e\x0a\x01\x92\x02\x39\x10\xbd"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      5 + flat_login.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x70\x47\x33\xfc\x5e\x87\x63\x8b\x0c\x2f"
        "\x0b\x65\x69\xd8\x92\x9b\x58\x18\xee\x09"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    WidgetScreen parent(drawable, 800, 600);
    struct Notify : NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    int16_t x = 10;
    int16_t y = 10;

    FlatLogin flat_login(drawable, x, y, parent, &notifier, "test6");
}


