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
#include "gdi/proxy.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/railgraphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "utils/range.hpp"

class GraphicCaptureImpl
{
public:
    // TODO
    using GdRef = ::GdRef;

private:
    using PtrColorConverter = std::unique_ptr<gdi::GraphicApi>;

    struct GraphicProxy
    {
        PtrColorConverter cmd_color_distributor;
        MouseTrace const & mouse;
        std::vector<GdRef> gds;
        std::vector<std::reference_wrapper<gdi::CaptureApi>> snapshoters;

        GraphicProxy(MouseTrace const & mouse) : mouse(mouse) {}

        using draw_tag = gdi::GraphicProxy::draw_tag;

        template<class Cmd, class... Ts>
        auto operator()(draw_tag, gdi::GraphicApi &, Cmd const & cmd, Ts const & ... args)
        // avoid some virtual call
        -> decltype(gdi::GraphicCmdColor::encode_cmd_color(decode_color15{}, cmd))
        {
            assert(bool(this->cmd_color_distributor));
            this->cmd_color_distributor->draw(args...);
        }

        void operator()(draw_tag tag, gdi::GraphicApi &, RDP::FrameMarker const & cmd) {
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

        template<class Tag, class... Ts>
        void operator()(Tag tag, gdi::GraphicApi & ngd, Ts const & ... args) {
            for (gdi::GraphicApi & gd : this->gds) {
                gdi::GraphicProxy()(tag, gd, args...);
            }
        }
    };


    // GraphicProxy::cmd_color_distributor
    //@{
    template<class CmdColorDistributor>
    struct CmdColorDistributorProxy
    {
        CmdColorDistributor distributor;

        using draw_tag = gdi::GraphicProxy::draw_tag;
        template<class... Ts>
        void operator()(draw_tag, gdi::GraphicApi &, Ts const & ... args) {
            this->distributor(args...);
        }

        template<class Tag, class... Ts>
        void operator()(Tag, gdi::GraphicApi &, Ts const & ...) {
            assert(false);
        }
    };

    struct RngByBpp
    {
        using iterator = std::vector<GdRef>::const_iterator;

        iterator its[5];
        range<iterator> rng8() const { return {its[0], its[1]}; }
        range<iterator> rng15() const { return {its[1], its[2]}; }
        range<iterator> rng16() const { return {its[2], its[3]}; }
        range<iterator> rng24() const { return {its[3], its[4]}; }
        range<iterator> rng_all() const { return {its[0], its[4]}; }
    };

    static PtrColorConverter choose_color_converter(std::vector<GdRef> gds, uint8_t order_bpp) {
        std::sort(gds.begin(), gds.end(), [](GdRef const & a, GdRef const & b) {
            return a.bpp < b.bpp;
        });

        RngByBpp rng_by_bpp{{gds.begin(), gds.begin(), gds.begin(), gds.begin(), gds.end()}};

        struct ge {
            uint8_t bpp;
            bool operator()(GdRef const & x) const {
                return x.bpp >= this->bpp;
            }
        };

        auto & its = rng_by_bpp.its;
        its[0] = std::find_if(its[0], its[4], ge{8});
        its[1] = std::find_if(its[0], its[4], ge{15});
        its[2] = std::find_if(its[1], its[4], ge{16});
        its[3] = std::find_if(its[2], its[4], ge{24});

        using dec8 = to_color8_palette<decode_color8_opaquerect>;
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

    template<class Dec, bool e8, bool e15, bool e16, bool e24>
    static PtrColorConverter make_converter(Dec dec, RngByBpp const & rng_by_bpp) {
        using ColorConv = gdi::GraphicCmdColorDistributor<RngByBpp, Dec, e8, e15, e16, e24>;
        using Proxy = CmdColorDistributorProxy<ColorConv>;
        using Gd = gdi::GraphicAdapter<Proxy>;
        return PtrColorConverter{new Gd(Proxy{{rng_by_bpp, dec}})};
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


    struct BasicRAILGraphic final
    : gdi::RAILGraphicAdapter<
        gdi::DispatcherProxy<
            gdi::RAILGraphicApi,
            gdi::RAILGraphicProxy
        >
    > {};

    struct BasicGraphic final
    : gdi::GraphicAdapter<GraphicProxy> {
        using gdi::GraphicAdapter<GraphicProxy>::GraphicAdapter;
    };

    BasicGraphic graphic_api;
    RDPDrawable drawable;
    BasicRAILGraphic rail_graphic_api;

public:
    using RAILGraphicApi = BasicRAILGraphic;
    using GraphicApi = BasicGraphic;

    GraphicCaptureImpl(uint16_t width, uint16_t height, uint8_t order_bpp, MouseTrace const & mouse)
    : graphic_api(mouse)
    , drawable(width, height, order_bpp)
    {
    }

    void attach_apis(ApisRegister & apis_register, const Inifile &) {
        assert(apis_register.graphic_list);
        apis_register.graphic_list->push_back({
            this->drawable, this->drawable.impl().bpp()
        });
        assert(apis_register.rail_graphic_list);
        apis_register.rail_graphic_list->push_back(this->drawable);
    }

    void start(uint8_t order_bpp) {
        this->graphic_api.get_proxy().cmd_color_distributor = this->choose_color_converter(
            this->graphic_api.get_proxy().gds, order_bpp
        );
    }

    GraphicApi & get_graphic_api() { return this->graphic_api; }
    RAILGraphicApi & get_rail_graphic_api() { return this->rail_graphic_api; }

    Drawable & impl() { return this->drawable.impl(); }
    RDPDrawable & rdp_drawable() { return this->drawable; }
};

#endif
