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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan

*/

#define RED_TEST_MODULE TestDiffTimeval
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/difftimeval.hpp"

namespace std
{
    // this is a hack...
    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, std::chrono::microseconds const & duration)
    {
        return out << duration.count();
    }
}

RED_AUTO_TEST_CASE(TestUstime)
{
    std::chrono::microseconds res(0);
    timeval timev;
    timev.tv_sec = 0;
    timev.tv_usec = 0;
    res = ustime(timev);
    RED_CHECK_EQUAL(res, std::chrono::microseconds::zero());

    timev.tv_sec = 0;
    timev.tv_usec = 57321;
    res = ustime(timev);
    RED_CHECK_EQUAL(res, std::chrono::microseconds(57321LL));


    timev.tv_sec = 489435;
    timev.tv_usec = 0;
    res = ustime(timev);
    RED_CHECK_EQUAL(res, std::chrono::microseconds(489435000000LL));

    timev.tv_sec = 1520;
    timev.tv_usec = 254321;

    res = ustime(timev);
    RED_CHECK_EQUAL(res, std::chrono::microseconds(1520254321LL));

}

RED_AUTO_TEST_CASE(TestTvtimeustime)
{
    timeval restv = tvtime();
    std::chrono::microseconds resus = ustime();
    RED_CHECK(restv.tv_usec > 0);
    RED_CHECK(restv.tv_sec > 0);
    RED_CHECK(resus.count() > 0);
}

// RED_AUTO_TEST_CASE(TestUsecToTimeval)
// {
//     uint64_t usec = 0L;
//     timeval res;
//     res = usectotimeval(usec);
//     RED_CHECK_EQUAL(res.tv_sec,  0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//
//     usec = 459327L;
//     res = usectotimeval(usec);
//     RED_CHECK_EQUAL(res.tv_sec,  0);
//     RED_CHECK_EQUAL(res.tv_usec, 459327);
//
//     usec = 5476000000LL;
//     res = usectotimeval(usec);
//     RED_CHECK_EQUAL(res.tv_sec,  5476);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//
//     usec = 751379115L;
//     res = usectotimeval(usec);
//     RED_CHECK_EQUAL(res.tv_sec,  751);
//     RED_CHECK_EQUAL(res.tv_usec, 379115);
//
// }

RED_AUTO_TEST_CASE(TestAddUsecTimeval)
{
    uint64_t usec = 0L;
    struct timeval init = { 0, 0 };
    timeval res;

    res = addusectimeval(std::chrono::microseconds(usec), init);
    RED_CHECK_EQUAL(res.tv_sec,  0);
    RED_CHECK_EQUAL(res.tv_usec, 0);


    usec = 654322;
    res = addusectimeval(std::chrono::microseconds(usec), init);
    RED_CHECK_EQUAL(res.tv_sec,  0);
    RED_CHECK_EQUAL(res.tv_usec, 654322);

    usec = 42737465L;
    res = addusectimeval(std::chrono::microseconds(usec), init);
    RED_CHECK_EQUAL(res.tv_sec,  42);
    RED_CHECK_EQUAL(res.tv_usec, 737465);

    usec = 0;
    init.tv_sec =  78;
    init.tv_usec = 426891;
    res = addusectimeval(std::chrono::microseconds(usec), init);
    RED_CHECK_EQUAL(res.tv_sec,  78);
    RED_CHECK_EQUAL(res.tv_usec, 426891);

    init.tv_sec =  78;
    init.tv_usec = 0;
    usec = 91234;
    res = addusectimeval(std::chrono::microseconds(usec), init);
    RED_CHECK_EQUAL(res.tv_sec,  78);
    RED_CHECK_EQUAL(res.tv_usec, 91234);

    init.tv_sec =  815;
    init.tv_usec = 945862;
    usec = 457369421;
    res = addusectimeval(std::chrono::microseconds(usec), init);
    RED_CHECK_EQUAL(res.tv_sec,  1273);
    RED_CHECK_EQUAL(res.tv_usec, 315283);


}

RED_AUTO_TEST_CASE(TestLessThanTimeVal)
{
    timeval before, after;
    before.tv_sec = 0;
    before.tv_usec = 0;
    after = before;

    RED_CHECK_EQUAL(lessthantimeval(before, after), false);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after.tv_sec += 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;
    after.tv_usec += 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;
    after.tv_sec += 1;
    after.tv_usec += 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);


    before.tv_sec = 184635;
    before.tv_usec = 564732;
    after = before;

    RED_CHECK_EQUAL(lessthantimeval(before, after), false);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after.tv_sec += 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;
    after.tv_usec += 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;
    after.tv_sec += 1;
    after.tv_usec += 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);


    after = before;

    RED_CHECK_EQUAL(lessthantimeval(before, after), false);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after.tv_sec += 4651;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;
    after.tv_usec += 573;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;
    after.tv_sec += 1272;
    after.tv_usec += 151236;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    after = before;

    RED_CHECK_EQUAL(lessthantimeval(before, after), false);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    before.tv_sec -= 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    before = after;
    before.tv_usec -= 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    before = after;
    before.tv_sec -= 1;
    before.tv_usec -= 1;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);


    before = after;

    RED_CHECK_EQUAL(lessthantimeval(before, after), false);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    before.tv_sec -= 13477;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    before = after;
    before.tv_usec -= 321697;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

    before = after;
    before.tv_sec -= 324;
    before.tv_usec -= 3277;
    RED_CHECK_EQUAL(lessthantimeval(before, after), true);
    RED_CHECK_EQUAL(lessthantimeval(after, before), false);

}


// RED_AUTO_TEST_CASE(Testaddtimeval)
// {
//     timeval time1, time2;
//     timeval res, sym;
//     time1.tv_sec  = 0;
//     time1.tv_usec = 0;
//     time2 = time1;
//
//     res = addtimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//     sym = addtimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time1.tv_usec = 486541;
//     res = addtimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 486541);
//     sym = addtimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time1.tv_usec = 0;
//     time1.tv_sec = 975;
//     res = addtimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 975);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//     sym = addtimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time1.tv_sec = 1237;
//     time1.tv_usec = 4873;
//     res = addtimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 1237);
//     RED_CHECK_EQUAL(res.tv_usec, 4873);
//     sym = addtimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time1.tv_sec = 234321;
//     time1.tv_usec = 354879;
//     time2.tv_sec = 157532;
//     time2.tv_usec = 245789;
//     res = addtimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 391853);
//     RED_CHECK_EQUAL(res.tv_usec, 600668);
//     sym = addtimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time1.tv_sec = 236321;
//     time1.tv_usec = 354879;
//     time2.tv_sec = 157532;
//     time2.tv_usec = 745789;
//     res = addtimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 393854);
//     RED_CHECK_EQUAL(res.tv_usec, 100668);
//     sym = addtimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
// }

RED_AUTO_TEST_CASE(TestHowLongToWait)
{
    timeval time1, time2;
    timeval res, sym;

    time1.tv_sec  = 0;
    time1.tv_usec = 0;
    time2 = time1;
    res = how_long_to_wait(time1, time2);
    RED_CHECK_EQUAL(res.tv_sec, 0);
    RED_CHECK_EQUAL(res.tv_usec, 0);
    sym = how_long_to_wait(time2, time1);
    RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
    RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);


    time1.tv_sec  = 3457;
    time1.tv_usec = 215733;
    res = how_long_to_wait(time1, time2);
    RED_CHECK_EQUAL(res.tv_sec, 3457);
    RED_CHECK_EQUAL(res.tv_usec, 215733);
    sym = how_long_to_wait(time2, time1);
    RED_CHECK_EQUAL(0, sym.tv_sec);
    RED_CHECK_EQUAL(0, sym.tv_usec);

    time2 = time1;
    res = how_long_to_wait(time1, time2);
    RED_CHECK_EQUAL(res.tv_sec, 0);
    RED_CHECK_EQUAL(res.tv_usec, 0);
    sym = how_long_to_wait(time2, time1);
    RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
    RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);


    time2.tv_usec = 215733 + 457352;
    res = how_long_to_wait(time2, time1);
    RED_CHECK_EQUAL(res.tv_sec, 0);
    RED_CHECK_EQUAL(res.tv_usec, 457352);

    time2.tv_sec = 3457 + 54573;
    res = how_long_to_wait(time2, time1);
    RED_CHECK_EQUAL(res.tv_sec, 54573);
    RED_CHECK_EQUAL(res.tv_usec, 457352);


    time2.tv_usec = time1.tv_usec;
    res = how_long_to_wait(time2, time1);
    RED_CHECK_EQUAL(res.tv_sec, 54573);
    RED_CHECK_EQUAL(res.tv_usec, 0);

    time1.tv_sec  = 67235;
    time1.tv_usec = 454324;
    time2.tv_sec  = 421;
    time2.tv_usec = 842136;
    res = how_long_to_wait(time1, time2);
    RED_CHECK_EQUAL(res.tv_sec, 66813);
    RED_CHECK_EQUAL(res.tv_usec, 612188);


}

// RED_AUTO_TEST_CASE(Testabsdifftimeval)
// {
//     timeval time1, time2;
//     timeval res, sym;
//
//     time1.tv_sec  = 0;
//     time1.tv_usec = 0;
//     time2 = time1;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time1.tv_sec  = 3457;
//     time1.tv_usec = 215733;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 3457);
//     RED_CHECK_EQUAL(res.tv_usec, 215733);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time2 = time1;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time2.tv_usec += 457352;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 457352);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time2.tv_sec += 54573;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 54573);
//     RED_CHECK_EQUAL(res.tv_usec, 457352);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time2.tv_usec = time1.tv_usec;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 54573);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time1.tv_sec  = 67235;
//     time1.tv_usec = 454324;
//     time2.tv_sec  = 421;
//     time2.tv_usec = 842136;
//     res = absdifftimeval(time1, time2);
//     RED_CHECK_EQUAL(res.tv_sec, 66813);
//     RED_CHECK_EQUAL(res.tv_usec, 612188);
//     sym = absdifftimeval(time2, time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
// }

// RED_AUTO_TEST_CASE(TestMultTimeval)
// {
//     timeval init, res;
//     uint64_t mult;
//
//     mult = 0;
//     init.tv_sec  = 0;
//     init.tv_usec = 0;
//     res = multtimeval(mult, init);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//
//     mult = 0;
//     init.tv_sec  = 3235357;
//     init.tv_usec = 243230;
//     res = multtimeval(mult, init);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//
//     mult = 43721099;
//     init.tv_sec  = 0;
//     init.tv_usec = 0;
//     res = multtimeval(mult, init);
//     RED_CHECK_EQUAL(res.tv_sec, 0);
//     RED_CHECK_EQUAL(res.tv_usec, 0);
//
//     mult = 1;
//     init.tv_sec  = 234221;
//     init.tv_usec = 1098;
//     res = multtimeval(mult, init);
//     RED_CHECK_EQUAL(res.tv_sec, 234221);
//     RED_CHECK_EQUAL(res.tv_usec, 1098);
//
//     mult = 12;
//     init.tv_sec  = 24671;
//     init.tv_usec = 11566;
//     res = multtimeval(mult, init);
//     RED_CHECK_EQUAL(res.tv_sec, 296052);
//     RED_CHECK_EQUAL(res.tv_usec, 138792);
//
//     mult = 33;
//     init.tv_sec  = 7499;
//     init.tv_usec = 111566;
//     res = multtimeval(mult, init);
//     RED_CHECK_EQUAL(res.tv_sec, 247470);
//     RED_CHECK_EQUAL(res.tv_usec, 681678);
//
// }

// RED_AUTO_TEST_CASE(Testmintimeval)
// {
//     timeval time1, time2;
//     timeval res, sym;
//
//     time1.tv_sec  = 67235;
//     time1.tv_usec = 454324;
//     time2.tv_sec  = 421;
//     time2.tv_usec = 842136;
//     res = mintimeval(time1,time2);
//     RED_CHECK_EQUAL(res.tv_sec, time2.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, time2.tv_usec);
//     sym = mintimeval(time2,time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//
//     time1.tv_sec  = 421;
//     time1.tv_usec = 454324;
//     time2.tv_sec  = 421;
//     time2.tv_usec = 842136;
//     res = mintimeval(time1,time2);
//     RED_CHECK_EQUAL(res.tv_sec, time1.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, time1.tv_usec);
//     sym = mintimeval(time2,time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time1.tv_sec  = 246575;
//     time1.tv_usec = 454324;
//     time2.tv_sec  = 421;
//     time2.tv_usec = 454324;
//     res = mintimeval(time1,time2);
//     RED_CHECK_EQUAL(res.tv_sec, time2.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, time2.tv_usec);
//     sym = mintimeval(time2,time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time1.tv_sec  = 246575;
//     time1.tv_usec = 457324;
//     time2.tv_sec  = 421;
//     time2.tv_usec = 454324;
//     res = mintimeval(time1,time2);
//     RED_CHECK_EQUAL(res.tv_sec, time2.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, time2.tv_usec);
//     sym = mintimeval(time2,time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
//     time2 = time1;
//     res = mintimeval(time1,time2);
//     RED_CHECK_EQUAL(res.tv_sec, time2.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, time2.tv_usec);
//     RED_CHECK_EQUAL(res.tv_sec, time1.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, time1.tv_usec);
//     sym = mintimeval(time2,time1);
//     RED_CHECK_EQUAL(res.tv_sec, sym.tv_sec);
//     RED_CHECK_EQUAL(res.tv_usec, sym.tv_usec);
//
// }
