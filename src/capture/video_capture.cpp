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

#include "core/RDP/RDPDrawable.hpp"

#include "gdi/capture_api.hpp"

#include "utils/png.hpp"
#include "utils/bitmap_shrink.hpp"
#include "utils/difftimeval.hpp"
#include "utils/log.hpp"
#include "utils/strutils.hpp"

#ifndef REDEMPTION_NO_FFMPEG
#include <libavformat/avio.h> // AVSEEK_SIZE
#endif

#include <cerrno>
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>


namespace
{
    void video_transport_log_error(Error const & error)
    {
        if (error.id == ERR_TRANSPORT_WRITE_FAILED) {
            LOG(LOG_ERR, "VideoTransport::send: %s [%d]", strerror(error.errnum), error.errnum);
        }
    }
}

// VideoTransportBase
//@{

VideoTransportBase::VideoTransportBase(const int groupid, ReportMessageApi * report_message)
: out_file(invalid_fd(), report_message
    ? ReportError([report_message](Error error){
        video_transport_log_error(error);
        report_and_transform_error(error, ReportMessageReporter{*report_message});
        return error;
    })
    : ReportError([](Error error){
        video_transport_log_error(error);
        report_and_transform_error(error, LogReporter{});
        return error;
    })
  )
, groupid(groupid)
{
    this->tmp_filename[0] = 0;
    this->final_filename[0] = 0;
}

void VideoTransportBase::seek(int64_t offset, int whence)
{
    this->out_file.seek(offset, whence);
}

VideoTransportBase::~VideoTransportBase()
{
    if (this->out_file.is_open()) {
        this->out_file.close();
        // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
        if (::rename(this->tmp_filename, this->final_filename) < 0) {
            LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed errno=%d : %s\n"
               , this->tmp_filename, this->final_filename, errno, strerror(errno));
        }
    }
}

void VideoTransportBase::force_open()
{
    assert(this->final_filename[0]);
    assert(!this->out_file.is_open());

    std::snprintf(this->tmp_filename, sizeof(this->tmp_filename), "%sred-XXXXXX.tmp", this->final_filename);
    int fd = ::mkostemps(this->tmp_filename, 4, O_WRONLY | O_CREAT);
    if (fd == -1) {
        int const errnum = errno;
        LOG( LOG_ERR, "can't open temporary file %s : %s [%d]"
           , this->tmp_filename
           , strerror(errnum), errnum);
        this->status = false;
        throw this->out_file.get_report_error()(Error(ERR_TRANSPORT_OPEN_FAILED, errnum));
    }

    if (fchmod(fd, this->groupid ? (S_IRUSR|S_IRGRP) : S_IRUSR) == -1) {
        int const errnum = errno;
        LOG( LOG_ERR, "can't set file %s mod to %s : %s [%d]"
           , this->tmp_filename
           , this->groupid ? "u+r, g+r" : "u+r"
           , strerror(errnum), errnum);
        ::close(fd);
        unlink(this->tmp_filename);
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errnum);
    }

    this->out_file.open(unique_fd{fd});
}

void VideoTransportBase::rename()
{
    assert(this->final_filename[0]);
    assert(this->out_file.is_open());

    this->out_file.close();
    // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);

    if (::rename(this->tmp_filename, this->final_filename) < 0)
    {
        int const errnum = errno;
        LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed errno=%d : %s\n"
            , this->tmp_filename, this->final_filename, errnum, strerror(errnum));
        this->status = false;
        throw Error(ERR_TRANSPORT_WRITE_FAILED, errnum);
    }

    this->tmp_filename[0] = 0;
}

bool VideoTransportBase::is_open() const
{
    return this->out_file.is_open();
}

void VideoTransportBase::do_send(const uint8_t * data, size_t len)
{
    this->out_file.send(data, len);
}

//@}

using Microseconds = gdi::CaptureApi::Microseconds;

// VideoCaptureCtx
//@{

VideoCaptureCtx::VideoCaptureCtx(
    timeval const & now,
    TraceTimestamp trace_timestamp,
    ImageByInterval image_by_interval,
    unsigned frame_rate,
    RDPDrawable & drawable,
    gdi::ImageFrameApi & imageFrameApi
)
: drawable(drawable)
, start_video_capture(now)
, frame_interval(std::chrono::microseconds(1000000L / frame_rate)) // `1000000L % frame_rate ` should be equal to 0
, current_video_time(0)
, start_frame_index(0)
, trace_timestamp(trace_timestamp)
, image_by_interval(image_by_interval)
, image_frame_api(imageFrameApi)
, timestamp_tracer(imageFrameApi.get_mutable_image_view())
{}

void VideoCaptureCtx::preparing_video_frame(video_recorder & recorder)
{
    this->drawable.trace_mouse();
    this->image_frame_api.prepare_image_frame();
    if (TraceTimestamp::Yes == this->trace_timestamp) {
        time_t rawtime = this->start_video_capture.tv_sec;
        tm tm_result;
        localtime_r(&rawtime, &tm_result);
        this->timestamp_tracer.trace(tm_result);
    }
    recorder.preparing_video_frame();
    this->previous_second = this->start_video_capture.tv_sec;

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

Microseconds VideoCaptureCtx::snapshot(
    video_recorder & recorder, timeval const & now, bool /*ignore_frame_in_timeval*/
)
{
    std::chrono::microseconds tick { difftimeval(now, this->start_video_capture) };
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
                    if (this->start_video_capture.tv_sec != this->previous_second) {
                        this->preparing_video_frame(recorder);
                    }
                    recorder.encoding_video_frame(frame_index++);
                    this->start_video_capture += frame_interval;
                }
            }
            break;
            case ImageByInterval::ZeroOrOne:
            {
                std::chrono::microseconds count = this->current_video_time - previous_video_time;
                while (count >= frame_interval) {
                    if (this->start_video_capture.tv_sec != this->previous_second) {
                        this->preparing_video_frame(recorder);
                    }
                    recorder.encoding_video_frame(
                        previous_video_time / frame_interval - this->start_frame_index);
                    auto elapsed = std::min(count, decltype(count)(std::chrono::seconds(1)));
                    this->start_video_capture += elapsed;
                    previous_video_time += elapsed;
                    count -= elapsed;
                }
            }
            break;
        }
    }
    return frame_interval - tick;
}

//@}


// FullVideoCaptureImpl::TmpFileTransport
//@{

FullVideoCaptureImpl::TmpFileTransport::TmpFileTransport(
    const char * const prefix,
    const char * const filename,
    const char * const extension,
    const int groupid,
    ReportMessageApi * report_message)
: VideoTransportBase(groupid, report_message)
{
    int const len = std::snprintf(
        this->final_filename,
        sizeof(this->final_filename),
        "%s%s.%s", prefix, filename, extension
    );
    if (len > int(sizeof(this->final_filename))) {
        LOG(LOG_ERR, "%s", "Video path length is too large.");
        throw Error(ERR_TRANSPORT_OPEN_FAILED);
    }

    ::unlink(this->final_filename);
    this->force_open();
}

//@}


// IOVideoRecorderWithTransport
//@{

template<class Transport>
struct IOVideoRecorderWithTransport
{
#ifndef REDEMPTION_NO_FFMPEG
    static int write(void * opaque, uint8_t * buf, int buf_size)
    {
        Transport * trans       = static_cast<Transport*>(opaque);
        int         return_code = buf_size;
        try {
            trans->send(buf, buf_size);
        }
        catch (Error const & e) {
            if (e.id == ERR_TRANSPORT_WRITE_NO_ROOM) {
                LOG(LOG_ERR, "Video write_packet failure, no space left on device (id=%u)", e.id);
            }
            else {
                LOG(LOG_ERR, "Video write_packet failure (id=%u, errnum=%d)", e.id, e.errnum);
            }
            return_code = -1;
        }
        return return_code;
    }

    static int64_t seek(void * opaque, int64_t offset, int whence)
    {
        // This function is like the fseek() C stdio function.
        // "whence" can be either one of the standard C values
        // (SEEK_SET, SEEK_CUR, SEEK_END) or one more value: AVSEEK_SIZE.

        // When "whence" has this value, your seek function must
        // not seek but return the size of your file handle in bytes.
        // This is also optional and if you don't implement it you must return <0.

        // Otherwise you must return the current position of your stream
        //  in bytes (that is, after the seeking is performed).
        // If the seek has failed you must return <0.
        if (whence == AVSEEK_SIZE) {
            LOG(LOG_ERR, "Video seek failure");
            return -1;
        }
        try {
            Transport * trans = static_cast<Transport*>(opaque);
            trans->seek(offset, whence);
            return offset;
        }
        catch (Error const & e){
            LOG(LOG_ERR, "Video seek failure (id=%u)", e.id);
            return -1;
        };
    }
#else
    static int write(void * /*opaque*/, uint8_t * /*buf*/, int buf_size) { return buf_size; }
    static int64_t seek(void * /*opaque*/, int64_t offset, int /*whence*/) { return offset; }
#endif
};

//@}

using TraceTimestamp = VideoCaptureCtx::TraceTimestamp;
using ImageByInterval = VideoCaptureCtx::ImageByInterval;

// FullVideoCaptureImpl
//@{

FullVideoCaptureImpl::FullVideoCaptureImpl(
    CaptureParams const & capture_params,
    RDPDrawable & drawable, gdi::ImageFrameApi & imageFrameApi,
    VideoParams const & video_params, FullVideoParams const & full_video_params)
: trans_tmp_file(
    capture_params.record_path, capture_params.basename, video_params.codec.c_str(),
    capture_params.groupid, capture_params.report_message)
, video_cap_ctx(capture_params.now,
    video_params.no_timestamp ? TraceTimestamp::No : TraceTimestamp::Yes,
    full_video_params.bogus_vlc_frame_rate ? ImageByInterval::One : ImageByInterval::ZeroOrOne,
    video_params.frame_rate, drawable, imageFrameApi)
, recorder(
    IOVideoRecorderWithTransport<TmpFileTransport>::write,
    IOVideoRecorderWithTransport<TmpFileTransport>::seek,
    &this->trans_tmp_file,
    imageFrameApi.get_image_view(),
    video_params.bitrate,
    video_params.frame_rate,
    video_params.qscale,
    video_params.codec.c_str(),
    video_params.verbosity)
{
    if (video_params.verbosity) {
        LOG(LOG_INFO, "Video recording %u x %u, rate: %u, qscale: %u, brate: %u, codec: %s",
            video_params.target_width, video_params.target_height,
            video_params.frame_rate, video_params.qscale, video_params.bitrate,
            video_params.codec.c_str()
        );
    }
}

FullVideoCaptureImpl::~FullVideoCaptureImpl()
{
    this->encoding_video_frame();
}


void FullVideoCaptureImpl::frame_marker_event(
    const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/)
{
    this->video_cap_ctx.frame_marker_event(this->recorder);
}

Microseconds FullVideoCaptureImpl::periodic_snapshot(
    const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval)
{
    return this->video_cap_ctx.snapshot(this->recorder, now, ignore_frame_in_timeval);
}

void FullVideoCaptureImpl::encoding_video_frame()
{
    this->video_cap_ctx.encoding_video_frame(this->recorder);
}

//@}


// SequencedVideoCaptureImpl
//@{

void SequencedVideoCaptureImpl::SequenceTransport::set_final_filename()
{
    int len = std::snprintf(
        this->final_filename, sizeof(this->final_filename), "%s%s-%06u%s",
        this->filegen.path, this->filegen.base, this->filegen.num, this->filegen.ext
    );
    if (len > int(sizeof(this->final_filename))) {
        LOG(LOG_ERR, "%s", "Video path length is too large.");
        throw Error(ERR_TRANSPORT_OPEN_FAILED);
    }
}

SequencedVideoCaptureImpl::SequenceTransport::SequenceTransport(
    const char * const prefix,
    const char * const filename,
    const char * const extension,
    const int groupid,
    ReportMessageApi * report_message)
: VideoTransportBase(groupid, report_message)
{
    if (!utils::strbcpy(this->filegen.path, prefix)
     || !utils::strbcpy(this->filegen.base, filename)
     || !utils::strbcpy(this->filegen.ext, extension)) {
        LOG(LOG_ERR, "Filename too long");
        throw Error(ERR_TRANSPORT);
    }
}

bool SequencedVideoCaptureImpl::SequenceTransport::next()
{
    if (!this->status) {
        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

    this->set_final_filename();
    this->rename();

    ++this->filegen.num;
    ++this->seqno;
    return true;
}

SequencedVideoCaptureImpl::SequenceTransport::~SequenceTransport()
{
    this->set_final_filename();
}

void SequencedVideoCaptureImpl::SequenceTransport::do_send(const uint8_t * data, size_t len)
{
    if (!this->is_open()) {
        this->set_final_filename();
        this->force_open();
    }

    this->VideoTransportBase::do_send(data, len);
}

Microseconds SequencedVideoCaptureImpl::periodic_snapshot(
    timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
{
    this->vc.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    if (!this->ic_has_first_img) {
        return this->first_image.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }
    return this->video_sequencer.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
}

void SequencedVideoCaptureImpl::frame_marker_event(
    timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
{
    this->vc.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    if (!this->ic_has_first_img) {
        this->first_image.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }
    else {
        this->video_sequencer.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }
}

void SequencedVideoCaptureImpl::FirstImage::frame_marker_event(
    timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
{
    this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
}

Microseconds SequencedVideoCaptureImpl::FirstImage::periodic_snapshot(
    const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
{
    Microseconds ret;

    auto const duration = difftimeval(now, this->first_image_start_capture);
    auto const interval = std::chrono::microseconds(std::chrono::seconds(3))/2;
    if (duration >= interval) {
        auto video_interval = first_image_impl.video_sequencer.get_interval();
        if (this->first_image_impl.ic_drawable.logical_frame_ended() || duration > std::chrono::seconds(2) || duration >= video_interval) {
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->first_image_impl.vc.prepare_video_frame();
            this->first_image_impl.vc.trace_timestamp(ptm);
            this->first_image_impl.ic_flush();
            this->first_image_impl.vc.clear_timestamp();
            this->first_image_impl.ic_has_first_img = true;
            this->first_image_impl.ic_trans.next();
            ret = video_interval;
        }
        else {
            ret = interval / 3;
        }
    }
    else {
        ret = interval - duration;
    }

    return std::min(ret, this->first_image_impl.video_sequencer.periodic_snapshot(
        now, cursor_x, cursor_y, ignore_frame_in_timeval));
}


SequencedVideoCaptureImpl::VideoCapture::VideoCapture(
    const timeval & now,
    SequenceTransport & trans,
    RDPDrawable & drawable,
    gdi::ImageFrameApi & imageFrameApi,
    VideoParams const& video_params)
: video_cap_ctx(now,
    video_params.no_timestamp ? TraceTimestamp::No : TraceTimestamp::Yes,
    video_params.bogus_vlc_frame_rate ? ImageByInterval::One : ImageByInterval::ZeroOrOne,
    video_params.frame_rate, drawable, imageFrameApi)
, trans(trans)
, video_params(video_params)
, image_frame_api(imageFrameApi)
{
    if (video_params.verbosity) {
        LOG(LOG_INFO, "Video recording %u x %u, rate: %u, qscale: %u, brate: %u, codec: %s",
            video_params.target_width, video_params.target_height,
            video_params.frame_rate, video_params.qscale, video_params.bitrate,
            video_params.codec.c_str());
    }

    this->next_video();
}

SequencedVideoCaptureImpl::VideoCapture::~VideoCapture()
{
    this->encoding_video_frame();
}

void SequencedVideoCaptureImpl::VideoCapture::next_video()
{
    if (this->recorder) {
        this->encoding_video_frame();
        this->recorder.reset();
        this->trans.next();
    }

    this->recorder = std::make_unique<video_recorder>(
        IOVideoRecorderWithTransport<SequenceTransport>::write,
        IOVideoRecorderWithTransport<SequenceTransport>::seek,
        &this->trans,
        this->image_frame_api.get_image_view(),
        this->video_params.bitrate,
        this->video_params.frame_rate,
        this->video_params.qscale,
        this->video_params.codec.c_str(),
        this->video_params.verbosity
    );
    this->recorder->preparing_video_frame();
    this->video_cap_ctx.next_video();
}

void SequencedVideoCaptureImpl::VideoCapture::encoding_video_frame()
{
    this->video_cap_ctx.encoding_video_frame(*this->recorder);
}

void SequencedVideoCaptureImpl::VideoCapture::frame_marker_event(
    const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/)
{
    this->video_cap_ctx.frame_marker_event(*this->recorder);
}

Microseconds SequencedVideoCaptureImpl::VideoCapture::periodic_snapshot(
    const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval)
{
    return this->video_cap_ctx.snapshot(*this->recorder, now, ignore_frame_in_timeval);
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

void SequencedVideoCaptureImpl::zoom(unsigned percent)
{
    percent = std::min(percent, 100u);
    auto const image_view = this->image_frame_api.get_image_view();
    const unsigned zoom_width = (image_view.width() * percent) / 100;
    const unsigned zoom_height = (image_view.height() * percent) / 100;
    this->ic_zoom_factor = percent;
    this->ic_scaled_width = (zoom_width + 3) & 0xFFC;
    this->ic_scaled_height = zoom_height;
    if (this->ic_zoom_factor != 100) {
        this->ic_scaled_buffer = std::make_unique<uint8_t[]>(this->ic_scaled_width * this->ic_scaled_height * 3);
    }
}

void SequencedVideoCaptureImpl::ic_flush()
{
    if (this->ic_zoom_factor == 100) {
        this->dump24();
    }
    else {
        this->scale_dump24();
    }
}

void SequencedVideoCaptureImpl::dump24()
{
    dump_png24(this->ic_trans, this->image_frame_api, true);
}

void SequencedVideoCaptureImpl::scale_dump24()
{
    auto image_view = this->image_frame_api.get_image_view();
    scale_data(
        this->ic_scaled_buffer.get(),
        image_view.data(),
        this->ic_scaled_width,
        image_view.width(),
        this->ic_scaled_height,
        image_view.height(),
        image_view.line_size());
    ::dump_png24(
        this->ic_trans, this->ic_scaled_buffer.get(),
        this->ic_scaled_width, this->ic_scaled_height,
        this->ic_scaled_width * 3, false);
}


void SequencedVideoCaptureImpl::VideoSequencer::reset_now(const timeval& now)
{
    this->start_break = now;
}

Microseconds SequencedVideoCaptureImpl::VideoSequencer::periodic_snapshot(
    const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/)
{
    assert(this->break_interval.count());
    auto const interval = difftimeval(now, this->start_break);
    if (interval >= this->break_interval) {
        this->impl.next_video_impl(now, NotifyNextVideo::reason::sequenced);
        this->start_break = now;
    }
    return this->break_interval;
}

void SequencedVideoCaptureImpl::VideoSequencer::frame_marker_event(
    timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
{
    this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
}


SequencedVideoCaptureImpl::SequencedVideoCaptureImpl(
    CaptureParams const & capture_params,
    unsigned image_zoom,
    /* const */RDPDrawable & drawable,
    gdi::ImageFrameApi & imageFrameApi,
    VideoParams const& video_params,
    NotifyNextVideo & next_video_notifier)
: first_image(capture_params.now, *this)
, vc_trans(
    capture_params.record_path, capture_params.basename, ("." + video_params.codec).c_str(),
    capture_params.groupid, capture_params.report_message)
, vc(capture_params.now, this->vc_trans, drawable, imageFrameApi, video_params)
, ic_trans(
    capture_params.record_path, capture_params.basename, ".png",
    capture_params.groupid, capture_params.report_message)
, ic_zoom_factor(std::min(image_zoom, 100u))
, ic_drawable(drawable)
, image_frame_api(imageFrameApi)
, smart_video_cropping(capture_params.smart_video_cropping)
, video_sequencer(
    capture_params.now,
    (video_params.video_interval > std::chrono::microseconds(0))
        ? video_params.video_interval
        : std::chrono::microseconds::max(),
    *this)
, next_video_notifier(next_video_notifier)
{
    auto const image_view = imageFrameApi.get_image_view();
    const unsigned zoom_width = (image_view.width() * this->ic_zoom_factor) / 100;
    const unsigned zoom_height = (image_view.height() * this->ic_zoom_factor) / 100;
    this->ic_scaled_width = (zoom_width + 3) & 0xFFC;
    this->ic_scaled_height = zoom_height;
    if (this->ic_zoom_factor != 100) {
        this->ic_scaled_buffer = std::make_unique<uint8_t[]>(this->ic_scaled_width * this->ic_scaled_height * 3);
    }
}

void SequencedVideoCaptureImpl::next_video_impl(const timeval& now, NotifyNextVideo::reason reason) {
    this->video_sequencer.reset_now(now);

    tm ptm;
    localtime_r(&now.tv_sec, &ptm);

    if (!this->ic_has_first_img) {
        this->vc.prepare_video_frame();
        this->vc.trace_timestamp(ptm);
        this->ic_flush();
        this->vc.clear_timestamp();
        this->ic_has_first_img = true;
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

void SequencedVideoCaptureImpl::next_video(const timeval& now)
{
    this->next_video_impl(now, NotifyNextVideo::reason::external);
}

void SequencedVideoCaptureImpl::encoding_video_frame()
{
    this->vc.encoding_video_frame();
}

//@}
