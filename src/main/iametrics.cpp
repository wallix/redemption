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

#include "main/version.hpp"
#include "main/iametrics.hpp"
#include "mod/metrics.hpp"
#include <stdint.h>


extern "C"
{
    const char* iametrics_version() noexcept {
        return VERSION;
    }

    char * new_hmac_sha256_hex(const char * src, const int src_len, const unsigned char * key_crypt) noexcept {
        char * dest = (char *)malloc(65);
        metrics_hmac_sha256_encrypt(dest, src, src_len, key_crypt);
        dest[64] = 0;
        return dest;
    }

    void delete_hmac_sha256_hex(char * sign) noexcept {
        free(sign);
    }


    Metrics * metrics_new( const char * version              // fields version
                         , const char * protocol_name
                         , const bool activate               // do nothing if false
                         , size_t     nbitems
                         , const char * path
                         , const char * session_id
                         , const char * primary_user_sig     // clear primary user account
                         , const char * account_sig          // secondary account
                         , const char * target_service_sig   // clear target service name + clear device name
                         , const char * session_info_sig     // info relative to client session
                         , const unsigned long now                         // time at beginning of metrics
                         , const int file_interval                  // daily rotation of filename (hours)
                         , const int log_delay                      // delay between 2 logs
                         ) noexcept {
        Metrics * metrics = new Metrics(version, protocol_name, activate, nbitems, path,
                session_id,
                primary_user_sig, account_sig, target_service_sig, session_info_sig,
                now, file_interval, log_delay);

        return metrics;
    }

    void metrics_delete(Metrics * metrics) noexcept {
        delete(metrics);
    }

    void metrics_log(Metrics * metrics, long int now_ms) noexcept {
        timeval tv_now = { now_ms / 1000 , (now_ms % 1000) * 1000 };
        metrics->log(tv_now);
    }

    void metrics_add_to_current_data(Metrics * metrics, int index, uint64_t value) noexcept {
        metrics->add_to_current_data(index, value);
    }
}
