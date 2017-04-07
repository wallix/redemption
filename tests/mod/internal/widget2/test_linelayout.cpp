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

#define UNIT_TEST_MODULE TestLineLayout
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/columnlayout.hpp"
#include "mod/internal/widget2/linelayout.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "mod/internal/widget2/stacklayout.hpp"
#include "mod/internal/widget2/label.hpp"
#include "mod/internal/widget2/flat_button.hpp"
#include "fake_draw.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"


RED_AUTO_TEST_CASE(TestLineLayoutwithColumns)
{

    TestDraw drawable(800, 600);

    // WidgetButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);
    NotifyApi * notifier = nullptr;
    int id = 0;

    WidgetColumnLayout wcolumn(drawable.gd, 50, 60, parent, notifier, id);


    WidgetLineLayout wline1(drawable.gd, Rect(0, 0, 1, 1), wcolumn, &wcolumn, id);

    WidgetLabel label11(drawable.gd, 1, 1, wline1, notifier, "target_group", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    WidgetLabel label12(drawable.gd, 1, 1, wline1, notifier, "target", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    WidgetLabel label13(drawable.gd, 1, 1, wline1, notifier, "protocol", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    label11.rect.cx = 120;
    label12.rect.cx = 400;
    label13.rect.cx = 80;
    wline1.add_widget(&label11);
    wline1.add_widget(&label12);
    wline1.add_widget(&label13);
    wcolumn.add_widget(&wline1);

    WidgetLineLayout wline2(drawable.gd, Rect(0, 0, 1, 1), wcolumn, &wcolumn, id);
    WidgetLabel label21(drawable.gd, 1, 1, wline2, notifier, "wingroup", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    WidgetLabel label22(drawable.gd, 1, 1, wline2, notifier, "tartempion@wallix.fr", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    WidgetLabel label23(drawable.gd, 1, 1, wline2, notifier, "RDP", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    label21.rect.cx = 120;
    label22.rect.cx = 400;
    label23.rect.cx = 80;
    wline2.add_widget(&label21);
    wline2.add_widget(&label22);
    wline2.add_widget(&label23);
    wcolumn.add_widget(&wline2);

    WidgetLineLayout wline3(drawable.gd, Rect(0, 0, 1, 1), wcolumn, &wcolumn, id);
    WidgetLabel label31(drawable.gd, 1, 1, wline3, notifier, "anothergroup", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    WidgetLabel label32(drawable.gd, 1, 1, wline3, notifier, "jambon@eau", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    WidgetLabel label33(drawable.gd, 1, 1, wline3, notifier, "WTF", true,
                       0, 0x000000, LIGHT_BLUE, 2, 2);
    label31.rect.cx = 120;
    label32.rect.cx = 400;
    label33.rect.cx = 80;
    wline3.add_widget(&label31);
    wline3.add_widget(&label32);
    wline3.add_widget(&label33);
    wcolumn.add_widget(&wline3);


    WidgetLineLayout wline4(drawable.gd, Rect(0, 0, 1, 1), wcolumn, &wcolumn, id);
    WidgetLabel label41(drawable.gd, 1, 1, wline4, notifier, "wingroup", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    WidgetLabel label42(drawable.gd, 1, 1, wline4, notifier, "tartempion@wallix.fr", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    WidgetLabel label43(drawable.gd, 1, 1, wline4, notifier, "RDP", true,
                       0, 0x000000, MEDIUM_BLUE, 2, 2);
    label41.rect.cx = 120;
    label42.rect.cx = 400;
    label43.rect.cx = 80;
    wline4.add_widget(&label41);
    wline4.add_widget(&label42);
    wline4.add_widget(&label43);
    wcolumn.add_widget(&wline4);

    wcolumn.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-columnline.png");

    WidgetFlatButton button(drawable.gd, 0, 0, wcolumn, &wcolumn, "un boutton", true, id,
                            WHITE, DARK_BLUE, DARK_BLUE);

    wcolumn.insert_widget(&button, 2);

    wcolumn.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-columnline2.png");

    wcolumn.remove_widget(&wline3);

    wcolumn.rdp_input_invalidate(parent.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "layout-columnline3.png");
}
