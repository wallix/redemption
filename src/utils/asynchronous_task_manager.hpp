/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

class SessionReactor;

class AsynchronousTask {
public:
    virtual ~AsynchronousTask() = default;

    struct DeleterFunction
    {
        using ptr_function = void(*)(void* data, AsynchronousTask&) noexcept;

        explicit DeleterFunction() = default;

        template<class T, class F>
        DeleterFunction(T* p, F f)
        : data(p)
        , f(reinterpret_cast<ptr_function>(
            static_cast<void(*)(T* data, AsynchronousTask&) noexcept>(f)))
        {}

        void operator()(AsynchronousTask& self) noexcept
        {
            this->f(this->data, self);
        }

    private:
        void* data = nullptr;
        ptr_function  f = [](void*, AsynchronousTask&) noexcept {};
    };

    virtual void configure_event(SessionReactor&, DeleterFunction) = 0;
};

