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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "utils/sugar/bounded_array_view.hpp"

class RfbD3DesEncrypter
{
    template<std::size_t N>
    static constexpr std::size_t value_identity_v = N;

public:
    RfbD3DesEncrypter(sized_u8_array_view<8> key) noexcept;

    /// Encrypts one block of eight bytes at address.
    /// \c from and \c to can be the same.
    void encrypt_block(sized_u8_array_view<8> from, sized_writable_u8_array_view<8> to) const noexcept;

    /// Encrypts a text of size multiple of 8.
    /// \c from and \c to can be the same.
    template<std::size_t N>
    void encrypt_text(
        sized_u8_array_view<value_identity_v<N>> from,
        sized_writable_u8_array_view<N> to,
        sized_u8_array_view<8> iv
    ) const noexcept
    {
        static_assert(N % 8 == 0);
        encrypt_text_impl(from.data(), to.data(), from.size(), iv);
    }

private:
    void encrypt_text_impl(uint8_t const* from, uint8_t* to, std::size_t length,
                           sized_u8_array_view<8> iv) const noexcept;

    uint32_t KnL[32] {};
};
