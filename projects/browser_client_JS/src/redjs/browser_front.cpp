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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "redjs/browser_front.hpp"
#include "redjs/clipboard.hpp"

#include "redjs/image_data_from_bitmap.hpp"
#include "redjs/image_data_from_pointer.hpp"

#include "red_emscripten/em_asm.hpp"

#include "gdi/screen_info.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/capabilities/order.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "utils/drawable_pointer.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/log.hpp"

#include "core/RDP/clipboard.hpp"
#include "client_redemption/client_channels/client_cliprdr_channel.hpp"

#include <numeric>


namespace
{
    struct Clip : ClientIOClipboardAPI
    {
        void emptyBuffer() override {}
        void set_local_clipboard_stream(bool) override {}

        void setClipboard_image(const uint8_t * /*data*/, const int /*image_width*/, const int /*image_height*/, const BitsPerPixel /*bpp*/) override {}
        void setClipboard_files(const std::string & /*name*/) override {}

        void setClipboard_text(const std::string & str) override
        {
            RED_EM_ASM(
                {
                    Module.RdpClientEventTable[$0].setClipboardUtf8(Pointer_stringify($1));
                },
                frontidx,
                str.data()
            );
        }

        void write_clipboard_temp_file(const std::string & /*fileName*/, const uint8_t * /*data*/, std::size_t /*data_len*/) override {}

        uint16_t get_buffer_type_id() override
        {
            return this->type_id;
        }

        int get_citems_number() override { return 1; }

        array_view_const_u8 get_cliboard_text() override
        {
            return this->clipboard_text_data;
        }

        size_t get_cliboard_data_length() override
        {
            return this->clipboard_text_data.size();
        }

        array_view_const_char get_file_item(int /*index*/) override { return {}; }
        std::string get_file_item_name(int /*index*/) override { return {}; }

        ConstImageDataView get_image() override
        {
            return ConstImageDataView{
                nullptr, 0, 0, 0, BytesPerPixel{},
                ConstImageDataView::Storage::BottomToTop
            };
        }

        void set_utf8(std::string_view utf8_string)
        {
            this->type_id = RDPECLIP::CF_UNICODETEXT;
            const size_t len = utf8_string.size() * 4;
            this->clipboard_text_data.resize(len + 2);
            auto* data = this->clipboard_text_data.data();
            auto real_len = UTF8toUTF16_CrLf(utf8_string, data, len);
            data[real_len] = 0;
            data[real_len + 1] = 0;
            this->clipboard_text_data.resize(real_len + 2);
        }

        void set_file(std::string_view /*name*/, std::vector<uint8_t> /*data*/)
        {
            // this->type_id = ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
        }

        int frontidx;

    private:
        std::vector<uint8_t> clipboard_text_data;
        uint16_t type_id = 0;
    };
}

namespace redjs
{

struct BrowserFront::Clipboard
{
    Clipboard(JsTableId id, RDPVerbose verbose)
    : clientCLIPRDRChannel(verbose, &channel_mod, []{
        RDPClipboardConfig config;
        config.add_format(RDPECLIP::CF_UNICODETEXT, {});
        return config;
    }())
    , clipboard(id, verbose)
    {
        clientCLIPRDRChannel.set_api(&clip);
        this->clip.frontidx = id.raw();
    }

    ClientChannelMod channel_mod;
    ClientCLIPRDRChannel clientCLIPRDRChannel;
    Clip clip;

    redjs::Clipboard clipboard;
};

BrowserFront::BrowserFront(redjs::JsTableId id, uint16_t width, uint16_t height, OrderCaps& order_caps, RDPVerbose verbose)
: gd(id, width, height, order_caps)
, verbose(verbose)
, clipboard(std::make_unique<Clipboard>(id, verbose))
{
    this->cl.push_back(CHANNELS::ChannelDef(channel_names::cliprdr, 0, 0));
}

BrowserFront::~BrowserFront() = default;

bool BrowserFront::can_be_start_capture()
{
    return false;
}

bool BrowserFront::must_be_stop_capture()
{
    return false;
}

BrowserFront::ResizeResult BrowserFront::server_resize(ScreenInfo screen_server)
{
    if (bool(this->verbose & RDPVerbose::graphics)) {
        LOG(LOG_INFO, "BrowserFront::server_resize(width=%d, height=%d, bpp=%d",
        screen_server.width, screen_server.height, screen_server.bpp);
    }

    return this->gd.resize_canvas(screen_server.width, screen_server.height)
        ? ResizeResult::instant_done
        : ResizeResult::fail;
}

void BrowserFront::set_mod(mod_api * mod)
{
    this->clipboard->channel_mod.set_mod(mod);
    this->clipboard->clipboard.set_cb(mod);
}

void BrowserFront::send_clipboard_utf8(std::string_view utf8_string)
{
    this->clipboard->clip.set_utf8(utf8_string);
    this->clipboard->clientCLIPRDRChannel.send_FormatListPDU();
}

void BrowserFront::send_file(std::string_view name, std::vector<uint8_t> data)
{
    this->clipboard->clip.set_file(name, std::move(data));
    this->clipboard->clientCLIPRDRChannel.send_FormatListPDU();
}


void BrowserFront::clipboard_send_request_format(uint32_t id)
{
    this->clipboard->clipboard.send_request_format(id);
}

void BrowserFront::send_to_channel(
    const CHANNELS::ChannelDef & channel, const uint8_t * data,
    std::size_t /*length*/, std::size_t chunk_size, int flags)
{
    LOG_IF(bool(this->verbose & RDPVerbose::channels),
        LOG_INFO, "BrowserFront::send_to_channel");

    assert(channel.name == channel_names::cliprdr);

    // InStream chunk(data, chunk_size);
    // this->clipboard->clientCLIPRDRChannel.receive(chunk, flags);
    this->clipboard->clipboard.receive(InStream(data, chunk_size), flags);
}

void BrowserFront::update_pointer_position(uint16_t /*unused*/, uint16_t /*unused*/)
{
    LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
        LOG_INFO, "BrowserFront::update_pointer_position");
}

} // namespace redjs
