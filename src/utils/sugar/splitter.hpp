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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#pragma once

#include "utils/sugar/array_view.hpp"

#include <type_traits>


template<class AV, class Sep = typename AV::value_type>
struct GetLineView
{
    template<class TSep>
    GetLineView(AV av, TSep&& sep)
    : first(av.begin())
    , last(av.end())
    , sep(static_cast<TSep&&>(sep))
    {}

    AV next()
    {
        first = cur;
        while (cur != last && !bool(sep == *cur)) {
            ++cur;
        }
        AV res{first, cur};
        if (cur != last) {
            ++cur;
        }
        return res;
    }

    [[nodiscard]] bool empty() const {
        return first == last;
    }

    class iterator
    {
        GetLineView & lines;
        AV av;

        friend class GetLineView;

        explicit iterator(GetLineView & s)
        : lines(s)
        , av(s.next())
        {}

        explicit iterator(GetLineView & s, int /*unused*/)
        : lines(s)
        {}

    public:
        iterator& operator++()
        {
            av = this->lines.next();
            return *this;
        }

        const AV& operator*() const
        {
            return av;
        }

        const AV* operator->() const
        {
            return &av;
        }

        bool operator==(iterator const & other) const noexcept
        {
            return lines.first == other.lines.last;
        }

        bool operator!=(iterator const & other) const noexcept
        {
            return !operator==(other);
        }
    };

    iterator begin()
    {
        return iterator(*this);
    }

    iterator end()
    {
        return iterator(*this, 1);
    }

private:
    typename AV::iterator first;
    typename AV::iterator last;
    typename AV::iterator cur = first;
    Sep sep;
};


template<class Sep>
auto split_with(chars_view s, Sep&& sep)
-> GetLineView<chars_view, std::enable_if_t<sizeof(sep == 'a'), std::decay_t<Sep>>>
{
    return {s, sep};
}

template<class Sep>
auto split_with(writable_chars_view s, Sep&& sep)
-> GetLineView<writable_chars_view, std::enable_if_t<sizeof(sep == 'a'), std::decay_t<Sep>>>
{
    return {s, sep};
}

inline GetLineView<chars_view> get_lines(chars_view s, char sep = '\n') /*NOLINT*/
{
    return {s, sep};
}

inline GetLineView<writable_chars_view> get_lines(writable_chars_view s, char sep = '\n') /*NOLINT*/
{
    return {s, sep};
}
