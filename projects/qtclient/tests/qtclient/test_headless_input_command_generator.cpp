/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "qtclient/headless_input_command_generator.hpp"
#include "utils/strutils.hpp"
#include <vector>

RED_AUTO_TEST_CASE(TestCursor)
{
    using Status = HeadlessInputCommandGenerator::Status;
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    // using KeyLocks = kbdtypes::KeyLocks;

    std::string input;
    HeadlessInputCommandGenerator cmd([&input](Status status, chars_view str){
        auto st = (status == Status::NewLine) ? "NewLine " : "UpdateLastLine ";
        str_append(input, st, str, '\n');
    });

    MonotonicTimePoint now{};
    using namespace std::chrono_literals;

    std::string ctx;
#define CHECK_INPUT(cmd, s) do {            \
    ctx += &"\n    " #cmd[ctx.empty() * 5]; \
    RED_TEST_CONTEXT(ctx) {                 \
        RED_CHECK((void(cmd), input) == s); \
        input.clear();                      \
    }                                       \
} while(0)

    auto next_time = [&now](std::chrono::milliseconds delay = 1ms) {
        now += delay;
        return now;
    };

    // normal sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "NewLine sleep 1ms\n"
        "NewLine key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::B),
        "UpdateLastLine key a{b down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::B),
        "UpdateLastLine key ab\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::F1),
        "UpdateLastLine key ab{F1 down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::F1),
        "UpdateLastLine key ab{F1}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode(0xFF)),
        "UpdateLastLine key ab{F1}{0xFF down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode(0xFF)),
        "UpdateLastLine key ab{F1}{0xFF}\n"_av);

    ctx.clear();
    // repetition sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "NewLine sleep 1ms\n"
        "NewLine key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {a down 2}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {a down 3}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key {a 3}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {a 3}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {a 3}{a down 2}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::B),
        "UpdateLastLine key {a 3}{a down 2}{b down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::B),
        "UpdateLastLine key {a 3}{a down 2}b\n"_av);

    ctx.clear();
    // no merge mod
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine sleep 1ms\n"
        "NewLine key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LShift down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine key {LShift down}{a down}{LShift up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key {LShift down}{a down}{LShift up}{a up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LShift down}{a down}{LShift up}{a up}{a down}\n"_av);

    ctx.clear();
    // no merge mod 2
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine sleep 1ms\n"
        "NewLine key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LCtrl),
        "UpdateLastLine key {LShift down}{LCtrl down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LShift down}{LCtrl down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine key {LShift down}{LCtrl down}{a down}{LShift up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LCtrl),
        "UpdateLastLine key {LShift down}{LCtrl down}{a down}{LShift up}{LCtrl up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key {LShift down}{LCtrl down}{a down}{LShift up}{LCtrl up}{a up}\n"_av);

    ctx.clear();
    // merge mod
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine sleep 1ms\n"
        "NewLine key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine key {LShift}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "UpdateLastLine key {LShift}{LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LShift}{LShift down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key {LShift}{LShift down}a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine key {LShift}A\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LShift}A{a down}\n"_av);

    ctx.clear();
    // merge mod 2
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine sleep 1ms\n"
        "NewLine key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LAlt),
        "UpdateLastLine key {LShift down}{LAlt down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LShift down}{LAlt down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key {LShift down}{LAlt down}a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LAlt),
        "UpdateLastLine key {LShift down}!a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine key +!a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key +!a{a down}\n"_av);

    ctx.clear();
    // merge mod 3
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LAlt),
        "NewLine sleep 1ms\n"
        "NewLine key {LAlt down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "UpdateLastLine key {LAlt down}{LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key {LAlt down}{LShift down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine key {LAlt down}{LShift down}a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine key {LAlt down}A\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LAlt),
        "UpdateLastLine key !A\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine key !A{a down}\n"_av);

    ctx.clear();
    // escaped sequence
    CHECK_INPUT(cmd.set_kbd_fr(),
        "NewLine kbd fr\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode(0x35)),
        "NewLine sleep 1ms\n"
        "NewLine key {! down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode(0x35)),
        "UpdateLastLine key {!}\n"_av);

#undef CHECK_INPUT
}
