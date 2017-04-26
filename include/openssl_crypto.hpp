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

   This is used to easily hide actual path to openssl header files
   Using system wide headers can be a problem if we want to control
   the exact ssl version used.

*/


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "openssl/rc4.h"
#include "openssl/md4.h"
#include "openssl/md5.h"
#include "openssl/sha.h"
#include "openssl/hmac.h"
#include "openssl/bn.h"
#include "openssl/rsa.h"
#include "openssl/x509v3.h"
#include "openssl/evp.h"
#include "openssl/aes.h"

// For sashimi
// #include "openssl/ec.h"
// #include "openssl/ecdsa.h"
#include "openssl/blowfish.h"
#include "openssl/rand.h"
#include "openssl/des.h"
//#include <openssl/des_old.h>

// We remove this macro from des_old because it conflicts with method set_key of some objects
#undef set_key

#ifdef __cplusplus
}
#endif
