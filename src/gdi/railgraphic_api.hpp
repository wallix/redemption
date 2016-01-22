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

#ifndef REDEMPTION_GDI_RAILGRAPHIC_API_HPP
#define REDEMPTION_GDI_RAILGRAPHIC_API_HPP

#include <memory>

#include <cstdint>
#include <type_traits>

#include "meta/meta.hpp"
#include "utils/virtual_deleter.hpp"

#include "proxy.hpp"

#include "noncopyable.hpp"


namespace RDP {
    namespace RAIL {
        class NewOrExistingWindow;
        class WindowIcon;
        class CachedIcon;
        class DeletedWindow;
    }
}

namespace gdi {

struct RAILGraphicApi : private noncopyable
{
    virtual ~RAILGraphicApi() = default;

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) = 0;
    virtual void draw(const RDP::RAIL::WindowIcon          & order) = 0;
    virtual void draw(const RDP::RAIL::CachedIcon          & order) = 0;
    virtual void draw(const RDP::RAIL::DeletedWindow       & order) = 0;
};

struct RAILGraphicProxy
{
    template<class Api, class Ts>
    void operator()(Api & api, Ts const & order) {
        api.draw(order);
    }
};

template<class Proxy, class InterfaceBase = RAILGraphicApi>
struct RAILGraphicAdaptor : AdaptorBase<Proxy, InterfaceBase>
{
    static_assert(std::is_base_of<RAILGraphicApi, InterfaceBase>::value, "InterfaceBase isn't a RAILGraphicApi");

    using AdaptorBase<Proxy, InterfaceBase>::AdaptorBase;

    void draw(const RDP::RAIL::NewOrExistingWindow & order) { this->prox()(*this, order); }
    void draw(const RDP::RAIL::WindowIcon          & order) { this->prox()(*this, order); }
    void draw(const RDP::RAIL::CachedIcon          & order) { this->prox()(*this, order); }
    void draw(const RDP::RAIL::DeletedWindow       & order) { this->prox()(*this, order); }
};

}

#endif
