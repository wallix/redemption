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

#include "capture/session_update_buffer.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <cstring>

struct SessionUpdateBuffer::Event::Access
{
    static KVLog* kv_logs(SessionUpdateBuffer::Event* ev)
    {
        return reinterpret_cast<KVLog*>(ptr_kv_logs(ev));
    }

    static int* string_sizes(SessionUpdateBuffer::Event* ev)
    {
        return reinterpret_cast<int*>(ptr_sizes(ev));
    }

    static char* keys_values(SessionUpdateBuffer::Event* ev)
    {
        return ptr_values(ev);
    }

private:
    static char* ptr_kv_logs(SessionUpdateBuffer::Event* ev)
    {
        return reinterpret_cast<char*>(ev) + sizeof(*ev) + sizeof(Event) % sizeof(KVLog);
    }

    static char* ptr_sizes(SessionUpdateBuffer::Event* ev)
    {
        return ptr_kv_logs(ev) + sizeof(KVLog) * ev->kv_list.size();
    }

    static char* ptr_values(SessionUpdateBuffer::Event* ev)
    {
        return ptr_sizes(ev) + 2 * sizeof(int) * ev->kv_list.size();
    }
};

SessionUpdateBuffer::SessionUpdateBuffer()
{
    events_.reserve(8);
}

void SessionUpdateBuffer::append(MonotonicTimePoint time, LogId id, KVLogList kv_list)
{
    int n = 0;
    for (auto const& kv : kv_list) {
        n += int(kv.key.size()) + int(kv.value.size());
    }

    static_assert(alignof(Event) >= alignof(int));

    void* data = ::operator new( /*NOLINT*/
        sizeof(Event)
      + sizeof(Event) % sizeof(KVLog)    // padding
      + sizeof(KVLog) * kv_list.size()   // kv_logs
      + 2 * sizeof(int) * kv_list.size() // string_sizes
      + size_t(n));                      // key/value buffers
    Event* event = new(data) Event{id, time, kv_list}; /*NOLINT*/

    KVLog* logs = Event::Access::kv_logs(event);
    char* s = Event::Access::keys_values(event);
    int* sizes = Event::Access::string_sizes(event);

    event->kv_list = KVLogList{{logs, kv_list.size()}};

    auto push_s = [&s](chars_view chars) {
        auto r = s;
        std::memcpy(s, chars.data(), chars.size());
        s += chars.size();
        return chars_view{r, chars.size()};
    };

    for (auto const& kv : kv_list) {
        *sizes++ = int(kv.key.size());
        *sizes++ = int(kv.value.size());
        auto k = push_s(kv.key);
        auto v = push_s(kv.value);
        *logs++ = {k, v};
    }

    events_.push_back(std::unique_ptr<Event, EventDeleter>(event));
}

void SessionUpdateBuffer::clear()
{
    events_.clear();
}

void SessionUpdateBuffer::EventDeleter::operator()(SessionUpdateBuffer::Event* event) noexcept
{
    event->~Event();
    ::operator delete(event);
}

std::size_t SessionUpdateBuffer::empty() const
{
    return events_.empty();
}

SessionUpdateBuffer::DataIterator SessionUpdateBuffer::begin() const
{
    return {events_.begin()};
}

SessionUpdateBuffer::DataIterator SessionUpdateBuffer::end() const
{
    return {events_.end()};
}

SessionUpdateBuffer::DataIterator::DataIterator(const EventContainer::const_iterator& iterator) noexcept
: iterator_(iterator)
{}

SessionUpdateBuffer::DataIterator & SessionUpdateBuffer::DataIterator::operator++() noexcept
{
    ++iterator_;
    return *this;
}

bool SessionUpdateBuffer::DataIterator::operator==(const SessionUpdateBuffer::DataIterator& other) const noexcept
{
    return iterator_ == other.iterator_;
}

bool SessionUpdateBuffer::DataIterator::operator!=(const SessionUpdateBuffer::DataIterator& other) const noexcept
{
    return !(*this == other);
}

SessionUpdateBuffer::Data const& SessionUpdateBuffer::DataIterator::operator*() noexcept
{
    return **iterator_;
}
