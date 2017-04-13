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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonathan Poelen

*/

#include "cxx/cxx.hpp"
#include "capture/cryptofile.hpp"

extern "C"
{
    class RedCryptoHandle;

    REDEMPTION_LIB_EXPORT
    RedCryptoHandle * redcryptofile_open_writer(int with_encryption, int with_checksum, char const * path,
        get_hmac_key_prototype * hmac_fn,
        get_trace_key_prototype * trace_fn);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_open_reader(char * path,
        get_hmac_key_prototype * hmac_fn,
        get_trace_key_prototype * trace_fn);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_write(RedCryptoHandle * handle, uint8_t const * buffer, size_t len);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_read(RedCryptoHandle * handle, uint8_t * buffer, size_t len);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_read_qhash(RedCryptoHandle * handle, char * qhashhex);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_read_fhash(RedCryptoHandle * handle, char * qhashhex);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_close_writer(RedCryptoHandle * handle, char * qhashhex, char * fhashhex);
}
