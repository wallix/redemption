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

#include "mod/internal/widget/widget_rect.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/rect/"

RED_AUTO_TEST_CASE(TraceWidgetRect)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 800x600 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; // unique identifier of widget used par parent, it will be sent back in case of event
    BGRColor color(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, color);
    wrect.set_wh(800, 600);
    wrect.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.x(),
                                    0 + wrect.y(),
                                    wrect.cx(),
                                    wrect.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRect2)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position -100,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(-100, -100);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.x(),
                                    0 + wrect.y(),
                                    wrect.cx(),
                                    wrect.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRect3)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position -100,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(-100, 500);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.x(),
                                    0 + wrect.y(),
                                    wrect.cx(),
                                    wrect.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRect4)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(700, 500);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.x(),
                                    0 + wrect.y(),
                                    wrect.cx(),
                                    wrect.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRect5)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(700, -100);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.x(),
                                    0 + wrect.y(),
                                    wrect.cx(),
                                    wrect.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRect6)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 300,200 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(300, 200);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.x(),
                                    0 + wrect.y(),
                                    wrect.cx(),
                                    wrect.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRectClip)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 300,200 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(300, 200);

    // ask to widget to redraw at position 400,300 and of size 100x100. After clip the size is of 100x50
    wrect.rdp_input_invalidate(Rect(150 + wrect.x(),
                                    100 + wrect.y(),
                                    100,
                                    100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetRectClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    BGRColor bgcolor(0xCCF604); /* BGR */

    WidgetRect wrect(drawable, parent, notifier, id, bgcolor);
    wrect.set_wh(200, 200);
    wrect.set_xy(700, -100);


    // ask to widget to redraw at position 720,20 and of size 50x50
    wrect.rdp_input_invalidate(Rect(20 + wrect.x(),
                                    120 + wrect.y(),
                                    50,
                                    50));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "rect_8.png");
}

/* TODO
 * As soon as composite widgets will be available, we will have to check these tests
 * are still working with two combination layers (conversion of coordinates
 * from parent coordinates to screen_coordinates can be tricky)")
 */
