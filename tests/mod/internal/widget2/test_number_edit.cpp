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
#define BOOST_TEST_MODULE TestWidgetNumberEdit
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/number_edit.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(WidgetNumberEditEventPushChar)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetNumberEdit wnumber_edit(drawable, x, y, cx, parent, &notifier, "123456", 0, GREEN, RED);

    wnumber_edit.rdp_input_invalidate(wnumber_edit.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb3\xbe\x97\x34\xbe\x84\x7e\xdb\x6b\xd8"
                   "\x9e\x1b\x73\xb2\x6d\x6a\x3f\x2b\xa9\x3a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push('a');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb3\xbe\x97\x34\xbe\x84\x7e\xdb\x6b\xd8"
                   "\x9e\x1b\x73\xb2\x6d\x6a\x3f\x2b\xa9\x3a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push('2');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.rect);
    //drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x79\x18\xe1\x55\xca\x0e\x30\x3b\x1e\xa0"
                   "\x83\xc7\x98\xd2\x73\x88\x83\x01\xe9\x05"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wnumber_edit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
}
