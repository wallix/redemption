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


class GraphicCaptureImpl
{
public:
    using PtrColorConverter = std::unique_ptr<gdi::GraphicApi>;
    using GdRef = std::reference_wrapper<gdi::GraphicApi>;

    struct Graphic final
    : gdi::GraphicProxyBase<Graphic>
    {
        friend gdi::GraphicCoreAccess;

        PtrColorConverter cmd_color_distributor;
        MouseTrace const & mouse;
        std::vector<GdRef> gds;
        std::vector<std::reference_wrapper<gdi::CaptureApi>> snapshoters;

        gdi::GraphicDepth order_depth = gdi::GraphicDepth::unspecified();
        gdi::RngByBpp<std::vector<GdRef>::iterator> rng_by_bpp;

        Graphic(gdi::GraphicDepth const & depth, MouseTrace const & mouse)
        : Graphic::base_type(depth)
        , mouse(mouse)
        {}

        gdi::GraphicDispatcherList<std::vector<GdRef>>
        get_graphic_proxy() {
            return {this->gds};
        }

        template<class Cmd, class... Ts>
        void draw_impl(Cmd const & cmd, Ts const & ... args)
        {
            if (gdi::GraphicCmdColor::is_encodable_cmd_color(cmd)) {
                assert(gdi::GraphicDepth::unspecified() != this->order_depth);
                gdi::draw_cmd_color_convert(order_depth, this->rng_by_bpp, cmd, args...);
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
        this->graphic_api.order_depth = order_depth;
    }

    GraphicApi & get_graphic_api() { return this->graphic_api; }

    Drawable & impl() { return this->drawable.impl(); }
    RDPDrawable & rdp_drawable() { return this->drawable; }
};

