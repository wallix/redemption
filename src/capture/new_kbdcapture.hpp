/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
               Jonathan Poelen, Raphael Zhou, Meng Tan
*/

#ifndef CAPTURE_NEW_KBDCAPTURE_HPP
#define CAPTURE_NEW_KBDCAPTURE_HPP

#include "utils/match_finder.hpp"
#include "stream.hpp"
#include "cast.hpp"

#include <ctime>

struct NewKbdCapture : public RDPCaptureDevice
{
private:
    StaticOutStream<49152> unlogged_data;
    StaticOutStream<24576> data;

    timeval last_snapshot;

    bool wait_until_next_snapshot;

    auth_api * authentifier;

    utils::MatchFinder::NamedRegexArray regexes_filter_kill;
    utils::MatchFinder::NamedRegexArray regexes_filter_notify;

    bool enable_keyboard_log_syslog;
    char const * const log_prefix;

    class Utf8KbdData {
        uint8_t kbd_data[128] = { 0, 0 };

    public:
        uint8_t const * get_data() {
            return kbd_data;
        }

        inline void pop_bytes(size_t count) {
            if (!count) { return; }

            size_t kbd_data_length = ::strlen(reinterpret_cast<char const *>(this->kbd_data));

            REDASSERT(kbd_data_length >= count);
            REDASSERT((this->kbd_data[kbd_data_length] == 0) || (this->kbd_data[kbd_data_length] & 0xC0));

            kbd_data_length -= count;
            if (kbd_data_length) {
                ::memmove(this->kbd_data, &this->kbd_data[count], kbd_data_length);
            }
            this->kbd_data[kbd_data_length] = '\0';
        }

        inline size_t push_utf8_char(uint8_t const * c, size_t data_length) {
            if (!(*c)) {
                return 0;
            }

            const size_t char_size = ::get_utf8_char_size(c);

            REDASSERT(char_size <= data_length);

            uint8_t * kbd_data_tmp    = this->kbd_data;
            size_t    kbd_data_length = ::strlen(reinterpret_cast<char const *>(kbd_data_tmp));

            while ((sizeof(kbd_data) - kbd_data_length) <= char_size) {
                const size_t first_char_size = ::get_utf8_char_size(kbd_data_tmp);

                kbd_data_tmp    += first_char_size;
                kbd_data_length -= first_char_size;
            }

            if (kbd_data_tmp != this->kbd_data) {
                ::memmove(this->kbd_data, kbd_data_tmp, kbd_data_length + 1);
            }

            ::memcpy(this->kbd_data + kbd_data_length, c, char_size);
            this->kbd_data[kbd_data_length + char_size] = '\0';

            return char_size;
        }
    };
    Utf8KbdData utf8_kbd_data_kill;
    Utf8KbdData utf8_kbd_data_notify;

public:
    static const uint64_t time_to_wait = 1000000L;

    NewKbdCapture( const timeval & now, auth_api * authentifier
                 , char const * const filters_kill
                 , char const * const filters_notify
                 , bool enable_keyboard_log_syslog
                 , char const * const log_prefix
                 , int verbose = 0)
    : last_snapshot(now)
    , wait_until_next_snapshot(false)
    , authentifier(authentifier)
    , enable_keyboard_log_syslog(enable_keyboard_log_syslog)
    , log_prefix(log_prefix) {
        if (filters_kill) {
            utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                filters_kill, this->regexes_filter_kill, verbose);
        }

        if (filters_notify) {
            utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                filters_notify, this->regexes_filter_notify, verbose);
        }
    }

public:
    virtual bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override
    {
        bool can_be_sent_to_server = true;

        InStream in_raw_kbd_data(input_data_32, data_sz);
        uint32_t uchar;

        bool loop = true;

        for (size_t i = 0, count = in_raw_kbd_data.get_capacity() / sizeof(uint32_t);
             (i < count) && this->unlogged_data.has_room(1) && loop; i++) {
            uchar = in_raw_kbd_data.in_uint32_le();

            switch (uchar)
            {
            case 0x00000008:  // backspace
                if (this->unlogged_data.has_room(12)) { this->unlogged_data.out_string("/<backspace>"); }
                else { loop = false; }
                break;

            case 0x00000009:  // tab
                if (this->unlogged_data.has_room(6)) { this->unlogged_data.out_string("/<tab>"); }
                else { loop = false; }
                break;

            case 0x0000000D:  // enter
                if (this->unlogged_data.has_room(8)) { this->unlogged_data.out_string("/<enter>"); }
                else { loop = false; }
                break;

            case 0x0000001B:  // enter
                if (this->unlogged_data.has_room(9)) { this->unlogged_data.out_string("/<escape>"); }
                else { loop = false; }
                break;

            case 0x0000002F:
                if (this->unlogged_data.has_room(2)) { this->unlogged_data.out_string("//"); }
                else { loop = false; }
                break;

            case 0x0000007F:  // delete
                if (this->unlogged_data.has_room(9)) { this->unlogged_data.out_string("/<delete>"); }
                else { loop = false; }
                break;

            case 0x00002190:  // leftwards arrow
                if (this->unlogged_data.has_room(7)) { this->unlogged_data.out_string("/<left>"); }
                else { loop = false; }
                break;

            case 0x00002191:  // upwards arrow (up)
                if (this->unlogged_data.has_room(5)) { this->unlogged_data.out_string("/<up>"); }
                else { loop = false; }
                break;

            case 0x00002192:  // rightwards arrow (right)
                if (this->unlogged_data.has_room(8)) { this->unlogged_data.out_string("/<right>"); }
                else { loop = false; }
                break;

            case 0x00002193:  // downwards arrow (home)
                if (this->unlogged_data.has_room(7)) { this->unlogged_data.out_string("/<down>"); }
                else { loop = false; }
                break;

            case 0x00002196:  // north west arrow (home)
                if (this->unlogged_data.has_room(7)) { this->unlogged_data.out_string("/<home>"); }
                else { loop = false; }
                break;

            case 0x00002198:  // south east arrow (end)
                if (this->unlogged_data.has_room(6)) { this->unlogged_data.out_string("/<end>"); }
                else { loop = false; }
                break;

            default:
                {
                    size_t len = UTF32toUTF8((uint8_t *)&uchar, 1, this->unlogged_data.get_current(), this->unlogged_data.tailroom());

                    if (len > 0) {
                        if (this->authentifier) {
                            if (this->regexes_filter_kill.begin()) {
                                this->utf8_kbd_data_kill.push_utf8_char(this->unlogged_data.get_current(), len);
                                char const * const char_kbd_data = ::char_ptr_cast(this->utf8_kbd_data_kill.get_data());

                                utils::MatchFinder::NamedRegexArray::iterator first = this->regexes_filter_kill.begin();
                                utils::MatchFinder::NamedRegexArray::iterator last = this->regexes_filter_kill.end();
                                size_t last_index = 0;
                                for (; first != last; ++first) {
                                    if (first->regex.search(char_kbd_data)) {
                                        this->flush();

                                        utils::MatchFinder::report(this->authentifier,
                                            "FINDPATTERN_KILL",
                                            utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                                            first->name.c_str(), char_kbd_data);
                                        can_be_sent_to_server = false;
                                        const size_t current_last_index = first->regex.last_index();
                                        if (last_index < current_last_index) {
                                            last_index = current_last_index;
                                        }
                                    }
                                }
                                if (last_index) {
                                    this->utf8_kbd_data_kill.pop_bytes(last_index);
                                }
                            }

                            if (this->regexes_filter_notify.begin()) {
                                this->utf8_kbd_data_notify.push_utf8_char(this->unlogged_data.get_current(), len);
                                char const * const char_kbd_data = ::char_ptr_cast(this->utf8_kbd_data_notify.get_data());

                                utils::MatchFinder::NamedRegexArray::iterator first = this->regexes_filter_notify.begin();
                                utils::MatchFinder::NamedRegexArray::iterator last = this->regexes_filter_notify.end();
                                size_t last_index = 0;
                                for (; first != last; ++first) {
                                    if (first->regex.search(char_kbd_data)) {
                                        this->flush();

                                        utils::MatchFinder::report(this->authentifier,
                                            "FINDPATTERN_NOTIFY",
                                            utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                                            first->name.c_str(), char_kbd_data);
                                        const size_t current_last_index = first->regex.last_index();
                                        if (last_index < current_last_index) {
                                            last_index = current_last_index;
                                        }
                                    }
                                }
                                if (last_index) {
                                    this->utf8_kbd_data_notify.pop_bytes(last_index);
                                }
                            }
                        }

                        this->unlogged_data.out_skip_bytes(len);
                    }
                    else { loop = false; }
                }
                break;
            }
        }

        return can_be_sent_to_server;
    }

    virtual void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval,
                          bool const & requested_to_stop) override {
        if ((difftimeval(now, this->last_snapshot) < 1000000) &&
            (this->unlogged_data.get_offset() < 8 * sizeof(uint32_t))) {
            return;
        }

        if (this->wait_until_next_snapshot) {
            this->wait_until_next_snapshot = false;

            return;
        }

        this->flush();

        this->last_snapshot = now;
    }

    void flush() {
        if (this->unlogged_data.get_offset()) {
            {
                char extra[65536];
                snprintf(extra, sizeof(extra), "data='%.*s'",
                    (unsigned)this->unlogged_data.get_offset(),
                    this->unlogged_data.get_data());
                this->authentifier->log4(this->enable_keyboard_log_syslog,
                    "KBD input", extra);
            }

            if (this->data.has_room(this->unlogged_data.get_offset())) {
                this->data.out_copy_bytes(this->unlogged_data.get_data(),
                    this->unlogged_data.get_offset());
            }

            this->unlogged_data.rewind();
        }

        this->wait_until_next_snapshot = true;
    }

    bool has_data() {
        return (this->unlogged_data.get_offset() != 0) || (this->data.get_offset() != 0);
    }

    void send_data(Transport & trans) {
        REDASSERT(!this->unlogged_data.get_offset());

        trans.send(this->data.get_data(), this->data.get_offset());

        this->data.rewind();
    }

    void reset_data() {
        this->data.rewind();
    }
};

#endif
