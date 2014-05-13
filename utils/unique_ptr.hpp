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

namespace detail {
    template<class T>
    struct is_trait_deleter
    { static const bool value = false; };

    template<class T>
    struct is_trait_deleter<default_delete<T> >
    { static const bool value = true; };
}

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

    template<class P, class T, class = void>
    struct use_pointer
    { typedef P type; };

    template<class P, class T>
    struct use_pointer<P, T, typename enable_if_type<typename T::pointer>::type>
    { typedef typename T::pointer type; };

    template<class T, class Deleter, class Pointer, bool = detail::is_trait_deleter<Deleter>::value>
    struct unique_ptr_data
    {
        Pointer p_;
        Deleter deleter_;

        typedef Deleter & deleter_reference;
        typedef Deleter const & const_deleter_reference;

        explicit unique_ptr_data()
        : p_(0)
        {}

        explicit unique_ptr_data(Pointer ptr)
        : p_(ptr)
        {}

        unique_ptr_data(Pointer ptr, Deleter & deleter)
        : p_(ptr)
        , deleter_(deleter)
        {}

        unique_ptr_data(Pointer ptr, Deleter const & deleter)
        : p_(ptr)
        , deleter_(deleter)
        {}

        template<class U>
        unique_ptr_data& operator=(aux_::unique_ptr_move<U> other)
        {
            p_ = other.u.release();
            deleter_ = other.u.get_deleter();
            return *this;
        }

        ~unique_ptr_data()
        {
            deleter_(p_);
        }

        Deleter & get_deleter()
        {
            return deleter_;
        }

        const Deleter & get_deleter() const
        {
            return deleter_;
        }

        void swap(unique_ptr_data& other)
        {
            using std::swap;
            swap(p_, other.p_);
            swap(deleter_, other.deleter_);
        }

    private:
        unique_ptr_data(const unique_ptr_data&);
    };

    template<class T, class Deleter, class Pointer>
    struct unique_ptr_data<T, Deleter, Pointer, true>
    {
        Pointer p_;

        typedef Deleter deleter_reference;
        typedef Deleter const_deleter_reference;

        explicit unique_ptr_data()
        : p_(0)
        {}

        explicit unique_ptr_data(Pointer ptr)
        : p_(ptr)
        {}

        unique_ptr_data(Pointer ptr, Deleter)
        : p_(ptr)
        {}

        template<class U>
        unique_ptr_data& operator=(aux_::unique_ptr_move<U> other)
        {
            p_ = other.u.release();
            return *this;
        }

        ~unique_ptr_data()
        {
            Deleter()(p_);
        }

        Deleter get_deleter() const
        {
            return Deleter();
        }

        void swap(unique_ptr_data& other)
        {
            using std::swap;
            swap(p_, other.p_);
        }

    private:
        unique_ptr_data(const unique_ptr_data&);
    };

    template<class T>
    struct pointer_trait
    { typedef T* type; };

    template<class T>
    struct pointer_trait<T[]>
    { typedef T* type; };
}

template<class T, class Deleter = default_delete<T> >
struct unique_ptr
{
    typedef T element_type;
    typedef Deleter deleter_type;
    typedef typename aux_::use_pointer<typename aux_::pointer_trait<T>::type, Deleter>::type pointer;

    explicit unique_ptr()
    : data()
    {}

    explicit unique_ptr(pointer ptr)
    : data(ptr)
    {}

    unique_ptr(pointer ptr, deleter_type & deleter)
    : data(ptr, deleter)
    {}

    unique_ptr(pointer ptr, deleter_type const & deleter)
    : data(ptr, deleter)
    {}

    unique_ptr(aux_::unique_ptr_move<unique_ptr> other)
    : data(other.u.release(), other.u.get_deleter())
    {}

    unique_ptr& operator=(aux_::unique_ptr_move<unique_ptr> other)
    {
        data = other;
        return *this;
    }

    pointer get() const
    {
        return data.p_;
    }

    typename aux_::unique_ptr_data<T, Deleter, pointer>::deleter_reference
    get_deleter()
    {
        return data.get_deleter();
    }

    typename aux_::unique_ptr_data<T, Deleter, pointer>::const_deleter_reference
    get_deleter() const
    {
        return data.get_deleter();
    }

    operator bool() const
    { return data.p_ != 0; }

    T& operator*() const
    { return data.p_; }

    pointer operator->() const
    { return data.p_; }

    void swap(unique_ptr& other)
    {
        data.swap(other.data);
    }

    void reset(pointer ptr = pointer())
    {
        data.get_deleter()(data.p_);
        data.p_ = ptr;
    }

    pointer release()
    {
        pointer ret = data.p_;
        data.p_ = 0;
        return ret;
    }

    T& operator[](std::size_t i) const
    {
        return data.p_[i];
    }

private:
    unique_ptr(const unique_ptr &);

    template<class U>
    void reset( U );

    aux_::unique_ptr_data<T, Deleter, pointer> data;
};

template<class T, class Deleter>
aux_::unique_ptr_move<unique_ptr<T, Deleter> >
move(unique_ptr<T, Deleter> & u)
{ return aux_::unique_ptr_move<unique_ptr<T, Deleter> >(u); }

#endif


#endif
