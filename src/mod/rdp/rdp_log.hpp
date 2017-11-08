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

    basic_trace         = 0x00000001,  // unspecific RDP loop events log
    connection          = 0x00000002,  // regroup connection log events which don't concern
                                       // directly license, security, channels or capabilities
    security            = 0x00000004,
    capabilities        = 0x00000008,

    license             = 0x00000010,
    asynchronous_task   = 0x00000020,
    graphics_pointer    = 0x00000040,
    graphics            = 0x00000080,

    input               = 0x00000100,
    rail_order          = 0x00000200,
    credssp             = 0x00000400,
    cache_persister     = 0x00000800,

    fsdrvmgr            = 0x00002000,
    sesprobe_launcher   = 0x00004000,
    sesprobe_repetitive = 0x00008000,

    drdynvc             = 0x00010000,

    drdynvc_dump        = 0x00100000,
    printer             = 0x00200000,
    rdpsnd              = 0x00400000,
    channels            = 0x00800000,

    rail                = 0x01000000,
    sesprobe            = 0x02000000,
    cliprdr             = 0x04000000,
    rdpdr               = 0x08000000,

    rail_dump           = 0x10000000,
    sesprobe_dump       = 0x20000000,
    cliprdr_dump        = 0x40000000,
    rdpdr_dump          = 0x80000000,
};
