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
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#pragma once

#include "configs/autogen/authid.hpp"

#include <cstdint>


struct AclFieldMask
{
    bool has(configs::authid_t i) const noexcept
    {
        return bits_[unsigned(i)/64] & (uint64_t{1} << (unsigned(i)%64));
    }

    template<class Cfg>
    bool has() const noexcept
    {
        static_assert(Cfg::is_sesman_to_proxy);
        return has(Cfg::index);
    }

    void set(configs::authid_t i) noexcept
    {
        bits_[unsigned(i)/64] |= (uint64_t{1} << (unsigned(i)%64));
    }

    bool is_empty() const noexcept
    {
        for (auto bits : bits_) {
            if (bits) {
                return false;
            }
        }
        return true;
    }

    void clear() noexcept
    {
        for (auto& bits : bits_)
            bits = 0;
    }

    void clear(configs::authid_t i) noexcept
    {
        bits_[unsigned(i)/64] &= ~(uint64_t{1} << (unsigned(i)%64));
    }

    void clear(AclFieldMask const& other) noexcept
    {
        auto it = other.bits_;
        for (auto& bits : bits_) {
            bits &= ~*it;
            ++it;
        }
    }

private:
    uint64_t bits_[unsigned(configs::max_authid) / 64 + unsigned(configs::max_authid) % 64] {};
};
