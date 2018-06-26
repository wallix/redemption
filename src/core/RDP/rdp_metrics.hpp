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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/uio.h>

#include "utils/log.hpp"

struct RDPMetrics {

    const char * filename;
    const uint32_t session_id;
    const char * account;
    const char * target_host;
    const char * primary_user;

    long int total_main_amount_data_rcv_from_client = 0;
    long int total_cliprdr_amount_data_rcv_from_client = 0;
    long int total_rail_amount_data_rcv_from_client = 0;
    long int total_rdpdr_amount_data_rcv_from_client = 0;
    long int total_drdynvc_amount_data_rcv_from_client = 0;

    long int total_main_amount_data_rcv_from_server = 0;
    long int total_cliprdr_amount_data_rcv_from_server = 0;
    long int total_rail_amount_data_rcv_from_server = 0;
    long int total_rdpdr_amount_data_rcv_from_server = 0;
    long int total_drdynvc_amount_data_rcv_from_server = 0;



    RDPMetrics( const char * filename
              , const uint32_t session_id
              , const char * account
              , const char * target_host
              , const char * primary_user)
      : filename(filename)
      , session_id(session_id)
      , account(account)
      , target_host(target_host)
      , primary_user(primary_user)
      {}

    void log() {

        char sentence[4096];
          ::snprintf(sentence, sizeof(sentence), "Session_id=%u user=\"%s\" account=\"%s\" target_host=\"%s\""
        "\n   Client data received by channels - main:%ld cliprdr:%ld rail:%ld rdpdr:%ld drdynvc:%ld"
        "\n   Server data received by channels - main:%ld cliprdr:%ld rail:%ld rdpdr:%ld drdynvc:%ld",
            this->session_id, this->primary_user, this->account, this->target_host,
            this->total_main_amount_data_rcv_from_client,
            this->total_cliprdr_amount_data_rcv_from_client,
            this->total_rail_amount_data_rcv_from_client,
            this->total_rdpdr_amount_data_rcv_from_client,
            this->total_drdynvc_amount_data_rcv_from_client,

            this->total_main_amount_data_rcv_from_server,
            this->total_cliprdr_amount_data_rcv_from_server,
            this->total_rail_amount_data_rcv_from_server,
            this->total_rdpdr_amount_data_rcv_from_server,
            this->total_drdynvc_amount_data_rcv_from_server
        );

        if (this->filename) {
            int fd = ::open(this->filename, O_WRONLY);

            if (fd == -1) {
                LOG(LOG_ERR, "Log Metrics error: can't open \"%s\"", this->filename);
                return;
            }

            struct iovec iov[1];
            iov[0].iov_base = sentence;
            iov[0].iov_len = std::strlen(sentence);

            ssize_t nwritten = ::writev(fd, iov, 1);

            if (nwritten == -1) {
                LOG(LOG_ERR, "Log Metrics error: can't write \"%s\"", this->filename);
                return;
            }

        } else {
            LOG(LOG_INFO, "%s", sentence);
        }
    }
};
