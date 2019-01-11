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

#include "utils/log.hpp"
#include "utils/png.hpp"
#include "utils/select.hpp"
#include "utils/difftimeval.hpp"
#include "utils/file.hpp"

#include "core/front_api.hpp"
#include "core/client_info.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/channel_list.hpp"
#include "cxx/cxx.hpp"
#include "front/execute_events.hpp"
#include "gdi/graphic_cmd_color.hpp"


class ClientFront : public FrontAPI
{
    bool verbose;
    ScreenInfo& screen_info;
    CHANNELS::ChannelDefArray   cl;

public:
    ClientFront(ScreenInfo& screen_info, bool verbose)
    : verbose(verbose)
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
    void draw(const RDPBitmapData & /*cmd*/, const Bitmap & /*bmp*/) override { }
    void set_palette(const BGRPalette& /*unused*/) override { }
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const & /*unused*/) override {}


    ResizeResult server_resize(int width, int height, BitsPerPixel bpp) override {
        this->screen_info.width = width;
        this->screen_info.height = height;
        this->screen_info.bpp = bpp;
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return ResizeResult::instant_done;
    }

    void set_pointer(const Pointer & /*unused*/) override { }

    void begin_update() override { }
    void end_update() override { }

    const CHANNELS::ChannelDefArray & get_channel_list() const override { return cl; }

    void send_to_channel( const CHANNELS::ChannelDef & /*channel*/, const uint8_t * /*data*/, std::size_t
                         /*length*/, std::size_t /*chunk_size*/, int /*flags*/) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void update_pointer_position(uint16_t /*unused*/, uint16_t /*unused*/) override {}
};

#include "mod/mod_api.hpp"

inline int run_connection_test(
    char const * type, SessionReactor& session_reactor, mod_api& mod, gdi::GraphicApi& gd)
{
    int       timeout_counter = 0;
    int const timeout_counter_max = 3;
    std::chrono::milliseconds const timeout = 5s;

    for (;;) {
        LOG(LOG_INFO, "run_connection_test");

        switch (execute_events(
            timeout, session_reactor,
            SessionReactor::EnableGraphics{true}, mod, gd
        )) {
            case ExecuteEventsResult::Error:
                LOG(LOG_INFO, "%s CLIENT :: errno = %d\n", type, errno);
                return 1;
            case ExecuteEventsResult::Continue:
                break;
            case ExecuteEventsResult::Timeout:
                ++timeout_counter;
                LOG(LOG_INFO, "%s CLIENT :: Timeout (%d/%d)", type, timeout_counter, timeout_counter_max);
                if (timeout_counter == timeout_counter_max) {
                    return 2;
                }
                break;
            case ExecuteEventsResult::Success:
                if (mod.is_up_and_running()) {
                    LOG(LOG_INFO, "%s CLIENT :: Done", type);
                    return 0;
                }
                break;
        }
    }
}

// return 0 : do screenshot, don't do screenshot an error occurred
inline int wait_for_screenshot(
    char const* type, SessionReactor& session_reactor, Callback& callback, gdi::GraphicApi & gd,
    std::chrono::milliseconds inactivity_time, std::chrono::milliseconds max_time)
{
    auto const time_start = ustime();

    for (;;) {
        auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            ustime() - time_start);

        if (elapsed >= max_time) {
            return 0;
        }

        std::chrono::milliseconds timeout = std::min(max_time - elapsed, inactivity_time);

        switch (execute_events(
            timeout, session_reactor,
            SessionReactor::EnableGraphics{true}, callback, gd
        )) {
            case ExecuteEventsResult::Error:
                LOG(LOG_INFO, "%s CLIENT :: errno = %d\n", type, errno);
                return 1;
            case ExecuteEventsResult::Success:
                LOG(LOG_INFO, "%s CLIENT :: draw_event", type);
                REDEMPTION_CXX_FALLTHROUGH;
            case ExecuteEventsResult::Continue:
            case ExecuteEventsResult::Timeout:
                if (timeout == 0ms) {
                    return 0;
                }
        }
    }
}

inline int run_test_client(
    char const* type, SessionReactor& session_reactor, mod_api& mod, gdi::GraphicApi& gd,
    std::chrono::milliseconds inactivity_time, std::chrono::milliseconds max_time,
    std::string const& screen_output)
{
    try {
        if (int err = run_connection_test(type, session_reactor, mod, gd)) {
            return err;
        }

        if (screen_output.empty()) {
            return 0;
        }

        File f(screen_output, "w");
        if (!f) {
            LOG(LOG_ERR, "%s CLIENT :: %s: %s", type, screen_output, strerror(errno));
            return ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE;
        }

        Dimension dim = mod.get_dim();
        RDPDrawable gd(dim.w, dim.h);

        if (int err = wait_for_screenshot(type, session_reactor, mod, gd, inactivity_time, max_time)) {
            return err;
        }

        dump_png24(f.get(), gd, true);

        return 0;
    }
    catch (Error const & e) {
        LOG(LOG_ERR, "%s CLIENT :: Exception raised = %s !\n", type, e.errmsg());
        return e.id;
    }
}
