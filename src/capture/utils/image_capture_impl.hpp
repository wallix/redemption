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

#ifndef REDEMPTION_UTILS_IMAGE_CAPTURE_IMPL_HPP
#define REDEMPTION_UTILS_IMAGE_CAPTURE_IMPL_HPP

#include <algorithm>

#include "transport/out_filename_sequence_transport.hpp"

#include "capture/image_capture.hpp"
#include "utils/drawable.hpp"

#include "apis_register.hpp"


class ImageCaptureImpl final : private gdi::CaptureApi
{
    struct ImageTransportBuilder final : private Transport
    {
        OutFilenameSequenceTransport trans;
        bool clear_png;
        uint32_t num_start = 0;
        unsigned png_limit;

        ImageTransportBuilder(
            const char * path, const char * basename, int groupid,
            auth_api * authentifier, bool clear_png, Inifile const & ini)
        : trans(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
            path, basename, ".png", groupid, authentifier)
        , clear_png(clear_png)
        , png_limit(clear_png ? ini.get<cfg::video::png_limit>() : 0)
        {}

        ~ImageTransportBuilder() {
            if (this->clear_png) {
                this->unlink_all_png();
            }
        }

        bool next() override {
            if (this->png_limit && this->trans.get_seqno() >= this->png_limit) {
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->trans.seqgen()->get(this->trans.get_seqno() - this->png_limit));
            }
            return this->trans.next();
        }

        void do_send(const char * const buffer, size_t len) override {
            this->trans.send(buffer, len);
        }

        Transport & get_transport() {
            return this->png_limit ? static_cast<Transport&>(*this) : this->trans;
        }

        void unlink_all_png()
        {
            for(uint32_t until_num = this->trans.get_seqno() + 1; this->num_start < until_num; ++this->num_start) {
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->trans.seqgen()->get(this->num_start));
            }
        }
    };

    struct SnapshotDisabled : gdi::CaptureApi
    {
        ImageCaptureImpl & impl;

        SnapshotDisabled(ImageCaptureImpl & impl)
        : impl(impl)
        {}

        std::chrono::microseconds snapshot(const timeval&, int, int, bool) override {
            return this->impl.png_interval;
        }

        void update_config(Inifile const & ini) override {
            this->impl.update_config(ini);
        }

        void pause_capture(timeval const &) override {}
        void resume_capture(timeval const &) override {}

        void external_breakpoint() override {}
        void external_time(timeval const &) override {}
    };

    std::chrono::microseconds png_interval;
    ImageTransportBuilder trans_builder;
    ImageCapture ic;
    SnapshotDisabled disable_capture;
    bool enable_rt_display = false;

    using capture_list_t = std::vector <std::reference_wrapper <gdi::CaptureApi > >;

    capture_list_t * capture_list = nullptr;
    capture_list_t * graphic_snapshot_list = nullptr;

public:
    ImageCaptureImpl(
        const timeval & now, bool clear_png, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const Inifile & ini)
    : png_interval(
        std::chrono::duration<std::chrono::microseconds::rep, std::ratio<1, 10>>(
            ini.get<cfg::video::png_interval>()))
    , trans_builder(record_tmp_path, basename, groupid, authentifier, clear_png, ini)
    , ic(now, drawable, this->trans_builder.get_transport(), this->png_interval)
    , disable_capture(*this)
    {}

    void attach_apis(ApisRegister & apis_register, const Inifile & ini) {
        if (this->trans_builder.clear_png) {
            this->capture_list = &apis_register.capture_list;
            this->graphic_snapshot_list = apis_register.graphic_snapshot_list;
            this->enable_rt_display = ini.get<cfg::video::rt_display>();
            if (this->enable_rt_display) {
                this->capture_list->push_back(*this);
                this->graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this));
            }
            else {
                this->capture_list->push_back(this->disable_capture);
            }
            this->update_config(ini);
        }
        else {
            apis_register.capture_list.push_back(this->ic);
            apis_register.graphic_snapshot_list->push_back(this->ic);
        }
    }

    void zoom(unsigned percent) {
        this->ic.zoom(percent);
    }

    void next(const timeval & now) {
        this->ic.breakpoint(now);
    }

private:
    void update_config(Inifile const & ini) override {
        if (this->trans_builder.clear_png) {
            assert(this->capture_list && this->graphic_snapshot_list);

            auto find_capture = [&](capture_list_t & list, gdi::CaptureApi & cap) {
                auto is_cap = [&cap](gdi::CaptureApi & other) { return &other == &cap; };
                return std::find_if(this->capture_list->begin(), this->capture_list->end(), is_cap);
            };

            auto const old_enable_rt_display = this->enable_rt_display;
            this->enable_rt_display = ini.get<cfg::video::rt_display>();

            if (old_enable_rt_display == this->enable_rt_display) {
                return ;
            }

            if (this->enable_rt_display) {
                auto p = find_capture(*this->capture_list, this->disable_capture);
                assert(p != this->capture_list->end());
                *p = *this;
                this->graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this));
                this->trans_builder.png_limit = ini.get<cfg::video::png_limit>();
            }
            else {
                auto p = find_capture(*this->capture_list, *this);
                assert(p != this->capture_list->end());
                *p = this->disable_capture;
                this->graphic_snapshot_list->erase(find_capture(*this->graphic_snapshot_list, *this));
                this->trans_builder.unlink_all_png();
            }
        }
    }

    std::chrono::microseconds snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override {
        return this->ic.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    void external_breakpoint() override { this->ic.external_breakpoint(); }
    void external_time(const timeval& now) override { this->ic.external_time(now); }
    void resume_capture(timeval const & now) override { this->ic.resume_capture(now); }
    void pause_capture(const timeval& now) override { this->ic.pause_capture(now); }
};

#endif
