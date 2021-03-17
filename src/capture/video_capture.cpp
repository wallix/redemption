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

namespace
{
    inline time_t to_time_t(
        MonotonicTimePoint t,
        MonotonicTimeToRealTime monotonic_to_real)
    {
        auto duration = monotonic_to_real.to_real_time_duration(t);
        return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    }

    inline time_t to_time_t(MonotonicTimePoint const& t)
    {
        auto duration = t.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    }

    inline std::chrono::microseconds to_us(MonotonicTimePoint::duration const& duration)
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(duration);
    }
}


using WaitingTimeBeforeNextSnapshot = gdi::CaptureApi::WaitingTimeBeforeNextSnapshot;

// VideoCaptureCtx
//@{

VideoCaptureCtx::VideoCaptureCtx(
    MonotonicTimePoint now,
    RealTimePoint real_now,
    TraceTimestamp trace_timestamp,
    ImageByInterval image_by_interval,
    unsigned frame_rate,
    RDPDrawable & drawable,
    gdi::ImageFrameApi & image_frame
)
: drawable(drawable)
, monotonic_last_time_capture(now)
, monotonic_start_capture(now)
, monotonic_to_real(now, real_now)
, frame_interval(std::chrono::microseconds(1000000L / frame_rate)) // `1000000L % frame_rate ` should be equal to 0
, current_video_time(0)
, start_frame_index(0)
, trace_timestamp(trace_timestamp)
, image_by_interval(image_by_interval)
, image_frame_api(image_frame)
, timestamp_tracer(image_frame.get_writable_image_view())
{}

void VideoCaptureCtx::preparing_video_frame(video_recorder & recorder)
{
    this->drawable.trace_mouse();
    this->image_frame_api.prepare_image_frame();
    if (TraceTimestamp::Yes == this->trace_timestamp) {
        tm tm_result;
        time_t rawtime = to_time_t(this->monotonic_last_time_capture, this->monotonic_to_real);
        localtime_r(&rawtime, &tm_result);
        this->timestamp_tracer.trace(tm_result);
    }
    recorder.preparing_video_frame();
    this->previous_second = to_time_t(this->monotonic_last_time_capture);

    if (TraceTimestamp::Yes == this->trace_timestamp) {
        this->timestamp_tracer.clear();
    }
    this->drawable.clear_mouse();
}

void VideoCaptureCtx::frame_marker_event(video_recorder & recorder)
{
    this->preparing_video_frame(recorder);
    this->has_frame_marker = true;
}

void VideoCaptureCtx::encoding_video_frame(video_recorder & recorder)
{
    this->preparing_video_frame(recorder);
    auto index = this->current_video_time / this->frame_interval - this->start_frame_index;
    recorder.encoding_video_frame(index + 1);
    if (!index) {
        ++index;
        long long count = std::max<long long>(2, std::chrono::seconds(1) / this->frame_interval);
        while (count--) {
            recorder.encoding_video_frame(++index);
        }
    }
}

void VideoCaptureCtx::next_video()
{
    if (this->frame_interval.count()) {
        this->start_frame_index = this->current_video_time / this->frame_interval;
    }
}

uint16_t VideoCaptureCtx::width() const noexcept
{
    return this->drawable.width();
}

uint16_t VideoCaptureCtx::height() const noexcept
{
    return this->drawable.height();
}

size_t VideoCaptureCtx::pix_len() const noexcept
{
    return this->drawable.pix_len();
}

const uint8_t * VideoCaptureCtx::data() const noexcept
{
    return this->drawable.data();
}

void VideoCaptureCtx::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
}

WaitingTimeBeforeNextSnapshot VideoCaptureCtx::snapshot(
    video_recorder & recorder, MonotonicTimePoint now
)
{
    std::chrono::microseconds tick { to_us(now - this->monotonic_last_time_capture) };
    std::chrono::microseconds const frame_interval = this->frame_interval;
    if (tick >= frame_interval) {
        if (!this->has_frame_marker) {
            this->preparing_video_frame(recorder);
        }

        std::chrono::microseconds previous_video_time = this->current_video_time;

        this->current_video_time += tick;
        tick %= frame_interval;
        this->current_video_time -= tick;

        // synchronize video time with the end of second

        switch (this->image_by_interval) {
            case ImageByInterval::One:
            {
                auto count = (this->current_video_time - previous_video_time) / frame_interval;
                auto frame_index = previous_video_time / frame_interval - this->start_frame_index;

                while (count--) {
                    if (to_time_t(this->monotonic_last_time_capture) != this->previous_second) {
                        this->preparing_video_frame(recorder);
                    }
                    recorder.encoding_video_frame(frame_index++);
                    this->monotonic_last_time_capture += frame_interval;
                }
            }
            break;
            case ImageByInterval::ZeroOrOne:
            {
                std::chrono::microseconds count = this->current_video_time - previous_video_time;
                while (count >= frame_interval) {
                    if (to_time_t(this->monotonic_last_time_capture) != this->previous_second) {
                        this->preparing_video_frame(recorder);
                    }
                    recorder.encoding_video_frame(
                        previous_video_time / frame_interval - this->start_frame_index);
                    auto elapsed = std::min(count, decltype(count)(std::chrono::seconds(1)));
                    this->monotonic_last_time_capture += elapsed;
                    previous_video_time += elapsed;
                    count -= elapsed;
                }
            }
            break;
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

using TraceTimestamp = VideoCaptureCtx::TraceTimestamp;
using ImageByInterval = VideoCaptureCtx::ImageByInterval;

// FullVideoCaptureImpl
//@{

FullVideoCaptureImpl::FullVideoCaptureImpl(
    CaptureParams const & capture_params,
    RDPDrawable & drawable, gdi::ImageFrameApi & image_frame,
    VideoParams const & video_params, FullVideoParams const & full_video_params)
: video_cap_ctx(capture_params.now, capture_params.real_now,
    video_params.no_timestamp ? TraceTimestamp::No : TraceTimestamp::Yes,
    full_video_params.bogus_vlc_frame_rate ? ImageByInterval::One : ImageByInterval::ZeroOrOne,
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
    this->encoding_video_frame();
}


void FullVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint /*now*/, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    this->video_cap_ctx.frame_marker_event(this->recorder);
}

WaitingTimeBeforeNextSnapshot FullVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    return this->video_cap_ctx.snapshot(this->recorder, now);
}

void FullVideoCaptureImpl::encoding_video_frame()
{
    this->video_cap_ctx.encoding_video_frame(this->recorder);
}

void FullVideoCaptureImpl::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->video_cap_ctx.synchronize_times(monotonic_time, real_time);
}

//@}


// SequencedVideoCaptureImpl
//@{

SequencedVideoCaptureImpl::SequenceTransport::SequenceTransport(
    std::string_view prefix,
    std::string_view filename,
    std::string_view extension,
    const int groupid,
    AclReportApi * acl_report)
: filename(str_concat(prefix, filename, "-000000."_av, extension))
, num_pos(int(this->filename.size() - (extension.size() + 1)))
, groupid(groupid)
, acl_report(acl_report)
{}

void SequencedVideoCaptureImpl::SequenceTransport::next()
{
    ++this->num;
    auto chars = int_to_decimal_chars(this->num);
    memcpy(this->filename.data() + this->num_pos - chars.size(), chars.data(), chars.size());
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    this->vc.periodic_snapshot(now);
    if (!this->ic_has_first_img) {
        return this->first_periodic_snapshot(now);
    }
    return this->video_sequencer_periodic_snapshot(now);
}

void SequencedVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint now, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/)
{
    this->vc.frame_marker_event();
    if (!this->ic_has_first_img) {
        this->first_periodic_snapshot(now);
    }
    else {
        this->video_sequencer_periodic_snapshot(now);
    }
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
            tm ptm;
            time_t t = to_time_t(now, this->monotonic_to_real);
            localtime_r(&t, &ptm);
            this->vc.prepare_video_frame();
            this->vc.trace_timestamp(ptm);
            this->ic_flush();
            this->vc.clear_timestamp();
            this->ic_trans.next();
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

SequencedVideoCaptureImpl::VideoCapture::VideoCapture(
    CaptureParams const & capture_params,
    RDPDrawable & drawable,
    gdi::ImageFrameApi & image_frame,
    VideoParams const & video_params)
: video_cap_ctx(capture_params.now, capture_params.real_now,
    video_params.no_timestamp ? TraceTimestamp::No : TraceTimestamp::Yes,
    video_params.bogus_vlc_frame_rate ? ImageByInterval::One : ImageByInterval::ZeroOrOne,
    video_params.frame_rate, drawable, image_frame)
, trans(
    capture_params.record_path, capture_params.basename, video_params.codec,
    capture_params.groupid, capture_params.session_log)
, video_params(video_params)
, image_frame_api(image_frame)
{
    log_video_params(video_params);
    this->next_video();
}

SequencedVideoCaptureImpl::VideoCapture::~VideoCapture()
{
    if (this->recorder) {
        this->encoding_video_frame();
    }
}

void SequencedVideoCaptureImpl::VideoCapture::next_video()
{
    if (this->recorder) {
        this->encoding_video_frame();
        this->recorder.reset();
        this->trans.next();
    }

    this->recorder = std::make_unique<video_recorder>(
        this->trans.filename.c_str(),
        this->trans.groupid,
        this->trans.acl_report,
        this->image_frame_api.get_image_view(),
        this->video_params.frame_rate,
        this->video_params.codec.c_str(),
        this->video_params.codec_options.c_str(),
        this->video_params.verbosity
    );
    this->recorder->preparing_video_frame();
    this->video_cap_ctx.next_video();
}

void SequencedVideoCaptureImpl::VideoCapture::encoding_video_frame()
{
    this->video_cap_ctx.encoding_video_frame(*this->recorder);
}

void SequencedVideoCaptureImpl::VideoCapture::frame_marker_event()
{
    this->video_cap_ctx.frame_marker_event(*this->recorder);
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::VideoCapture::periodic_snapshot(
    MonotonicTimePoint now)
{
    return this->video_cap_ctx.snapshot(*this->recorder, now);
}

void SequencedVideoCaptureImpl::VideoCapture::trace_timestamp(const tm & now)
{
    this->video_cap_ctx.timestamp_tracer.trace(now);
}

void SequencedVideoCaptureImpl::VideoCapture::clear_timestamp()
{
    this->video_cap_ctx.timestamp_tracer.clear();
}

void SequencedVideoCaptureImpl::VideoCapture::prepare_video_frame()
{
    this->image_frame_api.prepare_image_frame();
}

void SequencedVideoCaptureImpl::VideoCapture::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->video_cap_ctx.synchronize_times(monotonic_time, real_time);
}

void SequencedVideoCaptureImpl::ic_flush()
{
    this->ic_scaled_png.dump_png24(this->ic_trans.filename.c_str(), this->image_frame_api, true);
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
, vc(capture_params, drawable, image_frame, video_params)
, ic_trans(
    capture_params.record_path, capture_params.basename, "png",
    capture_params.groupid, capture_params.session_log)
, ic_drawable(drawable)
, image_frame_api(image_frame)
, ic_scaled_png(png_width, png_height)
, start_break(capture_params.now)
, break_interval((video_params.video_interval > std::chrono::microseconds::zero())
    ? video_params.video_interval
    : std::chrono::microseconds::max())
, next_video_notifier(next_video_notifier)
{}

void SequencedVideoCaptureImpl::next_video_impl(MonotonicTimePoint now, NotifyNextVideo::Reason reason)
{
    this->start_break = now;

    time_t t = to_time_t(now, this->monotonic_to_real);

    tm ptm;
    localtime_r(&t, &ptm);

    if (!this->ic_has_first_img) {
        this->ic_has_first_img = true;
        this->vc.prepare_video_frame();
        this->vc.trace_timestamp(ptm);
        this->ic_flush();
        this->vc.clear_timestamp();
        this->ic_trans.next();
    }

    this->vc.next_video();

    this->vc.prepare_video_frame();
    this->vc.trace_timestamp(ptm);
    this->ic_flush();
    this->vc.clear_timestamp();
    this->ic_trans.next();

    this->next_video_notifier.notify_next_video(now, reason);
}

void SequencedVideoCaptureImpl::next_video(MonotonicTimePoint now)
{
    this->next_video_impl(now, NotifyNextVideo::Reason::external);
}

void SequencedVideoCaptureImpl::encoding_video_frame()
{
    this->vc.encoding_video_frame();
}

void SequencedVideoCaptureImpl::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
    this->vc.synchronize_times(monotonic_time, real_time);
}

//@}
