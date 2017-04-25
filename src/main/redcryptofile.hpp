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
    class RedCryptoWriterHandle;
    class RedCryptoReaderHandle;

    using HashHexArray = char[MD_HASH::DIGEST_LENGTH * 2 + 1];

    REDEMPTION_LIB_EXPORT
    RedCryptoWriterHandle * redcryptofile_new_writer(
        int with_encryption, int with_checksum, 
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_open_writer(RedCryptoWriterHandle * handle, char const * path);

    REDEMPTION_LIB_EXPORT
    RedCryptoReaderHandle * redcryptofile_open_reader(
        char const * path,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len);

    // < 0: error, 0: eof, >0: length read
    REDEMPTION_LIB_EXPORT
    int redcryptofile_read(RedCryptoReaderHandle * handle, uint8_t * buffer, unsigned long len);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_close_writer(RedCryptoWriterHandle * handle);
    
    REDEMPTION_LIB_EXPORT
    const char * redcryptofile_qhashhex_writer(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    const char * redcryptofile_fhashhex_writer(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_close_reader(RedCryptoReaderHandle * handle);
}
