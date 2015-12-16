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

#ifndef REDEMPTION_GDI_SNAPSHOT_API_HPP
#define REDEMPTION_GDI_SNAPSHOT_API_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"

struct timeval;
class wait_obj;

namespace gdi {

struct SnapshotApi : private noncopyable
{
    virtual ~SnapshotApi() = default;

    virtual void snapshot(
        const timeval & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval,
        wait_obj & capture_event
    ) = 0;
};

using SnapshotApiDeleterBase = utils::virtual_deleter_base<SnapshotApi>;
using SnapshotApiPtr = utils::unique_ptr_with_virtual_deleter<SnapshotApi>;

using utils::default_delete;
using utils::no_delete;

template<class Snapshot, class... Args>
SnapshotApiPtr make_snapshot_ptr(Args && ... args) {
    return SnapshotApiPtr(new Snapshot(std::forward<Args>(args)...), default_delete);
}

template<class Snapshot>
SnapshotApiPtr make_snapshot_ref(Snapshot & gd) {
    return SnapshotApiPtr(&gd, no_delete);
}

}

#endif
