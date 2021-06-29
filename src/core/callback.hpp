/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni

   This class implement abstract callback used by front layer
   it is used to avoid explicit dependency between front layer
   and session (to enable to use front layer in tests).

*/

#pragma once

#include "utils/log.hpp"
#include "core/channel_names.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "keyboard/kbdtypes.hpp"
#include "utils/rect.hpp"

#include <string>

class InStream;
struct Keymap;
class ScreenInfo;

enum : uint16_t {
    RDP_INPUT_SYNCHRONIZE          = 0,
    // RDP_INPUT_CODEPOINT            = 1,
    // RDP_INPUT_VIRTKEY              = 2,
    RDP_INPUT_SCANCODE             = 4,
    RDP_INPUT_UNICODE              = 5,
    RDP_INPUT_MOUSE                = 0x8001
};

enum : uint16_t {
    MOUSE_FLAG_MOVE                = 0x0800,
    MOUSE_FLAG_BUTTON1             = 0x1000,
    MOUSE_FLAG_BUTTON2             = 0x2000,
    MOUSE_FLAG_BUTTON3             = 0x4000,
    MOUSE_FLAG_BUTTON4             = 0x0280,
    MOUSE_FLAG_BUTTON5             = 0x0380,
    MOUSE_FLAG_DOWN                = 0x8000,
    MOUSE_FLAG_HWHEEL              = 0x0400,
    MOUSE_FLAG_WHEEL               = 0x0200,
    MOUSE_FLAG_WHEEL_NEGATIVE      = 0x0100
};

struct WidgetApi : private noncopyable
{
    using KbdFlags = kbdtypes::KbdFlags;
    using Scancode = kbdtypes::Scancode;
    using KeyLocks = kbdtypes::KeyLocks;

    virtual ~WidgetApi() = default;
    virtual void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) = 0;
    virtual void rdp_input_unicode(KbdFlags flag, uint16_t unicode) { (void)unicode; (void)flag; }
    virtual void rdp_input_mouse(int device_flags, int x, int y) = 0;
    virtual void rdp_input_synchronize(KeyLocks locks) = 0;
    virtual void rdp_input_invalidate(Rect r) = 0;
    void rdp_input_invalidate2(array_view<Rect> vr) {
        for (Rect const & rect : vr) {
            if (!rect.isempty()) {
                this->rdp_input_invalidate(rect);
            }
        }
    }

    virtual void rdp_allow_display_updates(uint16_t /*left*/, uint16_t /*top*/, uint16_t /*right*/, uint16_t /*bottom*/)
    {}
    virtual void rdp_suppress_display_updates() {}

    virtual void refresh(Rect clip) = 0;
};


struct RdpInput : private noncopyable
{
    using KbdFlags = kbdtypes::KbdFlags;
    using Scancode = kbdtypes::Scancode;
    using KeyLocks = kbdtypes::KeyLocks;

    virtual ~RdpInput() = default;
    virtual void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) = 0;
    virtual void rdp_input_unicode(KbdFlags flag, uint16_t unicode) { (void)unicode; (void)flag; }
    virtual void rdp_input_mouse(int device_flags, int x, int y) = 0;
    virtual void rdp_input_synchronize(KeyLocks locks) = 0;
    virtual void rdp_input_invalidate(Rect r) = 0;
    virtual void rdp_input_invalidate2(array_view<Rect> vr) {
        for (Rect const & rect : vr) {
            if (!rect.isempty()) {
                this->rdp_input_invalidate(rect);
            }
        }
    }

    // Client Notify module that gdi is up and running
    virtual void rdp_gdi_up_and_running() = 0;

    // Client Notify module that gdi is not up and running any more
    virtual void rdp_gdi_down() = 0;

    virtual void rdp_allow_display_updates(uint16_t /*left*/, uint16_t /*top*/, uint16_t /*right*/, uint16_t /*bottom*/)
    {}
    virtual void rdp_suppress_display_updates() {}

    virtual void refresh(Rect clip) = 0;
};

struct Callback : RdpInput
{
    virtual void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) = 0;
};

