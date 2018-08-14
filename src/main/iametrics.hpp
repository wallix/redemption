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
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean, Clement Moroldo

*/

#include "cxx/cxx.hpp"
#include <stdint.h> // for uint64_t
#include <stddef.h> // for size_t


extern "C"
{
    REDEMPTION_LIB_EXPORT
    char const * iametrics_version();

    REDEMPTION_LIB_EXPORT
    char * new_hmac_sha256_hex(const char * src, const int src_len, const unsigned char * key_crypt) noexcept;

    REDEMPTION_LIB_EXPORT
    void delete_hmac_sha256_hex(char * sign) noexcept;

    class Metrics;

    REDEMPTION_LIB_EXPORT
    Metrics * metrics_new( const char * version             // fields version
                         , const char * protocol_name
                         , const bool activate              // do nothing if false
                         , size_t     nbitems
                         , const char * path
                         , const char * session_id
                         , const char * primary_user_sig    // clear primary user account
                         , const char * account_sig         // secondary account
                         , const char * target_service_sig  // clear target service name + clear device name
                         , const char * session_info_sig    // info relative to client session
                         , const unsigned long now                 // time at beginning of metrics
                         , const int file_interval          // daily rotation of filename (hours)
                         , const int log_delay              // delay between 2 logs
                         );

    REDEMPTION_LIB_EXPORT
    void metrics_delete(Metrics * metrics);

    REDEMPTION_LIB_EXPORT
    void metrics_log(long int now_ms, Metrics * metrics) noexcept;

    REDEMPTION_LIB_EXPORT
    void metrics_add_to_current_data(int index, uint64_t value, Metrics * metrics) noexcept;

}
