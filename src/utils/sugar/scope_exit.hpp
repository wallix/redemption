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
Copyright (C) Wallix 2017
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/pp.hpp"
#include "utils/sugar/finally.hpp"

#define SCOPE_EXIT(...)                \
    auto RED_PP_CAT(scope_exit__,      \
        RED_PP_CAT(__COUNTER__,        \
            RED_PP_CAT(__, __LINE__))) \
    = ::finally([&]{ __VA_ARGS__; })

// namespace ReDemPtion
// {
//     namespace detail
//     {
//         template <typename F>
//         struct ScopeGuardOnExit
//         {
//             constexpr ScopeGuardOnExit(F&& f) noexcept(noexcept(F(f)))
//             : f(static_cast<F>(f))
//             {}
//
//             ~ScopeGuardOnExit()
//             {
//                 f();
//             }
//
//         private:
//             F f;
//         };
//
//         struct ScopeGuardMaker
//         {
//             template <typename F>
//             constexpr ScopeGuardOnExit<F> operator+(F&& f) noexcept(noexcept(F(f)))
//             {
//                 return static_cast<F&&>(f);
//             }
//         };
//     }
// }
//
// #define scope_exit
//     auto REDEMPTION_PP_CONCAT(scope_exit__,
//         REDEMPTION_PP_CONCAT(__COUNTER__,
//             REDEMPTION_PP_CONCAT(__, __LINE__)))
//     = ::ReDemPtion::detail::ScopeGuardMaker() + [&]()
