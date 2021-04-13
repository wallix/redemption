/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "utils/sugar/numerics/safe_conversions.hpp"

#include <array>
#include <algorithm>

#include <type_traits>
#include <cassert>


template<std::size_t Max, class IndexType>
struct CacheIndexMapping
{
    static_assert(std::is_unsigned_v<IndexType>);

    using index_type = IndexType;

    static const index_type original_max_entries {Max};

    explicit CacheIndexMapping() = default;

    explicit CacheIndexMapping(IndexType max_entries)
    : max_entries(max_entries)
    {
        assert(max_entries <= original_max_entries);
        dst_to_src.fill(not_fount);
        src_to_dst.fill(not_fount);
    }

    void insert(IndexType source_idx)
    {
        auto dest_idx = src_to_dst[source_idx];
        if (dest_idx != not_fount) {
            src_to_dst[source_idx] = not_fount;
            dst_to_src[dest_idx] = not_fount;
            stamps[dest_idx] = 0;
        }
    }

    struct [[nodiscard]] CacheResult
    {
        IndexType destination_idx;
        bool is_cached;
    };

    CacheResult use(IndexType source_idx) noexcept
    {
        ++stamp;

        if (src_to_dst[source_idx] != not_fount) {
            auto dest_idx = src_to_dst[source_idx];
            stamps[dest_idx] = stamp;
            return CacheResult{dest_idx, true};
        }

        IndexType dest_idx;

        if (entries < max_entries) {
            dest_idx = entries;
            ++entries;
        }
        else {
            assert(max_entries > 0);
            // look for oldest
            auto first = stamps.begin();
            auto it = std::min_element(first, first + entries);
            dest_idx = checked_int(std::distance(first, it));

            auto old_source_id = dst_to_src[dest_idx];
            if (old_source_id != not_fount) {
                src_to_dst[old_source_id] = not_fount;
            }
        }

        src_to_dst[source_idx] = dest_idx;
        dst_to_src[dest_idx] = source_idx;
        stamps[dest_idx] = stamp;
        return CacheResult{dest_idx, false};
    }

private:
    static constexpr IndexType not_fount = IndexType(~IndexType{});

    std::array<IndexType, Max> src_to_dst;
    std::array<IndexType, Max> dst_to_src;
    std::array<unsigned, Max> stamps {};
    IndexType max_entries {};
    IndexType entries {};
    unsigned stamp {};
};
