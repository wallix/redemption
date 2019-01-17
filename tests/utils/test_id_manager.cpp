/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define RED_TEST_MODULE TestIDManager
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/id_manager.hpp"

RED_AUTO_TEST_CASE(TestIDManagerGeneral)
{
    constexpr unsigned int             NEXT_USABLE_ID   = 0x8FFFFFFF;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_FIRST = 0xFFFFFFFE;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_LAST  = 0x00000000;

    IDManager<
            std::remove_const<decltype(NEXT_USABLE_ID)>::type,
            NEXT_USABLE_ID,
            INVARIABLE_FIRST,
            INVARIABLE_LAST
        > id_manager;


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020u);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020u);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020u);

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(40020), false);


    uint32_t const dest_only_id = id_manager.reg_dest_only_id();

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(dest_only_id), true);


    uint32_t const src_id  = dest_only_id;
    uint32_t const dest_id = id_manager.get_dest_id_ex(src_id);

    RED_CHECK(src_id != dest_id);

    RED_CHECK_EQUAL(id_manager.get_dest_id(src_id), dest_id);
    RED_CHECK_EQUAL(id_manager.get_src_id(dest_id), src_id);

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(dest_id), false);
}

RED_AUTO_TEST_CASE(TestIDManagerGeneral2)
{
    constexpr unsigned int             NEXT_USABLE_ID   = 0x8FFFFFFF;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_FIRST = 0xFFFFFFFE;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_LAST  = 0x00000000;

    IDManager<
            std::remove_const<decltype(NEXT_USABLE_ID)>::type,
            NEXT_USABLE_ID,
            INVARIABLE_FIRST,
            INVARIABLE_LAST
        > id_manager;


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020u);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020u);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020u);

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(40020), false);


    decltype(NEXT_USABLE_ID) src_id = id_manager.get_src_id(40020);

    RED_CHECK_EQUAL(src_id, 40020u);

    id_manager.unreg_src_id(src_id);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.get_dest_id(40020), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.unreg_src_id(40020), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.is_dest_only_id(40020), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.get_src_id(40020), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.unreg_dest_only_id(40020), ERR_UNEXPECTED);
}

RED_AUTO_TEST_CASE(TestIDManagerUnregisterDestinationID)
{
    constexpr unsigned int             NEXT_USABLE_ID   = 0x8FFFFFFF;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_FIRST = 0xFFFFFFFE;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_LAST  = 0x00000000;

    IDManager<
            std::remove_const<decltype(NEXT_USABLE_ID)>::type,
            NEXT_USABLE_ID,
            INVARIABLE_FIRST,
            INVARIABLE_LAST
        > id_manager;


    decltype(NEXT_USABLE_ID) dest_only_id = id_manager.reg_dest_only_id();

    id_manager.unreg_dest_only_id(dest_only_id);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.is_dest_only_id(dest_only_id), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.get_src_id(dest_only_id), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.unreg_dest_only_id(dest_only_id), ERR_UNEXPECTED);
}

RED_AUTO_TEST_CASE(TestIDManagerUnregisterMappedSourceID0)
{
    constexpr unsigned int             NEXT_USABLE_ID   = 0x8FFFFFFF;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_FIRST = 0xFFFFFFFE;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_LAST  = 0x00000000;

    IDManager<
            std::remove_const<decltype(NEXT_USABLE_ID)>::type,
            NEXT_USABLE_ID,
            INVARIABLE_FIRST,
            INVARIABLE_LAST
        > id_manager;


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020u);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020u);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020u);

    id_manager.unreg_src_id(40020);


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020u);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020u);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020u);

    id_manager.unreg_src_id(40020);


    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.get_dest_id(40020), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.unreg_src_id(40020), ERR_UNEXPECTED);
}

RED_AUTO_TEST_CASE(TestIDManagerUnregisterMappedSourceID1)
{
    constexpr unsigned int             NEXT_USABLE_ID   = 0x8FFFFFFF;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_FIRST = 0xFFFFFFFE;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_LAST  = 0x00000000;

    IDManager<
            std::remove_const<decltype(NEXT_USABLE_ID)>::type,
            NEXT_USABLE_ID,
            INVARIABLE_FIRST,
            INVARIABLE_LAST
        > id_manager;


    decltype(NEXT_USABLE_ID) dest_only_id = id_manager.reg_dest_only_id();

    RED_CHECK(id_manager.is_dest_only_id(dest_only_id));


    decltype(NEXT_USABLE_ID) src_id  = dest_only_id;
    decltype(NEXT_USABLE_ID) dest_id = id_manager.get_dest_id_ex(src_id);

    RED_CHECK(src_id != dest_id);

    RED_CHECK_EQUAL(id_manager.get_dest_id(src_id), dest_id);
    RED_CHECK_EQUAL(id_manager.get_src_id(dest_id), src_id);

    id_manager.unreg_src_id(src_id);


    {
        decltype(NEXT_USABLE_ID) src_id_2  = dest_only_id;
        decltype(NEXT_USABLE_ID) dest_id_2 = id_manager.get_dest_id_ex(src_id_2);

        RED_CHECK(src_id_2 != dest_id_2);

        RED_CHECK_EQUAL(id_manager.get_dest_id(src_id_2), dest_id_2);
        RED_CHECK_EQUAL(id_manager.get_src_id(dest_id_2), src_id_2);

        id_manager.unreg_src_id(src_id_2);
    }


    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.get_dest_id(src_id), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.unreg_src_id(src_id), ERR_UNEXPECTED);


    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.is_dest_only_id(dest_id), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.get_src_id(dest_id), ERR_UNEXPECTED);

    RED_CHECK_EXCEPTION_ERROR_ID(id_manager.unreg_dest_only_id(dest_id), ERR_UNEXPECTED);
}

RED_AUTO_TEST_CASE(TestIDManagerInvariable)
{
    constexpr unsigned int             NEXT_USABLE_ID   = 0x8FFFFFFF;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_FIRST = 0xFFFFFFFE;
    constexpr decltype(NEXT_USABLE_ID) INVARIABLE_LAST  = 0x00000000;

    IDManager<
            std::remove_const<decltype(NEXT_USABLE_ID)>::type,
            NEXT_USABLE_ID,
            INVARIABLE_FIRST,
            INVARIABLE_LAST
        > id_manager;

    for (std::remove_const<decltype(NEXT_USABLE_ID)>::type i = INVARIABLE_FIRST;
         i <= INVARIABLE_LAST; ++i) {
        RED_CHECK_PREDICATE(id_manager.get_dest_id_ex, (i));
    }

    auto f = [](int i, int x) { return i+x; };
    RED_CHECK_PREDICATE(f, (2)(3));
}
