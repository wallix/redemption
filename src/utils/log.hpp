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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   log file including syslog
*/

#ifndef _REDEMPTION_UTILS_LOG_HPP_
#define _REDEMPTION_UTILS_LOG_HPP_

#include <string.h>

#define REDOC(x)

// These are used to help coverage chain when function length autodetection (using ctags and gcov) fails

#ifndef VERBOSE
#define TODO(x)
#else
#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " x))
#endif

// -Wnull-dereference and clang++
namespace { namespace compiler_aux_ {
    inline void * null_pointer()
    { return nullptr; }
} }
#define BOOM (*reinterpret_cast<int*>(compiler_aux_::null_pointer())=1)

// REDASSERT behave like assert but instaed of calling abort it triggers a segfault
// This is handy to get stacktrace while debugging.
#ifdef NDEBUG
#define REDASSERT(x)
#else
# if defined(LOGPRINT) || defined(REDASSERT_AS_ASSERT)
#  include <cassert>
#  define REDASSERT(x) assert(x)
# else
#  define REDASSERT(x) if(!(x)){BOOM;}
# endif
#endif

#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>


#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) a##b

#define PP_CALL(_, ...) _(__VA_ARGS__)

#define PP_DECL_0_LIST            \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0

#define PP_VARIADIC_HEAD(_, ...)                                                      \
  PP_CALL(PP_CAT(_,                                                                   \
    PP_CALL(PP_VARIADIC_HEAD_DIS, __VA_ARGS__, PP_DECL_0_LIST, 1, not_empty_variadic) \
  ), __VA_ARGS__)

#define PP_VARIADIC_HEAD_DIS(                          \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,           \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,  \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,  \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,  \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,  \
    _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,  \
    _61, _62, _63, _64, _65, _66, _67, _68, _69, _70,  \
    _71, _72, _73, _74, _75, _76, _77, _78, _79, _80,  \
    _81, _82, _83, _84, _85, _86, _87, _88, _89, _90,  \
    _91, _92, _93, _94, _95, _96, _97, _98, _99, _100, \
    _N, ...) _N


#define PP_HEAD_0(x, ...) x
#define PP_HEAD_1(x) x

#define PP_HEAD(...) PP_VARIADIC_HEAD(PP_HEAD_, __VA_ARGS__)


#define PP_TAIL_0(x, ...) __VA_ARGS__
#define PP_TAIL_1(x)

#define PP_TAIL(...) PP_VARIADIC_HEAD(PP_TAIL_, __VA_ARGS__)


#define PP_COMMA_IF_NOT_NULL_0(x, ...),
#define PP_COMMA_IF_NOT_NULL_1(x)

#define PP_COMMA_IF_NOT_NULL(...) PP_VARIADIC_HEAD(PP_COMMA_IF_NOT_NULL_, __VA_ARGS__)


// checked by the compiler
#define LOG_FORMAT_CHECK(...) \
    void(decltype(printf(" " __VA_ARGS__)){1})


#ifdef IN_IDE_PARSER
#  define LOG LOGSYSLOG__REDEMPTION__INTERNAL
#  define LOG_SESSION LOGSYSLOG__REDEMPTION__SESSION__INTERNAL

#elif defined(LOGPRINT)
#  define LOG(priority, ...)                                       \
    LOGCHECK__REDEMPTION__INTERNAL((void((                         \
        LOG_FORMAT_CHECK(__VA_ARGS__),                             \
        printf("%s (%d/%d) -- " PP_HEAD(__VA_ARGS__) "\n",         \
            prioritynames[priority], getpid(), getpid()            \
            PP_COMMA_IF_NOT_NULL(__VA_ARGS__) PP_TAIL(__VA_ARGS__) \
        )                                                          \
    )), 1))
#  define LOG_SESSION(duplicate_with_pid, session_type, type, session_id, \
        user, device, service, account, priority, ...)                    \
    LOGCHECK__REDEMPTION__INTERNAL((void((                                \
        LOG_FORMAT_CHECK(__VA_ARGS__),                                    \
        LOGNULL__REDEMPTION__SESSION__INTERNAL(                           \
            duplicate_with_pid,                                           \
            session_type,                                                 \
            type,                                                         \
            session_id,                                                   \
            user,                                                         \
            device,                                                       \
            service,                                                      \
            account                                                       \
        ),                                                                \
        printf("%s (%d/%d) -- " PP_HEAD(__VA_ARGS__) "\n",                \
            prioritynames[priority], getpid(), getpid()                   \
            PP_COMMA_IF_NOT_NULL(__VA_ARGS__) PP_TAIL(__VA_ARGS__)        \
        )                                                                 \
    )), 1))

#elif defined(LOGNULL)
#  define LOG(priority, ...) LOG_FORMAT_CHECK(__VA_ARGS__)
#  define LOG_SESSION(duplicate_with_pid, session_type, type, session_id, \
        user, device, service, account, priority, ...)                    \
    LOGCHECK__REDEMPTION__INTERNAL((void((                                \
        LOG_FORMAT_CHECK(__VA_ARGS__),                                    \
        LOGNULL__REDEMPTION__SESSION__INTERNAL(                           \
            duplicate_with_pid,                                           \
            session_type,                                                 \
            type,                                                         \
            session_id,                                                   \
            user,                                                         \
            device,                                                       \
            service,                                                      \
            account                                                       \
        )                                                                 \
    )), 1))

#else
#  define LOG(priority, ...)                                       \
    LOGCHECK__REDEMPTION__INTERNAL((void((                         \
        LOG_FORMAT_CHECK(__VA_ARGS__),                             \
        syslog(priority, "%s (%d/%d) -- " PP_HEAD(__VA_ARGS__),    \
            prioritynames[priority], getpid(), getpid()            \
            PP_COMMA_IF_NOT_NULL(__VA_ARGS__) PP_TAIL(__VA_ARGS__) \
        )                                                          \
    )), 1))
#  define LOG_SESSION(duplicate_with_pid, session_type, type, session_id, \
        user, device, service, account, priority, ...                     \
    )                                                                     \
    LOGCHECK__REDEMPTION__INTERNAL((void((                                \
        LOG_FORMAT_CHECK(__VA_ARGS__),                                    \
        LOGSYSLOG__REDEMPTION__SESSION__INTERNAL(                         \
            duplicate_with_pid, session_type, type, session_id,           \
            user, device, service, account, priority,                     \
            "%s (%d/%d) -- type='%s'%s" PP_HEAD(__VA_ARGS__),             \
            "[%s Session] "                                               \
                "type='%s' "                                              \
                "session_id='%s' "                                        \
                "user='%s' "                                              \
                "device='%s' "                                            \
                "service='%s' "                                           \
                "account='%s'%s"                                          \
                PP_HEAD(__VA_ARGS__),                                     \
            ((*PP_HEAD(__VA_ARGS__)) ? " " : "")                          \
            PP_COMMA_IF_NOT_NULL(__VA_ARGS__) PP_TAIL(__VA_ARGS__)        \
        )                                                                 \
    )), 1))
#endif

namespace {
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

    inline void LOGCHECK__REDEMPTION__INTERNAL(int)
    {}

    template<class... Ts>
    inline void LOGSYSLOG__REDEMPTION__SESSION__INTERNAL(
        bool duplicate_with_pid,

        const char * session_type,
        const char * type,
        const char * session_id,
        const char * user,
        const char * device,
        const char * service,
        const char * account,

        int priority,
        const char *format_with_pid,
        const char *format2,
        Ts const & ... args
    ) {
        #ifdef __GNUG__
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
        #endif
        if (duplicate_with_pid) {
            syslog(
                priority, format_with_pid,
                prioritynames[priority], getpid(), getpid(),
                type, args...
            );
        }
        syslog(
            priority, format2,
            session_type,
            type,
            session_id,
            user,
            device,
            service,
            account,
            args...
         );
        #ifdef __GNUG__
            #pragma GCC diagnostic pop
        #endif
    }

    inline void LOGNULL__REDEMPTION__SESSION__INTERNAL(
        bool duplicate_with_pid,
        char const * session_type,
        char const * type,
        char const * session_id,
        char const * user,
        char const * device,
        char const * service,
        char const * account
    ) {
        (void)duplicate_with_pid;
        (void)session_type;
        (void)type;
        (void)session_id;
        (void)user;
        (void)device;
        (void)service;
        (void)account;
    }
}

namespace {

inline void hexdump(const char * data, size_t size)
{
    char buffer[2048];
    for (size_t j = 0 ; j < size ; j += 16){
        char * line = buffer;
        line += sprintf(line, "%.4x ", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            line += sprintf(line, "%.2x ", static_cast<unsigned>(static_cast<unsigned char>(data[j+i])));
        }
        if (i < 16){
            line += sprintf(line, "%*c", static_cast<int>((16-i)*3), ' ');
        }
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = static_cast<unsigned>(static_cast<unsigned char>(data[j+i]));
            if ((tmp < ' ') || (tmp > '~')  || (tmp == '\\')){
                tmp = '.';
            }
            line += sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump(const unsigned char * data, size_t size)
{
    hexdump(reinterpret_cast<const char*>(data), size);
}

inline void hexdump_d(const char * data, size_t size, unsigned line_length = 16)
{
    char buffer[2048];
    for (size_t j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += sprintf(line, "/* %.4x */ ", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += sprintf(line, "0x%.2x, ", static_cast<unsigned>(static_cast<unsigned char>(data[j+i])));
        }
        if (i < line_length){
            line += sprintf(line, "%*c", static_cast<int>((line_length-i)*3), ' ');
        }

        line += sprintf(line, " // ");

        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = static_cast<unsigned>(static_cast<unsigned char>(data[j+i]));
            if ((tmp < ' ') || (tmp > '~') || (tmp == '\\')){
                tmp = '.';
            }
            line += sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump_d(const unsigned char * data, size_t size, unsigned line_length = 16)
{
    hexdump_d(reinterpret_cast<const char*>(data), size, line_length);
}

inline void hexdump_c(const char * data, size_t size)
{
    char buffer[2048];
    for (size_t j = 0 ; j < size ; j += 16){
        char * line = buffer;
        line += sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            line += sprintf(line, "\\x%.2x", static_cast<unsigned>(static_cast<unsigned char>(data[j+i])));
        }
        line += sprintf(line, "\"");
        if (i < 16){
            line += sprintf(line, "%*c", static_cast<int>((16-i)*4), ' ');
        }
        line += sprintf(line, " //");
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = static_cast<unsigned>(static_cast<unsigned char>(data[j+i]));
            if ((tmp < ' ') || (tmp > '~') || (tmp == '\\')){
                tmp = '.';
            }
            line += sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump_c(const unsigned char * data, size_t size)
{
    hexdump_c(reinterpret_cast<const char*>(data), size);
}

inline void hexdump96_c(const char * data, size_t size)
{
    char buffer[32768];
    const unsigned line_length = 96;
    for (size_t j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += sprintf(line, "\\x%.2x", static_cast<unsigned>(static_cast<unsigned char>(data[j+i])));
        }
        line += sprintf(line, "\"");
        if (i < line_length){
            line += sprintf(line, "%*c", static_cast<int>((line_length-i)*4), ' ');
        }
        line += sprintf(line, " //");
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = static_cast<unsigned>(static_cast<unsigned char>(data[j+i]));
            if ((tmp < ' ') || (tmp > '~')){
                tmp = '.';
            }
            line += sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump96_c(const unsigned char * data, size_t size)
{
    hexdump96_c(reinterpret_cast<const char*>(data), size);
}

inline void hexdump8_c(const char * data, size_t size)
{
    char buffer[1024];
    const unsigned line_length = 8;
    for (size_t j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += sprintf(line, "\\x%.2x", static_cast<unsigned>(static_cast<unsigned char>(data[j+i])));
        }
        line += sprintf(line, "\"");
        if (i < line_length){
            line += sprintf(line, "%*c", static_cast<int>((line_length-i)*4), ' ');
        }
        line += sprintf(line, " //");
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = static_cast<unsigned>(static_cast<unsigned char>(data[j+i]));
            if ((tmp < ' ') || (tmp > '~')){
                tmp = '.';
            }
            line += sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump8_c(const unsigned char * data, size_t size)
{
    hexdump8_c(reinterpret_cast<const char*>(data), size);
}

} // anonymous namespace

#endif
