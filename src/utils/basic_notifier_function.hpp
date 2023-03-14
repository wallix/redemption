/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <type_traits>

#include <cstring>


template<class... Args>
struct BasicNotifierFunction
{
    explicit BasicNotifierFunction() noexcept = default;

    explicit BasicNotifierFunction(decltype(nullptr)) noexcept
    {}

    BasicNotifierFunction(BasicNotifierFunction &&) noexcept = default;
    BasicNotifierFunction(BasicNotifierFunction const&) noexcept = default;

    BasicNotifierFunction& operator=(BasicNotifierFunction &&) noexcept = default;
    BasicNotifierFunction& operator=(BasicNotifierFunction const&) noexcept = default;

    template<class F>
    BasicNotifierFunction(F f) noexcept
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
        notifier = [](void* d, Args... args) { (*static_cast<F*>(d))(static_cast<Args&&>(args)...); };
    }

    explicit operator bool () const noexcept
    {
        return notifier != noop;
    }

    void operator()(Args... args)
    {
        notifier(data, static_cast<Args&&>(args)...);
    }

private:
    static void noop(void* /*d*/, Args... /*args*/)
    {
    }

    decltype(noop)* notifier = &noop;
    char data[sizeof(void*)];
};
