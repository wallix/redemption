/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/basic_notifier_function.hpp"


struct WidgetEventNotifier : BasicNotifierFunction<>
{
    using BasicNotifierFunction<>::BasicNotifierFunction;
};
