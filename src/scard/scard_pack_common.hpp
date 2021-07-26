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

#include <cstddef>
#include <stdexcept>
#include <string>

#include "scard/scard_common.hpp"


///////////////////////////////////////////////////////////////////////////////


class InStream;
class OutStream;

///
class scard_pack_packable
{
public:
    ///
    virtual ~scard_pack_packable() = default;

    ///
    virtual std::size_t pack(OutStream &/*stream*/) const
    {
        throw std::runtime_error("Not implemented");
    }

    ///
    virtual std::size_t unpack(InStream &/*stream*/)
    {
        throw std::runtime_error("Not implemented");
    }

    ///
    virtual std::size_t packed_size() const
    {
        throw std::runtime_error("Not implemented");
    }

    ///
    virtual std::size_t min_packed_size() const = 0;

    ///
    /*
    virtual std::size_t max_packed_size() const = 0;
    */

    ///
    virtual void log(int level) const = 0;
};


///
class scard_pack_exception : public scard_exception
{
public:
    ///
    scard_pack_exception(const char *message)
        :
        scard_exception(message)
    {
    }

    ///
    scard_pack_exception(const std::string &message)
        :
        scard_exception(message)
    {
    }
};