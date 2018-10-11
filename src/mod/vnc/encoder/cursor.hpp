/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
h
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Vnc encoder module for Cursor Pseudo Encoding
*/

#pragma once

#include "gdi/screen_info.hpp"
#include "mod/vnc/encoder/encoder_api.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "utils/rect.hpp"


namespace VNC
{
    namespace Encoder
    {
        Encoder cursor_encoder(
            BytesPerPixel Bpp, Rect rect,
            int red_shift, int red_max,
            int green_shift, int green_max,
            int blue_shift, int blue_max,
            VNCVerbose verbose);
    }  // namespace Encoder
} // namespace VNC
