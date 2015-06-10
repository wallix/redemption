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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_UTILS_TO_SERVER_SENDER_HPP_
#define _REDEMPTION_UTILS_TO_SERVER_SENDER_HPP_

class ToServerSender {
public:
    virtual ~ToServerSender() = default;

    virtual void operator() (size_t total_length, uint32_t flags, const uint8_t * chunk_data, size_t chunk_data_length) = 0;
};

 #endif // #ifndef _REDEMPTION_UTILS_TO_SERVER_SENDER_HPP_