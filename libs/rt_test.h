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

   new Test RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RT_TEST_H_
#define _REDEMPTION_LIBS_RT_TEST_H_

#include "rt_constants.h"

struct RTTest {
    RT * check;
    RT * generator;
};

extern "C" {
    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RT_ERROR rt_m_RTTest_constructor(RTTest * self, 
                                            const void * data_check, size_t len_check, 
                                            const void * data_gen, size_t len_gen)
    {
        RT_ERROR status = RT_ERROR_OK;
        self->check = rt_new_check(&status, data_check, len_check);
        if (status != RT_ERROR_OK){
            return status;
        }
        self->generator = rt_new_generator(&status, data_gen, len_gen);
        if (status != RT_ERROR_OK){
            rt_delete(self->check);
            return status;
        }
        return status;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RT_ERROR rt_m_RTTest_destructor(RTTest * self)
    {
        rt_delete(self->check);
        rt_delete(self->generator);
        return RT_ERROR_OK;
    }

    inline void rt_m_RTTest_close(RTTest * self)
    {
    }


    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTTest_recv(RTTest * self, void * data, size_t len)
    {
        
        RT_ERROR err_gen = rt_get_status(self->generator);
        if (err_gen != RT_ERROR_OK){
            return -err_gen;
        }
        RT_ERROR err_check = rt_get_status(self->check);
        if (err_check != RT_ERROR_OK){
            return -err_check;
        }
        return rt_recv(self->generator, data, len);
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTTest_send(RTTest * self, const void * data, size_t len)
    {
        RT_ERROR err_gen = rt_get_status(self->generator);
        if (err_gen != RT_ERROR_OK){
            return -err_gen;
        }
        RT_ERROR err_check = rt_get_status(self->check);
        if (err_check != RT_ERROR_OK){
            return -err_check;
        }
        return rt_send(self->check, data, len);
    }

};

#endif

