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

#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/buffer_view.hpp"
#include "cxx/cxx.hpp"

#include <cstring>


struct RemainingDataResult
{
    enum class [[nodiscard]] Status
    {
        Ok,
        DataToLarge,
        Eof,
    };

    Status status;
    writable_bytes_view data;
};

/// \pre remaining_data.data() != nullptr
/// \post RemainingDataResult.data.data() != nullptr
/// \post RemainingDataResult.data.size() <= buffer.size()
template<class PartialReader>
RemainingDataResult move_remaining_data(
    bytes_view remaining_data, writable_buffer_view buffer,
    PartialReader&& partial_read)
noexcept(noexcept(partial_read(buffer)))
{
    using Status = RemainingDataResult::Status;

    if (REDEMPTION_UNLIKELY(remaining_data.size() >= buffer.size()))
    {
        return {Status::DataToLarge, {buffer.data(), 0}};
    }

    memmove(buffer.data(), remaining_data.data(), remaining_data.size());

    const auto free_buffer_len = buffer.size() - remaining_data.size();

    size_t len = partial_read(buffer.subarray(remaining_data.size(), free_buffer_len));

    if (REDEMPTION_UNLIKELY(len == 0))
    {
        return {Status::Eof, {buffer.data(), 0}};
    }

    return {Status::Ok, buffer.first(len + remaining_data.size())};
}
