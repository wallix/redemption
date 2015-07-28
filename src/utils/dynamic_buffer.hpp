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

#ifndef REDEMPTION_SRC_UTILS_DYNAMIC_BUFFER_HPP
#define REDEMPTION_SRC_UTILS_DYNAMIC_BUFFER_HPP

#include <memory>


struct DynamicBuffer
{
    void reserve(std::size_t new_sz) {
        if (new_sz > this->sz) {
            this->buf.reset(new char[new_sz]);
            this->sz = new_sz;
        }
    }

    char * get() const noexcept {
        return this->buf.get();
    }

    char & operator[](std::size_t i) const noexcept {
        return this->buf[i];
    }

    std::size_t size() const noexcept {
        return this->sz;
    }

private:
    std::unique_ptr<char[]> buf;
    std::size_t sz = 0;
};

#endif
