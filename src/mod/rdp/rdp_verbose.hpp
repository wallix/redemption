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

#include "utils/verbose_flags.hpp"


REDEMPTION_VERBOSE_FLAGS_DEF(RDPVerbose)
{
    none,

    basic_trace         = 0x0000'0001, // unspecific RDP loop events log
    connection          = 0x0000'0002, // regroup connection log events which don't concern
                                       // directly license, security, channels or capabilities
    security            = 0x0000'0004,
    capabilities        = 0x0000'0008,

    license             = 0x0000'0010,
    asynchronous_task   = 0x0000'0020,
    graphics_pointer    = 0x0000'0040,
    graphics            = 0x0000'0080,

    input               = 0x0000'0100,
    rail_order          = 0x0000'0200,
    credssp             = 0x0000'0400,
    negotiation         = 0x0000'0800,

    cache_persister     = 0x0000'1000,

    fsdrvmgr            = 0x0000'2000,
    sesprobe_launcher   = 0x0000'4000,
    sesprobe_repetitive = 0x0000'8000,

    drdynvc             = 0x0001'0000,
    surfaceCmd          = 0x0002'0000,

    // BmpCachePersister
    //@{
    cache_from_disk     = 0x0004'0000,
    bmp_info            = 0x0008'0000,
    //@}

    drdynvc_dump        = 0x0010'0000,
    printer             = 0x0020'0000,
    rdpsnd              = 0x0040'0000,
    channels            = 0x0080'0000,

    rail                = 0x0100'0000,
    sesprobe            = 0x0200'0000,
    cliprdr             = 0x0400'0000,
    rdpdr               = 0x0800'0000,

    rail_dump           = 0x1000'0000,
    sesprobe_dump       = 0x2000'0000,
    cliprdr_dump        = 0x4000'0000,
    rdpdr_dump          = 0x8000'0000,
};
