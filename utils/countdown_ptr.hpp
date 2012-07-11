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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 */

#if !defined(__COUNTDOWN_PTR_HPP__)
#define __COUNTDOWN_PTR_HPP__

#include <cstddef>

template<typename _T>
class CountdownPtr
{
    template<typename _U>
    struct pointer_traits
    { typedef _U type; };
    template<typename _U>
    struct pointer_traits<_U[]>
    { typedef _U* type; };

public:
    typedef typename pointer_traits<_T>::type pointer;

private:
    template<typename _U>
    struct default_delete;
    template<typename _U>
    struct default_delete<_U*> {
        static void destroy(_U* ptr)
        {
            delete ptr;
        }
    };
    template<typename _U>
    struct default_delete<_U[]> {
        static void destroy(_U* ptr)
        {
            delete[] ptr;
        }
    };

private:
    std::size_t* count;
    pointer ptr;

public:
    CountdownPtr()
    : count(new std::size_t(0))
    , ptr(0)
    {}

    CountdownPtr(pointer ptr)
    : count(new std::size_t(ptr ? 1 : 0))
    , ptr(ptr)
    {}

    CountdownPtr(const CountdownPtr<_T>& other)
    : count(other.count)
    , ptr(other.ptr)
    {
        if (this->ptr)
            ++*this->count;
    }

    CountdownPtr& operator=(const CountdownPtr<_T>& other)
    {
        if (other.count != this->count)
        {
            if (*this->count && !--*this->count)
                this->destroy();
            this->count = other.count;
            this->ptr = other.ptr;
            if (this->ptr)
                ++*this->count;
        }
    }

    ~CountdownPtr()
    {
        if (*this->count && !--*this->count)
            this->destroy();
    }

    pointer get()
    { return this->ptr; }

private:
    void destroy()
    {
        default_delete<_T>::destroy(this->ptr);
        delete this->count;
    }
};

#endif
