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
   Copyright (C) Wallix 2018
   Author(s): David Fort

   A proxy that will capture all the traffic to the target
*/
#pragma once

#include "utils/difftimeval.hpp"
#include "utils/fixed_random.hpp"
#include "utils/utf.hpp"
#include "core/RDP/nego.hpp"
#include "core/RDP/nla/nla_server_ntlm.hpp"


class NegoServer
{
    FixedRandom rand;
public:
    NtlmServer credssp;

public:
    NegoServer(bytes_view key, const TimeBase & time_base, uint64_t verbosity)
    : credssp(false, true, "WIN7"_av, "WIN7"_av,"WIN7"_av,"win7"_av,"win7"_av, "win7"_av, key,
        {MsvAvNbDomainName,MsvAvNbComputerName,MsvAvDnsDomainName,MsvAvDnsComputerName,MsvAvTimestamp},
        rand, time_base, 6,
        NtlmVersion{WINDOWS_MAJOR_VERSION_6, WINDOWS_MINOR_VERSION_1, 7601, NTLMSSP_REVISION_W2K3},
        false, verbosity, verbosity)
    {
    }
};

