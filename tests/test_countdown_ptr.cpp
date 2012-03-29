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

    typedef CountdownPtr<A*> Countdown;
    allocator alloc(oss);

    BOOST_CHECK(1);
    {
        Countdown countdown_data(alloc());
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "1:~A"); oss.str("");

    {
        Countdown countdown_data(alloc());
        {
            Countdown countdown_data2(countdown_data);
        }
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "2:~A"); oss.str("");

    {
        Countdown countdown_data(alloc());
        {
            Countdown countdown_data2(countdown_data);
            Countdown countdown_data3(countdown_data);
            Countdown countdown_data4(countdown_data3);
        }
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "3:~A"); oss.str("");

    {
        Countdown countdown_data(alloc());
        {
            Countdown countdown_data2(countdown_data);
            Countdown countdown_data3(countdown_data);
            Countdown countdown_data4(countdown_data3);
        }
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "4:~A"); oss.str("");


    {
        Countdown* countdown_data = new Countdown(alloc());
        Countdown* countdown_data2 = new Countdown(*countdown_data);
        delete countdown_data;
        BOOST_CHECK_EQUAL(oss.str(), "");
        delete countdown_data2;
        BOOST_CHECK_EQUAL(oss.str(), "5:~A"); oss.str("");
    }
}

BOOST_AUTO_TEST_CASE(CountdownPtrTestArray)
{
    std::ostringstream oss;

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

    typedef CountdownPtr<A[]> Countdown;
    allocator alloc(oss);

    BOOST_CHECK(1);
    {
        Countdown countdown_data(alloc());
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "1:~A2:~A"); oss.str("");

    {
        Countdown countdown_data(alloc());
        {
            Countdown countdown_data2(countdown_data);
        }
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "3:~A4:~A"); oss.str("");

    {
        Countdown countdown_data(alloc());
        {
            Countdown countdown_data2(countdown_data);
            Countdown countdown_data3(countdown_data);
            Countdown countdown_data4(countdown_data3);
        }
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "5:~A6:~A"); oss.str("");

    {
        Countdown countdown_data(alloc());
        {
            Countdown countdown_data2(countdown_data);
            Countdown countdown_data3(countdown_data);
            Countdown countdown_data4(countdown_data3);
        }
        BOOST_CHECK_EQUAL(oss.str(), "");
    }
    BOOST_CHECK_EQUAL(oss.str(), "7:~A8:~A"); oss.str("");


    {
        Countdown* countdown_data = new Countdown(alloc());
        Countdown* countdown_data2 = new Countdown(*countdown_data);
        delete countdown_data;
        BOOST_CHECK_EQUAL(oss.str(), "");
        delete countdown_data2;
        BOOST_CHECK_EQUAL(oss.str(), "9:~A10:~A"); oss.str("");
    }
}