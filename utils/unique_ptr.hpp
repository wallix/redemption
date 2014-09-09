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

#ifndef REDEMPTION_PUBLIC_UTILS_UNIQUE_PTR_HPP
#define REDEMPTION_PUBLIC_UTILS_UNIQUE_PTR_HPP

#include <cstdlib>

struct default_free
{
    void operator()(void * p) const
    {
        ::free(p);
    }
};


#if __cplusplus >= 201103L

#include <memory>

using std::unique_ptr;
using std::default_delete;

#if __cplusplus == 201103L
namespace std {
    template<typename _Tp>
    struct _MakeUniq
    { typedef unique_ptr<_Tp> __single_object; };

    template<typename _Tp>
    struct _MakeUniq<_Tp[]>
    { typedef unique_ptr<_Tp[]> __array; };

    template<typename _Tp, size_t _Bound>
    struct _MakeUniq<_Tp[_Bound]>
    { struct __invalid_type { }; };

    /// std::make_unique for single objects
    template<typename _Tp, typename... _Args>
    inline typename _MakeUniq<_Tp>::__single_object
    make_unique(_Args&&... __args)
    { return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...)); }

    /// std::make_unique for arrays of unknown bound
    template<typename _Tp>
    inline typename _MakeUniq<_Tp>::__array
    make_unique(size_t __num)
    { return unique_ptr<_Tp>(new typename remove_extent<_Tp>::type[__num]()); }

    /// Disable std::make_unique for arrays of known bound
    template<typename _Tp, typename... _Args>
    inline typename _MakeUniq<_Tp>::__invalid_type
    make_unique(_Args&&...) = delete;
}
#endif

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

    template<bool, class T, class U>
    struct if_
    { typedef T type; };

    template<class T, class U>
    struct if_<false, T, U>
    { typedef U type; };

    template<class T>
    struct is_reference
    { static const bool value = false; };

    template<class T>
    struct is_reference<T&>
    { static const bool value = true; };
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

    struct DeleterReference {
        Deleter deleter;
        DeleterReference(deleter_type const & deleter)
        : deleter(deleter)
        {}

        void operator()(pointer p) const {
            deleter(p);
        }

        operator Deleter () const {
            return this->deleter;
        }
    };

    typedef typename aux_::if_<aux_::is_reference<Deleter>::value, DeleterReference, Deleter>::type deleter_base;

    struct Data : deleter_base {
        pointer p_;

        Data()
        : p_()
        {}

        Data(pointer p)
        : p_(p)
        {}

        Data(pointer p, deleter_type const & deleter)
        : deleter_base(deleter)
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
