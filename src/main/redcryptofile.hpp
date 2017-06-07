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
    char const * redcryptofile_version();

    REDEMPTION_LIB_EXPORT
    RedCryptoWriterHandle * redcryptofile_writer_new(
        int with_encryption, int with_checksum, const char * derivator,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn);

    REDEMPTION_LIB_EXPORT
    RedCryptoWriterHandle * redcryptofile_writer_new_with_test_random(
        int with_encryption, int with_checksum, const char * derivator,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn);

    REDEMPTION_LIB_EXPORT
    char const * redcryptofile_writer_error_message(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_writer_open(RedCryptoWriterHandle * handle, char const * path, char const * hashpath, int groupid);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_writer_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len);

    /// \return HashHexArray
    REDEMPTION_LIB_EXPORT
    char const * redcryptofile_writer_qhashhex(RedCryptoWriterHandle * handle);

    /// \return HashHexArray
    REDEMPTION_LIB_EXPORT
    char const * redcryptofile_writer_fhashhex(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_writer_close(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    void redcryptofile_writer_delete(RedCryptoWriterHandle * handle);



    REDEMPTION_LIB_EXPORT
    RedCryptoReaderHandle * redcryptofile_reader_new(const char * derivator, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn, int old_scheme, int one_shot);

    REDEMPTION_LIB_EXPORT
    char const * redcryptofile_reader_error_message(RedCryptoReaderHandle * handle);


    REDEMPTION_LIB_EXPORT
    int redcryptofile_reader_open(RedCryptoReaderHandle * handle, char const * path, char const * derivator);

    // < 0: error, 0: eof, >0: length read
    REDEMPTION_LIB_EXPORT
    int redcryptofile_reader_read(RedCryptoReaderHandle * handle, uint8_t * buffer, unsigned long len);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_reader_close(RedCryptoReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    void redcryptofile_reader_delete(RedCryptoReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    int redcryptofile_reader_hash(RedCryptoReaderHandle * handle, const char * file);

    REDEMPTION_LIB_EXPORT
    const char * redcryptofile_reader_qhashhex(RedCryptoReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    const char * redcryptofile_reader_fhashhex(RedCryptoReaderHandle * handle);
}
