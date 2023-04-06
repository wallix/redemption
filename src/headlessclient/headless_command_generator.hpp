/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "keyboard/kbdtypes.hpp"
#include "utils/basic_notifier_function.hpp"
#include "utils/monotonic_clock.hpp"
#include "utils/sugar/array_view.hpp"
#include "core/callback.hpp"

#include <string>


struct HeadlessCommandGenerator
{
    enum Status : uint8_t
    {
        UpdateLastLine,
        NewLine,
    };

    enum class MousePositionType : uint8_t
    {
        FirstCallIsAbsolute,
        Absolute,
        Relative,
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

    HeadlessCommandGenerator(Notifier notifier)
    : notifier(notifier)
    {}

    void reset_line()
    {
        cmd_type = {};
    }

    void set_mouse_position_type(MousePositionType mouse_position_type)
    {
        this->mouse_position_type = mouse_position_type;
    }

    void set_mouse_position(uint16_t mouse_x, uint16_t mouse_y)
    {
        this->mouse_x = mouse_x;
        this->mouse_y = mouse_y;
        this->mouse_x_old = mouse_x;
        this->mouse_y_old = mouse_y;
    }

    void set_previous_time(MonotonicTimePoint now)
    {
        previous_time = now;
    }

    void set_key_delay(DelayConfig key_delay);

    void set_mouse_delay(DelayConfig key_delay);

    void set_kbd_fr(bool enable = true);

    void start(MonotonicTimePoint now);

    void scancode(MonotonicTimePoint now, KbdFlags flags, Scancode scancode);

    void unicode(MonotonicTimePoint now, KbdFlags flag, uint16_t utf16);

    void mouse(MonotonicTimePoint now, uint16_t device_flags, uint16_t x, uint16_t y);

    void keylocks(MonotonicTimePoint now, KeyLocks locks);

private:
    enum class CmdType : uint8_t;

    /// \return command type of the last line
    CmdType _synchronize_cmd(CmdType new_type, MonotonicTimePoint now, MonotonicTimePoint::duration max_delay);

    union PreviousValues
    {
        struct Sc
        {
            KbdFlags flags;
            Scancode scancode;
            uint16_t repetition;
            // compressed stack: packed to 5 bits * 6
            uint32_t mods;
            std::size_t previous_len;
        };

        struct Uni
        {
            uint16_t high_surrogate;
        };

        struct Click
        {
            uint16_t flags;
        };

        struct Whell
        {
            unsigned negative_flag;
            unsigned step;
            std::size_t previous_len;
        };

        Sc scancode;
        Uni unicode;
        Click click;
        Whell whell;
    };

    using Ms = std::chrono::milliseconds;

    MonotonicTimePoint previous_time;
    MonotonicTimePoint::duration max_key_delay{std::chrono::milliseconds(1000)};
    MonotonicTimePoint::duration max_mouse_delay{std::chrono::milliseconds(200)};
    Notifier notifier;
    std::string cmd_buffer;
    uint16_t mouse_x = 0;
    uint16_t mouse_y = 0;
    uint16_t mouse_x_old = 0;
    uint16_t mouse_y_old = 0;
    CmdType cmd_type {};
    MousePositionType mouse_position_type = MousePositionType::FirstCallIsAbsolute;
    bool is_en_kbd = true;
    PreviousValues previous_values;
};


struct RdpInputHeadlessCommandGenerator : RdpInput
{
    RdpInputHeadlessCommandGenerator(RdpInput& rdp_input, HeadlessCommandGenerator::Notifier notifier)
    : command_generator(notifier)
    , rdp_input(rdp_input)
    {}

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override
    {
        rdp_input.rdp_input_scancode(flags, scancode, event_time, keymap);
        command_generator.scancode(MonotonicTimePoint::clock::now(), flags, scancode);
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        rdp_input.rdp_input_unicode(flag, unicode);
        command_generator.unicode(MonotonicTimePoint::clock::now(), flag, unicode);
    }

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        rdp_input.rdp_input_mouse(device_flags, x, y);
        command_generator.mouse(MonotonicTimePoint::clock::now(), device_flags, x, y);
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        rdp_input.rdp_input_synchronize(locks);
        command_generator.keylocks(MonotonicTimePoint::clock::now(), locks);
    }

    void rdp_input_invalidate(Rect r) override
    {
        rdp_input.rdp_input_invalidate(r);
    }

    void rdp_gdi_up_and_running() override
    {
        rdp_input.rdp_gdi_up_and_running();
    }

    void rdp_gdi_down() override
    {
        rdp_input.rdp_gdi_down();
    }

    HeadlessCommandGenerator command_generator;

private:
    RdpInput& rdp_input;
};
