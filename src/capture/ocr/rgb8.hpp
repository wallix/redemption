/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

namespace ocr
{
    class rgb8
    {
    public:
        using value_type = unsigned char;

    public:
        explicit rgb8() = default;

        constexpr rgb8(value_type c1, value_type c2, value_type c3) noexcept
        : components{c1, c2, c3}
        {}

        [[nodiscard]] constexpr value_type red()   const noexcept { return this->components[0]; }
        [[nodiscard]] constexpr value_type green() const noexcept { return this->components[1]; }
        [[nodiscard]] constexpr value_type blue()  const noexcept { return this->components[2]; }

        void red(value_type x)   noexcept { this->components[0] = x; }
        void green(value_type x) noexcept { this->components[1] = x; }
        void blue(value_type x)  noexcept { this->components[2] = x; }

        template<class Color>
        constexpr bool operator==(const Color & c) const noexcept
        {
            return this->red() == c.red()
                && this->green() == c.green()
                && this->blue() == c.blue();
        }

        template<class Color>
        constexpr bool operator!=(const Color & c) const noexcept
        { return !(*this == c); }

    private:
        value_type components[3];
    };
} // namespace ocr
