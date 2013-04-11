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

   new Socket RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RIO_SOCKET_TLS_H_
#define _REDEMPTION_LIBS_RIO_SOCKET_TLS_H_

#include "rio.h"
#include "netutils.hpp"
#include </usr/include/openssl/ssl.h>
#include </usr/include/openssl/err.h>

extern "C" {

    struct RIOSocketTLS {
        SSL * ssl;
    };

    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    static inline RIO_ERROR rio_m_RIOSocketTLS_constructor(RIOSocketTLS * self, SSL * ssl)
    {
        self->ssl = ssl;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    static inline RIO_ERROR rio_m_RIOSocketTLS_destructor(RIOSocketTLS * self)
    {
        return RIO_ERROR_CLOSED;
    }

    static inline size_t rio_m_RIOSocketTLS_recv(RIOSocketTLS * self, void * data, size_t len)
    {
        char * pbuffer = (char*)data;
        size_t remaining_len = len;
        while (remaining_len > 0) {
            ssize_t rcvd = ::SSL_read(self->ssl, pbuffer, remaining_len);
            unsigned long error = SSL_get_error(self->ssl, rcvd);
            switch (error) {
                case SSL_ERROR_NONE:
                    pbuffer += rcvd;
                    remaining_len -= rcvd;
                    break;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "recv_tls WANT READ");
                    continue;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "recv_tls WANT WRITE");
                    continue;

                case SSL_ERROR_WANT_CONNECT:
                    LOG(LOG_INFO, "recv_tls WANT CONNECT");
                    continue;

                case SSL_ERROR_WANT_ACCEPT:
                    LOG(LOG_INFO, "recv_tls WANT ACCEPT");
                    continue;

                case SSL_ERROR_WANT_X509_LOOKUP:
                    LOG(LOG_INFO, "recv_tls WANT X509 LOOKUP");
                    continue;

                case SSL_ERROR_ZERO_RETURN:
                    LOG(LOG_WARNING, "TLS receive for %u bytes, ZERO RETURN got %u", 
                        (unsigned)len, (unsigned)(remaining_len - len));
                    return remaining_len - len;
                default:
                {
                    uint32_t errcount = 0;
                    errcount++;
                    LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    while ((error = ERR_get_error()) != 0){
                        errcount++;
                        LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    }
                    TODO("if recv fail with partial read we should return the amount of data received, "
                         "close socket and store some delayed error value that will be sent back next call")
                    TODO("replace this with actual error management, EOF is not even an option for sockets")
                    TODO("Manage actual errors, check possible values")
                    rio_m_RIOSocketTLS_destructor(self);
                    return (RIO_ERROR)-RIO_ERROR_ANY;
                }
                break;
            }
        }
        return len;
    }

    static inline ssize_t rio_m_RIOSocketTLS_send(RIOSocketTLS * self, const void * data, size_t len)
    {
        LOG(LOG_INFO, "TLS send %u bytes", (unsigned)len);

        const char * const buffer = (const char * const)data;
        size_t remaining_len = len;
        size_t offset = 0;
        while (remaining_len > 0){
            int ret = SSL_write(self->ssl, buffer + offset, remaining_len);

            unsigned long error = SSL_get_error(self->ssl, ret);
            switch (error)
            {
                case SSL_ERROR_NONE:
                    LOG(LOG_INFO, "TLS send SSL_ERROR_NONE %u bytes", (unsigned)ret);
                    remaining_len -= ret;
                    offset += ret;
                    break;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "send_tls WANT READ");
                    continue;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "send_tls WANT WRITE");
                    continue;

                default:
                {
                    LOG(LOG_INFO, "Failure in SSL library");
                    uint32_t errcount = 0;
                    errcount++;
                    LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    while ((error = ERR_get_error()) != 0){
                        errcount++;
                        LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    }
                    rio_m_RIOSocketTLS_destructor(self);
                    return (RIO_ERROR)-RIO_ERROR_ANY;
                }
            }
        }
        LOG(LOG_INFO, "return from TLS send %u bytes (remaining_len=%u)", (unsigned)len, (unsigned)remaining_len);
        return len;
    }

    
    static inline RIO_ERROR rio_m_RIOSocketTLS_get_status(RIOSocketTLS * self)
    {
        TODO("when we will keep error value needed for recv we should return the stored error status")
        return RIO_ERROR_OK;
    }

};

#endif

