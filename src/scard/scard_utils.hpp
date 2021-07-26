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
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


#define ERR_SCARD ERR_RDP_DATA_TRUNCATED


///////////////////////////////////////////////////////////////////////////////


#define SCARD_CHECK_PREDICATE(predicate, eid) \
    check_predicate((predicate), __FILE__, __FUNCTION__, __LINE__, (#predicate), (eid));

///
void check_predicate(bool predicate, const char *file, const char *function,
    int line, const char *name, error_type eid);

///
std::string bytes_to_hex_string(bytes_view data,
    std::string_view byte_separator = "");

///
std::string trim(const std::string &string);

///
bool compare_case_insensitive(
    std::string_view string_1,
    std::string_view string_2);

///
template<typename HandleType, std::size_t MaxHandleCount, HandleType ReservedValue>
class static_handle_table
{
    static_assert(ReservedValue <= (MaxHandleCount - 1),
        "ReservedValue must be comprised between 0 and (MaxHandleCount - 1) (inclusive)");

public:
    ///
    static_handle_table()
    {
        _handles.fill(ReservedValue);
    }

    ///
    HandleType allocate()
    {
        const auto begin = _handles.begin();
        const auto end = _handles.end();

        auto start = begin;
        for (;;)
        {
            // find a free slot in the handle array
            // (a free slot is an array element that equals to the reserved value)
            auto it = std::find(start, end, ReservedValue);
            if (it == end)
            {
                /* table is full */

                break;
            }

            /* table is not full but the only free slot may be the reserved slot */

            // form the candidate handle
            // (a handle is the index of the selected array element)
            auto handle = std::distance(begin, it);
            if (handle != ReservedValue)
            {
                /* candidate handle is valid */

                // allocate the handle in the table
                *it = handle;

                return handle;
            }

            /* candidate handle is the reserved handle */

            // set the start position to the next element
            start = std::next(it);
        }

        /* table is full */

        return ReservedValue;
    }

    ///
    bool deallocate(HandleType handle)
    {
        if ((handle == ReservedValue)
                || (static_cast<std::size_t>(handle) >= MaxHandleCount))
        {
            return false;
        }

        assert(_handles[handle] == handle);

        // deallocate handle
        _handles[handle] = ReservedValue;

        return true;
    }

private:
    ///
    std::array<
        HandleType,
        MaxHandleCount + 1  // +1 as the reserved value is... well... reserved
    > _handles;
};