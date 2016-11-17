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

#include <algorithm>

#include "transport/out_meta_sequence_transport.hpp"

#include "capture/image_capture.hpp"
#include "utils/drawable.hpp"

#include "apis_register.hpp"


class ImageCaptureImpl final : private gdi::UpdateConfigCaptureApi, gdi::CaptureApi
{
    struct ImageTransportBuilder final : private Transport
    {
        OutFilenameSequenceTransport trans;
        bool enable_rt;
        uint32_t num_start = 0;
        unsigned png_limit;

        ImageTransportBuilder(
            const char * path, const char * basename, int groupid,
            auth_api * authentifier, bool enable_rt, Inifile const & ini)
        : trans(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
            path, basename, ".png", groupid, authentifier)
        , enable_rt(enable_rt)
        , png_limit(enable_rt ? ini.get<cfg::video::png_limit>() : 0)
        {}

        ~ImageTransportBuilder() {
            if (this->enable_rt) {
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

        void do_send(const uint8_t * const buffer, size_t len) override {
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

    std::chrono::microseconds png_interval;
    ImageTransportBuilder trans_builder;
    ImageCapture ic;
    bool enable_rt_display = false;

public:
    ImageCaptureImpl(
        const timeval & now, bool enable_rt, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const Inifile & ini)
    : png_interval(ini.get<cfg::video::png_interval>())
    , trans_builder(record_tmp_path, basename, groupid, authentifier, enable_rt, ini)
    , ic(now, drawable, this->trans_builder.get_transport(), this->png_interval)
    {}

    void attach_apis(ApisRegister & apis_register, const Inifile & ini) {
        if (this->trans_builder.enable_rt) {
            this->enable_rt_display = ini.get<cfg::video::rt_display>();
            apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this));
            apis_register.graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this));
            apis_register.update_config_capture_list.push_back(static_cast<gdi::UpdateConfigCaptureApi&>(*this));
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
        auto const old_enable_rt_display = this->enable_rt_display;
        this->enable_rt_display = ini.get<cfg::video::rt_display>();

        if (old_enable_rt_display == this->enable_rt_display) {
            return ;
        }

        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable real time: %d", int(this->enable_rt_display));
        }

        if (!this->enable_rt_display) {
            this->trans_builder.unlink_all_png();
        }
    }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override {
        if (this->enable_rt_display) {
            return this->ic.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->png_interval;
    }

    void do_pause_capture(timeval const & now) override {
        if (this->enable_rt_display) {
            this->ic.pause_capture(now);
        }
    }

    void do_resume_capture(timeval const & now) override {
        if (this->enable_rt_display) {
            this->ic.resume_capture(now);
        }
    }
};

