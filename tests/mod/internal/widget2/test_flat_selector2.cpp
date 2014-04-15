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

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/flat_selector2.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;
    Inifile ini;

    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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

    selector.selector_lines.set_selection(0);

    selector.rearrange();

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x01\x89\x14\x15\x5a\x7d\xac\x8f\xd6\x1c"
                   "\x52\x18\xf4\x00\x34\xa0\x32\xf8\xe3\x90")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x00\x8f\xa0\x8d\x85\x80\xb0\x33\x62\xf8"
                   "\xcd\x77\xc6\x1c\xce\xbf\xde\xdc\x0b\x76")){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatResize)
{
    TestDraw drawable(640, 480);

    // WidgetSelectorFlat2 is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 640, 480);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier,
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

    selector.selector_lines.set_selection(0);
    selector.rearrange();

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd9\x5e\xfb\x45\xb0\x23\x8d\x92\xf9\xef"
                   "\xa2\xdb\x5b\x43\xf1\x33\xf3\xa9\xcb\x87")){
        BOOST_CHECK_MESSAGE(false, message);
    }


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x19\x4d\x12\x17\xfd\x83\x2c\x61\x34\x52"
                   "\x70\x67\xae\xaa\xb0\xad\xb5\xb9\xbf\xc6")){
        BOOST_CHECK_MESSAGE(false, message);
    }

}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlat2)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Cible");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x30\x95\x3e\xcd\x37\xf2\x63\x59\x3d\xda"
                   "\x67\xe2\x06\x55\x52\x8e\x7e\x87\x63\x2c")){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x30\xf2\x5b\x0d\xb0\x4e\xce\x7d\xd6\xa2"
                   "\x4a\x68\x6f\x03\xfb\x47\x12\x17\xa0\x08"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
    selector.selector_lines.set_selection(0);

    selector.rearrange();

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.dx() + 20,
                                            selector.selector_lines.dy() + 40,
                                            NULL);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.drawable, message,
                   "\xaf\xc8\x83\x43\x03\x1c\xef\xd7\xe3\x82"
                   "\xf8\xc8\x44\xae\xd7\x1f\x23\xaf\xe7\x21"
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
                   "\x3c\x77\x54\x6c\x62\xe2\x75\x95\x6c\x24"
                   "\xa4\xec\xc1\x13\x53\x55\x65\xe4\xb4\xfd"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xae\x7c\xc6\xde\x1a\xee\xf5\x72\x14\xd5"
                   "\x3c\x96\x85\x69\x0f\xe1\x71\x12\x53\x3c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xf4\x4c\xfc\xc8\x1f\xe7\x7f\x20\x46\xb5"
                   "\x33\xec\x92\xc0\x0b\x1c\xd0\x29\x55\x06"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\x3c\x77\x54\x6c\x62\xe2\x75\x95\x6c\x24"
                   "\xa4\xec\xc1\x13\x53\x55\x65\xe4\xb4\xfd"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }




    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xf4\x4c\xfc\xc8\x1f\xe7\x7f\x20\x46\xb5"
                   "\x33\xec\x92\xc0\x0b\x1c\xd0\x29\x55\x06"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // int x = selector.selector_lines.rect.x + 5;
    // int y = selector.selector_lines.rect.y + 3;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);
    // x += selector.selector_lines.group_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);
    // x += selector.selector_lines.target_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);
    // x += selector.selector_lines.protocol_w;
    // selector.rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, NULL);


}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatFilter)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    Inifile ini;
//    ini.translation.target.set_from_cstr("Target");

    WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
    selector.rearrange();
    selector.selector_lines.set_selection(0);

    curx = selector.filter_target_group.centerx();
    cury = selector.filter_target_group.centery();
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
                   "\xbe\x79\x44\x05\x35\x34\xd1\xec\xb0\x55"
                   "\x54\xf0\xa2\xd4\xa2\x3d\x29\x6f\x66\xd8"
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
                   "\xbe\xc9\xbc\x0f\x63\x14\x75\x4f\x5b\xc4"
                   "\x12\xb1\x2c\x16\x17\x2d\x9a\x56\xe1\x7b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x01\x82\x15\x02\x6b\x93\x86\x7f\x3c\x20"
                   "\x5d\xcf\x90\x38\x73\x43\xc3\x61\x95\xd8"
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
                   "\x26\xb2\x00\x4a\x24\x7a\x64\x00\x40\x51"
                   "\xa7\x6c\x73\x88\xfc\x30\xa4\xfe\x27\x2d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xd7\x63\xd1\xe5\x37\x78\xd4\x0e\x0b\x2a"
                   "\x7e\xe0\xaf\xc2\x40\x8a\xb8\x73\x12\x47"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xfb\xda\x8b\x44\xa5\xdc\x28\xce\x92\x47"
                   "\xef\x2a\x56\x5f\x7c\xee\x78\xab\x10\x99"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xbc\xca\x4a\x1d\x62\x3a\x2c\x93\xe6\x25"
                   "\xb5\xde\x87\x71\x2d\x7d\x5a\xd1\xa0\x91"
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
                   "\x46\x52\x87\xdc\x5f\x0d\x8d\x88\x91\xb5"
                   "\x35\x05\xd0\x6d\xa1\x43\xe4\xbd\xd8\xe6"
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
                   "\x1c\x9c\xaa\x2d\x2b\xf0\x6f\xa4\xd9\xc1"
                   "\x4d\x35\x9c\xf1\x85\xf2\xd4\xb7\x73\x58"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

// BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatAdjustColumns)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelectorFlat2 is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable, 800, 600);
//     NotifyApi * notifier = NULL;
//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;

//     Inifile ini;
//     // ini.translation.target.set_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
//                              NULL);
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              curx, cury,
//                              NULL);

//     // selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust-1.png");

//     char message[1024];

//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     NotifyApi * notifier = NULL;
//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;

//     Inifile ini;
//     // ini.translation.target.set_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
//                              NULL);
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              curx, cury,
//                              NULL);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust2-1.png");

//     char message[1024];

//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     NotifyApi * notifier = NULL;
//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;

//     Inifile ini;
//     // ini.translation.target.set_from_cstr("Target");

//     WidgetSelectorFlat2 selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1", 0, 0, 0, ini);

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
//                              NULL);
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              curx, cury,
//                              NULL);

//     selector.fit_columns();

//     selector.rdp_input_invalidate(selector.rect);

//     // drawable.save_to_png(OUTPUT_FILE_PATH "selector-adjust3-1.png");

//     char message[1024];

//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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
//     if (!check_sig(drawable.gd.drawable, message,
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

//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;
//     Inifile ini;

//     ini.translation.target.set_from_cstr("Target");

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

//     selector.set_widget_focus(&selector.selector_lines);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     // selector.selector_lines.set_selection(1);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == NULL);


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

//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;
//     Inifile ini;

//     ini.translation.target.set_from_cstr("Target");

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

//     selector.set_widget_focus(&selector.selector_lines);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     // selector.selector_lines.set_selection(1);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == NULL);

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == NULL);
//     BOOST_CHECK(selector.current_focus == &selector.comment_edit);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == NULL);
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

//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;
//     Inifile ini;

//     ini.translation.target.set_from_cstr("Target");

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

//     selector.set_widget_focus(&selector.selector_lines);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == NULL);


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
//                              NULL);
//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(selector.current_focus == &selector.connect);
//     notifier.event = 0;
//     notifier.sender = 0;
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              selector.connect.centerx(),
//                              selector.connect.centery(),
//                              NULL);
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

//     int16_t w = drawable.gd.drawable.width;
//     int16_t h = drawable.gd.drawable.height;
//     Inifile ini;

//     ini.translation.target.set_from_cstr("Target");

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

//     selector.set_widget_focus(&selector.selector_lines);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     BOOST_CHECK_EQUAL(notifier.event, 0);
//     BOOST_CHECK(notifier.sender == NULL);


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
