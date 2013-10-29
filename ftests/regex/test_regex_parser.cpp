/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for bitmap class, compression performance

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDfaRegexParser
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "regex_parser.hpp"

using namespace re;

inline void st_to_string(StatesWrapper & stw, State * st,
                         std::ostream& os, unsigned depth = 0)
{
    size_t n = std::find(stw.states.begin(), stw.states.end(), st) - stw.states.begin() + 1;
    os << std::string(depth, '\t') << n;
    if (st && stw.get_num_at(st) != -30u) {
        os << "\t" << *st << "\n";
        stw.set_num_at(st, -30u);
        st_to_string(stw, st->out1, os, depth+1);
        st_to_string(stw, st->out2, os, depth+1);
    }
    else {
        os << "\n";
    }
}

inline std::string st_to_string(StatesWrapper & stw)
{
    std::ostringstream os;
    st_to_string(stw, stw.root, os);
    return os.str();
}

inline std::string st_to_string(State * st)
{
    std::ostringstream os;
    StatesWrapper stw(st);
    stw.init_nums();
    st_to_string(stw, stw.root, os);
    stw.states.clear();
    return os.str();
}

inline size_t multi_char(const char * c)
{
    return re::utf_consumer(c).bumpc();
}

BOOST_AUTO_TEST_CASE(TestRegexState)
{
    struct Reg {
        Reg(const char * s)
        {
            st_compile(this->stw, s);
        }

        std::string to_string()
        {
            return st_to_string(this->stw);
        }

        StatesWrapper stw;
    };

    {
        State st_d(RANGE, 'd', 'd');
        State st_c(RANGE, 'c', 'c', &st_d);
        State st_b(RANGE, 'b', 'b', &st_c);
        State st_a(RANGE, 'a', 'a', &st_b);
        Reg rgx("abcd");
        BOOST_CHECK_EQUAL(st_to_string(&st_a), rgx.to_string());
    }
    {
        State st_first(FIRST);
        State st_last(LAST);
        State st_a(RANGE, 'a', 'a', &st_last);
        st_first.out1 = &st_a;
        Reg rgx("^a$");
        BOOST_CHECK_EQUAL(st_to_string(&st_first), rgx.to_string());
    }
    {
        State split(SPLIT);
        State st_a(RANGE, 'a', 'a', &split);
        State finish(FINISH);
        split.out1 = &finish;
        split.out2 = &st_a;
        {
            Reg rgx("a*");
            BOOST_CHECK_EQUAL(st_to_string(&split), rgx.to_string());
        }
        {
            Reg rgx("a+");
            BOOST_CHECK_EQUAL(st_to_string(&st_a), rgx.to_string());
        }
        State st_b(RANGE, 'b', 'b');
        split.out1 = &st_b;
        {
            Reg rgx("a*b");
            BOOST_CHECK_EQUAL(st_to_string(&split), rgx.to_string());
        }
        {
            Reg rgx("a+b");
            BOOST_CHECK_EQUAL(st_to_string(&st_a), rgx.to_string());
        }
    }
    {
        State st_f(RANGE, 'f', 'f');
        State range(RANGE, 'a', 'c');
        State split(SPLIT, 0, 0, &range, &st_f);
        Reg rgx("[a-cf]");
        BOOST_CHECK_EQUAL(st_to_string(&split), rgx.to_string());
    }
    {
        State range1(RANGE, 'a', 'c');
        State range2(RANGE, 'f', 'h');
        State split1(SPLIT, 0, 0, &range1, &range2);
        State st_f(RANGE, 't', 't');
        State split2(SPLIT, 0, 0, &split1, &st_f);
        State st_vw(RANGE, 'v', 'w');
        State split3(SPLIT, 0, 0, &split2, &st_vw);
        State range3(RANGE, 'y', 'z');
        State split4(SPLIT, 0, 0, &split3, &range3);
        Reg rgx("[ta-cfy-zf-hvw]");
        BOOST_CHECK_EQUAL(st_to_string(&split4), rgx.to_string());
    }
    {
        State left(RANGE, 0, 't'-1);
        State right(RANGE, 't'+1, -1u);
        State split(SPLIT, 0, 0, &left, &right);
        Reg rgx("[^t]");
        BOOST_CHECK_EQUAL(st_to_string(&split), rgx.to_string());
    }
    {
        State st_b(RANGE, 'b', 'b');
        State st_a(RANGE, 'a', 'a', &st_b);
        State split(SPLIT, 0, 0, &st_b, &st_a);
        Reg rgx("a?b");
        BOOST_CHECK_EQUAL(st_to_string(&split), rgx.to_string());
    }
    {
        State last(LAST);
        State close2(CAPTURE_CLOSE, 0, 0, &last);
        State any(RANGE, 0, -1u);
        State split_any(SPLIT, 0, 0, &close2, &any);
        any.out1 = &split_any;

        State open2(CAPTURE_OPEN, 0, 0, &split_any);
        State st_d(RANGE, 'd', 'd', &open2);

        State close1(CAPTURE_CLOSE, 0, 0, &st_d);
        State st_c(RANGE, 'c', 'c', &close1);
        State split_c(SPLIT, 0, 0, &close1, &st_c);

        State st_b(RANGE, 'b', 'b', &split_c);
        State split_b(SPLIT, 0, 0, &split_c, &st_b);

        State st_a(RANGE, 'a', 'a', &split_b);
        State split_a(SPLIT, 0, 0, &split_b, &st_a);

        State open1(CAPTURE_OPEN, 0, 0, &split_a);

        Reg rgx("(a?b?c?)d(.*)$");
        BOOST_CHECK_EQUAL(st_to_string(&open1), rgx.to_string());
    }
    {
        State finish(FINISH);
        State st_a2(RANGE, 'a', 'a', &finish);
        State split2(SPLIT, 0, 0, &finish, &st_a2);
        State st_a1(RANGE, 'a', 'a', &split2);
        State split1(SPLIT, 0, 0, &finish, &st_a1);
        Reg rgx("a{0,2}");
        BOOST_CHECK_EQUAL(st_to_string(&split1), rgx.to_string());
    }
    {
        State st_b(RANGE, 'b', 'b');
        State st_a1(RANGE, 'a', 'a', &st_b);
        State split2(SPLIT, 0, 0, &st_b, &st_a1);
        State st_a2(RANGE, 'a', 'a', &split2);
        State split1(SPLIT, 0, 0, &st_b, &st_a2);
        State st_a3(RANGE, 'a', 'a', &split1);
        State st_a4(RANGE, 'a', 'a', &st_a3);
        Reg rgx("a{2,4}b");
        BOOST_CHECK_EQUAL(st_to_string(&st_a4), rgx.to_string());
    }
    {
        State st_b(RANGE, 'b', 'b');
        State st_a2(RANGE, 'a', 'a');
        State split1(SPLIT, 0, 0, &st_b, &st_a2);
        st_a2.out1 = &split1;
        {
            Reg rgx("a{0,}b");
            BOOST_CHECK_EQUAL(st_to_string(&split1), rgx.to_string());
        }
        State st_a3(RANGE, 'a', 'a', &st_a2);
        {
            Reg rgx("a{2,}b");
            BOOST_CHECK_EQUAL(st_to_string(&st_a3), rgx.to_string());
        }
    }
    {
        State st1(RANGE, '}', '}');
        State st2(RANGE, '-', '-', &st1);
        State st3(RANGE, '2', '2', &st2);
        State st4(RANGE, '{', '{', &st3);
        State st5(RANGE, 'a', 'a', &st4);
        Reg rgx("a{2-}");
        BOOST_CHECK_EQUAL(st_to_string(&st5), rgx.to_string());
    }
    {
        State finish(FINISH);
        State st_b(RANGE, 'b', 'b');
        State split2(SPLIT, 0, 0, &finish, &st_b);
        st_b.out1 = &split2;
        State st_a2(RANGE, 'a', 'a');
        State split1(SPLIT, 0, 0, &split2, &st_a2);
        st_a2.out1 = &split1;
        {
            Reg rgx("a{0,}b*");
            BOOST_CHECK_EQUAL(st_to_string(&split1), rgx.to_string());
        }
        split1.out1 = &st_b;
        State st_a3(RANGE, 'a', 'a', &st_a2);
        {
            Reg rgx("a{2,}b+");
            BOOST_CHECK_EQUAL(st_to_string(&st_a3), rgx.to_string());
        }
    }
    {
        State close1(CAPTURE_CLOSE);
        State digit(RANGE, '0', '9');
        State split(SPLIT, 0, 0, &close1, &digit);
        digit.out1 = &split;
        State open1(CAPTURE_OPEN, 0, 0, &split);
        Reg rgx("(\\d*)");
        BOOST_CHECK_EQUAL(st_to_string(&open1), rgx.to_string());
    }
    {
        State st_c4(RANGE, multi_char("Þ"), multi_char("Þ"));
        State st_c3(RANGE, 'a', 'a', &st_c4);
        State st_c2(RANGE, multi_char("Ë"), multi_char("Ë"), &st_c3);
        State st_c1(RANGE, multi_char("¥"), multi_char("¥"), &st_c2);
        Reg rgx("¥ËaÞ");
        BOOST_CHECK_EQUAL(st_to_string(&st_c1), rgx.to_string());
    }
}

