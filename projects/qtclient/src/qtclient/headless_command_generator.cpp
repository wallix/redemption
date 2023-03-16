/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/headless_command_generator.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/strutils.hpp"

#include <utility>


void HeadlessCommandGenerator::reset_command()
{
    cmd_type = CmdType::Default;
    cmd.clear();
}

void HeadlessCommandGenerator::scancode(KbdFlags flags, Scancode scancode)
{
    auto oldtype = std::exchange(cmd_type, CmdType::Scancode);

    auto status = Status::UpdateLastLine;

    // TODO 0xNNN for scancode to named key

    // sc,down + up => sc
    if (oldtype == CmdType::Scancode
      && previous_values.scancode.scancode == scancode
      && previous_values.scancode.flags == (flags & ~KbdFlags::Release)
      && bool(flags & KbdFlags::Release)
     ) {
        // TODO keep extended flag !
        // remove ",0x0000" (flag)
        cmd.erase(cmd.end() - 7, cmd.end());
    }
    else {
        if (oldtype != CmdType::Scancode) {
            cmd = "sc";
            status = Status::NewLine;
        }
        str_append(cmd,
            " 0x"_av, int_to_fixed_hexadecimal_upper_chars(underlying_cast(scancode)), ',',
            "0x"_av, int_to_fixed_hexadecimal_upper_chars(underlying_cast(flags))
        );
    }

    previous_values.scancode = {flags, scancode, 0};
    notifier(status, cmd);
}

void HeadlessCommandGenerator::unicode(KbdFlags flag, uint16_t unicode)
{
    // lines.emplace_back(str_concat(
    //     "uni "_av,
    //     int_to_fixed_hexadecimal_upper_chars(unicode), ',',
    //     int_to_fixed_hexadecimal_upper_chars(underlying_cast(flag))
    // ));
    // notifier(Status::NewLine, lines.back());
}

void HeadlessCommandGenerator::mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    // uint16_t button_mask = (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_BUTTON2 | MOUSE_FLAG_BUTTON3 | MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_BUTTON5);
    //
    // if (device_flags & MOUSE_FLAG_MOVE) {
    //     mouse_x = x;
    //     mouse_y = y;
    // }
    // else if (device_flags & MOUSE_FLAG_HWHEEL) {
    //     auto cmd = (device_flags & MOUSE_FLAG_WHEEL_NEGATIVE) ? "hscroll -1"_av : "hscroll"_av;
    //     lines.emplace_back(cmd.as<std::string>());
    //     notifier(Status::NewLine, lines.back());
    // }
    // else if (device_flags & MOUSE_FLAG_WHEEL) {
    //     auto cmd = (device_flags & MOUSE_FLAG_WHEEL_NEGATIVE) ? "scroll -1"_av : "scroll"_av;
    //     lines.emplace_back(cmd.as<std::string>());
    //     notifier(Status::NewLine, lines.back());
    // }
    // else if (device_flags & button_mask) {
    //     lines.emplace_back(str_concat(
    //         "m "_av,
    //         int_to_fixed_hexadecimal_upper_chars(checked_cast<uint16_t>(device_flags & button_mask)), ',',
    //         int_to_fixed_hexadecimal_upper_chars(device_flags)
    //     ));
    //     notifier(Status::NewLine, lines.back());
    // }
}

void HeadlessCommandGenerator::synchronize(KeyLocks locks)
{
    // lines.emplace_back(str_concat(
    //     "lock "_av,
    //     int_to_fixed_hexadecimal_upper_chars(underlying_cast(locks))
    // ));
    // notifier(Status::NewLine, lines.back());
}
