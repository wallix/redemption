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

#include "capture/CaptureDevice.hpp"
#include "transport/transport.hpp"
#include "utils/match_finder.hpp"
#include "utils/difftimeval.hpp"
#include "stream.hpp"
#include "cast.hpp"

#include "array_view.hpp"
#include "make_unique.hpp"

#include <algorithm>
#include <memory>

#include <ctime>

struct NewKbdCapture : public RDPCaptureDevice
{
private:
    StaticOutStream<49152> unlogged_data;
    StaticOutStream<24576> data;
    StaticOutStream<64> session_data;

    timeval last_snapshot;

    bool wait_until_next_snapshot;

    auth_api * authentifier;

    bool enable_keyboard_log_syslog;

    bool is_driven_by_ocr         = false;
    bool is_probe_enabled_session = false;

    bool keyboard_input_mask_enabled = false;

    uint32_t verbose;


    utils::MatchFinder::NamedRegexArray regexes_filter_kill;
    utils::MatchFinder::NamedRegexArray regexes_filter_notify;

    struct TextSearcher
    {
        re::Regex::PartOfText searcher;
        re::Regex::range_matches matches;

        void reset(re::Regex & rgx) {
            this->searcher = rgx.part_of_text_search(false);
        }

        bool next(uint8_t const * uchar) {
            return re::Regex::match_success == this->searcher.next(char_ptr_cast(uchar));
        }

        re::Regex::range_matches const & match_result(re::Regex & rgx) {
            this->matches.clear();
            return rgx.match_result(this->matches, false);
        }
    };
    std::unique_ptr<TextSearcher[]> regexes_searcher;

    class Utf8KbdData
    {
        static constexpr const size_t buf_len = 128;

        uint8_t kbd_data[buf_len] = { 0 };
        uint8_t * p = kbd_data;
        uint8_t * beg = p;

        uint8_t * data_begin() {
            using std::begin;
            return begin(this->kbd_data);
        }
        uint8_t * data_end() {
            using std::end;
            return end(this->kbd_data);
        }

    public:
        uint8_t const * get_data() const {
            return this->beg;
        }

        void reset() {
            this->p = this->kbd_data;
            this->beg = this->p;
        }

        void push_utf8_char(uint8_t const * c, size_t char_len) {
            assert(c && char_len <= 4);

            if (static_cast<size_t>(this->data_end() - this->beg) < char_len + 1u) {
                std::size_t pchar_len = 0;
                do {
                    size_t const len = get_utf8_char_size(this->beg);
                    size_t const tailroom = this->data_end() - this->beg;
                    if (tailroom < len) {
                        this->beg = this->data_begin() + (len - tailroom);
                    }
                    else {
                        this->beg += len;
                    }
                    pchar_len += len;
                } while (pchar_len < char_len + 1);
            }

            auto ec = c + char_len;
            for (; c != ec; ++c) {
                *this->p = *c;
                ++this->p;
                if (this->p == this->data_end()) {
                    this->p = this->data_begin();
                }
            }
            *this->p = 0;
        }

        void linearize() {
            if (!this->is_linearized()) {
                std::rotate(this->data_begin(), this->beg, this->data_end());
                auto const diff = this->beg - this->p;
                this->p = this->data_end() - diff;
                this->beg = this->data_begin();
            }
        }

        bool is_linearized() const {
            return this->beg <= this->p;
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
                 , bool is_driven_by_ocr
                 , int verbose = 0)
    : last_snapshot(now)
    , wait_until_next_snapshot(false)
    , authentifier(authentifier)
    , enable_keyboard_log_syslog(enable_keyboard_log_syslog)
    , is_driven_by_ocr(is_driven_by_ocr)
    , verbose(verbose) {
        if (filters_kill) {
            utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                filters_kill, this->regexes_filter_kill, verbose, true);
        }

        if (filters_notify) {
            utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                filters_notify, this->regexes_filter_notify, verbose, true);
        }

        auto count_regex = this->regexes_filter_kill.size() + this->regexes_filter_notify.size();
        if (count_regex) {
            this->regexes_searcher = std::make_unique<TextSearcher[]>(count_regex);
            auto searcher_it = this->regexes_searcher.get();
            for (auto & named_regex : this->regexes_filter_kill) {
                searcher_it->reset(named_regex.regex);
                ++searcher_it;
            }
            for (auto & named_regex : this->regexes_filter_notify) {
                searcher_it->reset(named_regex.regex);
                ++searcher_it;
            }
        }
    }

    ~NewKbdCapture() {
        this->send_session_data();
    }

public:
    bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override
    {
        constexpr struct {
            uint32_t uchar;
            array_view<char const> str;
            // for std::sort and std::lower_bound
            operator uint32_t () const { return this->uchar; }
        } noprint_table[] = {
            {0x00000008, cstr_array_view("/<backspace>")},
            {0x00000009, cstr_array_view("/<tab>")},
            {0x0000000D, cstr_array_view("/<enter>")},
            {0x0000001B, cstr_array_view("/<escape>")},
            {0x0000007F, cstr_array_view("/<delete>")},
            {0x00002190, cstr_array_view("/<left>")},
            {0x00002191, cstr_array_view("/<up>")},
            {0x00002192, cstr_array_view("/<right>")},
            {0x00002193, cstr_array_view("/<down>")},
            {0x00002196, cstr_array_view("/<home>")},
            {0x00002198, cstr_array_view("/<end>")},
        };
        using std::begin;
        using std::end;
        assert(std::is_sorted(begin(noprint_table), end(noprint_table)));

        bool can_be_sent_to_server = true;

        InStream in_raw_kbd_data(input_data_32, data_sz);
        uint32_t uchar;

        for (size_t i = 0, count = in_raw_kbd_data.get_capacity() / sizeof(uint32_t); i < count; ++i) {
            uchar = in_raw_kbd_data.in_uint32_le();

            auto p = std::lower_bound(begin(noprint_table), end(noprint_table), uchar);
            if (p != end(noprint_table) && *p == uchar) {
                if (this->unlogged_data.has_room(p->str.size())) {
                    this->unlogged_data.out_string(p->str.data());
                }
                else {
                    break;
                }
            }
            else {
                if (uchar == 0x0000002F /* '/' */) {
                    if (this->unlogged_data.has_room(1)) {
                        this->unlogged_data.out_string("/");
                    }
                    else {
                        break;
                    }
                }
                uint8_t buf_char[5]{};
                size_t const char_len = UTF32toUTF8(
                    in_raw_kbd_data.get_current() - 4,
                    1,
                    buf_char,
                    this->unlogged_data.tailroom()
                );
                this->unlogged_data.out_copy_bytes(buf_char, char_len);

                if (char_len > 0) {
                    if (this->authentifier) {
                        can_be_sent_to_server &= !this->check_filter(
                            true,   // pattern_kill = true -> FINDPATTERN_KILL
                            this->regexes_filter_kill,
                            this->regexes_searcher.get(),
                            this->utf8_kbd_data_kill,
                            buf_char, char_len
                        );
                        this->check_filter(
                            false,  // pattern_kill = false -> FINDPATTERN_NOTIFY
                            this->regexes_filter_notify,
                            this->regexes_searcher.get() + this->regexes_filter_kill.size(),
                            this->utf8_kbd_data_notify,
                            buf_char, char_len
                        );
                    }
                }
                else {
                    break;
                }
            }
        }

        return can_be_sent_to_server;
    }   // bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz)

private:
    bool check_filter(
        bool pattern_kill,
        utils::MatchFinder::NamedRegexArray const & regexes_filter,
        TextSearcher * test_searcher_it,
        Utf8KbdData & utf8_kbd_data,
        uint8_t const * utf8_char,
        std::size_t utf8_len
    ) {
        bool has_notify = false;
        if (!regexes_filter.empty()) {
            utf8_kbd_data.push_utf8_char(utf8_char, utf8_len);
            char const * char_kbd_data = ::char_ptr_cast(utf8_kbd_data.get_data());

            for (utils::MatchFinder::NamedRegex & named_regex : regexes_filter) {
                if (test_searcher_it->next(utf8_char)) {
                    utf8_kbd_data.linearize();
                    char_kbd_data = ::char_ptr_cast(utf8_kbd_data.get_data());
                    test_searcher_it->reset(named_regex.regex);

                    if (named_regex.regex.search_with_matches(char_kbd_data)) {
                        this->flush();

                        auto & match_result = test_searcher_it->match_result(named_regex.regex);
                        auto str = (!match_result.empty() && match_result[0].first)
                          ? match_result[0].first
                          : char_kbd_data;
                        utils::MatchFinder::report(
                            this->authentifier,
                            pattern_kill,
                            utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                            named_regex.name.c_str(), str
                        );
                        has_notify = true;
                    }
                }

                ++test_searcher_it;
            }
            if (has_notify) {
                utf8_kbd_data.reset();
            }
        }
        return has_notify;
    }

public:
    void enable_keyboard_input_mask(bool enable) {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();

            this->keyboard_input_mask_enabled = enable;
        }
    }

    virtual void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval,
                          bool const & requested_to_stop) override {
        if ((difftimeval(now, this->last_snapshot) < this->time_to_wait) &&
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

private:
    template<int N, class LogMgr>
    void log_input_data(LogMgr log_mgr, bool enable_mask, uint8_t const * data, size_t data_len) {
        const char prefix[] = "data=\"";
        const char suffix[] = "\"";

        char extra[N + sizeof(prefix) + sizeof(suffix) + 1];
        ::snprintf(extra, sizeof(extra), "%s%.*s%s",
            prefix,
            static_cast<unsigned>(data_len),
            data,
            suffix);
        if (enable_mask) {
            ::memset(&extra[0] + sizeof(prefix) - 1, '*', data_len);
        }

        log_mgr(extra);
    }

public:
    void flush() {
        const uint8_t * unlogged_data_p      = this->unlogged_data.get_data();
        const size_t    unlogged_data_length = this->unlogged_data.get_offset();

        if (unlogged_data_length) {
            if (this->enable_keyboard_log_syslog) {
                using Buffer = decltype(this->unlogged_data);
                this->log_input_data<Buffer::original_capacity()>(
                          [] (char const * data) {
                              LOG(LOG_INFO, "type=\"KBD input\" %s", data);
                          }
                        , this->keyboard_input_mask_enabled
                        , unlogged_data_p
                        , unlogged_data_length
                    );
            }

            if (this->data.has_room(unlogged_data_length)) {
                if (this->keyboard_input_mask_enabled) {
                    ::memset(this->data.get_current(), '*',
                        unlogged_data_length);
                    this->data.out_skip_bytes(unlogged_data_length);
                }
                else {
                    this->data.out_copy_bytes(unlogged_data_p,
                        unlogged_data_length);
                }
            }

            size_t stream_tail_room = this->session_data.tailroom();
            if (stream_tail_room < unlogged_data_length) {
                this->send_session_data();
                stream_tail_room = this->session_data.tailroom();
            }
            if (stream_tail_room >= unlogged_data_length) {
                if (this->keyboard_input_mask_enabled) {
                    if (this->is_probe_enabled_session) {
                        ::memset(this->session_data.get_current(), '*',
                            unlogged_data_length);
                        this->session_data.out_skip_bytes(unlogged_data_length);
                    }
                }
                else {
                    this->session_data.out_copy_bytes(unlogged_data_p,
                        unlogged_data_length);
                }
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

    void send_session_data() {
        if (!this->session_data.get_offset()) return;

        if (this->authentifier) {
            using Buffer = decltype(this->session_data);
            this->log_input_data<Buffer::original_capacity()>(
                      [this] (char const * data) {
                          this->authentifier->log4(false,
                              "KBD input", data);
                      }
                    , false
                    , this->session_data.get_data()
                    , this->session_data.get_offset()
                );
        }
        this->session_data.rewind();
    }

    void reset_data() {
        this->data.rewind();
    }

    virtual void session_update(const timeval & now, const char * message)
            override {
        this->is_driven_by_ocr          = true;
        this->is_probe_enabled_session  = (::strcmp(message, "Probe.Status=Unknown") != 0);

        if (!this->session_data.get_offset()) return;

        this->send_session_data();
    }

    void possible_active_window_change() override {
        if (this->is_driven_by_ocr) return;

        this->send_session_data();
    }
};

#endif
