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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/cache_index_mapping.hpp"

#include <string_view>


using Cache = CacheIndexMapping<5, unsigned short>;
struct CacheResult : Cache::CacheResult
{
    bool operator == (CacheResult const& other) const
    {
        return other.destination_idx == destination_idx
            && other.is_cached == is_cached;
    }

    bool operator != (CacheResult const& other) const
    {
        return !(*this == other);
    }
};

RED_TEST_DELEGATE_PRINT(CacheResult,
    "{" << _.destination_idx << ", " << (_.is_cached ? "true" : "false") << "}"
);


RED_AUTO_TEST_CASE(TestCacheMappingFull)
{
    Cache cache(5);

    cache.insert(4);
    cache.insert(3);
    cache.insert(0);
    cache.insert(1);
    cache.insert(2);

    RED_CHECK(CacheResult{cache.use(0)} == (CacheResult{{0, false}}));
    RED_CHECK(CacheResult{cache.use(0)} == (CacheResult{{0, true}}));
    RED_CHECK(CacheResult{cache.use(3)} == (CacheResult{{1, false}}));
    RED_CHECK(CacheResult{cache.use(1)} == (CacheResult{{2, false}}));
    RED_CHECK(CacheResult{cache.use(3)} == (CacheResult{{1, true}}));
    RED_CHECK(CacheResult{cache.use(3)} == (CacheResult{{1, true}}));
    RED_CHECK(CacheResult{cache.use(4)} == (CacheResult{{3, false}}));
    RED_CHECK(CacheResult{cache.use(2)} == (CacheResult{{4, false}}));
    RED_CHECK(CacheResult{cache.use(4)} == (CacheResult{{3, true}}));
    RED_CHECK(CacheResult{cache.use(2)} == (CacheResult{{4, true}}));

    cache.insert(1);
    RED_CHECK(CacheResult{cache.use(3)} == (CacheResult{{1, true}}));
    RED_CHECK(CacheResult{cache.use(1)} == (CacheResult{{2, false}}));
}

RED_AUTO_TEST_CASE(TestCacheMapping)
{
    Cache cache(3);

    cache.insert(0);
    cache.insert(1);
    cache.insert(2);
    cache.insert(3);
    cache.insert(4);

    RED_CHECK(CacheResult{cache.use(0)} == (CacheResult{{0, false}}));
    RED_CHECK(CacheResult{cache.use(0)} == (CacheResult{{0, true}}));
    RED_CHECK(CacheResult{cache.use(1)} == (CacheResult{{1, false}}));
    RED_CHECK(CacheResult{cache.use(1)} == (CacheResult{{1, true}}));
    RED_CHECK(CacheResult{cache.use(2)} == (CacheResult{{2, false}}));
    RED_CHECK(CacheResult{cache.use(2)} == (CacheResult{{2, true}}));

    RED_CHECK(CacheResult{cache.use(3)} == (CacheResult{{0, false}}));
    RED_CHECK(CacheResult{cache.use(3)} == (CacheResult{{0, true}}));
    RED_CHECK(CacheResult{cache.use(1)} == (CacheResult{{1, true}}));
    RED_CHECK(CacheResult{cache.use(0)} == (CacheResult{{2, false}}));
    RED_CHECK(CacheResult{cache.use(0)} == (CacheResult{{2, true}}));
    RED_CHECK(CacheResult{cache.use(4)} == (CacheResult{{0, false}}));
    RED_CHECK(CacheResult{cache.use(4)} == (CacheResult{{0, true}}));
    RED_CHECK(CacheResult{cache.use(2)} == (CacheResult{{1, false}}));
    RED_CHECK(CacheResult{cache.use(2)} == (CacheResult{{1, true}}));

    cache.insert(1);
    RED_CHECK(CacheResult{cache.use(1)} == (CacheResult{{2, false}}));
}
