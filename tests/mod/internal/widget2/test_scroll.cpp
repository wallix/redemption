/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetScroll
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "mod/internal/widget2/scroll.hpp"
#include "mod/internal/widget2/screen.hpp"
//#include "mod/internal/widget2/image.hpp"

#include "check_sig.hpp"
#include "fake_draw.hpp"

// #undef OUTPUT_FILE_PATH
//#define OUTPUT_FILE_PATH "./"

/*
BOOST_AUTO_TEST_CASE(TraceWidgetFrame)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);

    NotifyApi * notifier = nullptr;
    // int         fg_color = RED;
    // int         bg_color = YELLOW;
    int         group_id = 0;
    int16_t     x        = 0;
    int16_t     y        = 0;
    uint16_t    cx       = 400;
    uint16_t    cy       = 300;

    WidgetFrame frame(drawable.gd, Rect(x, y, cx, cy), parent, notifier, group_id);
    // WidgetImage wimage(drawable.gd, 0, 0, FIXTURES_PATH"/Philips_PM5544_192.bmp", parent, notifier);
    // WidgetImage wimage(drawable.gd, 0, 0, FIXTURES_PATH"/color_image.png", parent, notifier);
    WidgetImage wimage(drawable.gd, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);

    frame.set_widget(&wimage);

    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame1.png");

    char message[1024];
    if (!check_sig( drawable.gd.impl(), message,
                    "\xf8\x30\xb3\xcc\xe7\xe6\x7d\x5e\x05\x91"
                    "\x94\x47\x57\xb6\xcf\x98\xff\xb0\xcb\xb1")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(50);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame2.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xb5\x72\xb8\x01\x34\x0d\xed\x89\x70\x9a"
                    "\xfc\xad\x28\xe0\x60\x62\xc9\xbd\x72\x08")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    frame.set_horizontal_widget_pos(70);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame3.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x1e\x7e\xd6\xfd\x56\xb0\x20\xab\x86\x2a"
                    "\x56\xbf\xe4\xe2\xb2\x96\x3d\x5d\x41\x5b")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(0);
    // frame.rdp_input_invalidate(parent.rect);
    frame.set_horizontal_widget_pos(100);
    // ask to widget to redraw at it's current position
    frame.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "frame4.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x12\x05\xf5\x72\x72\x8e\xcd\x45\xf7\xdd"
                    "\x08\xa7\x73\x53\xad\x87\x60\x24\x7c\xa4")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_vertical_widget_pos(100);
    frame.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "frame5.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x11\x47\x9e\xb4\xc6\x81\xe5\xfe\x35\x16"
                    "\xd7\x02\xa3\x01\x2b\x65\x17\x70\x83\x63")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    frame.set_horizontal_widget_pos(20);
    frame.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "frame6.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x2c\xb3\xac\x67\x1a\xcc\xac\x6c\x16\x75"
                    "\x55\xa0\x4d\x66\x4c\x80\x89\xa5\xce\x76")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    frame.set_horizontal_widget_pos(0);
    // frame.rdp_input_invalidate(parent.rect);
    frame.set_vertical_widget_pos(95);
    frame.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "frame7.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x91\x21\x67\xa4\xc6\x04\xc0\x62\xa2\x0c"
                    "\xcd\x32\xfa\xc1\x88\x22\x27\x06\x5b\x59")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFrameScrollbarVertical)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);

    NotifyApi * notifier    = nullptr;
    int         fg_color    = MEDIUM_BLUE;
    int         bg_color    = LIGHT_BLUE;
    int         focus_color = WINBLUE;
    int         group_id    = 0;
    int16_t     x           = 0;
    int16_t     y           = 0;
    uint16_t    cx          = 750;
    uint16_t    cy          = 267;

    WidgetFrame frame(drawable.gd, Rect(x, y, cx, cy), parent, notifier, group_id);
    WidgetImage wimage(drawable.gd, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);
    WidgetVScrollBar bar(drawable.gd, parent, notifier, fg_color, bg_color, focus_color,
                         group_id);

    parent.add_widget(&frame);
    parent.add_widget(&bar);
    frame.set_widget(&wimage);
    bar.set_frame(&frame);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv1.png");

    char message[1024];
    if (!check_sig( drawable.gd.impl(), message,
                    "\xed\x43\x4b\x4d\xa8\x3d\x99\xc2\x73\xaa"
                    "\x29\xf4\xb8\x8c\x10\x2a\xde\xf1\x79\xee")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(30);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv2.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x52\x61\xcd\x0d\x63\xa6\xc0\xe7\xe2\xee"
                    "\x89\xa3\x7b\x89\x14\x33\x67\xf3\x8c\x7a")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(60);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv3.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xb5\xe6\x94\xba\x12\xdd\x08\x82\xe8\xdc"
                    "\x76\xe7\xe2\x78\x61\xa2\x32\xf1\xc9\x37")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(100);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv4.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x20\x99\x11\xa0\x76\x08\x85\xc0\x17\xe3"
                    "\xe4\x52\x67\x9b\xb9\x7d\xf9\x5f\x6d\xa0")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(40);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv5.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xdb\x0f\x0c\x59\xa0\x57\x9a\x18\x74\x93"
                    "\x0d\x0f\x82\x6f\x95\x26\x8c\x56\x11\x39")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(5);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv6.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xc7\xc7\x60\x9b\x89\x45\x5d\x6b\x76\x96"
                    "\xdd\x95\x83\x48\x59\xfa\x0e\xe3\x0d\xc5")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_vertical_scroll(0);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarv7.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xed\x43\x4b\x4d\xa8\x3d\x99\xc2\x73\xaa"
                    "\x29\xf4\xb8\x8c\x10\x2a\xde\xf1\x79\xee")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFrameScrollbarHorizontal)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);

    NotifyApi * notifier    = nullptr;
    int         fg_color    = MEDIUM_BLUE;
    int         bg_color    = LIGHT_BLUE;
    int         focus_color = WINBLUE;
    int         group_id    = 0;
    int16_t     x           = 0;
    int16_t     y           = 0;
    uint16_t    cx          = 379;
    uint16_t    cy          = 267;

    WidgetFrame frame(drawable.gd, Rect(x, y, cx, cy), parent, notifier, group_id);
    WidgetImage wimage(drawable.gd, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);
    WidgetHScrollBar bar(drawable.gd, parent, notifier, fg_color, bg_color, focus_color,
                         group_id);

    parent.add_widget(&frame);
    parent.add_widget(&bar);
    frame.set_widget(&wimage);
    bar.set_frame(&frame);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarh1.png");

    char message[1024];
    if (!check_sig( drawable.gd.impl(), message,
                    "\x8b\x80\xef\x39\xcb\xb3\x16\x8c\x4e\xee"
                    "\xe5\x40\x1a\xca\xbe\x5a\xe2\xec\xea\x03")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_horizontal_scroll(30);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "/tmp/framebarh2.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xf5\x92\xff\x5e\xe1\xcc\x0a\x37\x9b\x30"
                    "\x02\x75\xc1\x9f\xae\xbd\x9b\xbb\x44\x50")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_horizontal_scroll(60);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarh3.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x2f\xba\x8a\xe3\x1e\x9f\xe5\x95\x0b\x7f"
                    "\x3d\xbe\x2e\x72\x07\x75\xab\x9b\x4d\xdf")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_horizontal_scroll(100);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarh4.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x89\x02\x7a\xdd\x2f\x76\x2c\x70\x05\xc3"
                    "\xc7\x48\x29\x3b\xde\xab\xc2\x30\xe1\x93")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_horizontal_scroll(40);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarh5.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xa3\x43\x21\x16\x91\x79\x42\x55\xfb\xd3"
                    "\x8c\x64\x16\xfd\x5b\x6b\xc3\x9e\xad\xfa")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_horizontal_scroll(5);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarh6.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x19\xbc\xbb\x98\x7e\x3d\x61\x2f\x9f\xfe"
                    "\x30\xa5\xc5\xf2\x42\x0e\xba\xb9\x21\x16")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    bar.set_horizontal_scroll(0);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarh7.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x8b\x80\xef\x39\xcb\xb3\x16\x8c\x4e\xee"
                    "\xe5\x40\x1a\xca\xbe\x5a\xe2\xec\xea\x03")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFrameScrollbarControl)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);

    NotifyApi * notifier    = nullptr;
    int         fg_color    = MEDIUM_BLUE;
    int         bg_color    = LIGHT_BLUE;
    int         focus_color = WINBLUE;
    int         group_id    = 0;
    int16_t     x           = 0;
    int16_t     y           = 0;
    uint16_t    cx          = 400;
    uint16_t    cy          = 300;

    WidgetFrame frame(drawable.gd, Rect(x, y, cx, cy), parent, notifier, group_id);
    WidgetImage wimage(drawable.gd, 0, 0, FIXTURES_PATH"/win2008capture10.png", parent, notifier);
    WidgetVScrollBar vbar(drawable.gd, parent, notifier, fg_color, bg_color, focus_color,
                          group_id);
    WidgetHScrollBar hbar(drawable.gd, parent, notifier, fg_color, bg_color, focus_color,
                          group_id);

    parent.add_widget(&frame);
    parent.add_widget(&vbar);
    parent.add_widget(&hbar);
    frame.set_widget(&wimage);
    vbar.set_frame(&frame);
    hbar.set_frame(&frame);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc01.png");

    char message[1024];
    if (!check_sig( drawable.gd.impl(), message,
                    "\x50\xe3\x5f\xa8\x23\x4d\x4f\x96\x48\x90"
                    "\x54\xa6\x68\x75\x44\x29\x8a\xba\xdc\x29")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // first action
    // move horizontal bar to center
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.scroll.getCenteredX(), hbar.scroll.getCenteredY(),
                           nullptr);

    parent.rdp_input_mouse(MOUSE_FLAG_MOVE,
                           hbar.rect.getCenteredX(), hbar.rect.getCenteredY(),
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc02.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x12\x87\xa0\x8a\x64\xbe\xd1\xef\xcb\x45"
                    "\x30\x85\x06\xa9\xd7\x5c\x06\x04\xda\xdc")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // click on right bar in scroll control
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           (hbar.rect.right() + hbar.scroll.right()) / 2,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           (hbar.rect.right() + hbar.scroll.right()) / 2,
                           hbar.rect.getCenteredY(),
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc03.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x84\x49\x7a\x99\x01\x8c\x77\x3a\xb2\x61"
                    "\xee\x2c\x95\xd3\x96\x20\xc1\x9a\x99\x42")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // click on right bar in scroll control
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           (hbar.rect.right() + hbar.scroll.right()) / 2,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           (hbar.rect.right() + hbar.scroll.right()) / 2,
                           hbar.rect.getCenteredY(),
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc04.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xc8\x2a\x16\x07\x0f\x26\x82\x7f\xa8\x1e"
                    "\x36\x45\xa9\x44\xda\xab\xbe\x5d\x82\x1d")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // click on left bar in scroll control to the end
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           hbar.rect.x + 2 ,
                           hbar.rect.getCenteredY(),
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc05.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xa5\x8f\x11\x10\xc1\x1b\x88\x85\x4c\xcf"
                    "\x51\x5e\xe1\xdf\x2a\xed\xa0\x48\x79\x40")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    // keyboard right to the end
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc06.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xf7\x4c\x26\xa5\x95\x5f\x3a\x9e\x70\x9e"
                    "\x4c\xd6\xab\xe1\xa2\x29\xb4\xe1\xfc\xc6")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc07.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x26\x78\x76\xd4\x19\xef\xca\x14\x15\x80"
                    "\xf4\xc6\x44\x13\x05\x2d\xd6\x07\x8d\xf5")) {
        BOOST_CHECK_MESSAGE(false, message);
    }





    // first action
    // move horizontal bar to center
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.scroll.getCenteredX(), vbar.scroll.getCenteredY(),
                           nullptr);

    parent.rdp_input_mouse(MOUSE_FLAG_MOVE,
                           vbar.rect.getCenteredX(), vbar.rect.getCenteredY(),
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc08.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x86\x7f\x03\xf7\x8d\xc2\xb1\xc5\x35\x4b"
                    "\x48\x09\x04\x98\xe9\x9f\x25\x47\xb5\xee")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // click on right bar in scroll control
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           (vbar.rect.bottom() + vbar.scroll.bottom()) / 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           vbar.rect.getCenteredX(),
                           (vbar.rect.bottom() + vbar.scroll.bottom()) / 2,
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc09.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x40\x89\xcb\xfd\xa9\xe4\x7c\x87\xfd\xf5"
                    "\xa8\xf8\xe2\x4c\xe0\x4d\x81\x9e\x32\xbd")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // click on right bar in scroll control
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           (vbar.rect.bottom() + vbar.scroll.bottom()) / 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1),
                           vbar.rect.getCenteredX(),
                           (vbar.rect.bottom() + vbar.scroll.bottom()) / 2,
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc10.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\xcb\x83\x63\x93\x4f\xf6\x7d\xdf\x08\x93"
                    "\xd7\x98\x6b\xb6\x89\xa5\x51\xfa\x2c\x84")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // click on left bar in scroll control to the end
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                           vbar.rect.getCenteredX(),
                           vbar.rect.y + 2,
                           nullptr);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc11.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x7f\x06\xcd\xcf\xd4\xc3\xfa\xdd\x5d\x57"
                    "\xe3\x72\xd2\x28\x6e\xa2\x9b\xc0\x08\x27")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    // keyboard down to the end
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);

    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc12.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x73\xe1\x2f\x4a\x86\x59\xb6\x35\xae\x86"
                    "\x55\x3f\x82\x0c\x92\xe6\xf0\xd0\x69\x3a")) {
        BOOST_CHECK_MESSAGE(false, message);
    }


    // keyboard up
    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    parent.rdp_input_scancode(0,0,0,0, &keymap);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "framebarc13.png");

    if (!check_sig( drawable.gd.impl(), message,
                    "\x52\xb2\xa3\x27\x20\x97\x35\x95\xf8\xca"
                    "\xbc\xbf\x84\x09\x24\x7c\xb0\x00\xc1\x04")) {
        BOOST_CHECK_MESSAGE(false, message);
    }
}
*/

BOOST_AUTO_TEST_CASE(TestWidgetHScrollBar)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable.gd, parent, notifier, true, id,
                          fg_color, bg_color, focus_color, font, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(200, dim.h);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    drawable.save_to_png("scroll0.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xaf\x8d\xda\xea\xbe\x5c\x0b\x53\xf4\x28\xe3\xbc\x29\x49\xa1\x4c\x75\x7f\xc5\xbf"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    drawable.save_to_png("scroll1.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x68\xec\xba\xe1\xbe\x41\x74\x31\x17\xd2\x89\x2c\x35\x1a\xa0\x6a\x10\x06\xa1\x9a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TestWidgetVScrollBar)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int focus_color = WINBLUE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetScrollBar wscroll(drawable.gd, parent, notifier, false, id,
                          fg_color, bg_color, focus_color, font, 50);
    Dimension dim = wscroll.get_optimal_dim();
    wscroll.set_wh(dim.w, 200);
    wscroll.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wscroll.rdp_input_invalidate(wscroll.get_rect());


    drawable.save_to_png("scroll2.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x35\x2b\x65\xcf\x60\x37\xc3\x4f\x19\x5e\xf5\xf7\x55\xb6\x87\x3b\x2b\x84\x52\x41"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscroll.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x + 5, y + 5, nullptr);

    wscroll.rdp_input_invalidate(wscroll.get_rect());


    drawable.save_to_png("scroll3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x22\x74\x49\xb3\xe6\xc2\x89\x21\xf5\x98\xe0\x63\x21\xeb\x5f\xfd\xe2\xe4\x61\x02"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
