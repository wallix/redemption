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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_GDI_CAPTURE_API_HPP
#define REDEMPTION_GDI_CAPTURE_API_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"

// TODO array_view_fwd
#include "array_view.hpp"
#include "proxy.hpp"

#include <chrono>

struct timeval;
class wait_obj;
class Inifile;

namespace gdi {

struct CaptureApi : private noncopyable
{
    virtual ~CaptureApi() = default;

    virtual std::chrono::microseconds snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) = 0;


    // TODO for rt_display in StaticCapture ... enable_realtime(bool)
    virtual void update_config(Inifile const & ini) = 0;

    // TODO event ?
    //virtual void possible_active_window_change() = 0;
    //virtual void session_update(const timeval & now, array_view<const char> const & message) = 0;

    virtual void pause_capture(timeval const & now) = 0;
    virtual void resume_capture(timeval const & now) = 0;
    // virtual start_capture(timeval const & now) = 0;
    // virtual stop_capture(timeval const &  now) = 0;
};

struct CaptureProxy
{
    struct snapshot_tag {};
    struct update_config_tag {};
    struct pause_capture_tag {};
    struct resume_capture_tag {};

    template<class Api>
    std::chrono::microseconds operator()(
        snapshot_tag, Api & api,
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) {
        api.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    template<class Api>
    void operator()(update_config_tag, Api & api, Inifile const & ini) {
        api.update_config(ini);
    }

    template<class Api>
    void operator()(pause_capture_tag, Api & api, timeval const & now) {
        api.pause_capture(now);
    }

    template<class Api>
    void operator()(resume_capture_tag, Api & api, timeval const & now) {
        api.resume_capture(now);
    }
};

template<class Proxy, class InterfaceBase = CaptureApi>
struct CaptureDelegate : ProxyBase<Proxy, InterfaceBase>
{
    static_assert(std::is_base_of<CaptureApi, InterfaceBase>::value, "InterfaceBase isn't a CaptureApi");

    using ProxyBase<Proxy, InterfaceBase>::ProxyBase;

    std::chrono::microseconds snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override {
        return this->prox()(
            CaptureProxy::snapshot_tag{}, this->base(),
            now, cursor_x, cursor_y, ignore_frame_in_timeval
        );
    }

    void update_config(Inifile const & ini) {
        this->prox()(CaptureProxy::update_config_tag{}, this->base(), ini);
    }

    void pause_capture (timeval const & now) {
        this->prox()(CaptureProxy::pause_capture_tag{}, this->base(), now);
    }

    void resume_capture(timeval const & now) {
        this->prox()(CaptureProxy::resume_capture_tag{}, this->base(), now);
    }
};

}

#endif
