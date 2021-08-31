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

#include "client_redemption/client_input_output_api/rdp_disk_config.hpp"
#include "core/RDP/channels/rdpdr.hpp"


RDPDiskConfig::RDPDiskConfig() noexcept
: ioCode1(rdpdr::SUPPORT_ALL_REQUEST)
, extendedPDU(
    rdpdr::RDPDR_DEVICE_REMOVE_PDUS |
    rdpdr::RDPDR_CLIENT_DISPLAY_NAME_PDU  |
    rdpdr::RDPDR_USER_LOGGEDON_PDU)
, extraFlags1(rdpdr::ENABLE_ASYNCIO)
, SpecialTypeDeviceCap(0)
, general_capability_version(rdpdr::GENERAL_CAPABILITY_VERSION_02)
{}

void RDPDiskConfig::add_drive(const std::string & name, rdpdr::RDPDR_DTYP type)
{
    std::string tmp(name);
    // TODO find_last_of + std::string_view
    int pos(tmp.find('/'));
    while (pos != -1) {
        tmp = tmp.substr(pos+1, tmp.length());
        pos = tmp.find('/');
    }
    size_t size(tmp.size());
    if (size > 8) {
        size = 8;
    }
    std::array<char, 8> final_name = {0};
    memcpy(final_name.data(), tmp.data(), size);
    this->device_list.emplace_back(DeviceInfo{final_name, type});
}
