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

   Fake Front class for Unit Testing
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

#include "log.hpp"
#include "core/front_api.hpp"
#include "channel_list.hpp"
#include "client_info.hpp"
#include "mod_api.hpp"
#include "bitmap.hpp"
#include "keymap2.hpp"
#include "RDP/caches/glyphcache.hpp"
#include "RDP/capabilities/glyphcache.hpp"
#include "RDP/bitmapupdate.hpp"


#include <QtGui/QWidget>
#include <QtGui/QPicture>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <QtCore/QSocketNotifier>
#include <QtGui/QPushButton>


class Front_Qt : public QWidget, public FrontAPI
{
    
Q_OBJECT
    
    
public:
    uint32_t                    verbose;
    ClientInfo                & info;
    CHANNELS::ChannelDefArray   cl;
    
    // Graphic members
    uint8_t               mod_bpp;
    BGRPalette            mod_palette;
    bool                  notimestamp;
    bool                  nomouse;
    QLabel               _label;
    QPicture             _picture;
    int                  _width;
    int                  _height;
    QPen                 _pen;
    QPainter             _painter;
    
    // Connexion socket members
    QSocketNotifier      _sckRead;
    mod_api*             _callback;
    
    // Controllers members
    Keymap2                        _keymap;
    const Keylayout::KeyLayout_t * _layout;
    bool                           _ctrl_alt_delete;
    StaticOutStream<256>           _decoded_data;
    uint8_t                        _keyboardMods;
    int                            _timer;
    QPushButton                    _button;
    
    enum {
          CTRL_MOD     = 0x08
        , CAPSLOCK_MOD = 0x04
        , ALT_MOD      = 0x02
        , SHIFT_MOD    = 0x01
    };
    
    
    QColor u32_to_qcolor(uint32_t color){
        uint8_t b(color >> 16);
        uint8_t g(color >> 8);
        uint8_t r(color);
        return {r, g, b};
    }
    
    void reInitView() {
        this->_painter.begin(&(this->_picture));
        this->_painter.fillRect(0, 0, this->_width, this->_height, QColor(0, 0, 0, 0));
    }
    
    virtual void flush() override {
        if (this->verbose > 10) {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "flush()");
             LOG(LOG_INFO, "========================================\n");
        }
        this->_painter.end();
        this->_label.setPicture(this->_picture);
        this->show();
    }
    
    void refresh() {
        Rect rect(0, 0, this->_width, this->_height);
        this->_callback->rdp_input_invalidate(rect);
    }
    
    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override { return cl; }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length
                                , size_t chunk_size, int flags) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
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

    virtual void server_set_pointer(const Pointer & cursor) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }

        //this->gd.server_set_pointer(cursor);
    }

    virtual int server_resize(int width, int height, int bpp) override {
        this->mod_bpp = bpp;
        this->info.bpp = bpp;
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return 1;
    }
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //-------------------------------
    //      DRAWING FUNCTIONS 
    //------------------------------

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        //std::cout << "RDPOpaqueRect" << std::endl;

        RDPOpaqueRect new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        
        Rect rect(new_cmd24.rect.intersect(clip));
        this->_painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, this->u32_to_qcolor(new_cmd24.color));
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPScrBlt" << std::endl;
    }

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

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        //std::cout << "RDPPatBlt" << std::endl;

        /*RDPPatBlt new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPMemBlt" << std::endl;
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        std::cout << "RDPMem3Blt" << std::endl;
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        
        std::cout << "RDPLineTo" << std::endl;
        
        RDPLineTo new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.pen.color  = color_decode_opaquerect(cmd.pen.color,  this->mod_bpp, this->mod_palette);

        
        // TO DO clipping
        this->_pen.setBrush(this->u32_to_qcolor(new_cmd24.back_color));
        this->_painter.drawLine(new_cmd24.startx, new_cmd24.starty, new_cmd24.endx, new_cmd24.endy);
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

        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        //this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        //this->gd.draw(order);
    }
    
    virtual void draw(const RDPColCache   & cmd) {}
    
    virtual void draw(const RDPBrushCache & cmd) {}
    
    
    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
        size_t size, const Bitmap & bmp) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            bitmap_data.log(LOG_INFO, "FakeFront");
            LOG(LOG_INFO, "========================================\n");
        }
        
        //std::cout << "RDPBitmapData" << std::endl;
        if (!bmp.is_valid()){
            return;
        }

        const QRect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top, 
                             (bitmap_data.dest_right - bitmap_data.dest_left + 1), 
                             (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
        const QRect clipRect(0, 0, this->_width, this->_height);
        const QRect rect = rectBmp.intersected(clipRect);
            
        const int16_t mincx = std::min<int16_t>(bmp.cx(), std::min<int16_t>(this->_width - rect.x(), rect.width()));
        const int16_t mincy = 1;

        if (mincx <= 0 || mincy <= 0) {
            return;
        }        
        
        int rowYCoord(rect.y() + rect.height()-1);
        int rowsize(bmp.line_size()); //Bpp
      
        const uint8_t * row = bmp.data();
        
        QImage::Format format; //bpp
        if (bmp.bpp() == 16){
            format = QImage::Format_RGB16;
        }
        if (bmp.bpp() == 24){
            format = QImage::Format_RGB888;
        }
        if (bmp.bpp() == 32){
            format = QImage::Format_RGB32;
        }
        if (bmp.bpp() == 15){
            format = QImage::Format_RGB555;
        }
        
        for (size_t k = 0 ; k < bitmap_data.height; k++) {
            
            QImage qbitmap(const_cast<unsigned char*>(row), mincx, mincy, format);
            const QRect trect(rect.x(), rowYCoord, mincx, mincy);
            this->_painter.drawImage(trect, qbitmap);

            row += rowsize;
            rowYCoord--;
        }
    }
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //------------------------
    //      CONSTRUCTOR
    //------------------------
 
    Front_Qt(ClientInfo & info, uint32_t verbose, int client_sck)
    : QWidget(), FrontAPI(false, false)
    , verbose(verbose)
    , info(info)
    , mod_bpp(info.bpp)
    , mod_palette(BGRPalette::no_init())
    , notimestamp(true)
    , nomouse(true) 
    , _label(this)
    , _picture()
    , _width(info.width)
    , _height(info.height)
    , _pen() 
    , _painter()
    , _sckRead(client_sck, QSocketNotifier::Read, this)
    , _keymap() 
    , _ctrl_alt_delete(false)
    , _keyboardMods(0) 
    , _timer(0)
    , _button("CTRL + ALT + DELETE", this) {
        if (this->mod_bpp == 8) {
            this->mod_palette = BGRPalette::classic_332();
        }
        
        _keymap.init_layout(info.keylayout);
        this->_layout = &(this->_keymap.keylayout_WORK->noMod);
            
        this->setFixedSize(this->_width, this->_height+20);
            
        QSize size(sizeHint());
        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (size.width()/2);
        int centerH = (desktop->height()/2) - (size.height()/2);
        this->move(centerW, centerH);
            
        this->_label.setMouseTracking(true);
        this->_label.installEventFilter(this);
            
        this->_painter.setRenderHint(QPainter::Antialiasing);
        this->_pen.setWidth(1);
        this->_painter.setPen(this->_pen);
            
        this->setAttribute(Qt::WA_NoSystemBackground);
        
        this->_painter.fillRect(0, 0, this->_width, this->_height, Qt::white);
        //this->_painter.fillRect(0, this->_height, this->_width, 2, Qt::red);
        
        _button.setToolTip("CTRL + ALT + DELETE"); 
        _button.setGeometry(QRect(QPoint(0, this->_height+1),QSize(this->_width, 20)));
        QObject::connect(&_button, SIGNAL (pressed()),  this, SLOT (CtrlAltDelOn()));
        QObject::connect(&_button, SIGNAL (released()), this, SLOT (CtrlAltDelOut()));
        _button.show();


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
    }
    
    ~Front_Qt() {}
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //------------------------
    //      CONTROLLERS
    //------------------------
    
    void layout_Work_Update() {
        //                                ___________________________
        //                               |      |      |      |      |
        //   bitcode for _keyboardMods:  | CTRL | CAPS | ALT  | SHFT |
        //                               |______|______|______|______|
        //
        if ((this->_keyboardMods & CTRL_MOD) == CTRL_MOD) {
            this->_layout = &(this->_keymap.keylayout_WORK->ctrl);
        } else {
            switch (this->_keyboardMods) {
                
                case 0 : this->_layout = &(this->_keymap.keylayout_WORK->noMod);               break;
                case 1 : this->_layout = &(this->_keymap.keylayout_WORK->shift);               break;
                case 2 : this->_layout = &(this->_keymap.keylayout_WORK->altGr);               break;
                case 3 : this->_layout = &(this->_keymap.keylayout_WORK->shiftAltGr);          break;
                case 4 : this->_layout = &(this->_keymap.keylayout_WORK->capslock_noMod);      break;
                case 5 : this->_layout = &(this->_keymap.keylayout_WORK->capslock_shift);      break;
                case 6 : this->_layout = &(this->_keymap.keylayout_WORK->capslock_altGr);      break;
                case 7 : this->_layout = &(this->_keymap.keylayout_WORK->capslock_shiftAltGr); break;
                
                default: break;
            }   
        }
    }
    
    void keyQtEvent(int keyStatusFlag, QKeyEvent *e) { 
        int keyboardFlag(0);
        int keyCode(e->key()); 
        bool unrecognised(false);
        
        if (keyCode != 0) {
            
            if (keyCode < 256 && keyCode > 0 || keyCode == 338) {
                
                const Keylayout::KeyLayout_t &layout = *(this->_layout);
                
                if ((this->_keyboardMods & CTRL_MOD) == CTRL_MOD) {
                    
                    //---------------------
                    //      ShortCuts
                    //---------------------
                    this->_layout = &(this->_keymap.keylayout_WORK->noMod);

                    switch (keyCode) {
                        case Qt::Key_A         : keyCode = 'a'; break; // CTRL + A
                        case Qt::Key_Z         : keyCode = 'z'; break; // CTRL + Z
                        case Qt::Key_Y         : keyCode = 'y'; break; // CTRL + Y
                        case Qt::Key_X         : keyCode = 'x'; break; // CTRL + X
                        case Qt::Key_C         : keyCode = 'c'; break; // CTRL + C
                        case Qt::Key_V         : keyCode = 'v'; break; // CTRL + V
                        
                        case Qt::Key_E         : keyCode = 'e'; break; // CTRL + E
                        case Qt::Key_R         : keyCode = 'r'; break; // CTRL + R
                        case Qt::Key_T         : keyCode = 't'; break; // CTRL + T
                        case Qt::Key_U         : keyCode = 'u'; break; // CTRL + U
                        case Qt::Key_I         : keyCode = 'i'; break; // CTRL + I
                        case Qt::Key_B         : keyCode = 'b'; break; // CTRL + B
                        case Qt::Key_D         : keyCode = 'd'; break; // CTRL + D
                        case Qt::Key_F         : keyCode = 'f'; break; // CTRL + F
                        case Qt::Key_G         : keyCode = 'g'; break; // CTRL + G
                        case Qt::Key_H         : keyCode = 'h'; break; // CTRL + H
                        case Qt::Key_J         : keyCode = 'j'; break; // CTRL + J
                        case Qt::Key_K         : keyCode = 'k'; break; // CTRL + K
                        case Qt::Key_L         : keyCode = 'l'; break; // CTRL + L
                        case Qt::Key_M         : keyCode = 'm'; break; // CTRL + M
                        case Qt::Key_N         : keyCode = 'n'; break; // CTRL + N
                        case Qt::Key_O         : keyCode = 'o'; break; // CTRL + O
                        case Qt::Key_P         : keyCode = 'p'; break; // CTRL + P
                        case Qt::Key_Q         : keyCode = 'q'; break; // CTRL + Q
                        case Qt::Key_S         : keyCode = 's'; break; // CTRL + S
                        case Qt::Key_W         : keyCode = 'w'; break; // CTRL + W
                        
                        default: this->_layout = &(this->_keymap.keylayout_WORK->ctrl); break;
                    }
                    
                } else {
                    
                    //-------------------
                    //    Characters
                    //-------------------
                    switch (keyCode) { 
                        case Qt::Key_Eacute   : keyCode = 0xE9; break; //  é
                        case Qt::Key_Ccedilla : keyCode = 0xE7; break; //  ç
                        case Qt::Key_Agrave   : keyCode = 0xE0; break; //  à
                        case Qt::Key_Ugrave   : keyCode = 0xF9; break; //  ù
                        case 338              : keyCode = 0xB2; break; //  œ / square
                        case Qt::Key_Egrave   : keyCode = 0xE8; break; //  è
                        case Qt::Key_section  : keyCode = 0xA7; break; //  §
                        case Qt::Key_Slash    :
                            if (this->_keyboardMods == 0) {
                                keyCode = 0x21;
                                keyboardFlag = Keymap2::KBDFLAGS_EXTENDED;
                            }
                            break;
                            
                        default: keyCode = e->text().toStdString()[0]; break;
                    }
                }
                

                // to reverse 
                int i(0);
                if (this->_layout != nullptr) {
                    const Keylayout::KeyLayout_t & layout = *(this->_layout);
                    for (; i < 128 && layout[i] != keyCode; i++) {}
                }
                keyCode = i;
                // 
                
                
                if ((this->_keyboardMods & CTRL_MOD) == CTRL_MOD) {
                    this->_layout = &(this->_keymap.keylayout_WORK->ctrl);
                }     
                
            } else {
                
                
                switch (keyCode) {
                    
                    //----------------
                    //  keyboard mod
                    //----------------
                    case Qt::Key_Alt : keyCode = 0x38;         //  L ALT
                        keyboardFlag = Keymap2::KBDFLAGS_EXTENDED;
                        if (keyStatusFlag == 0) {
                            this->_keyboardMods += ALT_MOD;
                        } else {
                            this->_keyboardMods -= ALT_MOD;
                        }
                        this->layout_Work_Update();
                        break; 
                        
                    case Qt::Key_AltGr : keyCode = 0x38;       //  R ALT GR
                        keyboardFlag = Keymap2::KBDFLAGS_EXTENDED;
                        if (keyStatusFlag == 0) {
                            this->_keyboardMods += ALT_MOD;
                        } else {
                            this->_keyboardMods -= ALT_MOD;
                        }
                        this->layout_Work_Update();
                        break; 
                        
                    case Qt::Key_Control : keyCode = 0x1D;     //  R L CTRL
                        keyboardFlag = Keymap2::KBDFLAGS_EXTENDED;
                        if (keyStatusFlag == 0) {
                            this->_keyboardMods += CTRL_MOD;
                        } else {
                            this->_keyboardMods -= CTRL_MOD;
                        }
                        this->layout_Work_Update();
                        break; 
                        
                    case Qt::Key_Shift : keyCode = 0x36;       // R L SHFT
                        if (keyStatusFlag == 0) {
                            this->_keyboardMods += SHIFT_MOD;
                        } else {
                            this->_keyboardMods -= SHIFT_MOD;
                        }
                        this->layout_Work_Update();
                        break; 
                        
                    case Qt::Key_CapsLock : keyCode = 0x3A;    //  CAPSLOCK 
                        if (keyStatusFlag == 0) {
                            if ((this->_keyboardMods & CAPSLOCK_MOD) == CAPSLOCK_MOD) {
                                this->_keyboardMods -= CAPSLOCK_MOD;
                            } else {
                                this->_keyboardMods += CAPSLOCK_MOD;
                            }
                            this->layout_Work_Update();
                        }                 
                        break;
                     
                    //-----------------------------
                    //  Not mod neither char keys
                    //-----------------------------
                    case Qt::Key_Enter      : keyCode = 0x1C; break; //  ENTER KP
                    case Qt::Key_Return     : keyCode = 0x1C; break; //  ENTER
                    case Qt::Key_Backspace  : keyCode = 0x0E; break; //  BKSP
                    case Qt::Key_Escape     : keyCode = 0x01; break; //  ESCAPE
                    case Qt::Key_F1         : keyCode = 0x3B; break; //  F1
                    case Qt::Key_F2         : keyCode = 0x3C; break; //  F2
                    case Qt::Key_F3         : keyCode = 0x3D; break; //  F3
                    case Qt::Key_F4         : keyCode = 0x3E; break; //  F4
                    case Qt::Key_F5         : keyCode = 0x3F; break; //  F5
                    case Qt::Key_F6         : keyCode = 0x40; break; //  F6
                    case Qt::Key_F7         : keyCode = 0x41; break; //  F7
                    case Qt::Key_F8         : keyCode = 0x42; break; //  F8
                    case Qt::Key_F9         : keyCode = 0x43; break; //  F9
                    case Qt::Key_F10        : keyCode = 0x44; break; //  F10
                    case Qt::Key_F11        : keyCode = 0x57; break; //  F11
                    case Qt::Key_F12        : keyCode = 0x58; break; //  F12
                    case Qt::Key_ScrollLock : keyCode = 0x46; break; //  SCROLL 
                    case Qt::Key_Pause      : keyCode = 0xE1; break; //  PAUSE
                    case Qt::Key_Tab        : keyCode = 0x0F; break; //  TAB
                     
                    case Qt::Key_Home       : keyCode = 0x47; break; //  HOME
                    
                    // DeadKeys
                    case Qt::Key_Dead_Circumflex: keyCode = 0x1a; break; //  ^ ¨
                    case Qt::Key_Dead_Grave     : keyCode = 0x08; break; //  ` grave accent
                    
                    // Extended keyboard
                    case Qt::Key_NumLock    : keyCode = 0x45; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  NUMLOCK
                    case Qt::Key_Insert     : keyCode = 0x52; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  INSERT
                    case Qt::Key_Delete     : keyCode = 0x53; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  DELETE               
                    case Qt::Key_End        : keyCode = 0x4F; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  END
                    case Qt::Key_PageDown   : keyCode = 0x51; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  PG DN
                    case Qt::Key_PageUp     : keyCode = 0x49; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  PG UP
                    case Qt::Key_Up         : keyCode = 0x48; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  U ARROW
                    case Qt::Key_Left       : keyCode = 0x4B; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  L ARROW
                    case Qt::Key_Down       : keyCode = 0x50; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  D ARROW
                    case Qt::Key_Right      : keyCode = 0x4D; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  R ARROW
                    case Qt::Key_Meta       : keyCode = 0x5c; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  R WINDOW
                    case Qt::Key_Menu       : keyCode = 0x5D; keyboardFlag = Keymap2::KBDFLAGS_EXTENDED; break; //  MENU APPS
                    
                    //case 0  : keyCode = 0xE0; break; //  PRNT_SCRN  
                    //case 0  : keyCode = 0xE0; break; //  R GUI
                    //case 0  : keyCode = 0xE0; break; //  L GUI
                    //case 0  : keyCode = 0xE0; break; //  L WINDOW
                    
                    default: 
                        unrecognised = true;
                        break;
                }
            } 
            
            if (!unrecognised) {
                    this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
                    this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap)); 
            }
            std::cout << "keyPressed " << e->key() << " " << keyCode << std::endl;
            
        }
    }
    
    void mousePressEvent(QMouseEvent *e) {
        int flag(0); 
        switch (e->button()) {
            case 1: flag = MOUSE_FLAG_BUTTON1; break;
            case 2: flag = MOUSE_FLAG_BUTTON2; break; 
            case 4: flag = MOUSE_FLAG_BUTTON4; break;
            default: break;
        }
        this->_callback->rdp_input_mouse(flag | MOUSE_FLAG_DOWN, e->x(), e->y(), &(this->_keymap));
    }
    
    void mouseReleaseEvent(QMouseEvent *e) {
        int flag(0); 
        switch (e->button()) {
            case 1: flag = MOUSE_FLAG_BUTTON1; break; 
            case 2: flag = MOUSE_FLAG_BUTTON2; break; 
            case 4: flag = MOUSE_FLAG_BUTTON4; break; 
            default: break;
        }
        this->_callback->rdp_input_mouse(flag, e->x(), e->y(), &(this->_keymap));
        //this->refresh(); 
    }
    
    void keyPressEvent(QKeyEvent *e) { 
        this->keyQtEvent(0x0000,      e);
    }
    
    void keyReleaseEvent(QKeyEvent *e) {
        this->keyQtEvent(KBD_FLAG_UP, e);
    }
    
    void wheelEvent(QWheelEvent *e) {
        std::cout << "wheel " << " delta=" << e->delta() << std::endl;
    }
    
    bool eventFilter(QObject *obj, QEvent *e)
    {
        if (e->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
            this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, mouseEvent->x(), mouseEvent->y(), &(this->_keymap));
        }
        return false;
    }
    
    
public Q_SLOTS:
    void CtrlAltDelOn() {
        int keyboardFlag  = Keymap2::KBDFLAGS_EXTENDED;
        int keyStatusFlag = 0;
        
        int keyCode = 0x38;  // ALT
        this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap));
        
        keyCode     = 0x1D;  // CTRL
        this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap));
        
        keyCode     = 0x53;  // DELETE
        this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap));
                    
    }
    
    void CtrlAltDelOut() {
        int keyboardFlag  = Keymap2::KBDFLAGS_EXTENDED;
        int keyStatusFlag = KBD_FLAG_UP;
        
        int keyCode = 0x38;  // ALT
        this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap));
        
        keyCode     = 0x1D;  // CTRL
        this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap));
        
        keyCode     = 0x53;  // DELETE
        this->_keymap.event(keyStatusFlag | keyboardFlag, keyCode, this->_decoded_data, this->_ctrl_alt_delete); 
        this->_callback->rdp_input_scancode(keyCode, 0, keyStatusFlag | keyboardFlag, _timer, &(this->_keymap));
    }
    

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //-----------------------------------------
    //    SOCKET EVENTS LISTENING FUNCTIONS
    //-----------------------------------------
     
    void readSck_And_ShowView() {
        if (this->_callback != nullptr) {
            this->reInitView();
            this->_callback->draw_event(time(nullptr));
            this->flush();
        }
    }
    
    
public:
    void setCallback_And_StartListening(mod_api* callback) {
        this->_callback = callback;
        QObject::connect(&(this->_sckRead), SIGNAL(activated(int)), this, SLOT(readSck_And_ShowView()));
    }
};


#endif
