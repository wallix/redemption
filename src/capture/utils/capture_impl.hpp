/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h> // localtime_r
#include <memory>

#include "transport/out_meta_sequence_transport.hpp"

// private extension
#include "capture/sequencer.hpp"
#include "capture/video_capture.hpp"
#include "capture/utils/video_params_from_ini.hpp"
#include "utils/pattutils.hpp"
// end private extension

#include "capture/session_log_agent.hpp"
#include "capture/title_extractors/agent_title_extractor.hpp"
#include "capture/title_extractors/ocr_title_filter.hpp"
#include "capture/title_extractors/ocr_titles_extractor.hpp"
#include "capture/title_extractors/ppocr_titles_extractor.hpp"
#include "capture/title_extractors/ocr_title_extractor_builder.hpp"
#include "capture/utils/pattern_checker.hpp"

#include "capture/session_meta.hpp"
#include "capture/utils/image_capture_impl.hpp"

#include "openssl_crypto.hpp"

#include "utils/log.hpp"
#include "transport/out_file_transport.hpp"
#include "capture/cryptofile.hpp"
#include "utils/urandom_read.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/local_fd.hpp"

struct PreparingWhenFrameMarkerEnd final : gdi::CaptureApi
{
    PreparingWhenFrameMarkerEnd(VideoCapture & vc)
    : vc(vc)
    {}

private:
    VideoCapture & vc;

    std::chrono::microseconds do_snapshot(
        const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        vc.preparing_video_frame();
        return std::chrono::microseconds{};
    }
};

class FullVideoCaptureImpl
{
    OutFilenameSequenceSeekableTransport trans;
    VideoCapture vc;
    PreparingWhenFrameMarkerEnd preparing_vc{vc};

public:
    FullVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        const Drawable & drawable,
        VideoParams video_param)
    : trans(
        FilenameGenerator::PATH_FILE_EXTENSION,
        record_path, basename, ("." + video_param.codec).c_str(), groupid)
    , vc(now, this->trans, drawable, no_timestamp, std::move(video_param))
    {
        const char * const path = this->trans.seqgen()->get(this->trans.get_seqno());
        ::unlink(path);
    }

    void attach_apis(ApisRegister & apis_register, const Inifile &) {
        apis_register.capture_list.push_back(this->vc);
        apis_register.graphic_snapshot_list->push_back(this->preparing_vc);
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->trans.request_full_cleaning();
    }
};


struct NotifyNextVideo : private noncopyable
{
    enum class reason { sequenced, external };
    virtual void notify_next_video(const timeval& now, reason) = 0;
    virtual ~NotifyNextVideo() = default;
};

class SequencedVideoCaptureImpl
{
    class VideoTransport final : public OutFilenameSequenceSeekableTransport
    {
        using transport_base = OutFilenameSequenceSeekableTransport;

    public:
        VideoTransport(
            const char * const record_path,
            const char * const basename,
            const char * const suffix,
            const int groupid
        )
        : transport_base(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_path, basename, suffix, groupid)
        {
            this->remove_current_path();
        }

        bool next() override {
            if (transport_base::next()) {
                this->remove_current_path();
                return true;
            }
            return false;
        }

    private:
        void remove_current_path() {
            const char * const path = this->seqgen()->get(this->get_seqno());
            ::unlink(path);
        }
    };

    struct ImageToFile : private DrawableToFile
    {
        using DrawableToFile::DrawableToFile;
        using DrawableToFile::zoom;
        using DrawableToFile::logical_frame_ended;

        bool has_first_img = false;

        void breakpoint_image(const timeval& now)
        {
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            //const_cast<Drawable&>(this->drawable).trace_mouse();
            const_cast<Drawable&>(this->drawable).trace_timestamp(ptm);
            this->flush();
            const_cast<Drawable&>(this->drawable).clear_timestamp();
            //const_cast<Drawable&>(this->drawable).clear_mouse();
            this->has_first_img = true;
            this->trans.next();
        }
    };

    struct VideoSequencerAction
    {
        SequencedVideoCaptureImpl & impl;

        void operator()(const timeval& now) const {
            this->impl.next_video_impl(now, NotifyNextVideo::reason::sequenced);
        }
    };

    using VideoSequencer = SequencerCapture<VideoSequencerAction>;

    // first next_video is ignored
    struct FirstImage : gdi::CaptureApi
    {
        using capture_list_t = std::vector<std::reference_wrapper<gdi::CaptureApi>>;

        SequencedVideoCaptureImpl & impl;
        ApiRegisterElement<gdi::CaptureApi> cap_elem;
        ApiRegisterElement<gdi::CaptureApi> gcap_elem;

        using seconds = std::chrono::seconds;
        using microseconds = std::chrono::microseconds;

        const timeval start_capture;

        FirstImage(timeval const & now, SequencedVideoCaptureImpl & impl)
        : impl(impl)
        , start_capture(now)
        {}

        std::chrono::microseconds do_snapshot(
            const timeval& now, int x, int y, bool ignore_frame_in_timeval
        ) override {
            microseconds ret;

            auto const duration = microseconds(difftimeval(now, this->start_capture));
            auto const interval = microseconds(seconds(3))/2;
            if (duration >= interval) {
                auto video_interval = this->impl.video_sequencer.get_interval();
                if (this->impl.ic.logical_frame_ended() || duration > seconds(2) || duration >= video_interval) {
                    this->impl.ic.breakpoint_image(now);
                    assert(this->cap_elem == *this);
                    assert(this->gcap_elem == *this);
                    this->cap_elem = this->impl.video_sequencer;
                    this->gcap_elem = this->impl.video_sequencer;

                    ret = video_interval;
                }
                else {
                    ret = interval / 3;
                }
            }
            else {
                ret = microseconds(interval - duration);
            }

            return std::min(ret, this->impl.video_sequencer.snapshot(now, x, y, ignore_frame_in_timeval));
        }

        void do_resume_capture(const timeval& now) override { this->impl.video_sequencer.resume_capture(now); }
        void do_pause_capture(const timeval& now) override { this->impl.video_sequencer.pause_capture(now); }
    };

    VideoTransport vc_trans;
    VideoCapture vc;
    PreparingWhenFrameMarkerEnd preparing_vc{vc};

    OutFilenameSequenceTransport ic_trans;
    ImageToFile ic;

    VideoSequencer video_sequencer;
    FirstImage first_image;

    NotifyNextVideo & next_video_notifier;

    void next_video_impl(const timeval& now, NotifyNextVideo::reason reason) {
        this->video_sequencer.reset_now(now);
        if (!this->ic.has_first_img) {
            this->ic.breakpoint_image(now);
            this->first_image.cap_elem = this->video_sequencer;
            this->first_image.gcap_elem = this->video_sequencer;
        }
        this->vc.next_video();
        this->ic.breakpoint_image(now);
        this->next_video_notifier.notify_next_video(now, reason);
    }

public:
    SequencedVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        const Drawable & drawable,
        VideoParams video_param,
        std::chrono::microseconds video_interval,
        NotifyNextVideo & next_video_notifier)
    : vc_trans(record_path, basename, ("." + video_param.codec).c_str(), groupid)
    , vc(now, this->vc_trans, drawable, no_timestamp, std::move(video_param))
    , ic_trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_path, basename, ".png", groupid)
    , ic(this->ic_trans, drawable)
    , video_sequencer(
        now, video_interval <= std::chrono::microseconds(0) ? video_interval : std::chrono::microseconds::max(),
        VideoSequencerAction{*this})
    , first_image(now, *this)
    , next_video_notifier(next_video_notifier)
    {}

    void attach_apis(ApisRegister & apis_register, const Inifile &) {
        apis_register.capture_list.push_back(this->vc);
        apis_register.graphic_snapshot_list->push_back(this->preparing_vc);
        this->first_image.cap_elem = {apis_register.capture_list, this->first_image};
        this->first_image.gcap_elem = {*apis_register.graphic_snapshot_list, this->first_image};
    }

    void next_video(const timeval& now) {
        this->next_video_impl(now, NotifyNextVideo::reason::external);
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->vc_trans.request_full_cleaning();
        this->ic_trans.request_full_cleaning();
    }

    void image_zoom(unsigned percent) {
        this->ic.zoom(percent);
    }
};


class MetaCaptureImpl
{
    local_fd fd;
    OutFileTransport meta_trans;
    SessionMeta meta;
    SessionLogAgent session_log_agent;
    bool enable_agent;

public:
    MetaCaptureImpl(
        const timeval & now,
        std::string record_path,
        const char * const basename,
        bool enable_agent)
    : fd([](const char * filename){
        int fd = ::open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0440);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        return fd;
    }(record_path.append(basename).append(".meta").c_str()))
    , meta_trans(this->fd.get())
    , meta(now, this->meta_trans)
    , session_log_agent(this->meta)
    , enable_agent(enable_agent)
    {
    }

    void attach_apis(ApisRegister & apis_register, const Inifile & ini) {
        apis_register.capture_list.push_back(this->meta);
        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta)) {
            apis_register.kbd_input_list.push_back(this->meta);
            apis_register.capture_probe_list.push_back(this->meta);
        }

        if (this->enable_agent) {
            apis_register.capture_probe_list.push_back(this->session_log_agent);
        }
    }

    SessionMeta & get_session_meta() {
        return this->meta;
    }

    void request_full_cleaning() {
        this->meta_trans.request_full_cleaning();
    }
};


struct NotifyTitleChanged : private noncopyable
{
    virtual void notify_title_changed(const timeval & now, array_view_const_char title) = 0;
    virtual ~NotifyTitleChanged() = default;
};

class TitleCaptureImpl final : gdi::CaptureApi, gdi::CaptureProbeApi
{
    OcrTitleExtractorBuilder ocr_title_extractor_builder;
    AgentTitleExtractor agent_title_extractor;

    std::reference_wrapper<TitleExtractorApi> title_extractor;

    timeval  last_ocr;
    std::chrono::microseconds usec_ocr_interval;

    NotifyTitleChanged & notify_title_changed;

public:
    TitleCaptureImpl(
        const timeval & now,
        auth_api * authentifier,
        const Drawable & drawable,
        const Inifile & ini,
        NotifyTitleChanged & notify_title_changed)
    : ocr_title_extractor_builder(
        drawable, authentifier != nullptr,
        ini.get<cfg::ocr::version>(),
        static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
        ini.get<cfg::ocr::on_title_bar_only>(),
        ini.get<cfg::ocr::max_unrecog_char_rate>())
    , title_extractor(this->ocr_title_extractor_builder.get_title_extractor())
    , last_ocr(now)
    , usec_ocr_interval(ini.get<cfg::ocr::interval>())
    , notify_title_changed(notify_title_changed)
    {
    }

    void attach_apis(ApisRegister & apis_register, const Inifile & /*ini*/) {
        apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this));
        apis_register.capture_probe_list.push_back(static_cast<gdi::CaptureProbeApi&>(*this));
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const diff {difftimeval(now, this->last_ocr)};

        using std::chrono::milliseconds;
        using std::chrono::duration_cast;

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
