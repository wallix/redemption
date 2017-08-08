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

#define RED_TEST_MODULE TestWaitObj
#include "system/redemption_unit_tests.hpp"

#include "core/wait_obj.hpp"

RED_AUTO_TEST_CASE(Testwait_obj)
{
    wait_obj nonsocketobj;

    // nonsocketobj never interact with system fd_set and max;
    fd_set rfds;
    fd_set wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    struct timeval timeout = { 2L, 0L };

    bool res;


    // initialy, wait obj is not set
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);

//    nonsocketobj.reset();
    nonsocketobj.reset_trigger_time();
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);

    // add to fd set does not change anything on
    // timeout if the wait obj is not set
    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 2L);
    RED_CHECK_EQUAL(timeout.tv_usec, 0L);


    // set wait obj (no timer)
//    nonsocketobj.set();
    nonsocketobj.set_trigger_time(wait_obj::NOW);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, true);

    // adding it to fd set change the time out
    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 0L);
    RED_CHECK_EQUAL(timeout.tv_usec, 0L);
    timeout.tv_sec = 2L;

    // reset
//    nonsocketobj.reset();
    nonsocketobj.reset_trigger_time();
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);

    // set a waitobj with a timer will set it
    // after the timeout is triggered
//    nonsocketobj.set(500000);
    nonsocketobj.set_trigger_time(500000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);
    RED_CHECK_EQUAL(timeout.tv_sec, 2L);
    RED_CHECK_EQUAL(timeout.tv_usec, 0);

    // adding it to fd set while it has a timer to be set
    // will change de timeout to the remaining time to be set
    // if the timeout is longer than this remaining time.
    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 0L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 500000L) &&
                      (timeout.tv_usec > 200000L), true);

    timeout.tv_sec = 2L;
    // adding two non socket waitobj to fd set
    // setting them with differents timers.
    wait_obj nonsocketobj2;
//    nonsocketobj.set(1500000);
    nonsocketobj.set_trigger_time(1500000);
//    nonsocketobj2.set(300000);
    nonsocketobj2.set_trigger_time(300000);

    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 1L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 500000L) &&
                      (timeout.tv_usec > 0L), true);
    nonsocketobj2.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 0L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 300000L) &&
                      (timeout.tv_usec > 0L), true);


    // set a wait obj twice but with different timers:
    // only the last timer will be considered
    timeout.tv_sec = 2L;
//    nonsocketobj.set(300000);
    nonsocketobj.set_trigger_time(300000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);
//    nonsocketobj.set(1400000);
    nonsocketobj.set_trigger_time(1400000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);

    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 1L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 500000L) &&
                      (timeout.tv_usec > 200000L), true);


    // update a wait obj twice (or many times) will consider the closest timer.
    // test1
    timeout.tv_sec = 2L;
    timeout.tv_usec = 0L;

//    nonsocketobj.reset();
    nonsocketobj.reset_trigger_time();
    nonsocketobj.update_trigger_time(300000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);
    nonsocketobj.update_trigger_time(1400000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);

    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 0L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 300000L) &&
                      (timeout.tv_usec > 0L), true);

    // test2
    timeout.tv_sec = 2L;
    timeout.tv_usec = 0L;

//    nonsocketobj.reset();
    nonsocketobj.reset_trigger_time();
    nonsocketobj.update_trigger_time(1400000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);
    nonsocketobj.update_trigger_time(800000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);


    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 0L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 800000L) &&
                      (timeout.tv_usec > 500000L), true);

    // test 3
    timeout.tv_sec = 3L;
    timeout.tv_usec = 0L;

//    nonsocketobj.reset();
    nonsocketobj.reset_trigger_time();
    nonsocketobj.update_trigger_time(1400000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);
    nonsocketobj.update_trigger_time(2800000);
    res = nonsocketobj.is_set(INVALID_SOCKET, rfds);
    RED_CHECK_EQUAL(res, false);


    nonsocketobj.wait_on_timeout(timeout);
    RED_CHECK_EQUAL(timeout.tv_sec, 1L);
    RED_CHECK_EQUAL((timeout.tv_usec <= 400000L) &&
                      (timeout.tv_usec > 100000L), true);
}
