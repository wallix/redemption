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

   module object. Some kind of interface between core and modules

*/
#if !defined(__GRAPHIC_DEVICE_HPP__)
#define __GRAPHIC_DEVICE_HPP__

#include <stdint.h>
#include "client_info.hpp"
#include "rect.hpp"
#include "colors.hpp"
#include "font.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"


struct GraphicDevice
{
    virtual void text_metrics(const char * text, int & width, int & height) = 0;
    virtual int server_begin_update() = 0;
    virtual int server_end_update() = 0;
    virtual int get_front_bpp() const = 0;
    virtual int get_front_width() const = 0;
    virtual int get_front_height() const = 0;
    virtual const Rect get_front_rect() = 0;
    virtual void draw_window(const Rect & r, uint32_t bgcolor, const char * caption, bool has_focus, const Rect & clip) = 0;
    virtual void draw_combo(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip) = 0;
    virtual void draw_button(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip) = 0;
    virtual void draw_edit(const Rect & r, char password_char, char * buffer, size_t edit_pos, bool has_focus, const Rect & clip) = 0;
    virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) = 0;

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPLineTo & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) = 0;

    virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex) = 0;
    virtual void glyph_cache(const FontChar & font_char, int font_index, int char_index) = 0;


    virtual void mem_blt(const RDPMemBlt & memblt, Bitmap & bitmap, const BGRPalette & palette, const Rect & clip) = 0;

    virtual void server_set_pen(int style, int width) = 0;
    virtual void bitmap_update(Bitmap & bitmap, const Rect & dst, int srcx, int srcy, const Rect & clip) = 0;
    virtual void set_pointer(int cache_idx) = 0;
    virtual void send_global_palette() = 0;
    virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask) = 0;

    virtual void set_mod_palette(const BGRPalette & palette) = 0;
    virtual const BGRColor convert(const BGRColor color) const = 0;
    virtual const BGRColor convert_opaque(const BGRColor color) const = 0;
    virtual const BGRColor convert24_opaque(const BGRColor color) const = 0;
    virtual const BGRColor convert24(const BGRColor color) const = 0;
    virtual uint32_t convert_to_black(uint32_t color) = 0;



};


#endif
