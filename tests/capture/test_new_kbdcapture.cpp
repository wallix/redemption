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
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "capture/new_kbdcapture.hpp"
#include "transport/test_transport.hpp"


BOOST_AUTO_TEST_CASE(TestKbdCapture)
{
    struct : auth_api {
        mutable std::string s;

        void log4(bool duplicate_with_pid, const char* type, const char* extra = nullptr) const override {
            BOOST_REQUIRE(extra);
            s += extra;
        }

        void report(const char*, const char*) override {}
        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
    } auth;

    timeval const time = {0, 0};
    NewKbdCapture kbd_capture(time, &auth, nullptr, nullptr, false, false);

    const unsigned char input[] = {'a', 0, 0, 0};
    const std::size_t input_sz = std::end(input) - std::begin(input);
    MemoryTransport trans;

    {
        kbd_capture.input(time, input, input_sz);
        kbd_capture.flush();

        kbd_capture.send_session_data();
        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data=\"a\"", auth.s);
        trans.out_stream.rewind();
        kbd_capture.send_data(trans);
        BOOST_CHECK_EQUAL(trans.out_stream.get_offset(), 1);
        BOOST_CHECK_EQUAL('a', *trans.out_stream.get_data());
    }

    kbd_capture.enable_keyboard_input_mask(true);
    auth.s.clear();

    {
        kbd_capture.input(time, input, input_sz);
        kbd_capture.flush();

        kbd_capture.send_session_data();
        // prob is not enabled
        BOOST_CHECK_EQUAL(auth.s.size(), 0);
        trans.out_stream.rewind();
        kbd_capture.send_data(trans);
        BOOST_CHECK_EQUAL(trans.out_stream.get_offset(), 1);
        BOOST_CHECK_EQUAL('*', *trans.out_stream.get_data());
    }

    kbd_capture.enable_keyboard_input_mask(false);
    auth.s.clear();

    {
        kbd_capture.input(time, input, input_sz);
        kbd_capture.enable_keyboard_input_mask(true);
        kbd_capture.input(time, input, input_sz);
        kbd_capture.flush();

        trans.out_stream.rewind();
        kbd_capture.send_data(trans);
        BOOST_CHECK_EQUAL(trans.out_stream.get_offset(), 2);
        BOOST_CHECK_EQUAL('a', trans.out_stream.get_data()[0]);
        BOOST_CHECK_EQUAL('*', trans.out_stream.get_data()[1]);
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

    timeval const time = {0, 0};
    NewKbdCapture kbd_capture(time, &auth, "$kbd:abcd", nullptr, false, false);

    unsigned char input[] = {0, 0, 0, 0};
    char const str[] = "abcdaaaaaaaaaaaaaaaabcdeaabcdeaaaaaaaaaaaaabcde";
    unsigned count_ok = 0;
    for (auto c : str) {
        input[0] = c;
        if (!kbd_capture.input(time, input, 4)) {
            ++count_ok;
        }
    }
    kbd_capture.flush();
    BOOST_CHECK_EQUAL(4, count_ok);
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
        bool kill = 0;

        void report(const char* , const char* ) override {
            this->kill = 1;
        }

        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
        void log4(bool, const char*, const char*) const override {}
    } auth;

    timeval const time = {0, 0};
    NewKbdCapture kbd_capture(time, &auth, "$kbd:ab/cd", nullptr, false, false);

    unsigned char input[] = {0, 0, 0, 0};
    char const str[] = "abcdab/cdaa";
    unsigned count_ok = 0;
    for (auto c : str) {
        input[0] = c;
        if (!kbd_capture.input(time, input, 4)) {
            ++count_ok;
        }
    }
    kbd_capture.flush();
    BOOST_CHECK_EQUAL(1, count_ok);
    BOOST_CHECK_EQUAL(auth.kill, true);
}
