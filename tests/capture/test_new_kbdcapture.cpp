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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestKbdCapture
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "capture/new_kbdcapture.hpp"
#include "transport/test_transport.hpp"


BOOST_AUTO_TEST_CASE(TestKbdCapture)
{
    struct : auth_api {
        mutable std::string s;

        void log4(bool duplicate_with_pid, const char* type, const char* extra) const override {
            (void)duplicate_with_pid;
            (void)type;
            BOOST_REQUIRE(extra);
            s += extra;
        }

        void report(const char*, const char*) override {}
        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
    } auth;

    timeval const time = {0, 0};
    SessionLogKbd kbd_capture(auth);

    {
        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data='a'", auth.s);
    }

    kbd_capture.enable_kbd_input_mask(true);
    auth.s.clear();

    {
        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        // prob is not enabled
        BOOST_CHECK_EQUAL(auth.s.size(), 0);
    }

    kbd_capture.enable_kbd_input_mask(false);
    auth.s.clear();

    {
        kbd_capture.kbd_input(time, 'a');

        BOOST_CHECK_EQUAL(auth.s.size(), 0);

        kbd_capture.enable_kbd_input_mask(true);

        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data='a'", auth.s);
        auth.s.clear();

        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        BOOST_CHECK_EQUAL(auth.s.size(), 0);
    }
}


BOOST_AUTO_TEST_CASE(TestKbdCapturePatternNotify)
{
    struct : auth_api {
        mutable std::string s;

        void report(const char* reason, const char* message) override {
            s += reason;
            s += " -- ";
            s += message;
            s += "\n";
        }

        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
        void log4(bool, const char*, const char*) const override {}
    } auth;

    PatternKbd kbd_capture(&auth, "$kbd:abcd", nullptr);

    char const str[] = "abcdaaaaaaaaaaaaaaaabcdeaabcdeaaaaaaaaaaaaabcde";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }

    BOOST_CHECK_EQUAL(4, pattern_count);
    BOOST_CHECK_EQUAL(
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
      , auth.s
    );
}


BOOST_AUTO_TEST_CASE(TestKbdCapturePatternKill)
{
    struct : auth_api {
        bool is_killed = 0;

        void report(const char* , const char* ) override {
            this->is_killed = 1;
        }

        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
        void log4(bool, const char*, const char*) const override {}
    } auth;

    PatternKbd kbd_capture(&auth, "$kbd:ab/cd", nullptr);

    char const str[] = "abcdab/cdaa";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }
    BOOST_CHECK_EQUAL(1, pattern_count);
    BOOST_CHECK_EQUAL(auth.is_killed, true);
}
