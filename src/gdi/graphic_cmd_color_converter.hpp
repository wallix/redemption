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

#ifndef REDEMPTION_GDI_GRAPHIC_CMD_COLOR_CONVERTOR_HPP
#define REDEMPTION_GDI_GRAPHIC_CMD_COLOR_CONVERTOR_HPP

#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "utils/colors.hpp"

#include "graphic_api.hpp"

#include <type_traits>


namespace gdi {

struct GraphicCmdColor
{
    static uint32_t & cmd_color(RDPMultiOpaqueRect & cmd) { return cmd._Color; }

    static uint32_t & cmd_color1(RDP::RDPMultiPatBlt & cmd) { return cmd.BackColor; }
    static uint32_t & cmd_color2(RDP::RDPMultiPatBlt & cmd) { return cmd.ForeColor; }

    static uint32_t & cmd_color1(RDPPatBlt & cmd) { return cmd.back_color; }
    static uint32_t & cmd_color2(RDPPatBlt & cmd) { return cmd.fore_color; }

    static uint32_t & cmd_color1(RDPMem3Blt & cmd) { return cmd.back_color; }
    static uint32_t & cmd_color2(RDPMem3Blt & cmd) { return cmd.fore_color; }

    static uint32_t & cmd_color(RDPOpaqueRect & cmd) { return cmd.color; }

    static uint32_t & cmd_color1(RDPLineTo & cmd) { return cmd.back_color; }
    static uint32_t & cmd_color2(RDPLineTo & cmd) { return cmd.pen.color; }

    static uint32_t & cmd_color1(RDPGlyphIndex & cmd) { return cmd.back_color; }
    static uint32_t & cmd_color2(RDPGlyphIndex & cmd) { return cmd.fore_color; }

    static uint32_t & cmd_color(RDPPolygonSC & cmd) { return cmd.BrushColor; }

    static uint32_t & cmd_color1(RDPPolygonCB & cmd) { return cmd.backColor; }
    static uint32_t & cmd_color2(RDPPolygonCB & cmd) { return cmd.foreColor; }

    static uint32_t & cmd_color(RDPPolyline & cmd) { return cmd.PenColor; }

    static uint32_t & cmd_color(RDPEllipseSC & cmd) { return cmd.color; }

    static uint32_t & cmd_color1(RDPEllipseCB & cmd) { return cmd.back_color; }
    static uint32_t & cmd_color2(RDPEllipseCB & cmd) { return cmd.fore_color; }

private:
    template<class Enc, class Cmd>
    static auto encode_cmd_color_(Enc const & enc, Cmd & cmd) -> decltype(cmd_color(cmd), void()) {
        cmd_color(cmd) = enc(cmd_color(cmd));
    }

    template<class Enc, class Cmd>
    static auto encode_cmd_color_(Enc const & enc, Cmd & cmd) -> decltype(cmd_color2(cmd), void()) {
        cmd_color1(cmd) = enc(cmd_color1(cmd));
        cmd_color2(cmd) = enc(cmd_color2(cmd));
    }

    template<class Cmd>
    struct abort_const_cmd {
        static_assert(!std::is_const<Cmd>::value, "cmd is const qualified");
        using type = Cmd;
    };

public:
    template<class Enc, class Cmd>
    static auto encode_cmd_color(Enc const & enc, Cmd & cmd)
    -> decltype(encode_cmd_color_(enc, std::declval<typename abort_const_cmd<Cmd>::type&>())) {
        encode_cmd_color_(enc, cmd);
    }
};


// struct RngByBpp {
//   Range rng8();
//   Range rng15();
//   Range rng16();
//   Range rng24();
//   Range rng_all(); [optional]
//   void apply(T, Cmd, Args);
// };
template<class RngByBpp, class Dec, bool Enc8, bool Enc15, bool Enc16, bool Enc24>
struct GraphicCmdColorDistributor : private GraphicCmdColor, private Dec
{
    GraphicCmdColorDistributor(RngByBpp const & rng_by_bpp, Dec const & dec)
    : Dec(dec)
    , rng_by_bpp(rng_by_bpp)
    {}

    template<class Cmd, class... Ts>
    void operator()(Cmd const & cmd, Ts const & ... args) const {
        this->encode_cmd(1, cmd, args...);
    }

private:
    template<bool b> using bool_ = std::integral_constant<bool, b>;
    using true_ = std::true_type;
    using false_ = std::false_type;

    template<class Range, class... Ts>
    void dispatch_if(true_, Range && rng, Ts const & ... args) const {
        for (auto && gd : rng) {
            this->rng_by_bpp.apply(gd, args...);
        }
    }

    template<class Range, class... Ts>
    void dispatch_if(false_, Range const &, Ts const & ...) const {
    }


    template<class Mut, class Enc, class Range, class Cmd, class... Ts>
    auto encode_if(int, Mut is_mut, true_, Enc enc, Range && rng, Cmd & cmd, Ts const & ... args) const
    -> decltype(cmd_color(cmd), void()) {
        if (!is_mut) {
            this->cmd_color(cmd) = enc(this->cmd_color(cmd));
            this->dispatch_if(true_{}, rng, cmd, args...);
        }
        else {
            auto c1 = this->cmd_color(cmd); this->cmd_color(cmd) = enc(c1);
            this->dispatch_if(true_{}, rng, cmd, args...);
            this->cmd_color(cmd) = c1;
        }
    }

    template<class Mut, class Enc, class Range, class Cmd, class... Ts>
    auto encode_if(int, Mut is_mut, true_, Enc enc, Range && rng, Cmd & cmd, Ts const & ... args) const
    -> decltype(cmd_color2(cmd), void()) {
        if (!is_mut) {
            this->cmd_color1(cmd) = enc(this->cmd_color1(cmd));
            this->cmd_color2(cmd) = enc(this->cmd_color2(cmd));
            this->dispatch_if(true_{}, rng, cmd, args...);
        }
        else {
            auto c1 = this->cmd_color1(cmd); this->cmd_color1(cmd) = enc(c1);
            auto c2 = this->cmd_color2(cmd); this->cmd_color2(cmd) = enc(c2);
            this->dispatch_if(true_{}, rng, cmd, args...);
            this->cmd_color1(cmd) = c1;
            this->cmd_color2(cmd) = c2;
        }
    }

    template<class Mut, class Bool, class Enc, class Range, class Cmd, class... Ts>
    void encode_if(unsigned, Mut, Bool, Enc enc, Range && rng, Cmd & cmd, Ts const & ... args) const {
        this->dispatch_if(Bool{}, rng, cmd, args...);
    }

    Dec const & decoder() const { return static_cast<Dec const&>(*this); }

    template<class Cmd, class... Ts>
    auto encode_cmd(int, Cmd const & cmd, Ts const & ... args) const
    -> decltype(this->encode_cmd_color(this->decoder(), std::declval<Cmd&>())) {
        this->dispatch_if(bool_<Enc8  && Dec::bpp == 8 >{}, this->rng_by_bpp.rng8 (), cmd, args...);
        this->dispatch_if(bool_<Enc15 && Dec::bpp == 15>{}, this->rng_by_bpp.rng15(), cmd, args...);
        this->dispatch_if(bool_<Enc16 && Dec::bpp == 16>{}, this->rng_by_bpp.rng16(), cmd, args...);
        this->dispatch_if(bool_<Enc24 && Dec::bpp == 24>{}, this->rng_by_bpp.rng24(), cmd, args...);

        auto new_cmd = cmd;
        this->encode_cmd_color(this->decoder(), new_cmd);

        this->dispatch_if(bool_<Enc24 && Dec::bpp != 24>{}, this->rng_by_bpp.rng24(), new_cmd, args...);
        this->encode_if(1, bool_<bool(Enc15+Enc16)>{}, bool_<Enc8 && Dec::bpp != 8>{},
            encode_color8 {}, this->rng_by_bpp.rng8 (), new_cmd, args...);
        this->encode_if(1, bool_<bool(Enc16)>{}, bool_<Enc15 && Dec::bpp != 15>{},
            encode_color15{}, this->rng_by_bpp.rng15(), new_cmd, args...);
        this->encode_if(1, false_{}, bool_<Enc16 && Dec::bpp != 16>{},
            encode_color16{}, this->rng_by_bpp.rng16(), new_cmd, args...);
    }

    template<class Cmd, class... Ts>
    void encode_cmd(unsigned, Cmd const & cmd, Ts const & ... args) const {
        this->dispatch_all(1, cmd, args...);
    }

    template<class... Ts>
    auto dispatch_all(int, Ts const & ... args) const
    -> decltype(std::declval<RngByBpp const &>().rng_all(), void()) {
        this->dispatch_if(true_{}, this->rng_by_bpp.rng_all(), args...);
    }

    template<class... Ts>
    void dispatch_all(unsigned, Ts const & ... args) const {
        this->dispatch_if(bool_<Enc8 >{}, this->rng_by_bpp.rng8 (), args...);
        this->dispatch_if(bool_<Enc15>{}, this->rng_by_bpp.rng15(), args...);
        this->dispatch_if(bool_<Enc16>{}, this->rng_by_bpp.rng16(), args...);
        this->dispatch_if(bool_<Enc24>{}, this->rng_by_bpp.rng24(), args...);
    }

    RngByBpp rng_by_bpp;
};

template<class RngByBpp, class Dec>
struct GraphicCmdColorDistributor<RngByBpp, Dec, false, false, false, false>
{
    GraphicCmdColorDistributor(RngByBpp const &, Dec const &)
    {}

    template<class Cmd, class... Ts>
    void operator()(Cmd const &, Ts const & ...) const {
    }
};


template<class ColorConverter, class ProxyBase = gdi::GraphicProxy>
struct CmdColorConverterProxy : private GraphicCmdColor, ProxyBase
{
    template<class... ColorConverterArgs>
    CmdColorConverterProxy(ProxyBase && base, ColorConverterArgs &&...args)
    : ProxyBase(std::move(base))
    , cmd_color_converter(std::forward<ColorConverterArgs>(args)...)
    {}

    template<class... ColorConverterArgs>
    CmdColorConverterProxy(ProxyBase const & base, ColorConverterArgs &&...args)
    : ProxyBase(base)
    , cmd_color_converter(std::forward<ColorConverterArgs>(args)...)
    {}

    template<class... ColorConverterArgs>
    CmdColorConverterProxy(ColorConverterArgs &&...args)
    : cmd_color_converter(std::forward<ColorConverterArgs>(args)...)
    {}

    template<class... ColorConverterArgs>
    CmdColorConverterProxy(ProxyBase const & base)
    : ProxyBase(base)
    {}

    using draw_tag = gdi::GraphicProxy::draw_tag;
    template<class Gd, class... Ts>
    void operator()(draw_tag, Gd & gd, Ts const & ... args) {
        this->encode_cmd(1, gd, args...);
    }

    template<class Gd, class Tag, class... Ts>
    void operator()(Tag tag, Gd & gd, Ts const & ... args) {
        static_cast<ProxyBase&>(*this)(tag, gd, args...);
    }

    const ColorConverter & get_converter() const { return this->cmd_color_converter; }

private:
    ColorConverter cmd_color_converter;

    template<class Gd, class Cmd, class... Ts>
    auto encode_cmd(int, Gd & gd, Cmd const & cmd, Ts const & ... args)
    -> decltype(this->encode_cmd_color(this->cmd_color_converter, std::declval<Cmd&>())) {
        auto new_cmd = cmd;
        this->encode_cmd_color(this->cmd_color_converter, new_cmd);
        static_cast<ProxyBase&>(*this)(draw_tag{}, gd, new_cmd, args...);
    }

    template<class Gd, class Cmd, class... Ts>
    void encode_cmd(unsigned, Gd & gd, Cmd const & cmd, Ts const & ... args) {
        static_cast<ProxyBase&>(*this)(draw_tag{}, gd, cmd, args...);
    }
};

}

#endif
