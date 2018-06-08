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
#include "core/RDP/channels/rdpdr.hpp"



struct RDPDiskConfig {

	struct DeviceInfo {
		char name[8] = {0};
		rdpdr::RDPDR_DTYP type = rdpdr::RDPDR_DTYP_UNSPECIFIED;

		DeviceInfo(const char * name, rdpdr::RDPDR_DTYP type)
		: type(type)
		{
			for (int i = 0; i < 8; i++) {
				this->name[i] = name[i];
			}
		}
	};

	std::vector<DeviceInfo> device_list;

	void add_drive(const std::string & name, const rdpdr::RDPDR_DTYP type) {
 		std::string tmp(name);
		int pos(tmp.find('/'));
		while (pos != -1) {
		    tmp = tmp.substr(pos+1, tmp.length());
		    pos = tmp.find('/');
		}
		size_t size(tmp.size());
		if (size > 8) {
		    size = 8;
		}
		char final_name[8] = {0};
		for (size_t i = 0; i < size; i++) {
		    final_name[i] = tmp.data()[i];
		}
		DeviceInfo drive(final_name, type);
		this->device_list.push_back(drive);
	}

	bool enable_drive_type = true;
	bool enable_printer_type = true;
	bool enable_port_type = true;
	bool enable_smart_card_type = true;

    uint32_t ioCode1 = rdpdr::SUPPORT_ALL_REQUEST;
    uint32_t extendedPDU = rdpdr::RDPDR_DEVICE_REMOVE_PDUS |
                           rdpdr::RDPDR_CLIENT_DISPLAY_NAME_PDU  |
                           rdpdr::RDPDR_USER_LOGGEDON_PDU;
    uint32_t extraFlags1 = rdpdr::ENABLE_ASYNCIO;
    uint32_t SpecialTypeDeviceCap = 0;
    uint32_t general_capability_version = rdpdr::GENERAL_CAPABILITY_VERSION_02;
};

