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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/radio_list.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetRadioButton)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);
    Font font;


    WidgetRadioButton radio0(drawable.gd, 10, 300, parent, nullptr,
                             "Alphabetical", true, 0, WHITE, DARK_BLUE_BIS);

    WidgetRadioButton radio1(drawable.gd, radio0.lx() + 10, 300, parent, nullptr,
                            "Most Used First", true, 0, WHITE, DARK_BLUE_BIS);

    WidgetRadioButton radio2(drawable.gd, radio1.lx() + 10, 300, parent, nullptr,
                            "Last Used First", true, 0, WHITE, DARK_BLUE_BIS);
    // ask to widget to redraw at it's current position
    parent.add_widget(&radio0);
    parent.add_widget(&radio1);
    parent.add_widget(&radio2);

    radio0.selected = true;
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radio1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x94\x3a\xb8\xec\xe8\x01\x5e\xbe\x20\xca\xdb\x96\x54\xfd\x30\x2e\x10\x4f\x65\xaf"
    )){
        RED_CHECK_MESSAGE(false, message);
    }

    radio0.selected = false;
    radio1.selected = true;
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "radio2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x2c\x9f\xc4\xf6\xae\x42\x4d\xdd\xe1\x8d\x72\xdd\x9f\xdd\x90\x5d\x7e\x97\x93\x58"
    )){
        RED_CHECK_MESSAGE(false, message);
    }

    radio1.selected = false;
    radio2.selected = true;
    parent.rdp_input_invalidate(parent.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "radio3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xcb\x26\x1a\x2a\xdd\x15\x09\xa1\x5f\x48\x7e\xe8\xa9\xdb\xff\x06\x96\xce\xbc\x69"
    )){
        RED_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetRadioList)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);


    WidgetRadioList radiolist(drawable.gd, 50, 70, parent, nullptr, 0, WHITE, DARK_BLUE_BIS);

    parent.add_widget(&radiolist);

    radiolist.add_elem("Alphabetical");
    radiolist.add_elem("Most Used First");
    radiolist.add_elem("Last Used First");

    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x6c\xcc\xc2\xb1\x2b\xc7\xa6\xd7\x97\x08\x56\x3a\xd1\x1b\x45\x3d\x11\x6d\x8b\x8c"
    )){
        RED_CHECK_MESSAGE(false, message);
    }

    radiolist.select(1);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x2c\xaf\x58\x89\xe5\x7d\xd3\x58\xc5\x81\xd8\xb8\x6d\xcd\x4a\x80\xc6\x37\xa4\xf2"
    )){
        RED_CHECK_MESSAGE(false, message);
    }
    radiolist.select(0);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist3.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xa7\xce\x7e\x70\x1b\xec\x38\x14\x64\x3c\x91\xcd\xac\x1c\x97\xd5\x19\xd0\x70\x6b"
    )){
        RED_CHECK_MESSAGE(false, message);
    }
    radiolist.select(2);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolist4.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x17\x2a\xa9\xe8\xad\x93\x17\xff\x89\x8d\xf0\x8b\x96\xb9\xe8\x80\xf6\xc3\x9d\x14"
    )){
        RED_CHECK_MESSAGE(false, message);
    }

}

RED_AUTO_TEST_CASE(TraceWidgetRadioListNotify)
{
    TestDraw drawable(800, 600);

    // WidgetSelectorFlat is a selector widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600);
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

    WidgetRadioList radiolist(drawable.gd, 50, 30, parent, &notifier, 0, WHITE, DARK_BLUE_BIS);

    parent.add_widget(&radiolist);

    radiolist.add_elem("Alphabetical");
    radiolist.add_elem("Most Used First");
    radiolist.add_elem("Last Used First");

    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolistnotify1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x19\xad\x25\x31\x4e\x4f\x77\xb5\x25\xc7\x7c\xc2\x71\xe1\x79\x8f\x22\x54\x26\xbe"
    )){
        RED_CHECK_MESSAGE(false, message);
    }
    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.event == 0);
    RED_CHECK(radiolist.get_selected() == -1);


    int x = radiolist.dx() + 15;
    int y = radiolist.centery();

    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           x, y, nullptr);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolistnotify2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x36\x5f\xe9\x22\x0c\x76\x9c\x76\xd3\x14\x95\xe8\x8c\x94\x9b\xb3\xcd\x4d\xfa\x04"
    )){
        RED_CHECK_MESSAGE(false, message);
    }
    RED_CHECK(notifier.sender == &radiolist);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    RED_CHECK(radiolist.get_selected() == 0);
    notifier.sender = 0;
    notifier.event = 0;

    x = radiolist.dx() + 130;
    y = radiolist.centery();

    parent.rdp_input_mouse((MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN),
                           x, y, nullptr);
    parent.rdp_input_invalidate(parent.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "radiolistnotify3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x12\x16\x75\xe9\xf3\x49\xbc\x86\x28\x3e\xd1\x8a\x00\xd9\x17\xc2\xcb\x1b\x78\xea"
    )){
        RED_CHECK_MESSAGE(false, message);
    }
    RED_CHECK(notifier.sender == &radiolist);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    RED_CHECK(radiolist.get_selected() == 1);
    notifier.sender = 0;
    notifier.event = 0;
}
