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
Author(s): Jonathan Poelen
*/

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "red_emscripten/em_asm.hpp"

#include <emscripten/bind.h>

#define RED_JS_AUTO_TEST_CASE(name, func_params, ...)        \
    void test_ ## name ## _impl func_params;                 \
                                                             \
    EMSCRIPTEN_BINDINGS(test_ ## name ## _binding)           \
    {                                                        \
        emscripten::function(#name, test_ ## name ## _impl); \
    }                                                        \
                                                             \
    RED_AUTO_TEST_CASE(name)                                 \
    {                                                        \
        RED_EM_ASM({ Module. name (__VA_ARGS__); });         \
    }                                                        \
                                                             \
    void test_ ## name ## _impl func_params
