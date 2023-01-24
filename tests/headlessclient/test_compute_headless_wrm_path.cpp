/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/force_paris_timezone.hpp"
#include "headlessclient/compute_headless_wrm_path.hpp"


RED_AUTO_TEST_CASE(TestHeadlessWrmPath)
{
    force_paris_timezone();

    auto t = RealTimePoint(std::chrono::seconds(1677260276));
    auto sid = "sid"_av;

    RED_CHECK("sid.wrm"_av == compute_headless_wrm_path("", sid, t));

    RED_CHECK("file.wrm"_av == compute_headless_wrm_path("file.wrm", sid, t));
    RED_CHECK("d/file.wrm"_av == compute_headless_wrm_path("d/file.wrm", sid, t));
    RED_CHECK("d/d/file.wrm"_av == compute_headless_wrm_path("d/d/file.wrm", sid, t));

    RED_CHECK("d/sid.wrm"_av == compute_headless_wrm_path("d/", sid, t));

    RED_CHECK("2023-02-24_18:37:56.wrm"_av == compute_headless_wrm_path("", {}, t));
    RED_CHECK("d/2023-02-24_18:37:56.wrm"_av == compute_headless_wrm_path("d/", {}, t));
}
