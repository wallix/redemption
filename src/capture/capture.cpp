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
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan,
              Jennifer Inthavong
*/

#include <algorithm>
#include <chrono>
#include <string>
#include <iterator>
#include <type_traits>

#include <ctime> // localtime_r
#include <cstdio> // snprintf / sprintf
#include <cerrno>
#include <cassert>

#include "core/error.hpp"
#include "core/log_id.hpp"
#include "core/window_constants.hpp"
#include "core/RDP/RDPDrawable.hpp"

#include "utils/log.hpp"
#include "utils/recording_progress.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/ranges.hpp"

#include "utils/scaled_image24.hpp"
#include "utils/colors.hpp"
#include "utils/strutils.hpp"
#include "utils/fileutils.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/stream.hpp"
#include "utils/utf.hpp"
#include "utils/timestamp_tracer.hpp"

#include "transport/file_transport.hpp"
#include "transport/out_filename_sequence_transport.hpp"

#include "gdi/capture_probe_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/subrect4.hpp"
#include "gdi/kbd_input_api.hpp"

#include "capture/title_extractors/agent_title_extractor.hpp"
#include "capture/title_extractors/ocr_title_extractor_builder.hpp"

#include "capture/capture_params.hpp"
#include "capture/drawable_params.hpp"
#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"
#include "capture/png_params.hpp"
#include "capture/video_params.hpp"
#include "capture/wrm_params.hpp"

#include "capture/capture.hpp"
#include "capture/wrm_capture.hpp"
#include "capture/utils/match_finder.hpp"
#include "utils/video_cropper.hpp"
#include "utils/drawable_pointer.hpp"

#ifndef REDEMPTION_NO_FFMPEG
# include "capture/video_capture.hpp"
#else
class FullVideoCaptureImpl {};
class SequencedVideoCaptureImpl {};
#endif

using std::begin;
using std::end;
using namespace std::chrono_literals;

namespace
{

struct ZStrUtf8Char
{
    uint8_t buf_char[5];
    uint8_t char_len;

    ZStrUtf8Char(uint32_t uchar) noexcept
    : char_len(UTF32toUTF8(uchar, buf_char, sizeof(buf_char)))
    {
        buf_char[char_len] = 0;
    }

    [[nodiscard]] uint8_t const* data() const noexcept { return buf_char; }
    [[nodiscard]] uint8_t size() const noexcept { return char_len; }

    [[nodiscard]] uint8_t const* begin() const noexcept { return data(); }
    [[nodiscard]] uint8_t const* end() const noexcept { return data() + size(); }
};

class PatternSearcher
{
    struct TextSearcher
    {
        re::Regex::PartOfText searcher;
        re::Regex::range_matches matches;

        void reset(re::Regex & rgx) {
            this->searcher = rgx.part_of_text_search(false);
        }

        bool next(ZStrUtf8Char const& utf8_char) {
            return re::Regex::match_success == this->searcher.next(char_ptr_cast(utf8_char.data()));
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
        [[nodiscard]] uint8_t const * get_data() const {
            return this->beg;
        }

        void reset() {
            this->p = this->kbd_data;
            this->beg = this->p;
        }

        void push_utf8_char(ZStrUtf8Char const& utf8_char) {
            if (static_cast<size_t>(this->data_end() - this->beg) < utf8_char.size() + 1u) {
                std::size_t pchar_len = 0;
                do {
                    size_t const len = UTF8CharNbBytes(this->beg);
                    size_t const tailroom = this->data_end() - this->beg;
                    if (tailroom < len) {
                        this->beg = this->data_begin() + (len - tailroom);
                    }
                    else {
                        this->beg += len;
                    }
                    pchar_len += len;
                } while (pchar_len < utf8_char.size() + 1u);
            }

            for (uint8_t c : utf8_char) {
                *this->p = c;
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

        [[nodiscard]] bool is_linearized() const {
            return this->beg <= this->p;
        }
    };

    utils::MatchFinder::NamedRegexArray regexes_filter;
    std::unique_ptr<TextSearcher[]> regexes_searcher;
    Utf8KbdData utf8_kbd_data;

public:
    explicit PatternSearcher(utils::MatchFinder::ConfigureRegexes conf_regex, char const * filters, bool verbose) {
        utils::MatchFinder::configure_regexes(conf_regex, filters, this->regexes_filter, verbose,
            utils::MatchFinder::WithCapture(true));
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
    bool test_uchar(ZStrUtf8Char const& utf8_char, Report report)
    {
        if (utf8_char.size() == 0) {
            return false;
        }

        bool has_notify = false;

        utf8_kbd_data.push_utf8_char(utf8_char);
        TextSearcher * test_searcher_it = this->regexes_searcher.get();

        for (utils::MatchFinder::NamedRegex & named_regex : this->regexes_filter) {
            if (test_searcher_it->next(utf8_char)) {
                utf8_kbd_data.linearize();
                char const * char_kbd_data = ::char_ptr_cast(utf8_kbd_data.get_data());
                if (named_regex.regex.search_with_matches(char_kbd_data)) {
                    auto & match_result = test_searcher_it->match_result(named_regex.regex);
                    auto str = (!match_result.empty() && match_result[0].first)
                        ? match_result[0].first
                        : char_kbd_data;
                    report(named_regex.name, str);
                    has_notify = true;
                }
                test_searcher_it->reset(named_regex.regex);
            }

            ++test_searcher_it;
        }
        if (has_notify) {
            utf8_kbd_data.reset();
        }

        return has_notify;
    }

    [[nodiscard]] bool is_empty() const {
        return this->regexes_filter.empty();
    }
};

bool update_enable_probe(bool& enable_probe, LogId id, KVLogList kv_list)
{
    if (id == LogId::INPUT_LANGUAGE) {
        enable_probe = true;
        return false;
    }

    if (id == LogId::PROBE_STATUS && not kv_list.empty()) {
        enable_probe = ranges_equal("Ready"_av, kv_list[0].value);
        return true;
    }

    return false;
}

enum class FilteringSlash{ No, Yes };
using filter_slash = std::integral_constant<FilteringSlash, FilteringSlash::Yes>;
using nofilter_slash = std::integral_constant<FilteringSlash, FilteringSlash::No>;
template<class Utf8CharFn, class NoPrintableFn, class FilterSlash>
void filtering_kbd_input(uint32_t uchar, Utf8CharFn utf32_char_fn,
                        NoPrintableFn no_printable_fn, FilterSlash filter_slash)
{
    switch (uchar)
    {
        case '/':
            if (filter_slash == FilteringSlash::Yes) {
                no_printable_fn(cstr_array_view("//"));
            }
            else {
                utf32_char_fn(uchar);
            }
            break;
        #define Case(i, s) case i: no_printable_fn(cstr_array_view(s)); break
        Case(0x00000008, "/<backspace>");
        Case(0x00000009, "/<tab>");
        Case(0x0000000D, "/<enter>");
        Case(0x0000001B, "/<escape>");
        Case(0x0000007F, "/<delete>");
        Case(0x00002190, "/<left>");
        Case(0x00002191, "/<up>");
        Case(0x00002192, "/<right>");
        Case(0x00002193, "/<down>");
        Case(0x00002196, "/<home>");
        Case(0x00002198, "/<end>");
        #undef Case
        default: utf32_char_fn(uchar);
    }
}

inline time_t to_time_t(MonotonicTimePoint t, MonotonicTimeToRealTime monotonic_to_real)
{
    auto duration = monotonic_to_real.to_real_time_duration(t);
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

} // anonymous namespace

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wsubobject-linkage")
class Capture::PatternKbd final : public gdi::KbdInputApi
{
    SessionLogApi * session_log;
    PatternSearcher pattern_kill;
    PatternSearcher pattern_notify;

public:
    explicit PatternKbd(
        SessionLogApi * session_log,
        char const * str_pattern_kill, char const * str_pattern_notify,
        bool verbose)
    : session_log(session_log)
    , pattern_kill(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                   str_pattern_kill && session_log ? str_pattern_kill : nullptr, verbose)
    , pattern_notify(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                     str_pattern_notify && session_log ? str_pattern_notify : nullptr, verbose)
    {}

    [[nodiscard]] bool contains_pattern() const {
        return !this->pattern_kill.is_empty() || !this->pattern_notify.is_empty();
    }

    bool kbd_input(MonotonicTimePoint /*now*/, uint32_t uchar) override {
        bool can_be_sent_to_server = true;

        filtering_kbd_input(
            uchar,
            [this, &can_be_sent_to_server](ZStrUtf8Char utf8_char) {
                if (utf8_char.size() > 0) {
                    if (!this->pattern_kill.is_empty()) {
                        can_be_sent_to_server &= !this->test_pattern(
                            utf8_char, this->pattern_kill, true
                        );
                    }
                    if (!this->pattern_notify.is_empty()) {
                        this->test_pattern(utf8_char, this->pattern_notify, false);
                    }
                }
            },
            [this](chars_view const & /*noprintable_char*/) {
                this->pattern_kill.rewind_search();
                this->pattern_notify.rewind_search();
            },
            nofilter_slash{}
        );

        return can_be_sent_to_server;
    }

    void enable_kbd_input_mask(bool /*enable*/) override {
    }

private:
    bool test_pattern(
        ZStrUtf8Char const& utf8_char,
        PatternSearcher & searcher, bool is_pattern_kill
    ) {
        return searcher.test_uchar(
            utf8_char,
            [&, this](std::string const & pattern, char const * str) {
                assert(this->session_log);
                utils::MatchFinder::report(
                    *this->session_log,
                    is_pattern_kill,
                    utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                    pattern.c_str(),
                    str
                );
            }
        );
    }
};
REDEMPTION_DIAGNOSTIC_POP()

namespace
{
    const auto shadow_kbd_char = "********"_av;

    struct KbdBuffer
    {
        KbdBuffer(writable_buffer_view buffer)
        : kbd_stream(buffer)
        {}

        template<class Flusher>
        void enable_kbd_input_mask(bool enable, Flusher&& flusher)
        {
            if (this->keyboard_input_mask_enabled != enable) {
                this->flush(flusher);
                this->keyboard_input_mask_enabled = enable;
            }
        }

        template<class Flusher>
        bool kbd_input(uint32_t uchar, bool writable, Flusher&& flusher)
        {
            if (this->keyboard_input_mask_enabled) {
                if (writable) {
                    this->write_shadow_keys(flusher);
                }
            }
            else {
                this->write_char(uchar, flusher);
            }
            return true;
        }

        template<class Flusher>
        void flush(Flusher&& flusher)
        {
            if (this->kbd_stream.get_offset() || this->hidden_masked_char) {
                if (this->hidden_masked_char) {
                    this->kbd_stream.out_copy_bytes(shadow_kbd_char);
                }
                this->hidden_masked_char = false;
                flusher(this->kbd_stream.get_produced_bytes());
                this->kbd_stream.rewind();
            }
        }

        size_t size() const
        {
            return this->kbd_stream.get_offset();
        }

    private:
        template<class Flusher>
        void write_shadow_keys(Flusher& flusher)
        {
            if (!this->kbd_stream.has_room(shadow_kbd_char.size())) {
                this->flush(flusher);
            }
            this->hidden_masked_char = true;
        }

        template<class Flusher>
        void write_char(uint32_t uchar, Flusher& flusher)
        {
            auto copy_bytes = [this, &flusher](bytes_view bytes) {
                if (this->kbd_stream.tailroom() < bytes.size()) {
                    this->flush(flusher);
                }
                this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
            };

            filtering_kbd_input(
                uchar,
                [copy_bytes](uint32_t uchar) {
                    uint8_t buf_char[5];
                    if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                        copy_bytes({buf_char, char_len});
                    }
                },
                copy_bytes,
                filter_slash{}
            );
        }

        OutStream kbd_stream;
        bool hidden_masked_char = false;
        bool keyboard_input_mask_enabled = false;
    };
} // anonymous namespace

REDEMPTION_DIAGNOSTIC_PUSH()
// 'KbdBuffer kbd_buffer' is a type uses the anonymous namespace
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wsubobject-linkage")
class Capture::SyslogKbd final : public gdi::KbdInputApi, public gdi::CaptureApi
{
    uint8_t buffer[1024];
    KbdBuffer kbd_buffer;
    MonotonicTimePoint last_snapshot;

    static auto _flusher()
    {
        return [](bytes_view data){
            LOG(LOG_INFO, R"x(type="KBD input" data="%.*s")x",
                int(data.size()), data.as_charp());
        };
    }

public:
    explicit SyslogKbd(MonotonicTimePoint now)
    : kbd_buffer(this->buffer)
    , last_snapshot(now)
    {}

    ~SyslogKbd()
    {
        this->kbd_buffer.flush(_flusher());
    }

    void enable_kbd_input_mask(bool enable) override
    {
        this->kbd_buffer.enable_kbd_input_mask(enable, _flusher());
    }

    bool kbd_input(MonotonicTimePoint /*now*/, uint32_t uchar) override
    {
        this->kbd_buffer.kbd_input(uchar, true, _flusher());
        return true;
    }

private:
    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/
    ) override {
        MonotonicTimePoint::duration const time_to_wait = 2s;

        if (this->last_snapshot + time_to_wait >= now || this->kbd_buffer.size() >= 8 * sizeof(uint32_t)) {
            this->kbd_buffer.flush(_flusher());
            this->last_snapshot = now;
        }

        return WaitingTimeBeforeNextSnapshot(time_to_wait);
    }
};


class Capture::SessionLogKbd final : public gdi::KbdInputApi, public gdi::CaptureProbeApi
{
    uint8_t buffer[64];
    KbdBuffer kbd_buffer;
    bool is_probe_enabled_session = false;
    SessionLogApi& session_log;

    auto _flusher()
    {
        return [this](bytes_view data){
            this->session_log.log6(LogId::KBD_INPUT, {
                KVLog("data"_av, data.as_chars()),
            });
        };
    }

public:
    explicit SessionLogKbd(SessionLogApi& session_log)
    : kbd_buffer(this->buffer)
    , session_log(session_log)
    {}

    ~SessionLogKbd()
    {
        this->kbd_buffer.flush(this->_flusher());
    }

    bool kbd_input(MonotonicTimePoint /*now*/, uint32_t uchar) override
    {
        this->kbd_buffer.kbd_input(uchar, this->is_probe_enabled_session, _flusher());
        return true;
    }

    void enable_kbd_input_mask(bool enable) override
    {
        this->kbd_buffer.enable_kbd_input_mask(enable, _flusher());
    }

    void session_update(MonotonicTimePoint /*now*/, LogId id, KVLogList kv_list) override
    {
        update_enable_probe(this->is_probe_enabled_session, id, kv_list);
    }

    void possible_active_window_change() override
    {
        this->kbd_buffer.flush(this->_flusher());
    }
};
REDEMPTION_DIAGNOSTIC_POP()


class Capture::PatternsChecker : ::noncopyable
{
    utils::MatchFinder::NamedRegexArray regexes_filter_kill;
    utils::MatchFinder::NamedRegexArray regexes_filter_notify;
    SessionLogApi& session_log;

public:
    explicit PatternsChecker(SessionLogApi& session_log, PatternParams const & params)
    : session_log(session_log)
    {
        auto without_capture = utils::MatchFinder::WithCapture(false);
        utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
            params.pattern_kill, this->regexes_filter_kill, params.verbose, without_capture);

        utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
            params.pattern_notify, this->regexes_filter_notify, params.verbose, without_capture);
    }

    [[nodiscard]] bool contains_pattern() const {
        return !this->regexes_filter_kill.empty() || !this->regexes_filter_notify.empty();
    }

    void title_changed(chars_view str) {
        assert(str.data() && not str.empty());
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
                    utils::MatchFinder::report(this->session_log,
                        &regexes_filter == &this->regexes_filter_kill, // pattern_kill = FINDPATTERN_KILL
                        utils::MatchFinder::ConfigureRegexes::OCR,
                        first->name.c_str(), str);
                }
            }
        }
    }
};


class Capture::PngCapture : public gdi::CaptureApi
{
protected:
    OutFilenameSequenceTransport trans;
    Drawable & drawable;
    DrawablePointer const & drawable_pointer;
    MonotonicTimePoint last_time_capture;
    const std::chrono::microseconds frame_interval;

private:
    const MonotonicTimeToRealTime monotonic_to_real;
    const ScaledPng24 scaled_png;

protected:
    TimestampTracer timestamp_tracer;

protected:
    gdi::ImageFrameApi & image_frame_api;

    explicit PngCapture(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable & drawable, DrawablePointer const & drawable_pointer,
        gdi::ImageFrameApi & imageFrameApi, WritableImageView const & image_view)
    : trans(
        capture_params.record_tmp_path,
        png_params.real_basename,
        ".png",
        capture_params.groupid,
        [session_log = capture_params.session_log](const Error & error){
            if (session_log && error.errnum == ENOSPC) {
                // error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
                session_log->report("FILESYSTEM_FULL", "100|unknown");
            }
        })
    , drawable(drawable)
    , drawable_pointer(drawable_pointer)
    , last_time_capture(capture_params.now)
    , frame_interval(png_params.png_interval)
    , monotonic_to_real(capture_params.now, capture_params.real_now)
    , scaled_png{png_params.png_width, png_params.png_height}
    , timestamp_tracer(image_view)
    , image_frame_api(imageFrameApi)
    {}

    void resize(WritableImageView const & image_view) {
        this->timestamp_tracer = TimestampTracer(image_view);
    }

public:
    PngCapture(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable & drawable, DrawablePointer const & drawable_pointer,
        gdi::ImageFrameApi & imageFrameApi)
    : PngCapture(
        capture_params, png_params, drawable, drawable_pointer,
        imageFrameApi, imageFrameApi.get_writable_image_view())
    {}

    void resize(gdi::ImageFrameApi & imageFrameApi) {
        this->resize(imageFrameApi.get_writable_image_view());
    }

    void dump()
    {
        this->scaled_png.dump_png24(this->trans, this->image_frame_api, true);
    }

     virtual void clear_old() {}

     void clear_png_interval(uint32_t num_start, uint32_t num_end) {
        for(uint32_t num = num_start ; num < num_end ; num++) {
            // unlink may fail, for instance if file does not exist, just don't care
            ::unlink(this->trans.seqgen(num));
        }
     }

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*x*/, uint16_t /*y*/
    ) override {
        auto const duration = now - this->last_time_capture;
        std::chrono::microseconds const interval = this->frame_interval;
        if (duration >= interval) {
             // Snapshot at end of Frame or force snapshot if diff_time_val >= 1.5 x frame_interval.
            if (this->drawable.logical_frame_ended || (duration >= interval * 3 / 2)) {
                DrawablePointer::BufferSaver buffer_saver;
                this->drawable_pointer.trace_mouse(this->drawable, buffer_saver);
                tm ptm;
                time_t t = to_time_t(now, this->monotonic_to_real);
                localtime_r(&t, &ptm);
                this->image_frame_api.prepare_image_frame();
                this->timestamp_tracer.trace(ptm);

                this->dump();
                this->clear_old();
                this->trans.next();

                this->timestamp_tracer.clear();
                this->last_time_capture = now;
                this->drawable_pointer.clear_mouse(this->drawable, buffer_saver);

                return WaitingTimeBeforeNextSnapshot(interval.count()
                    ? interval - duration % interval
                    : interval);
            }
            // Wait 0.3 x frame_interval.
            return WaitingTimeBeforeNextSnapshot(this->frame_interval / 3);
        }
        return WaitingTimeBeforeNextSnapshot(interval - duration);
    }
};

class Capture::PngCaptureRT : public PngCapture
{
    uint32_t num_start;
    unsigned png_limit;

    bool enable_rt_display;

    SmartVideoCropping smart_video_cropping;

public:
    explicit PngCaptureRT(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable & drawable, DrawablePointer const & drawable_pointer,
        gdi::ImageFrameApi & imageFrameApi)
    : PngCapture(capture_params, png_params, drawable, drawable_pointer, imageFrameApi)
    , num_start(this->trans.get_seqno())
    , png_limit(png_params.png_limit)
    , enable_rt_display(png_params.rt_display)
    , smart_video_cropping(capture_params.smart_video_cropping)
    {}

    ~PngCaptureRT() /*NOLINT*/
    {
        this->clear_png_interval(this->num_start, this->trans.get_seqno() + 1);
    }

    Capture::RTDisplayResult set_rt_display(bool enable_rt_display) {
        if (enable_rt_display != this->enable_rt_display){
            LOG(LOG_INFO, "PngCaptureRT::enable_rt_display=%d", enable_rt_display);
            this->enable_rt_display = enable_rt_display;
            // clear files if we go from RT to non-RT
            if (!this->enable_rt_display) {
                this->clear_png_interval(this->num_start, this->trans.get_seqno() + 1);
                return Capture::RTDisplayResult::Disabled;
            }
            return Capture::RTDisplayResult::Enabled;
        }

        return Capture::RTDisplayResult::Unchanged;
    }

    void clear_old() override {
        if (this->trans.get_seqno() < this->png_limit) {
            return;
        }
        uint32_t num_start = this->trans.get_seqno() - this->png_limit;
        this->clear_png_interval(num_start, num_start + 1);
    }

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t x, uint16_t y
    ) override {
        if (this->enable_rt_display) {
            return this->PngCapture::periodic_snapshot(now, x, y);
        }
        auto const duration = now - this->last_time_capture;
        return WaitingTimeBeforeNextSnapshot(this->frame_interval - duration % this->frame_interval);
    }

    void visibility_rects_event(Rect rect)
    {
        if ((this->smart_video_cropping != SmartVideoCropping::disable) && !rect.isempty()) {
            rect = rect.intersect(
                {0, 0, this->drawable.width(), this->drawable.height()});



            bool     right         = true;
            unsigned failure_count = 0;
            while ((rect.cx & 3) && (failure_count < 2)) {
                if (right) {
                    if (rect.x + rect.cx < this->drawable.width()) {
                        rect.cx += 1;

                        failure_count = 0;
                    }
                    else {
                        failure_count++;
                    }

                    right = false;
                }
                else {
                    if (rect.x > 0) {
                        rect.x -=1;
                        rect.cx += 1;

                        failure_count = 0;
                    }
                    else {
                        failure_count++;
                    }

                    right = true;
                }
            }
            if (rect.cx & 3) {
                rect.cx &= ~ 3;
            }



            if (this->image_frame_api.reset(rect.x, rect.y, rect.cx, rect.cy)) {
                this->timestamp_tracer = TimestampTracer(this->image_frame_api.get_writable_image_view());
            }
        }
    }
};

namespace {

bool is_logable_kvlist(LogId id, KVLogList kv_list, MetaParams meta_params)
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch")
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (detail::log_id_category_map[underlying_cast(id)]) {
        case LogCategoryId::Drive:
            if (!bool(meta_params.log_file_system_activities)) {
                return false;
            }
            break;

        case LogCategoryId::Clipboard: {
            if (!bool(meta_params.log_clipboard_activities)) {
                return false;
            }

            if (bool(meta_params.log_only_relevant_clipboard_activities)) {
                auto is = [&](chars_view format){
                    return format.size() > kv_list[0].value.size()
                        && 0 == strncasecmp(format.data(), kv_list[0].value.data(), format.size());
                };
                switch (id)
                {
                    case LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION:
                    case LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION:
                    case LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX:
                    case LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX:
                        if (not kv_list.empty()
                         && (is("Preferred DropEffect("_av)
                          || is("FileGroupDescriptorW("_av))
                        ) {
                            return false;
                        }
                        break;
                    default:;
                }
            }
        }
        break;
    }
    REDEMPTION_DIAGNOSTIC_POP()

    return true;
}

} // anonymous namespace

namespace {
    constexpr chars_view session_meta_kbd_prefix() noexcept { return cstr_array_view("[Kbd]"); }
    constexpr chars_view session_meta_kbd_suffix() noexcept { return cstr_array_view("\n"); }
}

/*
* Format:
*
* $date " - [Kbd]" $kbd
* $date " + " $title
* $date " - " $line
* $date " + " (break)
*
* Info:
*
* + for new video file
*/
class Capture::SessionMeta final : public gdi::KbdInputApi, public gdi::CaptureApi, public gdi::CaptureProbeApi
{
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    uint8_t kbd_buffer[512];
    static const std::size_t kbd_buffer_usable_char =
        sizeof(kbd_buffer) - session_meta_kbd_prefix().size() - session_meta_kbd_suffix().size();
    uint8_t kbd_chars_size[kbd_buffer_usable_char];
    std::ptrdiff_t kbd_char_pos = 0;
    MonotonicTimeToRealTime monotonic_to_real;
    MonotonicTimePoint monotonic_last_time;
    Transport & trans;
    bool is_probe_enabled_session = false;
    bool previous_char_is_event_flush = false;
    const bool key_markers_hidden_state;
    const MetaParams meta_params;

    int hidden_masked_char_count = 0;

public:
    explicit SessionMeta(
        MonotonicTimePoint now,
        RealTimePoint real_now,
        Transport& trans,
        bool key_markers_hidden_state,
        MetaParams meta_params)
    : kbd_stream{{this->kbd_buffer + session_meta_kbd_prefix().size(), kbd_buffer_usable_char}}
    , monotonic_to_real(now, real_now)
    , monotonic_last_time(now)
    , trans(trans)
    , key_markers_hidden_state(key_markers_hidden_state)
    , meta_params(meta_params)
    {
        memcpy(this->kbd_buffer, session_meta_kbd_prefix().data(), session_meta_kbd_prefix().size());

        // force file creation even if no text recognized
        this->trans.send("", 0);
    }

    ~SessionMeta() {
        this->send_kbd();
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->send_kbd();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    bool kbd_input(MonotonicTimePoint now, uint32_t uchar) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys();
                this->send_kbd_if_special_char(uchar);
            }
        }
        else {
            this->write_keys(uchar);
            this->send_kbd_if_special_char(uchar);
        }
        this->monotonic_last_time = now;
        return true;
    }

    void send_line(MonotonicTimePoint now, chars_view data) {
        this->send_data(now, data, '-');
    }

    void next_video(MonotonicTimePoint now) {
        this->send_data(now, cstr_array_view("(break)"), '+');
    }

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
    {
        this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
    }

private:
    // buffer for log_format_set_info
    std::string formatted_message;

public:
    void title_changed(MonotonicTimePoint now, chars_view title) {
        log_format_set_info(this->formatted_message, LogId::TITLE_BAR, {
            KVLog("data"_av, title),
        });
        this->send_data(now, this->formatted_message, '+');
    }

    void session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list) override {
        if (!update_enable_probe(this->is_probe_enabled_session, id, kv_list)
          && is_logable_kvlist(id, kv_list, this->meta_params)
        ) {
            log_format_set_info(this->formatted_message, id, kv_list);
            this->send_line(now, this->formatted_message);
        }
    }

    void possible_active_window_change() override {
        this->send_kbd();
        this->previous_char_is_event_flush = true;
    }

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/
    ) override {
        this->monotonic_last_time = now;
        return WaitingTimeBeforeNextSnapshot(10s);
    }

private:
    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->send_kbd();
        }
        this->hidden_masked_char_count++;
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                    this->kbd_chars_size[this->kbd_char_pos] = char_len;
                    ++this->kbd_char_pos;
                }
            },
            [this, uchar](chars_view no_printable_str) {
                if (uchar == 0x08 && this->kbd_char_pos) {
                    --this->kbd_char_pos;
                    this->kbd_stream.rewind(
                        this->kbd_stream.get_offset()
                      - this->kbd_chars_size[this->kbd_char_pos]
                    );
                }
                else if (this->key_markers_hidden_state) {
                    if (uchar == '/') {
                        auto const single_slash = cstr_array_view("/");
                        this->copy_bytes(single_slash);
                        this->kbd_chars_size[this->kbd_char_pos] = single_slash.size();
                        ++this->kbd_char_pos;
                    }
                    else {
                        this->kbd_char_pos = 0;
                    }
                }
                else if (uchar == '/') {
                    this->copy_bytes(no_printable_str);
                    this->kbd_chars_size[this->kbd_char_pos] = no_printable_str.size();
                    ++this->kbd_char_pos;
                }
                else {
                    this->copy_bytes(no_printable_str);
                    this->kbd_char_pos = 0;
                }
            },
            filter_slash{}
        );
    }

    void copy_bytes(bytes_view bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->send_kbd();
        }
        this->kbd_stream.out_copy_bytes(bytes);
    }

    void send_data(MonotonicTimePoint now, chars_view data, char sep) {
        this->send_date(now, sep);
        this->trans.send(data);
        this->trans.send("\n"_av);
        this->monotonic_last_time = now;
    }

    void send_date(MonotonicTimePoint now, char sep) {
        auto rawtime = to_time_t(now, this->monotonic_to_real);

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

    void send_kbd_if_special_char(uint32_t uchar) {
        if (uchar == '\r' || uchar == '\t') {
            if (!this->previous_char_is_event_flush) {
                this->send_kbd();
            }
        }
        else {
            this->previous_char_is_event_flush = false;
        }
    }

    void send_kbd() {
          if (this->kbd_stream.get_offset() || (0!=this->hidden_masked_char_count)) {
            if (this->hidden_masked_char_count) {
                this->kbd_stream.out_copy_bytes("********"_av);
            }
            this->hidden_masked_char_count = 0;
            log_format_set_info(this->formatted_message, LogId::KBD_INPUT, {
                KVLog("data"_av, this->kbd_stream.get_produced_bytes().as_chars()),
            });
            this->send_data(this->monotonic_last_time, this->formatted_message, '-');
            this->kbd_stream.rewind();
            this->previous_char_is_event_flush = true;
            this->kbd_char_pos = 0;
        }
    }
};


class Capture::SessionLogAgent : public gdi::CaptureProbeApi
{
    std::string formatted_message;
    SessionMeta & session_meta;

    MetaParams meta_params;

public:
    explicit SessionLogAgent(SessionMeta & session_meta, MetaParams meta_params)
    : session_meta(session_meta)
    , meta_params(meta_params)
    {}

    void session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list) override {
        if (id != LogId::PROBE_STATUS && is_logable_kvlist(id, kv_list, this->meta_params)) {
            log_format_set_info(this->formatted_message, id, kv_list);
            this->session_meta.send_line(now, this->formatted_message);
        }
    }

    void possible_active_window_change() override {
    }
};


class Capture::MetaCaptureImpl
{
    OutFileTransport meta_trans;

public:
    SessionMeta meta;
    SessionLogAgent session_log_agent;
    const bool enable_agent;

    explicit MetaCaptureImpl(CaptureParams const& capture_params, MetaParams const & meta_params)
    : meta_trans(unique_fd{[&](){
        std::string record_path = str_concat(
            capture_params.record_tmp_path,
            capture_params.basename,
            ".meta");
        const char * filename = record_path.c_str();
        int const file_mode = capture_params.groupid ? (S_IRUSR|S_IRGRP) : S_IRUSR;
        int fd = ::open(filename, O_CREAT | O_TRUNC | O_WRONLY, file_mode);
        // umask (man umask) can change effective mode of created file
        if ((fd < 0) || (chmod(filename, file_mode) == -1)) {
            int const errnum = errno;
            if (fd < 0) {
                LOG(LOG_ERR, "can't open meta file %s: %s [%d]", filename, strerror(errnum), errnum);
            }
            else {
                LOG(LOG_ERR, "can't change mod of meta file %s: %s [%d]", filename, strerror(errnum), errnum);
            }
            Error error(ERR_TRANSPORT_OPEN_FAILED, errnum);
            if (capture_params.session_log && error.errnum == ENOSPC) {
                error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
                // ReportMessageReporter
                capture_params.session_log->report("FILESYSTEM_FULL", "100|unknown");
            }
            throw error; /* NOLINT */
        }
        return fd;
    }()}, [session_log = capture_params.session_log](const Error & error){
        if (session_log && error.errnum == ENOSPC) {
            // error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
            session_log->report("FILESYSTEM_FULL", "100|unknown");
        }
    })
    , meta(capture_params.now, capture_params.real_now, this->meta_trans, underlying_cast(meta_params.hide_non_printable), meta_params)
    , session_log_agent(this->meta, meta_params)
    , enable_agent(underlying_cast(meta_params.enable_session_log))
    {
    }
};


class Capture::TitleCaptureImpl : public gdi::CaptureApi, public gdi::CaptureProbeApi
{
    bool enable_probe = false;
    OcrTitleExtractorBuilder ocr_title_extractor_builder;
    AgentTitleExtractor agent_title_extractor;

    Ref<TitleExtractorApi> title_extractor;

    MonotonicTimePoint last_ocr;
    std::chrono::microseconds usec_ocr_interval;

    NotifyTitleChanged & notify_title_changed;

    SessionLogApi * session_log;
public:
    explicit TitleCaptureImpl(
        MonotonicTimePoint now,
        RDPDrawable & drawable,
        OcrParams ocr_params,
        NotifyTitleChanged & notify_title_changed,
        SessionLogApi * session_log)
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
    , session_log(session_log)
    {
    }


    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/
    ) override {
        auto const diff {now - this->last_ocr};

        if (diff >= this->usec_ocr_interval) {
            this->last_ocr = now;

            chars_view title = this->title_extractor.get().extract_title();

            if (title.data()/* && title.size()*/) {
                notify_title_changed.notify_title_changed(now, title);
                if (&this->title_extractor.get() != &this->agent_title_extractor
                 && this->session_log)
                {
                    this->session_log->log6(LogId::TITLE_BAR, {
                        KVLog("source"_av, "OCR"_av),
                        KVLog("window"_av, title),
                    });
                }
            }

            return WaitingTimeBeforeNextSnapshot(this->usec_ocr_interval);
        }

        return WaitingTimeBeforeNextSnapshot(this->usec_ocr_interval - diff);
    }

    void session_update(MonotonicTimePoint /*now*/, LogId id, KVLogList kv_list) override {
        update_enable_probe(this->enable_probe, id, kv_list);
        if (enable_probe) {
            this->title_extractor = this->agent_title_extractor;
            this->agent_title_extractor.session_update(id, kv_list);
        }
        else {
            this->title_extractor = this->ocr_title_extractor_builder.get_title_extractor();
        }
    }

    void possible_active_window_change() override {}
};


void Capture::NotifyTitleChanged::notify_title_changed(
    MonotonicTimePoint now, chars_view title
) {
    if (this->capture.patterns_checker) {
        this->capture.patterns_checker->title_changed(title);
    }
    if (this->capture.meta_capture_obj) {
        this->capture.meta_capture_obj->meta.title_changed(now, title);
    }
#ifndef REDEMPTION_NO_FFMPEG
    if (this->capture.sequenced_video_capture_obj) {
        this->capture.sequenced_video_capture_obj->next_video(now);
    }
#endif
    if (this->capture.update_progress_data) {
        this->capture.update_progress_data->next_video(now);
    }
}

void Capture::NotifyMetaIfNextVideo::notify_next_video(
    MonotonicTimePoint now, NotifyNextVideo::Reason reason
) {
    assert(this->session_meta);
    if (reason == NotifyNextVideo::Reason::sequenced) {
        this->session_meta->next_video(now);
    }
}


Capture::Capture(
    const CaptureParams& capture_params,
    const DrawableParams& drawable_params,
    bool capture_wrm, const WrmParams& wrm_params,
    bool capture_png, const PngParams& png_params,
    bool capture_pattern_checker, const PatternParams& pattern_params,
    bool capture_ocr, const OcrParams& ocr_params,
    bool capture_video, const SequencedVideoParams& sequenced_video_params,
    bool capture_video_full, const FullVideoParams& full_video_params,
    bool capture_meta, const MetaParams& meta_params,
    bool capture_kbd, const KbdLogParams& kbd_log_params,
    const VideoParams& video_params,
    UpdateProgressData * update_progress_data,
    Rect const & crop_rect,
    Rect const & rail_window_rect)
: gd_drawable(drawable_params.rdp_drawable)
, ptr_cache(drawable_params.ptr_cache)
, update_progress_data(update_progress_data)
, mouse_info{
    capture_params.now,
    static_cast<uint16_t>(drawable_params.rdp_drawable.width() / 2),
    static_cast<uint16_t>(drawable_params.rdp_drawable.height() / 2)}
, capture_drawable(capture_wrm || capture_video || capture_ocr || capture_png || capture_video_full)
, smart_video_cropping(capture_params.smart_video_cropping)
, rail_screen_visibility(capture_params.verbose)
{
    LOG(LOG_INFO,
        "Enable capture:  wrm=%s  png=%s  kbd=%s  video=%s  video_full=%s  pattern=%s  ocr=%s  meta=%s",
        capture_wrm ? "yes" : "no",
        capture_png ? "yes" : "no",
        capture_kbd ? "yes" : "no",
        capture_video ? "yes" : "no",
        capture_video_full ? "yes" : "no",
        capture_pattern_checker ? "yes" : "no",
        capture_ocr ? (ocr_params.ocr_version == OcrVersion::v2 ? "v2 " : "v1 ") : "no",
        capture_meta ? "yes" : "no"
    );

    if (capture_png || (capture_params.session_log && (capture_video || capture_ocr))) {
        if (recursive_create_directory(capture_params.record_tmp_path,
                S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP, -1) != 0) {
            LOG(LOG_INFO, "Failed to create directory: \"%s\"", capture_params.record_tmp_path);
        }
    }

    if (capture_meta) {
        this->meta_capture_obj = std::make_unique<MetaCaptureImpl>(capture_params, meta_params);
    }

    if (this->capture_drawable) {
        this->gds.emplace_back(this->gd_drawable);

        this->drawable_pointer = std::make_unique<DrawablePointer>(normal_pointer());
        drawable_pointer->set_position(this->gd_drawable.width() / 2,
                                       this->gd_drawable.height() / 2);

        not_null_ptr<gdi::ImageFrameApi> image_frame_api_ptr = &this->gd_drawable;

        if (!crop_rect.isempty()
         && ( (capture_png && !png_params.real_time_image_capture)
           || capture_video
           || capture_video_full
         )
        ) {
            this->video_cropper = std::make_unique<VideoCropper>(
                this->gd_drawable,
                crop_rect.x,
                crop_rect.y,
                crop_rect.cx,
                crop_rect.cy
            );

            image_frame_api_ptr = this->video_cropper.get();
        }

        if (capture_png) {
            if (png_params.real_time_image_capture) {
                not_null_ptr<gdi::ImageFrameApi> image_frame_api_real_time_ptr = &this->gd_drawable;

                if (png_params.remote_program_session) {
                    this->video_cropper_real_time = std::make_unique<VideoCropper>(
                        this->gd_drawable,
                        0,
                        0,
                        this->gd_drawable.width(),
                        this->gd_drawable.height()
                    );

                    image_frame_api_real_time_ptr = this->video_cropper_real_time.get();
                }
                this->png_real_time_capture_obj =
                    std::make_unique<PngCaptureRT>(capture_params,
                                                   png_params,
                                                   this->gd_drawable.impl(),
                                                   *this->drawable_pointer,
                                                   *image_frame_api_real_time_ptr);
            }
            else {
                this->png_capture_obj =
                    std::make_unique<PngCapture>(capture_params,
                                                 png_params,
                                                 this->gd_drawable.impl(),
                                                 *this->drawable_pointer,
                                                 *image_frame_api_ptr);
            }
        }
        if (capture_wrm) {
            this->wrm_capture_obj = std::make_unique<WrmCaptureImpl>(
                capture_params, wrm_params, this->gd_drawable, rail_window_rect,
                drawable_params.ptr_cache);
        }

#ifndef REDEMPTION_NO_FFMPEG
        if (capture_video) {
            Ref<NotifyNextVideo> notifier = this->null_notifier_next_video;
            if (this->meta_capture_obj) {
                this->notifier_next_video.session_meta = &this->meta_capture_obj->meta;
                notifier = this->notifier_next_video;
            }
            this->sequenced_video_capture_obj = std::make_unique<SequencedVideoCaptureImpl>(
                capture_params, png_params.png_width, png_params.png_height,
                this->gd_drawable.impl(), *this->drawable_pointer,
                *image_frame_api_ptr, video_params, sequenced_video_params, notifier);
        }

        if (capture_video_full) {
            this->full_video_capture_obj = std::make_unique<FullVideoCaptureImpl>(
                capture_params, this->gd_drawable.impl(), *this->drawable_pointer,
                *image_frame_api_ptr, video_params, full_video_params);
        }
#else
        if (capture_video || capture_video_full) {
            (void)full_video_params;
            (void)video_params;
            LOG(LOG_WARNING, "VideoCapture is disabled (-DREDEMPTION_NO_FFMPEG)");
        }
#endif

        if (capture_pattern_checker) {
            this->patterns_checker = std::make_unique<PatternsChecker>(
                *capture_params.session_log, pattern_params);
            if (!this->patterns_checker->contains_pattern()) {
                LOG(LOG_WARNING, "Disable pattern_checker");
                this->patterns_checker.reset();
            }
        }

        if (capture_ocr) {
            if (this->patterns_checker || this->meta_capture_obj || this->sequenced_video_capture_obj) {
                this->title_capture_obj = std::make_unique<TitleCaptureImpl>(
                    capture_params.now, this->gd_drawable, ocr_params,
                    this->notifier_title_changed, capture_params.session_log
                );
            }
            else {
                LOG(LOG_INFO, "Disable title_extractor");
            }
        }

        if (capture_wrm) {
            this->gds.emplace_back(*this->wrm_capture_obj);
            if (kbd_log_params.wrm_keyboard_log) {
                this->kbds.emplace_back(*this->wrm_capture_obj);
            }
            this->caps.emplace_back(*this->wrm_capture_obj);
            this->ext_caps.emplace_back(*this->wrm_capture_obj);
            this->probes.emplace_back(*this->wrm_capture_obj);
        }

        if (this->png_real_time_capture_obj) {
            this->caps.emplace_back(*this->png_real_time_capture_obj);
        }

        if (this->png_capture_obj) {
            this->caps.emplace_back(*this->png_capture_obj);
        }
#ifndef REDEMPTION_NO_FFMPEG
        if (this->sequenced_video_capture_obj) {
            this->caps.emplace_back(*this->sequenced_video_capture_obj);
            this->gds.emplace_back(this->sequenced_video_capture_obj->graphics_api());
        }

        if (this->full_video_capture_obj) {
            this->caps.emplace_back(*this->full_video_capture_obj);
            this->gds.emplace_back(this->full_video_capture_obj->graphics_api());
        }
#endif
    }

    if (capture_kbd) {
        if (kbd_log_params.syslog_keyboard_log) {
            this->syslog_kbd_capture_obj = std::make_unique<SyslogKbd>(capture_params.now);
            this->kbds.emplace_back(*this->syslog_kbd_capture_obj);
            this->caps.emplace_back(*this->syslog_kbd_capture_obj);
        }

        if (kbd_log_params.session_log_enabled) {
            this->session_log_kbd_capture_obj = std::make_unique<SessionLogKbd>(
                *capture_params.session_log);
            this->kbds.emplace_back(*this->session_log_kbd_capture_obj);
            this->probes.emplace_back(*this->session_log_kbd_capture_obj);
        }

        this->pattern_kbd_capture_obj = std::make_unique<PatternKbd>(
            capture_params.session_log,
            pattern_params.pattern_kill,
            pattern_params.pattern_notify,
            pattern_params.verbose);

        if (this->pattern_kbd_capture_obj->contains_pattern()) {
            this->kbds.emplace_back(*this->pattern_kbd_capture_obj);
        }
        else {
            this->pattern_kbd_capture_obj.reset();
        }
    }

    if (this->meta_capture_obj) {
        this->caps.emplace_back(this->meta_capture_obj->meta);
        this->probes.emplace_back(this->meta_capture_obj->meta);
        if (kbd_log_params.meta_keyboard_log) {
            this->kbds.emplace_back(this->meta_capture_obj->meta);
        }
        else if (this->meta_capture_obj->enable_agent) {
            // meta includes session_log_agent
            this->probes.emplace_back(this->meta_capture_obj->session_log_agent);
        }
    }

    if (this->title_capture_obj) {
        this->caps.emplace_back(*this->title_capture_obj);
        this->probes.emplace_back(*this->title_capture_obj);
    }

    if (!rail_window_rect.isempty()) {
        this->visibility_rects_event(rail_window_rect);
    }
}

Capture::~Capture()
{
    this->title_capture_obj.reset();
    this->session_log_kbd_capture_obj.reset();
    this->syslog_kbd_capture_obj.reset();
    this->pattern_kbd_capture_obj.reset();
    this->png_capture_obj.reset();
    this->png_real_time_capture_obj.reset();
    this->wrm_capture_obj.reset();

    if (this->sequenced_video_capture_obj) {
        try {
            this->sequenced_video_capture_obj.reset();
        }
        catch (Error const & e) {
            LOG(LOG_ERR, "Sequenced video: last encoding video frame error: %s", e.errmsg());
        }
    }
    if (this->full_video_capture_obj) {
        try {
            this->full_video_capture_obj.reset();
        }
        catch (Error const & e) {
            LOG(LOG_ERR, "Full video: last encoding video frame error: %s", e.errmsg());
        }
    }
}

void Capture::relayout(MonitorLayoutPDU const & monitor_layout_pdu) {
    if (this->wrm_capture_obj) {
        this->wrm_capture_obj->relayout(monitor_layout_pdu);
    }
}

void Capture::force_flush(MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    if (this->drawable_pointer) {
        this->drawable_pointer->set_position(cursor_x, cursor_y);
    }
    this->mouse_info = {now, cursor_x, cursor_y};

    if (this->wrm_capture_obj) {
        this->wrm_capture_obj->update_mouse_position(cursor_x, cursor_y);
        this->wrm_capture_obj->send_timestamp_chunk(now);
    }
}

void Capture::update_mouse(MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    if (cursor_x != this->mouse_info.last_x || cursor_y != this->mouse_info.last_y) {
        this->force_flush(now, cursor_x, cursor_y);
    }
}

void Capture::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    if (this->wrm_capture_obj) {
        this->wrm_capture_obj->synchronize_times(monotonic_time, real_time);
    }

#ifndef REDEMPTION_NO_FFMPEG
    if (this->sequenced_video_capture_obj) {
        this->sequenced_video_capture_obj->synchronize_times(monotonic_time, real_time);
    }

    if (this->full_video_capture_obj) {
        this->full_video_capture_obj->synchronize_times(monotonic_time, real_time);
    }
#endif

    if (this->meta_capture_obj) {
        this->meta_capture_obj->meta.synchronize_times(monotonic_time, real_time);
    }
}

void Capture::resize(uint16_t width, uint16_t height)
{
    if (this->sequenced_video_capture_obj || this->full_video_capture_obj) {
        if (this->gd_drawable.width() != width || this->gd_drawable.height() != height) {
            gdi::subrect4_t subrect4 = gdi::subrect4(
                Rect(0, 0, this->gd_drawable.width(), this->gd_drawable.height()),
                Rect(0, 0, width, height)
            );
            for (Rect const & rect : subrect4) {
                if (!rect.isempty()) {
                    this->gd_drawable.draw(
                        RDPOpaqueRect(rect, encode_color24()(BLACK)),
                        rect, gdi::ColorCtx::depth24()
                    );
                }
            }
        }

        return ;
    }

    if (this->capture_drawable) {
        this->gd_drawable.resize(width, height);
    }

    if (this->png_real_time_capture_obj) {
        not_null_ptr<gdi::ImageFrameApi> image_frame_api_real_time_ptr = &this->gd_drawable;

        if (this->video_cropper_real_time) {
            this->video_cropper_real_time->resize(this->gd_drawable);

            image_frame_api_real_time_ptr = this->video_cropper_real_time.get();
        }

        this->png_real_time_capture_obj->resize(*image_frame_api_real_time_ptr);
    }

    if (this->png_capture_obj) {
        not_null_ptr<gdi::ImageFrameApi> image_frame_api_ptr = &this->gd_drawable;

        if (this->video_cropper) {
            this->video_cropper->resize(this->gd_drawable);

            image_frame_api_ptr = this->video_cropper.get();
        }

        this->png_capture_obj->resize(*image_frame_api_ptr);
    }

    this->external_breakpoint();
}

Capture::RTDisplayResult Capture::set_rt_display(bool enable_rt_display)
{
    return this->png_real_time_capture_obj
        ? this->png_real_time_capture_obj->set_rt_display(enable_rt_display)
        : Capture::RTDisplayResult::Unchanged;
}

void Capture::set_row(size_t rownum, bytes_view data)
{
    if (this->capture_drawable) {
        this->gd_drawable.set_row(rownum, data);
    }
}

void Capture::sync()
{
    if (this->capture_drawable) {
        for (gdi::GraphicApi & gd : this->gds){
            gd.sync();
        }
    }
}

bool Capture::kbd_input(MonotonicTimePoint now, uint32_t uchar)
{
    bool ret = true;
    for (gdi::KbdInputApi & kbd : this->kbds) {
        ret &= kbd.kbd_input(now, uchar);
    }
    return ret;
}

void Capture::enable_kbd_input_mask(bool enable)
{
    if (this->old_kbd_input_mask_state != enable) {
        this->possible_active_window_change();

        this->old_kbd_input_mask_state = enable;
    }

    for (gdi::KbdInputApi & kbd : this->kbds) {
        kbd.enable_kbd_input_mask(enable);
    }
}

bool Capture::has_graphic_api() const
{
    return this->capture_drawable;
}

void Capture::add_graphic(gdi::GraphicApi & gd)
{
    if (this->capture_drawable) {
        this->gds.emplace_back(gd);
    }
}

Capture::WaitingTimeBeforeNextSnapshot Capture::periodic_snapshot(
    MonotonicTimePoint now,
    uint16_t cursor_x, uint16_t cursor_y
) {
    if (this->drawable_pointer) {
        this->drawable_pointer->set_position(cursor_x, cursor_y);
    }
    this->mouse_info = {now, cursor_x, cursor_y};

    auto time = MonotonicTimePoint::duration::max();
    if (!this->caps.empty()) {
        for (gdi::CaptureApi & cap : this->caps) {
            auto next_time = cap.periodic_snapshot(now, cursor_x, cursor_y);
            time = std::min(time, next_time.duration());
        }
    }
    return WaitingTimeBeforeNextSnapshot(time);
}

void Capture::visibility_rects_event(Rect rect) {
    if (this->png_real_time_capture_obj) {
        this->png_real_time_capture_obj->visibility_rects_event(rect);
    }
    if (this->wrm_capture_obj) {
        this->wrm_capture_obj->visibility_rects_event(rect);
    }

    if (this->smart_video_cropping == SmartVideoCropping::disable
     || this->smart_video_cropping == SmartVideoCropping::v1
     || !this->video_cropper
    ) {
        return;
    }

    uint16_t const drawable_width  = this->gd_drawable.width();
    uint16_t const drawable_height = this->gd_drawable.height();

    Rect const image_frame_rect = this->video_cropper->get_rect();

    assert((image_frame_rect.cx <= drawable_width) && (image_frame_rect.cy <= drawable_height));

    rect = Rect(0, 0, drawable_width, drawable_height).intersect(rect);

    if (image_frame_rect.contains(rect)) {
        return;
    }

    Rect new_image_frame_rect = image_frame_rect;

    if (new_image_frame_rect.x > rect.x) {
        new_image_frame_rect.x = rect.x;
    }
    else if ((new_image_frame_rect.x + new_image_frame_rect.cx) < (rect.x + rect.cx)) {
        new_image_frame_rect.x = (rect.x + rect.cx) - new_image_frame_rect.cx;
    }

    if (new_image_frame_rect.y > rect.y) {
        new_image_frame_rect.y = rect.y;
    }
    else if ((new_image_frame_rect.y + new_image_frame_rect.cy) < (rect.y + rect.cy)) {
        new_image_frame_rect.y = (rect.y + rect.cy) - new_image_frame_rect.cy;
    }

    bool const retval = this->video_cropper->reset(new_image_frame_rect);
    assert(!retval);
    (void)retval;
}


template<class... Ts>
void Capture::draw_impl(const Ts & ... args)
{
    if (this->capture_drawable) {
        for (gdi::GraphicApi & gd : this->gds){
            gd.draw(args...);
        }
    }
}

void Capture::draw(RDP::FrameMarker const & cmd)
{
    for (gdi::GraphicApi & gd : this->gds) {
        gd.draw(cmd);
    }

    if (cmd.action == RDP::FrameMarker::FrameEnd) {
        auto mouse = this->mouse_info;
        for (gdi::CaptureApi & cap : this->caps) {
            cap.frame_marker_event(mouse.last_now, mouse.last_x, mouse.last_y);
        }
    }
}

void Capture::draw(RDPDstBlt           const & cmd, Rect clip) { this->draw_impl(cmd, clip); }
void Capture::draw(RDPMultiDstBlt      const & cmd, Rect clip) { this->draw_impl(cmd, clip); }
void Capture::draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPScrBlt           const & cmd, Rect clip) { this->draw_impl(cmd, clip); }
void Capture::draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) { this->draw_impl(cmd, clip); }
void Capture::draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) { this->draw_impl(cmd, clip, color_ctx); }
void Capture::draw(RDPBitmapData       const & cmd, Bitmap const & bmp) { this->draw_impl(cmd, bmp); }
void Capture::draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) { this->draw_impl(cmd, clip, bmp);}
void Capture::draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) { this->draw_impl(cmd, clip, color_ctx, bmp); }
void Capture::draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) { this->draw_impl(cmd, clip, color_ctx, gly_cache); }
void Capture::draw(RDPSetSurfaceCommand const & cmd) { this->draw_impl(cmd); }
void Capture::draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) { this->draw_impl(cmd, content); }

void Capture::draw(const RDP::RAIL::WindowIcon                     & cmd) { this->draw_impl(cmd); }
void Capture::draw(const RDP::RAIL::CachedIcon                     & cmd) { this->draw_impl(cmd); }
void Capture::draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) { this->draw_impl(cmd); }
void Capture::draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) { this->draw_impl(cmd); }
void Capture::draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) { this->draw_impl(cmd); }

void Capture::draw(RDPColCache   const & cmd) { this->draw_impl(cmd); }
void Capture::draw(RDPBrushCache const & cmd) { this->draw_impl(cmd); }

void Capture::draw(const RDP::RAIL::NewOrExistingWindow & cmd)
{
    this->draw_impl(cmd);

    this->rail_screen_visibility.update(cmd);

    this->visibility_rects_event(
        this->rail_screen_visibility.get_join_visibility_rect()
    );
}

void Capture::draw(const RDP::RAIL::DeletedWindow & cmd)
{
    this->draw_impl(cmd);

    this->rail_screen_visibility.update(cmd);

    this->visibility_rects_event(
        this->rail_screen_visibility.get_join_visibility_rect()
    );
}

void Capture::draw(const RDP::RAIL::NonMonitoredDesktop & cmd)
{
    if (this->capture_drawable) {
        this->draw_impl(cmd);

        this->visibility_rects_event(Rect(
            0, 0, this->gd_drawable.width(), this->gd_drawable.height()
        ));
    }
}

void Capture::cached_pointer(gdi::CachePointerIndex cache_idx)
{
    if (this->capture_drawable) {
        for (gdi::GraphicApi & gd : this->gds) {
            gd.cached_pointer(cache_idx);
        }

        this->drawable_pointer->set_cursor(this->ptr_cache.pointer(cache_idx));
    }
}

void Capture::new_pointer(gdi::CachePointerIndex cache_idx, RdpPointerView const& cursor)
{
    if (this->capture_drawable) {
        for (gdi::GraphicApi & gd : this->gds) {
            gd.new_pointer(cache_idx, cursor);
        }
    }
}

void Capture::set_palette(const BGRPalette & palette)
{
    if (this->capture_drawable) {
        for (gdi::GraphicApi & gd : this->gds){
            gd.set_palette(palette);
        }
    }
}

void Capture::external_breakpoint()
{
    for (gdi::ExternalCaptureApi & obj : this->ext_caps) {
        obj.external_breakpoint();
    }
}

void Capture::external_monotonic_time_point(MonotonicTimePoint now)
{
    for (gdi::ExternalCaptureApi & obj : this->ext_caps) {
        obj.external_monotonic_time_point(now);
    }
}

void Capture::external_times(MonotonicTimePoint::duration monotonic_delay, RealTimePoint real_time)
{
    for (gdi::ExternalCaptureApi & obj : this->ext_caps) {
        obj.external_times(monotonic_delay, real_time);
    }
}

void Capture::session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list)
{
    for (gdi::CaptureProbeApi & cap_prob : this->probes) {
        cap_prob.session_update(now, id, kv_list);
    }
}

void Capture::possible_active_window_change()
{
    for (gdi::CaptureProbeApi & cap_prob : this->probes) {
        cap_prob.possible_active_window_change();
    }
}
