/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "qtclient/headless_input_command_generator.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/strutils.hpp"
#include <vector>

RED_AUTO_TEST_CASE(TestCursor)
{
    using Status = HeadlessInputCommandGenerator::Status;
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    // using KeyLocks = kbdtypes::KeyLocks;

    std::string input;
    HeadlessInputCommandGenerator cmd([&input](Status status, chars_view str, std::size_t updated_column){
        auto st = (status == Status::NewLine) ? "NewLine " : "UpdateLastLine ";
        str_append(input, st, int_to_decimal_chars(updated_column), ' ', str, '\n');
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
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 4 key a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::B),
        "UpdateLastLine 5 key a{b down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::B),
        "UpdateLastLine 5 key ab\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::F1),
        "UpdateLastLine 6 key ab{F1 down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::F1),
        "UpdateLastLine 6 key ab{F1}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode(0xFF)),
        "UpdateLastLine 10 key ab{F1}{0xFF down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode(0xFF)),
        "UpdateLastLine 10 key ab{F1}{0xFF}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Extended, Scancode(0xFF)),
        "UpdateLastLine 16 key ab{F1}{0xFF}{0x1FF down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release | KbdFlags::Extended, Scancode(0xFF)),
        "UpdateLastLine 16 key ab{F1}{0xFF}{0x1FF}\n"_av);

    ctx.clear();
    // repetition sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 4 key {a down 2}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 4 key {a down 3}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 4 key {a 3}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 9 key {a 3}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 9 key {a 3}{a down 2}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::B),
        "UpdateLastLine 19 key {a 3}{a down 2}{b down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::B),
        "UpdateLastLine 19 key {a 3}{a down 2}b\n"_av);

    ctx.clear();
    // no merge mod
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 17 key {LShift down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine 25 key {LShift down}{a down}{LShift up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 36 key {LShift down}{a down}{LShift up}{a up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 42 key {LShift down}{a down}{LShift up}{a up}{a down}\n"_av);

    ctx.clear();
    // no merge mod 2
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LCtrl),
        "UpdateLastLine 17 key {LShift down}{LCtrl down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 29 key {LShift down}{LCtrl down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine 37 key {LShift down}{LCtrl down}{a down}{LShift up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LCtrl),
        "UpdateLastLine 48 key {LShift down}{LCtrl down}{a down}{LShift up}{LCtrl up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 58 key {LShift down}{LCtrl down}{a down}{LShift up}{LCtrl up}{a up}\n"_av);

    ctx.clear();
    // merge mod
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine 4 key {LShift}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "UpdateLastLine 12 key {LShift}{LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 25 key {LShift}{LShift down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 25 key {LShift}{LShift down}a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine 12 key {LShift}A\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 13 key {LShift}A{a down}\n"_av);

    ctx.clear();
    // merge mod 2
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LAlt),
        "UpdateLastLine 17 key {LShift down}{LAlt down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 28 key {LShift down}{LAlt down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 28 key {LShift down}{LAlt down}a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LAlt),
        "UpdateLastLine 17 key {LShift down}!a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine 4 key +!a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 7 key +!a{a down}\n"_av);

    ctx.clear();
    // merge mod 3
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LAlt),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {LAlt down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::LShift),
        "UpdateLastLine 15 key {LAlt down}{LShift down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 28 key {LAlt down}{LShift down}{a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 28 key {LAlt down}{LShift down}a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LShift),
        "UpdateLastLine 15 key {LAlt down}A\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::LAlt),
        "UpdateLastLine 4 key !A\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "UpdateLastLine 6 key !A{a down}\n"_av);

    ctx.clear();
    // escaped sequence
    CHECK_INPUT(cmd.set_kbd_fr(),
        "NewLine 0 kbd fr\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode(0x35)),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {! down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode(0x35)),
        "UpdateLastLine 4 key {!}\n"_av);
    CHECK_INPUT(cmd.set_kbd_fr(false),
        "NewLine 0 kbd en\n"_av);

    ctx.clear();
    // Pause key
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Extended1, Scancode(0x1D)),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {0x21D down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode(0x45)),
        "UpdateLastLine 16 key {0x21D down}{NumLock down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Extended1 | KbdFlags::Release, Scancode(0x1D)),
        "UpdateLastLine 30 key {0x21D down}{NumLock down}{0x21D up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode(0x45)),
        "UpdateLastLine 40 key {0x21D down}{NumLock down}{0x21D up}{NumLock up}\n"_av);

#undef CHECK_INPUT
}
