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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/file_permissions.hpp"

RED_TEST_DELEGATE_PRINT(::FilePermissions, _.permissions_as_uint());


RED_AUTO_TEST_CASE(TestFilePermissions)
{
    RED_CHECK(FilePermissions(0) == FilePermissions(0));
    RED_CHECK(FilePermissions(0777) != FilePermissions(0));
    RED_CHECK(FilePermissions::user_permissions(BitPermissions::rw) == FilePermissions(0600));
    RED_CHECK(FilePermissions::group_permissions(BitPermissions::rw) == FilePermissions(0060));
    RED_CHECK(FilePermissions::other_permissions(BitPermissions::rw) == FilePermissions(0006));
    RED_CHECK(FilePermissions::user_permissions(BitPermissions::read) == FilePermissions(0400));
    RED_CHECK(FilePermissions::user_permissions(BitPermissions::write) == FilePermissions(0200));
    RED_CHECK(FilePermissions::user_permissions(BitPermissions::execute) == FilePermissions(0100));
    RED_CHECK(FilePermissions::user_permissions(BitPermissions::all) == FilePermissions(0700));
    RED_CHECK(FilePermissions::user_and_group_permissions(BitPermissions::read) == FilePermissions(0440));
    RED_CHECK(FilePermissions::user_permissions(BitPermissions::read | BitPermissions::write) == FilePermissions(0600));
}
