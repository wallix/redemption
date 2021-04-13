/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/js_auto_test_case.hpp"

#include "redjs/graphics.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "utils/bitmap.hpp"

#include <bitset>


RED_JS_AUTO_TEST_CASE(
    TestGraphics,
    (emscripten::val drawable),
    (() => {
        const Drawable = require("src/application/rdp_graphics").newRdpGraphics2D;
        const { ImageData, createCanvas, Canvas } = require("node_modules/canvas");

        global.OffsreenCanvas = Canvas;
        global.ImageData = ImageData;

        const canvas = createCanvas(0, 0);
        canvas.style = {}; /* for cursor style */

        const gd = new Drawable(canvas, Module);
        gd.ecanvas = canvas;
        return gd;
    })()
) {
    auto canvas = drawable["ecanvas"];
    RED_REQUIRE(!!canvas);

    auto to_data_url = [](emscripten::val& canvas){
        return canvas.call<std::string>("toDataURL");
    };

    Rect screen{0, 0, 400, 300};
    gdi::ColorCtx color_ctx(gdi::Depth::depth24(), nullptr);

    redjs::Graphics gd(drawable, 0, 0);
    gd.resize_canvas(ScreenInfo{screen.cx, screen.cy, BitsPerPixel(24)});

    using bitset = std::bitset<PrimaryDrawingOrdersSupport::max>;
    RED_CHECK(bitset{(
        PrimaryDrawingOrdersSupport{}
        | TS_NEG_DSTBLT_INDEX
        | TS_NEG_PATBLT_INDEX
        | TS_NEG_SCRBLT_INDEX
        | TS_NEG_MEMBLT_INDEX
        | TS_NEG_MEM3BLT_INDEX
        | TS_NEG_LINETO_INDEX
        | TS_NEG_MULTIDSTBLT_INDEX
        | TS_NEG_MULTIPATBLT_INDEX
        | TS_NEG_MULTISCRBLT_INDEX
        | TS_NEG_MULTIOPAQUERECT_INDEX
        // | TS_NEG_POLYGON_SC_INDEX
        // | TS_NEG_POLYGON_CB_INDEX
        | TS_NEG_POLYLINE_INDEX
        | TS_NEG_GLYPH_INDEX
    ).as_uint()} == bitset{gd.get_supported_orders().as_uint()});


    // RDPOpaqueRect
    {
        gd.draw(
            RDPOpaqueRect(Rect(50, 50, 50, 50), RDPColor::from(0xFF0000)),
            screen, color_ctx);
        gd.draw(
            RDPOpaqueRect(Rect(-25, -25, 50, 50), RDPColor::from(0x00FF00)),
            screen, color_ctx);
        gd.draw(
            RDPOpaqueRect(Rect(375, 275, 50, 50), RDPColor::from(0x0000FF)),
            screen, color_ctx);
        gd.draw(
            RDPOpaqueRect(Rect(250, 150, 100, 100), RDPColor::from(0x00FFFF)),
            Rect(275, 175, 50, 50), color_ctx);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAZAAAAEsCAIAAABi1XKVAAAABmJLR0QA/wD/AP+gvaeTA"
        "AAC4ElEQVR4nO3dMQ7CMBQFwTj3v7OpqAhSioDZMHOArydL2TZjm9tlxnWnAF7sqwcAnC"
        "VYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpA"
        "hWECGYAEZggVkCBaQIVhAhmABGYIFAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADw"
        "NFYPeGeuHnDoZ58L/oJf1QMZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhW"
        "ECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIV"
        "hAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCF"
        "YQIZgARmCBWQIFpAhWECGYAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "ADcx1g9AJLmXL3gyLj7B72vHgBwlmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZgg"
        "VkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYI"
        "FZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmC"
        "BWQIFpAhWECGYAEZggVkCBaQIVgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAF8zVg8Abm5ed2q/7hTAZwkWkCFYQIZgAR"
        "mCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAE"
        "ZggVkCBaQIVhAhmABGQ953QYolGtokwAAAABJRU5ErkJggg=="_av);


    // RDPMultiOpaqueRect
    {
        StaticOutStream<256> out_stream;
        for (int i = 0; i < 5; ++i) {
            out_stream.out_sint16_le(25);
            out_stream.out_sint16_le(25);
            out_stream.out_sint16_le(50);
            out_stream.out_sint16_le(50);
        }

        InStream in_stream(out_stream.get_produced_bytes());

        gd.draw(
            RDPMultiOpaqueRect(0, 0, 300, 400, RDPColor::from(0xFFFF00),
            5, in_stream),
            screen, color_ctx);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAZAAAAEsCAIAAABi1XKVAAAABmJLR0QA/wD/AP+gvaeTA"
        "AADw0lEQVR4nO3dwQ2DMBQFQUz/PZMCuCSRE7MwU4Dl0+rf3tiObZox7ymAk331BwDeJV"
        "hAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCF"
        "YQIZgARmCBWQIFpAhWECGYAEZggVkTN0+PSaOss4z7LvCTbiwgAzBAjIEC8gQLCBDsIAM"
        "wQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CAD"
        "MECMgQLyBAsIOMBQ6oT2WSFpVxYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGY"
        "IFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkHHVZVCbrMC"
        "JCwvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMEC"
        "MgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyHjDnaZMV7sKFBWQIFpAhWECGYAEZggVkC"
        "BaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZA"
        "gWkCFYQIZgARk2OD9hkxWWcmEBGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBa"
        "QIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhuHMRa65yWpI"
        "lWtzYQEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQI"
        "VhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECG4Uz4hiXcJVxYQIZgARmCBWQIFpAhWE"
        "CGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVh"
        "AhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFY"
        "QIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB/M1Z/AL"
        "i5Y95T+7ynAH5LsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIA"
        "MwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8h4AatkEetRevi8AAAAAElFTkSu"
        "QmCC"_av);


    // RDPScrBlt
    {
        gd.draw(RDPScrBlt(Rect(100, 75, 200, 150), 0x55, 100, 75), screen);
        gd.draw(RDPScrBlt(screen, 0xCC, screen.cx/2, screen.cy/2), screen);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAZAAAAEsCAIAAABi1XKVAAAABmJLR0QA/wD/AP+gvaeTA"
        "AAD1klEQVR4nO3dwW3EMAwAwehw/bfMdBA4gRxlnZkCBPrhBX9cM/PBNWut0yPAv/Y6PQ"
        "DAVYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZ"
        "gARmCBWQIFpAhWECGYAEZggVkCBaQIVhAxvv0ALfbePx0481ZJ1nhB2xYQIZgARmCBWQI"
        "FpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkC"
        "BaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZA"
        "gWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQMY6PQDPNDO"
        "nR8hYy294lQ0LyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAs"
        "IEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQL"
        "CBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIEC"
        "wgQ7CADMECMgQLyHifHoA/ZE4PAF+zYQEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARm"
        "CBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGQ6rf"
        "MLPv0uha257aaOMHwg1sWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIF"
        "pAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpDhkOohTpbGbbyE+/"
        "j7vBvZsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAv"
        "IECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQL"
        "yBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC"
        "8gQLCBDsAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAfs06PQDwcLPvqde+pwDuJVhAhmABGYIFZAgWkCFYQIZgARmCBWQ"
        "IFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVk"
        "fALbSRT3ukjuNQAAAABJRU5ErkJggg=="_av);


    // RDPPatBlt
    {
        gd.draw(RDPPatBlt(screen, 0x55, RDPColor(), RDPColor(), RDPBrush()), screen, color_ctx);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAZAAAAEsCAIAAABi1XKVAAAABmJLR0QA/wD/AP+gvaeTA"
        "AAEP0lEQVR4nO3dwW3EMAwAwTBw/y3rOgicgw7KOjMFCPTDC/44X9y21jo9Avxr36cHAL"
        "hLsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECw"
        "gQ7CADMECMgQLyBAsIEOwgAzBAjIEC8i4Tg/wcRuPn85se8pJVniDDQvIECwgQ7CADMEC"
        "MgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBA"
        "jIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQ"
        "IyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvImLXW6Rl4oJk"
        "5PUKGf/A+GxaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhA"
        "hmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQ"
        "IZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWE"
        "CGYAEZggVkCBaQcZ0egD9kTg8AP7NhARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYI"
        "FZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZDqr8w"
        "s+/S6Frbntpo4wfCB9iwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsI"
        "EOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIMMh1UOcLI3beAn38f"
        "d5N7JhARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpA"
        "hWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQ"
        "IVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWk"
        "CFYQIZgARmz1jo9A8AtNiwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBA"
        "vIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQ"
        "LyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIE"
        "C8gQLCBDsIAMwQIyBAvIECwg4zo9APBws+8pGxaQIVhAhmABGYIFZAgWkCFYQIZgARmCB"
        "WQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZgg"
        "VkvAC74BharyJ87gAAAABJRU5ErkJggg=="_av);


    // RDPDstBlt
    {
        // restore previous image
        gd.draw(RDPDstBlt(screen, 0x55), screen);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAZAAAAEsCAIAAABi1XKVAAAABmJLR0QA/wD/AP+gvaeTA"
        "AAD1klEQVR4nO3dwW3EMAwAwehw/bfMdBA4gRxlnZkCBPrhBX9cM/PBNWut0yPAv/Y6PQ"
        "DAVYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZ"
        "gARmCBWQIFpAhWECGYAEZggVkCBaQIVhAxvv0ALfbePx0481ZJ1nhB2xYQIZgARmCBWQI"
        "FpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkC"
        "BaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZA"
        "gWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQMY6PQDPNDO"
        "nR8hYy294lQ0LyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAs"
        "IEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQL"
        "CBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIEC"
        "wgQ7CADMECMgQLyHifHoA/ZE4PAF+zYQEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARm"
        "CBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGQ6rf"
        "MLPv0uha257aaOMHwg1sWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIF"
        "pAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpDhkOohTpbGbbyE+/"
        "j7vBvZsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAv"
        "IECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQL"
        "yBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC"
        "8gQLCBDsAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAfs06PQDwcLPvqde+pwDuJVhAhmABGYIFZAgWkCFYQIZgARmCBWQ"
        "IFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVk"
        "fALbSRT3ukjuNQAAAABJRU5ErkJggg=="_av);


    BGRPalette palette332(BGRPalette::classic_332());

    uint8_t raw24[] = {
        0x22, 0x17, 0x48, 0xc7, 0xcd, 0xc4, 0xad, 0xf8, 0x61, 0x6f, 0x32, 0xd6, 0x13, 0x61, 0xee,
        0xb2, 0x7b, 0x81, 0x0f, 0x66, 0x22, 0x17, 0x48, 0xc7, 0xcd, 0xc4, 0xad, 0xf8, 0x61, 0x6f,
        0x32, 0xd6, 0x13, 0x61, 0xee, 0xb2, 0x7b, 0x81, 0x0f, 0x66, 0x22, 0x17, 0x48, 0xc7, 0xcd,
        0xc4, 0xad, 0xf8, 0x61, 0x6f, 0x32, 0xd6, 0x13, 0x61, 0xee, 0xb2, 0x7b, 0x81, 0x0f, 0x66,
    };

    Bitmap bmp24(BitsPerPixel{24}, BitsPerPixel{24}, &palette332, 4, 5, raw24, sizeof(raw24));


    // RDPBitmapData
    {
        RDPBitmapData d;
        d.dest_right = bmp24.cx();
        d.dest_bottom = bmp24.cy();
        gd.draw(d, bmp24);

        d.dest_left = d.dest_right;
        d.dest_top = d.dest_bottom;
        d.dest_right *= 2;
        d.dest_bottom *= 2;
        gd.draw(d, bmp24);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAZAAAAEsCAIAAABi1XKVAAAABmJLR0QA/wD/AP+gvaeTA"
        "AAEd0lEQVR4nO3dMWoUYRiAYWfZfiEQkFwhhaCdjQQhB7CSgIVeQBu74BlEK4vExjNskd"
        "J7BESw0cYbjJ0khRuVibNvfJ5uYBi+Lfbln7+Yf7j7/OmdDy/33t/+9uTtu++vbvFrwzD"
        "MPQL815bnO3tfVq/Xu5/3T4/nHgZgk8Xpw0ePH7w5XH09OziZexiATRYHJ4c7e2er1+e7"
        "R+u5hwHYZHm0Xh2vdz/tn67uf5x7GIBNhnEcf17ce/HMBvwGNt1hXsuLFzbggW22uHhhA"
        "x7YZpeCZQMe2GaXXgltwAPb7NKmO5vZdId5La6+BWA7CBaQIVhAhmABGYIFZAgWkCFYQI"
        "ZgARmCBWQIFpAhWECGYAEZggVkCBaQsbz6lrgJPwkz4Zd4fKgG/oIVFpAhWECGYAEZggV"
        "kCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIF"
        "ZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCB"
        "WQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAxzD0AN9M4jn"
        "OPkDEM/oa/ywoLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBA"
        "sIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQ"
        "LCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyBAvIE"
        "CwgQ7CADMECMgQLyFjOPQBbZJx7ANjMCgvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8"
        "gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIEC8gQLCBDsIAMwQIyHKT"
        "6B8ZxupNGh2GyR01owh8I18AKC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOw"
        "gAzBAjIEC8gQLCBDsIAMwQIyBAvIECwgQ7CADMECMgQLyBAsIEOwgAzBAjIcpDoTR5bGT"
        "XgS7o0/n3dCVlhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhm"
        "ABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZ"
        "gARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQIFpAhWECG"
        "YAEZggVkCBaQIVgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAP/MMPcAwA03TveoxXSPArheggVkCBaQIVhAhmABGYIFZA"
        "gWkCFYQIZgARmCBWQIFpAhWECGYAEZggVkCBaQIVhAhmABGYIFZAgWkCFYQIZgARmCBWQ"
        "IFpAhWEDGD/u3TusgM5rAAAAAAElFTkSuQmCC"_av);

    // RDPBmpCache / RDPMemBlt
    {
        gd.draw(RDPBmpCache(bmp24, 1, 7, false, false));
        gd.draw(RDPMemBlt(1, Rect(0, 10, bmp24.cx(), bmp24.cy()), 0x22, 0, 0, 7), screen);
        gd.draw(RDPMemBlt(1, Rect(0, 20, bmp24.cx(), bmp24.cy()), 0x55, 0, 0, 7), screen);
        gd.draw(RDPMemBlt(1, Rect(0, 30, bmp24.cx(), bmp24.cy()), 0x22, 2, 2, 7), screen);
        gd.draw(RDPMemBlt(1, Rect(10, 10, bmp24.cx(), bmp24.cy()), 0xCC, 0, 0, 7), screen);
    }
    RED_CHECK(to_data_url(canvas) == "data:image/png;base64,"
        "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"
        "5wAAAABJRU5ErkJggg=="_av);


    // Pointer
    gd.new_pointer(
        0, RdpPointer(
            CursorSize{10, 10}, Hotspot{3, 4}, BitsPerPixel::BitsPP16,
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
            ""_av,
            "\xF3\xFF"
            "\xF3\xFF"
            "\xF3\xFF"
            "\xF3\xFF"
            "\x00\x00"
            "\x00\x00"
            "\xF3\xFF"
            "\xF3\xFF"
            "\xF3\xFF"
            "\xF3\xFF"
            ""_av));
    gd.cached_pointer(0);
    RED_CHECK(canvas["style"]["cursor"].as<std::string>() == "url(data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAYAAACNMs+9AAAABmJLR0QA/wD/AP+gvaeTA"
        "AAAI0lEQVQYlWNgwAT/oRgFMGFRiBVQXyEjAxb3UGQiNjDYfA0AHvsFCwqC2C8AAAAASU"
        "VORK5CYII=) 3 4, auto"_av);
}
