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
#include "core/client_info.hpp"
#include "system/linux/system/ssl_sha256.hpp"

#include "core/RDP/clipboard.hpp"
#include "core/RDP/channels/rdpdr.hpp"

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

inline std::string hmac_client_info(const char * client_host, const ClientInfo & info, const unsigned char * key) {
    char session_info[1024];
    ::snprintf(session_info, sizeof(session_info), "%s%d%u%u", client_host, info.bpp, info.width, info.height);
    return metrics_encrypt(session_info, strlen(session_info), key);
}


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


class RDPMetrics
{

private:

    const char * version = "v1.0";                  // fields version

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

        const char * rdp_metrics_name(int index) {

        switch (index) {
            case main_channel_data_from_client:             return "main_channel_data_from_client";
            case right_click:                               return "right_click";
            case left_click:                                return "left_click";
            case keys_pressed:                              return "keys_pressed";
            case mouse_displacement:                        return "mouse_displacement";
            case main_channel_data_from_server:            return "main_channel_data_from_server";
            case clipboard_channel_data_from_server:        return "clipboard_channel_data_from_server";
            case nb_paste_text_on_server:                   return "nb_paste_text_on_server";
            case nb_paste_image_on_server:                  return "nb_paste_image_on_server";
            case nb_paste_file_on_server:                   return "nb_paste_file_on_server";
            case total_data_paste_on_server:                return "total_data_paste_on_server";
            case nb_copy_text_from_server:                  return "nb_copy_text_on_server";
            case nb_copy_image_from_server:                 return "nb_copy_image_on_server";
            case nb_copy_file_from_server:                  return "nb_copy_file_on_server";
            case clipboard_channel_data_from_client:        return "clipboard_channel_data_from_client";
            case nb_paste_text_on_client:                   return "nb_paste_text_on_client";
            case nb_paste_image_on_client:                  return "nb_paste_image_on_client";
            case nb_paste_file_on_client:                   return "nb_paste_file_on_client";
            case total_data_paste_on_client:                return "total_data_paste_on_client";
            case nb_copy_text_from_client:                  return "nb_copy_text_on_client";
            case nb_copy_image_from_client:                 return "nb_copy_image_on_client";
            case nb_copy_file_from_client:                  return "nb_copy_file_on_client";
            case disk_redirection_channel_data_from_client: return "disk_redirection_channel_data_from_client";
            case disk_redirection_channel_data_from_server: return "disk_redirection_channel_data_from_server";
            case nb_files_read:                          return "nb_files_read";
            case nb_files_or_folders_deleted:               return "nb_files_or_folders_deleted";
            case nb_files_write:                            return "nb_files_write";
            case nb_files_rename:                           return "nb_files_rename";
            case total_files_data_write:                    return "total_files_data_write";
            case total_files_data_read:                     return "total_files_data_read";
            case total_rail_amount_data_rcv_from_client:    return "rail_channel_data_from_client";
            case total_rail_amount_data_rcv_from_server:    return "rail_channel_data_from_server";
            case total_other_amount_data_rcv_from_client:   return "other_channel_data_from_client";
            case total_other_amount_data_rcv_from_server:   return "other_channel_data_from_server";
            case COUNT_FIELD: break;
        }

        return " unknow_rdp_metrics_name";
    }

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
    uint64_t current_data[COUNT_FIELD] = { 0 };

    const time_t connection_time;

    const std::chrono::seconds log_delay;
    timeval next_log_time;

    // RDP context Info
    int last_x = -1;
    int last_y = -1;
    uint32_t file_contents_format_ID = 0;
    uint32_t last_formatID = 0;
    bool cliprdr_init_format_list_done = false;

   const bool debug;


    void encrypt(char * dest, const char * src, const size_t src_len, const unsigned char * key_crypt) {
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
        dest[64] = 0;
    }


public:

    bool active() {
        return this->active_;
    }

    RDPMetrics( const bool activate                        // do nothing if false
              , std::string path
              , const char * session_id
              , const std::string & primary_user_sig       // clear primary user account
              , const std::string & account_sig            // secondary account
              , const std::string & target_service_sig     // clear target service name + clear device name
              , const std::string & session_info_sig       // source_host + client info
              , const time_t now                           // time at beginning of metrics
              , const std::chrono::hours file_interval     // daily rotation of filename (hours)
              , const std::chrono::seconds log_delay       // delay between 2 logs
              , const bool debug = false
      )
      : file_interval{file_interval}
      , current_file_date(now-now%(this->file_interval.count()))
      , path(std::move(path))
      , session_id(session_id)
      , active_(activate)
      , connection_time(now)
      , log_delay(log_delay)
      , next_log_time{ this->log_delay.count()+now, 0}
      , debug(debug)
    {
        ::snprintf(header, sizeof(header), "%s user=%s account=%s target_service_device=%s client_info=%s\n", this->session_id, primary_user_sig.c_str(), account_sig.c_str(), target_service_sig.c_str(), session_info_sig.c_str());

        if (this->path.c_str() && activate) {
            std::string dir_path = this->path.substr(0, this->path.length()-1);
            int mkdir_res = mkdir(dir_path.c_str(), ACCESSPERMS);
            if (mkdir_res == -1) {
                LOG(LOG_ERR, "Impossible to create directory at %s, error(%d)", dir_path, mkdir_res);
            }

            this->new_day(this->current_file_date);
        }
    }


    ~RDPMetrics() {
         this->disconnect();
    }

    void disconnect() {

        this->rotate(this->next_log_time.tv_sec);

        std::string text_date = ((this->current_file_date % (24*3600)) == 0)?text_gmdate(this->current_file_date):filename_gmdatetime(this->current_file_date);

        char index_file_path[1024];
        ::snprintf(index_file_path, sizeof(index_file_path), "%s/rdp_metrics-%s-%s.logindex", path.c_str(), this->version, text_date.c_str());

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

    void add_to_current_data(int index, uint64_t value) {
        this->current_data[index] += value;
    }

    void server_other_channel_data(long int len) {
        this->add_to_current_data(total_other_amount_data_rcv_from_server, len);
    }

    void client_other_channel_data(long int len) {
        this->add_to_current_data(total_other_amount_data_rcv_from_client, len);
    }

    void server_rail_channel_data(long int len) {
        this->add_to_current_data(total_rail_amount_data_rcv_from_server, len);
    }

    void client_rail_channel_data(long int len) {
        this->add_to_current_data(total_rail_amount_data_rcv_from_client, len);
    }

    void set_server_rdpdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->add_to_current_data(disk_redirection_channel_data_from_server, length);

            rdpdr::SharedHeader header;
            header.receive(chunk);

            if (header.component == rdpdr::RDPDR_CTYP_CORE &&  header.packet_id == rdpdr::PAKID_CORE_DEVICE_IOREQUEST) {
                rdpdr::DeviceIORequest dior;
                dior.receive(chunk);

                switch (dior.MajorFunction()) {

                    case rdpdr::IRP_MJ_READ:
                    {
                        rdpdr::DeviceReadRequest drr;
                        drr.receive(chunk);
                        if (drr.Offset() == 0) {
                            this->add_to_current_data(nb_files_read, 1);
                        }
                        this->add_to_current_data(total_files_data_read, drr.Length());
                    }
                        break;

                    case rdpdr::IRP_MJ_WRITE:
                    {
                        rdpdr::DeviceWriteRequest dwr;
                        dwr.receive(chunk);

                        if (dwr.Offset == 0) {
                            this->add_to_current_data(nb_files_write, 1);
                        }
                        this->add_to_current_data(total_files_data_write, dwr.Length);
                    }
                        break;

                    case rdpdr::IRP_MJ_SET_INFORMATION:     // delete
                    {
                        rdpdr::ServerDriveSetInformationRequest sdsir;
                        sdsir.receive(chunk);

                        switch (sdsir.FsInformationClass()) {

                            case rdpdr::FileRenameInformation:
                                this->add_to_current_data(nb_files_rename, 1);
                                break;
                            case rdpdr::FileDispositionInformation:
                                this->add_to_current_data(nb_files_or_folders_deleted, 1);
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
            this->add_to_current_data(disk_redirection_channel_data_from_client, length);
        }
    }

    void set_server_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->add_to_current_data(clipboard_channel_data_from_server, length);
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            // TODO code duplicated
            switch (header.msgType()) {

                case RDPECLIP::CB_FORMAT_LIST:
                {
                    RDPECLIP::FormatListPDU_LongName fl_ln;
                    fl_ln.recv(chunk);;

                    switch (fl_ln.formatID) {

                        case RDPECLIP::CF_TEXT:
                        case RDPECLIP::CF_OEMTEXT:
                        case RDPECLIP::CF_UNICODETEXT:
                        case RDPECLIP::CF_DSPTEXT:
                        case RDPECLIP::CF_LOCALE:
                            this->add_to_current_data(nb_copy_text_from_server, 1);
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->add_to_current_data(nb_copy_image_from_server, 1);
                            break;
                        default:
                            // TODO string_view
                            std::string format_name_string(char_ptr_cast(fl_ln.formatUTF8Name));
                            if (format_name_string == RDPECLIP::FILECONTENTS.data()
                             || format_name_string == RDPECLIP::FILEGROUPDESCRIPTORW.data()
                            ) {
                                this->file_contents_format_ID = fl_ln.formatID;
                                this->add_to_current_data(nb_copy_file_from_server, 1);
                            }
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                {
                    this->last_formatID = chunk.in_uint32_le();

                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:
                        case RDPECLIP::CF_OEMTEXT:
                        case RDPECLIP::CF_UNICODETEXT:
                        case RDPECLIP::CF_DSPTEXT:
                        case RDPECLIP::CF_LOCALE:
                            this->add_to_current_data(nb_paste_text_on_server, 1);
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->add_to_current_data(nb_paste_image_on_server, 1);
                            break;
                        default:
                            if (this->file_contents_format_ID == this->last_formatID){
                                this->add_to_current_data(nb_paste_file_on_server, 1);
                            }
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                {
                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:
                        case RDPECLIP::CF_OEMTEXT:
                        case RDPECLIP::CF_UNICODETEXT:
                        case RDPECLIP::CF_DSPTEXT:
                        case RDPECLIP::CF_LOCALE:
                            this->add_to_current_data(total_data_paste_on_client, header.dataLen());
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->add_to_current_data(total_data_paste_on_client, header.dataLen());
                            break;
                        default:
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                {
                    chunk.in_skip_bytes(8); // streamId(4 bytes) + lindex(4 bytes)
                    uint32_t flag_filecontents = chunk.in_uint32_le();
                    if (flag_filecontents == RDPECLIP::FILECONTENTS_RANGE) {
                        uint64_t nPositionLow = chunk.in_uint32_le();
                        uint64_t nPositionHigh = chunk.in_uint32_le();
                        this->add_to_current_data(total_data_paste_on_server, nPositionLow + (nPositionHigh << 32));
                    }
                }
                    break;
            }
        }
    }

    void set_client_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->add_to_current_data(clipboard_channel_data_from_client, length);
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            // TODO code duplicated
            switch (header.msgType()) {

                case RDPECLIP::CB_FORMAT_LIST:
                    if (this->cliprdr_init_format_list_done) {
                        RDPECLIP::FormatListPDU_LongName fl_ln;
                        fl_ln.recv(chunk);

                        switch (fl_ln.formatID) {

                            case RDPECLIP::CF_TEXT:
                            case RDPECLIP::CF_OEMTEXT:
                            case RDPECLIP::CF_UNICODETEXT:
                            case RDPECLIP::CF_DSPTEXT:
                            case RDPECLIP::CF_LOCALE:
                                this->add_to_current_data(nb_copy_text_from_client, 1);
                                break;
                            case RDPECLIP::CF_METAFILEPICT:
                            case RDPECLIP::CF_DSPMETAFILEPICT:
                                this->add_to_current_data(nb_copy_image_from_client, 1);
                                break;
                            default:
                                // TODO string_view
                                std::string format_name_string(char_ptr_cast(fl_ln.formatUTF8Name));
                                if (format_name_string == RDPECLIP::FILECONTENTS.data()
                                 || format_name_string == RDPECLIP::FILEGROUPDESCRIPTORW.data()
                                ) {
                                    this->file_contents_format_ID = fl_ln.formatID;
                                    this->add_to_current_data(nb_copy_file_from_client, 1);
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
                        case RDPECLIP::CF_TEXT:
                        case RDPECLIP::CF_OEMTEXT:
                        case RDPECLIP::CF_UNICODETEXT:
                        case RDPECLIP::CF_DSPTEXT:
                        case RDPECLIP::CF_LOCALE:
                            this->add_to_current_data(nb_paste_text_on_client, 1);
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->add_to_current_data(nb_paste_image_on_client, 1);
                            break;
                        default:
                            if (this->file_contents_format_ID == this->last_formatID){
                                this->add_to_current_data(nb_paste_file_on_client, 1);
                            }
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                {
                    switch (this->last_formatID) {
                        case RDPECLIP::CF_TEXT:
                        case RDPECLIP::CF_OEMTEXT:
                        case RDPECLIP::CF_UNICODETEXT:
                        case RDPECLIP::CF_DSPTEXT:
                        case RDPECLIP::CF_LOCALE:
                            this->add_to_current_data(total_data_paste_on_server, header.dataLen());
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->add_to_current_data(total_data_paste_on_server, header.dataLen());
                            break;
                        default:
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                {
                    chunk.in_skip_bytes(8);    // streamId(4 bytes) + lindex(4 bytes)
                    uint32_t flag_filecontents = chunk.in_uint32_le();
                    if (flag_filecontents == RDPECLIP::FILECONTENTS_RANGE) {
                        uint64_t size = chunk.in_uint64_le();
                        this->add_to_current_data(total_data_paste_on_client, size);
                    }
                }
                    break;
            }
        }
    }

    void server_main_channel_data(long int len) {
        this->add_to_current_data(main_channel_data_from_server, len);
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
            this->add_to_current_data(mouse_displacement, x_shift + y_shift);
        }
        this->last_x = x;
        this->last_y = y;
    }

    void key_pressed() {
        this->add_to_current_data(keys_pressed, 1);
    }

    void right_click_pressed() {
        this->add_to_current_data(right_click, 1);
    }

    void left_click_pressed() {
        this->add_to_current_data(left_click, 1);
    }

    void client_main_channel_data(long int len) {
        this->add_to_current_data(main_channel_data_from_client, len);
    }


    void new_day(time_t now) {
        std::string text_date = ((now % (24*3600)) == 0)?text_gmdate(now):filename_gmdatetime(now);

        ::snprintf(this->complete_file_path, sizeof(this->complete_file_path), "%s/rdp_metrics-%s-%s.logmetrics", this->path.c_str(), this->version, text_date.c_str());

        this->fd = unique_fd(this->complete_file_path, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (!this->fd.is_open()) {
            LOG(LOG_ERR, "Log Metrics error(%d): can't open \"%s\"", this->fd.fd(), this->complete_file_path);
        }

        char index_file_path[1024];
        ::snprintf(index_file_path, sizeof(index_file_path), "%s/rdp_metrics-%s-%s.logindex", path.c_str(), this->version, text_date.c_str());

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


    void log(timeval & now) {

        timeval wait_log_metrics = ::how_long_to_wait(this->next_log_time, now);
        if (this->active_ && !wait_log_metrics.tv_sec && ! wait_log_metrics.tv_usec) {
            this->next_log_time.tv_sec += this->log_delay.count();
            this->next_log_time.tv_usec = now.tv_usec;

            this->rotate(now.tv_sec);

            std::string text_datetime(text_gmdatetime(now.tv_sec));

            if (debug) {

                std::string debug_sentence;
                char debug_header[128];
                ::snprintf(debug_header, sizeof(debug_header), "%s %s", text_datetime.c_str(), this->session_id);
                debug_sentence += debug_header;

                for (int i = 0; i < COUNT_FIELD; i++) {
                    if (this->current_data[i]) {
                        char elem[128];
                        ::snprintf(elem, sizeof(elem),  " %s=%lu", this->rdp_metrics_name(i), this->current_data[i]);
                        debug_sentence += elem;
                    }
                }

                LOG(LOG_INFO, "%s", debug_sentence);
            }

            char sentence[4096];
            ::snprintf(sentence, sizeof(sentence), "%s %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n", text_datetime.c_str(), this->session_id, current_data[0], current_data[1], current_data[2], current_data[3], current_data[4], current_data[5], current_data[6], current_data[7], current_data[8], current_data[9], current_data[10], current_data[11], current_data[12], current_data[13], current_data[14], current_data[15], current_data[16], current_data[17], current_data[18], current_data[19], current_data[20], current_data[21], current_data[22], current_data[23], current_data[24], current_data[25], current_data[26], current_data[27], current_data[28], current_data[29], current_data[30], current_data[31], current_data[32], current_data[33]);

            iovec iov[] = { {sentence, strlen(sentence)} };

            ssize_t nwritten = ::writev(this->fd.fd(), iov, 1);

            if (nwritten == -1) {
                // TODO bad filename
                LOG(LOG_ERR, "Log Metrics error(%d): can't write in\"%s\"", this->fd.fd(), this->complete_file_path);
            }
        }
    }
};
