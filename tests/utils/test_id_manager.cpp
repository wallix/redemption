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
#include "system/redemption_unit_tests.hpp"

#include "utils/id_manager.hpp"

RED_AUTO_TEST_CASE(TestIDManagerGeneral)
{
    constexpr unsigned int INVALID_ID = 0xFFFFFFFF;

    IDManager<
            std::remove_const<decltype(INVALID_ID)>::type,
            INVALID_ID,
            0,
            INVALID_ID,
            INVALID_ID
        > id_manager;


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020);

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(40020), false);


    uint32_t const dest_only_id = id_manager.reg_dest_only_id();

    RED_CHECK_EQUAL(id_manager.get_src_id(dest_only_id), INVALID_ID);

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(dest_only_id), true);


    uint32_t const src_id  = dest_only_id;
    uint32_t const dest_id = id_manager.get_dest_id_ex(src_id);

    RED_CHECK(src_id != dest_id);

    RED_CHECK_EQUAL(id_manager.get_dest_id(src_id), dest_id);
    RED_CHECK_EQUAL(id_manager.get_src_id(dest_id), src_id);

    RED_CHECK_EQUAL(id_manager.is_dest_only_id(dest_id), false);
}

RED_AUTO_TEST_CASE(TestIDManagerUnregisterDestinationID)
{
    constexpr unsigned int INVALID_ID = 0xFFFFFFFF;

    IDManager<
            std::remove_const<decltype(INVALID_ID)>::type,
            INVALID_ID,
            0,
            INVALID_ID,
            INVALID_ID
        > id_manager;


    uint32_t const dest_only_id = id_manager.reg_dest_only_id();

    RED_CHECK_EQUAL(id_manager.get_src_id(dest_only_id), INVALID_ID);

    id_manager.unreg_dest_only_id(dest_only_id);


    try {
        id_manager.is_dest_only_id(dest_only_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}

    try {
        id_manager.get_src_id(dest_only_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}

    try {
        id_manager.unreg_dest_only_id(dest_only_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}
}

RED_AUTO_TEST_CASE(TestIDManagerUnregisterMappedSourceID0)
{
    constexpr unsigned int INVALID_ID = 0xFFFFFFFF;

    IDManager<
            std::remove_const<decltype(INVALID_ID)>::type,
            INVALID_ID,
            0,
            INVALID_ID,
            INVALID_ID
        > id_manager;


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020);

    id_manager.unreg_src_id(40020);


    RED_CHECK_EQUAL(id_manager.get_dest_id_ex(40020), 40020);

    RED_CHECK_EQUAL(id_manager.get_dest_id(40020), 40020);
    RED_CHECK_EQUAL(id_manager.get_src_id(40020), 40020);

    id_manager.unreg_src_id(40020);


    try {
        id_manager.get_dest_id(40020);

        RED_CHECK(false);
    }
    catch (Error & e) {}

    try {
        id_manager.unreg_src_id(40020);

        RED_CHECK(false);
    }
    catch (Error & e) {}
}

RED_AUTO_TEST_CASE(TestIDManagerUnregisterMappedSourceID1)
{
    constexpr unsigned int INVALID_ID = 0xFFFFFFFF;

    IDManager<
            std::remove_const<decltype(INVALID_ID)>::type,
            INVALID_ID,
            0,
            INVALID_ID,
            INVALID_ID
        > id_manager;


    uint32_t const dest_only_id = id_manager.reg_dest_only_id();

    RED_CHECK_EQUAL(id_manager.get_src_id(dest_only_id), INVALID_ID);


    uint32_t const src_id  = dest_only_id;
    uint32_t const dest_id = id_manager.get_dest_id_ex(src_id);

    RED_CHECK(src_id != dest_id);

    RED_CHECK_EQUAL(id_manager.get_dest_id(src_id), dest_id);
    RED_CHECK_EQUAL(id_manager.get_src_id(dest_id), src_id);

    id_manager.unreg_src_id(src_id);


    {
        uint32_t const src_id_2  = dest_only_id;
        uint32_t const dest_id_2 = id_manager.get_dest_id_ex(src_id_2);

        RED_CHECK(src_id_2 != dest_id_2);

        RED_CHECK_EQUAL(id_manager.get_dest_id(src_id_2), dest_id_2);
        RED_CHECK_EQUAL(id_manager.get_src_id(dest_id_2), src_id_2);

        id_manager.unreg_src_id(src_id_2);
    }


    RED_CHECK_EQUAL(id_manager.get_src_id(dest_only_id), INVALID_ID);


    try {
        id_manager.get_dest_id(src_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}

    try {
        id_manager.unreg_src_id(src_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}


    try {
        id_manager.is_dest_only_id(dest_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}

    try {
        id_manager.get_src_id(dest_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}

    try {
        id_manager.unreg_dest_only_id(dest_id);

        RED_CHECK(false);
    }
    catch (Error & e) {}
}
