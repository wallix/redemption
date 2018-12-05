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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include <functional>


namespace redemption_unit_test__
{
    namespace emscripten
    {
        using exception_translator_fn = std::function<void()>;
        void add_register_exception_translator(exception_translator_fn f);
    }

    template<class Exception, class F>
    void register_exception_translator(F&& f)
    {
        emscripten::add_register_exception_translator(
            [f = static_cast<F&&>(f)](){
                try {
                    throw;
                }
                catch (Exception const& e)
                {
                    f(e);
                }
                catch (...)
                {
                }
            }
        );
    }
} // namespace redemption_unit_test__
