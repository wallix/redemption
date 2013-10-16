/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetRadioList
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/radio_list.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetRadioButton)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    Inifile ini;


    WidgetRadioButton radio0(drawable, 10, 300, parent, NULL,
                             "Alphabetical");

    WidgetRadioButton radio1(drawable, radio0.lx() + 10, 300, parent, NULL,
                            "Most Used First");

    WidgetRadioButton radio2(drawable, radio1.lx() + 10, 300, parent, NULL,
                            "Last Used First");
    // ask to widget to redraw at it's current position
    parent.add_widget(&radio0);
    parent.add_widget(&radio1);
    parent.add_widget(&radio2);

    radio0.selected = true;
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radio1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x6e\xbe\xb3\xa5\x4f\x57\x12\xa9\xa9\x26"
                   "\x32\xb5\xc9\x1d\xde\xbc\xa1\xe1\x1d\xe4"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    radio0.selected = false;
    radio1.selected = true;
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "radio2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x7b\xf5\x2f\xd9\x8d\x62\x9f\x3a\x30\xed"
                   "\x3c\xa6\x3f\xc3\xc0\x52\x2f\x1a\x5b\x0c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    radio1.selected = false;
    radio2.selected = true;
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "radio3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x43\x0a\xb0\xa1\x8e\x1a\x92\x87\xd5\x2d"
                   "\x53\xee\xcc\x6e\xf0\xa6\x68\x79\x0e\x78"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetRadioList)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);


    WidgetRadioList radiolist(drawable, 50, 70, parent, NULL);

    parent.add_widget(&radiolist);

    radiolist.add_elem("Alphabetical");
    radiolist.add_elem("Most Used First");
    radiolist.add_elem("Last Used First");

    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xa6\x3e\x25\x38\xd0\xbc\x2a\x41\x36\x44"
                   "\x9a\xc8\x1e\xe7\x41\x69\xaa\x93\x84\x59"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    radiolist.select(1);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x27\x50\x56\xc7\xef\xa0\x19\xd3\xf0\x42"
                   "\x76\xff\xc1\xb5\x57\xb2\x19\x52\xc8\x29"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    radiolist.select(0);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist3.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x92\x3c\xc0\x03\x39\x00\x73\xdf\xc0\x71"
                   "\x4c\x64\x13\x49\x20\x79\x81\xc8\x89\xdf"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    radiolist.select(2);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist4.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xa4\x77\x20\xba\x51\x68\x6b\xc1\xd3\xcd"
                   "\x40\x24\x72\x34\x0a\x48\xab\x4e\x86\xc2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetRadioListNotify)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {
        }
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    WidgetRadioList radiolist(drawable, 50, 30, parent, &notifier);

    parent.add_widget(&radiolist);

    radiolist.add_elem("Alphabetical");
    radiolist.add_elem("Most Used First");
    radiolist.add_elem("Last Used First");

    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolistnotify1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x66\x73\x53\xaf\x17\xe4\x5b\xad\xfc\x06"
                   "\x23\x73\x0b\x1e\x69\xe3\x29\x0c\x40\xfb"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    BOOST_CHECK(radiolist.get_selected() == -1);


    int x = radiolist.dx() + 15;
    int y = radiolist.centery();

    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           x, y, NULL);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolistnotify2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x02\x41\x3f\x86\x0a\x68\xd3\x52\xb0\xce"
                   "\xe0\x2d\x74\xce\x7e\x5c\xf3\xd0\x64\xb5"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &radiolist);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    BOOST_CHECK(radiolist.get_selected() == 0);
    notifier.sender = 0;
    notifier.event = 0;

    x = radiolist.dx() + 130;
    y = radiolist.centery();

    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           x, y, NULL);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolistnotify3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd7\x7e\x9f\x6d\xff\xbf\x75\xa2\xe0\x51"
                   "\x7a\x84\xd6\xe5\x00\x7e\xac\x15\x5d\x83"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &radiolist);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    BOOST_CHECK(radiolist.get_selected() == 1);
    notifier.sender = 0;
    notifier.event = 0;



}
