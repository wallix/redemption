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

#if !defined(__TEST_CHECK_SIG_HPP__)
#define __TEST_CHECK_SIG_HPP__

#include "drawable.hpp"

inline bool check_sig(const uint8_t* data, std::size_t height, uint32_t len,
                     char * message, const char * shasig)
{
   uint8_t sig[20];
   SslSha1 sha1;
   for (size_t y = 0; y < static_cast<size_t>(height); y++){
       sha1.update(data + y * len, len);
   }
   sha1.final(sig, 20);

   if (memcmp(shasig, sig, 20)){
       sprintf(message, "Expected signature: \""
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
       sig[ 0], sig[ 1], sig[ 2], sig[ 3],
       sig[ 4], sig[ 5], sig[ 6], sig[ 7],
       sig[ 8], sig[ 9], sig[10], sig[11],
       sig[12], sig[13], sig[14], sig[15],
       sig[16], sig[17], sig[18], sig[19]);
       return false;
   }
   return true;
}

inline bool check_sig(Drawable & data, char * message, const char * shasig)
{
   return check_sig(data.data, data.height, data.rowsize, message, shasig);
}


inline bool check_sig(Stream & stream, char * message, const char * shasig)
{
   uint8_t sig[20];
   SslSha1 sha1;
   sha1.update(stream.get_data(), stream.size());
   sha1.final(sig, sizeof(sig));

   if (memcmp(shasig, sig, 20)){
       sprintf(message, "Expected signature: \""
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
       sig[ 0], sig[ 1], sig[ 2], sig[ 3],
       sig[ 4], sig[ 5], sig[ 6], sig[ 7],
       sig[ 8], sig[ 9], sig[10], sig[11],
       sig[12], sig[13], sig[14], sig[15],
       sig[16], sig[17], sig[18], sig[19]);
       return false;
   }
   return true;
}

inline bool check_sig(const uint8_t * data, size_t length, char * message, const char * shasig)
{
   uint8_t sig[20];
   SslSha1 sha1;
   sha1.update(data, length);
   sha1.final(sig, sizeof(sig));

   if (memcmp(shasig, sig, 20)){
       sprintf(message, "Expected signature: \""
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
       "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
       sig[ 0], sig[ 1], sig[ 2], sig[ 3],
       sig[ 4], sig[ 5], sig[ 6], sig[ 7],
       sig[ 8], sig[ 9], sig[10], sig[11],
       sig[12], sig[13], sig[14], sig[15],
       sig[16], sig[17], sig[18], sig[19]);
       return false;
   }
   return true;
}

inline void get_sig(const uint8_t * data, size_t length, uint8_t * sig, size_t sig_length)
{
   SslSha1 sha1;
   sha1.update(data, length);
   sha1.final(sig, sig_length);
}

inline void get_sig(Stream & stream, uint8_t * sig, size_t sig_length)
{
   SslSha1 sha1;
   sha1.update(stream.get_data(), stream.size());
   sha1.final(sig, sig_length);
}

#endif
