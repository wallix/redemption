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

#include <type_traits>


namespace gdi {

struct GraphicCmdColor
{
private:
    struct color1_val
    {
        RDPColor color1;
    };

    struct color1_ref
    {
        RDPColor & color1;
        template<class ReEnc> void encode(ReEnc const & enc) { color1 = enc(color1); }
        void assign(color1_val other) { color1 = other.color1; }
        color1_val to_colors() const { return {color1}; }
    };

    struct color2_val
    {
        RDPColor color1;
        RDPColor color2;
    };

    struct color2_ref
    {
        RDPColor & color1;
        RDPColor & color2;
        template<class ReEnc> void encode(ReEnc const & enc) { color1 = enc(color1); color2 = enc(color2); }
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

public:
    template<class ReEnc, class Cmd>
    static void encode_cmd_color(ReEnc const & enc, Cmd const & cmd) = delete;

    template<class ReEnc, class Cmd>
    static void encode_cmd_color(ReEnc const & enc, Cmd & cmd)
    { cmd_color(cmd).encode(enc); }

private:
    template<class Cmd>
    static auto is_encodable_(int, Cmd & cmd) -> decltype(cmd_color(cmd), std::true_type());

    template<class Cmd>
    static std::false_type is_encodable_(char, Cmd & cmd);

public:
    template<class Cmd>
    static auto is_encodable_cmd_color(Cmd const & /*unused*/)
    -> decltype(is_encodable_(1, std::declval<Cmd&>()))
    { return {}; }
};

}  // namespace gdi
