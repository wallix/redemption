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

#ifndef REDEMPTION_GDI_SERVER_SET_POINTER_API_HPP
#define REDEMPTION_GDI_SERVER_SET_POINTER_API_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"

class Pointer;

namespace gdi {

struct ServerSetPointerApi : private noncopyable
{
    virtual ~ServerSetPointerApi() = default;

    virtual void server_set_pointer(const Pointer & cursor) = 0;
};

using ServerSetPointerApiDeleterBase = utils::virtual_deleter_base<ServerSetPointerApi>;
using ServerSetPointerApiPtr = utils::unique_ptr_with_virtual_deleter<ServerSetPointerApi>;

using utils::default_delete;
using utils::no_delete;

template<class ServerSetPointer, class... Args>
ServerSetPointerApiPtr make_server_set_pointer_ptr(Args && ... args) {
    return ServerSetPointerApiPtr(new ServerSetPointer(std::forward<Args>(args)...), default_delete);
}

template<class ServerSetPointer>
ServerSetPointerApiPtr make_server_set_pointer_ref(ServerSetPointer & gd) {
    return ServerSetPointerApiPtr(&gd, no_delete);
}

}

#endif
