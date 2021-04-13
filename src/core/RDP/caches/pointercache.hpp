/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/sugar/noncopyable.hpp"
#include "utils/cache_index_mapping.hpp"
#include "utils/log.hpp"
#include "core/error.hpp"
#include "cxx/cxx.hpp"
#include "gdi/graphic_api.hpp"
#include "core/RDP/rdp_pointer.hpp"


class PointerCache : noncopyable
{
public:
    static constexpr std::size_t MAX_POINTER_COUNT = gdi::CachePointerIndex::MAX_POINTER_COUNT;

    struct SourcePointersView
    {
        explicit SourcePointersView(std::array<RdpPointer, MAX_POINTER_COUNT> const& pointers) noexcept
        : pointers(pointers)
        {}

        RdpPointer const& pointer(gdi::CachePointerIndex cache_idx) const noexcept
        {
            assert(cache_idx.cache_index() < MAX_POINTER_COUNT + RESERVED_POINTER_COUNT);

            return cache_idx.is_predefined_pointer()
                ? predefined_pointer_to_pointer(cache_idx.as_predefined_pointer())
                : pointers[cache_idx.cache_index()];
        }

    private:
        std::array<RdpPointer, MAX_POINTER_COUNT> const& pointers;
    };

private:
    static constexpr std::size_t RESERVED_POINTER_COUNT = std::size_t(PredefinedPointer::COUNT);

public:
    using Cache = CacheIndexMapping<MAX_POINTER_COUNT + RESERVED_POINTER_COUNT, uint16_t>;
    using CacheResult = Cache::CacheResult;

    explicit PointerCache(uint16_t pointer_cache_entries)
    : cache(pointer_cache_entries)
    {}

    constexpr static uint16_t max_size() noexcept
    {
        return MAX_POINTER_COUNT;
    }

    void insert(gdi::CachePointerIndex cache_idx, RdpPointerView const& cursor)
    {
        auto idx = cache_idx.cache_index();
        check(idx, max_size());
        pointers[idx] = cursor;
        cache.insert(idx);
    }

    CacheResult use(gdi::CachePointerIndex cache_idx) noexcept
    {
        auto idx = cache_idx.cache_index();
        check(idx, max_size() + RESERVED_POINTER_COUNT);
        return cache.use(idx);
    }

    RdpPointer const& pointer(gdi::CachePointerIndex cache_idx) const noexcept
    {
        return SourcePointersView{pointers}.pointer(cache_idx);
    }

    SourcePointersView source_pointers_view() const noexcept
    {
        return SourcePointersView{pointers};
    }

private:
    inline static void check(uint16_t cache_idx, uint16_t max)
    {
        if (REDEMPTION_UNLIKELY(cache_idx >= max)) {
            LOG(LOG_ERR,
                "PointerCache::insert pointer cache idx overflow (%u)",
                cache_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
    }

    Cache cache;
    std::array<RdpPointer, MAX_POINTER_COUNT> pointers;
};  // struct PointerCache
