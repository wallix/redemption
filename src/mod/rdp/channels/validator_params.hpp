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
    Copyright (C) Wallix 2019
    Author(s): Christophe Grosjean
*/

#pragma once

#include "configs/autogen/enums.hpp" // Language

#include <string>
#include <chrono>


struct ValidatorParams
{
    std::string up_target_name;
    std::string down_target_name;
    bool log_if_accepted = false;
    bool enable_clipboard_text_up = false;
    bool enable_clipboard_text_down = false;

    bool block_invalid_file_up = false;
    bool block_invalid_file_down = false;
    uint64_t max_file_size_rejected = 1024*1024*1024;
    std::chrono::seconds osd_delay = std::chrono::seconds(5);
    Language lang = Language::en;
};
