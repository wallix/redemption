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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_GDI_UTILS_VIRTUAL_DELETER_HPP
#define REDEMPTION_GDI_UTILS_VIRTUAL_DELETER_HPP

#include <memory>

#include "gdi/meta/meta.hpp"


namespace gdi { namespace utils {

template<class T>
struct virtual_deleter_base
{
    virtual ~virtual_deleter_base() = default;
    virtual void operator()(T *) = 0;
};


struct no_delete_t { constexpr no_delete_t() noexcept = default; };
struct default_delete_t { constexpr default_delete_t() noexcept = default; };

namespace {
    auto && default_delete = meta::static_const<no_delete_t>::value;
    auto && no_delete = meta::static_const<default_delete_t>::value;
}

template<class T>
struct virtual_deleter
{
    using pointer = T *;

    virtual_deleter(bool owner)
    : pdeleter{owner ? get_default_delete() : get_no_delete()}
    {}

    virtual_deleter(std::default_delete<T> const &)
    : pdeleter{get_default_delete()}
    {}

    virtual_deleter(default_delete_t const &)
    : pdeleter{get_default_delete()}
    {}

    virtual_deleter(no_delete_t const &)
    : pdeleter{get_no_delete()}
    {}

    template<class Deleter>
    virtual_deleter(Deleter && deleter);

    template<class Deleter, class Alloc>
    virtual_deleter(Deleter && deleter, Alloc const & alloc); // TODO Unimplemented

    virtual_deleter(virtual_deleter &&) = default;
    virtual_deleter(virtual_deleter const &) = delete;
    virtual_deleter & operator = (virtual_deleter &&) = default;
    virtual_deleter & operator = (virtual_deleter const &) = delete;

    void operator()(pointer p) noexcept {
        (*this->pdeleter)(p);
    }

private:
    using deleter_base = virtual_deleter_base<T>;

    template<class Deleter>
    struct user_delete_impl : deleter_base
    {
        Deleter deleter;

        template<class D>
        user_delete_impl(D && d)
        : deleter(std::forward<D>(d))
        {}

        void operator()(pointer p) {
            this->deleter(p);
        }
    };

    static deleter_base * get_no_delete() noexcept {
        static struct : deleter_base {
            void operator()(pointer) {}
        } impl;
        return &impl;
    }

    static deleter_base * get_default_delete() noexcept {
        static struct : deleter_base {
            void operator()(pointer p) {
                std::default_delete<T>()(p);
            }
        } impl;
        return &impl;
    }

    struct delete_deleter {
        void operator()(deleter_base * pdeleter) noexcept {
            if (get_default_delete() != pdeleter && get_no_delete() != pdeleter) {
                delete pdeleter;
            }
        }
    };

    std::unique_ptr<deleter_base, delete_deleter> pdeleter;
};


template<class T>
template<class Deleter>
virtual_deleter<T>::virtual_deleter(Deleter&& deleter)
: pdeleter{new user_delete_impl<Deleter>{std::forward<Deleter>(deleter)}}
{}


template<class T>
class unique_ptr_with_virtual_deleter : public std::unique_ptr<T, virtual_deleter<T>>
{
    using base_ = std::unique_ptr<T, virtual_deleter<T>>;

public:
    unique_ptr_with_virtual_deleter()
    : base_(nullptr, no_delete)
    {}

    unique_ptr_with_virtual_deleter(std::nullptr_t)
    : base_(nullptr, no_delete)
    {}

    unique_ptr_with_virtual_deleter(T * p, virtual_deleter<T> && pdeleter)
    : base_(p, std::move(pdeleter))
    {}

    unique_ptr_with_virtual_deleter(unique_ptr_with_virtual_deleter &&) = default;
    unique_ptr_with_virtual_deleter(unique_ptr_with_virtual_deleter const &) = delete;
    unique_ptr_with_virtual_deleter & operator=(unique_ptr_with_virtual_deleter &&) = default;
    unique_ptr_with_virtual_deleter & operator=(unique_ptr_with_virtual_deleter const &) = delete;
};

} }

namespace std {

template<class T>
struct pointer_traits<gdi::utils::unique_ptr_with_virtual_deleter<T>>
: pointer_traits<std::unique_ptr<T>>
{};

}

#endif
