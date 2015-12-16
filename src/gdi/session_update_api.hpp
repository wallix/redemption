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

#ifndef REDEMPTION_GDI_SESSION_UPDATE_API_HPP
#define REDEMPTION_GDI_SESSION_UPDATE_API_HPP

#include "utils/virtual_deleter.hpp"

#include "array_view.hpp"
#include "noncopyable.hpp"

namespace gdi {

struct SessionUpdateApi : private noncopyable
{
    virtual ~SessionUpdateApi() = default;

    virtual void session_update(const timeval & now, array_view<const char> const & message) = 0;
};

using SessionUpdateApiDeleterBase = utils::virtual_deleter_base<SessionUpdateApi>;
using SessionUpdateApiPtr = utils::unique_ptr_with_virtual_deleter<SessionUpdateApi>;

using utils::default_delete;
using utils::no_delete;

template<class SessionUpdate, class... Args>
SessionUpdateApiPtr make_session_update_ptr(Args && ... args) {
    return SessionUpdateApiPtr(new SessionUpdate(std::forward<Args>(args)...), default_delete);
}

template<class SessionUpdate>
SessionUpdateApiPtr make_session_update_ref(SessionUpdate & gd) {
    return SessionUpdateApiPtr(&gd, no_delete);
}

}

#endif
