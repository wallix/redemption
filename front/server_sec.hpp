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

   header file, secure layer at core module, in charge of
   encryption / decryption methods

*/

#if !defined(__SERVER_SEC_HPP__)
#define __SERVER_SEC_HPP__

#include "RDP/x224.hpp"
#include "RDP/sec.hpp"
#include "ssl_calls.hpp"
#include "client_info.hpp"
#include "rsa_keys.hpp"
#include "constants.hpp"


#include <assert.h>
#include <stdint.h>

#include <iostream>
using namespace std;

/* sec */
struct server_sec : public Sec {

    /*****************************************************************************/

    server_sec(int crypt_level) : Sec(crypt_level)
    {
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);
        memset(this->client_random, 0, 64);
        memset(this->client_crypt_random, 0, 72);

        memset(this->sign_key, 0, 16);
        memset(this->pub_exp, 0, 4);
        memset(this->pub_mod, 0, 64);
        memset(this->pub_sig, 0, 64);
        memset(this->pri_exp, 0, 64);
    }


};


#endif
