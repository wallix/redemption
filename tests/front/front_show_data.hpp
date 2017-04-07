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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Clément Moroldo

*/

#pragma once

#include <cstdio>
#include <openssl/ssl.h>
#include <cstdint>

#include "core/RDP/caches/brushcache.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"

#include <algorithm>
#include <fstream>
//#include <boost/algorithm/string.hpp>

#include "core/RDP/pointer.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "mod/mod_api.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/caches/glyphcache.hpp"
//#include "core/RDP/capabilities/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "keymap2.hpp"
#include "core/client_info.hpp"

//#include "utils/png.hpp"

class Front_Show_Data : public FrontAPI
{

public:
    Front_Show_Data( ClientInfo info
                   , int verb)
    : FrontAPI(false, false)
    , verbose(verb)
    , info(info)
    , mod(nullptr)
    , mod_palette(BGRPalette::classic_332())
    , fichier("projects/browser_client_JS/src/test_rdp_client_test_card_JS_2.cpp", std::ios::out | std::ios::trunc)
    {
        fichier << "#define LOGNULL\n";

        fichier << "#include \"../projects/browser_client_JS/src/front_JS_natif.hpp\"\n";

        fichier << "extern \"C\" int main(int argc, char** argv) {\n";

        fichier << "ClientInfo info;\n";
        fichier << "info.keylayout = 0x040C;\n";
        fichier << "info.console_session = 0;\n";
        fichier << "info.brush_cache_code = 0;\n";
        fichier << "info.bpp = 16;\n";
        fichier << "info.width = 1024;\n";
        fichier << "info.height = 768;\n";

        fichier << "front.init_front(info);" << std::endl;
    }


public:

    uint32_t                    verbose;
    ClientInfo                & info;
    CHANNELS::ChannelDefArray   cl;
    uint8_t                     mod_bpp;
    mod_api         * mod;
    BGRPalette                  mod_palette;
    std::ofstream fichier;


    void over() {
       //fichier << "     emscripten_set_main_loop(eventLoop, front._fps, 0); \n";

       fichier << "     return 0;\n";
       fichier << "}" << std::endl;
    }



    const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return cl;
    }

    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {}

    //void send_global_palette() override {}

    void begin_update() override {}

    void end_update() override {}

    //void set_mod_palette(const BGRPalette & palette) override {}

    void update_pointer_position(uint16_t x, uint16_t y) override {}

    int server_resize(int width, int height, int bpp) override {
        return 0;
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    void draw(const RDPOpaqueRect & cmd, const Rect clip) override {
        fichier << "front.draw(RDPOpaqueRect(Rect("<< int(cmd.rect.x) << ", " << int(cmd.rect.y) << ", " << int(cmd.rect.cx) << ", " << int(cmd.rect.cy) << "), "<< int(cmd.color) << "), Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "));" << std::endl;
    }

    void draw(const RDPScrBlt & cmd, const Rect clip) override {
        fichier << "front.draw(RDPScrBlt(Rect("<< int(cmd.rect.x) << ", " << int(cmd.rect.y) << ", " << int(cmd.rect.cx) << ", " << int(cmd.rect.cy) << "), " << int(cmd.rop) << ", " << int(cmd.srcx) << ", " << int(cmd.srcy) << ")," << "Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "));" << std::endl;
    }

    void draw(const RDPMemBlt & cmd, const Rect clip, const Bitmap & bitmap) override {
        fichier << "{\n";
        fichier << "const uint8_t * data = {\n";
        for (int i = 0; i < int(bitmap.bmp_size()) - 1; i++) {
            fichier << int(bitmap.data()[i]) << ", ";
        }
        fichier << int(bitmap.data()[bitmap.bmp_size()-1]) << "\n";
        fichier << "};" << std::endl;

        fichier << "front.draw(RDPMemBlt(" << int(cmd.cache_id) << ", " << "Rect("<< int(cmd.rect.x) << ", " << int(cmd.rect.y) << ", " << int(cmd.rect.cx) << ", " << int(cmd.rect.cy) << ")," << int(cmd.rop) << ", " << int(cmd.srcx) << ", " << int(cmd.srcy) << ", " << int(cmd.cache_idx) << "), Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "), Bitmap(" << int(bitmap.bpp())  << ", " << int(bitmap.bpp()) << ", &(BGRPalette::classic_332()), " << int(bitmap.cx()) << ", " << bitmap.cy() << ", data, " << int(bitmap.bmp_size()) << ", false));\n";
        fichier << "}" << std::endl;
    }

    void draw(const RDPLineTo & cmd, const Rect clip) override {
        fichier << "front.draw(RDPLineTo(" << int(cmd.back_mode) << ", " << int(cmd.startx) << ", " << int(cmd.starty) << ", " << int(cmd.endx) << ", " << int(cmd.endy) << ", " << int(cmd.back_color) << ", " << int(cmd.rop2) << ", " << "RDPPen(" << int(cmd.pen.style) << ", " << int(cmd.pen.width) << ", " << int(cmd.pen.color) << ")), Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "));" << std::endl;
    }

    void draw(const RDPPatBlt & cmd, const Rect clip) override {
        //fichier << "RDPPatBlt" << std::endl;
        fichier << "front.draw(RDPPatBlt( Rect(" << int(cmd.rect.x) << ", " << int(cmd.rect.y) << ", " << int(cmd.rect.cx) << ", " << int(cmd.rect.cy) << "), " <<  int(cmd.rop) << ", " << int(cmd.back_color) <<  ", " << int(cmd.fore_color) << ",  RDPBrush(" << int(cmd.brush.org_x) <<  ", " << int(cmd.brush.org_y) <<  ", " << int(cmd.brush.style) <<   ", " << int(cmd.brush.hatch) << ") ), Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "));" <<  std::endl;
    }

    void draw(const RDPMem3Blt & cmd, const Rect clip, const Bitmap & bitmap) override {
        fichier << "{\n";
        fichier << "const uint8_t * data = {\n";
        for (int i = 0; i < bitmap.bmp_size() - 1; i++) {
            fichier << int(bitmap.data()[i]) << ", ";
        }
        fichier << int(bitmap.data()[bitmap.bmp_size()-1]) << "\n";
        fichier << "};" << std::endl;

        fichier << "front.draw(RDPMem3Blt( " << int(cmd.cache_id) << ", Rect(" << int(cmd.rect.x) << ", " << int(cmd.rect.y) << ", " << int(cmd.rect.cx) << ", " << int(cmd.rect.cy) << "), " <<  int(cmd.rop) << ", " << int(cmd.srcx) <<  ", " << int(cmd.srcy) << ", " << int(cmd.back_color) <<  ", " << int(cmd.fore_color) << ",  RDPBrush(" << int(cmd.brush.org_x) <<  ", " << int(cmd.brush.org_y) <<  ", " << int(cmd.brush.style) <<   ", " << int(cmd.brush.hatch) << ") ), " << int(cmd.cache_idx) << ", Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "),  " << "data" <<  ");\n";
        fichier << "}" << std::endl;
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        //fichier << "RDPBitmapData" << std::endl;
        fichier << "{\n";
        fichier << "RDPBitmapData bitmapData;\n";
        fichier << "bitmapData.dest_left = " << int(bitmap_data.dest_left) << ";\n";
        fichier << "bitmapData.dest_top = " << int(bitmap_data.dest_top) << ";\n";
        fichier << "bitmapData.dest_right = " << int(bitmap_data.dest_right) << ";\n";
        fichier << "bitmapData.dest_bottom = " << int(bitmap_data.dest_bottom) << ";\n";


        fichier << "const uint8_t data[] = {\n";
        fichier << int(bmp.data()[0]) << ", ";
        for (int i = 1; i < bmp.bmp_size() - 1; i++) {
            fichier << int(bmp.data()[i]) << ", ";
            if (i%20 == 0) {
                fichier << "\n";
            }
        }
        fichier << int(bmp.data()[bmp.bmp_size() - 1]) << "\n";
        fichier << "};" << std::endl;


        fichier << "front.draw(bitmapData, Bitmap(" << int(bmp.bpp())  << ", " << int(bmp.bpp()) << ", &(BGRPalette::classic_332()), " << int(bmp.cx()) << ", " << int(bmp.cy()) << ", data, " << int(bmp.bmp_size()) << ", false));\n";

        fichier << "}" << std::endl;
    }

    void draw(const RDPDestBlt & cmd, const Rect clip) override {
        //fichier << "RDPDestBlt" << std::endl;
        fichier << "front.draw(RDPScrBlt(Rect("<< int(cmd.rect.x) << ", " << int(cmd.rect.y) << ", " << int(cmd.rect.cx) << ", " << int(cmd.rect.cy) << "), " << int(cmd.rop) << "), Rect(" << int(clip.x) << ", " << int(clip.y) << ", " << int(clip.cx) << ", " << int(clip.cy) << "));" << std::endl;
    }



    void draw(const RDPMultiDstBlt & cmd, const Rect clip) override {}

    void draw(const RDPMultiOpaqueRect & cmd, const Rect clip) override {}

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect clip) override {}

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect clip) override {}

    void draw(const RDPGlyphIndex & cmd, const Rect clip, const GlyphCache & gly_cache) override {}

    void draw(const RDPPolygonSC & cmd, const Rect clip) override {}

    void draw(const RDPPolygonCB & cmd, const Rect clip) override {}

    void draw(const RDPPolyline & cmd, const Rect clip) override {}

    void draw(const RDPEllipseSC & cmd, const Rect clip) override {}

    void draw(const RDPEllipseCB & cmd, const Rect clip) override {}

    void draw(const RDP::FrameMarker & order) override {}

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {}

    void draw(const RDP::RAIL::WindowIcon & order) override {}

    void draw(const RDP::RAIL::CachedIcon & order) override {}

    void draw(const RDP::RAIL::DeletedWindow & order) override {}

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override {}

    void draw(const RDP::RAIL::DeletedNotificationIcons & order) override {}

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override {}

    void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {}

    void draw(const RDPColCache   & cmd) override {}

    void draw(const RDPBrushCache & cmd) override {}
};
