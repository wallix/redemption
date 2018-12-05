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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/internal/widget/selector.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av, "RDP"_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av, "RDP"_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av, "RDP"_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av, "RDP"_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av, "RDP"_av};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector1.png");

    RED_CHECK_SIG(drawable, "\x50\x25\xe5\x03\x76\x81\x53\xfc\x6e\x22\xd4\xf8\xfb\xf1\x70\x57\x4d\xb5\x16\x45");

    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector2.png");

    RED_CHECK_SIG(drawable, "\xa7\x47\xcc\x6f\x5c\x52\x40\x92\x70\x30\x6c\x75\xa0\xd7\xf8\x3b\x70\xe5\x45\x00");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorResize)
{
    TestGraphic drawable(640, 480);


    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(640, 480);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier,
                                "1", "1",  extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add5);

    selector.selector_lines.set_selection(0);

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector-resize1.png");

    RED_CHECK_SIG(drawable, "\xbb\x51\x75\x39\x8a\x7f\xad\x40\xdf\xd0\xc5\xc7\xac\x20\x2b\xa0\x89\x66\xdc\x90");


    selector.selector_lines.set_selection(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector-resize2.png");

    RED_CHECK_SIG(drawable, "\x6c\x77\x75\x9f\x0a\xf0\x98\x35\xa7\xfd\xf2\xf7\x37\xc5\xe7\x56\xb7\xf5\x23\x30");
}

RED_AUTO_TEST_CASE(TraceWidgetSelector2)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(),  Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector3.png");

    RED_CHECK_SIG(drawable, "\x84\x8f\x42\x84\x07\x86\x34\x90\x65\x3a\xd4\x5f\x72\xb9\xe9\xeb\x3b\x4a\xb5\x25");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      0 + selector.y(),
                                      selector.cx(),
                                      selector.cy()));

    // drawable.save_to_png("selector4.png");

    RED_CHECK_SIG(drawable, "\x68\x1e\x1c\x20\xb1\x2a\x68\x59\x45\xc1\x41\x0c\x03\x0d\x3e\xc9\xf3\xce\x22\x40");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorClip2)
{
    TestGraphic drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1",  extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.x(),
                                      5 + selector.y(),
                                      30,
                                      10));

    // drawable.save_to_png("selector5.png");

    RED_CHECK_SIG(drawable, "\x9d\xbe\x64\x88\x34\x4b\x79\x83\x6a\xa1\x36\xad\xf4\x9e\xe6\x89\xb8\xf3\x86\x87");
}

RED_AUTO_TEST_CASE(TraceWidgetSelectorEventSelect)
{
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";

    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "rdp"_av, "qa\\administrateur@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "rdp"_av, "administrateur@qa@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "rdp"_av, "administrateur@qa@10.10.14.27"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "rdp"_av, "administrateur@qa@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
        "rdp"_av, "administrateur@qa@10.10.14.33"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
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

    RED_CHECK_SIG(drawable, "\x72\x58\x16\x04\xe4\x09\xb6\x58\x40\x70\x62\xe1\xf8\xd7\xf6\xe4\x02\xd9\xd0\x96");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-2.png");


//     RED_CHECK_SIG(drawable, "\xac\x89\x98\x5b\xe1\x08\x1c\xab\xf6\x9f\x20\x26\xb5\xfa\x07\x57\x1b\x1a\x7c\xfe");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-3.png");


    RED_CHECK_SIG(drawable, "\x72\xf6\x86\x9a\xf8\x1c\x62\xae\xce\x8e\x40\x26\x9d\x4a\x65\xac\x7e\x13\x75\x52");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-4.png");


    RED_CHECK_SIG(drawable, "\x0a\x7a\xa1\x01\x97\xdb\x79\x24\x21\x49\x24\x61\xd6\x2b\x0f\xf7\xb4\x67\xdb\xd2");


    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-5.png");


    RED_CHECK_SIG(drawable, "\x72\x58\x16\x04\xe4\x09\xb6\x58\x40\x70\x62\xe1\xf8\xd7\xf6\xe4\x02\xd9\xd0\x96");


    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.selector_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector6-6.png");


    RED_CHECK_SIG(drawable, "\x0a\x7a\xa1\x01\x97\xdb\x79\x24\x21\x49\x24\x61\xd6\x2b\x0f\xf7\xb4\x67\xdb\xd2");

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
    TestGraphic drawable(800, 600);


    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int16_t w = drawable.width();
    int16_t h = drawable.height();
    WidgetFlatButton * extra_button = nullptr;
    WidgetSelectorParams params;
    params.nb_columns = 3;
    params.base_len[0] = 200;
    params.base_len[1] = 64000;
    params.base_len[2] = 80;
    params.label[0] = "Authorization";
    params.label[1] = "Target";
    params.label[2] = "Protocol";


    WidgetSelector selector(drawable, "x@127.0.0.1", 0, 0, w, h, parent, notifier, "1", "1", extra_button, params, global_font_deja_vu_14(), Theme(), Translation::EN);

    array_view_const_char const add1[] = {
        "reptile", "snake@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add1);

    array_view_const_char const add2[] = {
        "bird", "raven@10.10.14.111"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add2);

    array_view_const_char const add3[] = {
        "reptile", "lezard@10.10.14.27"_av,
        "VNC", ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add3);

    array_view_const_char const add4[] = {
        "fish", "shark@10.10.14.103"_av,
        "RDP"_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add4);

    array_view_const_char const add5[] = {
        "bird", "eagle@10.10.14.33"_av,
        "VNC", ""_av, ""_av, ""_av, ""_av, ""_av, ""_av, ""_av};
    selector.add_device(add5);

    int curx = 0;
    int cury = 0;

    selector.move_size_widget(selector.x(), selector.y(), selector.cx(), selector.cy());

    selector.selector_lines.set_selection(0);

    curx = selector.edit_filters[0].x() + 2;
    cury = selector.edit_filters[0].y() + 2;
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                             curx, cury,
                             nullptr);
    selector.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                             curx, cury,
                             nullptr);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-1.png");

    RED_CHECK_SIG(drawable, "\xcb\xa1\x9f\x4e\xb9\x42\x64\xa9\xa5\x9d\xcd\x06\xc2\x33\x56\xae\xab\xfa\x75\x65");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);

    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-2.png");


    RED_CHECK_SIG(drawable, "\x7d\x15\x56\x7d\xe4\x03\x75\xb5\xb7\x47\x7f\xb4\x24\x38\xed\xe6\x29\xe3\xd8\x81");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-3.png");

    RED_CHECK_SIG(drawable, "\x77\x74\x1e\x4e\xfa\x9e\xc1\x65\x8b\x53\x40\x08\xce\x5c\x32\xe8\x44\x00\x3d\x6b");
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-4.png");

    RED_CHECK_SIG(drawable, "\xce\xfd\x05\x29\xf5\xbc\x75\xb2\x06\xba\x1b\xe1\xbe\x6a\x00\xd3\x14\x2a\x16\x65");


    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-5.png");

    RED_CHECK_SIG(drawable, "\x7b\x90\xfa\xfc\x76\xd6\x9c\x2a\xba\xd7\x98\xc2\xbf\x59\xa2\xf0\x8b\x47\xdd\xb2");


    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-6.png");

    RED_CHECK_SIG(drawable, "\x03\xb4\xed\xd3\xd3\x6d\x38\x33\x96\x04\x39\x7d\xdf\xd4\xa0\xcd\xee\xd5\xd2\x46");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-7.png");

    RED_CHECK_SIG(drawable, "\xfe\x3b\x6e\x81\xba\x50\x46\xda\xe9\x67\x97\x6d\x59\x4d\x71\x54\x48\x01\xd5\xc5");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_TAB);
    selector.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.get_rect());

    // drawable.save_to_png("selector7-8.png");

    RED_CHECK_SIG(drawable, "\xfc\xaf\x04\x5d\x54\x8a\xe4\x93\x99\x1c\xef\xc0\xe8\x98\x69\x4b\xb8\xf6\xa6\x4f");

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

    RED_CHECK_SIG(drawable, "\x66\x8e\x41\x3b\xcd\xd7\xde\x26\x6e\x22\x93\xe2\x82\x42\x97\xf3\x22\xc4\xa9\x27");
}
