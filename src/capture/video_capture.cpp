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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#include "capture/capture_params.hpp"
#include "capture/video_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/video_capture.hpp"
#include "capture/video_recorder.hpp"
#include "utils/sugar/algostring.hpp"

#include "core/RDP/RDPDrawable.hpp"

#include "gdi/capture_api.hpp"

#include "utils/log.hpp"
#include "utils/strutils.hpp"

#include <cerrno>
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>


using namespace std::chrono_literals;

using ImageByInterval = VideoCaptureCtx::ImageByInterval;

namespace
{
    inline tm to_tm_t(
        MonotonicTimePoint t,
        MonotonicTimeToRealTime monotonic_to_real)
    {
        tm res;
        auto duration = monotonic_to_real.to_real_time_duration(t);
        time_t sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        localtime_r(&sec, &res);
        return res;
    }

    inline ImageByInterval video_params_to_image_by_interval(
        bool no_timestamp, bool bogus_vlc_frame_rate)
    {
        return no_timestamp
            ? (bogus_vlc_frame_rate
                ? ImageByInterval::OneWithoutTimestamp
                : ImageByInterval::ZeroOrOneWithoutTimestamp)
            : (bogus_vlc_frame_rate
                ? ImageByInterval::OneWithTimestamp
                : ImageByInterval::ZeroOrOneWithTimestamp)
            ;
    }
}


using WaitingTimeBeforeNextSnapshot = gdi::CaptureApi::WaitingTimeBeforeNextSnapshot;

// VideoCaptureCtx
//@{
template<class... Ts>
void VideoCaptureCtx::UpdatableDraw::draw(Ts const&...)
{
    this->has_draw_event = true;
}

void VideoCaptureCtx::UpdatableDraw::set_pointer(
    uint16_t /*cache_idx*/, Pointer const& /*cursor*/,
    gdi::GraphicApi::SetPointerMode /*mode*/)
{
    this->has_draw_event = true;
}

VideoCaptureCtx::VideoCaptureCtx(
    MonotonicTimePoint now,
    RealTimePoint real_now,
    ImageByInterval image_by_interval,
    unsigned frame_rate,
    RDPDrawable & drawable,
    gdi::ImageFrameApi & image_frame
)
: drawable(drawable)
, monotonic_last_time_capture(now)
, monotonic_to_real(now, real_now)
, frame_interval(std::chrono::microseconds(1000000L / frame_rate)) // `1000000L % frame_rate ` should be equal to 0
, next_trace_time(now)
, image_by_interval(image_by_interval)
, has_timestamp(
    image_by_interval == ImageByInterval::OneWithTimestamp
 || image_by_interval == ImageByInterval::ZeroOrOneWithTimestamp)
, image_frame_api(image_frame)
, timestamp_tracer(image_frame.get_writable_image_view())
{}

void VideoCaptureCtx::preparing_video_frame(video_recorder & recorder)
{
    this->image_frame_api.prepare_image_frame();

    if (this->has_timestamp) {
        this->timestamp_tracer.trace(to_tm_t(
            this->monotonic_last_time_capture, this->monotonic_to_real));
    }

    recorder.preparing_video_frame();

    if (this->has_timestamp) {
        this->timestamp_tracer.clear();
    }
}

void VideoCaptureCtx::frame_marker_event(
    video_recorder & recorder, MonotonicTimePoint now, bool & has_draw_event,
    uint16_t cursor_x, uint16_t cursor_y)
{
    if (has_draw_event
     // TODO Drawable::dont_show_mouse_cursor
     || this->cursor_x != cursor_x
     || this->cursor_y != cursor_y
    ) {
        this->drawable.trace_mouse();
        this->preparing_video_frame(recorder);
        this->drawable.clear_mouse();
    }

    this->has_frame_marker = true;
    this->cursor_x = cursor_x;
    this->cursor_y = cursor_y;
    has_draw_event = false;

    this->snapshot(recorder, now, has_draw_event, cursor_x, cursor_y);
}

void VideoCaptureCtx::encoding_end_frame(video_recorder & recorder, bool & has_draw_event)
{
    auto dur = std::max(this->frame_interval, MonotonicTimePoint::duration(400ms));
    auto save_monotonic_last_time_capture = this->monotonic_last_time_capture;
    auto save_next_trace_time = this->next_trace_time;
    this->snapshot(
        recorder, this->monotonic_last_time_capture + dur,
        has_draw_event, this->cursor_x, this->cursor_y);
    this->monotonic_last_time_capture = save_monotonic_last_time_capture;
    this->next_trace_time = save_next_trace_time;
}

void VideoCaptureCtx::next_video(video_recorder & recorder)
{
    this->frame_index = 0;
    this->drawable.trace_mouse();
    this->preparing_video_frame(recorder);
    this->drawable.clear_mouse();
    recorder.encoding_video_frame(++this->frame_index);
}

void VideoCaptureCtx::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
}

WaitingTimeBeforeNextSnapshot VideoCaptureCtx::snapshot(
    video_recorder & recorder, MonotonicTimePoint now, bool & has_draw_event,
    uint16_t cursor_x, uint16_t cursor_y
)
{
    auto tick = now - this->monotonic_last_time_capture;
    auto const frame_interval = this->frame_interval;
    if (tick >= frame_interval) {
        bool const update_timestamp = this->has_timestamp
                                   && now >= this->next_trace_time;
        bool const update_image = !this->has_frame_marker
                               && (has_draw_event
                               // TODO Drawable::dont_show_mouse_cursor
                                || this->cursor_x != cursor_x
                                || this->cursor_y != cursor_y
                               );
        bool const update_pointer = (update_image || update_timestamp);

        if (update_pointer) {
            this->drawable.trace_mouse();
        }

        if (update_image
         && (!this->has_timestamp || this->monotonic_last_time_capture < this->next_trace_time)
        ) {
            this->preparing_video_frame(recorder);
        }

        this->cursor_x = cursor_x;
        this->cursor_y = cursor_y;
        has_draw_event = false;

        // synchronize video time with the end of second

        switch (this->image_by_interval) {
            case ImageByInterval::OneWithTimestamp:
                do {
                    if (this->monotonic_last_time_capture >= this->next_trace_time) {
                        this->preparing_video_frame(recorder);
                        this->next_trace_time += 1s;
                    }

                    recorder.encoding_video_frame(++this->frame_index);

                    this->monotonic_last_time_capture += frame_interval;
                    tick -= frame_interval;
                } while (this->monotonic_last_time_capture + frame_interval <= now);
                break;

            case ImageByInterval::OneWithoutTimestamp:
                do {
                    recorder.encoding_video_frame(++this->frame_index);

                    this->monotonic_last_time_capture += frame_interval;
                    tick -= frame_interval;
                } while (this->monotonic_last_time_capture + frame_interval <= now);
                break;

            case ImageByInterval::ZeroOrOneWithTimestamp:
                do {
                    if (this->monotonic_last_time_capture >= this->next_trace_time) {
                        this->preparing_video_frame(recorder);
                        this->next_trace_time += 1s;
                    }

                    auto update_timer_at = std::max(this->monotonic_last_time_capture + frame_interval, this->next_trace_time)
                                         - this->monotonic_last_time_capture;
                    auto elapsed = std::min(tick, update_timer_at);
                    auto count = elapsed / frame_interval;
                    elapsed = count * frame_interval;

                    this->frame_index += count;
                    recorder.encoding_video_frame(this->frame_index);

                    this->monotonic_last_time_capture += elapsed;
                    tick -= elapsed;
                } while (this->monotonic_last_time_capture + frame_interval <= now);
                break;

            case ImageByInterval::ZeroOrOneWithoutTimestamp:
                constexpr MonotonicTimePoint::duration max_frame_interval = 2s;
                do {
                    auto elapsed = std::min(tick, max_frame_interval);
                    auto count = elapsed / frame_interval;
                    elapsed = count * frame_interval;

                    this->frame_index += count;
                    recorder.encoding_video_frame(this->frame_index);

                    this->monotonic_last_time_capture += elapsed;
                    tick -= elapsed;
                } while (this->monotonic_last_time_capture + frame_interval <= now);
                break;
        }

        if (update_pointer) {
            this->drawable.clear_mouse();
        }
    }
    return WaitingTimeBeforeNextSnapshot(frame_interval - tick);
}

//@}


static void log_video_params(VideoParams const& video_params)
{
    if (video_params.verbosity) {
        LOG(LOG_INFO, "Video recording: codec: %s, frame_rate: %u, options: %s",
            video_params.codec, video_params.frame_rate, video_params.codec_options);
    }
}


// FullVideoCaptureImpl
//@{

FullVideoCaptureImpl::FullVideoCaptureImpl(
    CaptureParams const & capture_params,
    RDPDrawable & drawable, gdi::ImageFrameApi & image_frame,
    VideoParams const & video_params, FullVideoParams const & full_video_params)
: video_cap_ctx(
    capture_params.now, capture_params.real_now,
    video_params_to_image_by_interval(
        video_params.no_timestamp,
        full_video_params.bogus_vlc_frame_rate),
    video_params.frame_rate, drawable, image_frame)
, recorder(
    str_concat(
        std::string_view{capture_params.record_path},
        std::string_view{capture_params.basename},
        '.',
        video_params.codec
    ).c_str(),
    capture_params.groupid, capture_params.session_log,
    image_frame.get_image_view(),
    checked_int{video_params.frame_rate},
    video_params.codec.c_str(),
    video_params.codec_options.c_str(),
    checked_int{video_params.verbosity})
{
    log_video_params(video_params);
}

FullVideoCaptureImpl::~FullVideoCaptureImpl()
{
    this->video_cap_ctx.encoding_end_frame(this->recorder, this->sink.has_draw_event);
}


void FullVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    this->video_cap_ctx.frame_marker_event(
        this->recorder, now, this->sink.has_draw_event, cursor_x, cursor_y);
}

WaitingTimeBeforeNextSnapshot FullVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    auto ret = this->video_cap_ctx.snapshot(
        this->recorder, now, this->sink.has_draw_event, cursor_x, cursor_y);
    return ret;
}

void FullVideoCaptureImpl::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->video_cap_ctx.synchronize_times(monotonic_time, real_time);
}

//@}


// SequencedVideoCaptureImpl
//@{

SequencedVideoCaptureImpl::FilenameGenerator::FilenameGenerator(
    std::string_view prefix,
    std::string_view filename,
    std::string_view extension)
: filename(str_concat(prefix, filename, "-000000."_av, extension))
, num_pos(int(this->filename.size() - (extension.size() + 1)))
{}

void SequencedVideoCaptureImpl::FilenameGenerator::next()
{
    ++this->num;
    auto chars = int_to_decimal_chars(this->num);
    memcpy(this->filename.data() + this->num_pos - chars.size(), chars.data(), chars.size());
}

char const* SequencedVideoCaptureImpl::FilenameGenerator::current_filename() const
{
    return this->filename.c_str();
}


WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    this->video_cap_ctx.snapshot(
        *this->recorder, now, this->sink.has_draw_event, cursor_x, cursor_y);
    if (!this->ic_has_first_img) {
        return this->first_periodic_snapshot(now);
    }
    return this->video_sequencer_periodic_snapshot(now);
}

void SequencedVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    this->video_cap_ctx.frame_marker_event(
        *this->recorder, now, this->sink.has_draw_event, cursor_x, cursor_y);
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::first_periodic_snapshot(MonotonicTimePoint now)
{
    WaitingTimeBeforeNextSnapshot ret;

    auto constexpr interval = std::chrono::microseconds(3s) / 2;
    auto const duration = now - this->monotonic_start_capture;
    if (duration >= interval) {
        auto video_interval = this->break_interval;
        if (this->ic_drawable.logical_frame_ended()
         || duration > 2s
         || duration >= video_interval
        ) {
            this->ic_flush(to_tm_t(now, this->monotonic_to_real));
            this->ic_has_first_img = true;
            ret = WaitingTimeBeforeNextSnapshot(video_interval);
        }
        else {
            ret = WaitingTimeBeforeNextSnapshot(interval / 3);
        }
    }
    else {
        ret = WaitingTimeBeforeNextSnapshot(interval - duration);
    }

    return std::min(ret.duration(), this->video_sequencer_periodic_snapshot(now).duration());
}


void SequencedVideoCaptureImpl::init_recorder()
{
    this->recorder.emplace(
        this->vc_filename_generator.current_filename(),
        this->groupid,
        this->acl_report,
        this->image_frame_api.get_image_view(),
        this->video_params.frame_rate,
        this->video_params.codec.c_str(),
        this->video_params.codec_options.c_str(),
        this->video_params.verbosity
    );
}

void SequencedVideoCaptureImpl::ic_flush(const tm& now)
{
    this->image_frame_api.prepare_image_frame();
    this->video_cap_ctx.timestamp_tracer.trace(now);
    this->ic_scaled_png.dump_png24(this->ic_filename_generator.current_filename(), this->image_frame_api, true);
    this->video_cap_ctx.timestamp_tracer.clear();
    this->ic_filename_generator.next();
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::video_sequencer_periodic_snapshot(
    MonotonicTimePoint now)
{
    assert(this->break_interval.count());
    auto const interval = now - this->start_break;
    if (interval >= this->break_interval) {
        this->next_video_impl(now, NotifyNextVideo::Reason::sequenced);
    }
    return WaitingTimeBeforeNextSnapshot(this->break_interval);
}


SequencedVideoCaptureImpl::SequencedVideoCaptureImpl(
    CaptureParams const & capture_params,
    unsigned png_width, unsigned png_height,
    /* const */RDPDrawable & drawable,
    gdi::ImageFrameApi & image_frame,
    VideoParams const & video_params,
    NotifyNextVideo & next_video_notifier)
: monotonic_start_capture(capture_params.now)
, monotonic_to_real(capture_params.now, capture_params.real_now)
, video_cap_ctx(
    capture_params.now, capture_params.real_now,
    video_params_to_image_by_interval(
        video_params.no_timestamp, video_params.bogus_vlc_frame_rate),
    video_params.frame_rate, drawable, image_frame)
, vc_filename_generator(capture_params.record_path, capture_params.basename, video_params.codec)
, ic_filename_generator(capture_params.record_path, capture_params.basename, "png")
, video_params(video_params)
, groupid(capture_params.groupid)
, acl_report(capture_params.session_log)
, ic_drawable(drawable)
, image_frame_api(image_frame)
, ic_scaled_png(png_width, png_height)
, start_break(capture_params.now)
, break_interval((video_params.video_interval > std::chrono::microseconds::zero())
    ? video_params.video_interval
    : std::chrono::microseconds::max())
, next_video_notifier(next_video_notifier)
{
    log_video_params(video_params);
    this->init_recorder();
}

SequencedVideoCaptureImpl::~SequencedVideoCaptureImpl()
{
    if (this->recorder) {
        this->video_cap_ctx.encoding_end_frame(*this->recorder, this->sink.has_draw_event);
    }
}


void SequencedVideoCaptureImpl::next_video_impl(MonotonicTimePoint now, NotifyNextVideo::Reason reason)
{
    this->start_break = now;

    tm ptm = to_tm_t(now, this->monotonic_to_real);

    if (!this->ic_has_first_img) {
        this->ic_has_first_img = true;
        this->ic_flush(ptm);
    }

    this->video_cap_ctx.encoding_end_frame(*this->recorder, this->sink.has_draw_event);
    this->recorder.reset();
    this->vc_filename_generator.next();

    this->init_recorder();
    this->sink.has_draw_event = false;
    this->video_cap_ctx.next_video(*this->recorder);

    this->ic_flush(ptm);

    this->next_video_notifier.notify_next_video(now, reason);
}

void SequencedVideoCaptureImpl::next_video(MonotonicTimePoint now)
{
    this->next_video_impl(now, NotifyNextVideo::Reason::external);
}

void SequencedVideoCaptureImpl::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
    this->video_cap_ctx.synchronize_times(monotonic_time, real_time);
}

//@}
