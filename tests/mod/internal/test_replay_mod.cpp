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
#include "core/events.hpp"

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/front/fake_front.hpp"

namespace std
{
    RED_TEST_DELEGATE_OSTREAM(::MonotonicTimePoint, "{" << _.time_since_epoch().count() << "}");
}

using namespace std::literals::chrono_literals;

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

    auto& event_cont = detail::ProtectedEventContainer::get_events(events);
    RED_REQUIRE(event_cont.size() == 1);
    Event& ev = *event_cont[0];
    RED_TEST_CONTEXT_DATA(MonotonicTimePoint t, t, {
        MonotonicTimePoint{1s + 308471us},
        MonotonicTimePoint{2s + 910153us},
        MonotonicTimePoint{4s + 111064us},
        MonotonicTimePoint{5s + 311743us},
        MonotonicTimePoint{6s + 516745us},
        MonotonicTimePoint{9s + 720549us},
        MonotonicTimePoint{11s + 723016us},
        MonotonicTimePoint{13s + 725721us},
        MonotonicTimePoint{14s + 965110us},
        MonotonicTimePoint{17s + 368991us},
        MonotonicTimePoint{18s + 970699us},
        MonotonicTimePoint{20s + 171903us},
        MonotonicTimePoint{21s + 774104us},
        MonotonicTimePoint{22s + 975249us},
        MonotonicTimePoint{24s + 176985us},
        MonotonicTimePoint{25s + 378462us},
        MonotonicTimePoint{26s + 579927us},
        MonotonicTimePoint{27s + 780921us},
        MonotonicTimePoint{28s + 983105us},
        MonotonicTimePoint{34s + 991458us},
        MonotonicTimePoint{36s + 592889us},
        MonotonicTimePoint{37s + 794320us},
        MonotonicTimePoint{38s + 995885us},
        MonotonicTimePoint{40s + 597508us},
        MonotonicTimePoint{41s + 799025us},
        MonotonicTimePoint{43s + 386us},
        MonotonicTimePoint{44s + 602224us},
        MonotonicTimePoint{45s + 802989us},
        MonotonicTimePoint{47s + 3974us},
        MonotonicTimePoint{48s + 605855us},
        MonotonicTimePoint{49s + 806837us},
        MonotonicTimePoint{51s + 7828us},
        MonotonicTimePoint{52s + 609606us},
        MonotonicTimePoint{53s + 151448us},
        MonotonicTimePoint{53s + 151448us},
    }) {
        RED_REQUIRE(!ev.garbage);
        ev.actions.exec_timeout(ev);
        RED_CHECK(ev.alarm.trigger_time == t);
    }

    RED_CHECK(replay_mod.get_mod_signal() == BackEvent_t::BACK_EVENT_STOP);
    RED_CHECK(ev.garbage);
    RED_REQUIRE(event_cont.size() == 1);

    RED_CHECK_IMG(gd, FIXTURES_PATH "/img_ref/replay_mod.png");
}
