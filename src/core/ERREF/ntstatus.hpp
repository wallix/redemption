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
    Author(s): Clément Moroldo
*/


#pragma once

#include <cinttypes>
#include <cinttypes>

#include "utils/sugar/cast.hpp"
#include "utils/stream.hpp"

namespace erref {

// 2.3.1 NTSTATUS Values
//
// By combining the NTSTATUS into a single 32-bit numbering space, the following NTSTATUS values are defined. Most values also have a defined default message that can be used to map the value to a human-readable text message. When this is done, the NTSTATUS value is also known as a message identifier.
//
// This document provides the common usage details of the NTSTATUS values; individual protocol specifications provide expanded or modified definitions when needed.
//
// In the following descriptions, a percentage sign that is followed by one or more alphanumeric characters (for example, "%1" or "%hs") indicates a variable that is replaced by text at the time the value is returned.

//  +--------------------------+-------------------------------------------------------+
//  | Return value/code        | Description                                           |
//  +--------------------------+-------------------------------------------------------+
//  | 0x00000000               | The operation completed successfully.                 |
//  | STATUS_SUCCESS           |                                                       |
//  +--------------------------+-------------------------------------------------------+
//  | 0xC000000F               | {File Not Found} The file %hs does not exist.         |
//  | STATUS_NO_SUCH_FILE      |                                                       |
//  |                          |                                                       |
//  +--------------------------+-------------------------------------------------------+
//  | 0x80000006               | {No More Files} No more files were found which match  |
//  | STATUS_NO_MORE_FILES     | the file specification.                               |
//  |                          |                                                       |
//  +--------------------------+-------------------------------------------------------+
//  | 0xC0000001               | {Operation Failed} The requested operation was        |
//  | STATUS_UNSUCCESSFUL      | unsuccessful.                                         |
//  |                          |                                                       |
//  +--------------------------+-------------------------------------------------------+
//  | 0xC000000D               | An invalid parameter was passed to a service or       |
//  | STATUS_INVALID_PARAMETER | function.                                             |
//  |                          |                                                       |
//  +--------------------------+-------------------------------------------------------+
//  | 0xC0000022               | {Access Denied} A process has requested access to an  |
//  | STATUS_ACCESS_DENIED     | object but has not been granted those access rights.  |
//  |                          |                                                       |
//  +--------------------------+-------------------------------------------------------+
//  | ...                      | ...                                                   |
//  |                          |                                                       |
//

enum class NTSTATUS : uint32_t
{
    STATUS_SUCCESS                  = 0x00000000,
    STATUS_UNSUCCESSFUL             = 0xC0000001,
    STATUS_INVALID_PARAMETER        = 0xC000000D,
    STATUS_NO_SUCH_FILE             = 0xC000000F,
    STATUS_NO_MORE_FILES            = 0x80000006,
    STATUS_ACCESS_DENIED            = 0xC0000022,
    STATUS_OBJECT_NAME_INVALID      = 0xC0000033,
    STATUS_OBJECT_NAME_COLLISION    = 0xC0000035,
    // ...
};

inline const char * get_NTStatus(NTSTATUS ntstatus) noexcept
{
#define CASE_TO_STR(name) case NTSTATUS::name: return #name;
    switch (ntstatus) {
        CASE_TO_STR(STATUS_SUCCESS)
        CASE_TO_STR(STATUS_UNSUCCESSFUL)
        CASE_TO_STR(STATUS_INVALID_PARAMETER)
        CASE_TO_STR(STATUS_NO_SUCH_FILE)
        CASE_TO_STR(STATUS_NO_MORE_FILES)
        CASE_TO_STR(STATUS_ACCESS_DENIED)
        CASE_TO_STR(STATUS_OBJECT_NAME_INVALID)
        CASE_TO_STR(STATUS_OBJECT_NAME_COLLISION)
        // ...
    }
#undef CASE_TO_STR

    return "<unknown>";
}

}  // namespace erref
