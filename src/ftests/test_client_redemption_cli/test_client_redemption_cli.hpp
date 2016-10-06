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

//#define LOGPRINT

#include <chrono>
#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
#include "transport/socket_transport.hpp"
#include "mod/rdp/rdp.hpp"

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
#include "keyboard/keymap2.hpp"
#include "core/client_info.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic pop


struct ModRDPParamsConfig
{
    bool * param;
    const bool default_value;
    const std::string cmd;
    const std::string descrpt;
    const std::string name;

    ModRDPParamsConfig( bool * param
                      , bool default_value
                      , std::string cmd
                      , std::string descrpt
                      , std::string name
                      )
    : param(param)
    , default_value(default_value)
    , cmd(cmd)
    , descrpt(descrpt)
    , name(name)
    {
        *param = default_value;
    }

    ModRDPParamsConfig( bool * param
                      , std::string cmd
                      , std::string descrpt
                      , std::string name
                      )
    : param(param)
    , default_value(default_value)
    , cmd(cmd)
    , descrpt(descrpt)
    , name(name)
    {}
};




class TestClientCLI : public FrontAPI
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
    uint32_t                    _verbose;
    enum : uint32_t {
        SHOW_USER_AND_TARGET_PARAMS = 1
      , SHOW_MOD_RDP_PARAMS         = 2
      , SHOW_DRAW_ORDERS_INFO       = 4
      , SHOW_CLPBRD_PDU_EXCHANGE    = 8
      , SHOW_CURSOR_STATE_CHANGE    = 16
      , SHOW_CORE_SERVER_INFO       = 32
      , SHOW_SECURITY_SERVER_INFO   = 64
      , SHOW_KEYBOARD_EVENT         = 128
    };
    CHANNELS::ChannelDefArray   _cl;
    int                      _timer;
    ClipboardVirtualChannel  _clipboard_channel;


    // Graphic members
    uint8_t               mod_bpp;
    BGRPalette            mod_palette;


    // Connexion socket members
    ClientInfo        _info;
    mod_api         * _callback;
    enum : int {
        INPUT_COMPLETE = 15
      , NAME           = 1
      , PWD            = 2
      , IP             = 4
      , PORT           = 8
    };


    // Keyboard Controllers members
    Keymap2              _keymap;
    bool                 _ctrl_alt_delete; // currently not used and always false
    StaticOutStream<256> _decoded_data;    // currently not initialised
    uint8_t              _keyboardMods;


    //  Clipboard Channel Management members
    enum : int {
        PDU_MAX_SIZE    = 1600
      , PDU_HEADER_SIZE =    8
    };
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
        enum : uint16_t {
              CF_CLIENT_FILEGROUPDESCRIPTORW = 48025
            , CF_CLIENT_FILECONTENTS         = 48026
        };
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

        void add_format(uint32_t ID, const std::string & name) {
            IDs[index]   = ID;
            names[index] = name;
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



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    TestClientCLI(ClientInfo info, uint32_t verbose)
    : FrontAPI(false, false)
    , _verbose(verbose)
    , _clipboard_channel(&(this->_to_client_sender), &(this->_to_server_sender) ,*this , [](){
        ClipboardVirtualChannel::Params params;

        params.authentifier = nullptr;
        params.exchanged_data_limit = ~decltype(params.exchanged_data_limit){};
        params.verbose = 0xfffffff;

        params.clipboard_down_authorized = true;
        params.clipboard_up_authorized = true;
        params.clipboard_file_authorized = true;

        params.dont_log_data_into_syslog = true;
        params.dont_log_data_into_wrm = true;

        return params;
    }())
    , mod_bpp(this->_info.bpp)
    , mod_palette(BGRPalette::classic_332())
    , _info(info)
    , _callback(nullptr)
    , _running(false)
    {
        this->_to_client_sender._front = this;
        this->_keymap.init_layout(this->_info.keylayout);

        CHANNELS::ChannelDef channel { channel_names::cliprdr
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

        this->_to_client_sender._channel = channel;
        this->_cl.push_back(channel);

        if (this->mod_bpp == this->_info.bpp) {
            this->mod_palette = BGRPalette::classic_332();
        }
    }

    ~TestClientCLI() {}



    bool is_running() {
        return this->_running;
    }

    virtual bool can_be_start_capture(auth_api *) override { return true; }
    virtual bool can_be_pause_capture() override { return true; }
    virtual bool can_be_resume_capture() override { return true; }
    virtual bool must_be_stop_capture() override { return true; }
    virtual void begin_update() override {}
    virtual void end_update() override {}

    virtual void set_pointer(Pointer const & cursor) override {
        if (this->_verbose & SHOW_CURSOR_STATE_CHANGE) {
            std::cout <<  "cursor=" << int(cursor.pointer_type) <<  std::endl;
        }
    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->_cl;
    }

    void update_pointer_position(uint16_t xPos, uint16_t yPos) override {
        if (this->_verbose & SHOW_CURSOR_STATE_CHANGE) {
            std::cout << "update_pointer_position " << int(xPos) << " " << int(yPos) << std::endl;
        }
    }

    virtual int server_resize(int width, int height, int bpp) override {
        //std::cout << "server_resize width=" << width << " height=" << height << " bpp=" << bpp << std::endl;
        this->mod_bpp = bpp;
        this->_info.bpp = bpp;
        this->_info.width = width;
        this->_info.height = height;

        return 1;
    }

    void printClpbrdPDUExchange(std::string str, uint16_t valid) {
        if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE) {
            std::cout << str;
            if (valid == RDPECLIP::CB_RESPONSE_FAIL) {
                std::cout << " FAILED" <<  std::endl;
            } else {
                std::cout <<  std::endl;
            }
        }
    }



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //         CLIPBOARD
    //-----------------------------

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t , size_t chunk_size, int flags) {
        const CHANNELS::ChannelDef * mod_channel = this->_cl.get_by_name(channel.name);
        if (!mod_channel) {
            return;
        }

        if (!strcmp(channel.name, channel_names::cliprdr)) {
            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            std::cout << std::dec;

            InStream chunk(data, chunk_size);

            InStream chunk_series = chunk.clone();

            if (!chunk.in_check_rem(2  /*msgType(2)*/ )) {
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            const uint16_t server_message_type = chunk.in_uint16_le();

            if (!this->_waiting_for_data) {
                switch (server_message_type) {
                    case RDPECLIP::CB_CLIP_CAPS:
                    if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE) {
                        std::cout << "server >> Clipboard Capabilities PDU" << std::endl;
                    }

                    break;

                    case RDPECLIP::CB_MONITOR_READY:
                        std::cout << "server >> Monitor Ready PDU" << std::endl;

                        {
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
                                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST |
                                                                  CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );

                            if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE) {
                                std::cout << "client >> Clipboard Capabilities PDU" << std::endl;
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
                            this->send_FormatListPDU(this->_clipbrd_formats_list.IDs, this->_clipbrd_formats_list.names, ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT, true);

                        }
                    break;

                    case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                        this->printClpbrdPDUExchange("server >> Format List Response PDU", chunk.in_uint16_le());
                        this->_running = true;
                    break;

                    default:
                        break;
                }
            }
        }
    }

    void send_buffer_to_clipboard() {}

    void process_server_clipboard_indata(int flags, InStream & chunk) {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
        }
        this->send_to_clipboard_Buffer(chunk);
    }

    void send_FormatListPDU(const uint32_t * formatIDs, const std::string * formatListDataShortName, std::size_t formatIDs_size,  bool isLong) {
        RDPECLIP::FormatListPDU format_list_pdu;
        StaticOutStream<1024> out_stream;
        if (isLong) {
            format_list_pdu.emit_long(out_stream, formatIDs, formatListDataShortName, formatIDs_size);
        } else {
            format_list_pdu.emit_short(out_stream, formatIDs, formatListDataShortName, formatIDs_size);
        }
        const uint32_t total_length = out_stream.get_offset();
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        this->_callback->send_to_mod_channel( channel_names::cliprdr
                                            , chunk
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                              CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                            );

        if (this->_verbose & SHOW_CLPBRD_PDU_EXCHANGE) {
            std::cout << "client >> Format List PDU" << std::endl;
        }
    }

    void send_to_clipboard_Buffer(InStream & chunk) {}

    void send_textBuffer_to_clipboard() {}

    void send_imageBuffer_to_clipboard() {}

    void empty_buffer() {}

    void process_client_clipboard_outdata(uint64_t total_length, OutStream & out_streamfirst, int firstPartSize, uint8_t const * data) {}



    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPOpaqueRect color=" << int(cmd.color) << std::endl;
        }
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPScrBlt " << std::endl;
        }
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMemBlt " << std::endl;
        }
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPLineTo " << std::endl;
        }
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPatBlt " << std::endl;
        }
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMem3Blt " << std::endl;
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPBitmapData " << std::endl;
        }
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPDestBlt " << std::endl;
        }
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiDstBlt " << std::endl;
        }
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiOpaqueRect " << std::endl;
        }
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiPatBlt " << std::endl;
        }
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPMultiScrBlt " << std::endl;
        }
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPGlyphIndex " << std::endl;
        }
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPolygonSC " << std::endl;
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPolygonCB " << std::endl;
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPPolyline " << std::endl;
        }
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPEllipseSC " << std::endl;
        }
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPEllipseCB " << std::endl;
        }
    }

    virtual void draw(const RDP::FrameMarker & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> FrameMarker " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> NewOrExistingWindow " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> WindowIcon " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> CachedIcon " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> DeletedWindow " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> NewOrExistingNotificationIcons " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::DeletedNotificationIcons & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> DeletedNotificationIcons " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> ActivelyMonitoredDesktop " << std::endl;
        }
    }

    virtual void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> NonMonitoredDesktop " << std::endl;
        }
    }

    virtual void draw(const RDPColCache   & cmd) override {
        if (this->_verbose & SHOW_DRAW_ORDERS_INFO) {
            std::cout << "server >> RDPColCache " << std::endl;
        }
    }

    virtual void draw(const RDPBrushCache & cmd) override {
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
                    std::cout << "mouseButtonPressed=" << int(button) << " x=" << int(x) << " y=" << int(y) << std::endl;
                } else {
                    std::cout << "mouseButtonReleased=" << int(button) << " x=" << int(x) << " y=" << int(y) << std::endl;
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
                std::cout << "mouseMove" << " x=" << int(x) << " y=" << int(y) << std::endl;
            }
            this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, &(this->_keymap));
        }
    }

    void keyPressed(uint32_t scanCode, uint32_t flag) {
        if (this->_verbose & SHOW_KEYBOARD_EVENT) {
            std::cout << "keyPressed=" << int(scanCode) << std::endl;
        }
        if (this->_callback != nullptr) {
            this->_callback->rdp_input_scancode(scanCode, 0, flag, this->_timer, &(this->_keymap));
        }
    }

    void keyReleased(uint32_t scanCode, uint32_t flag) {
        if (this->_verbose & SHOW_KEYBOARD_EVENT) {
            std::cout << "scanCode=" << int(scanCode) << std::endl;
        }
        if (this->_callback != nullptr) {
            this->_callback->rdp_input_scancode(scanCode, 0, flag | KBD_FLAG_UP, this->_timer, &(this->_keymap));
        }
    }


};



struct EventConfig
{
    TestClientCLI * front;
    long trigger_time;

    EventConfig(TestClientCLI * front)
    : front(front)
    , trigger_time(0)
    {}

    virtual ~EventConfig() {};

    virtual void emit() = 0;
};


struct ClipboardChange : EventConfig
{
    uint32_t formatIDs[RDPECLIP::FORMAT_LIST_MAX_SIZE];
    std::string formatListDataLongName[RDPECLIP::FORMAT_LIST_MAX_SIZE];
    size_t size;

    ClipboardChange( TestClientCLI * front
                   , uint32_t * formatIDs
                   , std::string * formatListDataLongName
                   , size_t size)
    : EventConfig(front)
    , size(size)
    {
        for (size_t i = 0; i < this->size; i++) {
            this->formatIDs[i] = formatIDs[i];
            this->formatListDataLongName[i] = formatListDataLongName[i];
        }
    }

    virtual void emit() override {
        this->front->send_FormatListPDU(this->formatIDs, this->formatListDataLongName, this->size, true);
    }
};


struct MouseButton : public EventConfig
{
    uint8_t button;
    uint32_t x;
    uint32_t y;
    const bool isPressed;

    MouseButton( TestClientCLI * front
               , uint8_t button
               , uint32_t x
               , uint32_t y
               , bool isPressed
               )
    : EventConfig(front)
    , button(button)
    , x(x)
    , y(y)
    , isPressed(isPressed)
    {}

    virtual void emit() override {
        this->front->mouseButtons(button, x, y, isPressed);
    }
};


struct MouseMove : public EventConfig
{
    uint32_t x;
    uint32_t y;

    MouseMove( TestClientCLI * front
             , uint32_t x
             , uint32_t y
             )
    : EventConfig(front)
    , x(x)
    , y(y)
    {}

    void emit() override {
        this->front->mouseMove(x, y);
    }
};


struct KeyPressed : public EventConfig
{
    uint32_t scanCode;
    uint32_t Flag;

    KeyPressed( TestClientCLI * front
               , uint32_t scanCode
               , uint32_t Flag = 0
               )
    : EventConfig(front)
    , scanCode(scanCode)
    , Flag(Flag)
    {}

    virtual void emit() override {
        this->front->keyPressed(scanCode, Flag);
    }
};


struct KeyReleased : public EventConfig
{
    uint32_t scanCode;
    uint32_t Flag;

    KeyReleased( TestClientCLI * front
               , uint32_t scanCode
               , uint32_t Flag = 0
               )
    : EventConfig(front)
    , scanCode(scanCode)
    , Flag(Flag)
    {}

    virtual void emit() override {
        this->front->keyReleased(scanCode, Flag);
    }
};


class EventList
{
public:
    std::vector<EventConfig *> list;
    long start_time;
    long wait_time;
    size_t index;

    EventList()
      : start_time(0)
      , wait_time(0)
      , index(0)
    {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
        this->start_time = ms.count();
    }

    ~EventList()
    {
        for (size_t i = 0; i < this->list.size(); i++) {
            delete(this->list[i]);
        }

        this->list.clear();
    }


    void setAction(EventConfig * action) {
        action->trigger_time = this->wait_time;
        this->list.push_back(action);
    }

    void wait(int ms) {
        this->wait_time += ms;
    }

    size_t size() {
        return this->list.size();
    }

    void emit() {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
        long current_time(ms.count());
        bool next(true);
        size_t i = index;
        while (next) {
            if (i >= this->size() || this->index >= this->size()) {
                return;
            }
            long triggger(this->list[i]->trigger_time + this->start_time);
            if (triggger <= current_time) {
                this->list[i]->emit();
                this->index++;

            } else {
                next = false;
            }
            i++;
        }
    }
};


