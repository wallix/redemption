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

#include "utils/sugar/cast.hpp"
#include "mod/metrics.hpp"

#include "core/client_info.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/channels/rdpdr.hpp"


class RDPMetrics
{

private:

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
            case main_channel_data_from_server:             return "main_channel_data_from_server";
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
            case nb_files_read:                             return "nb_files_read";
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

        return "unknow_rdp_metrics_name";
    }

    const char * rdp_protocol_name = "rdp";

    Metrics metrics;



    // RDP context Info
    int last_x = -1;
    int last_y = -1;
    uint32_t file_contents_format_ID = 0;
    uint32_t last_formatID = 0;
    bool cliprdr_init_format_list_done = false;
    bool use_long_format_names     = true;

    uint32_t flag_filecontents = 0;


public:
    RDPMetrics( const bool activate                         // do nothing if false
              , std::string path
              , std::string session_id
              , array_view_const_char primary_user_sig      // clear primary user account
              , array_view_const_char account_sig           // secondary account
              , array_view_const_char target_service_sig    // clear target service name + clear device name
              , array_view_const_char session_info_sig      // source_host + client info
              , const std::chrono::seconds now              // time at beginning of metrics
              , const std::chrono::hours file_interval      // daily rotation of filename (hours)
              , const std::chrono::seconds log_delay        // delay between 2 logs
              )
        : metrics(/*this->rdp_field_version*/"v1.0", this->rdp_protocol_name,
            activate, COUNT_FIELD, std::move(path), std::move(session_id),
            primary_user_sig, account_sig, target_service_sig, session_info_sig,
            now, file_interval, log_delay)
    {
    }

    bool active() {
        return this->metrics.active_;
    }

    void server_other_channel_data(long int len) {
        this->metrics.add_to_current_data(total_other_amount_data_rcv_from_server, len);
    }

    void client_other_channel_data(long int len) {
        this->metrics.add_to_current_data(total_other_amount_data_rcv_from_client, len);
    }

    void server_rail_channel_data(long int len) {
        this->metrics.add_to_current_data(total_rail_amount_data_rcv_from_server, len);
    }

    void client_rail_channel_data(long int len) {
        this->metrics.add_to_current_data(total_rail_amount_data_rcv_from_client, len);
    }

    void set_server_rdpdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics.add_to_current_data(disk_redirection_channel_data_from_server, length);

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
                            this->metrics.add_to_current_data(nb_files_read, 1);
                        }
                        this->metrics.add_to_current_data(total_files_data_read, drr.Length());
                    }
                        break;

                    case rdpdr::IRP_MJ_WRITE:
                    {
                        rdpdr::DeviceWriteRequest dwr;
                        dwr.receive(chunk);

                        if (dwr.Offset == 0) {
                            this->metrics.add_to_current_data(nb_files_write, 1);
                        }
                        this->metrics.add_to_current_data(total_files_data_write, dwr.Length);
                    }
                        break;

                    case rdpdr::IRP_MJ_SET_INFORMATION:     // delete
                    {
                        rdpdr::ServerDriveSetInformationRequest sdsir;
                        sdsir.receive(chunk);

                        switch (sdsir.FsInformationClass()) {

                            case rdpdr::FileRenameInformation:
                                this->metrics.add_to_current_data(nb_files_rename, 1);
                                break;
                            case rdpdr::FileDispositionInformation:
                                this->metrics.add_to_current_data(nb_files_or_folders_deleted, 1);
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
            this->metrics.add_to_current_data(disk_redirection_channel_data_from_client, length);
        }
    }

    void set_server_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics.add_to_current_data(clipboard_channel_data_from_server, length);
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            // TODO code duplicated
            switch (header.msgType()) {

                case RDPECLIP::CB_FORMAT_LIST:
                {
                    bool known_format_not_found = true;
                    while (known_format_not_found) {

                        uint32_t formatID = 0;
                        std::string formatName;
                        if (this->use_long_format_names) {
                            RDPECLIP::FormatListPDU_LongName fl_ln;
                            fl_ln.recv(chunk);
                            fl_ln.log();
                            formatID = fl_ln.formatID;
                            formatName = char_ptr_cast(fl_ln.formatUTF8Name);
                            if (chunk.in_remain() <= 6) {
                                known_format_not_found = false;
                            }
                        } else {
                            RDPECLIP::FormatListPDU_ShortName fl_sn;
                            fl_sn.recv(chunk);
                            formatID = fl_sn.formatID;
                            formatName = char_ptr_cast(fl_sn.formatUTF8Name);
                            if (chunk.in_remain() <= 36) {
                                known_format_not_found = false;
                            }
                        }

                        switch (formatID) {

                            case RDPECLIP::CF_TEXT:
                            case RDPECLIP::CF_LOCALE:
                            case RDPECLIP::CF_UNICODETEXT:
                            case RDPECLIP::CF_OEMTEXT:
                                this->metrics.add_to_current_data(nb_copy_text_from_server, 1);
                                known_format_not_found = false;
                                break;
                            case RDPECLIP::CF_METAFILEPICT:
                                this->metrics.add_to_current_data(nb_copy_image_from_server, 1);
                                known_format_not_found = false;
                                break;
                            default:
                                // TODO string_view
                                if (formatName == RDPECLIP::FILEGROUPDESCRIPTORW.data()) {
                                    this->file_contents_format_ID = formatID;
                                    this->metrics.add_to_current_data(nb_copy_file_from_server, 1);
                                    known_format_not_found = false;
                                }
                                break;
                        }
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
                            this->metrics.add_to_current_data(nb_paste_text_on_server, 1);
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->metrics.add_to_current_data(nb_paste_image_on_server, 1);
                            break;
                        default:
                            if (this->file_contents_format_ID == this->last_formatID){
                                this->metrics.add_to_current_data(nb_paste_file_on_server, 1);
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
                            this->metrics.add_to_current_data(total_data_paste_on_client, header.dataLen());
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->metrics.add_to_current_data(total_data_paste_on_client, header.dataLen());
                            break;
                        default:
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                {
                    chunk.in_skip_bytes(8); // streamId(4 bytes) + lindex(4 bytes)
                    this->flag_filecontents = chunk.in_uint32_le();
                    break;
                }

                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                {
                    if (this->flag_filecontents == RDPECLIP::FILECONTENTS_SIZE) {
                        chunk.in_skip_bytes(4);             // streamId(4 bytes)
                        uint32_t nPositionLow = chunk.in_uint32_le();
                        uint64_t nPositionHigh = chunk.in_uint32_le();
                        this->metrics.add_to_current_data(total_data_paste_on_client, nPositionLow + (nPositionHigh << 32));
                    }
                }
            }
        }
    }

    void set_client_cliprdr_metrics(InStream & chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics.add_to_current_data(clipboard_channel_data_from_client, length);
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            // TODO code duplicated
            switch (header.msgType()) {

                case RDPECLIP::CB_CLIP_CAPS:
                {
                    chunk.in_skip_bytes(4);                 // RDPECLIP::ClipboardCapabilitiesPDU

                    RDPECLIP::GeneralCapabilitySet pdu2;
                    pdu2.recv(chunk);

                    this->use_long_format_names = bool(pdu2.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
                }
                    break;

                case RDPECLIP::CB_FORMAT_LIST:
                    if (this->cliprdr_init_format_list_done) {

                        bool known_format_not_found = true;
                        while (known_format_not_found) {

                            uint32_t formatID = 0;
                            std::string formatName;
                            if (this->use_long_format_names) {

                                RDPECLIP::FormatListPDU_LongName fl_ln;
                                fl_ln.recv(chunk);
                                fl_ln.log();

                                formatID = fl_ln.formatID;
                                formatName = char_ptr_cast(fl_ln.formatUTF8Name);
                                if (chunk.in_remain() <= 6) {
                                    known_format_not_found = false;
                                }
                            } else {
                                RDPECLIP::FormatListPDU_ShortName fl_sn;
                                fl_sn.recv(chunk);
                                formatID = fl_sn.formatID;
                                formatName = char_ptr_cast(fl_sn.formatUTF8Name);
                                if (chunk.in_remain() <= 36) {
                                    known_format_not_found = false;
                                }
                            }

                            switch (formatID) {

                                case RDPECLIP::CF_TEXT:
                                case RDPECLIP::CF_LOCALE:
                                case RDPECLIP::CF_UNICODETEXT:
                                case RDPECLIP::CF_OEMTEXT:
                                    this->metrics.add_to_current_data(nb_copy_text_from_client, 1);
                                    known_format_not_found = false;
                                    break;
                                case RDPECLIP::CF_METAFILEPICT:
                                    this->metrics.add_to_current_data(nb_copy_image_from_client, 1);
                                    known_format_not_found = false;
                                    break;
                                default:
                                    if (formatName == RDPECLIP::FILEGROUPDESCRIPTORW.data()) {
                                        this->file_contents_format_ID = formatID;
                                        this->metrics.add_to_current_data(nb_copy_file_from_client, 1);
                                        known_format_not_found = false;
                                    }
                                    break;
                            }
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
                            this->metrics.add_to_current_data(nb_paste_text_on_client, 1);
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->metrics.add_to_current_data(nb_paste_image_on_client, 1);
                            break;
                        default:
                            if (this->file_contents_format_ID == this->last_formatID){
                                this->metrics.add_to_current_data(nb_paste_file_on_client, 1);
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
                            this->metrics.add_to_current_data(total_data_paste_on_server, header.dataLen());
                            break;
                        case RDPECLIP::CF_METAFILEPICT:
                        case RDPECLIP::CF_DSPMETAFILEPICT:
                            this->metrics.add_to_current_data(total_data_paste_on_server, header.dataLen());
                            break;
                        default:
                            break;
                    }
                }
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                {
                    chunk.in_skip_bytes(8); // streamId(4 bytes) + lindex(4 bytes)
                    this->flag_filecontents = chunk.in_uint32_le();
                    break;
                }

                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                {
                    if (this->flag_filecontents == RDPECLIP::FILECONTENTS_SIZE) {
                        chunk.in_skip_bytes(4);             // streamId(4 bytes)
                        uint32_t nPositionLow = chunk.in_uint32_le();
                        uint64_t nPositionHigh = chunk.in_uint32_le();
                        this->metrics.add_to_current_data(total_data_paste_on_server, nPositionLow + (nPositionHigh << 32));
                    }
                }
            }
        }
    }

    void log(timeval const& now) {
        this->metrics.log(now);
    }

    void server_main_channel_data(long int len) {
        this->metrics.add_to_current_data(main_channel_data_from_server, len);
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
            this->metrics.add_to_current_data(mouse_displacement, x_shift + y_shift);
        }
        this->last_x = x;
        this->last_y = y;
    }

    void key_pressed() {
        this->metrics.add_to_current_data(keys_pressed, 1);
    }

    void right_click_pressed() {
        this->metrics.add_to_current_data(right_click, 1);
    }

    void left_click_pressed() {
        this->metrics.add_to_current_data(left_click, 1);
    }

    void client_main_channel_data(long int len) {
        this->metrics.add_to_current_data(main_channel_data_from_client, len);
    }
};
