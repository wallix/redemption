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

#define QT4
//#define QT5

#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>

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
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "core/RDP/pointer.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "mod/mod_api.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "keyboard/keymap2.hpp"
#include "core/client_info.hpp"
#include "keymaps/Qt_ScanCode_KeyMap.hpp"

#ifdef QT5
#include </usr/include/x86_64-linux-gnu/qt5/QtGui/QImage>
#endif
#ifdef QT4
#include <QtGui/QImage>
#endif


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
            //std::cout << "operator()  server " << (int)flags  << std::endl;
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
            //std::cout << "operator()  client " << (int)flags  << std::endl;

            this->_front->send_to_channel(this->_channel, chunk_data, total_length, chunk_data_length, flags);
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
    ClipboardClientChannelDataSender _to_client_sender;
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
    {
        this->_to_client_sender._front = this;
    }


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
    virtual void send_FormatListPDU(uint32_t const * formatIDs, std::string const * formatListDataShortName, std::size_t formatIDs_size) = 0;
    virtual void empty_buffer() = 0;
};



class Front_Qt : public Front_Qt_API
{

public:

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
    CHANNELS::ChannelDefArray   _cl;
    uint32_t             _requestedFormatId = 0;
    std::string          _requestedFormatShortName;
    uint8_t            * _bufferRDPClipboardChannel;
    size_t               _bufferRDPClipboardChannelSize;
    size_t               _bufferRDPClipboardChannelSizeTotal;
    int                  _bufferRDPCLipboardMetaFilePic_width;
    int                  _bufferRDPCLipboardMetaFilePic_height;
    int                  _bufferRDPClipboardMetaFilePicBPP;


    enum : int {
        COMMAND_VALID = 15
      , NAME_GOTTEN   =  1
      , PWD_GOTTEN    =  2
      , IP_GOTTEN     =  4
      , PORT_GOTTEN   =  8
    };



    //bool setClientInfo() override;

    //void writeClientInfo() override;

    //virtual void flush() override;

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override;

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override;

    //virtual void send_global_palette() override;

    virtual void begin_update() override;

    virtual void end_update() override;

    virtual void set_mod_palette(const BGRPalette & palette) override;

    virtual void server_set_pointer(const Pointer & cursor) override;

    virtual int server_resize(int width, int height, int bpp) override;

    void send_buffer_to_clipboard();

    void process_server_clipboard_data(int flags, InStream & chunk);

    void send_FormatListPDU(const uint32_t * formatIDs, const std::string * formatListDataShortName, std::size_t formatIDs_size) override;

    std::string HTMLtoText(const std::string & html);

    void send_to_clipboard_Buffer(InStream & chunk);

    void send_textBuffer_to_clipboard(bool isTextHtml);

    void send_imageBuffer_to_clipboard();

    void empty_buffer() override;



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
    //       DRAW FUNCTIONS
    //-----------------------------

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override;

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) override;

    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override;

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override;

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override;

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override;

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override;

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override;

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override;

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override;

    void draw(const RDPPolyline & cmd, const Rect & clip) override;

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) override;

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) override;

    virtual void draw(const RDP::FrameMarker & order) override;

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) override;

    virtual void draw(const RDP::RAIL::WindowIcon & order) override;

    virtual void draw(const RDP::RAIL::CachedIcon & order) override;

    virtual void draw(const RDP::RAIL::DeletedWindow & order) override;

    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override;

    virtual void draw(const RDP::RAIL::DeletedNotificationIcons & order) override;

    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override;

    virtual void draw(const RDP::RAIL::NonMonitoredDesktop & order) override;

    virtual void draw(const RDPColCache   & cmd);

    virtual void draw(const RDPBrushCache & cmd);



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

    void mousePressEvent(QMouseEvent *e) override;

    void mouseReleaseEvent(QMouseEvent *e) override;

    void keyPressEvent(QKeyEvent *e) override;

    void keyReleaseEvent(QKeyEvent *e) override;

    void wheelEvent(QWheelEvent *e) override;

    bool eventFilter(QObject *obj, QEvent *e) override;

    void connexionPressed() override;

    void connexionReleased() override;

    void RefreshPressed() override;

    void RefreshReleased() override;

    void CtrlAltDelPressed() override;

    void CtrlAltDelReleased() override;

    void disconnexionPressed()  override;

    void disconnexionReleased() override;

    void refresh(int x, int y, int w, int h);

    void send_rdp_scanCode(int keyCode, int flag);

    void connect();

    void disconnect(std::string) override;

    void closeFromScreen() override;

    void dropScreen() override;



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    void call_Draw() override;


};




