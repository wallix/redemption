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

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

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
        "\x25\x2d\x00\xcb\x59\x0c\xea\x11\x02\x24"
        "\xd1\x06\xd4\x9b\x35\x7a\xe9\x3d\xb5\xb3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.selector_lines.set_current_index(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xd2\x78\xe1\x56\x1e\x55\x9b\x53\x2f\x19"
        "\xd7\xc4\x24\x32\xf9\x8b\xb5\x59\xde\x8a"
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

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xcf\xd2\xcb\x2b\x18\x63\x75\x23\xa0\x5d"
        "\x88\x24\x50\x7c\xa3\xc0\x41\xdf\x74\x9b"
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

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x9e\x31\xb8\x4d\xc3\xc4\x9e\x6b\x48\xdb"
        "\x24\xe6\xae\x2a\x38\x59\xc6\xa4\x66\x8f"
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

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

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

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

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
        "\x77\x83\xdc\x15\xd6\x3e\xdb\xcb\xbc\x71"
        "\x2f\xa3\xde\xc5\x65\xd7\x92\x86\x1d\x05"
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
        "\x77\x83\xdc\x15\xd6\x3e\xdb\xcb\xbc\x71"
        "\x2f\xa3\xde\xc5\x65\xd7\x92\x86\x1d\x05"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xe0\x9a\x27\xb8\xed\x76\x9e\x6e\x95\x3f"
        "\x84\x55\x48\x6b\x22\x54\x38\x96\xb7\x14"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x25\x2d\x00\xcb\x59\x0c\xea\x11\x02\x24"
        "\xd1\x06\xd4\x9b\x35\x7a\xe9\x3d\xb5\xb3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");

    if (!check_sig(drawable.gd.drawable, message,
         "\xd2\x78\xe1\x56\x1e\x55\x9b\x53\x2f\x19"
         "\xd7\xc4\x24\x32\xf9\x8b\xb5\x59\xde\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x25\x2d\x00\xcb\x59\x0c\xea\x11\x02\x24"
        "\xd1\x06\xd4\x9b\x35\x7a\xe9\x3d\xb5\xb3"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetSelectorFlatFilter)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelectorFlat selector(drawable, "x@127.0.0.1", w, h, parent, notifier, "1", "1");

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
        "\x73\x30\x83\x41\x5d\x9a\xd5\xd7\x91\x20"
        "\x4e\x4f\xe0\x23\x26\xc8\xa6\xf8\x5f\x5b"
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
        "\x41\x41\x64\x2f\x27\x5b\x4a\x31\xd5\x15"
        "\x53\x7f\xff\x08\xbe\xda\x48\x05\xfb\x4b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xd9\x12\xb6\xcd\x86\x11\xab\x8b\x24\xe9"
        "\xa3\x78\x86\x01\xec\xe6\xc5\x05\x5b\xc1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-4.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x4d\x0b\xbd\xff\xc1\xec\xda\x00\x44\x50"
        "\xa8\xe6\xa9\xf9\xe0\x6a\x8e\xee\x64\x7b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x4d\x0b\xbd\xff\xc1\xec\xda\x00\x44\x50"
        "\xa8\xe6\xa9\xf9\xe0\x6a\x8e\xee\x64\x7b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x4d\x0b\xbd\xff\xc1\xec\xda\x00\x44\x50"
        "\xa8\xe6\xa9\xf9\xe0\x6a\x8e\xee\x64\x7b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xb4\x6d\x54\xa5\x8d\x14\x95\x40\xd5\x10"
        "\xb9\xe1\xa5\xcb\x5d\x35\x06\xc5\x51\x5b"
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
        "\xeb\x7c\x0d\x3a\xd8\x4a\x26\xa5\x88\x69"
        "\xfd\x37\x59\xee\x85\x58\x21\x50\xb1\x66"
    )){
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
        "\x42\xd8\xf0\x9f\xb2\xae\x93\xc1\xa7\x4a"
        "\x73\xa4\x10\x2f\xc5\x26\x9d\x00\x57\x0e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
