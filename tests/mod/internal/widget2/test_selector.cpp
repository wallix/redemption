/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#define BOOST_TEST_MODULE TestWidgetSelector
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/selector.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP", "2013-04-20 19:56:50");

    selector.selector_lines.set_current_index(0);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x48\x84\x19\xc2\x89\x9a\x81\x1e\xd4\x23"
                   "\x9d\x18\x21\x47\xdc\x93\x06\x2c\x47\xec")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_current_index(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x96\x5d\x5f\xdc\x38\x5a\x13\xfc\x34\xeb"
                   "\x43\x03\x2d\x5f\x1b\x3d\xf9\x7f\xba\x51")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelector2)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3e\xda\x65\x16\x1e\xd4\xca\xa8\x24\x79"
                   "\x6a\x1d\x54\x02\x7e\xeb\x70\x45\xf4\x32")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorClip)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb3\x7a\x94\x02\x30\x30\x69\xc9\x7f\x48"
                   "\xec\x17\x5c\xe1\x0d\x46\x67\xa2\x54\x2b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      5 + selector.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xcf\x78\x34\x58\x72\x1c\xcb\xc0\xe2\x50"
        "\xc9\xfd\x25\x83\xe9\xe4\x6f\x16\x5b\xb5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorEventSelect)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP", "2013-04-20 19:56:50");

    selector.selector_lines.set_current_index(0);

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.dx() + 20,
                                            selector.selector_lines.dy() + 40,
                                            NULL);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x19\x59\x5f\xdc\xec\x68\x9e\x5a\x51\xe1"
                   "\x7b\x11\x58\x2b\x5c\x94\xce\xd2\x3f\xae")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x96\x5d\x5f\xdc\x38\x5a\x13\xfc\x34\xeb"
                   "\x43\x03\x2d\x5f\x1b\x3d\xf9\x7f\xba\x51")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xe2\x28\xf3\xbd\xcd\x75\x1a\xd1\xb4\x53"
                   "\x27\x87\x21\x38\xb2\x2a\x88\xb7\x71\x0d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x48\x84\x19\xc2\x89\x9a\x81\x1e\xd4\x23"
                   "\x9d\x18\x21\x47\xdc\x93\x06\x2c\x47\xec")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x96\x5d\x5f\xdc\x38\x5a\x13\xfc\x34\xeb"
                   "\x43\x03\x2d\x5f\x1b\x3d\xf9\x7f\xba\x51")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x48\x84\x19\xc2\x89\x9a\x81\x1e\xd4\x23"
                   "\x9d\x18\x21\x47\xdc\x93\x06\x2c\x47\xec")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFilter)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");

    int curx = 0;
    int cury = 0;

    selector.selector_lines.set_current_index(0);

    curx = selector.filter_device.centerx();
    cury = selector.filter_device.centery();
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             curx, cury,
                             NULL);
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             curx, cury,
                             NULL);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x1b\x30\xcb\x49\xe8\x96\xd5\x9b\x2a\xb4"
                   "\xcf\x36\x21\xd1\xfa\xce\x3e\xb9\x85\x5e")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x5c\x22\x41\x36\x86\xde\x3b\x73\x81\xf2"
                   "\xfe\x68\x03\x2f\x17\x44\x75\x62\x87\x98")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xad\x56\x54\xd3\x59\x74\xe5\xe5\xdc\xdb"
                   "\x50\xc0\x27\xe5\x90\xad\x65\x05\x99\x98")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xa5\x77\xbf\xd9\x72\x8e\x17\x91\x04\xb0"
                   "\x92\x25\x6a\x0c\xef\xcd\x1c\xe4\xa6\x1d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x73\xcc\x79\x9d\xab\x47\xee\xbd\x6b\x0d"
                   "\xf4\x39\x25\xd2\x77\x7f\xf7\x9a\x6c\x73")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xe2\x1e\xf5\xcf\xa8\x71\x83\xf5\xa6\x2f"
                   "\xb4\xdf\x72\x01\x57\xd0\xde\xa2\x06\xc4")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x80\x3d\xe0\xa6\xb9\x65\x41\x4b\xb9\xce"
                   "\x55\xad\xce\x13\x9e\x99\xa1\x92\x7f\xc6")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-8.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xda\xc2\x67\x80\xa0\x7b\x41\x9b\x0b\x5c"
                   "\xc4\x9f\x66\x9f\x57\x6b\x03\x42\x7b\x25")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-9.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x89\xcf\xc9\xf8\x01\x0d\x37\xf2\xa8\x9e"
                   "\x2e\xe0\x77\x11\x2b\xcb\x3e\x37\xf3\x16")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
