/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean
*/

#define REDEMPTION_UNIT_TEST_CPP 1
#include "system/redemption_unit_tests.hpp"

#include <vector>
#include <iostream>

#include <cstdio>


namespace redemption_unit_test__
{
    namespace
    {
        struct item
        {
            const char* name;
            void (*fn)();
        };

        struct UnitTest
        {
            std::vector<item> tests;
            int failure = 0;
            const char * current_name;
            const char * current_module_name;
        };

        // abort static initialization order fiasco
        UnitTest& TESTS() noexcept
        {
            static UnitTest r;
            return r;
        }
    }

    void failure() noexcept
    {
        ++TESTS().failure;
    }

    char const* current_name() noexcept
    {
        return TESTS().current_name;
    }

    std::ostream& get_output() noexcept
    {
        return std::cerr;
    }

    void add_test_case(char const* name, void(*fn)())
    {
        TESTS().tests.push_back({name, fn});
    }

    int execute_tests(char const* module_name)
    {
        get_output() << "Running " << TESTS().tests.size() << " test cases...\n";

        TESTS().current_module_name = module_name;

        for (auto& item: TESTS().tests){
            TESTS().current_name = item.name;
            item.fn();
        }

        if (TESTS().failure == 0) {
            get_output() << "\n*** No errors detected\n";
            return 0;
        }
        else {
            get_output()
              << "\n***" << TESTS().failure
              << " failure is detected in test module \"" << module_name << "\""
            ;
            return 1;
        }
    }
}
