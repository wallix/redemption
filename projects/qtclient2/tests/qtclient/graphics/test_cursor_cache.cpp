/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "qtclient/graphics/cursor_cache.hpp"

#include <QtGui/QGuiApplication>


RED_AUTO_TEST_CASE(TestCursor)
{
    char arg[] = "test";
    char* argv[] = {arg};
    int argc = 1;
    QGuiApplication app(argc, argv);

    using namespace qtclient;

    CursorCache cursor_cache;

    RED_CHECK(!cursor_cache.set_rdp_pointer(1111, slashed_circle_pointer()));
    RED_CHECK(cursor_cache.set_rdp_pointer(1, slashed_circle_pointer()));
    RED_CHECK(!cursor_cache.get_cursor(1111));
    RED_CHECK(cursor_cache.get_cursor(1));

    QCursor shaped_cursor;
    RED_CHECK(set_predefined_pointer(shaped_cursor, PredefinedPointer::SlashedCircle));
    RED_CHECK(!set_predefined_pointer(shaped_cursor, PredefinedPointer(111)));
}
