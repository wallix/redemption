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

//#define LOGPRINT

#ifndef Q_MOC_RUN
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
#include "core/RDP/clipboard.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "mod/mod_api.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "keymap2.hpp"
#include "core/client_info.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#include "keymaps/Qt4_ScanCode_KeyMap.hpp"
#include <QtGui/QImage>
#pragma GCC diagnostic pop
#endif


#define USER_CONF_PATH "userConfig.config"
#define TEMP_PATH_TEST "/home/cmoroldo/Bureau/redemption/projects/QT4Client/clipboard_temp/"

class Form_Qt;
class Screen_Qt;
class Mod_Qt;
class ClipBoard_Qt;


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

    enum : int {
        BUTTON_HEIGHT = 20
    };

    enum : int {
        MAX_MONITOR_COUNT = GCC::UserData::CSMonitor::MAX_MONITOR_COUNT / 4
    };

    uint32_t          verbose;
    ClientInfo        _info;
    int               _width;
    int               _height;
    std::string       _userName;
    std::string       _pwd;
    std::string       _targetIP;
    int               _port;
    std::string       _localIP;
    int               _nbTry;
    int               _retryDelay;
    mod_rdp         * _callback;
    QImage::Format    _imageFormatRGB;
    QImage::Format    _imageFormatARGB;
    ClipboardServerChannelDataSender _to_server_sender;
    ClipboardClientChannelDataSender _to_client_sender;
    Qt_ScanCode_KeyMap   _qtRDPKeymap;
    int                  _fps;
    int                  _monitorCount;
    const std::string    CB_TEMP_DIR;
    QPixmap            * _cache;
    bool                 _span;
    Rect                 _screen_dimensions[MAX_MONITOR_COUNT];


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
    , _monitorCount(1)
    , CB_TEMP_DIR(TEMP_PATH_TEST)
    , _cache(nullptr)
    , _span(false)
    {
        this->_to_client_sender._front = this;
    }

    virtual Screen_Qt * getMainScreen() = 0;
    virtual void connexionPressed() = 0;
    virtual bool connexionReleased() = 0;
    virtual void closeFromScreen(int screen_index) = 0;
    virtual void RefreshPressed() = 0;
    virtual void RefreshReleased() = 0;
    virtual void CtrlAltDelPressed() = 0;
    virtual void CtrlAltDelReleased() = 0;
    virtual void disconnexionPressed() = 0;
    virtual void disconnexionReleased() = 0;
    virtual void setMainScreenOnTopRelease() = 0;
    virtual void mousePressEvent(QMouseEvent *e, int screen_index) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *e, int screen_index) = 0;
    virtual void keyPressEvent(QKeyEvent *e) = 0;
    virtual void keyReleaseEvent(QKeyEvent *e) = 0;
    virtual void wheelEvent(QWheelEvent *e) = 0;
    virtual bool eventFilter(QObject *obj, QEvent *e, int screen_index) = 0;
    virtual void call_Draw() = 0;
    virtual void disconnect(std::string const & txt) = 0;
    virtual QImage::Format bpp_to_QFormat(int bpp, bool alpha) = 0;
    virtual void dropScreen() = 0;
    virtual bool setClientInfo() = 0;
    virtual void writeClientInfo() = 0;
    virtual void send_FormatListPDU(uint32_t const * formatIDs, std::string const * formatListDataShortName, std::size_t formatIDs_size, bool) = 0;
    virtual void empty_buffer() = 0;
    virtual bool can_be_start_capture(auth_api *) override { return true; }
    virtual bool can_be_pause_capture() override { return true; }
    virtual bool can_be_resume_capture() override { return true; }
    virtual bool must_be_stop_capture() override { return true; }
    virtual void emptyLocalBuffer() = 0;
};



class Front_Qt : public Front_Qt_API
{

public:
    enum : int {
        COMMAND_VALID = 15
      , NAME_GOTTEN   = 1
      , PWD_GOTTEN    = 2
      , IP_GOTTEN     = 4
      , PORT_GOTTEN   = 8
    };

    enum : int {
        PDU_MAX_SIZE    = 1600
      , PDU_HEADER_SIZE =    8
    };

    enum : int {
        PASTE_TEXT_CONTENT_SIZE = PDU_MAX_SIZE - PDU_HEADER_SIZE
      , PASTE_PIC_CONTENT_SIZE  = PDU_MAX_SIZE - RDPECLIP::METAFILE_HEADERS_SIZE - PDU_HEADER_SIZE
    };


    // Graphic members
    uint8_t               mod_bpp;
    BGRPalette            mod_palette;
    Form_Qt            * _form;
    Screen_Qt          * _screen[MAX_MONITOR_COUNT] {};

    // Connexion socket members
    Mod_Qt             * _mod_qt;
    ClipBoard_Qt       * _clipboard_qt;
    int                  _timer;
    bool                 _connected;
    bool                 _monitorCountNegociated;
    ClipboardVirtualChannel  _clipboard_channel;

    // Keyboard Controllers members
    Keymap2              _keymap;
    bool                 _ctrl_alt_delete; // currently not used and always false
    StaticOutStream<256> _decoded_data;    // currently not initialised
    uint8_t              _keyboardMods;
    CHANNELS::ChannelDefArray   _cl;

    //  Clipboard Channel Management members
    uint32_t                    _requestedFormatId = 0;
    std::string                 _requestedFormatName;
    std::unique_ptr<uint8_t[]>  _bufferRDPClipboardChannel;
    size_t                      _bufferRDPClipboardChannelSize;
    size_t                      _bufferRDPClipboardChannelSizeTotal;
    int                         _bufferRDPCLipboardMetaFilePic_width;
    int                         _bufferRDPCLipboardMetaFilePic_height;
    int                         _bufferRDPClipboardMetaFilePicBPP;
    struct Clipbrd_formats_list{
        enum : uint16_t {
              CF_QT_CLIENT_FILEGROUPDESCRIPTORW = 48025
            , CF_QT_CLIENT_FILECONTENTS         = 48026
        };
        enum : int {
              CLIPBRD_FORMAT_COUNT = 5
        };

        const std::string FILECONTENTS;
        const std::string FILEGROUPDESCRIPTORW;
        uint32_t          IDs[CLIPBRD_FORMAT_COUNT];
        std::string       names[CLIPBRD_FORMAT_COUNT];
        int index = 0;
        const double      ARBITRARY_SCALE;  //  module MetaFilePic resolution, value=40 is
                                            //  empirically close to original resolution.

        Clipbrd_formats_list()
          : FILECONTENTS(
              "F\0i\0l\0e\0C\0o\0n\0t\0e\0n\0t\0s\0\0\0"
            , 26)
          , FILEGROUPDESCRIPTORW(
              "F\0i\0l\0e\0G\0r\0o\0u\0p\0D\0e\0s\0c\0r\0i\0p\0t\0o\0r\0W\0\0\0"
            , 42)
          , ARBITRARY_SCALE(40)
        {}

        void add_format(uint32_t ID, const std::string & name) {
            if (index < CLIPBRD_FORMAT_COUNT) {
                IDs[index]   = ID;
                names[index] = name;
                index++;
            }
        }
    }                           _clipbrd_formats_list;
    int                         _cItems;
    int                         _lindexToRequest;
    int                         _streamIDToRequest;
    struct CB_in_Files {
        int         size;
        std::string name;
    };
    std::vector<CB_in_Files>    _items_list;
    bool                        _waiting_for_data;
    int                         _lindex;




    bool setClientInfo() override;

    void writeClientInfo() override;

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override;

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override;

    virtual void begin_update() override;

    virtual void end_update() override;

    virtual void update_pointer_position(uint16_t xPos, uint16_t yPos) override;

    virtual int server_resize(int width, int height, int bpp) override;

    void send_buffer_to_clipboard();

    void process_server_clipboard_indata(int flags, InStream & chunk);

    void send_FormatListPDU(const uint32_t * formatIDs, const std::string * formatListDataShortName, std::size_t formatIDs_size,  bool) override;

    void send_to_clipboard_Buffer(InStream & chunk);

    void send_textBuffer_to_clipboard();

    void send_imageBuffer_to_clipboard();

    void empty_buffer() override;

    void process_client_clipboard_outdata(uint64_t total_length, OutStream & out_streamfirst, int firstPartSize, uint8_t const * data);

    virtual void set_pointer(Pointer const & cursor) override;

    void show_in_stream(int flags, InStream & chunk, size_t length);

    void show_out_stream(int flags, OutStream & chunk, size_t length);

    Screen_Qt * getMainScreen();

    virtual void emptyLocalBuffer();



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //---------------------------------------
    //   GRAPHIC FUNCTIONS (factorization)
    //---------------------------------------

    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert);

    QColor u32_to_qcolor(uint32_t color);

    QImage::Format bpp_to_QFormat(int bpp, bool alpha) override;

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

    ~Front_Qt();



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //------------------------
    //      CONTROLLERS
    //------------------------

    void mousePressEvent(QMouseEvent *e, int screen_index) override;

    void mouseReleaseEvent(QMouseEvent *e, int screen_index) override;

    void keyPressEvent(QKeyEvent *e) override;

    void keyReleaseEvent(QKeyEvent *e) override;

    void wheelEvent(QWheelEvent *e) override;

    bool eventFilter(QObject *obj, QEvent *e, int screen_index) override;

    void connexionPressed() override;

    bool connexionReleased() override;

    void RefreshPressed() override;

    void RefreshReleased() override;

    void CtrlAltDelPressed() override;

    void CtrlAltDelReleased() override;

    void disconnexionPressed()  override;

    void disconnexionReleased() override;

    void setMainScreenOnTopRelease() override;

    void refresh(int x, int y, int w, int h);

    void send_rdp_scanCode(int keyCode, int flag);

    bool connect();

    void disconnect( std::string const &) override;

    void closeFromScreen(int screen_index) override;

    void dropScreen() override;



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    void call_Draw() override;


};


