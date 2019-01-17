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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan, Raphael Zhou
*/

#include "gdi/screen_functions.hpp"
#include "gdi/graphic_api.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "utils/rect.hpp"

void gdi_clear_screen(gdi::GraphicApi& drawable, Dimension const& dim)
{
    Rect const r(0, 0, dim.w, dim.h);
    RDPOpaqueRect cmd(r, color_encode(BLACK, BitsPerPixel{24}));
    drawable.begin_update();
    drawable.draw(cmd, r, gdi::ColorCtx::depth24());
    drawable.end_update();
}

void gdi_freeze_screen(gdi::GraphicApi& drawable, Dimension const& dim)
{
    Rect const r(0, 0, dim.w, dim.h);
    RDPPatBlt cmd(
        r, 0xA0, color_encode(BLACK, BitsPerPixel{24}), color_encode(WHITE, BitsPerPixel{24}),
        RDPBrush(0, 0, 3, 0xaa, cbyte_ptr("\x55\xaa\x55\xaa\x55\xaa\x55"))
    );
    drawable.begin_update();
    drawable.draw(cmd, r, gdi::ColorCtx::depth24());
    drawable.end_update();
}
