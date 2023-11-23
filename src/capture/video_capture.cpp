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
#include "capture/sequenced_video_params.hpp"
#include "capture/video_capture.hpp"
#include "capture/video_recorder.hpp"
#include "utils/drawable.hpp"

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

    inline ImageByInterval video_params_to_image_by_interval(bool no_timestamp)
    {
        return no_timestamp
            ? ImageByInterval::ZeroOrOneWithoutTimestamp
            : ImageByInterval::ZeroOrOneWithTimestamp
            ;
    }
} // anonymous namespace


using WaitingTimeBeforeNextSnapshot = gdi::CaptureApi::WaitingTimeBeforeNextSnapshot;

// VideoCaptureCtx
//@{
VideoCaptureCtx::VideoCropper::VideoCropper(Drawable& drawable, Rect crop_rect)
: crop_rect(crop_rect)
, original_dimension(crop_rect.cx, crop_rect.cy)
, is_fullscreen(drawable.width() == crop_rect.width()
             && drawable.height() == crop_rect.height())
, out_bmpdata(this->is_fullscreen
    ? nullptr
    : new uint8_t[drawable.width() * drawable.height() * drawable.Bpp] {} /*NOLINT*/
)
{
}

void VideoCaptureCtx::VideoCropper::set_cropping(Rect cropping) noexcept
{
    assert(cropping.cx <= original_dimension.w);
    assert(cropping.cy <= original_dimension.h);

    if (cropping.cx != crop_rect.cx) {
        uint8_t* out_bmpdata_tmp = out_bmpdata.get() + cropping.cx * Drawable::Bpp;
        std::size_t const rowsize = original_dimension.w * Drawable::Bpp;
        std::size_t const empty_rowsize = (original_dimension.w - cropping.cx) * Drawable::Bpp;

        for (uint16_t i = 0; i < cropping.cy; ++i) {
            std::memset(out_bmpdata_tmp, 0, empty_rowsize);
            out_bmpdata_tmp += rowsize;
        }
    }

    if (cropping.cx != crop_rect.cx || cropping.cy != crop_rect.cy) {
        uint8_t* out_bmpdata_tmp = out_bmpdata.get() + original_dimension.w * cropping.cy * Drawable::Bpp;
        std::size_t const rowsize = original_dimension.w * Drawable::Bpp;

        for (uint16_t i = cropping.cy; i < original_dimension.h; ++i) {
            std::memset(out_bmpdata_tmp, 0, rowsize);
            out_bmpdata_tmp += rowsize;
        }
    }

    crop_rect = cropping;
}

void VideoCaptureCtx::VideoCropper::prepare_image_frame(Drawable& drawable) noexcept
{
    if (this->is_fullscreen) {
        return ;
    }

    uint8_t* out_bmpdata_tmp = this->out_bmpdata.get();

    uint8_t const* in_bmpdata_tmp
        = drawable.data()
        + checked_cast<size_t>(this->crop_rect.y) * drawable.rowsize()
        + checked_cast<size_t>(this->crop_rect.x) * drawable.Bpp;

    unsigned const rowsize = this->original_dimension.w * drawable.Bpp;
    unsigned const datasize = this->crop_rect.cx * drawable.Bpp;

    for (uint16_t i = 0; i < this->crop_rect.cy; ++i) {
        std::memcpy(out_bmpdata_tmp, in_bmpdata_tmp, datasize);

        in_bmpdata_tmp  += drawable.rowsize();
        out_bmpdata_tmp += rowsize;
    }
}

WritableImageView VideoCaptureCtx::VideoCropper::get_image(Drawable& drawable) noexcept
{
    if (this->is_fullscreen) {
        return gdi::get_writable_image_view(drawable);
    }

    return WritableImageView{
        this->out_bmpdata.get(),
        this->original_dimension.w,
        this->original_dimension.h,
        this->original_dimension.w * drawable.Bpp,
        BytesPerPixel(drawable.Bpp),
        WritableImageView::Storage::TopToBottom,
    };
}

VideoCaptureCtx::VideoCaptureCtx(
    MonotonicTimePoint monotonic_now,
    RealTimePoint real_now,
    ImageByInterval image_by_interval,
    unsigned frame_rate,
    Drawable & drawable,
    LazyDrawablePointer & lazy_drawable_pointer,
    Rect crop_rect,
    array_view<BitsetInStream::underlying_type> updatable_frame_marker_end_bitset_view
)
: drawable(drawable)
, lazy_drawable_pointer(lazy_drawable_pointer)
, monotonic_last_time_capture(monotonic_now)
, monotonic_to_real(monotonic_now, real_now)
, frame_interval(std::chrono::microseconds(1000000L / frame_rate)) // `1000000L % frame_rate ` should be equal to 0
, next_trace_time(monotonic_now)
, image_by_interval(image_by_interval)
, has_timestamp(image_by_interval == ImageByInterval::ZeroOrOneWithTimestamp)
, video_cropper(drawable, crop_rect)
, updatable_frame_marker_end_bitset_stream(updatable_frame_marker_end_bitset_view.data())
, updatable_frame_marker_end_bitset_end(updatable_frame_marker_end_bitset_view.end())
{
    this->updatable_graphics.set_drawing_event(true);
}

void VideoCaptureCtx::preparing_video_frame(video_recorder & recorder)
{
    DrawablePointer::BufferSaver buffer_saver;

    auto& drawable_pointer = this->lazy_drawable_pointer.drawable_pointer();

    drawable_pointer.trace_mouse(this->drawable, buffer_saver);

    auto image = this->prepare_image_frame();

    if (this->has_timestamp) {
        this->timestamp_tracer.trace(image, this->get_tm());
    }

    recorder.preparing_video_frame();

    if (this->has_timestamp) {
        this->timestamp_tracer.clear(image);
    }

    drawable_pointer.clear_mouse(this->drawable, buffer_saver);
}

WritableImageView VideoCaptureCtx::prepare_image_frame() noexcept
{
    // TODO could be avoided when updatable_graphics.has_drawing_event() == false,
    // but the mouse pointer is drawn on Drawable
    this->video_cropper.prepare_image_frame(this->drawable);
    return this->video_cropper.get_image(this->drawable);
}

void VideoCaptureCtx::frame_marker_event(
    video_recorder & recorder, MonotonicTimePoint now,
    uint16_t cursor_x, uint16_t cursor_y)
{
    if (((updatable_frame_marker_end_bitset_stream.current() == updatable_frame_marker_end_bitset_end
       || updatable_frame_marker_end_bitset_stream.read()
      ) && this->updatable_graphics.has_drawing_event())
     || this->cursor_x != cursor_x
     || this->cursor_y != cursor_y
    ) {
        this->preparing_video_frame(recorder);
        this->updatable_graphics.set_drawing_event(false);
        this->cursor_x = cursor_x;
        this->cursor_y = cursor_y;
    }

    this->has_frame_marker = true;

    this->snapshot(recorder, now, cursor_x, cursor_y);
}

void VideoCaptureCtx::encoding_end_frame(video_recorder & recorder)
{
    auto dur = std::max(this->frame_interval, MonotonicTimePoint::duration(400ms));
    auto save_monotonic_last_time_capture = this->monotonic_last_time_capture;
    auto save_next_trace_time = this->next_trace_time;
    this->snapshot(
        recorder, this->monotonic_last_time_capture + dur,
        this->cursor_x, this->cursor_y);
    this->monotonic_last_time_capture = save_monotonic_last_time_capture;
    this->next_trace_time = save_next_trace_time;
}

void VideoCaptureCtx::next_video(video_recorder & recorder)
{
    this->frame_index = 0;
    this->preparing_video_frame(recorder);
    recorder.encoding_video_frame(++this->frame_index);
}

void VideoCaptureCtx::synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
{
    this->monotonic_to_real = MonotonicTimeToRealTime(monotonic_time, real_time);
}

void VideoCaptureCtx::set_cropping(Rect cropping) noexcept
{
    assert(cropping.x >= 0);
    assert(cropping.y >= 0);
    assert(cropping.eright() <= this->drawable.width());
    assert(cropping.ebottom() <= this->drawable.height());

    this->video_cropper.set_cropping(cropping);
}

bool VideoCaptureCtx::logical_frame_ended() const noexcept
{
    return this->drawable.logical_frame_ended;
}

WritableImageView VideoCaptureCtx::acquire_image_for_dump(
    DrawablePointer::BufferSaver& buffer_saver,
    const tm& now)
{
    auto& drawable_pointer = this->lazy_drawable_pointer.drawable_pointer();
    drawable_pointer.trace_mouse(this->drawable, buffer_saver);

    auto image = this->prepare_image_frame();

    if (this->has_timestamp) {
        this->timestamp_tracer.trace(image, now);
    }

    return image;
}

void VideoCaptureCtx::release_image_for_dump(
    WritableImageView image,
    DrawablePointer::BufferSaver const& buffer_saver)
{
    if (this->has_timestamp) {
        this->timestamp_tracer.clear(image);
    }

    auto& drawable_pointer = this->lazy_drawable_pointer.drawable_pointer();
    drawable_pointer.clear_mouse(this->drawable, buffer_saver);
}

tm VideoCaptureCtx::get_tm() const
{
    return to_tm_t(this->monotonic_last_time_capture, this->monotonic_to_real);
}

WaitingTimeBeforeNextSnapshot VideoCaptureCtx::snapshot(
    video_recorder & recorder, MonotonicTimePoint now,
    uint16_t cursor_x, uint16_t cursor_y
)
{
    auto tick = now - this->monotonic_last_time_capture;
    auto const frame_interval = this->frame_interval;
    if (tick >= frame_interval) {
        bool const update_timestamp = this->has_timestamp
                                   && now >= this->next_trace_time;
        bool const update_image = (!this->has_frame_marker
                                  && this->updatable_graphics.has_drawing_event())
                                || this->cursor_x != cursor_x
                                || this->cursor_y != cursor_y
                                ;
        bool const update_pointer = (update_image || update_timestamp);

        DrawablePointer::BufferSaver buffer_saver;

        if (update_pointer) {
            auto& drawable_pointer = this->lazy_drawable_pointer.drawable_pointer();
            drawable_pointer.trace_mouse(this->drawable, buffer_saver);
        }

        auto image = WritableImageView::create_null_view();

        if (update_pointer
         || (update_image
             && (!this->has_timestamp || this->monotonic_last_time_capture < this->next_trace_time))
        ) {
            image = this->prepare_image_frame();

            if (this->has_timestamp) {
                this->timestamp_tracer.trace(image, this->get_tm());

                if (this->monotonic_last_time_capture >= this->next_trace_time) {
                    this->next_trace_time += 1s;
                }
            }

            recorder.preparing_video_frame();
        }

        this->cursor_x = cursor_x;
        this->cursor_y = cursor_y;

        // synchronize video time with the end of second

        auto preparing_timestamp_video_frame = [&, this](video_recorder & recorder){
            if (not image.data()) {
                image = this->prepare_image_frame();
            }

            this->timestamp_tracer.trace(image, this->get_tm());

            recorder.preparing_timestamp_video_frame();
        };

        switch (this->image_by_interval) {
            case ImageByInterval::ZeroOrOneWithTimestamp:
                do {
                    if (this->monotonic_last_time_capture >= this->next_trace_time) {
                        preparing_timestamp_video_frame(recorder);
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

        this->updatable_graphics.set_drawing_event(false);

        if (update_timestamp && this->has_timestamp) {
            this->timestamp_tracer.clear(image);
        }

        if (update_pointer) {
            auto& drawable_pointer = this->lazy_drawable_pointer.drawable_pointer();
            drawable_pointer.clear_mouse(this->drawable, buffer_saver);
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
    Drawable & drawable,
    LazyDrawablePointer & lazy_drawable_pointer,
    Rect crop_rect,
    VideoParams const & video_params,
    FullVideoParams const & /*full_video_params*/ /*empty struct*/)
: video_cap_ctx(
    capture_params.now, capture_params.real_now,
    video_params_to_image_by_interval(video_params.no_timestamp),
    video_params.frame_rate, drawable, lazy_drawable_pointer, crop_rect,
    video_params.updatable_frame_marker_end_bitset_view)
, recorder(
    str_concat(
        std::string_view{capture_params.record_path},
        std::string_view{capture_params.basename},
        '.',
        video_params.codec
    ).c_str(),
    capture_params.file_permissions,
    capture_params.session_log,
    drawable,
    checked_int{video_params.frame_rate},
    video_params.codec.c_str(),
    video_params.codec_options.c_str(),
    checked_int{video_params.verbosity})
{
    log_video_params(video_params);
}

FullVideoCaptureImpl::~FullVideoCaptureImpl()
{
    this->video_cap_ctx.encoding_end_frame(this->recorder);
}


void FullVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    this->video_cap_ctx.frame_marker_event(
        this->recorder, now, cursor_x, cursor_y);
}

WaitingTimeBeforeNextSnapshot FullVideoCaptureImpl::periodic_snapshot(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    return this->video_cap_ctx.snapshot(this->recorder, now, cursor_x, cursor_y);
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
    this->video_cap_ctx.snapshot(*this->recorder, now, cursor_x, cursor_y);
    if (!this->ic_has_first_img) {
        return this->first_periodic_snapshot(now);
    }
    return this->video_sequencer_periodic_snapshot(now);
}

void SequencedVideoCaptureImpl::frame_marker_event(
    MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y)
{
    this->video_cap_ctx.frame_marker_event(
        *this->recorder, now, cursor_x, cursor_y);
}

WaitingTimeBeforeNextSnapshot SequencedVideoCaptureImpl::first_periodic_snapshot(MonotonicTimePoint now)
{
    WaitingTimeBeforeNextSnapshot ret;

    auto constexpr interval = std::chrono::microseconds(3s) / 2;
    auto const duration = now - this->monotonic_start_capture;
    if (duration >= interval) {
        auto video_interval = this->break_interval;
        if (this->video_cap_ctx.logical_frame_ended()
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
    DrawablePointer::BufferSaver buffer_saver;
    const auto now = this->video_cap_ctx.get_tm();
    auto image = this->video_cap_ctx.acquire_image_for_dump(buffer_saver, now);
    this->recorder.emplace(
        this->vc_filename_generator.current_filename(),
        this->recorder_params.file_permissions,
        this->recorder_params.acl_report,
        image,
        this->recorder_params.frame_rate,
        this->recorder_params.codec_name.c_str(),
        this->recorder_params.codec_options.c_str(),
        this->recorder_params.verbosity
    );
    this->video_cap_ctx.release_image_for_dump(image, buffer_saver);
}

void SequencedVideoCaptureImpl::ic_flush(const tm& now)
{
    DrawablePointer::BufferSaver buffer_saver;
    auto image = this->video_cap_ctx.acquire_image_for_dump(buffer_saver, now);
    this->ic_scaled_png.dump_png24(this->ic_filename_generator.current_filename(), image, true);
    this->video_cap_ctx.release_image_for_dump(image, buffer_saver);
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
    Drawable & drawable,
    LazyDrawablePointer & lazy_drawable_pointer,
    Rect crop_rect,
    VideoParams const & video_params,
    SequencedVideoParams const& sequenced_video_params,
    NotifyNextVideo & next_video_notifier)
: monotonic_start_capture(capture_params.now)
, monotonic_to_real(capture_params.now, capture_params.real_now)
, video_cap_ctx(
    capture_params.now, capture_params.real_now,
    video_params_to_image_by_interval(video_params.no_timestamp),
    video_params.frame_rate, drawable, lazy_drawable_pointer, crop_rect,
    video_params.updatable_frame_marker_end_bitset_view)
, vc_filename_generator(capture_params.record_path, capture_params.basename, video_params.codec)
, ic_filename_generator(capture_params.record_path, capture_params.basename, "png")
, ic_scaled_png(png_width, png_height)
, start_break(capture_params.now)
, break_interval((sequenced_video_params.break_interval > std::chrono::microseconds::zero())
    ? sequenced_video_params.break_interval
    : std::chrono::microseconds::max())
, next_video_notifier(next_video_notifier)
, recorder_params{
    capture_params.session_log,
    video_params.codec,
    video_params.codec_options,
    int(video_params.frame_rate),
    int(video_params.verbosity),
    capture_params.file_permissions,
}
{
    log_video_params(video_params);
    this->init_recorder();
}

SequencedVideoCaptureImpl::~SequencedVideoCaptureImpl()
{
    if (this->recorder) {
        this->video_cap_ctx.encoding_end_frame(*this->recorder);
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

    this->video_cap_ctx.encoding_end_frame(*this->recorder);
    this->recorder.reset();
    this->vc_filename_generator.next();

    this->init_recorder();
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
