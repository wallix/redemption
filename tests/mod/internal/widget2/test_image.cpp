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
#define BOOST_TEST_MODULE TestWidgetImage
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/image.hpp"
#include "internal/widget2/screen.hpp"
// #include "internal/widget2/widget_composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

struct TestDraw : DrawApi
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
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

    virtual void draw(const RDPMultiDstBlt &, const Rect &) {
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

    virtual void draw(const RDPGlyphIndex&, const Rect&, const GlyphCache * gly_cache)
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

    virtual void draw(const RDPPolyline &, const Rect &) {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPEllipseSC&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPEllipseCB&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int16_t , int16_t , const char* , uint32_t , uint32_t , const Rect& )
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

BOOST_AUTO_TEST_CASE(TraceWidgetImage)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, 0,0, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\xe2\x5c\x4a\x10\xe0\xbc\x8f\x3c\xb5\x0b\x10\x98\xd1\xdc\x3b\xb8\x33\x28\x76\xbb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage2)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, 10,100, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\xfe\x5c\x1a\x41\xb3\x22\xa4\xc8\xe3\x39\x31\xd3\xd2\xe8\xe6\x55\x56\xce\x9a\xc7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage3)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position -100,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, -100,500, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x15\xf3\xaf\x95\xac\x8e\x9b\xbc\x94\x91\x33\x79\x17\xf9\xee\x43\x74\x9c\x34\xc2"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage4)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 700,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, 700,500, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\xff\x70\xc7\xd1\x91\x5d\x2a\x6b\x1d\x70\xf8\xcb\x96\x8d\x04\xef\x03\xcf\x73\x0e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage5)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position -100,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, -100,-100, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x1a\x71\xe8\x5b\x2b\x93\x0c\x4b\x68\x9f\xf8\x65\xc8\x53\xdd\xb5\x59\x9f\x29\x28"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImage6)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, 700,-100, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.dx(),
                                     0 + wimage.dy(),
                                     wimage.rect.cx,
                                     wimage.rect.cy));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x34\x31\x5a\xd6\x44\x12\x6b\xb0\xb6\x61\x54\x70\x57\x63\xf3\x8f\x27\x76\xa3\x2e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImageClip)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, 700,-100, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at position 80,10 and of size 50x100. After clip the size is of 20x15
    wimage.rdp_input_invalidate(Rect(80 + wimage.dx(),
                                     10 + wimage.dy(),
                                     50,
                                     100));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x6d\xd7\xa9\xaa\xa3\x9b\x54\x39\xdd\x35\x66\x3c\x3e\xdd\x5c\xf8\xc2\xe7\xc8\x5f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetImageClip2)
{
    TestDraw drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);

    NotifyApi * notifier = NULL;

    WidgetImage wimage(drawable, 0,0, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wimage.rdp_input_invalidate(Rect(100 + wimage.dx(),
                                     25 + wimage.dy(),
                                     100,
                                     100));

    //drawable.save_to_png(OUTPUT_FILE_PATH "image8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x30\xd6\xba\x4a\xd4\x54\x54\xc8\xa6\x55\xe1\xe7\xd1\x95\x83\xca\x36\xd0\x96\x47"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetImageEvent)
// {
//     TestDraw drawable(800, 600);

//     struct WidgetReceiveEvent : public Widget2 {
//         Widget2* sender;
//         NotifyApi::notify_event_t event;

//         WidgetReceiveEvent()
//         : Widget2(drawable, Rect(), *this, NULL)
//         , sender(NULL)
//         , event(0)
//         {}

//         virtual void draw(const Rect&)
//         {}

//         virtual void notify(Widget2* sender, NotifyApi::notify_event_t event)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } widget_for_receive_event;

//     Widget2 & parent = widget_for_receive_event;
//     NotifyApi * notifier = NULL;

//     WidgetImage wimage(drawable, 0,0, FIXTURES_PATH"/logo-redemption.png", parent, notifier);

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

//     // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
//     WidgetScreen parent(drawable, 800, 600, NULL);

//     NotifyApi * notifier = NULL;

//     WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

//     WidgetImage wimage1(drawable, 0,0, FIXTURES_PATH"/logo-redemption.bmp",
//                         wcomposite, notifier);
//     WidgetImage wimage2(drawable, 0,100, FIXTURES_PATH"/logo-redemption.bmp",
//                         wcomposite, notifier);
//     WidgetImage wimage3(drawable, 100,100, FIXTURES_PATH"/logo-redemption.bmp",
//                         wcomposite, notifier);
//     WidgetImage wimage4(drawable, 300,300, FIXTURES_PATH"/logo-redemption.bmp",
//                         wcomposite, notifier);
//     WidgetImage wimage5(drawable, 700,-50, FIXTURES_PATH"/logo-redemption.bmp",
//                         wcomposite, notifier);
//     WidgetImage wimage6(drawable, -50,550, FIXTURES_PATH"/logo-redemption.bmp",
//                         wcomposite, notifier);

//     wcomposite.add_widget(&wimage1);
//     wcomposite.add_widget(&wimage2);
//     wcomposite.add_widget(&wimage3);
//     wcomposite.add_widget(&wimage4);
//     wcomposite.add_widget(&wimage5);
//     wcomposite.add_widget(&wimage6);

//     // ask to widget to redraw at position 100,25 and of size 100x100.
//     wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

//     //drawable.save_to_png(OUTPUT_FILE_PATH "image9.png");

//     char message[1024];
//     if (!check_sig(drawable.gd.drawable, message,
//         "\xa9\x05\x72\xca\xa4\xe1\x4e\x88\x48\x79"
//         "\xf0\x43\x37\xb8\xbc\xda\x77\x8d\x3d\x33")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }

//     // ask to widget to redraw at it's current position
//     wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

//     //drawable.save_to_png(OUTPUT_FILE_PATH "image10.png");

//     if (!check_sig(drawable.gd.drawable, message,
//         "\x76\xe4\xfd\xbb\x8e\x8e\x76\x2c\xc7\x37"
//         "\x5b\x46\xcd\xd4\xb2\x5a\xcd\x0a\x2d\x2b")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//     wcomposite.clear();
// }

