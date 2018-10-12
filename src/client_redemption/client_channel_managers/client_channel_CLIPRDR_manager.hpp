/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#pragma once

#include <unordered_map>

#include "utils/log.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/fileutils.hpp"
#include "core/RDP/clipboard.hpp"

#include "client_redemption/mod_wrapper/client_channel_mod.hpp"
#include "client_redemption/client_input_output_api/client_clipboard_api.hpp"
#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"

#include "mod/rdp/rdp_verbose.hpp"



// [MS-RDPECLIP]: Remote Desktop Protocol: CLIpboard Virtual Channel Extension
//
//
// 1.3.2.1 Initialization Sequence
//
// The goal of the Initialization Sequence is to establish the client and the server capabilities, exchange settings, and synchronize the initial state of the client and server clipboards.
//
// +-----------+                                                 +-----------+
// |  Client   |                                                 |  Server   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       | <------------Server Clipboard Capabilities PDU--------------+
//       |                                                             |
//       | <-------------------Monitor Ready PDU-----------------------+
//       |                                                             |
//       +--------------Client Clipboard Capabilities PDU------------> |
//       |                                                             |
//       +------------------Temporary Directory PDU------------------> | (optional)
//       |                                                             |
//       +---------------------Format List PDU-----------------------> |
//       |                                                             |
//       | <----------------Format List Response PDU-------------------+
//       |                                                             |
//
// Figure 1: Clipboard Redirection Initialization Sequence
//
//     The server sends a Clipboard Capabilities PDU to the client to advertise the capabilities that it supports.
//
//      The server sends a Monitor Ready PDU to the client.
//
//     Upon receiving the Monitor Ready PDU, the client transmits its capabilities to the server by using a Clipboard Capabilities PDU.
//
//     The client sends the Temporary Directory PDU to inform the server of a location on the client file system that can be used to deposit files being copied to the client. To make use of this location, the server has to be able to access it directly. At this point, the client and the server capability negotiation is complete.
//
//     The final stage of the Initialization Sequence involves synchronizing the Clipboard Formats on the server clipboard with the client. This is accomplished by effectively mimicking a copy operation on the client by forcing it to send a Format List PDU.
//
//     The server responds with a Format List Response PDU.
//
//
// 1.3.2.2 Data Transfer Sequences
//
// The goal of the Data Transfer Sequences is to perform a copy or paste operation. The diagram that follows presents a possible data transfer sequence.
//
// +-----------+                                                 +-----------+
// |  Shared   |                                                 |   Local   |
// | Clipboard |                                                 | Clipboard |
// |   Owner   |                                                 |   Owner   |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +------------------------Format List PDU--------------------> |
//       |                                                             |
//       | <-----------------Format List Response PDU------------------+
//       |                                                             |
//       | <------------Lock Clipboard Data PDU (Optional)-------------+
//       |                                                             |
//       | <-----------------Format Data Request PDU-------------------+
//       |                                                             |
//       +-------------------Format Data Response PDU----------------> |
//       |                                                             |
//       | <----------------File Contents Request PDU------------------+ (if file data)
//       |                                                             |
//       +-----------------File Contents Response PDU----------------> | (if file data)
//       |                                                             |
//       | <------------Unlock Clipboard Data PDU (Optional)-----------+
//       |                                                             |
//
// Figure 2: Data transfer using the shared clipboard
//
//     The sequence of messages for a copy operation is the same for all format
//     types, as specified in section 1.3.2.2.1.
//
//     However, the messages exchanged to transfer File Stream data during a
//     paste operation differs from those used to transfer other format data,
//     as specified in section 1.3.2.2.3.



class ClientChannelCLIPRDRManager
{

public:
    RDPVerbose verbose;
    ClientIOClipboardAPI * clientIOClipboardAPI;
    ClientChannelMod * callback;

    enum : int {
        PASTE_TEXT_CONTENT_SIZE = CHANNELS::CHANNEL_CHUNK_LENGTH - 8
      , PASTE_PIC_CONTENT_SIZE  = CHANNELS::CHANNEL_CHUNK_LENGTH - RDPECLIP::METAFILE_HEADERS_SIZE - 8
    };
    bool format_list_nego_done = false;

    uint32_t             _requestedFormatId = 0;
    std::string          _requestedFormatName;
    bool                 _waiting_for_data;

    timeval paste_data_request_time;
    long paste_data_len = 0;

    const uint32_t channel_flags;
    std::unordered_map<uint32_t, std::string> formats_map;
    const double arbitrary_scale;

    uint32_t file_content_flag;

    const std::string path;


    struct CB_Buffers {
        std::unique_ptr<uint8_t[]>  data = nullptr;
        size_t size = 0;
        size_t sizeTotal = 0;
        int    pic_width = 0;
        int    pic_height = 0;
        int    pic_bpp = 0;

    } _cb_buffers;

    struct CB_FilesList {
        struct CB_in_Files {
            int         size;
            std::string name;
        };
        uint32_t                 cItems = 0;
        uint32_t                 lindexToRequest = 0;
        int                      streamIDToRequest = 0;
        std::vector<CB_in_Files> itemslist;
        int                      lindex = 0;

    }  _cb_filesList;

    // Config negociation
    bool server_use_long_format_names = true;
    const uint16_t cCapabilitiesSets = 1;
    uint32_t generalFlags;

    RDPECLIP::FormatListPDUEx format_list_pdu;


    ClientChannelCLIPRDRManager(RDPVerbose verbose, ClientChannelMod * callback, ClientIOClipboardAPI * clientIOClipboardAPI, RDPClipboardConfig const& config);

    ~ClientChannelCLIPRDRManager();

private:
    void add_format(uint32_t ID, const std::string & name);

public:

    void receive(InStream & chunk, int flags);

    void process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList);

    void send_textBuffer_to_clipboard(bool is_utf16);
    void send_to_clipboard_Buffer(InStream & chunk);

    void send_imageBuffer_to_clipboard();

    void empty_buffer();

    void emptyLocalBuffer();
    void send_FormatListPDU();

    void send_UnlockPDU(uint32_t streamID);

};

