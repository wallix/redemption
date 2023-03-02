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
    // template<class SplitterView>
    class split_view_end_iterator
    {};

    template<class SplitterView>
    struct split_view_iterator
    {
    public:
        using value_type = typename SplitterView::value_type;

        explicit split_view_iterator(SplitterView & splitter)
        : splitter(&splitter)
        , has_value(splitter.next())
        {
            if (has_value) {
                av = splitter.value();
            }
        }

        explicit split_view_iterator(split_view_end_iterator /*end*/)
        : splitter(nullptr)
        {}

        split_view_iterator& operator++()
        {
            has_value = this->splitter->next();
            av = this->splitter->value();
            return *this;
        }

        const value_type& operator*() const
        {
            return av;
        }

        const value_type* operator->() const
        {
            return &av;
        }

        // should be operator==(split_view_end_iterator), but does not work with gcc-8.0
        // bool operator==(split_view_end_iterator<SplitterView> const & /*other*/) const
        // {
        //     return splitter->empty();
        // }

        bool operator==([[maybe_unused]] split_view_iterator const & other) const
        {
            assert(!other.splitter && splitter);
            return !has_value;
        }

        bool operator!=(split_view_iterator const & other) const
        {
            return !operator==(other);
        }

    private:
        SplitterView* splitter;
        bool has_value;
        value_type av;
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
        using value_type = AV;

        SplitterCView(Data data, int sep) noexcept
        : data_(data)
        , has_value_(*data.str)
        , sep_(sep)
        {}

        bool next() noexcept
        {
            if (REDEMPTION_UNLIKELY(!has_value_)) {
                return false;
            }

            data_.str = cur_;
            cur_ = strchr_impl(cur_, sep_);
            if (cur_) {
                end_av_ = cur_;
                ++cur_;
            }
            else {
                has_value_ = false;
                cur_ = data_.end_ptr();
                end_av_ = cur_;
            }
            return true;
        }

        value_type value() const
        {
            return AV{data_.str, end_av_};
        }

        split_view_iterator<SplitterCView> begin()
        {
            return split_view_iterator<SplitterCView>(*this);
        }

        // should be detail::split_view_end_iterator<SplitterView> end(), but does not work with gcc-8.0
        split_view_iterator<SplitterCView> end()
        {
            return split_view_iterator<SplitterCView>{detail::split_view_end_iterator()};
        }

    private:
        Data data_;
        typename Data::pointer cur_ = data_.str;
        typename Data::pointer end_av_;
        bool has_value_;
        int sep_;
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
} // namespace detail

template<class AV, class Sep = typename AV::value_type, bool = is_null_terminated_v<AV>>
struct SplitterView
{
    using value_type = AV;

    template<class TSep>
    SplitterView(AV av, TSep&& sep)
    : cur_(av.begin())
    , last_(av.end())
    , has_value_(cur_ != last_)
    , sep_(static_cast<TSep&&>(sep))
    {}

    bool next()
    {
        if (REDEMPTION_UNLIKELY(!has_value_)) {
            return false;
        }

        auto first = cur_;
        while (cur_ != last_ && !bool(sep_ == *cur_)) {
            ++cur_;
        }

        av_ = AV{first, cur_};

        if (cur_ != last_) {
            ++cur_;
        }
        else {
            has_value_ = false;
        }

        return true;
    }

    value_type value() const
    {
        return av_;
    }

    detail::split_view_iterator<SplitterView> begin()
    {
        return detail::split_view_iterator<SplitterView>(*this);
    }

    // should be detail::split_view_end_iterator<SplitterView> end(), but does not work with gcc-8.0
    detail::split_view_iterator<SplitterView> end()
    {
        return detail::split_view_iterator<SplitterView>{detail::split_view_end_iterator()};
    }

private:
    typename AV::iterator cur_;
    typename AV::iterator last_;
    AV av_;
    bool has_value_;
    Sep sep_;
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
} // namespace detail

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

template<class Chars, class Sep = char>
auto get_lines(Chars&& chars, Sep&& sep = '\n') /*NOLINT*/
{
    return split_with(static_cast<Chars&&>(chars), static_cast<Sep&&>(sep));
}
