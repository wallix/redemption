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
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/widget2_rect.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : DrawApi
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
    {}

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
    }

    virtual void draw(const RDPScrBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPDestBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPPatBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPMem3Blt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPLineTo&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphIndex&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPBrushCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPColCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int16_t , int16_t , const char*, uint32_t, uint32_t, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void text_metrics(const char* , int& , int& )
    {
        BOOST_CHECK(false);
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

class WidgetCompositeRect : public WidgetComposite
{
public:
    int color;

    WidgetCompositeRect(TestDraw & drawable)
    : WidgetComposite(drawable, Rect(0, 0,
                                      drawable.gd.drawable.width,
                                      drawable.gd.drawable.height),
                      *this, NULL)
    , color(0x27642F)
    {}

    virtual ~WidgetCompositeRect()
    {}

    virtual void draw(const Rect& clip)
    {
        this->drawable.draw(RDPOpaqueRect(clip, color), this->rect);
        this->WidgetComposite::draw(clip);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetComposite)
{
    TestDraw drawable(800, 600);
    NotifyApi * notifier = NULL;
    int id = 0;

    WidgetCompositeRect wcomposite(drawable);
    WidgetRect wrect1(drawable, Rect(0,0,100,100),
                      wcomposite, notifier, id++, CYAN);
    WidgetRect wrect2(drawable, Rect(0,100,100,100),
                      wcomposite, notifier, id++, RED);
    WidgetRect wrect3(drawable, Rect(100,100,100,100),
                      wcomposite, notifier, id++, BLUE);
    WidgetRect wrect4(drawable, Rect(300,300,100,100),
                      wcomposite, notifier, id++, GREEN);
    WidgetRect wrect5(drawable, Rect(700,-50,100,100),
                      wcomposite, notifier, id++, WHITE);
    WidgetRect wrect6(drawable, Rect(-50,550,100,100),
                      wcomposite, notifier, id++, GREY);
    wcomposite.child_list.push_back(&wrect1);
    wcomposite.child_list.push_back(&wrect2);
    wcomposite.child_list.push_back(&wrect3);
    wcomposite.child_list.push_back(&wrect4);
    wcomposite.child_list.push_back(&wrect5);
    wcomposite.child_list.push_back(&wrect6);

    // ask to widget to redraw at position 150,500 and of size 800x600
    wcomposite.rdp_input_invalidate(Rect(150 + wcomposite.dx(),
                                         150 + wcomposite.dy(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    //drawable.save_to_png("/tmp/composite.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x42\xfd\x22\x64\x4d\xb8\xea\xfd\xf8\x95\x7b\x4d\x51\x98\x9d\x67\xb5\xe6\x82\xba"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at position 0,500 and of size 100x100
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.dx(),
                                         500 + wcomposite.dy(),
                                         100,
                                         100));

    //drawable.save_to_png("/tmp/composite2.png");

    if (!check_sig(drawable.gd.drawable, message,
    "\xdf\x6a\xf5\x43\xba\x3f\xf7\xce\xeb\x2e\x8c\xe7\xa9\xf0\x3c\x1b\x78\x9f\x58\x20"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0 + wcomposite.dx(),
                                         0 + wcomposite.dy(),
                                         wcomposite.cx(),
                                         wcomposite.cy()));

    //drawable.save_to_png("/tmp/composite3.png");

    if (!check_sig(drawable.gd.drawable, message,
    "\xc8\x83\xe3\x4b\xe2\xd8\x39\x0c\xbe\x07\x93\x2f\x36\x24\x2e\xc1\x85\xae\x1a\x61"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.child_list.clear();
}

