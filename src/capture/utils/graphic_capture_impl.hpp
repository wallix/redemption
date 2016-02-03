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

#ifndef REDEMPTION_CAPTURE_UTILS_GRAPHIC_CAPTURE_IMPL_HPP
#define REDEMPTION_CAPTURE_UTILS_GRAPHIC_CAPTURE_IMPL_HPP

#include "apis_register.hpp"
#include "mouse_trace.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "gdi/graphic_cmd_color_converter.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "utils/range.hpp"

class GraphicCaptureImpl
{
private:
    using PtrColorConverter = std::unique_ptr<gdi::GraphicApi>;
    using GdRef = std::reference_wrapper<gdi::GraphicApi>;

    // GraphicProxy::cmd_color_distributor
    //@{
    struct RngByBpp
    {
        using iterator = std::vector<GdRef>::const_iterator;

        iterator its[5];
        range<iterator> rng8() const { return {its[0], its[1]}; }
        range<iterator> rng15() const { return {its[1], its[2]}; }
        range<iterator> rng16() const { return {its[2], its[3]}; }
        range<iterator> rng24() const { return {its[3], its[4]}; }
        range<iterator> rng_all() const { return {its[0], its[4]}; }

        template<class... Ts>
        void apply(gdi::GraphicApi & gd, Ts const & ... args) const {
            gd.draw(args...);
        }
    };

    static PtrColorConverter choose_color_converter(std::vector<GdRef> gds, uint8_t order_bpp) {
        auto const order_depth = gdi::GraphicDepth::from_bpp(order_bpp);
        assert(order_depth.is_defined());
        std::sort(gds.begin(), gds.end(), [order_depth](gdi::GraphicApi const & a, gdi::GraphicApi const & b) {
            return a.order_depth().depth_or(order_depth).id() < b.order_depth().depth_or(order_depth).id();
        });

        RngByBpp rng_by_bpp{{gds.begin(), gds.begin(), gds.begin(), gds.begin(), gds.end()}};

        struct ge {
            gdi::GraphicDepth order_depth;
            gdi::GraphicDepth bpp;
            bool operator()(gdi::GraphicApi const & x) const {
                return x.order_depth().depth_or(order_depth).id() >= this->bpp.id();
            }
        };

        auto & its = rng_by_bpp.its;
        its[0] = std::find_if(its[0], its[4], ge{order_depth, gdi::GraphicDepth::depth8()});
        its[1] = std::find_if(its[0], its[4], ge{order_depth, gdi::GraphicDepth::depth15()});
        its[2] = std::find_if(its[1], its[4], ge{order_depth, gdi::GraphicDepth::depth16()});
        its[3] = std::find_if(its[2], its[4], ge{order_depth, gdi::GraphicDepth::depth24()});

        using dec8 = with_color8_palette<decode_color8_opaquerect>;
        switch (order_bpp) {
            case 8 : return choose_encoder(dec8{BGRPalette::classic_332_rgb()}, rng_by_bpp);
            case 15: return choose_encoder(decode_color15_opaquerect{}, rng_by_bpp);
            case 16: return choose_encoder(decode_color16_opaquerect{}, rng_by_bpp);
            case 24:
            case 32: return choose_encoder(decode_color24_opaquerect{}, rng_by_bpp);
            default: assert(nullptr); return PtrColorConverter{};
        }
    }

    template<class Dec>
    static PtrColorConverter choose_encoder(Dec dec, RngByBpp const & rng_by_bpp) {
        return make_converter(
            dec, rng_by_bpp,
            rng_by_bpp.its[0] != rng_by_bpp.its[1],
            rng_by_bpp.its[1] != rng_by_bpp.its[2],
            rng_by_bpp.its[2] != rng_by_bpp.its[3],
            rng_by_bpp.its[3] != rng_by_bpp.its[4]
        );
    }

    template<class CmdColorDistributor>
    struct GraphicConverted : gdi::GraphicBase<GraphicConverted<CmdColorDistributor>>
    {
        friend gdi::GraphicCoreAccess;

        CmdColorDistributor distributor;

        GraphicConverted(CmdColorDistributor distributor)
        : distributor(distributor)
        {}

        template<class... Ts>
        void draw_impl(Ts const & ... args) {
            this->distributor(args...);
        }
    };

    template<class Dec, bool e8, bool e15, bool e16, bool e24>
    static PtrColorConverter make_converter(Dec dec, RngByBpp const & rng_by_bpp) {
        using ColorConv = gdi::GraphicCmdColorDistributor<RngByBpp, Dec, e8, e15, e16, e24>;
        return PtrColorConverter{new GraphicConverted<ColorConv>{{rng_by_bpp, dec}}};
    }

    template<class Dec, bool... Bools, class... Bool>
    static PtrColorConverter make_converter(Dec dec, RngByBpp const & rng_by_bpp, bool x, Bool ... y) {
        if (x) {
            return make_converter<Dec, Bools..., 1>(dec, rng_by_bpp, y...);
        } else {
            return make_converter<Dec, Bools..., 0>(dec, rng_by_bpp, y...);
        }
    }
    //@}


    struct BasicGraphic final
    : gdi::GraphicDispatcher<BasicGraphic>
    {
        friend gdi::GraphicCoreAccess;

        PtrColorConverter cmd_color_distributor;
        MouseTrace const & mouse;
        std::vector<GdRef> gds;
        std::vector<std::reference_wrapper<gdi::CaptureApi>> snapshoters;

        BasicGraphic(gdi::GraphicDepth const & depth, MouseTrace const & mouse)
        : BasicGraphic::base_type(depth)
        , mouse(mouse)
        {}

        std::vector<GdRef> & get_gd_list_impl() {
            return this->gds;
        }

        template<class Cmd, class... Ts>
        void draw_impl(Cmd const & cmd, Ts const & ... args)
        {
            this->draw_impl2(1, cmd, args...);
        }

        template<class Cmd, class... Ts>
        auto draw_impl2(int, Cmd const & cmd, Ts const & ... args)
        // avoid some virtual call
        -> decltype(void(gdi::GraphicCmdColor::encode_cmd_color(decode_color15{}, std::declval<Cmd&>(cmd))))
        {
            assert(bool(this->cmd_color_distributor));
            this->cmd_color_distributor->draw(cmd, args...);
        }

        template<class Cmd, class... Ts>
        void draw_impl2(unsigned, Cmd const & cmd, Ts const & ... args)
        {
            BasicGraphic::base_type::draw_impl(cmd, args...);
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

    BasicGraphic graphic_api;
    RDPDrawable drawable;
    uint8_t order_bpp;

public:
    using GraphicApi = BasicGraphic;

    GraphicCaptureImpl(uint16_t width, uint16_t height, uint8_t order_bpp, MouseTrace const & mouse)
    : graphic_api(gdi::GraphicDepth::depth24(), mouse)
    , drawable(width, height, order_bpp)
    , order_bpp(order_bpp)
    {
    }

    void attach_apis(ApisRegister & apis_register, const Inifile &) {
        assert(apis_register.graphic_list);
        apis_register.graphic_list->push_back(this->drawable);
    }

    void start() {
        this->graphic_api.cmd_color_distributor = this->choose_color_converter(
            this->graphic_api.gds, this->order_bpp
        );
    }

    GraphicApi & get_graphic_api() { return this->graphic_api; }

    Drawable & impl() { return this->drawable.impl(); }
    RDPDrawable & rdp_drawable() { return this->drawable; }
};

#endif
