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

#include <map>

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

    typedef std::map<pointer, std::size_t> map_type;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::value_type value_type;

private:
    map_type _map;

public:
    CountdownPtr()
    : _map()
    { }

    ~CountdownPtr()
    {
        for (iterator it = this->_map.begin(), end = this->_map.end(); it != end; ++it)
            this->destroy(it);
    }

    void insert(pointer data)
    {
        iterator it = this->_map.find(data);
        if (it == this->_map.end())
            this->_map.insert(value_type(data, 1));
        else
            ++it->second;
    }

    void erase(pointer data)
    {
        iterator it = this->_map.find(data);
        if (it != this->_map.end())
        {
            if (!--it->second)
            {
                this->destroy(it);
                this->_map.erase(it);
            }
        }
    }

private:
    void destroy(iterator& it)
    {
        default_delete<_T>::destroy(it->first);
    }
};

#endif
