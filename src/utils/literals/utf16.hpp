/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include <utility>

#include "cxx/diagnostic.hpp"
#include "utils/string_c.hpp"
#include "utils/sugar/array_view.hpp"


namespace detail
{
    template<std::size_t... I, class F>
    constexpr decltype(auto) apply(std::integer_sequence<std::size_t, I...>, F&& f)
    {
        return static_cast<F&&>(f)(std::integral_constant<std::size_t, I>{}...);
    }
}

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
template<class C, C... cs>
constexpr array_view_const_char const operator "" _utf16_le() noexcept
{
    constexpr auto unicode = [&]{
        std::array<uint8_t, sizeof...(cs) * 2> a{};
        auto p = a.data();
        for (char c : {cs...})
        {
            *p++ = c;
            *p++ = 0;
        }
        return a;
    }();

    return detail::apply(std::make_index_sequence<unicode.size()>{}, [&](auto... i){
        return array_view_const_char{jln::string_c<unicode[i.value]...>::value, sizeof...(i)};
    });
}
REDEMPTION_DIAGNOSTIC_POP
