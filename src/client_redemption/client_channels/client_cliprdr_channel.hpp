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

#include "mod/rdp/rdp_verbose.hpp"
#include "core/RDP/clipboard/format_name.hpp"
#include "utils/image_data_view.hpp"

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>


class ClientChannelMod;
class RDPClipboardConfig;
class InStream;


class ClientIOClipboardAPI
{
public:
    virtual ~ClientIOClipboardAPI() = default;

    // control state
    virtual void emptyBuffer() = 0;
    virtual void set_local_clipboard_stream(bool val) = 0;

    //  set distant clipboard data
    virtual void setClipboard_text(std::string const& str) = 0;
    virtual void setClipboard_image(ConstImageDataView const& image) = 0;
    virtual void setClipboard_files(std::string const& name) = 0;
    virtual void write_clipboard_temp_file(std::string const& fileName, bytes_view data) = 0;

    //  get local clipboard data
    virtual uint16_t get_buffer_type_id() = 0;
    virtual int get_citems_number() = 0;

    virtual u8_array_view get_cliboard_text() = 0;
    virtual ConstImageDataView get_image() = 0;
    virtual std::string get_file_item_name(int index) = 0;
    virtual chars_view get_file_item(int index) = 0;

    virtual size_t get_cliboard_data_length() = 0;
};

class ClientCLIPRDRChannel
{
    RDPVerbose verbose;
    ClientIOClipboardAPI * clientIOClipboardAPI;
    ClientChannelMod * callback;

    enum class CustomFormatName;

    uint32_t             _requestedFormatId = 0;
    CustomFormatName     _requestedFormatName {};

public:
    bool                 _waiting_for_data;

private:
    timeval paste_data_request_time;
public:
    long paste_data_len = 0;

private:
    const uint32_t channel_flags;
    std::unordered_map<uint32_t, std::string> formats_map;
    const double arbitrary_scale;

public:
    uint32_t file_content_flag;

private:
    const std::string path;


public:
    struct CB_Buffers {
        std::unique_ptr<uint8_t[]>  data = nullptr;
        uint64_t size = 0;
        uint64_t sizeTotal = 0;
        int    pic_width = 0;
        int    pic_height = 0;
        int    pic_bpp = 0;

        [[nodiscard]] u8_array_view av() const noexcept
        {
            return {this->data.get(), size_t(this->size)};
        }
    } _cb_buffers;

private:
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
public:
    bool server_use_long_format_names = true;

private:
    const uint16_t cCapabilitiesSets = 1;
    uint32_t generalFlags;

    Cliprdr::FormatNameInventory format_name_list;


public:
    ClientCLIPRDRChannel(RDPVerbose verbose, ClientChannelMod * callback, RDPClipboardConfig const& config);

    ~ClientCLIPRDRChannel();

private:
    void add_format(uint32_t format_id, bytes_view name);

public:
    void set_api(ClientIOClipboardAPI * clientIOClipboardAPI);

    void receive(InStream & chunk, int flags);

    void process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList);

    void send_textBuffer_to_clipboard(bool is_utf16);
    void send_to_clipboard_Buffer(InStream & chunk);

    void send_imageBuffer_to_clipboard();

    void empty_buffer();

    void emptyLocalBuffer() const;
    void send_FormatListPDU();

    void send_UnlockPDU(uint32_t streamID) const;

    void process_monitor_ready();

    void process_capabilities(InStream & chunk);

    void process_format_list(InStream & chunk, uint32_t msgFlags);

    void process_format_data_request(InStream & chunk) const;

    void process_filecontents_request(InStream & chunk) const;
};

