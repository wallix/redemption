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


#pragma once

#include <string.h>

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
#define REDASSERT(x)
#else
# if defined(LOGPRINT) || defined(REDASSERT_AS_ASSERT)
#  include <cassert>
#  define REDASSERT(x) assert(x)
# else
#  define REDASSERT(x) if(!(x)){BOOM;}
# endif
#endif

#include "cxx/diagnostic.hpp"

#include <cstdio> // std::printf family

#include <syslog.h>

#include <sys/types.h>
#include <unistd.h> // getpid


// checked by the compiler
#define LOG_FORMAT_CHECK(...) \
    void(sizeof(printf(" " __VA_ARGS__)))


#ifdef IN_IDE_PARSER
#  define LOG LOGSYSLOG__REDEMPTION__INTERNAL

#elif defined(LOGPRINT)
#  define LOG(priority, ...)                                                    \
    LOGCHECK__REDEMPTION__INTERNAL((                                            \
        LOG_FORMAT_CHECK(__VA_ARGS__),                                          \
        LOGPRINT__REDEMPTION__INTERNAL(priority, "%s (%d/%d) -- " __VA_ARGS__), \
        1                                                                       \
    ))

#elif defined(LOGNULL)
#  define LOG(priority, ...)                    \
    LOGCHECK__REDEMPTION__INTERNAL((            \
        LOG_FORMAT_CHECK(__VA_ARGS__), priority \
    ))

#elif defined(LOGASMJS) && defined(EM_ASM)
#  define LOG(priority, ...) LOGCHECK__REDEMPTION__INTERNAL((                \
        LOG_FORMAT_CHECK(__VA_ARGS__),                                       \
        LOGPRINT__REDEMPTION__ASMJS(priority, "%s (%d/%d) -- " __VA_ARGS__), \
        1                                                                    \
    ))

#else
#  define LOG(priority, ...)                                                     \
    LOGCHECK__REDEMPTION__INTERNAL((                                             \
        LOG_FORMAT_CHECK(__VA_ARGS__),                                           \
        LOGSYSLOG__REDEMPTION__INTERNAL(priority, "%s (%d/%d) -- " __VA_ARGS__), \
        1                                                                        \
    ))
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
    void LOGPRINT__REDEMPTION__INTERNAL(int priority, char const * format, Ts const & ... args)
    {
        int const pid = getpid();
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        std::printf(format, prioritynames[priority], pid, pid, args...);
        REDEMPTION_DIAGNOSTIC_POP
        std::puts("");
    }

#if defined(LOGASMJS) && defined(EM_ASM)
    template<class... Ts>
    void LOGPRINT__REDEMPTION__ASMJS(int priority, char const * format, Ts const & ... args)
    {
        int const pid = getpid();
        char buffer[4096];
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        int len = snprintf(buffer, sizeof(buffer)-2, format, prioritynames[priority], pid, pid, args...);
        REDEMPTION_DIAGNOSTIC_POP
        buffer[len] = '\n';
        buffer[len+1] = 0;
        EM_ASM_({console.log(Pointer_stringify($0));}, buffer);
    }
#endif  // #if defined(LOGASMJS) && defined(EM_ASM)

    template<class... Ts>
    void LOGSYSLOG__REDEMPTION__INTERNAL(int priority, char const * format, Ts const & ... args)
    {
        int const pid = getpid();
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        syslog(priority, format, prioritynames[priority], pid, pid, args...);
        REDEMPTION_DIAGNOSTIC_POP
    }
}

namespace {

inline void hexdump(const unsigned char * data, size_t size)
{
    char buffer[2048];
    for (size_t j = 0 ; j < size ; j += 16){
        char * line = buffer;
        line += std::sprintf(line, "%.4x ", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            line += std::sprintf(line, "%.2x ", static_cast<unsigned>(data[j+i]));
        }
        if (i < 16){
            line += std::sprintf(line, "%*c", static_cast<int>((16-i)*3), ' ');
        }
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = data[j+i];
            if ((tmp < ' ') || (tmp > '~')  || (tmp == '\\')){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump(const char * data, size_t size)
{
    hexdump(reinterpret_cast<const unsigned char*>(data), size);
}

inline void hexdump_d(const unsigned char * data, size_t size, unsigned line_length = 16)
{
    char buffer[2048];
    for (size_t j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += std::sprintf(line, "/* %.4x */ ", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += std::sprintf(line, "0x%.2x, ", static_cast<unsigned>(data[j+i]));
        }
        if (i < line_length){
            line += std::sprintf(line, "%*c", static_cast<int>((line_length-i)*3), ' ');
        }

        line += std::sprintf(line, " // ");

        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = data[j+i];
            if ((tmp < ' ') || (tmp > '~') || (tmp == '\\')){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump_d(const char * data, size_t size, unsigned line_length = 16)
{
    hexdump_d(reinterpret_cast<const unsigned char*>(data), size, line_length);
}

inline void hexdump_c(const unsigned char * data, size_t size)
{
    char buffer[2048];
    for (size_t j = 0 ; j < size ; j += 16){
        char * line = buffer;
        line += std::sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            line += std::sprintf(line, "\\x%.2x", static_cast<unsigned>(data[j+i]));
        }
        line += std::sprintf(line, "\"");
        if (i < 16){
            line += std::sprintf(line, "%*c", static_cast<int>((16-i)*4), ' ');
        }
        line += std::sprintf(line, " //");
        for (i = 0; i < 16; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = data[j+i];
            if ((tmp < ' ') || (tmp > '~') || (tmp == '\\')){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump_c(const char * data, size_t size)
{
    hexdump_c(reinterpret_cast<const unsigned char*>(data), size);
}

inline void hexdump96_c(const unsigned char * data, size_t size)
{
    char buffer[32768];
    const unsigned line_length = 96;
    for (size_t j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += std::sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += std::sprintf(line, "\\x%.2x", static_cast<unsigned>(data[j+i]));
        }
        line += std::sprintf(line, "\"");
        if (i < line_length){
            line += std::sprintf(line, "%*c", static_cast<int>((line_length-i)*4), ' ');
        }
        line += std::sprintf(line, " //");
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = data[j+i];
            if ((tmp < ' ') || (tmp > '~')){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump96_c(const char * data, size_t size)
{
    hexdump96_c(reinterpret_cast<const unsigned char*>(data), size);
}

inline void hexdump8_c(const unsigned char * data, size_t size)
{
    char buffer[1024];
    const unsigned line_length = 8;
    for (size_t j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += std::sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += std::sprintf(line, "\\x%.2x", static_cast<unsigned>(data[j+i]));
        }
        line += std::sprintf(line, "\"");
        if (i < line_length){
            line += std::sprintf(line, "%*c", static_cast<int>((line_length-i)*4), ' ');
        }
        line += std::sprintf(line, " //");
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = data[j+i];
            if ((tmp < ' ') || (tmp > '~')){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}

inline void hexdump8_c(const char * data, size_t size)
{
    hexdump8_c(reinterpret_cast<const unsigned char*>(data), size);
}

} // anonymous namespace

