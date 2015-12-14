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

#ifndef REDEMPTION_GDI_UPDATE_POINTER_POSITION_API_HPP
#define REDEMPTION_GDI_UPDATE_POINTER_POSITION_API_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"


namespace gdi {

struct UpdatePointerPositionApi : private noncopyable
{
    virtual ~UpdatePointerPositionApi() = default;

    virtual void update_pointer_position(uint16_t xPos, uint16_t yPos) = 0;
};

using UpdatePointerPositionApiDeleterBase = utils::virtual_deleter_base<UpdatePointerPositionApi>;
using UpdatePointerPositionApiPtr = utils::unique_ptr_with_virtual_deleter<UpdatePointerPositionApi>;

using utils::default_delete;
using utils::no_delete;

template<class UpdatePointerPosition, class... Args>
UpdatePointerPositionApiPtr make_update_pointer_position_ptr(Args && ... args) {
    return UpdatePointerPositionApiPtr(new UpdatePointerPosition(std::forward<Args>(args)...), default_delete);
}

template<class UpdatePointerPosition>
UpdatePointerPositionApiPtr make_update_pointer_position_ref(UpdatePointerPosition & gd) {
    return UpdatePointerPositionApiPtr(&gd, no_delete);
}

}

#endif
