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

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include <cstring>
#include <cerrno>

using std::begin;
using std::end;

template<class Inherit>
struct type_enumeration_base
{
    type_enumeration_base(char const * name, char const * desc, char const * info)
    : name(name)
    , desc(desc)
    , info(info)
    {}

    type_enumeration_base(type_enumeration_base const &) = delete;
    type_enumeration_base(type_enumeration_base &&) = default;

    type_enumeration_base & operator=(type_enumeration_base const &) = delete;
    type_enumeration_base & operator=(type_enumeration_base &&) = default;

    Inherit & set_string_parser() { this->is_string_parser = true; return this->inherit(); }

    Inherit & alias(char const * s) {
        if (this->inherit().values.back().alias) {
            throw std::runtime_error("'alias' is already defined");
        }
        this->inherit().values.back().alias = s;
        return this->inherit();
    }

protected:
    using base_type = type_enumeration_base;

    Inherit & inherit() { return static_cast<Inherit&>(*this); }

public:
    char const * name;
    char const * desc;
    char const * info;

    bool is_string_parser = false;
};


struct type_enumeration : type_enumeration_base<type_enumeration>
{
    enum flag_type { autoincrement, flags };

    type_enumeration(char const * name, flag_type flag, char const * desc, char const * info)
    : base_type(name, desc, info)
    , flag(flag)
    {}

    type_enumeration(type_enumeration &&) = default;
    type_enumeration & operator=(type_enumeration &&) = default;

    type_enumeration & value(char const * name, char const * desc = nullptr)
    {
        this->values.push_back({name, desc, nullptr});
        return *this;
    }

    unsigned long count() const
    {
        return this->values.size();
    }

    unsigned long min() const
    {
        return 0;
    }

    unsigned long max() const
    {
        return this->flag == autoincrement
            ? this->count() - 1u
            : (1u << (this->values.size() - 1u)) - 1u;
    }


    flag_type flag;

    friend base_type;

    struct Value
    {
        char const * name;
        char const * desc;
        const char * alias;
    };
    std::vector<Value> values;
};

struct type_enumeration_set : type_enumeration_base<type_enumeration_set>
{
    using base_type::base_type;

    type_enumeration_set(type_enumeration_set &&) = default;
    type_enumeration_set & operator=(type_enumeration_set &&) = default;

    type_enumeration_set & value(char const * name, long long val, char const * desc = nullptr)
    {
        this->values.push_back({name, desc, nullptr, val});
        return *this;
    }

    long long min() const
    {
        return std::min_element(this->values.begin(), this->values.end(), [](auto& a, auto& b){
            return a.val < b.val;
        })->val;
    }

    long long max() const
    {
        return std::max_element(this->values.begin(), this->values.end(), [](auto& a, auto& b){
            return a.val > b.val;
        })->val;
    }

    struct Value
    {
        char const * name;
        char const * desc;
        const char * alias;
        long long val;
    };
    std::vector<Value> values;
};


struct type_enumerations
{
    std::vector<type_enumeration> enumerations_;
    std::vector<type_enumeration_set> enumerations_set_;

    type_enumeration & enumeration_flags(
        char const * name, char const * desc = nullptr, char const * info = nullptr
    ) {
        this->enumerations_.push_back({name, type_enumeration::flags, desc, info});
        return this->enumerations_.back();
    }

    type_enumeration & enumeration_list(
        char const * name, char const * desc = nullptr, char const * info = nullptr
    ) {
        this->enumerations_.push_back({name, type_enumeration::autoincrement, desc, info});
        return this->enumerations_.back();
    }

    type_enumeration_set & enumeration_set(
        char const * name, char const * desc = nullptr, char const * info = nullptr
    ) {
        this->enumerations_set_.push_back({name, desc, info});
        return this->enumerations_set_.back();
    }

    template<class T, class Fn>
    void apply_for(Fn fn) const
    {
        auto tname = type_name<T>();
        std::string str_tname(tname.begin(), tname.end());
        if ( ! apply_enum(str_tname, fn, this->enumerations_)
         &&  ! apply_enum(str_tname, fn, this->enumerations_set_)) {
            throw std::runtime_error("unknown enum '" + str_tname + "'");
        }
    }

private:
    template<class E, class Fn>
    static bool apply_enum(std::string const & str_tname, Fn fn, std::vector<E> const & values)
    {
        auto p = std::find_if(begin(values), end(values), [&str_tname](auto & value){
            return str_tname == value.name;
        });
        if (p != end(values)) {
            fn(*p);
            return true;
        }
        return false;
    }
};

template<class T, class Fn>
void apply_enumeration_for(type_enumerations const & enums, Fn fn)
{ enums.template apply_for<T>(fn); }
