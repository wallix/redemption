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

#include <string>

#include "scard/scard_pack_type_native.hpp"


///////////////////////////////////////////////////////////////////////////////


///
const char * return_code_to_string(SCARD_PACK_NATIVE_LONG code);

///
const char * attribute_to_string(SCARD_PACK_NATIVE_LONG attribute);

///
const char * protocols_to_string(SCARD_PACK_NATIVE_LONG protocols);

///
const char * share_mode_to_string(SCARD_PACK_NATIVE_LONG share_mode);

///
const char * disposition_to_string(SCARD_PACK_NATIVE_LONG disposition);

///
const char * scope_to_string(SCARD_PACK_NATIVE_LONG scope);

///
const char * card_state_to_string(SCARD_PACK_NATIVE_LONG card_state);

///
std::string reader_state_to_string(SCARD_PACK_NATIVE_DWORD reader_state);