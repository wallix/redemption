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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#define RED_PP_STRINGIFY_I(x) #x
#define RED_PP_STRINGIFY(x) RED_PP_STRINGIFY_I(x)

#define RED_PP_CAT_I(a, b) a##b
#define RED_PP_CAT(a, b) RED_PP_CAT_I(a, b)

#define RED_PP_IS_EMPTY(name) (name+0 != -14) && (7-name-7 == 14)

#define RED_PP_IDENTITY(...) __VA_ARGS__
