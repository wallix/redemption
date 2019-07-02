/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#ifdef SHOW_DEBUG_TRACES

namespace
{
    struct Trace
    {
        Trace(char const * func_name) noexcept
        : func_name(func_name)
        {
            LOG(LOG_INFO, "%s()", func_name);
        }

        ~Trace()
        {
            LOG(LOG_INFO, "%s() done", func_name);
        }

        void exit_on_exception() noexcept
        {
            LOG(LOG_ERR, "%s() exit with exception", func_name);
        }

        void exit_on_error(Error const & e) noexcept
        {
            LOG(LOG_ERR, "%s() exit with exception Error: %s", func_name, e.errmsg());
        }

    private:
        char const * func_name;
    };
} // namespace


#define SCOPED_TRACE Trace trace_l_ {__FUNCTION__}
#define EXIT_ON_EXCEPTION() trace_l_.exit_on_exception()
#define EXIT_ON_ERROR(e) trace_l_.exit_on_error(e)

#else

#define SCOPED_TRACE
#define EXIT_ON_EXCEPTION()
#define EXIT_ON_ERROR(e)

#endif

#define CHECK_HANDLE(handle) if (!handle) return -1
#define CHECK_HANDLE_R(handle, return_err) if (!handle) return return_err

#define CHECK_NOTHROW_R(expr, return_err, error_ctx, errid) \
    do {                                                    \
        try { expr; }                                       \
        catch (Error const& err) {                          \
            EXIT_ON_ERROR(err);                             \
            error_ctx.set_error(err);                       \
            return return_err;                              \
        }                                                   \
        catch (...) {                                       \
            EXIT_ON_EXCEPTION();                            \
            error_ctx.set_error(Error{errid});              \
            return return_err;                              \
        }                                                   \
    } while (0)
