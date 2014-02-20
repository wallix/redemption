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
#define BOOST_TEST_MODULE TestWidgetSelectorFlat
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/flat_selector.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;
    Inifile ini;

    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
                   "\x97\x81\x51\x5c\x82\x59\xc1\x12\x08\x1a"
                   "\xf7\xcd\x50\xe5\x84\xa3\xd5\x61\x3d\xd1"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_current_index(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x68\xaf\xe3\x65\xa1\x8d\xdf\xf5\x40\xb1"
                   "\x4f\x42\x98\x3e\xde\xda\x74\x5b\x8e\xba"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatResize)
{
    TestDraw drawable(640, 480);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 640, 480);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier,
                                "1", "1", 0, 0, 0, ini);

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

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x78\x3c\x32\x0a\x2b\x3a\x61\xe2\xda\x7c"
                   "\x4f\x01\x95\x87\xec\x96\x98\x17\x0f\x3b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_current_index(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x52\x57\x5d\x4c\xd8\xe5\x16\xc5\x71\x98"
                   "\xc8\x89\xd4\xad\x94\x9f\xc9\x97\x1a\xe3"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat2)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xe0\x56\xfe\x91\x97\xc9\xc9\x09\xce\x5e"
                   "\x7d\x87\x26\x63\xf6\xe9\x81\x0d\xd0\x58"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x0f\x7d\x77\x78\x45\x3e\x80\xf5\xcd\x77"
                   "\x83\x9b\xed\xc6\x7f\xb3\xe2\xf0\x28\xa4"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      5 + selector.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x9d\xe3\xdc\x8b\x3e\xe0\x66\x51\x3f\x38"
        "\x8f\x1b\xe4\x7f\x5d\xe8\x60\xb0\x8f\xe9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatEventSelect)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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

    selector.set_widget_focus(&selector.selector_lines);
    selector.selector_lines.set_current_index(0);

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.dx() + 20,
                                            selector.selector_lines.dy() + 40,
                                            NULL);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\x07\xa6\xe2\x7c\x16\xf7\x41\x44\xee\x52"
                   "\xda\x6a\x28\x41\xbe\x35\xf8\xc7\x93\x59"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-2.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\x62\x48\x5a\x12\x3e\xcb\x36\x76\x64\x23"
                   "\xc2\xc0\x46\xe7\x4c\x3d\x0a\xc9\x76\x1c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xeb\x82\xe1\x12\x07\x21\x4d\x8c\x58\x29"
                   "\xa3\xac\x06\x1f\x0f\xf4\x73\xc4\x6d\x8b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xd3\x46\x9f\x74\xba\x03\x75\x4c\x38\x95"
                   "\x88\x95\x9e\x9c\x4d\x4b\x38\xcc\xa4\x96"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\x62\x48\x5a\x12\x3e\xcb\x36\x76\x64\x23"
                   "\xc2\xc0\x46\xe7\x4c\x3d\x0a\xc9\x76\x1c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }




    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xd3\x46\x9f\x74\xba\x03\x75\x4c\x38\x95"
                   "\x88\x95\x9e\x9c\x4d\x4b\x38\xcc\xa4\x96"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    int x = selector.selector_lines.rect.x + 5;
    int y = selector.selector_lines.rect.y + 3;
    selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);
    x += selector.selector_lines.group_w;
    selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);
    x += selector.selector_lines.target_w;
    selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);
    x += selector.selector_lines.protocol_w;
    selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);


}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatFilter)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
//    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
                   "\x18\xee\xe4\x8b\x13\x37\x5a\x5c\x23\x59"
                   "\x94\x7c\x34\x54\x08\x12\x87\xcd\xdd\xd6"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-2.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xb5\x73\xc3\x32\x5d\x4f\x79\x63\xfc\x20"
                   "\xe9\xb0\x34\x13\xf0\x52\xd7\x0a\xa5\x79"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x1a\x62\x2e\x39\xc4\x07\xbb\x06\x8d\xc8"
                   "\x54\x0b\x07\x45\x9a\x58\xd1\xdf\x49\xce"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x72\x69\x3b\x9c\x00\xd5\xd3\x93\x71\x48"
                   "\x7c\x1c\xdc\x8d\xd7\x55\xce\x03\x18\x84"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xb5\x5c\x66\xc7\x99\x66\x8a\x11\xa9\x59"
                   "\x1d\x3a\xf6\xc7\xb7\xa3\x89\x91\xde\x16"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xfc\x84\xda\xbf\x51\xcb\x89\xd6\x54\x73"
                   "\xdc\x8a\x14\xc9\x82\x00\xa5\xa6\x0e\x0d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x87\xc1\x85\xfd\x1a\x80\xa1\x06\x8b\xeb"
                   "\x45\x5b\x9c\xcd\x57\x06\x03\xe8\x27\x25"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-8.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x0f\x14\xf1\xc5\x86\x4b\xaf\x82\x82\x8c"
                   "\x61\x52\x60\x62\x40\x1f\xf5\x9b\x40\x27"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

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
                   "\xee\xab\x3a\x75\x0c\xf8\xc4\x8b\x82\xf0"
                   "\x92\x7f\x98\x02\x0a\x88\x91\x31\xa7\x81"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    // ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33azertyuiopŝdfghjklmx",
                        "VNC", "2013-04-20 19:56:50");
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

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\x36\xf9\x92\x34\x51\xa3\xe6\x18\xb8\xbb"
                   "\x7d\xd9\xa2\x92\x14\xe8\x40\xb3\x04\x2f"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsqlkmlsdkmaklzeerd", "eagle@10.10.14.33azert",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x08\x3d\xdd\xe9\x60\x8b\xf9\xc9\x74\xcc"
                   "\xb3\x4c\xb4\x1d\xa4\x85\x3f\xdd\xe1\xad"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelknelkaznelkssdlqk",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x6d\x23\xab\x06\x73\x7c\xbe\x69\x6b\x33"
                   "\xd4\x94\x7a\x4c\xc7\x38\x68\x01\xcd\xcf"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelkn",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fkljazelkjalkzjelakzejish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x82\x5c\xe0\x0e\x0e\xcb\x63\x48\x64\xb7"
                   "\x6f\x05\xa8\x92\x40\xe6\x51\x7f\x20\x32"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns2)
{
    TestDraw drawable(640, 480);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 640, 480);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    // ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33azertyuiopŝdfghjklmx",
                        "VNC", "2013-04-20 19:56:50");
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

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\xc3\x87\x52\xfc\xbc\x47\x3d\x20\x34\x33"
                   "\x7c\x54\xcd\x65\xf1\x3f\xab\xc3\x12\x64"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsqlkmlsdkmaklzeerd", "eagle@10.10.14.33azert",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x88\xff\x6f\x35\xb6\xca\xf3\x61\x5a\x8c"
                   "\xcf\x4b\xa0\x27\x98\x4e\xe4\x0a\x38\x2d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelknelkaznelkssdlqk",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x04\xf1\xeb\x69\xe9\xbc\xda\x26\xd4\xe0"
                   "\x96\xd1\xa7\x59\x52\x0a\x2e\x7d\x44\x01"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelkn",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fkljazelkjalkzjelakzejish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x8d\x52\x86\xa1\xbe\x32\x62\xa9\x14\xc6"
                   "\x06\xea\xa6\x27\x18\x10\x8b\xa8\x4b\xe7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns3)
{
    TestDraw drawable(1280, 1024);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 1280, 1024);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    // ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33azertyuiopŝdfghjklmx",
                        "VNC", "2013-04-20 19:56:50");
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

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\x7d\x56\xc8\x95\xa9\xb2\x32\x5c\x7d\x61"
                   "\x7f\x06\x0b\x3c\x6b\x1f\x37\x71\xad\x28"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsqlkmlsdkmaklzeerd", "eagle@10.10.14.33azert",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x4c\x80\xba\x39\xfc\xc3\xbe\x2a\x1f\xc4"
                   "\x25\xdf\xcf\xc7\x13\x57\x8d\xb0\xfa\xad"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelknelkaznelkssdlqk",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x8a\xb8\x20\xdf\xad\x13\x4e\x70\x6f\x7a"
                   "\xe3\xc6\x21\x8d\xa6\x3b\xa5\x16\x84\xe7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.clear();

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("birdsandoandjdanxqpa&éjmql&&,é", "raven@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelkn",
                        "VNC", "2013-04-20 19:56:50");
    selector.add_device("fkljazelkjalkzjelakzejish", "shark@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("bird", "eagle@10.10.14.33pezok&mlé,&m,qdns,nq&n&élkn&lnlnezl&ne&lkn&é",
                        "VNC", "2013-04-20 19:56:50");
    selector.selector_lines.set_current_index(0);

    selector.fit_columns();

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x1a\x2f\xf3\x80\x9f\x43\x8d\x11\xd5\x1a"
                   "\x56\xf6\xd4\xfc\xef\x4a\x3a\xc0\x80\x3c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}
