/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

class QWidget;

namespace qtclient
{
    class Profile;

    QWidget* create_widget_profile(Profile& profile, QWidget* parent);
}
