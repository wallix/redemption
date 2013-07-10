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

#include "internal/widget2/rect.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"
#include "callback.hpp"

struct TestDraw : DrawApi
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
    {}

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
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

    virtual void draw(const RDPMemBlt&, const Rect&, const Bitmap&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPMem3Blt&, const Rect&, const Bitmap&)
    {
        BOOST_CHECK(false);
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

BOOST_AUTO_TEST_CASE(TraceWidgetRect)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 800x600 at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; // unique identifier of widget used par parent, it will be sent back in case of event
    int color = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(0,0,800,600), parent, notifier, id, color);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.dx(),
                                    0 + wrect.dy(),
                                    wrect.cx(),
                                    wrect.cy()));

    //drawable.save_to_png("/tmp/rect.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xea\xe1\x3b\x4b\xdb\xda\xa6\x75\xf1\x17"
        "\xa2\xe8\x09\xf1\xd2\x42\x7a\xdf\x85\x6d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRect2)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position -100,-100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(-100,-100,200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.dx(),
                                    0 + wrect.dy(),
                                    wrect.cx(),
                                    wrect.cy()));

    //drawable.save_to_png("/tmp/rect2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7c\x96\x36\xc6\x5a\x1e\x29\xb4\xd7\x4a"
        "\x31\x64\x37\xec\x94\x5f\x7a\x3c\x4a\x52")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRect3)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position -100,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(-100,500,200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.dx(),
                                    0 + wrect.dy(),
                                    wrect.cx(),
                                    wrect.cy()));

    //drawable.save_to_png("/tmp/rect3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x00\x57\x28\x73\x89\x49\xd5\x9e\xc0\xc1"
        "\x77\xc9\xc5\x7b\x5e\x13\x88\xf0\xf6\x33")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRect4)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(700,500,200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.dx(),
                                    0 + wrect.dy(),
                                    wrect.cx(),
                                    wrect.cy()));

    //drawable.save_to_png("/tmp/rect4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc8\x60\xbd\xc0\xe3\x38\x4a\xe5\xd3\x29"
        "\x52\x7d\xf6\x9b\x3e\x83\x97\xf0\xbc\x90")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRect5)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,-100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(700,-100,200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.dx(),
                                    0 + wrect.dy(),
                                    wrect.cx(),
                                    wrect.cy()));

    //drawable.save_to_png("/tmp/rect5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x9c\xbe\xee\x0d\xd5\xa6\x50\xfb\x99\x4b"
        "\x2d\xae\xd9\xcc\x33\x65\x6f\xc1\x5e\x1e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRect6)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 300,200 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(300, 200,200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at it's current position
    wrect.rdp_input_invalidate(Rect(0 + wrect.dx(),
                                    0 + wrect.dy(),
                                    wrect.cx(),
                                    wrect.cy()));

    //drawable.save_to_png("/tmp/rect6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x0a\x0f\xb8\xff\x34\x91\xe5\xd0\x60\x52"
        "\x56\xcb\x3a\x56\x37\x21\xe8\xc4\x22\x19")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRectClip)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 300,200 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(300,200, 200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at position 400,300 and of size 100x100. After clip the size is of 100x50
    wrect.rdp_input_invalidate(Rect(150 + wrect.dx(),
                                    100 + wrect.dy(),
                                    100,
                                    100));

    //drawable.save_to_png("/tmp/rect7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x91\xe9\x85\x69\x98\x49\xea\x17\xe6\xc8"
        "\xaf\x9b\x78\x73\x71\x65\xdd\x67\x4b\xe3")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRectClip2)
{
    TestDraw drawable(800, 600);

    // WidgetRect is a monochrome rectangular widget of size 200x200 at position 700,-100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0; /* identifiant unique du widget pour le parent (renvoyé au parent en cas d'événement) */
    int bgcolor = 0xCCF604; /* BGR */

    WidgetRect wrect(&drawable, Rect(700,-100,200,200), parent, notifier, id, bgcolor);

    // ask to widget to redraw at position 720,20 and of size 50x50
    wrect.rdp_input_invalidate(Rect(20 + wrect.dx(),
                                    120 + wrect.dy(),
                                    50,
                                    50));

    //drawable.save_to_png("/tmp/rect8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe9\x67\x5a\xe9\xa3\xa2\xca\xf3\xbe\xc2"
        "\x37\xff\xa0\xff\x20\x3d\x8e\x28\x37\xc8")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetRectEvent)
{
    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent()
        : Widget2(NULL, Rect(), NULL, NULL)
        {}

        virtual void draw(const Rect&)
        {}

        virtual void notify(Widget2* sender, NotifyApi::notify_event_t event,
                            unsigned long, unsigned long)
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event;

    Widget2* parent = &widget_for_receive_event;
    DrawApi * drawable = NULL;
    NotifyApi * notifier = NULL;

    WidgetRect wrect(drawable, Rect(), parent, notifier);

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);

    // rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    wrect.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0, &keymap);

    TODO("j'ai un gros doute sur les choix faits pour le support des événements sur les widgets"
         "les choix actuels me semblent incorrects, mais ke n'ai pas encore mis le doigt"
         "sur le problème")
    TODO("les méthodes explicites notify_parent/notify_self ne semblent pas du tout objet")
    TODO("Mélange bizarre entre ce qui arrive de l'extérieur = rdp_input_xxx"
         "et la notification vers l'extérieur d'un événement widget."
         "les deux espaces d'événements devraint être séparés.")
    TODO("le problème conceptuel actuel est à régler absolument avant d'aller plus loin"
         "sinon la confusion ne va faire que s'aggraver")

    TODO("ce sont des tests verifiant un comportement technique = pas des vrais tests")
    // BOOST_CHECK(widget_for_receive_event.sender == &wrect);
    // BOOST_CHECK(widget_for_receive_event.event == MOUSE_FLAG_BUTTON1);

    wrect.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 0, 0, 0);
    // BOOST_CHECK(widget_for_receive_event.sender == &wrect);
    // BOOST_CHECK(widget_for_receive_event.event == MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN);

    //virtual void rdp_input_scancode(long param1, long param2, long device_flags, long time, Keymap2 * keymap){
    wrect.rdp_input_scancode(16, 0, 0x0000, 538384894, &keymap);
    // BOOST_CHECK(widget_for_receive_event.sender == &wrect);
    // BOOST_CHECK(widget_for_receive_event.event == KEYDOWN);

    wrect.rdp_input_scancode(16, 0, 0xC000, 538384920, &keymap);
    // BOOST_CHECK(widget_for_receive_event.sender == &wrect);
    // BOOST_CHECK(widget_for_receive_event.event == KEYUP);

}

TODO("As soon as composite widgets will be available, we will have to check these tests"
     " are still working with two combination layers (conversion of coordinates "
     "from parent coordinates to screen_coordinates can be tricky)")
