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
   Author(s): Christophe Grosjean

   Unit test to dictionnary class used by Wallix Authentication Protocol

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDico
#include <boost/test/auto_unit_test.hpp>

#include "dico.hpp"

static ProtocolKeyword KeywordsDefinitions[] = {
    {"un", TYPE_TEXT, "!uno"},
    {"deux", TYPE_TEXT, "!dos"},
    {"trois", TYPE_TEXT, "!tres"}
};


BOOST_AUTO_TEST_CASE(TestDicoBasicAccess)
{
    // test we can create a dico
    Dico test_dico(KeywordsDefinitions, 3);
    BOOST_CHECK_EQUAL(true, test_dico.get("un") != NULL);
    BOOST_CHECK_EQUAL(true, test_dico.cmp("un", "uno"));
    BOOST_CHECK_EQUAL(true, test_dico.get("deux") != NULL);
    BOOST_CHECK_EQUAL(true, test_dico.cmp("deux", "dos"));
    BOOST_CHECK_EQUAL(true, test_dico.get("trois") != NULL);
    BOOST_CHECK_EQUAL(true, test_dico.cmp("trois", "tres"));
    BOOST_CHECK_EQUAL((char*)NULL, test_dico.get("quatre"));

    // replace the value at key "trois" (was "tres") with "three"
    test_dico.cpy("trois", "three");
    BOOST_CHECK_EQUAL(false, test_dico.cmp("trois", "tres"));
    BOOST_CHECK_EQUAL(true, test_dico.cmp("trois", "three"));

    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", "");
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", "something");
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", true);
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", false);
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", 1);
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", 0);
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", "1");
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", "0");
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", "on");
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", "off");
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", "yes");
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", "no");
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.cpy("un", "true");
    BOOST_CHECK_EQUAL(true, test_dico.get_bool("un"));

    test_dico.cpy("un", "false");
    BOOST_CHECK_EQUAL(false, test_dico.get_bool("un"));

    test_dico.ask("un");
    BOOST_CHECK_EQUAL(true, test_dico.is_asked("un"));

    strcpy(test_dico.get("un"), "!AskXXX");
    BOOST_CHECK_EQUAL(false, test_dico.is_asked("un"));
    BOOST_CHECK_EQUAL(false, test_dico.cmp("un", "AskXXX"));
    BOOST_CHECK_EQUAL(true, test_dico.cmp("un", "!AskXXX"));

    strcpy(test_dico.get("un"), "AskXXX");
    BOOST_CHECK_EQUAL(false, test_dico.is_asked("un"));
    BOOST_CHECK_EQUAL(true, test_dico.cmp("un", "AskXXX"));

}
