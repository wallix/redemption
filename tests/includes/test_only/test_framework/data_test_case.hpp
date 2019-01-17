/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#ifdef IN_IDE_PARSER

# define RED_DATA_TEST_CASE(test_name, dataset, varname)                        \
    inline auto const& test_name##_type_element_() {                            \
        using std::begin; return *begin(dataset);                               \
    }                                                                           \
    class test_name {                                                           \
        public: void operator()(decltype(test_name##_type_element_()) varname); \
    };                                                                          \
    void test_name::operator()(decltype(test_name##_type_element_()) varname)

#else
# include "impl/data_test_case_impl.hpp"
#endif
