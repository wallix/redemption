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

    virtual int get_front_bpp() const = 0;
    virtual int get_front_width() const = 0;
    virtual int get_front_height() const = 0;
    virtual int get_front_build() const = 0;
    virtual int get_front_console_session() const = 0;
    virtual int get_front_brush_cache_code() const = 0;

    virtual const ChannelList & get_channel_list(void) const = 0;
    virtual void send_to_channel(const McsChannelItem & channel, uint8_t* data, size_t length, size_t chunk_size, int flags) = 0;

    virtual void set_front_resolution(uint16_t width, uint16_t height, uint8_t bpp) = 0;
    virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error) = 0;
    virtual void send_global_palette() throw (Error) = 0;
    virtual void set_pointer(int cache_idx) throw (Error) = 0;
    virtual void begin_update() = 0;
    virtual void end_update() = 0;
    virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex) = 0;
    virtual void set_mod_palette(const BGRPalette & palette) = 0;

    int mouse_x;
    int mouse_y;

    bool palette_sent;
    bool palette_memblt_sent[6];
    BGRPalette palette332;
    BGRPalette mod_palette;
    uint8_t mod_bpp;
    BGRPalette memblt_mod_palette;
    bool mod_palette_setted;

    RDPGraphicDevice * capture;
    struct Font font;
    Cache cache;
    bool notimestamp;
    bool nomouse;

    GraphicsUpdatePDU * orders;

    FrontAPI(Inifile * ini)
        : capture(NULL)
        , font(SHARE_PATH "/" DEFAULT_FONT_NAME)
        , cache()
        , notimestamp(ini->globals.notimestamp)
        , nomouse(ini->globals.nomouse)
        {}

};

#endif
