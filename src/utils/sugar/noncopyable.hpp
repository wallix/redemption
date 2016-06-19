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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#pragma once

namespace adl_barrier
{
    class noncopyable
    {
    protected:
        noncopyable() = default;
        ~noncopyable() = default;

        noncopyable(noncopyable&&) = delete;
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(noncopyable&&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };

    class noncopyable_but_movable
    {
    protected:
        noncopyable_but_movable() = default;
        ~noncopyable_but_movable() = default;

        noncopyable_but_movable(noncopyable_but_movable&&) = default;
        noncopyable_but_movable(const noncopyable_but_movable&) = delete;
        noncopyable_but_movable& operator=(noncopyable_but_movable&&) = default;
        noncopyable_but_movable& operator=(const noncopyable_but_movable&) = delete;
    };
}

using noncopyable = adl_barrier::noncopyable;
using noncopyable_but_movable = adl_barrier::noncopyable_but_movable;

#define REDEMPTION_NON_COPYABLE(class_name) \
    class_name(const class_name&) = delete; \
    class_name& operator=(const class_name&) = delete

