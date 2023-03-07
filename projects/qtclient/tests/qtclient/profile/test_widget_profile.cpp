/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

// #include "test_only/test_framework/redemption_unit_tests.hpp"

#include "qtclient/profile/widget_profile.hpp"
#include "qtclient/profile/profile.hpp"
#include <QtWidgets/QApplication>
#include <QtWidgets/qwidget.h>

int main(int ac, char** av)
// RED_AUTO_TEST_CASE(TestWidgetProfile)
{
    QApplication app(ac, av);
    qtclient::Profile profile;
    qtclient::create_widget_profile(profile, nullptr)->show();
    app.exec();
    // qtclient::Profile profile;
    // qtclient::create_widget_profile(profile);
}
