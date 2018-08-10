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
    class RedCryptoMetaReaderHandle;
    class RedCryptoKeyHandle;

    using HashHexArray = char[MD_HASH::DIGEST_LENGTH * 2 + 1];


    REDEMPTION_LIB_EXPORT
    char const * scytale_version();


    // Writer
    //@{
    REDEMPTION_LIB_EXPORT
    RedCryptoWriterHandle * scytale_writer_new(
        int with_encryption, int with_checksum, const char * derivator,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
        int old_scheme, int one_shot);

    REDEMPTION_LIB_EXPORT
    RedCryptoWriterHandle * scytale_writer_new_with_test_random(
        int with_encryption, int with_checksum, const char * derivator,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
        int old_scheme, int one_shot);

    REDEMPTION_LIB_EXPORT
    char const * scytale_writer_error_message(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_writer_open(RedCryptoWriterHandle * handle, char const * path, char const * hashpath, int groupid);

    REDEMPTION_LIB_EXPORT
    int scytale_writer_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len);

    /// \return HashHexArray
    REDEMPTION_LIB_EXPORT
    char const * scytale_writer_qhashhex(RedCryptoWriterHandle * handle);

    /// \return HashHexArray
    REDEMPTION_LIB_EXPORT
    char const * scytale_writer_fhashhex(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_writer_close(RedCryptoWriterHandle * handle);

    REDEMPTION_LIB_EXPORT
    void scytale_writer_delete(RedCryptoWriterHandle * handle);
    //@}


    // Reader
    //@{
    REDEMPTION_LIB_EXPORT
    RedCryptoReaderHandle * scytale_reader_new(const char * derivator, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn, int old_scheme, int one_shot);

    REDEMPTION_LIB_EXPORT
    char const * scytale_reader_error_message(RedCryptoReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_reader_open(RedCryptoReaderHandle * handle, char const * path, char const * derivator);

    // enum class EncryptionSchemeTypeResult
    // {
    //     Error = -1,
    //     NoEncrypted = 0,
    //     OldScheme,
    //     NewScheme,
    // };
    // result is a EncryptionSchemeTypeResult
    REDEMPTION_LIB_EXPORT
    int scytale_reader_open_with_auto_detect_encryption_scheme(
        RedCryptoReaderHandle * handle, char const * path, char const * derivator);

    // < 0: error, 0: eof, >0: length read
    REDEMPTION_LIB_EXPORT
    int scytale_reader_read(RedCryptoReaderHandle * handle, uint8_t * buffer, unsigned long len);

    REDEMPTION_LIB_EXPORT
    int scytale_reader_close(RedCryptoReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    void scytale_reader_delete(RedCryptoReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_reader_fhash(RedCryptoReaderHandle * handle, const char * file);

    REDEMPTION_LIB_EXPORT
    int scytale_reader_qhash(RedCryptoReaderHandle * handle, const char * file);

    /// \return HashHexArray
    REDEMPTION_LIB_EXPORT
    const char * scytale_reader_qhashhex(RedCryptoReaderHandle * handle);

    /// \return HashHexArray
    REDEMPTION_LIB_EXPORT
    const char * scytale_reader_fhashhex(RedCryptoReaderHandle * handle);
    //@}


    // Meta reader
    //@{
    REDEMPTION_LIB_EXPORT
    RedCryptoMetaReaderHandle * scytale_meta_reader_new(RedCryptoReaderHandle * reader);

    REDEMPTION_LIB_EXPORT
    char const * scytale_meta_reader_message(RedCryptoMetaReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_meta_reader_read_hash(RedCryptoMetaReaderHandle * handle, int version, int has_checksum);

    REDEMPTION_LIB_EXPORT
    int scytale_meta_reader_read_header(RedCryptoMetaReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_meta_reader_read_line(RedCryptoMetaReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    int scytale_meta_reader_read_line_eof(RedCryptoMetaReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    void scytale_meta_reader_delete(RedCryptoMetaReaderHandle * handle);

    struct RedCryptoMwrmHeader
    {
        int version;
        int has_checksum;
    };

    struct RedCryptoMwrmLine
    {
        char const * filename;
        uint64_t size;
        uint64_t mode;
        uint64_t uid;
        uint64_t gid;
        uint64_t dev;
        uint64_t ino;
        uint64_t mtime;
        uint64_t ctime;
        uint64_t start_time;
        uint64_t stop_time;
        int with_hash;
        char const * hash1;
        char const * hash2;
    };

    REDEMPTION_LIB_EXPORT
    RedCryptoMwrmHeader * scytale_meta_reader_get_header(RedCryptoMetaReaderHandle * handle);

    REDEMPTION_LIB_EXPORT
    RedCryptoMwrmLine * scytale_meta_reader_get_line(RedCryptoMetaReaderHandle * handle);
    //@}


    // Key
    //@{
    REDEMPTION_LIB_EXPORT
    RedCryptoKeyHandle * scytale_key_new(const char * masterkeyhex);

    REDEMPTION_LIB_EXPORT
    const char * scytale_key_derivate(RedCryptoKeyHandle * handle, const uint8_t * derivator, size_t len);

    REDEMPTION_LIB_EXPORT
    void scytale_key_delete(RedCryptoKeyHandle * handle);
    
    const char * scytale_key_master(RedCryptoKeyHandle * handle);
    const char * scytale_key_derivated(RedCryptoKeyHandle * handle);
    //@}
}
