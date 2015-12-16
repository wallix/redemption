/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_GDI_INPUT_KBD_API_HPP
#define REDEMPTION_GDI_INPUT_KBD_API_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"

namespace gdi {

struct InputKbdApi : private noncopyable
{
    virtual ~InputKbdApi() = default;

    virtual bool input_kbd(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) = 0;
};

using InputKbdApiDeleterBase = utils::virtual_deleter_base<InputKbdApi>;
using InputKbdApiPtr = utils::unique_ptr_with_virtual_deleter<InputKbdApi>;

using utils::default_delete;
using utils::no_delete;

template<class InputKbd, class... Args>
InputKbdApiPtr make_input_kbd_ptr(Args && ... args) {
    return InputKbdApiPtr(new InputKbd(std::forward<Args>(args)...), default_delete);
}

template<class InputKbd>
InputKbdApiPtr make_input_kbd_ref(InputKbd & gd) {
    return InputKbdApiPtr(&gd, no_delete);
}

}

#endif
