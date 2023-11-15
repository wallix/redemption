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
    enum class DisplayNameOption : bool { WithoutNameWhenDescription, WithNameWhenDdescription };

    struct value_type
    {
        std::string_view name;
        std::string_view desc;
        std::string_view alias;
        uint64_t val;
        // bool is_negative;
        bool exclude = false;

        std::string_view get_name() const { return alias.empty() ? name : alias; }
    };

    std::string_view name;
    std::string_view desc;
    std::string_view info;

    Category cat;
    DisplayNameOption display_name_option;

    std::vector<value_type> values {};

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
    void _alias(std::string_view s)
    {
        if (!this->values.back().alias.empty()) {
            throw std::runtime_error("'alias' is already defined");
        }
        this->values.back().alias = s;
    }
};

struct type_enumeration_inc : type_enumeration
{
    using type_enumeration::type_enumeration;

    type_enumeration_inc & value(std::string_view name, std::string_view desc = {})
    {
        uint64_t value = this->values.size();
        if (cat == Category::flags && value) {
            value = 1ull << (value - 1u);
        }
        this->values.push_back({name, desc, std::string_view(), value});
        return *this;
    }

    type_enumeration_inc & exclude()
    {
        this->values.back().exclude = true;
        return *this;
    }

    type_enumeration_inc & alias(std::string_view s)
    {
        this->_alias(s);
        return *this;
    }
};

struct type_enumeration_set : type_enumeration
{
    type_enumeration_set & value(std::string_view name, unsigned long long val, std::string_view desc = {})
    {
        this->values.push_back({name, desc, std::string_view(), val});
        return *this;
    }

    type_enumeration_set & alias(std::string_view s)
    {
        this->_alias(s);
        return *this;
    }
};


struct type_enumerations
{
    using DisplayNameOption = type_enumeration::DisplayNameOption;
    using Category = type_enumeration::Category;

    std::vector<type_enumeration> enumerations_;

    type_enumeration_inc & enumeration_flags(
        std::string_view name, DisplayNameOption display_opt,
        std::string_view desc = {}, std::string_view info = {})
    {
        this->enumerations_.push_back({name, desc, info, Category::flags, display_opt});
        return static_cast<type_enumeration_inc&>(this->enumerations_.back());
    }

    type_enumeration_inc & enumeration_list(
        std::string_view name, DisplayNameOption display_opt,
        std::string_view desc = {}, std::string_view info = {})
    {
        this->enumerations_.push_back({name, desc, info, Category::autoincrement, display_opt});
        return static_cast<type_enumeration_inc&>(this->enumerations_.back());
    }

    type_enumeration_set & enumeration_set(
        std::string_view name, DisplayNameOption display_opt,
        std::string_view desc = {}, std::string_view info = {})
    {
        this->enumerations_.push_back({name, desc, info, Category::set, display_opt});
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
            [&str_tname](type_enumeration const& e){ return str_tname == e.name; }
        );

        if (p != enumerations_.end()) {
            return *p;
        }

        throw std::runtime_error(str_concat("unknown enum '", str_tname, "'"));
    }
};
