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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#include "rwl/rwl_parser.hpp"
#include "rwl/rwl_value.hpp"
#include "basic_cstring.hpp"
#include "delete.hpp"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

struct widget_values
{
    widget_values * parent;
    struct border
    {
        unsigned color;
        uint size;
    };
    border borders[4];
    unsigned color;
    unsigned bgcolor;
    int x;
    int y;
    uint w;
    uint h;
    std::string id;
    std::vector<widget_values*> children;

    widget_values(widget_values * parent = 0)
    : parent(parent)
    , color(0)
    , bgcolor(0xEEEEEE)
    , x(0)
    , y(0)
    , w(0)
    , h(0)
    {
        borders[0].color = 0;
        borders[0].size = 0;
        borders[1].color = 0;
        borders[1].size = 0;
        borders[2].color = 0;
        borders[2].size = 0;
        borders[3].color = 0;
        borders[3].size = 0;
    }

    ~widget_values()
    {
        delete_all(this->children.begin(), this->children.end());
    }

    widget_values * new_child(const const_cstring& /*name*/)
    {
        widget_values * ret = new widget_values(this);
        this->children.push_back(ret);
        return ret;
    }
};

namespace rwl {

    template<typename T, T widget_values::*WidgetMember, T rwl_value::Val::*RwlMember>
    struct set_value
    {
        void operator()(widget_values& w, const rwl_value& val) const
        {
            (w.*WidgetMember) = val.*RwlMember;
        }
    };

    template<
        typename T, T widget_values::*WidgetMember,
        typename U, U rwl_value::Val::*RwlMember,
        typename Adapter
    >
    struct set_value_adapter
    {
        void operator()(widget_values& w, const rwl_value& val) const
        {
            (w.*WidgetMember) = Adapter()(val.*RwlMember);
        }
    };

    namespace adapter {
        struct i2u
        {
            uint operator()(int n) const
            {
                return (n < 0) ? 0 : n;
            }
        };

        struct vs2s
        {
            std::string operator()(const rwl_value::Val::S& s) const
            {
                return std::string(s.first, s.last);
            }
        };
    }

    typedef set_value<unsigned, &widget_values::color, &rwl_value::Val::color> set_color;
    typedef set_value<unsigned, &widget_values::bgcolor, &rwl_value::Val::color> set_bgcolor;
    typedef set_value<int, &widget_values::x, &rwl_value::Val::i> set_x;
    typedef set_value<int, &widget_values::y, &rwl_value::Val::i> set_y;
    typedef set_value_adapter<uint, &widget_values::w, int, &rwl_value::Val::i, adapter::i2u> set_w;
    typedef set_value_adapter<uint, &widget_values::h, int, &rwl_value::Val::i, adapter::i2u> set_h;
    typedef set_value_adapter<std::string, &widget_values::id, rwl_value::Val::S, &rwl_value::Val::s, adapter::vs2s> set_id;

}



struct set_property
{
    rwl::set_color set_color;
    int type;

    void assign(widget_values& w, const rwl_property& property)
    {
        rwl_value * val = property.root_value;
        if (property.properties.empty() ? 1 : (property.value.type != 0)) {
            if (this->check(property, "color", 'c')) {
                w.color = val->value.color;
            }
            else if (this->check(property, "bgcolor", 'c')) {
                w.bgcolor = val->value.color;
            }
            else if (this->check(property, "x", 'i')) {
                w.x = val->value.i;
            }
            else if (this->check(property, "y", 'i')) {
                w.y = val->value.i;
            }
            else if (this->check(property, "w", 'i')) {
                if (val->value.i < 0) {
                    throw std::runtime_error("number is negatif");
                }
                w.w = val->value.i;
            }
            else if (this->check(property, "h", 'i')) {
                if (val->value.i < 0) {
                    throw std::runtime_error("number is negatif");
                }
                w.h = val->value.i;
            }
            else if (this->check(property, "id", 's')) {
                w.id.assign(val->value.s.first, val->value.s.last);
            }
            else {
                std::string msg = "invalid name ";
                msg.append(property.name.begin(), property.name.size());
                throw std::runtime_error(msg);
            }
            return ;
        }
        else if (property.name == "border") {
            if (property.properties.empty()) {
                throw std::runtime_error("invalid name border.");
            }
            typedef std::vector<rwl_property*>::const_iterator prop_iterator;
            for (prop_iterator first = property.properties.begin(), last = property.properties.end(); first != last ; ++first) {
                if ((**first).properties.empty() ? 1 : ((**first).value.type != 0)) {
                    val = (*first)->root_value;
                    if (this->check(**first, "color", 'c')) {
                        w.borders[0].color = val->value.color;
                        w.borders[1].color = val->value.color;
                        w.borders[2].color = val->value.color;
                        w.borders[3].color = val->value.color;
                    }
                    else if (this->check(**first, "size", 'i')) {
                        w.borders[0].size = val->value.i;
                        w.borders[1].size = val->value.i;
                        w.borders[2].size = val->value.i;
                        w.borders[3].size = val->value.i;
                    }
                    else {
                        std::string msg = "invalid name ";
                        msg.append((*first)->name.begin(), (*first)->name.size());
                        throw std::runtime_error(msg);
                    }
                }
                else {
                    uint idx = -1u;
                    if ((*first)->name == "top") {
                        idx = 0;
                    }
                    else if ((*first)->name == "right") {
                        idx = 1;
                    }
                    else if ((*first)->name == "bottom") {
                        idx = 2;
                    }
                    else if ((*first)->name == "left") {
                        idx = 3;
                    }
                    else {
                        std::string msg = "invalid name ";
                        msg.append((*first)->name.begin(), (*first)->name.size());
                        throw std::runtime_error(msg);
                    }

                    for (prop_iterator first2 = (*first)->properties.begin(), last2 = (*first)->properties.end(); first2 != last2 ; ++first2) {
                        if ((**first2).properties.empty() ? 1 : ((**first2).value.type != 0)) {
                            val = (*first2)->root_value;
                            if (this->check(**first2, "color", 'c')) {
                                w.borders[idx].color = val->value.color;
                            }
                            else if (this->check(**first2, "size", 'i')) {
                                w.borders[idx].size = val->value.i;
                            }
                            else {
                                std::string msg = "invalid name border.";
                                msg += "top\0\0\0\0right\0\0bottom\0left\0\0"[idx*6];
                                msg.append((*first2)->name.begin(), (*first2)->name.size());
                                throw std::runtime_error(msg);
                            }
                        }
                        else {
                            std::string msg = "invalid name border.";
                            msg.append((*first2)->name.begin(), (*first2)->name.size());
                            throw std::runtime_error(msg);
                        }
                    }
                }
            }
            return ;
        }
        std::string msg = "invalid name ";
        msg.append(property.name.begin(), property.name.size());
        throw std::runtime_error(msg);
    }

    bool check(const rwl_property& property, const char * name, int type)
    {
        if (property.name == name) {
            if (property.root_value->type == type) {
                return true;
            }
            throw std::runtime_error(name);
        }
        return false;
    }
};

void rwl2widget(widget_values& w, const rwl_target& target)
{
    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
        set_property().assign(w, **first);
    }

    typedef std::vector<rwl_target*>::const_iterator target_iterator;
    for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
        widget_values * wchild = w.new_child(target.name);
        rwl2widget(*wchild, **first);
    }
}


const_cstring tb(unsigned tab = 0)
{
#define SPACE_S "                                                                      "
    return const_cstring(SPACE_S, SPACE_S + tab*2);
#undef SPACE_S
}

void display_widget_values(const widget_values& w, unsigned tab = 0)
{
    std::cout
    << tb(tab) << "border.top.color: " << w.borders[0].color << "\n"
    << tb(tab) << "border.top.size: " << w.borders[0].size << "\n"
    << tb(tab) << "border.right.color: " << w.borders[1].color << "\n"
    << tb(tab) << "border.right.size: " << w.borders[1].size << "\n"
    << tb(tab) << "border.bottom.color: " << w.borders[2].color << "\n"
    << tb(tab) << "border.bottom.size: " << w.borders[2].size << "\n"
    << tb(tab) << "border.left.color: " << w.borders[3].color << "\n"
    << tb(tab) << "border.left.size: " << w.borders[3].size << "\n"
    << tb(tab) << "color: " << w.color << "\n"
    << tb(tab) << "bgcolor: " << w.bgcolor << "\n"
    << tb(tab) << "x: " << w.x << "\n"
    << tb(tab) << "y: " << w.y << "\n"
    << tb(tab) << "w: " << w.w << "\n"
    << tb(tab) << "h: " << w.h << "\n"
    ;
    for (std::vector<widget_values*>::const_iterator first = w.children.begin(), last = w.children.end(); first != last; ++first) {
        display_widget_values(**first, tab+1);
    }
}


int main(int ac, char ** av)
{
    std::ios::sync_with_stdio(false);

    if (ac < 2) {
        std::cerr << av[0] << " rwl_file\n";
        return 2;
    }

    rwl_target screen;
    screen.name.assign("Screen", "Screen" + sizeof("Screen") - 1);
    rwl_parser parser(screen);

    std::ifstream ifs(av[1]);
    if (! ifs.is_open()) {
        return 3;
    }
    std::string str;
    std::getline(ifs, str, '\0'); //read all
    const char * cstr = str.c_str();
    const char * begin_cstr = cstr;

    while (parser.valid() && *cstr) {
        cstr = parser.next_event(cstr);
    }

    if (*cstr) {
        std::cout << "\n\n\nparsing error line "
        << (std::count(begin_cstr, cstr, '\n')+1) << "\n" << cstr;
    }
    else if (!parser.stop()) {
        std::cout << "\n\n\ninvalid state\n";
    }

    widget_values w;
    rwl2widget(w, screen);
    display_widget_values(w);
}