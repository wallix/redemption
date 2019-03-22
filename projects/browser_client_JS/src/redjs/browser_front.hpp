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

#pragma once

#ifdef IN_IDE_PARSER
# define __EMSCRIPTEN__
#endif

#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "redjs/js_table_id.hpp"
#include "redjs/browser_graphic.hpp"

#include <vector>
#include <string_view>

class ScreenInfo;
class OrderCaps;
class mod_api;

namespace redjs
{

class Channel;

class BrowserFront : public FrontAPI
{
public:
    BrowserFront(JsTableId id, uint16_t width, uint16_t height, OrderCaps& order_caps, RDPVerbose verbose);
    ~BrowserFront();

    void add_channel(Channel&& channel);

    bool can_be_start_capture() override;
    bool must_be_stop_capture() override;

    ResizeResult server_resize(ScreenInfo screen_server) override;

    const CHANNELS::ChannelDefArray & get_channel_list() const override
    {
        return cl;
    }

    void send_to_channel(
        const CHANNELS::ChannelDef& channel_def, uint8_t const* data,
        std::size_t length, std::size_t chunk_size, int flags) override;

    void update_pointer_position(uint16_t x, uint16_t y) override;

    void send_clipboard_utf8(std::string_view utf8_string);
    void send_file(std::string_view name, std::vector<uint8_t> data);
    void set_mod(mod_api * mod);

    gdi::GraphicApi& graphic_api() noexcept { return this->gd; }

private:
    BrowserGraphic gd;
    RDPVerbose verbose;
    CHANNELS::ChannelDefArray cl;

    class Clipboard;
    std::unique_ptr<Clipboard> clipboard;
};

}
