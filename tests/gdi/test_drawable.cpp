/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGDIDrawable
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "gdi/drawable.hpp"
#include "gdi/dummy_gd.hpp"

#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"

template<class Fn>
struct TestGD : gdi::DummyGD
{
    Fn fn;
    bool is_called = false;

    TestGD(Fn fn)
    : fn(fn)
    {}

    TestGD(TestGD && other)
    : fn(other.fn)
    {}

    using gdi::DummyGD::draw;
    void draw(RDPOpaqueRect const & cmd, Rect const & rect) override {
        this->is_called = true;
        this->fn(cmd, rect);
    }
};

template<class Fn>
TestGD<Fn> gd_class(Fn fn) {
    return {fn};
}


BOOST_AUTO_TEST_CASE(TestGDIDrawable)
{
    gdi::Drawable drawable;

    using gd_ptr = gdi::GraphicApiPtr;

    Rect clip{0, 0, 100, 100};
    RDPOpaqueRect cmd{clip, 0};
    auto dummy_fn = [](RDPOpaqueRect const &, Rect const &) {};

    drawable.draw(cmd, clip);

    auto gd1 = gd_class(dummy_fn);
    auto gd1_id = drawable.add_gdi(gd_ptr(&gd1, false));
    BOOST_REQUIRE(gd1_id);

    drawable.draw(cmd, clip);

    BOOST_CHECK(gd1.is_called);

    gd1.is_called = false;

    auto gd2 = gd_class(dummy_fn);
    auto gd2_id = drawable.add_gdi(gd_ptr(&gd2, false));
    BOOST_REQUIRE(gd2_id);

    drawable.draw(cmd, clip);

    BOOST_CHECK(gd1.is_called);
    BOOST_CHECK(gd2.is_called);

    gd1.is_called = false;
    gd2.is_called = false;

    auto gd3 = gd_class(dummy_fn);
    auto gd3_id = drawable.add_gdi(gd_ptr(&gd3, [](gdi::GraphicApi*) {}));
    BOOST_REQUIRE(gd3_id);

    drawable.draw(cmd, clip);

    BOOST_CHECK(gd1.is_called);
    BOOST_CHECK(gd2.is_called);
    BOOST_CHECK(gd3.is_called);

    gd1.is_called = false;
    gd2.is_called = false;
    gd3.is_called = false;

    struct FilterRect : gdi::Drawable::NullFilter {
        bool is_called = false;

        void operator()(gdi::GraphicApi &, RDPOpaqueRect const &, Rect const &) {
            this->is_called = true;
        }

        using gdi::Drawable::NullFilter::operator();
    } filter_rect;

    auto filter_id = drawable.add_filter(std::ref(filter_rect));
    BOOST_REQUIRE(filter_id);

    drawable.draw(cmd, clip);

    BOOST_CHECK(!gd1.is_called);
    BOOST_CHECK(!gd2.is_called);
    BOOST_CHECK(!gd3.is_called);
    BOOST_CHECK(filter_rect.is_called);

    filter_rect.is_called = false;

    {
        auto detached = drawable.remove_filter(filter_id);
        BOOST_REQUIRE(detached);
    }
    drawable.draw(cmd, clip);

    BOOST_CHECK(gd1.is_called);
    BOOST_CHECK(gd2.is_called);
    BOOST_CHECK(gd3.is_called);
    BOOST_CHECK(!filter_rect.is_called);

    gd1.is_called = false;
    gd2.is_called = false;
    gd3.is_called = false;

    {
        auto detached = drawable.remove_gdi(gd3_id);
        BOOST_REQUIRE(detached);
    }
    drawable.draw(cmd, clip);

    BOOST_CHECK(gd1.is_called);
    BOOST_CHECK(gd2.is_called);
    BOOST_CHECK(!gd3.is_called);

    gd1.is_called = false;
    gd2.is_called = false;

    {
        auto detached = drawable.remove_gdi(gd1_id);
        BOOST_REQUIRE(detached);
    }
    drawable.draw(cmd, clip);

    BOOST_CHECK(!gd1.is_called);
    BOOST_CHECK(gd2.is_called);

    gd2.is_called = false;

    {
        auto detached = drawable.remove_gdi(gd2_id);
        BOOST_REQUIRE(detached);
    }
    drawable.draw(cmd, clip);

    BOOST_CHECK(!gd2.is_called);
}
