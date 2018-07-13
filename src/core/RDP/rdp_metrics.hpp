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

#include <cstdio>
#include <cstring>

#include "utils/difftimeval.hpp"
#include <fcntl.h>
#include <sys/uio.h>


struct RDPMetrics {


    time_t last_date;
    char complete_file_path[4096] = {'\0'};

    const char * path_template;
    const uint32_t session_id;
    const char * account;
    const char * target_host;
    const char * primary_user;

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
              , const char * target_host
              , const char * primary_user)
      : path_template(path_template)
      , session_id(session_id)
      , account(account)
      , target_host(target_host)
      , primary_user(primary_user)
    {
        if (this->path_template) {
            this->new_day();
        }
    }

    ~RDPMetrics() {
        fcntl(this->fd, F_SETFD, FD_CLOEXEC);
    }

    void set_current_formated_date(char * date, bool keep_hhmmss) {
        timeval now = tvtime();
        this->last_date = now.tv_sec;
        char current_date[24] = {'\0'};
        memcpy(current_date, ctime(&(this->last_date)), 24);

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

        }
    }

//     void set_current_formated_time() {
//         timeval now = tvtime();
//         time_t time_now = now.tv_sec;
//
//         char current_date[24] = {'\0'};
//         memcpy(current_date, ctime(&time_now), 24);
//
//
//     }

//     std::string get_current_formated_date(char * dest, bool keep_hhmmss) {
//         timeval now = tvtime();
//         time_t time_now = now.tv_sec;
//
//         char current_date[24] = {'\0'};
//         memcpy(current_date, ctime(&time_now), 24);
//
// //         std::string current_date(ctime(&time_now));
// //         LOG(LOG_INFO, "current_date=%s", current_date);
//         std::string mmm(current_date.substr(4, 3));
//         std::string dd(current_date.substr(8, 2));
//         std::string yyyy(current_date.substr(20, 4));
//         std::string hhmmss;
//         if (keep_hhmmss) {
//             hhmmss +=  "-"+current_date.substr(11, 8);
//         }
//
//         return "-"+yyyy+"-"+mmm+"-"+dd+hhmmss;
//     }

    void new_day() {
        char last_date_formated[20] = {'\0'};
        this->set_current_formated_date(last_date_formated);
        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path), "%s%s.log", this->path_template, last_date_formated);

        this->fd = ::open(complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
        if (this->fd == -1) {
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd, this->complete_file_path);
        }
    }

    void log() {

        timeval now = tvtime();
        time_t time_date = now.tv_sec;

        if ((time_date -this->last_date) >= 3600*24) {
            fcntl(this->fd, F_SETFD, FD_CLOEXEC);
            this->new_day();
        }

        char sentence[4096];
        ::snprintf(sentence, sizeof(sentence), "Session_id=%u user=\"%s\" account=\"%s\" target_host=\"%s\""
          " right_click_sent=%d left_click_sent=%d keys_sent=%d"
          " Client data received by channels - main=%ld cliprdr=%ld rail=%ld rdpdr=%ld drdynvc=%ld"
          " Server data received by channels - main=%ld cliprdr=%ld rail=%ld rdpdr=%ld drdynvc=%ld",
            this->session_id, this->primary_user, this->account, this->target_host,
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

            struct iovec iov[1];
            iov[0].iov_base = sentence;
            iov[0].iov_len = std::strlen(sentence);

            ssize_t nwritten = ::writev(fd, iov, 1);

            if (nwritten == -1) {
                std::string file_path_template(this->path_template);
                file_path_template += this->last_date;
                file_path_template += ".log";
                LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\"",this->fd, file_path_template);
                return;
            }
        }
    }
};
