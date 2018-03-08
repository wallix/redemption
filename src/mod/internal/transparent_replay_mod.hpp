/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "core/session_reactor.hpp"
#include "transport/in_file_transport.hpp"
#include "capture/transparentplayer.hpp"
#include "mod/internal/internal_mod.hpp"

class TransparentReplayMod : public InternalMod
{
private:
    std::string * auth_error_message;

    InFileTransport   ift;
    TransparentPlayer player;
    SessionReactor& session_reactor;
    SessionReactor::GraphicFdPtr fd_event;

public:
    TransparentReplayMod( SessionReactor& session_reactor
                        , FrontAPI & front
                        , const char * replay_path
                        , uint16_t width
                        , uint16_t height
                        , std::string * auth_error_message
                        , Font const & font)
    : InternalMod(front, width, height, font, Theme{}, false)
    , auth_error_message(auth_error_message)
    , ift(unique_fd{[&]() {
        const int fd = ::open(replay_path, O_RDWR);
        if (fd == -1) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        return fd;
    }()})
    , player(this->ift, this->front)
    , session_reactor(session_reactor)
    {
        this->fd_event = session_reactor.create_graphic_fd_event(
            this->ift.get_fd(), std::ref(*this))
        .set_timeout(std::chrono::seconds{1})
        .on_timeout(jln::always_ready())
        .on_exit(jln::exit_with_success())
        .on_action(jln::always_ready([](gdi::GraphicApi& gd, TransparentReplayMod& self){
            self.draw_event(0, gd);
        }));
    }

    void draw_event(time_t /*now*/, gdi::GraphicApi &) override
    {
        try {
            if (!this->player.interpret_chunk()) {
                this->session_reactor.set_next_event(BACK_EVENT_NEXT);
            }
        }
        catch (Error const& e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                if (this->auth_error_message) {
                    *this->auth_error_message = "The recorded file is inaccessible or corrupted!";
                }
                this->session_reactor.set_next_event(BACK_EVENT_NEXT);
            }
            else {
                throw;
            }
        }
    }

    bool is_up_and_running() override { return true; }
};
