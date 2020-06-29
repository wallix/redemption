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

#include "cxx/diagnostic.hpp"
#include "cxx/compiler_version.hpp"
REDEMPTION_DIAGNOSTIC_PUSH
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
#endif
#include "utils/log.hpp"
REDEMPTION_DIAGNOSTIC_POP

#include "test_only/log_buffered.hpp"

#ifdef __EMSCRIPTEN__
# include "red_emscripten/em_js.hpp"
# include "red_emscripten/em_asm.hpp"
#endif

#include <cstdarg>
#include <cstdio>
#include <cstdlib>


namespace
{
    std::string log_buf = {};
    bool enable_buf_log = false;
# ifndef NDEBUG
    bool previous_is_line_marker = false;

    bool log_is_filename(int priority) noexcept
    {
        // see LOG_FILENAME
        if (priority != LOG_INFO && priority != LOG_DEBUG) {
            if (!previous_is_line_marker) {
                previous_is_line_marker = true;
                return true;
            }
            previous_is_line_marker = false;
        }
        return false;
    }
# else
    constexpr bool log_is_filename(int /*priority*/) noexcept
    {
        return false;
    }
# endif

#ifdef __EMSCRIPTEN__
    RED_EM_JS(int, red_is_loggable_impl, (), {
        if (!ENVIRONMENT_IS_NODE) {
            return 49 /* = '1' */;
        }
        const s = process.env["REDEMPTION_LOG_PRINT"];
        return (s && s[0]) ? s.charCodeAt(0) : 48 /* = '0' */;
    })
#else
    int red_is_loggable_impl()
    {
        auto s = std::getenv("REDEMPTION_LOG_PRINT");
        return (s && s[0]) ? s[0] : '0';
    }
#endif

    bool is_loggable(int priority)
    {
        static char logprint = []{
            switch (red_is_loggable_impl()) {
                case '1' : return 1;
                case 'd' : return 2;
                case 'e' : return 3;
                case 'w' : return 4;
            }
            return 0;
        }();
        switch (logprint) {
            default:
            case 0: return false;
            case 1: return true;
            case 2: return (priority == LOG_DEBUG);
            case 3: return (priority == LOG_DEBUG || priority == LOG_ERR);
            case 4: return (priority != LOG_INFO);
        }
    }
} // namespace


tu::log_buffered::log_buffered()
{
    log_buf.clear();
    enable_buf_log = true;
}

tu::log_buffered::~log_buffered()
{
    enable_buf_log = false;
}

std::string const& tu::log_buffered::buf() const noexcept
{
    return log_buf;
}

void tu::log_buffered::clear()
{
    log_buf.clear();
}


void LOG__REDEMPTION__INTERNAL__IMPL(int priority, char const * format, ...) noexcept /*NOLINT(cert-dcl50-cpp)*/
{
    if (enable_buf_log) {
        if (log_is_filename(priority)) {
            return ;
        }
        va_list ap;
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
        va_start(ap, format);
        auto sz = std::vsnprintf(nullptr, 0, format, ap) + 1; /*NOLINT*/
        va_end(ap);
        log_buf.resize(log_buf.size() + sz);
        va_start(ap, format);
        std::vsnprintf(&log_buf[log_buf.size() - sz], sz, format, ap);
        va_end(ap);
        REDEMPTION_DIAGNOSTIC_POP
        log_buf.back() = '\n';

        // replace "priority (31905/31905) -- message" by "priority - message"
        if (format[0] == '%' && format[1] == 's' && format[2] == ' ' && format[3] == '(') {
            auto p = log_buf.find('(', log_buf.size() - sz + 1);
            auto e = log_buf.find('-', p);
            log_buf.erase(p, e-p);
        }
    }
    else if (is_loggable(priority))
    {
        va_list ap;
        va_start(ap, format);

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
#ifdef __EMSCRIPTEN__
        char buffer[4096];
        int len = std::vsnprintf(buffer, sizeof(buffer)-2, format, ap); /*NOLINT*/
        RED_EM_ASM({console.log(UTF8ToString($0, $1));}, buffer, len);
#else
        std::printf("rdpproxy: "); /* works with tools/rdpproxy_color.awk */
        std::vprintf(format, ap); /*NOLINT*/
        std::puts("");
        std::fflush(stdout);
#endif
        REDEMPTION_DIAGNOSTIC_POP

        va_end(ap);
    }
}
