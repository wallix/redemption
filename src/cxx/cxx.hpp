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

// https://github.com/jonathanpoelen/falcon.cxx/blob/master/include/falcon/cxx/cxx.hpp

#include "cxx/compiler_version.hpp"
#include "cxx/diagnostic.hpp"

#define REDEMPTION_CXX_STD_03 199711L // 1998/2003
#define REDEMPTION_CXX_STD_11 201103L
#define REDEMPTION_CXX_STD_14 201402L
#define REDEMPTION_CXX_STD_17 201703L

#ifdef __has_include
# define REDEMPTION_HAS_INCLUDE(path) __has_include(path)
#else
# define REDEMPTION_HAS_INCLUDE(path) 0
#endif


// Attributes
//@{
#ifdef __has_cpp_attribute
# define REDEMPTION_CXX_HAS_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
# define REDEMPTION_CXX_HAS_ATTRIBUTE(attr) 0
#endif


#if defined(__clang__)
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(x)     __attribute__((x))
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_(x)           // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_CLANG_(x)         __attribute__((x))
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_(x)               __attribute__((x))
#elif defined(__GNUC__)
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(x)     __attribute__((x))
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_CLANG_(x)         // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_(x)           __attribute__((x))
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_(x)               __attribute__((x))
#elif defined(_MSC_VER)
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(x)     // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_CLANG_(x)         // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_(x)           // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_(x)               __declspec(x)
#else
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(x)     // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_CLANG_(x)         // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_(x)           // no-op
#  define REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_(x)               // no-op
#endif


#define REDEMPTION_ATTRIBUTE_NONNULL_ARGS __attribute__((nonnull))
#define REDEMPTION_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))


#define REDEMPTION_CXX_ATTRIBUTE_NO_SANITIZE_ADDRESS \
    REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(no_sanitize_address)

#define REDEMPTION_CXX_ATTRIBUTE_NO_SANITIZE_THREAD \
    REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(no_sanitize_thread)

#define REDEMPTION_CXX_ATTRIBUTE_NO_SANITIZE_MEMORY \
    REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_CLANG_(no_sanitize_memory)

#define REDEMPTION_CXX_ATTRIBUTE_NO_SANITIZE_UNDEFINED                   \
    REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_CLANG_(no_sanitize("undefined")) \
    REDEMPTION_CXX_ANNOTATION_ATTRIBUTE_GCC_(no_sanitize_undefined)
//@}


// Keywords
//@{
#if defined(__clang__) || defined(__GNUC__)
# define REDEMPTION_LIKELY(x) __builtin_expect(!!(x), 1)
# define REDEMPTION_UNLIKELY(x) __builtin_expect(!!(x), 0)
# define REDEMPTION_NOINLINE __attribute__((noinline))
# define REDEMPTION_ALWAYS_INLINE __attribute__((always_inline))
# define REDEMPTION_LIB_EXPORT __attribute__((visibility("default")))
#else
# define REDEMPTION_LIKELY(x) (x)
# define REDEMPTION_UNLIKELY(x) (x)
# ifdef _MSC_VER
#  define REDEMPTION_NOINLINE __declspec(noinline)
#  define REDEMPTION_ALWAYS_INLINE __forceinline
#  define REDEMPTION_LIB_EXPORT __declspec(dllexport)
# else
#  define REDEMPTION_NOINLINE
#  define REDEMPTION_ALWAYS_INLINE
#  define REDEMPTION_LIB_EXPORT // REDEMPTION_WARNING("Unknown dynamic link import semantics.")
# endif
#endif

// REDEMPTION_UNREACHABLE / REDEMPTION_UNREACHABLE_IF
#ifndef NDEBUG
# define REDEMPTION_UNREACHABLE() do {                            \
        REDEMPTION_DIAGNOSTIC_PUSH()                              \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunreachable-code")  \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wstring-conversion") \
        assert(!"Unreachable code reached."); /* NOLINT */        \
        REDEMPTION_DIAGNOSTIC_POP()                               \
    } while (0)

# define REDEMPTION_UNREACHABLE_IF(condition)                 \
    REDEMPTION_DIAGNOSTIC_PUSH()                              \
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wstring-conversion") \
    assert((condition) && "Unreachable code reached.")        \
    REDEMPTION_DIAGNOSTIC_POP()
#else
# define REDEMPTION_UNREACHABLE_IF(condition) \
  do { if (condition) REDEMPTION_UNREACHABLE(); } while (0)
#endif

#if defined(__GNUC__) || defined(__clang__)
// https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
# if !defined(REDEMPTION_UNREACHABLE) \
  && ( REDEMPTION_COMP_GNUC_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(4, 5, 0) \
    || defined(__clang__))
#  define REDEMPTION_UNREACHABLE() __builtin_unreachable()
# endif
#else
# ifdef _MSC_VER && _MSC_VER >= 1900
#  if !defined REDEMPTION_UNREACHABLE && _MSC_VER >= 1900
#   define REDEMPTION_UNREACHABLE() __assume(0)
#  endif
# endif
#endif

#ifndef REDEMPTION_UNREACHABLE
# define REDEMPTION_UNREACHABLE() do { } while (0)
#endif
//@}

#define REDEMPTION_DECLTYPE_AUTO_RETURN_NOEXCEPT(...) \
  REDEMPTION_DEDUCE_NOEXCEPT(__VA_ARGS__) \
  -> decltype(__VA_ARGS__) \
  { return __VA_ARGS__; }

#define REDEMPTION_AUTO_RETURN_NOEXCEPT(...) \
  REDEMPTION_DEDUCE_NOEXCEPT(__VA_ARGS__) \
  { return __VA_ARGS__; }

#define REDEMPTION_DECLTYPE_AUTO_RETURN(...) \
  -> decltype(__VA_ARGS__) \
  { return __VA_ARGS__; }

#define REDEMPTION_DECLTYPE_NOEXCEPT(...) \
  REDEMPTION_DEDUCE_NOEXCEPT(__VA_ARGS__) \
  -> decltype(__VA_ARGS__) \

# define REDEMPTION_LIFT(F) [&](auto&&... args) \
  REDEMPTION_DECLTYPE_AUTO_RETURN_NOEXCEPT(F(static_cast<decltype(args)&&>(args)...))

#define REDEMPTION_DEDUCE_NOEXCEPT(...) noexcept(noexcept(__VA_ARGS__))
