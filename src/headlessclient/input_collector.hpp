/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/callback.hpp"
#include "utils/monotonic_clock.hpp"

#include <vector>

/// Save events of \c RdpInput::rdp_input_* and resent them later with \c send_next_input()
class InputCollector : public RdpInput
{
private:
    enum class InputType : uint8_t
    {
        Scancode,
        Unicode,
        KeyLock,
        Mouse,
        MouseEx,
    };

    struct InputData
    {
        InputType type;
        uint16_t flags_or_locks;
        uint16_t sc_or_uc_or_x;
        uint16_t y;
    };

public:
    enum class ConsumedInput : uint8_t
    {
        None,
        KeyEvent,
        MouseEvent,
    };

    ConsumedInput send_next_input(RdpInput& mod, Keymap const& keymap);

    bool has_input() const noexcept;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override
    {
        (void)keymap;
        (void)event_time;
        push_data(InputData{InputType::Scancode, underlying_cast(flags), underlying_cast(scancode), 0});
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        push_data(InputData{InputType::Unicode, underlying_cast(flag), unicode, 0});
    }

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        push_data(InputData{InputType::Mouse, device_flags, x, y});
    }

    void rdp_input_mouse_ex(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        push_data(InputData{InputType::MouseEx, device_flags, x, y});
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        push_data(InputData{InputType::KeyLock, underlying_cast(locks), 0, 0});
    }

    void rdp_input_invalidate(Rect r) override
    {
        (void)r;
    }

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}

private:
    void push_data(InputData data);

    std::size_t current_input_index = 0;
    std::vector<InputData> inputs;
};
