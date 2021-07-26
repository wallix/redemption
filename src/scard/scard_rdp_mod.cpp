/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): 
 */

#include <string>
#include <utility>

#include "configs/config.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"
#include "utils/sugar/algostring.hpp"

#include "scard/scard_rdp_mod.hpp"


///////////////////////////////////////////////////////////////////////////////


SmartcardMod::SmartcardMod(
    SmartcardModVariables vars,
    FrontAPI &front,
    EventContainer &events,
    gdi::GraphicApi &graphics,
    uint16_t width, uint16_t height,
    ClientExecute &rail_client_execute,
    const Font &font,
    const Theme &theme)
    :
    RailModBase(graphics, front, width, height, rail_client_execute, font, theme),
    _variables(vars),
    _events_guard(events),
    _copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
{
    //
    screen.set_wh(front_width, front_height);
    screen.rdp_input_invalidate(screen.get_rect());

    // create bridge
    _scard_bridge_ptr = std::make_unique<scard_bridge>(
        front, events);
}

void SmartcardMod::init()
{
    RailModBase::init();
    _copy_paste.ready(front);
}

void SmartcardMod::acl_update(const AclFieldMask &/*acl_fields*/)
{
}

void SmartcardMod::notify(Widget &/*sender*/, notify_event_t event)
{
    switch (event)
    {
        case NOTIFY_SUBMIT:
            set_mod_signal(BACK_EVENT_NEXT);
            break;
        case NOTIFY_CANCEL:
            set_mod_signal(BACK_EVENT_STOP);
            break;
        default:
            break;
    }
}

void SmartcardMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name,
    InStream &chunk, size_t length, uint32_t flags)
{
    RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (_copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr)
    {
        _copy_paste.send_to_mod_channel(chunk, flags);
    }

    if (_scard_bridge_ptr)
    {
        bool complete;

        // process
        if (front_channel_name == CHANNELS::channel_names::rdpdr)
        {
            complete = _scard_bridge_ptr->process(
                length, flags, chunk.remaining_bytes());
        }
        else
        {
            complete = _scard_bridge_ptr->process();
        }

        // kill
        if (complete)
        {
            _scard_bridge_ptr.reset(nullptr);
        }
    }
}