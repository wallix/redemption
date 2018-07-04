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

#ifndef REDEMPTION_DECL_LOG_TEST
# define REDEMPTION_DECL_LOG_TEST
#endif

#include "cxx/diagnostic.hpp"
#include "cxx/compiler_version.hpp"
REDEMPTION_DIAGNOSTIC_PUSH
#if REDEMPTION_COMP_CLANG >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
#endif
#include "utils/log.hpp"
REDEMPTION_DIAGNOSTIC_POP

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

bool & LOG__REDEMPTION__AS__LOGPRINT()
{
    static bool logprint = []{
        auto s = std::getenv("REDEMPTION_LOG_PRINT");
        return s && s[0] == '1';
    }();
    return logprint;
}

namespace
{
    static std::string log_buf = {};
    static bool enable_buf_log = false;
# ifndef NDEBUG
    static bool previous_is_line_marker = false;
# endif
} // namespace

LOG__REDEMPTION__BUFFERED::LOG__REDEMPTION__BUFFERED()
{
    log_buf.clear();
    enable_buf_log = true;
}

LOG__REDEMPTION__BUFFERED::~LOG__REDEMPTION__BUFFERED()
{
    enable_buf_log = false;
}

std::string const& LOG__REDEMPTION__BUFFERED::buf() const
{
    return log_buf;
}

void LOG__REDEMPTION__BUFFERED::clear()
{
    log_buf.clear();
}


void LOG__REDEMPTION__INTERNAL__IMPL(int priority, char const * format, ...)
{
    if (enable_buf_log) {
# ifndef NDEBUG
        // see LOG_FILENAME
        if (priority != LOG_INFO && priority != LOG_DEBUG) {
            if (!previous_is_line_marker) {
                previous_is_line_marker = true;
                return ;
            }
            previous_is_line_marker = false;
        }
# endif
        va_list ap;
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        va_start(ap, format);
        auto sz = std::vsnprintf(nullptr, 0, format, ap) + 1;
        va_end(ap);
        log_buf.resize(log_buf.size() + sz);
        va_start(ap, format);
        std::vsnprintf(&log_buf[log_buf.size() - sz], sz, format, ap);
        va_end(ap);
        REDEMPTION_DIAGNOSTIC_POP
        log_buf.back() = '\n';

        // replace "priority (31905/31905) --  message" by "priority - message"
        auto p = log_buf.find('(', log_buf.size() - sz + 5);
        auto e = log_buf.find('-', p);
        log_buf.replace(p, e-p+3, "-");
    }
    else if (LOG__REDEMPTION__AS__LOGPRINT())
    {
        (void)priority;
        va_list ap;
        va_start(ap, format);
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        std::vprintf(format, ap);
        REDEMPTION_DIAGNOSTIC_POP
        std::puts("");
        std::fflush(stdout);
        va_end(ap);
    }
}
