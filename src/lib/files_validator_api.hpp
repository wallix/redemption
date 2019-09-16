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
   Copyright (C) Wallix 2019
   Author(s): Clement Moroldo
*/

#pragma once

#include "cxx/cxx.hpp"

#include <cstdint>


extern "C"
{
    class ValidatorApi;
    using ValidatorFileId = uint32_t;

    REDEMPTION_LIB_EXPORT
    ValidatorApi* validator_open_session(char const* socket_path) noexcept;

    REDEMPTION_LIB_EXPORT
    ValidatorApi* validator_open_fd_session(int fd) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_close_session(ValidatorApi* validator) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_get_fd(ValidatorApi* validator) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_get_errno(ValidatorApi* validator) noexcept;

    REDEMPTION_LIB_EXPORT
    ValidatorFileId validator_open_file(ValidatorApi* validator, char const* file_name, char const* target_name) noexcept;

    REDEMPTION_LIB_EXPORT
    ValidatorFileId validator_open_text(ValidatorApi* validator, uint32_t locale_identifier, char const* target_name) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_send_data(ValidatorApi* validator, ValidatorFileId id, char const* data, unsigned size) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_end_of_file(ValidatorApi* validator, ValidatorFileId id) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_abort_file(ValidatorApi* validator, ValidatorFileId id) noexcept;

    /// \return ICAPService::ResponseType
    REDEMPTION_LIB_EXPORT
    int validator_receive_response(ValidatorApi* validator) noexcept;

    /// \return ICAPService::ResponseType
    REDEMPTION_LIB_EXPORT
    int validator_get_response_type(ValidatorApi* validator) noexcept;

    /// \return LocalICAPProtocol::ValidationType
    REDEMPTION_LIB_EXPORT
    int validator_get_result_flag(ValidatorApi* validator) noexcept;

    REDEMPTION_LIB_EXPORT
    char const* validator_get_content(ValidatorApi* validator) noexcept;

    REDEMPTION_LIB_EXPORT
    ValidatorFileId validator_get_result_file_id(ValidatorApi* validator) noexcept;
}
