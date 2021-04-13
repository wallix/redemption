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

#include "core/RDP/caches/pointercache.hpp"


RED_AUTO_TEST_CASE(Test_PointerCache)
{
    PointerCache cache{5};

    using Index = gdi::CachePointerIndex;

    auto& cur1 = edit_pointer();
    Index idx1{2};

    auto& cur2 = normal_pointer();
    Index idx2{PredefinedPointer::Normal};

    cache.insert(idx1, cur1);

    RED_CHECK_EXCEPTION_ERROR_ID(
        cache.insert(idx2, cur2),
        ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);

    auto r = cache.use(idx2);
    RED_CHECK(!r.is_cached);
    RED_CHECK(r.destination_idx == 0);

    auto& cur = cache.pointer(idx2);
    RED_CHECK(&cur == &cur2);

    r = cache.use(idx2);
    RED_CHECK(r.is_cached);
    RED_CHECK(r.destination_idx == 0);

    r = cache.use(idx1);
    RED_CHECK(!r.is_cached);
    RED_CHECK(r.destination_idx == 1);
}
