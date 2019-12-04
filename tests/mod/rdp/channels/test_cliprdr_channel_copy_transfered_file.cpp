/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/channels/cliprdr_channel_copy_transfered_file.hpp"
#include "utils/sugar/algostring.hpp"


RED_AUTO_TEST_CASE(tfl_suffix_genarator)
{
    TflSuffixGenerator gen;

    RED_TEST(gen.next() == ",000001.tfl");
    RED_TEST(gen.next() == ",000002.tfl");
    RED_TEST(gen.name_at(99999) == ",099999.tfl");
    RED_TEST(gen.name_at(999999) == ",999999.tfl");
    RED_TEST(gen.name_at(1000000) == ",1000000.tfl");
}


RED_AUTO_TEST_CASE(fdx_name_generator)
{
    using namespace std::string_view_literals;

#define MY_RECORD_PATH "/my/record/path"
#define MY_HASH_PATH "/my/hash/path/"
#define MY_SID "SESSION_ID"

    FdxNameGenerator gen(MY_RECORD_PATH, MY_HASH_PATH, MY_SID);

    RED_TEST(gen.get_current_filename() == MY_SID ".fdx");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID ".fdx");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID ".fdx");

    gen.next_tfl();

    RED_TEST(gen.get_current_filename() == MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID ",000001.tfl");

    gen.next_tfl();

    RED_TEST(gen.get_current_filename() == MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID ",000002.tfl");

#undef MY_RECORD_PATH
#undef MY_HASH_PATH
#undef MY_SID
}
