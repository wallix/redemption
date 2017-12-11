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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include <ctime>

class Transport;
class FrontAPI;


class TransparentPlayer
{
public:
    TransparentPlayer(Transport & t, FrontAPI & consumer);

    bool interpret_chunk(bool real_time = true);

private:
    Transport & t;
    FrontAPI & consumer;

    bool meta_ok;

    timeval record_now;
    timeval replay_now;
};

