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

#include "core/channel_names.hpp"
#include "mod/mod_api.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/image_data_view.hpp"
#include "utils/stream.hpp"
#include "client_redemption/client_channels/client_cliprdr_channel.hpp"
#include "client_redemption/client_channels/client_rdpsnd_channel.hpp"
#include "client_redemption/client_channels/client_remoteapp_channel.hpp"
#include "client_redemption/client_channels/client_rdpdr_channel.hpp"
#include "client_redemption/client_input_output_api/client_keymap_api.hpp"
#include "client_redemption/client_redemption_api.hpp"

#include <chrono>

class FakeRDPChannelsMod : public mod_api
{
public:

    struct PDUData {
        uint8_t data[1600] = {0};
        size_t size = 0;

        u8_array_view av() noexcept
        {
            return {data, size};
        }
    } last_pdu[10];

    int index_in = 0;
    int index_out = 0;

    size_t get_total_stream_produced() const
    {
        return this->index_in;
    }

    FakeRDPChannelsMod::PDUData * stream() {
        if (this->index_out < 10) {
            this->index_out++;
            return &(this->last_pdu[this->index_out-1]);
        }

        return nullptr;
    }


    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override {
        (void) front_channel_name;
        (void) length;
        (void) flags;

        if (this->index_in < 10) {
            last_pdu[this->index_in].size = length;         //chunk.in_remain();
            std::memcpy(last_pdu[this->index_in].data, chunk.get_data(), chunk.in_remain());
            this->index_in++;
        }
    }

    void create_shadow_session(const char * /*userdata*/, const char * /*type*/) override {}
    void send_auth_channel_data(const char * /*data*/) override {}
    void send_checkout_channel_data(const char * /*data*/) override {}

    void rdp_gdi_up_and_running() override {}

    void rdp_gdi_down() override {}

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override {
        (void) param1;
        (void) param2;
        (void) param3;
        (void) param4;
        (void) keymap;
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {(void) device_flags; (void) x; (void) y; (void) keymap; }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override { (void) time; (void) device_flags; (void) param1; (void) param2; }

    void rdp_input_invalidate(Rect r) override { (void) r; }

    void refresh(Rect clip) override { (void) clip; }
};



class FakeClientIOClipboard : public ClientIOClipboardAPI
{
public:
    std::string data_text;
    std::unique_ptr<uint8_t[]>  _chunk;
    size_t offset = 0;
    size_t size = 42;
    std::string fileName;

    enum : int {
        FILEGROUPDESCRIPTORW_BUFFER_TYPE = 0,
        IMAGE_BUFFER_TYPE                = 1,
        TEXT_BUFFER_TYPE                 = 2
    };

    uint16_t    _bufferTypeID = 0;
    int         _bufferTypeNameIndex = 0;
    bool        _local_clipboard_stream = true;
    size_t      _cliboard_data_length = 0;
    int         _cItems = 0;

    std::string tmp_path;

    FakeClientIOClipboard() = default;

    void emptyBuffer() override {}

    void resize_chunk(size_t size) {
        this->size = size;
        this->_chunk = std::make_unique<uint8_t[]>(size);
    }

    //  set distant clipboard data
    void setClipboard_text(std::string const& str) override {
        this->data_text = str;
    }

    u8_array_view get_cliboard_text() override {
        return {this->_chunk.get(), this->_cliboard_data_length};
    }

    void setClipboard_image(ConstImageDataView const& image) override {
        (void)image;
    }

    void setClipboard_files(std::string const& /*name*/) override {}

    void write_clipboard_temp_file(std::string const& fileName, bytes_view data) override {
        this->fileName = fileName;
        size_t data_end = std::min(this->offset + data.size(), this->size);
        for (size_t i = this->offset; i < data_end; i++) {
            this->_chunk[i] = data[i-this->offset];
        }
        this->offset += data.size();
    }

    chars_view get_file_item(int /*index*/) override {
        return {char_ptr_cast(this->_chunk.get()), this->size};
    }

    std::string get_file_item_name(int /*index*/) override {
        return this->fileName;
    }

    void set_local_clipboard_stream(bool /*unused*/) override {}
    uint16_t get_buffer_type_id() override {return this->_bufferTypeID;}
    int get_citems_number() override {return this->_cItems;}
    size_t get_cliboard_data_length() override {return this->_cliboard_data_length;}
    ConstImageDataView get_image() override {
        return ConstImageDataView(
            byte_ptr_cast(""),
            0, 0, 0, ConstImageDataView::BitsPerPixel{},
            ConstImageDataView::Storage::TopToBottom
        );
    }
};


class FakeClientOutPutSound : public ClientOutputSoundAPI
{

public:
    void init(size_t raw_total_size) override { (void)raw_total_size; }
    void setData(bytes_view data) override { (void)data; }
    void play() override {}
};


class FakeClientOutputGraphic : public ClientRemoteAppGraphicAPI
{

public:
    FakeClientOutputGraphic(ClientCallback * controller, ClientRedemptionConfig * config) : ClientRemoteAppGraphicAPI(controller, config, 0, 0) {}

    void draw(RDP::FrameMarker    const & /*cmd*/) override {}
    void draw(RDPDestBlt          const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPMultiDstBlt      const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPScrBlt           const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDP::RDPMultiScrBlt const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPMemBlt           const & /*cmd*/, Rect /*clip*/, Bitmap const & /*bmp*/) override {}
    void draw(RDPBitmapData       const & /*cmd*/, Bitmap const & /*bmp*/) override {}

    void draw(RDPPatBlt           const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDP::RDPMultiPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPOpaqueRect       const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPMultiOpaqueRect  const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPLineTo           const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPPolygonSC        const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPPolygonCB        const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPPolyline         const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPEllipseSC        const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPEllipseCB        const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override {}
    void draw(RDPMem3Blt          const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, Bitmap const & /*bmp*/) override {}
    void draw(RDPGlyphIndex       const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, GlyphCache const & /*gly_cache*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/) override { }
    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const &/*content*/) override { }

    void draw(const RDP::RAIL::NewOrExistingWindow & /*cmd*/) override {}
    void draw(const RDP::RAIL::WindowIcon & /*cmd*/) override {}
    void draw(const RDP::RAIL::CachedIcon & /*cmd*/) override {}
    void draw(const RDP::RAIL::DeletedWindow & /*cmd*/) override {}
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*cmd*/) override {}
    void draw(const RDP::RAIL::DeletedNotificationIcons & /*cmd*/) override {}
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & /*cmd*/) override {}
    void draw(const RDP::RAIL::NonMonitoredDesktop & /*cmd*/) override {}

    void draw(RDPColCache   const & /*cmd*/) override {}
    void draw(RDPBrushCache const & /*cmd*/) override {}

    void set_pointer(uint16_t /*cache_idx*/, Pointer const& /*cursor*/, SetPointerMode /*mode*/) override {}
};

// class FakeClientInputMouseKeyboard : public ClientInputMouseKeyboardAPI {
//
// public:
//     FakeClientInputMouseKeyboard() = default;
//
//     // TODO string_view
//     void virtual keyPressEvent(const int key, std::string const& text) override { (void)key; (void)text; }
//
//     // TODO string_view
//     void virtual keyReleaseEvent(const int key, std::string const& text) override { (void)key; (void)text; }
//
//     virtual void init_form() override {}
//     virtual void update_keylayout() override {}
// };

class FakeClientKeyLayout : public ClientKeyLayoutAPI {

// private:
//     ClientCallback * callback = nullptr;
//     ClientRedemptionConfig * config = nullptr;

public:

    FakeClientKeyLayout() = default;


    void update_keylayout(const int /*LCID*/) override {}

    void setCustomKeyCode(const int /*qtKeyID*/, const int /*scanCode*/, const std::string & /*ASCII8*/, const int /*extended*/) override {}

    void init(const int /*flag*/, const int /*key*/, std::string const& /*text*/) override {}

    int get_scancode() override {
        return 0;
    }

    int get_flag() override {
        return 0;
    }

    void clearCustomKeyCode() override {}

    KeyCustomDefinition get_key_info(int /*keycode*/, std::string const& /*text*/) override {
        return KeyCustomDefinition{};
    }
};



class FakeClient : public ClientRedemptionAPI
{
    CHANNELS::ChannelDefArray channels;

public:
    int read_stream_index = -1;
    int read_stream_sub_index = -1;

    FakeRDPChannelsMod fake_mod;

    FakeClient() = default;

    void close() override {}

    size_t get_total_stream_produced() const
    {
        return this->fake_mod.index_in;
    }

    // TODO ??????
    FakeRDPChannelsMod::PDUData * stream() {
        if (this->fake_mod.index_out < 10) {
            this->fake_mod.index_out++;
            return &(this->fake_mod.last_pdu[this->fake_mod.index_out-1]);
        }

        return nullptr;
    }

    bool must_be_stop_capture() override { return true;}
    const CHANNELS::ChannelDefArray & get_channel_list() const override { return this->channels;}
    ResizeResult server_resize(ScreenInfo /*screen_server*/) override { return ResizeResult::instant_done;}
    int wait_and_draw_event(std::chrono::milliseconds /*timeout*/) override { return 0; }

    void session_update(timeval /*now*/, LogId /*id*/, KVList /*kv_list*/) override {}
    void possible_active_window_change() override {}

};



class FakeIODisk : public ClientIODiskAPI
{

public:
    int fil_size = 0;

    FakeIODisk() = default;

    bool ifile_good(const char * /*new_path*/) override {
        return true;
    }

    bool ofile_good(const char * /*new_path*/) override {
        return true;
    }

    bool dir_good(const char * /*new_path*/) override {
        return true;
    }

    void marke_dir(const char * /*new_path*/) override {

    }

    FileStat get_file_stat(const char * file_to_request) override {
        (void)file_to_request;
        return FileStat{};
    }

    FileStatvfs get_file_statvfs(const char * file_to_request) override {
        (void)file_to_request;
        return FileStatvfs{};
    }

    void read_data(std::string const& file_to_tread, int offset, writable_bytes_view data) override {
        (void)file_to_tread;
        (void)offset;
        (void)data;
    }

    bool set_elem_from_dir(std::vector<std::string> & elem_list, const std::string & str_dir_path) override {
        (void)str_dir_path;
        elem_list.clear();

        return true;
    }

    int get_device(const char * /*file_path*/) override {
        return 0;
    }

    uint32_t get_volume_serial_number(int /*device*/) override {

        return 0;
    }

    bool write_file(const char * file_to_write, bytes_view data) override {
        (void)file_to_write;
        (void)data;

        return true;
    }

    bool remove_file(const char * /*file_to_remove*/) override {
        return false;
    }

    bool rename_file(const char * file_to_rename,  const char * new_name) override {
        (void)file_to_rename;
        (void)new_name;
        return false;
    }

    int get_file_size(const char * path) override {
        (void) path;

        return this->fil_size;
    }
};
