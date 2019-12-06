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

#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"
#include "core/RDP/clipboard.hpp"


RDPClipboardConfig::RDPClipboardConfig() noexcept
: generalFlags(RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS)
{}

void RDPClipboardConfig::add_format(uint32_t id, Cliprdr::AsciiName name)
{
    this->formats.push(id, name);
}

void RDPClipboardConfig::add_format(RDPECLIP::CF cf)
{
    this->formats.push(cf, Cliprdr::AsciiName{{}});
}
