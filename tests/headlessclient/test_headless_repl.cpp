/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "headlessclient/headless_repl.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

RED_AUTO_TEST_CASE_WF(TestHeadlessCommandBuffer, wf)
{
    HeadlessRepl::CommandBuffer cmd_buffer;
    auto buffer =
        "\n"
        "a\n"
        "abcdef\n"
        "abcdef\n"
        "abcdef\n"
        "01234567890123456789012345678901234567890123456789\n"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
        "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n"
        "cccccccccccccccccccccccccccccccccccccccccccccccccc\n"
        "dddddddddddddddddddddddddddddddddddddddddddddddddd\n"
        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n"
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n"
        "gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg\n"
        "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh\n"
        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\n"
        "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj\n"
        "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk\n"
        "llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll\n"
        "XYZ\n"
        ""_av
    ;

    FILE* f = fopen(wf, "w");
    RED_REQUIRE(buffer.size() == fwrite(buffer.data(), 1, buffer.size(), f));
    fclose(f);

    int fd = open(wf, O_RDONLY);
    RED_REQUIRE(fd != -1);

    using Type = HeadlessRepl::CommandBuffer::ResultType;

#define check_line(rtype, rcmd) do {   \
    auto r = cmd_buffer.read_line(fd); \
    RED_CHECK(r.type == rtype);        \
    RED_CHECK(r.cmd == rcmd);          \
} while (0)

    check_line(Type::Extracted, ""_av);
    check_line(Type::Extracted, "a"_av);
    check_line(Type::Extracted, "abcdef"_av);
    check_line(Type::Extracted, "abcdef"_av);
    check_line(Type::Extracted, "abcdef"_av);
    check_line(Type::Extracted, "01234567890123456789012345678901234567890123456789"_av);
    check_line(Type::Extracted, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"_av);
    check_line(Type::Extracted, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"_av);
    check_line(Type::Extracted, "cccccccccccccccccccccccccccccccccccccccccccccccccc"_av);
    check_line(Type::Extracted, "dddddddddddddddddddddddddddddddddddddddddddddddddd"_av);
    check_line(Type::Extracted, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"_av);
    check_line(Type::Extracted,
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"_av);
    check_line(Type::Extracted,
        "gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg"_av);
    check_line(Type::Extracted,
        "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"_av);
    check_line(Type::Extracted,
        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"_av);
    check_line(Type::Extracted,
        "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj"_av);
    check_line(Type::Extracted,
        "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk"_av);
    check_line(Type::Extracted,
        "llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"_av);
    check_line(Type::Extracted, "XYZ"_av);
    check_line(Type::Eof, ""_av);

#undef check_line

    close(fd);
    wf.set_removed();
}
