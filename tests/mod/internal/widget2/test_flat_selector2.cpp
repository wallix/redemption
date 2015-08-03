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
#define BOOST_TEST_MODULE TestWidgetSelectorFlat2
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "log.hpp"

#include "config.hpp"
#include "internal/widget2/flat_selector2.hpp"
#include "internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP");

    selector.selector_lines.set_selection(0);

    selector.rearrange();

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x16\x28\xc5\x9c\x4f\x0f\xe6\xae\x46\x69\x24\xc5\xb3\xb6\x3f\x3f\x0a\x75\x44\xfa"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xb5\x1f\x5e\xfb\xcc\x37\x36\x5a\x21\xcb\xae\xa3\x40\x2f\x15\x3b\x0f\x5f\xa8\xc5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatResize)
{
    TestDraw drawable(640, 480);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 640, 480, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier,
                                "1", "1", nullptr, nullptr, nullptr, ini);

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP");

    selector.selector_lines.set_selection(0);
    selector.rearrange();

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc1\xca\x99\x1a\xce\x7e\xd9\xdc\x9f\x1b\x00\xfa\xae\x85\x39\xcc\x80\x82\x35\x6e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf5\x75\x72\x5e\xe7\x0a\x9e\xc3\xdf\x63\x93\xa9\x69\xdc\x11\xf3\x7e\x72\x41\x38"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat2)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x79\x57\x15\x0e\xb1\xa6\x08\x1e\x55\xd1\x49\xf2\x70\xf7\xd4\xbd\xf5\x94\xbe\xc8"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x44\x23\xde\xb7\x30\x9c\xe3\xad\x05\xa3\x3a\x79\x60\xc4\xe9\x7b\xd5\xfc\x86\x62"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

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

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP");

    selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
    selector.selector_lines.set_selection(0);

    selector.rearrange();

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.dx() + 20,
                                            selector.selector_lines.dy() + 40,
                                            nullptr);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xd3\xf2\xf4\x9c\x0c\x99\xbd\x39\xb1\x9d\xc7\x30\x1d\x42\x60\x55\x2c\x4d\xa4\xf1"
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
        "\xc4\xe8\x8b\x1a\xd7\x60\x5e\x2b\x65\xf0\x79\x7a\x30\xe6\x38\x03\x66\xdc\x76\xb1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xeb\xfc\x35\xe1\xd3\xeb\xe1\x9c\xf0\xf4\x56\xfd\x06\xfe\x62\x60\x2c\xf9\x60\x09"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xe8\x8b\x1a\xd7\x60\x5e\x2b\x65\xf0\x79\x7a\x30\xe6\x38\x03\x66\xdc\x76\xb1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xd3\xf2\xf4\x9c\x0c\x99\xbd\x39\xb1\x9d\xc7\x30\x1d\x42\x60\x55\x2c\x4d\xa4\xf1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xe8\x8b\x1a\xd7\x60\x5e\x2b\x65\xf0\x79\x7a\x30\xe6\x38\x03\x66\xdc\x76\xb1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // int x = selector.selector_lines.rect.x + 5;
    // int y = selector.selector_lines.rect.y + 3;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
    // x += selector.selector_lines.group_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
    // x += selector.selector_lines.target_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
    // x += selector.selector_lines.protocol_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, nullptr);
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatFilter)
{
    TestDraw drawable(800, 600);

    Inifile ini(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, ini.get<cfg::font>());
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

    selector.add_device("reptile", "snake@10.10.14.111",
                        "RDP");
    selector.add_device("bird", "raven@10.10.14.111",
                        "RDP");
    selector.add_device("reptile", "lezard@10.10.14.27",
                        "VNC");
    selector.add_device("fish", "shark@10.10.14.103",
                        "RDP");
    selector.add_device("bird", "eagle@10.10.14.33",
                        "VNC");

    int curx = 0;
    int cury = 0;
    selector.rearrange();
    selector.selector_lines.set_selection(0);

    curx = selector.filter_target_group.centerx();
    cury = selector.filter_target_group.centery();
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             curx, cury,
                             nullptr);
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             curx, cury,
                             nullptr);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x2e\xa9\x89\x41\x19\xab\xf4\x72\xff\x48\xcb\x17\x0d\x9f\x16\x1d\x17\xfc\xc3\x44"
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
        "\xa6\xba\x06\x4c\x4d\xea\x4f\xe3\x16\xd5\xbb\xb5\x27\x3a\x81\x1e\x58\x7f\xc2\xac"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x33\xb4\x94\x2e\x8c\xc2\x22\x44\x72\x84\xfc\x93\xd3\x31\xf4\xb4\x8a\x0e\x54\x9b"
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
        "\xb3\x5c\x83\xd6\x87\x5d\xd4\xb0\xae\x04\x86\xf8\x3e\x35\x35\x50\x41\x0a\xe3\xb0"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x48\x0d\x4b\xb5\x29\x9a\x5e\xeb\x11\xdc\x13\x3d\x5d\xff\x66\x53\x93\xd2\x44\xa9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x48\x09\xec\x73\x0c\x9b\xa4\x83\x65\x6e\xa6\x08\x59\x3c\x89\xab\x5a\x46\x76\x84"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x22\xdf\x1d\xbc\x6f\x05\x68\x41\x22\xcb\xd3\xd9\x1d\x08\xc0\x59\xef\x41\x97\x9b"
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
        "\xb2\xf4\xd9\xfd\xda\xdb\xa2\x00\xe7\x6a\x0d\xd2\xdc\xd7\x88\xc9\x7e\xab\xdc\xa6"
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
        "\x90\x01\xac\xaf\xe1\xcf\x20\xa4\xb0\xbe\x0c\x6c\x7f\x29\x27\xe3\x79\x00\xbc\x34"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Inifile ini;
//     // ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33azertyuiopŝdfghjklmx",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");

//     int curx = 0;
//     int cury = 0;

//     selector.selector_lines.set_selection(0);

//     curx = selector.filter_device.centerx();
//     cury = selector.filter_device.centery();
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
//                              curx, cury,
//                              nullptr);
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              curx, cury,
//                              nullptr);

//     // selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-1.png");

//     char message[1024];

//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x36\xf9\x92\x34\x51\xa3\xe6\x18\xb8\xbb"
//                    "\x7d\xd9\xa2\x92\x14\xe8\x40\xb3\x04\x2f"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }


//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsqlkmlsdkmaklzeerd", "eagle@10.10.14.33azert",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-2.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x08\x3d\xdd\xe9\x60\x8b\xf9\xc9\x74\xcc"
//                    "\xb3\x4c\xb4\x1d\xa4\x85\x3f\xdd\xe1\xad"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }


//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelknelkaznelkssdlqk",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-3.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x6d\x23\xab\x06\x73\x7c\xbe\x69\x6b\x33"
//                    "\xd4\x94\x7a\x4c\xc7\x38\x68\x01\xcd\xcf"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }

//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelkn",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fkljazelkjalkzjelakzejish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-4.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x82\x5c\xe0\x0e\x0e\xcb\x63\x48\x64\xb7"
//                    "\x6f\x05\xa8\x92\x40\xe6\x51\x7f\x20\x32"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns2)
// {
//     TestDraw drawable(640, 480);

//     // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable, 640, 480);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Inifile ini;
//     // ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33azertyuiopŝdfghjklmx",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");

//     int curx = 0;
//     int cury = 0;

//     selector.selector_lines.set_selection(0);

//     curx = selector.filter_device.centerx();
//     cury = selector.filter_device.centery();
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
//                              curx, cury,
//                              nullptr);
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              curx, cury,
//                              nullptr);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-1.png");

//     char message[1024];

//     if (!check_sig(drawable.gd.impl(), message,
//                    "\xc3\x87\x52\xfc\xbc\x47\x3d\x20\x34\x33"
//                    "\x7c\x54\xcd\x65\xf1\x3f\xab\xc3\x12\x64"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }



//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsqlkmlsdkmaklzeerd", "eagle@10.10.14.33azert",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-2.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x88\xff\x6f\x35\xb6\xca\xf3\x61\x5a\x8c"
//                    "\xcf\x4b\xa0\x27\x98\x4e\xe4\x0a\x38\x2d"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }



//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelknelkaznelkssdlqk",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-3.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x04\xf1\xeb\x69\xe9\xbc\xda\x26\xd4\xe0"
//                    "\x96\xd1\xa7\x59\x52\x0a\x2e\x7d\x44\x01"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }


//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelkn",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fkljazelkjalkzjelakzejish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-4.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x8d\x52\x86\xa1\xbe\x32\x62\xa9\x14\xc6"
//                    "\x06\xea\xa6\x27\x18\x10\x8b\xa8\x4b\xe7"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }

// }

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns3)
// {
//     TestDraw drawable(1280, 1024);

//     // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable, 1280, 1024);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Inifile ini;
//     // ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, ini);

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33azertyuiopŝdfghjklmx",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");

//     int curx = 0;
//     int cury = 0;

//     selector.selector_lines.set_selection(0);

//     curx = selector.filter_device.centerx();
//     cury = selector.filter_device.centery();
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
//                              curx, cury,
//                              nullptr);
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              curx, cury,
//                              nullptr);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-1.png");

//     char message[1024];

//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x7d\x56\xc8\x95\xa9\xb2\x32\x5c\x7d\x61"
//                    "\x7f\x06\x0b\x3c\x6b\x1f\x37\x71\xad\x28"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }



//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsqlkmlsdkmaklzeerd", "eagle@10.10.14.33azert",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-2.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x4c\x80\xba\x39\xfc\xc3\xbe\x2a\x1f\xc4"
//                    "\x25\xdf\xcf\xc7\x13\x57\x8d\xb0\xfa\xad"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }



//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelknelkaznelkssdlqk",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-3.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x8a\xb8\x20\xdf\xad\x13\x4e\x70\x6f\x7a"
//                    "\xe3\xc6\x21\x8d\xa6\x3b\xa5\x16\x84\xe7"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }


//     selector.selector_lines.clear();

//     selector.add_device("reptile", "snake@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("birdsandoandjdanxqpa&éjmql&&,é", "raven@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("reptile", "lezard@10.10.14.27",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("bilkmdsd", "eagle@10.10.14.33azezakljemlkeakemelmakrtdslkazelkn",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.add_device("fkljazelkjalkzjelakzejish", "shark@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("bird", "eagle@10.10.14.33pezok&mlé,&m,qdns,nq&n&élkn&lnlnezl&ne&lkn&é",
//                         "VNC", "2013-04-20 19:56:50");
//     selector.selector_lines.set_selection(0);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-4.png");
//     if (!check_sig(drawable.gd.impl(), message,
//                    "\x1a\x2f\xf3\x80\x9f\x43\x8d\x11\xd5\x1a"
//                    "\x56\xf6\xd4\xfc\xef\x4a\x3a\xc0\x80\x3c"
//                    )){
//         BOOST_CHECK_MESSAGE(false, message);
//     }

// }

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldVV)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     struct Notify : NotifyApi {
//         Widget2* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget2* sender, notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;

//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();
//     Inifile ini;

//     ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_VISIBLE | COMMENT_VISIBLE);

//     selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.27",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.33",
//                         "RDP", "2013-04-20 19:56:50");

//     selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     // selector.selector_lines.set_selection(1);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == nullptr);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     BOOST_CHECK(notifier.sender == &selector.connect);
// }

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldVM)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     struct Notify : NotifyApi {
//         Widget2* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget2* sender, notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;

//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();
//     Inifile ini;

//     ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_VISIBLE | COMMENT_MANDATORY);

//     selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.27",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.33",
//                         "RDP", "2013-04-20 19:56:50");

//     selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     // selector.selector_lines.set_selection(1);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == nullptr);

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == nullptr);
//     BOOST_CHECK(selector.current_focus == &selector.comment_edit);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == nullptr);
//     BOOST_CHECK(selector.current_focus == &selector.comment_edit);

//     selector.comment_edit.set_text("I would like to log on please !");

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     BOOST_CHECK(notifier.sender == &selector.connect);
// }

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldMV)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     struct Notify : NotifyApi {
//         Widget2* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget2* sender, notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;

//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();
//     Inifile ini;

//     ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_MANDATORY | COMMENT_VISIBLE);

//     selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.27",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.33",
//                         "RDP", "2013-04-20 19:56:50");

//     selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == nullptr);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.ticket_edit);

//     keymap.push_kevent(Keymap2::KEVENT_TAB);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);
//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.comment_edit);
//     selector.comment_edit.set_text("I would like to log on please !");
//     notifier.event = 0;
//     notifier.sender = 0;
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
//                              selector.connect.centerx(),
//                              selector.connect.centery(),
//                              nullptr);
//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.connect);
//     notifier.event = 0;
//     notifier.sender = 0;
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              selector.connect.centerx(),
//                              selector.connect.centery(),
//                              nullptr);
//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.ticket_edit);


//     selector.ticket_edit.set_text("18752");
//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     BOOST_CHECK(notifier.sender == &selector.connect);
// }


// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatDescFieldMM)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     struct Notify : NotifyApi {
//         Widget2* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget2* sender, notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;

//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();
//     Inifile ini;

//     ini.set<cfg::translation::target>_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, ini, TICKET_MANDATORY | COMMENT_MANDATORY);

//     selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.111",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.27",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.103",
//                         "RDP", "2013-04-20 19:56:50");
//     selector.add_device("rdp", "administrateur@qa@10.10.14.33",
//                         "RDP", "2013-04-20 19:56:50");

//     selector.set_widget_focus(&selector.selector_lines, Widget2::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == nullptr);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.ticket_edit);

//     selector.ticket_edit.set_text("18752");

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.comment_edit);
//     selector.comment_edit.set_text("I would like to log on please !");

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     BOOST_CHECK(notifier.sender == &selector.connect);
// }
