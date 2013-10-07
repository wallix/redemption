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

#include <boost/mpl/vector.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

struct widget_values
{
    widget_values * parent;
    struct border_t
    {
        unsigned color;
        unsigned size;
    };
    border_t borders[4];
    unsigned color;
    unsigned bgcolor;
    int x;
    int y;
    unsigned w;
    unsigned h;
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

namespace rwl
{
    struct rwl_evaluate
    {
        typedef std::pair<int, rwl_value::Val> evaluate_value_t;

        template<typename T>
        const_cstring to_cstring(const T& val)
        { return const_cstring(val.first, val.last); }

        static evaluate_value_t evaluate(const rwl_value& a)
        {
            switch (a.type) {
                case 'f':
                    return evaluate_function(a);
                    break;
                case 'o':
                    if (a.value.operation.l == 0 || a.value.operation.l == 0) {
                        //TODO error
                    }
                    return evaluate_operation(
                        a.value.operation.op,
                        *a.value.operation.l,
                        *a.value.operation.r
                    );
                    break;
                case 'g':
                    if (a.value.g) {
                        return evaluate(*a.value.g);
                    }
                    else {
                        return evaluate_value_t();
                    }
                    break;
                case 't':
                    return evaluate_identifier(a);
                    break;
                case 'l':
                    return evaluate_link_identifier(a);
                    break;
                case 'i':
                    return evaluate_value_t('i', a.value);
                    break;
                case 's':
                    return evaluate_value_t('s', a.value);
                    break;
                case 'c':
                    return evaluate_value_t('c', a.value);
                    break;
                default:
                    break;
            }
        }

        static evaluate_value_t evaluate_link_identifier(const rwl_value& /*val*/)
        {
            //TODO
            return evaluate_value_t();
        }

        static evaluate_value_t evaluate_identifier(const rwl_value& /*val*/)
        {
            //TODO
            return evaluate_value_t();
        }

        static evaluate_value_t evaluate_function(const rwl_value& val)
        {
            const_cstring name(val.value.func.first, val.value.func.last);
            if (name == "rgb") {
                if (val.value.func.size != 3) {
                    //TODO error
                }
                int r = to_i(*val.value.func.params[0]);
                int g = to_i(*val.value.func.params[1]);
                int b = to_i(*val.value.func.params[2]);

                r = std::min(0, std::max(255, r));
                g = std::min(0, std::max(255, g));
                b = std::min(0, std::max(255, b));

                evaluate_value_t val;
                val.first = 'c';
                val.second.color = (r << 16) | (g << 8) | b;
                return val;
            }
        }

        static int to_i(const rwl_value& a)
        {
            if (a.type != 'i') {
                evaluate_value_t v = evaluate(a);
                if (v.first == 'i') {
                    return v.second.i;
                }
                else {
                    //TODO error
                }
            }
            return a.value.i;
        }

        static evaluate_value_t to_val_i(int i)
        {
            evaluate_value_t val;
            val.first = 'i';
            val.second.i = i;
            return val;
        }

        static evaluate_value_t evaluate_operation(int op, const rwl_value& a, const rwl_value& b)
        {
            if (a.type == 'c' || a.type == 's' || b.type == 'c' || b.type == 's') {
                //TODO error
            }
            unsigned n1 = to_i(a);
            unsigned n2 = to_i(b);

            switch (op) {
                case '+': return to_val_i(n1+n2);
                case '-': return to_val_i(n1-n2);
                case '*': return to_val_i(n1*n2);
                case '/': return to_val_i(n1/n2);
                case '%': return to_val_i(n1%n2);
                default: //TODO error
                    return to_val_i(0);
                    break;
            }
        }
    };

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
    << tb(tab) << "id: '" << w.id << "'\n"
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


namespace rwl {

struct value_t
{
    static value_t make_s(const char * first, const char * last)
    {
        value_t ret;
        ret.type = 's';
        ret.val.s.first = first;
        ret.val.s.last = last;
        return ret;
    }

    static value_t make_color(unsigned color)
    {
        value_t ret;
        ret.type = 'c';
        ret.val.c = color;
        return ret;
    }

    static value_t make_i(int i)
    {
        value_t ret;
        ret.type = 'i';
        ret.val.i = i;
        return ret;
    }

    value_t(int type = 0)
    : type(type)
    {}

    int type;

    union {
        struct {
            const char * first;
            const char * last;
        } s;
        unsigned c;
        int i;
    } val;
};

void widget_set_value(widget_values& w, const std::string& property_name, const value_t& val)
{
    if (val.type == 'c') {
        if (property_name == "color") {
            w.color = val.val.c;
        }
        else if (property_name == "bgcolor") {
            w.bgcolor = val.val.c;
        }
        else if (property_name == "border.color") {
            w.borders[0].color = val.val.c;
            w.borders[1].color = val.val.c;
            w.borders[2].color = val.val.c;
            w.borders[3].color = val.val.c;
        }
        else if (property_name == "border.top.color") {
            w.borders[0].color = val.val.c;
        }
        else if (property_name == "border.right.color") {
            w.borders[1].color = val.val.c;
        }
        else if (property_name == "border.bottom.color") {
            w.borders[2].color = val.val.c;
        }
        else if (property_name == "border.left.color") {
            w.borders[3].color = val.val.c;
        }
        else {
            goto not_find;
        }
    }
    else if (val.type == 'i') {
        if (property_name == "x") {
            w.x = val.val.i;
        }
        else if (property_name == "y") {
            w.y = val.val.i;
        }
        else if (property_name == "w") {
            if (val.val.i < 0) {
                //TODO error
            }
            w.w = val.val.i;
        }
        else if (property_name == "h") {
            if (val.val.i < 0) {
                //TODO error
            }
            w.h = val.val.i;
        }
        else if (property_name == "border.size") {
            w.borders[0].size = val.val.c;
            w.borders[1].size = val.val.c;
            w.borders[2].size = val.val.c;
            w.borders[3].size = val.val.c;
        }
        else if (property_name == "border.top.size") {
            w.borders[0].size = val.val.c;
        }
        else if (property_name == "border.right.size") {
            w.borders[1].size = val.val.c;
        }
        else if (property_name == "border.bottom.size") {
            w.borders[2].size = val.val.c;
        }
        else if (property_name == "border.left.size") {
            w.borders[3].size = val.val.c;
        }
        else {
            goto not_find;
        }
    }
    else if (val.type == 's') {
        if (property_name == "id") {
            if (!w.id.empty()) {
                //TODO error
            }
            w.id.assign(val.val.s.first, val.val.s.last);
        }
        else {
            goto not_find;
        }
    }

    return ;

    not_find:
    //TODO ???(widget_type, w, property_name, val)
    std::cout << ("widget set value error ") << property_name << std::endl;
    return ;
}

value_t widget_get_value(widget_values& w, const std::string& property_name)
{
    if (property_name == "color") {
        return value_t::make_color(w.color);
    }
    if (property_name == "bgcolor") {
        return value_t::make_color(w.bgcolor);
    }
    if (property_name == "border.top.color") {
        return value_t::make_color(w.borders[0].color);
    }
    if (property_name == "border.right.color") {
        return value_t::make_color(w.borders[1].color);
    }
    if (property_name == "border.bottom.color") {
        return value_t::make_color(w.borders[2].color);
    }
    if (property_name == "border.left.color") {
        return value_t::make_color(w.borders[3].color);
    }

    if (property_name == "x") {
        return value_t::make_i(w.x);
    }
    if (property_name == "y") {
        return value_t::make_i(w.y);
    }
    if (property_name == "w") {
        return value_t::make_i(w.w);
    }
    if (property_name == "h") {
        return value_t::make_i(w.h);
    }
    if (property_name == "border.top.size") {
        return value_t::make_i(w.borders[0].size);
    }
    if (property_name == "border.right.size") {
        return value_t::make_i(w.borders[1].size);
    }
    if (property_name == "border.bottom.size") {
        return value_t::make_i(w.borders[2].size);
    }
    if (property_name == "border.left.size") {
        return value_t::make_i(w.borders[3].size);
    }

    if (property_name == "id") {
        return value_t::make_s(w.id.c_str(), w.id.c_str() + w.id.size());
    }

    //TODO error
    std::cout << ("widget get value error ") << property_name << std::endl;

    return value_t();
}

struct apply_rwl_impl
{
    std::string name;
    std::string name_ref;

    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    typedef std::vector<rwl_target*>::const_iterator target_iterator;

    struct IsName
    {
        const std::string& s;

        IsName(const std::string& s)
        : s(s)
        {}

        bool operator()(widget_values * w) const
        {
            return w->id == s;
        }
    };

    value_t get_value(std::vector<widget_values*>& v, widget_values& w, const rwl_value& val)
    {
        if (val.type == 't') {
            this->name_ref.assign(val.value.s.first, val.value.s.last);
            return widget_get_value(w, this->name_ref);
        }
        //if (val.type == 'l') {
        //   typedef std::vector<widget_values*>::iterator iterator;
        //   this->name_ref.assign(val.value.s.first, val.value.s.last);
        //   iterator first = std::find_if(v.begin(), v.end(), IsName(this->name_ref));
        //   if (first == v.end()) {
        //       //TODO error
        //       std::cout << ("id not find") << std::endl;
        //   }
        //
        //   this->name_ref.clear();
        //   for (const char **first = val.value.func.first, **last = val.value.func.last; first != last; ++first) {
        //       this->name_ref.append(*first, *(first+1));
        //   }
        //
        //}
        //if (val.type == 'f') {
        //
        //}
        //if (val.type == 'o') {
        //
        //}
        //if (val.type == 'g') {
        //
        //}

        if (val.type == 'i') {
            return value_t::make_i(val.value.i);
        }
        if (val.type == 'c') {
            return value_t::make_color(val.value.color);
        }
        if (val.type == 's') {
            return value_t::make_s(val.value.s.first, val.value.s.last);
        }

        //TODO error
        std::cout << ("get value error") << std::endl;
        return value_t();
    }

    void apply_rwl(std::vector<widget_values*>& v, widget_values& w, const rwl_property& prop)
    {
        size_t pos = this->name.size();
        this->name.append(prop.name.begin(), prop.name.end());
        if (prop.properties.empty()) {
            value_t val = this->get_value(v, w, *prop.root_value);
            widget_set_value(w, this->name, val);
        }
        else {
            this->name += '.';
            for (prop_iterator first = prop.properties.begin(), last = prop.properties.end(); first != last; ++first) {
                this->apply_rwl(v, w, **first);
            }
        }
        this->name.erase(pos);
    }

    void apply_rwl(std::vector<widget_values*>& v, widget_values& w, const rwl_target& target)
    {
        for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
            this->apply_rwl(v, w, **first);
        }

        for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
            widget_values * wchild = w.new_child(target.name);
            v.push_back(wchild);
            this->apply_rwl(v, *wchild, **first);
        }
    }
};

void apply_rwl(std::vector<widget_values*>& v, widget_values& w, const rwl_target& target)
{
    rwl::apply_rwl_impl().apply_rwl(v, w, target);
}

struct rwl_set_value_base
{
    void set(const rwl_value& val)
    {
        switch (val.type) {
            case 'i':
            case 's':
            case 'c':
                this->do_set(val.type, val.value);
                break;
            case 'f':

                break;
            case 'o':
                break;
            case 'g':
                break;
            case 't':
                break;
            case 'l':
                break;
            default:
                break;
        }
    }

private:
    virtual void do_set(int type, const rwl_value::Val& val) = 0;
};

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
    const char * begin_cstr = str.c_str();
    const char * cstr = parser.parse(begin_cstr);

    if (*cstr) {
        std::cout << "\n\n\nparsing error line "
        << (std::count(begin_cstr, cstr, '\n')+1) << "\n";
        if (const char * err = parser.message_error()) {
            std::cout << (err) << "\n";
        }
        std::cout << cstr << "\n";
    }
    else if (!parser.stop()) {
        std::cout << "\n\n\ninvalid state\n";
    }

    widget_values w;
//     rwl::state_machine_assign assigner;
//     assigner.assign(w, screen);
    std::vector<widget_values*> v;
    v.push_back(&w);
    rwl::apply_rwl(v, w, screen);
    display_widget_values(w);
}
