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


#define REDEMPTION_CXX_STD_11 201103
#define REDEMPTION_CXX_STD_14 201402

// C++14 constexpr functions are inline in C++11
#if __cplusplus >= REDEMPTION_CXX_STD_14
# define REDEMPTION_CXX14_CONSTEXPR constexpr
# define REDEMPTION_CONSTEXPR_AFTER_CXX11 constexpr
#else
# define REDEMPTION_CXX14_CONSTEXPR inline
# define REDEMPTION_CONSTEXPR_AFTER_CXX11
#endif

#if defined(__clang__) || defined(__GNUC__)
# define REDEMPTION_LIKELY(x) __builtin_expect(!!(x), 1)
# define REDEMPTION_UNLIKELY(x) __builtin_expect(!!(x), 0)
# define REDEMPTION_ALWAYS_INLINE __attribute__((always_inline))
#else
# define REDEMPTION_LIKELY(x) (x)
# define REDEMPTION_UNLIKELY(x) (x)
# ifdef _MSC_VER
#  define REDEMPTION_ALWAYS_INLINE __forceinline
# else
#  define REDEMPTION_ALWAYS_INLINE
# endif
#endif
