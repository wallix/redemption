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

#include "core/channel_names.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/rect.hpp"

class InStream;
struct Keymap2;

enum : uint16_t {
    RDP_INPUT_SYNCHRONIZE          = 0,
    RDP_INPUT_CODEPOINT            = 1,
    RDP_INPUT_VIRTKEY              = 2,
    RDP_INPUT_SCANCODE             = 4,
    RDP_INPUT_UNICODE              = 5,
    RDP_INPUT_MOUSE                = 0x8001
};

/* Device flags */
enum : uint16_t {
    KBD_FLAG_RIGHT                 = 0x0001,
    KBD_FLAG_EXT                   = 0x0100,
    KBD_FLAG_QUIET                 = 0x1000,
    KBD_FLAG_DOWN                  = 0x4000,
    KBD_FLAG_UP                    = 0x8000
};

/* These are for synchronization; not for keystrokes */
enum : uint16_t {
    KBD_FLAG_SCROLL                = 0x0001,
    KBD_FLAG_NUMLOCK               = 0x0002,
    KBD_FLAG_CAPITAL               = 0x0004
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

enum : uint8_t {
    FASTPATH_INPUT_KBDFLAGS_RELEASE  = 0x01
  , FASTPATH_INPUT_KBDFLAGS_EXTENDED = 0x02
};

enum: uint8_t {
    KBD_SCANCODE_ALTGR  = 0x38,
    KBD_SCANCODE_SHIFT  = 0x36,
    KBD_SCANCODE_ENTER  = 0x1C,
    KBD_SCANCODE_BK_SPC = 0x0E,
    KBD_SCANCODE_CTRL   = 0x1D,
    KBD_SCANCODE_DELETE = 0x53
};


struct RdpInput : private noncopyable
{
    virtual ~RdpInput() = default;
    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) = 0;
    virtual void rdp_input_unicode(uint16_t unicode, uint16_t flag) { (void)unicode; (void)flag; }
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) = 0;
    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) = 0;
    virtual void rdp_input_invalidate(Rect r) = 0;
    virtual void rdp_input_invalidate2(array_view<Rect const> vr) {
        for (Rect const & rect : vr) {
            if (!rect.isempty()) {
                this->rdp_input_invalidate(rect);
            }
        }
    }
    // Client calls this member function when it became up and running.
    virtual void rdp_input_up_and_running() { /* LOG(LOG_ERR, "CB:UP_AND_RUNNING"); */}

    virtual void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
    { (void)left; (void)top; (void)right; (void)bottom; }

    virtual void rdp_suppress_display_updates() {}

    virtual void refresh(Rect clip) = 0;

    virtual void set_last_tram_len(size_t tram_length) {(void)tram_length;}
};

struct Callback : RdpInput
{
    virtual void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags)
    {
        (void)front_channel_name;
        (void)chunk;
        (void)length;
        (void)flags;
    }
    // Interface for session to send back to mod_rdp for tse virtual channel target data (asked previously)
    virtual void send_auth_channel_data(const char * data) { (void)data; }
    virtual void send_checkout_channel_data(const char * data) { (void)data; }
};

