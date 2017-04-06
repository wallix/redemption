/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#pragma once

#include <cstdio>
#include <cstring>

#include "system/ssl_sha1.hpp"


#define CHECK_SIG(obj, sig)                                          \
    do {                                                             \
        char message[1024];                                          \
        if (!redemption_unit_test__::check_sig(obj, message, sig)) { \
            BOOST_CHECK_MESSAGE(false, message);                     \
        }                                                            \
    } while (0)

#define CHECK_SIG2(buf, sz, sig)                                            \
    do {                                                                    \
        char message[1024];                                                 \
        if (!redemption_unit_test__::check_sig(buf, 1, sz, message, sig)) { \
            BOOST_CHECK_MESSAGE(false, message);                            \
        }                                                                   \
    } while (0)


inline void get_sig(const uint8_t * data, size_t length, uint8_t * sig)
{
   SslSha1 sha1;
   sha1.update(data, length);
   sha1.final(sig);
}

template<class Stream>
inline void get_sig(Stream const & stream, uint8_t * sig)
{
   SslSha1 sha1;
   sha1.update(stream.get_data(), stream.get_offset());
   sha1.final(sig);
}


namespace redemption_unit_test__
{
    bool check_sig(const uint8_t* data, std::size_t height, uint32_t len, char * message, const void * shasig);

    template<class Drawable>
    auto check_sig(Drawable const & data, char * message, const void * shasig)
    -> decltype(redemption_unit_test__::check_sig(data.data(), data.height(), data.rowsize(), message, shasig))
    {  return redemption_unit_test__::check_sig(data.data(), data.height(), data.rowsize(), message, shasig); }

    template<class Bitmap>
    auto check_sig(Bitmap const & data, char * message, const void * shasig)
    -> decltype(redemption_unit_test__::check_sig(data.data(), data.cy(), data.line_size(), message, shasig))
    {  return redemption_unit_test__::check_sig(data.data(), data.cy(), data.line_size(), message, shasig); }

    template<class Stream>
    auto check_sig(Stream const & stream, char * message, const void * shasig)
    -> decltype(redemption_unit_test__::check_sig(stream.get_data(), 1, stream.get_offset(), message, shasig))
    {  return redemption_unit_test__::check_sig(stream.get_data(), 1, stream.get_offset(), message, shasig); }


    inline
    bool check_sig(const uint8_t* data, std::size_t height, uint32_t len, char * message, const void * shasig)
    {
        uint8_t sig[SslSha1::DIGEST_LENGTH];
        SslSha1 sha1;
        for (size_t y = 0; y < height; y++){
            sha1.update(data + y * len, len);
        }
        sha1.final(sig);

        if (memcmp(shasig, sig, sizeof(sig))){
            sprintf(message, "Expected signature: \""
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
            unsigned(sig[ 0]), unsigned(sig[ 1]), unsigned(sig[ 2]), unsigned(sig[ 3]),
            unsigned(sig[ 4]), unsigned(sig[ 5]), unsigned(sig[ 6]), unsigned(sig[ 7]),
            unsigned(sig[ 8]), unsigned(sig[ 9]), unsigned(sig[10]), unsigned(sig[11]),
            unsigned(sig[12]), unsigned(sig[13]), unsigned(sig[14]), unsigned(sig[15]),
            unsigned(sig[16]), unsigned(sig[17]), unsigned(sig[18]), unsigned(sig[19]));
            return false;
        }
        return true;
    }
}
