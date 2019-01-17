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
#include "transport/mwrm_reader.hpp" // WrmVersion
#include "core/session_reactor.hpp"
#include "mod/mod_api.hpp"

class FrontAPI;

class ReplayMod : public mod_api
{
    std::string& auth_error_message;

    uint16_t front_width;
    uint16_t front_height;
    FrontAPI& front;

    class Reader;
    std::unique_ptr<Reader> internal_reader;

    bool end_of_data;
    bool wait_for_escape;

    bool sync_setted;

    bool replay_on_loop;
    bool play_video_with_corrupted_bitmap;
    SessionReactor& session_reactor;
    SessionReactor::GraphicTimerPtr timer;

public:
    using Verbose = FileToGraphic::Verbose;

    ReplayMod( SessionReactor& session_reactor
             , FrontAPI & front
             , const char * replay_path
             , uint16_t width
             , uint16_t height
             , std::string & auth_error_message
             , bool wait_for_escape
             , bool replay_on_loop
             , bool play_video_with_corrupted_bitmap
             , Verbose debug_capture)
    : ReplayMod(
        session_reactor, front, replay_path, width, height, auth_error_message,
        wait_for_escape, timeval{0, 0}, timeval{0, 0}, 0, replay_on_loop,
        play_video_with_corrupted_bitmap, debug_capture)
    {
    }

    ReplayMod( SessionReactor& session_reactor
             , FrontAPI & front
             , const char * replay_path
             , uint16_t width
             , uint16_t height
             , std::string & auth_error_message
             , bool wait_for_escape
             , timeval const & begin_read
             , timeval const & end_read
             , time_t balise_time_frame
             , bool replay_on_loop
             , bool play_video_with_corrupted_bitmap
             , Verbose debug_capture);

    ~ReplayMod() override;

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr,
        gdi::ResizeApi * resize_ptr
    );

    void play();

    bool play_client();

    void set_sync();

    WrmVersion get_wrm_version() const;

    bool get_break_privplay_client();

    void instant_play_client(std::chrono::microseconds endin_frame);

    void rdp_input_invalidate(Rect /*rect*/) override
    {}

    void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/, Keymap2 * /*keymap*/) override {}

    void rdp_input_scancode(long /*param1*/, long /*param2*/,
                            long /*param3*/, long /*param4*/, Keymap2 * keymap) override;

    void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                               int16_t /*param1*/, int16_t /*param2*/) override
    {}

    void set_pause(timeval & time);

    void set_wait_after_load_client(timeval & time);

    time_t get_real_time_movie_begin();

    std::string get_mwrm_path() const;

    Dimension get_dim() const override;

    void refresh(Rect /*rect*/) override {}

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    void draw_event(time_t now, gdi::GraphicApi & gd) override;

    bool is_up_and_running() const override
    { return true; }
};
