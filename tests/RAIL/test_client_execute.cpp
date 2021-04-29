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

#include "RAIL/client_execute.hpp"

#include "test_only/test_framework/redemption_unit_tests.hpp"


RED_AUTO_TEST_CASE(TestDirections)
{
    Rect window(100,100,200,200);

    ClientExecute::Zone zone;
    using Area = ClientExecute::WindowArea;

    RED_CHECK(Rect(124, 100, 152, 3) == zone.get_zone(Area::N, window));
    RED_CHECK(Rect(100, 100, 24, 3) == zone.get_zone(Area::NWN, window));
    RED_CHECK(Rect(100, 100, 3, 24) == zone.get_zone(Area::NWW, window));
    RED_CHECK(Rect(100, 124, 3, 152) == zone.get_zone(Area::W, window));
    RED_CHECK(Rect(100, 276, 3, 24) == zone.get_zone(Area::SWW, window));
    RED_CHECK(Rect(100, 297, 24, 3) == zone.get_zone(Area::SWS, window));
    RED_CHECK(Rect(124, 297, 152, 3) == zone.get_zone(Area::S, window));
    RED_CHECK(Rect(276, 297, 24, 3) == zone.get_zone(Area::SES, window));
    RED_CHECK(Rect(297, 276, 3, 24) == zone.get_zone(Area::SEE, window));
    RED_CHECK(Rect(297, 124, 3, 152) == zone.get_zone(Area::E, window));
    RED_CHECK(Rect(297, 100, 3, 24) == zone.get_zone(Area::NEE, window));
    RED_CHECK(Rect(276, 100, 24, 3) == zone.get_zone(Area::NEN, window));

    RED_CHECK(Rect(101, 101, 21, 23) == zone.get_zone(Area::Icon, window));
    RED_CHECK(Rect(122, 101, 66, 23) == zone.get_zone(Area::Title, window));
    RED_CHECK(Rect(151, 101, 37, 23) == zone.get_zone(Area::Resize, window));
    RED_CHECK(Rect(188, 101, 37, 23) == zone.get_zone(Area::Mini, window));
    RED_CHECK(Rect(225, 101, 37, 23) == zone.get_zone(Area::Maxi, window));
    RED_CHECK(Rect(262, 101, 37, 23) == zone.get_zone(Area::Close, window));
}

RED_AUTO_TEST_CASE(TestClientExecuteAction)
{
    using MouseAction = ClientExecute::MouseAction;
    using Act = MouseAction::EventAction;
    auto maximixed = MouseAction::IsMaximized(1);
    auto not_maximixed = MouseAction::IsMaximized(0);

    auto maximixed_state = not_maximixed;

    MonotonicTimePoint::duration delay = std::chrono::milliseconds(500);
    MonotonicTimePoint t{};

    const uint16_t down        = 0x8000;
    const uint16_t release     = 0;
    const uint16_t left_click  = 0x1000;
    const uint16_t move        = 0x0800;

    MouseAction mouse_act;

    auto next_action = [&](uint16_t flags, uint16_t x, uint16_t y){
        return mouse_act.next_mouse_action(
            Rect(10, 10, 600, 500), maximixed_state,
            MouseAction::ResizableHosted(true), t, flags, x, y
        ).action;
    };

    RED_TEST_CONTEXT("double click icon => close") {
        RED_CHECK(Act::Nothing == next_action(left_click | down,    15, 15));
        t += delay;
        RED_CHECK(Act::Nothing == next_action(left_click | release, 15, 15));
        RED_CHECK(Act::Nothing == next_action(left_click | down,    15, 15));
        RED_CHECK(Act::Nothing == next_action(left_click | release, 15, 15));
        RED_CHECK(Act::Close   == next_action(left_click | down,    15, 15));
        RED_CHECK(Act::Nothing == next_action(left_click | release, 15, 15));
    }

    RED_TEST_CONTEXT("double click title bar") {
        RED_CHECK(Act::Nothing  == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));
        RED_CHECK(Act::Maximaze == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));

        RED_CHECK(Act::Nothing  == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));
        RED_CHECK(Act::Maximaze == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));

        RED_CHECK(Act::Nothing  == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));
        t += delay;
        RED_CHECK(Act::Nothing  == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));
        RED_CHECK(Act::Maximaze == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));

        RED_CHECK(Act::Nothing  == next_action(left_click | down,    50, 15));
        t += delay;
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | down,    50, 15));
        RED_CHECK(Act::Nothing  == next_action(left_click | release, 50, 15));
    }

    RED_TEST_CONTEXT("minimize") {
        RED_CHECK(Act::ActiveButton   == next_action(left_click | down,    500, 15));
        RED_CHECK(Act::Minimize       == next_action(left_click | release, 500, 15));

        RED_CHECK(Act::ActiveButton   == next_action(left_click | down,    500, 15));
        RED_CHECK(Act::CapturedClick  == next_action(move,                 560, 15));
        RED_CHECK(Act::UnactiveButton == next_action(left_click | release, 560, 15));

        RED_CHECK(Act::Nothing        == next_action(move,                 500, 15));

        RED_CHECK(Act::ActiveButton   == next_action(left_click | down,    500, 15));
        RED_CHECK(Act::CapturedClick  == next_action(move,                 400, 15));
        RED_CHECK(Act::CapturedClick  == next_action(move,                 501, 15));
        RED_CHECK(Act::Minimize       == next_action(left_click | release, 500, 15));
    }

    RED_TEST_CONTEXT("maximize vertical") {
        RED_CHECK(Act::Nothing          == next_action(left_click | down,    50, 10));
        RED_CHECK(Act::Nothing          == next_action(left_click | release, 50, 10));
        t += delay;
        RED_CHECK(Act::Nothing          == next_action(left_click | down,    50, 10));
        RED_CHECK(Act::Nothing          == next_action(left_click | release, 50, 10));
        RED_CHECK(Act::MaximazeVertical == next_action(left_click | down,    50, 10));
        RED_CHECK(Act::Nothing          == next_action(left_click | release, 50, 10));
    }

    RED_TEST_CONTEXT("move") {
        RED_CHECK(Act::Nothing         == next_action(left_click | down,    50,  15));
        RED_CHECK(Act::StartMoveResize == next_action(move,                 15,  15));
        RED_CHECK(Act::MoveResize      == next_action(move,                 400, 15));
        RED_CHECK(Act::StopMoveResize  == next_action(left_click | release, 400, 15));
    }

    RED_TEST_CONTEXT("resize") {
        RED_CHECK(Act::Nothing         == next_action(left_click | down,    599, 509));
        RED_CHECK(Act::StartMoveResize == next_action(move,                 599, 200));
        RED_CHECK(Act::MoveResize      == next_action(move,                 599, 519));
        RED_CHECK(Act::StopMoveResize  == next_action(left_click | release, 599, 519));
    }

    RED_TEST_CONTEXT("resize then move without click release") {
        RED_CHECK(Act::Nothing         == next_action(left_click | down,    599, 509));
        RED_CHECK(Act::StartMoveResize == next_action(move,                  50, 15));
        // StartMoveResize -!> MoveResize implies StopMoveResize
        RED_CHECK(Act::Nothing         == next_action(left_click | down,     50, 15));
        RED_CHECK(Act::StartMoveResize == next_action(move,                 400, 15));
        RED_CHECK(Act::StopMoveResize  == next_action(left_click | release, 400, 15));
    }

    RED_TEST_CONTEXT("not resizable border (maximixed window)") {
        maximixed_state = maximixed;
        RED_CHECK(Act::Nothing == next_action(left_click | down,    599, 509));
        RED_CHECK(Act::Nothing == next_action(move,                  50, 15));
        RED_CHECK(Act::Nothing == next_action(left_click | release, 500, 15));
    }
}

RED_AUTO_TEST_CASE(TestClientExecuteMoveResize)
{
    using Area = ClientExecute::WindowArea;

    auto move_resize = [](Area area, int offset_x, int offset_y){
        return ClientExecute::move_resize_rect(area, offset_x, offset_y,
                                               Rect(100, 100, 800, 600));
    };

    RED_CHECK(Rect(100, 110, 800, 590) == move_resize(Area::N, 10,  10));
    RED_CHECK(Rect(100, 110, 800, 590) == move_resize(Area::N, 10, -10));

    RED_CHECK(Rect(100, 100, 800, 610) == move_resize(Area::S, 10,  10));
    RED_CHECK(Rect(100, 100, 800, 590) == move_resize(Area::S, 10, -10));

    RED_CHECK(Rect(100, 100, 810, 600) == move_resize(Area::E, 10,  10));
    RED_CHECK(Rect(100, 100, 810, 600) == move_resize(Area::E, 10, -10));

    RED_CHECK(Rect(110, 100, 790, 600) == move_resize(Area::W, 10,  10));
    RED_CHECK(Rect(110, 100, 790, 600) == move_resize(Area::W, 10, -10));

    RED_CHECK(Rect(100,  90, 805, 610) == move_resize(Area::NEE,  5, -10));
    RED_CHECK(Rect(100, 110, 795, 590) == move_resize(Area::NEE, -5,  10));

    RED_CHECK(Rect(100,  90, 805, 610) == move_resize(Area::NEN,  5, -10));
    RED_CHECK(Rect(100, 110, 795, 590) == move_resize(Area::NEN, -5,  10));

    RED_CHECK(Rect(105,  90, 795, 610) == move_resize(Area::NWN,  5, -10));
    RED_CHECK(Rect( 95, 110, 805, 590) == move_resize(Area::NWN, -5,  10));

    RED_CHECK(Rect(105,  90, 795, 610) == move_resize(Area::NWW,  5, -10));
    RED_CHECK(Rect( 95, 110, 805, 590) == move_resize(Area::NWW, -5,  10));

    RED_CHECK(Rect(100, 100, 805, 590) == move_resize(Area::SEE,  5, -10));
    RED_CHECK(Rect(100, 100, 795, 610) == move_resize(Area::SEE, -5,  10));

    RED_CHECK(Rect(100, 100, 805, 590) == move_resize(Area::SES,  5, -10));
    RED_CHECK(Rect(100, 100, 795, 610) == move_resize(Area::SES, -5,  10));

    RED_CHECK(Rect(105, 100, 795, 590) == move_resize(Area::SWS,  5, -10));
    RED_CHECK(Rect( 95, 100, 805, 610) == move_resize(Area::SWS, -5,  10));

    RED_CHECK(Rect(105, 100, 795, 590) == move_resize(Area::SWW,  5, -10));
    RED_CHECK(Rect( 95, 100, 805, 610) == move_resize(Area::SWW, -5,  10));
}
