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

#include "core/font.hpp"
#include "mod/internal/widget/selector.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"


RED_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, font, Theme(), Translation::EN);

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

    // drawable.save_to_png("selector1.png");

    RED_CHECK_SIG(drawable.gd, "\x50\x25\xe5\x03\x76\x81\x53\xfc\x6e\x22\xd4\xf8\xfb\xf1\x70\x57\x4d\xb5\x16\x45");

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector2.png");

    RED_CHECK_SIG(drawable.gd, "\xa7\x47\xcc\x6f\x5c\x52\x40\x92\x70\x30\x6c\x75\xa0\xd7\xf8\x3b\x70\xe5\x45\x00");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorResize)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(640, 480);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier,
                                "1", "1",  extra_button, params, font, Theme(), Translation::EN);

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

    // drawable.save_to_png("selector-resize1.png");

    RED_CHECK_SIG(drawable.gd, "\xbb\x51\x75\x39\x8a\x7f\xad\x40\xdf\xd0\xc5\xc7\xac\x20\x2b\xa0\x89\x66\xdc\x90");


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector-resize2.png");

    RED_CHECK_SIG(drawable.gd, "\x6c\x77\x75\x9f\x0a\xf0\x98\x35\xa7\xfd\xf2\xf7\x37\xc5\xe7\x56\xb7\xf5\x23\x30");
}

RED_AUTO_TEST_CASE(TraceWidgetSelector2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    // WidgetSelector is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, font,  Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector3.png");

    RED_CHECK_SIG(drawable.gd, "\x84\x8f\x42\x84\x07\x86\x34\x90\x65\x3a\xd4\x5f\x72\xb9\xe9\xeb\x3b\x4a\xb5\x25");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    // WidgetSelector is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      0 + selector.y(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png("selector4.png");

    RED_CHECK_SIG(drawable.gd, "\x68\x1e\x1c\x20\xb1\x2a\x68\x59\x45\xc1\x41\x0c\x03\x0d\x3e\xc9\xf3\xce\x22\x40");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1",  extra_button, params, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      5 + selector.y(),
                                      30,
                                      10));

    // drawable.save_to_png("selector5.png");

    RED_CHECK_SIG(drawable.gd, "\x9d\xbe\x64\x88\x34\x4b\x79\x83\x6a\xa1\x36\xad\xf4\x9e\xe6\x89\xb8\xf3\x86\x87");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorEventSelect)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, font, Theme(), Translation::EN);

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

    // drawable.save_to_png("selector6-1.png");

    RED_CHECK_SIG(drawable.gd, "\x72\x58\x16\x04\xe4\x09\xb6\x58\x40\x70\x62\xe1\xf8\xd7\xf6\xe4\x02\xd9\xd0\x96");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-2.png");


//     RED_CHECK_SIG(drawable.gd, "\xac\x89\x98\x5b\xe1\x08\x1c\xab\xf6\x9f\x20\x26\xb5\xfa\x07\x57\x1b\x1a\x7c\xfe");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-3.png");


    RED_CHECK_SIG(drawable.gd, "\x72\xf6\x86\x9a\xf8\x1c\x62\xae\xce\x8e\x40\x26\x9d\x4a\x65\xac\x7e\x13\x75\x52");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-4.png");


    RED_CHECK_SIG(drawable.gd, "\x0a\x7a\xa1\x01\x97\xdb\x79\x24\x21\x49\x24\x61\xd6\x2b\x0f\xf7\xb4\x67\xdb\xd2");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-5.png");


    RED_CHECK_SIG(drawable.gd, "\x72\x58\x16\x04\xe4\x09\xb6\x58\x40\x70\x62\xe1\xf8\xd7\xf6\xe4\x02\xd9\xd0\x96");


    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-6.png");


    RED_CHECK_SIG(drawable.gd, "\x0a\x7a\xa1\x01\x97\xdb\x79\x24\x21\x49\x24\x61\xd6\x2b\x0f\xf7\xb4\x67\xdb\xd2");

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

    Font font(FIXTURES_PATH "/dejavu_14.rbf");

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.gd.width();
    int16_t h = drawable.gd.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";


    WidgetSelector selector(drawable.gd, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, font, Theme(), Translation::EN);

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

    curx = selector.edit_filter[0].x() + 2;
    cury = selector.edit_filter[0].y() + 2;
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             curx, cury,
                             nullptr);
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             curx, cury,
                             nullptr);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-1.png");

    RED_CHECK_SIG(drawable.gd, "\xcb\xa1\x9f\x4e\xb9\x42\x64\xa9\xa5\x9d\xcd\x06\xc2\x33\x56\xae\xab\xfa\x75\x65");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-2.png");


    RED_CHECK_SIG(drawable.gd, "\x7d\x15\x56\x7d\xe4\x03\x75\xb5\xb7\x47\x7f\xb4\x24\x38\xed\xe6\x29\xe3\xd8\x81");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-3.png");

    RED_CHECK_SIG(drawable.gd, "\x77\x74\x1e\x4e\xfa\x9e\xc1\x65\x8b\x53\x40\x08\xce\x5c\x32\xe8\x44\x00\x3d\x6b");
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-4.png");

    RED_CHECK_SIG(drawable.gd, "\xce\xfd\x05\x29\xf5\xbc\x75\xb2\x06\xba\x1b\xe1\xbe\x6a\x00\xd3\x14\x2a\x16\x65");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-5.png");

    RED_CHECK_SIG(drawable.gd, "\x7b\x90\xfa\xfc\x76\xd6\x9c\x2a\xba\xd7\x98\xc2\xbf\x59\xa2\xf0\x8b\x47\xdd\xb2");


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-6.png");

    RED_CHECK_SIG(drawable.gd, "\x03\xb4\xed\xd3\xd3\x6d\x38\x33\x96\x04\x39\x7d\xdf\xd4\xa0\xcd\xee\xd5\xd2\x46");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-7.png");

    RED_CHECK_SIG(drawable.gd, "\xfe\x3b\x6e\x81\xba\x50\x46\xda\xe9\x67\x97\x6d\x59\x4d\x71\x54\x48\x01\xd5\xc5");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-8.png");

    RED_CHECK_SIG(drawable.gd, "\xfc\xaf\x04\x5d\x54\x8a\xe4\x93\x99\x1c\xef\xc0\xe8\x98\x69\x4b\xb8\xf6\xa6\x4f");

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

    // drawable.save_to_png("selector7-9.png");

    RED_CHECK_SIG(drawable.gd, "\x66\x8e\x41\x3b\xcd\xd7\xde\x26\x6e\x22\x93\xe2\x82\x42\x97\xf3\x22\xc4\xa9\x27");
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

//     // drawable.save_to_png("selector-adjust-1.png");

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

//     // drawable.save_to_png("selector-adjust-2.png");
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

//     // drawable.save_to_png("selector-adjust-3.png");
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

//     // drawable.save_to_png("selector-adjust-4.png");
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

//     // drawable.save_to_png("selector-adjust2-1.png");

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

//     // drawable.save_to_png("selector-adjust2-2.png");
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

//     // drawable.save_to_png("selector-adjust2-3.png");
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

//     // drawable.save_to_png("selector-adjust2-4.png");
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

//     // drawable.save_to_png("selector-adjust3-1.png");

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

//     // drawable.save_to_png("selector-adjust3-2.png");
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

//     // drawable.save_to_png("selector-adjust3-3.png");
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

//     // drawable.save_to_png("selector-adjust3-4.png");
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
