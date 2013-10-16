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

inline void st_to_string(StatesWrapper & stw, StateBase * st,
                         std::ostream& os, unsigned depth = 0)
{
    size_t n = std::find(stw.states.begin(), stw.states.end(), st) - stw.states.begin() + 1;
    os << std::string(depth, '\t') << n;
    if (st && stw.get_num_at(st) != -30u) {
        os << "\t" << st->utfc << "\t" << *st << "\n";
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

inline std::string st_to_string(StateBase * st)
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
        StateChar st_d('d');
        StateChar st_c('c', &st_d);
        StateChar st_b('b', &st_c);
        StateChar st_a('a', &st_b);
        Reg rgx("abcd");
        BOOST_CHECK_EQUAL(st_to_string(&st_a), rgx.to_string());
    }
    {
        StateBorder st_first(1);
        StateBorder st_last(0);
        StateChar st_a('a', &st_last);
        st_first.out1 = &st_a;
        Reg rgx("^a$");
        BOOST_CHECK_EQUAL(st_to_string(&st_first), rgx.to_string());
    }
    {
        StateSplit split;
        StateChar st_a('a', &split);
        StateFinish finish;
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
        StateChar st_b('b');
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
        StateMultiTest st_multi;
        st_multi.push_checker(new CheckerInterval('a', 'c'));
        st_multi.push_checker(new CheckerString("f"));
        Reg rgx("[a-cf]");
        BOOST_CHECK_EQUAL(st_to_string(&st_multi), rgx.to_string());
    }
    {
        StateMultiTest st_multi;
        st_multi.push_checker(new CheckerInterval('a', 'c'));
        st_multi.push_checker(new CheckerInterval('y', 'z'));
        st_multi.push_checker(new CheckerInterval('f', 'h'));
        st_multi.push_checker(new CheckerString("tfv"));
        {
            Reg rgx("[ta-cfy-zf-hv]");
            BOOST_CHECK_EQUAL(st_to_string(&st_multi), rgx.to_string());
        }
        st_multi.result_true_check = false;
        {
            Reg rgx("[^ta-cfy-zf-hv]");
            BOOST_CHECK_EQUAL(st_to_string(&st_multi), rgx.to_string());
        }
    }
    {
        StateChar st_b('b');
        StateChar st_a('a', &st_b);
        StateSplit split(&st_b, &st_a);
        Reg rgx("a?b");
        BOOST_CHECK_EQUAL(st_to_string(&split), rgx.to_string());
    }
    {
        StateBorder last(0);
        StateClose close2(&last);
        StateAny any;
        StateSplit split_any(&close2, &any);
        any.out1 = &split_any;

        StateOpen open2(&split_any);
        StateChar st_d('d', &open2);

        StateClose close1(&st_d);
        StateChar st_c('c', &close1);
        StateSplit split_c(&close1, &st_c);

        StateChar st_b('b', &split_c);
        StateSplit split_b(&split_c, &st_b);

        StateChar st_a('a', &split_b);
        StateSplit split_a(&split_b, &st_a);

        StateOpen open1(&split_a);

        Reg rgx("(a?b?c?)d(.*)$");
        BOOST_CHECK_EQUAL(st_to_string(&open1), rgx.to_string());
    }
    {
        StateChar st_a2('a');
        StateSplit split2(0, &st_a2);
        StateChar st_a1('a', &split2);
        StateSplit split1(&split2, &st_a1);
        Reg rgx("a{0,2}");
         BOOST_CHECK_EQUAL(st_to_string(&split1), rgx.to_string());
    }
    {
        StateChar st_b('b');
        StateSplit split3(&st_b);
        StateChar st_a1('a', &split3);
        StateChar st_a2('a', &split3);
        StateSplit split2(&st_a1, &split3);
        StateSplit split1(&st_a2, &split2);
        StateChar st_a3('a', &split1);
        StateChar st_a4('a', &st_a3);
        Reg rgx("a{2,4}b");
        BOOST_CHECK_EQUAL(st_to_string(&st_a4), rgx.to_string());
    }
    {
        StateChar st_b('b');
        StateChar st_a2('a');
        StateSplit split1(&st_b, &st_a2);
        st_a2.out1 = &split1;
        {
            Reg rgx("a{0,}b");
            BOOST_CHECK_EQUAL(st_to_string(&split1), rgx.to_string());
        }
        StateChar st_a3('a', &split1);
        StateChar st_a4('a', &st_a3);
        {
            Reg rgx("a{2,}b");
            BOOST_CHECK_EQUAL(st_to_string(&st_a4), rgx.to_string());
        }
    }
    {
        StateChar st1('}');
        StateChar st2('-', &st1);
        StateChar st3('2', &st2);
        StateChar st4('{', &st3);
        StateChar st5('a', &st4);
        Reg rgx("a{2-}");
        BOOST_CHECK_EQUAL(st_to_string(&st5), rgx.to_string());
    }
    {
        StateChar st_b('b');
        StateFinish finish;
        StateSplit split2(&finish, &st_b);
        st_b.out1 = &split2;
        StateChar st_a2('a');
        StateSplit split1(&split2, &st_a2);
        st_a2.out1 = &split1;
        {
            Reg rgx("a{0,}b*");
            BOOST_CHECK_EQUAL(st_to_string(&split1), rgx.to_string());
        }
        StateChar st_a3('a', &split1);
        StateChar st_a4('a', &st_a3);
        split1.out1 = &st_b;
        {
            Reg rgx("a{2,}b+");
            BOOST_CHECK_EQUAL(st_to_string(&st_a4), rgx.to_string());
        }
    }
    {
        StateClose close1;
        StateDigit digit;
        StateSplit split(&close1, &digit);
        digit.out1 = &split;
        StateOpen open1(&split);
        Reg rgx("(\\d*)");
        BOOST_CHECK_EQUAL(st_to_string(&open1), rgx.to_string());
    }
    {
        StateChar st_c4(multi_char("Þ"));
        StateChar st_c3('a', &st_c4);
        StateChar st_c2(multi_char("Ë"), &st_c3);
        StateChar st_c1(multi_char("¥"), &st_c2);
        Reg rgx("¥ËaÞ");
        BOOST_CHECK_EQUAL(st_to_string(&st_c1), rgx.to_string());
    }
}

