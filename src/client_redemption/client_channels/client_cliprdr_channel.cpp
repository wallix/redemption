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


#include "client_redemption/client_channels/client_cliprdr_channel.hpp"
#include "utils/difftimeval.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <sys/stat.h>
#include <sys/types.h>



// [MS-RDPECLIP]: Remote Desktop Protocol: CLIpboard Virtual Channel Extension
//
//
// 1.3.2.1 Initialization Sequence
//
// The goal of the Initialization Sequence is to establish the client and the
// server capabilities, exchange settings, and synchronize the initial state of
// the client and server clipboards.
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
//     The server sends a Clipboard Capabilities PDU to the client to advertise
//     the capabilities that it supports.
//
//      The server sends a Monitor Ready PDU to the client.
//
//     Upon receiving the Monitor Ready PDU, the client transmits its capabilities
//     to the server by using a Clipboard Capabilities PDU.
//
//     The client sends the Temporary Directory PDU to inform the server of a
//     location on the client file system that can be used to deposit files being
//     copied to the client. To make use of this location, the server has to be
//     able to access it directly. At this point, the client and the server
//     capability negotiation is complete.
//
//     The final stage of the Initialization Sequence involves synchronizing the
//     Clipboard Formats on the server clipboard with the client. This is
//     accomplished by effectively mimicking a copy operation on the client by
//     forcing it to send a Format List PDU.
//
//     The server responds with a Format List Response PDU.
//
//
// 1.3.2.2 Data Transfer Sequences
//
// The goal of the Data Transfer Sequences is to perform a copy or paste operation.
// The diagram that follows presents a possible data transfer sequence.
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



    ClientCLIPRDRChannel::ClientCLIPRDRChannel(RDPVerbose verbose, ClientChannelMod * callback, RDPClipboardConfig const& config)
      :  verbose(verbose)
      , clientIOClipboardAPI(nullptr)
      , callback(callback)
      , _waiting_for_data(false)
      , channel_flags(CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL)
      , arbitrary_scale(config.arbitrary_scale)
      , file_content_flag(RDPECLIP::FILECONTENTS_SIZE)
      , path(config.path)
      , server_use_long_format_names(config.server_use_long_format_names)
      , cCapabilitiesSets(config.cCapabilitiesSets)
      , generalFlags(config.generalFlags)
    {
        if (!dir_exist(this->path.c_str())){
            mkdir(this->path.c_str(), 0777);
        }
        if (!dir_exist(this->path.c_str())){
            LOG(LOG_WARNING, "Can't enable shared clipboard, \"%s\" directory doesn't exist.", this->path);
        }

        for (auto const& format : config.formats) {
            this->add_format(format.ID, format.name);
        }
    }

    void ClientCLIPRDRChannel::set_api(ClientIOClipboardAPI * clientIOClipboardAPI) {
        this->clientIOClipboardAPI = clientIOClipboardAPI;
    }

    ClientCLIPRDRChannel::~ClientCLIPRDRChannel() {
        this->empty_buffer();
    }

    void ClientCLIPRDRChannel::add_format(uint32_t ID, const std::string & name) {
        this->format_list_pdu.add_format_name(ID, name.c_str());
        this->formats_map.emplace(ID, name);
    }

// MS-RDPECLIP

// 2.2.1 Clipboard PDU Header (CLIPRDR_HEADER)

// The CLIPRDR_HEADER structure is present in all clipboard PDUs. It is used to identify the PDU type,
//  specify the length of the PDU, and convey message flags.

// msgType : 16 bits
// msgFlags : 16 bits
// dataLen : 32 bits integer

// msgType (2 bytes): An unsigned, 16-bit integer that specifies the type of the
// clipboard PDU that follows the dataLen field.

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

// <1> Section 2.2.1:  The operating systems Windows Server 2003, Windows Vista, Windows Server 2008,
// Windows 7, Windows Server 2008 R2, Windows 8, Windows Server 2012, Windows 8.1, and
// Windows Server 2012 R2 append four bytes to the end of clipboard PDUs. These four bytes are not
// included in the PDU size specified by the dataLen field and can be ignored.

    void ClientCLIPRDRChannel::receive(InStream & chunk, int flags) {
        if (!this->clientIOClipboardAPI) {
            return;
        }

        // TODO: from the above documentation, are we not expecting at least 8 bytes ?
        if (!chunk.in_check_rem(8  /*msgType(2)*/ )) {
            LOG(LOG_ERR,
                "ClipboardVirtualChannel::process_client_message: "
                    "Truncated CliprdrHeader, need=8 remains=%zu",
                chunk.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        if (this->_waiting_for_data) {

            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO, "SERVER >> Process server next part PDU data");
            }
            this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
        } else {
            if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                return;
            }

            RDPECLIP::CliprdrHeader header;
            header.recv(chunk);

            switch (header.msgType()) {

                case RDPECLIP::CB_CLIP_CAPS:
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Clipboard Capabilities PDU");
                    }
                    this->process_capabilities(chunk);
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

                    this->process_monitor_ready();
                break;

    // 2.2.3.2 Format List Response PDU (FORMAT_LIST_RESPONSE)

    // The Format List Response PDU is sent as a reply to the Format List PDU. It is used to indicate
    // whether processing of the Format List PDU was successful.

    // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST
    // be set to CB_FORMAT_LIST_RESPONSE (0x0003). The CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002)
    // flag MUST be set in the msgFlags field of the Clipboard PDU Header.

                case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        if (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
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
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Format List PDU");
                    }

                    if (header.dataLen() > chunk.in_remain()) {
                        LOG(LOG_WARNING, "Server Format List PDU data length(%u) longer than chunk(%zu)", header.dataLen(), chunk.in_remain());
                    }

                    this->process_format_list(chunk, header.msgFlags());
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
                    if (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
                        LOG(LOG_WARNING, "SERVER >> CB Channel: Format Data Response PDU FAILED");
                    } else {
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Response PDU");
                        }

                        if(this->_requestedFormatName == RDPECLIP::FILEGROUPDESCRIPTORW.data()) {
                            this->_requestedFormatId = ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                        }

                        this->_cb_buffers.sizeTotal = header.dataLen();

                        this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
                    }
                break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Request PDU");
                    }

                    this->process_format_data_request(chunk);
                break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Resquest PDU");
                    }

                    this->process_filecontents_request(chunk);
                break;

                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                    if (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
                        LOG(LOG_WARNING, "SERVER >> CB Channel: File Contents Response PDU FAILED");
                    } else {
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU");
                        }

                        if (this->_requestedFormatId == ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW) {
                            this->_requestedFormatId = ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS;
                        }

                        this->_cb_buffers.sizeTotal = header.dataLen();

                        this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
                    }
                break;

                default:
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "SERVER >> Default Process server PDU data");
                    }
                    this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);

                break;
            }
        }
    }

    void ClientCLIPRDRChannel::process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList) {

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
        // received), then the data in the virtualChannelData field can be dispatched to the virtual channel
        // endpoint (no reassembly is required by the endpoint). If the CHANNEL_FLAG_SHOW_PROTOCOL
        // (0x00000010) flag is specified in the Channel PDU Header, then the channelPduHeader field MUST also
        // be dispatched to the virtual channel endpoint.

        // If the virtual channel data is part of a sequence of chunks, then the instructions in section 3.1.5.2.2.1
        //MUST be followed to reassemble the stream.

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->_waiting_for_data = true;
//             cb_buffers.sizeTotal = chunk.in_uint32_le();
            cb_buffers.data = std::make_unique<uint8_t[]>(cb_buffers.sizeTotal);
        }

        switch (this->_requestedFormatId) {

            case RDPECLIP::CF_UNICODETEXT:
                this->send_to_clipboard_Buffer(chunk);
                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_textBuffer_to_clipboard(true);
                    this->send_UnlockPDU(0);
                }
            break;

            case RDPECLIP::CF_TEXT:
                this->send_to_clipboard_Buffer(chunk);
                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_textBuffer_to_clipboard(false);
                    this->send_UnlockPDU(0);
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

                    this->send_UnlockPDU(0);

                    std::chrono::microseconds time = difftimeval(tvtime(), this->paste_data_request_time);
                    long duration = time.count();
                    LOG(LOG_INFO, "RDPECLIP::METAFILEPICT size=%ld octets  duration=%ld us", this->paste_data_len, duration);

                }
            break;

            case ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:

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

                    RDPECLIP::CliprdrHeader fileContentsRequestHeader(RDPECLIP::CB_FILECONTENTS_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, 28);

                    RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest+1
                                                                        , cb_filesList.lindexToRequest
                                                                        , RDPECLIP::FILECONTENTS_SIZE
                                                                        , 0
                                                                        , 0
                                                                        , RDPECLIP::FILECONTENTS_SIZE_CB_REQUESTED
                                                                        , 0
                                                                        , true);
                    StaticOutStream<64> out_streamRequest;
                    fileContentsRequestHeader.emit(out_streamRequest);
                    fileContentsRequest.emit(out_streamRequest);
                    const uint32_t total_length_FormatContentRequestPDU = out_streamRequest.get_offset();

                    InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatContentRequestPDU);

                    this->callback->send_to_mod_channel( channel_names::cliprdr
                                                  , chunkRequest
                                                  , total_length_FormatContentRequestPDU
                                                  , this->channel_flags
                                                  );
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "CLIENT >> CB channel: File Contents Resquest PDU SIZE");
                    }

                    this->empty_buffer();
                }
            break;

            case ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS:

                switch (this->file_content_flag) {

                    case RDPECLIP::FILECONTENTS_SIZE:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU SIZE ");
                        }

                        {
                        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                            this->_waiting_for_data = false;
                        }
                        this->file_content_flag = RDPECLIP::FILECONTENTS_RANGE;

                        cb_filesList.streamIDToRequest = chunk.in_uint32_le();

                        StaticOutStream<64> out_streamRequest;
                        RDPECLIP::CliprdrHeader fileContentsRequestHeader(RDPECLIP::CB_FILECONTENTS_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, 28);

                        RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest
                                                                            , cb_filesList.lindexToRequest
                                                                            , this->file_content_flag
                                                                            , cb_filesList.itemslist[cb_filesList.lindexToRequest].size
                                                                            , cb_filesList.itemslist[cb_filesList.lindexToRequest].size >> 32
                                                                            , 0
                                                                            , 0
                                                                            , true);
                        fileContentsRequestHeader.emit(out_streamRequest);
                        fileContentsRequest.emit(out_streamRequest);
                        const uint32_t total_length_FormatContentRequestPDU = out_streamRequest.get_offset();

                        InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatContentRequestPDU);

                        this->callback->send_to_mod_channel( channel_names::cliprdr
                                                              , chunkRequest
                                                              , total_length_FormatContentRequestPDU
                                                              , this->channel_flags
                                                              );

                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Request PDU RANGE ");
                        }
                        }
                        break;

                    case RDPECLIP::FILECONTENTS_RANGE:

                        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                            cb_filesList.itemslist[cb_filesList.lindexToRequest].size = cb_buffers.sizeTotal;
                            cb_filesList.streamIDToRequest = chunk.in_uint32_le();
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU RANGE");
                            }
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
                                for (auto& item : cb_filesList.itemslist) {
                                    this->clientIOClipboardAPI->setClipboard_files(item.name);
                                }
                                this->clientIOClipboardAPI->set_local_clipboard_stream(true);

                                this->send_UnlockPDU(cb_filesList.streamIDToRequest);

                                 this->file_content_flag = RDPECLIP::FILECONTENTS_SIZE;

                            } else {
                                cb_filesList.lindexToRequest++;

                                 this->file_content_flag = RDPECLIP::FILECONTENTS_SIZE;

                                StaticOutStream<32> out_streamRequest;
                                RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest
                                                                                    , this->file_content_flag
                                                                                    , cb_filesList.lindexToRequest
                                                                                    , 0
                                                                                    , 0
                                                                                    , RDPECLIP::FILECONTENTS_SIZE_CB_REQUESTED);
                                fileContentsRequest.emit(out_streamRequest);
                                const uint32_t total_length_FormatContentRequestPDU = out_streamRequest.get_offset();

                                InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatContentRequestPDU);

                                this->callback->send_to_mod_channel( channel_names::cliprdr
                                                                        , chunkRequest
                                                                        , total_length_FormatContentRequestPDU
                                                                        , this->channel_flags
                                                                        );

                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Request PDU SIZE ");
                                }
                            }

                            std::chrono::microseconds time  = difftimeval(tvtime(), this->paste_data_request_time);
                            long duration = time.count();;
                            LOG(LOG_INFO, "RDPECLIP::FILE size=%ld octets  duration=%ld us", this->paste_data_len, duration);

                            this->empty_buffer();
                        }
                        break;
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
    }

    void ClientCLIPRDRChannel::send_UnlockPDU(uint32_t streamID) {
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_UNLOCK_CLIPDATA, RDPECLIP::CB_RESPONSE__NONE_, 4);
        RDPECLIP::UnlockClipboardDataPDU unlockClipboardDataPDU(streamID);
        StaticOutStream<32> out_stream_unlock;
        header.emit(out_stream_unlock);
        unlockClipboardDataPDU.emit(out_stream_unlock);
        InStream chunk_unlock(out_stream_unlock.get_bytes());

        this->callback->send_to_mod_channel( channel_names::cliprdr
                                        , chunk_unlock
                                        , out_stream_unlock.get_offset()
                                        , this->channel_flags
                                        );
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO, "CLIENT >> CB channel: Unlock Clipboard Data PDU");
        }
    }

    void ClientCLIPRDRChannel::send_textBuffer_to_clipboard(bool is_utf16) {

        this->clientIOClipboardAPI->set_local_clipboard_stream(false);

        if (is_utf16) {
            auto utf8_string = std::make_unique<uint8_t[]>(this->_cb_buffers.sizeTotal+2);
            size_t len = ::UTF16toUTF8(
                this->_cb_buffers.data.get(), this->_cb_buffers.sizeTotal+2,
                utf8_string.get(), this->_cb_buffers.sizeTotal+2);
            char const* str_data = char_ptr_cast(utf8_string.get());
            this->clientIOClipboardAPI->setClipboard_text({str_data, len});
        } else {
            char const* str_data = char_ptr_cast(this->_cb_buffers.data.get());
            size_t len = this->_cb_buffers.size;
            this->clientIOClipboardAPI->setClipboard_text({str_data, len});
        }

        this->clientIOClipboardAPI->set_local_clipboard_stream(true);

        this->empty_buffer();
    }

    void ClientCLIPRDRChannel::send_to_clipboard_Buffer(InStream & chunk) {

        const size_t length_of_data_to_dump(chunk.in_remain());
        const size_t sum_buffer_and_data(this->_cb_buffers.size + length_of_data_to_dump);
        const uint8_t * utf8_data = chunk.get_current();

        for (size_t i = 0; i < length_of_data_to_dump && i + this->_cb_buffers.size < this->_cb_buffers.sizeTotal; i++) {
            this->_cb_buffers.data[i + this->_cb_buffers.size] = utf8_data[i];
        }

        this->_cb_buffers.size = sum_buffer_and_data;
    }

    void ClientCLIPRDRChannel::send_imageBuffer_to_clipboard() {

        this->clientIOClipboardAPI->set_local_clipboard_stream(false);
        this->clientIOClipboardAPI->setClipboard_image(this->_cb_buffers.data.get(),
                                                       this->_cb_buffers.pic_width,
                                                       this->_cb_buffers.pic_height,
                                                       checked_int(this->_cb_buffers.pic_bpp));
        this->clientIOClipboardAPI->set_local_clipboard_stream(true);

        this->empty_buffer();
    }

    void ClientCLIPRDRChannel::empty_buffer() {
        this->_cb_buffers.pic_bpp    = 0;
        this->_cb_buffers.sizeTotal  = 0;
        this->_cb_buffers.pic_width  = 0;
        this->_cb_buffers.pic_height = 0;
        this->_cb_buffers.size       = 0;
        this->_waiting_for_data = false;
    }

    void ClientCLIPRDRChannel::emptyLocalBuffer() {
        this->clientIOClipboardAPI->emptyBuffer();
    }

    void ClientCLIPRDRChannel::send_FormatListPDU() {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        std::string format_name = this->formats_map[this->clientIOClipboardAPI->get_buffer_type_id()];
        format_list_pdu.add_format_name(this->clientIOClipboardAPI->get_buffer_type_id(), format_name.c_str());

        const bool use_long_format_names = true;
        const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
            format_list_pdu.size(use_long_format_names));

        StaticOutStream<1600> out_stream;

        clipboard_header.emit(out_stream);
        format_list_pdu.emit(out_stream, use_long_format_names);

        InStream chunk(out_stream.get_bytes());

        this->callback->send_to_mod_channel( channel_names::cliprdr
                    , chunk
                    , out_stream.get_offset()
                    , this->channel_flags
                    );
    }

    void ClientCLIPRDRChannel::process_monitor_ready() {

        if (this->server_use_long_format_names) {
            this->generalFlags = this->generalFlags | RDPECLIP::CB_USE_LONG_FORMAT_NAMES;
        }

        {
            RDPECLIP::GeneralCapabilitySet general_cap_set(RDPECLIP::CB_CAPS_VERSION_2, this->generalFlags);
            RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(this->cCapabilitiesSets);
            RDPECLIP::CliprdrHeader header(RDPECLIP::CB_CLIP_CAPS, 0,
                clipboard_caps_pdu.size() + general_cap_set.size());

            StaticOutStream<1024> out_stream;
            header.emit(out_stream);
            clipboard_caps_pdu.emit(out_stream);
            general_cap_set.emit(out_stream);

            const uint32_t total_length = out_stream.get_offset();
            InStream chunk(out_stream.get_data(), total_length);

            this->callback->send_to_mod_channel( channel_names::cliprdr
                                                , chunk
                                                , total_length
                                                , this->channel_flags
                                                );

            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO, "CLIENT >> CB Channel: Clipboard Capabilities PDU");
            }
        }

        {
            const bool use_long_format_names = this->server_use_long_format_names;
            const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

            RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_LIST,
                RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
                format_list_pdu.size(use_long_format_names));

            StaticOutStream<1600> out_stream;

            header.emit(out_stream);
            format_list_pdu.emit(out_stream, use_long_format_names);

            InStream chunk(out_stream.get_bytes());

            this->callback->send_to_mod_channel( channel_names::cliprdr
                        , chunk
                        , out_stream.get_offset()
                        , this->channel_flags
                        );

            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO, "CLIENT >> CB Channel: Format List PDU");
            }
        }
    }

    void ClientCLIPRDRChannel::process_capabilities(InStream & chunk) {

        RDPECLIP::ClipboardCapabilitiesPDU pdu;
        pdu.recv(chunk);

        RDPECLIP::GeneralCapabilitySet pdu2;
        pdu2.recv(chunk);

        this->server_use_long_format_names = bool(pdu2.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    }

    void ClientCLIPRDRChannel::process_format_list(InStream & chunk, uint32_t msgFlags) {
        bool isSharedFormat = false;
        uint32_t formatID = 0;

        RDPECLIP::FormatListPDUEx format_list_pdu_local;

        format_list_pdu_local.recv(chunk, this->server_use_long_format_names, (msgFlags & RDPECLIP::CB_ASCII_NAMES));

        for (RDPECLIP::FormatName const & format_name_local : format_list_pdu_local) {
            if (isSharedFormat) {
                break;
            }

            formatID = format_name_local.formatId();
            std::string const& format_name = format_name_local.format_name();

            for (RDPECLIP::FormatName const & format_name_ : this->format_list_pdu) {
                if (isSharedFormat) {
                    break;
                }

                if (format_name_.formatId() == formatID) {
                    this->_requestedFormatId = formatID;
                    this->_requestedFormatName = format_name;
                    isSharedFormat = true;
                }
            }

            if ((format_name == RDPECLIP::FILEGROUPDESCRIPTORW.data()) && !isSharedFormat) {
                this->_requestedFormatId = ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                isSharedFormat = true;
            }
        }

        StaticOutStream<256> out_stream;
        RDPECLIP::CliprdrHeader formatListResponsePDUHeader(RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 0);
        formatListResponsePDUHeader.emit(out_stream);
        InStream chunk_format_list(out_stream.get_bytes());

        this->callback->send_to_mod_channel( channel_names::cliprdr
                                        , chunk_format_list
                                        , out_stream.get_offset()
                                        , this->channel_flags
                                        );
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO, "CLIENT >> CB Channel: Format List Response PDU");
        }

        RDPECLIP::CliprdrHeader lockClipboardDataHeader(RDPECLIP::CB_LOCK_CLIPDATA, RDPECLIP::CB_RESPONSE__NONE_, 4);
        RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
        StaticOutStream<32> out_stream_lock;
        lockClipboardDataHeader.emit(out_stream_lock);
        lockClipboardDataPDU.emit(out_stream_lock);
        InStream chunk_lock(out_stream_lock.get_bytes());

        this->callback->send_to_mod_channel( channel_names::cliprdr
                                        , chunk_lock
                                        , out_stream_lock.get_offset()
                                        , this->channel_flags
                                        );
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO, "CLIENT >> CB Channel: Lock Clipboard Data PDU");
        }

        RDPECLIP::CliprdrHeader formatListRequestPDUHeader(RDPECLIP::CB_FORMAT_DATA_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, 4);
        RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(formatID);
        StaticOutStream<256> out_streamRequest;
        formatListRequestPDUHeader.emit(out_streamRequest);
        formatDataRequestPDU.emit(out_streamRequest);
        InStream chunkRequest(out_streamRequest.get_bytes());

        this->callback->send_to_mod_channel( channel_names::cliprdr
                                        , chunkRequest
                                        , out_streamRequest.get_offset()
                                        , this->channel_flags
                                        );
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Request PDU");
        }

        this->paste_data_request_time = tvtime();
    }

    void ClientCLIPRDRChannel::process_format_data_request(InStream & chunk) {
        int first_part_data_size(0);
        uint32_t total_length(this->clientIOClipboardAPI->get_cliboard_data_length() + RDPECLIP::CliprdrHeader::size());
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

                    RDPECLIP::CliprdrHeader formatDataResponseHeader(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, this->clientIOClipboardAPI->get_cliboard_data_length()+RDPECLIP::METAFILE_HEADERS_SIZE);
                    RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr(
                            this->clientIOClipboardAPI->get_cliboard_data_length()
                        , image.width()
                        , image.height()
                        , safe_int(image.bits_per_pixel())
                        , this->arbitrary_scale
                    );
                    formatDataResponseHeader.emit(out_stream_first_part);
                    fdr.emit(out_stream_first_part);

                    this->callback->process_client_channel_out_data(
                            channel_names::cliprdr
                        , total_length
                        , out_stream_first_part
                        , first_part_data_size
                        , {
                            image.data(),
                            this->clientIOClipboardAPI->get_cliboard_data_length() + RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender::SIZE
                        }
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

                    RDPECLIP::CliprdrHeader formatDataResponseHeader(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, this->clientIOClipboardAPI->get_cliboard_data_length());

                    formatDataResponseHeader.emit(out_stream_first_part);

                    this->callback->process_client_channel_out_data(
                        channel_names::cliprdr
                        , total_length
                        , out_stream_first_part
                        , first_part_data_size
                        , {this->clientIOClipboardAPI->get_text()
                        , this->clientIOClipboardAPI->get_cliboard_data_length()}
                        , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                    );
                }
                break;

                case ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:
                {
                    int data_sent(0);
                    first_part_data_size = RDPECLIP::CliprdrHeader::size() + 4;
                    total_length = (RDPECLIP::FileDescriptor::size() * this->clientIOClipboardAPI->get_citems_number()) + 8 + RDPECLIP::CliprdrHeader::size();
                    int flag_first(CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                    //ClipBoard_Qt::CB_out_File file = this->clientIOClipboardAPI->_items_list[0];

                    RDPECLIP::CliprdrHeader formatDataResponseHeader_FileList(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, (RDPECLIP::FileDescriptor::size() * this->clientIOClipboardAPI->get_citems_number()) + 4);
                    RDPECLIP::FormatDataResponsePDU_FileList fdr(this->clientIOClipboardAPI->get_citems_number());
                    formatDataResponseHeader_FileList.emit(out_stream_first_part);
                    fdr.emit(out_stream_first_part);

                    RDPECLIP::FileDescriptor fdf(
                            this->clientIOClipboardAPI->get_file_item_name(0)
                        , this->clientIOClipboardAPI->get_file_item(0).size()
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

                    this->callback->send_to_mod_channel( channel_names::cliprdr
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
                            , this->clientIOClipboardAPI->get_file_item(i).size()
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

                        this->callback->send_to_mod_channel( channel_names::cliprdr
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

    void ClientCLIPRDRChannel::process_filecontents_request(InStream & chunk) {
        uint32_t streamID(chunk.in_uint32_le());
        int lindex(chunk.in_uint32_le());

        switch (chunk.in_uint32_le()) {         // flag

            case RDPECLIP::FILECONTENTS_SIZE :
            {
                StaticOutStream<32> out_stream;
                RDPECLIP::CliprdrHeader fileSizeHeader(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 16);
                RDPECLIP::FileContentsResponseSize fileSize(
                    streamID
                    , this->clientIOClipboardAPI->get_file_item(lindex).size()
                    );
                fileSizeHeader.emit(out_stream);
                fileSize.emit(out_stream);

                InStream chunk_to_send(out_stream.get_bytes());
                this->callback->send_to_mod_channel( channel_names::cliprdr
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , this->channel_flags
                                                );

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU SIZE");
                }
            }
            break;

            case RDPECLIP::FILECONTENTS_RANGE :
            {
                RDPECLIP::CliprdrHeader fileRangeHeader(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, this->clientIOClipboardAPI->get_file_item(lindex).size()+4);
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;
                RDPECLIP::FileContentsResponseRange fileRange(streamID);

                int first_part_data_size(this->clientIOClipboardAPI->get_file_item(lindex).size());
                int total_length(first_part_data_size + 12);
                if (first_part_data_size > CHANNELS::CHANNEL_CHUNK_LENGTH - 12) {
                    first_part_data_size = CHANNELS::CHANNEL_CHUNK_LENGTH - 12;
                }
                fileRangeHeader.emit(out_stream_first_part);
                fileRange.emit(out_stream_first_part);

                this->callback->process_client_channel_out_data(
                    channel_names::cliprdr
                    , total_length
                    , out_stream_first_part
                    , first_part_data_size
                    , this->clientIOClipboardAPI->get_file_item(lindex)
                    , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                );

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU RANGE");
                }
            }
            break;
        }
    }
// };
