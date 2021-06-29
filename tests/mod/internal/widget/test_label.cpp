/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/widget_receive_event.hpp"

#include <string_view>


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/label/"


RED_AUTO_TEST_CASE(TraceWidgetLabel)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetLabel wlabel(drawable, parent, notifier, "test1", id, fg_color, bg_color, global_font_lato_light_16(), xtext, ytext);

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel2)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    WidgetLabel wlabel(drawable, parent, notifier, "test2", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel3)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    WidgetLabel wlabel(drawable, parent, notifier, "test3", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel4)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    WidgetLabel wlabel(drawable, parent, notifier, "test4", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel5)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, parent, notifier, "test5", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabel6)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, parent, notifier, "test6", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, parent, notifier, "test6", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.x(),
                                     wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelClip2)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable, parent, notifier, "test6", id, fg_color, bg_color, global_font_lato_light_16());

    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.x(),
                                     5 + wlabel.y(),
                                     30,
                                     10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLabelEvent)
{
    TestGraphic drawable(800, 600);

    WidgetReceiveEvent widget_for_receive_event(drawable);

    Widget& parent = widget_for_receive_event;
    NotifyApi * notifier = nullptr;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable, parent, notifier, "", 0, BLACK, WHITE, global_font_lato_light_16());
    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);

    wlabel.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 0, 0);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
    wlabel.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x10)); // 'a'
    wlabel.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x10), 0, keymap);
    RED_CHECK(widget_for_receive_event.last_widget == nullptr);
    RED_CHECK(widget_for_receive_event.last_event == 0);
}

RED_AUTO_TEST_CASE(TraceWidgetLabelAndComposite)
{
    TestGraphic drawable(800, 600);

    //WidgetLabel is a label widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetLabel wlabel1(drawable, wcomposite, notifier,
                        "abababab", 0, YELLOW, BLACK, global_font_lato_light_16());
    Dimension dim = wlabel1.get_optimal_dim();
    wlabel1.set_wh(dim);
    wlabel1.set_xy(0, 0);

    WidgetLabel wlabel2(drawable, wcomposite, notifier,
                        "ggghdgh", 0, WHITE, BLUE, global_font_lato_light_16());
    dim = wlabel2.get_optimal_dim();
    wlabel2.set_wh(dim);
    wlabel2.set_xy(0, 100);

    WidgetLabel wlabel3(drawable, wcomposite, notifier,
                        "lldlslql", 0, BLUE, RED, global_font_lato_light_16());
    dim = wlabel3.get_optimal_dim();
    wlabel3.set_wh(dim);
    wlabel3.set_xy(100, 100);

    WidgetLabel wlabel4(drawable, wcomposite, notifier,
                        "LLLLMLLM", 0, PINK, DARK_GREEN, global_font_lato_light_16());
    dim = wlabel4.get_optimal_dim();
    wlabel4.set_wh(dim);
    wlabel4.set_xy(300, 300);

    WidgetLabel wlabel5(drawable, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, global_font_lato_light_16());
    dim = wlabel5.get_optimal_dim();
    wlabel5.set_wh(dim);
    wlabel5.set_xy(700, -10);

    WidgetLabel wlabel6(drawable, wcomposite, notifier,
                        "xxwwp", 0, ANTHRACITE, PALE_GREEN, global_font_lato_light_16());
    dim = wlabel6.get_optimal_dim();
    wlabel6.set_wh(dim);
    wlabel6.set_xy(-10, 550);

    wcomposite.add_widget(&wlabel1);
    wcomposite.add_widget(&wlabel2);
    wcomposite.add_widget(&wlabel3);
    wcomposite.add_widget(&wlabel4);
    wcomposite.add_widget(&wlabel5);
    wcomposite.add_widget(&wlabel6);

    //ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_9.png");

    //ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_10.png");

    wcomposite.clear();
}

/* TODO
 * the entry point exists in module: it's rdp_input_invalidate
 * je just have to change received values to widget messages */
RED_AUTO_TEST_CASE(TraceWidgetLabelMax)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    auto text =
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"
        "éàéàéàéàéàéàéàéàéàéàéàéàéàéàéàéà"_av;

    WidgetLabel wlabel(drawable, parent, notifier, text.data(),
        id, fg_color, bg_color, global_font_lato_light_16());
    Dimension dim = wlabel.get_optimal_dim();
    wlabel.set_wh(dim);
    wlabel.set_xy(x, y);


    RED_CHECK(std::string_view(wlabel.get_text()) == text.subarray(0, text.size()-2));

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.x(),
                                     0 + wlabel.y(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "label_11.png");
}
