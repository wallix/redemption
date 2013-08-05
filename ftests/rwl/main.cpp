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
#include <utility>
#include <typeinfo>
#include <iostream>
#include <iomanip>

namespace tokens {

    struct Identifier {
        const char * operator()(const char * s) const
        {
            while (*s && std::islower(*s)) {
                ++s;
            }
            return s;
        }
    };

    struct Target {
        const char * operator()(const char * s) const
        {
            if (std::isupper(*s)) {
                while (*++s && std::isalnum(*s)) {
                }
            }
            return s;
        }
    };

    typedef Target FunctionName;

    struct Integer {
        const char * operator()(const char * s) const
        {
            while (*s && std::isdigit(*s)) {
                ++s;
            }
            return s;
        }
    };

    struct HexaColor {
        const char * operator()(const char * s) const
        {
            const char * begin = s;
            while (*s && std::isxdigit(*s)) {
                ++s;
            }
            return (s - begin == 3 || s - begin == 6) ? s : begin;
        }
    };

    struct Operator {
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

    typedef CharSeparator<'.'> Linker;
    typedef CharSeparator<'{'> OpenBlock;
    typedef CharSeparator<'}'> CloseBlock;
    typedef CharSeparator<';'> Separator;
    typedef CharSeparator<'('> OpenExpression;
    typedef CharSeparator<')'> CloseExpression;
    typedef CharSeparator<'"'> StringDelimiter;
    typedef CharSeparator<':'> IdentifierDelimiter;
    typedef CharSeparator<','> Comma;
    typedef CharSeparator<'#'> DefiniedColor;
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

    typedef DoubleCharSeparator<'/', '/'> Commentaire;

    struct ContentString {
        const char * operator()(const char * s) const
        {
            if (*s && *s != '"') {
                ++s;
                while (*s && *s != '"') {
                    if (*s == '\\' && *(s+1)) {
                        ++s;
                    }
                    ++s;
                }
            }
            return s;
        }
    };

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

    struct Whitespaces {
        const char * operator()(const char * s) const
        {
            while (*s == '\n' || *s == ' ') {
                ++s;
            }
            return s;
        }
    };

}


#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>

using std::size_t;

template<typename T, typename U, typename Default>
struct default_if
{
    typedef typename boost::conditional<
        boost::is_same<T, U>::value,
        Default,
        T
    >::type type;
};



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
        typedef typename row::action action;

        const char * p = check()(s);

        if (p != s) {
            typedef typename row::consumer consumer;
            if (boost::is_same<consumer, check>::value) {
                action()(sm, event, s, p);
                s = p;
            }
            else {
                s = consumer()(p);
                action()(sm, event, p, s);
            }

            typedef typename row::ignore ignore;
            s = ignore()(s);

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

template<typename Derived>
struct state_machine_def
{
    typedef Derived derived_type;

    template<typename Event>
    void error(const char *, const Event&) const
    { std::cout << "machine error\n"; }

    struct null_check {
        const char * operator()(const char * s) const
        { return s; }
    };

    struct null_consumer {
        const char * operator()(const char * s) const
        { return s; }
    };

    struct null_transition {};

    template<
        typename Start,
        typename Check,
        typename Next,
        typename Consumer ,
        void (Derived::* Action)(const Start &, const char *, const char *),
        typename Ignore = tokens::Whitespaces
    >
    struct row {
        typedef Start event;
        typedef typename default_if<Check, void, null_check>::type check;
        typedef Next next;
        typedef typename default_if<Consumer, void, null_consumer>::type consumer;
        typedef typename default_if<Ignore, void, null_consumer>::type ignore;
        typedef void (Derived::*action_type)(const Start&);

        struct action
        {
            void operator()(state_machine_def& sm, const Start& event,
                            const char * b, const char * e)
            {
                (static_cast<Derived&>(sm).*Action)(event, b, e);
            }
        };
    };

    state_machine_def()
    : state_num(0)
    {}

    void start(const char * s)
    {
        typedef typename Derived::start_event event_t;
        transition(s, event_t());
    }

    template<typename Event>
    const char * transition(const char * s, const Event& event)
    {
        typedef typename Derived::table_transition table_transition;

        typedef typename Derived::range_transition range_transition;

        typedef typename boost::mpl::at<
            range_transition,
            boost::mpl::int_<
                state_machine_index_transition<
                    0,
                    boost::mpl::size<range_transition>::value,
                    Event,
                    table_transition,
                    range_transition
                >::value
            >
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

        typedef typename Derived::range_transition range_transition;

        using boost::mpl::int_;

        typedef typename boost::mpl::at<range_transition, int_<0> >::type range_type;

        return state_machine_transition<
            0,
            boost::mpl::size<range_transition>::value,
            table_transition,
            range_transition
        >::impl(*this, s, this->state_num);
    }

    bool valid() const
    {
        return this->state_num != -1u;
    }

    size_t state_num;
};

struct rwlparser : state_machine_def<rwlparser>
{
    //BEGIN State
    struct Value {};
    struct EndString {};
    struct PropOrFunc {};
    struct LinkProp {};
    struct Prop {};
    struct Parameter {};
    struct PropSep {};
    struct PropName {};
    struct DefinedProp {};
    //END State


    //BEGIN action
    void string(const Value&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void name(const Value&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void hex_color(const Value&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void link_prop(const PropOrFunc&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void open_expr(const PropOrFunc&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void property_get(const LinkProp&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void link_prop(const Prop&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void close_expr(const Parameter&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void prop_name(const PropName&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void link_prop(const DefinedProp&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void defined_prop(const DefinedProp&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };
    void confirmed_string(const EndString&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; };

    void not_action(const Parameter&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; }
    void not_action(const PropSep&, const char * p, const char * e)
    { (std::cout << __PRETTY_FUNCTION__ << "\n\033[31m").write(p, e-p) << "\033[00m\n"; }
    //END action


    //BEGIN cheker and consumer
    typedef tokens::StringDelimiter Quote;
    typedef tokens::Identifier Identifier;
    typedef tokens::DefiniedColor DefinedColor;
    typedef tokens::Linker Dot;
    typedef tokens::OpenExpression OpenExpr;
    typedef tokens::CloseExpression CloseExpr;
    typedef tokens::IdentifierDelimiter TwoPoint;
    typedef tokens::Separator Semicolon;

    typedef tokens::ContentString String;
    typedef tokens::HexaColor HexColor;
    //END cheker and consumer


    typedef Value start_event;

    typedef rwlparser p;

    struct table_transition : boost::mpl::vector<
        row< Value,        Quote,          EndString,    String,       &p::string       >,
        row< Value,        Identifier,     PropOrFunc,   Identifier,   &p::name         >,
        row< Value,        DefinedColor,   PropSep,      HexColor,     &p::hex_color    >,
        row< PropOrFunc,   Dot,            LinkProp,     Dot,          &p::link_prop    >,
        row< PropOrFunc,   OpenExpr,       Parameter,    OpenExpr,     &p::open_expr    >,
        row< LinkProp,     Identifier,     Prop,         Identifier,   &p::property_get >,
        row< Prop,         Dot,            LinkProp,     Dot,          &p::link_prop    >,
        row< Parameter,    CloseExpr,      PropSep,      CloseExpr,    &p::close_expr   >,
        row< Parameter,    void,           Value,        void,         &p::not_action   >,
        row< PropSep,      Semicolon,      PropName,     Semicolon,    &p::not_action   >,
        row< PropName,     Identifier,     DefinedProp,  Identifier,   &p::prop_name    >,
        row< DefinedProp,  Dot,            PropName,     Dot,          &p::link_prop    >,
        row< DefinedProp,  TwoPoint,       Value,        TwoPoint,     &p::defined_prop >,
        row< EndString,    Quote,          PropSep,      Quote,        &p::confirmed_string>
    > {};


    ///TODO in state_machine_def
    template<unsigned N1, unsigned N2>
    struct range
    {
        static const std::size_t begin = N1;
        static const std::size_t end = N2;
    };

    ///TODO deduced in state_machine_def
    struct range_transition : boost::mpl::vector<
        range<0, 3>,
        range<3, 5>,
        range<5, 6>,
        range<6, 7>,
        range<7, 9>,
        range<9, 10>,
        range<10, 11>,
        range<11, 13>,
        range<13, 14>
    > {};


};

int main()
{
    rwlparser parser;

    const char * str = "#122 ; color: #239 ; color:\"plop\" ; color:#ffffff ; color:#23G";

    const char * states[] = {
        "value",
        "property_or_function",
        "link_property",
        "property",
        "parameter",
        "property_separator",
        "property_name",
        "defined_property",
        "confirmed_string",
    };

    while (parser.valid()) {
        std::cout << (states[parser.state_num]) << "\n";
        str = parser.next_event(str);
        std::cout << (str) << std::endl;
    }
}
