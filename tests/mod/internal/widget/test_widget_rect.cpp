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

#define RED_TEST_MODULE TestWidgetRect
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/widget_rect.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetRect)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 800x600 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect.png");

    RED_CHECK_SIG(drawable, "\xea\xe1\x3b\x4b\xdb\xda\xa6\x75\xf1\x17\xa2\xe8\x09\xf1\xd2\x42\x7a\xdf\x85\x6d");
}

RED_AUTO_TEST_CASE(TraceWidgetRect2)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position -100,-100 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect2.png");

    RED_CHECK_SIG(drawable, "\x7c\x96\x36\xc6\x5a\x1e\x29\xb4\xd7\x4a\x31\x64\x37\xec\x94\x5f\x7a\x3c\x4a\x52");
}

RED_AUTO_TEST_CASE(TraceWidgetRect3)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position -100,500 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect3.png");

    RED_CHECK_SIG(drawable, "\x00\x57\x28\x73\x89\x49\xd5\x9e\xc0\xc1\x77\xc9\xc5\x7b\x5e\x13\x88\xf0\xf6\x33");
}

RED_AUTO_TEST_CASE(TraceWidgetRect4)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,500 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect4.png");

    RED_CHECK_SIG(drawable, "\xc8\x60\xbd\xc0\xe3\x38\x4a\xe5\xd3\x29\x52\x7d\xf6\x9b\x3e\x83\x97\xf0\xbc\x90");
}

RED_AUTO_TEST_CASE(TraceWidgetRect5)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect5.png");

    RED_CHECK_SIG(drawable, "\x9c\xbe\xee\x0d\xd5\xa6\x50\xfb\x99\x4b\x2d\xae\xd9\xcc\x33\x65\x6f\xc1\x5e\x1e");
}

RED_AUTO_TEST_CASE(TraceWidgetRect6)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 300,200 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect6.png");

    RED_CHECK_SIG(drawable, "\x0a\x0f\xb8\xff\x34\x91\xe5\xd0\x60\x52\x56\xcb\x3a\x56\x37\x21\xe8\xc4\x22\x19");
}

RED_AUTO_TEST_CASE(TraceWidgetRectClip)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 300,200 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect7.png");

    RED_CHECK_SIG(drawable, "\x91\xe9\x85\x69\x98\x49\xea\x17\xe6\xc8\xaf\x9b\x78\x73\x71\x65\xdd\x67\x4b\xe3");
}

RED_AUTO_TEST_CASE(TraceWidgetRectClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, global_font(), nullptr, Theme{});
    parent.set_wh(800, 600);

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

    //drawable.save_to_png(OUTPUT_FILE_PATH "rect8.png");

    RED_CHECK_SIG(drawable, "\xe9\x67\x5a\xe9\xa3\xa2\xca\xf3\xbe\xc2\x37\xff\xa0\xff\x20\x3d\x8e\x28\x37\xc8");
}

/* TODO
 * As soon as composite widgets will be available, we will have to check these tests
 * are still working with two combination layers (conversion of coordinates
 * from parent coordinates to screen_coordinates can be tricky)")
 */
