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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou,
              Jonathan Poelen

   Error exception object
*/

#include "utils/stacktrace.hpp"

#ifdef REDEMPTION_WITH_STACKTRACE

# include <iostream>
# include <csignal>
# include <cstdlib>

namespace
{
    struct SEGV_Handler
    {
        SEGV_Handler() noexcept
        {
            auto handler = [](int signum) {
                ::signal(signum, SIG_DFL);
                red::print_stacktrace([](int i, auto const& line) {
                    std::cerr << "#" << i << " " << line << "\n";
                });
                std::cerr.flush();
                //boost::stacktrace::safe_dump_to("./backtrace.dump");
                // ::raise(SIGSEGV);
            };
            ::signal(SIGSEGV, handler);
            ::signal(SIGABRT, handler);
        }
    } SEGV_Handler;
} // namespace
#endif
