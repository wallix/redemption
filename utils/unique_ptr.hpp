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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#ifndef REDEMPTION_PUBLIC_UTILS_UNIQUE_PTR_HPP
#define REDEMPTION_PUBLIC_UTILS_UNIQUE_PTR_HPP

#include <cstdlib>

template<class T>
struct default_free
{
    void operator()(T * p) const
    {
        ::free(p);
    }
};


#if __cplusplus >= 201103L && !defined(IN_IDE_PARSER)

#include <memory>

using std::unique_ptr;
using std::default_delete;

#else

#include <algorithm>

template<class T>
struct default_delete
{
    void operator()(T * p) const
    {
        delete p;
    }
};

template<class T>
struct default_delete<T[]>
{
    void operator()(T * p) const
    {
        delete [] p;
    }
};

namespace aux_ {
    template<class T>
    struct unique_ptr_move
    {
        T & u;
        unique_ptr_move(T & r)
        : u(r)
        {}
    };

    template<class T, class U = void>
    struct enable_if_type
    { typedef U type; };

    template<class T, class P, class = void>
    struct use_pointer
    { typedef P type; };

    template<class T, class P>
    struct use_pointer<T, P, typename enable_if_type<typename T::pointer>::type>
    { typedef typename T::pointer type; };

    template<class T>
    struct pointer_trait
    { typedef T* type; };

    template<class T>
    struct pointer_trait<T[]>
    { typedef T* type; };


    template<class T>
    struct add_lvalue_reference
    { typedef T& type;};

    template<class T>
    struct add_lvalue_reference<T[]>
    { typedef T& type;};
}

template<class T, class Deleter = default_delete<T> >
struct unique_ptr
{
    typedef T element_type;
    typedef Deleter deleter_type;
    typedef typename aux_::use_pointer<Deleter, typename aux_::pointer_trait<T>::type>::type pointer;

    unique_ptr()
    {}

    explicit unique_ptr(pointer ptr)
    : data_(ptr)
    {}

    explicit unique_ptr(pointer ptr, Deleter & deleter)
    : data_(ptr, deleter)
    {}

    explicit unique_ptr(pointer ptr, Deleter const & deleter)
    : data_(ptr, deleter)
    {}

    unique_ptr(aux_::unique_ptr_move<unique_ptr> other)
    : data_(other.u.release())
    {}

    ~unique_ptr()
    {}

    unique_ptr& operator=(aux_::unique_ptr_move<unique_ptr> other)
    {
        reset(other.u.release());
        return *this;
    }

    pointer get() const
    {
        return data_.p_;
    }

    Deleter & get_deleter()
    {
        return data_;
    }

    Deleter const & get_deleter() const
    {
        return data_;
    }

    operator bool() const
    {
        return data_.p_ != 0;
    }

    typename aux_::add_lvalue_reference<T>::type
    operator*() const
    {
        return *data_.p_;
    }

    pointer operator->() const
    {
        return data_.p_;
    }

    typename aux_::add_lvalue_reference<T>::type
    operator[](std::size_t i) const
    {
        return data_.p_[i];
    }

    void swap(unique_ptr& other)
    {
        using std::swap;
        swap(data_.p_, other.data_.p_);
        swap(get_deleter(), other.get_deleter());
    }

    void reset(pointer ptr = pointer())
    {
        get_deleter()(data_.p_);
        data_.p_ = ptr;
    }

    pointer release()
    {
        pointer ret = data_.p_;
        data_.p_ = 0;
        return ret;
    }

private:
    unique_ptr(const unique_ptr &);
    unique_ptr& operator=(const unique_ptr &);

    template<class U>
    void reset( U );

    struct Data : Deleter {
        pointer p_;

        Data()
        : p_()
        {}

        Data(pointer p)
        : p_(p)
        {}

        Data(pointer p, deleter_type const & deleter)
        : Deleter(deleter)
        , p_(p)
        {}

        Data(pointer p, deleter_type & deleter)
        : Deleter(deleter)
        , p_(p)
        {}

        ~Data()
        {
            (*this)(p_);
        }
    } data_;
};

template<class T, class Deleter>
aux_::unique_ptr_move<unique_ptr<T, Deleter> >
move(unique_ptr<T, Deleter> & u)
{ return aux_::unique_ptr_move<unique_ptr<T, Deleter> >(u); }

namespace std {
    template<class T, class Deleter>
    void swap(::unique_ptr<T, Deleter> & a, ::unique_ptr<T, Deleter> & b)
    { a.swap(b); }
}

#endif

#endif
