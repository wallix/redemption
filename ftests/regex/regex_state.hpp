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

#ifndef REDEMPTION_REGEX_STATE_HPP
#define REDEMPTION_REGEX_STATE_HPP

#include <ostream>
#include <vector>
#include <string>

#include "regex_consumer.hpp"

///TODO regex compiler ("..." -> C++)

namespace re {

    const unsigned NORMAL           = 0;
    const unsigned FINISH           = 1 << 8;
    const unsigned SPLIT            = 1 << 9;
    const unsigned CAPTURE_OPEN     = 1 << 10;
    const unsigned CAPTURE_CLOSE    = 1 << 11;
    const unsigned EPSILONE         = 1 << 12;
    const unsigned FIRST            = 1 << 13;
    const unsigned LAST             = 1 << 14;

    struct StateBase
    {
        StateBase(unsigned type, StateBase * out1 = 0, StateBase * out2 = 0)
        : type(type)
        , num(0)
        , out1(out1)
        , out2(out2)
        {}

    public:
        virtual ~StateBase()
        {}

        virtual bool check(char_int c) const = 0;
        virtual StateBase * clone() const = 0;
        virtual void display(std::ostream& os) const = 0;

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
        { return this->type == EPSILONE; }

        bool is_finish() const
        { return this->type == FINISH; }

        bool is_terminate() const
        { return this->type & (LAST|FINISH); }

        unsigned type;
        unsigned num;

        StateBase *out1;
        StateBase *out2;
    };

    inline std::ostream& operator<<(std::ostream& os, const StateBase& st)
    {
        st.display(os);
        return os;
    }

    struct StateSplit : public StateBase
    {
        StateSplit(StateBase * out1 = 0, StateBase * out2 = 0)
        : StateBase(SPLIT, out1, out2)
        {}

        virtual StateBase * clone() const
        {
            return new StateSplit;
        }

        virtual bool check(char_int /*c*/) const
        {
            return false;
        }

        virtual void display(std::ostream& os) const
        {
            os << "(split)";
        }
    };

    struct StateChar : public StateBase
    {
        StateChar(char_int c, StateBase * out1 = 0, StateBase * out2 = 0)
        : StateBase(NORMAL, out1, out2)
        , uc(c)
        {}

        virtual StateBase * clone() const
        {
            return new StateChar(this->uc);
        }

        virtual bool check(char_int c) const
        {
            return this->uc == c;
        }

        virtual void display(std::ostream& os) const
        {
            os << "'" << utf_char(this->uc) << "'";
        }

    private:
        char_int uc;
    };

    struct StateAny : public StateBase
    {
        StateAny(StateBase * out1 = 0, StateBase * out2 = 0)
        : StateBase(NORMAL, out1, out2)
        {}

        virtual StateBase * clone() const
        {
            return new StateAny();
        }

        virtual bool check(char_int /*c*/) const
        {
            return true;
        }

        virtual void display(std::ostream& os) const
        {
            os << "any";
        }
    };

    struct StateClose : public StateBase
    {
        StateClose(StateBase * out1 = 0, StateBase * out2 = 0)
        : StateBase(CAPTURE_CLOSE, out1, out2)
        {}

        virtual StateBase * clone() const
        {
            return new StateClose();
        }

        virtual bool check(char_int /*c*/) const
        {
            return false;
        }

        virtual void display(std::ostream& os) const
        {
            os << "(close)";
        }
    };

    struct StateOpen : public StateBase
    {
        StateOpen(StateBase * out1 = 0, StateBase * out2 = 0)
        : StateBase(CAPTURE_OPEN, out1, out2)
        {}

        virtual StateBase * clone() const
        {
            return new StateOpen();
        }

        virtual bool check(char_int /*c*/) const
        {
            return false;
        }

        virtual void display(std::ostream& os) const
        {
            os << "(open)";
        }
    };

    struct StateEpsilone : public StateBase
    {
        StateEpsilone(StateBase * out1 = 0, StateBase * out2 = 0)
        : StateBase(EPSILONE, out1, out2)
        {}

        virtual StateBase * clone() const
        {
            return new StateEpsilone();
        }

        virtual bool check(char_int /*c*/) const
        {
            return false;
        }

        virtual void display(std::ostream& os) const
        {
            os << "(epsilone)";
        }
    };

    struct StateFinish : StateBase
    {
        StateFinish()
        : StateBase(FINISH)
        {}

        virtual ~StateFinish()
        {}

        virtual bool check(char_int /*c*/) const
        {
            /**///std::cout << char(this->c&0xFF) << "-" << char(rend);
            return true;
        }

        virtual StateBase * clone() const
        {
            return new StateFinish;
        }

        virtual void display(std::ostream& os) const
        {
            os << "Finish";
        }
    };

    struct StateRange : StateBase
    {
        StateRange(char_int c1, char_int c2, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, out1, out2)
        , uc_first(c1)
        , uc_last(c2)
        {}

        virtual ~StateRange()
        {}

        virtual bool check(char_int c) const
        {
            /**///std::cout << char(this->c&0xFF) << "-" << char(rend);
            return this->uc_first <= c && c <= this->uc_last;
        }

        virtual StateBase * clone() const
        {
            return new StateRange(this->uc_first, this->uc_last);
        }

        virtual void display(std::ostream& os) const
        {
            os << "[" << utf_char(this->uc_first) << "-" << utf_char(this->uc_last) << "]";
        }


        char_int uc_first;
        char_int uc_last;
    };

    template<char Identifier, typename Trait>
    struct StateIdentifier : StateBase
    {
        StateIdentifier(StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, out1, out2)
        {}

        virtual ~StateIdentifier()
        {}

        virtual StateIdentifier * clone() const
        {
            return new StateIdentifier;
        }

        virtual void display(std::ostream& os) const
        {
            os << "\\" << Identifier;
        }

        virtual bool check(char_int c) const
        {
            return Trait::check(c);
        }
    };

    struct IdentifierDigitTrait
    {
        static bool check(char_int c)
        {
            return ('0' <= c && c <= '9');
        }
    };

    struct IdentifierWordTrait
    {
        static bool check(char_int c)
        {
            return ('a' <= c && c <= 'z')
                || ('A' <= c && c <= 'Z')
                || ('0' <= c && c <= '9')
                || c == '_';
        }
    };

    struct IdentifierSpaceTrait
    {
        static bool check(char_int c)
        {
            return (' ' == c || '\t' == c || '\n' == c);
        }
    };

    template<typename Trait>
    struct IdentifierNotTrait
    {
        static bool check(char_int c)
        {
            return !Trait::check(c);
        }
    };

    typedef IdentifierNotTrait<IdentifierWordTrait>  IdentifierNotWordTrait;
    typedef IdentifierNotTrait<IdentifierDigitTrait> IdentifierNotDigitTrait;
    typedef IdentifierNotTrait<IdentifierSpaceTrait> IdentifierNotSpaceTrait;

    typedef StateIdentifier<'w', IdentifierWordTrait>       StateWord;
    typedef StateIdentifier<'W', IdentifierNotWordTrait>    StateNoWord;
    typedef StateIdentifier<'d', IdentifierDigitTrait>      StateDigit;
    typedef StateIdentifier<'D', IdentifierNotDigitTrait>   SateNotDigit;
    typedef StateIdentifier<'s', IdentifierSpaceTrait>      StateSpace;
    typedef StateIdentifier<'S', IdentifierNotSpaceTrait>   StateNotSpace;

    struct StateCharacters : StateBase
    {
        StateCharacters(const std::string& s, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, out1, out2)
        , str(s)
        {}

        virtual ~StateCharacters()
        {}

        virtual bool check(char_int c) const
        {
            /**///std::cout << str;
            return utf_contains(this->str.c_str(), c);
        }

        virtual StateBase * clone() const
        {
            return new StateCharacters(this->str);
        }

        virtual void display(std::ostream& os) const
        {
            os << "[" << this->str << "]";
        }

        std::string str;
    };

    struct StateBorder : StateBase
    {
        StateBorder(bool is_first)
        : StateBase(is_first ? FIRST : LAST)
        {}

        virtual ~StateBorder()
        {}

        virtual bool check(char_int /*c*/) const
        {
            /**///std::cout << (this->c == FIRST ? "^" : "$");
            return false;
        }

        virtual StateBase * clone() const
        {
            return new StateBorder(this->type == FIRST);
        }

        virtual void display(std::ostream& os) const
        {
            os << (this->type == FIRST ? "^" : "$");
        }
    };

    struct StateMultiTest : StateBase
    {
        struct Checker {
            virtual bool check(char_int c) const = 0;
            virtual Checker * clone() const = 0;
            virtual void display(std::ostream& os) const = 0;
            virtual ~Checker() {}
        };

        std::vector<Checker*> checkers;
        typedef std::vector<Checker*>::iterator checker_iterator;
        typedef std::vector<Checker*>::const_iterator checker_const_iterator;
        bool result_true_check;


        StateMultiTest(StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, out1, out2)
        , checkers()
        , result_true_check(true)
        {}

        virtual ~StateMultiTest()
        {
            for (checker_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                delete *first;
            }
        }

        virtual bool check(char_int c) const
        {
            for (checker_const_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                if ((*first)->check(c)) {
                    return this->result_true_check;
                }
            }
            return !this->result_true_check;
        }

        virtual StateBase * clone() const
        {
            StateMultiTest * ret = new StateMultiTest;
            ret->result_true_check = this->result_true_check;
            for (checker_const_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                ret->push_checker((*first)->clone());
            }
            return ret;
        }

        virtual void display(std::ostream& os) const
        {
            checker_const_iterator first = this->checkers.begin();
            checker_const_iterator last = this->checkers.end();
            if (first != last) {
                (*first)->display(os);
                while (++first != last) {
                    os << "|";
                    (*first)->display(os);
                }
            }
        }

        void push_checker(Checker * checker)
        {
            this->checkers.push_back(checker);
        }
    };

    struct CheckerString : StateMultiTest::Checker
    {
        CheckerString(const std::string& s)
        : str(s)
        {}

        virtual ~CheckerString()
        {}

        virtual bool check(char_int c) const
        {
            return utf_contains(this->str.c_str(), c);
        }

        virtual Checker * clone() const
        {
            return new CheckerString(this->str);
        }

        virtual void display(std::ostream& os) const
        {
            os << "[" << this->str << "]";
        }

        std::string str;
    };

    struct CheckerInterval : StateMultiTest::Checker
    {
        CheckerInterval(int first, int last)
        : begin(first)
        , end(last)
        {}

        virtual ~CheckerInterval()
        {}

        virtual bool check(char_int c) const
        {
            return this->begin <= c && c <= this->end;
        }

        virtual Checker* clone() const
        {
            return new CheckerInterval(this->begin, this->end);
        }

        virtual void display(std::ostream& os) const
        {
            os << "[" << utf_char(this->begin) << "-" << utf_char(this->end) << "]";
        }

        char_int begin;
        char_int end;
    };

    template<char Identifier, typename Trait>
    struct CheckerIdentifier : StateMultiTest::Checker
    {
        CheckerIdentifier()
        {}

        virtual ~CheckerIdentifier()
        {}

        virtual bool check(char_int c) const
        {
            return Trait::check(c);
        }

        virtual Checker* clone() const
        {
            return new CheckerIdentifier;
        }

        virtual void display(std::ostream& os) const
        {
            os << "\\" << Identifier;
        }
    };

    typedef CheckerIdentifier<'w', IdentifierWordTrait>     CheckerWord;
    typedef CheckerIdentifier<'W', IdentifierNotWordTrait>  CheckerNoWord;
    typedef CheckerIdentifier<'d', IdentifierDigitTrait>    CheckerDigit;
    typedef CheckerIdentifier<'D', IdentifierNotDigitTrait> CheckerNotDigit;
    typedef CheckerIdentifier<'s', IdentifierSpaceTrait>    CheckerSpace;
    typedef CheckerIdentifier<'S', IdentifierNotSpaceTrait> CheckerNotSpace;

}

#endif
