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
#include "gdi/screen_info.hpp"
#include "utils/image_data_view.hpp"

#include <string>

#include "utils/log.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/fileutils.hpp"
#include "core/RDP/clipboard.hpp"

#include "client_redemption/mod_wrapper/client_channel_mod.hpp"
#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"

#include "mod/rdp/rdp_verbose.hpp"



class ClientIOClipboardAPI
{

public:
    virtual ~ClientIOClipboardAPI() = default;

    // control state
    virtual void emptyBuffer() = 0;
    virtual void set_local_clipboard_stream(bool val) = 0;

    //  set distant clipboard data
    virtual void setClipboard_text(std::string const& str) = 0;
    virtual void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const BitsPerPixel bpp) = 0;
    virtual void setClipboard_files(std::string const& name) = 0;
    virtual void write_clipboard_temp_file(std::string const& fileName, const uint8_t * data, size_t data_len) = 0;

    //  get local clipboard data
    virtual uint16_t get_buffer_type_id() = 0;
    virtual uint8_t * get_text() = 0;
    virtual int get_citems_number() = 0;
    virtual size_t get_cliboard_data_length() = 0;

    virtual ConstImageDataView get_image() = 0;

    // TODO should be `array_view_const_u8 (get_text + get_cliboard_data_length)`
    // files data (file index to identify a file among a files group descriptor)
    virtual std::string get_file_item_name(int index) = 0;

    // TODO should be `array_view_const_char get_file_item_size(int index)`
    virtual array_view_char get_file_item(int index) = 0;


};

class ClientCLIPRDRChannel
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
        uint64_t size = 0;
        uint64_t sizeTotal = 0;
        int    pic_width = 0;
        int    pic_height = 0;
        int    pic_bpp = 0;

        array_view_const_u8 av() const noexcept
        {
            return {this->data.get(), this->size};
        }
    } _cb_buffers;

    struct CB_FilesList {
        struct CB_in_Files {
            uint64_t         size;
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

    size_t last_header_data_len = 0;


    ClientCLIPRDRChannel(RDPVerbose verbose, ClientChannelMod * callback, RDPClipboardConfig const& config);

    ~ClientCLIPRDRChannel();

private:
    void add_format(uint32_t ID, const std::string & name);

public:
    void set_api(ClientIOClipboardAPI * clientIOClipboardAPI);

    void receive(InStream & chunk, int flags);

    void process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList);

    void send_textBuffer_to_clipboard(bool is_utf16);
    void send_to_clipboard_Buffer(InStream & chunk);

    void send_imageBuffer_to_clipboard();

    void empty_buffer();

    void emptyLocalBuffer();
    void send_FormatListPDU();

    void send_UnlockPDU(uint32_t streamID);

    void process_monitor_ready();

    void process_capabilities(InStream & chunk);

    void process_format_list(InStream & chunk, uint32_t msgFlags);

    void process_format_data_request(InStream & chunk);

    void process_filecontents_request(InStream & chunk);
};

