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
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

struct rdp_api {
    virtual void auth_rail_exec(uint16_t flags, const char* original_exe_or_file,
        const char* exe_or_file, const char* working_dir,
        const char* arguments, const char* account, const char* password) = 0;

    virtual void auth_rail_exec_cancel(uint16_t flags, const char* original_exe_or_file,
        uint16_t exec_result) = 0;

    virtual void sespro_rail_exec_result(uint16_t flags, const char* exe_or_file,
        uint16_t exec_result, uint32_t raw_result) = 0;

    virtual void sespro_ending_in_progress() = 0;

    virtual void sespro_launch_process_ended() = 0;

    virtual ~rdp_api() = default;
};
