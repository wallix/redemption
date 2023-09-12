/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <cstdint>


struct RdpPerformanceFlags
{
    uint32_t force_present = 0;
    uint32_t force_not_present = 0;
};
