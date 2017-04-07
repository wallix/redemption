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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

enum DATA_BLOCK_TYPE {
    // The data block that follows contains Client Core Data (section 2.2.1.3.2).
    CS_CORE = 0xC001,
    // The data block that follows contains Client Security Data (section 2.2.1.3.3).
    CS_SECURITY = 0xC002,
    // The data block that follows contains Client Network Data (section 2.2.1.3.4).
    CS_NET = 0xC003,
    // The data block that follows contains Client Cluster Data (section 2.2.1.3.5).
    CS_CLUSTER = 0xC004,
    // The data block that follows contains Client Monitor Data (section 2.2.1.3.6).
    CS_MONITOR = 0xC005,
    // The data block that follows contains Client Message Channel Data (section 2.2.1.3.7).
    CS_MCS_MSGCHANNEL = 0xC006,
    // The data block that follows contains Client Multitransport Channel Data (section 2.2.1.3.8).
    CS_MULTITRANSPORT = 0xC00A,
    // The data block that follows contains Client Monitor Ex Data (section 2.2.1.3.9).
    CS_MONITOR_EX = 0xc008,
    // The data block that follows contains Server Core Data (section 2.2.1.4.2).
    SC_CORE = 0x0C01,
    // The data block that follows contains Server Security Data (section 2.2.1.4.3).
    SC_SECURITY = 0x0C02,
    // The data block that follows contains Server Network Data (section 2.2.1.4.4).
    SC_NET = 0x0C03
};

