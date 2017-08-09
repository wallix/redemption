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

#define RED_TEST_MODULE TestWidgetSelector
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/log.hpp"

#include "core/font.hpp"
#include "mod/internal/widget/selector.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, 3, font, Theme(), Translation::EN);

    const char * add1[] = {"rdp", "qa\\administrateur@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add1);

    const char * add2[] = {"rdp", "administrateur@qa@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add2);

    const char * add3[] = {"rdp", "administrateur@qa@10.10.14.27",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add3);

    const char * add4[] = {"rdp", "administrateur@qa@10.10.14.103",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add4);

    const char * add5[] = {"rdp", "administrateur@qa@10.10.14.33",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector1.png");

    RED_CHECK_SIG(drawable.gd, "\xa0\xc9\x71\x22\x96\x6a\x6c\x75\x1a\x2b\xdf\xf9\x37\x2b\xaf\xe5\xc5\x21\x10\xbb");

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector2.png");

    RED_CHECK_SIG(drawable.gd, "\x16\x86\x97\xdf\xa8\x00\x36\x02\xe1\x8e\xb8\x8d\xe5\x78\x18\x07\x3b\xfa\x2b\x91");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorResize)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(640, 480);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier,
                                "1", "1",  extra_button, 3, font, Theme(), Translation::EN);

    const char * add1[] = {"rdp", "qa\\administrateur@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add1);

    const char * add2[] = {"rdp", "administrateur@qa@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add2);

    const char * add3[] = {"rdp", "administrateur@qa@10.10.14.27",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add3);

    const char * add4[] = {"rdp", "administrateur@qa@10.10.14.103",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add4);

    const char * add5[] = {"rdp", "administrateur@qa@10.10.14.33",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize1.png");

    RED_CHECK_SIG(drawable.gd, "\x02\x9e\x7c\xf9\x07\x53\x65\x71\x04\x36\x86\x20\x58\x40\xb1\xf4\x0b\xbd\xbd\xf6");


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector-resize2.png");

    RED_CHECK_SIG(drawable.gd, "\x75\x26\x59\xbd\x59\xe8\x22\x21\xcc\xb3\x43\x00\x62\x74\x2f\x6e\x69\x7e\x69\x7c");
}

RED_AUTO_TEST_CASE(TraceWidgetSelector2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelector is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, 3, font,  Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector3.png");

    RED_CHECK_SIG(drawable.gd, "\x7f\xcc\x04\x8e\x44\x5c\xe2\x11\xa5\x40\x3f\x1a\xfb\x21\xe7\xd7\x3d\x3a\xc4\xaa");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelector is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, 3, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      0 + selector.y(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector4.png");

    RED_CHECK_SIG(drawable.gd, "\xc0\xe4\x4e\x72\xb1\x97\x00\x1a\xea\x33\x23\xc0\xe3\x3d\xf2\x25\x3d\x7c\xcb\x0d");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1",  extra_button, 3, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      5 + selector.y(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector5.png");

    RED_CHECK_SIG(drawable.gd, "\x23\xea\xe9\x93\x5f\xe1\x7d\x1c\x73\x71\x45\x56\xe2\xde\x4b\xff\x0a\x27\x38\xd6");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorEventSelect)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, 3, font, Theme(), Translation::EN);

    const char * add1[] = {"rdp", "qa\\administrateur@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add1);

    const char * add2[] = {"rdp", "administrateur@qa@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add2);

    const char * add3[] = {"rdp", "administrateur@qa@10.10.14.27",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add3);

    const char * add4[] = {"rdp", "administrateur@qa@10.10.14.103",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add4);

    const char * add5[] = {"rdp", "administrateur@qa@10.10.14.33",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add5);

    selector.set_widget_focus(&selector.selector_lines, Widget::focus_reason_tabkey);
    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    selector.selector_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                            selector.selector_lines.x() + 20,
                                            selector.selector_lines.y() + 40,
                                            nullptr);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-1.png");

    RED_CHECK_SIG(drawable.gd, "\xd1\x00\xc5\x77\xec\x82\x12\x0d\x06\xfd\x37\xdf\x82\xad\x08\x85\xeb\x05\xff\x21");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-2.png");


//     RED_CHECK_SIG(drawable.gd, "\xac\x89\x98\x5b\xe1\x08\x1c\xab\xf6\x9f\x20\x26\xb5\xfa\x07\x57\x1b\x1a\x7c\xfe");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-3.png");


    RED_CHECK_SIG(drawable.gd, "\x37\x79\x54\xd3\xea\x21\xa4\xc0\x26\x88\xa7\xe3\xa4\xa4\xb6\x77\x1c\xb2\xe5\xb2");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-4.png");


    RED_CHECK_SIG(drawable.gd, "\xac\x89\x98\x5b\xe1\x08\x1c\xab\xf6\x9f\x20\x26\xb5\xfa\x07\x57\x1b\x1a\x7c\xfe");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-5.png");


    RED_CHECK_SIG(drawable.gd, "\xd1\x00\xc5\x77\xec\x82\x12\x0d\x06\xfd\x37\xdf\x82\xad\x08\x85\xeb\x05\xff\x21");


    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector6-6.png");


    RED_CHECK_SIG(drawable.gd, "\xac\x89\x98\x5b\xe1\x08\x1c\xab\xf6\x9f\x20\x26\xb5\xfa\x07\x57\x1b\x1a\x7c\xfe");

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

RED_AUTO_TEST_CASE(TraceWidgetSelectorFilter)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, 3, font, Theme(), Translation::EN);

    const char * add1[] = {"reptile", "snake@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add1);

    const char * add2[] = {"bird", "raven@10.10.14.111",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add2);

    const char * add3[] = {"reptile", "lezard@10.10.14.27",
                           "VNC", "", "", "", "", "", "", ""};
    selector.add_device(add3);

    const char * add4[] = {"fish", "shark@10.10.14.103",
                           "RDP", "", "", "", "", "", "", ""};
    selector.add_device(add4);

    const char * add5[] = {"bird", "eagle@10.10.14.33",
                           "VNC", "", "", "", "", "", "", ""};
    selector.add_device(add5);

    int curx = 0;
    int cury = 0;

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    selector.selector_lines.set_selection(0);

    curx = selector.edit_filter[0]->x() + 2;
    cury = selector.edit_filter[0]->y() + 2;
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             curx, cury,
                             nullptr);
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             curx, cury,
                             nullptr);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-1.png");

    RED_CHECK_SIG(drawable.gd, "\x2f\xde\xcc\xa9\x8d\xf3\x5b\x44\x18\x4e\xaf\x0f\x11\xb7\xdd\x10\x15\x4a\x32\x7e");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-2.png");


    RED_CHECK_SIG(drawable.gd, "\xf8\xd9\x4e\x69\xf5\xf1\x0c\x9a\x94\xb3\xef\x9d\xb0\x2a\xd9\x2c\x2e\xc1\x9a\x30");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-3.png");

    RED_CHECK_SIG(drawable.gd, "\xa1\x80\xb3\x77\xc9\x66\xd1\xcf\x45\x44\x96\x1d\x70\x5a\xb5\xdd\x29\x9e\xc5\x43");
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-4.png");

    RED_CHECK_SIG(drawable.gd, "\xbc\xcc\x11\xfe\x7a\xd2\x12\x43\xa9\xae\x72\x0e\x4d\x0d\xf2\x59\x9f\x6b\x89\xa0");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-5.png");

    RED_CHECK_SIG(drawable.gd, "\x98\x62\xe4\x6b\xe9\x28\xca\x4e\xbe\x0b\x85\x97\xb2\x3a\x49\x6a\xb6\xd0\xa5\x57");


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-6.png");

    RED_CHECK_SIG(drawable.gd, "\x27\xe2\x1a\x6b\x7d\x19\x43\x9b\x53\x9f\xf9\xf9\x3a\xe5\x8f\xe6\x6d\x4c\xec\x69");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-7.png");

    RED_CHECK_SIG(drawable.gd, "\x1d\xbc\x27\x87\xd8\x35\xe5\xbd\xdb\x57\xb9\x03\x48\x30\x5a\x3d\x13\xeb\x8b\xde");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-8.png");

    RED_CHECK_SIG(drawable.gd, "\x9f\xec\xf5\x04\xc9\xb6\x65\xf8\x75\xd8\x83\x6f\xa0\xda\xab\xcb\x51\x2e\x07\xe5");

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

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "selector7-9.png");

    RED_CHECK_SIG(drawable.gd, "\x0c\x88\x50\x25\x4a\xe0\x1a\xe8\x24\xdc\x5b\xde\x43\xa6\x21\x0a\x5e\xbb\x08\xba");
}

// RED_AUTO_TEST_CASE(TraceWidgetSelectorAdjustColumns)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable.gd, 800, 600);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Font font;
//     // ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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

//     curx = selector.filter_device.x() + 2;
//     cury = selector.filter_device.y() + 2;
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
//     }
// }

// RED_AUTO_TEST_CASE(TraceWidgetSelectorAdjustColumns2)
// {
//     TestDraw drawable(640, 480);

//     // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable.gd, 640, 480);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Font font;
//     // ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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

//     curx = selector.filter_device.x() + 2;
//     cury = selector.filter_device.y() + 2;
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceWidgetSelectorAdjustColumns3)
// {
//     TestDraw drawable(1280, 1024);

//     // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
//     WidgetScreen parent(drawable.gd, 1280, 1024);
//     NotifyApi * notifier = nullptr;
//     int16_t w = drawable.gd.width();
//     int16_t h = drawable.gd.height();

//     Font font;
//     // ini.set<cfg::translation::target>_from_cstr("Target");
//     WidgetFlatButton * extra_button = nullptr;

//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, notifier, "1", "1", nullptr, nullptr, nullptr, extra_button, font, Theme(), Translation::EN);

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

//     curx = selector.filter_device.x() + 2;
//     cury = selector.filter_device.y() + 2;
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
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
//         RED_CHECK_MESSAGE(false, message);
//     }

// }

// RED_AUTO_TEST_CASE(TraceWidgetSelectorDescFieldVV)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelector is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable.gd, 800, 600);
//     struct Notify : NotifyApi {
//         Widget* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget* sender, notify_event_t event)
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

//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_VISIBLE | COMMENT_VISIBLE);

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

//     selector.set_widget_focus(&selector.selector_lines, Widget::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     // selector.selector_lines.set_selection(1);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == nullptr);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     RED_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     RED_CHECK(notifier.sender == &selector.connect);
// }

// RED_AUTO_TEST_CASE(TraceWidgetSelectorDescFieldVM)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelector is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable.gd, 800, 600);
//     struct Notify : NotifyApi {
//         Widget* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget* sender, notify_event_t event)
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
//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_VISIBLE | COMMENT_MANDATORY);

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

//     selector.set_widget_focus(&selector.selector_lines, Widget::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     // selector.selector_lines.set_selection(1);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == nullptr);

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == nullptr);
//     RED_CHECK(selector.current_focus == &selector.comment_edit);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == nullptr);
//     RED_CHECK(selector.current_focus == &selector.comment_edit);

//     selector.comment_edit.set_text("I would like to log on please !");

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     RED_CHECK(notifier.sender == &selector.connect);
// }

// RED_AUTO_TEST_CASE(TraceWidgetSelectorDescFieldMV)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelector is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable.gd, 800, 600);
//     struct Notify : NotifyApi {
//         Widget* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget* sender, notify_event_t event)
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
//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_MANDATORY | COMMENT_VISIBLE);

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

//     selector.set_widget_focus(&selector.selector_lines, Widget::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == nullptr);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == 0);
//     RED_CHECK(selector.current_focus == &selector.ticket_edit);

//     keymap.push_kevent(Keymap2::KEVENT_TAB);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);
//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == 0);
//     RED_CHECK(selector.current_focus == &selector.comment_edit);
//     selector.comment_edit.set_text("I would like to log on please !");
//     notifier.event = 0;
//     notifier.sender = 0;
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
//                              selector.connect.x() + 2,
//                              selector.connect.y() + 2,
//                              nullptr);
//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == 0);
//     RED_CHECK(selector.current_focus == &selector.connect);
//     notifier.event = 0;
//     notifier.sender = 0;
//     selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
//                              selector.connect.x() + 2,
//                              selector.connect.y() + 2,
//                              nullptr);
//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == 0);
//     RED_CHECK(selector.current_focus == &selector.ticket_edit);


//     selector.ticket_edit.set_text("18752");
//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     RED_CHECK(notifier.sender == &selector.connect);
// }


// RED_AUTO_TEST_CASE(TraceWidgetSelectorDescFieldMM)
// {
//     TestDraw drawable(800, 600);

//     // WidgetSelector is a selector widget at position 0,0 in it's parent context
//     WidgetScreen parent(drawable.gd, 800, 600);
//     struct Notify : NotifyApi {
//         Widget* sender;
//         notify_event_t event;

//         Notify()
//         : sender(0)
//         , event(0)
//         {}

//         virtual void notify(Widget* sender, notify_event_t event)
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
//     WidgetSelector selector(drawable.gd, "x@127.0.0.1", w, h, parent, &notifier, "1", "1", 0, 0, 0, extra_button, ini, TICKET_MANDATORY | COMMENT_MANDATORY);

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

//     selector.set_widget_focus(&selector.selector_lines, Widget::focus_reason_tabkey);
//     selector.selector_lines.set_selection(0);

//     Keymap2 keymap;
//     keymap.init_layout(0x040C);
//     keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);


//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == nullptr);


//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == 0);
//     RED_CHECK(selector.current_focus == &selector.ticket_edit);

//     selector.ticket_edit.set_text("18752");

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, 0);
//     RED_CHECK(notifier.sender == 0);
//     RED_CHECK(selector.current_focus == &selector.comment_edit);
//     selector.comment_edit.set_text("I would like to log on please !");

//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     selector.rdp_input_scancode(0,0,0,0, &keymap);

//     RED_CHECK_EQUAL(notifier.event, NOTIFY_SUBMIT);
//     RED_CHECK(notifier.sender == &selector.connect);
// }
