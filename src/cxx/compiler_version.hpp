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
*   Copyright (C) Wallix 2013-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/pp.hpp"


#define REDEMPTION_COMP_VERSION_NUMBER(major, minor, patch) \
  ( (((major)%100)*10000000) + (((minor)%100)*100000) + ((patch)%100000) )

#define REDEMPTION_COMP_VERSION_NUMBER_NOT_AVAILABLE 0

// Clang

#ifdef __clang__
# define REDEMPTION_COMP_CLANG_VERSION \
    REDEMPTION_COMP_VERSION_NUMBER(    \
    __clang_major__,                   \
    __clang_minor__,                   \
    __clang_patchlevel__)
# define REDEMPTION_COMP_VERSION REDEMPTION_COMP_CLANG_VERSION
# define REDEMPTION_COMP_STRING_VERSION   \
    RED_PP_STRINGIFY(__clang_major__) "." \
    RED_PP_STRINGIFY(__clang_minor__) "." \
    RED_PP_STRINGIFY(__clang_patchlevel__)
# define REDEMPTION_COMP_NAME clang
#else
# define REDEMPTION_COMP_CLANG_VERSION REDEMPTION_COMP_VERSION_NUMBER_NOT_AVAILABLE
#endif


// Gcc

#ifdef __GNUC__
# ifdef __GNUC_PATCHLEVEL__
#  define REDEMPTION_COMP_GNUC_VERSION \
    REDEMPTION_COMP_VERSION_NUMBER(    \
      __GNUC__,                        \
      __GNUC_MINOR__,                  \
      __GNUC_PATCHLEVEL__)
#  ifndef REDEMPTION_COMP_STRING_VERSION
#   define REDEMPTION_COMP_STRING_VERSION  \
      RED_PP_STRINGIFY(__GNUC__) "."       \
      RED_PP_STRINGIFY(__GNUC_MINOR__) "." \
      RED_PP_STRINGIFY(__GNUC_PATCHLEVEL__)
#  endif
# else
#  define REDEMPTION_COMP_GNUC_VERSION \
    REDEMPTION_COMP_VERSION_NUMBER(    \
      __GNUC__,                        \
      __GNUC_MINOR__,                  \
      0)
#  ifndef REDEMPTION_COMP_STRING_VERSION
#   define REDEMPTION_COMP_STRING_VERSION \
      RED_PP_STRINGIFY(__GNUC__) "."      \
      RED_PP_STRINGIFY(__GNUC_MINOR__) ".0"
#  endif
# endif
# ifndef REDEMPTION_COMP_VERSION
#  define REDEMPTION_COMP_VERSION REDEMPTION_COMP_GNUC_VERSION
#  define REDEMPTION_COMP_NAME gcc
# endif
#else
# define REDEMPTION_COMP_GNUC_VERSION REDEMPTION_COMP_VERSION_NUMBER_NOT_AVAILABLE
#endif


// MSVC

#if defined(_MSC_VER)
# if !defined (_MSC_FULL_VER)
#   define REDEMPTION_COMP_MSVC_BUILD 0
# else
    /* how many digits does the build number have? */
#   if _MSC_FULL_VER / 10000 == _MSC_VER
      /* four digits */
#     define REDEMPTION_COMP_MSVC_BUILD (_MSC_FULL_VER % 10000)
#   elif _MSC_FULL_VER / 100000 == _MSC_VER
      /* five digits */
#     define REDEMPTION_COMP_MSVC_BUILD (_MSC_FULL_VER % 100000)
#   else
#     error "Cannot determine build number from _MSC_FULL_VER"
#   endif
# endif
  /*
  VS2014 was skipped in the release sequence for MS. Which
  means that the compiler and VS product versions are no longer
  in sync. Hence we need to use different formulas for
  mapping from MSC version to VS product version.
  */
# if (_MSC_VER >= 1900)
#  define REDEMPTION_COMP_MSVC_VERSION \
    REDEMPTION_COMP_VERSION_NUMBER(    \
      _MSC_VER/100-5,                  \
      _MSC_VER%100,                    \
      REDEMPTION_COMP_MSVC_BUILD)
# else
#  define REDEMPTION_COMP_MSVC_VERSION \
    REDEMPTION_COMP_VERSION_NUMBER(    \
      _MSC_VER/100-6,                  \
      _MSC_VER%100,                    \
      REDEMPTION_COMP_MSVC_BUILD)
# endif
# define REDEMPTION_COMP_VERSION REDEMPTION_COMP_MSVC_VERSION
# ifndef _MSC_FULL_VER
#  define REDEMPTION_COMP_STRING_VERSION RED_PP_STRINGIFY(_MSC_FULL_VER)
# else
#  define REDEMPTION_COMP_STRING_VERSION RED_PP_STRINGIFY(_MSC_VER)
# endif
# define REDEMPTION_COMP_NAME msvc
#else
# define REDEMPTION_COMP_MSVC_VERSION REDEMPTION_COMP_VERSION_NUMBER_NOT_AVAILABLE
#endif

#ifndef REDEMPTION_COMP_VERSION
# define REDEMPTION_COMP_VERSION REDEMPTION_COMP_VERSION_NUMBER_NOT_AVAILABLE
# define REDEMPTION_COMP_NAME unknown
#endif
