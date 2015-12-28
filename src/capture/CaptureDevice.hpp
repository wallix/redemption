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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CAPTURE_CAPTUREDEVICE_HPP_
#define _REDEMPTION_CAPTURE_CAPTUREDEVICE_HPP_

#include "noncopyable.hpp"

#include <cstdint>

class Stream;

struct RDPCaptureDevice : noncopyable {
    virtual void set_row(size_t rownum, const uint8_t * data) {}

    // Return false to prevent data to be sent to RDP server.
    virtual bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) { return true; }

    virtual void snapshot(const timeval & now, int mouse_x, int mouse_y,
        bool ignore_frame_in_timeval, bool const & requested_to_stop) {}

    virtual void set_pointer_display() {}

    // toggles externally genareted breakpoint.
    virtual void external_breakpoint() {}

    virtual void external_time(const timeval & now) {}

    virtual void session_update(const timeval & now, const char * message) {}

protected:
    // this to avoid calling constructor of base abstract class
    RDPCaptureDevice() = default;

public:
    // we choosed to make destructor virtual to allow destructing object
    // through pointer of base class. As this class is interface only
    // it does not looks really usefull.
    virtual ~RDPCaptureDevice() = default;
};

#endif
