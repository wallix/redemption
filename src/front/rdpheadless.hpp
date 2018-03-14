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


#pragma once
#include "configs/config.hpp"

#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/caches/brushcache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"

#include "keyboard/keymap2.hpp"
#include "mod/rdp/rdp.hpp"
#include "mod/vnc/vnc.hpp"
#include "test_only/lcg_random.hpp"
#include "transport/socket_transport.hpp"

#include "utils/bitmap.hpp"
#include "utils/netutils.hpp"
#include "utils/select.hpp"
#include "utils/word_identification.hpp"

#include "openssl_tls.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct RDPHeadlessFrontParams
{
    std::string out_path;
    int index;
    int keep_alive_freq;

    RDPHeadlessFrontParams() = default;

    RDPHeadlessFrontParams(std::string out_path, int index = 0, int keep_alive_freq = 0)
      : out_path(std::move(out_path))
      , index(index)
      , keep_alive_freq(keep_alive_freq)
    {}
};

class RDPHeadlessFront : public FrontAPI
{
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

//     class EventList
//     {
//         struct EventConfig : private ::noncopyable
//         {
//             RDPHeadlessFront * front;
//             long trigger_time;
//
//             EventConfig(RDPHeadlessFront * front)
//             : front(front)
//             , trigger_time(0)
//             {}
//
//             virtual ~EventConfig() = default;
//
//             virtual void emit() = 0;
//         };
//
//         struct MouseButton : public EventConfig
//         {
//             uint8_t button;
//             uint32_t x;
//             uint32_t y;
//             const bool isPressed;
//
//             MouseButton( RDPHeadlessFront * front
//                     , uint8_t button
//                     , uint32_t x
//                     , uint32_t y
//                     , bool isPressed
//                     )
//                 : EventConfig(front)
//                 , button(button)
//                 , x(x)
//                 , y(y)
//                 , isPressed(isPressed)
//             {}
//
//
//             void emit() override {
//                 this->front->mouseButtons(button, x, y, isPressed);
//             }
//         };
//
//
//         struct MouseMove : public EventConfig
//         {
//             uint32_t x;
//             uint32_t y;
//
//             MouseMove( RDPHeadlessFront * front
//                     , uint32_t x
//                     , uint32_t y
//                     )
//                 : EventConfig(front)
//                 , x(x)
//                 , y(y)
//             {}
//
//             void emit() override {
//                 this->front->mouseMove(x, y);
//             }
//         };
//
//
//         struct KeyPressed : public EventConfig
//         {
//             uint32_t scanCode;
//             uint32_t Flag;
//
//             KeyPressed( RDPHeadlessFront * front
//                     , uint32_t scanCode
//                     , uint32_t Flag = 0
//                     )
//             : EventConfig(front)
//             , scanCode(scanCode)
//             , Flag(Flag)
//             {}
//
//             void emit() override {
//                 this->front->keyPressed(scanCode, Flag);
//             }
//         };
//
//
//         struct KeyReleased : public EventConfig
//         {
//             uint32_t scanCode;
//             uint32_t Flag;
//
//             KeyReleased( RDPHeadlessFront * front
//                     , uint32_t scanCode
//                     , uint32_t Flag = 0
//                     )
//             : EventConfig(front)
//             , scanCode(scanCode)
//             , Flag(Flag)
//             {}
//
//             void emit() override {
//                 this->front->keyReleased(scanCode, Flag);
//             }
//         };
//
//         struct Loop : EventConfig {
//             EventList * list;
//             int jumpt_size;
//             int count_steps;
//
//             Loop( EventList * list
//                 , int jumpt_size
//                 , int count_steps)
//             : EventConfig(nullptr)
//             , list(list)
//             , jumpt_size(jumpt_size)
//             , count_steps(count_steps)
//             {}
//
//             void emit() override {
//                 if (count_steps) {
//                     this->list->index -= jumpt_size;
//                     count_steps--;
//                 }
//             }
//         };
//
//         struct ClipboardChange : EventConfig
//         {
//             uint32_t formatIDs[RDPECLIP::FORMAT_LIST_MAX_SIZE];
//             std::string formatListDataLongName[RDPECLIP::FORMAT_LIST_MAX_SIZE];
//             size_t size;
//
//             ClipboardChange(RDPHeadlessFront * front
//                         , uint32_t * formatIDs
//                         , std::string * formatListDataLongName
//                         , size_t size)
//                 : EventConfig(front)
//                 , size(size)
//             {
//                 for (size_t i = 0; i < this->size; i++) {
//                     this->formatIDs[i] = formatIDs[i];
//                     this->formatListDataLongName[i] = formatListDataLongName[i];
//                 }
//             }
//
//             void emit() override {
//                 this->front->send_FormatListPDU(this->formatIDs, this->formatListDataLongName, this->size);
//             }
//         };
//
//         template<class T, class... Args>
//         void setAction(Args&&... args) {
//             auto action = std::make_unique<T>(std::forward<Args>(args)...);
//             action->trigger_time = this->wait_time;
//             this->list.push_back(std::move(action));
//         }

//         std::vector<std::unique_ptr<EventConfig>> list;
//         long start_time;
//         long wait_time;
//         size_t index;
//
//     public:
//         EventList()
//         : start_time(0)
//         , wait_time(0)
//         , index(0)
//         {
//             std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
//             this->start_time = ms.count();
//         }
//
// //         void setLoop(int jump_size, int count_steps) {
// //             this->setAction<Loop>(this, jump_size, count_steps);
// //         }
//
//         void setClpbrd_change( RDPHeadlessFront * front
//                             , uint32_t * formatIDs
//                             , std::string * formatListDataLongName
//                             , size_t size) {
//             this->setAction<ClipboardChange>(front, formatIDs, formatListDataLongName, size);
//         }
//
//         void setKey_press( RDPHeadlessFront * front
//                         , uint32_t scanCode
//                         , uint32_t flag) {
//             this->setAction<KeyPressed>(front, scanCode, flag);
//         }
//
//         void setKey_release( RDPHeadlessFront * front
//                         , uint32_t scanCode
//                         , uint32_t flag) {
//             this->setAction<KeyReleased>(front, scanCode, flag);
//         }
//
//         void setMouse_button( RDPHeadlessFront * front
//                             , uint8_t button
//                             , uint32_t x
//                             , uint32_t y
//                             , bool isPressed) {
//             this->setAction<MouseButton>(front, button, x, y, isPressed);
//         }
//
//         void setKey( RDPHeadlessFront * front
//                 , uint32_t scanCode
//                 , uint32_t flag) {
//             this->setKey_press(front, scanCode, flag);
//             this->setKey_release(front, scanCode, flag);
//         }
//
//         void setClick( RDPHeadlessFront * front
//                     , uint8_t button
//                     , uint32_t x
//                     , uint32_t y) {
//             this->setMouse_button(front, button, x, y, true);
//             this->setMouse_button(front, button, x, y, false);
//         }
//
//         void setDouble_click( RDPHeadlessFront * front
//                             , uint32_t x
//                             , uint32_t y) {
//             setClick(front, 0x01, x, y);
//             setClick(front, 0x01, x, y);
//         }
//
//         void wait(int ms) {
//             this->wait_time += ms;
//         }
//
//         size_t size() {
//             return this->list.size();
//         }
//
//         void emit() {
//             std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
//             long current_time(ms.count());
//             bool next(true);
//             size_t i = index;
//             while (next) {
//                 if (i >= this->size() || this->index >= this->size()) {
//                     return;
//                 }
//                 long triggger(this->list[i]->trigger_time + this->start_time);
//                 if (triggger <= current_time) {
//                     this->list[i]->emit();
//                     this->index++;
//
//
//                 } else {
//                     next = false;
//                 }
//                 i++;
//             }
//         }
//     };


    uint32_t                    _verbose;

public:
    // TODO enum class
    enum : uint32_t {
        SHOW_USER_AND_TARGET_PARAMS = 1 << 0
      , SHOW_MOD_RDP_PARAMS         = 1 << 1
      , SHOW_DRAW_ORDERS_INFO       = 1 << 2
      , SHOW_CLPBRD_PDU_EXCHANGE    = 1 << 3
      , SHOW_CURSOR_STATE_CHANGE    = 1 << 4
      , SHOW_CORE_SERVER_INFO       = 1 << 5
      , SHOW_SECURITY_SERVER_INFO   = 1 << 6
      , SHOW_KEYBOARD_EVENT         = 1 << 7
      , SHOW_FILE_SYSTEM_EXCHANGE   = 1 << 8
      , SHOW_IN_PDU                 = 1 << 9
      , SHOW_OUT_PDU                = 1 << 10
      , SHOW_CAPS                   = 1 << 11
      , SHOW_ALL                    = (1 << 12) - 1
    };

private:
    CHANNELS::ChannelDefArray   _cl;
    int                      _timer;
    ClipboardVirtualChannel  _clipboard_channel;


    // Graphic members
    uint8_t               mod_bpp;
    BGRPalette            mod_palette;


    // Connexion socket members
    ClientInfo        _info;
    std::unique_ptr<mod_api> _callback;

public:
    // TODO enum class
    enum : int {
        LOG_COMPLETE   = 3
      , INPUT_COMPLETE = 12
      , NAME           = 1
      , PWD            = 2
      , IP             = 4
      , PORT           = 8
    };

    // TODO enum class
    enum : long {
        DEFAULT_MAX_TIMEOUT_MILISEC_RESPONSE = 2000
    };

private:
    // Keyboard Controllers members
    Keymap2              _keymap;
    bool                 _ctrl_alt_delete; // currently not used and always false
    StaticOutStream<256> _decoded_data;    // currently not initialised
    uint8_t              _keyboardMods;


    //  Clipboard Channel Management members
    // TODO enum class
    enum : int {
        PDU_MAX_SIZE    = 1600
      , PDU_HEADER_SIZE =    8
    };
    // TODO enum class
    enum : int {
        PASTE_TEXT_CONTENT_SIZE = PDU_MAX_SIZE - PDU_HEADER_SIZE
      , PASTE_PIC_CONTENT_SIZE  = PDU_MAX_SIZE - RDPECLIP::METAFILE_HEADERS_SIZE - PDU_HEADER_SIZE
    };
    ClipboardServerChannelDataSender _to_server_sender;
    ClipboardClientChannelDataSender _to_client_sender;
    uint32_t                    _requestedFormatId;
    /*std::string                 _requestedFormatName;
    std::unique_ptr<uint8_t[]>  _bufferRDPClipboardChannel;
    size_t                      _bufferRDPClipboardChannelSize;
    size_t                      _bufferRDPClipboardChannelSizeTotal;
    int                         _bufferRDPCLipboardMetaFilePic_width;
    int                         _bufferRDPCLipboardMetaFilePic_height;
    int                         _bufferRDPClipboardMetaFilePicBPP;*/
    struct ClipbrdFormatsList{
        // TODO enum class
        enum : uint16_t {
              CF_CLIENT_FILEGROUPDESCRIPTORW = 48025
            , CF_CLIENT_FILECONTENTS         = 48026
        };
        // TODO enum class
        enum : int {
              CLIPBRD_FORMAT_COUNT = 5
        };

        const std::string FILECONTENTS;
        const std::string FILEGROUPDESCRIPTORW;
        uint32_t          IDs[CLIPBRD_FORMAT_COUNT];
        std::string       names[CLIPBRD_FORMAT_COUNT];
        int               index = 0;
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

        void add_format(uint32_t ID, std::string name) {
            IDs[index]   = ID;
            names[index] = std::move(name);
            index++;
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
    bool                        _running;
    bool is_pipe_ok;
    timeval start_connection_time;                          // when socket is connected
    timeval start_wab_session_time;                         // when the first resize is received
    timeval start_win_session_time;                         // when the first memblt is received
    std::string out_path;
    bool secondary_connection_finished;
    bool primary_connection_finished;

    int keep_alive_freq;

    int index;

    std::string error_message;

    ReportMessageApi & report_message;
//     EventList eventList;

    // for VNC
    Theme      theme;

    //  RDP
    LCGRandom gen;
    TimeSystem timeSystem;

    NullAuthentifier authentifier;

    std::unique_ptr<SocketTransport> socket = nullptr;


public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    RDPHeadlessFront(ClientInfo const & info, ReportMessageApi & report_message, uint32_t verbose, RDPHeadlessFrontParams params = {})
    : _verbose(verbose)
    , _clipboard_channel(&(this->_to_client_sender), &(this->_to_server_sender) ,*this , [&report_message](){
        ClipboardVirtualChannel::Params params(report_message);

        params.exchanged_data_limit = ~decltype(params.exchanged_data_limit){};
        params.verbose = to_verbose_flags(0xfffffff);

        params.clipboard_down_authorized = true;
        params.clipboard_up_authorized = true;
        params.clipboard_file_authorized = true;

        params.dont_log_data_into_syslog = true;
        params.dont_log_data_into_wrm = true;

        return params;
    }())
    , mod_bpp(info.bpp)
    , mod_palette(BGRPalette::classic_332())
    , _info(info)
    , _callback(nullptr)
    , _running(false)
    , is_pipe_ok(true)
    , out_path(std::move(params.out_path))
    , secondary_connection_finished(false)
    , primary_connection_finished(false)
    , keep_alive_freq(params.keep_alive_freq)
    , index(params.index)
    , report_message(report_message)
    , gen(0) // To always get the same client random, in tests
    {
        SSL_load_error_strings();
        SSL_library_init();

        this->_to_client_sender._front = this;
        this->_keymap.init_layout(this->_info.keylayout);

        CHANNELS::ChannelDef channel_cliprdr { channel_names::cliprdr
                                             , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                               GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                               GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                             , PDU_MAX_SIZE+1
                                             };

        this->_clipbrd_formats_list.add_format( ClipbrdFormatsList::CF_CLIENT_FILECONTENTS
                                              , this->_clipbrd_formats_list.FILECONTENTS
                                              );
        this->_clipbrd_formats_list.add_format( ClipbrdFormatsList::CF_CLIENT_FILEGROUPDESCRIPTORW
                                              , this->_clipbrd_formats_list.FILEGROUPDESCRIPTORW
                                              );
        this->_clipbrd_formats_list.add_format( RDPECLIP::CF_UNICODETEXT
                                              , std::string("\0\0", 2)
                                              );
        this->_clipbrd_formats_list.add_format( RDPECLIP::CF_TEXT
                                              , std::string("\0\0", 2)
                                              );
        this->_clipbrd_formats_list.add_format( RDPECLIP::CF_METAFILEPICT
                                              , std::string("\0\0", 2)
                                              );

        this->_to_client_sender._channel = channel_cliprdr;
//         this->_cl.push_back(channel_cliprdr);

        CHANNELS::ChannelDef channel_rdpdr{ channel_names::rdpdr
                                      , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                        GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                        GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                      , PDU_MAX_SIZE+1
                                      };
//         this->_cl.push_back(channel_rdpdr);

        if (this->mod_bpp == this->_info.bpp) {
            this->mod_palette = BGRPalette::classic_332();
        }
    }

    ~RDPHeadlessFront() = default;

    void record_connection_nego_times() {
        if (!this->secondary_connection_finished) {
            this->secondary_connection_finished = true;

            std::chrono::microseconds prim_duration = difftimeval(this->start_wab_session_time, this->start_connection_time);
            uint64_t prim_len = prim_duration.count() / 1000;
            std::cout << "primary connection length = " << prim_len << " ms" <<  std::endl;

//             if (!this->out_path.empty()) {
//                 std::cout << "out_path is not empty: " << this->out_path << std::endl;
//                 std::ofstream file_movie(this->out_path + "_prim_length", std::ios::app);
//                 if (file_movie) {
//                     std::cout << "out_path is written " << this->out_path << std::endl;
//                     file_movie << this->index << " " << prim_len << "\n";
//                 } else {
//                     std::cout << "out_path is not written " << this->out_path << std::endl;
//                 }
//             }

            this->start_win_session_time = tvtime();

            std::chrono::microseconds sec_duration = difftimeval(this->start_win_session_time, this->start_wab_session_time);
            uint64_t sec_len = sec_duration.count() / 1000;
            time_t now;
            time(&now);

            struct tm * timeinfo;
            char buffer [80];
            timeinfo = localtime (&now);
            strftime (buffer,80,"%F_%r",timeinfo);
            std::string date(buffer);

            std::cout << "secondary connection length = " <<  sec_len << " ms" << " " << date <<  std::endl;

//             if (!this->out_path.empty()) {
//                 std::ofstream file_movie(this->out_path + "_nego_length", std::ios::app);
//                 if (file_movie) {
//                     file_movie << this->index << " " << sec_len << " " << date << "\n";
//                 }
//             }
        }
    }

private:
    template<class Mod>
    struct ModWithReactor : SessionReactor, Mod
    {
        template<class... Args>
        ModWithReactor(Transport& trans, Args&&... args)
        : Mod(trans, *this, std::forward<Args>(args)...)
        {}
    };

public:

    int connect(const char * ip, const char * userName, const char * userPwd, int port, bool protocol_is_VNC, ModRDPParams & mod_rdp_params, uint32_t encryptionMethods) {

        int const nbTry(3);
        int const retryDelay(1000);
        int const sck = ip_connect(ip, port, nbTry, retryDelay).release();
        if (sck < 0) {
            std::cerr << "ip_connect: Cannot connect to [" << ip << "]." << std::endl;
            return -42;
        }

        this->socket = std::make_unique<SocketTransport>(
            userName,
            unique_fd{sck},
            ip,
            port,
            std::chrono::seconds(1),
            to_verbose_flags(this->_verbose),
            &this->error_message);

        std::cout << " Connected to [" << ip <<  "]." << std::endl;

        this->start_connection_time = tvtime();

        Inifile ini;

        GCC::UserData::SCCore const original_sc_core;
        GCC::UserData::SCSecurity const original_sc_sec1;

        not_null_ptr<GCC::UserData::SCCore const> sc_core_ptr = &original_sc_core;
        not_null_ptr<GCC::UserData::SCSecurity const> sc_sec1_ptr = &original_sc_sec1;

        if (protocol_is_VNC) {
            this->_callback = std::make_unique<ModWithReactor<mod_vnc>>(
                *this->socket
              , userName
              , userPwd
              , *(this)
              , this->_info.width
              , this->_info.height
              , ini.get<cfg::font>()
              , ""
              , ""
              , this->theme
              , this->_info.keylayout
              , 0
              , true
              , true
              , "0,1,-239"
              , false
              , true
              , mod_vnc::ClipboardEncodingType::UTF8
              , VncBogusClipboardInfiniteLoop::delayed
              , this->report_message
              , false
              , nullptr
              , to_verbose_flags(this->_verbose)
              );
        } else {
            auto rdp = std::make_unique<ModWithReactor<mod_rdp>>(
                *this->socket
              , *this
              , this->_info
              , ini.get_ref<cfg::mod_rdp::redir_info>()
              , this->gen
              , this->timeSystem
              , mod_rdp_params
              , this->authentifier
              , this->report_message
              , ini);

            GCC::UserData::CSSecurity & cs_security = rdp->cs_security;
            cs_security.encryptionMethods = encryptionMethods;

            sc_core_ptr = &rdp->sc_core;
            sc_sec1_ptr = &rdp->sc_sec1;
            this->_callback = std::move(rdp);
        }

        this->_to_server_sender._callback = this->_callback.get();

        try {
            while (!this->_callback->is_up_and_running()) {
                // std::cout << " Early negociations...\n";
                if (int err = this->wait_and_draw_event(sck, *this->_callback, *(this), {3, 0})) {
                    return err;
                }
            }
            this->primary_connection_finished = true;
            this->start_wab_session_time = tvtime();

        } catch (const Error & e) {
            std::cout << " Error: Failed during RDP early negociations step. " << e.errmsg() << "\n";
            if (error_message.size()) {
                std::cout << " Error tls: " << error_message << "\n";
            }
            return 2;
        }
        std::cout << " Early negociations completes.\n";

        if (protocol_is_VNC) {
            if (this->_verbose & RDPHeadlessFront::SHOW_CORE_SERVER_INFO && !protocol_is_VNC) {
                std::cout << " ================================" << "\n";
                std::cout << " ======= Server Core Info =======" << "\n";
                std::cout << " ================================" << "\n";

                std::cout << " userDataType = " << sc_core_ptr->userDataType << "\n";
                std::cout << " length = " << sc_core_ptr->length << "\n";
                std::cout << " version = " << sc_core_ptr->version << "\n";
                std::cout << " clientRequestedProtocols = " << sc_core_ptr->clientRequestedProtocols << "\n";
                std::cout << " earlyCapabilityFlags = " << sc_core_ptr->earlyCapabilityFlags << "\n";
                std::cout << std::endl;
            }

            if (this->_verbose & RDPHeadlessFront::SHOW_SECURITY_SERVER_INFO && !protocol_is_VNC) {
                std::cout << " ================================" << "\n";
                std::cout << " ===== Server Security Info =====" << "\n";
                std::cout << " ================================" << "\n";

                std::cout << " userDataType = " << sc_sec1_ptr->userDataType << "\n";
                std::cout << " length = " << sc_sec1_ptr->length << "\n";
                std::cout << " encryptionMethod = " << GCC::UserData::SCSecurity::get_encryptionMethod_name(sc_sec1_ptr->encryptionMethod) << "\n";
                std::cout << " encryptionLevel = " << GCC::UserData::SCSecurity::get_encryptionLevel_name(sc_sec1_ptr->encryptionLevel) << "\n";
                std::cout << " serverRandomLen = " << sc_sec1_ptr->serverRandomLen << "\n";
                std::cout << " serverCertLen = " << sc_sec1_ptr->serverCertLen << "\n";
                std::cout << " dwVersion = " << sc_sec1_ptr->dwVersion << "\n";
                std::cout << " temporary = " << sc_sec1_ptr->temporary << "\n";

                auto print_hex_data = [&sc_sec1_ptr](array_view_const_u8 av){
                    for (size_t i = 0; i < av.size(); i++) {
                        if ((i % 16) == 0 && i != 0) {
                            std::cout << "\n                ";
                        }
                        std::cout <<"0x";
                        if (av[i] < 0x10) {
                            std::cout << "0";
                        }
                        std::cout << std::hex << int(sc_sec1_ptr->serverRandom[i]) << std::dec << " ";
                    }
                    std::cout << "\n";
                    std::cout << "\n";
                };

                std::cout << " serverRandom : "; print_hex_data(sc_sec1_ptr->serverRandom);
                std::cout << " pri_exp : "; print_hex_data(sc_sec1_ptr->pri_exp);
                std::cout << " pub_sig : "; print_hex_data(sc_sec1_ptr->pub_sig);

                std::cout << " proprietaryCertificate : " << "\n";
                std::cout << "     dwSigAlgId = " << sc_sec1_ptr->proprietaryCertificate.dwSigAlgId << "\n";
                std::cout << "     dwKeyAlgId = " << sc_sec1_ptr->proprietaryCertificate.dwKeyAlgId << "\n";
                std::cout << "     wPublicKeyBlobType = " << sc_sec1_ptr->proprietaryCertificate.wPublicKeyBlobType << "\n";
                std::cout << "     wPublicKeyBlobLen = " << sc_sec1_ptr->proprietaryCertificate.wPublicKeyBlobLen << "\n";
                std::cout << "\n";
                std::cout << "     RSAPK : " << "\n";
                std::cout << "        magic = " << sc_sec1_ptr->proprietaryCertificate.RSAPK.magic << "\n";
                std::cout << "\n" << std::endl;

            }
        }

        return sck;

    }

    int wait_and_draw_event(int sck, mod_api& mod, FrontAPI & front, timeval timeout) {
        unsigned max = 0;
        fd_set   rfds;

        io_fd_zero(rfds);

// TODO        auto & event = mod.get_event();
// TODO        event.wait_on_fd(sck, rfds, max, timeout);

        int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);
        // std::cout << "RDP CLIENT :: select num = " <<  num << "\n";

        if (num < 0) {
            if (errno == EINTR) {
                return 0;
                //continue;
            }

            std::cerr << "RDP CLIENT :: errno = " <<  strerror(errno) << "\n";
            return 9;
        }

// TODO        if (event.is_set(sck, rfds)) {
// TODO            mod.draw_event(time(nullptr), front);
// TODO        }

        return 0;
    }

    mod_api * mod() {
        return this->_callback.get();
    }

    void disconnect() {
        this->_callback->disconnect(tvtime().tv_sec);

        if (this->_callback != nullptr) {
            TimeSystem timeobj;
            if (is_pipe_ok) {
                this->_callback->disconnect(timeobj.get_time().tv_sec);
            }
            this->_callback.reset();
        }
        this->socket.reset();

        std::chrono::microseconds prim_duration = difftimeval(this->start_wab_session_time, this->start_connection_time);
        uint64_t prim_len = prim_duration.count() / 1000;

        std::chrono::microseconds sec_duration = difftimeval(this->start_win_session_time, this->start_wab_session_time);
        uint64_t sec_len = sec_duration.count() / 1000;

        std::chrono::microseconds duration = difftimeval(tvtime(), this->start_win_session_time);
        uint64_t movie_len = duration.count() / 1000;

        time_t now;
        time(&now);

        struct tm * timeinfo;
        char buffer [80];
        timeinfo = localtime (&now);
        strftime (buffer,80,"%F_%r",timeinfo);
        std::string date(buffer);

        std::cout << "movielength = " << movie_len << " ms" << " " << date <<  std::endl;

        if (!this->out_path.empty()) {

            std::ofstream file_movie(this->out_path + "_session_length", std::ios::app);
            if (file_movie) {
                file_movie << this->index <<  " " <<  prim_len << " " << sec_len <<  " " <<  movie_len << " " << date << "\n";
            }
        }
    }

    bool is_running() {
        return this->_running;
    }

    bool can_be_start_capture() override { return true; }
    bool must_be_stop_capture() override {
        this->is_pipe_ok = false;
        return true;
    }
    bool is_connected() const { return this->is_pipe_ok; }
    void begin_update() override {}
    void end_update() override {}

    void set_pointer(Pointer const & cursor) override {
        if (this->_verbose & SHOW_CURSOR_STATE_CHANGE) {
            // TODO: dump changes to the cursor (signature based ?)
            std::cout <<  "server >> change cursor " <<  std::endl;
        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->_cl;
    }

    void setIndex(int index) {
        this->index = index;
    }

    void update_pointer_position(uint16_t xPos, uint16_t yPos) override {
        if (this->_verbose & SHOW_CURSOR_STATE_CHANGE) {
            std::cout << "server >> update_pointer_position " << int(xPos) << " " << int(yPos) << std::endl;
        }
    }

    ResizeResult server_resize(int width, int height, int bpp) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server_resize width=" << width << " height=" << height << " bpp=" << bpp << std::endl;
        }
        this->mod_bpp = bpp;
        this->_info.bpp = bpp;
        this->_info.width = width;
        this->_info.height = height;

//         if (!this->primary_connection_finished) {
//             this->primary_connection_finished = true;
//
//             this->start_wab_session_time = tvtime();
//         }

        return ResizeResult::instant_done;
    }

    void printClpbrdPDUExchange(std::string const & str, uint16_t valid) {
        if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE) {
            std::cout << str;
            if (valid == RDPECLIP::CB_RESPONSE_FAIL) {
                std::cout << " FAILED" <<  std::endl;
            } else {
                std::cout <<  std::endl;
            }
        }
    }

    void show_out_stream(int flags, OutStream & chunk, size_t length) {
        InStream in(chunk.get_data(), chunk.get_capacity());
        this->show_in_stream(flags, in, length);
    }

    void show_in_stream(int flags, InStream & chunk, size_t length) {
        std::cout <<  std::hex << "     flag=0x" << flags << " total_length=" << std::dec << int(length) <<  std::hex <<  std::endl;
        std::cout << "      \"";
        for (size_t i = 0; i < length; i++) {
            int byte(chunk.in_uint8());
            if ((i % 16) == 0 && i != 0) {
                std::cout << "\"" << std::endl << "      \"";
            }

            std::cout << "\\x";
            if (byte < 0x10) {
                std::cout << "0";
            }
            std::cout << byte;
        }
        std::cout << "\"" << std::dec << std::endl;
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //         CLIPBOARD
    //-----------------------------

    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t , size_t chunk_size, int flags) override {
        const CHANNELS::ChannelDef * mod_channel = this->_cl.get_by_name(channel.name);
        if (!mod_channel) {
            return;
        }

        InStream chunk(data, chunk_size);

        InStream chunk_series = chunk.clone();

        if (!chunk.in_check_rem(2  /*msgType(2)*/ )) {
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }


        if (channel.name == channel_names::cliprdr) {
            //std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            const uint16_t server_message_type = chunk.in_uint16_le();

            if (!this->_waiting_for_data) {
                switch (server_message_type) {
                    case RDPECLIP::CB_CLIP_CAPS:
                    if (this->_verbose & SHOW_CAPS) {
                        std::cout << "server >> Clipboard Capabilities PDU" << std::endl;
                        if (this->_verbose & SHOW_IN_PDU) {
                            this->show_in_stream(flags, chunk_series, chunk_size);
                        }
                    }

                    break;

                    case RDPECLIP::CB_MONITOR_READY:
                        if (this->_verbose & SHOW_CAPS) {
                            std::cout << "server >> Monitor Ready PDU" << std::endl;
                            if (this->_verbose & SHOW_IN_PDU) {
                                this->show_in_stream(flags, chunk_series, chunk_size);
                            }
                        }

                        {
                            int flag_out = CHANNELS::CHANNEL_FLAG_LAST |
                                           CHANNELS::CHANNEL_FLAG_FIRST |
                                           CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
                            RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1, RDPECLIP::GeneralCapabilitySet::size());
                            RDPECLIP::GeneralCapabilitySet general_cap_set(RDPECLIP::CB_CAPS_VERSION_2, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_USE_LONG_FORMAT_NAMES | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);
                            StaticOutStream<1024> out_stream;
                            clipboard_caps_pdu.emit(out_stream);
                            general_cap_set.emit(out_stream);

                            const uint32_t total_length = out_stream.get_offset();
                            InStream chunk(out_stream.get_data(), total_length);

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk
                                                                , total_length
                                                                , flag_out
                                                                );

                            if (this->_verbose & SHOW_CAPS) {
                                std::cout << "client >> Clipboard Capabilities PDU" << std::endl;
                                if (this->_verbose & SHOW_OUT_PDU) {
                                    this->show_out_stream(flag_out, out_stream, total_length);
                                }
                            }

                            //this->_monitorCount = this->_info.cs_monitor.monitorCount;

                            //std::cout << "cs_monitor count negociated. MonitorCount=" << this->_info.cs_monitor.monitorCount << std::endl;
                            //std::cout << "width=" <<  this->_info.width <<  " " << "height=" << this->_info.height <<  std::endl;

                            /*this->_info.width  = (this->_width * this->_monitorCount);

                            if (!this->_monitorCountNegociated) {
                                for (int i = this->_monitorCount - 1; i >= 1; i--) {
                                    this->_screen[i] = new Screen_Qt(this, i);
                                    this->_screen[i]->show();
                                }
                                this->_screen[0]->activateWindow();
                                this->_monitorCountNegociated = true;

                            }*/
                            //this->_monitorCountNegociated = true;
                        }
                        {
                            this->send_FormatListPDU(this->_clipbrd_formats_list.IDs, this->_clipbrd_formats_list.names, ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT);

                        }
                    break;

                    case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                        this->printClpbrdPDUExchange("server >> Format List Response PDU", chunk.in_uint16_le());
                        if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE && this->_verbose & SHOW_IN_PDU) {
                            this->show_in_stream(flags, chunk_series, chunk_size);
                        }
                        this->_running = true;
                    break;

                    default:
                        break;
                }
            }
        }
        else if (channel.name == channel_names::rdpdr) {

            uint16_t component = chunk.in_uint16_le();
            uint16_t packetId  = chunk.in_uint16_le();

            switch (component) {
                case rdpdr::Component::RDPDR_CTYP_CORE:

                    switch (packetId) {
                        case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                        {
                            uint16_t flag_out = CHANNELS::CHANNEL_FLAG_LAST  |
                                                CHANNELS::CHANNEL_FLAG_FIRST |
                                                CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
                            {
                            if (this->_verbose & SHOW_FILE_SYSTEM_EXCHANGE) {
                                std::cout << "server >> PAKID_CORE_SERVER_ANNOUNCE" <<  std::endl;
                                if (this->_verbose & SHOW_IN_PDU) {
                                    this->show_in_stream(flags, chunk_series, chunk_size);
                                }
                            }

                            uint16_t versionMajor(chunk.in_uint16_le());
                            uint16_t versionMinor(chunk.in_uint16_le());
                            uint32_t clientId(chunk.in_uint32_le());

                            StaticOutStream<32> stream;

                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
                            sharedHeader.emit(stream);

                            rdpdr::ClientAnnounceReply clientAnnounceReply(versionMajor, versionMinor, clientId);
                            clientAnnounceReply.emit(stream);

                            int total_length(stream.get_offset());
                            InStream chunk_to_send(stream.get_data(), stream.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                , chunk_to_send
                                                                , total_length
                                                                , flag_out
                                                                );
                            if (this->_verbose & SHOW_FILE_SYSTEM_EXCHANGE) {
                                std::cout << "client >> PAKID_CORE_CLIENTID_CONFIRM" << std::endl;
                                if (this->_verbose & SHOW_OUT_PDU) {
                                    this->show_out_stream(flag_out, stream, total_length);
                                }
                            }
                            }

                            {
                            StaticOutStream<32> stream;

                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
                            sharedHeader.emit(stream);
                            char username[LOGIN_NAME_MAX];
                            gethostname(username, LOGIN_NAME_MAX);

                            rdpdr::ClientNameRequest clientNameRequest(username);
                            clientNameRequest.emit(stream);

                            int total_length(stream.get_offset());
                            InStream chunk_to_send(stream.get_data(), stream.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                , chunk_to_send
                                                                , total_length
                                                                , flag_out
                                                                );
                            if (this->_verbose & SHOW_FILE_SYSTEM_EXCHANGE) {
                                std::cout << "client >> PAKID_CORE_CLIENT_NAME" <<  std::endl;
                                if (this->_verbose & SHOW_OUT_PDU) {
                                    this->show_out_stream(flag_out, stream, total_length);
                                }
                            }
                            }
                        }
                        break;

                        case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                            if (this->_verbose & SHOW_FILE_SYSTEM_EXCHANGE) {
                                std::cout << "server >> PAKID_CORE_SERVER_CAPABILITY" << std::endl;
                                if (this->_verbose & SHOW_IN_PDU) {
                                    this->show_in_stream(flags, chunk_series, chunk_size);
                                }
                            }
                        break;

                        case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                            if (this->_verbose & SHOW_FILE_SYSTEM_EXCHANGE) {
                                std::cout << "server >> PAKID_CORE_CLIENTID_CONFIRM" <<  std::endl;
                                if (this->_verbose & SHOW_IN_PDU) {
                                    this->show_in_stream(flags, chunk_series, chunk_size);
                                }
                            }
                        break;

                        default:
                            break;
                    }

                break;

                case rdpdr::Component::RDPDR_CTYP_PRT:
                    if (this->_verbose & SHOW_FILE_SYSTEM_EXCHANGE) {
                        std::cout <<  "server >> RDPDR_CTYP_PRT" <<  std::endl;
                        if (this->_verbose & SHOW_IN_PDU) {
                            this->show_in_stream(flags, chunk_series, chunk_size);
                        }
                    }
                break;

                default:
                break;
            }
        }
    }

    void send_buffer_to_clipboard() {}

    void process_server_clipboard_indata(int flags, InStream & chunk) {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
        }
        this->send_to_clipboard_Buffer(chunk);
    }

    void send_FormatListPDU(uint32_t const * /*formatIDs*/, std::string const * /*formatListDataShortName*/, std::size_t /*formatIDs_size*/) {
//         StaticOutStream<1024> out_stream;
//         RDPECLIP::FormatListPDU_LongName format_list_pdu_long(formatIDs, formatListDataShortName, formatIDs_size);
//         format_list_pdu_long.emit(out_stream);
//         const uint32_t total_length = out_stream.get_offset();
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         this->_callback->send_to_mod_channel( channel_names::cliprdr
//                                             , chunk
//                                             , total_length
//                                             , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
//                                             CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
//                                             );
//         if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE) {
//             std::cout << "client >> Format List PDU" << std::endl;
//         }
    }

    void send_to_clipboard_Buffer(InStream & chunk) { (void)chunk; }

    void send_textBuffer_to_clipboard() {}

    void send_imageBuffer_to_clipboard();

    void empty_buffer() {}

    void process_client_clipboard_outdata(uint64_t total_length, OutStream & out_streamfirst, int firstPartSize, uint8_t const * data) {
        (void)total_length;
        (void)out_streamfirst;
        (void)firstPartSize;
        (void)data;
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    void draw(RDPNineGrid const & , Rect , gdi::ColorCtx , Bitmap const & ) override {}

    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPOpaqueRect color=" << cmd.color.as_bgr().to_u32();
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << " clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPScrBlt rop=" << int(cmd.rop);
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        (void)bitmap;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMemBlt rop=" << int(cmd.rop);
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }

        this->record_connection_nego_times();
    }

    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPLineTo " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPatBlt rop=" << int(cmd.rop);
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        (void)bitmap;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMem3Blt rop=" << int(cmd.rop);
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        (void)bitmap_data;
        (void)bmp;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPBitmapData " << std::endl;
        }

        this->record_connection_nego_times();
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPDestBlt rop=" << int(cmd.rop);
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiDstBlt " << std::endl;
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiOpaqueRect " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiPatBlt " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiScrBlt " << std::endl;
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        (void)cmd;
        (void)gly_cache;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPGlyphIndex " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPolygonSC " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPolygonCB " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPolyline " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPEllipseSC " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPEllipseCB " << std::endl;
            std::cout << " bpp: " << color_ctx.depth().to_bpp();
            std::cout << "clip x=" << int(clip.x) <<  std::endl;
        }
    }

    void draw(const RDP::FrameMarker & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> FrameMarker " << std::endl;
        }
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> NewOrExistingWindow " << std::endl;
        }
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> WindowIcon " << std::endl;
        }
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> CachedIcon " << std::endl;
        }
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> DeletedWindow " << std::endl;
        }
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> NewOrExistingNotificationIcons " << std::endl;
        }
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> DeletedNotificationIcons " << std::endl;
        }
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> ActivelyMonitoredDesktop " << std::endl;
        }
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {
        (void)order;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> NonMonitoredDesktop " << std::endl;
        }
    }

    void draw(const RDPColCache & cmd) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPColCache " << std::endl;
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        (void)cmd;
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPBrushCache " << std::endl;
        }
    }



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    void mouseButtons(uint8_t button, uint32_t x, uint32_t y, bool press) {
        if (this->_callback != nullptr) {
            if (this->_verbose & SHOW_CURSOR_STATE_CHANGE) {
                if (press) {
                    std::cout << "client >> mouseButtonPressed=" << int(button) << " x=" << int(x) << " y=" << int(y) << std::endl;
                } else {
                    std::cout << "client >> mouseButtonReleased=" << int(button) << " x=" << int(x) << " y=" << int(y) << std::endl;
                }
            }
            int flag(0);
            switch (button) {
                case 1: flag = MOUSE_FLAG_BUTTON1; break;
                case 2: flag = MOUSE_FLAG_BUTTON2; break;
                case 4: flag = MOUSE_FLAG_BUTTON4; break;
                default: break;
            }
            if (press) {
                flag = flag | MOUSE_FLAG_DOWN;
            }
            this->_callback->rdp_input_mouse(flag, x, y, &(this->_keymap));
        }
    }

    void mouseMove(uint32_t x, uint32_t y) {
        if (this->_callback != nullptr) {
            if (this->_verbose & SHOW_CURSOR_STATE_CHANGE) {
                std::cout << "client >> mouseMove" << " x=" << int(x) << " y=" << int(y) << std::endl;
            }
            this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, &(this->_keymap));
        }
    }

    void keyPressed(uint32_t scanCode, uint32_t flag) {
        if (this->_verbose & SHOW_KEYBOARD_EVENT) {
            std::cout << "client >> keyPressed=" << int(scanCode) << std::endl;
        }
        if (this->_callback != nullptr) {
            this->_callback->rdp_input_scancode(scanCode, 0, flag, this->_timer, &(this->_keymap));
        }
    }

    void keyReleased(uint32_t scanCode, uint32_t flag) {
        if (this->_verbose & SHOW_KEYBOARD_EVENT) {
            std::cout << "client >> keyRelease=" << int(scanCode) << std::endl;
        }
        if (this->_callback != nullptr) {
            this->_callback->rdp_input_scancode(scanCode, 0, flag | KBD_FLAG_UP, this->_timer, &(this->_keymap));
        }
    }

    void send_key_to_keep_alive() {
        if (this->keep_alive_freq) {
            std::chrono::microseconds duration = difftimeval(tvtime(), this->start_win_session_time);

            if ( ((duration.count() / 1000000) % this->keep_alive_freq) == 0) {
                this->keyPressed(0x1e, 0);
                this->keyReleased(0x1e, 0);
            }
        }
    }


//     void set_event_list(const char * script_file_path) {
//         std::ifstream ifichier(script_file_path);
//         if(ifichier) {
//             std::string ligne;
//             const std::string delimiter = " ";
//
//             while(std::getline(ifichier, ligne)) {
//                 auto pos(ligne.find(delimiter));
//                 std::string tag  = ligne.substr(0, pos);
//                 std::string info = ligne.substr(pos + delimiter.length(), ligne.length());
//
//                 if (       tag == "wait") {
//                     this->eventList.wait(std::stoi(info));
//
//                 } else if (tag == "key_press") {
//                     pos = info.find(delimiter);
//                     uint32_t scanCode(std::stoi(info.substr(0, pos)));
//                     uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setKey_press(this, scanCode, flag);
//
//                 } else if (tag == "key_release") {
//                     pos = info.find(delimiter);
//                     uint32_t scanCode(std::stoi(info.substr(0, pos)));
//                     uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setKey_release(this, scanCode, flag);
//
//                 } else if (tag == "mouse_press") {
//                     pos = info.find(delimiter);
//                     uint8_t button(std::stoi(info.substr(0, pos)));
//                     info = info.substr(pos + delimiter.length(), info.length());
//                     pos = info.find(delimiter);
//                     uint32_t x(std::stoi(info.substr(0, pos)));
//                     uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setMouse_button(this, button, x, y, true);
//
//                 } else if (tag == "mouse_release") {
//                     pos = info.find(delimiter);
//                     uint8_t button(std::stoi(info.substr(0, pos)));
//                     info = info.substr(pos + delimiter.length(), info.length());
//                     pos = info.find(delimiter);
//                     uint32_t x(std::stoi(info.substr(0, pos)));
//                     uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setMouse_button(this, button, x, y, false);
//
//                 } else if (tag == "clpbrd_change") {
//                     // TODO dynamique data and format injection
//                     uint32_t formatIDs                 = RDPECLIP::CF_TEXT;
//                     std::string formatListDataLongName("\0\0", 2);
//
//                     // TODO { formatListDataLongName, 1 } -> array_view
//                     // TODO { formatIDs, 1 } -> array_view
//                     this->eventList.setClpbrd_change(this, &formatIDs, &formatListDataLongName, 1);
//
//                 } else if (tag == "click") {
//                     pos = info.find(delimiter);
//                     uint8_t button(std::stoi(info.substr(0, pos)));
//                     info = info.substr(pos + delimiter.length(), info.length());
//                     pos = info.find(delimiter);
//                     uint32_t x(std::stoi(info.substr(0, pos)));
//                     uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setClick(this, button, x, y);
//
//                 } else if (tag == "double_click") {
//                     pos = info.find(delimiter);
//                     uint32_t x(std::stoi(info.substr(0, pos)));
//                     uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setDouble_click(this, x, y);
//
//                 } else if (tag == "key") {
//                     pos = info.find(delimiter);
//                     uint32_t scanCode(std::stoi(info.substr(0, pos)));
//                     uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
//                     this->eventList.setKey(this, scanCode, flag);
//
//                 } else if (tag == "loop") {
//                     pos = info.find(delimiter);
//                     uint32_t jump_size(std::stoi(info.substr(0, pos)));
//                     uint32_t count_steps(std::stoi(info.substr(pos + delimiter.length(), info.length())));
//
// //                     this->eventList.setLoop(jump_size, count_steps);
//                 }
//             }
//         } else {
//             std::cerr <<  "Can't open " << script_file_path << ": " << strerror(errno) << "\n";
//         }
//     }
};
