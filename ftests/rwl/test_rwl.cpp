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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
 */

#include "rwl/rwl_parser.hpp"
#include "rwl/rwl_value.hpp"
#include "basic_cstring.hpp"

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRwl
#include <boost/test/auto_unit_test.hpp>

#include <sstream>
#include <iostream>

const_cstring tb(unsigned tab = 0)
{
#define SPACE_S "                                                                      "
    return const_cstring(SPACE_S, SPACE_S + tab*2);
#undef SPACE_S

}

void display_rwl(std::ostream& os, const rwl_value& value)
{
    switch (value.type) {
        case 'c':
            os << "#" << std::hex << value.value.color << std::dec;
            break;
        case 'i':
            os << value.value.i;
            break;
        case 's':
            os << "\"" << const_cstring(value.value.s.first, value.value.s.last) << "\"";
            break;
        case 't':
            os << const_cstring(value.value.s.first, value.value.s.last);
            break;
        case 'f':
            os << const_cstring(value.value.func.first, value.value.func.last) << "( ";
            if (value.value.func.size) {
                display_rwl(os, *value.value.func.params[0]);
            }
            for (unsigned i = 1; i < value.value.func.size; ++i) {
                os << ", ";
                display_rwl(os, *value.value.func.params[i]);
            }
            os << " )";
            break;
        case 'o':
            display_rwl(os, *value.value.operation.l);
            os << " " << char(value.value.operation.op) << " ";
            display_rwl(os, *value.value.operation.r);
            break;
        case 'g':
            os << "( ";
            display_rwl(os, *value.value.g);
            os << " )";
            break;
        case 'l':
            os << const_cstring(value.value.linked.first, value.value.linked.last);
            for (unsigned i = 0; i < value.value.linked.size; ++i) {
                const char * * ps = value.value.linked.pair_strings;
                os << "." << const_cstring(ps[i*2], ps[i*2+1]);
            }
            break;
        default:
            os << "???";
            break;
    }
}

void display_rwl(std::ostream& os, const rwl_property& property, unsigned tab = 0)
{
    if (property.properties.empty() ? 1 : (property.value.type != 0)) {
        os << tb(tab) << property.name << ": ";
        display_rwl(os, *property.root_value);
        os << ";\n";
    }
    else {
        os << tb(tab) << property.name << ": {\n";
        typedef std::vector<rwl_property*>::const_iterator prop_iterator;
        for (prop_iterator first = property.properties.begin(), last = property.properties.end(); first != last ; ++first) {
            display_rwl(os, **first, tab+1);
        }
        os << tb(tab) << "}\n";
    }
}

void display_rwl(std::ostream& os, const rwl_target& target, unsigned tab = 0)
{
    os << tb(tab) << target.name << " {\n";
    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
        display_rwl(os, **first, tab+1);
    }

    typedef std::vector<rwl_target*>::const_iterator target_iterator;
    for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
        display_rwl(os, **first, tab+1);
    }
    os << tb(tab) << "}\n";
}

BOOST_AUTO_TEST_CASE(TestRwl)
{
    rwl_target screen;
    screen.name.assign("Screen", "Screen" + sizeof("Screen") - 1);
    rwl_parser parser(screen);

    const char * str = "Rect {"
        " bgcolor: #122 ;"
        " color: rgb( #239, 'l' ) ;"
        " color: rgb( #239 ) ;"
        " color: rgb( (#239) , \"l\", 234 ) ;"
        " x: label.w ;"
        " x: label.w + 2 ;"
        " border.top: 2 ;"
        " border.top.color: #233 ;"
        " Rect { z: 22+2 }"
        " k: 21+3 ;"
        " Rect { Rect { x: 20 ;} }"
        " Rect { Rect { x: 20 } ; }"
        " text: \"plop\" ;"
        " border: { left: 2 ; right: 2 ; top: 2}"
        " border: { left: 2 ; color: { top:3 ; bottom: 1; } ; right: 2 ; top: 2}"
        " x: (screen.w - this.w) / 2 ;"
        " y: screen.w - this.w / 2 ;"
        " n: 1 * 2 + 3 ;"
        " n: 1 + 2 * 3 ;"
        " z: x ;"
        " zz: y;"
    " }"
    " Rect { x: 202 }"
    " Rect { x: r(r(g(8),2),1) }"
    " x:1"
    ;

    while (parser.valid() && *str) {
        str = parser.next_event(str);
    }

    BOOST_CHECK(!*str);
    BOOST_CHECK(parser.stop());

    std::ostringstream os;
    display_rwl(os, screen);
    BOOST_CHECK_EQUAL(os.str(),
"Screen {\n"
"  x: 1;\n"
"  Rect {\n"
"    bgcolor: #112222;\n"
"    color: rgb( #223399, \"l\" );\n"
"    color: rgb( #223399 );\n"
"    color: rgb( ( #223399 ), \"l\", 234 );\n"
"    x: label.w;\n"
"    x: label.w + 2;\n"
"    border: {\n"
"      top: 2;\n"
"    }\n"
"    border: {\n"
"      top: {\n"
"        color: #223333;\n"
"      }\n"
"    }\n"
"    k: 21 + 3;\n"
"    text: \"plop\";\n"
"    border: {\n"
"      left: 2;\n"
"      right: 2;\n"
"      top: 2;\n"
"    }\n"
"    border: {\n"
"      left: 2;\n"
"      color: {\n"
"        top: 3;\n"
"        bottom: 1;\n"
"      }\n"
"      right: 2;\n"
"      top: 2;\n"
"    }\n"
"    x: ( screen.w - this.w ) / 2;\n"
"    y: screen.w - this.w / 2;\n"
"    n: 1 * 2 + 3;\n"
"    n: 1 + 2 * 3;\n"
"    z: x;\n"
"    zz: y;\n"
"    Rect {\n"
"      z: 22 + 2;\n"
"    }\n"
"    Rect {\n"
"      Rect {\n"
"        x: 20;\n"
"      }\n"
"    }\n"
"    Rect {\n"
"      Rect {\n"
"        x: 20;\n"
"      }\n"
"    }\n"
"  }\n"
"  Rect {\n"
"    x: 202;\n"
"  }\n"
"  Rect {\n"
"    x: r( r( g( 8 ), 2 ), 1 );\n"
"  }\n"
"}\n"
);
}
