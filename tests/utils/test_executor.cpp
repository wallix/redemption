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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to rect object

*/

#define RED_TEST_MODULE TestExecutor
#include "system/redemption_unit_tests.hpp"

#include "utils/executor.hpp"

#include <iostream>

#define TRACE_II(x) std::cout << "\x1b[31m" #x "\x1b[0m\n"
#define TRACE_I(x) TRACE_II(x)
#define TRACE TRACE_I(__LINE__)


RED_AUTO_TEST_CASE(TestExecutor)
{
    Executor executor;
    executor.initial_executor()
        .on_action([](auto ctx){
            TRACE;
            return ctx.exit_on_success();
        })
        .on_timeout([](auto ctx) {
            TRACE;
            return ctx.exit_on_success();
        })
        .on_exit([](auto ctx, bool) {
            TRACE;
            return ctx.exit_on_success();
        })
    ;

    executor.exec_all();

    executor.initial_executor(1, 2)
        .on_action([](auto ctx, int, int){
            TRACE;
            return ctx.sub_executor()
                .on_action([](auto ctx){
                    TRACE;
                    static int i = 0;
                    if (++i < 5) {
                        return ctx.retry();
                    }
                    return ctx.exit_on_success();
                })
                .on_timeout([](auto ctx){
                    TRACE;
                    return ctx.exit_on_success();
                })
                .on_exit([](auto ctx, bool){
                    TRACE;
                    return ctx.exit_on_success();
                })
            ;
        })
        .on_exit([](auto ctx, bool, int, int) {
            TRACE;
            return ctx.exit_on_success();
        })
        .on_timeout([](auto ctx, int, int) {
            TRACE;
            return ctx.exit_on_success();
        })
    ;

    executor.exec_all();
}
