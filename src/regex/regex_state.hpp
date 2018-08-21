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


#pragma once

#include <ostream>
#include <algorithm>

#include "regex_consumer.hpp"

///TODO regex compiler ("..." -> C++)

namespace re {

    const unsigned RANGE            = 1;
    const unsigned FINISH           = 1 << 8;
    const unsigned SPLIT            = 1 << 9;
    const unsigned CAPTURE_OPEN     = 1 << 10;
    const unsigned CAPTURE_CLOSE    = 1 << 11;
    const unsigned EPSILONE         = 1 << 12;
    const unsigned FIRST            = 1 << 13;
    const unsigned LAST             = 1 << 14;
    const unsigned SEQUENCE         = 1 << 15;


    struct Range
    {
        char_int l;
        char_int r;

        bool contains(char_int c) const {
            return this->l <= c && c <= this->r;
        }
    };

    struct SequenceString
    {
        const char_int * s;
        size_t len;

        SequenceString(const char_int * str, size_t slen)
        : s(str)
        , len(slen)
        {}
    };

    struct Sequence
    {
        const char_int * s;
        size_t len;

        unsigned contains(char_int c, utf8_consumer consumer) const {
            if (c == this->s[0]) {
                const char_int * s = this->s + 1;
                while (*s && *s == consumer.bumpc()) {
                    ++s;
                }
                return *s ? 0 : s - this->s;
            }
            return 0;
        }

        Sequence & operator=(const SequenceString& ss)
        {
            this->s = ss.s;
            this->len = ss.len;
            return *this;
        }
    };

    struct State
    {
        explicit State(unsigned type, char_int range_left = 0, char_int range_right = 0,
                       State * out1 = nullptr, State * out2 = nullptr)
        : type(type)
        , num(0)
        , out1(out1)
        , out2(out2)
        {
            this->data.range.l = range_left;
            this->data.range.r = range_right;
        }

        State(const State &) = delete;
        State& operator=(const State &) = delete;

        ~State()
        {
            if (this->type == SEQUENCE) {
                delete[] this->data.sequence.s;
            }
        }

        unsigned check(char_int c, utf8_consumer consumer) const {
            if (this->type == SEQUENCE) {
                return this->data.sequence.contains(c, consumer);
            }
            return this->data.range.contains(c) ? 1 : 0;
        }

        void display(std::ostream& os) const {
            switch (this->type) {
                case RANGE:
                    if (this->data.range.l == 0 && this->data.range.r == ~char_int{}) {
                        os << ".";
                    }
                    else if (this->data.range.l == this->data.range.r) {
                        os << "[" << this->data.range.l << "] '"
                        << utf8_char(this->data.range.l) << "'";
                    }
                    else {
                        os << "[" << this->data.range.l << "-" << this->data.range.r << "] ['"
                        << utf8_char(this->data.range.l) << "'-'" << utf8_char(this->data.range.r)
                        << "']";
                    }
                    break;
                case SEQUENCE: {
                    os << '"';
                    for (const char_int * p = this->data.sequence.s; *p; ++p) {
                        os << utf8_char(*p);
                    }
                    os << '"';
                    break;
                }
                case CAPTURE_CLOSE: os << ")"; break;
                case CAPTURE_OPEN: os << "("; break;
                case EPSILONE: os << "(epsilone)"; break;
                case FINISH: os << "(finish)"; break;
                case SPLIT: os << "(split)"; break;
                case FIRST: os << "^"; break;
                case LAST: os << "$"; break;
                default: os << "???"; break;
            }
        }

        bool is_border() const
        { return this->type & (FIRST|LAST); }

        bool is_cap() const
        { return this->type & (CAPTURE_OPEN|CAPTURE_CLOSE); }

        bool is_cap_open() const
        { return this->type == CAPTURE_OPEN; }

        bool is_cap_close() const
        { return this->type == CAPTURE_CLOSE; }

        bool is_split() const
        { return this->type == SPLIT; }

        bool is_epsilone() const
        { return this->type == EPSILONE || (this->type == FINISH && this->out1); }

        bool is_finish() const
        { return this->type == FINISH; }

        bool is_terminate() const
        { return this->type & (LAST|FINISH); }

        bool is_range() const
        { return this->type == RANGE; }

        bool is_simple_char() const
        { return this->is_range() && this->data.range.l == this->data.range.r; }

        bool is_sequence() const
        { return this->type == SEQUENCE; }

        bool is_uninitialized() const
        { return this->type == 0; }

        unsigned type;
        unsigned num;

        union {
            Range range;
            Sequence sequence;
        } data;

        State *out1;
        State *out2;
    };

    inline std::ostream& operator<<(std::ostream& os, const State& st)
    {
        st.display(os);
        return os;
    }

    inline State * new_character(char_int c, State * out1 = nullptr) {
        return new State(RANGE, c, c, out1); /*NOLINT*/
    }

    inline State * new_range(char_int left, char_int right, State * out1 = nullptr) {
        return new State(RANGE, left, right, out1); /*NOLINT*/
    }

    inline State * new_any(State * out1 = nullptr) {
        return new State(RANGE, 0, ~char_int{}, out1); /*NOLINT*/
    }

    inline State * new_split(State * out1 = nullptr, State * out2 = nullptr) {
        return new State(SPLIT, 0, 0, out1, out2); /*NOLINT*/
    }

    inline State * new_cap_open(State * out1 = nullptr) {
        return new State(CAPTURE_OPEN, 0, 0, out1); /*NOLINT*/
    }

    inline State * new_cap_close(State * out1 = nullptr) {
        return new State(CAPTURE_CLOSE, 0, 0, out1); /*NOLINT*/
    }

    inline State * new_epsilone(State * out1 = nullptr) {
        return new State(EPSILONE, 0, 0, out1); /*NOLINT*/
    }

    inline State * new_finish(State * out1 = nullptr) {
        return new State(FINISH, 0, 0, out1); /*NOLINT*/
    }

    inline State * new_begin(State * out1 = nullptr) {
        return new State(FIRST, 0, 0, out1); /*NOLINT*/
    }

    inline State * new_last() {
        return new State(LAST, 0, 0); /*NOLINT*/
    }

    inline State * new_sequence(const char_int * s, size_t len, State * out1 = nullptr) {
        State * ret = new State(SEQUENCE, 0, 0, out1); /*NOLINT*/
        ret->data.sequence.s = s;
        ret->data.sequence.len = len;
        return ret;
    }

    inline State * new_sequence(const SequenceString & ss, State * out1 = nullptr) {
        return new_sequence(ss.s, ss.len, out1);
    }

    inline SequenceString new_string_sequence(char_int c, std::size_t count) {
        char_int * s = new char_int[count + 1]; /*NOLINT*/
        std::fill(s, s + count, c);
        *(s+count) = 0;
        return SequenceString(s, count);
    }

    inline SequenceString new_string_sequence(const char_int * str, std::size_t len,
                                              std::size_t count) {
        char_int * ret = new char_int[count * len + 1]; /*NOLINT*/
        char_int * p = ret;
        while (count--) {
            std::copy(str, str + len, p);
            p += len;
        }
        *p = 0;
        return SequenceString(ret, count * len);
    }

    inline SequenceString new_string_sequence(const char_int * str, std::size_t count) {
        return new_string_sequence(str, std::char_traits<char_int>::length(str), count);
    }

    inline SequenceString new_string_sequence(const Sequence & seq, std::size_t count) {
        return new_string_sequence(seq.s, seq.len, count);
    }

    inline State * new_sequence(char_int c, std::size_t count, State * out1 = nullptr) {
        return new_sequence(new_string_sequence(c, count), out1);
    }

}  // namespace re

