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
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#pragma once

#include <cstdio>
#include <ctime>
#include <cstring>
#include <fcntl.h>
#include <sys/uio.h>

#include "utils/sugar/unique_fd.hpp"
#include "utils/log.hpp"
#include "utils/difftimeval.hpp"
#include "utils/texttime.hpp"
#include "system/linux/system/ssl_sha256.hpp"



inline void metrics_hmac_sha256_encrypt(char * dest, const char * src, const size_t src_len, const unsigned char * key_crypt) {
    SslHMAC_Sha256 sha256(key_crypt, 32);
    sha256.update(reinterpret_cast<const uint8_t *>(src), src_len);
    uint8_t sig[SslSha256::DIGEST_LENGTH];
    sha256.final(sig);

    unsigned char * pin = sig;
    const char * hex = "0123456789ABCDEF";
    for(; pin < &sig[32]; dest+=2, pin++){
        dest[0] = hex[(*pin>>4) & 0xF];
        dest[1] = hex[ *pin     & 0xF];
    }
}

class Metrics
{

public:



    const char * version;
    const char * protocol_name;

    //  output file info
    const int file_interval;
    time_t current_file_date;
    char complete_file_path[4096] = {'\0'};
    const char * path;
    unique_fd fd = invalid_fd();

    // LOG info
    char header[1024];
    const char * session_id;
    const bool active_ = false;

    const time_t connection_time;

    const int log_delay;
    timeval next_log_time;


    Metrics( const char * version                 // fields version
            , const char * protocol_name
            , const bool activate                 // do nothing if false
            , const char * path
            , const char * session_id
            , const char * primary_user_sig       // hashed primary user account
            , const char * account_sig            // hashed secondary account
            , const char * target_service_sig     // hashed (target service name + device name)
            , const char * session_info_sig       // hashed (source_host + client info)
            , const time_t now                    // time at beginning of metrics
            , const int file_interval             // daily rotation of filename (hours)
            , const int log_delay                 // delay between 2 logs flush
            )
    : version(version)
    , protocol_name(protocol_name)
    , file_interval{file_interval}
    , current_file_date(now-now%(this->file_interval*3600))
    , path(path)
    , session_id(session_id)
    , active_(activate)
    , connection_time(now)
    , log_delay(log_delay)
    , next_log_time{ this->log_delay+now, 0}
    {
        if (path && activate) {
            ::snprintf(this->header, sizeof(this->header), "%s user=%s account=%s target_service_device=%s client_info=%s\n", session_id, primary_user_sig, account_sig, target_service_sig, session_info_sig);

            this->new_file(this->current_file_date);
        }
    }

    ~Metrics() {
        this->disconnect();
    }

    void disconnect() {

        this->rotate(this->next_log_time.tv_sec);

        std::string text_date = ((this->current_file_date % (24*3600)) == 0) ? text_gmdate(this->current_file_date).c_str() : filename_gmdatetime(this->current_file_date).c_str();

        char index_file_path[1024];
        ::snprintf(index_file_path, sizeof(index_file_path), "%s/%s_metrics-%s-%s.logindex", this->path, this->protocol_name, this->version, text_date.c_str());

        char header_disconnection[2048];
        ::snprintf(header_disconnection, sizeof(header_disconnection), "%s disconnection %s", text_gmdatetime(this->next_log_time.tv_sec).c_str(), this->header);

        unique_fd fd_header(index_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );

        iovec iov[1] = { {header_disconnection, strlen(header_disconnection)} };

        ssize_t nwritten = ::writev(fd_header.fd(), iov, 1);

        if (nwritten == -1) {
            // TODO bad filename
             LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\"", fd_header.fd(), this->complete_file_path);
        }
    }

    void new_file(time_t now) {
        std::string text_date = ((now % (24*3600)) == 0)?text_gmdate(now):filename_gmdatetime(now);

        ::snprintf(this->complete_file_path, 4096, "%s/%s_metrics-%s-%s.logmetrics", this->path, this->protocol_name, this->version, text_date.c_str());

        this->fd = unique_fd(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!this->fd.is_open()) {
             LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd.fd(), this->complete_file_path);
        }

        char index_file_path[1024];
        ::snprintf(index_file_path, sizeof(index_file_path), "%s/%s_metrics-%s-%s.logindex", this->path, this->protocol_name, this->version, text_date.c_str());

        char connection_header[1036];
        ::snprintf(connection_header, sizeof(connection_header), "%s connection %s", text_gmdatetime(this->connection_time).c_str(), this->header);

        unique_fd fd_header(index_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );

            if (fd_header.fd() != -1) {
                iovec iov[1] = { {connection_header, strlen(connection_header)} };

                ssize_t nwritten = ::writev(fd_header.fd(), iov, 1);

            if (nwritten == -1) {
                // TODO bad filename
                 LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\"",this->fd.fd(), index_file_path);
            }
        } else {
             LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"",this->fd.fd(), index_file_path);
        }
    }

    void rotate(time_t now) {

        time_t next_file_date = now - now%(this->file_interval*3600);
        if (this->current_file_date != next_file_date) {
            this->current_file_date = next_file_date;

            this->new_file(next_file_date);
        }
    }
};
