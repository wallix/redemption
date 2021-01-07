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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Jonathan Poelen,
              Meng Tan, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Use (implemented) basic RDP orders to draw some known test pattern
*/


#pragma once

#include "capture/file_to_graphic.hpp" // FileToGraphic::Verbose
#include "mod/mod_api.hpp"
#include "core/events.hpp"


class FrontAPI;
struct EventContainer;
struct Event;

class ReplayMod : public mod_api
{
public:
    using Verbose = FileToGraphic::Verbose;

    ReplayMod( EventContainer & events
             , gdi::GraphicApi & drawable
             , FrontAPI & front
             , std::string replay_path
             , std::string & auth_error_message
             , bool wait_for_escape
             , bool replay_on_loop
             , bool play_video_with_corrupted_bitmap
             , Verbose debug_capture);

    bool next_timestamp();

    ~ReplayMod();

    void rdp_input_invalidate(Rect /*rect*/) override {}

    void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/) override {}

    void rdp_input_scancode(long /*param1*/, long /*param2*/,
                            long /*param3*/, long /*param4*/, Keymap2 * keymap) override;

    void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                               int16_t /*param1*/, int16_t /*param2*/) override {}

    void rdp_gdi_up_and_running() override {}

    void rdp_gdi_down() override {}

    [[nodiscard]] Dimension get_dim() const override;

    void refresh(Rect /*rect*/) override {}

    [[nodiscard]] bool is_up_and_running() const override { return true; }

    bool server_error_encountered() const override { return false; }

    void send_to_mod_channel(CHANNELS::ChannelNameId /*front_channel_name*/, InStream & /*chunk*/, std::size_t /*length*/, uint32_t /*flags*/) override {}

private:
    void init_reader();

    timeval start_time;
    timeval start_time_replay;

    std::string& auth_error_message;

    gdi::GraphicApi& drawable;
    FrontAPI& front;

    std::string const prefix_path;

    class Reader;
    std::unique_ptr<Reader> internal_reader;

    Verbose const debug_capture;
    bool const wait_for_escape;
    bool const replay_on_loop;
    bool const play_video_with_corrupted_bitmap;

    EventsGuard events_guards;
};
