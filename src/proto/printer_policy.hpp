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

#include "proto/proto.hpp"
#include <iostream>

struct Printer
{
    template<class var, class T>
    void operator()(proto::val<var, T> x) const {
        std::cout << x.var.name() << " = ";
        print(x.x, 1);
        std::cout
            << "  static: " << proto::is_static_buffer<typename var::desc_type>{}
            << "  dyn: " << proto::is_dynamic_buffer<typename var::desc_type>{}
            << "  view: " << proto::is_view_buffer<typename var::desc_type>{}
            << "  limited: " << proto::is_limited_buffer<typename var::desc_type>{}
            << "\n";
    }

    template<class T, class tag>
    void operator()(proto::var<proto::types::pkt_sz<T>, tag>) const {
        std::cout << "[pkt_sz]\n";
    }

    template<class T, class tag>
    void operator()(proto::var<proto::types::pkt_sz_with_self<T>, tag>) const {
        std::cout << "[pkt_sz_with_self]\n";
    }

    template<class T>
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.val))
    {
        using type = decltype(x.val);
        using casted_type = std::conditional_t<
            std::is_same<type, unsigned char>::value ||
            std::is_same<type, proto::safe_int<unsigned char>>::value,
            int,
            type const &
        >;
        std::cout << static_cast<casted_type>(x.val);
    }

    template<class T>
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.av.data()))
    {
        std::cout << x.av.data();
    }

    template<class T>
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.str.data()))
    {
        std::cout << x.str.data();
    }

    template<class T>
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.is_ok))
    {
        std::cout << x.is_ok << " ?: "; print(x.val_ok, 1);
    }

    template<class T>
    static void print(T const & x, char)
    {
        std::cout << x;
    }
};
