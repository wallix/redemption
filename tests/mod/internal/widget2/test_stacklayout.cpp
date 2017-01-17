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
 *   Author(s): Christophe Grosjean, Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestStacklayout
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/linelayout.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "mod/internal/widget2/stacklayout.hpp"
#include "mod/internal/widget2/widget2_rect.hpp"
#include "fake_draw.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

BOOST_AUTO_TEST_CASE(Testtab)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);
    NotifyApi * notifier = nullptr;
    int id = 0;

    WidgetStackLayout stack(drawable.gd, Rect(30, 20, 500, 400), parent, notifier, id);
    WidgetLineLayout control(drawable.gd, Rect(0,0,1,1), parent, &stack, id);

    WidgetRect page0(drawable.gd, Rect(0, 0, 300, 300), stack, &parent, id, BLUE);
    WidgetRect page1(drawable.gd, Rect(0, 0, 250, 300), stack, &parent, id, RED);
    WidgetRect page2(drawable.gd, Rect(0, 0, 300, 350), stack, &parent, id, GREEN);
    stack.add_widget(&page0);
    stack.add_widget(&page1);
    stack.add_widget(&page2);

    stack.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-stack1.png");

    control.notify_value = 2;
    control.send_notify(NOTIFY_SELECTION_CHANGED);

    stack.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-stack2.png");

    control.notify_value = 1;
    control.send_notify(NOTIFY_SELECTION_CHANGED);

    stack.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-stack3.png");
}
