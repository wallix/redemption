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


class KVListBuffer
{
    static const std::size_t maximal_nb_key_value = 256;

    struct Event
    {
        LogId id;
        int nb_kv_log;
        timeval time;

        // lenght of each key/value
        std::array<int, maximal_nb_key_value*2> string_sizes;
        // all key/value in KVList

        char* keys_values();
    };

    struct EventDeleter
    {
        void operator()(Event* event) noexcept;
    };

    using EventContainer = std::vector<std::unique_ptr<Event, EventDeleter>>;
    EventContainer events_;

public:
    struct KVEvent
    {
        LogId id;
        timeval time;
        KVList kv_list;
    };

    struct KVListIterator
    {
        KVListIterator& operator++();
        KVEvent operator*();
        bool operator==(KVListIterator const& other);
        bool operator!=(KVListIterator const& other);

    private:
        EventContainer::const_iterator iterator_;

        friend class KVListBuffer;
        KVListIterator(EventContainer::const_iterator const& iterator);

        KVLog kv_logs[maximal_nb_key_value];
    };

    KVListBuffer();

    void append(timeval time, LogId id, KVList kv_list);
    void clear();

    KVListIterator begin() const;
    KVListIterator end() const;
};
