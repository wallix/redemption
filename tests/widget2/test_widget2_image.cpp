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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetRect
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/image.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : ModApi
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h, true)
    {}

    virtual void draw(const RDPOpaqueRect&, const Rect&)
    {
        BOOST_CHECK(false);
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
    {
        BOOST_CHECK(false);
    }

    virtual void end_update()
    {
        BOOST_CHECK(false);
    }

    virtual void server_draw_text(int , int , const char* , uint32_t , const Rect& )
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
                   this->gd.drawable.height, this->gd.drawable.rowsize);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetImage)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 0,0, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.send_event(WM_DRAW, 0, 0, 0);

    //drawable.save_to_png("/tmp/image.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xea\x3a\xa7\xb0\x19\x23\x98\xfe\xe8\x5f"
        "\x80\x3a\xae\xd3\xf8\x3e\x4f\x4f\xcd\xad")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage2)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 10,100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 10,100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.send_event(WM_DRAW, 0, 0, 0);

    //drawable.save_to_png("/tmp/image2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x81\xdb\x77\xd3\x15\x74\x39\x60\x2e\x73"
        "\x93\xf2\x61\x0b\x38\x18\x0f\x79\xd2\xa1")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage3)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position -100,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, -100,500, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.send_event(WM_DRAW, 0, 0, 0);

    //drawable.save_to_png("/tmp/image3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x55\xd7\xc2\x12\xb1\x92\x26\x5f\xb7\x2c"
        "\x32\xfe\xde\x84\x04\xb3\x97\x62\xb0\xd9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage4)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 700,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 700,500, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.send_event(WM_DRAW, 0, 0, 0);

    //drawable.save_to_png("/tmp/image4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd2\xca\x0d\xa6\x3f\xa3\x75\x1c\xd5\x3c"
        "\x0c\xff\xd4\x4f\x56\x2d\x75\x2c\x66\xe1")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage5)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position -100,-100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, -100,-100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.send_event(WM_DRAW, 0, 0, 0);

    //drawable.save_to_png("/tmp/image5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x69\xa3\x35\xb1\x1d\x7d\xd9\x8e\x3d\x7e"
        "\x54\x60\x9a\xc7\xc9\xd9\xae\xdc\xad\xf5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage6)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 700,-100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 700,-100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.send_event(WM_DRAW, 0, 0, 0);

    //drawable.save_to_png("/tmp/image6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x11\xe6\x83\x39\x2c\xf7\x8a\x9c\xb5\xc1"
        "\x70\xf8\x97\xa7\x52\xa2\xfa\xae\xf6\xcc")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

TODO("Add some tests where WM_DRAW receive coordinates of rect to refresh (refresh is clipped)"
     "proposal:"
     "- param1 is 32 bits and contains 2 packed 16 bits values x and y"
     "- param2 is 32 bits and contains 2  packed 16 bits values cx and cy")
     
TODO("the entry point exists in module: it's rdp_input_invalidate"
     "je just have to change received values to widget messages")
     
TODO("As soon as composite widgets will be available, we will have to check these tests"
     " are still working with two combination layers (conversion of coordinates "
     "from parent coordinates to screen_coordinates can be tricky)")
     
TODO("add test with keyboard and mouse events to check they are transmitted as expected")
