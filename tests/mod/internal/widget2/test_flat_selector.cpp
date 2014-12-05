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
//#define LOGPRINT

#include "internal/widget2/flat_selector.hpp"
#include "internal/widget2/screen.hpp"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
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
    if (!check_sig(drawable.gd.impl(), message,
        "\x1e\xa4\x56\xaa\x3a\xf6\x75\x64\x40\x4c\x90\x37\xda\xa2\x77\xaa\x62\x61\xc9\x8c"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_current_index(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x08\x01\x32\x64\x83\xe5\x98\x83\x87\x27\xf9\x00\x3b\x5c\x2c\x20\xc7\xc0\xea\x14"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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
    if (!check_sig(drawable.gd.impl(), message,
        "\x9a\x8e\xd7\xbd\x97\x51\x79\x8d\x3b\x0d\x4e\x22\x85\xfd\xff\x45\x6b\xc2\x0e\x80"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_current_index(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x6c\x46\xd6\xf7\xc0\x80\xf1\xcf\xbb\x50\x6b\xc6\x59\xb0\x9e\xf1\xda\x3a\x65\xdc"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x89\xd2\xe5\xb7\x76\x62\xc8\x47\x92\xeb\x9e\x6f\x3a\xcb\x02\x58\x50\x7b\x79\xb5"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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
    if (!check_sig(drawable.gd.impl(), message,
        "\xb0\x60\x4c\x7d\xc3\xe1\x51\xe1\x63\x6a\x5e\x61\x59\x8c\xab\xec\x2f\x3c\x24\x87"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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
    if (!check_sig(drawable.gd.impl(), message,
        "\x39\xd3\x5b\x20\x81\x7b\xa9\x67\x12\x79\x1d\xaf\x20\xff\xcf\xd6\xf7\xc8\xdd\x5a"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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

    selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
    selector.selector_lines.set_current_index(0);

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.dx() + 20,
                                            selector.selector_lines.dy() + 40,
                                            NULL);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x99\x6c\xa7\x11\x99\x5b\xbc\x7e\xd0\x10\x29\x10\x6c\x86\xac\x34\xef\xfd\xf7\xaf"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-2.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x6f\xc3\xcf\xf8\x8b\x03\xf2\xcc\x49\x68\x8a\xa7\x9a\x5c\x95\xdf\x9a\x1c\xae\xa1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x58\x9b\x86\xc6\xb1\xc9\x47\x25\x6d\xca\x86\x5b\xec\x43\xed\x14\x6b\xa4\xb3\xdc"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x13\x79\x2b\x56\xe9\x22\x23\x0f\x8c\xfe\x2a\x74\xad\x8c\xd2\xcc\x69\x77\x91\x33"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x6f\xc3\xcf\xf8\x8b\x03\xf2\xcc\x49\x68\x8a\xa7\x9a\x5c\x95\xdf\x9a\x1c\xae\xa1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }




    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x13\x79\x2b\x56\xe9\x22\x23\x0f\x8c\xfe\x2a\x74\xad\x8c\xd2\xcc\x69\x77\x91\x33"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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

    if (!check_sig(drawable.gd.impl(), message,
        "\x28\xe4\xfd\x68\xfa\xf6\xae\x2c\xa7\x06\xa6\xab\xac\x90\x2d\xfb\xd8\xf4\x71\x3d"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-2.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xce\x86\x57\x03\x66\x37\x98\x50\xdf\x72\xb2\xd0\x2c\x8d\xf7\xee\x48\xbb\xe1\xf7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xc8\xbc\xff\x38\x44\x2b\x28\xda\xdc\x7f\xe2\x1d\x3f\x3a\x04\x86\x8d\xf4\xcb\x0a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x1e\x0b\x1e\x6a\x30\xe3\xfd\x32\x38\xa2\x94\x56\x4b\xf2\x56\x6a\xde\x42\x14\xc7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xe5\xac\x83\xa8\xa5\x44\x10\x61\x1f\x67\xe2\xa5\xa2\x84\x2e\xa1\x16\x2f\xce\x80"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x10\xab\xfe\x73\xbf\xbf\x7d\xe9\x8b\xa2\x1e\x7e\x5f\x77\xd5\x2b\xf1\xf5\xc8\x4a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x2d\xad\x69\xaa\xe0\x63\x57\xc0\x87\x77\x36\x2c\xfe\x8d\x9c\x44\xcb\xad\x6a\xf3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-8.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x40\xce\xd1\x64\x2d\x6a\xbf\xb1\xa2\x0c\x14\xad\x86\x60\xc6\xf1\x46\x10\xb6\x7d"
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

    if (!check_sig(drawable.gd.impl(), message,
        "\x33\xb6\xf1\xfa\x4e\xab\xe8\x60\x4e\xe5\x20\x5f\x72\x08\x87\x01\x57\xd1\xa6\xa7"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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

    if (!check_sig(drawable.gd.impl(), message,
        "\xc3\x70\x7c\x38\x59\x7a\xdd\x84\x17\x14\x0d\x8e\xdc\x31\xf8\xaf\xd4\x9b\x5d\x3d"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\xa2\xfb\xd4\xed\x7e\x49\xa7\xf6\x05\x8a\xb4\xb7\xb3\x70\xf2\xf4\x6e\x0d\x60\xe2"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\xdc\x02\xac\x7c\x8f\x93\xed\x8d\x10\x02\xd3\x82\x84\x96\xbd\x04\xd1\x6a\x1b\x91"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\xa9\x33\x1f\x05\x22\x81\xae\xcc\xff\x33\xbc\x80\xd2\x25\xbc\x43\x45\x96\x17\xa3"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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

    if (!check_sig(drawable.gd.impl(), message,
        "\xa2\x66\xe9\x32\x18\xa9\xae\xd1\x0b\x00\x59\xcc\x10\x3a\x31\xd8\x90\x09\xfa\x40"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\x39\xcd\x5e\x3d\x86\xc1\x16\x04\xd7\xdd\xb9\xd3\x3c\x2e\x51\x68\x6f\x93\xaa\xba"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\x65\xd4\x2d\x84\x1d\x0f\xcf\x52\x22\xd8\x2d\x29\x7e\xc2\x65\x5c\x60\x33\xa2\xf5"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\xd3\x3d\x5f\x73\xcc\x09\x21\x24\xae\x01\x81\x3d\xdc\xdf\xb5\xdd\x51\x02\x36\xff"
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
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

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

    if (!check_sig(drawable.gd.impl(), message,
        "\xaa\x46\x80\xf1\x0d\xa6\x64\x27\xc1\x3a\x99\x05\x31\xc4\xac\x56\xa5\xcd\x8e\x50"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\xa4\xfb\x35\x2b\x5b\xc4\x0a\x7f\x30\x4c\x24\x19\xad\x92\x7b\xdc\xc0\x73\x75\xb7"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\xa3\x4d\x46\x3b\x2c\x4f\x02\x82\xee\x6a\xba\x94\x2c\x1f\xf8\x0f\x25\xa8\x05\xc1"
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
    if (!check_sig(drawable.gd.impl(), message,
        "\x43\x81\xa7\x68\xf2\x72\x09\x8d\x58\xcb\xb5\xe1\xc1\x08\xa3\x25\x5f\xc3\x53\x7f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldVV)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
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

    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    Inifile ini;

    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_VISIBLE | COMMENT_VISIBLE);

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

    selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
    selector.selector_lines.set_current_index(0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    // selector.selector_lines.set_current_index(1);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == NULL);


    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);


    BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
    BOOST_CHECK(notifier.sender == &selector.connect);
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldVM)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
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

    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    Inifile ini;

    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_VISIBLE | COMMENT_MANDATORY);

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

    selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
    selector.selector_lines.set_current_index(0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    // selector.selector_lines.set_current_index(1);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == NULL);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == NULL);
    BOOST_CHECK(selector.current_focus == &selector.comment_edit);


    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == NULL);
    BOOST_CHECK(selector.current_focus == &selector.comment_edit);

    selector.comment_edit.set_text("I would like to log on please !");

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
    BOOST_CHECK(notifier.sender == &selector.connect);
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldMV)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
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

    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    Inifile ini;

    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_MANDATORY | COMMENT_VISIBLE);

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

    selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
    selector.selector_lines.set_current_index(0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);


    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == NULL);


    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(selector.current_focus == &selector.ticket_edit);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(selector.current_focus == &selector.comment_edit);
    selector.comment_edit.set_text("I would like to log on please !");
    notifier.event = 0;
    notifier.sender = 0;
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             selector.connect.centerx(),
                             selector.connect.centery(),
                             NULL);
    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(selector.current_focus == &selector.connect);
    notifier.event = 0;
    notifier.sender = 0;
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             selector.connect.centerx(),
                             selector.connect.centery(),
                             NULL);
    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(selector.current_focus == &selector.ticket_edit);


    selector.ticket_edit.set_text("18752");
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
    BOOST_CHECK(notifier.sender == &selector.connect);
}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldMM)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
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

    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    Inifile ini;

    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_MANDATORY | COMMENT_MANDATORY);

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

    selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
    selector.selector_lines.set_current_index(0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);


    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == NULL);


    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(selector.current_focus == &selector.ticket_edit);

    selector.ticket_edit.set_text("18752");

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(selector.current_focus == &selector.comment_edit);
    selector.comment_edit.set_text("I would like to log on please !");

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
    BOOST_CHECK(notifier.sender == &selector.connect);
}
