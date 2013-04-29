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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_DRAW_API_HPP)
#define REDEMPTION_MOD_DRAW_API_HPP

#include "RDP/RDPGraphicDevice.hpp"

class DrawApi : public RDPGraphicDevice
{
public:
    virtual ~DrawApi()
    {}

    virtual void begin_update() = 0;
    virtual void end_update() = 0;

    virtual void text_metrics(const char * text, int & width, int & height) = 0;

    virtual void server_draw_text(int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) = 0;
};

#endif