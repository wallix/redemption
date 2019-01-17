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

#ifdef LOGPRINT
# error LOGPRINT is deprecated. Used REDEMPTION_LOG_PRINT environment variable instead. Ex: `REDEMPTION_LOG_PRINT=1 bjam`
#endif
#ifdef LOGNULL
# error LOGNULL is deprecated. Used REDEMPTION_LOG_PRINT environment variable instead. Ex: `REDEMPTION_LOG_PRINT=0 bjam`. By default REDEMPTION_LOG_PRINT = 0
#endif

#include <sys/types.h> // getpid
#include <unistd.h> // getpid

#include <type_traits>
#include <cstdint>
#include <cstdio>

#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"

#include <syslog.h>

namespace detail_ {
    template<class T>
    struct vlog_wrap
    {
        T x;
        T const & value() const noexcept { return x; }
    };
} // namespace detail_

// T* to void* for %p
template<class T> detail_::vlog_wrap<void const*> log_value(T* p) noexcept { return {p}; }
template<class T> detail_::vlog_wrap<void const*> log_value(T const* p) noexcept { return {p}; }
inline detail_::vlog_wrap<char const*> log_value(char* p) noexcept { return {p}; } // NOLINT
inline detail_::vlog_wrap<char const*> log_value(char const* p) noexcept { return {p}; }
inline detail_::vlog_wrap<uint8_t const*> log_value(uint8_t* p) noexcept { return {p}; } // NOLINT
inline detail_::vlog_wrap<uint8_t const*> log_value(uint8_t const* p) noexcept { return {p}; }

// enum type
template<class T, typename std::enable_if<std::is_enum<T>::value, bool>::type = 1>
detail_::vlog_wrap<typename std::underlying_type<T>::type>
log_value(T const & e) noexcept
{ return {static_cast<typename std::underlying_type<T>::type>(e)}; }

namespace detail_ {
    // has c_str() member
    template<class T>
    auto log_value(T const & x, int /*unused*/)
    -> typename std::enable_if<
        std::is_convertible<decltype(x.c_str()), char const *>::value,
        vlog_wrap<char const *>
    >::type
    { return {x.c_str()}; }

    template<class T>
    vlog_wrap<T const &> log_value(T const & x, char /*unused*/)
    { return {x}; }
} // namespace detail_

// not enum type
template<class T, typename std::enable_if<!std::is_enum<T>::value, bool>::type = 1>
auto log_value(T const & x)
-> decltype(detail_::log_value(x, 1))
{ return detail_::log_value(x, 1); }

template<std::size_t n>
struct redemption_log_s
{
    char data[n];
};

template<std::size_t n>
detail_::vlog_wrap<char const*> log_value(redemption_log_s<n> const & x)
{ return {x.data}; }

template<std::size_t n>
redemption_log_s<n*2+1>
log_array_02x_format(uint8_t const (&d)[n]) noexcept
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
    "" BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)            \
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
#       pragma GCC warning "Cannot check format in 'LOG' (no boost preprocessor headers or unsupported variadic macro). Disable this warning with -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING."
#   else
#       warning "Cannot check format in 'LOG' (no boost preprocessor headers or unsupported variadic macro)"
#   endif
# endif

# define LOG_REDEMPTION_FORMAT_CHECK(...) ::compiler_aux_::unused_variables(__VA_ARGS__)
# define REDEMPTION_LOG_VALUE(x) log_value(x).value()
# define LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(...) __VA_ARGS__

# define LOG_UNCHECKED_FORMAT 1

#endif


#ifdef IN_IDE_PARSER
# define LOG(priority, ...) ::compiler_aux_::unused_variables(priority, "" __VA_ARGS__)
# define LOG_SIEM(...) ::compiler_aux_::unused_variables("" __VA_ARGS__)
# define LOG_PROXY_SIEM(type, ...) ::compiler_aux_::unused_variables("" type, "" __VA_ARGS__)
# define LOG_UNCHECKED_FORMAT 1

#else
# ifdef NDEBUG
#   define LOG_REDEMPTION_FILENAME(priority)
# else
#   define LOG_REDEMPTION_FILENAME(priority)                 \
    REDEMPTION_DIAGNOSTIC_PUSH                               \
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunreachable-code") \
    if (priority != LOG_INFO && priority != LOG_DEBUG) {     \
        ::detail::LOG__REDEMPTION__INTERNAL(                 \
            priority, "%s (%d/%d) -- ◢ In %s:%d",            \
            __FILE__, __LINE__                               \
        );                                                   \
    }                                                        \
    REDEMPTION_DIAGNOSTIC_POP
# endif

# define LOG(priority, ...) do {                                       \
    using ::log_value;                                                 \
    LOG_REDEMPTION_FILENAME(priority)                                  \
    ::detail::LOGCHECK__REDEMPTION__INTERNAL((                         \
        LOG_REDEMPTION_FORMAT_CHECK(__VA_ARGS__),                      \
        ::detail::LOG__REDEMPTION__INTERNAL(priority, "%s (%d/%d) -- " \
            LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(__VA_ARGS__)),   \
        1                                                              \
    ));                                                                \
 } while (0)

# define LOG_SIEM(...) do {                                          \
    using ::log_value;                                               \
    ::detail::LOGCHECK__REDEMPTION__INTERNAL((                       \
        LOG_REDEMPTION_FORMAT_CHECK(__VA_ARGS__),                    \
        ::detail::LOG__SIEM__REDEMPTION__INTERNAL(""                 \
            LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(__VA_ARGS__)), \
        1                                                            \
    ));                                                              \
 } while (0)

# define LOG_PROXY_SIEM(type, ...) do {                              \
    using ::log_value;                                               \
    ::detail::LOGCHECK__REDEMPTION__INTERNAL((                       \
        LOG_REDEMPTION_FORMAT_CHECK(__VA_ARGS__),                    \
        ::detail::LOG__PROXY__SIEM__REDEMPTION__INTERNAL(            \
            "[RDP Proxy] spid=\"%s\" user=\"%s\" type=\"" type "\" " \
            LOG_REDEMPTION_VARIADIC_TO_LOG_PARAMETERS(__VA_ARGS__)), \
        1                                                            \
    ));                                                              \
 } while (0)

namespace detail
{
    inline void LOGCHECK__REDEMPTION__INTERNAL(int /*unused*/)
    {}
}
#endif

#define LOG_IF(cond, priority, ...) if (cond) LOG(priority, __VA_ARGS__)

#if defined(LOG_UNCHECKED_FORMAT)
namespace compiler_aux_
{
    template<class... Ts>
    void unused_variables(Ts const & ...)
    {}
}
#endif

void LOG__REDEMPTION__INTERNAL__IMPL(int priority, char const * format, ...);

#ifdef REDEMPTION_DECL_LOG_TEST
# include <string>
struct LOG__REDEMPTION__OSTREAM__BUFFERED
{
    LOG__REDEMPTION__OSTREAM__BUFFERED();
    ~LOG__REDEMPTION__OSTREAM__BUFFERED();
    std::string str() const;

private:
    class D;
    D * d;
};

struct LOG__REDEMPTION__BUFFERED
{
    LOG__REDEMPTION__BUFFERED();
    ~LOG__REDEMPTION__BUFFERED();
    std::string const& buf() const;
    void clear();
};
#endif

namespace detail
{
    template<class... Ts>
    void LOG__REDEMPTION__INTERNAL(int priority, char const * format, Ts const & ... args)
    {
        // LOG_EMERG      system is unusable
        // LOG_ALERT      action must be taken immediately
        // LOG_CRIT       critical conditions
        // LOG_ERR        error conditions
        // LOG_WARNING    warning conditions
        // LOG_NOTICE     normal, but significant, condition
        // LOG_INFO       informational message
        // LOG_DEBUG      debug-level message

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

        using ::log_value;
        int const pid = getpid();
        LOG__REDEMPTION__INTERNAL__IMPL(
            priority,
            format,
            prioritynames[priority],
            pid,
            pid,
            REDEMPTION_LOG_VALUE(args)...
        );
    }

    template<class... Ts>
    void LOG__SIEM__REDEMPTION__INTERNAL(char const * format, Ts const & ... args)
    {
        using ::log_value;
        LOG__REDEMPTION__INTERNAL__IMPL(
            LOG_INFO,
            format,
            REDEMPTION_LOG_VALUE(args)...
        );
    }

    void log_proxy_init(char const* psid, char const* source_ip, int source_port) noexcept;
    void log_proxy_set_user(char const* username) noexcept;
    void log_proxy_target_disconnection(char const* reason) noexcept;
    void log_proxy_logout(char const* reason = nullptr) noexcept;
    void log_proxy_disconnection(char const* reason = nullptr) noexcept;

    char const* log_proxy_get_psid() noexcept;
    char const* log_proxy_get_user() noexcept;

    template<class... Ts>
    void LOG__PROXY__SIEM__REDEMPTION__INTERNAL(char const * format, Ts const & ... args)
    {
        using ::log_value;
        LOG__REDEMPTION__INTERNAL__IMPL(
            LOG_INFO,
            format,
            log_proxy_get_psid(),
            log_proxy_get_user(),
            REDEMPTION_LOG_VALUE(args)...
        );
    }
} // namespace detail
