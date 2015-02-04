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

   Main entry point file for RIO *Transport library
*/

#ifndef WABCRYPTOFILE_CCRYPTOFILE_H
#define WABCRYPTOFILE_CCRYPTOFILE_H

#include "cryptofile.h"

#ifdef __cplusplus
extern "C" {
#endif

struct crypto_file;

struct crypto_file * crypto_open_read(int systemfd, unsigned char * trace_key,  struct CryptoContext * cctx);
struct crypto_file * crypto_open_write(int systemfd, unsigned char * trace_key, struct CryptoContext * cctx, const unsigned char * iv);
int crypto_flush(struct crypto_file * cf);
int crypto_read(struct crypto_file * cf, char * buf, unsigned int buf_size);
int crypto_write(struct crypto_file *cf, const char * buf, unsigned int size);
int crypto_close(struct crypto_file *cf, unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key);

#ifdef __cplusplus
}
#endif

#endif
