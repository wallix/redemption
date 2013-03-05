/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_WIDGET2_MOD_API_HPP_)
#define REDEMPTION_MOD_WIDGET2_MOD_API_HPP_

#include "RDP/RDPGraphicDevice.hpp"
#include "font.hpp"

class ModApi : public RDPGraphicDevice
{
public:
    virtual void begin_update() = 0;
    virtual void end_update() = 0;

    virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) = 0;
    virtual void text_metrics(const char * text, int & width, int & height) = 0;
    virtual FontChar * get_font(uint32_t c) = 0;

    class ContextText
    {
    public:
        size_t cx;
        size_t cy;

        ContextText()
        : cx(0)
        , cy(0)
        {}

        virtual void draw_in(ModApi* drawable, const Rect& rect, uint16_t x_screen, uint16_t y_screen, const Rect&  clip, int color) = 0;
    };

    virtual ContextText* create_context_text(const char * s) = 0;
};

#endif