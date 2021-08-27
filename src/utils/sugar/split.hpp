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
#include "utils/sugar/cast.hpp"
#include "utils/traits/is_null_terminated.hpp"

#include <type_traits>
#include <cstring>


namespace detail
{
    template<class SplitterView>
    class end_iterator
    {};

    template<class SplitterView>
    struct iterator
    {
    public:
        using element_view = typename SplitterView::element_view;

        explicit iterator(SplitterView & s)
        : lines(&s)
        , av(s.next())
        {}

        iterator& operator++()
        {
            av = this->lines->next();
            return *this;
        }

        const element_view& operator*() const
        {
            return av;
        }

        const element_view* operator->() const
        {
            return &av;
        }

        bool operator==(end_iterator<SplitterView> const & /*other*/) const
        {
            return lines->empty();
        }

        bool operator!=(iterator const & other) const
        {
            return !operator==(other);
        }

    private:
        SplitterView* lines;
        element_view av;
    };

    inline char* strchr_impl(char* s, int c) noexcept
    {
        return std::strchr(s, c);
    }

    inline char const* strchr_impl(char const* s, int c) noexcept
    {
        return std::strchr(s, c);
    }

    inline unsigned char* strchr_impl(unsigned char* s, int c) noexcept
    {
        return byte_ptr_cast(std::strchr(char_ptr_cast(s), c));
    }

    inline unsigned char const* strchr_impl(unsigned char const* s, int c) noexcept
    {
        return byte_ptr_cast(std::strchr(char_ptr_cast(s), c));
    }

    template<class Data, class AV>
    struct SplitterCView
    {
        using element_view = AV;

        SplitterCView(Data data, int sep) noexcept
        : data(data)
        , sep(sep)
        {}

        AV next() noexcept
        {
            data.str = cur;
            cur = strchr_impl(cur, sep);
            if (cur) {
                AV res{data.str, cur};
                ++cur;
                return res;
            }
            AV res{data.str, data.end_ptr()};
            cur = res.end();
            return res;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return !*data.str;
        }

        iterator<SplitterCView> begin()
        {
            return iterator<SplitterCView>(*this);
        }

        end_iterator<SplitterCView> end()
        {
            return {};
        }

    private:
        Data data;
        typename Data::pointer cur = data.str;
        int sep;
    };

    template<class Ptr>
    struct SplitterCViewDataStr
    {
        using pointer = Ptr;

        Ptr end_ptr() const
        {
            return str + strlen(str);
        }
        Ptr str;
    };

    template<class Ptr>
    struct SplitterCViewDataView
    {
        using pointer = Ptr;

        Ptr end_ptr() const
        {
            return end;
        }
        Ptr str;
        Ptr end;
    };
}

template<class AV, class Sep = typename AV::value_type, bool = is_null_terminated_v<AV>>
struct SplitterView
{
    using element_view = AV;

    template<class TSep>
    SplitterView(AV av, TSep&& sep)
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

    [[nodiscard]] bool empty() const
    {
        return first == last;
    }

    detail::iterator<SplitterView> begin()
    {
        return detail::iterator<SplitterView>(*this);
    }

    detail::end_iterator<SplitterView> end()
    {
        return {};
    }

private:
    typename AV::iterator first;
    typename AV::iterator last;
    typename AV::iterator cur = first;
    Sep sep;
};


namespace detail
{
    template<class> struct split_with_char_sep : std::false_type {};
    template<> struct split_with_char_sep<char> : std::true_type {};
    template<> struct split_with_char_sep<unsigned char> : std::true_type {};

    template<class> struct split_with_chars_ptr : std::false_type {};
    template<> struct split_with_chars_ptr<char*> : std::true_type
    {
        using view = writable_chars_view;
    };
    template<> struct split_with_chars_ptr<char const*> : std::true_type
    {
        using view = chars_view;
    };
    template<> struct split_with_chars_ptr<unsigned char*> : std::true_type
    {
        using view = writable_array_view<unsigned char*>;
    };
    template<> struct split_with_chars_ptr<unsigned char const*> : std::true_type
    {
        using view = array_view<unsigned char*>;
    };

    inline int normalize_split_with_sep_char(char c) noexcept
    {
        return static_cast<unsigned char>(c);
    }

    inline int normalize_split_with_sep_char(unsigned char c) noexcept
    {
        return c;
    }
}

/// \brief Returns a view which allows to loop on strings separated by a character.
/// \note There is no result when the separator is on the last character
/// \code
/// split_with(",a,b,", ',') -> "", "a", "b"
/// \endcode
template<class Chars, class Sep>
auto split_with(Chars&& chars, Sep&& sep)
{
    using Chars2 = std::decay_t<Chars>;
    using Sep2 = std::decay_t<Sep>;
    using IsCharPtr = detail::split_with_chars_ptr<Chars2>;
    using IsCharSep = detail::split_with_char_sep<Sep2>;

    if constexpr (IsCharPtr::value && IsCharSep::value) {
        using CharsView = typename IsCharPtr::view;
        using Data = detail::SplitterCViewDataStr<Chars2>;
        return detail::SplitterCView<Data, CharsView>(
            Data{chars}, detail::normalize_split_with_sep_char(sep)
        );
    }
    else {
        using CharsView = std::conditional_t<
            std::is_constructible_v<writable_chars_view, Chars&>,
            writable_chars_view,
            chars_view>;
        if constexpr (IsCharSep::value && is_null_terminated_v<Chars2>) {
            CharsView av(chars);
            using Data = detail::SplitterCViewDataView<typename CharsView::pointer>;
            return detail::SplitterCView<Data, CharsView>(
                Data{av.data(), av.data() + av.size()},
                detail::normalize_split_with_sep_char(sep)
            );
        }
        else {
            return SplitterView<CharsView, Sep2>(CharsView(chars), static_cast<Sep&&>(sep));
        }
    }
}

template<class Chars, class Sep>
auto get_lines(Chars&& chars, Sep&& sep = '\n') /*NOLINT*/
{
    return split_with(static_cast<Chars&&>(chars), static_cast<Sep&&>(sep));
}
