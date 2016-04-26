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
   Author(s): Clément Moroldo
*/

#ifndef FRONT_QT_NATIF_HPP
#define FRONT_QT_NATIF_HPP

#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
//#include <SDL/SDL.h>
#include <math.h>


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "RDP/caches/brushcache.hpp"
#include "RDP/capabilities/colcache.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "RDP/orders/AlternateSecondaryWindowing.hpp"

//#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
//#include <boost/algorithm/string.hpp>

#include "core/RDP/pointer.hpp"
#include "front_api.hpp"
#include "channel_list.hpp"
#include "mod_api.hpp"
#include "bitmap_without_png.hpp"
#include "RDP/caches/glyphcache.hpp"
//#include "RDP/capabilities/glyphcache.hpp"
#include "RDP/bitmapupdate.hpp"
#include "keymap2.hpp"
#include "client_info.hpp"




// bjam client_rdp_JS_natif |& grep error || iceweasel file:///home/cmoroldo/Bureau/redemption/projects/browser_client_JS/sandbox/client_rdp_JS_natif.html



// #--shell-file templates/penta_template.html
// -s EXPORTED_FUNCTIONS="['_run_main']"

// source emsdk_env.sh
// . ./emsdk_env.sh
#include "../../utils/colors.hpp"





class Front_JS_Natif : public FrontAPI
{


/*
    class ClipboardServerChannelDataSender : public VirtualChannelDataSender
    {
    public:
        mod_api        * _callback;

        ClipboardServerChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            InStream chunk(chunk_data, chunk_data_length);
            this->_callback->send_to_mod_channel(channel_names::cliprdr, chunk, total_length, flags);
        }
    };

    class ClipboardClientChannelDataSender : public VirtualChannelDataSender
    {
    public:
        FrontAPI            * _front;
        CHANNELS::ChannelDef  _channel;

        ClipboardClientChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            this->_front->send_to_channel(this->_channel, chunk_data, total_length, chunk_data_length, flags);
        }
    };
*/

public:

    uint32_t          verbose;
    ClientInfo        _info;
    std::string       _userName;
    std::string       _pwd;
    std::string       _targetIP;
    int               _port;
    std::string       _localIP;
    int               _nbTry;
    int               _retryDelay;
    mod_api         * _callback;
    //ClipboardServerChannelDataSender _to_server_sender;
    //ClipboardClientChannelDataSender _to_client_sender;
    int                  _fps;

    // Graphic members
    uint8_t                 _mod_bpp;
    BGRPalette  mod_palette;


    // Connexion socket members

    int                  _timer;
    bool                 _connected;
    //ClipboardVirtualChannel  _clipboard_channel;

    // Keyboard Controllers members
    Keymap2              _keymap;
    bool                 _ctrl_alt_delete; // currentinit_frontly not used and always false
    StaticOutStream<256> _decoded_data;    // currently not initialised
    uint8_t              _keyboardMods;
    CHANNELS::ChannelDefArray   _cl;
    uint32_t             _requestedFormatId;
    std::string          _requestedFormatShortName;
    uint8_t            * _bufferRDPClipboardChannel;
    size_t               _bufferRDPClipboardChannelSize;
    size_t               _bufferRDPClipboardChannelSizeTotal;
    int                  _bufferRDPCLipboardMetaFilePic_width;
    int                  _bufferRDPCLipboardMetaFilePic_height;
    int                  _bufferRDPClipboardMetaFilePicBPP;




    //bool setClientInfo() ;

    //void writeClientInfo() ;

    void setClientInfo(ClientInfo & info) {
        this->_info = info;
        this->_mod_bpp = this->_info.bpp;
    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->_cl;
    }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {}

    virtual void begin_update() override {}

    virtual void end_update() override {}

    //virtual void setmod_palette(const BGRPalette & palette) {}

    virtual void update_pointer_position(uint16_t x, uint16_t y) override {}

    virtual int server_resize(int width, int height, int bpp) override {
        return 0;
    }
    /*
    void send_buffer_to_clipboard();

    void process_server_clipboard_data(int flags, InStream & chunk);

    void send_FormatListPDU(const uint32_t * formatIDs, const std::string * formatListDataShortName, std::size_t formatIDs_size) ;

    std::string HTMLtoText(const std::string & html);

    void send_to_clipboard_Buffer(InStream & chunk);

    void send_textBuffer_to_clipboard(bool isTextHtml);

    void send_imageBuffer_to_clipboard();

    void empty_buffer() ;
    */


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //---------------------------------------
    //   GRAPHIC FUNCTIONS (factorization)
    //---------------------------------------

    double abs(double x) {
        if (x > 0) {
            return x;
        }
        return -x;
    }

    void swap(double & x, double & y) {
        double z = x;
        x = y;
        y = z;
    }

    int switchRGBA(int bgr) {

        uint8_t b = bgr >> 24;
        uint8_t g = bgr >> 16;
        uint8_t r = bgr >> 8;
        uint8_t a = bgr;

        return ( (r << 24) + (g << 16) + (b << 8) + a );
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        Rect rect(cmd.rect.intersect(clip).intersect(this->_info.width, this->_info.height));
        uint32_t color((uint32_t) color_decode_opaquerect(cmd.color, 16, this->mod_palette));

        EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                     rect.x, rect.y, rect.cx, rect.cy, color    );
    }


    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        const Rect rect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);
        if (rect.isempty()) {
            return;
        }

        int srcx(rect.x + cmd.srcx - cmd.rect.x);
        int srcy(rect.y + cmd.srcy - cmd.rect.y);

        switch (cmd.rop) {

            case 0x00: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0x000000ff);
                break;

            case 0x55: EM_ASM_({drawable.rDPScrBlt_Invert($0  , $1  , $2      , $3      , $4       , $5); },
                                                          srcx, srcy, rect.x-1, rect.y-1, rect.cx+2, rect.cy+2 );
                break;

            case 0xAA: // nothing to change
                break;

            case 0xCC: EM_ASM_({drawable.rDPScrBlt($0  , $1  , $2      , $3      , $4       , $5); },
                                                   srcx, srcy, rect.x-1, rect.y-1, rect.cx+2, rect.cy+2 );
                break;

            case 0xFF: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0xffffffff);
                break;

            default: //std::cout << "RDPScrBlt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        Rect rectBmp(cmd.rect);
        const Rect& rect = clip.intersect(rectBmp);
        if (rect.isempty()){
            return ;
        }

        switch (cmd.rop) {

            case 0x00: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0x000000ff);
                break;

            case 0x22: // TODO
            {
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);

                       Bitmap bitmapBpp(32, bitmap);
                       const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width  - rect.x, rect.cx));
                       const int16_t mincy = std::min<int16_t>(bitmapBpp.cy(), std::min<int16_t>(this->_info.height - rect.y, rect.cy));

                       EM_ASM_({drawable.rDPMemBlt_0x22($0    , $1    , $2     , $3     , HEAPU8.subarray($4, $4 + $5 - 1), $6,  $7,  $8);},
                                                rect.x, rect.y, mincx, mincy, bitmapBpp.data(), bitmapBpp.bmp_size(), 0, srcx,  srcy);
            }
            break;

            case 0x55: // TODO
            {
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);

                       Bitmap bitmapBpp(32, bitmap);
                       const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width  - rect.x, rect.cx));
                       const int16_t mincy = std::min<int16_t>(bitmapBpp.cy(), std::min<int16_t>(this->_info.height - rect.y, rect.cy));

                       EM_ASM_({drawable.rDPMemBlt_0x55($0    , $1    , $2     , $3     , HEAPU8.subarray($4, $4 + $5 - 1), $6,  $7,  $8);},
                                                rect.x, rect.y, mincx, mincy, bitmapBpp.data(), bitmapBpp.bmp_size(), 0, srcx,  srcy);
            }
                break;

            case 0x66: // TODO
            {
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);

                       Bitmap bitmapBpp(32, bitmap);
                       const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width  - rect.x, rect.cx));
                       const int16_t mincy = std::min<int16_t>(bitmapBpp.cy(), std::min<int16_t>(this->_info.height - rect.y, rect.cy));

                       EM_ASM_({drawable.rDPMemBlt_0x66($0    , $1    , $2     , $3     , HEAPU8.subarray($4, $4 + $5 - 1), $6,  $7,  $8);},
                                                rect.x, rect.y, mincx, mincy, bitmapBpp.data(), bitmapBpp.bmp_size(), 0, srcx,  srcy);
            }
            break;

            case 0x99:  // nothing to change
                break;

            case 0xCC: // TODO
            {
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);

                       Bitmap bitmapBpp(32, bitmap);
                       const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width  - rect.x, rect.cx));
                       const int16_t mincy = std::min<int16_t>(bitmapBpp.cy(), std::min<int16_t>(this->_info.height - rect.y, rect.cy));

                       EM_ASM_({drawable.rDPMemBlt($0    , $1    , $2     , $3     , HEAPU8.subarray($4, $4 + $5 - 1), $6,  $7,  $8);},
                                                rect.x, rect.y, mincx, mincy, bitmapBpp.data(), bitmapBpp.bmp_size(), 0, srcx,  srcy);

            }
                break;

            case 0xEE: // TODO
            {
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);

                       Bitmap bitmapBpp(32, bitmap);
                       const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width  - rect.x, rect.cx));
                       const int16_t mincy = std::min<int16_t>(bitmapBpp.cy(), std::min<int16_t>(this->_info.height - rect.y, rect.cy));

                       EM_ASM_({drawable.rDPMemBlt($0    , $1    , $2     , $3     , HEAPU8.subarray($4, $4 + $5 - 1), $6,  $7,  $8);},
                                                rect.x, rect.y, mincx, mincy, bitmapBpp.data(), bitmapBpp.bmp_size(), 0, srcx,  srcy);
            }
                break;

            case 0xFF: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0xffffffff);
                break;

            default: //std::cout << "RDPMemBlt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        const Rect rect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);
        if (rect.isempty()) {
            return;
        }

        switch (cmd.rop) {

            case 0x00: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0x000000ff);
                break;

            case 0x55: EM_ASM_({ drawable.rDPScrBlt_Invert($0      , $1      , $2      , $3      , $4       , $5); },
                                                           rect.x-1, rect.y-1, rect.x-1, rect.y-1, rect.cx+2, rect.cy+2 );
                break;

            case 0xAA: // nothing to change
                break;

            case 0xFF: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0xffffffff);
                break;
            default: //std::cout << "RDPScrBlt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        RDPPatBlt new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->_mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->_mod_bpp, this->mod_palette);
        const Rect rect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);

        // external
        //if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) {

        switch (cmd.rop) {

            case 0x5A: EM_ASM_({drawable.RDPPatBlt_0x5A($0    , $1    , $2     , $3     , $4,  $5,  $6,  $7,  $8,  $9);},
                                                        rect.x, rect.y, rect.cx, rect.cy, uint8_t(new_cmd24.back_color >> 16)
                                                                                        , uint8_t(new_cmd24.back_color >> 8)
                                                                                        , uint8_t(new_cmd24.back_color)
                                                                                        , uint8_t(new_cmd24.fore_color >> 16)
                                                                                        , uint8_t(new_cmd24.fore_color >> 8)
                                                                                        , uint8_t(new_cmd24.fore_color)     );
                break;

            case 0xF0: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, new_cmd24.back_color);

                       EM_ASM_({drawable.RDPPatBlt_0xF0($0    , $1    , $2     , $3     , $4,  $5,  $6 );},
                                                        rect.x, rect.y, rect.cx, rect.cy, uint8_t(new_cmd24.fore_color >> 16)
                                                                                        , uint8_t(new_cmd24.fore_color >> 8)
                                                                                        , uint8_t(new_cmd24.fore_color)      );
                break;

            case 0x00: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0x000000ff);
                break;

            case 0x55: EM_ASM_({ drawable.rDPScrBlt_Invert($0  , $1  , $2      , $3      , $4       , $5); },
                                                           rect.x-1, rect.y-1, rect.x-1, rect.y-1, rect.cx+2, rect.cy+2 );
                break;

            case 0xAA: // change nothing
                break;

            case 0xFF: EM_ASM_({drawable.opaqueRect($0    , $1    , $2     , $3     , $4   );},
                                                    rect.x, rect.y, rect.cx, rect.cy, 0xffffffff);
                break;

            default:  //std::cout << "RDPPatBlt " << (int) cmd.rop << std::endl;
                break;
        }
    }


    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override {
        //const Rect rect = clip.intersect(this->_info.width, this->_info.height);
        uint32_t color = (uint32_t)color_decode_opaquerect(cmd.back_color, this->_mod_bpp, BGRPalette::classic_332());
        uint8_t b(color >> 16);
        uint8_t g(color >> 8);
        uint8_t r(color);

        double x1(cmd.startx);
        double y1(cmd.starty);
        double x2(cmd.endx);
        double y2(cmd.endy);

        if(x1 > x2)
        {
            this->swap(x1, x2);
            this->swap(y1, y2);
        }

        EM_ASM_({drawable.lineTo($0, $1, $2, $3, $4, $5, $6);},
                                 x1, y1, x2, y2, r , g , b     );
    }


    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        const Rect& rect = clip.intersect(cmd.rect);
        if (rect.isempty() || bitmap.cx() <= 0 || bitmap.cy() <= 0){
            return ;
        }

        switch (cmd.rop) {
            case 0xB8: // TODO
                {
                    Bitmap bitmapBpp(32, bitmap);
                    const uint8_t * bitMapData = bitmapBpp.data();

                    EM_ASM_({drawable.rDPMem3Blt_0xB8($0    , $1    , $2   , $3   , HEAPU8.subarray($4, $4 + $5),  $6,  $7);},
                                                      rect.x, rect.y, rect.cx, rect.cy, bitmapBpp.data(), bitmapBpp.bmp_size(), 0, cmd.back_color);
                }
                break;

            default: //std::cout << "RDPMem3Blt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    virtual void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {

        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                     (bitmap_data.dest_right  - bitmap_data.dest_left + 5),
                     (bitmap_data.dest_bottom - bitmap_data.dest_top  + 5));
        const Rect clipRect(0, 0, this->_info.width, this->_info.height);
        const Rect rect = rectBmp.intersect(clipRect);

        Bitmap bitmapBpp(32, bmp);
        const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width  - rect.x, rect.cx));
        const int16_t mincy = std::min<int16_t>(bitmapBpp.cy(), std::min<int16_t>(this->_info.height - rect.y, rect.cy));
        const int shift = bitmapBpp.cx() - (bitmap_data.dest_right  - bitmap_data.dest_left + 1);

        EM_ASM_({drawable.bitmap($0    , $1    , $2   , $3   , HEAPU8.subarray($4, $4 + $5),  $6);},
                                 rect.x, rect.y, mincx, mincy, bitmapBpp.data(), bitmapBpp.bmp_size(), shift);
    }


    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {}

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {}

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {}

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {}

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override {}

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {}

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {}

    void draw(const RDPPolyline & cmd, const Rect & clip) override {}

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) override {}

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) override {}

    virtual void draw(const RDP::FrameMarker & order) override {}

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) override {}

    virtual void draw(const RDP::RAIL::WindowIcon & order) override {}

    virtual void draw(const RDP::RAIL::CachedIcon & order) override {}

    virtual void draw(const RDP::RAIL::DeletedWindow & order) override {}

    virtual void draw(const RDPColCache   & cmd) override {}

    virtual void draw(const RDPBrushCache & cmd) override {}



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //------------------------
    //      CONSTRUCTOR
    //------------------------

    Front_JS_Natif(ClientInfo & info,  int verb)
    : FrontAPI(false, false)
    , verbose((uint32_t)verb)
    , _info(info)
    , _port(0)
    , _callback(nullptr)
    , _fps(30)
    , _mod_bpp(this->_info.bpp)
    , mod_palette(BGRPalette::classic_332())
    {
        //this->_to_client_sender._front = this;
    }

    Front_JS_Natif(int verb)
    : FrontAPI(false, false)
    , verbose((uint32_t)verb)
    , _port(0)
    , _callback(nullptr)
    , _fps(30)
    , _mod_bpp(32)
    , mod_palette(BGRPalette::classic_332())
    {
        //this->_to_client_sender._front = this;
    }

    ~Front_JS_Natif() {}



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //------------------------
    //      CONTROLLERS
    //------------------------

    void mousePressEvent(int x, int y, int button) {
        EM_ASM_({ console.log('up ' + $0 + ' ' + $1 + ' ' + $2); }, x, y, button);
    }

    void mouseReleaseEvent(int x, int y, int button) {
        EM_ASM_({ console.log('down ' + $0 + ' ' + $1 + ' ' + $2); }, x, y, button);
    }

    void mouseMoveEvent(int x, int y, int button) {
        EM_ASM_({ console.log('Move ' + $0 + ' ' + $1); }, x, y);
    }
/*
    void keyPressEvent(QKeyEvent *e) override {}

    void keyReleaseEvent(QKeyEvent *e) override {}

    void wheelEvent(QWheelEvent *e) override {}

    bool eventFilter(QObject *obj, QEvent *e) override {}

    void connexionPressed() override {}

    void connexionReleased() override {}

    void RefreshPressed() override {}

    void RefreshReleased() override {}

    void CtrlAltDelPressed() override {}

    void CtrlAltDelReleased() override {}

    void disconnexionPressed()  override {}

    void disconnexionReleased() override {}

    void refresh(int x, int y, int w, int h);

    void send_rdp_scanCode(int keyCode, int flag);

    void connect();

    void disconnect(std::string) override {}

    void closeFromScreen() override {}

    void dropScreen() override {}

*/

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------


};


Front_JS_Natif front(0);




/*
int main(int argc, char** argv){

    ClientInfo info;
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 32;
    info.width = 800;
    info.height = 600;

    front.init_front(info);

    emscripten_set_main_loop(eventLoop, front._fps, 0);

    return 0;
}*/


#endif
