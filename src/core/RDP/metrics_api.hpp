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
// #include "core/client_info.hpp"
#include "system/linux/system/ssl_sha256.hpp"

// #include "core/RDP/clipboard.hpp"
// #include "core/RDP/channels/rdpdr.hpp"

std::string metrics_encrypt(const char * src, const size_t src_len, const unsigned char * key_crypt);


inline std::string hmac_user(const std::string & user, const unsigned char * key) {
    return metrics_encrypt(user.c_str(), user.length(), key);
}

inline std::string hmac_account(const std::string & account, const unsigned char * key) {
    return metrics_encrypt(account.c_str(), account.length(), key);
}

inline std::string hmac_device_service(const std::string & device, const std::string & service, const unsigned char * key) {
    std::string target_device_and_service(service+" "+device);
    return metrics_encrypt(target_device_and_service.c_str(), target_device_and_service.length(), key);
}

// inline std::string hmac_client_info(const char * client_host, const ClientInfo & info, const unsigned char * key) {
//     char session_info[1024];
//     ::snprintf(session_info, sizeof(session_info), "%s%d%u%u", client_host, info.bpp, info.width, info.height);
//     return metrics_encrypt(session_info, strlen(session_info), key);
// }


inline std::string metrics_encrypt(const char * src, const size_t src_len, const unsigned char * key_crypt) {
    char res[SslSha256::DIGEST_LENGTH*2];
    char * dest = res;
    SslHMAC_Sha256 sha256(key_crypt, 32);
    sha256.update(byte_ptr_cast(src), src_len);
    uint8_t sig[SslSha256::DIGEST_LENGTH];
    sha256.final(sig);

    unsigned char * pin = sig;
    const char * hex = "0123456789ABCDEF";
    for(; pin < &sig[32]; dest+=2, pin++){
        dest[0] = hex[(*pin>>4) & 0xF];
        dest[1] = hex[ *pin     & 0xF];
    }

    return std::string (res, 64);
}


class MetricsApi
{


public:
    const std::string version;
    const std::string protocol_name;

    //  output file info
    const std::chrono::seconds file_interval;
    time_t current_file_date;
    char complete_file_path[4096] = {'\0'};
    const std::string path;
    unique_fd fd = invalid_fd();

    // LOG info
    char header[1024];
    const char * session_id;
    const bool active_ = false;

    const time_t connection_time;

    const std::chrono::seconds log_delay;
    timeval next_log_time;

    const bool debug;




    bool active() {
        return this->active_;
    }

    MetricsApi( const std::string & version                 // fields version
              , const std::string & protocol_name
              , const bool activate                         // do nothing if false
              , std::string path
              , const char * session_id
              , const std::string & primary_user_sig       // clear primary user account
              , const std::string & account_sig            // secondary account
              , const std::string & target_service_sig     // clear target service name + clear device name
              , const std::string & session_info_sig       // source_host + client info
              , const time_t now                           // time at beginning of metrics
              , const std::chrono::hours file_interval     // daily rotation of filename (hours)
              , const std::chrono::seconds log_delay       // delay between 2 logs
              , const bool debug
      )
      : version(version)
      , protocol_name(protocol_name)
      , file_interval{file_interval}
      , current_file_date(now-now%(this->file_interval.count()))
      , path(std::move(path))
      , session_id(session_id)
      , active_(activate)
      , connection_time(now)
      , log_delay(log_delay)
      , next_log_time{ this->log_delay.count()+now, 0}
      , debug(debug)
    {
        if (this->path.c_str() && activate) {
            ::snprintf(header, sizeof(header), "%s user=%s account=%s target_service_device=%s client_info=%s\n", this->session_id, primary_user_sig.c_str(), account_sig.c_str(), target_service_sig.c_str(), session_info_sig.c_str());

            std::string dir_path = this->path.substr(0, this->path.length()-1);
            int mkdir_res = mkdir(dir_path.c_str(), ACCESSPERMS);
            if (mkdir_res == -1) {
                LOG(LOG_ERR, "Impossible to create directory at %s, error(%d)", dir_path, mkdir_res);
            }

            this->new_day(this->current_file_date);
        }
    }


    virtual ~MetricsApi() {
         this->disconnect();
    }

    void disconnect() {

        this->rotate(this->next_log_time.tv_sec);

        std::string text_date = ((this->current_file_date % (24*3600)) == 0)?text_gmdate(this->current_file_date):filename_gmdatetime(this->current_file_date);

        char index_file_path[1024];
        ::snprintf(index_file_path, sizeof(index_file_path), "%s/%s_metrics-%s-%s.logindex", path.c_str(), this->protocol_name.c_str(), this->version.c_str(), text_date.c_str());

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

    void new_day(time_t now) {
        std::string text_date = ((now % (24*3600)) == 0)?text_gmdate(now):filename_gmdatetime(now);

        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path), "%s/%s_metrics-%s-%s.logmetrics", this->path.c_str(), this->protocol_name.c_str(), this->version.c_str(), text_date.c_str());

        this->fd = unique_fd(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!this->fd.is_open()) {
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd.fd(), this->complete_file_path);
        }

        char index_file_path[1024];
        ::snprintf(index_file_path, sizeof(index_file_path), "%s/%s_metrics-%s-%s.logindex", this->path.c_str(), this->protocol_name.c_str(), this->version.c_str(), text_date.c_str());

        char connection_header[1036];
        ::snprintf(connection_header, sizeof(connection_header), "%s connection %s", text_gmdatetime(this->connection_time).c_str(), this->header);

        unique_fd fd_header(index_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );

            if (fd_header.fd() != -1) {
                iovec iov[1] = { {connection_header, strlen(connection_header)} };

                ssize_t nwritten = ::writev(fd_header.fd(), iov, 1);

            if (nwritten == -1) {
                // TODO bad filename
                LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"",this->fd.fd(), index_file_path);
            }
        } else {
            LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"",this->fd.fd(), index_file_path);
        }
    }

    void rotate(time_t now) {

        time_t next_file_date = now - now%(this->file_interval.count());
        if (this->current_file_date != next_file_date) {
            this->current_file_date = next_file_date;

            this->new_day(next_file_date);
        }
    }

    virtual void write_log(std::string & text_datetime, char * sentence, size_t sentence_max_size/*4096*/) = 0;


    void log(timeval & now) {

        if (this->active_ ) {
            timeval wait_log_metrics = ::how_long_to_wait(this->next_log_time, now);
            if (!wait_log_metrics.tv_sec && ! wait_log_metrics.tv_usec) {
                this->next_log_time.tv_sec += this->log_delay.count();
                this->next_log_time.tv_usec = now.tv_usec;

                this->rotate(now.tv_sec);

                std::string text_datetime(text_gmdatetime(now.tv_sec));

                char sentence[4096];

                this->write_log(text_datetime, sentence, sizeof(sentence));

                iovec iov[] = { {sentence, strlen(sentence)} };

                ssize_t nwritten = ::writev(this->fd.fd(), iov, 1);

                if (nwritten == -1) {
                    // TODO bad filename
                    LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"", this->fd.fd(), this->complete_file_path);
                }
            }
        }
    }
};
