/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   New Outfilename RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_CRYPTOOUTFILENAME_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_CRYPTOOUTFILENAME_H_

#include "rio.h"

extern "C" {
    /***********************
    * RIOCryptoOutfilename *
    ***********************/

    struct RIOCryptoOutfilename {
        RIO *trans;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOCryptoOutfilename_constructor(RIOCryptoOutfilename * self, const char * filename, const int groupid)
    {
        RIO_ERROR error = RIO_ERROR_OK;
        self->trans = rio_new_crypto(&error, filename, O_WRONLY|O_CREAT);
        if (error != RIO_ERROR_OK){
            LOG(LOG_ERR, "Failed to create encrypted trace file %s %u : %s [%u]", filename, strerror(errno), errno);
        }
        else {
            if (groupid){
                if (chown(filename, (uid_t)-1, groupid) == -1){
                    LOG(LOG_ERR, "can't set file %s group to %u : %s [%u]", filename, groupid, strerror(errno), errno);
                }
                if (chmod(filename, S_IRUSR|S_IRGRP) == -1){
                    LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", filename, strerror(errno), errno);
                }
            }
        }
        return error;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOCryptoOutfilename_destructor(RIOCryptoOutfilename * self)
    {
        rio_delete(self->trans);
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOCryptoOutfilename_sign(RIOCryptoOutfilename * self, unsigned char * buf, size_t size, size_t & len) {
        memset(buf, 0, size);
        len = 0;
        return RIO_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOCryptoOutfilename_recv(RIOCryptoOutfilename * self, void * data, size_t len)
    {
         rio_m_RIOCryptoOutfilename_destructor(self);
         return -RIO_ERROR_SEND_ONLY;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOCryptoOutfilename_send(RIOCryptoOutfilename * self, const void * data, size_t len)
    {
        return rio_send(self->trans, data, len);
    }

    static inline RIO_ERROR rio_m_RIOCryptoOutfilename_get_status(RIOCryptoOutfilename * self)
    {
        return RIO_ERROR_OK;
    }
};

#endif

