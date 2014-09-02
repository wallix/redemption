/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean

   Data storage used for input data parsing
*/

#ifndef _REDEMPTION_UTILS_INPUTARRAY_HPP_
#define _REDEMPTION_UTILS_INPUTARRAY_HPP_

#include <stdint.h>


// TODO: use a functor object instead of that class
struct DataSource 
{
    virtual ~DataSource() {};
    // DataSource knows the length of data actually read, not InputArray
    virtual uint64_t read(uint8_t * target, size_t target_size) = 0;    
};


template <size_t AUTOSIZE>
class StaticInputArray
{
private:
     // we don't use size_t before it's size change on implementations 
     // and it's a portability hassle.
    uint64_t len;
    // uint8_t convey meaning of byte better than unsigned char
    uint8_t array[AUTOSIZE];
public:
    // datasource, datalen => datasource should be some function to call to provide data
    // datasource should provide the "read" interface
    // problem: what to do if read fails...
    // -> we need some result status, either exception, or member variable or result parameter
    // as member variable or exception are not desirable... => result parameter
    // result parameter should probably be part of the datasource provider
    // looks like it's failure henceforth it's responsibility if we can't get enough data
    // for our inputarray.
    
    StaticInputArray(DataSource & source)
    {
        this->len = source.read(this->array, AUTOSIZE);
    }
    
    uint64_t capacity()
    {
        return AUTOSIZE;
    }

    uint64_t size()
    {
        return this->len;
    }
    
    uint8_t get_uint8(size_t offset) {
        return array[offset];
    }

    
};

#endif

