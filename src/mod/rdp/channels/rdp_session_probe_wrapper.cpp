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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "mod/rdp/channels/rdp_session_probe_wrapper.hpp"
#include "mod/rdp/rdp.hpp"
#include "utils/stream.hpp"


RdpSessionProbeWrapper::RdpSessionProbeWrapper(mod_rdp& rdp)
: rdp(rdp)
{}

void RdpSessionProbeWrapper::send_cliprdr_message(bytes_view message)
{
    InStream in_stream(message);
    this->rdp.send_to_mod_channel(channel_names::cliprdr,
                                  in_stream, message.size(),
                                  CHANNELS::CHANNEL_FLAG_FIRST
                                | CHANNELS::CHANNEL_FLAG_LAST
                                | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
}

void RdpSessionProbeWrapper::send_scancode(kbdtypes::KbdFlags flags, kbdtypes::Scancode scancode)
{
    rdp.send_input_scancode(0, flags, scancode);
}

void RdpSessionProbeWrapper::reset_keyboard_status()
{
    rdp.reset_keyboard_status();
}
