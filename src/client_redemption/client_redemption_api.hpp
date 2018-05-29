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
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "utils/log.hpp"

#ifndef Q_MOC_RUN

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <climits>
#include <cstdint>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>

#include <openssl/ssl.h>

#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"
#include "gdi/graphic_api.hpp"
// #include "keyboard/keymap2.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/replay_mod.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "transport/crypto_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/bitmap.hpp"
#include "utils/genfstat.hpp"
#include "utils/genrandom.hpp"
#include "utils/netutils.hpp"
#include "utils/fileutils.hpp"
#include "main/version.hpp"

#endif

#define CLIENT_REDEMPTION_REPLAY_PATH "/DATA/replay"
#define CLIENT_REDEMPTION_LOGINS_PATH "/DATA/config/login.config"
#define CLIENT_REDEMPTION_WINODW_CONF_PATH "/DATA/config/windows_config.config"
#define CLIENT_REDEMPTION_SHARE_PATH "/DATA/share"
#define CLIENT_REDEMPTION_CB_FILE_TEMP_PATH "/DATA/clipboard_temp"
#define CLIENT_REDEMPTION_KEY_SETTING_PATH "/DATA/config/keySetting.config"
#define CLIENT_REDEMPTION_USER_CONF_PATH "/DATA/config/userConfig.config"
#define CLIENT_REDEMPTION_SOUND_TEMP_PATH "DATA/sound_temp"

#define CLIENT_REDEMPTION_DATA_PATH "/DATA"
#define CLIENT_REDEMPTION_DATA_CONF_PATH "/DATA/config"

#ifndef CLIENT_REDEMPTION_MAIN_PATH
//# error "undefined CLIENT_REDEMPTION_MAIN_PATH macro"
# define CLIENT_REDEMPTION_MAIN_PATH ""
#endif



class ClientRedemptionAPI : public FrontAPI
{
public:
    const std::string    MAIN_DIR;
    const std::string    REPLAY_DIR;
    const std::string    USER_CONF_LOG;
    const std::string    WINDOWS_CONF;
    const std::string    CB_TEMP_DIR;
    std::string          SHARE_DIR;
    const std::string    USER_CONF_DIR;
    const std::string    SOUND_TEMP_DIR;
    const std::string    DATA_DIR;
    const std::string    DATA_CONF_DIR;

    mod_api            * mod;
    ClientInfo           info;

    //  Remote App
    std::string source_of_ExeOrFile;
    std::string source_of_WorkingDir;
    std::string source_of_Arguments;
    std::string full_cmd_line;

    struct MouseData {
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;



    struct WindowsData {
        int form_x = 0;
        int form_y = 0;
        int screen_x = 0;
        int screen_y = 0;

        bool no_data = true;

    } windowsData;

    enum : int {
        COMMAND_VALID = 15
      , NAME_GOT      = 1
      , PWD_GOT       = 2
      , IP_GOT        = 4
      , PORT_GOT      = 8
    };

    enum : uint8_t {
        MOD_RDP            = 1,
        MOD_VNC            = 2,
        MOD_RDP_REMOTE_APP = 3,
        MOD_RDP_REPLAY     = 4
    };

    enum : int {
        BALISED_FRAME = 15,
        MAX_ACCOUNT_DATA = 15
    };

    struct AccountData {
        std::string title;
        std::string IP;
        std::string name;
        std::string pwd;
        int port = 0;
        int options_profil = 0;
        int index = -1;
        int protocol = MOD_RDP;
    }    _accountData[MAX_ACCOUNT_DATA];
    int  _accountNB;
    bool _save_password_account;
    int  _last_target_index;

    int current_user_profil;

    uint8_t mod_state;

    struct UserProfil {
        int id;
        std::string name;

        UserProfil(int id, std::string name)
          : id(id)
          , name(std::move(name)) {}
    };
    std::vector<UserProfil> userProfils;

    bool                 enable_shared_clipboard;
    bool                 enable_shared_virtual_disk;

    struct KeyCustomDefinition {
        int qtKeyID;
        int scanCode;
        std::string ASCII8;
        int extended;
        std::string name;

        KeyCustomDefinition(int qtKeyID, int scanCode, std::string ASCII8, int extended, std::string name)
          : qtKeyID(qtKeyID)
          , scanCode(scanCode)
          , ASCII8(std::move(ASCII8))
          , extended(extended ? 0x0100 : 0)
          , name(std::move(name))
        {}
    };
    std::vector<KeyCustomDefinition> keyCustomDefinitions;

    bool                 _recv_disconnect_ultimatum;


    struct IconMovieData {
        const std::string file_name;
        const std::string file_path;
        const std::string file_version;
        const std::string file_resolution;
        const std::string file_checksum;
        const long int movie_len = 0;

        IconMovieData(std::string file_name,
                      std::string file_path,
                      std::string file_version,
                      std::string file_resolution,
                      std::string file_checksum,
                      long int movie_len)
            : file_name(std::move(file_name))
            , file_path(std::move(file_path))
            , file_version(std::move(file_version))
            , file_resolution(std::move(file_resolution))
            , file_checksum(std::move(file_checksum))
            , movie_len(movie_len)
            {}
    };
    std::vector<IconMovieData> icons_movie_data;


    // VNC mod
    struct ModVNCParamsData {
        bool is_apple;
        Theme      theme;
        WindowListCaps windowListCaps;
        ClientExecute exe;
        std::string vnc_encodings;
        int keylayout = 0x040C;
        int width = 800;
        int height = 600;

        bool enable_tls = false;
        bool enable_nla = false;
        bool enable_sound = false;
        bool enable_shared_clipboard = false;

        std::vector<UserProfil> userProfils;
        int current_user_profil = 0;

        ModVNCParamsData(SessionReactor& session_reactor, ClientRedemptionAPI & client)
          : is_apple(false)
          , exe(session_reactor, client, this->windowListCaps, false)
          , vnc_encodings("5,16,0,1,-239")
        {}
    } vnc_conf;

    struct ModRDPParamsData
    {
        int rdp_width;
        int rdp_height;
        bool enable_tls   = false;
        bool enable_nla   = false;
        bool enable_sound = false;
    } modRDPParamsData;

    bool                 is_recording;
    bool                 is_spanning;

    int rdp_width;
    int rdp_height;

    bool                 is_full_capturing;
    bool                 is_full_replaying;
    std::string          full_capture_file_name;
    bool                 is_replaying;
    bool                 is_loading_replay_mod;
    bool                 connected;

    std::string _movie_name;
    std::string _movie_dir;
    std::string _movie_full_path;

    uint8_t           connection_info_cmd_complete;

    std::string       user_name;
    std::string       user_password;
    std::string       target_IP;
    int               port;
    BGRPalette           mod_palette;



    ClientRedemptionAPI(SessionReactor& session_reactor)
    : MAIN_DIR(CLIENT_REDEMPTION_MAIN_PATH)
    , REPLAY_DIR(CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_REPLAY_PATH)
    , USER_CONF_LOG(CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_LOGINS_PATH)
    , WINDOWS_CONF(CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_WINODW_CONF_PATH)
    , CB_TEMP_DIR(MAIN_DIR + CLIENT_REDEMPTION_CB_FILE_TEMP_PATH)
    , SHARE_DIR(MAIN_DIR + CLIENT_REDEMPTION_SHARE_PATH)
    , USER_CONF_DIR(MAIN_DIR + CLIENT_REDEMPTION_USER_CONF_PATH)
    , SOUND_TEMP_DIR(CLIENT_REDEMPTION_SOUND_TEMP_PATH)
    , DATA_DIR(MAIN_DIR + CLIENT_REDEMPTION_DATA_PATH)
    , DATA_CONF_DIR(MAIN_DIR + CLIENT_REDEMPTION_DATA_CONF_PATH)
    , mod(nullptr)
    , info()
    , _accountNB(0)
    , _save_password_account(false)
    , _last_target_index(0)
    , current_user_profil(0)
    , mod_state(MOD_RDP)
    , enable_shared_clipboard(true)
    , enable_shared_virtual_disk(true)
    , vnc_conf(session_reactor, *(this))
    , is_recording(false)
    , is_spanning(false)
    , is_replaying(false)
    , is_loading_replay_mod(false)
    , connected(false)
    , is_full_capturing(false)
    , is_full_replaying(false)
    , connection_info_cmd_complete(PORT_GOT)
    , port(3389)
    , mod_palette(BGRPalette::classic_332())
    {}

    private:
    void parse_options(int argc, char const* const argv[]);

public:
    virtual ~ClientRedemptionAPI() = default;

    virtual void send_clipboard_format() {}

    void send_to_channel( const CHANNELS::ChannelDef & , uint8_t const *
                        , std::size_t , std::size_t , int ) override {}


    // CONTROLLER
    virtual void connect() {}
    virtual void disconnect(std::string const &, bool) {}
    virtual void replay(const std::string &, const std::string &) {}
    virtual bool load_replay_mod(std::string const &, std::string const &, timeval, timeval) { return true; }
    virtual timeval reload_replay_mod(int, timeval) { return timeval{}; }
    virtual bool is_replay_on() { return true; }
    virtual char const * get_mwrm_filename() { return ""; }
    virtual time_t get_real_time_movie_begin() { return time_t{}; }
    virtual void delete_replay_mod() {}
    virtual void callback(bool /*is_timeout*/) {}
    virtual void draw_frame(int ) {}
    virtual void closeFromScreen() {}
    virtual void disconnexionReleased() {}
    virtual void replay_set_pause(timeval) {}
    virtual void replay_set_sync() {}
    virtual void mouseButtonEvent(int , int , int) {}
    virtual void wheelEvent(int ,  int , int) {}
    virtual bool mouseMouveEvent(int , int ) {}
    virtual void send_rdp_scanCode(int, int) {}
    virtual void send_rdp_unicode(uint16_t, uint16_t) {}
    virtual void refreshPressed() {}
    virtual void CtrlAltDelPressed() {}
    virtual void CtrlAltDelReleased() {}

    virtual void update_keylayout() {}

    bool can_be_start_capture() override { return true; }



    // SET CONFIG FUNCTIONS
    virtual void openWindowsData() {}
    virtual void setClientInfo() {}
    virtual void writeWindowsData() {}
    virtual void setUserProfil() {}
    virtual void setCustomKeyConfig() {}
    virtual void writeCustomKeyConfig() {}
    virtual void add_key_custom_definition(int, int, const std::string &, int, const std::string &) {}
    virtual void setAccountData() {}
    virtual void writeAccoundData(const std::string &, const std::string &, const std::string &, const int) {}
    virtual std::vector<IconMovieData> get_icon_movie_data() {}
    virtual void set_remoteapp_cmd_line(const std::string &) {}
    virtual bool is_no_win_data() {}
    virtual void deleteCurrentProtile() {}
    virtual void setDefaultConfig() {}
    virtual void writeClientInfo() {}


    virtual time_t get_movie_time_length(char const *) {}
    virtual void instant_play_client(std::chrono::microseconds) {}

};






class ClientIO
{
public:
    ClientRedemptionAPI * client;

    void set_client(ClientRedemptionAPI * client) {
        this->client = client;
    }
};


class ClientIOClipboardAPI : public ClientIO {

public:
    enum : int {
        FILEGROUPDESCRIPTORW_BUFFER_TYPE = 0,
        IMAGE_BUFFER_TYPE                = 1,
        TEXT_BUFFER_TYPE                 = 2
    };

    uint16_t    _bufferTypeID;
    int         _bufferTypeNameIndex;
    bool        _local_clipboard_stream;
    size_t      _cliboard_data_length;
    int         _cItems;

public:
    ClientIOClipboardAPI()
      : _bufferTypeID(0)
      , _bufferTypeNameIndex(0)
      , _local_clipboard_stream(true)
      , _cliboard_data_length(0)
      , _cItems(0)
      {}

    virtual ~ClientIOClipboardAPI() = default;

    // control state
    virtual void emptyBuffer() = 0;

    void set_local_clipboard_stream(bool val) {
        this->_local_clipboard_stream = val;
    }

    bool get_local_clipboard_stream() {
        return this->_local_clipboard_stream;
    }

    //  set distant clipboard data
    virtual void setClipboard_text(std::string const& str) = 0;
    virtual void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const int bpp) = 0;
    virtual void setClipboard_files(std::string const& name) = 0;
    virtual void write_clipboard_temp_file(std::string const& fileName, const uint8_t * data, size_t data_len) = 0;


    //  get local clipboard data
    uint16_t get_buffer_type_id() {
        return this->_bufferTypeID;
    }

    size_t get_cliboard_data_length() {
        return _cliboard_data_length;
    }

    int get_buffer_type_long_name() {
        return this->_bufferTypeNameIndex;
    }

    virtual ConstImageDataView get_image()
    {
        return ConstImageDataView(
            reinterpret_cast<uint8_t const*>(""),
            0, 0, 0, ConstImageDataView::BitsPerPixel{},
            ConstImageDataView::Storage::TopToBottom
        );
    }

    virtual uint8_t * get_text() = 0;

    // files data (file index to identify a file among a files group descriptor)
    virtual std::string get_file_item_name(int index) {(void)index; return {};}

    // TODO should be `array_view_const_char get_file_item_size(int index)`
    virtual  int get_file_item_size(int index) {(void) index; return 0;}
    virtual char * get_file_item_data(int index) {(void) index; /*TODO char const/string_view*/ return const_cast<char*>("");}

    int get_citems_number() {
        return this->_cItems;
    }
};



#include "core/FSCC/FileInformation.hpp"

constexpr long long WINDOWS_TICK = 10000000;
constexpr long long SEC_TO_UNIX_EPOCH = 11644473600LL;

class ClientIODiskAPI : public ClientIO {


public:
    virtual ~ClientIODiskAPI() = default;

    struct FileStat {

        uint64_t LastAccessTime = 0;
        uint64_t LastWriteTime  = 0;
        uint64_t CreationTime   = 0;
        uint64_t ChangeTime     = 0;
        uint32_t FileAttributes = 0;

        int64_t  AllocationSize = 0;
        int64_t  EndOfFile      = 0;
        uint32_t NumberOfLinks  = 0;
        uint8_t  DeletePending  = 0;
        uint8_t  Directory      = 0;
    };

    struct FileStatvfs {

        uint64_t VolumeCreationTime             = 0;
        const char * VolumeLabel                = "";
        const char * FileSystemName             = "ext4";

        uint32_t FileSystemAttributes           = fscc::NEW_FILE_ATTRIBUTES;
        uint32_t SectorsPerAllocationUnit       = 8;

        uint32_t BytesPerSector                 = 0;
        uint32_t MaximumComponentNameLength     = 0;
        uint64_t TotalAllocationUnits           = 0;
        uint64_t CallerAvailableAllocationUnits = 0;
        uint64_t AvailableAllocationUnits       = 0;
        uint64_t ActualAvailableAllocationUnits = 0;
    };




    unsigned WindowsTickToUnixSeconds(long long windowsTicks) {
        return unsigned((windowsTicks / WINDOWS_TICK) - SEC_TO_UNIX_EPOCH);
    }

    long long UnixSecondsToWindowsTick(unsigned unixSeconds) {
        return ((unixSeconds + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK);
    }

    //  TODO put this somewhere
    //this->parse_options(argc, argv);

    uint32_t string_to_hex32(unsigned char * str) {
        size_t size = sizeof(str);
        uint32_t hex32(0);
        for (size_t i = 0; i < size; i++) {
            int s = str[i];
            if(s > 47 && s < 58) {                      //this covers 0-9
                hex32 += (s - 48) << (size - i - 1);
            } else if (s > 64 && s < 71) {              // this covers A-F
                hex32 += (s - 55) << (size - i - 1);
            } else if (s > 'a'-1 && s < 'f'+1) {        // this covers a-f
                hex32 += (s - 'a') << (size - i - 1);
            }
        }
        return hex32;
    }

    virtual bool ifile_good(const char * new_path) = 0;

    virtual bool ofile_good(const char * new_path) = 0;

    virtual bool dir_good(const char * new_path) = 0;

    virtual void marke_dir(const char * new_path) = 0;

    virtual FileStat get_file_stat(const char * file_to_request) = 0;

    virtual FileStatvfs get_file_statvfs(const char * file_to_request) = 0;

    // TODO `log_error_on` is suspecious
    virtual erref::NTSTATUS read_data(
        std::string const& file_to_tread, int offset, byte_array data,
        bool log_error_on) = 0;

    virtual bool set_elem_from_dir(std::vector<std::string> & elem_list, const std::string & str_dir_path) = 0;

    virtual int get_device(const char * file_path) = 0;

    virtual uint32_t get_volume_serial_number(int device) = 0;

    virtual bool write_file(const char * file_to_write, const char * data, int data_len) = 0;

    virtual bool remove_file(const char * file_to_remove) = 0;

    virtual bool rename_file(const char * file_to_rename,  const char * new_name) = 0;
};



class ClientOutputSoundAPI : public ClientIO {

public:
    uint32_t n_sample_per_sec = 0;
    uint16_t bit_per_sample = 0;
    uint16_t n_channels = 0;
    uint16_t n_block_align = 0;
    uint32_t bit_per_sec = 0;

    std::string path;

    void set_path(const std::string & path) {
        this->path = path;
    }

    virtual void init(size_t raw_total_size) = 0;
    virtual void setData(const uint8_t * data, size_t size) = 0;
    virtual void play() = 0;

    virtual ~ClientOutputSoundAPI() = default;

};



class ClientInputSocketAPI : public ClientIO {

public:
    mod_api * _callback = nullptr;

    virtual bool start_to_listen(int client_sck, mod_api * mod) = 0;
    virtual void disconnect() = 0;

    virtual ~ClientInputSocketAPI() = default;
};



class ClientInputMouseKeyboardAPI : public ClientIO {


public:

    ClientInputMouseKeyboardAPI() = default;

    virtual ~ClientInputMouseKeyboardAPI() = default;


    virtual ClientRedemptionAPI * get_client() {
        return this->client;
    }

    virtual void update_keylayout() = 0;

    virtual void init_form() = 0;

    virtual void pre_load_movie() {}


    // CONTROLLER
    virtual void connexionReleased() {
        this->client->connect();

    }

    void add_key_custom_definition(int qtKeyID, int scanCode, const std::string & ASCII8, int extended, const std::string & name) {
        //this->keyCustomDefinitions.emplace_back(qtKeyID, scanCode, ASCII8, extended, name);
    }

    virtual void disconnexionReleased() {
        this->client->disconnexionReleased();
    }

    void CtrlAltDelPressed() {
        this->client->CtrlAltDelPressed();
    }

    void CtrlAltDelReleased() {
        this->client->CtrlAltDelReleased();
    }

    virtual void mouseButtonEvent(int x, int y, int flag) {
        this->client->mouseButtonEvent(x, y, flag);
    }

    virtual void wheelEvent(int x,  int y, int delta) {
        this->client->wheelEvent(x, y, delta);
    }

    virtual bool mouseMouveEvent(int x, int y) {
        return this->client->mouseMouveEvent(x, y);
    }

    // TODO string_view
    void virtual keyPressEvent(const int key, std::string const& text)  = 0;

    // TODO string_view
    void virtual keyReleaseEvent(const int key, std::string const& text)  = 0;

    void virtual refreshPressed() {
        this->client->refreshPressed();
    }

    virtual void open_options() {}

    // TODO string_view
    virtual ClientRedemptionAPI::KeyCustomDefinition get_key_info(int, std::string const&) {
        return ClientRedemptionAPI::KeyCustomDefinition(0, 0, "", 0, "");
    }

};



class ClientOutputGraphicAPI {

public:
    ClientRedemptionAPI * drawn_client;

    const int screen_max_width;
    const int screen_max_height;

    bool is_pre_loading;

    ClientOutputGraphicAPI(int max_width, int max_height)
      : drawn_client(nullptr),
		screen_max_width(max_width)
      , screen_max_height(max_height)
      , is_pre_loading(false) {
    }

    virtual ~ClientOutputGraphicAPI() = default;

    virtual void set_drawn_client(ClientRedemptionAPI * client) {
        this->drawn_client = client;
    }

    virtual void set_ErrorMsg(std::string const & movie_path) = 0;

    virtual void dropScreen() = 0;

    virtual void show_screen() = 0;

    virtual void reset_cache(int w,  int h) = 0;

    virtual void create_screen() = 0;

    virtual void closeFromScreen() = 0;

    virtual void set_screen_size(int x, int y) = 0;

    virtual void update_screen() = 0;


    // replay mod

    virtual void create_screen(std::string const & , std::string const & ) {}

    virtual void draw_frame(int ) {}


    // remote app

    virtual void create_remote_app_screen(uint32_t , int , int , int , int ) {}

    virtual void move_screen(uint32_t , int , int ) {}

    virtual void set_screen_size(uint32_t , int , int ) {}

    virtual void set_pixmap_shift(uint32_t , int , int ) {}

    virtual int get_visible_width(uint32_t ) {return 0;}

    virtual int get_visible_height(uint32_t ) {return 0;}

    virtual int get_mem_width(uint32_t ) {return 0;}

    virtual int get_mem_height(uint32_t ) {return 0;}

    virtual void set_mem_size(uint32_t , int , int ) {}

    virtual void show_screen(uint32_t ) {}

    virtual void dropScreen(uint32_t ) {}

    virtual void clear_remote_app_screen() {}




    virtual FrontAPI::ResizeResult server_resize(int width, int height, int bpp) = 0;

    virtual void set_pointer(Pointer      const &) {}

    virtual void draw(RDP::FrameMarker    const & cmd) = 0;
    virtual void draw(RDPNineGrid const & , Rect , gdi::ColorCtx , Bitmap const & ) = 0;
    virtual void draw(RDPDestBlt          const & cmd, Rect clip) = 0;
    virtual void draw(RDPMultiDstBlt      const & cmd, Rect clip) = 0;
    virtual void draw(RDPScrBlt           const & cmd, Rect clip) = 0;
    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) = 0;
    virtual void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) = 0;

    virtual void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) = 0;
    virtual void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) = 0;


    // TODO The 2 methods below should not exist and cache access be done before calling drawing orders
//     virtual void draw(RDPColCache   const &) {}
//     virtual void draw(RDPBrushCache const &) {}

    virtual void begin_update() {}
    virtual void end_update() {}
};


