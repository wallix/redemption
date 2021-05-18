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

#pragma once

#include "configs/type_name.hpp"
#include "utils/strutils.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <cassert>


struct type_enumeration
{
    enum class Category { autoincrement, flags, set };

    struct value_type
    {
        char const * name;
        char const * desc;
        char const * alias;
        uint64_t val;
        // bool is_negative;
        bool exclude = false;

        char const * get_name() const { return alias ? alias : name; }
    };

    char const * name;
    char const * desc;
    char const * info;

    Category cat;

    std::vector<value_type> values;

    type_enumeration(char const * name, char const * desc, char const * info, Category cat)
    : name(name)
    , desc(desc)
    , info(info)
    , cat(cat)
    {}

    uint64_t min() const
    {
        assert(this->values.size());

        switch (cat)
        {
            case Category::autoincrement:
            case Category::flags:
                return 0;
            case Category::set:
                return std::min_element(
                    this->values.begin(), this->values.end(),
                    [](auto& a, auto& b){ return a.val < b.val; }
                )->val;
        }

        REDEMPTION_UNREACHABLE();
    }

    uint64_t max() const
    {
        assert(this->values.size());

        switch (cat)
        {
            case Category::autoincrement:
                return this->values.size() - 1u;
            case Category::flags:
                return ~uint64_t() >> (64u - (this->values.size() - 1u));
            case Category::set:
                return std::min_element(
                    this->values.begin(), this->values.end(),
                    [](auto& a, auto& b){ return a.val > b.val; }
                )->val;
        }

        REDEMPTION_UNREACHABLE();
    }

protected:
    void _alias(char const * s)
    {
        if (this->values.back().alias) {
            throw std::runtime_error("'alias' is already defined");
        }
        this->values.back().alias = s;
    }
};

struct type_enumeration_inc : type_enumeration
{
    using type_enumeration::type_enumeration;

    type_enumeration_inc & value(char const * name, char const * desc = nullptr)
    {
        uint64_t value = this->values.size();
        if (cat == Category::flags && value) {
            value = 1ull << (value - 1u);
        }
        this->values.push_back({name, desc, nullptr, value});
        return *this;
    }

    type_enumeration_inc & exclude()
    {
        this->values.back().exclude = true;
        return *this;
    }

    type_enumeration_inc & alias(char const * s)
    {
        this->_alias(s);
        return *this;
    }
};

struct type_enumeration_set : type_enumeration
{
    type_enumeration_set & value(char const * name, unsigned long long val, char const * desc = nullptr)
    {
        this->values.push_back({name, desc, nullptr, val});
        return *this;
    }

    type_enumeration_set & alias(char const * s)
    {
        this->_alias(s);
        return *this;
    }
};


struct type_enumerations
{
    std::vector<type_enumeration> enumerations_;

    type_enumeration_inc & enumeration_flags(
        char const * name, char const * desc = nullptr, char const * info = nullptr
    ) {
        this->enumerations_.push_back({name, desc, info, type_enumeration::Category::flags});
        return static_cast<type_enumeration_inc&>(this->enumerations_.back());
    }

    type_enumeration_inc & enumeration_list(
        char const * name, char const * desc = nullptr, char const * info = nullptr
    ) {
        this->enumerations_.push_back({name, desc, info, type_enumeration::Category::autoincrement});
        return static_cast<type_enumeration_inc&>(this->enumerations_.back());
    }

    type_enumeration_set & enumeration_set(
        char const * name, char const * desc = nullptr, char const * info = nullptr
    ) {
        this->enumerations_.push_back({name, desc, info, type_enumeration::Category::set});
        return static_cast<type_enumeration_set&>(this->enumerations_.back());
    }

    template<class T>
    type_enumeration const & get_enum() const
    {
        return this->_get_enum(type_name<T>());
    }

private:
    type_enumeration const & _get_enum(std::string_view str_tname) const
    {
        auto p = std::find_if(
            enumerations_.begin(),
            enumerations_.end(),
            [&str_tname](auto & value){ return str_tname == value.name; }
        );

        if (p != enumerations_.end()) {
            return *p;
        }

        throw std::runtime_error(str_concat("unknown enum '", str_tname, "'"));
    }
};
