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

#include <cassert>


class RDPMetrics
{
    enum FieldIndex : int {

        CLIENT_MAIN_CHANNEL_DATA,        // number of byte sent from client to main channel.
        RIGHT_CLICK,                     // number of right click.
        LEFT_CLICK,                      // number of left click.
        KEYS_PRESSED,                    // number of keys pressed.
        MOUSE_DISPLACEMENT,              // total mouse move cumulated on x and y axis.
        SERVER_MAIN_CHANNEL_DATA,        // number of byte sent from server to main channel.

        SERVER_CLIPBOARD_CHANNEL_DATA,   // number of byte sent from server to clipboard channel.
        ON_SERVER_TEXT_PASTE_COUNT,      // number of times a text is pasted on server from client clipboard data.
        ON_SERVER_IMAGE_PASTE_COUNT,     // number of times a image is pasted on server from client clipboard data.
        ON_SERVER_FILE_PASTE_COUNT,      // number of times a file is pasted on server from client clipboard data.
        ON_SERVER_TOTAL_DATA_PASTE,      // number of byte sent from server to clipboard channel.
        SERVER_TEXT_COPY_COUNT,          // number of copy text local copy from server data.
        SERVER_IMAGE_COPY_COUNT,         // number of copy image local copy from server data.
        SERVER_FILE_COPY_COUNT,          // number of copy file local copy from server data.

        CLIENT_CLIPBOARD_CHANNEL_DATA,   // number of byte sent from client, to clipboard channel.
        ON_CLIENT_TEXT_PASTE_COUNT,      // number of times a text is pasted on client, from server clipboard data.
        ON_CLIENT_IMAGE_PASTE_COUNT,     // number of times a image is pasted on client, from server clipboard data.
        ON_CLIENT_FILE_PASTE_COUNT,      // number of times a file is pasted on client, from server clipboard data.
        ON_CLIENT_TOTAL_DATA_PASTE,      // number of byte sent from client to clipboard channel.
        CLIENT_TEXT_COPY_COUNT,          // number of copy text local copy from client data.
        CLIENT_IMAGE_COPY_COUNT,         // number of copy image local copy from client data.
        CLIENT_FILE_COPY_COUNT,          // number of copy file local copy from client data.

        CLIENT_DISK_REDIR_CHANNEL_DATA,  // number of byte sent from client to shared disk channel.
        SERVER_DISK_REDIR_CHANNEL_DATA,  // number of byte sent from server to shared disk channel.
        FILES_READ_COUNT,                // number of files read on shared disk channel.
        FILES_OR_FOLDERS_DELEDTE_COUNT,  // number of files or folders delete on shared disk channel.
        FILES_WRITE_COUNT,               // number of files write on shared disk channel.
        FILES_RENAME_COUNT,              // number of files rename on shared disk channel.
        TOTAL_READ_DATA,                 // number bytes read from files on shared disk channel.
        TOTAL_FILES_WRITTEN_DATA,        // number of bytes written from files on shared disk channel.

        CLIENT_TOTAL_RAIL_CHANNEL_DATA,  // number of byte sent from client to remote app channel.
        SERVER_TOTAL_RAIL_CHANNEL_DATA,  // number of byte sent from server to remote app channel.

        CLIENT_TOTAL_OTHERS_CHANNEL_DATA,// number of byte sent from client to others channels.
        SERVER_TOTAL_OTHERS_CHANNEL_DATA,// number of byte sent from server to others channels.

        COUNT_FIELD
    };

    const char * rdp_metrics_name(int index) noexcept
    {
        switch (index) {
            case CLIENT_MAIN_CHANNEL_DATA:         return "CLIENT_MAIN_CHANNEL_DATA";
            case RIGHT_CLICK:                      return "RIGHT_CLICK";
            case LEFT_CLICK:                       return "LEFT_CLICK";
            case KEYS_PRESSED:                     return "KEYS_PRESSED";
            case MOUSE_DISPLACEMENT:               return "MOUSE_DISPLACEMENT";
            case SERVER_MAIN_CHANNEL_DATA:         return "SERVER_MAIN_CHANNEL_DATA";
            case SERVER_CLIPBOARD_CHANNEL_DATA:    return "SERVER_CLIPBOARD_CHANNEL_DATA";
            case ON_SERVER_TEXT_PASTE_COUNT:       return "ON_SERVER_TEXT_PASTE_COUNT";
            case ON_SERVER_IMAGE_PASTE_COUNT:      return "ON_SERVER_IMAGE_PASTE_COUNT";
            case ON_SERVER_FILE_PASTE_COUNT:       return "ON_SERVER_FILE_PASTE_COUNT";
            case ON_SERVER_TOTAL_DATA_PASTE:       return "ON_SERVER_TOTAL_DATA_PASTE";
            case SERVER_TEXT_COPY_COUNT:           return "SERVER_TEXT_COPY_COUNT";
            case SERVER_IMAGE_COPY_COUNT:          return "SERVER_IMAGE_COPY_COUNT";
            case SERVER_FILE_COPY_COUNT:           return "SERVER_FILE_COPY_COUNT";
            case CLIENT_CLIPBOARD_CHANNEL_DATA:    return "CLIENT_CLIPBOARD_CHANNEL_DATA";
            case ON_CLIENT_TEXT_PASTE_COUNT:       return "ON_CLIENT_TEXT_PASTE_COUNT";
            case ON_CLIENT_IMAGE_PASTE_COUNT:      return "ON_CLIENT_IMAGE_PASTE_COUNT";
            case ON_CLIENT_FILE_PASTE_COUNT:       return "ON_CLIENT_FILE_PASTE_COUNT";
            case ON_CLIENT_TOTAL_DATA_PASTE:       return "ON_CLIENT_TOTAL_DATA_PASTE";
            case CLIENT_TEXT_COPY_COUNT:           return "CLIENT_TEXT_COPY_COUNT";
            case CLIENT_IMAGE_COPY_COUNT:          return "CLIENT_IMAGE_COPY_COUNT";
            case CLIENT_FILE_COPY_COUNT:           return "CLIENT_FILE_COPY_COUNT";
            case CLIENT_DISK_REDIR_CHANNEL_DATA:   return "CLIENT_DISK_REDIR_CHANNEL_DATA";
            case SERVER_DISK_REDIR_CHANNEL_DATA:   return "SERVER_DISK_REDIR_CHANNEL_DATA";
            case FILES_READ_COUNT:                 return "FILES_READ_COUNT";
            case FILES_OR_FOLDERS_DELEDTE_COUNT:   return "FILES_OR_FOLDERS_DELEDTE_COUNT";
            case FILES_WRITE_COUNT:                return "FILES_WRITE_COUNT";
            case FILES_RENAME_COUNT:               return "FILES_RENAME_COUNT";
            case TOTAL_FILES_WRITTEN_DATA:         return "TOTAL_FILES_WRITTEN_DATA";
            case TOTAL_READ_DATA:                  return "TOTAL_READ_DATA";
            case CLIENT_TOTAL_RAIL_CHANNEL_DATA:   return "CLIENT_TOTAL_RAIL_CHANNEL_DATA";
            case SERVER_TOTAL_RAIL_CHANNEL_DATA:   return "SERVER_TOTAL_RAIL_CHANNEL_DATA";
            case CLIENT_TOTAL_OTHERS_CHANNEL_DATA: return "CLIENT_TOTAL_OTHERS_CHANNEL_DATA";
            case SERVER_TOTAL_OTHERS_CHANNEL_DATA: return "SERVER_TOTAL_OTHERS_CHANNEL_DATA";
            case COUNT_FIELD: break;
        }

        assert(false);
        return "unknow_rdp_metrics_name";
    }

    const char * rdp_protocol_name = "rdp";

    Metrics * metrics;

    // RDP context Info
    int last_x = -1;
    int last_y = -1;
    uint32_t file_contents_format_ID = 0;
    uint32_t last_formatID = 0;
    bool cliprdr_init_format_list_done = false;
    bool use_long_format_names     = true;

    uint32_t flag_filecontents = 0;


public:
    RDPMetrics(Metrics * metrics) : metrics(metrics)
    {
        this->metrics->set_protocol("v1.0", this->rdp_protocol_name, COUNT_FIELD);
        LOG(LOG_INFO, "starting RDP Metrics");
    }

    void server_other_channel_data(long int len) {
        this->metrics->add_to_current_data(SERVER_TOTAL_OTHERS_CHANNEL_DATA, len);
    }

    void client_other_channel_data(long int len) {
        this->metrics->add_to_current_data(CLIENT_TOTAL_OTHERS_CHANNEL_DATA, len);
    }

    void server_rail_channel_data(long int len) {
        this->metrics->add_to_current_data(SERVER_TOTAL_RAIL_CHANNEL_DATA, len);
    }

    void client_rail_channel_data(long int len) {
        this->metrics->add_to_current_data(CLIENT_TOTAL_RAIL_CHANNEL_DATA, len);
    }

    void set_server_rdpdr_metrics(InStream chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics->add_to_current_data(SERVER_DISK_REDIR_CHANNEL_DATA, length);

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
                            this->metrics->add_to_current_data(FILES_READ_COUNT, 1);
                        }
                        this->metrics->add_to_current_data(TOTAL_READ_DATA, drr.Length());
                        break;
                    }

                    case rdpdr::IRP_MJ_WRITE:
                    {
                        rdpdr::DeviceWriteRequest dwr;
                        dwr.receive(chunk);
                        if (dwr.Offset == 0) {
                            this->metrics->add_to_current_data(FILES_WRITE_COUNT, 1);
                        }
                        this->metrics->add_to_current_data(TOTAL_FILES_WRITTEN_DATA, dwr.Length);
                        break;
                    }

                    case rdpdr::IRP_MJ_SET_INFORMATION:     // delete
                    {
                        rdpdr::ServerDriveSetInformationRequest sdsir;
                        sdsir.receive(chunk);

                        switch (sdsir.FsInformationClass()) {
                            case rdpdr::FileRenameInformation:
                                this->metrics->add_to_current_data(FILES_RENAME_COUNT, 1);
                                break;
                            case rdpdr::FileDispositionInformation:
                                this->metrics->add_to_current_data(FILES_OR_FOLDERS_DELEDTE_COUNT, 1);
                                break;
                        }
                        break;
                    }
                }
            }
        }
    }

    void set_client_rdpdr_metrics(InStream const & /*chunk*/, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics->add_to_current_data(CLIENT_DISK_REDIR_CHANNEL_DATA, length);
        }
    }

    void set_server_cliprdr_metrics(InStream chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics->add_to_current_data(SERVER_CLIPBOARD_CHANNEL_DATA, length);
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            switch (header.msgType()) {
                case RDPECLIP::CB_FORMAT_LIST:
                    this->format_list_process(
                        chunk, header,
                        SERVER_TEXT_COPY_COUNT,
                        SERVER_IMAGE_COPY_COUNT,
                        SERVER_FILE_COPY_COUNT);
                    break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                    this->process_format_data_request(
                        chunk,
                        ON_SERVER_TEXT_PASTE_COUNT,
                        ON_SERVER_IMAGE_PASTE_COUNT,
                        ON_SERVER_FILE_PASTE_COUNT);
                    break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                    this->process_format_data_response(header, ON_CLIENT_TOTAL_DATA_PASTE);
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                    this->process_filecontents_request(chunk);
                    break;

                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                    this->process_filecontents_response(chunk, ON_CLIENT_TOTAL_DATA_PASTE);
                    break;
            }
        }
    }

    void set_client_cliprdr_metrics(InStream chunk, size_t length, uint32_t flags) {
        if (bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            this->metrics->add_to_current_data(CLIENT_CLIPBOARD_CHANNEL_DATA, length);
            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            switch (header.msgType()) {
                case RDPECLIP::CB_CLIP_CAPS:
                {
                    RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu;
                    clipboard_caps_pdu.recv(chunk);
                    assert(1 == clipboard_caps_pdu.cCapabilitiesSets());

                    RDPECLIP::CapabilitySetRecvFactory cliboard_cap_set_recv_factory(chunk);
                    assert(RDPECLIP::CB_CAPSTYPE_GENERAL == cliboard_cap_set_recv_factory.capabilitySetType());

                    RDPECLIP::GeneralCapabilitySet general_cap_set;
                    general_cap_set.recv(chunk, cliboard_cap_set_recv_factory);

                    this->use_long_format_names = bool(general_cap_set.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
                }
                    break;

                case RDPECLIP::CB_FORMAT_LIST:
                    if (this->cliprdr_init_format_list_done) {
                        this->format_list_process(
                            chunk, header,
                            CLIENT_TEXT_COPY_COUNT,
                            CLIENT_IMAGE_COPY_COUNT,
                            CLIENT_FILE_COPY_COUNT);
                    } else {
                        this->cliprdr_init_format_list_done = true;
                    }
                    break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                    this->process_format_data_request(
                        chunk,
                        ON_CLIENT_TEXT_PASTE_COUNT,
                        ON_CLIENT_IMAGE_PASTE_COUNT,
                        ON_CLIENT_FILE_PASTE_COUNT);
                    break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                    this->process_format_data_response(header, ON_SERVER_TOTAL_DATA_PASTE);
                    break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                    this->process_filecontents_request(chunk);
                    break;

                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                    this->process_filecontents_response(chunk, ON_SERVER_TOTAL_DATA_PASTE);
                    break;
            }
        }
    }

    void server_main_channel_data(long int len) {
        this->metrics->add_to_current_data(SERVER_MAIN_CHANNEL_DATA, len);
    }

    void mouse_move(const int x, const int y) {
        if (this->last_x >= 0 && this->last_y >= 0) {
            int x_shift = x - this->last_x;
            if (x_shift < 0) {
                x_shift *= -1;
            }
            int y_shift = y - this->last_y;
            if (y_shift < 0) {
                y_shift *= -1;
            }
            this->metrics->add_to_current_data(MOUSE_DISPLACEMENT, x_shift + y_shift);
        }
        this->last_x = x;
        this->last_y = y;
    }

    void key_pressed() {
        this->metrics->add_to_current_data(KEYS_PRESSED, 1);
    }

    void right_click_pressed() {
        this->metrics->add_to_current_data(RIGHT_CLICK, 1);
    }

    void left_click_pressed() {
        this->metrics->add_to_current_data(LEFT_CLICK, 1);
    }

    void client_main_channel_data(long int len) {
        this->metrics->add_to_current_data(CLIENT_MAIN_CHANNEL_DATA, len);
    }

private:
    void format_list_process(
        InStream& chunk, RDPECLIP::CliprdrHeader const& header,
        FieldIndex nb_copy_text, FieldIndex nb_copy_image, FieldIndex nb_copy_file)
    {
        RDPECLIP::FormatListPDUEx format_list_pdu;
        format_list_pdu.recv(chunk, this->use_long_format_names, (header.msgFlags() & RDPECLIP::CB_ASCII_NAMES));

        for (RDPECLIP::FormatName const & format_name_local : format_list_pdu) {
            format_name_local.log(LOG_INFO);

            switch (uint32_t formatID = format_name_local.formatId()) {
                case RDPECLIP::CF_TEXT:
                case RDPECLIP::CF_LOCALE:
                case RDPECLIP::CF_UNICODETEXT:
                case RDPECLIP::CF_OEMTEXT:
                    this->metrics->add_to_current_data(nb_copy_text, 1);
                    return;
                case RDPECLIP::CF_METAFILEPICT:
                    this->metrics->add_to_current_data(nb_copy_image, 1);
                    return;
                default:
                    // TODO string_view
                    if (format_name_local.format_name() == RDPECLIP::FILEGROUPDESCRIPTORW.data()) {
                        this->file_contents_format_ID = formatID;
                        this->metrics->add_to_current_data(nb_copy_file, 1);
                        return;
                    }
                    break;
            }
        }
    }

    void process_format_data_request(
        InStream& chunk,
        FieldIndex nb_paste_text, FieldIndex nb_paste_image, FieldIndex nb_paste_file)
    {
        this->last_formatID = chunk.in_uint32_le();

        switch (this->last_formatID) {
            case RDPECLIP::CF_TEXT:
            case RDPECLIP::CF_OEMTEXT:
            case RDPECLIP::CF_UNICODETEXT:
            case RDPECLIP::CF_DSPTEXT:
            case RDPECLIP::CF_LOCALE:
                this->metrics->add_to_current_data(nb_paste_text, 1);
                break;
            case RDPECLIP::CF_METAFILEPICT:
            case RDPECLIP::CF_DSPMETAFILEPICT:
                this->metrics->add_to_current_data(nb_paste_image, 1);
                break;
            default:
                if (this->file_contents_format_ID == this->last_formatID){
                    this->metrics->add_to_current_data(nb_paste_file, 1);
                }
                break;
        }
    }

    void process_format_data_response(
        RDPECLIP::CliprdrHeader const& header,
        FieldIndex total_data_paste)
    {
        switch (this->last_formatID) {
            case RDPECLIP::CF_TEXT:
            case RDPECLIP::CF_OEMTEXT:
            case RDPECLIP::CF_UNICODETEXT:
            case RDPECLIP::CF_DSPTEXT:
            case RDPECLIP::CF_LOCALE:
                this->metrics->add_to_current_data(total_data_paste, header.dataLen());
                break;
            case RDPECLIP::CF_METAFILEPICT:
            case RDPECLIP::CF_DSPMETAFILEPICT:
                this->metrics->add_to_current_data(total_data_paste, header.dataLen());
                break;
            default:
                break;
        }
    }

    void process_filecontents_request(InStream& chunk)
    {
        chunk.in_skip_bytes(8); // streamId(4 bytes) + lindex(4 bytes)
        this->flag_filecontents = chunk.in_uint32_le();
    }

    void process_filecontents_response(InStream& chunk, FieldIndex total_data_paste)
    {
        if (this->flag_filecontents == RDPECLIP::FILECONTENTS_SIZE) {
            chunk.in_skip_bytes(4);             // streamId(4 bytes)
            uint32_t nPositionLow = chunk.in_uint32_le();
            uint64_t nPositionHigh = chunk.in_uint32_le();
            this->metrics->add_to_current_data(
                total_data_paste, nPositionLow + (nPositionHigh << 32));
        }
    }
};
