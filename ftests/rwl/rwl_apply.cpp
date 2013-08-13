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
#include <sstream>
#include <fstream>
#include <stdexcept>

struct widget_values
{
    widget_values * parent;
    struct border
    {
        unsigned color;
        unsigned size;
    };
    border borders[4];
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

namespace rwl
{
    class state_machine_assign;

    struct get_property_i
    {
        unsigned operator()(state_machine_assign&, const rwl_value& val) const
        {
            return val.value.i;
        }
    };

    struct get_property_u
    {
        unsigned operator()(state_machine_assign&, const rwl_value& val) const
        {
            if (val.value.i < 0) {
                throw std::runtime_error("number is negatif");
            }
            return val.value.i;
        }
    };

    struct get_property_color
    {
        unsigned operator()(state_machine_assign&, const rwl_value& val) const
        {
            return val.value.color;
        }
    };

    struct get_property_s
    {
        const_cstring operator()(state_machine_assign&, const rwl_value& val) const
        {
            return const_cstring(val.value.s.first, val.value.s.last);
        }
    };

#define IS_NAME(name) struct is_##name { \
    bool operator()(const const_cstring& cname){ \
        return cname == #name; \
    }\
}

    IS_NAME(x);
    IS_NAME(y);
    IS_NAME(w);
    IS_NAME(h);
    IS_NAME(color);
    IS_NAME(bgcolor);
    IS_NAME(id);
    IS_NAME(size);
    IS_NAME(type);
    IS_NAME(border);
    IS_NAME(top);
    IS_NAME(right);
    IS_NAME(bottom);
    IS_NAME(left);

#undef IS_NAME

#define GET_ATTR(type, name) \
    struct get_attr_##name { \
        template<typename T> \
        type & operator()(T& w) { \
            return w.name; \
        } \
    }

    GET_ATTR(int, x);
    GET_ATTR(int, y);
    GET_ATTR(unsigned, w);
    GET_ATTR(unsigned, h);
    GET_ATTR(unsigned, color);
    GET_ATTR(unsigned, bgcolor);
    GET_ATTR(unsigned, size);

#undef GET_ATTR

    struct get_attr_id
    {
        template<typename T>
        struct dispatcher {
            T& w;

            dispatcher(T& w)
            : w(w)
            {}

            void operator=(const const_cstring& s) const
            {
                w.id.assign(s.begin(), s.end());
            }
        };

        template<typename T>
        dispatcher<T> operator()(T& w)
        {
            return dispatcher<T>(w);
        }
    };


    template<std::size_t N, std::size_t Max, typename Table>
    struct state_machine_assign_row
    {
        static bool impl(state_machine_assign& sm, widget_values& w,
                         const rwl_property& property)
        {
            typedef typename boost::mpl::at<Table, boost::mpl::int_<N> >::type row;
            typedef typename row::check_name check_name;
            typedef typename row::get_property get_property;
            typedef typename row::get_attr get_attr;
            if (row::integral_type == property.value.type && check_name()(property.name)) {
                get_attr()(w) = get_property()(sm, *property.root_value);
                return true;
            }
            return state_machine_assign_row<N+1, Max, Table>::impl(sm, w, property);
        }
    };

    template<std::size_t Max, typename Table>
    struct state_machine_assign_row<Max, Max, Table>
    {
        static bool impl(state_machine_assign&, widget_values&, const rwl_property&)
        { return false; }
    };


    template<typename Group>
    class state_machine_assign_row_group;

    template<std::size_t N, std::size_t Max, typename Table>
    struct state_machine_assign_group
    {
        static bool impl(state_machine_assign& sm, widget_values& w,
                         const rwl_property& property)
        {
            typedef typename RowGroup::check_name check_name;
            if (check_name()(property)) {
                if (property.properties.empty()) {
                    state_machine_assign_row<
                    0,
                    boost::mpl::size<typename RowGroup::row_type>::value,
                    typename RowGroup::row_type
                    >::impl(sm, w, property);
                }
                else {
                    state_machine_assign_row_group<typename RowGroup::group_type>
                    ::impl(sm, w, property);
                }
            }
            return false;
        }
    };

    template<std::size_t Max, typename Table>
    struct state_machine_assign_group<Max, Max, Table>
    {
        static bool impl(state_machine_assign&, widget_values&, const rwl_property&)
        { return false; }
    };


    template<typename RowGroup>
    struct state_machine_assign_row_group
    {
        static bool impl(state_machine_assign& sm, widget_values& w,
                         const rwl_property& property)
        {
            typedef typename RowGroup::check_name check_name;
            if (check_name()(property)) {
                if (property.properties.empty()) {
                    state_machine_assign_row<
                        0,
                        boost::mpl::size<typename RowGroup::row_type>::value,
                        typename RowGroup::row_type
                    >::impl(sm, w, property);
                }
                else {
                    state_machine_assign_row_group<typename RowGroup::group_type>
                    ::impl(sm, w, property);
                }
            }
            return false;
        }
    };

    template<>
    struct state_machine_assign_row_group<boost::mpl::vector<> >
    {
        static bool impl(state_machine_assign&, widget_values&, const rwl_property&)
        { return false; }
    };


    class state_machine_assign
    {
    public:
        template <typename Row, typename Group = boost::mpl::vector<> >
        struct row_group
        {
            typedef Row row_type;
            typedef Group group_type;
        };

        template<int IntegralType, typename Is, typename GetProperty, typename GetAttr>
        struct row
        {
            static const bool is_row = true;
            static const int integral_type = IntegralType;
            typedef Is check_name;
            typedef GetProperty get_property;
            typedef GetAttr get_attr;
        };

        template<typename Is, typename RowGroup>
        struct group
        {
            static const bool is_row = false;
            typedef Is check_name;
            typedef RowGroup::row row_type;
            typedef typename RowGroup::group group_type;
        };

        class table_properties;

        void assign(widget_values& w, const rwl_property& property)
        {
            state_machine_assign_row_group<table_properties>::impl(*this, w, property);
        }

        void assign(widget_values& w, const rwl_target& target)
        {
            typedef std::vector<rwl_property*>::const_iterator prop_iterator;
            for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
                this->assign(w, **first);
            }

            typedef std::vector<rwl_target*>::const_iterator target_iterator;
            for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
                widget_values * wchild = w.new_child(target.name);
                this->assign(*wchild, **first);
            }
        }

        typedef row< 'i',   is_x,       get_property_i,         get_attr_x      > row_x;
        typedef row< 'i',   is_y,       get_property_i,         get_attr_y      > row_y;
        typedef row< 'i',   is_w,       get_property_u,         get_attr_w      > row_w;
        typedef row< 'i',   is_h,       get_property_u,         get_attr_h      > row_h;
        typedef row< 'c',   is_color,   get_property_color,     get_attr_color  > row_color;
        typedef row< 'c',   is_bgcolor, get_property_color,     get_attr_bgcolor> row_bgcolor;
        typedef row< 's',   is_id,      get_property_s,         get_attr_id     > row_id;
        typedef row< 's',   is_size,    get_property_u,         get_attr_size   > row_size;

        struct table_properties : row_group<
            boost::mpl::vector<
                row_x,
                row_y,
                row_w,
                row_h,
                row_color,
                row_bgcolor,
                row_id
            >,
            boost::mpl::vector<
                group<
                    is_border,
                    row_group<
                        boost::mpl::vector<
                            row_color,
                            row_size
                        >
                    >
                >
            >
        >
        {
            struct check_name {
                bool operator()(const rwl_property&) const
                { return true; }
            };
        };
    };
}


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
    //rwl2widget(w, screen);
    rwl::state_machine_assign assigner;
    assigner.assign(w, screen);
    display_widget_values(w);
}