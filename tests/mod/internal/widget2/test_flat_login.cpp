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
#include "internal/widget2/notify_api.hpp"
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
    // TestDraw drawable(1280, 1024);

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    // WidgetScreen parent(drawable, 1280, 1024);
    NotifyApi * notifier = NULL;
    int id = 0;

    Inifile ini;

    FlatLogin flat_login(drawable, parent.cx(), parent.cy(), parent, notifier, "test1",
                         false, id, "rec", "rec", "Login", "Password", ini);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\xf7\xa0\x89\xc7\xe0\x6c\x08\xf0\xe6"
                   "\x5b\x77\xb2\xdf\x14\xf1\x01\xf0\x2d\x06"
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

    Inifile ini;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test2",
                         false, 0, 0, 0, "Login", "Password", ini);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9b\xb2\xe7\x6f\x82\x60\x36\xbd\x08\x42"
                   "\xc9\x24\x6a\x23\x5e\xbd\xa6\xdd\xc7\x1e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin3)
{
    ClientInfo info(1, true, true);
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;


    TestDraw drawable(800, 600);
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

    // FlatLogin is a flat_login widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    Inifile ini;
    FlatLogin flat_login(drawable, 800, 600, parent, &notifier, "test3",
                         false, 0, 0, 0, "Login", "Password", ini);

    flat_login.set_widget_focus(&flat_login.password_edit);

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    Keymap2 keymap;
    keymap.init_layout(info.keylayout);
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    BOOST_CHECK(notifier.sender == &flat_login);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf5\xce\xa3\xfe\xad\xf8\x4c\x85\x17\x2b"
                   "\xac\xe0\x24\x0a\xd1\x1c\x9c\x1c\xd8\x0e"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    notifier.sender = 0;
    notifier.event = 0;
    keymap.push_kevent(Keymap2::KEVENT_ESC); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    BOOST_CHECK(notifier.sender == &flat_login);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginHelp)
{
    TestDraw drawable(800, 600);

    // FlatLogin is a flat_login widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    Inifile ini;
    ini.translation.help_message.set_from_cstr("");


    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test4",
                         false, 0, 0, 0, "Login", "Password", ini);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9c\x7a\xff\xf1\x50\x76\xb3\x56\x3f\x38"
                   "\x6e\xbb\x3a\xab\x07\x9c\xb6\x74\x4c\xd5"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.centerx(), flat_login.helpicon.centery(), NULL);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xf1\xe9\x9e\xa5\x45\xac\x18\x41\xce\x1c"
                   "\x9e\x88\x08\x16\xb7\x88\x4a\xd9\x38\xdd"
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

    Inifile ini;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6",
                         false, 0, 0, 0, "Login", "Password", ini);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xa3\xcc\x9e\xa5\xea\xdd\x20\xd0\xb7\xf4"
                   "\x00\x12\x09\x89\xd7\x7a\xf1\x42\xb6\xc5"
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

    Inifile ini;
    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6",
                         false, 0, 0, 0, "Login", "Password", ini);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      5 + flat_login.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x31\x82\xdc\x89\xfd\xda\x77\xc1\xf9\xa1"
                   "\x44\x23\xdb\xc5\x09\xae\xb9\xb7\x2b\x35"
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

    Inifile ini;

    FlatLogin flat_login(drawable, 800, 600, parent, &notifier, "test6",
                         false, 0, 0, 0, "Login", "Password", ini);
}


