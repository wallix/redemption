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
#define BOOST_TEST_MODULE TestWindowLogin
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/window_login.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif


struct TestDraw : DrawApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
    , font(FIXTURES_PATH "/dejavu-sans-10.fv1")
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

    virtual void draw(const RDPPatBlt& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
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

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, bgcolor, clip, this->font);
    }

    virtual void text_metrics(const char* text, int& width, int& height)
    {
        height = 0;
        width = 0;
        uint32_t uni[256];
        size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
        if (len_uni){
            for (size_t index = 0; index < len_uni; index++) {
                FontChar *font_item = this->gd.get_font(this->font, uni[index]);
                width += font_item->width + 2;
                height = std::max(height, font_item->height);
            }
            width -= 2;
        }
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWindowLogin)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;
    int id = 0;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test1", id, "rec", "rec");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(window_login.rect);

    //drawable.save_to_png("/tmp/window_login.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfc\x43\xd3\xf2\x92\x41\xe9\x6b\x27\x95"
        "\xd4\xe2\x20\x27\x52\xbe\xa8\xd9\xc3\x0a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin2)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 10;
    int16_t y = 100;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test2");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x80\xcd\xfa\xc5\x68\x32\x32\xf3\xf3\xab"
        "\x3e\x58\xa1\x62\x73\xd7\x17\x25\x84\xb4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin3)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -10;
    int16_t y = 500;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test3");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x78\xca\x63\x89\x75\xbe\x83\x3f\x65\xa7"
        "\xcc\x77\x57\xf5\x17\xb3\x04\xed\x36\x9f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin4)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 770;
    int16_t y = 500;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test4");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe2\x15\x7f\x77\x79\xcc\x77\x71\x67\xcb"
        "\x28\x4a\x29\x73\x16\x8e\xe1\xa3\xb7\xdb")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin5)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -20;
    int16_t y = -7;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test5");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x1d\xd5\xde\xb4\x6c\xfb\x48\xf8\x23\xba"
        "\x8a\x88\x39\xbc\x72\xad\xc0\x33\x37\x69")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin6)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test6");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x66\xc2\x0e\x79\x4d\x30\xf3\x81\x35\x38"
        "\x96\x6e\x4f\x47\x0a\xe2\xd7\x45\x91\x3e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLoginClip)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test6");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    window_login.rdp_input_invalidate(Rect(20 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x17\x57\xb2\xc8\x32\xf1\x63\x5b\x19\x8a"
        "\x19\xc5\xa6\x8c\x71\x90\xb4\x81\x5b\xf9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLoginClip2)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowLogin window_login(drawable, x, y, parent, notifier, "test6");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    window_login.rdp_input_invalidate(Rect(20 + window_login.dx(),
                                      5 + window_login.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/window_login8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x41\x37\xe3\x2f\xb2\xfb\x1e\x6f\x6c\x9a"
        "\x93\x72\x4f\x8c\x7c\x90\xf3\x9d\x0d\xa4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    Widget2* parent = NULL;
    struct Notify : NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget2* sender, notify_event_t event, long unsigned int param, long unsigned int param2)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    int16_t x = 10;
    int16_t y = 10;

    WindowLogin window_login(drawable, x, y, parent, &notifier, "test6");


    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    window_login.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                    window_login.ok.dx(), window_login.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    window_login.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                    window_login.ok.dx(), window_login.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == &window_login);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
}

BOOST_AUTO_TEST_CASE(EventWidgetHelp)
{
    TestDraw drawable(800, 600);

    class Screen : public WidgetComposite {
    public:
        Screen(DrawApi* drawable)
        : WidgetComposite(drawable, Rect(0,0,800,600), 0, 0)
        {}

        virtual ~Screen()
        {}

        virtual void draw(const Rect& clip)
        {
            this->WidgetComposite::draw(clip);
            Rect new_clip = clip.intersect(this->rect);
            Region region;
            region.rects.push_back(new_clip);

            for (std::size_t i = 0, size = this->child_list.size(); i < size; ++i) {
                Rect rect = new_clip.intersect(this->child_list[i]->rect);

                if (!rect.isempty()) {
                    region.subtract_rect(rect);
                }
            }

            for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
                this->drawable->draw(RDPOpaqueRect(region.rects[i], 0x000000),
                                     region.rects[i]);
            }
        }
    };

    Screen parent(&drawable);

    int16_t x = 10;
    int16_t y = 10;

    WindowLogin window_login(drawable, x, y, &parent, 0, "test6");
    parent.child_list.push_back(&window_login);

    parent.rdp_input_invalidate(parent.rect);

    window_login.help.send_notify(NOTIFY_SUBMIT);
    parent.rdp_input_invalidate(parent.rect);

    //drawable.save_to_png("/tmp/window_login-help.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfd\xea\x6e\xac\x4c\x62\xe6\xb8\x72\x36"
        "\x71\x3d\xc8\x02\xba\x0d\x8a\x3b\xba\xf2")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    //close window_help and redraw
    window_login.window_help->button_close.send_notify(NOTIFY_CANCEL);

    //drawable.save_to_png("/tmp/window_login-help2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xc8\xd8\x06\xc1\xcd\x95\x34\xf5\xb3\x52"
        "\xc9\x68\x3e\x14\x21\x56\xc4\xb9\x44\x25")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

