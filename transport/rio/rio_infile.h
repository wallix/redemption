/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   New Outfile RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RIO_INFILE_H_
#define _REDEMPTION_LIBS_RIO_INFILE_H_

#include "rio.h"

extern "C" {
    struct RIOInfile {
        int fd;
    };

    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOInfile_constructor(RIOInfile * self, int fd)
    {
        self->fd = fd;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOInfile_destructor(RIOInfile * self)
    {
        return RIO_ERROR_CLOSED;
    }


    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOInfile_recv(RIOInfile * self, void * data, size_t len)
    {
        ssize_t ret = 0;
        size_t remaining_len = len;
        while (remaining_len) {
            ret = ::read(self->fd, &(((char*)data)[len - remaining_len]), remaining_len);
            if (ret < 0){
                if (errno == EINTR){ continue; }
                // Error should still be there next time we try to read
                if (remaining_len != len){
                    return len - remaining_len;
                }
                RIO_ERROR err = RIO_ERROR_OK;
                switch (errno){
                    case EAGAIN:
                        err = RIO_ERROR_EAGAIN;
                        break;
                    case EBADF:
                        err = RIO_ERROR_EBADF;
                        break;
                    case EFAULT:
                        err = RIO_ERROR_EFAULT;
                        break;
                    case EINVAL:
                        err = RIO_ERROR_EINVAL;
                        break;
                    case EIO:
                        err = RIO_ERROR_EIO;
                        break;
                    case EISDIR:
                        err = RIO_ERROR_EISDIR;
                        break;
                    default:
                        err = RIO_ERROR_POSIX;
                        break;
                }
                rio_m_RIOInfile_destructor(self);
                return -err;
            }
            // We must exit loop or we will enter infinite loop
            if (ret == 0){
                break;
            }
            remaining_len -= ret;
        }
        return len - remaining_len;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOInfile_send(RIOInfile * self, const void * data, size_t len)
    {
         rio_m_RIOInfile_destructor(self);
         return -RIO_ERROR_RECV_ONLY;
    }

    static inline RIO_ERROR rio_m_RIOInfile_get_status(RIOInfile * self)
    {
        // either OK, or error has already been intercepted
        return RIO_ERROR_OK;
    }

};

#endif

