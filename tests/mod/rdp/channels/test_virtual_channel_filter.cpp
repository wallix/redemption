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

#include "mod/rdp/channels/virtual_channel_filter.hpp"

struct DummyVirtualChannelProcessor {};

class ModRDPVCFilter : public VirtualChannelFilter<DummyVirtualChannelProcessor>
{
public:
    ModRDPVCFilter() : VirtualChannelFilter<DummyVirtualChannelProcessor>(*this, *this)
    {
    }
};

class DummyVCFiler : public RemovableVirtualChannelFilter<DummyVirtualChannelProcessor> {};

RED_AUTO_TEST_CASE(TestFilterInsertBefore)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_before(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    mod_rdp_filter.insert_before(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &mod_rdp_filter);
}

RED_AUTO_TEST_CASE(TestFilterInsertBefore2)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_before(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    dummy_filter.insert_before(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);
}

RED_AUTO_TEST_CASE(TestFilterInsertAfter)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_after(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    mod_rdp_filter.insert_after(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);
}

RED_AUTO_TEST_CASE(TestFilterInsertAfter2)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_after(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    dummy_filter.insert_after(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &mod_rdp_filter);
}

RED_AUTO_TEST_CASE(TestFilterRemoveSelf)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_before(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    mod_rdp_filter.insert_before(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &mod_rdp_filter);

    dummy_filter.remove_self();

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &mod_rdp_filter);
}

RED_AUTO_TEST_CASE(TestFilterRemoveSelf2)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_before(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    mod_rdp_filter.insert_before(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &mod_rdp_filter);

    dummy_filter2.remove_self();

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);
}

RED_AUTO_TEST_CASE(TestFilterRemoveSelf3)
{
    ModRDPVCFilter mod_rdp_filter;
    DummyVCFiler   dummy_filter;

    mod_rdp_filter.insert_before(dummy_filter);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    DummyVCFiler dummy_filter2;

    mod_rdp_filter.insert_before(dummy_filter2);

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter2);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &dummy_filter2);

    RED_CHECK_EQUAL(dummy_filter2.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(dummy_filter2.get_next_filter_ptr(), &mod_rdp_filter);

    dummy_filter2.remove_self();

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &dummy_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &dummy_filter);

    RED_CHECK_EQUAL(dummy_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(dummy_filter.get_next_filter_ptr(), &mod_rdp_filter);

    dummy_filter.remove_self();

    RED_CHECK_EQUAL(mod_rdp_filter.get_previous_filter_ptr(), &mod_rdp_filter);
    RED_CHECK_EQUAL(mod_rdp_filter.get_next_filter_ptr(), &mod_rdp_filter);
}
