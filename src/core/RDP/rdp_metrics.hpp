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
*   Author(s): Clément Moroldo
*/

#pragma once

#include "utils/log.hpp"
#include "utils/difftimeval.hpp"
#include "core/client_info.hpp"
#include "system/linux/system/ssl_sha1.hpp"

#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <fcntl.h>


struct RDPMetrics {


    time_t last_date;
    char complete_file_path[4096] = {'\0'};

    time_t stat_time;

    const char * path_template;
    const uint32_t session_id;
    const char * account;
    const char * primary_user;
    const char * target_host;
    ClientInfo info;
    const uint32_t sccore_version;


    int fd = -1;

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

    int total_right_clicks = 0;
    int total_left_clicks = 0;
    int total_keys_pressed = 0;



    RDPMetrics( const char * path_template
              , const uint32_t session_id
              , const char * account
              , const ClientInfo & info
              , const char * target_host
              , const char * primary_user
              , const uint32_t sccore_version)
      : path_template(path_template)
      , session_id(session_id)
      , account(account)
      , primary_user(primary_user)
      , target_host(target_host)
      , info(info)
      , sccore_version(sccore_version)
    {
        timeval now = tvtime();
        this->stat_time = now.tv_sec;

        if (this->path_template) {
            timeval now = tvtime();
            this->last_date = now.tv_sec;
            this->new_day();
        }
    }

    ~RDPMetrics() {
        fcntl(this->fd, F_SETFD, FD_CLOEXEC);
    }


    void sha1_encrypt(char * dest, const char * src, const size_t src_len) {
        SslSha1 sha1;
        sha1.update(reinterpret_cast<const uint8_t*>(src), src_len);
        uint8_t sig[SslSha1::DIGEST_LENGTH];
        sha1.final(sig);
        snprintf(dest, SslSha1::DIGEST_LENGTH,
                 "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                 sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7], sig[8], sig[9],
                 sig[10], sig[11], sig[12], sig[13], sig[14], sig[15], sig[16], sig[17], sig[18], sig[19]);
    }

    void set_current_formated_date(char * date, bool keep_hhmmss, time_t time) {
        char current_date[24] = {'\0'};
        memcpy(current_date, ctime(&time), 24);

        date[0] = '-';
        date[1] = current_date[20];
        date[2] = current_date[21];
        date[3] = current_date[22];
        date[4] = current_date[23];
        date[5] = '-';
        date[6] =  current_date[4];
        date[7] =  current_date[5];
        date[8] =  current_date[6];
        date[9] = '-';
        date[10] = current_date[8];
        date[11] = current_date[9];

        if (keep_hhmmss) {
            date[12] = current_date[11];
            date[13] = current_date[12];
            date[14] = current_date[13];
            date[15] = current_date[14];
            date[16] = current_date[15];
            date[17] = current_date[16];
            date[18] = current_date[17];
            date[19] = current_date[18];
        }
    }


    void new_day() {
        char last_date_formated[20] = {'\0'};
        this->set_current_formated_date(last_date_formated, false, this->last_date);
        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path), "%s%s.log", this->path_template, last_date_formated);

        this->fd = ::open(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
        if (this->fd == -1) {
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd, this->complete_file_path);
        }
    }


    void log() {

        timeval now = tvtime();
        time_t time_date = now.tv_sec;

        if ((time_date -this->last_date) >= 3600*24) {
            fcntl(this->fd, F_SETFD, FD_CLOEXEC);
            this->last_date = time_date;
            this->new_day();
        }

        char primary_user_sig[SslSha1::DIGEST_LENGTH];
        this->sha1_encrypt(primary_user_sig, this->primary_user, sizeof(this->primary_user));

        char account_sig[SslSha1::DIGEST_LENGTH];
        this->sha1_encrypt(primary_user_sig, this->account, sizeof(this->account));


        char sentence[4096];
        int len = ::snprintf(sentence, sizeof(sentence), "Session_id=%u user=\"%s\" account=\"%s\" target_host=\"%s\""
          " right_click_sent=%d left_click_sent=%d keys_sent=%d"
          " Client data received by channels - main=%ld cliprdr=%ld rail=%ld rdpdr=%ld drdynvc=%ld"
          " Server data received by channels - main=%ld cliprdr=%ld rail=%ld rdpdr=%ld drdynvc=%ld",


            this->session_id, primary_user_sig, account_sig, this->info.hostname,
            this->total_right_clicks, this->total_left_clicks, this->total_keys_pressed,

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

        if (this->fd == -1) {
            LOG(LOG_INFO, "sentence=%s", sentence);
        } else {
            ssize_t nwritten = ::write(this->fd, sentence, len);

            if (nwritten == -1) {
                LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s%ld.log\"",
                    this->fd, this->path_template, this->last_date);
                return;
            }
        }
    }
};
