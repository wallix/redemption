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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat

   Front object API (server),
   used to communicate with RDP client or other drawing backends
*/

#ifndef _REDEMPTION_CORE_FRONT_API_HPP_
#define _REDEMPTION_CORE_FRONT_API_HPP_

#include "channel_list.hpp"
#include "draw_api.hpp"
#include "stream.hpp"

class FrontAPI : public DrawApi {
    public:
    virtual void draw_vnc(const Rect & rect
                         , const uint8_t bpp
                         , const BGRPalette & palette332
                         , const uint8_t * raw
                         , uint32_t need_size) {}

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const = 0;
    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t * data
                                , size_t length, size_t chunk_size, int flags) = 0;

    virtual void send_global_palette() throw(Error) = 0;
    virtual void set_mod_palette(const BGRPalette & palette) = 0;

    virtual int server_resize(int width, int height, int bpp) = 0;
    virtual void update_config(const timeval & now, const Inifile & ini) {}

    int mouse_x;
    int mouse_y;
    bool notimestamp;
    bool nomouse;

    FrontAPI(bool notimestamp, bool nomouse)
        : mouse_x(0)
        , mouse_y(0)
        , notimestamp(notimestamp)
        , nomouse(nomouse) {}

    virtual void send_data_indication_ex(uint16_t channelId, HStream & stream) {}
};

#endif
