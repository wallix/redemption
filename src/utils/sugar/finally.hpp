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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#pragma once

template<class F>
struct basic_finally {
    F f;
    basic_finally(basic_finally const &) = default;
    basic_finally(basic_finally &&) = default;
    basic_finally & operator=(basic_finally const &) = default;
    basic_finally & operator=(basic_finally &&) = default;
    ~basic_finally() { f(); }
};

template<class F>
basic_finally<F> finally(F&& f) {
    return {static_cast<F&&>(f)};
}

template<class F>
basic_finally<F> finally(F& f) {
    return {static_cast<F&>(f)};
}

/// \attention finnaly not must throw an exception
template<class F, class Finally>
auto try_except(F f, Finally finally) -> decltype(f()) {
    basic_finally<Finally&> d{finally};
    return f();
}


namespace detail {
    template<class F, class R = decltype(std::declval<F>()())>
    struct rethrow_try_except_result
    { using no_void_type = R; };

    template<class F>
    struct rethrow_try_except_result<F, void>
    { using void_type = void; };
}

template<class F, class Finally>
typename detail::rethrow_try_except_result<F>::void_type
rethrow_try_except(F f, Finally finally)
{
    if (noexcept(finally())) {
        basic_finally<Finally&> d{finally};
        f();
        return ;
    }

    try {
        f();
    }
    catch (...) {
        finally();
        throw;
    }
    finally();
}

template<class F, class Finally>
typename detail::rethrow_try_except_result<F>::no_void_type
rethrow_try_except(F f, Finally finally)
{
    if (noexcept(finally())) {
        basic_finally<Finally&> d{finally};
        return f();
    }

    decltype(f()) ret;
    try {
        ret = f();
    }
    catch (...) {
        finally();
        throw;
    }
    finally();
    return ret;
}

