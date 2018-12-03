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

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0188r1.pdf
#if (__cplusplus > REDEMPTION_CXX_STD_14 && REDEMPTION_CXX_HAS_ATTRIBUTE(fallthrough)) \
 || ( REDEMPTION_COMP_GNUC_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(7, 1, 0))
#  define REDEMPTION_CXX_FALLTHROUGH [[fallthrough]]
#elif defined(__clang__)
#  define REDEMPTION_CXX_FALLTHROUGH [[clang::fallthrough]]
#else
#  define REDEMPTION_CXX_FALLTHROUGH void(0)
#endif

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0189r1.pdf
#if __cplusplus > REDEMPTION_CXX_STD_14 && REDEMPTION_CXX_HAS_ATTRIBUTE(nodiscard)
#  define REDEMPTION_CXX_NODISCARD [[nodiscard]]
#elif defined(__clang__) || defined(__GNUC__)
#  define REDEMPTION_CXX_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
#  define REDEMPTION_CXX_NODISCARD _Check_return_
#else
#  define REDEMPTION_CXX_NODISCARD
#endif

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0212r1.pdf
#if __cplusplus > REDEMPTION_CXX_STD_14 && REDEMPTION_CXX_HAS_ATTRIBUTE(maybe_unused)
#  define REDEMPTION_CXX_MAYBE_UNUSED [[maybe_unused]]
#elif defined(__clang__) || defined(__GNUC__)
#  define REDEMPTION_CXX_MAYBE_UNUSED __attribute__((unused))
#else
#  define REDEMPTION_CXX_MAYBE_UNUSED
#endif

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3760.html
#if !defined(REDEMPTION_DISABLE_DEPRECATED_WARNINGS)
# if __cplusplus > REDEMPTION_CXX_STD_14 && REDEMPTION_CXX_HAS_ATTRIBUTE(deprecated)
#  define REDEMPTION_ATTRIBUTE_DEPRECATED [[deprecated]]
#  define REDEMPTION_ATTRIBUTE_DEPRECATED_MSG(msg) [[deprecated(msg)]]
# elif defined(__GNUC__) || defined(__clang__)
#  define REDEMPTION_ATTRIBUTE_DEPRECATED __attribute__((deprecated("deprecated")))
#  define REDEMPTION_ATTRIBUTE_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
# elif defined(_MSC_VER)
#  define REDEMPTION_ATTRIBUTE_DEPRECATED __declspec(deprecated("deprecated"))
#  define REDEMPTION_ATTRIBUTE_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
# else
#  define REDEMPTION_ATTRIBUTE_DEPRECATED
#  define REDEMPTION_ATTRIBUTE_DEPRECATED_MSG(msg)
# endif
#else
# define REDEMPTION_ATTRIBUTE_DEPRECATED
# define REDEMPTION_ATTRIBUTE_DEPRECATED_MSG(msg)
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
# define REDEMPTION_LIB_EXPORT __attribute__((visibility("default")))
#else
# define REDEMPTION_LIKELY(x) (x)
# define REDEMPTION_UNLIKELY(x) (x)
# ifdef _MSC_VER
#  define REDEMPTION_ALWAYS_INLINE __forceinline
#  define REDEMPTION_LIB_EXPORT __declspec(dllexport)
# else
#  define REDEMPTION_ALWAYS_INLINE
#  define REDEMPTION_LIB_EXPORT // REDEMPTION_WARNING("Unknown dynamic link import semantics.")
# endif
#endif

// REDEMPTION_UNREACHABLE / REDEMPTION_UNREACHABLE_IF
#ifndef NDEBUG
# define REDEMPTION_UNREACHABLE() do {                           \
        REDEMPTION_DIAGNOSTIC_PUSH                               \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunreachable-code") \
        assert(!"Unreachable code reached."); /* NOLINT */       \
        REDEMPTION_DIAGNOSTIC_POP                                \
    } while (0)

# define REDEMPTION_UNREACHABLE_IF(condition) \
  assert((condition) && "Unreachable code reached.")
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
