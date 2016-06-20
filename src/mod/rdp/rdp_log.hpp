/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#define MODRDP_LOGLEVEL_FSDRVMGR            0x00002000
#define MODRDP_LOGLEVEL_SESPROBE_LAUNCHER   0x00004000
#define MODRDP_LOGLEVEL_SESPROBE_REPETITIVE 0x00008000

#define MODRDP_LOGLEVEL_SESPROBE            0x02000000
#define MODRDP_LOGLEVEL_CLIPRDR             0x04000000
#define MODRDP_LOGLEVEL_RDPDR               0x08000000

#define MODRDP_LOGLEVEL_SESPROBE_DUMP       0x20000000
#define MODRDP_LOGLEVEL_CLIPRDR_DUMP        0x40000000
#define MODRDP_LOGLEVEL_RDPDR_DUMP          0x80000000

