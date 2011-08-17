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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rdp secure layer header

*/

#if !defined(__RDP_SEC_HPP__)
#define __RDP_SEC_HPP__

#include "RDP/x224.hpp"
#include "RDP/sec.hpp"

#include "constants.hpp"
#include "ssl_calls.hpp"

#include <stdint.h>

/* sec */
struct rdp_sec : public Sec {

    rdp_sec(const char * hostname, const char * username) : Sec(0)
    {
        #warning and if hostname is really larger, what happens ? We should at least emit a warning log
        strncpy(this->hostname, hostname, 15);
        this->hostname[15] = 0;
        #warning and if username is really larger, what happens ? We should at least emit a warning log
        strncpy(this->username, username, 127);
        this->username[127] = 0;
        memset(this->client_crypt_random, 0, 512);
        memset(this->sign_key, 0, 16);
        this->server_public_key_len = 0;
    }

};

#endif
