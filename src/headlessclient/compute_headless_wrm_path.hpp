/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <string>

#include "utils/monotonic_clock.hpp"
#include "utils/strutils.hpp"
#include "utils/real_clock.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/tm_to_chars.hpp"

inline std::string compute_headless_wrm_path(std::string filename, chars_view session_id, RealTimePoint real_time)
{
    if (filename.empty() || filename.back() == '/') {
        using date_format = dateformats::YYYY_mm_dd_HH_MM_SS;
        char buffer[date_format::output_length];

        if (session_id.empty()) {
            tm res;
            auto duration = real_time.time_since_epoch();
            time_t sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            localtime_r(&sec, &res);
            session_id = chars_view(buffer, date_format::to_chars(buffer, res, '_'));
        }

        str_append(filename, session_id, ".wrm"_av);
    }

    return filename;
}
