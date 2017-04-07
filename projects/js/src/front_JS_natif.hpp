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

#ifndef FRONT_JS_NATIF_HPP
#define FRONT_JS_NATIF_HPP

#include <stdio.h>
//#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
//#include <SDL/SDL.h>
#include <math.h>


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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

//#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
//#include <boost/algorithm/string.hpp>

#include "core/RDP/pointer.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "mod/mod_api.hpp"
#include "bitmap_without_png.hpp"
#include "core/RDP/caches/glyphcache.hpp"
//#include "core/RDP/capabilities/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "keyboard/keymap2.hpp"
#include "core/client_info.hpp"
#include "keyboard/reversed_keymaps/keylayouts_r.hpp"
#include "utils/colors.hpp"

// bjam client_rdp_JS_natif |& grep error || iceweasel file:///home/cmoroldo/Bureau/redemption/projects/browser_client_JS/sandbox/client_rdp_JS_natif.html



// #--shell-file templates/penta_template.html
// -s EXPORTED_FUNCTIONS="['_run_main']"

// source emsdk_env.sh
// . ./emsdk_env.sh






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
    const Keylayout_r  * _keylayout;

    enum: int {
        KBD_FLAGS_EXTENDED = 0x0100,
        KBD_FLAGS          = 0
    };

    enum: int {
        SCANCODE_ALTGR = 0x38,
        SCANCODE_SHIFT = 0x36
    };


    //bool setClientInfo() ;

    //void writeClientInfo() ;

    void setClientInfo(ClientInfo & info) {
        this->_info = info;
        this->_mod_bpp = this->_info.bpp;
        this->setKeyboardLayout(this->_info.keylayout);
    }

    void setKeyboardLayout(int LCID) {
        bool found = false;
        for (uint8_t i = 0 ; i < KEYLAYOUTS_LIST_SIZE; i++) {
            if (keylayoutsList[i]->LCID == LCID){
                this->_keylayout = keylayoutsList[i];
                found = true;
                break;
            }
        }
        if (!found){
            std::cout << std::hex << "Unknown keyboard layout (0x" << LCID << "). Reverting to default (English - United States - International)." << std::endl;
            this->setKeyboardLayout(KEYBOARDS::EN_US_INTERNATIONAL);
        }
/*
        this->_layoutMods[NO_MOD                              ] = this->_keylayout->getnoMod();
        this->_layoutMods[SHIFT_MOD                           ] = this->_keylayout->getshift();
        this->_layoutMods[ALTGR_MOD                           ] = this->_keylayout->getaltGr();
        this->_layoutMods[ALTGR_MOD    + SHIFT_MOD            ] = this->_keylayout->getshiftAltGr();
        this->_layoutMods[CAPSLOCK_MOD + NO_MOD               ] = this->_keylayout->getcapslock_noMod();
        this->_layoutMods[CAPSLOCK_MOD + SHIFT_MOD            ] = this->_keylayout->getcapslock_shift();
        this->_layoutMods[CAPSLOCK_MOD + ALTGR_MOD            ] = this->_keylayout->getcapslock_altGr();
        this->_layoutMods[CAPSLOCK_MOD + ALTGR_MOD + SHIFT_MOD] = this->_keylayout->getcapslock_shiftAltGr();
        this->_layoutMods[CTRL_MOD                            ] = this->_keylayout->getctrl();*/

    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->_cl;
    }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {}

    virtual void begin_update() override {}

    virtual void end_update() override {}

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
        const +Rect rect = clip.intersect(rectBmp);
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
        const +Rect rect = clip.intersect(cmd.rect);
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

    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons order) override {}

    virtual void draw(const RDP::RAIL::DeletedNotificationIcons order) override {}

    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop order) override {}

    virtual void draw(const RDP::RAIL::NonMonitoredDesktop order) override {}

    virtual void draw(const RDPColCache   & cmd) override {}

    virtual void draw(const RDPBrushCache & cmd) override {}



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //------------------------
    //      CONSTRUCTOR
    //------------------------

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

    void sendRDPScanCode(int code,  int flag) {
        try {
            EM_ASM_({ console.log('KeyPressed ' + $0); }, code);
            //  this->_callback->rdp_input_scancode(code, 0, flag | KBD_FLAGS_DOWN, 0, &(this->_keymap));
            //  this->_callback->rdp_input_scancode(code, 0, flag | KBD_FLAGS_RELEASE, 0, &(this->_keymap))
        } catch (const Error & e) { }
    }

    void mousePressEvent(int x, int y, int button) {
        int flag = 0;
        switch (button) {
            case 1: flag = MOUSE_FLAG_BUTTON1; break;
            case 2: flag = MOUSE_FLAG_BUTTON2; break;
            case 4: flag = MOUSE_FLAG_BUTTON4; break;
            default: break;
        }
        //  this->_callback->rdp_input_mouse(flag | MOUSE_FLAG_DOWN, x, y, &(this->_keymap));
        EM_ASM_({ console.log('down ' + $0 + ' ' + $1 + ' ' + $2); }, x, y, button);
    }

    void mouseReleaseEvent(int x, int y, int button) {
        int flag = 0;
        switch (button) {
            case 1: flag = MOUSE_FLAG_BUTTON1; break;
            case 2: flag = MOUSE_FLAG_BUTTON2; break;
            case 4: flag = MOUSE_FLAG_BUTTON4; break;
            default: break;
        }
        //  this->_callback->rdp_input_mouse(flag, x, y, &(this->_keymap));
        EM_ASM_({ console.log('up ' + $0 + ' ' + $1 + ' ' + $2); }, x, y, button);
    }

    void mouseMoveEvent(int x, int y) {
        EM_ASM_({ console.log('Move '); }, x, y);
        //this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, &(this->_keymap));
    }

    void keyPressEvent(int code) {

        int flag = 0;

        if (code < 0) {
            code += 256;
        }

        switch (code) {

            //-----------------------
            //  Keylayout SHIFT MOD
            //-----------------------
            case 168 : /* ¨ */ // this->_callback->rdp_input_scancode(SCANCODE_SHIFT, 0, flag | KBD_FLAGS_DOWN, 0,         &(this->_keymap));
                               this->sendRDPScanCode(this->_keylayout->deadkeys.at(code), flag);
                               // this->_callback->rdp_input_scancode(SCANCODE_SHIFT, 0, flag | KBD_FLAGS_RELEASE, 0, &(this->_keymap));
                            break;
            case 37  : /* % */
            case 43  : /* + */
            case 46  : /* . */
            case 63  : /* ? */
            case 65  : /* A */
            case 90  : /* Z */
            case 69  : /* E */
            case 82  : /* R */
            case 84  : /* T */
            case 89  : /* Y */
            case 85  : /* U */
            case 73  : /* I */
            case 79  : /* O */
            case 80  : /* P */
            case 81  : /* Q */
            case 83  : /* S */
            case 68  : /* D */
            case 70  : /* F */
            case 71  : /* G */
            case 72  : /* H */
            case 74  : /* J */
            case 75  : /* K */
            case 76  : /* L */
            case 77  : /* M */
            case 87  : /* W */
            case 88  : /* X */
            case 67  : /* C */
            case 86  : /* V */
            case 66  : /* B */
            case 78  : /* N */
            case 162 : /* > */
            case 163 : /* £ */
            case 167 : /* § */
            case 176 : /* ° */
            case 181 : /* µ */ // this->_callback->rdp_input_scancode(SCANCODE_SHIFT, 0, flag | KBD_FLAGS_DOWN, 0,         &(this->_keymap));
                               this->sendRDPScanCode(this->_keylayout->getshift()->at(code), flag);
                               // this->_callback->rdp_input_scancode(SCANCODE_SHIFT, 0, flag | KBD_FLAGS_RELEASE, 0, &(this->_keymap));
                            break;


            //-----------------------
            //  Keylayout ALTGR MOD
            //-----------------------
            case 96  : /* ` */ // this->_callback->rdp_input_scancode(SCANCODE_ALTGR, 0, flag | KBD_FLAGS_DOWN, 0,         &(this->_keymap));
                              this->sendRDPScanCode(this->_keylayout->deadkeys.at(code), flag);
                              // this->_callback->rdp_input_scancode(SCANCODE_ALTGR, 0, flag | KBD_FLAGS_RELEASE, 0, &(this->_keymap));
                            break;
            case 35  : /* # */
            case 64  : /* @ */
            case 91  : /* [ */
            case 92  : /* \ */
            case 93  : /* ] */
            case 123 : /* { */
            case 124 : /* | */
            case 125 : /* } */
            case 126 : /* ~ */
            case 234 : /* ê */ // this->_callback->rdp_input_scancode(SCANCODE_ALTGR, 0, flag | KBD_FLAGS_DOWN, 0,         &(this->_keymap));
                               this->sendRDPScanCode(this->_keylayout->getaltGr()->at(code), flag);
                               // this->_callback->rdp_input_scancode(SCANCODE_ALTGR, 0, flag | KBD_FLAGS_RELEASE, 0, &(this->_keymap));
                            break;


            //-----------------------
            //   Keylayout NO MOD
            //-----------------------
            case 94  : /* ^ */ this->sendRDPScanCode(this->_keylayout->deadkeys.at(code), flag);
                            break;
            case 47  : /* / */ flag = KBD_FLAGS_EXTENDED;
            case 224 : /* à */
            case 231 : /* ç */
            case 232 : /* è */
            case 233 : /* é */
            case 249 : /* ù */
            default  :         this->sendRDPScanCode(this->_keylayout->getnoMod()->at(code), flag);
                            break;
        }

    }

    void keyReleaseEvent(char code) {
        EM_ASM_({ console.log('KeyRelease ' + $0); }, code);
    }

/*
    void wheelEvent(QWheelEvent *e) override {}

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

    front.setClientInfo(info);

    //emscripten_set_main_loop(eventLoop, front._fps, 0);

    return 0;
}*/


#endif
