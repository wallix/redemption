/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/


#pragma once

#include "mouse_trace.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "gdi/graphic_cmd_color_converter.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "utils/sugar/range.hpp"


namespace gdi {

    template<class Graphic>
    struct GraphicUniformDistribution
    {
        Graphic graphic_;

        template<class... Ts>
        void operator()(draw_tag, Ts const & ... args) {
            this->graphic_.draw(args...);
        }

        void operator()(set_tag, Pointer const & pointer) {
            this->graphic_.set_pointer(pointer);
        }

        void operator()(set_tag, BGRPalette const & palette) {
            this->graphic_.set_palette(palette);
        }

        void operator()(sync_tag) {
            this->graphic_.sync();
        }

        void operator()(set_row_tag, std::size_t rownum, const uint8_t * data) {
            this->graphic_.set_row(rownum, data);
        }

        void operator()(begin_update_tag) {
            this->graphic_.begin_update();
        }

        void operator()(end_update_tag) {
            this->graphic_.end_update();
        }
    };

    struct self_fn
    {
        template<class T> T & operator()(T & x) const { return x; }
        template<class T> T operator()(T && x) const { return std::move(x); }

        template<class T> T & operator()(std::reference_wrapper<T> x) const { return x; }
    };

    struct GraphicUniformProjection
    {
        self_fn projection_;

        template<class Graphic>
        auto operator()(Graphic & graphic)
        -> GraphicUniformDistribution<decltype(projection_(graphic))>
        {
            return {this->projection_(graphic)};
        }
    };

    template<class GraphicList>
    struct GraphicUniformDispatcherList
    {
        GraphicList & graphics_;
        GraphicUniformProjection projection_;

        template<class Tag, class... Ts>
        void operator()(Tag tag, Ts const & ... args) {
            for (auto && graphic : this->graphics_) {
                this->projection_(graphic)(tag, args...);
            }
        }
    };

    template<class GraphicList>
    struct GraphicDispatcherList : GraphicUniformProxy<GraphicUniformDispatcherList<GraphicList>>
    {
        GraphicDispatcherList(GraphicList & graphic_list, self_fn proj = self_fn{})
        : GraphicDispatcherList::proxy_type{{graphic_list, GraphicUniformProjection{proj}}}
        {}
    };
}

class GraphicCaptureImpl
{
public:
    using PtrColorConverter = std::unique_ptr<gdi::GraphicApi>;
    using GdRef = std::reference_wrapper<gdi::GraphicApi>;

    struct Graphic final : public gdi::GraphicApi
    {
    public:
        void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPPatBlt           const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPLineTo           const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolyline         const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
        void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
        void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
        void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

        void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

        void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

        void set_pointer(Pointer    const & pointer) override {
            this->get_graphic_proxy().set_pointer(pointer);
        }

        void set_palette(BGRPalette const & palette) override {
            this->get_graphic_proxy().set_palette(palette);
        }

        void sync() override {
            this->get_graphic_proxy().sync();
        }

        void set_row(std::size_t rownum, const uint8_t * data) override {
            this->get_graphic_proxy().set_row(rownum, data);
        }

        void begin_update() override {
            this->get_graphic_proxy().begin_update();
        }

        void end_update() override {
            this->get_graphic_proxy().end_update();
        }

    protected:
        template<class... Ts>
        void draw_impl(Ts const & ... args) {
            this->get_graphic_proxy().draw(args...);
        }

    public:
        PtrColorConverter cmd_color_distributor;
        MouseTrace const & mouse;
        std::vector<GdRef> gds;
        std::vector<std::reference_wrapper<gdi::CaptureApi>> snapshoters;

        gdi::GraphicDepth order_depth_ = gdi::GraphicDepth::unspecified();
        gdi::RngByBpp<std::vector<GdRef>::iterator> rng_by_bpp;

        Graphic(gdi::GraphicDepth const & depth, MouseTrace const & mouse)
        : mouse(mouse)
        {}

        gdi::GraphicDispatcherList<std::vector<GdRef>>
        get_graphic_proxy() {
            return {this->gds};
        }

        template<class Cmd, class... Ts>
        void draw_impl(Cmd const & cmd, Ts const & ... args)
        {
            if (gdi::GraphicCmdColor::is_encodable_cmd_color(cmd)) {
                assert(gdi::GraphicDepth::unspecified() != this->order_depth_);
                gdi::draw_cmd_color_convert(this->order_depth_, this->rng_by_bpp, cmd, args...);
            }
            else {
                this->get_graphic_proxy().draw(cmd, args...);
            }
        }

        void draw_impl(RDP::FrameMarker const & cmd) {
            for (gdi::GraphicApi & gd : this->gds) {
                gd.draw(cmd);
            }

            if (cmd.action == RDP::FrameMarker::FrameEnd) {
                for (gdi::CaptureApi & cap : this->snapshoters) {
                    cap.snapshot(
                        this->mouse.last_now,
                        this->mouse.last_x,
                        this->mouse.last_y,
                        false
                    );
                }
            }
        }

        void set_depths(gdi::GraphicDepth const & depth) override {
            this->order_depth_ = depth;
        }

        gdi::GraphicDepth const & order_depth() const override {
            return this->order_depth_;
        }

    };

    Graphic graphic_api;
    RDPDrawable drawable;
    uint8_t order_bpp;

public:
    using GraphicApi = Graphic;

    GraphicCaptureImpl(uint16_t width, uint16_t height, uint8_t order_bpp, MouseTrace const & mouse)
    : graphic_api(gdi::GraphicDepth::unspecified(), mouse)
    , drawable(width, height, order_bpp)
    , order_bpp(order_bpp)
    {
    }

    void update_order_bpp(uint8_t order_bpp) {
        if (this->order_bpp != order_bpp) {
            this->order_bpp = order_bpp;
            this->drawable.set_depths(gdi::GraphicDepth::from_bpp(order_bpp));
            this->start();
        }
    }

    void start()
    {
        auto const order_depth = gdi::GraphicDepth::from_bpp(this->order_bpp);
        auto & gds = this->graphic_api.gds;
        this->graphic_api.rng_by_bpp = {order_depth, gds.begin(), gds.end()};
        this->graphic_api.order_depth_ = order_depth;
    }

    GraphicApi & get_graphic_api() { return this->graphic_api; }

    Drawable & impl() { return this->drawable.impl(); }
    RDPDrawable & rdp_drawable() { return this->drawable; }
};

