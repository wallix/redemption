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

#include "transport/transport.hpp"
#include "utils/match_finder.hpp"
#include "utils/difftimeval.hpp"
#include "stream.hpp"
#include "cast.hpp"

#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/capture_probe_api.hpp"

#include "utils/array_view.hpp"
#include "utils/bytes_t.hpp"
#include "utils/make_unique.hpp"

#include <algorithm>
#include <memory>

#include <ctime>


class PatternSearcher
{
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

    utils::MatchFinder::NamedRegexArray regexes_filter;
    std::unique_ptr<TextSearcher[]> regexes_searcher;
    Utf8KbdData utf8_kbd_data;

public:
    PatternSearcher(utils::MatchFinder::ConfigureRegexes conf_regex, char const * filters, int verbose = 0) {
        if (!filters) {
            return ;
        }
        utils::MatchFinder::configure_regexes(conf_regex, filters, this->regexes_filter, verbose, true);
        auto const count_regex = this->regexes_filter.size();
        if (count_regex) {
            this->regexes_searcher = std::make_unique<TextSearcher[]>(count_regex);
            auto searcher_it = this->regexes_searcher.get();
            for (auto & named_regex : this->regexes_filter) {
                searcher_it->reset(named_regex.regex);
                ++searcher_it;
            }
        }
    }

    void rewind_search() {
        TextSearcher * test_searcher_it = this->regexes_searcher.get();
        for (utils::MatchFinder::NamedRegex & named_regex : this->regexes_filter) {
            test_searcher_it->reset(named_regex.regex);
            ++test_searcher_it;
        }
    }

    template<class Report>
    bool test_uchar(uint8_t const * const utf8_char, size_t const char_len, Report report)
    {
        if (char_len == 0) {
            return false;
        }

        bool has_notify = false;

        utf8_kbd_data.push_utf8_char(utf8_char, char_len);
        TextSearcher * test_searcher_it = this->regexes_searcher.get();

        for (utils::MatchFinder::NamedRegex & named_regex : this->regexes_filter) {
            if (test_searcher_it->next(utf8_char)) {
                utf8_kbd_data.linearize();
                char const * char_kbd_data = ::char_ptr_cast(utf8_kbd_data.get_data());
                test_searcher_it->reset(named_regex.regex);

                if (named_regex.regex.search_with_matches(char_kbd_data)) {
                    auto & match_result = test_searcher_it->match_result(named_regex.regex);
                    auto str = (!match_result.empty() && match_result[0].first)
                        ? match_result[0].first
                        : char_kbd_data;
                    report(named_regex.name.c_str(), str);
                    has_notify = true;
                }
            }

            ++test_searcher_it;
        }
        if (has_notify) {
            utf8_kbd_data.reset();
        }

        return has_notify;
    }

    template<class Report>
    bool test_uchar(uint32_t uchar, Report report)
    {
        uint8_t utf8_char[5];
        size_t const char_len = UTF32toUTF8(uchar, utf8_char, 4u);
        return this->test_uchar(utf8_char, char_len, report);
    }

    bool is_empty() const {
        return this->regexes_filter.empty();
    }
};


template<class Utf8CharFn, class NoPrintableFn>
void filtering_kbd_input(gdi::KbdInputApi::Keys const & k, Utf8CharFn utf32_char_fn, NoPrintableFn no_printable_fn)
{
    auto send_uchar = [&](uint32_t uchar) -> bool {
        constexpr struct {
            uint32_t uchar;
            array_const_char str;
            // for std::sort and std::lower_bound
            operator uint32_t () const { return this->uchar; }
        } noprintable_table[] = {
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
        // TODO used static_assert
        assert(std::is_sorted(begin(noprintable_table), end(noprintable_table)));

        auto p = std::lower_bound(begin(noprintable_table), end(noprintable_table), uchar);
        if (p != end(noprintable_table) && *p == uchar) {
            if (!no_printable_fn(p->str)) {
                return false;
            }
        }
        else {
            if (!utf32_char_fn(uchar)) {
                return false;
            }
        }
        return true;
    };

    if (k.count == 1) {
        send_uchar(k.fisrt());
    }
    else if (k.count == 2) {
        if (send_uchar(k.fisrt())) {
            send_uchar(k.second());
        }
    }
}


class PatternKbd : public gdi::KbdInputApi
{
    auth_api * authentifier;
    PatternSearcher pattern_kill;
    PatternSearcher pattern_notify;

public:
    PatternKbd(
        auth_api * authentifier,
        char const * str_pattern_kill, char const * str_pattern_notify,
        int verbose = 0)
    : authentifier(authentifier)
    , pattern_kill(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                   str_pattern_kill && authentifier ? str_pattern_kill : nullptr, verbose)
    , pattern_notify(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                     str_pattern_notify && authentifier ? str_pattern_notify : nullptr, verbose)
    {}

    bool contains_pattern() const {
        return !this->pattern_kill.is_empty() || !this->pattern_notify.is_empty();
    }

    bool kbd_input(const timeval& /*now*/, Keys const & k) override {
        bool can_be_sent_to_server = true;
        uint8_t buf_char[5];

        filtering_kbd_input(
            k,
            [this, &buf_char, &can_be_sent_to_server](uint32_t uchar) {
                size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char));

                if (char_len > 0) {
                    buf_char[char_len] = '\0';
                    if (!this->pattern_kill.is_empty()) {
                        can_be_sent_to_server &= !this->test_pattern(
                            buf_char, char_len, this->pattern_kill, true
                        );
                    }
                    if (!this->pattern_notify.is_empty()) {
                        this->test_pattern(
                            buf_char, char_len, this->pattern_notify, false
                        );
                    }
                    return true;
                }

                return false;
            },
            [this](array_const_char const &) {
                this->pattern_kill.rewind_search();
                this->pattern_notify.rewind_search();
                return true;
            }
        );

        return can_be_sent_to_server;
    }

    void enable_kbd_input_mask(bool /*enable*/) override {
    }

private:
    bool test_pattern(
        uint8_t const * uchar, size_t char_len,
        PatternSearcher & searcher, bool is_pattern_kill
    ) {
        return searcher.test_uchar(
            uchar, char_len,
            [&, this](std::string const & pattern, char const * str) {
                assert(this->authentifier);
                utils::MatchFinder::report(
                    *this->authentifier,
                    is_pattern_kill,
                    utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                    pattern.c_str(),
                    str
                );
            }
        );
    }
};


template<class Inherit>
class TextKbd : public gdi::KbdInputApi
{
protected:
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;

    TextKbd(array_u8 buffer)
    : kbd_stream(buffer)
    {}

public:
    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->inherit_flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

protected:
    void write_shadow_keys(Keys const & k) {
        static const char shadow_buf[] = "**";
        if (!this->kbd_stream.has_room(k.count)) {
            this->inherit_flush();
        }
        this->kbd_stream.out_copy_bytes(shadow_buf, k.count);
    }

    void write_keys(Keys const & k) {
        uint8_t buf_char[5];

        filtering_kbd_input(
            k,
            [this, &buf_char](uint32_t uchar) {
                if (uchar == '/') {
                    return this->copy_bytes({"//", 2});
                }
                else {
                    size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char));
                    return char_len && this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_const_char no_printable_str) {
                return this->copy_bytes(no_printable_str);
            }
        );
    }

private:
    bool copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->inherit_flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
        return true;
    }

    void inherit_flush() {
        static_cast<Inherit&>(*this).flush();
    }
};


class SyslogKbd : public TextKbd<SyslogKbd>, public gdi::CaptureApi
{
    uint8_t kbd_buffer[1024];
    timeval last_snapshot;

public:
    SyslogKbd(timeval const & now)
    : TextKbd<SyslogKbd>(this->kbd_buffer)
    , last_snapshot(now)
    {}

    ~SyslogKbd() {
        this->flush();
    }

    bool kbd_input(const timeval& /*now*/, gdi::KbdInputApi::Keys const & keys) override {
        if (this->keyboard_input_mask_enabled) {
            this->write_shadow_keys(keys);
        }
        else {
            this->write_keys(keys);
        }
        return true;
    }

    std::chrono::microseconds snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override {
        std::chrono::microseconds const time_to_wait = std::chrono::seconds{2};
        std::chrono::microseconds const diff {difftimeval(now, this->last_snapshot)};

        if (diff < time_to_wait && this->kbd_stream.get_offset() < 8 * sizeof(uint32_t)) {
            return time_to_wait;
        }

        this->flush();

        this->last_snapshot = now;

        return time_to_wait;
    }

    void flush() {
        if (this->kbd_stream.get_offset()) {
            LOG(LOG_INFO, "type=\"KBD input\" data='%*s'",
                int(this->kbd_stream.get_offset()),
                reinterpret_cast<char const *>(this->kbd_stream.get_data()));
            this->kbd_stream.rewind();
        }
    }

    void external_breakpoint() override {}
    void external_time(const timeval&) override {}
    void pause_capture(const timeval&) override {}
    void resume_capture(const timeval&) override {}
    void update_config(const Inifile&) override {}
};


namespace {
    constexpr array_const_char session_log_prefix() { return cstr_array_view("data='"); }
    constexpr array_const_char session_log_suffix() { return cstr_array_view("'"); }
}

class SessionLogKbd : public TextKbd<SessionLogKbd>, public gdi::CaptureProbeApi
{
    static const std::size_t buffer_size = 64;
    uint8_t buffer[buffer_size + session_log_prefix().size() + session_log_suffix().size() + 1];
    bool is_probe_enabled_session = false;
    auth_api & authentifier;

public:
    SessionLogKbd(auth_api & authentifier)
    : TextKbd<SessionLogKbd>({this->buffer + session_log_prefix().size(), buffer_size})
    , authentifier(authentifier)
    {
        memcpy(this->buffer, session_log_prefix().data(), session_log_prefix().size());
    }

    ~SessionLogKbd() {
        this->flush();
    }

    bool kbd_input(const timeval& /*now*/, gdi::KbdInputApi::Keys const & keys) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys(keys);
            }
        }
        else {
            this->write_keys(keys);
        }
        return true;
    }

    void flush() {
        if (this->kbd_stream.get_offset()) {
            memcpy(this->kbd_stream.get_current(), session_log_suffix().data(), session_log_suffix().size() + 1);
            this->authentifier.log4(false, "KBD_INPUT", reinterpret_cast<char const *>(this->buffer));
            this->kbd_stream.rewind();
        }
    }

    void session_update(const timeval& /*now*/, const array_const_char & message) override {
        this->is_probe_enabled_session = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
        this->flush();
    }

    void possible_active_window_change() override {
        this->flush();
    }
};

#endif
