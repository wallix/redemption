/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/force_paris_timezone.hpp"

#include "headlessclient/headless_path.hpp"
#include "utils/strutils.hpp"


RED_AUTO_TEST_CASE(TestHeadlessPath)
{
    force_paris_timezone();

    HeadlessPath hpath;

    auto to_s = [](HeadlessPath::ComputedPath computed_path){
        return str_concat(computed_path.is_regular ? "regular=1 "_av : "regular=0 "_av, computed_path.path);
    };

#define TEST_COMPUTE_PATH(path, expected) \
    hpath.compile(path ""_av); \
    RED_TEST_CONTEXT(path) do { \
        RED_CHECK(to_s(hpath.compute_path(ctx)) == expected); \
    } while (0)

    auto ctx = HeadlessPath::Context{
        .counter = 12,
        .global_counter = 3456789,
        .real_time = RealTimePoint{std::chrono::seconds(1681225418)},
        .extension = ".png"_av,
        .filename = "${FILENAME}"_zv,
        .suffix = "-suffix"_av,
        .home = "HOME/"_av,
    };

    TEST_COMPUTE_PATH("", "regular=1 ${FILENAME}"_av);
    TEST_COMPUTE_PATH("prefix-", "regular=1 prefix-${FILENAME}"_av);
    TEST_COMPUTE_PATH("~", "regular=1 HOME/${FILENAME}"_av);
    TEST_COMPUTE_PATH("~/", "regular=1 HOME/${FILENAME}"_av);
    TEST_COMPUTE_PATH("~/prefix-", "regular=1 HOME/prefix-${FILENAME}"_av);
    TEST_COMPUTE_PATH("%i", "regular=0 000012${FILENAME}"_av);
    TEST_COMPUTE_PATH("a%i", "regular=0 a000012${FILENAME}"_av);
    TEST_COMPUTE_PATH("%ib", "regular=0 000012b${FILENAME}"_av);
    TEST_COMPUTE_PATH("a%ib", "regular=0 a000012b${FILENAME}"_av);
    TEST_COMPUTE_PATH("a%%ib", "regular=1 a%ib${FILENAME}"_av);
    TEST_COMPUTE_PATH("%i %I %d %h %e %s ", "regular=0 000012 3456789 2023-04-11 17:03:38 png -suffix ${FILENAME}"_av);
    // second call
    ++ctx.global_counter;
    RED_CHECK(to_s(hpath.compute_path(ctx)) == "regular=0 000012 3456790 2023-04-11 17:03:38 png -suffix ${FILENAME}"_av);
    TEST_COMPUTE_PATH("%E", "regular=1 ${FILENAME}.png"_av);
    TEST_COMPUTE_PATH("%Edir/", "regular=1 dir/${FILENAME}.png"_av);

    ctx.filename = "/a_file"_zv;
    TEST_COMPUTE_PATH(/* ctx.filename = "/a_file" */ "prefix-", "regular=1 /a_file"_av);

#undef TEST_COMPUTE_PATH
}
