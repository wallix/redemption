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
    Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
               Jonathan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include "transport/transport.hpp"
#include "utils/match_finder.hpp"
#include "utils/difftimeval.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"

#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/capture_probe_api.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/bytes_t.hpp"
#include "utils/sugar/make_unique.hpp"

#include <algorithm>
#include <memory>

#include <ctime>



