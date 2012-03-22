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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat

   Front object API (server),
   used to communicate with RDP client or other drawing backends

*/

#if !defined(__CORE_FRONT_API_HPP__)
#define __CORE_FRONT_API_HPP__


class FrontAPI : public RDPGraphicDevice {
    public:

    using RDPGraphicDevice::draw;

    virtual int get_front_width() const = 0;
    virtual int get_front_height() const = 0;

    virtual const ChannelList & get_channel_list(void) const = 0;
    virtual void send_to_channel(const McsChannelItem & channel, uint8_t* data, size_t length, size_t chunk_size, int flags) = 0;

    virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error) = 0;
    virtual void send_global_palette() throw (Error) = 0;
    virtual void set_pointer(int cache_idx) throw (Error) = 0;
    virtual void begin_update() = 0;
    virtual void end_update() = 0;
    virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex) = 0;
    virtual void set_mod_palette(const BGRPalette & palette) = 0;
    virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask) = 0;
    virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) = 0;
    virtual void text_metrics(const char * text, int & width, int & height) = 0;
    virtual void init_mod() = 0;
    virtual int server_resize(int width, int height, int bpp) = 0;
    virtual void set_mod_bpp(uint8_t bpp) = 0;
    virtual void set_mod_bpp_to_front_bpp() = 0;

    int mouse_x;
    int mouse_y;
    bool notimestamp;
    bool nomouse;

    FrontAPI(bool notimestamp, bool nomouse) :
          notimestamp(notimestamp)
        , nomouse(nomouse)
        {}

};

#endif
