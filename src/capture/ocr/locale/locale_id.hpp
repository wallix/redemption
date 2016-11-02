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

#pragma once

#include <type_traits>

namespace ocr { namespace locale {

struct LocaleId {
    using type_id = unsigned;
    enum E : type_id {
        latin
      , cyrillic
      , max
    };

    explicit LocaleId(type_id id) noexcept
    : id(id)
    {}

    template<class T, class = std::enable_if<std::is_same<E, T>::value>>
    LocaleId(T id) noexcept
    : id(id)
    {}

    LocaleId & operator=(type_id id) noexcept
    {
        this->id = id;
        return *this;
    }

    operator unsigned () const noexcept
    { return this->id; }

private:
    type_id id;
};

} }
