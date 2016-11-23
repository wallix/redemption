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
#include <memory>

#include "transport/out_meta_sequence_transport.hpp"

// private extension
#include "capture/sequencer.hpp"
#include "capture/video_capture.hpp"
#include "capture/utils/video_params_from_ini.hpp"
#include "capture/rdp_ppocr/get_ocr_constants.hpp"
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

class VideoCaptureImpl
{
    class VideoTransport final : public OutFilenameSequenceSeekableTransport
    {
        using transport_base = OutFilenameSequenceSeekableTransport;

    public:
        VideoTransport(
            FilenameGenerator::Format format,
            const char * const record_path,
            const char * const basename,
            const char * const suffix,
            const int groupid
        )
        : transport_base(format, record_path, basename, suffix, groupid)
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

public:
    struct SynchronizerNext
    {
        SessionMeta * meta;
        ImageCapture * image;
    };

private:
    struct VideoSequencerAction
    {
        VideoCaptureImpl & impl;

        void operator()(const timeval& now) const {
            this->impl.vc.next_video();
            if (this->impl.synchronizer_next.meta) {
                this->impl.synchronizer_next.meta->send_line(now.tv_sec, cstr_array_view("(break)"));
            }
            if (this->impl.synchronizer_next.image) {
                this->impl.synchronizer_next.image->breakpoint(now);
            }
        }
    };

    using VideoSequencer = SequencerCapture<VideoSequencerAction>;

    struct FirstImage : gdi::CaptureApi
    {
        using capture_list_t = std::vector <std::reference_wrapper <gdi::CaptureApi > >;

        VideoSequencer & video_sequencer;
        ImageCapture * image;
        ApiRegisterElement<gdi::CaptureApi> cap_elem;
        ApiRegisterElement<gdi::CaptureApi> gcap_elem;

        FirstImage(VideoSequencer & video_sequencer, ImageCapture * image)
        : video_sequencer(video_sequencer)
        , image(image)
        {}

        std::chrono::microseconds do_snapshot(const timeval& now, int, int, bool) override {
            this->image->breakpoint(now);
            assert(this->cap_elem == *this);
            assert(this->gcap_elem == *this);
            this->cap_elem = this->video_sequencer;
            this->gcap_elem = this->video_sequencer;
            return {};
        }

        void do_resume_capture(const timeval& now) override { this->video_sequencer.resume_capture(now); }
        void do_pause_capture(const timeval& now) override { this->video_sequencer.pause_capture(now); }
    };

    struct PreparingWhenFrameMarkerEnd : gdi::CaptureApi
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

    VideoTransport trans;
    VideoCapture vc;
    PreparingWhenFrameMarkerEnd preparing_vc{vc};
    SynchronizerNext synchronizer_next;
    VideoSequencer video_sequencer;
    FirstImage first_image;
    bool enable_preparing_video;

public:
    VideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        auth_api * authentifier,
        bool no_timestamp,
        const Drawable & drawable,
        VideoParams video_param,
        std::chrono::microseconds video_interval,
        SynchronizerNext video_synchronizer_next)
    : trans(
        video_interval.count()
            ? FilenameGenerator::PATH_FILE_COUNT_EXTENSION
            : FilenameGenerator::PATH_FILE_EXTENSION,
        record_path, basename, ("." + video_param.codec).c_str(), groupid)
    , vc(now, this->trans, drawable, no_timestamp, std::move(video_param))
    , synchronizer_next(video_synchronizer_next)
    , video_sequencer(now, video_interval, VideoSequencerAction{*this})
    , first_image(this->video_sequencer, this->synchronizer_next.image)
    , enable_preparing_video(!authentifier)
    {}

    void attach_apis(ApisRegister & apis_register, const Inifile &) {
        apis_register.capture_list.push_back(this->vc);
        apis_register.graphic_snapshot_list->push_back(
            this->enable_preparing_video
          ? static_cast<gdi::CaptureApi&>(this->preparing_vc)
          : static_cast<gdi::CaptureApi&>(this->vc)
        );
        if (this->video_sequencer.get_interval().count()) {
            if (this->synchronizer_next.image) {
                this->first_image.cap_elem = {apis_register.capture_list, this->first_image};
                this->first_image.gcap_elem = {*apis_register.graphic_snapshot_list, this->first_image};
            }
            else {
                apis_register.capture_list.push_back(this->video_sequencer);
                apis_register.graphic_snapshot_list->push_back(this->video_sequencer);
            }
        }
    }

    void next_video(const timeval& now) {
        this->video_sequencer.reset_now(now);
        this->vc.next_video();
        if (this->synchronizer_next.image) {
            this->synchronizer_next.image->breakpoint(now);
        }
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->trans.request_full_cleaning();
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


class TitleCaptureImpl final : gdi::CaptureApi, gdi::CaptureProbeApi
{
    OcrTitleExtractorBuilder ocr_title_extractor_builder;
    AgentTitleExtractor agent_title_extractor;

    std::reference_wrapper<TitleExtractorApi> title_extractor;

    timeval  last_ocr;
    std::chrono::microseconds usec_ocr_interval;

    PatternsChecker pattern_checker;

    SessionMeta * session_meta;
    VideoCaptureImpl * video;

public:
    TitleCaptureImpl(
        const timeval & now,
        auth_api * authentifier,
        const Drawable & drawable,
        SessionMeta * session_meta,
        VideoCaptureImpl * video,
        const Inifile & ini)
    : ocr_title_extractor_builder(
        drawable, authentifier != nullptr,
        ini.get<cfg::ocr::version>(),
        static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
        ini.get<cfg::ocr::on_title_bar_only>(),
        ini.get<cfg::ocr::max_unrecog_char_rate>())
    , title_extractor(this->ocr_title_extractor_builder.get_title_extractor())
    , last_ocr(now)
    , usec_ocr_interval(ini.get<cfg::ocr::interval>() * 10000L)
    , pattern_checker(
        authentifier,
        ini.get<cfg::context::pattern_kill>().c_str(),
        ini.get<cfg::context::pattern_notify>().c_str())
    , session_meta(session_meta)
    , video(video)
    {
    }

    void attach_apis(ApisRegister & apis_register, const Inifile & /*ini*/) {
        // TODO this->session_meta || this->video || this->pattern_checker.contains_pattern() ?
        apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this));
        apis_register.capture_probe_list.push_back(static_cast<gdi::CaptureProbeApi&>(*this));
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const diff {difftimeval(now, this->last_ocr)};

        if (diff >= this->usec_ocr_interval) {
            this->last_ocr = now;

            auto title = this->title_extractor.get().extract_title();

            if (title.data()/* && title.size()*/) {
                if (this->session_meta) {
                    this->session_meta->title_changed(now.tv_sec, title);
                }
                if (this->video) {
                    this->video->next_video(now);
                }
                this->pattern_checker(title);
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
