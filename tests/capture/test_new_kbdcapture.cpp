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

    struct Translog : KbdNotifyFlushApi {
        MemoryTransport trans;
        bool enable_mask = false;

        void notify_flush(const array_const_char& data, bool enable_mask) override {
            BOOST_CHECK_EQUAL(this->enable_mask, enable_mask);
            trans.send(data.data(), data.size());
        }

        void rewind() { this->trans.out_stream.rewind(); }
        size_t get_offset() { return this->trans.out_stream.get_offset(); }
        char * get_data() { return reinterpret_cast<char*>(this->trans.out_stream.get_data()); }
    } trans;

    timeval const time = {0, 0};
    NewKbdCapture kbd_capture(time, &auth, nullptr, nullptr);
    KbdSessionLogNotify session_log(auth);

    kbd_capture.attach_flusher(session_log);
    kbd_capture.attach_flusher(trans);

    const unsigned char input[] = {'a', 0, 0, 0};

    {
        kbd_capture.input_kbd(time, input);
        kbd_capture.flush();

        session_log.send_session_data();
        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data=\"a\"", auth.s);
        BOOST_CHECK_EQUAL(trans.get_offset(), 1);
        BOOST_CHECK_EQUAL('a', *trans.get_data());
    }

    kbd_capture.enable_keyboard_input_mask(true);
    trans.enable_mask = true;
    trans.rewind();
    auth.s.clear();

    {
        kbd_capture.input_kbd(time, input);
        kbd_capture.flush();

        session_log.send_session_data();
        // prob is not enabled
        BOOST_CHECK_EQUAL(auth.s.size(), 0);
        BOOST_CHECK_EQUAL(trans.get_offset(), 1);
        BOOST_CHECK_EQUAL('*', *trans.get_data());
    }

    kbd_capture.enable_keyboard_input_mask(false);
    trans.enable_mask = false;
    trans.rewind();
    auth.s.clear();

    {
        kbd_capture.input_kbd(time, input);
        kbd_capture.enable_keyboard_input_mask(true);

        session_log.send_session_data();
        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL(trans.get_offset(), 1);
        trans.enable_mask = true;

        kbd_capture.input_kbd(time, input);
        kbd_capture.flush();

        session_log.send_session_data();
        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data=\"a\"", auth.s);
        BOOST_CHECK_EQUAL(trans.get_offset(), 2);
        BOOST_CHECK_EQUAL('a', trans.get_data()[0]);
        BOOST_CHECK_EQUAL('*', trans.get_data()[1]);
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
    NewKbdCapture kbd_capture(time, &auth, nullptr, "$kbd:abcd");

    unsigned char input[] = {0, 0, 0, 0};
    char const str[] = "abcdaaaaaaaaaaaaaaaabcdeaabcdeaaaaaaaaaaaaabcde";
    for (auto c : str) {
        input[0] = c;
        kbd_capture.input_kbd(time, input);
    }
    kbd_capture.flush();
    BOOST_CHECK_EQUAL(
        "FINDPATTERN_NOTIFY -- $kbd:abcd|abcd\n"
        "FINDPATTERN_NOTIFY -- $kbd:abcd|abcd\n"
        "FINDPATTERN_NOTIFY -- $kbd:abcd|abcd\n"
        "FINDPATTERN_NOTIFY -- $kbd:abcd|abcd\n"
      , auth.s
    );
}
