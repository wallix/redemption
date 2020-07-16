/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "redjs/front.hpp"
#include "red_emscripten/em_asm.hpp"

#include "gdi/screen_info.hpp"
#include "utils/log.hpp"

#include "red_emscripten/constants.hpp"


namespace redjs
{

Front::Front(emscripten::val callbacks, uint16_t width, uint16_t height, RDPVerbose verbose)
: gd(std::move(callbacks), width, height)
, verbose(verbose)
{}

Front::~Front() = default;

PrimaryDrawingOrdersSupport Front::get_supported_orders() const
{
    return this->gd.get_supported_orders();
}

void Front::add_channel_receiver(ChannelReceiver channel_receiver)
{
    int channid = int(this->cl.size()) + 1;
    this->cl.push_back(CHANNELS::ChannelDef(channel_receiver.channel_name, 0, channid));
    this->channels.emplace_back(Channel{channel_receiver.ctx, channel_receiver.do_receive});
}

bool Front::can_be_start_capture(bool /*force_capture*/)
{
    return false;
}

bool Front::must_be_stop_capture()
{
    return false;
}

bool Front::is_capture_in_progress() const
{
    return false;
}

Front::ResizeResult Front::server_resize(ScreenInfo screen_server)
{
    if (bool(this->verbose & RDPVerbose::graphics)) {
        LOG(LOG_INFO, "Front::server_resize(width=%d, height=%d, bpp=%d)",
        screen_server.width, screen_server.height, screen_server.bpp);
    }

    return this->gd.resize_canvas(screen_server)
        ? ResizeResult::instant_done
        : ResizeResult::fail;
}

void Front::send_to_channel(
    CHANNELS::ChannelDef const& channel_def, bytes_view chunk_data,
    std::size_t total_data_len, int channel_flags)
{
    LOG_IF(bool(this->verbose & RDPVerbose::channels),
        LOG_INFO, "Front::send_to_channel('%s', ...)", channel_def.name.c_str());

    size_t idx = checked_int(&channel_def - &this->cl[0]);
    Channel& chann = this->channels[idx];
    chann.do_receive(chann.ctx, chunk_data, total_data_len, channel_flags);
}

void Front::update_pointer_position(uint16_t x, uint16_t y)
{
    LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
        LOG_INFO, "Front::update_pointer_position");

    this->gd.update_pointer_position(x, y);
}

} // namespace redjs


// 2.2.8.1.1.3.1.1.3 Mouse Event (TS_POINTER_EVENT)
RED_JS_BINDING_CONSTANTS(
    MouseFlags,

    // 2.2.7.1.6 Input Capability Set (TS_INPUT_CAPABILITYSET): TS_INPUT_FLAG_MOUSE_HWHEEL
    (("HorizontalWheel",   0x0400))
    (("VerticalWheel",     0x0200))
    (("WheelNegative",     0x0100))
    (("WheelRotationMask", 0x00FF))

    (("Move", 0x0800))
    (("Up",   0))
    (("Down", 0x8000))

    (("LeftButton",   0x1000))
    (("RightButton",  0x2000))
    (("MiddleButton", 0x4000))

    // 2.2.7.1.6 Input Capability Set (TS_INPUT_CAPABILITYSET): INPUT_FLAG_MOUSEX
    (("Button4", 0x0001))
    (("Button5", 0x0002))
);

//@{
// EXTENDED: Indicates that the keystroke message contains an extended scancode.
// For enhanced 101-key and 102-key keyboards, extended keys include the right ALT
// and right CTRL keys on the main section of the keyboard; the INS, DEL, HOME, END,
// PAGE UP, PAGE DOWN and ARROW keys in the clusters to the left of the numeric
// keypad; and the Divide ("/") and ENTER keys in the numeric keypad.
//
// EXTENDED1: Used to send keyboard events triggered by the PAUSE key.
// A PAUSE key press and release MUST be sent as the following sequence of keyboard events:
//     CTRL (0x1D) DOWN
//     NUMLOCK (0x45) DOWN
//     CTRL (0x1D) UP
//     NUMLOCK (0x45) UP
// The CTRL DOWN and CTRL UP events MUST both include the KBDFLAGS_EXTENDED1 flag.
//
// KBDFLAGS_DOWN: Indicates that the key was down prior to this event.
//
// KBDFLAGS_RELEASE: The absence of this flag indicates a key-down event,
// while its presence indicates a key-release event.

// 2.2.8.1.1.3.1.1.1 Keyboard Event (TS_KEYBOARD_EVENT)
RED_JS_BINDING_CONSTANTS(
    KbdFlags,

    (("Down",      0x4000))
    (("Release",   0x8000))
    (("Extended",  0x0100))
    (("Extended1", 0x0200))
);

// Unicode:
// 2.2.8.1.1.3.1.1.2 Unicode Keyboard Event (TS_UNICODE_KEYBOARD_EVENT)
// 2.2.8.1.2.2.2 Fast-Path Unicode Keyboard Event (TS_FP_UNICODE_KEYBOARD_EVENT)
// Release Only
//@}

// 2.2.7.1.6 Input Capability Set (TS_INPUT_CAPABILITYSET)
RED_JS_BINDING_CONSTANTS(
    InputFlags,

    (("Scancodes",       0x0001))
    (("MouseX",          0x0004))
    (("FastPath",        0x0008 | 0x0020))
    (("Unicode",         0x0010))
    (("HorizontalWheel", 0x0100))
);
