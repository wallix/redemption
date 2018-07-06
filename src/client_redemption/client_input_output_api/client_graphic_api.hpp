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
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "utils/log.hpp"


#include "gdi/graphic_api.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "client_redemption/client_redemption_api.hpp"





class ClientOutputGraphicAPI {

public:
    ClientRedemptionAPI * drawn_client;

    const int screen_max_width;
    const int screen_max_height;

    bool is_pre_loading;

    ClientOutputGraphicAPI(int max_width, int max_height)
      : drawn_client(nullptr),
		screen_max_width(max_width)
      , screen_max_height(max_height)
      , is_pre_loading(false) {
    }

    virtual ~ClientOutputGraphicAPI() = default;

    virtual void set_drawn_client(ClientRedemptionAPI * client) {
        this->drawn_client = client;
    }

    virtual void set_ErrorMsg(std::string const & movie_path) = 0;

    virtual void dropScreen() = 0;

    virtual void show_screen() = 0;

    virtual void reset_cache(int w,  int h) = 0;

    virtual void create_screen() = 0;

    virtual void closeFromScreen() = 0;

    virtual void set_screen_size(int x, int y) = 0;

    virtual void update_screen() = 0;


    // replay mod

    virtual void create_screen(std::string const &  /*unused*/, std::string const &  /*unused*/) {}

    virtual void draw_frame(int  /*unused*/) {}


    // remote app

    virtual void create_remote_app_screen(uint32_t  /*unused*/, int  /*unused*/, int  /*unused*/, int  /*unused*/, int  /*unused*/) {}

    virtual void move_screen(uint32_t  /*unused*/, int  /*unused*/, int  /*unused*/) {}

    virtual void set_screen_size(uint32_t  /*unused*/, int  /*unused*/, int  /*unused*/) {}

    virtual void set_pixmap_shift(uint32_t  /*unused*/, int  /*unused*/, int  /*unused*/) {}

    virtual int get_visible_width(uint32_t  /*unused*/) {return 0;}

    virtual int get_visible_height(uint32_t  /*unused*/) {return 0;}

    virtual int get_mem_width(uint32_t  /*unused*/) {return 0;}

    virtual int get_mem_height(uint32_t  /*unused*/) {return 0;}

    virtual void set_mem_size(uint32_t  /*unused*/, int  /*unused*/, int  /*unused*/) {}

    virtual void show_screen(uint32_t  /*unused*/) {}

    virtual void dropScreen(uint32_t  /*unused*/) {}

    virtual void clear_remote_app_screen() {}




    virtual FrontAPI::ResizeResult server_resize(int width, int height, int bpp) = 0;

    virtual void set_pointer(Pointer      const & /*unused*/) {}

    virtual void draw(RDP::FrameMarker    const & cmd) = 0;
    virtual void draw(RDPNineGrid const & , Rect , gdi::ColorCtx , Bitmap const & ) = 0;
    virtual void draw(RDPDestBlt          const & cmd, Rect clip) = 0;
    virtual void draw(RDPMultiDstBlt      const & cmd, Rect clip) = 0;
    virtual void draw(RDPScrBlt           const & cmd, Rect clip) = 0;
    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) = 0;
    virtual void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) = 0;

    virtual void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) = 0;
    virtual void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) = 0;



    virtual void begin_update() {}
    virtual void end_update() {}
};


