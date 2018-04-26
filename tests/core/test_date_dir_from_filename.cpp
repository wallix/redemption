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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   Unit test for color conversion primitives

*/

#define RED_TEST_MODULE TestDateDirFromFilename
#include "system/redemption_unit_tests.hpp"

#include "core/date_dir_from_filename.hpp"

RED_AUTO_TEST_CASE(TestDateDirFromFilename)
{
    {
        auto date_from_file = DateDirFromFilename::extract_date("abc");
        RED_CHECK(date_from_file.has_error());
    }
    {
        auto date_from_file = DateDirFromFilename::extract_date("SESSIONID-0000,rec@127.0.0.1,Tester@PROXY@linux,2018041-104218,jpoelen-mini,5560.log");
        RED_CHECK(date_from_file.has_error());
    }
    {
        auto date_from_file = DateDirFromFilename::extract_date("SESSIONID-0000,rec@127.0.0.1,Tester@PROXY@linux,20180418-104218,jpoelen-mini,5560.log");
        RED_CHECK(!date_from_file.has_error());
        RED_CHECK_EQ(date_from_file.c_str(), "2018-04-18/");
    }
}
