/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <ctime>
#include <cstdlib>

// force timezone
inline void force_paris_timezone()
{
    static bool is_initialized = false;
    if (is_initialized) {
        return;
    }
    is_initialized = true;
    setenv("TZ", "Europe/Paris", 1);
    tzset();
}
