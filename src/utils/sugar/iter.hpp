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
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#pragma once

#include <cstdlib>

template<class Iterator>
struct iter_type {
    Iterator first;
    Iterator last;

    Iterator begin() const { return this->first; }
    Iterator end() const { return this->last; }
};

namespace aux_ {
    template<class> struct iter_pointer;
    template<class T> struct iter_pointer<T*> { using type = iter_type<T *>; };

    template<class> struct iter_array;
    template<class T, std::size_t N> struct iter_array<T[N]> { using type = iter_type<T *>; };

    template<class T, class U = void>
    struct enable_type { using type = U; };

    template<class, class = void> struct iter_cont;
    template<class T>
    struct iter_cont<T, typename enable_type<typename T::iterator>::type>
    { using type = iter_type<typename T::iterator>; };

    template<class, class = void> struct iter_const_cont;
    template<class T>
    struct iter_const_cont<T, typename enable_type<typename T::const_iterator>::type>
    { using type = iter_type<typename T::const_iterator>; };

    template<class, class = void> struct iter_iterator;
    template<class T>
    struct iter_iterator<T, typename enable_type<typename T::iterator_category>::type>
    { using type = iter_type<T>; };

    template<class, class> struct iter_array_pointer;
    template<class T, std::size_t N>
    struct iter_array_pointer<T[N], T*>
    { using type = iter_type<T *>; };
    template<class T, std::size_t N>
    struct iter_array_pointer<const T[N], T*>
    { using type = iter_type<const T *>; };
    template<class T, std::size_t N>
    struct iter_array_pointer<T[N], const T*>
    { using type = iter_type<const T *>; };
    template<class T, std::size_t N>
    struct iter_array_pointer<const T[N], const T*>
    { using type = iter_type<const T *>; };
}  // namespace aux_

/**
 * \example
 *  char s[10];
 *  //...
 *  for (char c : iter(s, 3)) {
 *     ...
 *  }
 * @{
 */
template<class T>
iter_type<T> iter(T first, T last)
{ return {first,  last}; }

template<class T, class U>
typename aux_::iter_array_pointer<T, U>::type
iter(T & first, U last)
{ return {first,  last}; }

template<class T>
typename aux_::iter_iterator<T>::type
iter(T first, std::size_t n)
{ return {first, first+n}; }

template<class T>
typename aux_::iter_pointer<T>::type
iter(T p, std::size_t n)
{ return {p, p+n}; }

template<class T, std::size_t N>
typename aux_::iter_array<T>::type
iter(T & p, std::size_t n)
{ return {&p[0], p+n}; }

template<class T>
typename aux_::iter_cont<T>::type
iter(T & cont, std::size_t n)
{ return {cont.begin(), cont.begin()+n}; }

template<class T>
typename aux_::iter_const_cont<const T>::type
iter(const T & cont, std::size_t n)
{ return {cont.begin(), cont.begin()+n}; }
//@}

