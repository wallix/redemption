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

#include "utils/log.hpp"
#include "client_redemption/client_input_output_api.hpp"
#include "core/RDP/clipboard.hpp"



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
//       +------------------Temporary Directroy PDU------------------> | (optional)
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
//     The sequence of messages for a copy operation is the same for all format types, as specified in section 1.3.2.2.1.
//
//     However, the messages exchanged to transfer File Stream data during a paste operation differs from those used to transfer other format data, as specified in section 1.3.2.2.3.



class ClientChannelCLIPRDRManager {

public:
    RDPVerbose verbose;
    ClientIOClipboardAPI * clientIOClipboardAPI;
    ClientRedemptionAPI * client;

    enum : int {
       PDU_HEADER_SIZE =    8
    };

    enum : int {
        PASTE_TEXT_CONTENT_SIZE = CHANNELS::CHANNEL_CHUNK_LENGTH - PDU_HEADER_SIZE
      , PASTE_PIC_CONTENT_SIZE  = CHANNELS::CHANNEL_CHUNK_LENGTH - RDPECLIP::METAFILE_HEADERS_SIZE - PDU_HEADER_SIZE
    };



    struct ClipbrdFormatsList{

        enum : uint16_t {
              CF_QT_CLIENT_FILEGROUPDESCRIPTORW = 48025
            , CF_QT_CLIENT_FILECONTENTS         = 48026
        };

        enum : int {
              CLIPBRD_FORMAT_COUNT = 4
        };

        size_t size = 0;

        const std::string FILECONTENTS;
        const std::string FILEGROUPDESCRIPTORW;
        uint32_t          IDs[CLIPBRD_FORMAT_COUNT];
        std::string       names[CLIPBRD_FORMAT_COUNT];
        int index = 0;
        const double      ARBITRARY_SCALE;  //  module MetaFilePic resolution, value=40 is
                                            //  empirically close to original resolution.

        ClipbrdFormatsList()
          : FILECONTENTS(
              "F\0i\0l\0e\0C\0o\0n\0t\0e\0n\0t\0s\0\0\0"
            , 26)
          , FILEGROUPDESCRIPTORW(
              "F\0i\0l\0e\0G\0r\0o\0u\0p\0D\0e\0s\0c\0r\0i\0p\0t\0o\0r\0W\0\0\0"
            , 42)
          , ARBITRARY_SCALE(40)
        {}

        void add_format(uint32_t ID, const std::string & name) {
            if (this->size < CLIPBRD_FORMAT_COUNT) {
                this->IDs[size]   = ID;
                this->names[size] = name;
                this->size++;
            }
        }

    } clipbrdFormatsList;

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

    uint32_t             _requestedFormatId = 0;
    std::string          _requestedFormatName;
    bool                 _waiting_for_data;


    timeval paste_data_request_time;
    long paste_data_len = 0;

    bool server_use_long_format_names;



    ClientChannelCLIPRDRManager(RDPVerbose verbose, ClientRedemptionAPI * client, ClientIOClipboardAPI * clientIOClipboardAPI)
      : verbose(verbose)
      , clientIOClipboardAPI(clientIOClipboardAPI)
      , client(client)
      , _cb_buffers()
      , _cb_filesList()
      , _waiting_for_data(false)
      , server_use_long_format_names(true)
      {
          this->clipbrdFormatsList.index = 0;
            DIR *pDir;
            pDir = opendir (this->client->CB_TEMP_DIR.c_str());

            if (!pDir) {
                mkdir(this->client->CB_TEMP_DIR.c_str(), 0777);
            }

            pDir = opendir (this->client->CB_TEMP_DIR.c_str());

            if (pDir) {

                this->clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS
                                                , this->clipbrdFormatsList.FILECONTENTS
                                                );
                this->clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW
                                                , this->clipbrdFormatsList.FILEGROUPDESCRIPTORW
                                                );
//                 this->clipbrdFormatsList.add_format( RDPECLIP::CF_UNICODETEXT
//                                                 , std::string("\0\0", 2)
//                                                 );
                this->clipbrdFormatsList.add_format( RDPECLIP::CF_TEXT
                                                , std::string("\0\0", 2)
                                                );
                this->clipbrdFormatsList.add_format( RDPECLIP::CF_METAFILEPICT
                                                , std::string("\0\0", 2)
                                                );
            } else {
                LOG(LOG_WARNING, "Can't enable shared clipboard, %s directory doesn't exist.", this->client->CB_TEMP_DIR);
            }
    }

    ~ClientChannelCLIPRDRManager() {
        this->empty_buffer();
    }

// MS-RDPECLIP

// 2.2.1 Clipboard PDU Header (CLIPRDR_HEADER)

// The CLIPRDR_HEADER structure is present in all clipboard PDUs. It is used to identify the PDU type,
//  specify the length of the PDU, and convey message flags.

// msgType : 16 bits
// msgFlags : 16 bits
// dataLen : 32 bits integer

//             Value                    Description
// ------------------------------------+-------------------------------
//    CB_MONITOR_READY         0x0001  |   Monitor Ready PDU
// ------------------------------------+-------------------------------
//    CB_FORMAT_LIST           0x0002  |   Format List PDU
// ------------------------------------+-------------------------------
//    CB_FORMAT_LIST_RESPONSE  0x0003  |   Format List Response PDU
// ------------------------------------+-------------------------------
//    CB_FORMAT_DATA_REQUEST   0x0004  |   Format Data Request PDU
// ------------------------------------+-------------------------------
//    CB_FORMAT_DATA_RESPONSE  0x0005  |   Format Data Response PDU
// ------------------------------------+-------------------------------
//    CB_TEMP_DIRECTORY        0x0006  |   Temporary Directory PDU
// ------------------------------------+-------------------------------
//    CB_CLIP_CAPS             0x0007  |   Clipboard Capabilities PDU
// ------------------------------------+-------------------------------
//    CB_FILECONTENTS_REQUEST  0x0008  |   File Contents Request PDU
// ------------------------------------+-------------------------------
//    CB_FILECONTENTS_RESPONSE 0x0009  |   File Contents Response PDU
// ------------------------------------+-------------------------------
//    CB_LOCK_CLIPDATA         0x000A  |  Lock Clipboard Data PDU
// ------------------------------------+-------------------------------
//    CB_UNLOCK_CLIPDATA       0x000B  |  Unlock Clipboard Data PDU
// ------------------------------------+-------------------------------

// msgFlags (2 bytes): An unsigned, 16-bit integer that indicates message flags.

//    CB_RESPONSE_OK      0x0001        Used by the Format List Response PDU, Format Data Response PDU,
//                                      and File Contents Response PDU to indicate that the associated
//                                      request Format List PDU, Format Data Request PDU, and File
//                                      Contents Request PDU were processed successfully.

//    CB_RESPONSE_FAIL    0x0002        Used by the Format List Response PDU, Format Data Response PDU,
//                                      and File Contents Response PDU to indicate that the associated
//                                      Format List PDU, Format Data Request PDU, and File Contents
//                                      Request PDU were not processed successfully.

//    CB_ASCII_NAMES      0x0004        Used by the Short Format Name variant of the Format List Response
//                                      PDU to indicate that the format names are in ASCII 8.

// dataLen (4 bytes): An unsigned, 32-bit integer that specifies the size, in bytes, of the data which
//  follows the Clipboard PDU Header.<1>

    void receive(InStream & chunk, int flags) {
        if (clientIOClipboardAPI) {
        InStream chunk_series = chunk.clone();

        if (!chunk.in_check_rem(2  /*msgType(2)*/ )) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_message: "
                        "Truncated msgType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            const uint16_t server_message_type = chunk.in_uint16_le();

            if (!this->_waiting_for_data) {
                switch (server_message_type) {
                    case RDPECLIP::CB_CLIP_CAPS:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Clipboard Capabilities PDU");
                        }
                        {
                        RDPECLIP::ClipboardCapabilitiesPDU pdu;
                        pdu.recv(chunk_series);

                        RDPECLIP::GeneralCapabilitySet pdu2;
                        pdu2.recv(chunk_series);
                        this->server_use_long_format_names = bool(pdu2.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
                        }
                    break;


//    2.2.2.2 Server Monitor Ready PDU (CLIPRDR_MONITOR_READY)

//    The Monitor Ready PDU is sent from the server to the client to indicate that the server is
//    initialized and ready. This PDU is transmitted by the server after it has sent the Clipboard
//    Capabilities PDU to the client.
//
//    clipHeader (8 bytes):  A Clipboard PDU Header. The msgType field of the Clipboard PDU Header
//    MUST be set to CB_MONITOR_READY (0x0001), while the msgFlags field MUST be set to 0x0000.

                    case RDPECLIP::CB_MONITOR_READY:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Monitor Ready PDU");
                        }

                        {
                            RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1, RDPECLIP::GeneralCapabilitySet::size());
                            uint32_t generalFlags = RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS;
                            if (this->server_use_long_format_names) {
                                generalFlags = generalFlags | RDPECLIP::CB_USE_LONG_FORMAT_NAMES;
                            }
                            RDPECLIP::GeneralCapabilitySet general_cap_set(RDPECLIP::CB_CAPS_VERSION_2, generalFlags);

                            StaticOutStream<1024> out_stream;
                            clipboard_caps_pdu.emit(out_stream);
                            general_cap_set.emit(out_stream);

                            const uint32_t total_length = out_stream.get_offset();
                            InStream chunk(out_stream.get_data(), total_length);

                            this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk
                                                                , total_length
                                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST
                                                                |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );

                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "CLIENT >> CB Channel: Clipboard Capabilities PDU");
                            }
                        }

                        // TODO: check this, dangerous, we don't know how many formats are really available
                        // sizes array seems to be the length of names, this should no be managed separately from names
                        if (this->server_use_long_format_names) {
                            const uint16_t * names[] = {
                                        reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[0].data()),
                                        reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[1].data()),
                                        reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[2].data()),
                                        reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[3].data())
                                        //reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[4].data())
                                                       };

                            size_t sizes[] = {26, 42, 2, 2, /*2*/};

                            this->send_FormatListPDU(this->clipbrdFormatsList.IDs, names, sizes, this->clipbrdFormatsList.size);
                        } else {
                            const uint16_t * names[] = {
                                        reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[2].data()),
                                        reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[3].data())
                                        //reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[4].data())
                                                       };

                            size_t sizes[] = {2, 2, /*2*/};

                            // TODO: check documentation, what are we setting from IDs array ?
                            //       before In changed IDs array size, there was no IDs[4]!!!!
                            uint32_t ids[] = { this->clipbrdFormatsList.IDs[2]
                                             , this->clipbrdFormatsList.IDs[3]
//                                             , this->clipbrdFormatsList.IDs[4]
                                             };
                            LOG(LOG_INFO, "short format to send");
                            this->send_FormatListPDU(ids, names, sizes, 2);
                        }

                    break;

    // 2.2.3.2 Format List Response PDU (FORMAT_LIST_RESPONSE)

    // The Format List Response PDU is sent as a reply to the Format List PDU. It is used to indicate
    // whether processing of the Format List PDU was successful.

    // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST
    // be set to CB_FORMAT_LIST_RESPONSE (0x0003). The CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002)
    // flag MUST be set in the msgFlags field of the Clipboard PDU Header.

                    case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: Format List Response PDU FAILED");
                            } else {
                                LOG(LOG_INFO, "SERVER >> CB Channel: Format List Response PDU");
                            }
                        }

                    break;

    // 2.2.3.1 Format List PDU (CLIPRDR_FORMAT_LIST)

    // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be
    // set to CB_FORMAT_LIST (0x0002), while the msgFlags field MUST be set to 0x0000 or CB_ASCII_NAMES (0x0004)
    // depending on the type of data present in the formatListData field.

    // formatListData (variable): An array consisting solely of either Short Format Names or Long Format Names.
    // The type of structure used in the array is determined by the presence of the CB_USE_LONG_FORMAT_NAMES (0x00000002)
    // flag in the generalFlags field of the General Capability Set (section 2.2.2.1.1.1).

    // Each array holds a list of the Clipboard Format ID and name pairs available on the local system clipboard
    // of the sender.

    // If Short Format Names are being used, and the embedded Clipboard Format names are in ASCII 8 format, then
    // the msgFlags field of the clipHeader must contain the CB_ASCII_NAMES (0x0004) flag.

                    case RDPECLIP::CB_FORMAT_LIST:
                        {
                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: Format List PDU FAILED");
                            } else {
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "SERVER >> CB Channel: Format List PDU");
                                }

                                int formatAvailable = chunk.in_uint32_le();

                                bool isSharedFormat = false;

                                while (formatAvailable > 0) {
                                    uint32_t formatID = chunk.in_uint32_le();
                                    formatAvailable -=  4;

                                    if (!this->server_use_long_format_names) {
                                        formatAvailable -=  32;
                                        uint8_t utf16_string[32];
                                        chunk.in_copy_bytes(utf16_string, 32);
                                        this->_requestedFormatName = std::string(reinterpret_cast<const char*>(utf16_string), 32);
                                    } else {

                                        uint16_t utf16_string[120];
                                        int k(0);
                                        bool isEndString = false;
                                        while (!isEndString) {
                                            u_int16_t bit(chunk.in_uint16_le());
                                            if (bit == 0) {
                                                isEndString = true;
                                            }
                                            utf16_string[k] = bit;
                                            k++;

                                            formatAvailable -=  2;
                                        }
                                        this->_requestedFormatName = std::string(reinterpret_cast<const char*>(utf16_string), k*2);
                                    }


                                    for (int j = 0; j < ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT && !isSharedFormat; j++) {
                                        if (this->clipbrdFormatsList.IDs[j] == formatID) {
                                            this->_requestedFormatId = formatID;
                                            isSharedFormat = true;
                                            formatAvailable = 0;
                                        }
                                    }

                                    if (this->_requestedFormatName == this->clipbrdFormatsList.FILEGROUPDESCRIPTORW && !isSharedFormat) {
                                        this->_requestedFormatId = formatID;
                                        isSharedFormat = true;
                                        formatAvailable = 0;
                                    }
                                }

                                RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
                                StaticOutStream<256> out_stream;
                                formatListResponsePDU.emit(out_stream);
                                InStream chunk(out_stream.get_data(), out_stream.get_offset());

                                this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                              , chunk
                                                              , out_stream.get_offset()
                                                              , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                              );
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Format List Response PDU");
                                }

                                RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
                                StaticOutStream<32> out_stream_lock;
                                lockClipboardDataPDU.emit(out_stream_lock);
                                InStream chunk_lock(out_stream_lock.get_data(), out_stream_lock.get_offset());

                                this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                              , chunk_lock
                                                              , out_stream_lock.get_offset()
                                                              , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                              );
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Lock Clipboard Data PDU");
                                }

                                RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(this->_requestedFormatId);
                                StaticOutStream<256> out_streamRequest;
                                formatDataRequestPDU.emit(out_streamRequest);
                                InStream chunkRequest(out_streamRequest.get_data(), out_streamRequest.get_offset());

                                this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                              , chunkRequest
                                                              , out_streamRequest.get_offset()
                                                              , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                              );
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Request PDU");
                                }

                                this->paste_data_request_time = tvtime();
                            }
                        }
                    break;

                    case RDPECLIP::CB_LOCK_CLIPDATA:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Lock Clipboard Data PDU");
                        }
                    break;

                    case RDPECLIP::CB_UNLOCK_CLIPDATA:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Unlock Clipboard Data PDU");
                        }
                    break;

                    case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                        if(this->_requestedFormatName == this->clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                            this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                        }

                        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: Format Data Response PDU FAILED");
                            } else {
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Response PDU");
                                }

                                this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
                            }
                        }
                    break;

                    case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                    {
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_WARNING, "SERVER >> CB Channel: Format Data Request PDU FAILED");
                        } else {
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Request PDU");
                            }

                            chunk.in_skip_bytes(4);
                            int first_part_data_size(0);
                            uint32_t total_length(this->clientIOClipboardAPI->get_cliboard_data_length() + PDU_HEADER_SIZE);
                            StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;

                            if (this->clientIOClipboardAPI->get_buffer_type_id() == chunk.in_uint32_le()) {

                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Response PDU");
                                }



                                switch(this->clientIOClipboardAPI->get_buffer_type_id()) {

                                    case RDPECLIP::CF_METAFILEPICT:
                                    {
                                        first_part_data_size = this->clientIOClipboardAPI->get_cliboard_data_length();
                                        if (first_part_data_size > PASTE_PIC_CONTENT_SIZE) {
                                            first_part_data_size = PASTE_PIC_CONTENT_SIZE;
                                        }
                                        total_length += RDPECLIP::METAFILE_HEADERS_SIZE;
                                        auto image = this->clientIOClipboardAPI->get_image();
                                        RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr(
                                              this->clientIOClipboardAPI->get_cliboard_data_length()
                                            , image.width()
                                            , image.height()
                                            , image.bits_per_pixel()
                                            , this->clipbrdFormatsList.ARBITRARY_SCALE
                                        );
                                        fdr.emit(out_stream_first_part);

                                        this->process_client_clipboard_out_data(
                                              channel_names::cliprdr
                                            , total_length
                                            , out_stream_first_part
                                            , first_part_data_size
                                            , image.data()
                                            , this->clientIOClipboardAPI->get_cliboard_data_length() + RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender::SIZE
                                            , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                        );
                                    }
                                    break;

                                    case RDPECLIP::CF_TEXT:
                                    case RDPECLIP::CF_UNICODETEXT:
                                    {
                                        first_part_data_size = this->clientIOClipboardAPI->get_cliboard_data_length();
                                        if (first_part_data_size > PASTE_TEXT_CONTENT_SIZE ) {
                                            first_part_data_size = PASTE_TEXT_CONTENT_SIZE;
                                        }

                                        RDPECLIP::FormatDataResponsePDU_Text fdr(this->clientIOClipboardAPI->get_cliboard_data_length());

                                        fdr.emit(out_stream_first_part);

                                        auto image = this->clientIOClipboardAPI->get_image();
                                        this->process_client_clipboard_out_data(
                                            channel_names::cliprdr
                                          , total_length
                                          , out_stream_first_part
                                          , first_part_data_size
                                          , image.data()
                                          , this->clientIOClipboardAPI->get_cliboard_data_length()
                                          , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                        );
                                    }
                                    break;

                                    case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:
                                    {
                                        int data_sent(0);
                                        first_part_data_size = PDU_HEADER_SIZE + 4;
                                        total_length = (RDPECLIP::FileDescriptor::size() * this->clientIOClipboardAPI->get_citems_number()) + 8 + PDU_HEADER_SIZE;
                                        int flag_first(CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                                        //ClipBoard_Qt::CB_out_File file = this->clientIOClipboardAPI->_items_list[0];
                                        RDPECLIP::FormatDataResponsePDU_FileList fdr(this->clientIOClipboardAPI->get_citems_number());
                                        fdr.emit(out_stream_first_part);

                                        RDPECLIP::FileDescriptor fdf(
                                            this->clientIOClipboardAPI->get_file_item_name(0)
                                          , this->clientIOClipboardAPI->get_file_item_size(0)
                                          , fscc::FILE_ATTRIBUTE_ARCHIVE
                                        );
                                        fdf.emit(out_stream_first_part);

                                        if (this->clientIOClipboardAPI->get_citems_number() == 1) {
                                            flag_first = flag_first | CHANNELS::CHANNEL_FLAG_LAST;
                                            out_stream_first_part.out_uint32_le(0);
                                            data_sent += 4;
                                        }
                                        InStream chunk_first_part( out_stream_first_part.get_data()
                                                                , out_stream_first_part.get_offset()
                                                                );

                                        this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                                            , chunk_first_part
                                                                            , total_length
                                                                            , flag_first
                                                                            );
                                        data_sent += first_part_data_size + RDPECLIP::FileDescriptor::size();
                                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                            LOG(LOG_INFO, "CLIENT >> CB Channel: Data PDU %d/%u", data_sent, total_length);
                                        }

                                        for (int i = 1; i < this->clientIOClipboardAPI->get_citems_number(); i++) {

                                            StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                                            //file = this->clientIOClipboardAPI->_items_list[i];
                                            RDPECLIP::FileDescriptor fdn(
                                                this->clientIOClipboardAPI->get_file_item_name(i)
                                              , this->clientIOClipboardAPI->get_file_item_size(i)
                                              , fscc::FILE_ATTRIBUTE_ARCHIVE
                                              );
                                            int flag_next(CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

                                            fdn.emit(out_stream_next_part);

                                            if (i == this->clientIOClipboardAPI->get_citems_number() - 1) {
                                                flag_next = flag_next | CHANNELS::CHANNEL_FLAG_LAST;
                                                out_stream_next_part.out_uint32_le(0);
                                                data_sent += 4;
                                            }

                                            InStream chunk_next_part( out_stream_next_part.get_data()
                                                                    , out_stream_next_part.get_offset()
                                                                    );

                                            this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                                          , chunk_next_part
                                                                          , total_length
                                                                          , flag_next
                                                                          );

                                            data_sent += RDPECLIP::FileDescriptor::size();
                                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                                LOG(LOG_INFO, "CLIENT >> CB Channel: Data PDU %d/%u", data_sent, total_length);
                                            }
                                        }
                                    }
                                    break;

                                    default: LOG(LOG_WARNING, "SERVER >> CB Channel: unknow CB format ID %x", this->clientIOClipboardAPI->get_buffer_type_id());
                                    break;
                                }
                            }
                        }
                    }
                    break;

                    case RDPECLIP::CB_FILECONTENTS_REQUEST:
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Resquest PDU FAIL");
                        } else {
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Resquest PDU");
                            }

                            chunk.in_skip_bytes(4);                 // data_len
                            int streamID(chunk.in_uint32_le());
                            int lindex(chunk.in_uint32_le());

                            switch (chunk.in_uint32_le()) {         // flag

                                case RDPECLIP::FILECONTENTS_SIZE :
                                {
                                    StaticOutStream<32> out_stream;
                                    RDPECLIP::FileContentsResponse_Size fileSize(
                                        streamID
                                      , this->clientIOClipboardAPI->get_file_item_size(lindex)
                                      );
                                    fileSize.emit(out_stream);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
                                    this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                                  , chunk_to_send
                                                                  , out_stream.get_offset()
                                                                  , CHANNELS::CHANNEL_FLAG_LAST |
                                                                    CHANNELS::CHANNEL_FLAG_FIRST |  CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                  );

                                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                        LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU SIZE");
                                    }
                                }
                                break;

                                case RDPECLIP::FILECONTENTS_RANGE :
                                {
                                    StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;
                                    RDPECLIP::FileContentsResponse_Range fileRange(
                                        streamID
                                      , this->clientIOClipboardAPI->get_file_item_size(lindex));

                                    //this->clientIOClipboardAPI->get_cliboard_data_length() = this->clientIOClipboardAPI->get_file_item_size(lindex);
                                    int total_length(this->clientIOClipboardAPI->get_file_item_size(lindex) + 12);
                                    int first_part_data_size(this->clientIOClipboardAPI->get_file_item_size(lindex));
                                    if (first_part_data_size > CHANNELS::CHANNEL_CHUNK_LENGTH - 12) {
                                        first_part_data_size = CHANNELS::CHANNEL_CHUNK_LENGTH - 12;
                                    }
                                    fileRange.emit(out_stream_first_part);

                                    this->process_client_clipboard_out_data(
                                        channel_names::cliprdr
                                      , total_length
                                      , out_stream_first_part
                                      , first_part_data_size
                                      , reinterpret_cast<uint8_t *>(
                                        this->clientIOClipboardAPI->get_file_item_data(lindex))
                                      , this->clientIOClipboardAPI->get_file_item_size(lindex)
                                      , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                    );

                                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                        LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU RANGE");
                                    }
                                }
                                break;
                            }
                        }
                    break;

                    case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: File Contents Response PDU FAILED");
                            } else {
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU");
                                }

                                if(this->_requestedFormatName == this->clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                                    this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS;
                                    this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
                                }
                            }
                        }
                    break;

                    default:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "Process sever next part PDU data");
                        }
                        this->process_server_clipboard_indata(flags, chunk_series, this->_cb_buffers, this->_cb_filesList);


                    break;
                }

            } else {
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO, "Process sever next part PDU data");
                }
                this->process_server_clipboard_indata(flags, chunk_series, this->_cb_buffers, this->_cb_filesList);
            }
        }
    }

    void process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList) {

        // 3.1.5.2.2 Processing of Virtual Channel PDU

        // The Virtual Channel PDU is received by both the client and the server. Its structure
        // and fields are specified in section 2.2.6.1.

        // If Enhanced RDP Security (section 5.4) is in effect, the External Security Protocol (section 5.4.5)
        // being used to secure the connection MUST be used to decrypt and verify the integrity of the entire
        // PDU prior to any processing taking place.

        // The embedded length fields within the tpktHeader ([T123] section 8) and mcsPdu ([T125] section 7, parts
        // 7 and 10) fields MUST be examined for consistency with the received data. If there is any discrepancy,
        // the connection SHOULD be dropped.

        // The mcsPdu field encapsulates either an MCS Send Data Request PDU (if the PDU is being sent from client
        // to server) or an MCS Send Data Indication PDU (if the PDU is being sent from server to client). In both
        // of these cases, the embedded channelId field MUST contain the server-assigned virtual channel ID. This
        // ID MUST be used to route the data in the virtualChannelData field to the appropriate virtual channel
        // endpoint after decryption of the PDU and any necessary decompression of the payload has been conducted.

        // The conditions mandating the presence of the securityHeader field, as well as the type of Security
        // Header structure present in this field, are explained in section 2.2.6.1. If the securityHeader field is
        // present, the embedded flags field MUST be examined for the presence of the SEC_ENCRYPT (0x0008) flag
        // (section 2.2.8.1.1.2.1), and, if it is present, the data following the securityHeader field MUST be
        // verified and decrypted using the methods and techniques specified in section 5.3.6. If the MAC signature
        // is incorrect, or the data cannot be decrypted correctly, the connection SHOULD be dropped.

        // If the data in the virtualChannelData field is compressed, then the data MUST be decompressed using
        // the techniques detailed in section 3.1.8.3 (the Virtual Channel PDU compression flags are specified
        // in section 2.2.6.1.1).

        // If the embedded flags field of the channelPduHeader field (the Channel PDU Header structure is specified
        // in section 2.2.6.1.1) does not contain the CHANNEL_FLAG_FIRST (0x00000001) flag or CHANNEL_FLAG_LAST
        // (0x00000002) flag, and the data is not part of a chunked sequence (that is, a start chunk has not been
        // received), then the data in the virtualChannelData field can be dispatched to the appropriate virtual
        // channel endpoint (no reassembly is required by the endpoint). If the CHANNEL_FLAG_SHOW_PROTOCOL
        // (0x00000010) flag is specified in the Channel PDU Header, then the channelPduHeader field MUST also
        // be dispatched to the virtual channel endpoint.

        // If the virtual channel data is part of a sequence of chunks, then the instructions in section 3.1.5.2.2.1
        //MUST be followed to reassemble the stream.

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->_waiting_for_data = true;
            cb_buffers.sizeTotal = chunk.in_uint32_le();
            cb_buffers.data = std::make_unique<uint8_t[]>(cb_buffers.sizeTotal);
        }

        switch (this->_requestedFormatId) {

            case RDPECLIP::CF_UNICODETEXT:
                this->send_to_clipboard_Buffer(chunk);
                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_textBuffer_to_clipboard(true);
                }
            break;
            case RDPECLIP::CF_TEXT:
                this->send_to_clipboard_Buffer(chunk);
                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_textBuffer_to_clipboard(false);
                }
            break;

            case RDPECLIP::CF_METAFILEPICT:


                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                    RDPECLIP::MetaFilePicDescriptor mfpd;
                    mfpd.receive(chunk);

                    cb_buffers.pic_height = mfpd.height;
                    cb_buffers.pic_width  = mfpd.width;
                    cb_buffers.pic_bpp    = mfpd.bpp;
                    cb_buffers.sizeTotal  = mfpd.imageSize;
                    cb_buffers.data       = std::make_unique<uint8_t[]>(cb_buffers.sizeTotal);

                    this->paste_data_len = mfpd.height * mfpd.width * (mfpd.bpp/8);
                }

                this->send_to_clipboard_Buffer(chunk);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_imageBuffer_to_clipboard();

                    std::chrono::microseconds time  = difftimeval(tvtime(), this->paste_data_request_time);
                    long duration = time.count();;
                    LOG(LOG_INFO, "RDPECLIP::METAFILEPICT size=%ld octets  duration=%ld us", this->paste_data_len, duration);

                }
            break;

            case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    cb_buffers.sizeTotal -= 4;
                    cb_filesList.itemslist.clear();
                    cb_filesList.cItems= chunk.in_uint32_le();
                    cb_filesList.lindexToRequest= 0;
                    this->emptyLocalBuffer();

                }

                this->send_to_clipboard_Buffer(chunk);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    InStream stream(cb_buffers.data.get(), cb_buffers.sizeTotal);

                    RDPECLIP::FileDescriptor fd;

                    for (uint32_t i = 0; i < cb_filesList.cItems; i++) {
                        fd.receive(stream);
                        CB_FilesList::CB_in_Files file;
                        file.size = fd.file_size();
                        this->paste_data_len = file.size;
                        file.name = fd.fileName();
                        cb_filesList.itemslist.push_back(file);
                    }

                    RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest
                                                                        , RDPECLIP::FILECONTENTS_RANGE
                                                                        , cb_filesList.lindexToRequest
                                                                        , cb_filesList.itemslist[cb_filesList.lindexToRequest].size);
                    StaticOutStream<32> out_streamRequest;
                    fileContentsRequest.emit(out_streamRequest);
                    const uint32_t total_length_FormatDataRequestPDU = out_streamRequest.get_offset();

                    InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatDataRequestPDU);

                    this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                  , chunkRequest
                                                  , total_length_FormatDataRequestPDU
                                                  , CHANNELS::CHANNEL_FLAG_LAST  |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |
                                                    CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                  );
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "CLIENT >> CB channel: File Contents Resquest PDU FILECONTENTS_RANGE");
                    }

                    this->empty_buffer();
                }
            break;

            case ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS:

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    cb_filesList.streamIDToRequest = chunk.in_uint32_le();
                }

                if (cb_filesList.lindexToRequest == cb_filesList.itemslist.size()) {
                    cb_filesList.lindexToRequest--;
                }

                this->clientIOClipboardAPI->write_clipboard_temp_file( cb_filesList.itemslist[cb_filesList.lindexToRequest].name
                                                       , chunk.get_current()
                                                       , chunk.in_remain()
                                                       );

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->_waiting_for_data = false;

                    cb_filesList.lindexToRequest++;

                    if (cb_filesList.lindexToRequest >= cb_filesList.cItems) {

                        this->clientIOClipboardAPI->set_local_clipboard_stream(false);
                        for (size_t i = 0; i < cb_filesList.itemslist.size(); i++) {
                            //cb_filesList.itemslist);
                            this->clientIOClipboardAPI->setClipboard_files(cb_filesList.itemslist[i].name);
                        }
                        this->clientIOClipboardAPI->set_local_clipboard_stream(true);

//                         RDPECLIP::UnlockClipboardDataPDU unlockClipboardDataPDU(0);
//                         StaticOutStream<32> out_stream_unlock;
//                         unlockClipboardDataPDU.emit(out_stream_unlock);
//                         InStream chunk_unlock(out_stream_unlock.get_data(), out_stream_unlock.get_offset());
//
//                         this->client->mod->send_to_mod_channel( channel_names::cliprdr
//                                                       , chunk_unlock
//                                                       , out_stream_unlock.get_offset()
//                                                       , CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
//                                                         CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
//                                                       );
//                         if (bool(this->verbose & RDPVerbose::cliprdr)) {
//                             LOG(LOG_INFO, "CLIENT >> CB channel: Unlock Clipboard Data PDU");
//                         }

                    } else {
                        cb_filesList.streamIDToRequest++;

                        StaticOutStream<32> out_streamRequest;
                        RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest
                                                                            , RDPECLIP::FILECONTENTS_RANGE
                                                                            , cb_filesList.lindexToRequest
                                                                            ,   cb_filesList.itemslist[cb_filesList.lindexToRequest].size);
                        fileContentsRequest.emit(out_streamRequest);
                        const uint32_t total_length_FormatDataRequestPDU = out_streamRequest.get_offset();

                        InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatDataRequestPDU);

                        this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                                      , chunkRequest
                                                      , total_length_FormatDataRequestPDU
                                                      , CHANNELS::CHANNEL_FLAG_LAST  |
                                                        CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                      );
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "CLIENT >> CB channel: File Contents Resquest PDU FILECONTENTS_RANGE");
                        }
                    }

                    std::chrono::microseconds time  = difftimeval(tvtime(), this->paste_data_request_time);
                    long duration = time.count();;
                    LOG(LOG_INFO, "RDPECLIP::FILE size=%ld octets  duration=%ld us", this->paste_data_len*3, duration);

                    this->empty_buffer();
                }
            break;

            default:
                if (strcmp(this->_requestedFormatName.c_str(), RDPECLIP::get_format_short_name(RDPECLIP::SF_TEXT_HTML)) == 0) {
                    this->send_to_clipboard_Buffer(chunk);

                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        this->send_textBuffer_to_clipboard(false);
                    }

                }  else {
                    LOG(LOG_WARNING, "SERVER >> CB channel: unknow CB Format = %x", this->_requestedFormatId);
                }
            break;
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            RDPECLIP::UnlockClipboardDataPDU unlockClipboardDataPDU(0);
            StaticOutStream<32> out_stream_unlock;
            unlockClipboardDataPDU.emit(out_stream_unlock);
            InStream chunk_unlock(out_stream_unlock.get_data(), out_stream_unlock.get_offset());

            this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                            , chunk_unlock
                                            , out_stream_unlock.get_offset()
                                            , CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                            CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                            );
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO, "CLIENT >> CB channel: Unlock Clipboard Data PDU");
            }
        }
    }

    void send_textBuffer_to_clipboard(bool is_utf16) {

        const char * str_data;
        size_t length_of_utf8_string;

        if (is_utf16) {
            std::unique_ptr<uint8_t[]> utf8_string = std::make_unique<uint8_t[]>(this->_cb_buffers.sizeTotal);
            length_of_utf8_string = ::UTF16toUTF8(
            this->_cb_buffers.data.get(), this->_cb_buffers.sizeTotal,
            utf8_string.get(), this->_cb_buffers.sizeTotal);
            str_data = reinterpret_cast<const char*>(utf8_string.get());
        } else {
            str_data = reinterpret_cast<const char*>(this->_cb_buffers.data.get());
            length_of_utf8_string = this->_cb_buffers.size;
        }

        std::string str(str_data, length_of_utf8_string);

        this->clientIOClipboardAPI->set_local_clipboard_stream(false);
        this->clientIOClipboardAPI->setClipboard_text(str);
        this->clientIOClipboardAPI->set_local_clipboard_stream(true);

        this->empty_buffer();
    }

    void send_to_clipboard_Buffer(InStream & chunk) {

        const size_t length_of_data_to_dump(chunk.in_remain());
        const size_t sum_buffer_and_data(this->_cb_buffers.size + length_of_data_to_dump);
        const uint8_t * utf8_data = chunk.get_current();

        for (size_t i = 0; i < length_of_data_to_dump && i + this->_cb_buffers.size < this->_cb_buffers.sizeTotal; i++) {
            this->_cb_buffers.data[i + this->_cb_buffers.size] = utf8_data[i];
        }

        this->_cb_buffers.size = sum_buffer_and_data;
    }

    void send_imageBuffer_to_clipboard() {

        this->clientIOClipboardAPI->set_local_clipboard_stream(false);
        this->clientIOClipboardAPI->setClipboard_image(this->_cb_buffers.data.get(),
                                                       this->_cb_buffers.pic_width,
                                                       this->_cb_buffers.pic_height,
                                                       this->_cb_buffers.pic_bpp);
        this->clientIOClipboardAPI->set_local_clipboard_stream(true);

        this->empty_buffer();
    }

    void empty_buffer() {
        this->_cb_buffers.pic_bpp    = 0;
        this->_cb_buffers.sizeTotal  = 0;
        this->_cb_buffers.pic_width  = 0;
        this->_cb_buffers.pic_height = 0;
        this->_cb_buffers.size       = 0;
        this->_waiting_for_data = false;
    }

    void emptyLocalBuffer() {
        this->clientIOClipboardAPI->emptyBuffer();
    }

    void send_FormatListPDU() {
        uint32_t formatIDs[] = { this->clientIOClipboardAPI->get_buffer_type_id() };

        int index = 0;
        size_t format_size_name = 0;
        if (this->clientIOClipboardAPI->get_buffer_type_long_name() == ClientIOClipboardAPI::FILEGROUPDESCRIPTORW_BUFFER_TYPE) {
            index = 1;
            format_size_name = 42;
        } else {
            index = 2;
            format_size_name = 2;
        }

        const uint16_t * formatListDataName[] = {reinterpret_cast<const uint16_t *>(this->clipbrdFormatsList.names[index].data())};
        const size_t size_names[] = {format_size_name};

        this->send_FormatListPDU(formatIDs, formatListDataName, size_names, 1);
    }

    void send_FormatListPDU(uint32_t const * formatIDs, const uint16_t ** formatListName, const std::size_t * size_names, const std::size_t formatIDs_size) {

        StaticOutStream<1600> out_stream;
        if (this->server_use_long_format_names) {
            RDPECLIP::FormatListPDU_LongName format_list_pdu_long(formatIDs, formatListName, size_names, formatIDs_size);
            format_list_pdu_long.emit(out_stream);
        } else {
            RDPECLIP::FormatListPDU_ShortName format_list_pdu_short(formatIDs, formatListName, size_names, formatIDs_size);
            format_list_pdu_short.emit(out_stream);
        }

        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        this->client->mod->send_to_mod_channel( channel_names::cliprdr
                                      , chunk
                                      , out_stream.get_offset()
                                      , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                      );

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO, "CLIENT >> CB channel: Format List PDU");
        }
    }


    void process_client_clipboard_out_data(const CHANNELS::ChannelNameId & front_channel_name, const uint64_t total_length, OutStream & out_stream_first_part, const size_t first_part_data_size,  uint8_t const * data, const size_t data_len, uint32_t flags){

        // 3.1.5.2.2.1 Reassembly of Chunked Virtual Channel Dat

        // Virtual channel data can span multiple Virtual Channel PDUs (section 3.1.5.2.1).
        // If this is the case, the embedded length field of the channelPduHeader field
        // (the Channel PDU Header structure is specified in section 2.2.6.1.1) specifies
        // the total length of the uncompressed virtual channel data spanned across all of
        // the associated Virtual Channel PDUs. This length is referred to as totalLength.
        // For example, assume that the virtual channel chunking size specified in the Virtual
        // Channel Capability Set (section 2.2.7.1.10) is 1,000 bytes and that 2,062 bytes need
        // to be transmitted on a given virtual channel. In this example,
        // the following sequence of Virtual Channel PDUs will be sent (only relevant fields are listed):

        //    Virtual Channel PDU 1:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_FIRST
        //    Actual virtual channel data is 1000 bytes (the chunking size).

        //    Virtual Channel PDU 2:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = 0
        //    Actual virtual channel data is 1000 bytes (the chunking size).

        //    Virtual Channel PDU 3:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_LAST
        //    Actual virtual channel data is 62 bytes.

    //     // The size of the virtual channel data in the last PDU (the data in the virtualChannelData field)
        // is determined by subtracting the offset of the virtualChannelData field in the encapsulating
        // Virtual Channel PDU from the total size specified in the tpktHeader field. This length is
        // referred to as chunkLength.

        // Upon receiving each Virtual Channel PDU, the server MUST dispatch the virtual channel data to
        // the appropriate virtual channel endpoint. The sequencing of the chunk (whether it is first,
        // intermediate, or last), totalLength, chunkLength, and the virtualChannelData fields MUST
        // be dispatched to the virtual channel endpoint so that the data can be correctly reassembled.
        // If the CHANNEL_FLAG_SHOW_PROTOCOL (0x00000010) flag is specified in the Channel PDU Header,
        // then the channelPduHeader field MUST also be dispatched to the virtual channel endpoint.

        // A reassembly buffer MUST be created by the virtual channel endpoint using the size specified
        // by totalLength when the first chunk is received. After the reassembly buffer has been created
        // the first chunk MUST be copied into the front of the buffer. Subsequent chunks MUST then be
        // copied into the reassembly buffer in the order in which they are received. Upon receiving the
        // last chunk of virtual channel data, the reassembled data is processed by the virtual channel endpoint.

        if (data_len > first_part_data_size ) {

            int real_total = data_len - first_part_data_size;
            const int cmpt_PDU_part(real_total  / CHANNELS::CHANNEL_CHUNK_LENGTH);
            const int remains_PDU  (real_total  % CHANNELS::CHANNEL_CHUNK_LENGTH);
            int data_sent(0);

            // First Part
                out_stream_first_part.out_copy_bytes(data, first_part_data_size);

                data_sent += first_part_data_size;
                InStream chunk_first(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_first
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_FIRST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_first_part.get_data(), out_stream_first_part.get_offset());


            for (int i = 0; i < cmpt_PDU_part; i++) {

            // Next Part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                out_stream_next_part.out_copy_bytes(data + data_sent, CHANNELS::CHANNEL_CHUNK_LENGTH);

                data_sent += CHANNELS::CHANNEL_CHUNK_LENGTH;
                InStream chunk_next(out_stream_next_part.get_data(), out_stream_next_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_next
                                                    , total_length
                                                    , flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_next_part.get_data(), out_stream_next_part.get_offset());
            }

            // Last part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
                out_stream_last_part.out_copy_bytes(data + data_sent, remains_PDU);

                data_sent += remains_PDU;
                InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_last
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_LAST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_last_part.get_data(), out_stream_last_part.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data, data_len);
            InStream chunk(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

            this->client->mod->send_to_mod_channel( front_channel_name
                                                , chunk
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                                  flags
                                                );
        }
    }
};
