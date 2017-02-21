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


# define REDEMPTION_PRAGMA(X) _Pragma(#X)

#if !defined(IN_IDE_PARSER) && (defined(__GNUC__) || defined(__clang__))
# define REDEMPTION_DIAGNOSTIC_PUSH REDEMPTION_PRAGMA(GCC diagnostic push)
# define REDEMPTION_DIAGNOSTIC_POP REDEMPTION_PRAGMA(GCC diagnostic pop)
# define REDEMPTION_DIAGNOSTIC_GCC_IGNORE(X) REDEMPTION_PRAGMA(GCC diagnostic ignored X)
# define REDEMPTION_DIAGNOSTIC_GCC_WARNING(X) REDEMPTION_PRAGMA(GCC diagnostic warning X)
# ifdef __clang__
#  define REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE(X)
#  define REDEMPTION_DIAGNOSTIC_CLANG_IGNORE REDEMPTION_DIAGNOSTIC_GCC_IGNORE
# else
#  define REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE REDEMPTION_DIAGNOSTIC_GCC_IGNORE
#  define REDEMPTION_DIAGNOSTIC_CLANG_IGNORE(X)
# endif
#else
# define REDEMPTION_DIAGNOSTIC_PUSH
# define REDEMPTION_DIAGNOSTIC_POP
# define REDEMPTION_DIAGNOSTIC_GCC_IGNORE(X)
# define REDEMPTION_DIAGNOSTIC_GCC_WARNING(X)
# define REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE(X)
# define REDEMPTION_DIAGNOSTIC_CLANG_IGNORE(X)
#endif
