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
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#ifndef _REDEMPTION_MOD_CLI_CLI_MOD_HPP_
#define _REDEMPTION_MOD_CLI_CLI_MOD_HPP_

#include "transitory/transitory.hpp"

struct cli_mod : public transitory_mod {
    cli_mod(
        struct ModContext & context, Inifile & ini, FrontAPI & front, ClientInfo & client_info, const uint16_t front_width, const uint16_t front_height)
            : transitory_mod(front, front_width, front_height)
    {
//        context.parse_username(client_info.username, ini);
      ini.parse_username(client_info.username);

        if (client_info.password[0]){
            ini.context_set_value(_AUTHID_PASSWORD, client_info.password);
        }
    }

    virtual ~cli_mod()
    {
    }

    virtual void begin_update() {}
    virtual void end_update() {}
    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) {}
    virtual void draw(const RDPScrBlt & cmd, const Rect &clip) {}
    virtual void draw(const RDPDestBlt & cmd, const Rect &clip) {}
    virtual void draw(const RDPPatBlt & cmd, const Rect &clip) {}
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) {}
    virtual void draw(const RDPLineTo& cmd, const Rect & clip) {}
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {}
    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) {}
    virtual void text_metrics(const char * text, int & width, int & height) {}
    virtual void send_to_front_channel(const char * const mod_channel_name, uint8_t* data, size_t length, size_t chunk_size, int flags){}
};

#endif
