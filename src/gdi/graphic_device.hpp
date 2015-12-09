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

#ifndef REDEMPTION_GDI_GRAPHIC_DEVICE_HPP
#define REDEMPTION_GDI_GRAPHIC_DEVICE_HPP

#include <memory>

#include <cstdint>

#include "meta/meta.hpp"

#include "log.hpp"
#include "noncopyable.hpp"
#include "rect.hpp"


class Rect;
class RDPOpaqueRect;


namespace gdi {

using BGRColor = uint32_t;

class GraphicDevice;

struct GraphicDeviceDeleterBase
{
    virtual ~GraphicDeviceDeleterBase() = default;
    virtual void operator()(GraphicDevice * gd) = 0;
};


using DefaultDelete = std::default_delete<GraphicDeviceDeleterBase>;
struct NoDelete { constexpr NoDelete() noexcept = default; };

namespace {
    auto && default_delete = meta::static_const<DefaultDelete>::value;
    auto && no_delete = meta::static_const<NoDelete>::value;
}

struct GraphicDeviceDeleter
{
    GraphicDeviceDeleter(bool owner)
    : pdeleter{owner ? gdi_default_delete() : gdi_no_delete()}
    {}

    GraphicDeviceDeleter(DefaultDelete)
    : pdeleter{gdi_default_delete()}
    {}

    GraphicDeviceDeleter(NoDelete)
    : pdeleter{gdi_no_delete()}
    {}

    template<class Deleter>
    GraphicDeviceDeleter(Deleter && deleter);

    GraphicDeviceDeleter(GraphicDeviceDeleter &&) = default;
    GraphicDeviceDeleter(GraphicDeviceDeleter const &) = delete;
    GraphicDeviceDeleter & operator = (GraphicDeviceDeleter &&) = default;
    GraphicDeviceDeleter & operator = (GraphicDeviceDeleter const &) = delete;

    void operator()(GraphicDevice * gd) noexcept {
        (*this->pdeleter)(gd);
    }

private:
    template<class Deleter>
    struct gdi_user_delete_impl : GraphicDeviceDeleterBase
    {
        Deleter deleter;

        template<class D>
        gdi_user_delete_impl(D && d)
        : deleter(std::forward<D>(d))
        {}

        void operator()(GraphicDevice * gd) {
            this->deleter(gd);
        }
    };

    static GraphicDeviceDeleterBase * gdi_no_delete() noexcept {
        static struct : GraphicDeviceDeleterBase {
            void operator()(GraphicDevice *) {}
        } impl;
        return &impl;
    }

    static GraphicDeviceDeleterBase * gdi_default_delete() noexcept {
        static struct : GraphicDeviceDeleterBase {
            void operator()(GraphicDevice * gd) {
                std::default_delete<GraphicDevice>()(gd);
            }
        } impl;
        return &impl;
    }

    struct gd_deleter {
        void operator()(GraphicDeviceDeleterBase * gd) noexcept {
            if (gdi_default_delete() != gd && gdi_no_delete() != gd) {
                delete gd;
            }
        }
    };

    std::unique_ptr<GraphicDeviceDeleterBase, gd_deleter> pdeleter;
};


template<class Deleter>
GraphicDeviceDeleter::GraphicDeviceDeleter(Deleter&& deleter)
: pdeleter{new gdi_user_delete_impl<Deleter>{std::forward<Deleter>(deleter)}}
{}



using GraphicDevicePtr = std::unique_ptr<GraphicDevice, GraphicDeviceDeleter>;


template<class Gd, class... Args>
GraphicDevicePtr make_gd_ptr(Args && ... args) {
    return GraphicDevicePtr(new Gd(std::forward<Args>(args)...), default_delete);
}

template<class Gd>
GraphicDevicePtr make_gd_ref(Gd & gd) {
    return GraphicDevicePtr(&gd, no_delete);
}


struct GraphicDevice : private noncopyable
{
    GraphicDevice() = default;
    virtual ~GraphicDevice() = default;

    virtual void draw(RDPOpaqueRect const & cmd, Rect const & rect) = 0;
    virtual void replaced_by(GraphicDevicePtr &&) { }
};

}

#endif
