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

#ifndef REDEMPTION_GDI_CACHE_API_HPP
#define REDEMPTION_GDI_CACHE_API_HPP

#include "utils/virtual_deleter.hpp"

#include "proxy.hpp"

#include "noncopyable.hpp"

class RDPColCache;
class RDPBrushCache;

namespace gdi {

struct CacheApi : private noncopyable
{
    virtual ~CacheApi() = default;

    virtual void cache(RDPColCache   const & cmd) = 0;
    virtual void cache(RDPBrushCache const & cmd) = 0;
};

struct CacheProxy
{
    template<class Api, class T>
    void operator()(Api & api, T const & cmd) {
        api.cache(cmd);
    }
};


template<class Proxy, class InterfaceBase = CacheApi>
struct CacheAdaptor : AdaptorBase<Proxy, InterfaceBase>
{
    static_assert(std::is_base_of<CacheApi, InterfaceBase>::value, "InterfaceBase isn't a CacheApi");

    using AdaptorBase<Proxy, InterfaceBase>::AdaptorBase;

    void cache(RDPColCache   const & cmd) override { this->prox()(*this, cmd); }
    void cache(RDPBrushCache const & cmd) override { this->prox()(*this, cmd); }
};

}

#endif
