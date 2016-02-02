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

#include "noncopyable.hpp"

#include "adapter_base.hpp"

#include <chrono>
#include <algorithm> // std::min

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

    virtual void pause_capture(timeval const & now) = 0;
    virtual void resume_capture(timeval const & now) = 0;
    // virtual start_capture(timeval const & now) = 0;
    // virtual stop_capture(timeval const &  now) = 0;

    // TODO other interface
    virtual void external_breakpoint() = 0;
    virtual void external_time(const timeval& now) = 0;
};

struct CaptureProxy
{
    struct snapshot_tag {};
    struct update_config_tag {};
    struct pause_capture_tag {};
    struct resume_capture_tag {};
    struct external_breakpoint {};
    struct external_time {};

    template<class Api>
    std::chrono::microseconds operator()(
        snapshot_tag, Api & api,
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) {
        return api.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
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

    template<class Api>
    void operator()(external_time, Api & api, timeval const & now) {
        api.external_time(now);
    }

    template<class Api>
    void operator()(external_breakpoint, Api & api) {
        api.external_breakpoint();
    }
};

template<class Proxy, class InterfaceBase = CaptureApi>
struct CaptureAdapter : AdapterBase<Proxy, InterfaceBase>
{
    static_assert(std::is_base_of<CaptureApi, InterfaceBase>::value, "InterfaceBase isn't a CaptureApi");

    using AdapterBase<Proxy, InterfaceBase>::AdapterBase;

    std::chrono::microseconds snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override {
        return this->get_proxy()(
            CaptureProxy::snapshot_tag{}, *this,
            now, cursor_x, cursor_y, ignore_frame_in_timeval
        );
    }

    void update_config(Inifile const & ini) override {
        this->get_proxy()(CaptureProxy::update_config_tag{}, *this, ini);
    }

    void pause_capture (timeval const & now) override {
        this->get_proxy()(CaptureProxy::pause_capture_tag{}, *this, now);
    }

    void resume_capture(timeval const & now) override {
        this->get_proxy()(CaptureProxy::resume_capture_tag{}, *this, now);
    }

    void external_breakpoint() override {
        this->get_proxy()(CaptureProxy::external_breakpoint{}, *this);
    }

    void external_time(const timeval& now) override {
        this->get_proxy()(CaptureProxy::external_time{}, *this, now);
    }
};


namespace {
    struct CaptureCoreAccess
    {
        template<class Derived>
        static auto get_capture_list(Derived & derived)
        -> decltype(derived.get_capture_list_impl()) {
            return derived.get_capture_list_impl();
        }

        template<class Derived, class Gd>
        static auto to_capture_facade(Derived & derived, Gd & cap)
        -> decltype(derived.to_capture_facade_impl(cap)) {
            return derived.to_capture_facade_impl(cap);
        }
    };
}

template<class Derived, class InterfaceBase = CaptureApi, class CoreAccess = CaptureCoreAccess>
class CaptureDispatcher : public InterfaceBase
{
    static_assert(std::is_base_of<CaptureApi, InterfaceBase>::value, "InterfaceBase isn't a CaptureApi");

    friend CoreAccess;

protected:
    using base_type = CaptureDispatcher;

public:
    using InterfaceBase::InterfaceBase;

    std::chrono::microseconds snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override {
        std::chrono::microseconds time = std::chrono::microseconds::max();
        for (auto && cap : CoreAccess::get_capture_list(this->derived())) {
            time = std::min(
                time,
                CoreAccess::to_capture_facade(this->derived(), cap)
                    .snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval)
            );
        }
        return time;
    }

    void update_config(Inifile const & ini) override {
        for (auto && cap : CoreAccess::get_capture_list(this->derived())) {
            CoreAccess::to_capture_facade(this->derived(), cap).update_config(ini);
        }
    }

    void pause_capture (timeval const & now) override {
        for (auto && cap : CoreAccess::get_capture_list(this->derived())) {
            CoreAccess::to_capture_facade(this->derived(), cap).pause_capture(now);
        }
    }

    void resume_capture(timeval const & now) override {
        for (auto && cap : CoreAccess::get_capture_list(this->derived())) {
            CoreAccess::to_capture_facade(this->derived(), cap).resume_capture(now);
        }
    }

    void external_breakpoint() override {
        for (auto && cap : CoreAccess::get_capture_list(this->derived())) {
            CoreAccess::to_capture_facade(this->derived(), cap).external_breakpoint();
        }
    }

    void external_time(const timeval& now) override {
        for (auto && cap : CoreAccess::get_capture_list(this->derived())) {
            CoreAccess::to_capture_facade(this->derived(), cap).external_time(now);
        }
    }

protected:
    Derived & derived() {
        return static_cast<Derived&>(*this);
    }

    CaptureApi & to_capture_facade_impl(CaptureApi & cap) {
        return cap;
    }
};

}

#endif
