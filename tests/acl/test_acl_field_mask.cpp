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
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/acl_field_mask.hpp"

RED_AUTO_TEST_CASE(TestAclFieldMask)
{
    const auto id1 = configs::authid_t(3);
    const auto id2 = configs::authid_t(7);
    static_assert(id1 < configs::max_authid && id2 < configs::max_authid);

    AclFieldMask acl_fields;
    RED_TEST(acl_fields.is_empty());

    acl_fields.set(id1);
    RED_TEST(!acl_fields.is_empty());
    RED_TEST(acl_fields.has(id1));
    RED_TEST(!acl_fields.has(id2));

    acl_fields.set(id2);
    RED_TEST(!acl_fields.is_empty());
    RED_TEST(acl_fields.has(id1));
    RED_TEST(acl_fields.has(id2));

    auto acl_fields2 = acl_fields;
    RED_TEST(!acl_fields.is_empty());
    RED_TEST(acl_fields2.has(id1));
    RED_TEST(acl_fields2.has(id2));

    acl_fields.clear();
    RED_TEST(acl_fields.is_empty());
    acl_fields = acl_fields2;
    RED_TEST(!acl_fields.is_empty());

    acl_fields.clear(id2);
    RED_TEST(!acl_fields.is_empty());
    RED_TEST(acl_fields.has(id1));
    RED_TEST(!acl_fields.has(id2));

    acl_fields2.clear(acl_fields);
    RED_TEST(!acl_fields2.is_empty());
    RED_TEST(!acl_fields2.has(id1));
    RED_TEST(acl_fields2.has(id2));
}
