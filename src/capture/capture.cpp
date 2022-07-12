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
#include "utils/sugar/split.hpp"

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
#include "capture/redis.hpp"

#include "capture/capture_params.hpp"
#include "capture/drawable_params.hpp"
#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"
#include "capture/png_params.hpp"
#include "capture/redis_params.hpp"
#include "capture/video_params.hpp"
#include "capture/wrm_params.hpp"

#include "capture/capture.hpp"
#include "capture/wrm_capture.hpp"
#include "capture/utils/pattern_searcher.hpp"
#include "utils/drawable_pointer.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>
#include <iterator>
#include <type_traits>

#include <ctime> // localtime_r
#include <cstdio> // snprintf / sprintf
#include <cerrno>
#include <cassert>


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

    chars_view av() const noexcept
    {
        return {char_ptr_cast(buf_char), char_len};
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
                no_printable_fn("//"_av);
            }
            else {
                utf32_char_fn(uchar);
            }
            break;
        #define Case(i, s) case i: no_printable_fn(s ""_av); break
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

struct FilesystemFullReporter
{
    SessionLogApi* session_log;

    void operator()(const Error & error) const
    {
        if (session_log && error.errnum == ENOSPC) {
            session_log->report("FILESYSTEM_FULL", "100|unknown");
        }
    }
};

void report_pattern(
    char const* type, SessionLogApi& session_log,
    PatternSearcher::PatternFound found, chars_view data)
{
    char message[4096];
    snprintf(message, sizeof(message), "$%s:%.*s|%.*s",
        type,
        static_cast<int>(found.pattern.size()), found.pattern.data(),
        static_cast<int>(data.size()), data.data());
    utils::back(message) = '\0';

    LogId logid = LogId::NOTIFY_PATTERN_DETECTED;
    char const* findpattern = "FINDPATTERN_NOTIFY";

    if (found.is_pattern_kill) {
        logid = LogId::KILL_PATTERN_DETECTED;
        findpattern = "FINDPATTERN_KILL";
    }

    session_log.log6(logid, {KVLog("pattern"_av, std::string_view{message})});
    session_log.report(findpattern, message);
}

class Utf8KbdBuffer
{
    static constexpr const uint64_t buf_len = 128;

    char kbd_data[buf_len] = { 0 };
    uint64_t total_len = 0;
    char * p = kbd_data;

public:
    void push(chars_view utf8_char)
    {
        auto remaining = static_cast<std::size_t>(std::end(kbd_data) - p);
        if (REDEMPTION_UNLIKELY(remaining < utf8_char.size())) {
            std::size_t n = std::min(remaining, utf8_char.size());
            memcpy(p, utf8_char.data(), n);
            p = kbd_data;
            utf8_char = utf8_char.drop_front(n);
        }

        memcpy(p, utf8_char.data(), utf8_char.size());
        p += utf8_char.size();
        total_len += utf8_char.size();
    }

    chars_view get(uint64_t match_len)
    {
        auto len = static_cast<std::size_t>(p - kbd_data);
        if (len >= match_len) {
            return {p-match_len, p};
        }
        if (p != std::end(kbd_data) && total_len > buf_len) {
            std::rotate(kbd_data, p, std::end(kbd_data));
            p = std::end(kbd_data);
            return {p-std::min(match_len, buf_len), p};
        }
        return {kbd_data, p};
    }
};

} // anonymous namespace


REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wsubobject-linkage")
class Capture::PatternKbd final : public gdi::KbdInputApi
{
public:
    explicit PatternKbd(
        SessionLogApi& session_log,
        array_view<CapturePattern> cap_patterns_kill,
        array_view<CapturePattern> cap_patterns_notify)
    : patterns(cap_patterns_kill, cap_patterns_notify, CapturePattern::CaptureType::kbd)
    , session_log(session_log)
    {}

    [[nodiscard]] bool has_pattern() const
    {
        return !this->patterns.has_pattern();
    }

    bool kbd_input(MonotonicTimePoint /*now*/, uint32_t uchar) override
    {
        bool can_be_sent_to_server = true;

        filtering_kbd_input(
            uchar,
            [this, &can_be_sent_to_server](uint32_t uchar) {
                ZStrUtf8Char utf8_char_buf{uchar};
                auto utf8_char = utf8_char_buf.av();
                if (utf8_char.size() > 0) {
                    utf8_kbd_buffer.push(utf8_char);
                    for (auto result : patterns.scan(utf8_char)) {
                        if (result.is_pattern_kill) {
                            can_be_sent_to_server = false;
                        }
                        auto text = utf8_kbd_buffer.get(result.match_len);
                        report_pattern("kbd", session_log, result, text);
                    }
                }
            },
            [this](chars_view const & /*noprintable_char*/) {
                this->patterns.reset_kbd_streams();
            },
            nofilter_slash{}
        );

        return can_be_sent_to_server;
    }

    void enable_kbd_input_mask(bool /*enable*/) override
    {}

private:
    PatternSearcher patterns;
    Utf8KbdBuffer utf8_kbd_buffer;
    SessionLogApi& session_log;
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
    PatternSearcher patterns;
    SessionLogApi& session_log;

public:
    explicit PatternsChecker(
        SessionLogApi& session_log,
        array_view<CapturePattern> cap_patterns_kill,
        array_view<CapturePattern> cap_patterns_notify)
    : patterns(cap_patterns_kill, cap_patterns_notify, CapturePattern::CaptureType::ocr)
    , session_log(session_log)
    {}

    [[nodiscard]] bool has_pattern() const
    {
        return !this->patterns.has_pattern();
    }

    void title_changed(chars_view str)
    {
        assert(str.data() && not str.empty());
        for (auto result : patterns.scan(str)) {
            report_pattern("ocr", session_log, result, str);
        }
    }
};


namespace
{

struct PngCaptureData
{
    PngCaptureData(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable& drawable, DrawablePointer const & drawable_pointer,
        Rect cropping)
    : drawable(drawable)
    , drawable_pointer(drawable_pointer)
    , last_time_capture(capture_params.now)
    , frame_interval(png_params.png_interval)
    , monotonic_to_real(capture_params.now, capture_params.real_now)
    , scaled_png{png_params.png_width, png_params.png_height}
    , cropping(cropping)
    {}

    MonotonicTimePoint::duration interval() const
    {
        return this->frame_interval;
    }

    void set_cropping(Rect cropping) noexcept
    {
        this->cropping = cropping;
    }

    template<class NextCaptureFn>
    Capture::WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, Transport& trans, NextCaptureFn&& next_capture_fn)
    {
        auto const duration = now - this->last_time_capture;
        auto const interval = this->frame_interval;
        if (duration >= interval) {
            // Snapshot at end of Frame or force snapshot if diff_time_val >= 1.5 x frame_interval.
            if (this->drawable.logical_frame_ended || (duration >= interval * 3 / 2)) {
                this->last_time_capture = now;
                this->dump_png(to_time_t(now, this->monotonic_to_real), trans);
                next_capture_fn();
                return Capture::WaitingTimeBeforeNextSnapshot(interval.count()
                    ? interval - duration % interval
                    : interval);
            }
            // Wait 0.3 x frame_interval.
            return Capture::WaitingTimeBeforeNextSnapshot(interval / 3);
        }
        return Capture::WaitingTimeBeforeNextSnapshot(interval - duration);
    }

    void dump_png(time_t t, Transport& trans)
    {
        DrawablePointer::BufferSaver buffer_saver;
        this->drawable_pointer.trace_mouse(this->drawable, buffer_saver);

        auto writable_image = gdi::get_writable_image_view(this->drawable)
            .mutable_sub_view(this->cropping);

        tm ptm;
        localtime_r(&t, &ptm);
        this->timestamp_tracer.trace(writable_image, ptm);

        this->scaled_png.dump_png24(trans, writable_image, true);

        this->timestamp_tracer.clear(writable_image);
        this->drawable_pointer.clear_mouse(this->drawable, buffer_saver);
    }

private:
    Drawable& drawable;
    DrawablePointer const & drawable_pointer;
    MonotonicTimePoint last_time_capture;
    const MonotonicTimePoint::duration frame_interval;
    const MonotonicTimeToRealTime monotonic_to_real;
    const ScaledPng24 scaled_png;
    Rect cropping;
    TimestampTracer timestamp_tracer;
};

} // anonymous namespace

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wsubobject-linkage")
class Capture::PngCapture final : public gdi::CaptureApi
{
public:
    PngCapture(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable& drawable, DrawablePointer const & drawable_pointer,
        Rect cropping)
    : png_data(capture_params, png_params, drawable, drawable_pointer, cropping)
    , trans(
        capture_params.record_tmp_path,
        png_params.real_basename,
        ".png",
        capture_params.groupid,
        FilesystemFullReporter{capture_params.session_log})
    {}

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*x*/, uint16_t /*y*/
    ) override
    {
        return this->png_data.periodic_snapshot(now, this->trans, [this]{
            this->trans.next();
        });
    }

    void set_cropping(Rect cropping) noexcept
    {
        this->png_data.set_cropping(cropping);
    }

private:
    PngCaptureData png_data;
    OutFilenameSequenceTransport trans;
};

class Capture::PngCaptureRT final : public gdi::CaptureApi
{
public:
    explicit PngCaptureRT(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable& drawable, DrawablePointer const & drawable_pointer,
        Rect cropping)
    : png_limit(png_params.png_limit)
    , png_data(capture_params, png_params, drawable, drawable_pointer, cropping)
    , trans(
        capture_params.record_tmp_path,
        png_params.real_basename,
        ".png",
        capture_params.groupid,
        FilesystemFullReporter{capture_params.session_log})
    {}

    ~PngCaptureRT()
    {
        this->clear_png_interval();
    }

    RTDisplayResult set_rt_display(bool enable_rt_display)
    {
        if (enable_rt_display != this->enable_rt_display){
            LOG(LOG_INFO, "PngCaptureRT::enable_rt_display=%d", enable_rt_display);
            this->enable_rt_display = enable_rt_display;
            // clear files if we go from RT to non-RT
            if (!this->enable_rt_display) {
                this->clear_png_interval();
                return RTDisplayResult::Disabled;
            }
            return RTDisplayResult::Enabled;
        }

        return RTDisplayResult::Unchanged;
    }

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*x*/, uint16_t /*y*/
    ) override
    {
        if (!this->enable_rt_display) {
            return WaitingTimeBeforeNextSnapshot(this->png_data.interval());
        }

        return this->png_data.periodic_snapshot(now, this->trans, [this]{
            this->trans.next();
            if (this->trans.get_seqno() - this->num_start >= this->png_limit) {
                ::unlink(this->trans.seqgen(this->num_start));
                ++this->num_start;
            }
        });
    }

    void set_cropping(Rect cropping) noexcept
    {
        this->png_data.set_cropping(cropping);
    }

private:
    void clear_png_interval()
    {
        uint32_t num_end = this->trans.get_seqno() + 1;
        for(; this->num_start < num_end ; ++this->num_start) {
            // unlink may fail, for instance if file does not exist, just don't care
            ::unlink(this->trans.seqgen(this->num_start));
        }
    }

    uint32_t num_start = 0;
    uint32_t png_limit;

    bool enable_rt_display = false;

    PngCaptureData png_data;
    OutFilenameSequenceTransport trans;
};

class Capture::PngCaptureRTRedis final : public gdi::CaptureApi
{
public:
    explicit PngCaptureRTRedis(
        const CaptureParams & capture_params, const PngParams & png_params,
        Drawable& drawable, DrawablePointer const & drawable_pointer,
        Rect cropping)
    : png_data(capture_params, png_params, drawable, drawable_pointer, cropping)
    , redis_cmd(png_params.redis_key_name, [&]{
        auto expiration_delay = png_params.png_interval * 2;
        if (expiration_delay < 2min) {
            expiration_delay = 2min;
        }
        return std::chrono::duration_cast<std::chrono::seconds>(expiration_delay);
    }())
    {
    }

    void set_cropping(Rect cropping) noexcept
    {
        this->png_data.set_cropping(cropping);
    }

    RTDisplayResult set_rt_display(bool enable_rt_display, RedisParams const& redis_params)
    {
        if (enable_rt_display != this->enable_rt_display) {
            LOG(LOG_INFO, "PngCaptureRTRedis::enable_rt_display=%d", enable_rt_display);
            this->enable_rt_display = enable_rt_display;

            if (!this->enable_rt_display) {
                this->redis_session.close();
                return RTDisplayResult::Disabled;
            }

            this->connect_to_redis(redis_params);

            return RTDisplayResult::Enabled;
        }

        return RTDisplayResult::Unchanged;
    }

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t /*x*/, uint16_t /*y*/
    ) override
    {
        if (!this->enable_rt_display) {
            return WaitingTimeBeforeNextSnapshot(this->png_data.interval());
        }

        struct BufTransport : Transport
        {
            RedisCmdSet* redis_cmd;

            void do_send(const uint8_t * buffer, size_t len) override
            {
                redis_cmd->append({buffer, len});
            }
        };

        BufTransport buf;
        buf.redis_cmd = &this->redis_cmd;

        return this->png_data.periodic_snapshot(now, buf, [this]{
            auto cmd = this->redis_cmd.build_command();
            auto result_code = this->redis_session.send(cmd);
            this->redis_cmd.clear();
            this->check(result_code, "PngCaptureRTRedis::periodic_snapshot()",
                        ERR_RECORDER_REDIS_RESPONSE);
        });
    }

private:
    void connect_to_redis(RedisParams const& redis_params)
    {
        auto result_code = this->redis_session.open(
            redis_params.address,
            redis_params.port,
            truncated_bounded_array_view(redis_params.password),
            redis_params.db,
            redis_params.timeout, RedisSyncSession::TlsParams{
                .enable_tls = redis_params.tls.enable_tls,
                .cert_file = redis_params.tls.cert_file,
                .key_file = redis_params.tls.key_file,
                .ca_cert_file = redis_params.tls.ca_cert_file
            });
        this->check(result_code, "PngCaptureRTRedis::connect_to_redis()",
                    ERR_SOCKET_CONNECT_FAILED);
    }

    void check(RedisIOCode code, char const* context, error_type err)
    {
        if (REDEMPTION_UNLIKELY(code != RedisIOCode::Ok)) {
            int errnum = redis_session.get_last_errno();
            auto msg = redis_session.get_last_error_message();
            LOG(LOG_ERR, "%s: %s: %s", context, redios_io_code_to_zstring(code), msg);
            throw Error(err, errnum);
        }
    }

    bool enable_rt_display = false;

    PngCaptureData png_data;
    RedisCmdSet redis_cmd;
    RedisSyncSession redis_session;
};
REDEMPTION_DIAGNOSTIC_POP()

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
    inline constexpr chars_view session_meta_kbd_prefix = "[Kbd]"_av;
    inline constexpr chars_view session_meta_kbd_suffix = "\n"_av;
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
        sizeof(kbd_buffer) - session_meta_kbd_prefix.size() - session_meta_kbd_suffix.size();
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
    : kbd_stream{{this->kbd_buffer + session_meta_kbd_prefix.size(), kbd_buffer_usable_char}}
    , monotonic_to_real(now, real_now)
    , monotonic_last_time(now)
    , trans(trans)
    , key_markers_hidden_state(key_markers_hidden_state)
    , meta_params(meta_params)
    {
        memcpy(this->kbd_buffer, session_meta_kbd_prefix.data(), session_meta_kbd_prefix.size());

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
        this->send_data(now, "(break)"_av, '+');
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
                        auto const single_slash = "/"_av;
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
    }()}, FilesystemFullReporter{capture_params.session_log})
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
    const CropperInfo cropper_info,
    const Rect rail_window_rect)
: gd_drawable(drawable_params.rdp_drawable)
, ptr_cache(drawable_params.ptr_cache)
, update_progress_data(update_progress_data)
, mouse_info{
    capture_params.now,
    static_cast<uint16_t>(drawable_params.rdp_drawable.width() / 2),
    static_cast<uint16_t>(drawable_params.rdp_drawable.height() / 2)}
, capture_drawable(capture_wrm || capture_video || capture_ocr || capture_png || capture_video_full)
, smart_video_cropping(capture_params.smart_video_cropping)
, rail_screen_offset{
    checked_int(-cropper_info.screen_position.x),
    checked_int(-cropper_info.screen_position.y),
}
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

    if (cropper_info.crop_rect.isempty() || !(capture_png || capture_video || capture_video_full)) {
        smart_video_cropping = SmartVideoCropping::disable;
    }

    Rect real_crop_rect = (SmartVideoCropping::disable == smart_video_cropping)
        ? Rect(0, 0, this->gd_drawable.width(), this->gd_drawable.height())
        : cropper_info.crop_rect;

    if (capture_meta) {
        this->meta_capture_obj = std::make_unique<MetaCaptureImpl>(capture_params, meta_params);
    }

    std::vector<CapturePattern> cap_patterns;
    std::size_t nb_patt_kill = 0;

    // parse pattern_kill and pattern_notify
    if ((this->capture_drawable && capture_pattern_checker) || capture_kbd) {
        cap_patterns.reserve(64);
        auto parse = [&cap_patterns, &pattern_params](char const* patterns){
            if (!patterns) {
                return;
            }
            for (auto pattern : get_lines(patterns, capture_pattern_separator)) {
                LOG_IF(pattern_params.verbose, LOG_INFO, "filter=\"%.*s\"",
                    static_cast<int>(pattern.size()), pattern.data());

                CapturePattern const cap_pattern = parse_capture_pattern(pattern);
                if (not cap_pattern.pattern().empty()) {
                    cap_patterns.push_back(cap_pattern);
                }
            }
        };
        parse(pattern_params.pattern_kill);
        nb_patt_kill = cap_patterns.size();
        parse(pattern_params.pattern_notify);
    }

    if (this->capture_drawable) {
        this->gds.emplace_back(this->gd_drawable);

        this->drawable_pointer = std::make_unique<DrawablePointer>(normal_pointer());
        drawable_pointer->set_position(this->gd_drawable.width() / 2,
                                       this->gd_drawable.height() / 2);

        if (capture_png) {
            if (png_params.real_time_image_capture) {
                if (png_params.use_redis_with_rt_display) {
                    this->png_real_time_redis_capture_obj = std::make_unique<PngCaptureRTRedis>(
                        capture_params,
                        png_params,
                        this->gd_drawable.impl(),
                        *this->drawable_pointer,
                        real_crop_rect
                    );
                }
                else {
                    this->png_real_time_capture_obj = std::make_unique<PngCaptureRT>(
                        capture_params,
                        png_params,
                        this->gd_drawable.impl(),
                        *this->drawable_pointer,
                        real_crop_rect
                    );
                }
            }
            else {
                this->png_capture_obj = std::make_unique<PngCapture>(
                    capture_params,
                    png_params,
                    this->gd_drawable.impl(),
                    *this->drawable_pointer,
                    real_crop_rect
                );
            }
        }

        if (capture_wrm) {
            this->wrm_capture_obj = std::make_unique<WrmCaptureImpl>(
                capture_params, wrm_params, this->gd_drawable.impl(), rail_window_rect,
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
                real_crop_rect, video_params, sequenced_video_params, notifier);
        }

        if (capture_video_full) {
            this->full_video_capture_obj = std::make_unique<FullVideoCaptureImpl>(
                capture_params, this->gd_drawable.impl(), *this->drawable_pointer,
                real_crop_rect, video_params, full_video_params);
        }
#else
        if (capture_video || capture_video_full) {
            (void)sequenced_video_params;
            (void)full_video_params;
            (void)video_params;
            LOG(LOG_WARNING, "VideoCapture is disabled (-DREDEMPTION_NO_FFMPEG)");
        }
#endif

        if (capture_pattern_checker) {
            this->patterns_checker = std::make_unique<PatternsChecker>(
                *capture_params.session_log,
                array_view{cap_patterns}.first(nb_patt_kill),
                array_view{cap_patterns}.drop_front(nb_patt_kill)
            );
            if (!this->patterns_checker->has_pattern()) {
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
        else if (this->png_real_time_redis_capture_obj) {
            this->caps.emplace_back(*this->png_real_time_redis_capture_obj);
        }
        else if (this->png_capture_obj) {
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
            *capture_params.session_log,
            array_view{cap_patterns}.first(nb_patt_kill),
            array_view{cap_patterns}.drop_front(nb_patt_kill)
        );

        if (this->pattern_kbd_capture_obj->has_pattern()) {
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
    this->png_real_time_redis_capture_obj.reset();
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

    if (this->png_real_time_capture_obj
     || this->png_real_time_redis_capture_obj
     || this->png_capture_obj
    ) {
        auto rect = Rect(0, 0, this->gd_drawable.width(), this->gd_drawable.height());

        if (this->png_real_time_capture_obj) {
            this->png_real_time_capture_obj->set_cropping(rect);
        }
        else if (this->png_real_time_redis_capture_obj) {
            this->png_real_time_redis_capture_obj->set_cropping(rect);
        }
        else if (this->png_capture_obj) {
            this->png_capture_obj->set_cropping(rect);
        }
    }

    this->external_breakpoint();
}

Capture::RTDisplayResult Capture::set_rt_display(bool enable_rt_display, const RedisParams& redis_params)
{
    if (this->png_real_time_capture_obj) {
        return this->png_real_time_capture_obj->set_rt_display(enable_rt_display);
    }

    if (this->png_real_time_redis_capture_obj) {
        return this->png_real_time_redis_capture_obj->set_rt_display(enable_rt_display, redis_params);
    }

    return Capture::RTDisplayResult::Unchanged;
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

void Capture::visibility_rects_event(Rect rect)
{
    if (this->wrm_capture_obj) {
        this->wrm_capture_obj->visibility_rects_event(rect);
    }

    switch (this->smart_video_cropping) {
        case SmartVideoCropping::disable:
        case SmartVideoCropping::v1:
            return;
        case SmartVideoCropping::v2:;
    }

    if (this->png_real_time_capture_obj
     || this->png_real_time_redis_capture_obj
     || this->png_capture_obj
    #ifndef REDEMPTION_NO_FFMPEG
     || this->sequenced_video_capture_obj
     || this->full_video_capture_obj
    #endif
    ) {
        rect.x += this->rail_screen_offset.x;
        rect.y += this->rail_screen_offset.y;

        auto cropping = rect.intersect(this->gd_drawable.width(),
                                       this->gd_drawable.height());

        if (this->png_real_time_capture_obj) {
            this->png_real_time_capture_obj->set_cropping(cropping);
        }
        else if (this->png_real_time_redis_capture_obj) {
            this->png_real_time_redis_capture_obj->set_cropping(cropping);
        }
        else if (this->png_capture_obj) {
            this->png_capture_obj->set_cropping(cropping);
        }

        #ifndef REDEMPTION_NO_FFMPEG
        if (this->sequenced_video_capture_obj) {
            this->sequenced_video_capture_obj->set_cropping(cropping);
        }

        if (this->full_video_capture_obj) {
            this->full_video_capture_obj->set_cropping(cropping);
        }
        #endif
    }
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
