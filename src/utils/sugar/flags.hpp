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

#include "cxx/diagnostic.hpp"

#include <initializer_list>
#include <type_traits>
#include <cstdint>
#include <cassert>


namespace utils
{

template<class E>
class enum_as_flag
{};

template<class E, class = void>
struct is_enum_flag : std::false_type
{};

template<class E>
struct is_enum_flag<E, decltype(void(enum_as_flag<E>::max))> : std::true_type
{};

template<class E>
class flags_t
{
    static_assert(std::is_enum<E>::value, "Type must be an enumeration");
    static_assert(is_enum_flag<E>::value, "Enum has not been enabled as flags by an enum_as_flag specialization");

public:
    static constexpr std::size_t max = enum_as_flag<E>::max;

    using bitfield = std::conditional_t<
        (max > 16),
        std::conditional_t<(max > 32), uint64_t, uint32_t>,
        std::conditional_t<(max > 8), uint16_t, uint8_t>
    >;

    static constexpr bitfield mask = (sizeof(E) == max) ? ~bitfield{} : bitfield((1ull << (max + 1u)) - 1u);

    constexpr flags_t() = default;

    constexpr explicit flags_t(bitfield value) noexcept
    : value(value)
    {}

    constexpr flags_t(std::initializer_list<E> l) noexcept
    {
        for (auto i : l) {
            this->set(i);
        }
    }

    constexpr flags_t(E i) noexcept
    : value(bit(i))
    {}

    [[nodiscard]] constexpr bool test(E idx) const noexcept { return this->value & bit(idx); }
    constexpr void set(E idx) noexcept { this->value |= bit(idx); }
    constexpr void clear(E idx) noexcept { this->value &= ~bit(idx); }

    constexpr void normalize() noexcept { this->value &= mask; }
    [[nodiscard]] constexpr flags_t normalized() const noexcept { return flags_t(this->value & mask); }

    [[nodiscard]] constexpr bitfield as_uint() const noexcept { return this->value & mask; }

    static constexpr std::size_t max_value() noexcept { return max; }

    constexpr flags_t operator~() const noexcept { return flags_t(~value); }

    constexpr flags_t operator|(flags_t const& rhs) const noexcept { return flags_t(value | rhs.value); }
    constexpr flags_t operator&(flags_t const& rhs) const noexcept { return flags_t(value & rhs.value); }
    constexpr flags_t operator^(flags_t const& rhs) const noexcept { return flags_t(value ^ rhs.value); }

    constexpr flags_t operator+(flags_t const& rhs) const noexcept { return flags_t(value | rhs.value); }
    constexpr flags_t operator-(flags_t const& rhs) const noexcept { return flags_t(value & ~rhs.value); }

    constexpr bool operator==(flags_t const& rhs) const noexcept { return value == rhs.value; }

    constexpr bool operator!=(flags_t const& rhs) const noexcept { return value != rhs.value; }

    constexpr flags_t& operator|=(flags_t const& rhs) noexcept
    {
        value |= rhs.value;
        return *this;
    }

    constexpr flags_t& operator&=(flags_t const& rhs) noexcept
    {
        value &= rhs.value;
        return *this;
    }

    constexpr flags_t& operator^=(flags_t const& rhs) noexcept
    {
        value ^= rhs.value;
        return *this;
    }

    constexpr flags_t& operator+=(flags_t const& other) noexcept
    {
        this->value |= other.value;
        return *this;
    }

    constexpr flags_t& operator-=(flags_t const& other) noexcept
    {
        this->value &= ~other.value;
        return *this;
    }

private:
    constexpr static bitfield bit(E idx) noexcept
    {
        auto i = bitfield(idx);
        assert(i < max);
        return bitfield{1} << i;
    }

    bitfield value = 0;
};

template<class E> constexpr flags_t<E> operator|(E lhs, flags_t<E> rhs) noexcept { return rhs | lhs; }
template<class E> constexpr flags_t<E> operator&(E lhs, flags_t<E> rhs) noexcept { return rhs & lhs; }
template<class E> constexpr flags_t<E> operator^(E lhs, flags_t<E> rhs) noexcept { return rhs ^ lhs; }
template<class E> constexpr flags_t<E> operator+(E lhs, flags_t<E> rhs) noexcept { return rhs + lhs; }
template<class E> constexpr flags_t<E> operator-(E lhs, flags_t<E> rhs) noexcept { return rhs - lhs; }


inline namespace flags_operators
{
    template<class E> constexpr std::enable_if_t<is_enum_flag<E>::value, flags_t<E>>
    operator~(E lhs) noexcept { return ~flags_t<E>(lhs); }

    template<class E> constexpr std::enable_if_t<is_enum_flag<E>::value, flags_t<E>>
    operator|(E lhs, E rhs) noexcept { return flags_t<E>(lhs) | rhs; }

    template<class E> constexpr std::enable_if_t<is_enum_flag<E>::value, flags_t<E>>
    operator&(E lhs, E rhs) noexcept { return flags_t<E>(lhs) & rhs; }

    template<class E> constexpr std::enable_if_t<is_enum_flag<E>::value, flags_t<E>>
    operator^(E lhs, E rhs) noexcept { return flags_t<E>(lhs) ^ rhs; }
} // namespace flags_operators

} // namespace utils

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wheader-hygiene")
using namespace utils::flags_operators; /*NOLINT*/
REDEMPTION_DIAGNOSTIC_POP()
