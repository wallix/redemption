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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean

   Fake Front class for Unit Testing
*/

#pragma once

#include "core/front_api.hpp"
#include "utils/image_view.hpp"

#include <memory>

namespace gdi
{
    class GraphicApi;
}

class FakeFront : public FrontAPI
{
public:
    FakeFront(ScreenInfo& screen_info);
    ~FakeFront();

    bool can_be_start_capture(bool /*force_capture*/) override { return false; }
    bool must_be_stop_capture() override { return false; }
    bool is_capture_in_progress() const override { return false; }

    const CHANNELS::ChannelDefArray & get_channel_list() const override;

    void send_to_channel( const CHANNELS::ChannelDef & /*channel*/, bytes_view /*chunk_data*/
                        , std::size_t /*total_length*/, int /*flags*/) override {}

    ResizeResult server_resize(ScreenInfo screen_server) override;

    void update_pointer_position(uint16_t /*x*/, uint16_t /*y*/) override {}

    operator ImageView () const;

    gdi::GraphicApi& gd() noexcept;

    void session_update(timeval /*now*/, LogId /*id*/, KVLogList /*kv_list*/) override {}
    void possible_active_window_change() override {}

private:
    class D;
    std::unique_ptr<D> d;
};
