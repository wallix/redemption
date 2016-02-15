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

#ifndef FRONT_QT_HPP
#define FRONT_QT_HPP

#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>

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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "core/RDP/pointer.hpp"
#include "front_api.hpp"
#include "channel_list.hpp"
#include "mod_api.hpp"
#include "bitmap.hpp"
#include "RDP/caches/glyphcache.hpp"
#include "RDP/capabilities/glyphcache.hpp"
#include "RDP/bitmapupdate.hpp"
#include "keymap2.hpp"
#include "client_info.hpp"
#include "reversed_keymaps/Qt_ScanCode_KeyMap.hpp"

#include <QtGui/QImage>
 



class Form_Qt;
class Screen_Qt;
class Connector_Qt;


class Front_Qt_API : public FrontAPI
{

private:
    class ClipboardServerChannelDataSender : public VirtualChannelDataSender
    {
    public:
        mod_api        * _callback;

        ClipboardServerChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            std::cout << "operator()  call" << std::endl;
            InStream chunk(chunk_data, chunk_data_length);
            this->_callback->send_to_mod_channel(channel_names::cliprdr, chunk, chunk_data_length, flags);
        }
    };
    
    
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
    QImage::Format    _imageFormatRGB;  
    QImage::Format    _imageFormatARGB;
    ClipboardServerChannelDataSender _to_server_sender;
    Qt_ScanCode_KeyMap   _qtRDPKeymap;  
    int                  _fps;
    
    
    Front_Qt_API( bool param1
                , bool param2
                , int verb)
    : FrontAPI(param1, param2)
    , verbose(verb)
    , _info()
    , _port(0)
    , _callback(nullptr)
    , _qtRDPKeymap()
    , _fps(30)
    {}
    
    
    virtual void connexionPressed() = 0;
    virtual void connexionReleased() = 0;
    virtual void closeFromScreen() = 0;
    virtual void RefreshPressed() = 0;
    virtual void RefreshReleased() = 0;
    virtual void CtrlAltDelPressed() = 0;
    virtual void CtrlAltDelReleased() = 0;
    virtual void disconnexionPressed() = 0;
    virtual void disconnexionReleased() = 0;
    virtual void mousePressEvent(QMouseEvent *e) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *e) = 0;
    virtual void keyPressEvent(QKeyEvent *e) = 0;
    virtual void keyReleaseEvent(QKeyEvent *e) = 0;
    virtual void wheelEvent(QWheelEvent *e) = 0;
    virtual bool eventFilter(QObject *obj, QEvent *e) = 0;
    virtual void call_Draw() = 0;
    virtual void disconnect(std::string txt) = 0;
    virtual QImage::Format bpp_to_QFormat(int bpp, bool alpha) = 0;
    virtual void dropScreen() = 0;
    virtual bool setClientInfo() = 0;
    virtual void writeClientInfo() = 0;
    virtual void send_Cliboard(uint32_t total_length,
                       uint32_t flags, 
                       const uint8_t* chunk_data,
                       uint32_t chunk_data_length) = 0;
};
 
    
    
class Front_Qt : public Front_Qt_API
{    
   
public:
    CHANNELS::ChannelDefArray   cl;
    
    // Graphic members
    uint8_t               mod_bpp;
    BGRPalette            mod_palette;
    Form_Qt            * _form;
    Screen_Qt          * _screen;
    
    // Connexion socket members
    Connector_Qt       * _connector;
    int                  _timer;
    bool                 _connected;
    ClipboardVirtualChannel  _clipboard_channel;

    // Keyboard Controllers members
    Keymap2              _keymap;
    bool                 _ctrl_alt_delete; // currently not used and always false
    StaticOutStream<256> _decoded_data;    // currently not initialised
    uint8_t              _keyboardMods;    

    
    
    enum : int {
        COMMAND_VALID = 15
      , NAME_GOTTEN   =  1
      , PWD_GOTTEN    =  2
      , IP_GOTTEN     =  4
      , PORT_GOTTEN   =  8
    };
    
    bool setClientInfo() override;
    
    void writeClientInfo() override;
  
    virtual void flush() override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "flush()");
            LOG(LOG_INFO, "========================================\n"); 
        }
    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override { return cl; }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }

        const CHANNELS::ChannelDef * mod_channel = this->cl.get_by_name(channel.name);
        if (!mod_channel) {
            return;
        }
        
        if (!strcmp(channel.name, channel_names::cliprdr)) {
            std::unique_ptr<AsynchronousTask> out_asynchronous_task;
            std::cout << channel.name << " send_to_channel" << std::endl;
            this->_clipboard_channel.process_server_message(length, flags, data, chunk_size, out_asynchronous_task);
        }
    }  

    virtual void send_global_palette() override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_global_palette()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void begin_update() override {
        //if (this->verbose > 10) {
        //    LOG(LOG_INFO, "--------- FRONT ------------------------");
        //    LOG(LOG_INFO, "begin_update");
        //    LOG(LOG_INFO, "========================================\n");
        //}
    }

    virtual void end_update() override {
        //if (this->verbose > 10) {
        //    LOG(LOG_INFO, "--------- FRONT ------------------------");
        //    LOG(LOG_INFO, "end_update");
        //    LOG(LOG_INFO, "========================================\n");
        //}
    }

    virtual void set_mod_palette(const BGRPalette & palette) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_mod_palette");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void server_set_pointer(const Pointer & cursor) override;

    virtual int server_resize(int width, int height, int bpp) override;
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

    //---------------------------------------
    //   GRAPHIC FUNCTIONS (factorization)
    //---------------------------------------

    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert);
    
    QColor u32_to_qcolor(uint32_t color);
    
    QImage::Format bpp_to_QFormat(int bpp, bool alpha) override; 
    
    void draw_bmp(const Rect & drect, const Bitmap & bitmap, bool invert);
    
    void draw_MemBlt(const Rect & drect, const Bitmap & bitmap, bool invert, int srcx, int srcy);
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //-----------------------------
    //      DRAWING FUNCTIONS 
    //-----------------------------

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override;

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override;
    
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) override;
    
    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override;
    
    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override;
    
    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data, size_t size, const Bitmap & bmp) override;
    
    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPDestBlt" << std::endl;
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPMultiDstBlt" << std::endl;
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPMultiOpaqueRect" << std::endl;
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPMultiPatBlt" << std::endl;
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPMultiScrBlt" << std::endl;
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPGlyphIndex" << std::endl;

       /* RDPGlyphIndex new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPPolygonSC" << std::endl;

        /*RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPPolygonCB" << std::endl;

        /*RDPPolygonCB new_cmd24 = cmd;
        new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
        new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPPolyline" << std::endl;

        /*RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPEllipseSC" << std::endl;

        /*RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPEllipseCB" << std::endl;
/*
        RDPEllipseCB new_cmd24 = cmd;
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    virtual void draw(const RDP::FrameMarker & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "FrameMarker" << std::endl;
        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "NewOrExistingWindow" << std::endl;
        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "WindowIcon" << std::endl;
        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
        
         std::cout << "CachedIcon" << std::endl;

        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "DeletedWindow" << std::endl;

        //this->gd.draw(order);
    }
    
    virtual void draw(const RDPColCache   & cmd) {}
    
    virtual void draw(const RDPBrushCache & cmd) {}

    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //------------------------
    //      CONSTRUCTOR
    //------------------------
    
    Front_Qt(char* argv[], int argc, uint32_t verbose);
    
    // -------- Start of system wide SSL_Ctx option ------------------------------

    // ERR_load_crypto_strings() registers the error strings for all libcrypto
    // functions. SSL_load_error_strings() does the same, but also registers the
    // libssl error strings.

    // One of these functions should be called before generating textual error
    // messages. However, this is not required when memory usage is an issue.

    // ERR_free_strings() frees all previously loaded error strings.

    //SSL_load_error_strings();

    // SSL_library_init() registers the available SSL/TLS ciphers and digests.
    // OpenSSL_add_ssl_algorithms() and SSLeay_add_ssl_algorithms() are synonyms
    // for SSL_library_init().

    // - SSL_library_init() must be called before any other action takes place.
    // - SSL_library_init() is not reentrant.
    // - SSL_library_init() always returns "1", so it is safe to discard the return
    // value.

    // Note: OpenSSL 0.9.8o and 1.0.0a and later added SHA2 algorithms to
    // SSL_library_init(). Applications which need to use SHA2 in earlier versions
    // of OpenSSL should call OpenSSL_add_all_algorithms() as well.

    //SSL_library_init();
    
    ~Front_Qt();
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //------------------------
    //      CONTROLLERS
    //------------------------
    
    void mousePressEvent(QMouseEvent *e) override {
        if (this->_callback != nullptr) {
            int flag(0); 
            switch (e->button()) {
                case 1: flag = MOUSE_FLAG_BUTTON1; break;
                case 2: flag = MOUSE_FLAG_BUTTON2; break; 
                case 4: flag = MOUSE_FLAG_BUTTON4; break;
                default: break; 
            }
            //std::cout << "mousePressed" << std::endl;
            this->_callback->rdp_input_mouse(flag | MOUSE_FLAG_DOWN, e->x(), e->y(), &(this->_keymap));
        } 
    }
    
    void mouseReleaseEvent(QMouseEvent *e) override {
        if (this->_callback != nullptr) {
            int flag(0); 
            switch (e->button()) {
                case 1: flag = MOUSE_FLAG_BUTTON1; break; 
                case 2: flag = MOUSE_FLAG_BUTTON2; break; 
                case 4: flag = MOUSE_FLAG_BUTTON4; break; 
                default: break;
            }
            //std::cout << "mouseRelease" << std::endl;
            this->_callback->rdp_input_mouse(flag, e->x(), e->y(), &(this->_keymap)); 
        }
    }
    
    void keyPressEvent(QKeyEvent *e) override { 
        this->_qtRDPKeymap.keyEvent(0       ,      e);
        if (this->_qtRDPKeymap.scanCode != 0) {
            this->send_rdp_scanCode(this->_qtRDPKeymap.scanCode, this->_qtRDPKeymap.flag);
        }
    }
    
    void keyReleaseEvent(QKeyEvent *e) override {
        this->_qtRDPKeymap.keyEvent(0x8000, e);
        if (this->_qtRDPKeymap.scanCode != 0) {
            this->send_rdp_scanCode(this->_qtRDPKeymap.scanCode, this->_qtRDPKeymap.flag);
        }
    }
    
    void wheelEvent(QWheelEvent *e) override {
        //std::cout << "wheel " << " delta=" << e->delta() << std::endl;
        int flag(MOUSE_FLAG_HWHEEL);
        if (e->delta() < 0) {
            flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
        }
        if (this->_callback != nullptr) {
            //this->_callback->rdp_input_mouse(flag, e->x(), e->y(), &(this->_keymap));
        }
    }
    
    bool eventFilter(QObject *obj, QEvent *e) override {
        if (e->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
            //std::cout << "MouseMove " <<  mouseEvent->x() << " " <<  mouseEvent->y()<< std::endl;
            if (this->_callback != nullptr) {
                this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, mouseEvent->x(), mouseEvent->y(), &(this->_keymap));  
            }
        }
        return false;
    }
    
    void connexionPressed() override {}
    
    void connexionReleased() override;

    void RefreshPressed() override {
        this->refresh(0, 0, this->_info.width, this->_info.height);
    }
    
    void RefreshReleased() override {}
    
    void CtrlAltDelPressed() override {
        int flag = Keymap2::KBDFLAGS_EXTENDED;

        this->send_rdp_scanCode(0x38, flag);  // ALT
        this->send_rdp_scanCode(0x1D, flag);  // CTRL
        this->send_rdp_scanCode(0x53, flag);  // DELETE       
    }
    
    void CtrlAltDelReleased() override {
        int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;
        
        this->send_rdp_scanCode(0x38, flag);  // ALT
        this->send_rdp_scanCode(0x1D, flag);  // CTRL
        this->send_rdp_scanCode(0x53, flag);  // DELETE  
    }
    
    void disconnexionPressed()  override {}
    
    void disconnexionReleased() override;
 
    void refresh(int x, int y, int w, int h) {
        Rect rect(x, y, w, h);
        this->_callback->rdp_input_invalidate(rect);
    }
    
    void send_rdp_scanCode(int keyCode, int flag) {
        this->_keymap.event(flag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        if (this->_callback != nullptr) {
            this->_callback->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->_keymap)); 
        }
    }
    
    void connect();
    
    void disconnect(std::string) override;
    
    void closeFromScreen() override;
    
    void dropScreen() override;
        
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------
    
    void call_Draw() override {
        if (this->_callback != nullptr) {
            try {
                this->_callback->draw_event(time(nullptr));
            } catch (const Error & e) {
                this->dropScreen();
                const std::string errorMsg("Error: connexion to [" + this->_targetIP +  "] is closed.");
                std::cout << errorMsg << std::endl;
                std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
                
                this->disconnect(labelErrorMsg);
            }
        }
    }
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //--------------------------------
    //   CLIPBOARD EVENTS FUNCTIONS
    //--------------------------------
    
    void send_Cliboard(uint32_t total_length,
                       uint32_t flags, 
                       const uint8_t* chunk_data,
                       uint32_t chunk_data_length) override {
        
                        //RDPECLIP::FormatDataRequestPDU().recv(stream, recv_factory);
            //    this->send_to_front_channel_and_set_buf_size(
            //        this->clipboard_str_.size() * 2 /*utf8 to utf16*/ + sizeof(RDPECLIP::CliprdrHeader) + 4 /*data_len*/,
            //        RDPECLIP::FormatDataResponsePDU(true), this->clipboard_str_.c_str()
            //    );

        this->_clipboard_channel.process_client_message(total_length, flags, chunk_data, chunk_data_length);
    }
    
};


  

#endif
