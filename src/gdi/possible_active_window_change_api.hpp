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

#ifndef REDEMPTION_GDI_POSSIBLE_ACTIVE_WINDOW_CHANGE_HPP
#define REDEMPTION_GDI_POSSIBLE_ACTIVE_WINDOW_CHANGE_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"

struct timeval;
class wait_obj;

namespace gdi {

struct PossibleActiveWindowChangeApi : private noncopyable
{
    virtual ~PossibleActiveWindowChangeApi() = default;

    virtual void possible_active_window_change() = 0;
};

using PossibleActiveWindowChangeApiDeleterBase = utils::virtual_deleter_base<PossibleActiveWindowChangeApi>;
using PossibleActiveWindowChangeApiPtr = utils::unique_ptr_with_virtual_deleter<PossibleActiveWindowChangeApi>;

using utils::default_delete;
using utils::no_delete;

template<class PossibleActiveWindowChange, class... Args>
PossibleActiveWindowChangeApiPtr make_snapshot_ptr(Args && ... args) {
    return PossibleActiveWindowChangeApiPtr(new PossibleActiveWindowChange(std::forward<Args>(args)...), default_delete);
}

template<class PossibleActiveWindowChange>
PossibleActiveWindowChangeApiPtr make_snapshot_ref(PossibleActiveWindowChange & gd) {
    return PossibleActiveWindowChangeApiPtr(&gd, no_delete);
}

}

#endif
