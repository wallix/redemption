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
   Author(s): Christophe Grosjean, Javier Caverni, Jonathan Poelen
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   log file including syslog
*/


#pragma once

#define LOG_SIEM syslog

// These are used to help coverage chain when function length autodetection (using ctags and gcov) fails

// -Wnull-dereference and clang++
namespace { namespace compiler_aux_ {
    inline void * null_pointer()
    { return nullptr; }
} }
#define BOOM (*reinterpret_cast<int*>(compiler_aux_::null_pointer())=1)

// REDASSERT behave like assert but instaed of calling abort it triggers a segfault
// This is handy to get stacktrace while debugging.

#ifdef NDEBUG
# define REDASSERT(x)
#else
# if defined(REDASSERT_AS_ASSERT)
#  include <cassert>
#  define REDASSERT(x) assert(x)
# else
#  define REDASSERT(x) if(!(x)){BOOM;}
# endif
#endif

#if !defined(LOGNULL)
# include <sys/types.h> // getpid
# include <unistd.h> // getpid
#endif

#include <type_traits>
#include <cstdint>

#include "cxx/cxx.hpp"

#include <syslog.h>

namespace detail_ {
    template<class T>
    struct vlog_wrap
    {
        T x;
        T const & value() const noexcept { return x; }
    };
}

// enum type
template<class T, typename std::enable_if<std::is_enum<T>::value, bool>::type = 1>
detail_::vlog_wrap<typename std::underlying_type<T>::type>
log_value(T const & e) { return {static_cast<typename std::underlying_type<T>::type>(e)}; }

namespace detail_ {
    // has c_str() member
    template<class T>
    auto log_value(T const & x, int)
    -> typename std::enable_if<
        std::is_convertible<decltype(x.c_str()), char const *>::value,
        vlog_wrap<char const *>
    >::type
    { return {x.c_str()}; }

    template<class T>
    vlog_wrap<T const &> log_value(T const & x, char)
    { return {x}; }
}

// not enum type
template<class T, typename std::enable_if<!std::is_enum<T>::value, bool>::type = 1>
auto log_value(T const & x)
-> decltype(detail_::log_value(x, 1))
{ return detail_::log_value(x, 1); }

namespace {
    template<std::size_t n>
    struct redemption_log_s
    {
        char data[n];
    };

    template<std::size_t n>
    detail_::vlog_wrap<char const*> log_value(redemption_log_s<n> const & x)
    { return {x.data}; }
}

template<std::size_t n>
redemption_log_s<n*2+1>
log_array_02x_format(uint8_t const (&d)[n])
{
    redemption_log_s<n*2+1> r;
    char * p = r.data;
    for (uint8_t c : d) {
        *p++ = "012345678abcdef"[c >> 4];
        *p++ = "012345678abcdef"[c & 0xf];
    }
    *p = 0;
    return r;
}

#if ! defined(IN_IDE_PARSER) && REDEMPTION_HAS_INCLUDE(<boost/preprocessor/config/config.hpp>)
# include <boost/preprocessor/config/config.hpp>

# if BOOST_PP_VARIADICS

#  include <boost/preprocessor/cat.hpp>
#  include <boost/preprocessor/comparison/greater.hpp>
#  include <boost/preprocessor/array/pop_front.hpp>
#  include <boost/preprocessor/array/pop_back.hpp>
#  include <boost/preprocessor/array/to_list.hpp>
#  include <boost/preprocessor/array/elem.hpp>
#  include <boost/preprocessor/list/for_each.hpp>
#  include <boost/preprocessor/variadic/elem.hpp>
#  include <boost/preprocessor/variadic/to_list.hpp>

#  define REDEMPTION_LOG_VALUE_PARAM_0(elem)
#  define REDEMPTION_LOG_VALUE_PARAM_1(elem) , log_value(elem).value()

#  define REDEMPTION_LOG_VALUE_PARAM(r, data, elem) \
    BOOST_PP_CAT(REDEMPTION_LOG_VALUE_PARAM_, BOOST_PP_BOOL(BOOST_PP_GREATER(r, 2)))(elem)

#  define LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(...) \
    " " BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)           \
    BOOST_PP_LIST_FOR_EACH(                              \
        REDEMPTION_LOG_VALUE_PARAM,                      \
        BOOST_PP_NIL,                                    \
        BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)           \
    )

// checked by the compiler
#  define LOG_REDEMPTION_FORMAT_CHECK(...)                     \
    void(sizeof(printf(                                        \
        LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(__VA_ARGS__) \
    )))

#  define REDEMPTION_LOG_VALUE(x) (x)

# endif
#endif

#ifndef LOG_REDEMPTION_FORMAT_CHECK

# ifndef REDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING
#   if defined __GNUC__ || defined __clang__
#       pragma GCC warning "Cannot check format in 'LOG' (no boost preprocessor headers or unsupported variadic macro)"
#   else
#       warning "Cannot check format in 'LOG' (no boost preprocessor headers or unsupported variadic macro)"
#   endif
# endif

# define LOG_REDEMPTION_FORMAT_CHECK(...) ::compiler_aux_::unused_variables(__VA_ARGS__)
# define REDEMPTION_LOG_VALUE(x) log_value(x).value()
# define LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(...) __VA_ARGS__

#endif


#ifdef IN_IDE_PARSER
#  define LOG(priority, ...) compiler_aux_::unused_variables(priority, "" __VA_ARGS__)

#else
#  define LOG(priority, ...) do {                                \
    using ::log_value;                                           \
    LOGCHECK__REDEMPTION__INTERNAL((                             \
        LOG_REDEMPTION_FORMAT_CHECK(__VA_ARGS__),                \
        LOG__REDEMPTION__INTERNAL(priority, "%s (%d/%d) -- "     \
        LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(__VA_ARGS__)), \
        1                                                        \
    ));                                                          \
 } while (0)
#endif

#define COND_LOG(cond, priority, ...) if (cond) LOG(priority, __VA_ARGS__)

namespace {
    // LOG_EMERG      system is unusable
    // LOG_ALERT      action must be taken immediately
    // LOG_CRIT       critical conditions
    // LOG_ERR        error conditions
    // LOG_WARNING    warning conditions
    // LOG_NOTICE     normal, but significant, condition
    // LOG_INFO       informational message
    // LOG_DEBUG      debug-level message

#if !defined(LOGNULL)
    constexpr const char * const prioritynames[] =
    {
        "EMERG"/*, LOG_EMERG*/,
        "ALERT"/*, LOG_ALERT*/,
        "CRIT"/*, LOG_CRIT*/,
        "ERR"/*, LOG_ERR*/,
        "WARNING"/*, LOG_WARNING*/,
        "NOTICE"/*, LOG_NOTICE*/,
        "INFO"/*, LOG_INFO*/,
        "DEBUG"/*, LOG_DEBUG*/,
        //{ nullptr/*, -1*/ }
    };
#endif

    inline void LOGCHECK__REDEMPTION__INTERNAL(int)
    {}

    namespace compiler_aux_
    {
        template<class... Ts>
        void unused_variables(Ts const & ...)
        {}
    }
}

void LOG__REDEMPTION__INTERNAL__IMPL(int priority, char const * format, ...);

#ifdef REDEMPTION_DECL_LOG_TEST
bool & LOG__REDEMPTION__AS__LOGPRINT();
# ifdef LOGNULL
static struct LOG__REDEMPTION__AS__LOGNULL__INIT
{
    LOG__REDEMPTION__AS__LOGNULL__INIT()
    {
        LOG__REDEMPTION__AS__LOGPRINT() = false;
    }
} LOG__REDEMPTION__AS__LOGNULL__INIT_;
# endif
#endif

namespace
{
    template<class... Ts>
    void LOG__REDEMPTION__INTERNAL(int priority, char const * format, Ts const & ... args)
    {
    #if defined(LOGNULL)
        compiler_aux_::unused_variables(priority, format, ((void)(args), 1)...);
    #else
        # if ! BOOST_PP_VARIADICS
        using ::log_value;
        #endif
        int const pid = getpid();
        LOG__REDEMPTION__INTERNAL__IMPL(
            priority,
            format,
            prioritynames[priority],
            pid,
            pid,
            REDEMPTION_LOG_VALUE(args)...
        );
    #endif
    }
}
