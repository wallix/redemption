/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan
*/

#include <algorithm>
#include <chrono>
#include <utility>
#include <string>
#include <chrono>

#include <ctime> // localtime_r
#include <cstdio> //snprintf
#include <cstdlib> //mkostemps
#include <cerrno>
#include <cassert>

#include "core/error.hpp"

#include "utils/log.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/bytes_t.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/make_unique.hpp"

#include "utils/match_finder.hpp"
#include "utils/difftimeval.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"
#include "utils/colors.hpp"
#include "utils/stream.hpp"
#include "utils/png.hpp"

#include "transport/out_file_transport.hpp"
#include "transport/out_filename_sequence_transport.hpp"

#include "gdi/capture_probe_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"

#include "capture/utils/match_finder.hpp"
#include "capture/title_extractors/agent_title_extractor.hpp"
#include "capture/title_extractors/ocr_title_filter.hpp"
#include "capture/title_extractors/ocr_titles_extractor.hpp"
#include "capture/title_extractors/ppocr_titles_extractor.hpp"
#include "capture/title_extractors/ocr_title_extractor_builder.hpp"

#include "capture/wrm_params.hpp"
#include "capture/png_params.hpp"
#include "capture/flv_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/sequenced_video_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/pattern_checker_params.hpp"
#include "capture/kbdlog_params.hpp"
#include "capture/capture.hpp"
#include "capture/wrm_capture.hpp"

#include "capture/video_capture.hpp"

#include "utils/apps/recording_progress.hpp"


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
void filtering_kbd_input(uint32_t uchar, Utf8CharFn utf32_char_fn, NoPrintableFn no_printable_fn)
{
    constexpr struct {
        uint32_t uchar;
        array_view_const_char str;
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
        no_printable_fn(p->str);
    }
    else {
        utf32_char_fn(uchar);
    }
}


class PatternKbd : public gdi::KbdInputApi
{
    ReportMessageApi * report_message;
    PatternSearcher pattern_kill;
    PatternSearcher pattern_notify;

public:
    PatternKbd(
        ReportMessageApi * report_message,
        char const * str_pattern_kill, char const * str_pattern_notify,
        int verbose = 0)
    : report_message(report_message)
    , pattern_kill(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                   str_pattern_kill && report_message ? str_pattern_kill : nullptr, verbose)
    , pattern_notify(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                     str_pattern_notify && report_message ? str_pattern_notify : nullptr, verbose)
    {}

    bool contains_pattern() const {
        return !this->pattern_kill.is_empty() || !this->pattern_notify.is_empty();
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        bool can_be_sent_to_server = true;

        filtering_kbd_input(
            uchar,
            [this, &can_be_sent_to_server](uint32_t uchar) {
                uint8_t buf_char[5];
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
                }
            },
            [this](array_view_const_char const &) {
                this->pattern_kill.rewind_search();
                this->pattern_notify.rewind_search();
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
                assert(this->report_message);
                utils::MatchFinder::report(
                    *this->report_message,
                    is_pattern_kill,
                    utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                    pattern.c_str(),
                    str
                );
            }
        );
    }
};




class SyslogKbd : public gdi::KbdInputApi, public gdi::CaptureApi
{
    uint8_t kbd_buffer[1024];
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    timeval last_snapshot;

private:
    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->flush();
        }
        this->kbd_stream.out_uint8('*');
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (uchar == '/') {
                    this->copy_bytes({"//", 2});
                }
                else if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_view_const_char no_printable_str) {
                this->copy_bytes(no_printable_str);
            }
        );
    }

    void copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
    }

public:
    explicit SyslogKbd(timeval const & now)
    : kbd_stream(this->kbd_buffer)
    , last_snapshot(now)
    {}

    ~SyslogKbd() {
        this->flush();
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    bool kbd_input(const timeval& /*now*/, uint32_t keys) override {
        if (this->keyboard_input_mask_enabled) {
            this->write_shadow_keys();
        }
        else {
            this->write_keys(keys);
        }
        return true;
    }

    void flush() {
        if (this->kbd_stream.get_offset()) {
            LOG(LOG_INFO, R"x(type="KBD input" data="%.*s")x",
                int(this->kbd_stream.get_offset()),
                reinterpret_cast<char const *>(this->kbd_stream.get_data()));
            this->kbd_stream.rewind();
        }
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override {
        (void)cursor_x;
        (void)cursor_y;
        (void)ignore_frame_in_timeval;
        std::chrono::microseconds const time_to_wait = std::chrono::seconds{2};
        std::chrono::microseconds const diff {difftimeval(now, this->last_snapshot)};

        if (diff < time_to_wait && this->kbd_stream.get_offset() < 8 * sizeof(uint32_t)) {
            return time_to_wait;
        }

        this->flush();
        this->last_snapshot = now;

        return time_to_wait;
    }
};


namespace {
    constexpr array_view_const_char session_log_prefix() { return cstr_array_view("data='"); }
    constexpr array_view_const_char session_log_suffix() { return cstr_array_view("'"); }
}


class SessionLogKbd : public gdi::KbdInputApi, public gdi::CaptureProbeApi
{
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    static const std::size_t buffer_size = 64;
    uint8_t buffer[buffer_size + session_log_prefix().size() + session_log_suffix().size() + 1];
    bool is_probe_enabled_session = false;
    ReportMessageApi & report_message;

    void copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
    }

    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->flush();
        }
        this->kbd_stream.out_uint8('*');
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (uchar == '/') {
                    this->copy_bytes({"//", 2});
                }
                else if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_view_const_char no_printable_str) {
                this->copy_bytes(no_printable_str);
            }
        );
    }

public:
    explicit SessionLogKbd(ReportMessageApi & report_message)
    : kbd_stream{this->buffer + session_log_prefix().size(), buffer_size}
    , report_message(report_message)
    {
        memcpy(this->buffer, session_log_prefix().data(), session_log_prefix().size());
    }

    ~SessionLogKbd() {
        this->flush();
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys();
            }
        }
        else {
            this->write_keys(uchar);
        }
        return true;
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    void flush() {
        if (this->kbd_stream.get_offset()) {
            memcpy(this->kbd_stream.get_current(), session_log_suffix().data(), session_log_suffix().size() + 1);
            this->report_message.log4(false, "KBD_INPUT", reinterpret_cast<char const *>(this->buffer));
            this->kbd_stream.rewind();
        }
    }

    void session_update(const timeval& /*now*/, array_view_const_char message) override {
        this->is_probe_enabled_session = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
        this->flush();
    }

    void possible_active_window_change() override {
        this->flush();
    }
};




class PatternsChecker : noncopyable
{
    utils::MatchFinder::NamedRegexArray regexes_filter_kill;
    utils::MatchFinder::NamedRegexArray regexes_filter_notify;
    ReportMessageApi & report_message;

public:
    PatternsChecker(
        ReportMessageApi & report_message,
        const char * const filters_kill,
        const char * const filters_notify,
        int verbose = 0
    )
    : report_message(report_message)
    {
        utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
            filters_kill, this->regexes_filter_kill, verbose);

        utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
            filters_notify, this->regexes_filter_notify, verbose);
    }

    bool contains_pattern() const {
        return !this->regexes_filter_kill.empty() || !this->regexes_filter_notify.empty();
    }

    void operator()(array_view_const_char str) {
        assert(str.data() && str.size());
        this->check_filter(this->regexes_filter_kill, str.data());
        this->check_filter(this->regexes_filter_notify, str.data());
    }

private:
    void check_filter(utils::MatchFinder::NamedRegexArray & regexes_filter, char const * str) {
        if (regexes_filter.begin()) {
            utils::MatchFinder::NamedRegexArray::iterator first = regexes_filter.begin();
            utils::MatchFinder::NamedRegexArray::iterator last = regexes_filter.end();
            for (; first != last; ++first) {
                if (first->search(str)) {
                    utils::MatchFinder::report(this->report_message,
                        &regexes_filter == &this->regexes_filter_kill, // pattern_kill = FINDPATTERN_KILL
                        utils::MatchFinder::ConfigureRegexes::OCR,
                        first->name.c_str(), str);
                }
            }
        }
    }
};


namespace gdi {
    class GraphicApi;
    class CaptureApi;
    class CaptureProbeApi;
    class KbdInputApi;
    class ExternalCaptureApi;
}


class PngCapture : public gdi::CaptureApi
{
public:
    OutFilenameSequenceTransport trans;
    RDPDrawable & drawable;
    timeval start_capture;
    std::chrono::microseconds frame_interval;

    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    std::unique_ptr<uint8_t[]> scaled_buffer;


    PngCapture(const timeval & now, RDPDrawable & drawable, const PngParams & png_params)
    : trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, png_params.record_tmp_path, png_params.basename, ".png", png_params.groupid, report_error_from_reporter(png_params.report_message))
    , drawable(drawable)
    , start_capture(now)
    , frame_interval(png_params.png_interval)
    , zoom_factor(png_params.zoom)
    , scaled_width{(((this->drawable.width() * this->zoom_factor) / 100)+3) & 0xFFC}
    , scaled_height{((this->drawable.height() * this->zoom_factor) / 100)}
    {
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    void dump(void)
    {
        if (this->zoom_factor == 100) {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }
        else {
            scale_data(
                this->scaled_buffer.get(), this->drawable.data(),
                this->scaled_width, this->drawable.width(),
                this->scaled_height, this->drawable.height(),
                this->drawable.rowsize());
            ::transport_dump_png24(
                this->trans, this->scaled_buffer.get(),
                this->scaled_width, this->scaled_height,
                this->scaled_width * 3, false);
        }
    }

     virtual void clear_old() {}

     void clear_png_interval(uint32_t num_start, uint32_t num_end){
        for(uint32_t num = num_start ; num < num_end ; num++) {
            // unlink may fail, for instance if file does not exist, just don't care
            ::unlink(this->trans.seqgen()->get(num));
        }
     }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        (void)x;
        (void)y;
        (void)ignore_frame_in_timeval;
        std::chrono::microseconds const duration = difftimeval(now, this->start_capture);
        std::chrono::microseconds const interval = this->frame_interval;
        if (duration >= interval) {
             // Snapshot at end of Frame or force snapshot if diff_time_val >= 1.5 x frame_interval.
            if (this->drawable.logical_frame_ended() || (duration >= interval * 3 / 2)) {
                this->drawable.trace_mouse();
                tm ptm;
                localtime_r(&now.tv_sec, &ptm);
                this->drawable.trace_timestamp(ptm);

                this->dump();
                this->clear_old();
                this->trans.next();

                this->drawable.clear_timestamp();
                this->start_capture = now;
                this->drawable.clear_mouse();

                return interval.count() ? interval - duration % interval : interval;
            }
            else {
                // Wait 0.3 x frame_interval.
                return this->frame_interval / 3;
            }
        }
        return interval - duration;
    }
};

class PngCaptureRT : public PngCapture
{
public:
    uint32_t num_start;
    unsigned png_limit;

    bool enable_rt_display = false;

    PngCaptureRT(
        const timeval & now, RDPDrawable & drawable, const PngParams & png_params)
    : PngCapture(now, drawable, png_params)
    , num_start(this->trans.get_seqno())
    , png_limit(png_params.png_limit)
    {
    }

    ~PngCaptureRT(){
        this->clear_png_interval(this->num_start, this->trans.get_seqno() + 1);
    }

    void update_config(bool enable_rt_display) {
        if (enable_rt_display != this->enable_rt_display){
            this->enable_rt_display = enable_rt_display;
            // clear files if we go from RT to non-RT
            if (!this->enable_rt_display) {
                this->clear_png_interval(this->num_start, this->trans.get_seqno() + 1);
            }
        }
    }

     void clear_old() override {
        if (this->trans.get_seqno() < this->png_limit) {
            return;
        }
        uint32_t num_start = this->trans.get_seqno() - this->png_limit;
        this->clear_png_interval(num_start, num_start + 1);
    }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        (void)x;
        (void)y;
        (void)ignore_frame_in_timeval;
        std::chrono::microseconds const duration = difftimeval(now, this->start_capture);
        std::chrono::microseconds const interval = this->frame_interval;
        if (this->enable_rt_display) {
            return this->PngCapture::do_snapshot(now, x, y, ignore_frame_in_timeval);
        }
        return interval - duration % interval;
    }
};


namespace {
    template<std::size_t N>
    inline bool cstr_equal(char const (&s1)[N], array_view_const_char s2) {
        return N - 1 == s2.size() && std::equal(s1, s1 + N - 1, begin(s2));
    }

    template<std::size_t N>
    void str_append(std::string & s, char const (&s2)[N]) {
        s.append(s2, N-1);
    }

    inline void str_append(std::string & s, array_view_const_char const & s2) {
        s.append(s2.data(), s2.size());
    }

    template<class... S>
    void str_append(std::string & s, S const & ... strings) {
        (void)std::initializer_list<int>{
            (str_append(s, strings), 0)...
        };
    }
}

inline void agent_data_extractor(std::string & line, array_view_const_char data)
{
    using Av = array_view_const_char;

    auto find = [](Av & s, char c) {
        auto p = std::find(begin(s), end(s), c);
        return p == end(s) ? nullptr : p;
    };

    auto separator = find(data, '=');

    if (separator) {
        auto left = [](Av s, char const * pos) { return Av(begin(s), pos - begin(s)); };
        auto right = [](Av s, char const * pos) { return Av(pos + 1, begin(s) + s.size() - (pos + 1)); };

        auto order = left(data, separator);
        auto parameters = right(data, separator);

        auto line_with_1_var = [&](Av var1) {
            str_append(
                line,
                "type=\"", order, "\" ",
                Av(var1.data(), var1.size()-1), "=\"", parameters, "\""
            );
        };
        auto line_with_2_var = [&](Av var1, Av var2) {
            if (auto subitem_separator = find(parameters, '\x01')) {
                str_append(
                    line,
                    "type=\"", order, "\" ",
                    Av(var1.data(), var1.size()-1), "=\"", left(parameters, subitem_separator), "\" ",
                    Av(var2.data(), var2.size()-1), "=\"", right(parameters, subitem_separator), "\""
                );
            }
        };
        auto line_with_3_var = [&](Av var1, Av var2, Av var3) {
            if (auto subitem_separator = find(parameters, '\x01')) {
                auto text = left(parameters, subitem_separator);
                auto remaining = right(parameters, subitem_separator);
                if (auto subitem_separator2 = find(remaining, '\x01')) {
                    str_append(
                        line,
                        "type=\"", order, "\" ",
                        Av(var1.data(), var1.size()-1), "=\"", text, "\" ",
                        Av(var2.data(), var2.size()-1), "=\"", left(remaining, subitem_separator2), "\" ",
                        Av(var3.data(), var3.size()-1), "=\"", right(remaining, subitem_separator2), "\""
                    );
                }
            }
        };

        // TODO used string_id: switch (sid(order)) { case "string"_sid: ... }
        if (cstr_equal("PASSWORD_TEXT_BOX_GET_FOCUS", order)
         || cstr_equal("UAC_PROMPT_BECOME_VISIBLE", order)) {
            line_with_1_var("status");
        }
        else if (cstr_equal("INPUT_LANGUAGE", order)) {
            line_with_2_var("identifier", "display_name");
        }
        else if (cstr_equal("NEW_PROCESS", order)
              || cstr_equal("COMPLETED_PROCESS", order)) {
            line_with_1_var("command_line");
        }
        else if (cstr_equal("OUTBOUND_CONNECTION_BLOCKED", order)) {
            line_with_2_var("rule", "application_name");
        }
        else if (cstr_equal("FOREGROUND_WINDOW_CHANGED", order)) {
            line_with_3_var("windows", "class", "command_line");
        }
        else if (cstr_equal("BUTTON_CLICKED", order)) {
            line_with_2_var("windows", "button");
        }
        else if (cstr_equal("EDIT_CHANGED", order)) {
            line_with_2_var("windows", "edit");
        }
        else {
            LOG(LOG_WARNING,
                "MetaDataExtractor(): Unexpected order. Data=\"%.*s\"",
                int(data.size()), data.data());
            return;
        }
    }

    if (line.empty()) {
        LOG(LOG_WARNING,
            "MetaDataExtractor(): Invalid data format. Data=\"%.*s\"",
            int(data.size()), data.data());
        return;
    }
}

namespace {
    constexpr array_view_const_char session_meta_kbd_prefix() { return cstr_array_view("[Kbd]"); }
    constexpr array_view_const_char session_meta_kbd_suffix() { return cstr_array_view("\n"); }
}

/*
* Format:
*
* $date ' - [Kbd]' $kbd
* $date ' ' [+-] ' ' $title? '[Kbd]' $kbd
* $date ' - ' $line
*/

class SessionMeta final : public gdi::KbdInputApi, public gdi::CaptureApi, public gdi::CaptureProbeApi
{
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    uint8_t kbd_buffer[1024];
    timeval last_snapshot;
    time_t last_flush;
    Transport & trans;
    std::string title;
    bool require_kbd = false;
    char current_seperator = '-';
    bool is_probe_enabled_session = false;

    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->flush();
        }
        this->kbd_stream.out_uint8('*');
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (uchar == '/') {
                    this->copy_bytes({"//", 2});
                }
                else if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_view_const_char no_printable_str) {
                this->copy_bytes(no_printable_str);
            }
        );
    }

    void copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
    }

public:
    SessionMeta(const timeval & now, Transport & trans)
    : kbd_stream{
        this->kbd_buffer + session_meta_kbd_prefix().size(),
        sizeof(this->kbd_buffer) - session_meta_kbd_prefix().size() - session_meta_kbd_suffix().size()}
    , last_snapshot(now)
    , last_flush(now.tv_sec)
    , trans(trans)
    {
        OutStream(this->kbd_buffer).out_copy_bytes(session_meta_kbd_prefix().data(), session_meta_kbd_prefix().size());

        // force file creation even if no text recognized
        this->trans.send("", 0);
    }

    ~SessionMeta() {
        this->send_kbd();
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys();
            }
        }
        else {
            this->write_keys(uchar);
        }
        return true;
    }

    void title_changed(time_t rawtime, array_view_const_char title) {
        this->send_kbd();
        this->send_date(rawtime, '+');
        this->trans.send(title.data(), title.size());
        this->last_flush = rawtime;

        this->title.assign(title.data(), title.size());
        this->require_kbd = true;
    }

    void send_line(time_t rawtime, array_view_const_char line) {
        this->send_kbd();
        this->send_date(rawtime, '+');
        this->trans.send(line.data(), line.size());
        this->trans.send("\n", 1);
        this->last_flush = rawtime;
    }

    void session_update(const timeval& now, array_view_const_char message) override {
        this->is_probe_enabled_session = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);

        this->send_kbd();
        this->send_date(now.tv_sec, '-');
        this->trans.send(message.data(), message.size());
        this->trans.send("\n", 1);
        this->last_flush = now.tv_sec;
    }

    void possible_active_window_change() override {
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const time_to_wait = std::chrono::seconds{2};
        std::chrono::microseconds const diff {difftimeval(now, this->last_snapshot)};

        if (diff < time_to_wait && this->kbd_stream.get_offset() < 8 * sizeof(uint32_t)) {
            return time_to_wait;
        }

        this->send_kbd();

        this->last_snapshot = now;
        this->last_flush = this->last_snapshot.tv_sec;

        return time_to_wait;
    }

    void flush() {
        this->send_kbd();
    }

    void send_date(time_t rawtime, char sep) {
        tm ptm;
        localtime_r(&rawtime, &ptm);

        char string_date[256];

        auto const data_sz = std::sprintf(
            string_date, "%4d-%02d-%02d %02d:%02d:%02d %c ",
            ptm.tm_year+1900, ptm.tm_mon+1, ptm.tm_mday,
            ptm.tm_hour, ptm.tm_min, ptm.tm_sec, sep
        );

        this->trans.send(string_date, data_sz);
    }

    void send_kbd() {
        if (this->kbd_stream.get_offset()) {
            if (!this->require_kbd) {
                this->send_date(this->last_flush, this->current_seperator);
                this->trans.send(this->title.data(), this->title.size());
            }
            auto end = this->kbd_stream.get_current();
            memcpy(end, session_meta_kbd_suffix().data(), session_meta_kbd_suffix().size());
            end += session_meta_kbd_suffix().size();
            this->trans.send(this->kbd_buffer, std::size_t(end - this->kbd_buffer));
            this->kbd_stream.rewind();
            this->require_kbd = false;
        }
        else if (this->require_kbd) {
            this->trans.send("\n", 1);
            this->require_kbd = false;
        }
        this->current_seperator = '-';
    }
};

class SessionLogAgent : public gdi::CaptureProbeApi
{
    std::string line;
    SessionMeta & session_meta;

public:
    SessionLogAgent(SessionMeta & session_meta)
    : session_meta(session_meta)
    {}

    void session_update(const timeval& now, array_view_const_char message) override {
        line.clear();
        agent_data_extractor(this->line, message);
        if (!this->line.empty()) {
            this->session_meta.send_line(now.tv_sec, this->line);
        }
    }

    void possible_active_window_change() override {
    }
};

class MetaCaptureImpl
{
public:
    OutFileTransport meta_trans;
    SessionMeta meta;
    SessionLogAgent session_log_agent;
    bool enable_agent;

    MetaCaptureImpl(
        const timeval & now,
        std::string record_path,
        const char * const basename,
        bool enable_agent,
        ReportError report_error)
    : meta_trans(unique_fd{[&](){
        record_path.append(basename).append(".meta");
        const char * filename = record_path.c_str();
        int fd = ::open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0440);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        return fd;
    }()}, std::move(report_error))
    , meta(now, this->meta_trans)
    , session_log_agent(this->meta)
    , enable_agent(enable_agent)
    {
    }

    SessionMeta & get_session_meta() {
        return this->meta;
    }
};


class TitleCaptureImpl : public gdi::CaptureApi, public gdi::CaptureProbeApi
{
public:
    OcrTitleExtractorBuilder ocr_title_extractor_builder;
    AgentTitleExtractor agent_title_extractor;

    std::reference_wrapper<TitleExtractorApi> title_extractor;

    timeval  last_ocr;
    std::chrono::microseconds usec_ocr_interval;

    NotifyTitleChanged & notify_title_changed;

    TitleCaptureImpl(
        const timeval & now,
        RDPDrawable & drawable,
        OcrParams ocr_params,
        NotifyTitleChanged & notify_title_changed)
    : ocr_title_extractor_builder(
        drawable.impl(),
        ocr_params.verbosity,
        ocr_params.ocr_version,
        ocr_params.ocr_locale,
        ocr_params.on_title_bar_only,
        ocr_params.max_unrecog_char_rate)
    , title_extractor(this->ocr_title_extractor_builder.get_title_extractor())
    , last_ocr(now)
    , usec_ocr_interval(ocr_params.interval)
    , notify_title_changed(notify_title_changed)
    {
    }


    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const diff {difftimeval(now, this->last_ocr)};

        if (diff >= this->usec_ocr_interval) {
            this->last_ocr = now;

            auto title = this->title_extractor.get().extract_title();

            if (title.data()/* && title.size()*/) {
                notify_title_changed.notify_title_changed(now, title);
            }

            return this->usec_ocr_interval;
        }
        else {
            return this->usec_ocr_interval - diff;
        }
    }

    void session_update(timeval const & /*now*/, array_view_const_char message) override {
        bool const enable_probe = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
        if (enable_probe) {
            this->title_extractor = this->agent_title_extractor;
        }
        else {
            this->title_extractor = this->ocr_title_extractor_builder.get_title_extractor();
        }

        this->agent_title_extractor.session_update(message);
    }

    void possible_active_window_change() override {}
};


void Capture::TitleChangedFunctions::notify_title_changed(
    timeval const & now, array_view_const_char title
) {
    if (this->capture.patterns_checker) {
        this->capture.patterns_checker->operator()(title);
    }
    if (this->capture.meta_capture_obj) {
        this->capture.meta_capture_obj->get_session_meta().title_changed(now.tv_sec, title);
    }
    if (this->capture.sequenced_video_capture_obj) {
        this->capture.sequenced_video_capture_obj->next_video(now);
    }
    if (this->capture.update_progress_data) {
        this->capture.update_progress_data->next_video(now.tv_sec);
    }
}

void Capture::NotifyMetaIfNextVideo::notify_next_video(
    const timeval& now, NotifyNextVideo::reason reason
) {
    assert(this->session_meta);
    if (reason == NotifyNextVideo::reason::sequenced) {
        this->session_meta->send_line(now.tv_sec, cstr_array_view("(break)"));
    }
}


Capture::Capture(
    bool capture_wrm, const WrmParams wrm_params,
    bool capture_png, const PngParams png_params,
    bool capture_pattern_checker, const PatternCheckerParams /* pattern_checker_params */,
    bool capture_ocr, const OcrParams ocr_params,
    bool capture_flv, const SequencedVideoParams /*sequenced_video_params*/,
    bool capture_flv_full, const FullVideoParams /*full_video_params*/,
    bool capture_meta, const MetaParams /*meta_params*/,
    bool capture_kbd, const KbdLogParams /*kbd_log_params*/,
    const char * basename,
    const timeval & now,
    int width,
    int height,
    int /*order_bpp*/,
    int /*capture_bpp*/,
    const char * record_tmp_path,
    const char * record_path,
    const int groupid,
    const FlvParams flv_params,
    bool no_timestamp,
    ReportMessageApi * report_message,
    UpdateProgressData * update_progress_data,
    const char * pattern_kill,
    const char * pattern_notify,
    int debug_capture,
    bool flv_capture_chunk,
    bool meta_enable_session_log,
    const std::chrono::duration<long int> flv_break_interval,
    bool syslog_keyboard_log,
    bool rt_display,
    bool disable_keyboard_log,
    bool session_log_enabled,
    bool keyboard_fully_masked,
    bool meta_keyboard_log)
: is_replay_mod(!report_message)
, gd_drawable(nullptr)
, update_progress_data(update_progress_data)
, mouse_info{now, width / 2, height / 2}
, capture_event{}
, capture_drawable(capture_wrm || capture_flv || capture_ocr || capture_png || capture_flv_full)
{
   //REDASSERT(report_message ? order_bpp == capture_bpp : true);


    if (capture_png || (report_message && (capture_flv || capture_ocr))) {
        if (recursive_create_directory(record_tmp_path, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP, -1) != 0) {
            LOG(LOG_INFO, "Failed to create directory: \"%s\"", record_tmp_path);
        }
    }

    if (capture_wrm || capture_flv || capture_ocr || capture_png || capture_flv_full) {
        this->gd_drawable.reset(new RDPDrawable(width, height));
        this->gds.push_back(*this->gd_drawable);

        this->graphic_api.reset(new Graphic(this->mouse_info, this->gds, this->caps));

        if (capture_png) {
            if (png_params.real_time_image_capture) {
                this->png_capture_real_time_obj.reset(new PngCaptureRT(
                    now, *this->gd_drawable, png_params));
            }
            else {
                this->png_capture_obj.reset(new PngCapture(
                    now, *this->gd_drawable, png_params));
            }
        }

        if (capture_wrm) {
            this->wrm_capture_obj.reset(new WrmCaptureImpl(now, wrm_params, report_message, *this->gd_drawable));
        }

        if (capture_meta) {
            this->meta_capture_obj.reset(new MetaCaptureImpl(
                now, record_tmp_path, basename,
                meta_enable_session_log,
                report_error_from_reporter(report_message)
            ));
        }

        if (capture_flv) {

            std::reference_wrapper<NotifyNextVideo> notifier = this->null_notifier_next_video;
            if (flv_capture_chunk && this->meta_capture_obj) {
                this->notifier_next_video.session_meta = &this->meta_capture_obj->get_session_meta();
                notifier = this->notifier_next_video;
            }
            this->sequenced_video_capture_obj.reset(new SequencedVideoCaptureImpl(
                now, record_path, basename, groupid, no_timestamp, png_params.zoom, *this->gd_drawable,
                flv_params,
                flv_break_interval, notifier
            ));
        }

        if (capture_flv_full) {
            this->full_video_capture_obj.reset(new FullVideoCaptureImpl(
                now, record_path, basename, groupid, no_timestamp, *this->gd_drawable,
                flv_params));
        }

        if (capture_pattern_checker) {
            this->patterns_checker.reset(new PatternsChecker(
                *report_message,
                pattern_kill,
                pattern_notify,
                debug_capture)
            );
            if (!this->patterns_checker->contains_pattern()) {
                LOG(LOG_WARNING, "Disable pattern_checker");
                this->patterns_checker.reset();
            }
        }

        if (capture_ocr) {
            if (this->patterns_checker || this->meta_capture_obj || this->sequenced_video_capture_obj) {
                this->title_capture_obj.reset(new TitleCaptureImpl(
                    now, *this->gd_drawable, ocr_params,
                    this->notifier_title_changed
                ));
            }
            else {
                LOG(LOG_INFO, "Disable title_extractor");
            }
        }

        if (capture_wrm) {
            this->gds.push_back(*this->wrm_capture_obj);
            this->caps.push_back(*this->wrm_capture_obj);
            this->objs.push_back(*this->wrm_capture_obj);
            this->probes.push_back(*this->wrm_capture_obj);

            if (!disable_keyboard_log) {
                this->wrm_capture_obj->enable_keyboard_log();
            }
        }

        if (this->png_capture_real_time_obj) {
            this->png_capture_real_time_obj->enable_rt_display = rt_display;
            this->caps.push_back(*this->png_capture_real_time_obj);
        }

        if (this->png_capture_obj) {
            this->caps.push_back(*this->png_capture_obj);
        }

        if (this->sequenced_video_capture_obj) {
            //this->caps.push_back(this->sequenced_video_capture_obj->vc);
            this->caps.push_back(*this->sequenced_video_capture_obj);
       }

        if (this->full_video_capture_obj) {
            this->caps.push_back(*this->full_video_capture_obj);
        }
    }

    if (capture_kbd) {
        this->syslog_kbd_capture_obj.reset(new SyslogKbd(now));
        this->session_log_kbd_capture_obj.reset(new SessionLogKbd(*report_message));
        this->pattern_kbd_capture_obj.reset(new PatternKbd(report_message, pattern_kill, pattern_notify, debug_capture));
    }

    if (this->syslog_kbd_capture_obj.get() && (!syslog_keyboard_log)) {
        this->kbds.push_back(*this->syslog_kbd_capture_obj.get());
        this->caps.push_back(*this->syslog_kbd_capture_obj.get());
    }

    if (this->session_log_kbd_capture_obj.get() && session_log_enabled && keyboard_fully_masked) {
        this->kbds.push_back(*this->session_log_kbd_capture_obj.get());
        this->probes.push_back(*this->session_log_kbd_capture_obj.get());
    }

    if (this->pattern_kbd_capture_obj.get() && this->pattern_kbd_capture_obj->contains_pattern()) {
        this->kbds.push_back(*this->pattern_kbd_capture_obj.get());
    }

    if (this->meta_capture_obj) {
        this->caps.push_back(this->meta_capture_obj->meta);
        if (!meta_keyboard_log) {
            this->kbds.push_back(this->meta_capture_obj->meta);
            this->probes.push_back(this->meta_capture_obj->meta);
        }

        if (this->meta_capture_obj->enable_agent) {
            this->probes.push_back(this->meta_capture_obj->session_log_agent);
        }
    }
    if (this->title_capture_obj) {
        this->caps.push_back(*this->title_capture_obj);
        this->probes.push_back(*this->title_capture_obj);
    }
}

Capture::~Capture()
{
    if (this->is_replay_mod) {
        this->png_capture_obj.reset();
        if (this->png_capture_real_time_obj) { this->png_capture_real_time_obj.reset(); }
        this->wrm_capture_obj.reset();
        if (this->sequenced_video_capture_obj) {
            try {
                this->sequenced_video_capture_obj->encoding_video_frame();
            }
            catch (Error const & e) {
                LOG(LOG_ERR, "Sequenced video: last encoding video frame error: %s", e.errmsg());
            }
            this->sequenced_video_capture_obj.reset();
        }
        if (this->full_video_capture_obj) {
            try {
                this->full_video_capture_obj->encoding_video_frame();
            }
            catch (Error const & e) {
                LOG(LOG_ERR, "Full video: last encoding video frame error: %s", e.errmsg());
            }
            this->full_video_capture_obj.reset();
        }
    }
    else {
        this->title_capture_obj.reset();
        this->session_log_kbd_capture_obj.reset();
        this->syslog_kbd_capture_obj.reset();
        this->pattern_kbd_capture_obj.reset();
        this->sequenced_video_capture_obj.reset();
        this->png_capture_obj.reset();
        if (this->png_capture_real_time_obj) { this->png_capture_real_time_obj.reset(); }

        if (this->wrm_capture_obj) {
            timeval now = tvtime();
            this->wrm_capture_obj->send_timestamp_chunk(now, false);
            this->wrm_capture_obj.reset();
        }
    }
}

// TODO: this could be done directly in external png_capture_real_time_obj object
void Capture::update_config(bool enable_rt_display)
{
    if (this->png_capture_real_time_obj) {
        this->png_capture_real_time_obj->update_config(enable_rt_display);
    }
}

void Capture::set_row(size_t rownum, const uint8_t * data)
{
    if (this->capture_drawable) {
        this->gd_drawable->set_row(rownum, data);
    }
}

std::chrono::microseconds Capture::do_snapshot(
    timeval const & now,
    int cursor_x, int cursor_y,
    bool ignore_frame_in_timeval
) {
    this->capture_event.reset();

    if (this->gd_drawable) {
        this->gd_drawable->set_mouse_cursor_pos(cursor_x, cursor_y);
    }
    this->mouse_info = {now, cursor_x, cursor_y};

    std::chrono::microseconds time = std::chrono::microseconds::max();
    if (!this->caps.empty()) {
        for (gdi::CaptureApi & cap : this->caps) {
            time = std::min(time, cap.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval));
        }
        this->capture_event.update(time.count());
    }
    return time;
}

void Capture::set_pointer_display() {
    if (this->capture_drawable) {
        this->gd_drawable->show_mouse_cursor(false);
    }
}
