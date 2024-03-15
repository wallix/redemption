/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#ifdef REDEMPTION_NO_FFMPEG
#  undef REDEMPTION_NO_FFMPEG
#endif

#define REDEMPTION_NO_FFMPEG 1
#include "capture/capture.hpp"
