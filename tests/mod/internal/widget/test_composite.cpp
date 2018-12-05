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

#define RED_TEST_MODULE TestWidgetComposite
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/widget_rect.hpp"
#include "mod/internal/widget/composite.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"

class WidgetCompositeRect : public WidgetComposite
{
public:
    BGRColor color;

    WidgetCompositeRect(TestGraphic & drawable)
    : WidgetComposite(drawable, *this, nullptr)
    , color(0x27642F)
    {}

    BGRColor get_bg_color() const override {
        return this->color;
    }
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

    // drawable.save_to_png("/tmp/composite.png");

    RED_CHECK_SIG(drawable, "\x42\xfd\x22\x64\x4d\xb8\xea\xfd\xf8\x95\x7b\x4d\x51\x98\x9d\x67\xb5\xe6\x82\xba");

    // ask to widget to redraw at position 0,500 and of size 100x100
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.x(),
                                         500 + wcomposite.y(),
                                         100,
                                         100));

    //drawable.save_to_png("/tmp/composite2.png");

    RED_CHECK_SIG(drawable, "\xdf\x6a\xf5\x43\xba\x3f\xf7\xce\xeb\x2e\x8c\xe7\xa9\xf0\x3c\x1b\x78\x9f\x58\x20");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.x(),
                                         0 + wcomposite.y(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    //drawable.save_to_png("/tmp/composite3.png");

    RED_CHECK_SIG(drawable, "\xc8\x83\xe3\x4b\xe2\xd8\x39\x0c\xbe\x07\x93\x2f\x36\x24\x2e\xc1\x85\xae\x1a\x61");
    wcomposite.clear();
}
