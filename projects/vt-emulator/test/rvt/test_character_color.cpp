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

#include "rvt/character_color.hpp"

#include <iomanip>

namespace rvt {
    inline std::ostream & operator <<(std::ostream & out, rvt::Color const & color)
    {
        return out
            << "{0x" << std::hex << int(color.red())
            << " 0x" << std::hex << int(color.green())
            << " 0x" << std::hex << int(color.blue())
            << '}'
        ;
    }
}

BOOST_AUTO_TEST_CASE(TestCharacterColor)
{
    const rvt::Color color_table[] = {
        rvt::Color(0x00, 0x00, 0x00), // Dfore
        rvt::Color(0xFF, 0xFF, 0xFF), // Dback
        rvt::Color(0x00, 0x00, 0x00), // Black
        rvt::Color(0xB2, 0x18, 0x18), // Red
        rvt::Color(0x18, 0xB2, 0x18), // Green
        rvt::Color(0xB2, 0x68, 0x18), // Yellow
        rvt::Color(0x18, 0x18, 0xB2), // Blue
        rvt::Color(0xB2, 0x18, 0xB2), // Magenta
        rvt::Color(0x18, 0xB2, 0xB2), // Cyan
        rvt::Color(0xB2, 0xB2, 0xB2), // White
        // intensive versions
        rvt::Color(0x00, 0x00, 0x00),
        rvt::Color(0xFF, 0xFF, 0xFF),
        rvt::Color(0x68, 0x68, 0x68),
        rvt::Color(0xFF, 0x54, 0x54),
        rvt::Color(0x54, 0xFF, 0x54),
        rvt::Color(0xFF, 0xFF, 0x54),
        rvt::Color(0x54, 0x54, 0xFF),
        rvt::Color(0xFF, 0x54, 0xFF),
        rvt::Color(0x54, 0xFF, 0xFF),
        rvt::Color(0xFF, 0xFF, 0xFF),
    };
    static_assert(rvt::TABLE_COLORS == sizeof(color_table)/sizeof(color_table[0]), "");

    rvt::Color black = {0x00, 0x00, 0x00};
    rvt::Color white = {0xFF, 0xFF, 0xFF};
    rvt::Color red   = {0xB2, 0x18, 0x18};
    rvt::Color green = {0x18, 0xB2, 0x18};

    BOOST_CHECK_EQUAL(black, black);
    BOOST_CHECK_EQUAL(red, red);
    BOOST_CHECK_NE(black, white);
    BOOST_CHECK_NE(black, red);
    BOOST_CHECK_NE(red, green);

    rvt::CharacterColor color;
    BOOST_CHECK_EQUAL(color.isValid(), false);

    color = rvt::CharacterColor(rvt::ColorSpace::Default, 0);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[0]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[0+10]);

    color = rvt::CharacterColor(rvt::ColorSpace::Default, 1);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[1]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[1+10]);


    color = rvt::CharacterColor(rvt::ColorSpace::System, 0);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+0]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+0+10]);

    color = rvt::CharacterColor(rvt::ColorSpace::System, 1);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+1]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+1+10]);

    color = rvt::CharacterColor(rvt::ColorSpace::System, 7);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+7]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+7+10]);


    color = rvt::CharacterColor(rvt::ColorSpace::Index256, 3);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+3]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+3]);

    color = rvt::CharacterColor(rvt::ColorSpace::Index256, 15);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+7+10]);
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), color_table[2+7+10]);

    color = rvt::CharacterColor(rvt::ColorSpace::Index256, 200);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), rvt::Color(0xff, 0, 0xd7));
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), rvt::Color(0xff, 0, 0xd7));

    color = rvt::CharacterColor(rvt::ColorSpace::Index256, 236);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), rvt::Color(0x30, 0x30, 0x30));
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), rvt::Color(0x30, 0x30, 0x30));


    color = rvt::CharacterColor(rvt::ColorSpace::RGB, 0x123456);
    BOOST_CHECK_EQUAL(color.isValid(), true);
    BOOST_CHECK_EQUAL(color.color(color_table), rvt::Color(0x12, 0x34, 0x56));
    color.setIntensive();
    BOOST_CHECK_EQUAL(color.color(color_table), rvt::Color(0x12, 0x34, 0x56));
}
