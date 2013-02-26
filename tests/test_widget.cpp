/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to widget.cpp file
   Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCreateWidget
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "client_mod.hpp"

#include "internal/widget/screen.hpp"
#include "internal/widget/window.hpp"

struct TestWidgetMod : mod_api
{
    char buffer[32768];
    char * result;
    TestWidgetMod() : result(this->buffer) {
    }
    virtual void mod_event(int event_id) {}
    virtual void begin_update() { result += sprintf(result, "begin_update()\n"); }
    virtual void end_update() {result += sprintf(result, "end_update()\n");}
    virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        result += sprintf(result, "server_draw_text()\n");
    }
    virtual void text_metrics(const char * text, int & width, int & height)
    {
        result += sprintf(result, "text_metrics()\n");
    }
    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        result += sprintf(result, "draw(RDPOpaqueRect(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        result += sprintf(result, "draw(RDPScrBlt(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        result += sprintf(result, "draw(RDPDestBlt(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        result += sprintf(result, "draw(RDPPatBlt(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        result += sprintf(result, "draw(RDPMemBlt(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        result += sprintf(result, "draw(RDPLineTo(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        result += sprintf(result, "draw(RDPGlyphIndex(), clip(%d, %d, %d, %d))\n", clip.x, clip.y, clip.cx, clip.cy);
    }
};


BOOST_AUTO_TEST_CASE(TestCreateScreen)
{
    TestWidgetMod mod;
    Widget *screen = new widget_screen(&mod, 20, 10);
    BOOST_CHECK_EQUAL((int)WND_TYPE_SCREEN, screen->type);
    BOOST_CHECK_EQUAL(Rect(0, 0, 20, 10), screen->rect);
    const Rect rect(1, 2, 3, 4);
    screen->invalidate(rect);
    const char * expected = 
        "begin_update()\n"
        "draw(RDPOpaqueRect(), clip(1, 2, 3, 4))\n"
        "end_update()\n"
        ;
        
    if (0 != strcmp(expected, mod.buffer)){
        LOG(LOG_ERR, "expected:\n%s\n", expected); 
        LOG(LOG_ERR, "got:\n %s\n", mod.buffer); 
        BOOST_CHECK(false);
    }
    delete screen;
}

BOOST_AUTO_TEST_CASE(TestCreateScreen2)
{
    TestWidgetMod mod;
    Widget *screen = new widget_screen(&mod, 100, 100);
    BOOST_CHECK_EQUAL((int)WND_TYPE_SCREEN, screen->type);
    BOOST_CHECK_EQUAL(Rect(0, 0, 100, 100), screen->rect);
    Widget *w = new window(&mod, Rect(10, 10, 10, 10), screen, RED, "window 1");

    const Rect rect(10, 10, 50, 50);
    screen->invalidate(rect);
    const char * expected = 
        "begin_update()\n"
        "draw(RDPOpaqueRect(), clip(10, 10, 50, 50))\n"
        "end_update()\n"
        ;
        
    if (0 != strcmp(expected, mod.buffer)){
        LOG(LOG_ERR, "expected:\n%s\n", expected); 
        LOG(LOG_ERR, "got:\n %s\n", mod.buffer); 
        BOOST_CHECK(false);
    }
    delete screen;
}

//BOOST_AUTO_TEST_CASE(TestCreateWidgetScreen)
//{
////    cout << "Test Creating Screen object";
//    TODO(" passing in a null front is not a good idea  define a test front")
//    Widget *screen = new Widget((internal_mod*)0, 20, 10, NULL, WND_TYPE_SCREEN);
//    BOOST_CHECK_EQUAL(20, screen->rect.cx);
//    BOOST_CHECK_EQUAL(10, screen->rect.cy);
//    BOOST_CHECK_EQUAL((int)WND_TYPE_SCREEN, screen->type);
//    delete screen;
//}


