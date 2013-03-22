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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to string.hpp file
   Using lib boost functions, some tests need to be added

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestContextAsMap
#include <boost/test/auto_unit_test.hpp>

#include"log.hpp"
#define LOGNULL

#include <string.h>
#include <map>

BOOST_AUTO_TEST_CASE(TestStringStringMap)
{
    std::map<const char *, const char *> context;

    context["key1"] = "value1";
    context["key2"] = "value2";

    // We can get the value associated with a key using the syntax below
    BOOST_CHECK_EQUAL("value1", context["key1"]);
    BOOST_CHECK_EQUAL("value2", context["key2"]);

    // beware, accessing to a value create it if it does not exists and return
    // the value returned by the default constructor of the value class
    // for a const char * it's a NULL pointer
    BOOST_CHECK(!context["key3"]);

    // The only way to check if an element exist of check if it exists without
    // creating it is to use the find function. It will return an iterator to
    // the element and this iterator will be map::end() if the item is not found.
    std::map<const char *, const char *>::iterator it2 = context.find("key2");

    // can't use boost check equal as there is not default output function
    // working with iterators.
    BOOST_CHECK(context.end() == context.find("key4"));

    const char * value = it2->first;
    BOOST_CHECK(strcmp("value2", value));

}
