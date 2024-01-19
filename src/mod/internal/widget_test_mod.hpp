/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "mod/internal/rail_mod_base.hpp"

#include <memory>


class WidgetTestMod : public RailInternalModBase
{
public:
    WidgetTestMod(
        gdi::GraphicApi & gd,
        EventContainer & events,
        FrontAPI & front, uint16_t width, uint16_t height,
        ClientExecute & rail_client_execute, Font const & font,
        Theme const & theme, CopyPaste& copy_paste);

    ~WidgetTestMod() override;

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}

    void rdp_input_invalidate(Rect clip) override;

    void rdp_input_mouse(uint16_t /*device_flags*/, uint16_t x, uint16_t y) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override;

    void rdp_input_synchronize(KeyLocks locks) override;

    void acl_update(AclFieldMask const&/* acl_fields*/) override
    {}

private:
    class WidgetTestModPrivate;
    friend WidgetTestModPrivate;
    std::unique_ptr<WidgetTestModPrivate> d;
};
