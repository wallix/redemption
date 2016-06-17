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
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "utils/log.hpp"

#include "core/font.hpp"
#include "mod/internal/widget2/flat_selector2.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
        "\x94\x2d\xae\x12\x7a\x0b\xf8\x2c\xfc\xa9\x24\xd8\x57\xeb\xab\x7f\xa1\x5b\x7a\x44"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x61\x13\x7f\x30\x4b\xc6\xba\x16\xaa\xe8\x0f\x0e\x6d\xe4\xf0\x70\x92\x94\x25\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatResize)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 640, 480, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier,
                                "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
        "\x52\xe4\x45\x0c\xbf\x17\x7e\x68\xaf\x87\x69\xeb\x3f\x59\xdb\x2e\x54\xc2\x72\xf0"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xc1\xc5\x3b\x48\x52\x4e\xa1\x7d\x05\xf2\x86\x18\xd9\x53\xb2\x7b\xf5\x62\xdf\xa4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x8a\xe8\x2d\x28\xaf\xd5\x73\x5e\xdb\x17\xe6\x86\xa8\x54\x7a\x4f\x8c\x2b\x9c\x42"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x2a\x66\xcd\x49\x52\xfb\x46\xe8\x0b\x2f\x40\xa8\xe4\xba\x7e\xb3\x77\xd5\xdc\x92"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      5 + selector.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x23\xea\xe9\x93\x5f\xe1\x7d\x1c\x73\x71\x45\x56\xe2\xde\x4b\xff\x0a\x27\x38\xd6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatEventSelect)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
        "\x5b\x99\xb3\x29\xb0\x03\xc4\x56\x36\x00\xf2\x57\x7e\xe3\xf5\x33\xd1\xa9\x73\x65"
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
        "\x73\xe0\x8c\x66\x6a\x36\xfd\x4a\xd1\xa2\x3f\xaa\x9d\x9f\xb1\x8b\x75\x89\xd7\x1e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x3e\xa7\x28\x0c\x6f\xe4\x37\x64\x4f\xcd\x94\x0b\xfc\x70\xf7\x5b\x80\xa1\xa2\xc9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x73\xe0\x8c\x66\x6a\x36\xfd\x4a\xd1\xa2\x3f\xaa\x9d\x9f\xb1\x8b\x75\x89\xd7\x1e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x5b\x99\xb3\x29\xb0\x03\xc4\x56\x36\x00\xf2\x57\x7e\xe3\xf5\x33\xd1\xa9\x73\x65"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x73\xe0\x8c\x66\x6a\x36\xfd\x4a\xd1\xa2\x3f\xaa\x9d\x9f\xb1\x8b\x75\x89\xd7\x1e"
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

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
        "\x98\x12\x0d\x39\x04\x0a\x38\xbb\xac\x6b\x8d\x4c\xf4\x5c\x19\x99\x7c\x55\x75\xac"
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
        "\x95\x48\x80\x5c\xf3\xac\x0b\x78\x3d\xb9\xc8\x35\x5f\x94\x53\xee\xca\x28\xeb\x9d"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x84\x83\x35\x62\xbe\x9d\xe4\x95\x67\xb0\x47\xdf\xd1\xc8\xf7\xda\x15\x56\x12\xfa"
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
        "\x89\xd6\xff\xa1\xc3\x8d\x8a\x4f\x3c\xf2\x22\xd2\xe2\x02\xb1\x7b\xd9\xb3\x7e\x1a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x29\x59\xf5\xa7\xb5\xce\x79\xba\xcf\xb8\x5a\x32\x4c\x3f\x74\x14\x80\x1c\x61\xd5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x7e\xb0\xad\x51\x8e\x02\x70\x25\xf2\x48\xe8\xed\xc2\x73\xfe\x59\x75\x95\x6e\x57"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf2\xed\x93\x85\x32\x6d\xbd\x16\x8c\x4e\xe4\x23\xcd\x72\xb9\x53\xaa\x4b\xc9\x2e"
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
        "\x8e\x01\x88\x64\x1c\x14\x6e\x79\x4b\x88\x30\x50\x3d\x07\x7a\x53\x80\x1a\x80\xc7"
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
        "\xb8\x56\xbd\x15\xf9\x0e\xf4\x32\x2f\x30\x33\xe5\xf7\x1d\x63\xe8\xb6\xd6\xef\x3e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable.gd, 800, 600);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Font font;
//     // ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
//     WidgetScreen parent(drawable.gd, 640, 480);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Font font;
//     // ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
//     WidgetScreen parent(drawable.gd, 1280, 1024);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Font font;
//     // ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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
//     WidgetScreen parent(drawable.gd, 800, 600);
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
//     Font font;

//     ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_VISIBLE | COMMENT_VISIBLE);

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
//     WidgetScreen parent(drawable.gd, 800, 600);
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
//     Font font;

//     ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;
//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_VISIBLE | COMMENT_MANDATORY);

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
//     WidgetScreen parent(drawable.gd, 800, 600);
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
//     Font font;

//     ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;
//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_MANDATORY | COMMENT_VISIBLE);

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
//     WidgetScreen parent(drawable.gd, 800, 600);
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
//     Font font;

//     ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;
//     WidgetSelectorFlat2 selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_MANDATORY | COMMENT_MANDATORY);

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
