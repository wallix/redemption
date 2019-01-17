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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/date_dir_from_filename.hpp"

RED_AUTO_TEST_CASE(TestDateDirFromFilename)
{
    {
        auto s = cstr_array_view("abc");
        DateDirFromFilename d(s);
        RED_CHECK(!d.has_date());
        RED_CHECK_SMEM(s, d.full_path());
        RED_CHECK_SMEM(s, d.filename());
        RED_CHECK_SMEM(array_view_char{}, d.base_path());
        RED_CHECK_SMEM(array_view_char{}, d.directory());
        RED_CHECK_SMEM(array_view_char{}, d.date_path());
    }
    {
        auto s = cstr_array_view("/var/recorder/SESSIONID-0000,rec@127.0.0.1,Tester@PROXY@linux,2018041-104218,jpoelen-mini,5560.log");
        auto base = cstr_array_view("/var/recorder/");
        DateDirFromFilename d(s);
        RED_CHECK(!d.has_date());
        RED_CHECK_SMEM(s, d.full_path());
        RED_CHECK_SMEM(base, d.base_path());
        RED_CHECK_SMEM(base, d.directory());
        RED_CHECK_SMEM(s.array_from_offset(base.size()), d.filename());
        RED_CHECK_SMEM(array_view_char{}, d.date_path());
    }
    {
        auto s = cstr_array_view("/SESSIONID-0000,rec@127.0.0.1,Tester@PROXY@linux,2018041-104218,jpoelen-mini,5560.log");
        auto base = cstr_array_view("/");
        DateDirFromFilename d(s);
        RED_CHECK(!d.has_date());
        RED_CHECK_SMEM(s, d.full_path());
        RED_CHECK_SMEM(base, d.base_path());
        RED_CHECK_SMEM(base, d.directory());
        RED_CHECK_SMEM(s.array_from_offset(base.size()), d.filename());
        RED_CHECK_SMEM(array_view_char{}, d.date_path());
    }
    {
        auto s = cstr_array_view("2018-04-01/SESSIONID-0000,rec@127.0.0.1,Tester@PROXY@linux,2018041-104218,jpoelen-mini,5560.log");
        auto date = cstr_array_view("2018-04-01/");
        DateDirFromFilename d(s);
        RED_CHECK(d.has_date());
        RED_CHECK_SMEM(s, d.full_path());
        RED_CHECK_SMEM(array_view_char{}, d.base_path());
        RED_CHECK_SMEM(s.array_from_offset(date.size()), d.filename());
        RED_CHECK_SMEM(date, d.date_path());
        RED_CHECK_SMEM(date, d.directory());
    }
    {
        auto s = cstr_array_view("var/recorder/2018-04-01/SESSIONID-0000,rec@127.0.0.1,Tester@PROXY@linux,2018041-104218,jpoelen-mini,5560.log");
        auto base = cstr_array_view("var/recorder/");
        auto date = cstr_array_view("2018-04-01/");
        auto dir = s.subarray(0, base.size()+date.size());
        DateDirFromFilename d(s);
        RED_CHECK(d.has_date());
        RED_CHECK_SMEM(s, d.full_path());
        RED_CHECK_SMEM(base, d.base_path());
        RED_CHECK_SMEM(s.array_from_offset(dir.size()), d.filename());
        RED_CHECK_SMEM(date, d.date_path());
        RED_CHECK_SMEM(dir, d.directory());
    }
}
