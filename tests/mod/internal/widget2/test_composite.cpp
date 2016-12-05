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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetComposite
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/widget2_rect.hpp"
#include "mod/internal/widget2/composite.hpp"
#include "check_sig.hpp"
#include "fake_draw.hpp"

class WidgetCompositeRect : public WidgetComposite
{
public:
    int color;

    WidgetCompositeRect(TestDraw & drawable)
    : WidgetComposite(drawable.gd, Rect(0, 0,
                                      drawable.gd.width(),
                                      drawable.gd.height()),
                      *this, nullptr)
    , color(0x27642F)
    {}

    void draw(const Rect& clip) override
    {
        this->drawable.draw(RDPOpaqueRect(clip, color), this->get_rect());
        this->WidgetComposite::draw(clip);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetComposite)
{
    TestDraw drawable(800, 600);
    NotifyApi * notifier = nullptr;
    int id = 0;

    WidgetCompositeRect wcomposite(drawable);
    WidgetRect wrect1(drawable.gd, Rect(0,0,100,100),
                      wcomposite, notifier, id++, CYAN);
    WidgetRect wrect2(drawable.gd, Rect(0,100,100,100),
                      wcomposite, notifier, id++, RED);
    WidgetRect wrect3(drawable.gd, Rect(100,100,100,100),
                      wcomposite, notifier, id++, BLUE);
    WidgetRect wrect4(drawable.gd, Rect(300,300,100,100),
                      wcomposite, notifier, id++, GREEN);
    WidgetRect wrect5(drawable.gd, Rect(700,-50,100,100),
                      wcomposite, notifier, id++, WHITE);
    WidgetRect wrect6(drawable.gd, Rect(-50,550,100,100),
                      wcomposite, notifier, id++, GREY);
    wcomposite.add_widget(&wrect1);
    wcomposite.add_widget(&wrect2);
    wcomposite.add_widget(&wrect3);
    wcomposite.add_widget(&wrect4);
    wcomposite.add_widget(&wrect5);
    wcomposite.add_widget(&wrect6);

    {
        WidgetRect wrect7(drawable.gd, Rect(0, 0, 800, 600),
                          wcomposite, notifier, id++, GREY);
        wcomposite.add_widget(&wrect7);
        wcomposite.remove_widget(&wrect7);
    }

    // ask to widget to redraw at position 150,500 and of size 800x600
    wcomposite.rdp_input_invalidate(Rect(150 + wcomposite.x(),
                                         150 + wcomposite.y(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    //drawable.save_to_png("/tmp/composite.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
    "\x42\xfd\x22\x64\x4d\xb8\xea\xfd\xf8\x95\x7b\x4d\x51\x98\x9d\x67\xb5\xe6\x82\xba"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at position 0,500 and of size 100x100
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.x(),
                                         500 + wcomposite.y(),
                                         100,
                                         100));

    //drawable.save_to_png("/tmp/composite2.png");

    if (!check_sig(drawable.gd.impl(), message,
    "\xdf\x6a\xf5\x43\xba\x3f\xf7\xce\xeb\x2e\x8c\xe7\xa9\xf0\x3c\x1b\x78\x9f\x58\x20"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.x(),
                                         0 + wcomposite.y(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    //drawable.save_to_png("/tmp/composite3.png");

    if (!check_sig(drawable.gd.impl(), message,
    "\xc8\x83\xe3\x4b\xe2\xd8\x39\x0c\xbe\x07\x93\x2f\x36\x24\x2e\xc1\x85\xae\x1a\x61"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.clear();
}

