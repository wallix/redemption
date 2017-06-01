/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2013-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

namespace configs
{
    template<class... Ts>
    struct Pack : Ts...
    { static const std::size_t size = sizeof...(Ts); };
}

// members
//@{
#include "core/font.hpp"
#include "utils/theme.hpp"
#include "utils/redirection_info.hpp"
#include <string>
#include <chrono>
//@}

#include "configs/io.hpp"
#include "configs/autogen/enums.hpp"
#include "configs/autogen/variables_configuration.hpp"
