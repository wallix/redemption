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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
*/

#include "sashimi/libssh/libssh.h"

struct error_struct * ssh_new_error()
{
    return new error_struct;   
}

void ssh_free_error(struct error_struct * error)
{
    delete error;
}

/**
 * @brief Retrieve the error text message from the last error.
 *
 * @param  error        An ssh_session or ssh_bind.
 *
 * @return A static string describing the error.
 */
const char * ssh_get_error(error_struct * error) {
  return error->error_buffer;
}

/**
 * @brief Retrieve the error code from the last error.
 *
 * @param  error        An ssh_session or ssh_bind.
 *
 * \return SSH_NO_ERROR       No error occurred\n
 *         SSH_REQUEST_DENIED The last request was denied but situation is
 *                            recoverable\n
 *         SSH_FATAL          A fatal error occurred. This could be an unexpected
 *                            disconnection\n
 *
 *         Other error codes are internal but can be considered same than
 *         SSH_FATAL.
 */
int ssh_get_error_code(error_struct * error) {
  return error->error_code;
}


