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
   Author(s): Clément Moroldo
*/


#include "lib/files_validator_api.hpp"
<<<<<<< HEAD
#include "mod/icap_files_service.hpp"
=======
#include "lib/icap_files_service.hpp"
>>>>>>> d2ca37c04... update files_validator_api



extern "C"
{
    ICAPService * validator_open_session(const char * socket_path) noexcept {
        return icap_open_session(socket_path);
    }

    int validator_open_file(ICAPService * service, const char * file_name) noexcept {
        return icap_open_file(service, file_name);
    }

    int validator_send_data(const ICAPService * service, const int file_id, const char * data, const int size) noexcept {
        return icap_send_data(service, file_id, data, size);
    }

    void validator_receive_result(ICAPService * service) noexcept {
        return icap_receive_result(service);
    }

    int validator_end_of_file(ICAPService * service, const int file_id) noexcept {
        return icap_end_of_file(service, file_id);
    }

    int validator_close_session(ICAPService * service) noexcept {
        return icap_close_session(service);
    }

    int validator_abort_file(ICAPService * service, const int file_id) noexcept {
        return icap_abort_file(service, file_id);
    }


    //test utilitaries functions
    int validator_get_result(ICAPService * service) noexcept {
        return service->result;
    }

    const char * validator_get_content(ICAPService * service) noexcept {
        return service->content.c_str();
    }

    int validator_get_fd(ICAPService * service) noexcept {
        return service.fd.fd();
    }

    bool validator_session_is_open(ICAPService * service) noexcept {
        return service->fd.is_open();
    }
}
