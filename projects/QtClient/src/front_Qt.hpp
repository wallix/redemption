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


#ifndef Q_MOC_RUN
#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/hdreg.h>
//#include <linux/fcntl.h>

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

#include "mod/rdp/rdp_log.hpp"
#include "core/RDP/pointer.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "mod/mod_api.hpp"
#include "mod/internal/replay_mod.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "keyboard/keymap2.hpp"
#include "core/client_info.hpp"
#include "keymaps/Qt_ScanCode_KeyMap.hpp"
#include "capture/capture.hpp"

#include "Qt4/Qt.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic pop

#endif



#define _WINDOWS_TICK 10000000
#define _SEC_TO_UNIX_EPOCH 11644473600LL

#define USER_CONF_PATH "/config/userConfig.config"
#define CB_FILE_TEMP_PATH "/clipboard_temp"
#define REPLAY_PATH "/replay"
#define KEY_SETTING_PATH "/config/keySetting.config"
#define LOGINS_PATH "/config/logins.config"
#define _SHARE_PATH "/share"


class Form_Qt;
class Screen_Qt;
class Mod_Qt;
class ClipBoard_Qt;


struct DummyAuthentifier : public auth_api
{
public:
    virtual void set_auth_channel_target(const char *) {}
    virtual void set_auth_error_message(const char *) {}
    virtual void report(const char * , const char *) {}
    virtual void log4(bool , const char *, const char * = nullptr) {}
    virtual void disconnect_target() {}
};

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

    implicit_bool_flags<RDPVerbose>   verbose;
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
    mod_api         * _callback;
    std::unique_ptr<ReplayMod> _replay_mod;
    Mod_Qt          * _mod_qt;
    QImage::Format    _imageFormatRGB;
    QImage::Format    _imageFormatARGB;
    ClipboardServerChannelDataSender _to_server_sender;
    ClipboardClientChannelDataSender _to_client_sender;
    Qt_ScanCode_KeyMap   _qtRDPKeymap;
    int                  _fps;
    int                  _monitorCount;
    const std::string    MAIN_DIR;
    const std::string    CB_TEMP_DIR;
    const std::string    USER_CONF_DIR;
    const std::string    REPLAY_DIR;
    const std::string    SHARE_DIR;
    QPixmap            * _cache;
    QPixmap            * _cache_replay;
    bool                 _span;
    Rect                 _screen_dimensions[MAX_MONITOR_COUNT];
    bool                 _record;
    bool                 _replay;
    int                  _delta_time;
    int                  _current_screen_index;
    bool                 _recv_disconnect_ultimatum;



    Front_Qt_API( bool param1
                , bool param2
                , RDPVerbose verbose)
    : FrontAPI(param1, param2)
    , verbose(verbose)
    , _info()
    , _port(0)
    , _callback(nullptr)
    , _replay_mod(nullptr)
    , _mod_qt(nullptr)
    , _qtRDPKeymap()
    , _fps(30)
    , _monitorCount(1)
    , MAIN_DIR(MAIN_PATH)
    , CB_TEMP_DIR(MAIN_DIR + std::string(CB_FILE_TEMP_PATH))
    , USER_CONF_DIR(MAIN_DIR + std::string(USER_CONF_PATH))
    , REPLAY_DIR(MAIN_DIR + std::string(REPLAY_PATH))
    , SHARE_DIR(MAIN_DIR + std::string(_SHARE_PATH))
    , _cache(nullptr)
    , _cache_replay(nullptr)
    , _span(false)
    , _record(false)
    , _replay(false)
    , _delta_time(1000000)
    , _current_screen_index(0)
    , _recv_disconnect_ultimatum(false)
    {
        this->_to_client_sender._front = this;
    }

    virtual Screen_Qt * getMainScreen() = 0;
    virtual void connexionPressed() = 0;
    virtual bool connexionReleased() = 0;
    virtual void closeFromScreen(int screen_index) = 0;
    virtual void RefreshPressed() = 0;
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
    virtual void send_FormatListPDU(uint32_t const * formatIDs, std::string const * formatListDataShortName, std::size_t formatIDs_size) = 0;
    virtual void empty_buffer() = 0;
    virtual bool can_be_start_capture() override { return true; }
//     virtual bool can_be_pause_capture() override { return true; }
//     virtual bool can_be_resume_capture() override { return true; }
    virtual bool must_be_stop_capture() override { return true; }
    virtual void emptyLocalBuffer() = 0;
    virtual void replay(std::string const & movie_path) = 0;
    virtual void load_replay_mod(std::string const & movie_name) = 0;
    virtual void delete_replay_mod() = 0;
};



class Front_Qt : public Front_Qt_API
{
    struct Snapshoter : gdi::CaptureApi
    {
        Front_Qt & front;

        Snapshoter(Front_Qt & front) : front(front) {}

        std::chrono::microseconds do_snapshot(
            const timeval& /*now*/, int cursor_x, int cursor_y, bool /*ignore_frame_in_timeval*/
        ) override {
            this->front.update_pointer_position(cursor_x, cursor_y);
            std::chrono::microseconds res(1);
            return res;
        }
//         void do_pause_capture(const timeval&) override {}
//         void do_resume_capture(const timeval&) override {}
    };
    Snapshoter snapshoter;

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
    QPixmap            * _cache;
    QPixmap            * _trans_cache;
    gdi::GraphicApi    * _graph_capture;

    struct MouseData {
        QImage cursor_image;
        uint16_t x = 0;
        uint16_t y = 0;
    } _mouse_data;

    // Connexion socket members
    ClipBoard_Qt       * _clipboard_qt;
    int                  _timer;
    bool                 _connected;
    bool                 _monitorCountNegociated;
    ClipboardVirtualChannel  _clipboard_channel;
    Capture            * _capture;
    Font                 _font;
    std::string          _error;

    // Keyboard Controllers members
    Keymap2              _keymap;
    bool                 _ctrl_alt_delete; // currently not used and always false
    StaticOutStream<256> _decoded_data;    // currently not initialised
    uint8_t              _keyboardMods;
    CHANNELS::ChannelDefArray   _cl;

    // Clipboard Channel Management members
    uint32_t             _requestedFormatId = 0;
    std::string          _requestedFormatName;
    bool                 _waiting_for_data;

    struct ClipbrdFormatsList{
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

        ClipbrdFormatsList()
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

    } _clipbrdFormatsList;

    struct CB_FilesList {
        struct CB_in_Files {
            int         size;
            std::string name;
        };
        uint32_t                 cItems = 0;
        uint32_t                 lindexToRequest = 0;
        int                      streamIDToRequest = 0;
        std::vector<CB_in_Files> itemslist;
        int                      lindex = 0;

    }  _cb_filesList;

    struct CB_Buffers {
        std::unique_ptr<uint8_t[]>  data = nullptr;
        size_t size = 0;
        size_t sizeTotal = 0;
        int    pic_width = 0;
        int    pic_height = 0;
        int    pic_bpp = 0;

    } _cb_buffers;




    struct FileSystemData {

        struct DeviceData {
            char name[8] = {0};
            uint32_t ID = 0;
            uint32_t status = -1;
            uint32_t file_id = 0;
        };

        bool drives_created = false;
        bool fileSystemCapacity[5] = { false };
        const size_t drivesCount = 1;
        DeviceData drives[2];

        uint32_t next_file_id = 0;

        uint32_t get_file_id() {
            this->next_file_id++;
            return this->next_file_id;
        }

        std::vector<std::string> paths;

        uint32_t current_dir_id = 0;
        int current_file_index = 0;

        int writeData_to_wait = 0;
        int file_to_write_id = 0;

        int last_file_read_id = 0;
        int last_file_read_offset = 0;

    } fileSystemData;


//     struct OutStreamData {
//
//         size_t current_pos = 0;
//
//         virtual const uint8_t * get_data(size_t size) = 0;
//
//         virtual ~OutStreamData() {}
//
//     };
//
//     struct OutStreamDataRaw : OutStreamData {
//         const uint8_t * data;
//
//         OutStreamDataRaw(const uint8_t * data)
//           : data(data)
//           {}
//
//         virtual const uint8_t * get_data(size_t size) {
//             const uint8_t * data_to_send = data;
//             data += size;
//             return data_to_send;
//         }
//     };
//
//     struct OutStreamDataFStream : OutStreamData {
//         std::ifstream ifile;
//         char data_to_send[1600];
//
//         OutStreamDataFStream(std::string & file_name)
//           : ifile(file_name.x, std::ios::in | std::ios::binary)
//           {}
//
//         const uint8_t * get_data(size_t size) {
//             this->ifile.read(this->data_to_send, size);
//             return reinterpret_cast<const uint8_t *>(this->data_to_send);
//         }
//
//         bool is_open() {
//             return this->ifile.is_open();
//         }
//
//         ~OutStreamDataFStream() {
//             this->ifile.close();
//         }
//     };




    bool setClientInfo() override;

    void writeClientInfo() override;

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override;

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length, size_t chunk_size, int flags) override;

    virtual void begin_update() override;

    virtual void end_update() override;

    virtual void update_pointer_position(uint16_t xPos, uint16_t yPos) override;

    virtual ResizeResult server_resize(int width, int height, int bpp) override;

    void send_buffer_to_clipboard();

    void process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList, ClipBoard_Qt * clipboard_qt);

    void send_FormatListPDU(const uint32_t * formatIDs, const std::string * formatListDataShortName, std::size_t formatIDs_size) override;

    void send_to_clipboard_Buffer(InStream & chunk);

    void send_textBuffer_to_clipboard();

    void send_imageBuffer_to_clipboard();

    void empty_buffer() override;

    void process_client_clipboard_out_data(const char * const front_channel_name, const uint64_t total_length, OutStream & out_streamfirst, size_t firstPartSize, const uint8_t * data, const size_t data_len, uint32_t flags);

    virtual void set_pointer(Pointer const & cursor) override;

    void show_in_stream(int flags, InStream & chunk, size_t length);

    void show_out_stream(int flags, OutStream & chunk, size_t length);

    Screen_Qt * getMainScreen() override;

    virtual void emptyLocalBuffer() override;

    void setScreenDimension();

    void load_replay_mod(std::string const & movie_name) override;

    void delete_replay_mod() override;

    void removeDriveDevice(const FileSystemData::DeviceData *, const size_t);

//     virtual gdi::Depth const & order_depth() const override {
//         return gdi::Depth::from_bpp(this->mod_bpp);
//     }

    virtual void recv_disconnect_provider_ultimatum() override;

//     virtual void set_depths(gdi::Depth const & depths) override {}

    unsigned WindowsTickToUnixSeconds(long long windowsTicks)
    {
        return unsigned((windowsTicks / _WINDOWS_TICK) - _SEC_TO_UNIX_EPOCH);
    }

    long long UnixSecondsToWindowsTick(unsigned unixSeconds)
    {
        return ((unixSeconds + _SEC_TO_UNIX_EPOCH) * _WINDOWS_TICK);
    }

    uint32_t string_to_hex32(unsigned char * str) {
        size_t size = sizeof(str);
        uint32_t hex32(0);
        for (size_t i = 0; i < size; i++) {
            int s = str[i];
            if(s > 47 && s < 58) {                           //this covers 0-9
                hex32 += (s - 48) << (size - i - 1);
            } else if (s > 64 && s < 71) {                 // this covers A-F
                hex32 += (s - 55) << (size - i - 1);
            }
        }

        return hex32;
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //---------------------------------------
    //   GRAPHIC FUNCTIONS (factorization)
    //----------------------------- ----------

    template<class Op>
    void draw_memblt_op(const Rect & drect, const Bitmap & bitmap);


    struct Op_0x11 {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
             return ~(src | dst);                           // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
         }                                                  // |      | RPN: DSon                     |
     };                                                     // +------+-------------------------------+

    struct Op_0x22 {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
             return (~src & dst);                           // | 0x22 | ROP: 0x00220326               |
         }                                                  // |      | RPN: DSna                     |
     };                                                     // +------+-------------------------------+

     struct Op_0x33 {
        uchar op(const uchar src, const uchar) const {      // +------+-------------------------------+
             return (~src);                                 // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
        }                                                   // |      | RPN: Sn                       |
     };                                                     // +------+-------------------------------+

     struct Op_0x44 {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
            return (src & ~dst);                            // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
        }                                                   // |      | RPN: SDna                     |
    };                                                      // +------+-------------------------------+

    struct Op_0x55 {
        uchar op(const uchar, const uchar dst) const {      // +------+-------------------------------+
             return (~dst);                                 // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
        }                                                   // |      | RPN: Dn                       |
     };                                                     // +------+-------------------------------+

    struct Op_0x66 {
         uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src ^ dst);                             // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
         }                                                  // |      | RPN: DSx                      |
     };                                                     // +------+-------------------------------+

     struct Op_0x77 {
         uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
             return ~(src & dst);                           // | 0x77 | ROP: 0x007700E6               |
         }                                                  // |      | RPN: DSan                     |
     };                                                     // +------+-------------------------------+

    struct Op_0x88 {
         uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src & dst);                             // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
         }                                                  // |      | RPN: DSa                      |
     };                                                     // +------+-------------------------------+

     struct Op_0x99 {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
            return ~(src ^ dst);                            // | 0x99 | ROP: 0x00990066               |
        }                                                   // |      | RPN: DSxn                     |
     };                                                     // +------+-------------------------------+

     struct Op_0xBB {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
            return (~src | dst);                            // | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
        }                                                   // |      | RPN: DSno                     |
     };                                                     // +------+-------------------------------+

     struct Op_0xDD {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
            return (src | ~dst);                            // | 0xDD | ROP: 0x00DD0228               |
        }                                                   // |      | RPN: SDno                     |
     };                                                     // +------+-------------------------------+

    struct Op_0xEE {
        uchar op(const uchar src, const uchar dst) const {  // +------+-------------------------------+
            return (src | dst);                             // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
        }                                                   // |      | RPN: DSo                      |
    };                                                      // +------+-------------------------------+


    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert);

    QColor u32_to_qcolor(uint32_t color);

    QColor u32_to_qcolor_r(uint32_t color);

    QImage::Format bpp_to_QFormat(int bpp, bool alpha) override;

    void draw_MemBlt(const Rect & drect, const Bitmap & bitmap, bool invert, int srcx, int srcy);

    void draw_RDPPatBlt(const Rect & rect, const QColor color, const QPainter::CompositionMode mode, const Qt::BrushStyle style);

    void draw_RDPPatBlt(const Rect & rect, const QPainter::CompositionMode mode);



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    using Front_Qt_API::draw;

    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPScrBlt & cmd, Rect clip) override;

    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override;

    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override;

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override;

    void draw(const RDPDestBlt & cmd, Rect clip) override;

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override;

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override;

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override;

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDP::FrameMarker & order) override;

//     void draw(const RDP::RAIL::NewOrExistingWindow & order) override;
//
//     void draw(const RDP::RAIL::WindowIcon & order) override;
//
//     void draw(const RDP::RAIL::CachedIcon & order) override;
//
//     void draw(const RDP::RAIL::DeletedWindow & order) override;
//
//     void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override;
//
//     void draw(const RDP::RAIL::DeletedNotificationIcons & order) override;
//
//     void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override;
//
//     void draw(const RDP::RAIL::NonMonitoredDesktop & order) override;

//     void draw(const RDPColCache   & cmd) override;
//
//     void draw(const RDPBrushCache & cmd) override;



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    Front_Qt(char* argv[], int argc, RDPVerbose verbose);

    ~Front_Qt();



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    void mousePressEvent(QMouseEvent *e, int screen_shift) override;

    void mouseReleaseEvent(QMouseEvent *e, int screen_shift) override;

    void keyPressEvent(QKeyEvent *e) override;

    void keyReleaseEvent(QKeyEvent *e) override;

    void wheelEvent(QWheelEvent *e) override;

    bool eventFilter(QObject *obj, QEvent *e, int screen_shift) override;

    void connexionPressed() override;

    bool connexionReleased() override;

    void RefreshPressed() override;

    void CtrlAltDelPressed() override;

    void CtrlAltDelReleased() override;

    void disconnexionPressed()  override;

    void disconnexionReleased() override;

    void setMainScreenOnTopRelease() override;

    void refresh(int x, int y, int w, int h);

    void send_rdp_scanCode(int keyCode, int flag);

    bool connect();

    void disconnect(std::string const &) override;

    void closeFromScreen(int screen_index) override;

    void dropScreen() override;

    void replay(std::string const & movie_path) override;



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    void call_Draw() override;


};

