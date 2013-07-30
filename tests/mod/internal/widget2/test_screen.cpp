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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestScreen
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/button.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
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


struct Notify : public NotifyApi {
    Widget2* sender;
    notify_event_t event;

    Notify()
    : sender(0)
    , event(0)
    {}

    virtual void notify(Widget2* sender, notify_event_t event,
                        long unsigned int, long unsigned int)
    {
        this->sender = sender;
        this->event = event;
    }
};

BOOST_AUTO_TEST_CASE(TestScreenEvent)
{
    TestDraw drawable(800, 600);
    WidgetScreen wscreen(drawable, drawable.gd.drawable.width, drawable.gd.drawable.height);

    wscreen.refresh(wscreen.rect);

    Notify notifier1;
    Notify notifier2;
    Notify notifier3;
    Notify notifier4;

    WidgetButton wbutton1(drawable, 0, 0, &wscreen, &notifier1, "button 1");
    WidgetButton wbutton2(drawable, 0, 30, &wscreen, &notifier2, "button 2");
    WidgetButton wbutton3(drawable, 100, 0, &wscreen, &notifier3, "button 3");
    WidgetButton wbutton4(drawable, 100, 30, &wscreen, &notifier4, "button 4");
    struct init_border {
        static void on(WidgetButton& wbutton) {
            wbutton.border_right_bottom_color = YELLOW;
            wbutton.border_right_bottom_color2 = CYAN;
            wbutton.border_top_left_color = GREEN;
            wbutton.border_top_left_color2 = RED;
        }
    };
    init_border::on(wbutton1);
    init_border::on(wbutton2);
    init_border::on(wbutton3);
    init_border::on(wbutton4);

    wscreen.child_list.push_back(&wbutton1);
    wscreen.child_list.push_back(&wbutton2);
    wscreen.child_list.push_back(&wbutton3);
    wscreen.child_list.push_back(&wbutton4);

    wscreen.set_widget_focus(&wbutton2);

    wscreen.refresh(wscreen.rect);

    char message[1024];

    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == 0);
    //drawable.save_to_png("/tmp/screen.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x6f\xab\x0e\x51\x96\x87\x57\xd8\xc8\x7a\x83\x58\xae\xdb\xbc\xce\xcd\xa5\x1c\xc5")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier2.event == FOCUS_END);
    BOOST_CHECK(notifier3.event == FOCUS_BEGIN);
    notifier2.sender = 0;
    notifier3.sender = 0;
    notifier2.event = 0;
    notifier3.event = 0;
    //drawable.save_to_png("/tmp/screen2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5e\xfb\x88\xfc\xf9\x34\xe3\x9a\x3e\xe8\x52\xca\x9b\x98\xcc\xad\xa2\x72\x6d\x43")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier3.event == FOCUS_END);
    BOOST_CHECK(notifier4.event == FOCUS_BEGIN);
    notifier3.sender = 0;
    notifier4.sender = 0;
    notifier3.event = 0;
    notifier4.event = 0;
    //drawable.save_to_png("/tmp/screen3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x9b\xba\x24\x38\x67\xbc\xfb\x24\x85\x85\x37\x75\xb0\x47\x97\x8f\x52\xb9\x2b\x5d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier1.event == FOCUS_BEGIN);
    BOOST_CHECK(notifier4.event == FOCUS_END);
    notifier1.sender = 0;
    notifier4.sender = 0;
    notifier1.event = 0;
    notifier4.event = 0;
    //drawable.save_to_png("/tmp/screen4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xe4\x23\x73\x85\x17\x45\xb0\x13\x3b\xe1\x67\x6b\xee\x3d\x1a\x94\x0f\x34\x1d\x51")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier1.event == FOCUS_END);
    BOOST_CHECK(notifier4.event == FOCUS_BEGIN);
    notifier1.sender = 0;
    notifier4.sender = 0;
    notifier1.event = 0;
    notifier4.event = 0;
    //drawable.save_to_png("/tmp/screen5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x9b\xba\x24\x38\x67\xbc\xfb\x24\x85\x85\x37\x75\xb0\x47\x97\x8f\x52\xb9\x2b\x5d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier3.event == FOCUS_BEGIN);
    BOOST_CHECK(notifier4.event == FOCUS_END);
    notifier3.sender = 0;
    notifier4.sender = 0;
    notifier3.event = 0;
    notifier4.event = 0;
    //drawable.save_to_png("/tmp/screen6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5e\xfb\x88\xfc\xf9\x34\xe3\x9a\x3e\xe8\x52\xca\x9b\x98\xcc\xad\xa2\x72\x6d\x43")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton1.dx(), wbutton1.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == &wbutton3);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier1.event == FOCUS_BEGIN);
    BOOST_CHECK(notifier3.event == FOCUS_END);
    notifier1.sender = 0;
    notifier3.sender = 0;
    notifier1.event = 0;
    notifier3.event = 0;
    //drawable.save_to_png("/tmp/screen7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x6e\xb8\x88\xe6\x06\x88\x47\xb6\xce\x82\xe5\xc1\xd7\xaf\x7e\x65\xf6\x80\x8d\xf2")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton2.dx(), wbutton2.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier1.event == 0);
    //drawable.save_to_png("/tmp/screen8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xe4\x23\x73\x85\x17\x45\xb0\x13\x3b\xe1\x67\x6b\xee\x3d\x1a\x94\x0f\x34\x1d\x51")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    wscreen.rdp_input_scancode(0,0,0,0, &keymap);
    BOOST_CHECK(notifier1.sender == &wbutton1);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == 0);
    BOOST_CHECK(notifier1.event == FOCUS_END);
    BOOST_CHECK(notifier2.event == FOCUS_BEGIN);
    notifier1.sender = 0;
    notifier2.sender = 0;
    notifier1.event = 0;
    notifier2.event = 0;
    //drawable.save_to_png("/tmp/screen9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x6f\xab\x0e\x51\x96\x87\x57\xd8\xc8\x7a\x83\x58\xae\xdb\xbc\xce\xcd\xa5\x1c\xc5")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                            wbutton4.dx(), wbutton4.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == &wbutton2);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier2.event == FOCUS_END);
    BOOST_CHECK(notifier4.event == FOCUS_BEGIN);
    notifier2.sender = 0;
    notifier4.sender = 0;
    notifier2.event = 0;
    notifier4.event = 0;
    //drawable.save_to_png("/tmp/screen10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xc9\xfd\x61\x7c\x63\xac\xc7\xae\xc4\x1a\x96\xcc\x8f\xea\x17\x22\x8d\x53\x5f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wscreen.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                            wbutton4.dx(), wbutton4.dy(), &keymap);
    BOOST_CHECK(notifier1.sender == 0);
    BOOST_CHECK(notifier2.sender == 0);
    BOOST_CHECK(notifier3.sender == 0);
    BOOST_CHECK(notifier4.sender == &wbutton4);
    BOOST_CHECK(notifier4.event == NOTIFY_SUBMIT);
    //drawable.save_to_png("/tmp/screen11.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x9b\xba\x24\x38\x67\xbc\xfb\x24\x85\x85\x37\x75\xb0\x47\x97\x8f\x52\xb9\x2b\x5d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
