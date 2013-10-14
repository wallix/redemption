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

//    drawable.save_to_png(OUTPUT_FILE_PATH "selector1.png");

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
                   "\x8a\x25\x30\x50\x6f\x1c\xa5\x52\x5a\x25"
                   "\x2e\xc8\x9e\xf7\xca\x56\x1b\x53\x7f\x78"
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
                   "\xb8\x3b\x86\x28\xc8\x7b\x0a\xd4\x4b\x45"
                   "\x34\xbe\xe7\x2a\xd2\xf0\x88\xdb\x62\x68"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x59\x34\x0a\x59\x47\xc9\x3f\xf8\xec\xc7"
                   "\xd9\xcb\xde\xec\x06\xe2\xe9\xcc\x9d\x90"
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
                   "\xe6\x34\x5d\xb9\x6a\xe5\x10\x77\xe3\x08"
                   "\x2a\xc6\xa4\x15\xd3\x58\x82\xd4\x67\x16"
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
                   "\xfc\x0e\x56\x51\x65\xc3\x02\xc3\x88\xe1"
                   "\x63\x36\x4d\x23\x9e\xdb\x61\xab\x89\x6d"
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
                   "\x6d\x73\x5a\x99\xe2\x44\xe5\x0a\x09\x04"
                   "\x09\xa0\x82\x2d\xf9\xb8\x9c\x53\xa8\xea"
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
                   "\xea\xac\x4e\x3f\xfe\x36\x95\xf6\x69\x1c"
                   "\x2c\x04\xb0\x24\x8f\xa1\x34\x3e\x3f\xa3"
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
                   "\x23\xcf\x81\x59\xfa\x63\xdb\x85\xea\x02"
                   "\x55\x24\x1e\xa5\xbc\x2f\x02\x6c\x82\xe4"
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
                   "\xa4\x95\x0f\xe8\x17\x4b\xd7\x05\xe5\xa8"
                   "\xff\x37\x11\x5c\xee\x22\x68\x3e\x3a\x16"
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
                   "\x0c\x4a\x35\x4e\x14\x47\xaa\xc4\x45\x23"
                   "\x66\x66\xa1\x89\xd5\x4a\xa8\x0f\x19\x18"
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
                   "\x7a\x71\x3e\x26\x23\xee\x1d\x7e\x10\x4b"
                   "\xc2\x7c\xbe\xe9\x37\xaa\x5e\x33\x37\xd5"
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
                   "\xaf\x94\x59\xd1\x45\x0e\xd9\x15\xe1\x82"
                   "\xab\xb9\x69\xa3\xd2\xeb\xa5\x48\x16\x1e"
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
                   "\x75\xf9\x1b\xd3\xfb\xa7\x8f\xe1\xf1\xcb"
                   "\x51\xde\x7a\xea\x0f\x25\x96\x82\x10\xe7"
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
                   "\x4e\xb1\x2a\x7f\x44\x9d\xf3\xae\xb2\x69"
                   "\xe1\xff\x20\x63\x82\x12\x35\xea\xea\x02"
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
                   "\xb7\x3d\x64\xff\xcd\x52\x27\xe0\xf3\x80"
                   "\xf2\x54\x3d\x5c\x12\xdb\x00\xbc\xd6\xea"
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
                   "\xdd\x2c\x02\xe9\x31\xa0\xb8\x52\xc2\x0f"
                   "\xb9\xde\x64\xfe\xbc\x0d\x13\xdb\xdd\x9d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}
