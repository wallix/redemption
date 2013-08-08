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
#include <vector>
#include <iostream>
#include <new>
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace tokens {

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

    template<char Delimiter, char AntiDelimiter = '\\'>
    struct ContainsWithDelimiter {
        const char * operator()(const char * s) const
        {
            if (*s && *s != Delimiter) {
                ++s;
                while (*s && *s != Delimiter) {
                    if (*s == AntiDelimiter && *(s+1)) {
                        ++s;
                    }
                    ++s;
                }
            }
            return s;
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

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 40
#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/push_back.hpp>

using std::size_t;

template<typename T, typename Default>
struct value_or
{
    typedef typename boost::conditional<
        boost::is_same<T, void>::value,
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
        typedef typename row::base_check base_check;
        typedef typename row::base_consumer base_consumer;
        typedef typename row::consumer consumer;
        typedef typename row::action action;
        typedef typename row::confirmed confirmed;

        const char * p = boost::is_same<base_check, void>::value ? s : check()(s);

        if (boost::is_same<base_check, void>::value || p != s) {
            if (confirmed()(sm, event)) {
                if (boost::is_same<consumer, check>::value) {
                    action()(sm, event, s, p);
                    if (!boost::is_same<base_consumer, void>::value) {
                        s = p;
                    }
                }
                else {
                    s = consumer()(p);
                    action()(sm, event, p, s);
                }


                if (!boost::is_same<typename row::base_ignore, void>::value) {
                    typedef typename row::ignore ignore;
                    s = ignore()(s);
                }

                return state_machine_index_transition<
                    0,
                    boost::mpl::size<RangeTransition>::value,
                    typename row::next,
                    TableTransition,
                    RangeTransition
                >::value;
            }
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
struct state_machine_def
{
    typedef Derived derived_type;

    template<typename Event>
    void error(const char *, const Event&) const
    {std::cout << "machine error\n"; }

    struct null_check {
        const char * operator()(const char * s) const
        {return s; }
    };

    struct null_consumer {
        const char * operator()(const char * s) const
        {return s; }
    };

    struct null_transition {};


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

public:
    template<
        typename Start,
        typename Check,
        typename Next,
        typename Consumer,
        void (Derived::*Action)(const Start &, const char *, const char *),
        typename Ignore = tokens::Whitespaces
    >
    struct row {
        typedef Start event;
        typedef typename value_or<Check, null_check>::type check;
        typedef Next next;
        typedef Check base_check;
        typedef Consumer base_consumer;
        typedef Ignore base_ignore;
        typedef typename value_or<Consumer, null_consumer>::type consumer;
        typedef typename value_or<Ignore, null_consumer>::type ignore;
        typedef void (Derived::*action_type)(const Start&);

        struct action
        {
            void operator()(state_machine_def& sm, const Start& event,
                            const char * b, const char * e)
            {
                (static_cast<Derived&>(sm).*Action)(event, b, e);
            }
        };

        struct confirmed {
            bool operator()(state_machine_def&, const Start&)
            {
                return true;
            }
        };
    };


    state_machine_def()
    : state_num(defined_index_transition<
        typename Derived::table_transition,
        typename Derived::start_event
    >::value)
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

struct const_cstring
{
    const_cstring(const char * first, const char * last)
    : first(first)
    , last(last)
    {}

    const_cstring()
    : first("")
    , last("")
    {}

    bool empty() const
    { return this->first == this->last; }

    size_t size() const
    {return this->last - this->first; }

    void assign(const char * first, const char * last)
    {
        this->first = first;
        this->last = last;
    }

    bool operator==(const char * s) const
    {
        const char * p = this->first;
        while (p != last && *p == *s) {
            ++p;
            ++s;
        }
        return p == this->last && !*s;
    }

    bool operator==(const const_cstring& other) const
    {
        if (this->size() != other.size()) {
            return false;
        }
        const char * p1 = this->first;
        const char * p2 = other.first;
        while (p1 != this->last && *p1 == *p2) {
            ++p1;
            ++p2;
        }
        return p1 == this->last;
    }

    bool operator<(const char * s) const
    {
        const char * p = this->first;
        while (p != last && *p < *s) {
            ++p;
            ++s;
        }
        return p == this->last ? *s : (*p < *s);
    }

    bool operator<(const const_cstring& other) const
    {
        size_t min = std::min(this->size(), other.size());
        const char * p1 = this->first;
        const char * p2 = other.first;
        const char * e = p1 + min;
        while (p1 != e && *p1 < *p2) {
            ++p1;
            ++p2;
        }
        return p1 == e ? (other.size() > min) : (*p1 < *p2);
    }

    char operator[](size_t i) const
    {return this->first[i]; }

    const char * first;
    const char * last;
};

std::ostream& operator<<(std::ostream& os, const const_cstring& cs)
{
    return os.write(cs.first, cs.size());
}


unsigned int chex2i(char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    if ('A' <= c && c <= 'Z') {
        return c - 'A' + 10;
    }
    return c - 'a' + 10;
}

struct rwl_value
{
    rwl_value()
    : type(0)
    {}

private:
    rwl_value(const rwl_value& other);
//     : type(other.type)
//     {
//         switch (this->type) {
//             case 'i':
//             case 'c':
//                 this->value.i = other.value.i;
//                 break;
//             case 'l': {
//                 this->value.linked.size = other.value.linked.size;
//                 const unsigned size = other.value.linked.size * 2;
//                 this->value.linked.pair_strings = new const char *[size];
//                 std::copy(other.value.linked.pair_strings,
//                           other.value.linked.pair_strings + size,
//                           this->value.linked.pair_strings);
//                 this->value.linked.first = other.value.linked.first;
//                 this->value.linked.last = other.value.linked.last;
//             }   break;
//             case 'f':{
//                 this->value.func.size = other.value.func.size;
//                 const unsigned size = other.value.func.size;
//                 this->value.func.params = static_cast<rwl_value*>(::operator new (size * sizeof(rwl_value)));
//                 std::uninitialized_copy(other.value.func.params,
//                                         other.value.func.params + size,
//                                         this->value.func.params);
//             } //no break
//             case 's':
//             case 't':
//                 this->value.s.first = other.value.s.first;
//                 this->value.s.last = other.value.s.last;
//                 break;
//             default:
//                 break;
//             case 'o':
//                 this->value.operation.l = new rwl_value(*other.value.operation.l);
//                 this->value.operation.r = new rwl_value(*other.value.operation.r);
//                 this->value.operation.op = other.value.operation.op;
//                 break;
//         }
//     }

    struct deleter { void operator()(rwl_value* val) const {
        delete val;
    } };

public:
    ~rwl_value()
    {
        switch (this->type) {
            case 'o':
                delete this->value.operation.l;
                delete this->value.operation.r;
                break;
            case 'l':
                delete[] this->value.linked.pair_strings;
                break;
            case 'f':
                std::for_each(this->value.func.params + 0,
                              this->value.func.params + this->value.func.size,
                              deleter());
                delete [] this->value.func.params;
                break;
            default:
                break;
        }
    }

    void add_string(const char * p, const char * e)
    {
        this->type = 's';
        this->value.s.first = p;
        this->value.s.last = e;
    }

    void add_identifier(const char * p, const char * e)
    {
        this->type = 't';
        this->value.s.first = p;
        this->value.s.last = e;
    }

    void add_color(const char * p, const char * e)
    {
        if (e - p == 3) {
            const unsigned r = chex2i(p[0]);
            const unsigned g = chex2i(p[1]);
            const unsigned b = chex2i(p[2]);
            this->value.color
            = (r << 20) + (r << 16)
            + (g << 12) + (g << 8)
            + (b << 4) + b;
        }
        else {
            this->value.color
            = (chex2i(p[0]) << 20)
            + (chex2i(p[1]) << 16)
            + (chex2i(p[2]) << 12)
            + (chex2i(p[3]) << 8)
            + (chex2i(p[4]) << 4)
            + (chex2i(p[5]));
        }
        this->type = 'c';
    }

    void add_integer(const char * p, const char * /*e*/)
    {
        this->type = 'i';
        this->value.i = atoi(p);
    }

    void add_operator(const char * p, const char * /*e*/)
    {
        rwl_value * val = new rwl_value;
        std::memcpy(val, this, sizeof(rwl_value));
        this->value.operation.l = val;
        this->value.operation.r = new rwl_value;
        this->value.operation.op = *p;
        this->type = 'o';
    }

    void add_linked(const char * p, const char * e)
    {
        if (this->type == 'l') {
            const unsigned size = this->value.linked.size * 2;
            const char * * values = new const char *[size + 2];
            std::copy(this->value.linked.pair_strings,
                      this->value.linked.pair_strings + size,
                      values);
            values[size] = p;
            values[size+1] = e;
            ++this->value.linked.size;
            this->value.linked.pair_strings = values;
        }
        else {
            this->value.linked.size = 1;
            this->value.linked.pair_strings = new const char * [2];
            this->value.linked.pair_strings[0] = p;
            this->value.linked.pair_strings[1] = e;
            this->type = 'l';
        }
    }

    rwl_value* to_function()
    {
        this->type = 'f';
        this->value.func.size = 1;
        this->value.func.params = new rwl_value * [1];
        this->value.func.params[0] = new rwl_value;
        return this->value.func.params[0];
    }

    rwl_value* next_parameter()
    {
        const unsigned size = this->value.func.size;
        rwl_value ** values = new rwl_value * [size+1];
        std::copy(this->value.func.params,
                  this->value.func.params + size,
                  values);
        values[size] = new rwl_value;
        ++this->value.func.size;
        delete [] this->value.func.params;
        this->value.func.params = values;
        return this->value.func.params[size];
    }

    union {
        struct {
            const char * first;
            const char * last;
        } s;
        struct {
            const char * first;
            const char * last;
            rwl_value ** params;
            unsigned size;
        } func;
        struct {
            const char * first;
            const char * last;
            const char * * pair_strings;
            unsigned size;
        } linked;
        rwl_value * g;
        struct {
            rwl_value * l;
            rwl_value * r;
            int op;
        } operation;
        unsigned color;
        int i;
//         unsigned u;
//         long l;
//         unsigned long ul;
    } value;
    /**
     * 'i': integer
     * 'f': function
     * 's': string
     * 't': identifier
     * 'o': group operator
     * 'c': color
     * 'l': linked
     * 'g': group
     */
    int type;
};

struct rwl_property
{
    rwl_property()
    : root_value(&value)
    {}

    rwl_property(const const_cstring& name)
    : name(name)
    , root_value(&value)
    {}

    rwl_property& new_property(const const_cstring& name)
    {
        this->properties.push_back(new rwl_property(name));
        return *this->properties.back();
    }

    const_cstring name;
    rwl_value value;
    rwl_value * root_value;
    std::vector<rwl_property*> properties;

private:
    rwl_property(const rwl_property&);
};

struct rwl_target
{
    rwl_target()
    {}

    rwl_target(const const_cstring& name)
    : name(name)
    {}

    rwl_target& new_target(const const_cstring& name)
    {
        this->targets.push_back(new rwl_target(name));
        return *this->targets.back();
    }

    rwl_property& new_property(const const_cstring& name)
    {
        this->properties.push_back(new rwl_property(name));
        return *this->properties.back();
    }

    const_cstring name;
    std::vector<rwl_property*> properties;
    std::vector<rwl_target*> targets;

private:
    rwl_target(const rwl_target&);
};

struct rwlparser : state_machine_def<rwlparser>
{
    //BEGIN Event
    struct Target {};
    struct DefinedTarget {};
    struct ClosingTarget {};
    struct PropName {};
    struct DefinedProp {};
    struct Value {};
    struct EndString {};
    struct PropOrFunc {};
    struct PropSep {};
    struct LinkProp {};
    struct Prop {};
    struct Expression {};
    struct Operation {};
    struct ValueBlock {};
    //END Event


    rwlparser()
    : current_target(&this->screen)
    , linked_prop(false)
    {
        this->states.reserve(32);
        this->target_depth.reserve(8);
        this->property_depth.reserve(3);
        this->value_depth.reserve(8);
    }

    rwl_target screen;
    rwl_target * current_target;
    rwl_property * current_property;
    rwl_value * current_value;
    std::vector<rwl_target*> target_depth;
    std::vector<rwl_property*> property_depth;
    std::vector<rwl_value*> value_depth;
    std::vector<char> states;
    bool linked_prop;

    //BEGIN action
    template<typename Event>
    void target(const Event&, const char * p, const char * e)
    {
        this->current_target = &this->current_target->new_target(const_cstring(p, e));
        this->target_depth.push_back(this->current_target);
    }

    void confirmed_target(const DefinedTarget&, const char * /*p*/, const char * /*e*/)
    {
        this->states.push_back('t');
    }

    void close_block()
    {
        this->target_depth.pop_back();
        if (this->target_depth.empty()) {
            this->current_target = &this->screen;
        }
        else {
            this->current_target = this->target_depth.back();
        }
    }

    void empty_target(const DefinedTarget&, const char * /*p*/, const char * /*e*/)
    {
        this->current_target->targets.pop_back();
        this->close_block();
    }

    void prop_name(const PropName&, const char * p, const char * e)
    {
        if ( ! this->property_depth.empty()) {
            this->current_property = &this->property_depth.back()->new_property(const_cstring(p, e));
        }
        else {
            this->current_property = &this->current_target->new_property(const_cstring(p, e));
        }
    }

    void defined_prop(const DefinedProp&, const char * /*p*/, const char * /*e*/)
    {
        this->linked_prop = false;
        this->current_value = &this->current_property->value;
    }

    void link_prop(const DefinedProp&, const char * /*p*/, const char * /*e*/)
    {
        this->linked_prop = true;
    }

    template<typename Event>
    void link_prop(const Event&, const char * /*p*/, const char * /*e*/)
    {

    }

    void string(const Value&, const char * p, const char * e)
    {
        this->current_value->add_string(p, e);
    }

    void name(const Value&, const char * p, const char * e)
    {
        this->current_value->add_identifier(p, e);
    }

    void hex_color(const Value&, const char * p, const char * e)
    {
        this->current_value->add_color(p, e);
    }

    void integer(const Value&, const char * p, const char * e)
    {
        this->current_value->add_integer(p, e);
    }

    template<typename Event>
    void math_operator(const Event&, const char * p, const char * e)
    {
        if (!this->value_depth.empty()) {
            rwl_value * val = this->value_depth.back();
            if ((val->type != 'o' || (val->value.operation.op != '+' && val->value.operation.op != '-')) && (*p == '+' || *p == '-')) {
                rwl_value * new_val = new rwl_value;
                new_val->type = 'o';
                new_val->value.operation.l = val;
                new_val->value.operation.r = new rwl_value;
                new_val->value.operation.op = *p;
                this->current_value = new_val->value.operation.r;
                this->value_depth.back() = new_val;
            }
            else {
                this->current_value->add_operator(p, e);
                this->current_value = this->current_value->value.operation.r;
            }
        }
        else {
            this->value_depth.push_back(this->current_value);
            this->current_value->add_operator(p, e);
            this->current_value = this->current_value->value.operation.r;
        }
    }

    void property_get(const LinkProp&, const char * p, const char * e)
    {
        this->current_value->add_linked(p, e);
    }

    void open_expr(const PropOrFunc&, const char * /*p*/, const char * /*e*/)
    {
        this->value_depth.push_back(this->current_value);
        this->current_value = this->current_value->to_function();
    }

    void value_separator(const PropSep&, const char * /*p*/, const char * /*e*/)
    {
        this->current_value = this->value_depth.back()->next_parameter();
        this->value_depth.back() = this->current_value;
    }

    void value_block(const ValueBlock&, const char * /*p*/, const char * /*e*/)
    {
        this->states.push_back('p');
        this->linked_prop = true;
        this->property_depth.push_back(this->current_property);
    }

    template<typename Event>
    void close_block(const Event&, const char * /*p*/, const char * /*e*/)
    {
        if (this->states.empty()) {
            throw std::runtime_error("expected declaration before '}' token");
        }

        if (this->states.back() == 't') {
            if (this->target_depth.empty()) {
                throw std::runtime_error("expected declaration before '}' token");
            }
            this->close_block();

        }
        else {
            this->property_depth.pop_back();
            this->value_depth.clear();
        }

        this->states.pop_back();
    }

    void group_value(const Operation&, const char * /*p*/, const char * /*e*/)
    {}

    void prop_separator(const PropSep&, const char * /*p*/, const char * /*e*/)
    {
        if (!this->value_depth.empty()) {
            rwl_value * val = this->value_depth.back();
            if (&this->current_property->value != val) {
                this->current_property->root_value = val;
            }
            this->value_depth.clear();
        }
    }

    void group_value(const Value&, const char * /*p*/, const char * /*e*/)
    {
        this->current_value->type = 'g';
        this->current_value->value.g = new rwl_value;
        this->value_depth.push_back(this->current_value);
        this->current_value = this->current_value->value.g;
    }

    void close_expr(const PropSep&, const char * /*p*/, const char * /*e*/)
    {
        this->current_value = this->value_depth.back();
        this->value_depth.pop_back();
    }





    void prop_value(const Prop&, const char * /*p*/, const char * /*e*/)
    {}
    void confirmed_string(const EndString&, const char * /*p*/, const char * /*e*/)
    {}

    template<typename Event>
    void not_action(const Event&, const char * /*p*/, const char * /*e*/)
    {/*(std::cout << __PRETTY_FUNCTION__ << "\n'\033[31m").write(p, e-p) << "\033[00m'\n"; */}
    //END action


    //BEGIN cheker and consumer
    typedef tokens::Quote Quote;
    typedef tokens::StringLower Identifier;
    typedef tokens::Pound DefinedColor;
    typedef tokens::Dot Dot;
    typedef tokens::OpenParenthesis OpenExpr;
    typedef tokens::CloseParenthesis CloseExpr;
    typedef tokens::TwoPoint TwoPoint;
    typedef tokens::Semicolon Semicolon;
    typedef tokens::Comma Comma;
    typedef tokens::Integer Integer;
    typedef tokens::Name TargetName;
    typedef tokens::OpenBlock OpenBlock;
    typedef tokens::CloseBlock CloseBlock;
    typedef tokens::MathOperator MathOperator;

    typedef tokens::String String;
    typedef tokens::HexColor HexColor;
    //END cheker and consumer


    typedef Target start_event;

    typedef rwlparser p;

    struct table_transition : boost::mpl::vector<
        //-+- event -------+ check -------+ event next --+ consumer ---+ action --------+
        row< Target,        TargetName,    DefinedTarget, TargetName,   &p::target       >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< DefinedTarget, OpenBlock,     PropName,      OpenBlock,    &p::confirmed_target>,
        row< DefinedTarget, CloseBlock,    ClosingTarget, CloseBlock,   &p::empty_target >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< ClosingTarget, CloseBlock,    ClosingTarget, CloseBlock,   &p::close_block >,
        row< ClosingTarget, Semicolon,     PropName,      Semicolon,    &p::not_action   >,
        row< ClosingTarget, TargetName,    DefinedTarget, TargetName,   &p::target       >,
        row< ClosingTarget, void,          PropName,      void,         &p::not_action   >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< PropName,      CloseBlock,    ClosingTarget, CloseBlock,   &p::close_block >,
        row< PropName,      TargetName,    DefinedTarget, TargetName,   &p::target       >,
        row< PropName,      Identifier,    DefinedProp,   Identifier,   &p::prop_name    >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< DefinedProp,   Dot,           PropName,      Dot,          &p::link_prop    >,
        row< DefinedProp,   TwoPoint,      ValueBlock,    TwoPoint,     &p::defined_prop >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< ValueBlock,    OpenBlock,     PropName,      OpenBlock,    &p::value_block  >,
        row< ValueBlock,    void,          Value,         void,         &p::not_action   >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< Value,         Quote,         EndString,     String,       &p::string       >,
        row< Value,         Identifier,    PropOrFunc,    Identifier,   &p::name         >,
        row< Value,         DefinedColor,  PropSep,       HexColor,     &p::hex_color    >,
        row< Value,         Integer,       Expression,    Integer,      &p::integer      >,
        row< Value,         OpenExpr,      Value,         OpenExpr,     &p::group_value  >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< EndString,     Quote,         PropSep,       Quote,        &p::confirmed_string>,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< PropOrFunc,    Dot,           LinkProp,      Dot,          &p::link_prop    >,
        row< PropOrFunc,    OpenExpr,      Value,         OpenExpr,     &p::open_expr    >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< PropSep,       Comma,         Value,         Comma,        &p::value_separator>,
        row< PropSep,       CloseExpr,     Expression,    CloseExpr,    &p::close_expr   >,
        row< PropSep,       Semicolon,     PropName,      Semicolon,    &p::prop_separator>,
        row< PropSep,       CloseBlock,    ClosingTarget, CloseBlock,   &p::close_block >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< LinkProp,      Identifier,    Prop,          Identifier,   &p::property_get >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< Prop,          Dot,           LinkProp,      Dot,          &p::link_prop    >,
        row< Prop,          MathOperator,  Operation,     MathOperator, &p::math_operator>,
        row< Prop,          void,          PropSep,       void,         &p::prop_value   >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< Expression,    MathOperator,  Operation,     MathOperator, &p::math_operator>,
        row< Expression,    void,          PropSep,       void,         &p::not_action   >,
        //-+---------------+--------------+--------------+-------------+----------------+
        row< Operation,     OpenExpr,      Value,         OpenExpr,     &p::group_value  >,
        row< Operation,     void,          Value,         void,         &p::not_action   >
    > {};
};

const_cstring tb(unsigned tab = 0)
{
#define SPACE_S "                                                                      "
    return const_cstring(SPACE_S, SPACE_S + tab*2);
#undef SPACE_S

}

void display_value(const rwl_value& value, unsigned tab = 0)
{
    switch (value.type) {
        case 'c':
            std::cout << tb(tab) << "color " << value.value.color << "\n";
            break;
        case 'i':
            std::cout << tb(tab) << "integer " << value.value.i << "\n";
            break;
        case 's':
            std::cout << tb(tab) << "string " << const_cstring(value.value.s.first, value.value.s.last) << "\n";
            break;
        case 't':
            std::cout << tb(tab) << "identity " << const_cstring(value.value.s.first, value.value.s.last) << "\n";
            break;
        case 'f':
            std::cout << tb(tab) << "function " << const_cstring(value.value.func.first, value.value.func.last) << "\n";
            for (unsigned i = 0; i < value.value.func.size; ++i) {
                std::cout << tb(tab) << i << "\n";
                display_value(*value.value.func.params[i], tab+1);
            }
            break;
        case 'o':
            std::cout << tb(tab) << "group\n";
            display_value(*value.value.operation.l, tab+1);
            std::cout << tb(tab+1) << char(value.value.operation.op) << "\n";
            display_value(*value.value.operation.r, tab+1);
            break;
        case 'g':
            std::cout << tb(tab) << "(\n";
            display_value(*value.value.g, tab+1);
            std::cout << tb(tab) << ")\n";
            break;
        case 'l':
            std::cout << tb(tab) << "identity " << const_cstring(value.value.linked.first, value.value.linked.last);
            for (unsigned i = 0; i < value.value.linked.size; ++i) {
                const char * * ps = value.value.linked.pair_strings;
                std::cout << " -> " << const_cstring(ps[i*2], ps[i*2+1]);
            }
            std::cout << "\n";
            break;
        default:
            std::cout << tb(tab) << "none\n";
            break;
    }
}

void display_property(const rwl_property& property, unsigned tab = 0)
{
    std::cout << tb(tab) << "Property " << property.name << ":\n";

    if (property.properties.empty() ? 1 : (property.value.type != 0)) {
        display_value(*property.root_value, tab+1);
    }

    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = property.properties.begin(), last = property.properties.end(); first != last ; ++first) {
        display_property(**first, tab+1);
    }
}

void display_target(const rwl_target& target, unsigned tab = 0)
{
    std::cout << tb(tab) << "Target " << target.name << ":\n";
    typedef std::vector<rwl_property*>::const_iterator prop_iterator;
    for (prop_iterator first = target.properties.begin(), last = target.properties.end(); first != last ; ++first) {
        display_property(**first, tab+1);
    }

    typedef std::vector<rwl_target*>::const_iterator target_iterator;
    for (target_iterator first = target.targets.begin(), last = target.targets.end(); first != last ; ++first) {
        display_target(**first, tab+1);
    }
}


int main()
{
    std::ios::sync_with_stdio(false);

    rwlparser parser;

    const char * str = "Rect {"
        " bgcolor: #122 ;"
        " color: rgb( #239, \"l\" ) ;"
        " color: rgb( #239 ) ;"
        " color: rgb( (#239) , \"l\", 234 ) ;"
        " x: label.w ;"
        " x: label.w + 2 ;"
        " border.top: 2 ;"
        " border.top.color: #233 ;"
        " Rect { x: 20+2 }"
        " x: 20+2 ;"
        " Rect { Rect { x: 20 ;} }"
        " Rect { Rect { x: 20 } ; }"
        " text: \"plop\" ;"
        " border: { left: 2 ; right: 2 ; top: 2}"
        " border: { left: 2 ; color: { top:3 ; bottom: 1; } ; right: 2 ; top: 2}"
        " x: (screen.w - this.w) / 2 ;"
        " y: screen.w - this.w / 2 ;"
        " n: 1 * 2 + 3 ;"
        " n: 1 + 2 * 3 ;"
    " }"
    " Rect { x: 202 }"
    ;

    const char * states[] = {
        "target",
        "confirmed_target",
        "closing_target",
        "property",
        "defined_property",
        "value_block",
        "value",
        "confirmed_string",
        "property_or_function",
        "property_separator",
        "link_property",
        "property_name",
        "expression",
        "operation",
    };

    while (parser.valid() && *str) {
        str = parser.next_event(str);
    }
    if (*str) {
        std::cout << ("parsing error\n");
    }

    display_target(parser.screen);
}
