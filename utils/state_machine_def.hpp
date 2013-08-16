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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
 */

#include <cctype>

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/push_back.hpp>

namespace tokens
{
    struct StringLower {
        const char * operator()(const char * s) const
        {
            while (*s && std::islower(*s)) {
                ++s;
            }
            return s;
        }
    };

    struct Name {
        const char * operator()(const char * s) const
        {
            if (std::isupper(*s)) {
                while (*++s && std::isalnum(*s)) {
                }
            }
            return s;
        }
    };

    struct Integer {
        const char * operator()(const char * s) const
        {
            while (*s == '-' || *s == '+') {
                ++s;
            }
            while (*s && std::isdigit(*s)) {
                ++s;
            }
            return s;
        }
    };

    struct HexColor {
        const char * operator()(const char * s) const
        {
            const char * begin = s;
            while (*s && std::isxdigit(*s)) {
                ++s;
            }
            return (s - begin == 3 || s - begin == 6) ? s : begin;
        }
    };

    struct MathOperator {
        const char * operator()(const char * s) const
        {
            if (*s == '/' || *s == '*' || *s == '-' || *s == '+' || *s == '%') {
                ++s;
            }
            return s;
        }
    };

    template<char c>
    struct CharSeparator {
        const char * operator()(const char * s) const
        {
            if (*s == c) {
                ++s;
            }
            return s;
        }
    };

    typedef CharSeparator<'.'> Dot;
    typedef CharSeparator<'{'> OpenBlock;
    typedef CharSeparator<'}'> CloseBlock;
    typedef CharSeparator<';'> Semicolon;
    typedef CharSeparator<'('> OpenParenthesis;
    typedef CharSeparator<')'> CloseParenthesis;
    typedef CharSeparator<'"'> Quote;
    typedef CharSeparator<':'> TwoPoint;
    typedef CharSeparator<','> Comma;
    typedef CharSeparator<'#'> Pound;
    typedef CharSeparator<' '> Space;
    typedef CharSeparator<'\n'> Newline;

    template<char c1, char c2>
    struct DoubleCharSeparator {
        const char * operator()(const char * s) const
        {
            if (*s == c1 && *(s+1) == c2) {
                s += 2;
            }
            return s;
        }
    };

    const char * contains_with_delimiter(const char * s, char c, char antic = '\\')
    {
        if (*s && *s != c) {
            ++s;
            while (*s && *s != c) {
                if (*s == antic && *(s+1)) {
                    ++s;
                }
                ++s;
            }
        }
        return s;
    }

    template<char Delimiter, char AntiDelimiter = '\\'>
    struct ContainsWithDelimiter {
        const char * operator()(const char * s) const
        {
            return contains_with_delimiter(Delimiter, s, AntiDelimiter);
        }
    };

    typedef ContainsWithDelimiter<'"'> String;

    template<typename Consumer>
    struct FullConsumer {
        const char * operator()(const char * s) const
        {
            const char * p = Consumer()(s);
            while (p != s) {
                s = p;
                p = Consumer()(p);
            }
            return p;
        }
    };

    typedef FullConsumer<Space> Spaces;

    template<typename Consumer1, typename Consumer2>
    struct DoubleConsumer {
        const char * operator()(const char * s) const
        {
            const char * p = Consumer1()(s);
            if (p == s) {
                p = Consumer2()(s);
            }
            return p;
        }
    };

    struct Blank {
        const char * operator()(const char * s) const
        {
            while (*s == '\t' || *s == ' ' || *s == '\n') {
                ++s;
            }
            return s;
        }
    };
}


using std::size_t;

template<typename T, typename Default>
struct type_or
{ typedef T type; };

template<typename Default>
struct type_or<void, Default>
{ typedef Default type; };

template<typename T, typename U>
struct is_same
{ static const bool value = false; };

template<typename T>
struct is_same<T, T>
{ static const bool value = true; };

template<bool, typename T, typename U>
struct if_c
{ typedef T type; };

template<typename T, typename U>
struct if_c<false, T, U>
{ typedef U type; };


template<size_t N, size_t Max,
    typename T, typename TableTransition, typename RangeTransition, bool = false>
struct state_machine_index_transition
{
    typedef typename boost::mpl::at<
        RangeTransition,
        boost::mpl::int_<N>
    >::type range;

    typedef typename boost::mpl::at<
        TableTransition,
        boost::mpl::int_<range::begin>
    >::type row;

    static const unsigned value = state_machine_index_transition<
        N+1,
        Max,
        T,
        TableTransition,
        RangeTransition,
        boost::is_same<typename row::event, T>::value
    >::value;
};

template<size_t N, size_t Max,
typename T, typename TableTransition, typename RangeTransition>
struct state_machine_index_transition<N, Max, T, TableTransition, RangeTransition, true>
{
    static const unsigned value = N-1;
};

template<size_t Max,
typename T, typename TableTransition, typename RangeTransition>
struct state_machine_index_transition<Max, Max, T, TableTransition, RangeTransition, true>
{
    static const unsigned value = Max-1;
};


template<size_t N, size_t Max, typename TableTransition, typename RangeTransition>
struct state_machine_transition
{
    template<typename SM, typename Event>
    static size_t impl(SM& sm, const char *& s, const Event& event)
    {
        typedef typename boost::mpl::at<
            TableTransition,
            boost::mpl::int_<N>
        >::type row;
        typedef typename row::check check;
        typedef typename row::base_check base_check;
        typedef typename row::base_consumer base_consumer;
        typedef typename row::consumer consumer;
        typedef typename row::action action;

        const char * p = boost::is_same<base_check, void>::value ? s : check()(sm, s);

        if (boost::is_same<base_check, void>::value || p != s) {
            if (boost::is_same<consumer, check>::value) {
                action()(sm, event, s, p);
                if (!boost::is_same<base_consumer, void>::value) {
                    s = p;
                }
            }
            else {
                s = consumer()(sm, p);
                action()(sm, event, p, s);
            }


            if (!boost::is_same<typename row::base_ignore, void>::value) {
                typedef typename row::ignore ignore;
                s = ignore()(static_cast<typename SM::derived_type&>(sm), s);
            }

            return state_machine_index_transition<
                0,
                boost::mpl::size<RangeTransition>::value,
                typename row::next,
                TableTransition,
                RangeTransition
            >::value;
        }

        return state_machine_transition<
            N+1,
            Max,
            TableTransition,
            RangeTransition
        >::impl(sm, s, event);
    }

    template<typename SM>
    static const char * impl(SM& sm, const char * s, size_t idx)
    {
        if (N == idx) {
            typedef  typename boost::mpl::at<
                RangeTransition,
                boost::mpl::int_<N>
            >::type range;

            typedef typename boost::mpl::at<
                TableTransition,
                boost::mpl::int_<range::begin>
            >::type row;

            typedef typename row::event event;

            return sm.transition(s, event());
        }
        return state_machine_transition<
            N+1,
            Max,
            TableTransition,
            RangeTransition
        >::impl(sm, s, idx);
    }
};

template<size_t N, typename TableTransition, typename RangeTransition>
struct state_machine_transition<N,N,TableTransition, RangeTransition>
{
    template<typename SM, typename Event>
    static size_t impl(SM& sm, const char * s, const Event& event)
    {
        static_cast<typename SM::derived_type&>(sm).error(s, event);
        return -1u;
    }

    template<typename SM>
    static const char * impl(SM& sm, const char * s, size_t)
    {
        typedef typename SM::null_transition null_transition;
        static_cast<typename SM::derived_type&>(sm).error(s, null_transition());
        return s;
    }
};


template<unsigned N1, unsigned N2>
struct state_machine_range
{
    static const std::size_t begin = N1;
    static const std::size_t end = N2;
};


template<typename Base, size_t P, size_t N, size_t Max, typename TableTransition>
class state_machine_defined_range_dispatch;

template<typename Base, size_t P, size_t N, size_t Max, typename TableTransition,
    typename U = typename boost::mpl::at<TableTransition, boost::mpl::int_<P> >::type::event,
    bool is_same = boost::is_same<
        typename boost::mpl::at<TableTransition, boost::mpl::int_<N> >::type::event,
        U
    >::value
>
struct state_machine_defined_range
: state_machine_defined_range_dispatch<Base, P, N+1, Max, TableTransition>::type
{};

template<typename Base, size_t P, size_t N, size_t Max, typename TableTransition>
struct state_machine_defined_range_dispatch
: state_machine_defined_range<Base, P, N, Max, TableTransition>::type
{};

template<typename Base, size_t P, size_t Max, typename TableTransition>
struct state_machine_defined_range_dispatch<Base, P, Max, Max, TableTransition>
{
    typedef typename boost::mpl::push_back<Base, state_machine_range<P, Max> >::type type;
};

template<typename Base, size_t P, size_t N, size_t Max, typename TableTransition, typename U>
struct state_machine_defined_range<Base, P, N, Max, TableTransition, U, false>
: state_machine_defined_range_dispatch<
    typename boost::mpl::push_back<Base, state_machine_range<P, N> >::type,
    N, N+1, Max, TableTransition
>::type
{};


template<typename Derived>
class state_machine_def
{
public:
    typedef Derived derived_type;

    state_machine_def()
    : state_num(defined_index_transition<
    typename Derived::table_transition,
    typename Derived::start_event
    >::value)
    , state_error(-1u)
    {}

    template<typename Event>
    void error(const char *, const Event&)
    {
        this->state_error = this->state_num;
    }

    const char * start(const char * s)
    {
        typedef typename Derived::start_event event_t;
        return transition(s, event_t());
    }

    struct null_transition {};

    struct null_check {
        const char * operator()(state_machine_def&, const char * s) const
        { return s; }
    };

    typedef null_check null_consumer;

    typedef null_consumer default_blank;


private:
    template<typename TableTransition>
    struct defined_range_transition
    : state_machine_defined_range<
        boost::mpl::vector<>,
        0,
        1,
        boost::mpl::size<TableTransition>::value,
        TableTransition
    >::type
    {};

    template<typename TableTransition, typename Event>
    struct defined_index_transition
    {
        static const size_t value = state_machine_index_transition<
            0,
            boost::mpl::size<TableTransition>::value,
            Event,
            TableTransition,
            defined_range_transition<TableTransition>
        >::value;
    };

protected:
    template<typename Functor>
    struct consumer_functor {
        const char * operator()(state_machine_def& /*sm*/, const char * s)
        {
            return Functor()(s);
        }
    };

    template<const char * (Derived::*Check)(const char *)>
    struct consumer_mem {
        const char * operator()(state_machine_def& sm, const char * s)
        {
            return (static_cast<Derived&>(sm).*Check)(s);
        }
    };

    class unspecified_blank;

    template<
        typename Start,
        typename Check,
        typename Next,
        typename Consumer = Check,
        typename Ignore = unspecified_blank
    >
    struct a_row
    {
        typedef Start event;
        typedef typename type_or<Check, null_check>::type check;
        typedef Next next;
        typedef Check base_check;
        typedef Consumer base_consumer;
        typedef Ignore base_ignore;
        typedef typename type_or<Consumer, null_consumer>::type consumer;
        typedef typename type_or<
            typename if_c<
                is_same<Ignore, unspecified_blank>::value,
                typename Derived::default_blank,
                Ignore
            >::type,
            null_consumer
        >::type ignore;
        typedef void (Derived::*action_type)(const Start&);

        struct action
        {
            void operator()(state_machine_def& /*sm*/, const Start& /*event*/,
                            const char * /*b*/, const char * /*e*/)
            {}
        };
    };

    template<
        typename Start,
        typename Check,
        typename Next,
        typename Consumer,
        void (Derived::*Action)(const Start &, const char *, const char *),
        typename Ignore = unspecified_blank
    >
    struct row
    : a_row<Start, Check, Next, Consumer, Ignore>
    {
        struct action
        {
            void operator()(state_machine_def& sm, const Start& event,
                            const char * b, const char * e)
            {
                (static_cast<Derived&>(sm).*Action)(event, b, e);
            }
        };
    };

    template<
        typename Start,
        typename CheckAndConsumer,
        typename Next,
        void (Derived::*Action)(const Start &, const char *, const char *),
        typename Ignore = unspecified_blank
    >
    struct c_row
    : row<Start, CheckAndConsumer, Next, CheckAndConsumer, Action, Ignore>
    {};

    template<
        typename Start,
        typename Next,
        typename Ignore = null_consumer
    >
    struct t_row
    : a_row<Start, void, Next, void, Ignore>
    {};

public:
    template<typename Event>
    const char * transition(const char * s, const Event& event)
    {
        typedef typename Derived::table_transition table_transition;

        typedef defined_range_transition<table_transition> range_transition;

        typedef typename boost::mpl::at<
            range_transition,
            boost::mpl::int_<defined_index_transition<
                table_transition,
                Event
            >::value>
        >::type range_type;

        this->state_num = state_machine_transition<
            range_type::begin,
            range_type::end,
            table_transition,
            range_transition
        >::impl(*this, s, event);
        return s;
    }

    const char * next_event(const char * s)
    {
        typedef typename Derived::table_transition table_transition;

        typedef defined_range_transition<table_transition> range_transition;

        return state_machine_transition<
            0,
            boost::mpl::size<range_transition>::value,
            table_transition,
            range_transition
        >::impl(*this, s, this->state_num);
    }

    bool valid() const
    {
        return -1u == this->state_error;
    }

    ///\return  Event error. -1u if no error
    size_t error() const
    {
        return this->state_error;
    }

private:
    size_t state_num;
    size_t state_error;
};

