/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "keyboard/kbdtypes.hpp"
#include "utils/basic_notifier_function.hpp"
#include "utils/monotonic_clock.hpp"
#include "utils/sugar/array_view.hpp"

#include <string>


struct HeadlessCommandGenerator
{
    enum Status : uint8_t
    {
        UpdateLastLine,
        NewLine,
    };

    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    using KeyLocks = kbdtypes::KeyLocks;

    using Notifier = BasicNotifierFunction<Status, chars_view>;

    HeadlessCommandGenerator(Notifier notifier)
    : notifier(notifier)
    {}

    void reset_command();

    void scancode(KbdFlags flags, Scancode scancode);

    void unicode(KbdFlags flag, uint16_t unicode);

    void mouse(uint16_t device_flags, uint16_t x, uint16_t y);

    void synchronize(KeyLocks locks);

private:
    enum class CmdType : uint8_t
    {
        Default,
        Scancode,
        Unicode,
        Key,
        Text,
        Mouse,
        Move,
        Scroll,
        Lock,
        Kbd,
        Connect,
        Disconnect,
    };

    union PreviousValues
    {
        struct Sc
        {
            KbdFlags flags;
            Scancode scancode;
            uint16_t repetition;
        };

        struct Uni
        {
            KbdFlags flags;
            uint16_t unicode;
        };

        uint16_t mouse_flags;
        Sc scancode;
        Uni unicode;
    };

    MonotonicTimePoint monotonic_time;
    Notifier notifier;
    std::string cmd;
    uint16_t mouse_x = 0;
    uint16_t mouse_y = 0;
    CmdType cmd_type = CmdType::Default;
    PreviousValues previous_values;
};
