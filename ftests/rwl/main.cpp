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

#include <iostream>

const_cstring tb(unsigned tab = 0)
{
#define SPACE_S "                                                                      "
    return const_cstring(SPACE_S, SPACE_S + tab*2);
#undef SPACE_S

}

void display_value(const rwl_value& value, unsigned tab = 0)
{
    switch (value.type) {
        case 'c':
            std::cout << tb(tab) << "color " << value.value.color << "\n";
            break;
        case 'i':
            std::cout << tb(tab) << "integer " << value.value.i << "\n";
            break;
        case 's':
            std::cout << tb(tab) << "string " << const_cstring(value.value.s.first, value.value.s.last) << "\n";
            break;
        case 't':
            std::cout << tb(tab) << "identity " << const_cstring(value.value.s.first, value.value.s.last) << "\n";
            break;
        case 'f':
            std::cout << tb(tab) << "function " << const_cstring(value.value.func.first, value.value.func.last) << "\n";
            for (unsigned i = 0; i < value.value.func.size; ++i) {
                std::cout << tb(tab) << i << "\n";
                display_value(*value.value.func.params[i], tab+1);
            }
            break;
        case 'o':
            std::cout << tb(tab) << "operation\n";
            display_value(*value.value.operation.l, tab+1);
            std::cout << tb(tab+1) << char(value.value.operation.op) << "\n";
            display_value(*value.value.operation.r, tab+1);
            break;
        case 'g':
            std::cout << tb(tab) << "(\n";
            display_value(*value.value.g, tab+1);
            std::cout << tb(tab) << ")\n";
            break;
        case 'l':
            std::cout << tb(tab) << "identity " << const_cstring(value.value.linked.first, value.value.linked.last);
            for (unsigned i = 0; i < value.value.linked.size; ++i) {
                const char * * ps = value.value.linked.pair_strings;
                std::cout << " -> " << const_cstring(ps[i*2], ps[i*2+1]);
            }
            std::cout << "\n";
            break;
        default:
            std::cout << tb(tab) << "none\n";
            break;
    }
}

void display_property(const rwl_property& property, unsigned tab = 0)
{
    std::cout << tb(tab) << "Property " << property.name << ":\n";

    if (property.properties.empty() ? 1 : (property.value.type != 0)) {
        display_value(*property.root_value, tab+1);
    }

    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = property.properties.begin(), last = property.properties.end(); first != last ; ++first) {
        display_property(**first, tab+1);
    }
}

void display_target(const rwl_target& target, unsigned tab = 0)
{
    std::cout << tb(tab) << "Target " << target.name << ":\n";
    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
        display_property(**first, tab+1);
    }

    typedef std::vector<rwl_target*>::const_iterator target_iterator;
    for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
        display_target(**first, tab+1);
    }
}



void rwl_to_string(const rwl_value& value)
{
    switch (value.type) {
        case 'c':
            std::cout << "#" << std::hex << value.value.color << std::dec;
            break;
        case 'i':
            std::cout << value.value.i;
            break;
        case 's':
            std::cout << "\"" << const_cstring(value.value.s.first, value.value.s.last) << "\"";
            break;
        case 't':
            std::cout << const_cstring(value.value.s.first, value.value.s.last);
            break;
        case 'f':
            std::cout << const_cstring(value.value.func.first, value.value.func.last) << "( ";
            if (value.value.func.size) {
                rwl_to_string(*value.value.func.params[0]);
            }
            for (unsigned i = 1; i < value.value.func.size; ++i) {
                std::cout << ", ";
                rwl_to_string(*value.value.func.params[i]);
            }
            std::cout << " )";
            break;
        case 'o':
            rwl_to_string(*value.value.operation.l);
            std::cout << " " << char(value.value.operation.op) << " ";
            rwl_to_string(*value.value.operation.r);
            break;
        case 'g':
            std::cout << "( ";
            rwl_to_string(*value.value.g);
            std::cout << " )";
            break;
        case 'l':
            std::cout << const_cstring(value.value.linked.first, value.value.linked.last);
            for (unsigned i = 0; i < value.value.linked.size; ++i) {
                const char * * ps = value.value.linked.pair_strings;
                std::cout << "." << const_cstring(ps[i*2], ps[i*2+1]);
            }
            break;
        default:
            std::cout << "???";
            break;
    }
}

void rwl_to_string(const rwl_property& property, unsigned tab = 0)
{
    if (property.properties.empty() ? 1 : (property.value.type != 0)) {
        std::cout << tb(tab) << property.name << ": ";
        rwl_to_string(*property.root_value);
        std::cout << ";\n";
    }
    else {
        std::cout << tb(tab) << property.name << ": {\n";
        typedef std::vector<rwl_property*>::const_iterator prop_iterator;
        for (prop_iterator first = property.properties.begin(), last = property.properties.end(); first != last ; ++first) {
            rwl_to_string(**first, tab+1);
        }
        std::cout << tb(tab) << "}\n";
}
}

void rwl_to_string(const rwl_target& target, unsigned tab = 0)
{
    std::cout << tb(tab) << target.name << " {\n";
    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
        rwl_to_string(**first, tab+1);
    }

    typedef std::vector<rwl_target*>::const_iterator target_iterator;
    for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
        rwl_to_string(**first, tab+1);
    }
    std::cout << tb(tab) << "}\n";
}


int main()
{
    std::ios::sync_with_stdio(false);

    rwl_target screen;
    screen.name.assign("Screen", "Screen" + sizeof("Screen"));
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

//     const char * states[] = {
//         "target",
//         "confirmed_target",
//         "closing_target",
//         "property",
//         "defined_property",
//         "defined_link",
//         "value_block",
//         "value",
//         "confirmed_string",
//         "property_or_function",
//         "property_separator",
//         "link_property",
//         "property_name",
//         "expression",
//     };

    while (parser.valid() && *str) {
        str = parser.next_event(str);
    }

    display_target(screen);
    std::cout << "\n";
    rwl_to_string(screen);

    if (*str) {
        std::cout << ("parsing error\n");
    }
    else if (!parser.stop()) {
        std::cout << ("invalid state\n");
    }
}
