/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   new Check RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RT_CHECK_H_
#define _REDEMPTION_LIBS_RT_CHECK_H_

#include "rt_constants.h"

struct RTCheck {
};

extern "C" {
    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RT_ERROR rt_m_RTCheck_constructor(RTCheck * self, const void * data, size_t len)
    {
        return RT_ERROR_NOT_IMPLEMENTED;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RT_ERROR rt_m_RTCheck_destructor(RTCheck * self)
    {
        return RT_ERROR_OK;
    }

    /* This method close ressource without calling destructor
       Any subsequent call should return an error
    */
    inline RT_ERROR rt_m_RTCheck_close(RTCheck * self)
    {
        return RT_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTCheck_recv(RTCheck * self, void * data, size_t len)
    {
         return -RT_ERROR_SEND_ONLY;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTCheck_send(RTCheck * self, const void * data, size_t len)
    {
         return -RT_ERROR_RECV_ONLY;
    }


};

#endif

