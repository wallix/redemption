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


#pragma once

#include <iostream>
#include <boost/timer.hpp>

class display_timer
{
    boost::timer timer;
public:
    display_timer()
    {}

    ~display_timer()
    {
        const double elapsed = this->timer.elapsed();
        std::ios::fmtflags old_flags = std::cout.setf(std::istream::fixed, std::istream::floatfield);
        std::streamsize old_prec = std::cout.precision(2);
        std::cout << elapsed << " s" << std::endl;
        std::cout.flags(old_flags);
        std::cout.precision(old_prec);
    }
};

template<typename Test>
class basic_benchmark
{
    const char * name_test;
    Test test;

public:
    explicit basic_benchmark(const char * name, const Test & t = Test())
    : name_test(name)
    , test(t)
    {}

    bool operator()(unsigned n, const char * s) const
    {
        std::cout << this->name_test << ":\t";
        if (!this->test.check_pre_condition(s)) {
            std::cout << "none\n";
            return false;
        }

        display_timer timer;
        for (unsigned i = 0; i < n; ++i) {
            this->test.exec(s);
        }

        return true;
    }
};

template<typename Test>
bool test(const basic_benchmark<Test> & bench, unsigned n, const char * s)
{
    return bench(n, s);
}

struct basic_test
{
    bool check_pre_condition(const char *) const
    { return true; }

    void exec(const char *) const
    {}
};

