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

#ifndef REDEMPTION_FTESTS_REGEX_REGEX_PARSE_HPP
#define REDEMPTION_FTESTS_REGEX_REGEX_PARSE_HPP

#include "regex.hpp"

namespace re {

    typedef StateIdentifier<'w', IdentifierWordTrait>       StateWord;
    typedef StateIdentifier<'W', IdentifierNoWordTrait>     StateNoWord;
    typedef StateIdentifier<'d', IdentifierDigitTrait>      StateDigit;
    typedef StateIdentifier<'D', IdentifierNoDigitTrait>    SateNoDigit;
    typedef StateIdentifier<'s', IdentifierSpaceTrait>      StateSpace;
    typedef StateIdentifier<'S', IdentifierNoSpaceTrait>    StateNoSpace;

    inline StateBase * c2st(char_int c)
    {
        switch (c) {
            case 'd': return new StateDigit;
            case 'D': return new SateNoDigit;
            case 'w': return new StateWord;
            case 'W': return new StateNoWord;
            case 's': return new StateSpace;
            case 'S': return new StateNoSpace;
            case 'n': return new_character('\n');
            case 't': return new_character('\t');
            case 'r': return new_character('\r');
            //case 'v': return new_character('\v');
            default : return new_character(c);
        }
    }

    inline const char * check_interval(char_int a, char_int b)
    {
        bool valid = ('0' <= a && a <= '9' && '0' <= b && b <= '9')
                  || ('a' <= a && a <= 'z' && 'a' <= b && b <= 'z')
                  || ('A' <= a && a <= 'Z' && 'A' <= b && b <= 'Z');
        return (valid && a <= b) ? 0 : "range out of order in character class";
    }

    inline StateBase * str2stchar(utf_consumer & consumer, char_int c, const char * & msg_err)
    {
        if (c == '\\' && consumer.valid()) {
            return c2st(consumer.bumpc());
        }

        if (c == '[') {
            StateMultiTest * st = new StateMultiTest;
            std::string str;
            if (consumer.valid() && (c = consumer.bumpc()) != ']') {
                if (c == '^') {
                    st->result_true_check = false;
                    c = consumer.bumpc();
                }
                if (c == '-') {
                    str += '-';
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
                                case 'd': st->push_checker(new CheckerDigit); break;
                                case 'D': st->push_checker(new CheckerNoDigit); break;
                                case 'w': st->push_checker(new CheckerWord); break;
                                case 'W': st->push_checker(new CheckerNoWord); break;
                                case 's': st->push_checker(new CheckerSpace); break;
                                case 'S': st->push_checker(new CheckerNoSpace); break;
                                case 'n': str += '\n'; break;
                                case 't': str += '\t'; break;
                                case 'r': str += '\r'; break;
                                //case 'v': str += '\v'; break;
                                default : str += utf_char(cc); break;
                            }
                        }
                        else {
                            str += utf_char(c);
                        }

                        if ( ! consumer.valid()) {
                            break;
                        }

                        prev_c = c;
                        c = consumer.bumpc();
                    }

                    if (c == '-') {
                        if (cs == consumer.s) {
                            str += '-';
                        }
                        else if (!consumer.valid()) {
                            msg_err = "missing terminating ]";
                            return 0;
                        }
                        else if (consumer.getc() == ']') {
                            str += '-';
                            consumer.bumpc();
                        }
                        else if (consumer.s == p) {
                            str += '-';
                        }
                        else {
                            c = consumer.bumpc();
                            if ((msg_err = check_interval(prev_c, c))) {
                                return 0;
                            }
                            if (str.size()) {
                                str.erase(str.size()-1);
                            }
                            st->push_checker(new CheckerInterval(prev_c, c));
                            cs = consumer.s;
                            if (consumer.valid()) {
                                c = consumer.bumpc();
                            }
                        }
                    }
                }
            }

            if (!str.empty()) {
                st->push_checker(new CheckerString(str));
            }

            if (c != ']') {
                msg_err = "missing terminating ]";
                delete st;
                st = 0;
            }

            return st;
        }

        return c == '.' ? new_any() : new_character(c);
    }

    inline bool is_range_repetition(const char * s)
    {
        const char * begin = s;
        while (*s && '0' <= *s && *s <= '9') {
            ++s;
        }
        if (begin == s || !*s || (*s != ',' && *s != '}')) {
            return false;
        }
        if (*s == '}') {
            return true;
        }
        begin = ++s;
        while (*s && '0' <= *s && *s <= '9') {
            ++s;
        }
        return *s && *s == '}';
    }

    inline bool is_meta_char(utf_consumer & consumer, char_int c)
    {
        return c == '*' || c == '+' || c == '?' || c == '|' || c == '(' || c == ')' || c == '^' || c == '$' || (c == '{' && is_range_repetition(consumer.str()));
    }


    struct ContextClone {
        std::vector<StateBase*> sts;
        std::vector<StateBase*> sts2;
        const StateBase * st;
        size_t p2;
        const bool definied_out1;
        const bool definied_out2;

        ContextClone(const StateBase * st_base)
        : sts()
        , sts2()
        , st(st_base)
        , definied_out1(st->out1)
        , definied_out2(st->out2)
        {
            if (this->definied_out1) {
                append_state(st->out1, this->sts);
            }
            if (this->definied_out2) {
                this->p2 = this->sts.size();
                append_state(st->out2, this->sts);
            }
            this->sts2.reserve(this->sts.size());
        }

        StateBase * clone()
        {
            StateBase * replicant = this->simple_clone(this->st);
            if (this->definied_out1 || this->definied_out2) {
                typedef std::vector<StateBase*>::iterator iterator;
                iterator last = this->sts.end();
                for (iterator first = this->sts.begin(), first2 = sts2.begin(); last != this->sts.end(); ++first, ++first2) {
                    *first2 = this->simple_clone(*first);
                    (*first)->id = 0;
                }
                for (iterator first = this->sts.begin(), first2 = sts2.begin(); last != this->sts.end(); ++first, ++first2) {
                    if ((*first)->out1) {
                        (*first2)->out1 = this->sts2[std::find(this->sts.begin(), this->sts.end(), (*first)->out1) - this->sts.begin()];
                    }
                    if ((*first)->out2) {
                        (*first2)->out2 = this->sts2[std::find(this->sts.begin(), this->sts.end(), (*first)->out2) - this->sts.begin()];
                    }
                }
                if (this->definied_out1) {
                    replicant->out1 = this->sts2[0];
                }
                if (this->definied_out2) {
                    replicant->out2 = this->sts2[p2];
                }
            }
            return replicant;
        }

        StateBase * simple_clone(const StateBase * st)
        {
            return st == &state_finish ? &state_finish : st->clone();
        }
    };

    typedef std::pair<StateBase*, StateBase**> IntermendaryState;

    inline IntermendaryState intermendary_str2reg(utf_consumer & consumer,
                                                  bool & has_epsilone,
                                                  const char * & msg_err,
                                                  int recusive = 0, bool ismatch = true)
    {
        struct FreeState {
            static IntermendaryState invalide(StateBase& st)
            {
                free_st(st.out2);
                return IntermendaryState(0,0);
            }
        };

        struct selected {
            static StateBase ** next_pst(StateBase ** pst)
            {
                if (*pst && *pst != &state_finish) {
                    pst = &(*pst)->out1;
                }
                return pst;
            }
        };

        StateBase st(0);
        StateBase ** pst = &st.out1;
        StateBase ** st_one = 0;
        StateBase ** prev_st_one = 0;
        StateBase * bst = &st;

        StateBase ** besplit[50] = {0};
        StateBase *** pesplit = besplit;

        char_int c = consumer.bumpc();

        while (c) {
            /**///std::cout << "c: " << (c) << std::endl;
            if (c == '^' || c == '$') {
                pst = selected::next_pst(pst);
                *pst = new StateBorder(c == '^');
                if (st_one) {
                    *st_one = *pst;
                    prev_st_one = st_one;
                    st_one = 0;
                }

                if ((c = consumer.bumpc()) && !is_meta_char(consumer, c)) {
                    pst = &(*pst)->out1;
                }
                continue;
            }

            if (!is_meta_char(consumer, c)) {
                pst = selected::next_pst(pst);
                if (!(*pst = str2stchar(consumer, c, msg_err))) {
                    return FreeState::invalide(st);
                }

                if (st_one) {
                    *st_one = *pst;
                    prev_st_one = st_one;
                    st_one = 0;
                }

                while ((c = consumer.bumpc()) && !is_meta_char(consumer, c)) {
                    pst = &(*pst)->out1;
                    if (!(*pst = str2stchar(consumer, c, msg_err))) {
                        return FreeState::invalide(st);
                    }
                }
            }
            else {
                if (c != '(' && c != ')' && (bst->out1 == 0 || bst->out1->is_border())) {
                    msg_err = "nothing to repeat";
                    return FreeState::invalide(st);
                }
                switch (c) {
                    case '?': {
                        StateBase ** tmp = st_one;
                        st_one = &(*pst)->out1;
                        *pst = new_split(0, *pst);
                        if (prev_st_one) {
                            *prev_st_one = *pst;
                            prev_st_one = tmp;
                        }
                        pst = &(*pst)->out1;
                        break;
                    }
                    case '*':
                        *pst = new_split(&state_finish, *pst);
                        if (prev_st_one) {
                            *prev_st_one = *pst;
                            prev_st_one = 0;
                        }
                        (*pst)->out2->out1 = *pst;
                        pst = &(*pst)->out1;
                        break;
                    case '+':
                        (*pst)->out1 = new_split(&state_finish, *pst);
                        pst = &(*pst)->out1->out1;
                        break;
                    case '|':
                        *pesplit = *pst ? &(*pst)->out1 : pst;
                        ++pesplit;
                        bst->out1 = new_split(0, bst->out1);
                        bst = bst->out1;
                        if (st_one) {
                            *pesplit = st_one;
                            st_one = 0;
                            ++pesplit;
                        }
                        pst = &bst->out1;
                        break;
                    case '{': {
                        /**///std::cout << ("{") << std::endl;
                        char * end = 0;
                        unsigned m = strtoul(consumer.str(), &end, 10);
                        /**///std::cout << ("end ") << *end << std::endl;
                        /**///std::cout << "m: " << (m) << std::endl;
                        if (*end != '}') {
                            /**///std::cout << ("reste") << std::endl;
                            if (*(end+1) == '}') {
                                /**///std::cout << ("infini") << std::endl;
                                if (m == 1) {
                                    (*pst)->out1 = new_split(0, *pst);
                                    pst = &(*pst)->out1->out1;
                                }
                                else if (m) {
                                    ContextClone cloner(*pst);
                                    pst = selected::next_pst(pst);
                                    while (--m) {
                                        *pst = cloner.clone();
                                        pst = &(*pst)->out1;
                                    }
                                    *pst = new_split(0, cloner.clone());
                                    (*pst)->out2->out1 = *pst;
                                    pst = &(*pst)->out1;
                                }
                                else {
                                    *pst = new_split(0, *pst);
                                    if (prev_st_one) {
                                        *prev_st_one = *pst;
                                        prev_st_one = 0;
                                    }
                                    (*pst)->out2->out1 = *pst;
                                    pst = &(*pst)->out1;
                                }
                            }
                            else {
                                /**///std::cout << ("range") << std::endl;
                                unsigned n = strtoul(end+1, &end, 10);
                                if (m > n || (0 == m && 0 == n)) {
                                    msg_err = "numbers out of order in {} quantifier";
                                    return FreeState::invalide(st);
                                }
                                /**///std::cout << "n: " << (n) << std::endl;
                                n -= m;
                                if (n > 50) {
                                    msg_err = "numbers too large in {} quantifier";
                                    return FreeState::invalide(st);
                                }
                                if (0 == m) {
                                    --end;
                                    /**///std::cout << ("m = 0") << std::endl;
                                    if (n != 1) {
                                        /**///std::cout << ("n != 1") << std::endl;
                                        ContextClone cloner(*pst);
                                        StateBase ** tmp = st_one;
                                        st_one = &(*pst)->out1;
                                        *pst = new_split(0, *pst);
                                        if (prev_st_one) {
                                            *prev_st_one = *pst;
                                            prev_st_one = tmp;
                                        }
                                        pst = &(*pst)->out1;

                                        while (--n) {
                                            *pst = cloner.clone();
                                            *st_one = *pst;
                                            prev_st_one = st_one;
                                            st_one = &(*pst)->out1;
                                            *pst = new_split(0, *pst);
                                            *prev_st_one = *pst;
                                            pst = &(*pst)->out1;
                                        }
                                    }
                                }
                                else {
                                    --end;
                                    ContextClone cloner(*pst);
                                    pst = selected::next_pst(pst);
                                    while (--m) {
                                        *pst = cloner.clone();
                                        pst = &(*pst)->out1;
                                    }

                                    StateBase * split = new_split();
                                    while (n--) {
                                        *pst = new_split(0, split);
                                        (*pst)->out1 = cloner.clone();
                                        (*pst)->out1->out1 = split;
                                        pst = &(*pst)->out2;
                                    }
                                    pst = &split->out1;
                                }
                            }
                        }
                        else if (0 == m) {
                            msg_err = "numbers is 0 in {}";
                            return FreeState::invalide(st);
                        }
                        else {
                            /**///std::cout << ("fixe ") << m << std::endl;
                            ContextClone cloner(*pst);
                            pst = selected::next_pst(pst);
                            while (--m) {
                                /**///std::cout << ("clone") << std::endl;
                                *pst = cloner.clone();
                                pst = &(*pst)->out1;
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
                            return FreeState::invalide(st);
                        }

                        if (ismatch) {
                            pst = selected::next_pst(pst);
                            *pst = new StateBase(CAPTURE_CLOSE);
                        }
                        else if (besplit != pesplit) {
                            has_epsilone = true;
                            pst = selected::next_pst(pst);
                            *pst = new StateBase(ismatch ? CAPTURE_CLOSE : EPSILONE);
                        }

                        if (st_one) {
                            *st_one = *pst;
                            prev_st_one = st_one;
                            st_one = 0;
                        }
                        if (prev_st_one) {
                            *prev_st_one = *pst;
                        }

                        for (StateBase *** first = besplit; first != pesplit; ++first) {
                            if (**first) {
                                (**first)->out1 = *pst;
                            }
                            else {
                                (**first) = *pst;
                            }
                        }

                        return IntermendaryState(st.out1, pst);
                        break;
                    default:
                        return FreeState::invalide(st);
                    case '(':
                        if (*consumer.s == '?' && *(consumer.s+1) == ':') {
                            if (!*consumer.s || !(*consumer.s+1) || !(*consumer.s+2)) {
                                msg_err = "unmatched parentheses";
                                return FreeState::invalide(st);
                            }
                            consumer.s += 2;
                            IntermendaryState intermendary = intermendary_str2reg(consumer, has_epsilone, msg_err, recusive+1, false);
                            if (intermendary.first) {
                                pst = selected::next_pst(pst);
                                *pst= intermendary.first;
                                if (st_one) {
                                    *st_one = *pst;
                                    prev_st_one = st_one;
                                    st_one = 0;
                                }
                                pst = intermendary.second;
                            }
                            else if (0 == intermendary.second) {
                                return FreeState::invalide(st);
                            }
                            break;
                        }
                        IntermendaryState intermendary = intermendary_str2reg(consumer, has_epsilone, msg_err, recusive+1);
                        if (intermendary.first) {
                            pst = selected::next_pst(pst);
                            *pst = new StateBase(CAPTURE_OPEN, 0, intermendary.first);
                            if (st_one) {
                                *st_one = *pst;
                                prev_st_one = st_one;
                                st_one = 0;
                            }
                            pst = intermendary.second;
                        }
                        else if (0 == intermendary.second) {
                            return FreeState::invalide(st);
                        }
                        break;
                }
                c = consumer.bumpc();
            }
        }

        if (0 != recusive) {
            msg_err = "unmatched parentheses";
            return FreeState::invalide(st);
        }
        return IntermendaryState(st.out1, pst);
    }

    inline void remove_epsilone(StateBase * st, std::vector<StateBase*>& epsilone_sts)
    {
        if (st && st->id != -3u) {
            st->id = -3u;
            StateBase * nst = st->out1;
            while (nst && nst->is_epsilone()) {
                if (nst->id != -3u) {
                    epsilone_sts.push_back(nst);
                    nst->id = -3u;
                }
                nst = nst->out1;
            }
            st->out1 = nst;
            remove_epsilone(st->out1, epsilone_sts);
            remove_epsilone(st->out2, epsilone_sts);
        }
    }

    inline StateBase* str2reg(const char * s, const char * * msg_err = 0,
                              size_t * pos_err = 0)
    {
        bool has_epsilone = false;
        const char * err = 0;
        utf_consumer consumer(s);
        StateBase * st = intermendary_str2reg(consumer, has_epsilone, err).first;
        if (err) {
            if (msg_err) {
                *msg_err = err;
            }
            if (pos_err) {
                *pos_err = consumer.str() - s;
            }
        }
        else if (has_epsilone) {
            typedef std::vector<StateBase*> states_t;
            states_t removed;
            remove_epsilone(st, removed);
            std::for_each(removed.begin(), removed.end(), StateDeleter());
        }
        return st;
    }
}

#endif