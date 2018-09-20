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

#pragma once

#include "cxx/cxx.hpp"
#include <cstdint> // for uint64_t


extern "C"
{
    REDEMPTION_LIB_EXPORT
    char const * iametrics_version() noexcept;

    class Metrics;

    REDEMPTION_LIB_EXPORT
    Metrics * metrics_new( const char * version             // fields version
                         , const char * protocol_name
                         , unsigned     nbitems
                         , const char * path
                         , const char * session_id
                         , const char * primary_user_sig    // clear primary user account
                         , const char * account_sig         // secondary account
                         , const char * target_service_sig  // clear target service name + clear device name
                         , const char * session_info_sig    // info relative to client session
                         , unsigned long now_seconds        // time at beginning of metrics
                         , int           file_interval_hours     // daily rotation of filename (hours)
                         , int           log_delay_seconds  // delay between 2 logs
                         ) noexcept;

    REDEMPTION_LIB_EXPORT
    void metrics_delete(Metrics * metrics) noexcept;

    REDEMPTION_LIB_EXPORT
    void metrics_log(Metrics * metrics, uint64_t now_ms) noexcept;

    REDEMPTION_LIB_EXPORT
    int metrics_add_to_current_data(Metrics * metrics, unsigned index, uint64_t value) noexcept;
}
