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

#include "acl/kvlist_buffer.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"


char* KVListBuffer::Event::keys_values()
{
    return reinterpret_cast<char*>(this) + sizeof(*this);
}

KVListBuffer::KVListBuffer()
{
    events_.reserve(8);
}

void KVListBuffer::append(timeval time, LogId id, KVList kv_list)
{
    assert(kv_list.size() <= maximal_nb_key_value);

    int n = 0;
    std::array<int, maximal_nb_key_value*2> string_sizes;
    auto p = string_sizes.begin();
    for (auto const& kv : kv_list) {
        n += int(kv.key.size()) + int(kv.value.size());
        *p++ = int(kv.key.size());
        *p++ = int(kv.value.size());
    }

    void* data = ::operator new(sizeof(Event) + n); /*NOLINT*/
    Event* event = new(data) Event{id, checked_int{kv_list.size()}, time, string_sizes}; /*NOLINT*/

    char* s = event->keys_values();
    auto push_s = [&s](chars_view chars) {
        memcpy(s, chars.data(), chars.size());
        s += chars.size();
    };
    for (auto const& kv : kv_list) {
        push_s(kv.key);
        push_s(kv.value);
    }

    events_.push_back(std::unique_ptr<Event, EventDeleter>(event));
}

void KVListBuffer::clear()
{
    events_.clear();
}

void KVListBuffer::EventDeleter::operator()(KVListBuffer::Event* event) noexcept
{
    event->~Event();
    ::operator delete(event);
}

KVListBuffer::KVListIterator KVListBuffer::begin() const
{
    return {events_.begin()};
}

KVListBuffer::KVListIterator KVListBuffer::end() const
{
    return {events_.end()};
}

KVListBuffer::KVListIterator::KVListIterator(const EventContainer::const_iterator& iterator)
: iterator_(iterator)
{}

KVListBuffer::KVListIterator & KVListBuffer::KVListIterator::operator++()
{
    ++iterator_;
    return *this;
}

bool KVListBuffer::KVListIterator::operator==(const KVListBuffer::KVListIterator& other)
{
    return iterator_ == other.iterator_;
}

bool KVListBuffer::KVListIterator::operator!=(const KVListBuffer::KVListIterator& other)
{
    return !(*this == other);
}

KVListBuffer::KVEvent KVListBuffer::KVListIterator::operator*()
{
    auto& event = *iterator_->get();
    auto* p = kv_logs;
    auto* s = event.keys_values();
    auto* n = event.string_sizes.data();
    auto extract_chars = [&s, &n]{
        chars_view chars {s, s + *n};
        s += *n;
        ++n;
        return chars;
    };
    for (int i = 0; i < event.nb_kv_log; ++i) {
        chars_view key = extract_chars();
        chars_view value = extract_chars();
        *p = {key, value};
        ++p;
    }

    return {event.id, event.time, array_view{kv_logs, p}};
}
