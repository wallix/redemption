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
#define BOOST_TEST_MODULE TestWidgetImage
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/image.hpp"
// #include "internal/widget2/widget_composite.hpp"
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
    {}

    virtual void end_update()
    {}

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
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 0,0, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

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
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 10,100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

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
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, -100,500, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

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
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 700,500, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

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
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, -100,-100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

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
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 700,-100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png("/tmp/image6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x11\xe6\x83\x39\x2c\xf7\x8a\x9c\xb5\xc1"
        "\x70\xf8\x97\xa7\x52\xa2\xfa\xae\xf6\xcc")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImageClip)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 700,-100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 700,-100, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at position 80,10 and of size 50x100. After clip the size is of 20x15
    wimage.rdp_input_invalidate(Rect(80 + wimage.dx(),
                                     10 + wimage.dy(),
                                     50,
                                     100));

    //drawable.save_to_png("/tmp/image7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe2\x17\xab\x79\xd8\x69\x05\x14\x70\xe4"
        "\xba\x9c\x35\xe8\x39\x45\xa9\x63\x74\x20")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImageClip2)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetImage wimage(&drawable, 0,0, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wimage.rdp_input_invalidate(Rect(100 + wimage.dx(),
                                     25 + wimage.dy(),
                                     100,
                                     100));

    //drawable.save_to_png("/tmp/image8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x0f\xad\x91\xdf\xa5\xbd\x7d\x2b\x3b\xd2"
        "\x19\xaa\x15\xa9\x78\x80\x11\x7f\x36\x88")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetImageEvent)
// {
//     struct WidgetReceiveEvent : public Widget {
//         Widget2* sender;
//         NotifyApi::notify_event_t event;
//
//         WidgetReceiveEvent()
//         : Widget(NULL, Rect(), NULL, NULL)
//         {}
//
//         virtual void draw(const Rect&)
//         {}
//
//         virtual void notify(Widget2* sender, NotifyApi::notify_event_t event,
//                             unsigned long, unsigned long)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } widget_for_receive_event;
//
//     Widget2* parent = &widget_for_receive_event;
//     ModApi * drawable = NULL;
//     NotifyApi * notifier = NULL;
//
//     WidgetImage wimage(drawable, 0,0, FIXTURES_PATH"/logo-redemption.bmp", parent, notifier);
//
//     wimage.send_event(CLIC_BUTTON1_UP, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == &wimage);
//     BOOST_CHECK(widget_for_receive_event.event == CLIC_BUTTON1_UP);
//     wimage.send_event(CLIC_BUTTON1_DOWN, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == &wimage);
//     BOOST_CHECK(widget_for_receive_event.event == CLIC_BUTTON1_DOWN);
//     wimage.send_event(KEYUP, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == &wimage);
//     BOOST_CHECK(widget_for_receive_event.event == KEYUP);
//     wimage.send_event(KEYDOWN, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == &wimage);
//     BOOST_CHECK(widget_for_receive_event.event == KEYDOWN);
// }

// BOOST_AUTO_TEST_CASE(TraceWidgetImageAndComposite)
// {
//     TestDraw drawable(800, 600);
//
//     // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
//     Widget2* parent = NULL;
//     NotifyApi * notifier = NULL;
//
//     WidgetComposite wcomposite(&drawable, Rect(0,0,800,600), parent, notifier);
//
//     WidgetImage wimage1(&drawable, 0,0, FIXTURES_PATH"/logo-redemption.bmp",
//                         &wcomposite, notifier);
//     WidgetImage wimage2(&drawable, 0,100, FIXTURES_PATH"/logo-redemption.bmp",
//                         &wcomposite, notifier);
//     WidgetImage wimage3(&drawable, 100,100, FIXTURES_PATH"/logo-redemption.bmp",
//                         &wcomposite, notifier);
//     WidgetImage wimage4(&drawable, 300,300, FIXTURES_PATH"/logo-redemption.bmp",
//                         &wcomposite, notifier);
//     WidgetImage wimage5(&drawable, 700,-50, FIXTURES_PATH"/logo-redemption.bmp",
//                         &wcomposite, notifier);
//     WidgetImage wimage6(&drawable, -50,550, FIXTURES_PATH"/logo-redemption.bmp",
//                         &wcomposite, notifier);
//
//     wcomposite.child_list.push_back(&wimage1);
//     wcomposite.child_list.push_back(&wimage2);
//     wcomposite.child_list.push_back(&wimage3);
//     wcomposite.child_list.push_back(&wimage4);
//     wcomposite.child_list.push_back(&wimage5);
//     wcomposite.child_list.push_back(&wimage6);
//
//     // ask to widget to redraw at position 100,25 and of size 100x100.
//     wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));
//
//     //drawable.save_to_png("/tmp/image9.png");
//
//     char message[1024];
//     if (!check_sig(drawable.gd.drawable, message,
//         "\xa9\x05\x72\xca\xa4\xe1\x4e\x88\x48\x79"
//         "\xf0\x43\x37\xb8\xbc\xda\x77\x8d\x3d\x33")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     // ask to widget to redraw at it's current position
//     wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));
//
//     //drawable.save_to_png("/tmp/image10.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x76\xe4\xfd\xbb\x8e\x8e\x76\x2c\xc7\x37"
//         "\x5b\x46\xcd\xd4\xb2\x5a\xcd\x0a\x2d\x2b")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

