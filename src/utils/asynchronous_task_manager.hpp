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

    struct TerminateEventNotifier
    {
        using ptr_function = void(*)(void* data, AsynchronousTask&) noexcept;

        explicit TerminateEventNotifier() = default;

        template<class T, class F>
        TerminateEventNotifier(T* p, F f)
        noexcept(noexcept(static_cast<void(*)(T*, AsynchronousTask&) noexcept>(f)))
        : data(p)
        , f([](void* data, AsynchronousTask& t) noexcept {
            char f[1]{};
            reinterpret_cast<F&>(f)(static_cast<T*>(data), t); /*NOLINT*/
        })
        {}

        template<class T>
        TerminateEventNotifier(T* p, ptr_function f) noexcept
        : data(p)
        , f(f)
        {}

        TerminateEventNotifier(TerminateEventNotifier&&) = default;
        TerminateEventNotifier(TerminateEventNotifier const&) = default;
        TerminateEventNotifier& operator=(TerminateEventNotifier&&) = default;
        TerminateEventNotifier& operator=(TerminateEventNotifier const&) = default;

        void operator()(AsynchronousTask& self) noexcept
        {
            this->f(this->data, self);
        }

    private:
        void* data = nullptr;
        ptr_function  f = [](void* /*unused*/, AsynchronousTask& /*unused*/) noexcept {};
    };

    virtual void configure_event(SessionReactor&, TerminateEventNotifier) = 0;
};

