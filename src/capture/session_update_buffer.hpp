/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#pragma once

#include "acl/auth_api.hpp"
#include "core/log_id.hpp"

#include <vector>
#include <memory>
#include <array>

#include <sys/time.h> // timeval


class SessionUpdateBuffer
{
    struct Event
    {
        LogId id;
        timeval time;
        KVLogList kv_list;

        class Access;
    };

    struct EventDeleter
    {
        void operator()(Event* event) noexcept;
    };

    using EventContainer = std::vector<std::unique_ptr<Event, EventDeleter>>;
    EventContainer events_;

public:
    using Data = Event;

    struct DataIterator
    {
        DataIterator& operator++() noexcept;
        Data const& operator*() noexcept;
        bool operator==(DataIterator const& other) noexcept;
        bool operator!=(DataIterator const& other) noexcept;

    private:
        EventContainer::const_iterator iterator_;

        friend class SessionUpdateBuffer;
        DataIterator(EventContainer::const_iterator const& iterator) noexcept;
    };

    SessionUpdateBuffer();

    void append(timeval time, LogId id, KVLogList kv_list);
    void clear();

    std::size_t empty() const;

    DataIterator begin() const;
    DataIterator end() const;
};
