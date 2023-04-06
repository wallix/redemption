/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "headlessclient/headless_command_generator.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/strutils.hpp"

using namespace std::chrono_literals;

struct HeadlessCommandGeneratorTextCtx
{
    using Status = HeadlessCommandGenerator::Status;
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    using KeyLocks = kbdtypes::KeyLocks;

    std::string input;
    HeadlessCommandGenerator cmd;

    MonotonicTimePoint now{};

    std::string ctx;

    HeadlessCommandGeneratorTextCtx()
    : cmd([this](Status status, chars_view str, std::size_t updated_column){
        auto st = (status == Status::NewLine) ? "NewLine " : "UpdateLastLine ";
        str_append(input, st, int_to_decimal_chars(updated_column), ' ', str, '\n');
    })
    {}

    MonotonicTimePoint next_time(std::chrono::milliseconds delay = 1ms)
    {
        now += delay;
        return now;
    }
};

#define CHECK_INPUT(cmd, s) do {            \
    ctx += &"\n    " #cmd[ctx.empty() * 5]; \
    RED_TEST_CONTEXT(ctx) {                 \
        RED_CHECK((cmd, input) == s); \
        input.clear();                      \
    }                                       \
} while(0)

RED_FIXTURE_TEST_CASE(TestGenerateInputScancodeEvent, HeadlessCommandGeneratorTextCtx)
{
    // normal sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 4 key a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::Space),
        "UpdateLastLine 5 key a{Space down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::Space),
        "UpdateLastLine 5 key a \n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::F1),
        "UpdateLastLine 6 key a {F1 down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::F1),
        "UpdateLastLine 6 key a {F1}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode(0xFF)),
        "UpdateLastLine 10 key a {F1}{0xFF down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode(0xFF)),
        "UpdateLastLine 10 key a {F1}{0xFF}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Extended, Scancode(0xFF)),
        "UpdateLastLine 16 key a {F1}{0xFF}{0x1FF down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release | KbdFlags::Extended, Scancode(0xFF)),
        "UpdateLastLine 16 key a {F1}{0xFF}{0x1FF}\n"_av);

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

    ctx.clear();
    // Long delay
    CHECK_INPUT(cmd.set_key_delay({80ms, 250ms}),
        "NewLine 0 keydelay 80ms\n"_av);
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::NoFlags, Scancode::A),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 4 key a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(400ms), KbdFlags::NoFlags, Scancode::A),
        "NewLine 0 sleep 400ms\n"
        "NewLine 0 key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(), KbdFlags::Release, Scancode::A),
        "UpdateLastLine 4 key a\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(400ms), KbdFlags::NoFlags, Scancode::A),
        "NewLine 0 sleep 400ms\n"
        "NewLine 0 key {a down}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(2s), KbdFlags::Release, Scancode::A),
        "NewLine 0 sleep 2s\n"
        "NewLine 0 key {a up}\n"_av);
    CHECK_INPUT(cmd.scancode(next_time(1200ms), KbdFlags::Release, Scancode::A),
        "NewLine 0 sleep 1s 200ms\n"
        "NewLine 0 key {a up}\n"_av);

}


RED_FIXTURE_TEST_CASE(TestGenerateInputUnicodeEvent, HeadlessCommandGeneratorTextCtx)
{
    // click sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::NoFlags, 'a'),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 text a\n"_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::Release, 'a'), ""_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::NoFlags, 'b'), "UpdateLastLine 6 text ab\n"_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::Release, 'b'), ""_av);
    // rocket !
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::NoFlags, 0xD83D), ""_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::NoFlags, 0xDE80), "UpdateLastLine 7 text ab🚀\n"_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::Release, 0xD83D), ""_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::Release, 0xDE80), ""_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::NoFlags, 'd'), "UpdateLastLine 11 text ab🚀d\n"_av);
    CHECK_INPUT(cmd.unicode(next_time(), KbdFlags::Release, 'd'), ""_av);
}


RED_FIXTURE_TEST_CASE(TestGenerateInputKeyLocksEvent, HeadlessCommandGeneratorTextCtx)
{
    // click sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.keylocks(next_time(), KeyLocks::NoLocks),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 lock None\n"_av);
    CHECK_INPUT(cmd.keylocks(next_time(), KeyLocks::NumLock),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 lock Num\n"_av);
    CHECK_INPUT(cmd.keylocks(next_time(), KeyLocks::NumLock | KeyLocks::CapsLock),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 lock Caps Num\n"_av);
}


RED_FIXTURE_TEST_CASE(TestGenerateInputMouseEvent, HeadlessCommandGeneratorTextCtx)
{
    // click sequence
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_MOVE, 3, 6), ""_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_MOVE, 4, 5), ""_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, 3, 6),
        "NewLine 0 sleep 3ms\n"
        "NewLine 0 move 4 5\n"
        "NewLine 0 mouse Left,down\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON1, 3, 6),
        "UpdateLastLine 10 mouse Left\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_DOWN, 3, 6),
        "UpdateLastLine 10 mouse Left b4,down\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON4, 3, 6),
        "UpdateLastLine 13 mouse Left b4\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON2 | MOUSE_FLAG_DOWN, 3, 6),
        "UpdateLastLine 13 mouse Left b4 Right,down\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON2, 3, 6),
        "UpdateLastLine 19 mouse Left b4 Right\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_MOVE, 4, 1), ""_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON2 | MOUSE_FLAG_DOWN, 4, 1),
        "NewLine 0 move +0 -4\n"
        "NewLine 0 mouse Right,down\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, 3, 6),
        "UpdateLastLine 16 mouse Right,down Left,down\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON2, 4, 1),
        "UpdateLastLine 26 mouse Right,down Left,down Right,up\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_BUTTON1, 4, 1),
        "UpdateLastLine 35 mouse Right,down Left,down Right,up Left,up\n"_av);
    // unknown click
    CHECK_INPUT(cmd.mouse(next_time(), 0xBC, 4, 1),
        "UpdateLastLine 43 mouse Right,down Left,down Right,up Left,up 0xBC\n"_av);

    /*
     * scroll
     */

    ctx.clear();
    // scroll
    CHECK_INPUT(cmd.start(now), ""_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_WHEEL, 3, 6),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 scroll 1\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_WHEEL, 3, 6),
        "UpdateLastLine 7 scroll 2\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_WHEEL | MOUSE_FLAG_WHEEL_NEGATIVE, 3, 6),
        "UpdateLastLine 7 scroll 2 -1\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_WHEEL | MOUSE_FLAG_WHEEL_NEGATIVE, 3, 6),
        "UpdateLastLine 9 scroll 2 -2\n"_av);
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_WHEEL, 3, 6),
        "UpdateLastLine 9 scroll 2 -2 1\n"_av);

    ctx.clear();
    // hscroll
    CHECK_INPUT(cmd.mouse(next_time(), MOUSE_FLAG_HWHEEL, 3, 6),
        "NewLine 0 sleep 1ms\n"
        "NewLine 0 hscroll 1\n"_av);

    /*
     * move
     */

    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_MOVE, 5, 6), ""_av);
    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_BUTTON1, 0, 0),
        "NewLine 0 move +1 +5\n"
        "NewLine 0 mouse Left,up\n"_av);

    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_MOVE, 3, 2), ""_av);
    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_BUTTON1, 0, 0),
        "NewLine 0 move -2 -4\n"
        "NewLine 0 mouse Left,up\n"_av);

    cmd.set_mouse_position_type(HeadlessCommandGenerator::MousePositionType::Absolute);

    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_MOVE, 1, 6), ""_av);
    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_BUTTON1, 0, 0),
        "NewLine 0 move 1 6\n"
        "NewLine 0 mouse Left,up\n"_av);

    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_MOVE, 0, 2), ""_av);
    CHECK_INPUT(cmd.mouse(now, MOUSE_FLAG_BUTTON1, 0, 0),
        "NewLine 0 move 0 2\n"
        "NewLine 0 mouse Left,up\n"_av);

}

#undef CHECK_INPUT
