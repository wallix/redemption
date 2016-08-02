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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestColumnLayout
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/columnlayout.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "mod/internal/widget2/label.hpp"
#include "fake_draw.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

BOOST_AUTO_TEST_CASE(TraceWidgetColumn)
{
    TestDraw drawable(800, 600);


    // WidgetButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);
    NotifyApi * notifier = nullptr;
    int id = 0;

    WidgetColumnLayout wcolumn(drawable.gd, 50, 60, parent, notifier, id);

    WidgetLabel label1(drawable.gd, 1, 1, wcolumn, notifier, "premier widget", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    WidgetLabel label2(drawable.gd, 1, 1, wcolumn, notifier, "deuxieme widget", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    WidgetLabel label3(drawable.gd, 1, 1, wcolumn, notifier, "troisieme widget", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    WidgetLabel label4(drawable.gd, 1, 1, wcolumn, notifier, "quatrieme widget", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    label1.rect.cx = 500;
    label2.rect.cx = 500;
    label3.rect.cx = 500;
    label4.rect.cx = 500;

    wcolumn.add_widget(&label1);
    wcolumn.add_widget(&label2);
    wcolumn.add_widget(&label3);
    wcolumn.add_widget(&label4);

    // ask to widget to redraw at it's current position
    wcolumn.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-column.png");

    label2.rect.cy = 130;
    label3.rect.cy = 50;
    wcolumn.rearrange(2);

    wcolumn.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-column2.png");

    // char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x7d\xfe\xb4\x41\x31\x06\x68\xe8\xbb\x75"
    //     "\x8c\x35\x11\x19\x97\x2a\x16\x0f\x65\x28")){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
}
