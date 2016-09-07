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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "mod/internal/client_execute.hpp"
#include "mod/internal/internal_mod.hpp"

struct LocallyIntegrableMod : public InternalMod {
    ClientExecute & client_execute;

    LocallyIntegrableMod(FrontAPI & front,
                         uint16_t front_width, uint16_t front_height,
                         Font const & font, ClientExecute & client_execute,
                         Theme const & theme = Theme())
    : InternalMod(front, front_width, front_height, font, theme)
    , client_execute(client_execute) {}

    ~LocallyIntegrableMod() override {
        this->client_execute.reset();
    }

    void draw_event(time_t, gdi::GraphicApi &) override {
        if (!this->client_execute && event.waked_up_by_time) {
            this->client_execute.ready(*this);
        }
    }

    void send_to_mod_channel(const char * front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        if (this->client_execute && !strcmp(front_channel_name, CHANNELS::channel_names::rail)) {
            this->client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
    }
};