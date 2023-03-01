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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#pragma once

#include <type_traits>

#include <cstring>


struct WidgetEventNotifier
{
    explicit WidgetEventNotifier() noexcept = default;

    explicit WidgetEventNotifier(decltype(nullptr)) noexcept
    {}

    WidgetEventNotifier(WidgetEventNotifier &&) noexcept = default;
    WidgetEventNotifier(WidgetEventNotifier const&) noexcept = default;

    WidgetEventNotifier& operator=(WidgetEventNotifier &&) noexcept = default;
    WidgetEventNotifier& operator=(WidgetEventNotifier const&) noexcept = default;

    template<class F>
    WidgetEventNotifier(F f) noexcept
    {
        static_assert(sizeof(F) <= sizeof(data));
        static_assert(std::is_trivially_copyable_v<F>);
        static_assert(std::is_trivially_destructible_v<F>);

        if constexpr (std::is_pointer_v<F>) {
            if (!f) {
                notifier = &noop;
                return;
            }
        }

        std::memcpy(data, &f, sizeof(F));
        notifier = [](void* d) { (*static_cast<F*>(d))(); };
    }

    explicit operator bool () const noexcept
    {
        return notifier != noop;
    }

    void operator()()
    {
        notifier(data);
    }

private:
    static void noop(void* /*d*/)
    {
    }

    decltype(noop)* notifier = &noop;
    char data[sizeof(void*)];
};
