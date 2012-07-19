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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__UTILS_AUTO_BUFFER_HPP__)
#define __UTILS_AUTO_BUFFER_HPP__

#include <stdint.h>
#include <cstddef>

class AutoBuffer
{
public:
    static const uint AUTOSIZE = 8192;

private:
    uint8_t _autobuffer[AUTOSIZE];
    uint8_t* _buffer;
    std::size_t _size;
    std::size_t _capacity;

public:
    AutoBuffer()
    : _buffer(this->_autobuffer)
    , _size(AUTOSIZE)
    , _capacity(AUTOSIZE)
    {}

    AutoBuffer(std::size_t size)
    : _buffer(size > AUTOSIZE ? new uint8_t[size] : this->_autobuffer)
    , _size(size)
    , _capacity(size > AUTOSIZE ? size : AUTOSIZE)
    {}

    ~AutoBuffer()
    {
        if (this->_buffer != this->_autobuffer)
        {
            delete[] this->_buffer;
        }
    }

    uint8_t* get()
    {
        return this->_autobuffer;
    }

    const uint8_t* get() const
    {
        return this->_autobuffer;
    }

    std::size_t size() const
    {
        return this->_size;
    }

    std::size_t capacity() const
    {
        return this->_capacity;
    }

    void alloc(std::size_t new_size)
    {
        if (new_size > this->_capacity)
        {
            if (this->_buffer != this->_autobuffer)
            {
                delete[] this->_buffer;
            }
            this->_buffer = new uint8_t[new_size];
            this->_capacity = new_size;
        }
        else if (new_size > AUTOSIZE)
        {
            this->_buffer = new uint8_t[new_size];
            this->_capacity = new_size;
        }
        this->_size = new_size;
    }

    //void realloc(std::size_t new_size, std::size copy = -1);
};

#endif