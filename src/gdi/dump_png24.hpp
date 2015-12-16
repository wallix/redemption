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

#ifndef REDEMPTION_GDI_DUMP_PNG24_HPP
#define REDEMPTION_GDI_DUMP_PNG24_HPP

#include "utils/virtual_deleter.hpp"

#include "noncopyable.hpp"

class Transport;

namespace gdi {

struct DumpPng24Api : private noncopyable
{
    virtual ~DumpPng24Api() = default;

    virtual void dump_png24(Transport & trans, bool bgr) const = 0;
};

using DumpPng24ApiDeleterBase = utils::virtual_deleter_base<DumpPng24Api>;
using DumpPng24ApiPtr = utils::unique_ptr_with_virtual_deleter<DumpPng24Api>;

using utils::default_delete;
using utils::no_delete;

template<class DumpPng24, class... Args>
DumpPng24ApiPtr make_dump_png24_ptr(Args && ... args) {
    return DumpPng24ApiPtr(new DumpPng24(std::forward<Args>(args)...), default_delete);
}

template<class DumpPng24>
DumpPng24ApiPtr make_dump_png24_ref(DumpPng24 & gd) {
    return DumpPng24ApiPtr(&gd, no_delete);
}

}

#endif
