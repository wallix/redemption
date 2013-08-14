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

#ifndef REDEMPTION_RWL_RWL_VALUE_HPP
#define REDEMPTION_RWL_RWL_VALUE_HPP

#include "basic_cstring.hpp"
#include "delete.hpp"

#include <vector>
#include <algorithm>
#include <cstring>


struct rwl_value
{
    rwl_value()
    : type(0)
    {}

private:
    rwl_value(const rwl_value& other);

public:
    ~rwl_value()
    {
        switch (this->type) {
            case 'o':
                delete this->value.operation.r;
                delete this->value.operation.l;
                break;
            case 'g':
                delete this->value.g;
                break;
            case 'l':
                delete [] this->value.linked.pair_strings;
                break;
            case 'f':
                delete_all(this->value.func.params + 0,
                           this->value.func.params + this->value.func.size);
                delete [] this->value.func.params;
                break;
            default:
                break;
        }
    }

    bool detach(rwl_value * val)
    {
        struct detach_impl {
            inline static bool impl(rwl_value * val, rwl_value *& p) {
                if (val == p) {
                    p = 0;
                    return true;
                }
                return p->detach(val);
            }
        };
        switch (this->type) {
            case 'o':
                return detach_impl::impl(val, this->value.operation.l)
                || detach_impl::impl(val, this->value.operation.r);
            case 'g':
                return detach_impl::impl(val, this->value.g);
            case 'f': {
                rwl_value** last = this->value.func.params + this->value.func.size;
                rwl_value** first= this->value.func.params;
                for (; first != last; ++first) {
                    if (detach_impl::impl(val, *first)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void add_string(const char * p, const char * e)
    {
        this->type = 's';
        this->value.s.first = p;
        this->value.s.last = e;
    }

    void add_identifier(const char * p, const char * e)
    {
        this->type = 't';
        this->value.s.first = p;
        this->value.s.last = e;
    }

    static unsigned int chex2i(char c)
    {
        if ('0' <= c && c <= '9') {
            return c - '0';
        }
        if ('A' <= c && c <= 'Z') {
            return c - 'A' + 10;
        }
        return c - 'a' + 10;
    }

    void add_color(const char * p, const char * e)
    {
        if (e - p == 3) {
            const unsigned r = this->chex2i(p[0]);
            const unsigned g = this->chex2i(p[1]);
            const unsigned b = this->chex2i(p[2]);
            this->value.color
            = (r << 20) + (r << 16)
            + (g << 12) + (g << 8)
            + (b << 4) + b;
        }
        else {
            this->value.color
            = (this->chex2i(p[0]) << 20)
            + (this->chex2i(p[1]) << 16)
            + (this->chex2i(p[2]) << 12)
            + (this->chex2i(p[3]) << 8)
            + (this->chex2i(p[4]) << 4)
            + (this->chex2i(p[5]));
        }
        this->type = 'c';
    }

    void add_integer(const char * p, const char * /*e*/)
    {
        this->type = 'i';
        this->value.i = atoi(p);
    }

    void add_operator(const char * p, const char * /*e*/)
    {
        rwl_value * val = new rwl_value;
        std::memcpy(val, this, sizeof(rwl_value));
        this->value.operation.l = val;
        this->value.operation.r = new rwl_value;
        this->value.operation.op = *p;
        this->type = 'o';
    }

    void add_linked(const char * p, const char * e)
    {
        if (this->type == 'l') {
            const unsigned size = this->value.linked.size * 2;
            const char * * values = new const char *[size + 2];
            std::copy(this->value.linked.pair_strings,
                      this->value.linked.pair_strings + size,
                      values);
            values[size] = p;
            values[size+1] = e;
            ++this->value.linked.size;
            this->value.linked.pair_strings = values;
        }
        else {
            this->value.linked.size = 1;
            this->value.linked.pair_strings = new const char * [2];
            this->value.linked.pair_strings[0] = p;
            this->value.linked.pair_strings[1] = e;
            this->type = 'l';
        }
    }

    rwl_value * to_function()
    {
        this->type = 'f';
        this->value.func.size = 1;
        this->value.func.params = new rwl_value * [1];
        this->value.func.params[0] = new rwl_value;
        return this->value.func.params[0];
    }

    rwl_value * next_parameter()
    {
        const unsigned size = this->value.func.size;
        rwl_value ** values = new rwl_value * [size+1];
        std::copy(this->value.func.params,
                  this->value.func.params + size,
                  values);
        values[size] = new rwl_value;
        ++this->value.func.size;
        delete [] this->value.func.params;
        this->value.func.params = values;
        return this->value.func.params[size];
    }

    void add_group()
    {
        this->type = 'g';
        this->value.g = new rwl_value;
    }


    union Val {
        struct S {
            const char * first;
            const char * last;
        } s;
        struct Func {
            const char * first;
            const char * last;
            rwl_value ** params;
            unsigned size;
        } func;
        struct Linked {
            const char * first;
            const char * last;
            const char * * pair_strings;
            unsigned size;
        } linked;
        rwl_value * g;
        struct Operation {
            rwl_value * l;
            rwl_value * r;
            int op;
        } operation;
        unsigned color;
        int i;
//         unsigned u;
//         long l;
//         unsigned long ul;
    } value;
    /**
     * 'i': integer
     * 'f': function
     * 's': string
     * 't': identifier
     * 'o': group operator
     * 'c': color
     * 'l': linked
     * 'g': group
     */
    int type;
};

struct rwl_property
{
    rwl_property()
    : root_value(&value)
    {}

    rwl_property(const const_cstring& name)
    : name(name)
    , root_value(&value)
    {}

    rwl_property& new_property(const const_cstring& name)
    {
        this->properties.push_back(new rwl_property(name));
        return *this->properties.back();
    }

    ~rwl_property()
    {
        delete_all(this->properties.begin(), this->properties.end());
        if (&this->value != this->root_value) {
            this->root_value->detach(&this->value);
            delete this->root_value;
        }
    }

    const_cstring name;
    rwl_value value;
    rwl_value * root_value;
    std::vector<rwl_property*> properties;

private:
    rwl_property(const rwl_property&);
};

struct rwl_target
{
    rwl_target()
    {}

    rwl_target(const const_cstring& name)
    : name(name)
    {}

    rwl_target& new_target(const const_cstring& name)
    {
        this->targets.push_back(new rwl_target(name));
        return *this->targets.back();
    }

    rwl_property& new_property(const const_cstring& name)
    {
        this->properties.push_back(new rwl_property(name));
        return *this->properties.back();
    }

    void pop_back()
    {
        delete this->targets.back();
        this->targets.pop_back();
    }

    ~rwl_target()
    {
        delete_all(this->properties.begin(), this->properties.end());
        delete_all(this->targets.begin(), this->targets.end());
    }

    const_cstring name;
    std::vector<rwl_property*> properties;
    std::vector<rwl_target*> targets;

private:
    rwl_target(const rwl_target&);
};

#endif
