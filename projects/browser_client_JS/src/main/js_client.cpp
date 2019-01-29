/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2015
 *
 * free RDP client main program
 *
 */

#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/set_server_redirection_target.hpp"
#include "front/client_front.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "utils/fixed_random.hpp"
#include "utils/genrandom.hpp"
#include "utils/redirection_info.hpp"
#include "utils/hexdump.hpp"
#include "utils/theme.hpp"

#include <memory>
#include <string>

#include "red_emscripten/bind.hpp"
#include "red_emscripten/em_js.hpp"
#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/val.hpp"

#include "redjs/image_data.hpp"


constexpr int FD_TRANS = 42;

struct BrowserTransport : Transport
{
    std::vector<std::string> in_buffers;
    std::vector<uint8_t> out_buffers;
    std::size_t current_pos = 0;

    TlsResult enable_client_tls(
        bool /*server_cert_store*/,
        ServerCertCheck /*server_cert_check*/,
        ServerNotifier & /*server_notifier*/,
        const char * /*certif_path*/
    ) override
    {
        LOG(LOG_ERR, "enable_client_tls");
        return TlsResult::Fail;
    }

    size_t do_partial_read(uint8_t * data, size_t len) override
    {
        if (in_buffers.empty()) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        auto remaining = len;

        while (remaining) {
            auto& s = in_buffers.front();
            auto const s_len = s.size() - current_pos;
            auto const min_len = std::min(s_len, remaining);
            memcpy(data, s.data() + current_pos, min_len);
            current_pos += min_len;
            remaining -= min_len;
            data += min_len;
            if (min_len == s_len) {
                current_pos = 0;
                in_buffers.erase(in_buffers.begin());
                if (in_buffers.empty()) {
                    break;
                }
            }
        }

        size_t const data_len = len - remaining;

        // LOG(LOG_DEBUG, "BrowserTransport::read %zu bytes", data_len);
        // hexdump(data - data_len, data_len);

        return data_len;
    }

    void do_send(const uint8_t * buffer, size_t len) override
    {
        // LOG(LOG_DEBUG, "BrowserTransport::send %zu bytes (total %zu)", len, out_buffers.size() + len);
        // hexdump(buffer, len);
        out_buffers.insert(out_buffers.end(), buffer, buffer + len);
    }

    int get_fd() const override { return FD_TRANS; }
};


#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/bitmap.hpp"

extern "C" {
  extern void draw_image(Bitmap& image);
}


class JsFront : public FrontAPI
{
public:
    JsFront(ScreenInfo& screen_info, bool verbose)
    : width(screen_info.width)
    , height(screen_info.height)
    , verbose(verbose)
    , screen_info(screen_info)
    {}

    bool can_be_start_capture() override
    {
        return false;
    }

    bool must_be_stop_capture() override
    {
        return false;
    }

    Rect intersect(Rect const& a, Rect const& b)
    {
        return a.intersect(width, height).intersect(b);
    }

    void draw(RDPOpaqueRect const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }

    void draw(const RDPScrBlt & /*cmd*/, Rect /*clip*/) override { }
    void draw(const RDPDestBlt & /*cmd*/, Rect /*clip*/) override { }
    void draw(const RDPMultiDstBlt & /*cmd*/, Rect /*clip*/) override { }
    void draw(RDPMultiOpaqueRect const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(RDP::RDPMultiPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(const RDP::RDPMultiScrBlt & /*cmd*/, Rect /*clip*/) override { }
    void draw(RDPPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }

    void draw(const RDPMemBlt & /*cmd*/, Rect /*clip*/, const Bitmap & /*bmp*/) override { }

    void draw(RDPMem3Blt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, const Bitmap & /*bmp*/) override { }
    void draw(RDPLineTo const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(RDPGlyphIndex const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, const GlyphCache & /*gly_cache*/) override { }
    void draw(RDPPolygonSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(RDPPolygonCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(RDPPolyline const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(RDPEllipseSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(RDPEllipseCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override { }
    void draw(const RDPColCache   & /*unused*/) override { }
    void draw(const RDPBrushCache & /*unused*/) override { }
    void draw(const RDP::FrameMarker & /*cmd*/) override { }
    void draw(const RDP::RAIL::NewOrExistingWindow & /*unused*/) override { }
    void draw(const RDP::RAIL::WindowIcon & /*unused*/) override { }
    void draw(const RDP::RAIL::CachedIcon & /*unused*/) override { }
    void draw(const RDP::RAIL::DeletedWindow & /*unused*/) override { }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*unused*/) override { }
    void draw(const RDP::RAIL::DeletedNotificationIcons & /*unused*/) override { }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & /*unused*/) override { }
    void draw(const RDP::RAIL::NonMonitoredDesktop & /*unused*/) override { }

    void draw(const RDPBitmapData & cmd, const Bitmap & bmp) override
    {
        // LOG(LOG_INFO, "JsFront::RDPBitmapData");

        // void ctx.drawImage(image, dx, dy);
        // void ctx.drawImage(image, dx, dy, dWidth, dHeight);
        // void ctx.drawImage(image, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight);

        redjs::ImageData image(bmp);

        RED_EM_ASM(
            {
                RdpClientEventTable[$0].drawImage($1, $2, $3, $4, $5, 0, 0, $6, $7);
            },
            this,
            image.data(),
            image.width(),
            image.height(),
            cmd.dest_left,
            cmd.dest_top,
            cmd.dest_right - cmd.dest_left + 1,
            cmd.dest_bottom - cmd.dest_top + 1
        );
    }

    void set_palette(const BGRPalette& /*unused*/) override { }
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const & /*unused*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const & /*content*/) override { }


    ResizeResult server_resize(int width, int height, BitsPerPixel bpp) override {
        this->width = width;
        this->height = height;
        this->screen_info.width = width;
        this->screen_info.height = height;
        this->screen_info.bpp = bpp;
        if (this->verbose) {
            LOG(LOG_INFO, "JsFront::server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
        }
        return ResizeResult::instant_done;
    }

    void set_pointer(const Pointer & /*unused*/) override { }

    void begin_update() override { }
    void end_update() override { }

    const CHANNELS::ChannelDefArray & get_channel_list() const override { return cl; }

    void send_to_channel(
        const CHANNELS::ChannelDef & /*channel*/, const uint8_t * /*data*/,
        std::size_t /*length*/, std::size_t /*chunk_size*/, int /*flags*/) override
    {
        if (this->verbose) {
            LOG(LOG_INFO, "JsFront::send_to_channel");
        }
    }

    void update_pointer_position(uint16_t /*unused*/, uint16_t /*unused*/) override
    {
        if (this->verbose) {
            LOG(LOG_INFO, "JsFront::update_pointer_position");
        }
    }

private:
    uint16_t width;
    uint16_t height;
    bool verbose;
    ScreenInfo& screen_info;
    CHANNELS::ChannelDefArray cl;
};


#ifndef JS_CLIENT_USERNAME
# define JS_CLIENT_USERNAME "x"
#endif
#ifndef JS_CLIENT_PASSWORD
# define JS_CLIENT_PASSWORD "x"
#endif

using Ms = std::chrono::milliseconds;

struct RdpClient
{
    std::unique_ptr<mod_api> mod;

    uint64_t verbose = 0xfffff;
    std::string target_device = "10.10.46.73";

    std::string screen_output;

    std::string username = JS_CLIENT_USERNAME;
    std::string password = JS_CLIENT_PASSWORD;

    BrowserTransport browser_trans;

    ClientInfo client_info;

    JsFront front = JsFront(client_info.screen_info, verbose);
    NullReportMessage report_message;
    SessionReactor session_reactor;
    TimeSystem system_timeobj;

    Inifile ini;

    // TODO JsRandom
    FixedRandom lcg_gen;
    LCGTime lcg_timeobj;
    NullAuthentifier authentifier;
    RedirectionInfo redir_info;

    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;


    RdpClient(unsigned long verbose)
    : verbose(verbose)
    {
        // TODO extra username/password from GET param

        client_info.screen_info.width = 800;
        client_info.screen_info.height = 600;
        client_info.screen_info.bpp = BitsPerPixel{24};

        ini.set<cfg::mod_rdp::server_redirection_support>(false);
        ini.set<cfg::mod_rdp::enable_nla>(false);
        ini.set<cfg::client::tls_fallback_legacy>(true);

        new_mod();
    }

    void new_mod()
    {
        ModRDPParams mod_rdp_params(
            username.c_str(),
            password.c_str(),
            target_device.c_str(),
            "0.0.0.0", // client ip is silenced
            /*front.keymap.key_flags*/ 0,
            font,
            theme,
            server_auto_reconnect_packet,
            close_box_extra_message,
            to_verbose_flags(verbose)
        );

        mod_rdp_params.device_id                  = "device_id";
        mod_rdp_params.enable_tls                 = false;
        mod_rdp_params.enable_nla                 = false;
        mod_rdp_params.enable_fastpath            = true;
        mod_rdp_params.enable_mem3blt             = true;
        mod_rdp_params.enable_new_pointer         = true;
        mod_rdp_params.enable_glyph_cache         = true;
        mod_rdp_params.enable_ninegrid_bitmap     = true;
        std::string allow_channels                = "*";
        mod_rdp_params.allow_channels             = &allow_channels;
        mod_rdp_params.deny_channels              = nullptr;
        mod_rdp_params.enable_rdpdr_data_analysis = false;
        mod_rdp_params.server_cert_check          = ServerCertCheck::always_succeed;

        if (verbose > 128) {
            mod_rdp_params.log();
        }

        this->mod = new_mod_rdp(
            browser_trans, session_reactor, front, client_info, redir_info, lcg_gen,
            lcg_timeobj, mod_rdp_params, authentifier, report_message, ini, nullptr);
    }

    /// \return milliseconds before next timer, or 0 if no timer
    Ms update_time(timeval current_time)
    {
        session_reactor.set_current_time(current_time);

        session_reactor.execute_timers(
            SessionReactor::EnableGraphics{true},
            [&]() -> gdi::GraphicApi& { return front; });

        std::chrono::microseconds us =
            session_reactor.get_next_timeout(SessionReactor::EnableGraphics{true}, 1h)
          - session_reactor.get_current_time();

        if (us <= 1ms) {
            return 1ms;
        }
        return std::chrono::duration_cast<Ms>(us);
    }

    const_bytes_view get_sending_data_view() const
    {
        return browser_trans.out_buffers;
    }

    void clear_sending_data()
    {
        browser_trans.out_buffers.clear();
    }

    void add_receiving_data(std::string data)
    {
        browser_trans.in_buffers.emplace_back(std::move(data));
        // browser_trans.out_buffers.insert(browser_trans.out_buffers.end(), data.begin(), data.end());
        session_reactor.execute_callbacks(*mod);
        auto fd_isset = [](int fd, auto& /*e*/){ return fd == FD_TRANS; };
        session_reactor.execute_graphics(fd_isset, front);
    }
};

// Binding code
EMSCRIPTEN_BINDINGS(client)
{
    redjs::class_<RdpClient>("RdpClient")
        .constructor<unsigned long>()
        /// long long is not embind type. Use long or double (safe for 53 bits);
        .function_ptr("updateTime", [](RdpClient& client) {
            Ms ms = client.update_time(tvtime());
            return static_cast<unsigned long>(ms.count());
        })
        .function_ptr("getSendingData", [](RdpClient& client) {
            return redjs::emval_from_view(client.get_sending_data_view());
        })
        .function_ptr("thisptr", +[](RdpClient& ref) {
            return RED_EM_ASM_INT({ return $0; }, &ref.front);
        })
        .function("clearSendingData", &RdpClient::clear_sending_data)
        .function("addReceivingData", &RdpClient::add_receiving_data)
    ;
}
