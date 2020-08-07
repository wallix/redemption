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
#include "mod/internal/widget/composite.hpp"
#include "test_only/gdi/test_graphic.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/composite/"

class WidgetCompositeRect : public WidgetComposite
{
public:
    WidgetCompositeRect(TestGraphic & drawable)
    : WidgetComposite(drawable, *this, nullptr)
    {}
};

RED_AUTO_TEST_CASE(TraceWidgetComposite)
{
    TestGraphic drawable(800, 600);
    NotifyApi * notifier = nullptr;
    int id = 0;

    WidgetCompositeRect wcomposite(drawable);
    wcomposite.set_wh(drawable.width(),
                      drawable.height());
    wcomposite.set_xy(0, 0);

    WidgetRect wrect1(drawable,
                      wcomposite, notifier, id++, CYAN);
    wrect1.set_wh(100, 100);
    wrect1.set_xy(0, 0);

    WidgetRect wrect2(drawable,
                      wcomposite, notifier, id++, RED);
    wrect2.set_wh(100, 100);
    wrect2.set_xy(0, 100);

    WidgetRect wrect3(drawable,
                      wcomposite, notifier, id++, BLUE);
    wrect3.set_wh(100, 100);
    wrect3.set_xy(100, 100);

    WidgetRect wrect4(drawable,
                      wcomposite, notifier, id++, GREEN);
    wrect4.set_wh(100, 100);
    wrect4.set_xy(300, 300);

    WidgetRect wrect5(drawable,
                      wcomposite, notifier, id++, WHITE);
    wrect5.set_wh(100, 100);
    wrect5.set_xy(700, -50);

    WidgetRect wrect6(drawable,
                      wcomposite, notifier, id++, GREY);
    wrect6.set_wh(100, 100);
    wrect6.set_xy(-50, 550);

    wcomposite.add_widget(&wrect1);
    wcomposite.add_widget(&wrect2);
    wcomposite.add_widget(&wrect3);
    wcomposite.add_widget(&wrect4);
    wcomposite.add_widget(&wrect5);
    wcomposite.add_widget(&wrect6);

    {
        WidgetRect wrect7(drawable,
                          wcomposite, notifier, id++, GREY);
        wrect7.set_wh(800, 800);
        wrect7.set_xy(0, 0);

        wcomposite.add_widget(&wrect7);
        wcomposite.remove_widget(&wrect7);
    }

    // ask to widget to redraw at position 150,500 and of size 800x600
    wcomposite.rdp_input_invalidate(Rect(150 + wcomposite.x(),
                                         150 + wcomposite.y(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "composite_1.png");

    // ask to widget to redraw at position 0,500 and of size 100x100
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.x(),
                                         500 + wcomposite.y(),
                                         100,
                                         100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "composite_2.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.x(),
                                         0 + wcomposite.y(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "composite_3.png");

    wcomposite.clear();
}
