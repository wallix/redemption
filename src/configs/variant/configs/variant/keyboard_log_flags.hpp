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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_CORE_CONFIGS_KEYBOARD_LOG_FLAGS_HPP
#define REDEMPTION_SRC_CORE_CONFIGS_KEYBOARD_LOG_FLAGS_HPP

#include "configs/mk_enum_def.hpp"

#include <cstdlib>
#include <cerrno>


namespace configs {

enum class KeyboardLogFlags : unsigned {
    none,
    syslog = 1 << 0,
    wrm = 1 << 1,
    FULL = ((1 << 2) - 1)
};
MK_PARSER_ENUM_FLAGS(::configs::KeyboardLogFlags)

}

#include "configs/mk_enum_undef.hpp"

#endif
