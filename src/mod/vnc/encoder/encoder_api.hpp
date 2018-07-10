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

   Vnc encoder module for RAW bitmaps
*/

#pragma once

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "core/buf64k.hpp"
#include "gdi/graphic_api.hpp"


namespace VNC {
    namespace Encoder {
        enum class EncoderState {
            Ready,
            NeedMoreData,
            Exit
        };
        class EncoderApi {
        public:
            virtual EncoderState consume(Buf64k & buf, gdi::GraphicApi & drawable) = 0;
            virtual ~EncoderApi() = default;
        };
    }  // namespace Encoder
} // namespace VNC
