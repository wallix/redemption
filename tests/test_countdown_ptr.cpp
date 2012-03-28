/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Jonathan Poelen
 *
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCountdownPtr
#include <boost/test/auto_unit_test.hpp>
#include <boost/current_function.hpp>

#include <sstream>
#include "countdown_ptr.hpp"

struct A {
    unsigned i;
    std::ostringstream* oss;

    A(unsigned i, std::ostringstream& oss)
    : i (i)
    , oss(&oss)
    {}
    A()
    : i (0)
    , oss(0)
    {}
    ~A()
    {
        *oss << i << ":~A";
    }
};

BOOST_AUTO_TEST_CASE(CountdownPtrTestPtr)
{
    std::ostringstream oss;
    {
        struct allocator {
            unsigned i;
            std::ostringstream& oss;
            allocator(std::ostringstream& oss, unsigned i = 0)
            : i(i)
            , oss(oss)
            {}

            A* operator()()
            { return new A(++i, oss); }
        };

        allocator alloc(oss);

        A* a1 = alloc();
        A* a2 = alloc();
        A* a3 = alloc();
        A* a4 = alloc();
        A* a5 = alloc();

        BOOST_CHECK(1);
        CountdownPtr<A*> countdown_data;

        BOOST_CHECK(1);
        countdown_data.insert(a1);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a1);
        BOOST_CHECK_EQUAL(oss.str(), "1:~A"); oss.str("");
        countdown_data.insert(a2);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a2);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a2);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a3);
        BOOST_CHECK_EQUAL(oss.str(), "3:~A"); oss.str("");
        countdown_data.insert(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a2);
        BOOST_CHECK_EQUAL(oss.str(), "2:~A"); oss.str("");
        countdown_data.erase(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a4);
        BOOST_CHECK_EQUAL(oss.str(), "4:~A"); oss.str("");
        countdown_data.erase(a5);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a5);
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "5:~A"); oss.str("");
}

BOOST_AUTO_TEST_CASE(CountdownPtrTestArray)
{
    std::ostringstream oss;
    {
        struct allocator {
            unsigned i;
            std::ostringstream& oss;
            allocator(std::ostringstream& oss, unsigned i = 0)
            : i(i)
            , oss(oss)
            {}

            A* operator()()
            {
                A* r = new A[2];
                r[1].i = ++i;
                r[1].oss = &oss;
                r[0].i = ++i;
                r[0].oss = &oss;
                return r;
            }
        };

        allocator alloc(oss);

        A* a1 = alloc();
        A* a2 = alloc();
        A* a3 = alloc();
        A* a4 = alloc();
        A* a5 = alloc();

        BOOST_CHECK(1);
        CountdownPtr<A[]> countdown_data;

        BOOST_CHECK(1);
        countdown_data.insert(a1);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a1);
        BOOST_CHECK_EQUAL(oss.str(), "1:~A2:~A"); oss.str("");
        countdown_data.insert(a2);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a2);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a2);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a3);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a3);
        BOOST_CHECK_EQUAL(oss.str(), "5:~A6:~A"); oss.str("");
        countdown_data.insert(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a2);
        BOOST_CHECK_EQUAL(oss.str(), "3:~A4:~A"); oss.str("");
        countdown_data.erase(a4);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.erase(a4);
        BOOST_CHECK_EQUAL(oss.str(), "7:~A8:~A"); oss.str("");
        countdown_data.erase(a5);
        BOOST_CHECK_EQUAL(oss.str(), "");
        countdown_data.insert(a5);
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "9:~A10:~A");
}