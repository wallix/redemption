/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "keyboard/kbdtypes.hpp"
#include "utils/basic_notifier_function.hpp"
#include "utils/monotonic_clock.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/ref.hpp"
#include "core/callback.hpp"

#include <string>


struct HeadlessInputCommandGenerator
{
    enum Status : uint8_t
    {
        UpdateLastLine,
        NewLine,
    };

    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;
    using KeyLocks = kbdtypes::KeyLocks;

    using Notifier = BasicNotifierFunction<Status, chars_view, std::size_t /*updated_column*/>;

    struct DelayConfig
    {
        std::chrono::milliseconds delay;
        std::chrono::milliseconds threshold;
    };

    HeadlessInputCommandGenerator(Notifier notifier)
    : notifier(notifier)
    {}

    void set_key_delay(DelayConfig key_delay);

    void set_kbd_fr(bool enable = true);

    void start(MonotonicTimePoint now);

    void scancode(MonotonicTimePoint now, KbdFlags flags, Scancode scancode);

    void unicode(MonotonicTimePoint now, KbdFlags flag, uint16_t unicode);

    void mouse(MonotonicTimePoint now, uint16_t device_flags, uint16_t x, uint16_t y);

    void synchronize(MonotonicTimePoint now, KeyLocks locks);

private:
    class Private;
    friend class Private;

    enum class CmdType : uint8_t;

    union PreviousValues
    {
        struct Sc
        {
            KbdFlags flags;
            Scancode scancode;
            uint16_t repetition;
            std::size_t previous_len;
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

    using Ms = std::chrono::milliseconds;

    MonotonicTimePoint previous_time;
    MonotonicTimePoint::duration max_keydelay{std::chrono::milliseconds(250)};
    Notifier notifier;
    std::string cmd;
    // uint16_t mouse_x = 0;
    // uint16_t mouse_y = 0;
    // compressed stack: packed to 5 bits * 5
    uint32_t mods = 0;
    CmdType cmd_type = CmdType();
    bool is_en_kbd = true;
    PreviousValues previous_values;
};


struct DispatchRdpInputCommandGenerator : RdpInput
{
    DispatchRdpInputCommandGenerator(CRef<MonotonicTimePoint> monotonic_time, RdpInput& rdp_input, HeadlessInputCommandGenerator::Notifier notifier)
    : command_generator(notifier)
    , monotonic_time(monotonic_time)
    , rdp_input(rdp_input)
    {}

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override
    {
        rdp_input.rdp_input_scancode(flags, scancode, event_time, keymap);
        command_generator.scancode(monotonic_time, flags, scancode);
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        rdp_input.rdp_input_unicode(flag, unicode);
        command_generator.unicode(monotonic_time, flag, unicode);
    }

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        rdp_input.rdp_input_mouse(device_flags, x, y);
        command_generator.mouse(monotonic_time, device_flags, x, y);
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        (void)locks;
    }

    void rdp_input_invalidate(Rect r) override
    {
        (void)r;
    }

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}

    HeadlessInputCommandGenerator command_generator;

private:
    MonotonicTimePoint const& monotonic_time;
    RdpInput& rdp_input;
};
