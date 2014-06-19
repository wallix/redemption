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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestUniquePtr
#include <boost/test/auto_unit_test.hpp>

#include "unique_ptr.hpp"
#include <sstream>

struct A {
    int i;
    std::ostringstream * os;

    A()
    {}

    A(int n, std::ostringstream & oss)
    : i(n)
    , os(&oss)
    {}

    ~A()
    { *os << "~dtor(" << i << ")\n"; }
};

BOOST_AUTO_TEST_CASE(TestUnique)
{
    std::ostringstream oss;
    int i = 0;
    {
        unique_ptr<A const> u1(new A(++i, oss));
        unique_ptr<A const> u2(new A(++i, oss));
    }
    {
        unique_ptr<A> u1(new A(++i, oss));
        unique_ptr<A> u2(new A(++i, oss));
        u1.swap(u2);
    }
    {
        unique_ptr<A> u1(new A(++i, oss));
        u1.reset(new A(++i, oss));
    }
    {
        unique_ptr<A> u1(new A(++i, oss));
        unique_ptr<A> u2(new A(++i, oss));
        u1 = move(u2);
    }
    {
        unique_ptr<A> u1(new A(++i, oss));
        delete u1.release();
        unique_ptr<A> u2(new A(++i, oss));
        u1 = move(u2);
    }
    {
        unique_ptr<A> u1(new A(++i, oss));
        unique_ptr<A> u2(move(u1));
    }
    {
        unique_ptr<A> u1(new A(++i, oss));
        u1 = move(u1);
    }
    {
        unique_ptr<A[]> u(new A[2]);
        u[1].i = ++i;
        u[0].i = ++i;
        u[0].os = &oss;
        u[1].os = &oss;
        BOOST_CHECK_EQUAL(u.get()[0].i, u[0].i);
        BOOST_CHECK_EQUAL(u.get()[1].i, u[1].i);
    }

    BOOST_CHECK_EQUAL(oss.str(),
                      "~dtor(2)\n"
                      "~dtor(1)\n"
                      "~dtor(3)\n"
                      "~dtor(4)\n"
                      "~dtor(5)\n"
                      "~dtor(6)\n"
                      "~dtor(7)\n"
                      "~dtor(8)\n"
                      "~dtor(9)\n"
                      "~dtor(10)\n"
                      "~dtor(11)\n"
                      "~dtor(12)\n"
                      "~dtor(13)\n"
                      "~dtor(14)\n"
                     );
}
