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
    static const std::size_t maximal_nb_key_value = 8;

    struct Event
    {
        LogId id;
        int nb_kv_log;
        timeval time;

        // lenght of each key/value
        std::array<int, maximal_nb_key_value*2> string_sizes;
        // all key/value in KVLogList

        char* keys_values();
    };

    struct EventDeleter
    {
        void operator()(Event* event) noexcept;
    };

    using EventContainer = std::vector<std::unique_ptr<Event, EventDeleter>>;
    EventContainer events_;

public:
    struct Data
    {
        LogId id;
        timeval time;
        KVLogList kv_list;
    };

    struct DataIterator
    {
        DataIterator& operator++();
        Data operator*();
        bool operator==(DataIterator const& other);
        bool operator!=(DataIterator const& other);

    private:
        EventContainer::const_iterator iterator_;

        friend class SessionUpdateBuffer;
        DataIterator(EventContainer::const_iterator const& iterator);

        KVLog kv_logs[maximal_nb_key_value];
    };

    SessionUpdateBuffer();

    void append(timeval time, LogId id, KVLogList kv_list);
    void clear();

    std::size_t empty() const;

    DataIterator begin() const;
    DataIterator end() const;
};
