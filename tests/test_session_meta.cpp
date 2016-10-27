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
   Copyright (C) Wallix 2010-2012
   Author(s): Jonathan Poelen

   Unit test to detect memory leak in OCR module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSessionMeta
#include "system/redemption_unit_tests.hpp"

// #define LOGNULL
#define LOGPRINT

#include "capture/session_meta.hpp"
#include "transport/test_transport.hpp"

BOOST_AUTO_TEST_CASE(TestSessionMeta)
{
    char const out_data[] =
        "1970-01-01 01:16:40 + [Kbd]ABCDABCDABCDABCDABCDABCDABCDABCDABCD\n"
        "1970-01-01 01:16:49 - [Kbd]ABCD\n"
        "1970-01-01 01:16:50 + Blah1\n"
        "1970-01-01 01:16:51 + Blah2[Kbd]ABCDABCD\n"
        "1970-01-01 01:16:54 + Blah3\n"
    ;
    CheckTransport trans(out_data, sizeof(out_data) - 1);

    timeval now;
    now.tv_sec  = 1000;
    now.tv_usec = 0;

    {
        SessionMeta meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
        send_kbd(); now.tv_sec += 1;
        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
    }
}


BOOST_AUTO_TEST_CASE(TestSessionMeta2)
{
    char const out_data[] =
        "1970-01-01 01:16:40 + Blah1\n"
        "1970-01-01 01:16:41 + Blah2[Kbd]ABCDABCD\n"
        "1970-01-01 01:16:44 + Blah3\n"
        "1970-01-01 01:16:45 + (break)\n"
    ;
    CheckTransport trans(out_data, sizeof(out_data) - 1);

    timeval now;
    now.tv_sec  = 1000;
    now.tv_usec = 0;

    {
        SessionMeta meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.snapshot(now, 0, 0, 0);
        meta.send_line(now.tv_sec, cstr_array_view("(break)"));
    }
}
