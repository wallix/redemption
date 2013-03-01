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

   new Socket RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RT_SOCKET_H_
#define _REDEMPTION_LIBS_RT_SOCKET_H_

#include "rio_constants.h"
#include "netutils.hpp"

struct RTSocket {
    int sck;
};

extern "C" {
    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RT_ERROR rt_m_RTSocket_constructor(RTSocket * self, int sck)
    {
        self->sck = sck;
        return RT_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RT_ERROR rt_m_RTSocket_destructor(RTSocket * self)
    {
        return RT_ERROR_OK;
    }

    inline void rt_m_RTSocket_close(RTSocket * self)
    {
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTSocket_recv(RTSocket * self, void * data, size_t len)
    {
        char * pbuffer = (char*)data;
        size_t remaining_len = len;

        while (remaining_len > 0) {
            ssize_t res = ::recv(self->sck, pbuffer, len, 0);
            switch (res) {
                case -1: /* error, maybe EAGAIN */
                    if (try_again(errno)) {
                        fd_set fds;
                        struct timeval time = { 0, 100000 };
                        FD_ZERO(&fds);
                        FD_SET(self->sck, &fds);
                        select(self->sck + 1, &fds, NULL, NULL, &time);
                        continue;
                    }
                    TODO("replace this with actual error management, EOF is not even an option for sockets")
                    return -RT_ERROR_EOF;
                case 0: /* no data received, socket closed */
                    return -RT_ERROR_EOF;
                default: /* some data received */
                    pbuffer += res;
                    remaining_len -= res;
                break;
            }
        }
        return len;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTSocket_send(RTSocket * self, const void * data, size_t len)
    {
        size_t total = 0;
        while (total < len) {
            ssize_t sent = ::send(self->sck, &(((uint8_t*)data)[total]), len - total, 0);
            switch (sent){
            case -1:
                if (try_again(errno)) {
                    fd_set wfds;
                    struct timeval time = { 0, 10000 };
                    FD_ZERO(&wfds);
                    FD_SET(self->sck, &wfds);
                    select(self->sck + 1, NULL, &wfds, NULL, &time);
                    continue;
                }
                return RT_ERROR_EOF;
            case 0:
                return RT_ERROR_EOF;
            default:
                total = total + sent;
            }
        }
        return len;
    }
};

#endif

