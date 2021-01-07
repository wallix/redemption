/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2020
Author(s): Wallix Team
*/

#include "mod/internal/replay_mod.hpp"

#include "utils/timebase.hpp"
#include "utils/timeval_ops.hpp"
#include "core/events.hpp"

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/front/fake_front.hpp"

RED_TEST_DELEGATE_OSTREAM(timeval, "{" << _.tv_sec << ", " << _.tv_usec << "}");

RED_AUTO_TEST_CASE(TestInteractiveTargetMod)
{
    uint16_t w = 1024;
    uint16_t h = 768;
    EventContainer events;
    TestGraphic gd(w, h);
    ScreenInfo screen_info{w, h, BitsPerPixel::BitsPP16};
    FakeFront front(screen_info);
    std::string error_msg;
    bool wait_for_escape = false;
    bool replay_on_loop = false;
    bool play_video_with_corrupted_bitmap = false;
    ReplayMod::Verbose verbose {};

    ReplayMod replay_mod(
        events,
        gd,
        front,
        FIXTURES_PATH "/kpd_input.mwrm",
        error_msg,
        wait_for_escape,
        replay_on_loop,
        play_video_with_corrupted_bitmap,
        verbose
    );

    RED_REQUIRE(events.queue.size() == 1);
    Event& ev = *events.queue[0];
    RED_TEST_CONTEXT_DATA(timeval t, t, {
        timeval{1, 308471},
        timeval{2, 910153},
        timeval{4, 111064},
        timeval{5, 311743},
        timeval{6, 516745},
        timeval{9, 720549},
        timeval{11, 723016},
        timeval{13, 725721},
        timeval{14, 965110},
        timeval{17, 368991},
        timeval{18, 970699},
        timeval{20, 171903},
        timeval{21, 774104},
        timeval{22, 975249},
        timeval{24, 176985},
        timeval{25, 378462},
        timeval{26, 579927},
        timeval{27, 780921},
        timeval{28, 983105},
        timeval{34, 991458},
        timeval{36, 592889},
        timeval{37, 794320},
        timeval{38, 995885},
        timeval{40, 597508},
        timeval{41, 799025},
        timeval{43, 386},
        timeval{44, 602224},
        timeval{45, 802989},
        timeval{47, 3974},
        timeval{48, 605855},
        timeval{49, 806837},
        timeval{51, 7828},
        timeval{52, 609606},
        timeval{53, 151448},
        timeval{53, 151448},
    }) {
        RED_REQUIRE(!ev.garbage);
        ev.actions.exec_timeout(ev);
        RED_CHECK(ev.alarm.trigger_time == t);
    }

    RED_CHECK(replay_mod.get_mod_signal() == BackEvent_t::BACK_EVENT_STOP);
    RED_CHECK(ev.garbage);
    RED_REQUIRE(events.queue.size() == 1);

    RED_CHECK_IMG(gd, FIXTURES_PATH "/img_ref/replay_mod.png");
}
