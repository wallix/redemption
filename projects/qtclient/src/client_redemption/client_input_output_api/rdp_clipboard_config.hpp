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

#include "core/RDP/clipboard/format_name.hpp"

namespace ClientCLIPRDRConfig
{
    // Arbitrary format ID client redemption specifique must be superior to 0xbb00, else collision is a risk
    enum : uint32_t {
        CF_QT_CLIENT_FILEGROUPDESCRIPTORW = 48025
        , CF_QT_CLIENT_FILECONTENTS       = 48026
    };
}

namespace RDPECLIP
{
    enum CF : uint16_t;
}

struct RDPClipboardConfig
{
    std::string path;

    // Arbitrary picture resolution ratio, a 40 value empirically keep native resolution.
    // because we are using MM_ANISOTROPIC mapping mode
    // Data are lost if to low.
    double arbitrary_scale = 40;

    bool server_use_long_format_names = true;
    uint16_t cCapabilitiesSets = 1;
    uint32_t generalFlags;

    Cliprdr::FormatNameInventory formats;

    RDPClipboardConfig() noexcept;

    void add_format(uint32_t id, Cliprdr::AsciiName name);
    void add_format(RDPECLIP::CF cf);
};
