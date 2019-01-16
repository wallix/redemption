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

#define BOOST_TEST_NO_OLD_TOOLS

#ifdef RED_TEST_MODULE
# define BOOST_TEST_MODULE RED_TEST_MODULE
#endif

#include <boost/test/auto_unit_test.hpp>


#define RED_AUTO_TEST_CASE BOOST_AUTO_TEST_CASE

#define RED_FAIL BOOST_FAIL
#define RED_ERROR BOOST_ERROR

#define RED_TEST_DONT_PRINT_LOG_VALUE BOOST_TEST_DONT_PRINT_LOG_VALUE
#define RED_TEST_PRINT_TYPE_FUNCTION_NAME boost_test_print_type
#define RED_TEST_PRINT_TYPE_STRUCT_NAME boost::test_tools::tt_detail::print_log_value

#define RED_CHECK_NO_THROW BOOST_CHECK_NO_THROW
#define RED_CHECK_THROW BOOST_CHECK_THROW
#define RED_CHECK_EXCEPTION BOOST_CHECK_EXCEPTION
#define RED_CHECK_EQUAL BOOST_CHECK_EQUAL
#define RED_CHECK_NE BOOST_CHECK_NE
#define RED_CHECK_LT BOOST_CHECK_LT
#define RED_CHECK_LE BOOST_CHECK_LE
#define RED_CHECK_GT BOOST_CHECK_GT
#define RED_CHECK_GE BOOST_CHECK_GE
#define RED_CHECK BOOST_CHECK
#define RED_CHECK_MESSAGE BOOST_CHECK_MESSAGE
#define RED_CHECK_EQUAL_COLLECTIONS BOOST_CHECK_EQUAL_COLLECTIONS
#define RED_CHECK_PREDICATE BOOST_CHECK_PREDICATE


#define RED_REQUIRE_NO_THROW BOOST_REQUIRE_NO_THROW
#define RED_REQUIRE_THROW BOOST_REQUIRE_THROW
#define RED_REQUIRE_EXCEPTION BOOST_REQUIRE_EXCEPTION
#define RED_REQUIRE_EQUAL BOOST_REQUIRE_EQUAL
#define RED_REQUIRE_NE BOOST_REQUIRE_NE
#define RED_REQUIRE_LT BOOST_REQUIRE_LT
#define RED_REQUIRE_LE BOOST_REQUIRE_LE
#define RED_REQUIRE_GT BOOST_REQUIRE_GT
#define RED_REQUIRE_GE BOOST_REQUIRE_GE
#define RED_REQUIRE BOOST_REQUIRE
#define RED_REQUIRE_MESSAGE BOOST_REQUIRE_MESSAGE
#define RED_REQUIRE_EQUAL_COLLECTIONS BOOST_REQUIRE_EQUAL_COLLECTIONS
#define RED_REQUIRE_PREDICATE BOOST_REQUIRE_PREDICATE
