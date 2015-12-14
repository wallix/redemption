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

#include "noncopyable.hpp"

class RDPColCache;
class RDPBrushCache;
class RDPMemBlt;
class RDPMem3Blt;

namespace gdi {

struct CacheApi : private noncopyable
{
    virtual ~CacheApi() = default;

    virtual void cache(RDPColCache   const & cmd) = 0;
    virtual void cache(RDPBrushCache const & cmd) = 0;
    virtual void cache(RDPMemBlt     const & cmd) = 0;
    virtual void cache(RDPMem3Blt    const & cmd) = 0;
};

using CacheApiDeleterBase = utils::virtual_deleter_base<CacheApi>;
using CacheApiPtr = utils::unique_ptr_with_virtual_deleter<CacheApi>;

using utils::default_delete;
using utils::no_delete;

template<class Cache, class... Args>
CacheApiPtr make_cache_ptr(Args && ... args) {
    return CacheApiPtr(new Cache(std::forward<Args>(args)...), default_delete);
}

template<class Cache>
CacheApiPtr make_cache_ref(Cache & gd) {
    return CacheApiPtr(&gd, no_delete);
}

}

#endif
