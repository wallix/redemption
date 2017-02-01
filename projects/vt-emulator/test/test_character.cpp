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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRect
#include "system/redemption_unit_tests.hpp"

#include "rvt/character.hpp"

BOOST_AUTO_TEST_CASE(TestCharacter)
{
    rvt::Character ch('e');
    rvt::ExtendedCharTable ext_ch_table;

    BOOST_CHECK_EQUAL(ch.character, 'e');
    BOOST_CHECK_EQUAL(ch.is_extended(), false);
    {
        bool r = ext_ch_table.growChar(ch, 0x0311);
        BOOST_CHECK_EQUAL(r, true);
        BOOST_CHECK_EQUAL(ch.is_extended(), true);
        rvt::uc_t ucs[] {'e', 0x0311};
        BOOST_CHECK_EQUAL_RANGES(ucs, ext_ch_table[ch.character]);
    }
    {
        bool r = ext_ch_table.growChar(ch, 0x0312);
        BOOST_CHECK_EQUAL(r, true);
        BOOST_CHECK_EQUAL(ch.is_extended(), true);
        rvt::uc_t ucs[] {'e', 0x0311, 0x0312};
        BOOST_CHECK_EQUAL_RANGES(ucs, ext_ch_table[ch.character]);
    }
}
