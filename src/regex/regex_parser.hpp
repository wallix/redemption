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


#pragma once

#include "regex_utils.hpp"

#include <algorithm>
#include <utility> //std::pair
#include <vector>
#include <new>

namespace re {

    class StateAccu
    {
    public:
        unsigned num_cap = 0;
        state_list_t sts;
        std::vector<unsigned> indexes;
        using c_range_type = std::vector<std::pair<char_int, char_int>>;
        c_range_type c_ranges;

    private:
#ifdef RE_PARSER_POOL_STATE
        struct memory_list_t
        {
            static const unsigned reserve_state = 31;
            memory_list_t * prev;
            unsigned len;
            State states[1];

            static memory_list_t * allocate(memory_list_t * cur)
            {
                memory_list_t * ret = static_cast<memory_list_t*>(
                    ::operator new(sizeof(memory_list_t) + (reserve_state - 1) * sizeof(State))
                );
                ret->prev = cur;
                ret->len = 0;
                return ret;
            }

            struct Deleter
            {
                void operator()(State & st) const
                {
                    st.~State();
                }
            };

            void clear()
            {
                std::for_each(this->states + 0, this->states + this->len, Deleter());
                this->len = 0;
            }

            static memory_list_t * deallocate(memory_list_t * cur)
            {
                memory_list_t * ret = cur->prev;
                cur->clear();
                ::operator delete(cur);
                return ret;
            }

            bool is_full() const
            {
                return this->len == reserve_state;
            }

            void * next()
            {
                return this->states + this->len++;
            }
        };

        memory_list_t * mem;
#endif

        State * push(unsigned type,
                     char_int range_left = 0, char_int range_right = 0,
                     State * out1 = nullptr, State * out2 = nullptr)
        {
#ifdef RE_PARSER_POOL_STATE
            if (this->mem->is_full()) {
                this->mem = memory_list_t::allocate(this->mem);
            }
            State * st = new(this->mem->next()) State(type, range_left, range_right, out1, out2);
#else
            State * st = new State(type, range_left, range_right, out1, out2);
#endif
            sts.push_back(st);
            return st;
        }

    public:
        StateAccu()
#ifdef RE_PARSER_POOL_STATE
        : mem(memory_list_t::allocate(nullptr))
        {}
#else
        = default;
#endif

        ~StateAccu()
        {
            this->delete_state();
#ifdef RE_PARSER_POOL_STATE
            memory_list_t::deallocate(this->mem);
#endif
        }

        State * normal(unsigned type, char_int range_left = 0, char_int range_right = 0,
                       State * out1 = nullptr, State * out2 = nullptr)
        {
            return this->push(type, range_left, range_right, out1, out2);
        }

        State * character(char_int c, State * out1 = nullptr) {
            return this->push(RANGE, c, c, out1);
        }

        State * range(char_int left, char_int right, State * out1 = nullptr) {
            return this->push(RANGE, left, right, out1);
        }

        State * any(State * out1 = nullptr) {
            return this->push(RANGE, 0, ~char_int{}, out1);
        }

        State * split(State * out1 = nullptr, State * out2 = nullptr) {
            return this->push(SPLIT, 0, 0, out1, out2);
        }

        State * cap_open(State * out1 = nullptr) {
            State * ret = this->push(CAPTURE_OPEN, 0, 0, out1);
            ret->num = this->num_cap++;
            return ret;
        }

        State * cap_close(State * out1 = nullptr) {
            State * ret = this->push(CAPTURE_CLOSE, 0, 0, out1);
            ret->num = this->num_cap++;
            return ret;
        }

        State * epsilone(State * out1 = nullptr) {
            return this->push(EPSILONE, 0, 0, out1);
        }

        State * finish(State * out1 = nullptr) {
            return this->push(FINISH, 0, 0, out1);
        }

        State * begin(State * out1 = nullptr) {
            return this->push(FIRST, 0, 0, out1);
        }

        State * last() {
            return this->push(LAST);
        }

        State * sequence(const char_int * s, size_t len, State * out1 = nullptr) {
            State * ret = this->push(SEQUENCE, 0, 0, out1);
            ret->data.sequence.s = s;
            ret->data.sequence.len = len;
            return ret;
        }

        State * sequence(const SequenceString & ss, State * out1 = nullptr) {
            return this->sequence(ss.s, ss.len, out1);
        }

        void clear()
        {
            this->delete_state();
            this->num_cap = 0;
            this->c_ranges.clear();
            this->indexes.clear();
            this->sts.clear();
        }

        void clear_and_shrink()
        {
            this->delete_state();
            this->num_cap = 0;
#if __cplusplus >= 201103L
            this->c_ranges.clear();
            this->c_ranges.shrink_to_fit();
            this->indexes.clear();
            this->indexes.shrink_to_fit();
            this->sts.clear();
            this->sts.shrink_to_fit();
#else
            this->c_ranges.~vector();
            new (&this->c_ranges) c_range_type();
            this->indexes.~vector();
            new (&this->indexes) std::vector<unsigned>();
            this->sts.~vector();
            new (&this->sts) state_list_t();
#endif
        }

    private:
        void delete_state()
        {
#ifdef RE_PARSER_POOL_STATE
            while (this->mem->prev) {
                this->mem = memory_list_t::deallocate(this->mem);
            }
            this->mem->clear();
#else
            std::for_each(this->sts.begin(), this->sts.end(), StateDeleter());
#endif
        }
    };

    inline State ** c2range(StateAccu & accu,
                            State ** pst, State * eps,
                            char_int l1, char_int r1,
                            char_int l2, char_int r2)
    {
        *pst = accu.split(accu.range(l1, r1, eps),
                          accu.range(l2, r2, eps)
                         );
        return &eps->out1;
    }

    inline State ** c2range(StateAccu & accu,
                            State ** pst, State * eps,
                            char_int l1, char_int r1,
                            char_int l2, char_int r2,
                            char_int l3, char_int r3)
    {
        *pst = accu.split(accu.range(l1, r1, eps),
                          accu.split(accu.range(l2, r2, eps),
                                     accu.range(l3, r3, eps)
                                    )
                         );
        return &eps->out1;
    }

    inline State ** c2range(StateAccu & accu,
                            State ** pst, State * eps,
                            char_int l1, char_int r1,
                            char_int l2, char_int r2,
                            char_int l3, char_int r3,
                            char_int l4, char_int r4)
    {
        *pst = accu.split(accu.range(l1, r1, eps),
                          accu.split(accu.range(l2, r2, eps),
                                     accu.split(accu.range(l3, r3, eps),
                                                accu.range(l4, r4, eps)
                                     )
                         )
        );
        return &eps->out1;
    }

    inline State ** c2range(StateAccu & accu,
                            State ** pst, State * eps,
                            char_int l1, char_int r1,
                            char_int l2, char_int r2,
                            char_int l3, char_int r3,
                            char_int l4, char_int r4,
                            char_int l5, char_int r5)
    {
        *pst = accu.split(accu.range(l1, r1, eps),
                          accu.split(accu.range(l2, r2, eps),
                                     accu.split(accu.range(l3, r3, eps),
                                                accu.split(accu.range(l4, r4, eps),
                                                           accu.range(l5, r5, eps)
                                               )
                                    )
                         )
        );
        return &eps->out1;
    }

    inline State ** ident_D(StateAccu & accu, State ** pst, State * eps) {
        return c2range(accu, pst, eps, 0,'0'-1, '9'+1,-1u);
    }

    inline State ** ident_w(StateAccu & accu, State ** pst, State * eps) {
        return c2range(accu, pst, eps, 'a','z', 'A','Z', '0','9', '_', '_');
    }

    inline State ** ident_W(StateAccu & accu, State ** pst, State * eps) {
        return c2range(accu, pst, eps, 0,'0'-1, '9'+1,'A'-1, 'Z'+1,'_'-1, '_'+1,'a'-1, 'z'+1,-1u);
    }

    inline State ** ident_s(StateAccu & accu, State ** pst, State * eps) {
        return c2range(accu, pst, eps, ' ',' ', '\t','\v');
    }

    inline State ** ident_S(StateAccu & accu, State ** pst, State * eps) {
        return c2range(accu, pst, eps, 0,'\t'-1, '\v'+1,' '-1, ' '+1,-1u);
    }

    inline const char * check_interval(char_int a, char_int b)
    {
        bool valid = ('0' <= a && a <= '9' && '0' <= b && b <= '9')
                  || ('a' <= a && a <= 'z' && 'a' <= b && b <= 'z')
                  || ('A' <= a && a <= 'Z' && 'A' <= b && b <= 'Z');
        return (valid && a <= b) ? nullptr : "range out of order in character class";
    }

    struct VectorRange
    {
        using range_t = std::pair<char_int, char_int>;
        using container_type = std::vector<range_t>;
        using iterator = container_type::iterator;

        container_type & ranges;

        explicit VectorRange(container_type & c_ranges)
        : ranges(c_ranges)
        {
            this->ranges.clear();
        }

        void push(char_int left, char_int right) {
            ranges.push_back(range_t(left, right));
        }

        void push(char_int c) {
            this->push(c,c);
        }
    };

    inline char_int get_c(utf8_consumer & consumer, char_int c)
    {
        if (c != '[' && c != '.') {
            if (c == '\\') {
                char_int c2 = consumer.bumpc();
                switch (c2) {
                    case 0:
                        return '\\';
                    case 'd':
                    case 'D':
                    case 'w':
                    case 'W':
                    case 's':
                    case 'S':
                        return 0;
                    case 'n': return '\n';
                    case 't': return '\t';
                    case 'r': return '\r';
                    //case 'v': return '\v';
                    default : return c2;
                }
            }
            return c;
        }
        return 0;
    }

    inline bool is_range_repetition(const char * s)
    {
        const char * begin = s;
        while ('0' <= *s && *s <= '9') {
            ++s;
        }
        if (begin == s || !*s || (*s != ',' && *s != '}')) {
            return false;
        }
        if (*s == '}') {
            return true;
        }
        ++s;
        while ('0' <= *s && *s <= '9') {
            ++s;
        }
        return *s == '}';
    }

    inline bool is_meta_char(utf8_consumer & consumer, char_int c)
    {
        return c == '*' || c == '+' || c == '?' || c == '|' || c == '(' || c == ')' || c == '^' || c == '$' || (c == '{' && is_range_repetition(consumer.str()));
    }

    inline State ** st_compilechar(StateAccu & accu, State ** pst,
                                   utf8_consumer & consumer, char_int c, const char * & msg_err)
    {
        if (consumer.valid())
        {
            unsigned n = 0;
            char_int c2 = c;
            utf8_consumer cons = consumer;
            while (get_c(cons, c2)) {
                ++n;
                if (!(c2 = cons.bumpc())) {
                    break;
                }
                if (is_meta_char(cons, c2)) {
                    if (c2 == '*' || c2 == '+' || c2 == '?') {
                        --n;
                    }
                    break;
                }
            }
            if (n > 1) {
                char_int * str = new char_int[n+1];
                char_int * p = str;
                *p = get_c(consumer, c);
                for (unsigned i = 1; i != n; ++i) {
                    *++p = get_c(consumer, consumer.bumpc());
                }
                *++p = 0;
                return &(*pst = accu.sequence(str, n))->out1;
            }
        }

        if (c == '\\' && consumer.valid()) {
            c = consumer.bumpc();
            switch (c) {
                case 'd': return &(*pst = accu.range('0','9'))->out1;
                case 'D': return ident_D(accu, pst, accu.epsilone());
                case 'w': return ident_w(accu, pst, accu.epsilone());
                case 'W': return ident_W(accu, pst, accu.epsilone());
                case 's': return ident_s(accu, pst, accu.epsilone());
                case 'S': return ident_S(accu, pst, accu.epsilone());
                case 'n': return &(*pst = accu.character('\n'))->out1;
                case 't': return &(*pst = accu.character('\t'))->out1;
                case 'r': return &(*pst = accu.character('\r'))->out1;
                case 'v': return &(*pst = accu.character('\v'))->out1;
                default : return &(*pst = accu.character(c))->out1;
            }
        }

        if (c == '[') {
            bool reverse_result = false;
            VectorRange ranges(accu.c_ranges);
            if (consumer.valid() && (c = consumer.bumpc()) != ']') {
                if (c == '^') {
                    reverse_result = true;
                    c = consumer.bumpc();
                }
                if (c == '-') {
                    ranges.push('-');
                    c = consumer.bumpc();
                }
                const unsigned char * cs = consumer.s;
                while (consumer.valid() && c != ']') {
                    const unsigned char * p = consumer.s;
                    char_int prev_c = c;
                    while (c != ']' && c != '-') {
                        if (c == '\\') {
                            char_int cc = consumer.bumpc();
                            switch (cc) {
                                case 'd':
                                    ranges.push('0',    '9');
                                    break;
                                case 'D':
                                    ranges.push(0,      '0'-1);
                                    ranges.push('9'+1,  -1u);
                                    break;
                                case 'w':
                                    ranges.push('a',    'z');
                                    ranges.push('A',    'Z');
                                    ranges.push('0',    '9');
                                    ranges.push('_');
                                    break;
                                case 'W':
                                    ranges.push(0,      '0'-1);
                                    ranges.push('9'+1,  'A'-1);
                                    ranges.push('Z'+1,  '_'-1);
                                    ranges.push('_'+1,  'a'-1);
                                    ranges.push('z'+1,  -1u);
                                    break;
                                case 's':
                                    ranges.push(' ');
                                    ranges.push('\t',   '\v');
                                    break;
                                case 'S':
                                    ranges.push(0,      '\t'-1);
                                    ranges.push('\v'+1, ' '-1);
                                    ranges.push(' '+1,  -1u);
                                    break;
                                case 'n': ranges.push('\n'); break;
                                case 't': ranges.push('\t'); break;
                                case 'r': ranges.push('\r'); break;
                                case 'v': ranges.push('\v'); break;
                                default : ranges.push(cc); break;
                            }
                        }
                        else {
                            ranges.push(c);
                        }

                        if ( ! consumer.valid()) {
                            break;
                        }

                        prev_c = c;
                        c = consumer.bumpc();
                    }

                    if (c == '-') {
                        if (cs == consumer.s) {
                            ranges.push('-');
                        }
                        else if (!consumer.valid()) {
                            msg_err = "missing terminating ]";
                            return nullptr;
                        }
                        else if (consumer.getc() == ']') {
                            ranges.push('-');
                            consumer.bumpc();
                        }
                        else if (consumer.s == p) {
                            ranges.push('-');
                        }
                        else {
                            c = consumer.bumpc();
                            if ((msg_err = check_interval(prev_c, c))) {
                                return nullptr;
                            }
                            if (!ranges.ranges.empty()) {
                                ranges.ranges.pop_back();
                            }
                            ranges.push(prev_c, c);
                            cs = consumer.s;
                            if (consumer.valid()) {
                                c = consumer.bumpc();
                            }
                        }
                    }
                }
            }

            if (ranges.ranges.empty() || c != ']') {
                msg_err = "missing terminating ]";
                return pst;
            }

            if (ranges.ranges.size() == 1) {
                if (reverse_result) {
                    State * eps = accu.epsilone();
                    *pst = accu.split(accu.range(0, ranges.ranges[0].first-1, eps),
                                      accu.range(ranges.ranges[0].second+1, -1u, eps));
                    return &eps->out1;
                }
                *pst = accu.range(ranges.ranges[0].first, ranges.ranges[0].second);
                return &(*pst)->out1;
            }

            std::sort(ranges.ranges.begin(), ranges.ranges.end());

            VectorRange::iterator first = ranges.ranges.begin();
            VectorRange::iterator last = ranges.ranges.end();
            VectorRange::iterator result = first;

            if (first != last) {
                while (++first != last && !(result->second + 1 >= first->first)) {
                    ++result;
                }
                for (; first != last; ++first) {
                    if (result->second + 1 >= first->first) {
                        if (result->second < first->second) {
                            result->second = first->second;
                        }
                    }
                    else {
                        ++result;
                        *result = *first;
                    }
                }
                ++result;
                ranges.ranges.erase(result, ranges.ranges.end());
                result = ranges.ranges.end();
            }

            State * eps = accu.epsilone();
            first = ranges.ranges.begin();
            if (reverse_result) {
                State * st = accu.range(0, first->first-1, eps);
                char_int cr = first->second;
                while (++first != result) {
                    st = accu.split(st, accu.range(cr+1, first->first-1, eps));
                    cr = first->second;
                }
                st = accu.split(st, accu.range(cr+1, -1u, eps));
                *pst = st;
            }
            else {
                State * st = accu.range(first->first, first->second, eps);
                while (++first != result) {
                    st = accu.split(st, accu.range(first->first, first->second, eps));
                }
                *pst = st;
            }
            return &eps->out1;
        }

        return &(*pst = (c == '.') ? accu.any() : accu.character(c))->out1;
    }


    class ContextClone
    {
        size_t pos;
        size_t poslast;
    public:
        state_list_t sts2;
    private:
        StateAccu & accu;
        unsigned nb_clone;

    public:
        ContextClone(StateAccu & accu, State * st_base, unsigned nb_clone)
        : pos(std::find(accu.sts.rbegin(), accu.sts.rend(), st_base).base() - accu.sts.begin() - 1)
        , poslast(accu.sts.size())
        , accu(accu)
        , nb_clone(nb_clone)
        {
            const size_t size = this->poslast - this->pos;
            this->sts2.resize(size);
            this->accu.indexes.resize(size*2);
            this->accu.sts.reserve(this->accu.sts.size() + size*nb_clone);
            state_list_t::iterator first = accu.sts.begin() + this->pos;
            state_list_t::iterator last = accu.sts.begin() + this->poslast;
            std::vector<unsigned>::iterator idxit = this->accu.indexes.begin();
            for (; first != last; ++first) {
                if ((*first)->out1) {
                    *idxit = this->get_idx((*first)->out1);
                    ++idxit;
                }
                if ((*first)->out2) {
                    *idxit = this->get_idx((*first)->out2);
                    ++idxit;
                }
            }
        }

        State * clone()
        {
            --this->nb_clone;
            state_list_t::iterator last = this->accu.sts.begin() + this->poslast;
            state_list_t::iterator first = this->accu.sts.begin() + this->pos;
            state_list_t::iterator first2 = this->sts2.begin();
            for (; first != last; ++first, ++first2) {
                *first2 = this->copy(*first);
            }
            std::vector<unsigned>::iterator idxit = this->accu.indexes.begin();
            first = this->accu.sts.begin() + this->pos;
            first2 = this->sts2.begin();
            for (; first != last; ++first, ++first2) {
                if ((*first)->out1) {
                    (*first2)->out1 = this->sts2[*idxit];
                    ++idxit;
                }
                if ((*first)->out2) {
                    (*first2)->out2 = this->sts2[*idxit];
                    ++idxit;
                }
            }
            return this->sts2.front();
        }

        std::size_t get_idx(State * st) const {
            return std::find(accu.sts.begin() + this->pos,
                             accu.sts.begin() + this->poslast, st)
            - (accu.sts.begin() + this->pos);
        }

    private:
        State * copy(State * st) {
            if (st->type == SEQUENCE) {
                return this->accu.sequence(new_string_sequence(st->data.sequence, 1));
            }
            State * ret = this->accu.normal(st->type, st->data.range.l, st->data.range.r);
            if (st->is_cap()) {
                if (!this->nb_clone) {
                    ret->num = st->num;
                    st->num = 0;
                    st->type = EPSILONE;
                }
                else {
                    ret->type = EPSILONE;
                }
            }
            return ret;
        }
    };

    using IntermendaryState = std::pair<State*, State**>;

    inline bool is_unique_string_state(State * first, State * last)
    {
        return (first->out1 == last && first->is_simple_char())
            || (first->is_sequence()
                && (!first->out1 || (first->out1->is_epsilone() && first->out1->out1 == last)));
    }

    inline void transform_to_sequence(State * st, size_t m)
    {
        if (st->is_simple_char()) {
            st->data.sequence = new_string_sequence(st->data.range.l, m);
        }
        else {
            const char_int * seq = st->data.sequence.s;
            st->data.sequence = new_string_sequence(st->data.sequence, m);
            delete [] seq;
        }
        st->type = SEQUENCE;
    }

    inline IntermendaryState intermendary_st_compile(StateAccu & accu,
                                                     utf8_consumer & consumer,
                                                     const char * & msg_err,
                                                     int recusive = 0)
    {
        State st(EPSILONE);
        State ** pst = &st.out1;
        State ** spst = pst;
        State * bst = &st;
        State * eps = nullptr;
        bool special = true;

        char_int c = consumer.bumpc();

        while (c) {
            /**///std::cout << "c: " << (c) << std::endl;
            if (c == '^' || c == '$') {
                *pst = c == '^' ? accu.begin() : accu.last();
                c = consumer.bumpc();
                pst = &(*pst)->out1;
                special = true;
                continue;
            }

            if (!is_meta_char(consumer, c)) {
                do {
                    spst = pst;
                    if (!(pst = st_compilechar(accu, pst, consumer, c, msg_err))) {
                        return IntermendaryState(nullptr, nullptr);
                    }
                    if (is_meta_char(consumer, c = consumer.bumpc())) {
                        break;
                    }
                } while (c);
                special = false;
            }
            else {
                if (special && c != '(' && c != ')' && c != '|') {
                    msg_err = "nothing to repeat";
                    return IntermendaryState(nullptr, nullptr);
                }
                switch (c) {
                    case '?': {
                        *pst = accu.finish();
                        *spst = accu.split(*pst, *spst);
                        pst = &(*pst)->out1;
                        spst = pst;
                        special = true;
                        break;
                    }
                    case '*':
                        *spst = accu.split(accu.finish(), *spst);
                        *pst = *spst;
                        pst = &(*spst)->out1->out1;
                        spst = pst;
                        special = true;
                        break;
                    case '+':
                        *pst = accu.split(accu.finish(), *spst);
                        spst = pst;
                        pst = &(*pst)->out1->out1;
                        special = true;
                        break;
                    case '|':
                        if (!eps) {
                            eps = accu.epsilone();
                        }
                        *pst = eps;
                        bst = accu.split(bst == &st ? st.out1 : bst);
                        pst = &bst->out2;
                        spst = pst;
                        special = true;
                        break;
                    case '{': {
                        special = true;
                        /**///std::cout << ("{") << std::endl;
                        char * end = nullptr;
                        unsigned m = strtoul(consumer.str(), &end, 10);
                        /**///std::cout << ("end ") << *end << std::endl;
                        /**///std::cout << "m: " << (m) << std::endl;
                        if (*end != '}') {
                            /**///std::cout << ("reste") << std::endl;
                            //{m,}
                            if (*(end+1) == '}') {
                                /**///std::cout << ("infini") << std::endl;
                                if (m == 1) {
                                    *pst = accu.split(accu.finish(), *spst);
                                    spst = pst;
                                    pst = &(*pst)->out1->out1;
                                }
                                else if (m) {
                                    State * e = accu.finish();
                                    if (m > 2 && is_unique_string_state(*spst, *pst)) {
                                        transform_to_sequence(*spst, m);
                                        (*spst)->out1 = e;
                                        *spst = accu.split(e, *spst);
                                    }
                                    else {
                                        *pst = e;
                                        ContextClone cloner(accu, *spst, m-1);
                                        std::size_t idx = cloner.get_idx(e);
                                        State ** lst = &e->out1;
                                        while (--m) {
                                            *pst = cloner.clone();
                                            lst = pst;
                                            pst = &cloner.sts2[idx]->out1;
                                        }
                                        *pst = accu.split(e, *lst);
                                    }
                                    pst = &e->out1;
                                }
                                else {
                                    *spst = accu.split(accu.finish(), *spst);
                                    *pst = *spst;
                                    pst = &(*spst)->out1->out1;
                                    spst = pst;
                                }
                            }
                            //{m,n}
                            else {
                                /**///std::cout << ("range") << std::endl;
                                unsigned n = strtoul(end+1, &end, 10);
                                if (m > n || (0 == m && 0 == n)) {
                                    msg_err = "numbers out of order in {} quantifier";
                                    return IntermendaryState(nullptr, nullptr);
                                }
                                /**///std::cout << "n: " << (n) << std::endl;
                                n -= m;
                                if (n > 50) {
                                    msg_err = "numbers too large in {} quantifier";
                                    return IntermendaryState(nullptr, nullptr);
                                }
                                if (0 == m) {
                                    --end;
                                    /**///std::cout << ("m = 0") << std::endl;
                                    if (n != 1) {
                                        /**///std::cout << ("n != 1") << std::endl;
                                        State * e = accu.finish();
                                        State * split = accu.split();
                                        *pst = split;
                                        ContextClone cloner(accu, *spst, n-1);
                                        std::size_t idx = cloner.get_idx(split);
                                        split->out1 = e;
                                        State * cst = split;

                                        while (--n) {
                                            cst->out2 = cloner.clone();
                                            cst = cloner.sts2[idx];
                                            cst->out1 = e;
                                        }
                                        cst->type = EPSILONE;
                                        pst = &e->out1;
                                        *spst = accu.split(e, *spst);
                                    }
                                    else {
                                        *pst = accu.finish();
                                        *spst = accu.split(*pst, *spst);
                                        pst = &(*pst)->out1;
                                    }
                                }
                                else {
                                    --end;
                                    State * finish = accu.finish();
                                    if (m > 1 && is_unique_string_state(*spst, *pst)) {
                                        State * lst = finish;
                                        if ((*spst)->is_simple_char()) {
                                            char_int cst = (*spst)->data.range.l;
                                            while (n--) {
                                                lst = accu.split(finish, accu.character(cst, lst));
                                            }
                                        }
                                        else {
                                            while (n--) {
                                                lst = accu.split(finish, accu.sequence(
                                                    new_string_sequence((*spst)->data.sequence, 1), lst
                                                ));
                                            }
                                        }
                                        transform_to_sequence(*spst, m);
                                        (*spst)->out1 = lst;
                                    }
                                    else {
                                        State * e = accu.epsilone();
                                        *pst = e;
                                        ContextClone cloner(accu, *spst, m-1+n);
                                        std::size_t idx = cloner.get_idx(e);
                                        pst = &e->out1;
                                        State * lst = e;
                                        while (--m) {
                                            *pst = cloner.clone();
                                            lst = cloner.sts2[idx];
                                            pst = &lst->out1;
                                        }

                                        while (n--) {
                                            lst->type = SPLIT;
                                            lst->out1 = finish;
                                            lst->out2 = cloner.clone();
                                            lst = cloner.sts2[idx];
                                        }
                                        lst->out1 = finish;
                                        lst->type = EPSILONE;
                                    }
                                    pst = &finish->out1;
                                }
                            }
                        }
                        //{0}
                        else if (0 == m) {
                            msg_err = "numbers is 0 in {}";
                            return IntermendaryState(nullptr, nullptr);
                        }
                        //{m}
                        else {
                            if (1 != m) {
                                if (is_unique_string_state(*spst, *pst)) {
                                    transform_to_sequence(*spst, m);
                                }
                                else {
                                    /**///std::cout << ("fixe ") << m << std::endl;
                                    State * e = accu.epsilone();
                                    *pst = e;
                                    ContextClone cloner(accu, *spst, m-1);
                                    std::size_t idx = cloner.get_idx(e);
                                    while (--m) {
                                        /**///std::cout << ("clone") << std::endl;
                                        *pst = cloner.clone();
                                        pst = &cloner.sts2[idx]->out1;
                                    }
                                }
                            }
                            end -= 1;
                        }
                        consumer.str(end + 1 + 1);
                        /**///std::cout << "'" << (*consumer.s) << "'" << std::endl;
                        break;
                    }
                    case ')':
                        if (0 == recusive) {
                            msg_err = "unmatched parentheses";
                            return IntermendaryState(nullptr, nullptr);
                        }

                        if (!eps) {
                            eps = accu.epsilone();
                        }
                        *pst = eps;
                        pst = &eps->out1;
                        return IntermendaryState(bst == &st ? st.out1 : bst, pst);
                    default:
                        return IntermendaryState(nullptr, nullptr);
                    case '(':
                        special = false;
                        if (*consumer.s == '?' && *(consumer.s+1) == ':') {
                            if (!*consumer.s || !(*consumer.s+1) || !(*consumer.s+2)) {
                                msg_err = "unmatched parentheses";
                                return IntermendaryState(nullptr, nullptr);
                            }
                            consumer.s += 2;
                            State * epsgroup = accu.epsilone();
                            IntermendaryState intermendary = intermendary_st_compile(accu, consumer, msg_err, recusive+1);
                            if (intermendary.first) {
                                epsgroup->out1 = intermendary.first;
                                *pst = epsgroup;
                                spst = pst;
                                pst = intermendary.second;
                            }
                            else if (nullptr == intermendary.second) {
                                return IntermendaryState(nullptr, nullptr);
                            }
                            break;
                        }
                        State * stopen = accu.cap_open();
                        IntermendaryState intermendary = intermendary_st_compile(accu, consumer, msg_err, recusive+1);
                        if (intermendary.first) {
                            stopen->out1 = intermendary.first;
                            *pst = stopen;
                            spst = pst;
                            pst = intermendary.second;
                            *pst = accu.cap_close();
                            pst = &(*pst)->out1;
                        }
                        else if (nullptr == intermendary.second) {
                            return IntermendaryState(nullptr, nullptr);
                        }
                        break;
                }
                c = consumer.bumpc();
            }
        }

        if (0 != recusive) {
            msg_err = "unmatched parentheses";
            return IntermendaryState(nullptr, nullptr);
        }
        return IntermendaryState(bst == &st ? st.out1 : bst, pst);
    }

    class StateParser
    {
#ifdef RE_PARSER_POOL_STATE
        struct Deleter {
            void operator()(State *) const
            {}
        };
#else
        using Deleter = StateDeleter;
#endif
    public:
        StateParser()
        {
            this->m_accu.sts.reserve(32);
        }

        StateParser(const StateParser &) = delete;
        StateParser& operator=(const StateParser &) = delete;

        void compile(const char * s, const char * * msg_err = nullptr, size_t * pos_err = nullptr)
        {
            this->m_accu.clear();

            const char * err = nullptr;
            utf8_consumer consumer(s);
            this->m_root = intermendary_st_compile(this->m_accu, consumer, err).first;
            if (msg_err) {
                *msg_err = err;
            }
            if (pos_err) {
                *pos_err = err ? consumer.str() - s : 0;
            }

            while (this->m_root && this->m_root->is_epsilone()) {
                this->m_root = this->m_root->out1;
            }

            if (err || ! this->m_root) {
                this->m_accu.clear();
            }
            else if (this->m_root) {
                remove_epsilone(this->m_accu.sts, Deleter());

                state_list_t::iterator first = this->m_accu.sts.begin();
                state_list_t::iterator last = this->m_accu.sts.end();
                state_list_t::iterator first_cap = std::stable_partition(first, last, IsNotCapture());

                for (unsigned n = this->nb_capture(); first != first_cap; ++first, ++n) {
                    (*first)->num = n;
                }
            }
        }

        ~StateParser()
        {
            this->m_accu.clear();
        }

        const State * root() const
        {
            return this->m_root;
        }

        const state_list_t & states() const
        {
            return this->m_accu.sts;
        }

        unsigned nb_capture() const
        {
            return this->m_accu.num_cap;
        }

        bool empty() const
        {
            return this->m_accu.sts.empty();
        }

        void clear()
        {
            this->m_accu.clear();
            this->m_root = nullptr;
        }

        void clear_and_shrink()
        {
            this->m_accu.clear_and_shrink();
            this->m_root = nullptr;
        }

    private:
        State * m_root = nullptr;
        StateAccu m_accu;
    };
}  // namespace re

