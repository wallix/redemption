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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   mcs layer at core module

*/

#if !defined(__SERVER_MCS_HPP__)
#define __SERVER_MCS_HPP__

#include "stream.hpp"
#include "constants.hpp"
#include "file_loc.hpp"
#include "log.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"

#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <vector>
using namespace std;

struct server_mcs : public Mcs {

};


#endif
