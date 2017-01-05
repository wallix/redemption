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

#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "utils/colors.hpp"
#include "utils/sugar/range.hpp"

#include "graphic_api.hpp"

#include <type_traits>


namespace gdi {

struct GraphicCmdColor
{
    struct color1_val
    {
        uint32_t color1;
    };

    struct color1_ref
    {
        uint32_t & color1;
        template<class Enc> void encode(Enc const & enc) { color1 = enc(color1); }
        void assign(color1_val other) { color1 = other.color1; }
        color1_val to_colors() const { return {color1}; }
    };

    struct color2_val
    {
        uint32_t color1;
        uint32_t color2;
    };

    struct color2_ref
    {
        uint32_t & color1;
        uint32_t & color2;
        template<class Enc> void encode(Enc const & enc) { color1 = enc(color1); color2 = enc(color2); }
        void assign(color2_val other) { color1 = other.color1; color2 = other.color2; }
        color2_val to_colors() const { return {color1, color2}; }
    };

    static color1_ref cmd_color(RDPMultiOpaqueRect & cmd) { return {cmd._Color}; }

    static color2_ref cmd_color(RDP::RDPMultiPatBlt & cmd) { return {cmd.BackColor, cmd.ForeColor}; }

    static color2_ref cmd_color(RDPPatBlt & cmd) { return {cmd.back_color, cmd.fore_color}; }

    static color2_ref cmd_color(RDPMem3Blt & cmd) { return {cmd.back_color, cmd.fore_color}; }

    static color1_ref cmd_color(RDPOpaqueRect & cmd) { return {cmd.color}; }

    static color2_ref cmd_color(RDPLineTo & cmd) { return {cmd.back_color, cmd.pen.color}; }

    static color2_ref cmd_color(RDPGlyphIndex & cmd) { return {cmd.back_color, cmd.fore_color}; }

    static color1_ref cmd_color(RDPPolygonSC & cmd) { return {cmd.BrushColor}; }

    static color2_ref cmd_color(RDPPolygonCB & cmd) { return {cmd.backColor, cmd.foreColor}; }

    static color1_ref cmd_color(RDPPolyline & cmd) { return {cmd.PenColor}; }

    static color1_ref cmd_color(RDPEllipseSC & cmd) { return {cmd.color}; }

    static color2_ref cmd_color(RDPEllipseCB & cmd) { return {cmd.back_color, cmd.fore_color}; }

private:
    template<class Cmd>
    struct abort_const_cmd
    {
        static_assert(!std::is_const<Cmd>::value, "cmd is const qualified");
        using type = Cmd;
    };

public:
    template<class Enc, class Cmd>
    static auto encode_cmd_color(Enc const & enc, Cmd & cmd)
    -> decltype(cmd_color(std::declval<typename abort_const_cmd<Cmd>::type&>()).encode(enc), void())
    { cmd_color(cmd).encode(enc); }

private:
    template<class Cmd>
    static auto is_encodable_(int, Cmd & cmd) -> decltype(cmd_color(cmd), std::true_type());

    template<class Cmd>
    static std::false_type is_encodable_(char, Cmd & cmd);

public:
    template<class Cmd>
    static auto is_encodable_cmd_color(Cmd const &)
    -> decltype(is_encodable_(1, std::declval<Cmd&>()))
    { return {}; }
};


template<class Iterator>
struct RngByBpp
{
    using iterator = Iterator;

    RngByBpp() = default;

    RngByBpp(gdi::GraphicDepth order_depth, iterator first, iterator last)
    : its{first, first, first, first, last}
    {
        assert(order_depth.is_defined());

        std::sort(first, last, [order_depth](gdi::GraphicApi const & a, gdi::GraphicApi const & b) {
            return a.order_depth().depth_or(order_depth).id() < b.order_depth().depth_or(order_depth).id();
        });

        struct ge {
            gdi::GraphicDepth order_depth;
            gdi::GraphicDepth bpp;
            bool operator()(gdi::GraphicApi const & x) const {
                return x.order_depth().depth_or(order_depth).id() >= this->bpp.id();
            }
        };

        this->its[0] = std::find_if(this->its[0], this->its[4], ge{order_depth, gdi::GraphicDepth::depth8()});
        this->its[1] = std::find_if(this->its[0], this->its[4], ge{order_depth, gdi::GraphicDepth::depth15()});
        this->its[2] = std::find_if(this->its[1], this->its[4], ge{order_depth, gdi::GraphicDepth::depth16()});
        this->its[3] = std::find_if(this->its[2], this->its[4], ge{order_depth, gdi::GraphicDepth::depth24()});
    }

    range<iterator> rng8() const { return {this->its[0], this->its[1]}; }
    range<iterator> rng15() const { return {this->its[1], this->its[2]}; }
    range<iterator> rng16() const { return {this->its[2], this->its[3]}; }
    range<iterator> rng24() const { return {this->its[3], this->its[4]}; }
    range<iterator> rng(std::size_t i) const { return {this->its[i], this->its[i+1]}; }
    range<iterator> rng_all() const { return {this->its[0], this->its[4]}; }

    constexpr static std::size_t count_range() { return 4; }

private:
    iterator its[5];
};

struct graphic_draw_fn
{
    template<class Cmd, class... Ts>
    void operator()(GraphicApi & api, Cmd const & cmd, Ts const & ... args) const
    { api.draw(cmd, args...); }

    template<class Cmd, class... Ts>
    void operator()(GraphicApi * api, Cmd const & cmd, Ts const & ... args) const
    { api->draw(cmd, args...); }
};

namespace detail
{
    template<class Fn, class Rng, class Cmd, class... Ts>
    void draw_rng(Fn & apply, Rng && rng, Cmd const & cmd, Ts const & ... args) {
        for (auto && gd : rng) {
            apply(gd, cmd, args...);
        }
    }

    template<class Fn, class Iterator, class Cmd, class... Ts>
    void draw_cmd_color_convert(
        std::false_type, Fn & apply, gdi::GraphicDepth, RngByBpp<Iterator> const & rng_by_bpp,
        Cmd const & cmd, Ts const & ... args
    ) { draw_rng(apply, rng_by_bpp.rng_all(), cmd, args...); }

    template<class Fn, class Iterator, class Cmd, class... Ts>
    void draw_cmd_color_convert(
        std::true_type, Fn & apply, gdi::GraphicDepth order_depth, RngByBpp<Iterator> const & rng_by_bpp,
        Cmd const & cmd, Ts const & ... args
    ) {
        for (std::size_t i = rng_by_bpp.count_range(); i > 0; --i) {
            if (order_depth.id() == i) {
                draw_rng(apply, rng_by_bpp.rng(i-1), cmd, args...);
                if (rng_by_bpp.rng_all().size() == rng_by_bpp.rng(i-1).size()) {
                    return;
                }
                break;
            }
        }

        auto new_cmd = cmd;
        auto new_cmd_colors_ref = GraphicCmdColor::cmd_color(new_cmd);

        using dec8 = with_color8_palette<decode_color8_opaquerect>;
        switch (order_depth) {
            case GraphicDepth::depth24() : /*new_cmd_colors_ref.encode(decode_color24_opaquerect{});*/ break;
            case GraphicDepth::depth16() : new_cmd_colors_ref.encode(decode_color16_opaquerect{}); break;
            case GraphicDepth::depth15() : new_cmd_colors_ref.encode(decode_color15_opaquerect{}); break;
            case GraphicDepth::depth8() : new_cmd_colors_ref.encode(dec8{BGRPalette::classic_332_rgb()}); break;
            case GraphicDepth::unspecified() : assert(false && "unknown value in order_bpp"); break;
        }

        if (GraphicDepth::depth24() != order_depth) {
            draw_rng(apply, rng_by_bpp.rng24(), new_cmd, args...);
        }

        auto decoded_colors = new_cmd_colors_ref.to_colors();

        if (GraphicDepth::depth16() != order_depth) {
            new_cmd_colors_ref.encode(encode_color16{});
            draw_rng(apply, rng_by_bpp.rng16(), new_cmd, args...);
        }
        if (GraphicDepth::depth15() != order_depth) {
            new_cmd_colors_ref.assign(decoded_colors);
            new_cmd_colors_ref.encode(encode_color15{});
            draw_rng(apply, rng_by_bpp.rng15(), new_cmd, args...);
        }
        if (GraphicDepth::depth8() != order_depth) {
            new_cmd_colors_ref.assign(decoded_colors);
            new_cmd_colors_ref.encode(encode_color8{});
            draw_rng(apply, rng_by_bpp.rng8(), new_cmd, args...);
        }
    }
}


template<class Fn, class Iterator, class Cmd, class... Ts>
void draw_cmd_color_convert(
    Fn apply, gdi::GraphicDepth order_depth, RngByBpp<Iterator> rng,
    Cmd const & cmd, Ts const & ... args
) {
    detail::draw_cmd_color_convert(
        GraphicCmdColor::is_encodable_cmd_color(cmd),
        apply, order_depth, rng, cmd, args...
    );
}


template<class Iterator, class Cmd, class... Ts>
void draw_cmd_color_convert(
    GraphicDepth order_depth, RngByBpp<Iterator> rng,
    Cmd const & cmd, Ts const & ... args
) { return draw_cmd_color_convert(graphic_draw_fn{}, order_depth, rng, cmd, args...); }

}

