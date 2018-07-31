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
#include <time.h>
#include <cstring>
#include <fcntl.h>
#include <sys/uio.h>

#include "utils/sugar/unique_fd.hpp"
#include "utils/log.hpp"
#include "utils/difftimeval.hpp"
#include "core/client_info.hpp"
#include "system/linux/system/ssl_sha256.hpp"

#include "core/RDP/clipboard.hpp"
#include "core/RDP/channels/rdpdr.hpp"


class RDPMetrics
{

private:

    const char * version = "v1.0";                  // field version

    enum : int {

        main_channel_data_from_client,              // number of byte sent from client to main channel.
        right_click,                                // number of right click.
        left_click,                                 // number of left click.
        keys_pressed,                               // number of keys pressed.
        mouse_displacement,                         // total mouse move cumulated on x and y axis.
        main_channel_data_from_server,              // number of byte sent from server to main channel.

        clipboard_channel_data_from_server,         // number of byte sent from server to clipboard channel.
        nb_paste_text_on_server,                    // number of times a text is pasted on server from client clipboard data.
        nb_paste_image_on_server,                   // number of times a image is pasted on server from client clipboard data.
        nb_paste_file_on_server,                    // number of times a file is pasted on server from client clipboard data.
        total_data_paste_on_server,                 // number of byte sent from server to clipboard channel.
        nb_copy_text_from_server,                   // number of copy text local copy from server data.
        nb_copy_image_from_server,                  // number of copy image local copy from server data.
        nb_copy_file_from_server,                   // number of copy file local copy from server data.

        clipboard_channel_data_from_client,         // number of byte sent from client, to clipboard channel.
        nb_paste_text_on_client,                    // number of times a text is pasted on client, from server clipboard data.
        nb_paste_image_on_client,                   // number of times a image is pasted on client, from server clipboard data.
        nb_paste_file_on_client,                    // number of times a file is pasted on client, from server clipboard data.
        total_data_paste_on_client,                 // number of byte sent from client to clipboard channel.
        nb_copy_text_from_client,                   // number of copy text local copy from client data.
        nb_copy_image_from_client,                  // number of copy image local copy from client data.
        nb_copy_file_from_client,                   // number of copy file local copy from client data.

        disk_redirection_channel_data_from_client,  // number of byte sent from client to shared disk channel.
        disk_redirection_channel_data_from_server,  // number of byte sent from server to shared disk channel.
        nb_files_read,                              // number of files read on shared disk channel.
        nb_files_or_folders_deleted,                // number of files or folders delete on shared disk channel.
        nb_files_write,                             // number of files write on shared disk channel.
        nb_files_rename,                            // number of files rename on shared disk channel.
        total_files_data_read,                      // number bytes read from files on shared disk channel.
        total_files_data_write,                     // number of bytes written from files on shared disk channel.

        total_rail_amount_data_rcv_from_client,     // number of byte sent from client to remote app channel.
        total_rail_amount_data_rcv_from_server,     // number of byte sent from server to remote app channel.

        total_other_amount_data_rcv_from_client,    // number of byte sent from client to others channels.
        total_other_amount_data_rcv_from_server,    // number of byte sent from server to others channels.

        COUNT_FIELD
    };

//     const char * rdp_metrics_name(int index) {
//
//         switch (index) {
//             case main_channel_data_from_client:             return "main_channel_data_from_client";
//             case right_click:                               return "right_click";
//             case left_click:                                return "left_click";
//             case keys_pressed:                              return "keys_pressed";
//             case mouse_displacement:                        return "mouse_displacement";
//             case main_channel_data_from_server:            return "main_channel_data_from_server";
//             case clipboard_channel_data_from_server:        return "clipboard_channel_data_from_server";
//             case nb_paste_text_on_server:                   return "nb_paste_text_on_server";
//             case nb_paste_image_on_server:                  return "nb_paste_image_on_server";
//             case nb_paste_file_on_server:                   return "nb_paste_file_on_server";
//             case total_data_paste_on_server:                return "total_data_paste_on_server";
//             case nb_copy_text_from_server:                  return "nb_copy_text_on_server";
//             case nb_copy_image_from_server:                 return "nb_copy_image_on_server";
//             case nb_copy_file_from_server:                  return "nb_copy_file_on_server";
//             case clipboard_channel_data_from_client:        return "clipboard_channel_data_from_client";
//             case nb_paste_text_on_client:                   return "nb_paste_text_on_client";
//             case nb_paste_image_on_client:                  return "nb_paste_image_on_client";
//             case nb_paste_file_on_client:                   return "nb_paste_file_on_client";
//             case total_data_paste_on_client:                return "total_data_paste_on_client";
//             case nb_copy_text_from_client:                  return "nb_copy_text_on_client";
//             case nb_copy_image_from_client:                 return "nb_copy_image_on_client";
//             case nb_copy_file_from_client:                  return "nb_copy_file_on_client";
//             case disk_redirection_channel_data_from_client: return "disk_redirection_channel_data_from_client";
//             case disk_redirection_channel_data_from_server: return "disk_redirection_channel_data_from_server";
//             case nb_files_read:                          return "nb_files_read";
//             case nb_files_or_folders_deleted:               return "nb_files_or_folders_deleted";
//             case nb_files_write:                            return "nb_files_write";
//             case nb_files_rename:                           return "nb_files_rename";
//             case total_files_data_write:                    return "total_files_data_write";
//             case total_files_data_read:                     return "total_files_data_read";
//             case total_rail_amount_data_rcv_from_client:    return "rail_channel_data_from_client";
//             case total_rail_amount_data_rcv_from_server:    return "rail_channel_data_from_server";
//             case total_other_amount_data_rcv_from_client:   return "other_channel_data_from_client";
//             case total_other_amount_data_rcv_from_server:   return "other_channel_data_from_server";
//             case COUNT_FIELD: break;
//         }
//
//         return " unknow_rdp_metrics_name";
//     }


    //  output file info
    const int file_interval;
    time_t utc_last_date;
    char complete_file_path[4096] = {'\0'};
    //time_t utc_stat_time;
    const std::string path_template;
    unique_fd fd = invalid_fd();

    // LOG info
    const char * session_id;
    const bool active_ = false;
//     long int previous_data[COUNT_FIELD] = { 0 };
    long int current_data[COUNT_FIELD] = { 0 };

    timeval local_next_log_time;
    const time_t log_delay;

    // RDP context Info
    int last_x = -1;
    int last_y = -1;
    uint32_t file_contents_format_ID = 0;
    uint32_t last_formatID = 0;


    void encrypt(char * dest, const char * src, const size_t src_len, const unsigned char * key_crypt) {
        SslHMAC_Sha256 sha256(key_crypt, 32);
        sha256.update(byte_ptr_cast(src), src_len);
        uint8_t sig[SslSha256::DIGEST_LENGTH];
        sha256.final(sig);
        snprintf(dest, 1+SslSha256::DIGEST_LENGTH*2,
                 "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
                 "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
          ,sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7], sig[8], sig[9],
                 sig[10], sig[11], sig[12], sig[13], sig[14], sig[15], sig[16], sig[17], sig[18], sig[19]
          ,sig[20], sig[21], sig[22], sig[23], sig[24], sig[25], sig[26], sig[27], sig[28], sig[29],
                 sig[30], sig[31]);
    }


public:
    void set_utc_last_date(time_t date) {
        this->utc_last_date = date;
    }

    time_t get_utc_last_date() {
        return this->utc_last_date;
    }

    bool active() {
        return this->active_;
    }

    RDPMetrics( const std::string & path_template
              , const char * session_id_
              , const char * account
              , const char * primary_user
              , const char * target_host
              , const ClientInfo & info
              , const std::string & target_service
              , const std::string & target_device
              , const unsigned char * key_crypt
              , const long file_interval
              , const bool activate
              , const time_t log_delay
      )
      : file_interval(file_interval*3600)
      , path_template(path_template+"rdp_metrics")
      , session_id(session_id_)
      , active_(activate)
      , log_delay(log_delay)
    {
        char primary_user_sig[1+SslSha256::DIGEST_LENGTH*2] = {'\0'};
        char account_sig[1+SslSha256::DIGEST_LENGTH*2] = {'\0'};
        char target_service_sig[1+SslSha256::DIGEST_LENGTH*2] = {'\0'};
        char session_info_sig[1+SslSha256::DIGEST_LENGTH*2] = {'\0'};

        std::string target_device_and_service(target_service+" "+target_device);

        this->encrypt(primary_user_sig, primary_user, std::strlen(primary_user), key_crypt);
        this->encrypt(account_sig, account, std::strlen(account), key_crypt);
        this->encrypt(target_service_sig, target_device_and_service.c_str(), target_device_and_service.length(), key_crypt);

        char session_info[1024];
        ::snprintf(session_info, sizeof(session_info), "%s%d%u%u", target_host, info.bpp, info.width, info.height);
        this->encrypt(session_info_sig, session_info, std::strlen(session_info), key_crypt);

        char start_full_date_time[24];
        this->local_next_log_time = tvtime();
        this->set_current_formated_date(start_full_date_time, true, this->local_next_log_time.tv_sec);
        local_next_log_time.tv_sec += this->log_delay;

        char header[1024];
        ::snprintf(header, sizeof(header), "%s %s user=%s account=%s target_service_device=%s client_info=%s\n", start_full_date_time, this->session_id, primary_user_sig, account_sig, target_service_sig, session_info_sig);

        if (this->path_template.c_str() && activate) {

            time ( &(this->utc_last_date) );
            //this->utc_stat_time = this->utc_last_date;
            this->new_day(this->utc_last_date);

            char utc_last_date_formated[24] = {'\0'};
            char complete_header_file_path[1024];
            this->set_current_formated_date(utc_last_date_formated, false, this->utc_last_date);
            ::snprintf(complete_header_file_path, sizeof(complete_header_file_path), "%sindex_rdp_metrics-%s.log", path_template.c_str(), utc_last_date_formated);

            int fd = ::open(complete_header_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO );

            unique_fd fd_header(fd);

             if (fd_header.fd() != -1) {
                iovec iov[1] = { {header, strlen(header)} };

                ssize_t nwritten = ::writev(fd_header.fd(), iov, 1);

                if (nwritten == -1) {
                    // TODO bad filename
                    LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"",this->fd.fd(), complete_header_file_path);
                } else {
                  ::close(fd_header.fd());
                }
            } else {
                LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"",this->fd.fd(), complete_header_file_path);
            }
        }

    }


    ~RDPMetrics() {
         this->disconnect();
        ::close(this->fd.fd());
    }

    void disconnect() {
        char start_full_date_time[24];
        timeval local_time = tvtime();
        this->set_current_formated_date(start_full_date_time, true, local_time.tv_sec);

        char header_delta[2048];
        ::snprintf(header_delta, sizeof(header_delta), "%s %s", start_full_date_time, this->session_id);
        std::string sentence(header_delta);
        sentence += " disconnection\n";

        iovec iov[1] = { {const_cast<char *>(sentence.c_str()), sentence.length()} };

        ssize_t nwritten = ::writev(fd.fd(), iov, 1);

        if (nwritten == -1) {
            // TODO bad filename
            LOG(LOG_ERR, "Log Metrics error(%d): can't write \"%s\"", this->fd.fd(), this->complete_file_path);
        }

        this->fd.close();
    }

    bool cliprdr_init_format_list_done = false;

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
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->current_data[disk_redirection_channel_data_from_server] += length;

            rdpdr::SharedHeader header;
            header.receive(chunk);

            if (header.component == rdpdr::RDPDR_CTYP_CORE && (header.packet_id == rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION || header.packet_id == rdpdr::PAKID_CORE_DEVICE_IOREQUEST)) {
                rdpdr::DeviceIORequest dior;
                dior.receive(chunk);

                switch (dior.MajorFunction()) {

                    case rdpdr::IRP_MJ_READ:
                    {
                        rdpdr::DeviceReadRequest drr;
                        drr.receive(chunk);
                        if (drr.Offset() == 0) {
                            this->current_data[nb_files_read] += 1;
                        }
                        this->current_data[total_files_data_read] += drr.Length();
                    }
                        break;

                    case rdpdr::IRP_MJ_WRITE:
                    {
                        rdpdr::DeviceWriteRequest dwr;
                        dwr.receive(chunk);

                        if (dwr.Offset == 0) {
                            this->current_data[nb_files_write] += 1;
                        }
                        this->current_data[total_files_data_write] += dwr.Length;
                    }
                        break;

                    case rdpdr::IRP_MJ_SET_INFORMATION:     // delete
                    {
                        rdpdr::ServerDriveSetInformationRequest sdsir;
                        sdsir.receive(chunk);

                        switch (sdsir.FsInformationClass()) {

                            case rdpdr::FileRenameInformation:
                                this->current_data[nb_files_rename] += 1;
                                break;
                            case rdpdr::FileDispositionInformation:
                                this->current_data[nb_files_or_folders_deleted] += 1;
                                break;
                        }
                    }
                        break;
                }
            }
        }
    }

    void set_client_rdpdr_metrics(InStream & /*chunk*/, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->current_data[disk_redirection_channel_data_from_client] += length;
        }
    }

    void set_server_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->current_data[clipboard_channel_data_from_server] += length;
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            switch (header.msgType()) {

                case RDPECLIP::CB_FORMAT_LIST:
                    if (this->cliprdr_init_format_list_done) {
                        RDPECLIP::FormatListPDU_LongName fl_ln;
                        fl_ln.recv(chunk);

                        switch (fl_ln.formatID) {

                            case RDPECLIP::CF_TEXT:        [[fallthrough]];
                            case RDPECLIP::CF_OEMTEXT:     [[fallthrough]];
                            case RDPECLIP::CF_UNICODETEXT: [[fallthrough]];
                            case RDPECLIP::CF_DSPTEXT:     [[fallthrough]];
                            case RDPECLIP::CF_LOCALE:
                                this->current_data[nb_copy_text_from_server] += 1;
                                break;
                            case RDPECLIP::CF_METAFILEPICT: [[fallthrough]];
                            case RDPECLIP::CF_DSPMETAFILEPICT:
                                this->current_data[nb_copy_image_from_server] += 1;
                                break;
                            default:
                                std::string format_name_string(reinterpret_cast<const char *>(fl_ln.formatUTF8Name));
                                std::string file_desciptor_name(RDPECLIP::FILEGROUPDESCRIPTORW);
                                std::string file_contents_name(RDPECLIP::FILECONTENTS);
                                if (format_name_string == file_contents_name || format_name_string == file_desciptor_name) {
                                    this->file_contents_format_ID = fl_ln.formatID;
                                    this->current_data[nb_copy_file_from_server] += 1;
                                }
                                break;
                        }

                    } else {
                        this->cliprdr_init_format_list_done = true;
                    }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                {
                    this->last_formatID = chunk.in_uint32_le();

                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:        [[fallthrough]];
                        case RDPECLIP::CF_OEMTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_UNICODETEXT: [[fallthrough]];
                        case RDPECLIP::CF_DSPTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_LOCALE:
                            this->current_data[nb_paste_text_on_server] += 1;
                            break;
                        case RDPECLIP::CF_METAFILEPICT: [[fallthrough]];
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->current_data[nb_paste_image_on_server] += 1;
                            break;
                        default:
                            if (this->file_contents_format_ID == this->last_formatID){
                                this->current_data[nb_paste_file_on_server] += 1;
                            }
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                {
                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:        [[fallthrough]];
                        case RDPECLIP::CF_OEMTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_UNICODETEXT: [[fallthrough]];
                        case RDPECLIP::CF_DSPTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_LOCALE:
                            this->current_data[total_data_paste_on_client] += header.dataLen();
                            break;
                        case RDPECLIP::CF_METAFILEPICT: [[fallthrough]];
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->current_data[total_data_paste_on_client] += header.dataLen();
                            break;
                        default:
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                {
                    chunk.in_skip_bytes(8);
                    uint32_t flag_filecontents = chunk.in_uint32_le();
                    if (flag_filecontents == RDPECLIP::FILECONTENTS_RANGE) {
                        long int size = chunk.in_uint32_le();
                        size = size << 32;
                        size += chunk.in_uint32_le();
                        this->current_data[total_data_paste_on_server] += size;
                    }
                }
                    break;
            }
        }
    }

    void set_client_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->current_data[clipboard_channel_data_from_client] += length;
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            switch (header.msgType()) {

                case RDPECLIP::CB_FORMAT_LIST:
                    if (this->cliprdr_init_format_list_done) {
                        RDPECLIP::FormatListPDU_LongName fl_ln;
                        fl_ln.recv(chunk);

                        switch (fl_ln.formatID) {

                            case RDPECLIP::CF_TEXT:        [[fallthrough]];
                            case RDPECLIP::CF_OEMTEXT:     [[fallthrough]];
                            case RDPECLIP::CF_UNICODETEXT: [[fallthrough]];
                            case RDPECLIP::CF_DSPTEXT:     [[fallthrough]];
                            case RDPECLIP::CF_LOCALE:
                                this->current_data[nb_copy_text_from_client] += 1;
                                break;
                            case RDPECLIP::CF_METAFILEPICT: [[fallthrough]];
                            case RDPECLIP::CF_DSPMETAFILEPICT:
                                this->current_data[nb_copy_image_from_client] += 1;
                                break;
                            default:
                                std::string format_name_string(reinterpret_cast<const char *>(fl_ln.formatUTF8Name));
                                std::string file_desciptor_name(RDPECLIP::FILEGROUPDESCRIPTORW);
                                std::string file_contents_name(RDPECLIP::FILECONTENTS);
                                if (format_name_string == file_contents_name || format_name_string == file_desciptor_name) {
                                    this->file_contents_format_ID = fl_ln.formatID;
                                    this->current_data[nb_copy_file_from_client] += 1;
                                }
                                break;
                        }

                    } else {
                        this->cliprdr_init_format_list_done = true;
                    }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                {
                    this->last_formatID = chunk.in_uint32_le();

                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:        [[fallthrough]];
                        case RDPECLIP::CF_OEMTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_UNICODETEXT: [[fallthrough]];
                        case RDPECLIP::CF_DSPTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_LOCALE:
                            this->current_data[nb_paste_text_on_client] += 1;
                            break;
                        case RDPECLIP::CF_METAFILEPICT: [[fallthrough]];
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->current_data[nb_paste_image_on_client] += 1;
                            break;
                        default:
                            if (this->file_contents_format_ID == this->last_formatID){
                                this->current_data[nb_paste_file_on_client] += 1;
                            }
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                {
                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:        [[fallthrough]];
                        case RDPECLIP::CF_OEMTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_UNICODETEXT: [[fallthrough]];
                        case RDPECLIP::CF_DSPTEXT:     [[fallthrough]];
                        case RDPECLIP::CF_LOCALE:
                            this->current_data[total_data_paste_on_server] += header.dataLen();
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->current_data[total_data_paste_on_server] += header.dataLen();
                            break;
                        default:
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                {
                    chunk.in_skip_bytes(8);
                    uint32_t flag_filecontents = chunk.in_uint32_le();
                    if (flag_filecontents == RDPECLIP::FILECONTENTS_RANGE) {
                        long int size = chunk.in_uint32_le();
                        size = size << 32;
                        size += chunk.in_uint32_le();
                        this->current_data[total_data_paste_on_client] += size;
                    }
                }
                    break;
            }
        }
    }

    void server_main_channel_data(long int len) {
        this->current_data[main_channel_data_from_server] += len;
    }

    void mouse_mouve(const int x, const int y) {
        if (this->last_x >= 0 && this->last_y >= 0) {
            int x_shift = x - this->last_x;
            if (x_shift < 0) {
                x_shift *=  -1;
            }
            int y_shift = y - this->last_y;
            if (y_shift < 0) {
                y_shift *=  -1;
            }
            this->current_data[mouse_displacement] += x_shift + y_shift;
        }
        this->last_x = x;
        this->last_y = y;
    }

    void key_pressed() {
        this->current_data[keys_pressed]++;
    }

    void right_click_pressed() {
        this->current_data[right_click]++;
    }

    void left_click_pressed() {
        this->current_data[left_click]++;
    }

    void client_main_channel_data(long int len) {
        this->current_data[main_channel_data_from_client] += len;
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


    void new_day(time_t utc_time_date) {
        this->utc_last_date = utc_time_date;
        char utc_last_date_formated[24] = {'\0'};
        this->set_current_formated_date(utc_last_date_formated, false, this->utc_last_date);
        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path), "%s-%s-%s.log", this->path_template.c_str(), this->version, utc_last_date_formated);

        this->fd = unique_fd(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!this->fd.is_open()) {
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd.fd(), this->complete_file_path);
        }
    }


    void log() {
        timeval local_time = tvtime();
        timeval wait_log_metrics = ::how_long_to_wait(this->local_next_log_time, local_time);
        if (!wait_log_metrics.tv_sec && ! wait_log_metrics.tv_usec) {
            local_next_log_time.tv_sec += this->log_delay;

            time_t utc_time_date;
            time ( &utc_time_date );

            if ((utc_time_date -this->utc_last_date) >= this->file_interval) {
                this->new_day(utc_time_date);
            }

            char start_full_date_time[24];

            this->set_current_formated_date(start_full_date_time, true, local_time.tv_sec);

            char sentence[4096];
            ::snprintf(sentence, sizeof(sentence), "%s %s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", start_full_date_time, this->session_id, current_data[0], current_data[1], current_data[2], current_data[3], current_data[4], current_data[5], current_data[6], current_data[7], current_data[8], current_data[9], current_data[10], current_data[11], current_data[12], current_data[13], current_data[14], current_data[15], current_data[16], current_data[17], current_data[18], current_data[19], current_data[20], current_data[21], current_data[22], current_data[23], current_data[24], current_data[25], current_data[26], current_data[27], current_data[28], current_data[29], current_data[30], current_data[31], current_data[32], current_data[33]);

            iovec iov[] = { {sentence, strlen(sentence)} };

            ssize_t nwritten = ::writev(this->fd.fd(), iov, 1);

            if (nwritten == -1) {
                // TODO bad filename
                LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"", this->fd.fd(), this->complete_file_path);
            }
        }


//         iovec iov[COUNT_FIELD+2];
//         char sentence_data[COUNT_FIELD+2][128];
//         int nb_elem = 0;
//         for (int i = 0; i < COUNT_FIELD; i++) {
//             if (this->current_data[i] - this->previous_data[i]) {
//                 nb_elem++;
//                 ::snprintf(sentence_data[nb_elem], sizeof(sentence_data[nb_elem]), " %s=%ld", this->rdp_metrics_name(i), this->current_data[i]);
//                 iov[nb_elem].iov_base = sentence_data[nb_elem];
//                 iov[nb_elem].iov_len  = strlen(sentence_data[nb_elem]);
//                 this->previous_data[i] = this->current_data[i];
//             }
//         }

//         if (nb_elem) {
//             if (this->fd.fd() != -1) {
//                 char start_full_date_time[24];
//                 timeval local_time = tvtime();
//                 this->set_current_formated_date(start_full_date_time, true, local_time.tv_sec);
//
//                 ::snprintf(sentence_data[0], sizeof(sentence_data[0]), "%s %s", start_full_date_time, this->session_id);
//                 iov[0].iov_base = sentence_data[0];
//                 iov[0].iov_len = strlen(sentence_data[0]);
//
//                 char end[] = {'\n', '\0'};
//                 iov[nb_elem+1].iov_base = end;
//                 iov[nb_elem+1].iov_len = strlen(end);

//                 ssize_t nwritten = ::writev(this->fd.fd(), iov, nb_elem+2);

//                 if (nwritten == -1) {
//                     // TODO bad filename
//                     LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"", this->fd.fd(), this->complete_file_path);
//                 }
//             } else {
//                 LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"", this->fd.fd(), this->complete_file_path);
//             }
//         }


    }
};
