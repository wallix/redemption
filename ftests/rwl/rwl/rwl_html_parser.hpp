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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_RWL_RWL_HTML_PARSER_HPP
#define REDEMPTION_RWL_RWL_HTML_PARSER_HPP

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 40

#include "state_machine_def.hpp"
#include "rwl_value.hpp"

#include <boost/mpl/vector.hpp>

#include <stdexcept>

class rwl_html_parser : public state_machine_def<rwl_html_parser>
{
public:
    rwl_html_parser(rwl_target& target)
    : screen(target)
    , current_target(&this->screen)
    {
        this->states.reserve(32);
        this->target_depth.reserve(8);
        this->property_depth.reserve(3);
        this->value_depth.reserve(8);
    }

private:
    rwl_html_parser(const rwl_html_parser&);


private:
    rwl_target & screen;
    rwl_target * current_target;
    rwl_property * current_property;
    rwl_value * current_value;
    std::vector<rwl_target*> target_depth;
    std::vector<rwl_property*> property_depth;
    std::vector<rwl_value*> value_depth;
    std::vector<char> states;
    char quote_string;


    //BEGIN Event
    struct Target {};
    struct DefinedTarget {};
    struct ClosedTarget {};
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
    struct DefinedLink {};
    //END Event


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
        this->current_target->pop_back();
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

    void prop_name(const DefinedLink&, const char * p, const char * e)
    {
        this->current_property = &this->current_property->new_property(const_cstring(p, e));
    }

    void defined_prop(const DefinedProp&, const char * /*p*/, const char * /*e*/)
    {
        this->current_value = &this->current_property->value;
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
            if ((val->type == 'o' && (val->value.operation.op != '+' && val->value.operation.op != '-')) && (*p == '+' || *p == '-')) {
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
    }

    void value_block(const ValueBlock&, const char * /*p*/, const char * /*e*/)
    {
        this->states.push_back('p');
        this->property_depth.push_back(this->current_property);
    }

    void complet_value()
    {
        if (!this->value_depth.empty()) {
            rwl_value * val = this->value_depth.back();
            if (&this->current_property->value != val) {
                this->current_property->root_value = val;
            }
            this->value_depth.clear();
        }
    }

    template<typename Event>
    void close_block(const Event&, const char * /*p*/, const char * /*e*/)
    {
        if (this->states.empty()) {
            throw std::runtime_error("expected declaration before '}' token");
        }

        this->complet_value();

        if (this->states.back() == 't') {
            if (this->target_depth.empty()) {
                throw std::runtime_error("expected declaration before '}' token");
            }
            this->close_block();
        }
        else {
            this->property_depth.pop_back();
        }

        this->states.pop_back();
    }

    void prop_separator(const PropSep&, const char * /*p*/, const char * /*e*/)
    {
        this->complet_value();
    }

    void group_value(const Value&, const char * /*p*/, const char * /*e*/)
    {
        this->current_value->add_group();
        this->value_depth.push_back(this->current_value);
        this->current_value = this->current_value->value.g;
    }

    void close_expr(const PropSep&, const char * /*p*/, const char * /*e*/)
    {
        this->current_value = this->value_depth.back();
        this->value_depth.pop_back();
    }
    //END action


    //BEGIN cheker and consumer
    const char * consume_quote(const char * s)
    {
        if (*s == '\'' || *s == '"') {
            this->quote_string = *s;
            return s+1;
        }
        return s;
    }

    const char * consume_string(const char * s)
    {
        return tokens::contains_with_delimiter(s, this->quote_string);
    }

    const char * consume_attr_name(const char * s)
    {
        const char * tmp = s;
        if (*s == 'n' && *(s+1) == 'a' && *(s+2) == 'm' && *(s+3) == 'e') {
            s = tokens::Blank()(s+4);
            if (*s == '=') {
                return s + 1;
            }
        }
        return tmp;
    }

    const char * consume_target_tag(const char * s)
    {
        return (*s == 't' && *(s+1) == 'a' && *(s+2) == 'r' && *(s+3) == 'g' && *(s+4) == 'e' && *(s+5) == 't') ? s + 6 : s;
    }

    const char * consume_property_tag(const char * s)
    {
        return (*s == 'p' && *(s+1) == 'r' && *(s+2) == 'o' && *(s+3) == 'p') ? s + 4 : s;
    }

    typedef consumer_mem<&p::consume_quote>               Quote;
    typedef consumer_functor<tokens::CharSeparator<'>'> > EndTag;
    typedef consumer_functor<tokens::CharSeparator<'<'> > OpenTag;
    typedef consumer_functor<tokens::DoubleCharSeparator<'<','/'> > CloseTag;
    typedef consumer_functor<tokens::StringLower>         Identifier;
    typedef consumer_functor<tokens::Pound>               DefinedColor;
    typedef consumer_functor<tokens::Dot>                 Dot;
    typedef consumer_functor<tokens::OpenParenthesis>     OpenExpr;
    typedef consumer_functor<tokens::CloseParenthesis>    CloseExpr;
    typedef consumer_functor<tokens::Comma>               Comma;
    typedef consumer_functor<tokens::Integer>             Integer;
    typedef consumer_functor<tokens::Name>                TargetName;
    typedef consumer_functor<tokens::MathOperator>        MathOperator;
    typedef consumer_mem<&p::consume_attr_name>           AttrName;
    typedef consumer_mem<&p::consume_target_tag>          TargetTag;
    typedef consumer_mem<&p::consume_property_tag>        PropTag;

    typedef consumer_mem<&p::consume_string>              String;
    typedef consumer_functor<tokens::HexColor>            HexColor;
    //END chconsumer_functor<eker and consumer


    typedef rwl_html_parser p;

public:
    typedef PropName start_event;

    struct table_transition : boost::mpl::vector<
        //---+- event -------+ check ------+ event next --+ consumer + action ------------+
        c_row< PropName,      OpenTag,      DefinedTag,               &p::prop_name       >,
        c_row< PropName,      CloseTag,     ClosedTarget,             &p::close_block     >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< DefinedTag,    TargetTag,    TargetName,               &p::confirmed_target>,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< DefinedTarget, OpenBlock,    PropName,                 &p::confirmed_target>,
        c_row< DefinedTarget, CloseBlock,   ClosedTarget,             &p::empty_target    >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< ClosedTarget,  CloseBlock,   ClosedTarget,             &p::close_block     >,
        a_row< ClosedTarget,  Semicolon,    PropName                                      >,
        c_row< ClosedTarget,  TargetName,   DefinedTarget,            &p::target          >,
        t_row< ClosedTarget,                PropName                                      >,
        //---+---------------+-------------+--------------+----------+--------------------+
        a_row< DefinedProp,   Dot,          DefinedLink                                   >,
        c_row< DefinedProp,   TwoPoint,     ValueBlock,               &p::defined_prop    >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< DefinedLink,   Identifier,   DefinedProp,              &p::prop_name       >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< ValueBlock,    OpenBlock,    PropName,                 &p::value_block     >,
        t_row< ValueBlock,                  Value                                         >,
        //---+---------------+-------------+--------------+----------+--------------------+
          row< Value,         Quote,        EndString,     String,    &p::string          >,
        c_row< Value,         Identifier,   PropOrFunc,               &p::name            >,
          row< Value,         DefinedColor, PropSep,       HexColor,  &p::hex_color       >,
        c_row< Value,         Integer,      Expression,               &p::integer         >,
        c_row< Value,         OpenExpr,     Value,                    &p::group_value     >,
        //---+---------------+-------------+--------------+----------+--------------------+
        a_row< EndString,     Quote,        PropSep                                       >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< PropOrFunc,    OpenExpr,     Value,                    &p::open_expr       >,
        t_row< PropOrFunc,                  Prop                                          >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< PropSep,       Comma,        Value,                    &p::value_separator >,
        c_row< PropSep,       CloseExpr,    Expression,               &p::close_expr      >,
        c_row< PropSep,       Semicolon,    PropName,                 &p::prop_separator  >,
        c_row< PropSep,       CloseBlock,   ClosedTarget,             &p::close_block     >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< LinkProp,      Identifier,   Prop,                     &p::property_get    >,
        //---+---------------+-------------+--------------+----------+--------------------+
        a_row< Prop,          Dot,          LinkProp                                      >,
        c_row< Prop,          MathOperator, Value,                    &p::math_operator   >,
        t_row< Prop,                        PropSep                                       >,
        //---+---------------+-------------+--------------+----------+--------------------+
        c_row< Expression,    MathOperator, Value,                    &p::math_operator   >,
        t_row< Expression,                  PropSep                                       >
    > {};

    bool stop() const
    {
        return this->target_depth.empty();
    }
};

#endif
