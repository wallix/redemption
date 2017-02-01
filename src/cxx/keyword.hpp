/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once


#define FALCON_CXX_STD_11 201103
#define FALCON_CXX_STD_14 201402

// C++14 constexpr functions are inline in C++11
#if __cplusplus >= FALCON_CXX_STD_14
# define FALCON_CXX14_CONSTEXPR constexpr
# define FALCON_CONSTEXPR_AFTER_CXX11 constexpr
#else
# define FALCON_CXX14_CONSTEXPR inline
# define FALCON_CONSTEXPR_AFTER_CXX11
#endif
