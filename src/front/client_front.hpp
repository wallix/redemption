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
#include "utils/sugar/scope_exit.hpp"

#include "core/front_api.hpp"
#include "core/client_info.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/channel_list.hpp"
#include "gdi/graphic_cmd_color.hpp"

class ClientFront : public FrontAPI
{
    bool verbose;
    ClientInfo &info;
    CHANNELS::ChannelDefArray   cl;
    bool is_capture_state_;

public:
    ClientFront(ClientInfo & info, bool verbose)
    : verbose(verbose)
    , info(info)
    , is_capture_state_(false)
    {}

    bool can_be_start_capture() override
    {
        this->is_capture_state_ = true;
        return false;
    }

    bool must_be_stop_capture() override
    {
        return false;
    }

    bool is_capture_state() const
    {
        return this->is_capture_state_;
    }

    void flush() { }

    void draw(RDPOpaqueRect const &, Rect, gdi::ColorCtx) override { }
    void draw(const RDPScrBlt &, Rect) override { }
    void draw(const RDPDestBlt &, Rect) override { }
    void draw(const RDPMultiDstBlt &, Rect) override { }
    void draw(RDPMultiOpaqueRect const &, Rect, gdi::ColorCtx) override { }
    void draw(RDP::RDPMultiPatBlt const &, Rect, gdi::ColorCtx) override { }
    void draw(const RDP::RDPMultiScrBlt &, Rect) override { }
    void draw(RDPPatBlt const &, Rect, gdi::ColorCtx) override { }
    void draw(const RDPMemBlt &, Rect, const Bitmap &) override { }
    void draw(RDPMem3Blt const &, Rect, gdi::ColorCtx, const Bitmap &) override { }
    void draw(RDPLineTo const &, Rect, gdi::ColorCtx) override { }
    void draw(RDPGlyphIndex const &, Rect, gdi::ColorCtx, const GlyphCache &) override { }
    void draw(RDPPolygonSC const &, Rect, gdi::ColorCtx) override { }
    void draw(RDPPolygonCB const &, Rect, gdi::ColorCtx) override { }
    void draw(RDPPolyline const &, Rect, gdi::ColorCtx) override { }
    void draw(RDPEllipseSC const &, Rect, gdi::ColorCtx) override { }
    void draw(RDPEllipseCB const &, Rect, gdi::ColorCtx) override { }
    void draw(const RDPColCache   &) override { }
    void draw(const RDPBrushCache &) override { }
    void draw(const RDP::FrameMarker &) override { }
    void draw(const RDP::RAIL::NewOrExistingWindow &) override { }
    void draw(const RDP::RAIL::WindowIcon &) override { }
    void draw(const RDP::RAIL::CachedIcon &) override { }
    void draw(const RDP::RAIL::DeletedWindow &) override { }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons &) override { }
    void draw(const RDP::RAIL::DeletedNotificationIcons &) override { }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop &) override { }
    void draw(const RDP::RAIL::NonMonitoredDesktop &) override { }
    void draw(const RDPBitmapData &, const Bitmap &) override { }
    void set_palette(const BGRPalette&) override { }
    void draw(RDPNineGrid const & , Rect , gdi::ColorCtx , Bitmap const &) override {}


    ResizeResult server_resize(int width, int height, int bpp) override {
        this->info.bpp = bpp;
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return ResizeResult::instant_done;
    }

    void set_pointer(const Pointer &) override { }

    void begin_update() override { }
    void end_update() override { }

    const CHANNELS::ChannelDefArray & get_channel_list() const override { return cl; }

    void send_to_channel( const CHANNELS::ChannelDef &, const uint8_t *, std::size_t
                        , std::size_t, int) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void update_pointer_position(uint16_t, uint16_t) override {}
};

#include "mod/mod_api.hpp"

struct Select
{
    Select(char const * type, int sck_fd, mod_api & mod, gdi::GraphicApi & gd)
      : type(type)
      , sck_fd(sck_fd)
      , mod(mod)
      , gd(gd)
    {
        io_fd_zero(rfds);
    }

    enum class Res { Ok, Error, Continue, Timeout, };

    Res next_event(timeval & timeout)
    {
        auto & event = mod.get_event();
        event.wait_on_timeout(timeout);
        io_fd_set(sck_fd, rfds);

        int num = select(sck_fd + 1, &rfds, nullptr, nullptr, &timeout);

        if (num < 0) {
            if (errno == EINTR) {
                return Res::Continue;
            }

            LOG(LOG_INFO, "%s CLIENT :: errno = %d\n", type, errno);
            return Res::Error;
        }

        if (event.is_set(sck_fd, rfds)) {
            LOG(LOG_INFO, "%s CLIENT :: draw_event", type);
            mod.draw_event(time(nullptr), gd);
            return Res::Ok;
        }

        return Res::Timeout;
    }

private:
    fd_set rfds;
    char const * type;
    int sck_fd;
    mod_api & mod;
    gdi::GraphicApi & gd;
};

inline int run_connection_test(char const * type, int sck_fd, mod_api & mod, gdi::GraphicApi & gd)
{
    int       timeout_counter = 0;
    int const timeout_counter_max = 3;

    Select select(type, sck_fd, mod, gd);

    for (;;) {
        LOG(LOG_INFO, "run_connection_test");
        timeval timeout = {5, 0};

        switch (select.next_event(timeout))
        {
            case Select::Res::Error: return 1;
            case Select::Res::Continue: break;
            case Select::Res::Timeout:
                ++timeout_counter;
                LOG(LOG_INFO, "%s CLIENT :: Timeout (%d/%d)", type, timeout_counter, timeout_counter_max);
                if (timeout_counter == timeout_counter_max) {
                    return 2;
                }
                break;
            case Select::Res::Ok:
                if (mod.is_up_and_running()) {
                    LOG(LOG_INFO, "%s CLIENT :: Done", type);
                    return 0;
                }
        }
    }
}


inline int wait_for_screenshot(char const * type, int sck_fd, mod_api & mod, gdi::GraphicApi & gd, std::chrono::milliseconds inactivity_time, std::chrono::milliseconds max_time)
{
    auto const time_start = ustime();

    Select select(type, sck_fd, mod, gd);

    for (;;) {
        auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            ustime() - time_start);

        if (elapsed >= max_time) {
            return 0;
        }

        auto const ms = std::min(max_time - elapsed, inactivity_time);
        auto const seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
        timeval timeout = {seconds.count(), (ms - seconds).count()};

        switch (select.next_event(timeout))
        {
            case Select::Res::Error: return 1;
            case Select::Res::Continue:
            case Select::Res::Timeout:
            case Select::Res::Ok:
                if (!timeout.tv_sec && !timeout.tv_usec) {
                    return 0;
                }
        }
    }
}

inline int run_test_client(char const * type, int sck_fd, mod_api & mod, gdi::GraphicApi & gd, std::chrono::milliseconds inactivity_time, std::chrono::milliseconds max_time, std::string const & screen_output)
{
    try {
        if (int err = run_connection_test(type, sck_fd, mod, gd)) {
            return err;
        }

        if (screen_output.empty()) {
            return 0;
        }

        FILE * f = fopen(screen_output.c_str(), "w");
        if (!f) {
            LOG(LOG_ERR, "%s CLIENT :: %s: %s", type, screen_output.c_str(), strerror(errno));
            return 1;
        }
        SCOPE_EXIT(fclose(f));

        Dimension dim = mod.get_dim();
        RDPDrawable gd(dim.w, dim.h);

        if (int err = wait_for_screenshot(type, sck_fd, mod, gd, inactivity_time, max_time)) {
            return err;
        }

        dump_png24(f, gd, true);

        return 0;
    }
    catch (Error const & e) {
        LOG(LOG_ERR, "%s CLIENT :: Exception raised = %s !\n", type, e.errmsg());
        return 1;
    }
}
