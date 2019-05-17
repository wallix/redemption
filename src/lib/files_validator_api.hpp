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



extern "C"
{
    class ICAPService;

    REDEMPTION_LIB_EXPORT
    ICAPService * validator_open_session(const char * socket_path) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_open_file(ICAPService * service, const char * file_name, const char * target_name) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_send_data(const ICAPService * service, const int file_id, const char * data, const int size) noexcept;

    REDEMPTION_LIB_EXPORT
    void validator_receive_response(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_end_of_file(ICAPService * service, const int file_id) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_close_session(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_abort_file(ICAPService * service, const int file_id) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_get_result_flag(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    const char * validator_get_content(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    bool validator_session_is_open(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_get_fd(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    int validator_get_result_file_id(ICAPService * service) noexcept;

    REDEMPTION_LIB_EXPORT
    bool service_is_up(ICAPService * service) noexcept;
}
