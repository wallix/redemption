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

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/uio.h>

#include "utils/log.hpp"
#include "utils/difftimeval.hpp"
#include "core/client_info.hpp"
#include "system/linux/system/ssl_sha1.hpp"

#include "core/RDP/clipboard.hpp"




struct RDPMetrics {

    enum : int {
        total_main_amount_data_rcv_from_client,
        total_right_clicks,
        total_left_clicks,
        total_keys_pressed,
        total_mouse_move,
        total_main_amount_data_rcv_from_server,

        total_cliprdr_amount_data_rcv_from_server,
        nb_text_paste_from_serveur,
        nb_image_paste_from_serveur,
        nb_file_paste_from_serveur,
        nb_text_copy_from_serveur,
        nb_image_copy_from_serveur,
        nb_file_copy_from_serveu,
        total_cliprdr_amount_data_rcv_from_client,
        nb_text_paste_from_client,
        nb_image_paste_from_client,
        nb_file_paste_from_client,
        nb_text_copy_from_client,
        nb_image_copy_from_client,
        nb_file_copy_from_client,

        total_rdpdr_amount_data_rcv_from_client,
        total_rdpdr_amount_data_rcv_from_server,
        nb_more_1k_byte_read_file,
        nb_deleted_file_or_folder,
        nb_write_file,
        nb_rename_file,
        nb_open_folder,

        total_rail_amount_data_rcv_from_client,
        total_rail_amount_data_rcv_from_server,

        total_other_amount_data_rcv_from_client,
        total_other_amount_data_rcv_from_server
    };

    const char * rdp_metrics_name(int index) {
        switch (index) {
            case total_main_amount_data_rcv_from_client: return " main_channel_data_from_client";
            case total_right_clicks: return " right_click_sent=";
            case total_left_clicks: return " left_click_sent=";
            case total_keys_pressed: return " keys_sent=";
            case total_mouse_move: return " mouse_move=";
            case total_main_amount_data_rcv_from_server: return " main_channel_data_from_serveur=";
            case total_cliprdr_amount_data_rcv_from_server: return " cliprdr_channel_data_from_server=";
            case nb_text_paste_from_serveur: return " nb_text_paste_server=";
            case nb_image_paste_from_serveur: return " nb_image_paste_server=";
            case nb_file_paste_from_serveur: return " nb_file_paste_server=";
            case nb_text_copy_from_serveur: return " nb_text_copy_server=";
            case nb_image_copy_from_serveur: return " nb_image_copy_server=";
            case nb_file_copy_from_serveu: return " nb_file_copy_server=";
            case total_cliprdr_amount_data_rcv_from_client: return " cliprdr_channel_data_from_client=";
            case nb_text_paste_from_client: return " nb_text_paste_client=";
            case nb_image_paste_from_client: return " nb_image_paste_client=";
            case nb_file_paste_from_client: return " nb_file_paste_client=";
            case nb_text_copy_from_client: return " nb_text_copy_client=";
            case nb_image_copy_from_client: return " nb_image_copy_client=";
            case nb_file_copy_from_client: return " nb_file_copy_client=";
            case total_rdpdr_amount_data_rcv_from_client: return " rdpdr_channel_data_from_client=";
            case total_rdpdr_amount_data_rcv_from_server: return " rdpdr_channel_data_from_server=";
            case nb_more_1k_byte_read_file: return " nb_more_1k_byte_read_file=";
            case nb_deleted_file_or_folder: return " nb_deleted_file_or_folder=";
            case nb_write_file: return " nb_write_file=";
            case nb_rename_file: return " nb_rename_file=";
            case nb_open_folder: return " nb_open_folder=";
            case total_rail_amount_data_rcv_from_client: return " rail_channel_data_from_client=";
            case total_rail_amount_data_rcv_from_server: return " rail_channel_data_from_server=";
            case total_other_amount_data_rcv_from_client: return " other_channel_data_from_client=";
            case total_other_amount_data_rcv_from_server: return " other_channel_data_from_server=";
        }
    }

    void server_other_channel_data(long int len) {
        this->current_data[total_other_amount_data_rcv_from_server] += len;
    }

    void client_other_channel_data(long int len) {
        this->current_data[total_other_amount_data_rcv_from_client] += len;
    }

    void server_rail_channel_data(long int len) {
        this->current_data[total_rail_amount_data_rcv_from_server] += len;
    }

    void client_rail_channel_data(long int len) {
        this->current_data[total_rail_amount_data_rcv_from_client] += len;
    }

    void set_server_rdpdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        this->current_data[total_rdpdr_amount_data_rcv_from_server] += length;
    }

    void set_client_rdpdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        this->current_data[total_rdpdr_amount_data_rcv_from_client] += length;
    }

    void set_server_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        this->current_data[total_cliprdr_amount_data_rcv_from_server] += length;
        RDPECLIP::CliprdrHeader header;
        header.recv(chunk);

        switch (header.msgType()) {

        }
    }

    void set_client_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        this->current_data[total_cliprdr_amount_data_rcv_from_client] += length;
    }

    void server_main_channel_data(long int len) {
        this->current_data[total_main_amount_data_rcv_from_server] += len;
    }

    void mouse_mouve(int xy) {
        this->current_data[total_mouse_move] += xy;
    }

    void key_pressed() {
        this->current_data[total_keys_pressed]++;
    }

    void right_click() {
        this->current_data[total_right_clicks]++;
    }

    void left_click() {
        this->current_data[total_left_clicks]++;
    }

    void client_main_channel_data(long int len) {
        this->current_data[total_main_amount_data_rcv_from_client] += len;
    }





    time_t last_date;
    char complete_file_path[4096] = {'\0'};
    time_t start_time;
    const char * path_template;
    int fd = -1;


    // Header
    const uint32_t session_id;
    char primary_user_sig[SslSha1::DIGEST_LENGTH*2];
    char account_sig[SslSha1::DIGEST_LENGTH*2];
    char hostname_sig[SslSha1::DIGEST_LENGTH*2];
    char target_service_sig[SslSha1::DIGEST_LENGTH*2];
    char session_info_sig[SslSha1::DIGEST_LENGTH*2];
    char start_full_date_time[24];

    char header_part1[64];
    char header_part2[1024];

    long int current_data[32] = { 0 };
    long int previous_data[32] = { 0 };




    RDPMetrics( const char * path_template
              , const uint32_t session_id
              , const char * account
              , const char * primary_user
              , const char * target_host
              , const ClientInfo & info
              , const uint32_t sccore_version
              , const char * target_service)
      : path_template(path_template)
      , session_id(session_id)
    {
        timeval now = tvtime();
        this->start_time = now.tv_sec;

        if (this->path_template) {
            timeval now = tvtime();
            this->last_date = now.tv_sec;
            this->new_day();
        }

        this->set_current_formated_date(this->start_full_date_time, true, this->start_time);
        this->sha1_encrypt(this->primary_user_sig, primary_user, std::strlen(primary_user));
        this->sha1_encrypt(this->account_sig, account, std::strlen(account));
        this->sha1_encrypt(this->hostname_sig, info.hostname, std::strlen(info.hostname));
        this->sha1_encrypt(this->target_service_sig, target_service, std::strlen(target_service));

        char session_info[64];
        ::snprintf(session_info, sizeof(session_info), "%u%s%d%u%u", sccore_version, target_host, info.bpp, info.width, info.height);
        this->sha1_encrypt(this->session_info_sig, session_info, std::strlen(session_info));

//          char header[1024];
        ::snprintf(this->header_part1, sizeof(this->header_part1), "Session_starting_time=%s delta_time(s)=",
        this->start_full_date_time);

        ::snprintf(this->header_part2, sizeof(this->header_part2), " Session_id=%u user=%s account=%s hostname=%s target_service=%s session_info=%s", this->session_id, this->primary_user_sig, this->account_sig, this->hostname_sig, this->target_service_sig, this->session_info_sig);
    }


    ~RDPMetrics() {
          ::close(this->fd);
    }


    void sha1_encrypt(char * dest, const char * src, const size_t src_len) {
        SslSha1 sha1;
        sha1.update(byte_ptr_cast(src), src_len);
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

        date[0] = current_date[20];
        date[1] = current_date[21];
        date[2] = current_date[22];
        date[3] = current_date[23];
        date[4] = '-';
        date[5] =  current_date[4];
        date[6] =  current_date[5];
        date[7] =  current_date[6];
        date[8] = '-';
        date[9] = current_date[8];
        date[10] = current_date[9];
        date[11] = '\0';

        if (keep_hhmmss) {
            date[11] = '-';
            date[12] = current_date[11];
            date[13] = current_date[12];
            date[14] = current_date[13];
            date[15] = current_date[14];
            date[16] = current_date[15];
            date[17] = current_date[16];
            date[18] = current_date[17];
            date[19] = current_date[18];
            date[20] = '\0';
        }
    }


    void new_day() {
        char last_date_formated[24] = {'\0'};
        this->set_current_formated_date(last_date_formated, false, this->last_date);
        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path), "%s-%s.log", this->path_template, last_date_formated);

        this->fd = ::open(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );
        if (this->fd == -1) {
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd, this->complete_file_path);
        }
    }



    void log() {

        timeval now = tvtime();
        time_t time_date = now.tv_sec;
        if ((time_date -this->last_date) >= 3600*24) {
            ::close(this->fd);
            this->last_date = time_date;
            this->new_day();
        }
        const long int delta_time = time_date - this->start_time;

        char header[1024];
        ::snprintf(header, sizeof(header), "%s%ld%s", this->header_part1, delta_time, this->header_part2);

        std::string sentence(header);
        for (int i = 0; i < 31; i++) {
            if (this->current_data[i] - this->previous_data[i]) {
                char current_metrics[64];
                ::snprintf(current_metrics, sizeof(current_metrics), "%s%ld", this->rdp_metrics_name(i), this->current_data[i]);
                sentence += current_metrics;
                this->previous_data[i] = this->current_data[i];
            }
        }

        if (this->fd == -1) {
            LOG(LOG_INFO, "sentence=%s", sentence);

        } else {
            struct iovec iov[1];                             // = { {sentence.c_str(), sentence.length} };

            char sentence_char[4096] = {'\0'};
            memcpy(sentence_char, sentence.c_str(), sentence.length());
            iov[0].iov_base = sentence_char;
            iov[0].iov_len = sentence.length();

            ssize_t nwritten = ::writev(fd, iov, 1);
            //LOG(LOG_INFO, "nwritten=%zu sentence=%s ", nwritten, sentence);

            if (nwritten == -1) {
                std::string file_path_template(this->path_template);
                file_path_template += this->last_date;
                file_path_template += ".log";
                LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\"",this->fd, file_path_template);
            }
        }
    }
};
