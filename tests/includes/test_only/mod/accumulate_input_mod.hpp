/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "mod/null/null.hpp"
#include "test_only/session_log_test.hpp"


struct AccumulateInputMod : null_mod
{
    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override;

    void rdp_input_synchronize(KeyLocks locks) override;

    void rdp_input_invalidate(Rect rect) override;

    std::string events()
    {
        return session_log.events();
    }

    SessionLogTest session_log;
};
