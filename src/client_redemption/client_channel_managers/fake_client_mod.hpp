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


#include "client_redemption/client_input_output_api.hpp"

#include "core/RDP/clipboard.hpp"


class FakeRDPChannelsMod : public mod_api
{
public:
//     RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu;

    std::vector<uint16_t> types;
    std::vector<uint16_t> sub_types;

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override {
        (void) front_channel_name;
        (void) length;
        (void) flags;

        this->types.push_back(chunk.in_uint16_le());
        this->sub_types.push_back(chunk.in_uint16_le());

        //InStream stream =  chunk.clone();

//         switch (chunk.in_uint16_le()) {
//
//             case RDPECLIP::CB_MONITOR_READY:
//                 break;
//             case RDPECLIP::CB_FORMAT_LIST:
//     case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
//     case RDPECLIP::CB_FORMAT_DATA_REQUEST:
//     case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
//     case RDPECLIP::CB_TEMP_DIRECTORY:
//     case RDPECLIP::CB_CLIP_CAPS:
//     case RDPECLIP::CB_FILECONTENTS_REQUEST:
//     case RDPECLIP::CB_FILECONTENTS_RESPONSE:
//     case RDPECLIP::CB_LOCK_CLIPDATA:
//     case RDPECLIP::CB_UNLOCK_CLIPDATA:
//
//         }

        //this->streams.push_back(chunk.clone());
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
    void emptyBuffer() {}

    //  set distant clipboard data
    void setClipboard_text(std::string & str) override {
        this->data_text = str;
    }

    void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const int bpp) override {
        (void) data;
        (void) image_width;
        (void) image_height;
        (void) bpp;
    }
    void setClipboard_files(std::string & name) override {}
    void write_clipboard_temp_file(std::string fileName, const uint8_t * data, size_t data_len) override {
        (void) fileName;
        (void) data;
        (void) data_len;
    }

        // image data
    // TODO should be `ImageDataView get_image_view()`
    int get_image_buffer_width() override {return 0;}
    int get_image_buffer_height() override {return 0;}
    uint8_t * get_image_buffer_data() override {return 0;}
    int get_image_buffer_depth() override {return 0;}

        // files data (file index to identify a file among a files group descriptor)
    std::string get_file_item_name(int index) override {return std::string("");}
    // TODO should be `array_view_const_char get_file_item_size(int index)`
    int get_file_item_size(int index) override {(void) index; return 0;}
    char * get_file_item_data(int index) override {(void) index; return const_cast<char*>("");}

};

class FakeClientOutPutSound : public ClientOutputSoundAPI {

public:
    void init(size_t raw_total_size) override {(void) raw_total_size; };
    void setData(const uint8_t * data, size_t size) override {(void) data; (void) size; };
    void play() override {};
};



class FakeClient : public ClientRedemptionAPI
{

public:
    int read_stream_index = -1;
    int read_stream_sub_index = -1;

    FakeRDPChannelsMod fake_mod;

    FakeClient()
      : ClientRedemptionAPI()
    {
        this->mod = &(this->fake_mod);
    }

    size_t get_total_stream_produced() {
        return this->fake_mod.types.size();
    }

    uint16_t get_next_pdu_type() {
        this->read_stream_index++;
        if (this->read_stream_index < fake_mod.types.size()) {
            return fake_mod.types[this->read_stream_index];
        }

        return -1;
    }

    uint16_t get_next_pdu_sub_type() {
        this->read_stream_sub_index++;
        if (this->read_stream_sub_index < fake_mod.sub_types.size()) {
            return fake_mod.sub_types[this->read_stream_sub_index];
        }

        return -1;
    }



    void draw(RDP::FrameMarker    const & cmd) override { (void) cmd; };
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
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { (void) cmd; (void) bmp; (void) clip;}
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { (void) cmd; (void) clip; (void) gly_cache; (void) color_ctx;}
    bool must_be_stop_capture() override { return true;}
    const CHANNELS::ChannelDefArray & get_channel_list() const override {const CHANNELS::ChannelDefArray c; return c;}
    ResizeResult server_resize(int , int , int ) override { return ResizeResult::instant_done;}



};
