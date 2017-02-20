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

    basic_trace         = 0x00000001,
    basic_trace2        = 0x00000002,
    basic_trace3        = 0x00000004,
    basic_trace4        = 0x00000008,
    basic_trace5        = 0x00000020,
    basic_trace6        = 0x00000010,
    graphics            = 0x00000040,
    basic_trace7        = 0x00000080,
    credssp             = 0x00000400,

    cache_persister     = 0x00000100,

    rail_order          = 0x00000200,

    fsdrvmgr            = 0x00002000,
    sesprobe_launcher   = 0x00004000,
    sesprobe_repetitive = 0x00008000,

    asynchronous_task   = 0x00800000,
    rail                = 0x01000000,
    sesprobe            = 0x02000000,
    cliprdr             = 0x04000000,
    rdpdr               = 0x08000000,

    rail_dump           = 0x10000000,
    sesprobe_dump       = 0x20000000,
    cliprdr_dump        = 0x40000000,
    rdpdr_dump          = 0x80000000,
};

