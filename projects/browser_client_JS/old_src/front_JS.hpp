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

#ifndef FRONT_JS_SDL_HPP
#define FRONT_JS_SDL_HPP

#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
#include <SDL/SDL.h>
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


// ./../../emsdk_portable/emscripten/master/em++ -O2 tests/client_mods/test_rdp_client_test_card_JS.cpp -o bin/gcc-4.9.2/release/client_rdp_JS.html -I src/ -I src/utils -I src/core -std=c++11 -I src/configs/autogen -I src/configs/variant -I src/configs/include -I src/mod -I src/front -I src/acl -I src/capture -I src/keyboard -I src/keyboard/reversed_keymaps -I src/regex -I src/headers -I src/main -I tests -I modules/includes -I src/system/JS && iceweasel file:///home/cmoroldo/Bureau/redemption/bin/gcc-4.9.2/release/client_rdp_JS.html


// bjam client_rdp_JS.js |& grep error || iceweasel file:///home/cmoroldo/Bureau/redemption/projects/browser_client_JS/bin/gcc-4.9.2/release/client_rdp_JS.html



// #--shell-file templates/penta_template.html
// -s EXPORTED_FUNCTIONS="['_run_main']"

// source emsdk_env.sh
// . ./emsdk_env.sh
#include "../../utils/colors.hpp"

/*
extern "C" {
    SDL_Surface* SDL_CreateRGBSurfaceFrom(void*  pixels,
                                          int    width,
                                          int    height,
                                          int    depth,
                                          int    pitch,
                                          Uint32 Rmask,
                                          Uint32 Gmask,
                                          Uint32 Bmask,
                                          Uint32 Amask)
        {
            return &((SDL_Surface)EM_ASM_({SDL_CreateRGBSurfaceFrom($0    , $1   , $2    , $3   , $4   , $5   , $6   , $7   , $8);},
                                                                    pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask));
        }
}*/


class Front_JS_SDL : public FrontAPI
{
    enum uint32_t {
        INVERT_MASK = 0xffffffff,
        NO_MASK     = 0x00000000
    };

private:
    class Screen_JS
    {
    public:
        Front_JS_SDL    * _front;
        SDL_Surface * _screen;


        Screen_JS(Front_JS_SDL * front)
            : _front(front)
        {
            SDL_Init(SDL_INIT_VIDEO);
            //SDL_WM_SetCaption("Remote Desktop Plop", NULL);
            this->_screen = SDL_SetVideoMode( this->_front->_info.width
                                            , this->_front->_info.height
                                            , 32
                                            , SDL_SWSURFACE | SDL_DOUBLEBUF
                                            );
        }

    };

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

    Screen_JS _browser;



    SDL_Surface * getScreen(){
        return this->_browser._screen;
    }

    //bool setClientInfo() ;

    //void writeClientInfo() ;

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->_cl;
    }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {}

    virtual void begin_update() override {}

    virtual void end_update() override {}

    virtual void setmod_palette(const BGRPalette & palette) {}

    virtual void update_pointer_position(uint16_t x, uint16_t y) {}

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

    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect){
        SDL_Rect dPosition = {drect.x, drect.y, drect.cx, drect.cy};
        SDL_Rect sPosition = {srcx   , srcy   , drect.cx, drect.cy};
        SDL_BlitSurface(this->_browser._screen, &sPosition, this->_browser._screen, &dPosition);
    }


    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, uint32_t mask){
        SDL_Rect dPosition = {drect.x, drect.y, drect.cx, drect.cy};
        SDL_Rect sPosition = {srcx   , srcy   , drect.cx, drect.cy};
        SDL_BlitSurface(this->_browser._screen, &sPosition, this->_browser._screen, &dPosition);

        SDL_LockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
        for (int y = 0; y <  drect.cy; y++) {
            for (int x = 0; x < drect.cx; x++) {
                Uint32 & currentPixel = *((Uint32*)this->_browser._screen->pixels + ((y + drect.y) * this->_info.width) + x + drect.x);
                currentPixel = currentPixel ^ mask;
            }
        }
        SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
    }


    void draw_bmp(const Rect & drect, const Bitmap & bitmap) {

        Bitmap bitmapBpp(32, bitmap);
        const int16_t mincx = std::min<int16_t>(bitmapBpp.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
        const uint8_t * bitMapData = bitmapBpp.data();
/*
        SDL_Surface* image = SDL_CreateRGBSurfaceFrom(reinterpret_cast<const void *>(bitMapData), mincx, drect.cy, 32, 4,  0,  0,  0, 0);

        SDL_Rect position;
        position.x = drect.x;
        position.y = drect.y;

        SDL_BlitSurface(image, NULL, this->_browser._screen, &position);
*/

        SDL_LockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
        int yStart(drect.cy + drect.y-1);
        for (int y = 0; y <  drect.cy; y++) {
            for (int x = 0; x < mincx; x++) {
                int srcIndice( ((y * bitmapBpp.cx()) + x) * ((bitmapBpp.bpp()+1)/8) );
                *((Uint32*)this->_browser._screen->pixels + ((yStart - y) * this->_info.width) + x + drect.x) = newPixel(
                    bitMapData, srcIndice);
            }
        }
        SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))

    }


    void draw_MemBlt(const Rect & drect, const Bitmap & bitmap, int srcx, int srcy, uint32_t mask) {
        Bitmap bitmapBpp(32, bitmap);
        if (bitmapBpp.cx() <= 0 || bitmapBpp.cy() <= 0) {
            return;
        }

        const uint8_t * bitMapData = bitmapBpp.data();

        SDL_LockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
        int yStart(drect.cy + drect.y-1);
        for (int y = 0; y <  drect.cy; y++) {
            for (int x = 0; x < drect.cx; x++) {
                int indice( (((y + srcy) * (drect.cx + srcx)) + x + srcx) * ((bitmapBpp.bpp()+1)/8) );
                *((Uint32*)this->_browser._screen->pixels + ((yStart - y) * this->_info.width) + x + drect.x) = newPixel(bitMapData, indice) ^ mask;
            }
        }
        SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
    }


    Uint32 newPixel(const uint8_t * bitMapData, const int indice) {

        uint8_t b =  bitMapData[indice  ];
        uint8_t g =  bitMapData[indice+1];
        uint8_t r =  bitMapData[indice+2];

        return SDL_MapRGBA(this->_browser._screen->format, r, g, b,  255);
    }


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



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        Rect rect(cmd.rect.intersect(clip).intersect(this->_info.width, this->_info.height));
        uint32_t color((uint32_t) color_decode_opaquerect(cmd.color, 16, this->mod_palette));

        uint8_t b = (color & 0x00ff0000) >> 16;
        uint8_t g = (color & 0x0000ff00) >> 8;
        uint8_t r =  color & 0x000000ff;

        SDL_Rect trect = {rect.x, rect.y, rect.cx, rect.cy};
        SDL_FillRect(this->_browser._screen, &trect, SDL_MapRGB(this->_browser._screen->format, r, g, b));
    }


    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        const Rect rect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);
        if (rect.isempty()) {
            return;
        }

        int srcx(rect.x + cmd.srcx - cmd.rect.x);
        int srcy(rect.y + cmd.srcy - cmd.rect.y);

        switch (cmd.rop) {

            case 0x00: {SDL_Rect trect = {rect.y, rect.x, rect.cx, rect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, 0);}
                break;

            case 0x55: this->draw_RDPScrBlt(srcx, srcy, rect, INVERT_MASK);
                break;

            case 0xAA: // nothing to change
                break;

            case 0xCC: this->draw_RDPScrBlt(srcx, srcy, rect);
                break;

            case 0xFF: {SDL_Rect trect = {rect.y, rect.x, rect.cx, rect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, 0xffffffff);}
                break;

            default: std::cout << "RDPScrBlt (" << std::hex << (int)cmd.rop << ")" << std::endl;
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

            case 0x00: {SDL_Rect trect = {rect.y, rect.x, rect.cx, rect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, 0);}
                break;

            case 0x22:
            {
                       const uint8_t * srcData = bitmap.data();
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);
                       int lenx = srcy + rect.cy;
                       int leny = srcx+ rect.cx;
                       SDL_LockSurface(this->_browser._screen);
                       for (int y = srcy; y < leny; y++) {
                           for (int x = srcx; x < lenx; x++) {
                               Uint32 & destPixel = *((Uint32*)this->_browser._screen->pixels + ((y + rect.y) * this->_info.width)+ x + rect.x);
                               destPixel = destPixel & ~(srcData[((y - srcy + rect.y) * this->_info.width) + x + rect.x - srcx]);
                           }
                       }
                       SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
            }
            break;

            case 0x55: this->draw_MemBlt(rect, bitmap, cmd.srcx + (rect.x - cmd.rect.x), cmd.srcy + (rect.y - cmd.rect.y), INVERT_MASK);
                break;

            case 0x66:
            {
                       const uint8_t * srcData = bitmap.data();
                       int srcx = cmd.srcx + (rect.x - cmd.rect.x);
                       int srcy = cmd.srcy + (rect.y - cmd.rect.y);
                       int lenx = srcy + rect.cy;
                       int leny = srcx+ rect.cx;
                       SDL_LockSurface(this->_browser._screen);
                       for (int y = srcy; y < leny; y++) {
                           for (int x = srcx; x < lenx; x++) {
                               Uint32 & destPixel = *((Uint32*)this->_browser._screen->pixels + ((y + rect.y) * this->_info.width)+ x + rect.x);
                               destPixel = destPixel ^ srcData[((y - srcy + rect.y) * rect.cx) + x + rect.x - srcx];
                           }
                       }
                       SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
            }
            break;

            case 0x99:  // nothing to change
                break;

            case 0xCC: this->draw_MemBlt(rect, bitmap, cmd.srcx + (rect.x - cmd.rect.x), cmd.srcy + (rect.y - cmd.rect.y), NO_MASK);
                break;

            case 0xEE: this->draw_MemBlt(rect, bitmap, cmd.srcx + (rect.x - cmd.rect.x), cmd.srcy + (rect.y - cmd.rect.y), NO_MASK);
                break;

            case 0xFF: {SDL_Rect trect = {rect.y, rect.x, rect.cx, rect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, 0xffffffff);}
                break;

            default: std::cout << "RDPMemBlt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        const Rect rect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);
        if (rect.isempty()) {
            return;
        }

        switch (cmd.rop) {

            case 0x00: {SDL_Rect trect = {rect.y, rect.x, rect.cx, rect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, 0x000000ff);}
                break;

            case 0x55: this->draw_RDPScrBlt(rect.y, rect.x, rect, INVERT_MASK);
                break;

            case 0xAA: // nothing to change
                break;

            case 0xFF: {SDL_Rect trect = {rect.y, rect.x, rect.cx, rect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, 0xffffffff);}
                break;
            default: std::cout << "RDPScrBlt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        RDPPatBlt new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->_mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->_mod_bpp, this->mod_palette);
        const Rect drect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);

        if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) { // external

            switch (cmd.rop) {

                case 0x5A:
                    {
                        int shift = 0;
                        SDL_LockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
                        for (int y = 0; y < drect.cy; y++) {
                            shift = y%2;
                            for (int x = 0; x < drect.cx; x += 2) {
                                Uint32 & currentPixel = *((Uint32*)this->_browser._screen->pixels + ((y + drect.y) * this->_info.width) + x + shift + drect.x);
                                currentPixel = currentPixel ^ new_cmd24.fore_color;
                            }
                        }
                        for (int y = 0; y < drect.cy; y++) {
                            shift = y%2;
                            for (int x = 1; x < drect.cx; x += 2) {
                                Uint32 & currentPixel = *((Uint32*)this->_browser._screen->pixels + ((y + drect.y) * this->_info.width) + x + shift + drect.x);
                                currentPixel = currentPixel ^ new_cmd24.back_color;
                            }
                        }
                        SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
                    }
                    break;

                case 0xF0:
                    {

                        SDL_Rect trect = {drect.x, drect.y, drect.cx, drect.cy};
                        SDL_FillRect(this->_browser._screen, &trect, SDL_MapRGBA(this->_browser._screen->format,
                                                                                 new_cmd24.back_color >> 16,
                                                                                 new_cmd24.back_color >> 8,
                                                                                 new_cmd24.back_color,
                                                                                 255));

                        SDL_LockSurface(this->_browser._screen);
                        int shift = 0;
                        for (int y = 0; y < drect.cy; y++) {
                            shift = y%2;
                            for (int x = 0; x < drect.cx; x += 2) {
                                *((Uint32*)this->_browser._screen->pixels + ((y + drect.y) * this->_info.width) + x + shift + drect.x) = SDL_MapRGBA(this->_browser._screen->format,
                                            new_cmd24.fore_color >> 16,
                                            new_cmd24.fore_color >> 8,
                                            new_cmd24.fore_color,
                                            255);
                            }
                        }
                        SDL_UnlockSurface(this->_browser._screen);
                    }
                    break;

                default:
                    std::cout << "RDPPatBlt brush_style = 03 " << (int) cmd.rop << std::endl;
                    break;
        }

        } else {
            switch (cmd.rop) {

                case 0x00: {SDL_Rect trect = {drect.y, drect.x, drect.cx, drect.cy};
                            SDL_FillRect(this->_browser._screen, &trect, 0x000000ff);}
                    break;

                case 0x55: // inversion
                    //this->invert_color(rect);
                    break;

                case 0x5A:
                    break;

                case 0xAA: // change nothing
                    break;

                case 0xFF: {SDL_Rect trect = {drect.y, drect.x, drect.cx, drect.cy};
                            SDL_FillRect(this->_browser._screen, &trect, 0xffffffff);}
                    break;

                default:
                    std::cout << "RDPPatBlt " << (int) cmd.rop << std::endl;
                    break;
            }
        }
    }


    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override {
        const Rect rect = clip.intersect(this->_info.width, this->_info.height);
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

        const float dx = x2 - x1;
        const float dy = y2 - y1;

        float err = 0.0;
        const float e1    = dy/dx;

        const int ystep = (y1 < y2) ? 1 : -1;
        int y = (int)y1;

        const int xMin(rect.x);
        const int yMin(rect.y);
        const int xMax(rect.cx + rect.x);
        const int yMax(rect.cy + rect.y);

        SDL_LockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
        for(int x = 0; x < this->abs(dx); x++)
        {
            err += e1;
            if (x >= xMin && x < xMax && y >= yMin && y < yMax) {
                *((Uint32*)this->_browser._screen->pixels + (y * this->_info.width) + x) = SDL_MapRGBA(this->_browser._screen->format, r, g, b, 255);
            }

            if(err >= 0.5)
            {
                err -= 1;
                y += ystep;
            }
        }
        SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
    }


    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        const +Rect drect = clip.intersect(cmd.rect);
        if (drect.isempty() || bitmap.cx() <= 0 || bitmap.cy() <= 0){
            return ;
        }

        switch (cmd.rop) {
            case 0xB8: // TODO
                {
                    Bitmap bitmapBpp(32, bitmap);
                    const uint8_t * bitMapData = bitmapBpp.data();

                    SDL_LockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
                    int yStart(drect.cy + drect.y-1);
                    for (int y = 0; y < drect.cy; y++) {
                        for (int x = 0; x < drect.cx; x++) {
                            int indice(((y * drect.cx) + x) * ((bitmapBpp.bpp()+1)/8));
                            Uint32 & currentPixel = *((Uint32*)this->_browser._screen->pixels + ((yStart - y) * this->_info.width) + x + drect.x);

                            if (this->newPixel(bitMapData, indice) != cmd.back_color) {
                                currentPixel = newPixel(bitMapData, indice);
                            }
                        }
                    }
                    SDL_UnlockSurface(this->_browser._screen); // if (SDL_MUSTLOCK(this->_browser._screen))
                }
                break;

            default: std::cout << "RDPMem3Blt (" << std::hex << (int)cmd.rop << ")" << std::endl;
                break;
        }
    }


    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {

        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                     (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                     (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
        const Rect clipRect(0, 0, this->_info.width, this->_info.height);
        const Rect rect = rectBmp.intersect(clipRect);

        this->draw_bmp(rect, bmp);
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

   Front_JS_SDL(ClientInfo & info,  int verb)
    : FrontAPI(false, false)
    , verbose((uint32_t)verb)
    , _info(info)
    , _port(0)
    , _callback(nullptr)
    , _fps(30)
    , _mod_bpp(this->_info.bpp)
    , mod_palette(BGRPalette::classic_332())
    , _browser(this)
    {
        //this->_to_client_sender._front = this;
    }


    ~Front_JS_SDL() {}



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //------------------------
    //      CONTROLLERS
    //------------------------
/*
    void mousePressEvent(QMouseEvent *e) override {}

    void mouseReleaseEvent(QMouseEvent *e) override {}

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
