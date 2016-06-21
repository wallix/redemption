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
    class movable_noncopyable
    {
    protected:
        movable_noncopyable() = default;
        ~movable_noncopyable() = default;

        movable_noncopyable(const movable_noncopyable&) = delete;
        movable_noncopyable& operator=(movable_noncopyable&&) = default;

        movable_noncopyable(movable_noncopyable&&) = default;
        movable_noncopyable& operator=(const movable_noncopyable&) = delete;
    };
}

using movable_noncopyable = adl_barrier::movable_noncopyable;

#define REDEMPTION_MOVABLE(class_name)  \
    class_name(class_name&&) = default; \
    class_name& operator=(class_name&&) = default

