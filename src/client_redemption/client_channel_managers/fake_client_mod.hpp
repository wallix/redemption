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


#include "client_redemption/client_input_output_api/client_clipboard_api.hpp"
#include "client_redemption/client_input_output_api/client_graphic_api.hpp"
#include "client_redemption/client_input_output_api/client_mouse_keyboard_api.hpp"
#include "client_redemption/client_input_output_api/client_sound_api.hpp"
#include "client_redemption/client_input_output_api/client_clipboard_api.hpp"
#include "client_redemption/client_input_output_api/client_iodisk_api.hpp"


#include "client_redemption/client_redemption_api.hpp"


#include "core/RDP/clipboard.hpp"
#include "utils/sugar/byte.hpp"


class FakeRDPChannelsMod : public mod_api
{
public:

    struct PDUData {
        uint8_t data[1600] = {0};
        size_t size = 0;
    } last_pdu[10];

    int index_in = 0;
    int index_out = 0;


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

    void draw_event(time_t now, gdi::GraphicApi & drawable) override { (void) now; (void) drawable; }

    void send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name,
        uint8_t const * data, size_t length, size_t chunk_size, int flags) override {
            (void) mod_channel_name;
            (void) data;
            (void) length;
            (void) chunk_size;
            (void) flags;
    }
};



class FakeClientIOClipboard : public ClientIOClipboardAPI
{
public:
    std::string data_text;
    std::unique_ptr<uint8_t[]>  _chunk;
    void emptyBuffer() override {}

    //  set distant clipboard data
    void setClipboard_text(std::string const& str) override {
        this->data_text = str;
    }

    uint8_t * get_text() override {
        return this->_chunk.get();
    }

    void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const int bpp) override {
        (void) data;
        (void) image_width;
        (void) image_height;
        (void) bpp;
    }
    void setClipboard_files(std::string const& /*name*/) override {}
    void write_clipboard_temp_file(std::string const& fileName, const uint8_t * data, size_t data_len) override {
        (void) fileName;
        (void) data;
        (void) data_len;
    }
};

class FakeClientOutPutSound : public ClientOutputSoundAPI {

public:
    void init(size_t raw_total_size) override {(void) raw_total_size; }
    void setData(const uint8_t * data, size_t size) override {(void) data; (void) size; }
    void play() override {}
};


class FakeClientOutputGraphic : public ClientOutputGraphicAPI {

public:
    FakeClientOutputGraphic() : ClientOutputGraphicAPI(0, 0) {}

    void set_ErrorMsg(std::string const & movie_path) override { (void)movie_path; }

    void dropScreen() override {}

    void show_screen() override {}

    void reset_cache(int w,  int h) override { (void)w; (void)h; }

    void create_screen() override {}

    void closeFromScreen() override {}

    void set_screen_size(int x, int y) override { (void)x; (void)y; }

    void update_screen() override {}


    // TODO bpp -> gdi::Depth
    FrontAPI::ResizeResult server_resize(int width, int height, int bpp) override
    {
        (void)width;
        (void)height;
        (void)bpp;
        return FrontAPI::ResizeResult::done;
    }

    void draw(RDP::FrameMarker    const & /*cmd*/) override {}
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}
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
};

class FakeClientInputMouseKeyboard : public ClientInputMouseKeyboardAPI {

public:
    FakeClientInputMouseKeyboard() : ClientInputMouseKeyboardAPI() {}

    // TODO string_view
    void virtual keyPressEvent(const int key, std::string const& text) override { (void)key; (void)text; }

    // TODO string_view
    void virtual keyReleaseEvent(const int key, std::string const& text) override { (void)key; (void)text; }

    virtual void init_form() override {}
    virtual void update_keylayout() override {}
};


class FakeClient : public ClientRedemptionAPI
{
    CHANNELS::ChannelDefArray channels;

public:
    int read_stream_index = -1;
    int read_stream_sub_index = -1;

    FakeRDPChannelsMod fake_mod;

    FakeClient(SessionReactor& session_reactor)
      : ClientRedemptionAPI( session_reactor)
    {
        this->mod = &(this->fake_mod);
    }

    size_t get_total_stream_produced() {
        return this->fake_mod.index_in;
    }

    FakeRDPChannelsMod::PDUData * stream() {
        if (this->fake_mod.index_out < 10) {
            this->fake_mod.index_out++;
            return &(this->fake_mod.last_pdu[this->fake_mod.index_out-1]);
        }

        return nullptr;
    }

    void draw(RDP::FrameMarker    const & cmd) override { (void) cmd; }
    void draw(RDPNineGrid const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { (void) cmd; (void) clip; (void) color_ctx; (void) bmp; }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { (void) cmd; (void) clip; }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { (void) cmd; (void) clip; }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { (void) cmd; (void) clip; }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { (void) cmd; (void) clip; }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { (void) cmd; (void) clip; (void) bmp; }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { (void) cmd; (void) bmp; }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { (void) cmd; (void) clip; (void) color_ctx; }
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { (void) cmd; (void) bmp; (void) clip; (void)color_ctx; }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { (void) cmd; (void) clip; (void) gly_cache; (void) color_ctx;}
    using ClientRedemptionAPI::draw;
    bool must_be_stop_capture() override { return true;}
    const CHANNELS::ChannelDefArray & get_channel_list() const override { return this->channels;}
    ResizeResult server_resize(int  /*width*/, int  /*height*/, int  /*bpp*/) override { return ResizeResult::instant_done;}
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

    void read_data(std::string const& file_to_tread, int offset, byte_array data) override {
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

    bool write_file(const char * file_to_write, const char * data, int data_len) override {
        (void)file_to_write;
        (void)data;
        (void)data_len;

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
