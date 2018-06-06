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
Copyright (C) Wallix 2010
Author(s): Jonathan Poelen
*/

#include "utils/set_exception_handler_pretty_message.hpp"
#include "core/error.hpp"

#include <exception>
#include <iostream>
#include <cstring>


namespace
{
    std::terminate_handler old_terminate_handler;
}

void set_exception_handler_pretty_message() noexcept
{
    old_terminate_handler = std::set_terminate([]{
        auto eptr = std::current_exception();
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch(const Error& e) {
            if (e.errnum) {
                std::cerr << e.errmsg() << " - " << strerror(e.errnum) << "\n";
            }
            else {
                std::cerr << e.errmsg() << "\n";
            }
        } catch(...) {
        }
        old_terminate_handler();
    });
}
