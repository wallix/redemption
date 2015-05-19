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
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#ifndef _REDEMPTION_MOD_RDP_RDP_HPP_
#define _REDEMPTION_MOD_RDP_RDP_HPP_

#include "rdp/rdp_orders.hpp"

/* include "ther h files */
#include "stream.hpp"
#include "ssl_calls.hpp"
#include "mod_api.hpp"
#include "auth_api.hpp"
#include "front_api.hpp"

#include "RDP/x224.hpp"
#include "RDP/nego.hpp"
#include "RDP/mcs.hpp"
#include "RDP/lic.hpp"
#include "RDP/logon.hpp"
#include "channel_list.hpp"
#include "RDP/gcc.hpp"
#include "RDP/sec.hpp"
#include "colors.hpp"
#include "RDP/autoreconnect.hpp"
#include "RDP/ServerRedirection.hpp"
#include "RDP/bitmapupdate.hpp"
#include "RDP/clipboard.hpp"
#include "RDP/fastpath.hpp"
#include "RDP/PersistentKeyListPDU.hpp"
#include "RDP/protocol.hpp"
#include "RDP/RefreshRectPDU.hpp"
#include "RDP/SaveSessionInfoPDU.hpp"
#include "RDP/pointer.hpp"
#include "RDP/mppc_unified_dec.hpp"
#include "RDP/capabilities/cap_bitmap.hpp"
#include "RDP/capabilities/order.hpp"
#include "RDP/capabilities/bmpcache.hpp"
#include "RDP/capabilities/bmpcache2.hpp"
#include "RDP/capabilities/colcache.hpp"
#include "RDP/capabilities/activate.hpp"
#include "RDP/capabilities/control.hpp"
#include "RDP/capabilities/pointer.hpp"
#include "RDP/capabilities/cap_share.hpp"
#include "RDP/capabilities/input.hpp"
#include "RDP/capabilities/cap_sound.hpp"
#include "RDP/capabilities/cap_font.hpp"
#include "RDP/capabilities/glyphcache.hpp"
#include "RDP/capabilities/rail.hpp"
#include "RDP/capabilities/window.hpp"
#include "RDP/channels/rdpdr.hpp"
#include "RDP/channels/rdpdr_file_system_drive_manager.hpp"
#include "RDP/remote_programs.hpp"
#include "rdp_params.hpp"
#include "transparentrecorder.hpp"
#include "FSCC/FileInformation.hpp"

#include "cast.hpp"
#include "client_info.hpp"
#include "genrandom.hpp"
#include "authorization_channels.hpp"
#include "parser.hpp"
#include "channel_names.hpp"
#include "finally.hpp"
#include "apply_for_delim.hpp"
#include "timeout.hpp"

class mod_rdp : public mod_api {
    FrontAPI & front;

    CHANNELS::ChannelDefArray mod_channel_list;

    const AuthorizationChannels authorization_channels;

    typedef int_fast32_t data_size_type;
    data_size_type max_clipboard_data = 0;
    data_size_type total_clipboard_data = 0;
    data_size_type max_rdpdr_data = 0;
    data_size_type total_rdpdr_data = 0;

    int  use_rdp5;

    int  keylayout;

    uint8_t   lic_layer_license_key[16];
    uint8_t   lic_layer_license_sign_key[16];
    uint8_t * lic_layer_license_data;
    size_t    lic_layer_license_size;

    rdp_orders orders;

    int      share_id;
    uint16_t userid;

    char hostname[16];
    char username[128];
    char password[256];
    char domain[256];
    char program[512];
    char directory[512];

    char client_name[128];

    uint8_t bpp;

    int encryptionLevel;
    int encryptionMethod;

    const int    key_flags;

    uint32_t     server_public_key_len;
    uint8_t      client_crypt_random[512];
    CryptContext encrypt, decrypt;

    enum {
          MOD_RDP_NEGO
        , MOD_RDP_BASIC_SETTINGS_EXCHANGE
        , MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER
        , MOD_RDP_GET_LICENSE
        , MOD_RDP_CONNECTED
    };

    enum {
        EARLY,
        WAITING_SYNCHRONIZE,
        WAITING_CTL_COOPERATE,
        WAITING_GRANT_CONTROL_COOPERATE,
        WAITING_FONT_MAP,
        UP_AND_RUNNING
    } connection_finalization_state;

    int state;
    Pointer cursors[32];
    const bool console_session;
    const uint8_t front_bpp;
    const uint32_t performanceFlags;
    Random & gen;
    const uint32_t verbose;
    const uint32_t cache_verbose;

    char auth_channel[8];
    int  auth_channel_flags;
    int  auth_channel_chanid;
    //int  auth_channel_state;    // 0 means unused, 1 means session running

    auth_api * acl;

    RdpNego nego;

    char clientAddr[512];

    const bool enable_fastpath;                    // choice of programmer
          bool enable_fastpath_client_input_event; // choice of programmer + capability of server
    const bool enable_fastpath_server_update;      // = choice of programmer
    const bool enable_glyph_cache;
    const bool enable_wab_agent;
    const bool enable_mem3blt;
    const bool enable_new_pointer;
    const bool enable_transparent_mode;
    const bool enable_persistent_disk_bitmap_cache;
    const bool enable_cache_waiting_list;
    const int  rdp_compression;
    const bool persist_bitmap_cache_on_disk;


    size_t recv_bmp_update;

    rdp_mppc_unified_dec mppc_dec;

    std::string * error_message;

    const bool     disconnect_on_logon_user_change;
    const uint32_t open_session_timeout;

    TimeoutT<time_t> open_session_timeout_checker;

    const uint64_t          client_device_list_announce_timeout         = 1000000;  // Timeout in microseconds.
          bool              client_device_list_announce_timer_enabled   = false;
          TimeoutT<TimeVal> client_device_list_announce_timeout_checker;

    std::string output_filename;

    std::string end_session_reason;
    std::string end_session_message;

    const unsigned certificate_change_action;

    bool enable_polygonsc;
    bool enable_polygoncb;
    bool enable_polyline;
    bool enable_ellipsesc;
    bool enable_ellipsecb;
    bool enable_multidstblt;
    bool enable_multiopaquerect;
    bool enable_multipatblt;
    bool enable_multiscrblt;

    const bool remote_program;

    bool server_redirection_support;

    TransparentRecorder * transparent_recorder;
    Transport           * persistent_key_list_transport;

    //uint64_t total_data_received;

    const uint32_t password_printing_mode;

    bool deactivation_reactivation_in_progress;

    typedef std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>
        device_io_request_collection_type; // DeviceId, CompletionId, MajorFunction, (extra data).
    device_io_request_collection_type device_io_requests;

    FileSystemDriveManager file_system_drive_manager;
    bool                   device_capability_version_02_supported = false;
    bool                   proxy_managed_rdpdr_channel            = false;  // Otherwise, the channel is managed by client.

    RedirectionInfo & redir_info;

    const uint32_t max_chunked_virtual_channel_data_length;

    std::unique_ptr<uint8_t[]> chunked_virtual_channel_data_byte;
    FixedSizeStream            chunked_virtual_channel_data_stream;

    static const uint32_t default_chunked_virtual_channel_data_length = 1024 * 64;

    const bool bogus_sc_net_size;

    std::string real_alternate_shell;
    std::string real_working_dir;

public:
    mod_rdp( Transport & trans
           , FrontAPI & front
           , const ClientInfo & info
           , RedirectionInfo & redir_info
           , Random & gen
           , const ModRDPParams & mod_rdp_params
           )
        : mod_api(info.width - (info.width % 4), info.height)
        , front(front)
        , authorization_channels(
            mod_rdp_params.allow_channels ? *mod_rdp_params.allow_channels : std::string{},
            mod_rdp_params.deny_channels ? *mod_rdp_params.deny_channels : std::string{}
          )
        , use_rdp5(1)
        , keylayout(info.keylayout)
        , orders( mod_rdp_params.target_host, mod_rdp_params.enable_persistent_disk_bitmap_cache
                , mod_rdp_params.persist_bitmap_cache_on_disk, mod_rdp_params.verbose)
        , share_id(0)
        , userid(0)
        , bpp(0)
        , encryptionLevel(0)
        , key_flags(mod_rdp_params.key_flags)
        , server_public_key_len(0)
        , connection_finalization_state(EARLY)
        , state(MOD_RDP_NEGO)
        , console_session(info.console_session)
        , front_bpp(info.bpp)
        , performanceFlags(info.rdp5_performanceflags)
        , gen(gen)
        , verbose(mod_rdp_params.verbose)
        , cache_verbose(mod_rdp_params.cache_verbose)
        , auth_channel_flags(0)
        , auth_channel_chanid(0)
        //, auth_channel_state(0) // 0 means unused
        , acl(mod_rdp_params.acl)
        , nego( mod_rdp_params.enable_tls, trans, mod_rdp_params.target_user
              , mod_rdp_params.enable_nla, mod_rdp_params.target_host
              , mod_rdp_params.enable_krb, mod_rdp_params.verbose)
        , enable_fastpath(mod_rdp_params.enable_fastpath)
        , enable_fastpath_client_input_event(false)
        , enable_fastpath_server_update(mod_rdp_params.enable_fastpath)
        , enable_glyph_cache(mod_rdp_params.enable_glyph_cache)
        , enable_wab_agent(mod_rdp_params.enable_wab_agent)
        , enable_mem3blt(mod_rdp_params.enable_mem3blt)
        , enable_new_pointer(mod_rdp_params.enable_new_pointer)
        , enable_transparent_mode(mod_rdp_params.enable_transparent_mode)
        , enable_persistent_disk_bitmap_cache(mod_rdp_params.enable_persistent_disk_bitmap_cache)
        , enable_cache_waiting_list(mod_rdp_params.enable_cache_waiting_list)
        , rdp_compression(mod_rdp_params.rdp_compression)
        , persist_bitmap_cache_on_disk(mod_rdp_params.persist_bitmap_cache_on_disk)
        , recv_bmp_update(0)
        , error_message(mod_rdp_params.error_message)
        , disconnect_on_logon_user_change(mod_rdp_params.disconnect_on_logon_user_change)
        , open_session_timeout(mod_rdp_params.open_session_timeout)
        , open_session_timeout_checker(0)
        , client_device_list_announce_timeout_checker(0)
        , output_filename(mod_rdp_params.output_filename)
        , certificate_change_action(mod_rdp_params.certificate_change_action)
        , enable_polygonsc(false)
        , enable_polygoncb(false)
        , enable_polyline(false)
        , enable_ellipsesc(false)
        , enable_ellipsecb(false)
        , enable_multidstblt(false)
        , enable_multiopaquerect(false)
        , enable_multipatblt(false)
        , enable_multiscrblt(false)
        , remote_program(mod_rdp_params.remote_program)
        , server_redirection_support(mod_rdp_params.server_redirection_support)
        , transparent_recorder(nullptr)
        , persistent_key_list_transport(mod_rdp_params.persistent_key_list_transport)
        //, total_data_received(0)
        , password_printing_mode(mod_rdp_params.password_printing_mode)
        , deactivation_reactivation_in_progress(false)
        , redir_info(redir_info)
        , max_chunked_virtual_channel_data_length(std::min(mod_rdp_params.max_chunked_virtual_channel_data_length,
            CHANNELS::PROXY_CHUNKED_VIRTUAL_CHANNEL_DATA_LENGTH_LIMIT))
        , chunked_virtual_channel_data_byte(std::make_unique<uint8_t[]>(mod_rdp::default_chunked_virtual_channel_data_length))
        , chunked_virtual_channel_data_stream(chunked_virtual_channel_data_byte.get(), mod_rdp::default_chunked_virtual_channel_data_length)
        , bogus_sc_net_size(mod_rdp_params.bogus_sc_net_size)
    {
        if (this->verbose & 1) {
            if (!enable_transparent_mode) {
                LOG(LOG_INFO, "Creation of new mod 'RDP'");
            }
            else {
                LOG(LOG_INFO, "Creation of new mod 'RDP Transparent'");

                if (this->output_filename.empty()) {
                    LOG(LOG_INFO, "Use transparent capabilities.");
                }
                else {
                    LOG(LOG_INFO, "Use proxy default capabilities.");
                }
            }

            mod_rdp_params.log();
        }

        this->chunked_virtual_channel_data_stream.reset();

        if (this->enable_wab_agent) {
            this->file_system_drive_manager.EnableWABAgentDrive();
        }

        if (mod_rdp_params.transparent_recorder_transport) {
            this->transparent_recorder = new TransparentRecorder(mod_rdp_params.transparent_recorder_transport);
        }

        this->configure_extra_orders(mod_rdp_params.extra_orders);

        this->event.object_and_time = (this->open_session_timeout > 0);

        memset(this->auth_channel, 0, sizeof(this->auth_channel));
        strncpy(this->auth_channel, mod_rdp_params.auth_channel, sizeof(this->auth_channel) - 1);

        memset(this->clientAddr, 0, sizeof(this->clientAddr));
        strncpy(this->clientAddr, mod_rdp_params.client_address, sizeof(this->clientAddr) - 1);
        this->lic_layer_license_data = nullptr;
        this->lic_layer_license_size = 0;
        memset(this->lic_layer_license_key, 0, 16);
        memset(this->lic_layer_license_sign_key, 0, 16);
        TODO("CGR: license loading should be done before creating protocol layers");
        struct stat st;
        char path[256];
        snprintf(path, sizeof(path), LICENSE_PATH "/license.%s", info.hostname);
        int fd = open(path, O_RDONLY);
        if (fd != -1){
            if (fstat(fd, &st) != 0){
                this->lic_layer_license_data = (uint8_t *)malloc(this->lic_layer_license_size);
                if (this->lic_layer_license_data){
                    size_t lic_size = read(fd, this->lic_layer_license_data, this->lic_layer_license_size);
                    if (lic_size != this->lic_layer_license_size){
                        LOG(LOG_ERR, "license file truncated : expected %u, got %u", this->lic_layer_license_size, lic_size);
                    }
                }
            }
            close(fd);
        }

        // from rdp_sec
        memset(this->client_crypt_random, 0, sizeof(this->client_crypt_random));

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        this->decrypt.encryptionMethod = 2; /* 128 bits */
        this->encrypt.encryptionMethod = 2; /* 128 bits */

        if (::strlen(info.hostname) >= sizeof(this->hostname)) {
            LOG(LOG_WARNING, "mod_rdp: hostname too long! %u >= %u", ::strlen(info.hostname), sizeof(this->hostname));
        }
        strncpy(this->hostname, info.hostname, 15);
        this->hostname[15] = 0;


        const char * domain_pos   = nullptr;
        size_t       domain_len   = 0;
        const char * username_pos = nullptr;
        size_t       username_len = 0;
        const char * separator = strchr(mod_rdp_params.target_user, '\\');
        if (separator)
        {
            domain_pos   = mod_rdp_params.target_user;
            domain_len   = separator - mod_rdp_params.target_user;
            username_pos = ++separator;
            username_len = strlen(username_pos);
        }
        else
        {
            separator = strchr(mod_rdp_params.target_user, '@');
            if (separator)
            {
                domain_pos   = separator + 1;
                domain_len   = strlen(domain_pos);
                username_pos = mod_rdp_params.target_user;
                username_len = separator - mod_rdp_params.target_user;
                LOG(LOG_INFO, "mod_rdp: username_len=%u", username_len);
            }
            else
            {
                username_pos = mod_rdp_params.target_user;
                username_len = strlen(username_pos);
            }
        }

        if (username_len >= sizeof(this->username)) {
            LOG(LOG_INFO, "mod_rdp: username too long! %u >= %u", username_len, sizeof(this->username));
        }
        size_t count = std::min(sizeof(this->username) - 1, username_len);
        if (count > 0) strncpy(this->username, username_pos, count);
        this->username[count] = 0;

        if (domain_len >= sizeof(this->domain)) {
            LOG(LOG_INFO, "mod_rdp: domain too long! %u >= %u", domain_len, sizeof(this->domain));
        }
        count = std::min(sizeof(this->domain) - 1, domain_len);
        if (count > 0) strncpy(this->domain, domain_pos, count);
        this->domain[count] = 0;

        LOG(LOG_INFO, "Remote RDP Server domain=\"%s\" login=\"%s\" host=\"%s\"",
            this->domain, this->username, this->hostname);


        strncpy(this->password, mod_rdp_params.target_password, sizeof(this->password) - 1);
        this->password[sizeof(this->password) - 1] = 0;

        snprintf(this->client_name, sizeof(this->client_name), "%s", mod_rdp_params.client_name);

        std::string alternate_shell(mod_rdp_params.alternate_shell);
        if (mod_rdp_params.target_application_account && *mod_rdp_params.target_application_account) {
            const char * user_marker = "${USER}";
            size_t pos = alternate_shell.find(user_marker, 0);
            if (pos != std::string::npos) {
                alternate_shell.replace(pos, strlen(user_marker), mod_rdp_params.target_application_account);
            }
        }
        if (mod_rdp_params.target_application_password && *mod_rdp_params.target_application_password) {
            const char * password_marker = "${PASSWORD}";
            size_t pos = alternate_shell.find(password_marker, 0);
            if (pos != std::string::npos) {
                alternate_shell.replace(pos, strlen(password_marker), mod_rdp_params.target_application_password);
            }
        }

        if (this->enable_wab_agent) {

            this->real_alternate_shell = std::move(alternate_shell);
            this->real_working_dir     = mod_rdp_params.shell_working_directory;

            const char * wab_agent_alternate_shell =
                    "cmd /c "
                    "ECHO @SET X=WABAgent.exe>S&"
                    "ECHO @SET P=\\\\TSCLIENT\\WABAGT\\%X%>>S&"
                    "ECHO :B>>S&"
                    "ECHO @IF EXIST %P% GOTO E>>S&"
                    "ECHO @PING 1 -n 1 -w 1000^>NUL>>S&"
                    "ECHO @GOTO B>>S&"
                    "ECHO :E>>S&"
                    "ECHO @COPY %P%^>NUL>>S&"
                    "ECHO @START %X%>>S&"
                    "REN S S.BAT&"
                    "S"
                ;
            const char * wab_agent_working_dir = "%TMP%";

            strncpy(this->program, wab_agent_alternate_shell, sizeof(this->program) - 1);
            this->program[sizeof(this->program) - 1] = 0;
            strncpy(this->directory, wab_agent_working_dir, sizeof(this->directory) - 1);
            this->directory[sizeof(this->directory) - 1] = 0;
        }
        else {
            strncpy(this->program, alternate_shell.c_str(), sizeof(this->program) - 1);
            this->program[sizeof(this->program) - 1] = 0;
            strncpy(this->directory, mod_rdp_params.shell_working_directory, sizeof(this->directory) - 1);
            this->directory[sizeof(this->directory) - 1] = 0;
        }

        LOG(LOG_INFO, "Server key layout is %x", this->keylayout);

        this->nego.set_identity(this->username,
                                this->domain,
                                this->password,
                                this->hostname);

        if (this->verbose & 128){
            this->redir_info.log(LOG_INFO, "Init with Redir_info");
            LOG(LOG_INFO, "ServerRedirectionSupport=%s",
                this->server_redirection_support?"true":"false");
        }
        if (this->server_redirection_support) {
            if (this->redir_info.valid && (this->redir_info.lb_info_length > 0)) {
                this->nego.set_lb_info(this->redir_info.lb_info,
                                       this->redir_info.lb_info_length);
            }
        }

        while (UP_AND_RUNNING != this->connection_finalization_state){
            this->draw_event(time(nullptr));
            if (this->event.signal != BACK_EVENT_NONE){
                char statestr[256];
                switch (this->state) {
                case MOD_RDP_NEGO:
                    snprintf(statestr, sizeof(statestr), "RDP_NEGO");
                    break;
                case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
                    snprintf(statestr, sizeof(statestr), "RDP_BASIC_SETTINGS_EXCHANGE");
                    break;
                case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
                    snprintf(statestr, sizeof(statestr),
                             "RDP_CHANNEL_CONNECTION_ATTACH_USER");
                    break;
                case MOD_RDP_GET_LICENSE:
                    snprintf(statestr, sizeof(statestr), "RDP_GET_LICENSE");
                    break;
                case MOD_RDP_CONNECTED:
                    snprintf(statestr, sizeof(statestr), "RDP_CONNECTED");
                    break;
                default:
                    snprintf(statestr, sizeof(statestr), "UNKNOWN");
                    break;
                }
                statestr[255] = 0;
                LOG(LOG_ERR, "Creation of new mod 'RDP' failed at %s state", statestr);
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }

        if (this->acl) {
            this->acl->report("CONNECTION_SUCCESSFUL", "Ok.");
        }

        // this->end_session_reason.copy_c_str("OPEN_SESSION_FAILED");
        // this->end_session_message.copy_c_str("Open RDP session cancelled.");
    }   // mod_rdp

    virtual ~mod_rdp() {
        delete this->transparent_recorder;

        if (this->acl && !this->end_session_reason.empty() &&
            !this->end_session_message.empty()) {
            this->acl->report(this->end_session_reason.c_str(),
                this->end_session_message.c_str());
        }

        if (this->lic_layer_license_data) {
            free(this->lic_layer_license_data);
        }

        if (this->verbose & 1) {
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp cache count  = %llu",
                this->orders.recv_bmp_cache_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv order count      = %llu",
                this->orders.recv_order_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp update count = %llu",
                this->recv_bmp_update);
        }
    }

    void configure_extra_orders(const char * extra_orders) {
        if (verbose) {
            LOG(LOG_INFO, "RDP Extra orders=\"%s\"", extra_orders);
        }

        apply_for_delim(extra_orders, ',', [this](const char *order) {
            int const order_number = long_from_cstr(order);
            if (verbose) {
                LOG(LOG_INFO, "RDP Extra orders number=%d", order_number);
            }
            switch (order_number) {
            case RDP::MULTIDSTBLT:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiDstBlt");
                }
                this->enable_multidstblt = true;
                break;
            case RDP::MULTIOPAQUERECT:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiOpaqueRect");
                }
                this->enable_multiopaquerect = true;
                break;
            case RDP::MULTIPATBLT:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiPatBlt");
                }
                this->enable_multipatblt = true;
                break;
            case RDP::MULTISCRBLT:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiScrBlt");
                }
                this->enable_multiscrblt = true;
                break;
            case RDP::POLYGONSC:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=PolygonSC");
                }
                this->enable_polygonsc = true;
                break;
            case RDP::POLYGONCB:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=PolygonCB");
                }
                this->enable_polygoncb = true;
                break;
            case RDP::POLYLINE:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=Polyline");
                }
                this->enable_polyline = true;
                break;
            case RDP::ELLIPSESC:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=EllipseSC");
                }
                this->enable_ellipsesc = true;
                break;
            case RDP::ELLIPSECB:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Extra orders=EllipseCB");
                }
                this->enable_ellipsecb = true;
                break;
            default:
                if (verbose) {
                    LOG(LOG_INFO, "RDP Unknown Extra orders");
                }
                break;
            }
        }, [](char c) { return c == ' ' || c == '\t' || c == ','; });
    }   // configure_extra_orders

    virtual void rdp_input_scancode( long param1, long param2, long device_flags, long time
                                     , Keymap2 * keymap) {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
        }
    }

    virtual void rdp_input_synchronize( uint32_t time, uint16_t device_flags, int16_t param1
                                        , int16_t param2) {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_SYNCHRONIZE, device_flags, param1, 0);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_MOUSE, device_flags, x, y);
        }
    }

    virtual void send_to_front_channel( const char * const mod_channel_name, uint8_t * data
                                        , size_t length, size_t chunk_size, int flags) {
        if (this->transparent_recorder) {
            this->transparent_recorder->send_to_front_channel( mod_channel_name, data, length
                                                             , chunk_size, flags);
        }

        const CHANNELS::ChannelDef * front_channel = this->front.get_channel_list().get_by_name(mod_channel_name);
        if (front_channel) {
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
    }

private:
    template<class PDU, class... Args>
    void send_clipboard_pdu_to_front_channel(bool response_ok, Args&&... args) {
        PDU             pdu(response_ok);
        uint8_t         data[256];
        FixedSizeStream out_s(data, sizeof(data));

        pdu.emit(out_s, args...);

        this->send_to_front_channel( channel_names::cliprdr
                                   , out_s.get_data()
                                   , out_s.size()
                                   , out_s.size()
                                   , CHANNELS::CHANNEL_FLAG_FIRST
                                   | CHANNELS::CHANNEL_FLAG_LAST
                                   );
    }

    void update_total_clipboard_data(uint16_t msgType, uint32_t len) {
        if (this->max_clipboard_data == 0) {
            return ;
        }

        if (RDPECLIP::CB_FORMAT_DATA_RESPONSE == msgType || RDPECLIP::CB_FILECONTENTS_RESPONSE == msgType) {
            this->total_clipboard_data += len;
            if (this->total_clipboard_data >= this->max_clipboard_data && this->acl) {
                this->acl->report("CLIPBOARD_LIMIT", "");
                this->max_clipboard_data = 0;
            }
        }
    }

    void update_total_rdpdr_data(rdpdr::PacketId e, uint32_t len) {
        if (this->max_rdpdr_data == 0) {
            return ;
        }

        if (rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION == e) {
            this->total_rdpdr_data += len;
            if (this->total_rdpdr_data >= this->max_rdpdr_data && this->acl) {
                this->acl->report("RDPDR_LIMIT", "");
                this->max_rdpdr_data = 0;
            }
        }
    }

public:
    virtual void send_to_mod_channel( const char * const front_channel_name
                                    , Stream & chunk
                                    , size_t length
                                    , uint32_t flags) {
        if (this->verbose & 16) {
            LOG(LOG_INFO,
                "mod_rdp::send_to_mod_channel: front_channel_channel=\"%s\"",
                front_channel_name);
        }

        const CHANNELS::ChannelDef * mod_channel = this->mod_channel_list.get_by_name(front_channel_name);
        if (!mod_channel) {
            return;
        }
        if (this->verbose & 16) {
            mod_channel->log(unsigned(mod_channel - &this->mod_channel_list[0]));
        }

             if (!strcmp(front_channel_name, channel_names::cliprdr)) {
            this->send_to_mod_cliprdr_channel(mod_channel, chunk, length, flags);
        }
        else if (!strcmp(front_channel_name, channel_names::rail)) {
            this->send_to_mod_rail_channel(mod_channel, chunk, length, flags);
        }
        else if (!strcmp(front_channel_name, channel_names::rdpdr)) {
            this->send_to_mod_rdpdr_channel(mod_channel, chunk, length, flags);
        }
        else {
            this->send_to_channel(*mod_channel, chunk, length, flags);
        }
    }

private:
    void send_to_mod_cliprdr_channel(const CHANNELS::ChannelDef * cliprdr_channel,
                                     Stream & chunk, size_t length, uint32_t flags) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp client clipboard PDU");
        }

        if (!chunk.in_check_rem(2)) {
            LOG(LOG_INFO, "mod_rdp::send_to_mod_cliprdr_channel: truncated msgType, need=2 remains=%u",
                chunk.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }
        const uint16_t msgType = chunk.in_uint16_le();
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_to_mod_cliprdr_channel: client clipboard PDU: msgType=%d", msgType);
        }

        if ((msgType == RDPECLIP::CB_FORMAT_LIST)) {
            if (!this->authorization_channels.cliprdr_up_is_authorized() &&
                !this->authorization_channels.cliprdr_down_is_authorized()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp::send_to_mod_cliprdr_channel: clipboard is fully disabled (c)");
                }
                this->send_clipboard_pdu_to_front_channel<RDPECLIP::FormatListResponsePDU>(
                    true);
                return;
            }
        }
        else if (msgType == RDPECLIP::CB_FORMAT_DATA_REQUEST) {
            if (!this->authorization_channels.cliprdr_down_is_authorized()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp::send_to_mod_cliprdr_channel: clipboard down is unavailable");
                }

                this->send_clipboard_pdu_to_front_channel<RDPECLIP::FormatDataResponsePDU>(
                    false, static_cast<uint8_t *>(nullptr), 0);
                return;
            }
        }
        else if (msgType == RDPECLIP::CB_FILECONTENTS_REQUEST) {
            if (!this->authorization_channels.cliprdr_file_is_authorized()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp::send_to_mod_cliprdr_channel: requesting the contents of server file is denied");
                }
                this->send_clipboard_pdu_to_front_channel<RDPECLIP::FileContentsResponse>(
                    false);
                return;
            }
        }

        this->update_total_clipboard_data(msgType, length);

        chunk.p -= 2;

        this->send_to_channel(*cliprdr_channel, chunk, length, flags);
    }

    void send_to_mod_rail_channel(const CHANNELS::ChannelDef * rail_channel,
                                  Stream & chunk, size_t length, uint32_t flags) {
        //LOG(LOG_INFO, "mod_rdp::send_to_mod_rail_channel: chunk.size=%u length=%u",
        //    chunk.size(), length);
        //hexdump_d(chunk.get_data(), chunk.size());

        const auto saved_chunk_p = chunk.p;

        const uint16_t orderType   = chunk.in_uint16_le();
        const uint16_t orderLength = chunk.in_uint16_le();

        //LOG(LOG_INFO, "mod_rdp::send_to_mod_rail_channel: orderType=%u orderLength=%u",
        //    orderType, orderLength);

        switch (orderType) {
            case TS_RAIL_ORDER_EXEC:
            {
                ClientExecutePDU_Recv cepdur(chunk);

                LOG(LOG_INFO,
                    "mod_rdp::send_to_mod_rail_channel: Client Execute PDU - "
                        "flags=0x%X exe_or_file=\"%s\" working_dir=\"%s\" arguments=\"%s\"",
                    cepdur.Flags(), cepdur.exe_or_file(), cepdur.working_dir(), cepdur.arguments());
            }
            break;

            case TS_RAIL_ORDER_SYSPARAM:
            {
                ClientSystemParametersUpdatePDU_Recv cspupdur(chunk);

                switch(cspupdur.SystemParam()) {
                    case SPI_SETDRAGFULLWINDOWS:
                    {
                        const unsigned expected = 1 /* Body(1) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint8_t Body = chunk.in_uint8();

                        LOG(LOG_INFO,
                            "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                "Full Window Drag is %s.",
                            (!Body ? "disabled" : "enabled"));
                    }
                    break;

                    case SPI_SETKEYBOARDCUES:
                    {
                        const unsigned expected = 1 /* Body(1) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint8_t Body = chunk.in_uint8();

                        if (Body) {
                            LOG(LOG_INFO,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "Menu Access Keys are always underlined.");
                        }
                        else {
                            LOG(LOG_INFO,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "Menu Access Keys are underlined only when the menu is activated by the keyboard.");
                        }
                    }
                    break;

                    case SPI_SETKEYBOARDPREF:
                    {
                        const unsigned expected = 1 /* Body(1) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint8_t Body = chunk.in_uint8();

                        if (Body) {
                            LOG(LOG_INFO,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "The user prefers the keyboard over mouse.");
                        }
                        else {
                            LOG(LOG_INFO,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "The user does not prefer the keyboard over mouse.");
                        }
                    }
                    break;

                    case SPI_SETMOUSEBUTTONSWAP:
                    {
                        const unsigned expected = 1 /* Body(1) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint8_t Body = chunk.in_uint8();

                        if (Body) {
                            LOG(LOG_INFO,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "Swaps the meaning of the left and right mouse buttons.");
                        }
                        else {
                            LOG(LOG_INFO,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "Restores the meaning of the left and right mouse buttons to their original meanings.");
                        }
                    }
                    break;

                    case SPI_SETWORKAREA:
                    {
                        const unsigned expected = 8 /* Body(8) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint16_t Left   = chunk.in_uint16_le();
                        uint16_t Top    = chunk.in_uint16_le();
                        uint16_t Right  = chunk.in_uint16_le();
                        uint16_t Bottom = chunk.in_uint16_le();

                        LOG(LOG_INFO,
                            "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                "work area in virtual screen coordinates is (left=%u top=%u right=%u bottom=%u).",
                            Left, Top, Right, Bottom);
                    }
                    break;

                    case RAIL_SPI_DISPLAYCHANGE:
                    {
                        const unsigned expected = 8 /* Body(8) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint16_t Left   = chunk.in_uint16_le();
                        uint16_t Top    = chunk.in_uint16_le();
                        uint16_t Right  = chunk.in_uint16_le();
                        uint16_t Bottom = chunk.in_uint16_le();

                        LOG(LOG_INFO,
                            "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                "New display resolution in virtual screen coordinates is (left=%u top=%u right=%u bottom=%u).",
                            Left, Top, Right, Bottom);
                    }
                    break;

                    case RAIL_SPI_TASKBARPOS:
                    {
                        const unsigned expected = 8 /* Body(8) */;
                        if (!chunk.in_check_rem(expected)) {
                            LOG(LOG_ERR,
                                "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                    "expected=%u remains=%u (0x%04X)",
                                expected, chunk.in_remain(),
                                cspupdur.SystemParam());
                            throw Error(ERR_RAIL_PDU_TRUNCATED);
                        }

                        uint16_t Left   = chunk.in_uint16_le();
                        uint16_t Top    = chunk.in_uint16_le();
                        uint16_t Right  = chunk.in_uint16_le();
                        uint16_t Bottom = chunk.in_uint16_le();

                        LOG(LOG_INFO,
                            "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                "Size of the client taskbar is (left=%u top=%u right=%u bottom=%u).",
                            Left, Top, Right, Bottom);
                    }
                    break;

                    case SPI_SETHIGHCONTRAST:
                    {
                        HighContrastSystemInformationStructure_Recv hcsisr(chunk);

                        LOG(LOG_INFO,
                            "mod_rdp::send_to_mod_rail_channel: Client System Parameters Update PDU - "
                                "parameters for the high-contrast accessibility feature, Flags=0x%X, ColorScheme=\"%s\".",
                            hcsisr.Flags(), hcsisr.ColorScheme());
                    }
                    break;
                }
            }
            break;

            case TS_RAIL_ORDER_CLIENTSTATUS:
            {
                ClientInformationPDU_Recv cipdur(chunk);

                LOG(LOG_INFO,
                    "mod_rdp::send_to_mod_rail_channel: Client Information PDU - Flags=0x%08X",
                    cipdur.Flags());
            }
            break;

            case TS_RAIL_ORDER_HANDSHAKE:
            {
                HandshakePDU_Recv hpdur(chunk);

                LOG(LOG_INFO,
                    "mod_rdp::send_to_mod_rail_channel: Handshake PDU - buildNumber=%u",
                    hpdur.buildNumber());
            }
            break;

            default:
                LOG(LOG_INFO,
                    "mod_rdp::send_to_mod_rail_channel: undecoded PDU - orderType=%u orderLength=%u",
                    orderType, orderLength);
            break;
        }

        chunk.p = saved_chunk_p;

        this->send_to_channel(*rail_channel, chunk, length, flags);
    }   // send_to_mod_rail_channel

public:
    static uint32_t filter_unsupported_device(AuthorizationChannels const & authorization_channels,
            Stream & chunk, uint32_t device_count, Stream & result,
            FileSystemDriveManager & file_system_drive_manager,
            bool device_capability_version_02_supported, uint32_t verbose = 0) {
        //LOG(LOG_INFO, "filter_unsupported_device: device_count=%u", device_count);
        result.out_uint16_le(static_cast<uint16_t>(rdpdr::Component::RDPDR_CTYP_CORE));
        result.out_uint16_le(static_cast<uint16_t>(rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE));

        const uint32_t device_count_offset = result.get_offset();
        result.out_skip_bytes(4);

        uint32_t real_device_count = 0;

        for (uint32_t device_index = 0; device_index < device_count; ++device_index) {
            rdpdr::DeviceAnnounceHeader device_announce_header;

            device_announce_header.receive(chunk);
            if (verbose) {
                LOG(LOG_INFO, "rdp::filter_unsupported_device");
                device_announce_header.log(LOG_INFO);
            }

            if (authorization_channels.rdpdr_type_is_authorized(device_announce_header.DeviceType())) {
                //LOG(LOG_INFO, "DeviceType=%u", device_announce_header.DeviceType());
                if (result.has_room(device_announce_header.size())) {
                    device_announce_header.emit(result);

                    real_device_count++;
                }
                else {
                    LOG(LOG_WARNING,
                        "rdp::filter_unsupported_device: "
                            "Too much data for Announce Driver buffer! "
                            "The device is ignored. length=%u limite=%u",
                        result.get_offset() + device_announce_header.size(),
                        result.get_capacity());
                }
            }
        }

        // Add proxy managed File System Drives.
        real_device_count += file_system_drive_manager.AnnounceDrivePartially(result,
            device_capability_version_02_supported, verbose);

        result.set_out_uint32_le(real_device_count, device_count_offset);

        result.mark_end();

        if (verbose) {
            LOG(LOG_INFO, "rdp::filter_unsupported_device: real_device_count=%u", real_device_count);
        }

        return real_device_count;
    }

private:
    void adjust_chunked_virtual_channel_data_stream_size(size_t desired_size) {
        if (this->chunked_virtual_channel_data_stream.get_capacity() < desired_size) {
            size_t rounded_length = this->chunked_virtual_channel_data_stream.get_capacity();
            for (; rounded_length < desired_size; rounded_length *= 2);
            if (rounded_length > this->max_chunked_virtual_channel_data_length) {
                rounded_length = this->max_chunked_virtual_channel_data_length;
            }
            this->chunked_virtual_channel_data_byte = std::make_unique<uint8_t[]>(rounded_length);
            this->chunked_virtual_channel_data_stream.~FixedSizeStream();
            new (&this->chunked_virtual_channel_data_stream) FixedSizeStream(
                this->chunked_virtual_channel_data_byte.get(), rounded_length);
        }
    }

    void send_to_mod_rdpdr_channel(const CHANNELS::ChannelDef * rdpdr_channel,
                                   Stream & chunk, size_t length, uint32_t flags) {
        if (this->verbose) {
            LOG(LOG_INFO, "mod_rdp::send_to_mod_rdpdr_channel: length=%u chunk_size=%u flags=0x%X",
                length, chunk.size(), flags);
        }

        if (this->authorization_channels.rdpdr_type_all_is_authorized() &&
            !this->file_system_drive_manager.HasManagedDrive()) {
            if (this->verbose && (flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                LOG(LOG_INFO,
                    "mod_rdp::send_to_mod_rdpdr_channel: send Chunked Virtual Channel Data transparently.");
            }

            this->send_to_channel(*rdpdr_channel, chunk, length, flags);
            return;
        }

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->adjust_chunked_virtual_channel_data_stream_size(length);
            this->chunked_virtual_channel_data_stream.reset();
        }

        if (length > this->chunked_virtual_channel_data_stream.get_capacity()) {
            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                LOG(LOG_WARNING,
                    "mod_rdp::send_to_mod_rdpdr_channel: "
                        "Too much data for Chunked Virtual Channel Data buffer! "
                        "The PDU is ignored. length=%u limite=%u flags=0x%X",
                    length, this->chunked_virtual_channel_data_stream.get_capacity(), flags);
                return;
            }
        }
        else {
            REDASSERT(((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) ||
                !this->chunked_virtual_channel_data_stream.size());

            this->chunked_virtual_channel_data_stream.out_copy_bytes(chunk.get_data(), chunk.size());

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->chunked_virtual_channel_data_stream.mark_end();
                this->chunked_virtual_channel_data_stream.rewind();

                REDASSERT(this->chunked_virtual_channel_data_stream.size() == length);

                flags |= CHANNELS::CHANNEL_FLAG_FIRST;

                this->send_unchunked_data_to_mod_rdpdr_channel(rdpdr_channel,
                    this->chunked_virtual_channel_data_stream, length, flags);

                this->chunked_virtual_channel_data_stream.reset();
            }
        }
    }

    void send_unchunked_data_to_mod_rdpdr_channel(const CHANNELS::ChannelDef * rdpdr_channel,
                                                  Stream & chunk, size_t length, uint32_t flags) {
        //LOG(LOG_INFO, "chunk.size=%u, length=%u flags=0x%X", chunk.size(), length, flags);
        REDASSERT(chunk.size() == length);

        // filtering device redirection (printer, smartcard, etc)
        const auto saved_chunk_p = chunk.p;

        rdpdr::SharedHeader sh_r;

        sh_r.receive(chunk);
        if (this->verbose) {
            sh_r.log(LOG_INFO);
        }

        const rdpdr::PacketId packet_id = sh_r.packet_id;

        switch (packet_id) {
            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
            {
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: Client Announce Reply");

                    rdpdr::ClientAnnounceReply client_announce_reply;

                    client_announce_reply.receive(chunk);
                    client_announce_reply.log(LOG_INFO);
                }
            }
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_NAME:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: Client Name Request");

                    rdpdr::ClientNameRequest client_name_request;

                    client_name_request.receive(chunk);
                    client_name_request.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
            {
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                            "Client Device List Announce timer is disabled.");
                }
                this->client_device_list_announce_timer_enabled = false;
                this->client_device_list_announce_timeout_checker.cancel_timeout();

                const uint32_t DeviceCount = chunk.in_uint32_le();

                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                            "Client Device List Announce Request - DeviceCount=%u",
                        DeviceCount);
                }

                uint8_t * const DeviceList = chunk.p;

                {
                    BStream result(this->chunked_virtual_channel_data_stream.get_capacity());

                    if (this->filter_unsupported_device(this->authorization_channels,
                                                        chunk, DeviceCount,
                                                        result,
                                                        this->file_system_drive_manager,
                                                        this->device_capability_version_02_supported,
                                                        this->verbose)) {
                        if (this->verbose) {
                            LOG(LOG_INFO, "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel");
                            hexdump_d(result.get_data(), result.size());
                        }
                        this->send_to_channel(*rdpdr_channel, result, result.size(),
                                              flags);
                    }
                }

                chunk.p = DeviceList;

                for (uint32_t device_index = 0; device_index < DeviceCount; ++device_index) {
                    const uint32_t DeviceType       = chunk.in_uint32_le();
                    const uint32_t DeviceId         = chunk.in_uint32_le();
                    chunk.in_skip_bytes(8);                   /* PreferredDosName(8) */
                    const uint32_t DeviceDataLength = chunk.in_uint32_le();
                    chunk.in_skip_bytes(DeviceDataLength);    /* DeviceData(variable) */

                    if (!this->authorization_channels.rdpdr_type_is_authorized(DeviceType)) {
                        rdpdr::ServerDeviceAnnounceResponse server_device_announce_response(
                                DeviceId,
                                0xC0000001  // STATUS_UNSUCCESSFUL
                            );

                        if (this->verbose) {
                            LOG(LOG_INFO,
                                "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: Server Device Announce Response");
                            server_device_announce_response.log(LOG_INFO);
                        }

                        uint8_t data[128];
                        FixedSizeStream stream(data, sizeof(data));

                        stream.out_uint16_le(static_cast<uint16_t>(rdpdr::Component::RDPDR_CTYP_CORE));
                        stream.out_uint16_le(static_cast<uint16_t>(rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY));

                        server_device_announce_response.emit(stream);
                        stream.mark_end();

                        this->send_to_front_channel( channel_names::rdpdr
                                                   , stream.get_data()
                                                   , stream.size()
                                                   , stream.size()
                                                   , CHANNELS::CHANNEL_FLAG_FIRST
                                                   | CHANNELS::CHANNEL_FLAG_LAST
                                                   );
                    }
                }

                return;
            }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: Device I/O Response");

                    rdpdr::DeviceIOResponse device_io_response;

                    device_io_response.receive(chunk);
                    device_io_response.log(LOG_INFO);

                    bool     corresponding_device_io_request_is_not_found = true;
                    uint32_t major_function = 0;
                    uint32_t extra_data     = 0;
                    device_io_request_collection_type::iterator iter;
                    for (iter = this->device_io_requests.begin();
                         iter !=  this->device_io_requests.end(); ++iter) {
                        if ((std::get<0>(*iter) == device_io_response.DeviceId()) &&
                            (std::get<1>(*iter) == device_io_response.CompletionId())) {

                            major_function = std::get<2>(*iter);
                            extra_data     = std::get<3>(*iter);

                            LOG(LOG_INFO,
                                "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: MajorFunction=0x%X extra_data=0x%X",
                                major_function, extra_data);

                            this->device_io_requests.erase(iter);

                            corresponding_device_io_request_is_not_found = false;
                            break;
                        }
                    }
                    if (corresponding_device_io_request_is_not_found) {
                        LOG(LOG_ERR,
                            "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                                "The corresponding Device I/O Request is not found!");
                        REDASSERT(false);
                    }
                    else {
                        switch (major_function) {
                            case rdpdr::IRP_MJ_CREATE:
                            {
                                rdpdr::DeviceCreateResponse device_create_response;

                                device_create_response.receive(chunk);
                                device_create_response.log(LOG_INFO);
                            }
                            break;

                            case rdpdr::IRP_MJ_CLOSE:
                            break;

                            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                            {
                                const uint32_t FsInformationClass = extra_data;

                                switch (extra_data) {
                                    case rdpdr::FileFsAttributeInformation:
                                    {
                                        chunk.in_skip_bytes(4); // Length(4)

                                        fscc::FileFsAttributeInformation
                                            file_fs_Attribute_information;

                                        file_fs_Attribute_information.receive(chunk);
                                        if (this->verbose) {
                                            file_fs_Attribute_information.log(LOG_INFO);
                                        }

                                        // BStream out_stream(65535);

                                        // sh_r.emit(out_stream);
                                        // device_io_response.emit(out_stream);

                                        // uint32_t FileSystemAttributes =
                                        //     file_fs_Attribute_information.FileSystemAttributes();

                                        // FileSystemAttributes |= fscc::FILE_READ_ONLY_VOLUME;

                                        // file_fs_Attribute_information.set_FileSystemAttributes(
                                        //     FileSystemAttributes);
                                        // file_fs_Attribute_information.log(LOG_INFO);

                                        // out_stream.out_uint32_le(                   // Length(4)
                                        //     file_fs_Attribute_information.size());

                                        // file_fs_Attribute_information.emit(out_stream);

                                        // out_stream.mark_end();

                                        // this->send_to_channel( *rdpdr_channel
                                        //                      , out_stream
                                        //                      , out_stream.size()
                                        //                      , flags
                                        //                      );

                                        // return;
                                    }
                                    break;

                                    default:
                                        LOG(LOG_INFO,
                                            "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                                                "IRP_MJ_QUERY_VOLUME_INFORMATION - "
                                                "undecoded FsInformationClass(0x%X)",
                                            FsInformationClass);
                                    break;
                                }
                            }
                            break;

                            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                            {
                                const uint32_t FsInformationClass = extra_data;

                                switch (extra_data) {
                                    case rdpdr::FileBasicInformation:
                                    {
                                        chunk.in_skip_bytes(4); // Length(4)

                                        fscc::FileBasicInformation file_basic_information;

                                        file_basic_information.receive(chunk);
                                        file_basic_information.log(LOG_INFO);
                                    }
                                    break;

                                    default:
                                        LOG(LOG_INFO,
                                            "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                                                "IRP_MJ_QUERY_INFORMATION - "
                                                "undecoded FsInformationClass(0x%X)",
                                            FsInformationClass);
                                    break;
                                }
                            }
                            break;

/*
                            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                            {
                                const uint32_t FsInformationClass = extra_data;

                                switch (extra_data) {
                                    case rdpdr::FileBasicInformation:
                                    {
                                        chunk.in_skip_bytes(4); // Length(4)

                                        fscc::FileBasicInformation file_basic_information;

                                        file_basic_information.receive(chunk);
                                        file_basic_information.log(LOG_INFO);
                                    }
                                    break;

                                    default:
                                        LOG(LOG_INFO,
                                            "mod_rdp::send_to_mod_rail_channel: "
                                                "IRP_MJ_QUERY_INFORMATION - "
                                                "undecoded FsInformationClass(0x%X)",
                                            FsInformationClass);
                                    break;
                                }
                            }
                            break;
*/

                            default:
                                LOG(LOG_INFO,
                                    "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: undecoded MajorFunction(0x%X)",
                                    major_function);
                            break;
                        }
                    }
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY:
            {
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: Client Core Capability Response");
                }

                const uint16_t numCapabilities = chunk.in_uint16_le();
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: numCapabilities=%u", numCapabilities);
                }

                chunk.in_skip_bytes(2); // Padding(2)

                for (uint16_t idx_capabilities = 0; idx_capabilities < numCapabilities;
                     ++idx_capabilities) {
                    const uint16_t CapabilityType   = chunk.in_uint16_le();
                    const uint16_t CapabilityLength = chunk.in_uint16_le();
                    const uint32_t Version          = chunk.in_uint32_le();

                    if (this->verbose) {
                        LOG(LOG_INFO,
                            "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                                "CapabilityType=0x%04X CapabilityLength=%u Version=0x%X",
                            CapabilityType, CapabilityLength, Version);
                    }

                    switch (CapabilityType) {
                        case rdpdr::CAP_GENERAL_TYPE:
                        {
                            rdpdr::GeneralCapabilitySet general_capability_set;

                            general_capability_set.receive(chunk, Version);

                            if (this->verbose) {
                                general_capability_set.log(LOG_INFO);
                            }
                        }
                        break;

                        default:
                            chunk.in_skip_bytes(CapabilityLength -
                                8 /* CapabilityType(2) + CapabilityLength(2) + Version(4) */);
                        break;
                    }

                    if ((CapabilityType == rdpdr::CAP_DRIVE_TYPE) &&
                        (Version == rdpdr::DRIVE_CAPABILITY_VERSION_02)) {
                        this->device_capability_version_02_supported = true;
                        if (this->verbose) {
                            LOG(LOG_INFO,
                                "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: "
                                    "Client supports DRIVE_CAPABILITY_VERSION_02.");
                        }
                    }
                }
            }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: Client Drive Device List Remove");
                }
            break;

            default:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_unchunked_data_to_mod_rdpdr_channel: undecoded PDU - Component=0x%X PacketId=0x%X",
                        static_cast<uint16_t>(sh_r.component), static_cast<uint16_t>(sh_r.packet_id));
                }

                this->update_total_rdpdr_data(packet_id, length);
            break;
        }

        chunk.p = saved_chunk_p;

        this->send_to_channel(*rdpdr_channel, chunk, length, flags);
    }   // send_to_mod_rdpdr_channel

public:
    // Method used by session to transmit sesman answer for auth_channel
    virtual void send_auth_channel_data(const char * string_data) {
        //if (strncmp("Error", string_data, 5)) {
        //    this->auth_channel_state = 1; // session started
        //}

        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        BStream stream_data(65536);
        uint32_t data_size = std::min(::strlen(string_data) + 1, stream_data.get_capacity());

        stream_data.out_copy_bytes(string_data, data_size);
        stream_data.mark_end();

        virtual_channel_pdu.send_to_server( this->nego.trans, this->encrypt, this->encryptionLevel
                            , this->userid, this->auth_channel_chanid
                            , stream_data.size()
                            , this->auth_channel_flags
                            , stream_data.get_data()
                            , stream_data.size());
    }

    void send_to_channel(const CHANNELS::ChannelDef & channel, Stream & chunk, size_t length, uint32_t flags) {
        if (this->verbose & 16) {
            LOG( LOG_INFO, "mod_rdp::send_to_channel length=%u chunk_size=%u", static_cast<unsigned>(length)
                 , (unsigned)chunk.size());
            channel.log(-1u);
        }

        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        }

        if (chunk.size() <= CHANNELS::CHANNEL_CHUNK_LENGTH) {
            CHANNELS::VirtualChannelPDU virtual_channel_pdu;

            virtual_channel_pdu.send_to_server( this->nego.trans, this->encrypt, this->encryptionLevel
                                              , this->userid, channel.chanid, length, flags, chunk.get_data(), chunk.size());
        }
        else {
            uint8_t const * virtual_channel_data = chunk.get_data();
            size_t          remaining_data_length = length;

            auto get_channel_control_flags = [] (uint32_t flags, size_t data_length,
                                                 size_t remaining_data_length,
                                                 size_t virtual_channel_data_length) -> uint32_t {
                if (remaining_data_length == data_length) {
                    return (flags & (~CHANNELS::CHANNEL_FLAG_LAST));
                }
                else if (remaining_data_length == virtual_channel_data_length) {
                    return (flags & (~CHANNELS::CHANNEL_FLAG_FIRST));
                }

                return (flags & (~(CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)));
            };

            do {
                const size_t virtual_channel_data_length =
                    std::min<size_t>(remaining_data_length, CHANNELS::CHANNEL_CHUNK_LENGTH);

                CHANNELS::VirtualChannelPDU virtual_channel_pdu;

                virtual_channel_pdu.send_to_server( this->nego.trans, this->encrypt, this->encryptionLevel
                                                  , this->userid, channel.chanid, length
                                                  , get_channel_control_flags(flags, length, remaining_data_length, virtual_channel_data_length)
                                                  , virtual_channel_data, virtual_channel_data_length);

                remaining_data_length -= virtual_channel_data_length;
                virtual_channel_data  += virtual_channel_data_length;
            }
            while (remaining_data_length);
        }

        if (this->verbose & 16) {
            LOG(LOG_INFO, "mod_rdp::send_to_channel done");
        }
    }

    void send_data_request(uint16_t channelId, HStream & stream) {
        if (this->verbose & 16) {
            LOG(LOG_INFO, "send data request");
        }

        BStream x224_header(256);
        OutPerBStream mcs_header(256);

        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channelId, 1,
                                      3, stream.size(), MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans.send(x224_header, mcs_header, stream);

        if (this->verbose & 16) {
            LOG(LOG_INFO, "send data request done");
        }
    }

    void send_data_request_ex(uint16_t channelId, HStream & stream) {
        BStream x224_header(256);
        OutPerBStream mcs_header(256);
        BStream sec_header(256);

        SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel);
        stream.copy_to_head(sec_header.get_data(), sec_header.size());

        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channelId, 1,
                                      3, stream.size(), MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans.send(x224_header, mcs_header, stream);
    }

    virtual void draw_event(time_t now) {
        if (!this->event.waked_up_by_time) {
            try{
                char * hostname = this->hostname;

                switch (this->state){
                case MOD_RDP_NEGO:
                    if (this->verbose & 1){
                        LOG(LOG_INFO, "mod_rdp::Early TLS Security Exchange");
                    }
                    switch (this->nego.state){
                    default:
                        this->nego.server_event(this->certificate_change_action == 1);
                        break;
                    case RdpNego::NEGO_STATE_FINAL:
                        {
                            // Basic Settings Exchange
                            // -----------------------

                            // Basic Settings Exchange: Basic settings are exchanged between the client and
                            // server by using the MCS Connect Initial and MCS Connect Response PDUs. The
                            // Connect Initial PDU contains a GCC Conference Create Request, while the
                            // Connect Response PDU contains a GCC Conference Create Response.

                            // These two Generic Conference Control (GCC) packets contain concatenated
                            // blocks of settings data (such as core data, security data and network data)
                            // which are read by client and server


                            // Client                                                     Server
                            //    |--------------MCS Connect Initial PDU with-------------> |
                            //                   GCC Conference Create Request
                            //    | <------------MCS Connect Response PDU with------------- |
                            //                   GCC conference Create Response

                            /* Generic Conference Control (T.124) ConferenceCreateRequest */

                            HStream stream(1024, 65536);
                            // ------------------------------------------------------------
                            GCC::UserData::CSCore cs_core;
                            cs_core.version = this->use_rdp5?0x00080004:0x00080001;
                            cs_core.desktopWidth = this->front_width;
                            cs_core.desktopHeight = this->front_height;
                            //cs_core.highColorDepth = this->front_bpp;
                            cs_core.highColorDepth = ((this->front_bpp == 32)
                                ? uint16_t(GCC::UserData::HIGH_COLOR_24BPP) : this->front_bpp);
                            cs_core.keyboardLayout = this->keylayout;
                            if (this->front_bpp == 32) {
                                cs_core.supportedColorDepths = 15;
                                cs_core.earlyCapabilityFlags |= GCC::UserData::RNS_UD_CS_WANT_32BPP_SESSION;
                            }

                            uint16_t hostlen = strlen(hostname);
                            uint16_t maxhostlen = std::min((uint16_t)15, hostlen);
                            for (size_t i = 0; i < maxhostlen ; i++){
                                cs_core.clientName[i] = hostname[i];
                            }
                            memset(&(cs_core.clientName[hostlen]), 0, 16-hostlen);

                            if (this->nego.tls){
                                cs_core.serverSelectedProtocol = this->nego.selected_protocol;
                            }
                            if (this->verbose & 1) {
                                cs_core.log("Sending to Server");
                            }
                            cs_core.emit(stream);
                            // ------------------------------------------------------------

                            GCC::UserData::CSCluster cs_cluster;
                            TODO("CGR: values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?");
                            if (this->server_redirection_support) {
                                LOG(LOG_INFO, "CS_Cluster: Server Redirection Supported");
                                if (!this->nego.tls){
                                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTION_SUPPORTED;
                                    cs_cluster.flags |= (2 << 2); // REDIRECTION V3
                                } else {
                                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTION_SUPPORTED;
                                    cs_cluster.flags |= (3 << 2);  // REDIRECTION V4
                                }
                                if (this->redir_info.valid) {
                                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID;
                                    cs_cluster.redirectedSessionID = this->redir_info.session_id;
                                    LOG(LOG_INFO, "Effective Redirection SessionId=%u",
                                        cs_cluster.redirectedSessionID);
                                }
                            }
                            if (this->console_session) {
                                cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID;
                            }
                            // if (!this->nego.tls){
                            //     if (this->console_session){
                            //         cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID | (3 << 2) ; // REDIRECTION V4
                            //     }
                            //     else {
                            //         cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED            | (2 << 2) ; // REDIRECTION V3
                            //     }
                            //     }
                            // else {
                            //     cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED * ((3 << 2)|1);  // REDIRECTION V4
                            //     if (this->console_session){
                            //         cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID ;
                            //     }
                            // }
                            if (this->verbose & 1) {
                                cs_cluster.log("Sending to server");
                            }
                            cs_cluster.emit(stream);
                            // ------------------------------------------------------------
                            GCC::UserData::CSSecurity cs_security;
                            if (this->verbose & 1) {
                                cs_security.log("Sending to server");
                            }
                            cs_security.emit(stream);
                            // ------------------------------------------------------------

                            const CHANNELS::ChannelDefArray & channel_list = this->front.get_channel_list();
                            size_t num_channels = channel_list.size();
                            if ((num_channels > 0) || this->auth_channel[0] ||
                                this->file_system_drive_manager.HasManagedDrive()) {
                                /* Here we need to put channel information in order
                                   to redirect channel data
                                   from client to server passing through the "proxy" */
                                GCC::UserData::CSNet cs_net;
                                cs_net.channelCount = num_channels;
                                bool has_rdpdr_channel  = false;
                                bool has_rdpsnd_channel = false;
                                for (size_t index = 0; index < num_channels; index++) {
                                    const CHANNELS::ChannelDef & channel_item = channel_list[index];
                                    if (this->authorization_channels.is_authorized(channel_item.name) ||
                                        (!strcmp(channel_item.name, channel_names::rdpdr) &&
                                         this->file_system_drive_manager.HasManagedDrive())
                                       ) {
                                        if (!strcmp(channel_item.name, channel_names::rdpdr)) {
                                            has_rdpdr_channel = true;
                                        }
                                        memcpy(cs_net.channelDefArray[index].name, channel_item.name, 8);
                                    }
                                    else {
                                        memcpy(cs_net.channelDefArray[index].name, "\0\0\0\0\0\0\0", 8);
                                    }
                                    cs_net.channelDefArray[index].options = channel_item.flags;
                                    CHANNELS::ChannelDef def;
                                    memcpy(def.name, cs_net.channelDefArray[index].name, 8);
                                    def.flags = channel_item.flags;
                                    if (this->verbose & 16) {
                                        def.log(index);
                                    }
                                    this->mod_channel_list.push_back(def);
                                }

                                // Inject a new channel for file system virtual channel (rdpdr)
                                if (!has_rdpdr_channel && this->file_system_drive_manager.HasManagedDrive()) {
                                    ::snprintf(cs_net.channelDefArray[cs_net.channelCount].name,
                                             sizeof(cs_net.channelDefArray[cs_net.channelCount].name),
                                             "%s", channel_names::rdpdr);
                                    cs_net.channelDefArray[cs_net.channelCount].options =
                                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                                        | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP;
                                    CHANNELS::ChannelDef def;
                                    ::snprintf(def.name, sizeof(def.name), "%s", channel_names::rdpdr);
                                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                                    if (this->verbose & 16){
                                        def.log(cs_net.channelCount);
                                    }
                                    this->mod_channel_list.push_back(def);
                                    cs_net.channelCount++;

                                    this->proxy_managed_rdpdr_channel = true;
                                }
                                else {
                                    this->proxy_managed_rdpdr_channel =
                                        (has_rdpdr_channel &&
                                         !this->authorization_channels.is_authorized(channel_names::rdpdr));
                                }

                                // The RDPDR channel advertised by the client is ONLY accepted by the RDP
                                //  server 2012 if the RDPSND channel is also advertised.
                                if (this->file_system_drive_manager.HasManagedDrive() &&
                                    !has_rdpsnd_channel) {
                                    ::snprintf(cs_net.channelDefArray[cs_net.channelCount].name,
                                             sizeof(cs_net.channelDefArray[cs_net.channelCount].name),
                                             "%s", channel_names::rdpsnd);
                                    cs_net.channelDefArray[cs_net.channelCount].options =
                                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                                        | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP;
                                    CHANNELS::ChannelDef def;
                                    ::snprintf(def.name, sizeof(def.name), "%s", channel_names::rdpsnd);
                                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                                    if (this->verbose & 16){
                                        def.log(cs_net.channelCount);
                                    }
                                    this->mod_channel_list.push_back(def);
                                    cs_net.channelCount++;
                                }

                                // Inject a new channel for auth_channel virtual channel (wablauncher)
                                if (this->auth_channel[0]) {
                                    memcpy(cs_net.channelDefArray[num_channels].name, this->auth_channel, 8);
                                    cs_net.channelDefArray[num_channels].options =
                                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED;
                                    cs_net.channelCount++;
                                    CHANNELS::ChannelDef def;
                                    memcpy(def.name, this->auth_channel, 8);
                                    def.flags = cs_net.channelDefArray[num_channels].options;
                                    if (this->verbose & 16){
                                        def.log(num_channels);
                                    }
                                    this->mod_channel_list.push_back(def);
                                }

                                if (this->enable_wab_agent) {
                                    const char * wab_agent_channel_name = "wabagt\0\0";
                                    memcpy(cs_net.channelDefArray[cs_net.channelCount].name, wab_agent_channel_name, 8);
                                    cs_net.channelDefArray[cs_net.channelCount].options =
                                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED;
                                    CHANNELS::ChannelDef def;
                                    memcpy(def.name, wab_agent_channel_name, 8);
                                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                                    if (this->verbose & 16){
                                        def.log(cs_net.channelCount);
                                    }
                                    this->mod_channel_list.push_back(def);
                                    cs_net.channelCount++;
                                }

                                if (this->verbose & 1) {
                                    cs_net.log("Sending to server");
                                }
                                cs_net.emit(stream);
                            }
                            // ------------------------------------------------------------

                            OutPerBStream gcc_header(65536);
                            GCC::Create_Request_Send(gcc_header, stream.size());

                            BStream mcs_header(65536);
                            MCS::CONNECT_INITIAL_Send mcs(mcs_header, gcc_header.size() + stream.size(), MCS::BER_ENCODING);

                            BStream x224_header(256);
                            X224::DT_TPDU_Send(x224_header, mcs_header.size() + gcc_header.size() + stream.size());
                            this->nego.trans.send(x224_header, mcs_header, gcc_header, stream);

                            this->state = MOD_RDP_BASIC_SETTINGS_EXCHANGE;
                        }
                        break;
                    }
                    break;

                case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
                    if (this->verbose & 1){
                        LOG(LOG_INFO, "mod_rdp::Basic Settings Exchange");
                    }
                    {
                        Array array(65536);
                        uint8_t * end = array.get_data();
                        X224::RecvFactory f(this->nego.trans, &end, array.size());
                        InStream x224_data(array, 0, 0, end - array.get_data());
                        X224::DT_TPDU_Recv x224(x224_data);

                        MCS::CONNECT_RESPONSE_PDU_Recv mcs(x224.payload, MCS::BER_ENCODING);

                        GCC::Create_Response_Recv gcc_cr(mcs.payload);

                        while (gcc_cr.payload.in_check_rem(4)) {

                            GCC::UserData::RecvFactory f(gcc_cr.payload);
                            switch (f.tag) {
                            case SC_CORE:
                                {
                                    GCC::UserData::SCCore sc_core;
                                    sc_core.recv(f.payload);
                                    if (this->verbose & 1) {
                                        sc_core.log("Received from server");
                                    }
                                    if (0x0080001 == sc_core.version){ // can't use rdp5
                                        this->use_rdp5 = 0;
                                    }
                                }
                                break;
                            case SC_SECURITY:
                                {
                                    GCC::UserData::SCSecurity sc_sec1;
                                    sc_sec1.recv(f.payload);
                                    if (this->verbose & 1) {
                                        sc_sec1.log("Received from server");
                                    }

                                    this->encryptionLevel = sc_sec1.encryptionLevel;
                                    this->encryptionMethod = sc_sec1.encryptionMethod;
                                    if (sc_sec1.encryptionLevel == 0
                                        &&  sc_sec1.encryptionMethod == 0) { /* no encryption */
                                        LOG(LOG_INFO, "No encryption");
                                    }
                                    else {

                                        uint8_t serverRandom[SEC_RANDOM_SIZE] = {};
                                        uint8_t modulus[SEC_MAX_MODULUS_SIZE];
                                        memset(modulus, 0, sizeof(modulus));
                                        uint8_t exponent[SEC_EXPONENT_SIZE];
                                        memset(exponent, 0, sizeof(exponent));

                                        memcpy(serverRandom, sc_sec1.serverRandom, sc_sec1.serverRandomLen);

                                        // serverCertificate (variable): The variable-length certificate containing the
                                        //  server's public key information. The length in bytes is given by the
                                        // serverCertLen field. If the encryptionMethod and encryptionLevel fields are
                                        // both set to 0 then this field MUST NOT be present.

                                        /* RSA info */
                                        if (sc_sec1.dwVersion == GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
                                            memcpy(exponent, sc_sec1.proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
                                            memcpy(modulus, sc_sec1.proprietaryCertificate.RSAPK.modulus,
                                                   sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE);

                                            this->server_public_key_len = sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE;

                                        }
                                        else {

                                            uint32_t certcount = sc_sec1.x509.certCount;
                                            if (certcount < 2){
                                                LOG(LOG_WARNING, "Server didn't send enough X509 certificates");
                                                throw Error(ERR_SEC);
                                            }

                                            uint32_t cert_len = sc_sec1.x509.cert[certcount - 1].len;
                                            X509 *cert =  sc_sec1.x509.cert[certcount - 1].cert;
                                            (void)cert_len;

                                            TODO("CGR: Currently, we don't use the CA Certificate, we should"
                                                 "*) Verify the server certificate (server_cert) with the CA certificate."
                                                 "*) Store the CA Certificate with the hostname of the server we are connecting"
                                                 " to as key, and compare it when we connect the next time, in order to prevent"
                                                 " MITM-attacks.")

                                                /* By some reason, Microsoft sets the OID of the Public RSA key to
                                                   the oid for "MD5 with RSA Encryption" instead of "RSA Encryption"

                                                   Kudos to Richard Levitte for the following (. intuitive .)
                                                   lines of code that resets the OID and let's us extract the key. */

                                                int nid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
                                            if ((nid == NID_md5WithRSAEncryption) || (nid == NID_shaWithRSAEncryption)){
                                                ASN1_OBJECT_free(cert->cert_info->key->algor->algorithm);
                                                cert->cert_info->key->algor->algorithm = OBJ_nid2obj(NID_rsaEncryption);
                                            }

                                            EVP_PKEY * epk = X509_get_pubkey(cert);
                                            if (nullptr == epk){
                                                LOG(LOG_WARNING, "Failed to extract public key from certificate\n");
                                                throw Error(ERR_SEC);
                                            }

                                            TODO("see possible factorisation with ssl_calls.hpp/ssllib::rsa_encrypt")
                                            RSA * server_public_key = EVP_PKEY_get1_RSA(epk);
                                            EVP_PKEY_free(epk);
                                            this->server_public_key_len = RSA_size(server_public_key);

                                            if (nullptr == server_public_key){
                                                LOG(LOG_WARNING, "Failed to parse X509 server key");
                                                throw Error(ERR_SEC);
                                            }

                                            if ((this->server_public_key_len < SEC_MODULUS_SIZE) ||
                                                (this->server_public_key_len > SEC_MAX_MODULUS_SIZE)){
                                                LOG(LOG_WARNING, "Wrong server public key size (%u bits)", this->server_public_key_len * 8);
                                                throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
                                            }

                                            if ((BN_num_bytes(server_public_key->e) > SEC_EXPONENT_SIZE)
                                                ||  (BN_num_bytes(server_public_key->n) > SEC_MAX_MODULUS_SIZE)){
                                                LOG(LOG_WARNING, "Failed to extract RSA exponent and modulus");
                                                throw Error(ERR_SEC);
                                            }
                                            int len_e = BN_bn2bin(server_public_key->e, (unsigned char*)exponent);
                                            reverseit(exponent, len_e);
                                            int len_n = BN_bn2bin(server_public_key->n, (unsigned char*)modulus);
                                            reverseit(modulus, len_n);
                                            RSA_free(server_public_key);
                                        }

                                        uint8_t client_random[SEC_RANDOM_SIZE];
                                        memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));

                                        /* Generate a client random, and determine encryption keys */
                                        this->gen.random(client_random, SEC_RANDOM_SIZE);

                                        ssllib ssl;

                                        ssl.rsa_encrypt(client_crypt_random, client_random, SEC_RANDOM_SIZE, this->server_public_key_len, modulus, exponent);
                                        SEC::KeyBlock key_block(client_random, serverRandom);
                                        memcpy(encrypt.sign_key, key_block.blob0, 16);
                                        if (sc_sec1.encryptionMethod == 1){
                                            ssl.sec_make_40bit(encrypt.sign_key);
                                        }
                                        this->decrypt.generate_key(key_block.key1, sc_sec1.encryptionMethod);
                                        this->encrypt.generate_key(key_block.key2, sc_sec1.encryptionMethod);
                                    }
                                }
                                break;
                            case SC_NET:
                                {
                                    GCC::UserData::SCNet sc_net;
                                    sc_net.recv(f.payload, this->bogus_sc_net_size);

                                    /* We assume that the channel_id array is confirmed in the same order
                                       that it has been sent. If there are any channels not confirmed, they're
                                       going to be the last channels on the array sent in MCS Connect Initial */
                                    if (this->verbose & 16){
                                        LOG(LOG_INFO, "server_channels_count=%u sent_channels_count=%u",
                                            sc_net.channelCount,
                                            mod_channel_list.size());
                                    }
                                    for (uint32_t index = 0; index < sc_net.channelCount; index++) {
                                        if (this->verbose & 16){
                                            this->mod_channel_list[index].log(index);
                                        }
                                        this->mod_channel_list.set_chanid(index, sc_net.channelDefArray[index].id);
                                    }
                                    if (this->verbose & 1) {
                                        sc_net.log("Received from server");
                                    }
                                }
                                break;
                            default:
                                LOG(LOG_WARNING, "unsupported GCC UserData response tag 0x%x", f.tag);
                                throw Error(ERR_GCC);
                            }
                        }
                        if (gcc_cr.payload.in_check_rem(1)) {
                            LOG(LOG_WARNING, "Error while parsing GCC UserData : short header");
                            throw Error(ERR_GCC);
                        }

                    }

                    if (this->verbose & (1|16)){
                        LOG(LOG_INFO, "mod_rdp::Channel Connection");
                    }

                    // Channel Connection
                    // ------------------
                    // Channel Connection: The client sends an MCS Erect Domain Request PDU,
                    // followed by an MCS Attach User Request PDU to attach the primary user
                    // identity to the MCS domain.

                    // The server responds with an MCS Attach User Response PDU containing the user
                    // channel ID.

                    // The client then proceeds to join the :
                    // - user channel,
                    // - the input/output (I/O) channel
                    // - and all of the static virtual channels

                    // (the I/O and static virtual channel IDs are obtained from the data embedded
                    //  in the GCC packets) by using multiple MCS Channel Join Request PDUs.

                    // The server confirms each channel with an MCS Channel Join Confirm PDU.
                    // (The client only sends a Channel Join Request after it has received the
                    // Channel Join Confirm for the previously sent request.)

                    // From this point, all subsequent data sent from the client to the server is
                    // wrapped in an MCS Send Data Request PDU, while data sent from the server to
                    //  the client is wrapped in an MCS Send Data Indication PDU. This is in
                    // addition to the data being wrapped by an X.224 Data PDU.

                    // Client                                                     Server
                    //    |-------MCS Erect Domain Request PDU--------------------> |
                    //    |-------MCS Attach User Request PDU---------------------> |

                    //    | <-----MCS Attach User Confirm PDU---------------------- |

                    //    |-------MCS Channel Join Request PDU--------------------> |
                    //    | <-----MCS Channel Join Confirm PDU--------------------- |

                    if (this->verbose & 1){
                        LOG(LOG_INFO, "Send MCS::ErectDomainRequest");
                    }
                    {
                        BStream x224_header(256);
                        OutPerBStream mcs_header(256);
                        HStream data(512, 512);
                        data.mark_end();

                        MCS::ErectDomainRequest_Send mcs(mcs_header, 0, 0, MCS::PER_ENCODING);
                        X224::DT_TPDU_Send(x224_header, mcs_header.size());
                        this->nego.trans.send(x224_header, mcs_header, data);
                    }
                    if (this->verbose & 1){
                        LOG(LOG_INFO, "Send MCS::AttachUserRequest");
                    }
                    {
                        BStream x224_header(256);
                        HStream mcs_data(256, 512);

                        MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);

                        X224::DT_TPDU_Send(x224_header, mcs_data.size());
                        this->nego.trans.send(x224_header, mcs_data);
                    }
                    this->state = MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER;
                    break;

                case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
                    if (this->verbose & 1){
                        LOG(LOG_INFO, "mod_rdp::Channel Connection Attach User");
                    }
                    {
                        {
                            Array array(65536);
                            uint8_t * end = array.get_data();
                            X224::RecvFactory f(this->nego.trans, &end, array.size());
                            InStream stream(array, 0, 0, end - array.get_data());
                            X224::DT_TPDU_Recv x224(stream);
                            SubStream & payload = x224.payload;

                            MCS::AttachUserConfirm_Recv mcs(payload, MCS::PER_ENCODING);
                            if (mcs.initiator_flag){
                                this->userid = mcs.initiator;
                            }
                        }

                        {
                            size_t num_channels = this->mod_channel_list.size();
                            uint16_t channels_id[CHANNELS::MAX_STATIC_VIRTUAL_CHANNELS + 2];
                            channels_id[0] = this->userid + GCC::MCS_USERCHANNEL_BASE;
                            channels_id[1] = GCC::MCS_GLOBAL_CHANNEL;
                            for (size_t index = 0; index < num_channels; index++){
                                channels_id[index+2] = this->mod_channel_list[index].chanid;
                            }

                            for (size_t index = 0; index < num_channels+2; index++) {
                                BStream x224_header(256);
                                HStream mcs_cjrq_data(256, 512);
                                if (this->verbose & 16){
                                    LOG(LOG_INFO, "cjrq[%u] = %u", index, channels_id[index]);
                                }
                                MCS::ChannelJoinRequest_Send(mcs_cjrq_data, this->userid, channels_id[index], MCS::PER_ENCODING);
                                X224::DT_TPDU_Send(x224_header, mcs_cjrq_data.size());
                                this->nego.trans.send(x224_header, mcs_cjrq_data);

                                Array array(65536);
                                uint8_t * end = array.get_data();
                                X224::RecvFactory f(this->nego.trans, &end, array.size());
                                InStream x224_data(array, 0, 0, end - array.get_data());

                                X224::DT_TPDU_Recv x224(x224_data);
                                SubStream & mcs_cjcf_data = x224.payload;
                                MCS::ChannelJoinConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
                                TODO("If mcs.result is negative channel is not confirmed and should be removed from mod_channel list");
                                if (this->verbose & 16){
                                    LOG(LOG_INFO, "cjcf[%u] = %u", index, mcs.channelId);
                                }
                            }
                        }

                        // RDP Security Commencement
                        // -------------------------

                        // RDP Security Commencement: If standard RDP security methods are being
                        // employed and encryption is in force (this is determined by examining the data
                        // embedded in the GCC Conference Create Response packet) then the client sends
                        // a Security Exchange PDU containing an encrypted 32-byte random number to the
                        // server. This random number is encrypted with the public key of the server
                        // (the server's public key, as well as a 32-byte server-generated random
                        // number, are both obtained from the data embedded in the GCC Conference Create
                        //  Response packet).

                        // The client and server then utilize the two 32-byte random numbers to generate
                        // session keys which are used to encrypt and validate the integrity of
                        // subsequent RDP traffic.

                        // From this point, all subsequent RDP traffic can be encrypted and a security
                        // header is include " with the data if encryption is in force (the Client Info
                        // and licensing PDUs are an exception in that they always have a security
                        // header). The Security Header follows the X.224 and MCS Headers and indicates
                        // whether the attached data is encrypted.

                        // Even if encryption is in force server-to-client traffic may not always be
                        // encrypted, while client-to-server traffic will always be encrypted by
                        // Microsoft RDP implementations (encryption of licensing PDUs is optional,
                        // however).

                        // Client                                                     Server
                        //    |------Security Exchange PDU ---------------------------> |
                        if (this->verbose & 1){
                            LOG(LOG_INFO, "mod_rdp::RDP Security Commencement");
                        }

                        if (this->encryptionLevel){
                            if (this->verbose & 1){
                                LOG(LOG_INFO, "mod_rdp::SecExchangePacket keylen=%u",
                                    this->server_public_key_len);
                            }
                            HStream stream(512, 512 + this->server_public_key_len + 32);
                            SEC::SecExchangePacket_Send mcs(stream, client_crypt_random,
                                this->server_public_key_len);
                            this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, stream);
                        }

                        // Secure Settings Exchange
                        // ------------------------

                        // Secure Settings Exchange: Secure client data (such as the username,
                        // password and auto-reconnect cookie) is sent to the server using the Client
                        // Info PDU.

                        // Client                                                     Server
                        //    |------ Client Info PDU      ---------------------------> |

                        if (this->verbose & 1){
                            LOG(LOG_INFO, "mod_rdp::Secure Settings Exchange");
                        }

                        this->send_client_info_pdu(this->userid, this->password);

                        this->state = MOD_RDP_GET_LICENSE;
                    }
                    break;

                case MOD_RDP_GET_LICENSE:
                    if (this->verbose & 2){
                        LOG(LOG_INFO, "mod_rdp::Licensing");
                    }
                    // Licensing
                    // ---------

                    // Licensing: The goal of the licensing exchange is to transfer a
                    // license from the server to the client.

                    // The client should store this license and on subsequent
                    // connections send the license to the server for validation.
                    // However, in some situations the client may not be issued a
                    // license to store. In effect, the packets exchanged during this
                    // phase of the protocol depend on the licensing mechanisms
                    // employed by the server. Within the context of this document
                    // we will assume that the client will not be issued a license to
                    // store. For details regarding more advanced licensing scenarios
                    // that take place during the Licensing Phase, see [MS-RDPELE].

                    // Client                                                     Server
                    //    | <------ License Error PDU Valid Client ---------------- |

                    // 2.2.1.12 Server License Error PDU - Valid Client
                    // ================================================

                    // The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
                    // from server to client during the Licensing phase of the RDP Connection
                    // Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
                    // phases). This licensing PDU indicates that the server will not issue the
                    // client a license to store and that the Licensing Phase has ended
                    // successfully. This is one possible licensing PDU that may be sent during the
                    // Licensing Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible
                    // licensing PDUs).

                    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

                    // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

                    // mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU)
                    // which encapsulates an MCS Send Data Indication structure (SDin, choice 26
                    // from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure
                    // definitions are given in [T125] section 7, parts 7 and 10). The userData
                    // field of the MCS Send Data Indication contains a Security Header and a Valid
                    // Client License Data (section 2.2.1.12.1) structure.

                    // securityHeader (variable): Security header. The format of the security header
                    // depends on the Encryption Level and Encryption Method selected by the server
                    // (sections 5.3.2 and 2.2.1.4.3).

                    // This field MUST contain one of the following headers:
                    //  - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
                    // selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW
                    // (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008)
                    // flag.
                    //  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method
                    // selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
                    // ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
                    // (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008)
                    // flag.
                    //  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
                    // selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the
                    // embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

                    // If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2),
                    // ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field
                    // of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the
                    // licensing PDU is not encrypted), then the field MUST contain a Basic Security
                    // Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not
                    // set on the Security Exchange PDU (section 2.2.1.10).

                    // The flags field of the security header MUST contain the SEC_LICENSE_PKT
                    // (0x0080) flag (see Basic (TS_SECURITY_HEADER)).

                    // validClientLicenseData (variable): The actual contents of the License Error
                    // (Valid Client) PDU, as specified in section 2.2.1.12.1.

                    {
                        const char * hostname = this->hostname;
                        const char * username;
                        char username_a_domain[512];
                        if (this->domain[0]) {
                            snprintf(username_a_domain, sizeof(username_a_domain), "%s@%s", this->username, this->domain);
                            username = username_a_domain;
                        }
                        else {
                            username = this->username;
                        }
                        LOG(LOG_INFO, "Rdp::Get license: username=\"%s\"", username);
                        // read tpktHeader (4 bytes = 3 0 len)
                        // TPDU class 0    (3 bytes = LI F0 PDU_DT)

                        Array array(65536);
                        uint8_t * end = array.get_data();
                        X224::RecvFactory f(this->nego.trans, &end, array.size());
                        InStream stream(array, 0, 0, end - array.get_data());
                        X224::DT_TPDU_Recv x224(stream);
                        TODO("Shouldn't we use mcs_type to manage possible Deconnection Ultimatum here")
                        //int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);
                        MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);

                        SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);

                        if (sec.flags & SEC::SEC_LICENSE_PKT) {
                            LIC::RecvFactory flic(sec.payload);

                            switch (flic.tag) {
                            case LIC::LICENSE_REQUEST:
                                if (this->verbose & 2) {
                                    LOG(LOG_INFO, "Rdp::License Request");
                                }
                                {
                                    LIC::LicenseRequest_Recv lic(sec.payload);
                                    uint8_t null_data[SEC_MODULUS_SIZE];
                                    memset(null_data, 0, sizeof(null_data));
                                    /* We currently use null client keys. This is a bit naughty but, hey,
                                       the security of license negotiation isn't exactly paramount. */
                                    SEC::SessionKey keyblock(null_data, null_data, lic.server_random);

                                    /* Store first 16 bytes of session key as MAC secret */
                                    memcpy(this->lic_layer_license_sign_key, keyblock.get_MAC_salt_key(), 16);
                                    memcpy(this->lic_layer_license_key, keyblock.get_LicensingEncryptionKey(), 16);

                                    BStream sec_header(256);
                                    HStream lic_data(1024, 65535);

                                    if (this->lic_layer_license_size > 0) {
                                        uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                                        buf_out_uint32(hwid, 2);
                                        memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                                        /* Generate a signature for the HWID buffer */
                                        uint8_t signature[LIC::LICENSE_SIGNATURE_SIZE];

                                        uint8_t lenhdr[4];
                                        buf_out_uint32(lenhdr, sizeof(hwid));

                                        Sign sign(this->lic_layer_license_sign_key, 16);
                                        sign.update(lenhdr, sizeof(lenhdr));
                                        sign.update(hwid, sizeof(hwid));

                                        assert(MD5_DIGEST_LENGTH == LIC::LICENSE_SIGNATURE_SIZE);
                                        sign.final(signature, sizeof(signature));


                                        /* Now encrypt the HWID */

                                        SslRC4 rc4;
                                        rc4.set_key(this->lic_layer_license_key, 16);

                                        // in, out
                                        rc4.crypt(LIC::LICENSE_HWID_SIZE, hwid, hwid);

                                        LIC::ClientLicenseInfo_Send(lic_data, this->use_rdp5?3:2,
                                                                    this->lic_layer_license_size, this->lic_layer_license_data, hwid, signature);
                                    }
                                    else {
                                        LIC::NewLicenseRequest_Send(lic_data, this->use_rdp5?3:2, username, hostname);
                                    }

                                    SEC::Sec_Send sec(sec_header, lic_data,
                                        SEC::SEC_LICENSE_PKT, this->encrypt, 0);
                                    lic_data.copy_to_head(sec_header.get_data(), sec_header.size());

                                    this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, lic_data);
                                }
                                break;
                            case LIC::PLATFORM_CHALLENGE:
                                if (this->verbose & 2){
                                    LOG(LOG_INFO, "Rdp::Platform Challenge");
                                }
                                {
                                    LIC::PlatformChallenge_Recv lic(sec.payload);


                                    uint8_t out_token[LIC::LICENSE_TOKEN_SIZE];
                                    uint8_t decrypt_token[LIC::LICENSE_TOKEN_SIZE];
                                    uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                                    uint8_t crypt_hwid[LIC::LICENSE_HWID_SIZE];
                                    uint8_t out_sig[LIC::LICENSE_SIGNATURE_SIZE];

                                    memcpy(out_token, lic.encryptedPlatformChallenge.blob, LIC::LICENSE_TOKEN_SIZE);
                                    /* Decrypt the token. It should read TEST in Unicode. */
                                    memcpy(decrypt_token, lic.encryptedPlatformChallenge.blob, LIC::LICENSE_TOKEN_SIZE);
                                    SslRC4 rc4_decrypt_token;
                                    rc4_decrypt_token.set_key(this->lic_layer_license_key, 16);
                                    // size, in, out
                                    rc4_decrypt_token.crypt(LIC::LICENSE_TOKEN_SIZE, decrypt_token, decrypt_token);

                                    /* Generate a signature for a buffer of token and HWID */
                                    buf_out_uint32(hwid, 2);
                                    memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                                    uint8_t sealed_buffer[LIC::LICENSE_TOKEN_SIZE + LIC::LICENSE_HWID_SIZE];
                                    memcpy(sealed_buffer, decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                                    memcpy(sealed_buffer + LIC::LICENSE_TOKEN_SIZE, hwid, LIC::LICENSE_HWID_SIZE);

                                    uint8_t lenhdr[4];
                                    buf_out_uint32(lenhdr, sizeof(sealed_buffer));

                                    Sign sign(this->lic_layer_license_sign_key, 16);
                                    sign.update(lenhdr, sizeof(lenhdr));
                                    sign.update(sealed_buffer, sizeof(sealed_buffer));

                                    assert(MD5_DIGEST_LENGTH == LIC::LICENSE_SIGNATURE_SIZE);
                                    sign.final(out_sig, sizeof(out_sig));

                                    /* Now encrypt the HWID */
                                    memcpy(crypt_hwid, hwid, LIC::LICENSE_HWID_SIZE);
                                    SslRC4 rc4_hwid;
                                    rc4_hwid.set_key(this->lic_layer_license_key, 16);
                                    // size, in, out
                                    rc4_hwid.crypt(LIC::LICENSE_HWID_SIZE, crypt_hwid, crypt_hwid);

                                    BStream sec_header(256);
                                    HStream lic_data(1024, 65535);

                                    LIC::ClientPlatformChallengeResponse_Send(lic_data, this->use_rdp5?3:2, out_token, crypt_hwid, out_sig);
                                    SEC::Sec_Send sec(sec_header, lic_data, SEC::SEC_LICENSE_PKT, this->encrypt, 0);
                                    lic_data.copy_to_head(sec_header.get_data(), sec_header.size());
                                    this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, lic_data);
                                }
                                break;
                            case LIC::NEW_LICENSE:
                                {
                                    if (this->verbose & 2){
                                        LOG(LOG_INFO, "Rdp::New License");
                                    }

                                    LIC::NewLicense_Recv lic(sec.payload, this->lic_layer_license_key);

                                    TODO("CGR: Save license to keep a local copy of the license of a remote server thus avoiding to ask it every time we connect. Not obvious files is the best choice to do that");
                                        this->state = MOD_RDP_CONNECTED;

                                    LOG(LOG_WARNING, "New license not saved");
                                }
                                break;
                            case LIC::UPGRADE_LICENSE:
                                {
                                    if (this->verbose & 2){
                                        LOG(LOG_INFO, "Rdp::Upgrade License");
                                    }
                                    LIC::UpgradeLicense_Recv lic(sec.payload, this->lic_layer_license_key);

                                    LOG(LOG_WARNING, "Upgraded license not saved");
                                }
                                break;
                            case LIC::ERROR_ALERT:
                                {
                                    if (this->verbose & 2){
                                        LOG(LOG_INFO, "Rdp::Get license status");
                                    }
                                    LIC::ErrorAlert_Recv lic(sec.payload);
                                    if ((lic.validClientMessage.dwErrorCode == LIC::STATUS_VALID_CLIENT)
                                        && (lic.validClientMessage.dwStateTransition == LIC::ST_NO_TRANSITION)){
                                        this->state = MOD_RDP_CONNECTED;
                                    }
                                    else {
                                        LOG(LOG_ERR, "RDP::License Alert: error=%u transition=%u",
                                            lic.validClientMessage.dwErrorCode, lic.validClientMessage.dwStateTransition);
                                    }
                                    this->state = MOD_RDP_CONNECTED;
                                }
                                break;
                            default:
                                {
                                    LOG(LOG_WARNING, "Unexpected license tag sent from server (tag = %x)", flic.tag);
                                    throw Error(ERR_SEC);
                                }
                                break;
                            }

                            if (sec.payload.p != sec.payload.end){
                                LOG(LOG_ERR, "all data should have been consumed %s:%u tag = %x", __FILE__, __LINE__, flic.tag);
                                throw Error(ERR_SEC);
                            }
                        }
                        else {
                            LOG(LOG_WARNING, "Failed to get expected license negotiation PDU");
                            hexdump(x224.payload.get_data(), x224.payload.size());
                            //throw Error(ERR_SEC);
                            this->state = MOD_RDP_CONNECTED;
                            sec.payload.p = sec.payload.end;
                            hexdump(sec.payload.get_data(), sec.payload.size());
                        }
                    }
                    break;

                    // Capabilities Exchange
                    // ---------------------

                    // Capabilities Negotiation: The server sends the set of capabilities it
                    // supports to the client in a Demand Active PDU. The client responds with its
                    // capabilities by sending a Confirm Active PDU.

                    // Client                                                     Server
                    //    | <------- Demand Active PDU ---------------------------- |
                    //    |--------- Confirm Active PDU --------------------------> |

                    // Connection Finalization
                    // -----------------------

                    // Connection Finalization: The client and server send PDUs to finalize the
                    // connection details. The client-to-server and server-to-client PDUs exchanged
                    // during this phase may be sent concurrently as long as the sequencing in
                    // either direction is maintained (there are no cross-dependencies between any
                    // of the client-to-server and server-to-client PDUs). After the client receives
                    // the Font Map PDU it can start sending mouse and keyboard input to the server,
                    // and upon receipt of the Font List PDU the server can start sending graphics
                    // output to the client.

                    // Client                                                     Server
                    //    |----------Synchronize PDU------------------------------> |
                    //    |----------Control PDU Cooperate------------------------> |
                    //    |----------Control PDU Request Control------------------> |
                    //    |----------Persistent Key List PDU(s)-------------------> |
                    //    |----------Font List PDU--------------------------------> |

                    //    | <--------Synchronize PDU------------------------------- |
                    //    | <--------Control PDU Cooperate------------------------- |
                    //    | <--------Control PDU Granted Control------------------- |
                    //    | <--------Font Map PDU---------------------------------- |

                    // All PDU's in the client-to-server direction must be sent in the specified
                    // order and all PDU's in the server to client direction must be sent in the
                    // specified order. However, there is no requirement that client to server PDU's
                    // be sent before server-to-client PDU's. PDU's may be sent concurrently as long
                    // as the sequencing in either direction is maintained.


                    // Besides input and graphics data, other data that can be exchanged between
                    // client and server after the connection has been finalized include "
                    // connection management information and virtual channel messages (exchanged
                    // between client-side plug-ins and server-side applications).

                case MOD_RDP_CONNECTED:
                    {
                        // read tpktHeader (4 bytes = 3 0 len)
                        // TPDU class 0    (3 bytes = LI F0 PDU_DT)

                        // Detect fast-path PDU
                        Array array(65536);
                        uint8_t * end = array.get_data();
                        X224::RecvFactory fx224(this->nego.trans, &end, array.size(), true);
                        InStream stream(array, 0, 0, end - array.get_data());

                        if (fx224.fast_path) {
                            FastPath::ServerUpdatePDU_Recv su(stream, this->decrypt);
                            if (this->enable_transparent_mode) {
                                //total_data_received += su.payload.size();
                                //LOG(LOG_INFO, "total_data_received=%llu", total_data_received);
                                if (this->transparent_recorder) {
                                    this->transparent_recorder->send_fastpath_data(su.payload);
                                }
                                this->front.send_fastpath_data(su.payload);

                                break;
                            }

                            while (su.payload.in_remain()) {
                                FastPath::Update_Recv upd(su.payload, &this->mppc_dec);

                                switch (upd.updateCode) {
                                case FastPath::FASTPATH_UPDATETYPE_ORDERS:
                                    this->front.begin_update();
                                    this->orders.process_orders(this->bpp, upd.payload, true, *this->gd,
                                                                this->front_width, this->front_height);
                                    this->front.end_update();

                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_ORDERS"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_BITMAP:
                                    this->front.begin_update();
                                    this->process_bitmap_updates(upd.payload, true);
                                    this->front.end_update();

                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_BITMAP"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PALETTE:
                                    this->front.begin_update();
                                    this->process_palette(upd.payload, true);
                                    this->front.end_update();

                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PALETTE"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_SYNCHRONIZE:
                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_SYNCHRONIZE"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PTR_NULL:
                                    {
                                        if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PTR_NULL"); }
                                        struct Pointer cursor;
                                        memset(cursor.mask, 0xff, sizeof(cursor.mask));
                                        this->front.server_set_pointer(cursor);
                                    }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PTR_DEFAULT:
                                    {
                                        if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PTR_DEFAULT"); }
                                        Pointer cursor(Pointer::POINTER_SYSTEM_DEFAULT);
                                        this->front.server_set_pointer(cursor);
                                    }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PTR_POSITION:
                                    {
                                        if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PTR_POSITION"); }
                                        uint16_t xPos = upd.payload.in_uint16_le();
                                        uint16_t yPos = upd.payload.in_uint16_le();
                                        this->front.update_pointer_position(xPos, yPos);
                                    }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_COLOR:
                                    this->process_color_pointer_pdu(upd.payload);

                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_COLOR"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_POINTER:
                                    this->process_new_pointer_pdu(upd.payload);

                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_POINTER"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_CACHED:
                                    this->process_cached_pointer_pdu(upd.payload);

                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_CACHED"); }
                                    break;

                                default:
                                    LOG( LOG_INFO
                                       , "mod::rdp: received unexpected fast-path PUD, updateCode = %u"
                                       , upd.updateCode);
                                    throw Error(ERR_RDP_FASTPATH);
                                }
                            }

                            TODO("Chech all data in the PDU is consumed");
                            break;
                        }

                        X224::DT_TPDU_Recv x224(stream);

                        const int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);

                        if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum){
                            LOG(LOG_INFO, "mod::rdp::DisconnectProviderUltimatum received");
                            x224.payload.rewind();
                            MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
                            const char * reason = MCS::get_reason(mcs.reason);
                            LOG(LOG_INFO, "mod::rdp::DisconnectProviderUltimatum: reason=%s [%d]", reason, mcs.reason);
                            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
                        }


                        MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);
                        SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);

                        if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL){
                            if (this->verbose & 16) {
                                LOG(LOG_INFO, "received channel data on mcs.chanid=%u", mcs.channelId);
                            }

                            int num_channel_src = this->mod_channel_list.get_index_by_id(mcs.channelId);
                            if (num_channel_src == -1) {
                                LOG(LOG_WARNING, "mod::rdp::MOD_RDP_CONNECTED::Unknown Channel id=%d", mcs.channelId);
                                throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                            }

                            const CHANNELS::ChannelDef & mod_channel = this->mod_channel_list[num_channel_src];
                            if (this->verbose & 16) {
                                mod_channel.log(num_channel_src);
                            }

                            uint32_t length = sec.payload.in_uint32_le();
                            int flags = sec.payload.in_uint32_le();
                            size_t chunk_size = sec.payload.in_remain();

                            // If channel name is our virtual channel, then don't send data to front
                                 if (  this->auth_channel[0] /*&& this->acl */
                                    && !strcmp(mod_channel.name, this->auth_channel)) {
                                this->process_auth_event(mod_channel, sec.payload, length, flags, chunk_size);
                            }
                            else if (!strcmp(mod_channel.name, "wabagt")) {
                                this->process_wab_agent_event(mod_channel, sec.payload, length, flags, chunk_size);
                            }
                            // Clipboard is a Clipboard PDU
                            else if (!strcmp(mod_channel.name, channel_names::cliprdr)) {
                                this->process_cliprdr_event(mod_channel, sec.payload, length, flags, chunk_size);
                            }
                            else if (!strcmp(mod_channel.name, channel_names::rail)) {
                                this->process_rail_event(mod_channel, sec.payload, length, flags, chunk_size);
                            }
                            else if (!strcmp(mod_channel.name, channel_names::rdpdr)) {
                                this->process_rdpdr_event(mod_channel, sec.payload, length, flags, chunk_size);
                            }
                            else {
                                this->send_to_front_channel(
                                    mod_channel.name, sec.payload.p, length, chunk_size, flags
                                );
                            }
                            sec.payload.p = sec.payload.end;
                        }
                        else {
                            uint8_t * next_packet = sec.payload.p;
                            while (next_packet < sec.payload.end) {
                                sec.payload.p = next_packet;

                                uint8_t * current_packet = next_packet;

                                if  (peekFlowPDU(sec.payload)){
                                    if (this->verbose & 128) {
                                        LOG(LOG_WARNING, "FlowPDU TYPE");
                                    }
                                    ShareFlow_Recv sflow(sec.payload);
                                    // ignoring
                                    // if (sctrl.flow_pdu_type == FLOW_TEST_PDU) {
                                    //     this->send_flow_response_pdu(sctrl.flow_id,
                                    //                                  sctrl.flow_number);
                                    // }
                                    next_packet = sec.payload.p;
                                }
                                else {
                                    ShareControl_Recv sctrl(sec.payload);
                                    next_packet += sctrl.totalLength;

                                    if (this->verbose & 128) {
                                        LOG(LOG_WARNING, "LOOPING on PDUs: %u", (unsigned)sctrl.totalLength);
                                    }

                                    switch (sctrl.pduType) {
                                    case PDUTYPE_DATAPDU:
                                        if (this->verbose & 128) {
                                            LOG(LOG_WARNING, "PDUTYPE_DATAPDU");
                                        }
                                        switch (this->connection_finalization_state){
                                        case EARLY:
                                            LOG(LOG_WARNING, "Rdp::finalization is early");
                                            throw Error(ERR_SEC);
                                        case WAITING_SYNCHRONIZE:
                                            if (this->verbose & 1){
                                                LOG(LOG_WARNING, "WAITING_SYNCHRONIZE");
                                            }
                                            //this->check_data_pdu(PDUTYPE2_SYNCHRONIZE);
                                            this->connection_finalization_state = WAITING_CTL_COOPERATE;
                                            {
                                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                                sdata.payload.p = sdata.payload.end;
                                            }
                                            break;
                                        case WAITING_CTL_COOPERATE:
                                            if (this->verbose & 1){
                                                LOG(LOG_WARNING, "WAITING_CTL_COOPERATE");
                                            }
                                            //this->check_data_pdu(PDUTYPE2_CONTROL);
                                            this->connection_finalization_state = WAITING_GRANT_CONTROL_COOPERATE;
                                            {
                                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                                sdata.payload.p = sdata.payload.end;
                                            }
                                            break;
                                        case WAITING_GRANT_CONTROL_COOPERATE:
                                            if (this->verbose & 1){
                                                LOG(LOG_WARNING, "WAITING_GRANT_CONTROL_COOPERATE");
                                            }
                                            //                            this->check_data_pdu(PDUTYPE2_CONTROL);
                                            this->connection_finalization_state = WAITING_FONT_MAP;
                                            {
                                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                                sdata.payload.p = sdata.payload.end;
                                            }
                                            break;
                                        case WAITING_FONT_MAP:
                                            if (this->verbose & 1){
                                                LOG(LOG_WARNING, "PDUTYPE2_FONTMAP");
                                            }
                                            //this->check_data_pdu(PDUTYPE2_FONTMAP);
                                            this->connection_finalization_state = UP_AND_RUNNING;

                                            // Synchronize sent to indicate server the state of sticky keys (x-locks)
                                            // Must be sent at this point of the protocol (sent before, it xwould be ignored or replaced)
                                            rdp_input_synchronize(0, 0, (this->key_flags & 0x07), 0);
                                            {
                                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                                sdata.payload.p = sdata.payload.end;
                                            }
                                            break;
                                        case UP_AND_RUNNING:
                                            if (this->enable_transparent_mode)
                                            {
                                                sec.payload.p = current_packet;

                                                HStream copy_stream(1024, 65535);

                                                copy_stream.out_copy_bytes(current_packet, next_packet - current_packet);
                                                copy_stream.mark_end();

                                                //total_data_received += copy_stream.size();
                                                //LOG(LOG_INFO, "total_data_received=%llu", total_data_received);

                                                if (this->transparent_recorder) {
                                                    this->transparent_recorder->send_data_indication_ex(mcs.channelId,
                                                        copy_stream);
                                                }
                                                this->front.send_data_indication_ex(mcs.channelId, copy_stream);

                                                next_packet = sec.payload.end;

                                                break;
                                            }

                                            {
                                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                                switch (sdata.pdutype2) {
                                                case PDUTYPE2_UPDATE:
                                                    {
                                                        if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_UPDATE"); }
                                                        // MS-RDPBCGR: 1.3.6
                                                        // -----------------
                                                        // The most fundamental output that a server can send to a connected client
                                                        // is bitmap images of the remote session using the Update Bitmap PDU. This
                                                        // allows the client to render the working space and enables a user to
                                                        // interact with the session running on the server. The global palette
                                                        // information for a session is sent to the client in the Update Palette PDU.

                                                        SlowPath::GraphicsUpdate_Recv gur(sdata.payload);
                                                        switch (gur.update_type) {
                                                        case RDP_UPDATE_ORDERS:
                                                            if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_ORDERS"); }
                                                            this->front.begin_update();
                                                            this->orders.process_orders(this->bpp, sdata.payload, false, *this->gd,
                                                                                        this->front_width, this->front_height);
                                                            this->front.end_update();
                                                            break;
                                                        case RDP_UPDATE_BITMAP:
                                                            if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_BITMAP");}
                                                            this->front.begin_update();
                                                            this->process_bitmap_updates(sdata.payload, false);
                                                            this->front.end_update();
                                                            break;
                                                        case RDP_UPDATE_PALETTE:
                                                            if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_PALETTE");}
                                                            this->front.begin_update();
                                                            this->process_palette(sdata.payload, false);
                                                            this->front.end_update();
                                                            break;
                                                        case RDP_UPDATE_SYNCHRONIZE:
                                                            if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_SYNCHRONIZE");}
                                                            sdata.payload.in_skip_bytes(2);
                                                            break;
                                                        default:
                                                            if (this->verbose & 8){ LOG(LOG_WARNING, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_UNKNOWN");}
                                                            break;
                                                        }
                                                    }
                                                    break;
                                                case PDUTYPE2_CONTROL:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_CONTROL");}
                                                    TODO("CGR: Data should actually be consumed");
                                                        sdata.payload.p = sdata.payload.end;
                                                    break;
                                                case PDUTYPE2_SYNCHRONIZE:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");}
                                                    TODO("CGR: Data should actually be consumed");
                                                        sdata.payload.p = sdata.payload.end;
                                                    break;
                                                case PDUTYPE2_POINTER:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_POINTER");}
                                                    this->process_pointer_pdu(sdata.payload, this);
                                                    TODO("CGR: Data should actually be consumed");
                                                        sdata.payload.p = sdata.payload.end;
                                                    break;
                                                case PDUTYPE2_PLAY_SOUND:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_PLAY_SOUND");}
                                                    TODO("CGR: Data should actually be consumed");
                                                        sdata.payload.p = sdata.payload.end;
                                                    break;
                                                case PDUTYPE2_SAVE_SESSION_INFO:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SAVE_SESSION_INFO");}
                                                    TODO("CGR: Data should actually be consumed");
                                                    this->process_save_session_info(sdata.payload);
                                                    break;
                                                case PDUTYPE2_SET_ERROR_INFO_PDU:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");}
                                                    this->process_disconnect_pdu(sdata.payload);
                                                    break;
                                                case PDUTYPE2_SHUTDOWN_DENIED:
                                                    if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED");}
                                                    LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED Received");
                                                    break;
                                                default:
                                                    LOG(LOG_WARNING, "PDUTYPE2 unsupported tag=%u", sdata.pdutype2);
                                                    TODO("CGR: Data should actually be consumed");
                                                        sdata.payload.p = sdata.payload.end;
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                        break;
                                    case PDUTYPE_DEMANDACTIVEPDU:
                                        {
                                            if (this->verbose & 128){
                                                 LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");
                                            }

                                            this->orders.reset();

        // 2.2.1.13.1.1 Demand Active PDU Data (TS_DEMAND_ACTIVE_PDU)
        // ==========================================================

        //    shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1 ) containing information
        //  about the packet. The type subfield of the pduType field of the Share Control Header MUST be set to
        // PDUTYPE_DEMANDACTIVEPDU (1).

        //    shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the packet (see [T128]
        // section 8.4.2 for more information regarding share IDs).

                                            this->share_id = sctrl.payload.in_uint32_le();

        //    lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes of the sourceDescriptor
        // field.
                                            uint16_t lengthSourceDescriptor = sctrl.payload.in_uint16_le();

        //    lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined size in bytes of the
        // numberCapabilities, pad2Octets, and capabilitySets fields.

                                            uint16_t lengthCombinedCapabilities = sctrl.payload.in_uint16_le();

        //    sourceDescriptor (variable): A variable-length array of bytes containing a source descriptor (see
        // [T128] section 8.4.1 for more information regarding source descriptors).

                                            TODO("before skipping we should check we do not go outside current stream");
                                            sctrl.payload.in_skip_bytes(lengthSourceDescriptor);

        // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets included in the
        // Demand Active PDU.

        // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

        // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number
        //  of capability sets is specified by the numberCapabilities field.

                                            this->process_server_caps(sctrl.payload, lengthCombinedCapabilities);

        // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.

                                            uint32_t sessionId = sctrl.payload.in_uint32_le();
                                            (void)sessionId;

                                            this->send_confirm_active(this);
                                            this->send_synchronise();
                                            this->send_control(RDP_CTL_COOPERATE);
                                            this->send_control(RDP_CTL_REQUEST_CONTROL);

                                            /* Including RDP 5.0 capabilities */
                                            if (this->use_rdp5){
                                                LOG(LOG_INFO, "use rdp5");
                                                if (this->enable_persistent_disk_bitmap_cache &&
                                                    this->persist_bitmap_cache_on_disk) {
                                                    if (!this->deactivation_reactivation_in_progress) {
                                                        this->send_persistent_key_list();
                                                    }
                                                }
                                                this->send_fonts(3);
                                            }
                                            else{
                                                LOG(LOG_INFO, "not using rdp5");
                                                this->send_fonts(1);
                                                this->send_fonts(2);
                                            }

                                            this->send_input(0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);

                                            LOG(LOG_INFO, "Resizing to %ux%ux%u", this->front_width, this->front_height, this->bpp);
                                            if (this->transparent_recorder) {
                                                this->transparent_recorder->server_resize(this->front_width,
                                                    this->front_height, this->bpp);
                                            }
                                            if (-1 == this->front.server_resize(this->front_width, this->front_height, this->bpp)){
                                                LOG(LOG_WARNING, "Resize not available on older clients,"
                                                    " change client resolution to match server resolution");
                                                throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
                                            }
    //                                        this->orders.reset();
                                            this->connection_finalization_state = WAITING_SYNCHRONIZE;

                                            this->deactivation_reactivation_in_progress = false;
                                        }
                                        break;
                                    case PDUTYPE_DEACTIVATEALLPDU:
                                        if (this->verbose & 128){ LOG(LOG_INFO, "PDUTYPE_DEACTIVATEALLPDU"); }
                                        LOG(LOG_INFO, "Deactivate All PDU");
                                        this->deactivation_reactivation_in_progress = true;
                                        TODO("CGR: Data should actually be consumed");
                                            TODO("CGR: Check we are indeed expecting Synchronize... dubious");
                                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                                        break;
                                    case PDUTYPE_SERVER_REDIR_PKT:
                                        {
                                            if (this->verbose & 128){
                                                LOG(LOG_INFO, "PDUTYPE_SERVER_REDIR_PKT");
                                            }
                                            sctrl.payload.in_skip_bytes(2);
                                            ServerRedirectionPDU server_redirect;
                                            server_redirect.receive(sctrl.payload);
                                            sctrl.payload.in_skip_bytes(1);
                                            server_redirect.export_to_redirection_info(this->redir_info);
                                            if (this->verbose & 128){
                                                server_redirect.log(LOG_INFO, "Got Packet");
                                                this->redir_info.log(LOG_INFO, "RInfo Ini");
                                            }
                                            if (!server_redirect.Noredirect()) {
                                                LOG(LOG_INFO, "Server Redirection thrown");
                                                throw Error(ERR_RDP_SERVER_REDIR);
                                            }
                                        }
                                        break;
                                    default:
                                        LOG(LOG_INFO, "unknown PDU %u", sctrl.pduType);
                                        break;
                                    }
                                TODO("check sctrl.payload is completely consumed");
                                }
                            }
                        }
                    }
                }
            }
            catch(Error const & e){
                if (e.id == ERR_RDP_SERVER_REDIR) {
                    throw;
                }
                if (this->acl)
                {
                    char message[128];
                    snprintf(message, sizeof(message), "Code=%d", e.id);
                    this->acl->report("SESSION_EXCEPTION", message);

                    this->end_session_reason.clear();
                    this->end_session_message.clear();
                }

                BStream stream(256);
                X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
                try {
                    this->nego.trans.send(stream);
                    LOG(LOG_INFO, "Connection to server closed");
                }
                catch(Error const & e){
                    LOG(LOG_INFO, "Connection to server Already closed: error=%d", e.id);
                };
                this->event.signal = BACK_EVENT_NEXT;

                if ((e.id == ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED) ||
                    (e.id == ERR_NLA_AUTHENTICATION_FAILED))
                {
                    throw;
                }
            }
        }

        if (this->open_session_timeout) {
            switch(this->open_session_timeout_checker.check(now)) {
            case TimeoutT<time_t>::TIMEOUT_REACHED:
                if (this->error_message) {
                    *this->error_message = "Logon timer expired!";
                }
                LOG(LOG_ERR,
                    "Logon timer expired on %s. The session will be disconnected.",
                    this->hostname);
                if (this->acl)
                {
                    this->acl->report("CONNECTION_FAILED", "Logon timer expired.");
                }

                this->event.signal = BACK_EVENT_NEXT;
                this->event.set();
            break;
            case TimeoutT<time_t>::TIMEOUT_NOT_REACHED:
                this->event.set(1000000);
            break;
            case TimeoutT<time_t>::TIMEOUT_INACTIVE:
            break;
            }
        }

        if (this->client_device_list_announce_timer_enabled) {
            TimeVal tv_now;
            switch (this->client_device_list_announce_timeout_checker.check(tv_now)) {
                case TimeoutT<TimeVal>::TIMEOUT_REACHED:
                {
                    LOG(LOG_INFO,
                        "mod_rdp::draw_event: Client Device List announce timer expired.");
                    this->event.reset();
                    this->client_device_list_announce_timer_enabled = false;


                    {
                        const CHANNELS::ChannelDef * rdpdr_channel =
                            this->mod_channel_list.get_by_name(channel_names::rdpdr);
                        if (rdpdr_channel) {
                            BStream result(this->chunked_virtual_channel_data_stream.get_capacity());

                            if (this->filter_unsupported_device(this->authorization_channels,
                                                                result, // Fake data.
                                                                0,
                                                                result,
                                                                this->file_system_drive_manager,
                                                                this->device_capability_version_02_supported,
                                                                this->verbose)) {
                                this->send_to_channel(*rdpdr_channel, result, result.size(),
                                                      CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
                            }
                        }
                    }
                }
                break;

                case TimeoutT<TimeVal>::TIMEOUT_NOT_REACHED:
                {
                    TimeVal timeleft = this->client_device_list_announce_timeout_checker.timeleft(tv_now);

                    this->event.set(timeleft.tv_sec * 1000000LL + timeleft.tv_usec);
                }
                break;

                case TimeoutT<TimeVal>::TIMEOUT_INACTIVE:
                break;
            }
        }
    }   // draw_event

    // 1.3.1.3 Deactivation-Reactivation Sequence
    // ==========================================

    // After the connection sequence has run to completion, the server may determine
    // that the client needs to be connected to a waiting, disconnected session. To
    // accomplish this task the server signals the client with a Deactivate All PDU.
    // A Deactivate All PDU implies that the connection will be dropped or that a
    // capability renegotiation will occur. If a capability renegotiation needs to
    // be performed then the server will re-execute the connection sequence,
    // starting with the Demand Active PDU (the Capability Negotiation and
    // Connection Finalization phases as described in section 1.3.1.1) but excluding
    // the Persistent Key List PDU.


    // 2.2.1.13.1.1 Demand Active PDU Data (TS_DEMAND_ACTIVE_PDU)
    // ==========================================================
    // The TS_DEMAND_ACTIVE_PDU structure is a standard T.128 Demand Active PDU (see [T128] section 8.4.1).

    // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1) containing information about the packet. The type subfield of the pduType field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).

    // shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the packet (see [T128] section 8.4.2 for more information regarding share IDs).

    // lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes of the sourceDescriptor field.

    // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined size in bytes of the numberCapabilities, pad2Octets, and capabilitySets fields.

    // sourceDescriptor (variable): A variable-length array of bytes containing a source descriptor (see [T128] section 8.4.1 for more information regarding source descriptors).

    // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets include " in the Demand Active PDU.

    // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

    // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number of capability sets is specified by the numberCapabilities field.

    // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.

    void send_confirm_active(mod_api * mod) throw(Error) {
        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_confirm_active");
        }

        BStream stream(65536);

        RDP::ConfirmActivePDU_Send confirm_active_pdu(stream);

        confirm_active_pdu.emit_begin(this->share_id);

        GeneralCaps general_caps;
        general_caps.extraflags  =
            this->use_rdp5
            ? NO_BITMAP_COMPRESSION_HDR | AUTORECONNECT_SUPPORTED | LONG_CREDENTIALS_SUPPORTED
            : 0
            ;
        // Slow/Fast-path
        general_caps.extraflags |=
            this->enable_fastpath_server_update
            ? FASTPATH_OUTPUT_SUPPORTED
            : 0
            ;
        if (this->enable_transparent_mode) {
            this->front.retrieve_client_capability_set(general_caps);
        }
        if (this->verbose & 1) {
            general_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(general_caps);

        BitmapCaps bitmap_caps;
        TODO("Client SHOULD set this field to the color depth requested in the Client Core Data")
        bitmap_caps.preferredBitsPerPixel = this->bpp;
        //bitmap_caps.preferredBitsPerPixel = this->front_bpp;
        bitmap_caps.desktopWidth          = this->front_width;
        bitmap_caps.desktopHeight         = this->front_height;
        bitmap_caps.bitmapCompressionFlag = 0x0001; // This field MUST be set to TRUE (0x0001).
        //bitmap_caps.drawingFlags = DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY | DRAW_ALLOW_COLOR_SUBSAMPLING | DRAW_ALLOW_SKIP_ALPHA;
        bitmap_caps.drawingFlags = DRAW_ALLOW_SKIP_ALPHA;
        if (this->enable_transparent_mode) {
            this->front.retrieve_client_capability_set(bitmap_caps);
        }
        if (this->verbose & 1) {
            bitmap_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(bitmap_caps);

        OrderCaps order_caps;
        order_caps.numberFonts                                   = 0;
        order_caps.orderFlags                                    = /*0x2a*/
                                                                    NEGOTIATEORDERSUPPORT   /* 0x02 */
                                                                  | ZEROBOUNDSDELTASSUPPORT /* 0x08 */
                                                                  | COLORINDEXSUPPORT       /* 0x20 */
                                                                  | ORDERFLAGS_EXTRA_FLAGS  /* 0x80 */
                                                                  ;
        order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX]        = (this->enable_multidstblt     ? 1 : 0);
        order_caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]    = (this->enable_multiopaquerect ? 1 : 0);
        order_caps.orderSupport[TS_NEG_MULTIPATBLT_INDEX]        = (this->enable_multipatblt     ? 1 : 0);
        order_caps.orderSupport[TS_NEG_MULTISCRBLT_INDEX]        = (this->enable_multiscrblt     ? 1 : 0);
        order_caps.orderSupport[TS_NEG_PATBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]            = (this->enable_mem3blt         ? 1 : 0);
        order_caps.orderSupport[TS_NEG_LINETO_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = 0;
        order_caps.orderSupport[UnusedIndex3]                    = 1;
        order_caps.orderSupport[UnusedIndex5]                    = 1;
        order_caps.orderSupport[TS_NEG_POLYGON_SC_INDEX]         = (this->enable_polygonsc       ? 1 : 0);
        order_caps.orderSupport[TS_NEG_POLYGON_CB_INDEX]         = (this->enable_polygoncb       ? 1 : 0);
        order_caps.orderSupport[TS_NEG_POLYLINE_INDEX]           = (this->enable_polyline        ? 1 : 0);
        order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]         = (this->enable_ellipsesc       ? 1 : 0);
        order_caps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX]         = (this->enable_ellipsecb       ? 1 : 0);
        order_caps.orderSupport[TS_NEG_INDEX_INDEX]              = 1;

        order_caps.textFlags                                     = 0x06a1;
        order_caps.orderSupportExFlags                           = ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT;
        order_caps.textANSICodePage                              = 0x4e4; // Windows-1252 codepage is passed (latin-1)

        // Apparently, these primary drawing orders are supported
        // by both rdesktop and xfreerdp :
        // TS_NEG_DSTBLT_INDEX
        // TS_NEG_PATBLT_INDEX
        // TS_NEG_SCRBLT_INDEX
        // TS_NEG_MEMBLT_INDEX
        // TS_NEG_LINETO_INDEX
        // others orders may not be supported.

        // intersect with client order capabilities
        // which may not be supported by clients.
        this->front.intersect_order_caps(TS_NEG_DSTBLT_INDEX,             order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_PATBLT_INDEX,             order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_SCRBLT_INDEX,             order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_LINETO_INDEX,             order_caps.orderSupport);

        this->front.intersect_order_caps(TS_NEG_MULTIDSTBLT_INDEX,        order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_MULTIOPAQUERECT_INDEX,    order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_MULTIPATBLT_INDEX,        order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_MULTISCRBLT_INDEX,        order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_MEMBLT_INDEX,             order_caps.orderSupport);
        if ((this->verbose & 1) && (!order_caps.orderSupport[TS_NEG_MEMBLT_INDEX])) {
            LOG(LOG_INFO, "MemBlt Primary Drawing Order is disabled.");
        }
        this->front.intersect_order_caps(TS_NEG_MEM3BLT_INDEX,            order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_MULTI_DRAWNINEGRID_INDEX, order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_POLYGON_SC_INDEX,         order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_POLYGON_CB_INDEX,         order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_POLYLINE_INDEX,           order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_ELLIPSE_SC_INDEX,         order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_ELLIPSE_CB_INDEX,         order_caps.orderSupport);
        this->front.intersect_order_caps(TS_NEG_INDEX_INDEX,              order_caps.orderSupport);

        this->front.intersect_order_caps_ex(order_caps);

        // LOG(LOG_INFO, ">>>>>>>>ORDER CAPABILITIES : ELLIPSE : %d",
        //     order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]);
        if (this->enable_transparent_mode) {
            this->front.retrieve_client_capability_set(order_caps);
        }
        if (this->verbose & 1) {
            order_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(order_caps);


        BmpCacheCaps bmpcache_caps;
        bmpcache_caps.cache0Entries         = 0x258;
        bmpcache_caps.cache0MaximumCellSize = nbbytes(this->bpp) * 0x100;
        bmpcache_caps.cache1Entries         = 0x12c;
        bmpcache_caps.cache1MaximumCellSize = nbbytes(this->bpp) * 0x400;
        bmpcache_caps.cache2Entries         = 0x106;
        bmpcache_caps.cache2MaximumCellSize = nbbytes(this->bpp) * 0x1000;

        BmpCache2Caps bmpcache2_caps;
        bmpcache2_caps.cacheFlags           = PERSISTENT_KEYS_EXPECTED_FLAG | (this->enable_cache_waiting_list ? ALLOW_CACHE_WAITING_LIST_FLAG : 0);
        bmpcache2_caps.numCellCaches        = 3;
        bmpcache2_caps.bitmapCache0CellInfo = 120;
        bmpcache2_caps.bitmapCache1CellInfo = 120;
        bmpcache2_caps.bitmapCache2CellInfo = (2553 | 0x80000000);

        bool use_bitmapcache_rev2 = false;

        if (this->enable_transparent_mode) {
            if (!this->front.retrieve_client_capability_set(bmpcache_caps)) {
                this->front.retrieve_client_capability_set(bmpcache2_caps);
                use_bitmapcache_rev2 = true;
            }
        }
        else {
            use_bitmapcache_rev2 = this->enable_persistent_disk_bitmap_cache;
        }

        if (use_bitmapcache_rev2) {
            if (this->verbose & 1) {
                bmpcache2_caps.log("Sending to server");
            }
            confirm_active_pdu.emit_capability_set(bmpcache2_caps);

            if (!this->enable_transparent_mode && !this->deactivation_reactivation_in_progress) {
                this->orders.create_cache_bitmap(this->bpp,
                    120,   nbbytes(this->bpp) * 16 * 16, false,
                    120,   nbbytes(this->bpp) * 32 * 32, false,
                    2553,  nbbytes(this->bpp) * 64 * 64, this->enable_persistent_disk_bitmap_cache,
                    this->cache_verbose);
            }
        }
        else {
            if (this->verbose & 1) {
                bmpcache_caps.log("Sending to server");
            }
            confirm_active_pdu.emit_capability_set(bmpcache_caps);

            if (!this->enable_transparent_mode && !this->deactivation_reactivation_in_progress) {
                this->orders.create_cache_bitmap(this->bpp,
                    0x258, nbbytes(this->bpp) * 0x100,   false,
                    0x12c, nbbytes(this->bpp) * 0x400,   false,
                    0x106, nbbytes(this->bpp) * 0x1000,  false,
                    this->cache_verbose);
            }
        }

        ColorCacheCaps colorcache_caps;
        if (this->verbose & 1) {
            colorcache_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(colorcache_caps);

        ActivationCaps activation_caps;
        if (this->verbose & 1) {
            activation_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(activation_caps);

        ControlCaps control_caps;
        if (this->verbose & 1) {
            control_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(control_caps);

        PointerCaps pointer_caps;
        pointer_caps.len                       = 10;
        if (this->enable_new_pointer == false) {
            pointer_caps.pointerCacheSize      = 0;
            pointer_caps.colorPointerCacheSize = 20;
            pointer_caps.len                   = 8;
            REDASSERT(pointer_caps.colorPointerCacheSize <= sizeof(this->cursors) / sizeof(Pointer));
        }
        if (this->verbose & 1) {
            pointer_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(pointer_caps);

        ShareCaps share_caps;
        if (this->verbose & 1) {
            share_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(share_caps);

        InputCaps input_caps;
        if (this->verbose & 1) {
            input_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(input_caps);

        SoundCaps sound_caps;
        if (this->verbose & 1) {
            sound_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(sound_caps);

        FontCaps font_caps;
        if (this->verbose & 1) {
            font_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(font_caps);

        GlyphCacheCaps glyphcache_caps;
        if (this->enable_glyph_cache) {
            this->front.retrieve_client_capability_set(glyphcache_caps);

            glyphcache_caps.FragCache         = 0;  // Not yet supported
            glyphcache_caps.GlyphSupportLevel &= GlyphCacheCaps::GLYPH_SUPPORT_PARTIAL;
        }
        if (this->verbose & 1) {
            glyphcache_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(glyphcache_caps);

        if (this->remote_program) {
            RailCaps rail_caps;
            rail_caps.RailSupportLevel = TS_RAIL_LEVEL_SUPPORTED;
            if (this->verbose & 1) {
                rail_caps.log("Sending to server");
            }
            confirm_active_pdu.emit_capability_set(rail_caps);

            WindowListCaps window_list_caps;
            window_list_caps.WndSupportLevel = TS_WINDOW_LEVEL_SUPPORTED;
            window_list_caps.NumIconCaches = 3;
            window_list_caps.NumIconCacheEntries = 12;
            if (this->verbose & 1) {
                window_list_caps.log("Sending to server");
            }
            confirm_active_pdu.emit_capability_set(window_list_caps);
        }
        confirm_active_pdu.emit_end();

        // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
        // containing information about the packet. The type subfield of the pduType
        // field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).
        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_CONFIRMACTIVEPDU,
            this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_confirm_active done");
            LOG(LOG_INFO, "Waiting for answer to confirm active");
        }
    }   // send_confirm_active

    void process_pointer_pdu(Stream & stream, mod_api * mod) throw(Error)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_pointer_pdu");
        }

        int message_type = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */
        switch (message_type) {
        case RDP_POINTER_CACHED:
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer cached");
            }
            this->process_cached_pointer_pdu(stream);
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer cached done");
            }
            break;
        case RDP_POINTER_COLOR:
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer color");
            }
            this->process_system_pointer_pdu(stream);
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer system done");
            }
            break;
        case RDP_POINTER_NEW:
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer new");
            }
            if (enable_new_pointer) {
                this->process_new_pointer_pdu(stream); // Pointer with arbitrary color depth
            }
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer new done");
            }
            break;
        case RDP_POINTER_SYSTEM:
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer system");
            }
        case RDP_POINTER_MOVE:
            {
                if (this->verbose & 4) {
                    LOG(LOG_INFO, "Process pointer move");
                }
                uint16_t xPos = stream.in_uint16_le();
                uint16_t yPos = stream.in_uint16_le();
                this->front.update_pointer_position(xPos, yPos);
            }
            break;
        default:
            break;
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_pointer_pdu done");
        }
    }

    void process_palette(Stream & stream, bool fast_path) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_palette");
        }

        RDP::UpdatePaletteData_Recv(stream, fast_path, this->orders.global_palette);
        this->front.set_mod_palette(this->orders.global_palette);

        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_palette done");
        }
    }

    // 2.2.5.1.1 Set Error Info PDU Data (TS_SET_ERROR_INFO_PDU)
    // =========================================================
    // The TS_SET_ERROR_INFO_PDU structure contains the contents of the Set Error
    // Info PDU, which is a Share Data Header (section 2.2.8.1.1.1.2) with an error
    // value field.

    // shareDataHeader (18 bytes): Share Data Header containing information about
    // the packet. The type subfield of the pduType field of the Share Control
    // Header (section 2.2.8.1.1.1.1) MUST be set to PDUTYPE_DATAPDU (7). The
    // pduType2 field of the Share Data Header MUST be set to
    // PDUTYPE2_SET_ERROR_INFO_PDU (47), and the pduSource field MUST be set to 0.

    // errorInfo (4 bytes): A 32-bit, unsigned integer. Error code.

    // Protocol-independent codes:
    // +---------------------------------------------+-----------------------------+
    // | 0x00000001 ERRINFO_RPC_INITIATED_DISCONNECT | The disconnection was       |
    // |                                             | initiated by an             |
    // |                                             | administrative tool on the  |
    // |                                             | server in another session.  |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000002 ERRINFO_RPC_INITIATED_LOGOFF     | The disconnection was due   |
    // |                                             | to a forced logoff initiated|
    // |                                             | by an administrative tool   |
    // |                                             | on the server in another    |
    // |                                             | session.                    |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000003 ERRINFO_IDLE_TIMEOUT             | The idle session limit timer|
    // |                                             | on the server has elapsed.  |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000004 ERRINFO_LOGON_TIMEOUT            | The active session limit    |
    // |                                             | timer on the server has     |
    // |                                             | elapsed.                    |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000005                                  | Another user connected to   |
    // | ERRINFO_DISCONNECTED_BY_OTHERCONNECTION     | the server, forcing the     |
    // |                                             | disconnection of the current|
    // |                                             | connection.                 |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000006 ERRINFO_OUT_OF_MEMORY            | The server ran out of       |
    // |                                             | available memory resources. |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000007 ERRINFO_SERVER_DENIED_CONNECTION | The server denied the       |
    // |                                             | connection.                 |
    // +---------------------------------------------+-----+-----------------------+
    // | 0x00000009                                  | The user cannot connect to  |
    // | ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES      | the server due to           |
    // |                                             | insufficient access         |
    // |                                             | privileges.                 |
    // +---------------------------------------------+-----------------------------+
    // | 0x0000000A                                  | The server does not accept  |
    // | ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED   | saved user credentials and  |
    // |                                             | requires that the user enter|
    // |                                             | their credentials for each  |
    // |                                             | connection.                 |
    // +-----------------------------------------+---+-----------------------------+
    // | 0x0000000B                              | The disconnection was initiated |
    // | ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER | by an administrative tool on    |
    // |                                         | the server running in the user's|
    // |                                         | session.                        |
    // +-----------------------------------------+---------------------------------+
    // | 0x0000000C ERRINFO_LOGOFF_BY_USER       | The disconnection was initiated |
    // |                                         | by the user logging off his or  |
    // |                                         | her session on the server.      |
    // +-----------------------------------------+---------------------------------+

    // Protocol-independent licensing codes:
    // +-------------------------------------------+-------------------------------+
    // | 0x00000100 ERRINFO_LICENSE_INTERNAL       | An internal error has occurred|
    // |                                           | in the Terminal Services      |
    // |                                           | licensing component.          |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000101                                | A Remote Desktop License      |
    // | ERRINFO_LICENSE_NO_LICENSE_SERVER         | Server ([MS-RDPELE] section   |
    // |                                           | 1.1) could not be found to    |
    // |                                           | provide a license.            |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000102 ERRINFO_LICENSE_NO_LICENSE     | There are no Client Access    |
    // |                                           | Licenses ([MS-RDPELE] section |
    // |                                           | 1.1) available for the target |
    // |                                           | remote computer.              |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000103 ERRINFO_LICENSE_BAD_CLIENT_MSG | The remote computer received  |
    // |                                           | an invalid licensing message  |
    // |                                           | from the client.              |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000104                                | The Client Access License     |
    // | ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE | ([MS-RDPELE] section 1.1)     |
    // |                                           | stored by the client has been |
    // |                                           |  modified.                    |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000105                                | The Client Access License     |
    // | ERRINFO_LICENSE_BAD_CLIENT_LICENSE        | ([MS-RDPELE] section 1.1)     |
    // |                                           | stored by the client is in an |
    // |                                           | invalid format.               |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000106                                | Network problems have caused  |
    // | ERRINFO_LICENSE_CANT_FINISH_PROTOCOL      | the licensing protocol        |
    // |                                           | ([MS-RDPELE] section 1.3.3)   |
    // |                                           | to be terminated.             |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000107                                | The client prematurely ended  |
    // | ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL     | the licensing protocol        |
    // |                                           | ([MS-RDPELE] section 1.3.3).  |
    // +---------------------------------------+---+-------------------------------+
    // | 0x00000108                            | A licensing message ([MS-RDPELE]  |
    // | ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION | sections 2.2 and 5.1) was         |
    // |                                       | incorrectly encrypted.            |
    // +---------------------------------------+-----------------------------------+
    // | 0x00000109                            | The Client Access License         |
    // | ERRINFO_LICENSE_CANT_UPGRADE_LICENSE  | ([MS-RDPELE] section 1.1) stored  |
    // |                                       | by the client could not be        |
    // |                                       | upgraded or renewed.              |
    // +---------------------------------------+-----------------------------------+
    // | 0x0000010A                            | The remote computer is not        |
    // | ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS | licensed to accept remote         |
    // |                                       |  connections.                     |
    // +---------------------------------------+-----------------------------------+

    // Protocol-independent codes generated by Connection Broker:
    // +----------------------------------------------+----------------------------+
    // | Value                                        | Meaning                    |
    // +----------------------------------------------+----------------------------+
    // | 0x0000400                                    | The target endpoint could  |
    // | ERRINFO_CB_DESTINATION_NOT_FOUND             | not be found.              |
    // +----------------------------------------------+----------------------------+
    // | 0x0000402                                    | The target endpoint to     |
    // | ERRINFO_CB_LOADING_DESTINATION               | which the client is being  |
    // |                                              | redirected is              |
    // |                                              | disconnecting from the     |
    // |                                              | Connection Broker.         |
    // +----------------------------------------------+----------------------------+
    // | 0x0000404                                    | An error occurred while    |
    // | ERRINFO_CB_REDIRECTING_TO_DESTINATION        | the connection was being   |
    // |                                              | redirected to the target   |
    // |                                              | endpoint.                  |
    // +----------------------------------------------+----------------------------+
    // | 0x0000405                                    | An error occurred while    |
    // | ERRINFO_CB_SESSION_ONLINE_VM_WAKE            | the target endpoint (a     |
    // |                                              | virtual machine) was being |
    // |                                              | awakened.                  |
    // +----------------------------------------------+----------------------------+
    // | 0x0000406                                    | An error occurred while    |
    // | ERRINFO_CB_SESSION_ONLINE_VM_BOOT            | the target endpoint (a     |
    // |                                              | virtual machine) was being |
    // |                                              | started.                   |
    // +----------------------------------------------+----------------------------+
    // | 0x0000407                                    | The IP address of the      |
    // | ERRINFO_CB_SESSION_ONLINE_VM_NO_DNS          | target endpoint (a virtual |
    // |                                              | machine) cannot be         |
    // |                                              | determined.                |
    // +----------------------------------------------+----------------------------+
    // | 0x0000408                                    | There are no available     |
    // | ERRINFO_CB_DESTINATION_POOL_NOT_FREE         | endpoints in the pool      |
    // |                                              | managed by the Connection  |
    // |                                              | Broker.                    |
    // +----------------------------------------------+----------------------------+
    // | 0x0000409                                    | Processing of the          |
    // | ERRINFO_CB_CONNECTION_CANCELLED              | connection has been        |
    // |                                              | cancelled.                 |
    // +----------------------------------------------+----------------------------+
    // | 0x0000410                                    | The settings contained in  |
    // | ERRINFO_CB_CONNECTION_ERROR_INVALID_SETTINGS | the routingToken field of  |
    // |                                              | the X.224 Connection       |
    // |                                              | Request PDU (section       |
    // |                                              | 2.2.1.1) cannot be         |
    // |                                              | validated.                 |
    // +----------------------------------------------+----------------------------+
    // | 0x0000411                                    | A time-out occurred while  |
    // | ERRINFO_CB_SESSION_ONLINE_VM_BOOT_TIMEOUT    | the target endpoint (a     |
    // |                                              | virtual machine) was being |
    // |                                              | started.                   |
    // +----------------------------------------------+----------------------------+
    // | 0x0000412                                    | A session monitoring error |
    // | ERRINFO_CB_SESSION_ONLINE_VM_SESSMON_FAILED  | occurred while the target  |
    // |                                              | endpoint (a virtual        |
    // |                                              | machine) was being         |
    // |                                              | started.                   |
    // +----------------------------------------------+----------------------------+

    // RDP specific codes:
    // +------------------------------------+--------------------------------------+
    // | 0x000010C9 ERRINFO_UNKNOWNPDUTYPE2 | Unknown pduType2 field in a received |
    // |                                    | Share Data Header (section           |
    // |                                    | 2.2.8.1.1.1.2).                      |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CA ERRINFO_UNKNOWNPDUTYPE  | Unknown pduType field in a received  |
    // |                                    | Share Control Header (section        |
    // |                                    | 2.2.8.1.1.1.1).                      |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CB ERRINFO_DATAPDUSEQUENCE | An out-of-sequence Slow-Path Data PDU|
    // |                                    | (section 2.2.8.1.1.1.1) has been     |
    // |                                    | received.                            |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CD                         | An out-of-sequence Slow-Path Non-Data|
    // | ERRINFO_CONTROLPDUSEQUENCE         | PDU (section 2.2.8.1.1.1.1) has been |
    // |                                    | received.                            |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CE                         | A Control PDU (sections 2.2.1.15 and |
    // | ERRINFO_INVALIDCONTROLPDUACTION    | 2.2.1.16) has been received with an  |
    // |                                    | invalid action field.                |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CF                         | (a) A Slow-Path Input Event (section |
    // | ERRINFO_INVALIDINPUTPDUTYPE        | 2.2.8.1.1.3.1.1) has been received   |
    // |                                    | with an invalid messageType field.   |
    // |                                    | (b) A Fast-Path Input Event (section |
    // |                                    | 2.2.8.1.2.2) has been received with  |
    // |                                    | an invalid eventCode field.          |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D0                         | (a) A Slow-Path Mouse Event (section |
    // | ERRINFO_INVALIDINPUTPDUMOUSE       | 2.2.8.1.1.3.1.1.3) or Extended Mouse |
    // |                                    | Event (section 2.2.8.1.1.3.1.1.4)    |
    // |                                    | has been received with an invalid    |
    // |                                    | pointerFlags field.                  |
    // |                                    | (b) A Fast-Path Mouse Event (section |
    // |                                    | 2.2.8.1.2.2.3) or Fast-Path Extended |
    // |                                    | Mouse Event (section 2.2.8.1.2.2.4)  |
    // |                                    | has been received with an invalid    |
    // |                                    | pointerFlags field.                  |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D1                         | An invalid Refresh Rect PDU (section |
    // | ERRINFO_INVALIDREFRESHRECTPDU      | 2.2.11.2) has been received.         |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D2                         | The server failed to construct the   |
    // | ERRINFO_CREATEUSERDATAFAILED       | GCC Conference Create Response user  |
    // |                                    | data (section 2.2.1.4).              |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D3 ERRINFO_CONNECTFAILED   | Processing during the Channel        |
    // |                                    | Connection phase of the RDP          |
    // |                                    | Connection Sequence (see section     |
    // |                                    | 1.3.1.1 for an overview of the RDP   |
    // |                                    | Connection Sequence phases) has      |
    // |                                    | failed.                              |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D4                         | A Confirm Active PDU (section        |
    // | ERRINFO_CONFIRMACTIVEWRONGSHAREID  | 2.2.1.13.2) was received from the    |
    // |                                    | client with an invalid shareId field.|
    // +------------------------------------+-+------------------------------------+
    // | 0x000010D5                           | A Confirm Active PDU (section      |
    // | ERRINFO_CONFIRMACTIVEWRONGORIGINATOR | 2.2.1.13.2) was received from the  |
    // |                                      | client with an invalid originatorId|
    // |                                      | field.                             |
    // +--------------------------------------+------------------------------------+
    // | 0x000010DA                           | There is not enough data to process|
    // | ERRINFO_PERSISTENTKEYPDUBADLENGTH    | a Persistent Key List PDU (section |
    // |                                      | 2.2.1.17).                         |
    // +--------------------------------------+------------------------------------+
    // | 0x000010DB                           | A Persistent Key List PDU (section |
    // | ERRINFO_PERSISTENTKEYPDUILLEGALFIRST | 2.2.1.17) marked as                |
    // |                                      | PERSIST_PDU_FIRST (0x01) was       |
    // |                                      | received after the reception of a  |
    // |                                      | prior Persistent Key List PDU also |
    // |                                      | marked as PERSIST_PDU_FIRST.       |
    // +--------------------------------------+---+--------------------------------+
    // | 0x000010DC                               | A Persistent Key List PDU      |
    // | ERRINFO_PERSISTENTKEYPDUTOOMANYTOTALKEYS | (section 2.2.1.17) was received|
    // |                                          | which specified a total number |
    // |                                          | of bitmap cache entries larger |
    // |                                          | than 262144.                   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010DD                               | A Persistent Key List PDU      |
    // | ERRINFO_PERSISTENTKEYPDUTOOMANYCACHEKEYS | (section 2.2.1.17) was received|
    // |                                          | which specified an invalid     |
    // |                                          | total number of keys for a     |
    // |                                          | bitmap cache (the number of    |
    // |                                          | entries that can be stored     |
    // |                                          | within each bitmap cache is    |
    // |                                          | specified in the Revision 1 or |
    // |                                          | 2 Bitmap Cache Capability Set  |
    // |                                          | (section 2.2.7.1.4) that is    |
    // |                                          | sent from client to server).   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010DE ERRINFO_INPUTPDUBADLENGTH     | There is not enough data to    |
    // |                                          | process Input Event PDU Data   |
    // |                                          | (section 2.2.8.1.1.3.          |
    // |                                          | 2.2.8.1.2).                    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010DF                               | There is not enough data to    |
    // | ERRINFO_BITMAPCACHEERRORPDUBADLENGTH     | process the shareDataHeader,   |
    // |                                          | NumInfoBlocks, Pad1, and Pad2  |
    // |                                          | fields of the Bitmap Cache     |
    // |                                          | Error PDU Data ([MS-RDPEGDI]   |
    // |                                          | section 2.2.2.3.1.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E0  ERRINFO_SECURITYDATATOOSHORT | (a) The dataSignature field of |
    // |                                          | the Fast-Path Input Event PDU  |
    // |                                          | (section 2.2.8.1.2) does not   |
    // |                                          | contain enough data.           |
    // |                                          | (b) The fipsInformation and    |
    // |                                          | dataSignature fields of the    |
    // |                                          | Fast-Path Input Event PDU      |
    // |                                          | (section 2.2.8.1.2) do not     |
    // |                                          | contain enough data.           |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E1 ERRINFO_VCHANNELDATATOOSHORT  | (a) There is not enough data   |
    // |                                          | in the Client Network Data     |
    // |                                          | (section 2.2.1.3.4) to read the|
    // |                                          | virtual channel configuration  |
    // |                                          | data.                          |
    // |                                          | (b) There is not enough data   |
    // |                                          | to read a complete Channel     |
    // |                                          | PDU Header (section 2.2.6.1.1).|
    // +------------------------------------------+--------------------------------+
    // | 0x000010E2 ERRINFO_SHAREDATATOOSHORT     | (a) There is not enough data   |
    // |                                          | to process Control PDU Data    |
    // |                                          | (section 2.2.1.15.1).          |
    // |                                          | (b) There is not enough data   |
    // |                                          | to read a complete Share       |
    // |                                          | Control Header (section        |
    // |                                          | 2.2.8.1.1.1.1).                |
    // |                                          | (c) There is not enough data   |
    // |                                          | to read a complete Share Data  |
    // |                                          | Header (section 2.2.8.1.1.1.2) |
    // |                                          | of a Slow-Path Data PDU        |
    // |                                          | (section 2.2.8.1.1.1.1).       |
    // |                                          | (d) There is not enough data   |
    // |                                          | to process Font List PDU Data  |
    // |                                          | (section 2.2.1.18.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E3 ERRINFO_BADSUPRESSOUTPUTPDU   | (a) There is not enough data   |
    // |                                          | to process Suppress Output PDU |
    // |                                          | Data (section 2.2.11.3.1).     |
    // |                                          | (b) The allowDisplayUpdates    |
    // |                                          | field of the Suppress Output   |
    // |                                          | PDU Data (section 2.2.11.3.1)  |
    // |                                          | is invalid.                    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E5                               | (a) There is not enough data   |
    // | ERRINFO_CONFIRMACTIVEPDUTOOSHORT         | to read the shareControlHeader,|
    // |                                          | shareId, originatorId,         |
    // |                                          | lengthSourceDescriptor, and    |
    // |                                          | lengthCombinedCapabilities     |
    // |                                          | fields of the Confirm Active   |
    // |                                          | PDU Data (section              |
    // |                                          | 2.2.1.13.2.1).                 |
    // |                                          | (b) There is not enough data   |
    // |                                          | to read the sourceDescriptor,  |
    // |                                          | numberCapabilities, pad2Octets,|
    // |                                          | and capabilitySets fields of   |
    // |                                          | the Confirm Active PDU Data    |
    // |                                          | (section 2.2.1.13.2.1).        |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E7 ERRINFO_CAPABILITYSETTOOSMALL | There is not enough data to    |
    // |                                          | read the capabilitySetType and |
    // |                                          | the lengthCapability fields in |
    // |                                          | a received Capability Set      |
    // |                                          | (section 2.2.1.13.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E8 ERRINFO_CAPABILITYSETTOOLARGE | A Capability Set (section      |
    // |                                          | 2.2.1.13.1.1.1) has been       |
    // |                                          | received with a                |
    // |                                          | lengthCapability field that    |
    // |                                          | contains a value greater than  |
    // |                                          | the total length of the data   |
    // |                                          | received.                      |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E9 ERRINFO_NOCURSORCACHE         | (a) Both the                   |
    // |                                          | colorPointerCacheSize and      |
    // |                                          | pointerCacheSize fields in the |
    // |                                          | Pointer Capability Set         |
    // |                                          | (section 2.2.7.1.5) are set to |
    // |                                          | zero.                          |
    // |                                          | (b) The pointerCacheSize field |
    // |                                          | in the Pointer Capability Set  |
    // |                                          | (section 2.2.7.1.5) is not     |
    // |                                          | present, and the               |
    // |                                          | colorPointerCacheSize field is |
    // |                                          | set to zero.                   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010EA ERRINFO_BADCAPABILITIES       | The capabilities received from |
    // |                                          | the client in the Confirm      |
    // |                                          | Active PDU (section 2.2.1.13.2)|
    // |                                          | were not accepted by the       |
    // |                                          | server.                        |
    // +------------------------------------------+--------------------------------+
    // | 0x000010EC                               | An error occurred while using  |
    // | ERRINFO_VIRTUALCHANNELDECOMPRESSIONERR   | the bulk compressor (section   |
    // |                                          | 3.1.8 and [MS- RDPEGDI] section|
    // |                                          | 3.1.8) to decompress a Virtual |
    // |                                          | Channel PDU (section 2.2.6.1). |
    // +------------------------------------------+--------------------------------+
    // | 0x000010ED                               | An invalid bulk compression    |
    // | ERRINFO_INVALIDVCCOMPRESSIONTYPE         | package was specified in the   |
    // |                                          | flags field of the Channel PDU |
    // |                                          | Header (section 2.2.6.1.1).    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010EF ERRINFO_INVALIDCHANNELID      | An invalid MCS channel ID was  |
    // |                                          | specified in the mcsPdu field  |
    // |                                          | of the Virtual Channel PDU     |
    // |                                          | (section 2.2.6.1).             |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F0 ERRINFO_VCHANNELSTOOMANY      | The client requested more than |
    // |                                          | the maximum allowed 31 static  |
    // |                                          | virtual channels in the Client |
    // |                                          | Network Data (section          |
    // |                                          | 2.2.1.3.4).                    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F3 ERRINFO_REMOTEAPPSNOTENABLED  | The INFO_RAIL flag (0x00008000)|
    // |                                          | MUST be set in the flags field |
    // |                                          | of the Info Packet (section    |
    // |                                          | 2.2.1.11.1.1) as the session   |
    // |                                          | on the remote server can only  |
    // |                                          | host remote applications.      |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F4 ERRINFO_CACHECAPNOTSET        | The client sent a Persistent   |
    // |                                          | Key List PDU (section 2.2.1.17)|
    // |                                          | without including the          |
    // |                                          | prerequisite Revision 2 Bitmap |
    // |                                          | Cache Capability Set (section  |
    // |                                          | 2.2.7.1.4.2) in the Confirm    |
    // |                                          | Active PDU (section            |
    // |                                          | 2.2.1.13.2).                   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F5                               | The NumInfoBlocks field in the |
    // |ERRINFO_BITMAPCACHEERRORPDUBADLENGTH2     | Bitmap Cache Error PDU Data is |
    // |                                          | inconsistent with the amount   |
    // |                                          | of data in the Info field      |
    // |                                          | ([MS-RDPEGDI] section          |
    // |                                          | 2.2.2.3.1.1).                  |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F6                               | There is not enough data to    |
    // | ERRINFO_OFFSCRCACHEERRORPDUBADLENGTH     | process an Offscreen Bitmap    |
    // |                                          | Cache Error PDU ([MS-RDPEGDI]  |
    // |                                          | section 2.2.2.3.2).            |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F7                               | There is not enough data to    |
    // | ERRINFO_DNGCACHEERRORPDUBADLENGTH        | process a DrawNineGrid Cache   |
    // |                                          | Error PDU ([MS-RDPEGDI]        |
    // |                                          | section 2.2.2.3.3).            |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F8 ERRINFO_GDIPLUSPDUBADLENGTH   | There is not enough data to    |
    // |                                          | process a GDI+ Error PDU       |
    // |                                          | ([MS-RDPEGDI] section          |
    // |                                          | 2.2.2.3.4).                    |
    // +------------------------------------------+--------------------------------+
    // | 0x00001111 ERRINFO_SECURITYDATATOOSHORT2 | There is not enough data to    |
    // |                                          | read a Basic Security Header   |
    // |                                          | (section 2.2.8.1.1.2.1).       |
    // +------------------------------------------+--------------------------------+
    // | 0x00001112 ERRINFO_SECURITYDATATOOSHORT3 | There is not enough data to    |
    // |                                          | read a Non- FIPS Security      |
    // |                                          | Header (section 2.2.8.1.1.2.2) |
    // |                                          | or FIPS Security Header        |
    // |                                          | (section 2.2.8.1.1.2.3).       |
    // +------------------------------------------+--------------------------------+
    // | 0x00001113 ERRINFO_SECURITYDATATOOSHORT4 | There is not enough data to    |
    // |                                          | read the basicSecurityHeader   |
    // |                                          | and length fields of the       |
    // |                                          | Security Exchange PDU Data     |
    // |                                          | (section 2.2.1.10.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x00001114 ERRINFO_SECURITYDATATOOSHORT5 | There is not enough data to    |
    // |                                          | read the CodePage, flags,      |
    // |                                          | cbDomain, cbUserName,          |
    // |                                          | cbPassword, cbAlternateShell,  |
    // |                                          | cbWorkingDir, Domain, UserName,|
    // |                                          | Password, AlternateShell, and  |
    // |                                          | WorkingDir fields in the Info  |
    // |                                          | Packet (section 2.2.1.11.1.1). |
    // +------------------------------------------+--------------------------------+
    // | 0x00001115 ERRINFO_SECURITYDATATOOSHORT6 | There is not enough data to    |
    // |                                          | read the CodePage, flags,      |
    // |                                          | cbDomain, cbUserName,          |
    // |                                          | cbPassword, cbAlternateShell,  |
    // |                                          | and cbWorkingDir fields in the |
    // |                                          | Info Packet (section           |
    // |                                          | 2.2.1.11.1.1).                 |
    // +------------------------------------------+--------------------------------+
    // | 0x00001116 ERRINFO_SECURITYDATATOOSHORT7 | There is not enough data to    |
    // |                                          | read the clientAddressFamily   |
    // |                                          | and cbClientAddress fields in  |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001117 ERRINFO_SECURITYDATATOOSHORT8 | There is not enough data to    |
    // |                                          | read the clientAddress field in|
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001118 ERRINFO_SECURITYDATATOOSHORT9 | There is not enough data to    |
    // |                                          | read the cbClientDir field in  |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001119 ERRINFO_SECURITYDATATOOSHORT10| There is not enough data to    |
    // |                                          | read the clientDir field in the|
    // |                                          | Extended Info Packet (section  |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111A ERRINFO_SECURITYDATATOOSHORT11| There is not enough data to    |
    // |                                          | read the clientTimeZone field  |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111B ERRINFO_SECURITYDATATOOSHORT12| There is not enough data to    |
    // |                                          | read the clientSessionId field |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111C ERRINFO_SECURITYDATATOOSHORT13| There is not enough data to    |
    // |                                          | read the performanceFlags      |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111D ERRINFO_SECURITYDATATOOSHORT14| There is not enough data to    |
    // |                                          | read the cbAutoReconnectLen    |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111E ERRINFO_SECURITYDATATOOSHORT15| There is not enough data to    |
    // |                                          | read the autoReconnectCookie   |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111F ERRINFO_SECURITYDATATOOSHORT16| The cbAutoReconnectLen field   |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1)       |
    // |                                          | contains a value which is      |
    // |                                          | larger than the maximum        |
    // |                                          | allowed length of 128 bytes.   |
    // +------------------------------------------+--------------------------------+
    // | 0x00001120 ERRINFO_SECURITYDATATOOSHORT17| There is not enough data to    |
    // |                                          | read the clientAddressFamily   |
    // |                                          | and cbClientAddress fields in  |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001121 ERRINFO_SECURITYDATATOOSHORT18| There is not enough data to    |
    // |                                          | read the clientAddress field in|
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001122 ERRINFO_SECURITYDATATOOSHORT19| There is not enough data to    |
    // |                                          | read the cbClientDir field in  |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001123 ERRINFO_SECURITYDATATOOSHORT20| There is not enough data to    |
    // |                                          | read the clientDir field in    |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001124 ERRINFO_SECURITYDATATOOSHORT21| There is not enough data to    |
    // |                                          | read the clientTimeZone field  |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001125 ERRINFO_SECURITYDATATOOSHORT22| There is not enough data to    |
    // |                                          | read the clientSessionId field |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001126 ERRINFO_SECURITYDATATOOSHORT23| There is not enough data to    |
    // |                                          | read the Client Info PDU Data  |
    // |                                          | (section 2.2.1.11.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x00001129 ERRINFO_BADMONITORDATA        | The monitorCount field in the  |
    // |                                          | Client Monitor Data (section   |
    // |                                          | 2.2.1.3.6) is invalid.         |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112A                               | The server-side decompression  |
    // | ERRINFO_VCDECOMPRESSEDREASSEMBLEFAILED   | buffer is invalid, or the size |
    // |                                          | of the decompressed VC data    |
    // |                                          | exceeds the chunking size      |
    // |                                          | specified in the Virtual       |
    // |                                          | Channel Capability Set         |
    // |                                          | (section 2.2.7.1.10).          |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112B ERRINFO_VCDATATOOLONG         | The size of a received Virtual |
    // |                                          | Channel PDU (section 2.2.6.1)  |
    // |                                          | exceeds the chunking size      |
    // |                                          | specified in the Virtual       |
    // |                                          | Channel Capability Set         |
    // |                                          | (section 2.2.7.1.10).          |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112C ERRINFO_BAD_FRAME_ACK_DATA    | There is not enough data to    |
    // |                                          | read a                         |
    // |                                          | TS_FRAME_ACKNOWLEDGE_PDU ([MS- |
    // |                                          | RDPRFX] section 2.2.3.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112D                               | The graphics mode requested by |
    // | ERRINFO_GRAPHICSMODENOTSUPPORTED         | the client is not supported by |
    // |                                          | the server.                    |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112E                               | The server-side graphics       |
    // | ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED     | subsystem failed to reset.     |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112F                               | The server-side graphics       |
    // | ERRINFO_GRAPHICSSUBSYSTEMFAILED          | subsystem is in an error state |
    // |                                          | and unable to continue         |
    // |                                          | graphics encoding.             |
    // +------------------------------------------+--------------------------------+
    // | 0x00001130                               | There is not enough data to    |
    // | ERRINFO_TIMEZONEKEYNAMELENGTHTOOSHORT    | read the                       |
    // |                                          | cbDynamicDSTTimeZoneKeyName    |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x00001131                               | The length reported in the     |
    // | ERRINFO_TIMEZONEKEYNAMELENGTHTOOLONG     | cbDynamicDSTTimeZoneKeyName    |
    // |                                          | field of the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1) is too long.   |
    // +------------------------------------------+--------------------------------+
    // | 0x00001132                               | The                            |
    // | ERRINFO_DYNAMICDSTDISABLEDFIELDMISSING   | dynamicDaylightTimeDisabled    |
    // |                                          | field is not present in the    |
    // |                                          | Extended Info Packet (section  |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x00001191                               | An attempt to update the       |
    // | ERRINFO_UPDATESESSIONKEYFAILED           | session keys while using       |
    // |                                          | Standard RDP Security          |
    // |                                          | mechanisms (section 5.3.7)     |
    // |                                          | failed.                        |
    // +------------------------------------------+--------------------------------+
    // | 0x00001192 ERRINFO_DECRYPTFAILED         | (a) Decryption using Standard  |
    // |                                          | RDP Security mechanisms        |
    // |                                          | (section 5.3.6) failed.        |
    // |                                          | (b) Session key creation using |
    // |                                          | Standard RDP Security          |
    // |                                          | mechanisms (section 5.3.5)     |
    // |                                          | failed.                        |
    // +------------------------------------------+--------------------------------+
    // | 0x00001193 ERRINFO_ENCRYPTFAILED         | Encryption using Standard RDP  |
    // |                                          | Security mechanisms (section   |
    // |                                          | 5.3.6) failed.                 |
    // +------------------------------------------+--------------------------------+
    // | 0x00001194 ERRINFO_ENCPKGMISMATCH        | Failed to find a usable        |
    // |                                          | Encryption Method (section     |
    // |                                          | 5.3.2) in the encryptionMethods|
    // |                                          | field of the Client Security   |
    // |                                          | Data (section 2.2.1.4.3).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001195 ERRINFO_DECRYPTFAILED2        | Encryption using Standard RDP  |
    // |                                          | Security mechanisms (section   |
    // |                                          | 5.3.6) failed. Unencrypted     |
    // |                                          | data was encountered in a      |
    // |                                          | protocol stream which is meant |
    // |                                          | to be encrypted with Standard  |
    // |                                          | RDP Security mechanisms        |
    // |                                          | (section 5.3.6).               |
    // +------------------------------------------+--------------------------------+

    enum {
        ERRINFO_RPC_INITIATED_DISCONNECT          = 0x00000001,
        ERRINFO_RPC_INITIATED_LOGOFF              = 0x00000002,
        ERRINFO_IDLE_TIMEOUT                      = 0x00000003,
        ERRINFO_LOGON_TIMEOUT                     = 0x00000004,
        ERRINFO_DISCONNECTED_BY_OTHERCONNECTION   = 0x00000005,
        ERRINFO_OUT_OF_MEMORY                     = 0x00000006,
        ERRINFO_SERVER_DENIED_CONNECTION          = 0x00000007,
        ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES    = 0x00000009,
        ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED = 0x0000000A,
        ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER   = 0x0000000B,
        ERRINFO_LOGOFF_BY_USER                    = 0x0000000C,
        ERRINFO_LICENSE_INTERNAL                  = 0x00000100,
        ERRINFO_LICENSE_NO_LICENSE_SERVER         = 0x00000101,
        ERRINFO_LICENSE_NO_LICENSE                = 0x00000102,
        ERRINFO_LICENSE_BAD_CLIENT_MSG            = 0x00000103,
        ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE = 0x00000104,
        ERRINFO_LICENSE_BAD_CLIENT_LICENSE        = 0x00000105,
        ERRINFO_LICENSE_CANT_FINISH_PROTOCOL      = 0x00000106,
        ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL     = 0x00000107,
        ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION     = 0x00000108,
        ERRINFO_LICENSE_CANT_UPGRADE_LICENSE      = 0x00000109,
        ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS     = 0x0000010A,

        ERRINFO_CB_DESTINATION_NOT_FOUND             = 0x00000400,
        ERRINFO_CB_LOADING_DESTINATION               = 0x00000402,
        ERRINFO_CB_REDIRECTING_TO_DESTINATION        = 0x00000404,
        ERRINFO_CB_SESSION_ONLINE_VM_WAKE            = 0x00000405,
        ERRINFO_CB_SESSION_ONLINE_VM_BOOT            = 0x00000406,
        ERRINFO_CB_SESSION_ONLINE_VM_NO_DNS          = 0x00000407,
        ERRINFO_CB_DESTINATION_POOL_NOT_FREE         = 0x00000408,
        ERRINFO_CB_CONNECTION_CANCELLED              = 0x00000409,
        ERRINFO_CB_CONNECTION_ERROR_INVALID_SETTINGS = 0x00000410,
        ERRINFO_CB_SESSION_ONLINE_VM_BOOT_TIMEOUT    = 0x00000411,
        ERRINFO_CB_SESSION_ONLINE_VM_SESSMON_FAILED  = 0x00000412,

        ERRINFO_UNKNOWNPDUTYPE2                   = 0x000010C9,
        ERRINFO_UNKNOWNPDUTYPE                    = 0x000010CA,
        ERRINFO_DATAPDUSEQUENCE                   = 0x000010CB,
        ERRINFO_CONTROLPDUSEQUENCE                = 0x000010CD,
        ERRINFO_INVALIDCONTROLPDUACTION           = 0x000010CE,
        ERRINFO_INVALIDINPUTPDUTYPE               = 0x000010CF,
        ERRINFO_INVALIDINPUTPDUMOUSE              = 0x000010D0,
        ERRINFO_INVALIDREFRESHRECTPDU             = 0x000010D1,
        ERRINFO_CREATEUSERDATAFAILED              = 0x000010D2,
        ERRINFO_CONNECTFAILED                     = 0x000010D3,
        ERRINFO_CONFIRMACTIVEWRONGSHAREID         = 0x000010D4,
        ERRINFO_CONFIRMACTIVEWRONGORIGINATOR      = 0x000010D5,
        ERRINFO_PERSISTENTKEYPDUBADLENGTH         = 0x000010DA,
        ERRINFO_PERSISTENTKEYPDUILLEGALFIRST      = 0x000010DB,
        ERRINFO_PERSISTENTKEYPDUTOOMANYTOTALKEYS  = 0x000010DC,
        ERRINFO_PERSISTENTKEYPDUTOOMANYCACHEKEYS  = 0x000010DD,
        ERRINFO_INPUTPDUBADLENGTH                 = 0x000010DE,
        ERRINFO_BITMAPCACHEERRORPDUBADLENGTH      = 0x000010DF,
        ERRINFO_SECURITYDATATOOSHORT              = 0x000010E0,
        ERRINFO_VCHANNELDATATOOSHORT              = 0x000010E1,
        ERRINFO_SHAREDATATOOSHORT                 = 0x000010E2,
        ERRINFO_BADSUPRESSOUTPUTPDU               = 0x000010E3,
        ERRINFO_CONFIRMACTIVEPDUTOOSHORT          = 0x000010E5,
        ERRINFO_CAPABILITYSETTOOSMALL             = 0x000010E7,
        ERRINFO_CAPABILITYSETTOOLARGE             = 0x000010E8,
        ERRINFO_NOCURSORCACHE                     = 0x000010E9,
        ERRINFO_BADCAPABILITIES                   = 0x000010EA,
        ERRINFO_VIRTUALCHANNELDECOMPRESSIONERR    = 0x000010EC,
        ERRINFO_INVALIDVCCOMPRESSIONTYPE          = 0x000010ED,
        ERRINFO_INVALIDCHANNELID                  = 0x000010EF,
        ERRINFO_VCHANNELSTOOMANY                  = 0x000010F0,
        ERRINFO_REMOTEAPPSNOTENABLED              = 0x000010F3,
        ERRINFO_CACHECAPNOTSET                    = 0x000010F4,
        ERRINFO_BITMAPCACHEERRORPDUBADLENGTH2     = 0x000010F5,
        ERRINFO_OFFSCRCACHEERRORPDUBADLENGTH      = 0x000010F6,
        ERRINFO_DNGCACHEERRORPDUBADLENGTH         = 0x000010F7,
        ERRINFO_GDIPLUSPDUBADLENGTH               = 0x000010F8,
        ERRINFO_SECURITYDATATOOSHORT2             = 0x00001111,
        ERRINFO_SECURITYDATATOOSHORT3             = 0x00001112,
        ERRINFO_SECURITYDATATOOSHORT4             = 0x00001113,
        ERRINFO_SECURITYDATATOOSHORT5             = 0x00001114,
        ERRINFO_SECURITYDATATOOSHORT6             = 0x00001115,
        ERRINFO_SECURITYDATATOOSHORT7             = 0x00001116,
        ERRINFO_SECURITYDATATOOSHORT8             = 0x00001117,
        ERRINFO_SECURITYDATATOOSHORT9             = 0x00001118,
        ERRINFO_SECURITYDATATOOSHORT10            = 0x00001119,
        ERRINFO_SECURITYDATATOOSHORT11            = 0x0000111A,
        ERRINFO_SECURITYDATATOOSHORT12            = 0x0000111B,
        ERRINFO_SECURITYDATATOOSHORT13            = 0x0000111C,
        ERRINFO_SECURITYDATATOOSHORT14            = 0x0000111D,
        ERRINFO_SECURITYDATATOOSHORT15            = 0x0000111E,
        ERRINFO_SECURITYDATATOOSHORT16            = 0x0000111F,
        ERRINFO_SECURITYDATATOOSHORT17            = 0x00001120,
        ERRINFO_SECURITYDATATOOSHORT18            = 0x00001121,
        ERRINFO_SECURITYDATATOOSHORT19            = 0x00001122,
        ERRINFO_SECURITYDATATOOSHORT20            = 0x00001123,
        ERRINFO_SECURITYDATATOOSHORT21            = 0x00001124,
        ERRINFO_SECURITYDATATOOSHORT22            = 0x00001125,
        ERRINFO_SECURITYDATATOOSHORT23            = 0x00001126,
        ERRINFO_BADMONITORDATA                    = 0x00001129,
        ERRINFO_VCDECOMPRESSEDREASSEMBLEFAILED    = 0x0000112A,
        ERRINFO_VCDATATOOLONG                     = 0x0000112B,
        ERRINFO_BAD_FRAME_ACK_DATA                = 0x0000112C,
        ERRINFO_GRAPHICSMODENOTSUPPORTED          = 0x0000112D,
        ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED      = 0x0000112E,
        ERRINFO_GRAPHICSSUBSYSTEMFAILED           = 0x0000112F,
        ERRINFO_TIMEZONEKEYNAMELENGTHTOOSHORT     = 0x00001130,
        ERRINFO_TIMEZONEKEYNAMELENGTHTOOLONG      = 0x00001131,
        ERRINFO_DYNAMICDSTDISABLEDFIELDMISSING    = 0x00001132,
        ERRINFO_UPDATESESSIONKEYFAILED            = 0x00001191,
        ERRINFO_DECRYPTFAILED                     = 0x00001192,
        ERRINFO_ENCRYPTFAILED                     = 0x00001193,
        ERRINFO_ENCPKGMISMATCH                    = 0x00001194,
        ERRINFO_DECRYPTFAILED2                    = 0x00001195
    };

    void process_disconnect_pdu(Stream & stream) {
        uint32_t errorInfo = stream.in_uint32_le();
        switch (errorInfo){
        case ERRINFO_RPC_INITIATED_DISCONNECT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "RPC_INITIATED_DISCONNECT");
            break;
        case ERRINFO_RPC_INITIATED_LOGOFF:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "RPC_INITIATED_LOGOFF");
            break;
        case ERRINFO_IDLE_TIMEOUT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "IDLE_TIMEOUT");
            break;
        case ERRINFO_LOGON_TIMEOUT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LOGON_TIMEOUT");
            break;
        case ERRINFO_DISCONNECTED_BY_OTHERCONNECTION:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "DISCONNECTED_BY_OTHERCONNECTION");
            break;
        case ERRINFO_OUT_OF_MEMORY:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "OUT_OF_MEMORY");
            break;
        case ERRINFO_SERVER_DENIED_CONNECTION:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SERVER_DENIED_CONNECTION");
            break;
        case ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SERVER_INSUFFICIENT_PRIVILEGES");
            break;
        case ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SERVER_FRESH_CREDENTIALS_REQUIRED");
            break;
        case ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "RPC_INITIATED_DISCONNECT_BYUSER");
            break;
        case ERRINFO_LOGOFF_BY_USER:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LOGOFF_BY_USER");
            break;
        case ERRINFO_LICENSE_INTERNAL:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_INTERNAL");
            break;
        case ERRINFO_LICENSE_NO_LICENSE_SERVER:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_NO_LICENSE_SERVER");
            break;
        case ERRINFO_LICENSE_NO_LICENSE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_NO_LICENSE");
            break;
        case ERRINFO_LICENSE_BAD_CLIENT_MSG:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_BAD_CLIENT_MSG");
            break;
        case ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_HWID_DOESNT_MATCH_LICENSE");
            break;
        case ERRINFO_LICENSE_BAD_CLIENT_LICENSE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_BAD_CLIENT_LICENSE");
            break;
        case ERRINFO_LICENSE_CANT_FINISH_PROTOCOL:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_CANT_FINISH_PROTOCOL");
            break;
        case ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_CLIENT_ENDED_PROTOCOL");
            break;
        case ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_BAD_CLIENT_ENCRYPTION");
            break;
        case ERRINFO_LICENSE_CANT_UPGRADE_LICENSE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_CANT_UPGRADE_LICENSE");
            break;
        case ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "LICENSE_NO_REMOTE_CONNECTIONS");
            break;
        case ERRINFO_CB_DESTINATION_NOT_FOUND:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_DESTINATION_NOT_FOUND");
            break;
        case ERRINFO_CB_LOADING_DESTINATION:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_LOADING_DESTINATION");
            break;
        case ERRINFO_CB_REDIRECTING_TO_DESTINATION:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_REDIRECTING_TO_DESTINATION");
            break;
        case ERRINFO_CB_SESSION_ONLINE_VM_WAKE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_SESSION_ONLINE_VM_WAKE");
            break;
        case ERRINFO_CB_SESSION_ONLINE_VM_BOOT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_SESSION_ONLINE_VM_BOOT");
            break;
        case ERRINFO_CB_SESSION_ONLINE_VM_NO_DNS:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_SESSION_ONLINE_VM_NO_DNS");
            break;
        case ERRINFO_CB_DESTINATION_POOL_NOT_FREE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_DESTINATION_POOL_NOT_FREE");
            break;
        case ERRINFO_CB_CONNECTION_CANCELLED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_CONNECTION_CANCELLED");
            break;
        case ERRINFO_CB_CONNECTION_ERROR_INVALID_SETTINGS:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_CONNECTION_ERROR_INVALID_SETTINGS");
            break;
        case ERRINFO_CB_SESSION_ONLINE_VM_BOOT_TIMEOUT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_SESSION_ONLINE_VM_BOOT_TIMEOUT");
            break;
        case ERRINFO_CB_SESSION_ONLINE_VM_SESSMON_FAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CB_SESSION_ONLINE_VM_SESSMON_FAILED");
            break;
        case ERRINFO_UNKNOWNPDUTYPE2:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "UNKNOWNPDUTYPE2");
            break;
        case ERRINFO_UNKNOWNPDUTYPE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "UNKNOWNPDUTYPE");
            break;
        case ERRINFO_DATAPDUSEQUENCE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "DATAPDUSEQUENCE");
            break;
        case ERRINFO_CONTROLPDUSEQUENCE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CONTROLPDUSEQUENCE");
            break;
        case ERRINFO_INVALIDCONTROLPDUACTION:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INVALIDCONTROLPDUACTION");
            break;
        case ERRINFO_INVALIDINPUTPDUTYPE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INVALIDINPUTPDUTYPE");
            break;
        case ERRINFO_INVALIDINPUTPDUMOUSE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INVALIDINPUTPDUMOUSE");
            break;
        case ERRINFO_INVALIDREFRESHRECTPDU:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INVALIDREFRESHRECTPDU");
            break;
        case ERRINFO_CREATEUSERDATAFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CREATEUSERDATAFAILED");
            break;
        case ERRINFO_CONNECTFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CONNECTFAILED");
            break;
        case ERRINFO_CONFIRMACTIVEWRONGSHAREID:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CONFIRMACTIVEWRONGSHAREID");
            break;
        case ERRINFO_CONFIRMACTIVEWRONGORIGINATOR:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CONFIRMACTIVEWRONGORIGINATOR");
            break;
        case ERRINFO_PERSISTENTKEYPDUBADLENGTH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "PERSISTENTKEYPDUBADLENGTH");
            break;
        case ERRINFO_PERSISTENTKEYPDUILLEGALFIRST:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "PERSISTENTKEYPDUILLEGALFIRST");
            break;
        case ERRINFO_PERSISTENTKEYPDUTOOMANYTOTALKEYS:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "PERSISTENTKEYPDUTOOMANYTOTALKEYS");
            break;
        case ERRINFO_PERSISTENTKEYPDUTOOMANYCACHEKEYS:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "PERSISTENTKEYPDUTOOMANYCACHEKEYS");
            break;
        case ERRINFO_INPUTPDUBADLENGTH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INPUTPDUBADLENGTH");
            break;
        case ERRINFO_BITMAPCACHEERRORPDUBADLENGTH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "BITMAPCACHEERRORPDUBADLENGTH");
            break;
        case ERRINFO_SECURITYDATATOOSHORT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT");
            break;
        case ERRINFO_VCHANNELDATATOOSHORT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "VCHANNELDATATOOSHORT");
            break;
        case ERRINFO_SHAREDATATOOSHORT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SHAREDATATOOSHORT");
            break;
        case ERRINFO_BADSUPRESSOUTPUTPDU:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "BADSUPRESSOUTPUTPDU");
            break;
        case ERRINFO_CONFIRMACTIVEPDUTOOSHORT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CONFIRMACTIVEPDUTOOSHORT");
            break;
        case ERRINFO_CAPABILITYSETTOOSMALL:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CAPABILITYSETTOOSMALL");
            break;
        case ERRINFO_CAPABILITYSETTOOLARGE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CAPABILITYSETTOOLARGE");
            break;
        case ERRINFO_NOCURSORCACHE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "NOCURSORCACHE");
            break;
        case ERRINFO_BADCAPABILITIES:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "BADCAPABILITIES");
            break;
        case ERRINFO_VIRTUALCHANNELDECOMPRESSIONERR:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "VIRTUALCHANNELDECOMPRESSIONERR");
            break;
        case ERRINFO_INVALIDVCCOMPRESSIONTYPE:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INVALIDVCCOMPRESSIONTYPE");
            break;
        case ERRINFO_INVALIDCHANNELID:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "INVALIDCHANNELID");
            break;
        case ERRINFO_VCHANNELSTOOMANY:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "VCHANNELSTOOMANY");
            break;
        case ERRINFO_REMOTEAPPSNOTENABLED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "REMOTEAPPSNOTENABLED");
            break;
        case ERRINFO_CACHECAPNOTSET:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "CACHECAPNOTSET");
            break;
        case ERRINFO_BITMAPCACHEERRORPDUBADLENGTH2:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "BITMAPCACHEERRORPDUBADLENGTH2");
            break;
        case ERRINFO_OFFSCRCACHEERRORPDUBADLENGTH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "OFFSCRCACHEERRORPDUBADLENGTH");
            break;
        case ERRINFO_DNGCACHEERRORPDUBADLENGTH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "DNGCACHEERRORPDUBADLENGTH");
            break;
        case ERRINFO_GDIPLUSPDUBADLENGTH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "GDIPLUSPDUBADLENGTH");
            break;
        case ERRINFO_SECURITYDATATOOSHORT2:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT2");
            break;
        case ERRINFO_SECURITYDATATOOSHORT3:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT3");
            break;
        case ERRINFO_SECURITYDATATOOSHORT4:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT4");
            break;
        case ERRINFO_SECURITYDATATOOSHORT5:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT5");
            break;
        case ERRINFO_SECURITYDATATOOSHORT6:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT6");
            break;
        case ERRINFO_SECURITYDATATOOSHORT7:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT7");
            break;
        case ERRINFO_SECURITYDATATOOSHORT8:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT8");
            break;
        case ERRINFO_SECURITYDATATOOSHORT9:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT9");
            break;
        case ERRINFO_SECURITYDATATOOSHORT10:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT10");
            break;
        case ERRINFO_SECURITYDATATOOSHORT11:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT11");
            break;
        case ERRINFO_SECURITYDATATOOSHORT12:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT12");
            break;
        case ERRINFO_SECURITYDATATOOSHORT13:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT13");
            break;
        case ERRINFO_SECURITYDATATOOSHORT14:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT14");
            break;
        case ERRINFO_SECURITYDATATOOSHORT15:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT15");
            break;
        case ERRINFO_SECURITYDATATOOSHORT16:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT16");
            break;
        case ERRINFO_SECURITYDATATOOSHORT17:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT17");
            break;
        case ERRINFO_SECURITYDATATOOSHORT18:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT18");
            break;
        case ERRINFO_SECURITYDATATOOSHORT19:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT19");
            break;
        case ERRINFO_SECURITYDATATOOSHORT20:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT20");
            break;
        case ERRINFO_SECURITYDATATOOSHORT21:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT21");
            break;
        case ERRINFO_SECURITYDATATOOSHORT22:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT22");
            break;
        case ERRINFO_SECURITYDATATOOSHORT23:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "SECURITYDATATOOSHORT23");
            break;
        case ERRINFO_BADMONITORDATA:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "BADMONITORDATA");
            break;
        case ERRINFO_VCDECOMPRESSEDREASSEMBLEFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "VCDECOMPRESSEDREASSEMBLEFAILED");
            break;
        case ERRINFO_VCDATATOOLONG:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "VCDATATOOLONG");
            break;
        case ERRINFO_BAD_FRAME_ACK_DATA:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "BAD_FRAME_ACK_DATA");
            break;
        case ERRINFO_GRAPHICSMODENOTSUPPORTED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "GRAPHICSMODENOTSUPPORTED");
            break;
        case ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "GRAPHICSSUBSYSTEMRESETFAILED");
            break;
        case ERRINFO_GRAPHICSSUBSYSTEMFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "GRAPHICSSUBSYSTEMFAILED");
            break;
        case ERRINFO_TIMEZONEKEYNAMELENGTHTOOSHORT:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "TIMEZONEKEYNAMELENGTHTOOSHORT");
            break;
        case ERRINFO_TIMEZONEKEYNAMELENGTHTOOLONG:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "TIMEZONEKEYNAMELENGTHTOOLONG");
            break;
        case ERRINFO_DYNAMICDSTDISABLEDFIELDMISSING:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "DYNAMICDSTDISABLEDFIELDMISSING");
            break;
        case ERRINFO_UPDATESESSIONKEYFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "UPDATESESSIONKEYFAILED");
            break;
        case ERRINFO_DECRYPTFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "DECRYPTFAILED");
            break;
        case ERRINFO_ENCRYPTFAILED:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "ENCRYPTFAILED");
            break;
        case ERRINFO_ENCPKGMISMATCH:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "ENCPKGMISMATCH");
            break;
        case ERRINFO_DECRYPTFAILED2:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "DECRYPTFAILED2");
            break;
        default:
            LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "?");
            break;
        }
    }   // process_disconnect_pdu

    void process_logon_info(const char * domain, const char * username) {
        char domain_username_format_0[2048];
        char domain_username_format_1[2048];

        snprintf(domain_username_format_0, sizeof(domain_username_format_0),
            "%s@%s", username, domain);
        snprintf(domain_username_format_1, sizeof(domain_username_format_0),
            "%s\\%s", domain, username);
        //LOG(LOG_INFO,
        //    "Domain username format 0=(%s) Domain username format 1=(%s)",
        //    domain_username_format_0, domain_username_format_0);

        if (this->disconnect_on_logon_user_change &&
            ((strcasecmp(domain, this->domain) || strcasecmp(username, this->username)) &&
             (this->domain[0] ||
              (strcasecmp(domain_username_format_0, this->username) &&
               strcasecmp(domain_username_format_1, this->username) &&
               strcasecmp(username, this->username))))) {
            if (this->error_message) {
                *this->error_message = "Unauthorized logon user change detected!";
            }
            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s\\%s) -> (%s\\%s). "
                    "The session will be disconnected.",
                this->hostname, this->domain, this->username, domain, username);
            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s\\%s). "
                    "The session will be disconnected.",
                this->hostname, domain, username);

            this->end_session_reason = "OPEN_SESSION_FAILED";
            this->end_session_message = "Unauthorized logon user change detected.";

            throw Error(ERR_RDP_LOGON_USER_CHANGED);
        }

        if (this->acl)
        {
            this->acl->report("OPEN_SESSION_SUCCESSFUL", "Ok.");
        }
        this->end_session_reason = "CLOSE_SESSION_SUCCESSFUL";
        this->end_session_message = "OK.";

        if (this->open_session_timeout) {
            this->open_session_timeout_checker.cancel_timeout();

            this->event.reset();
        }
    }

    void process_save_session_info(Stream & stream) {
        RDP::SaveSessionInfoPDUData_Recv ssipdudata(stream);

        switch (ssipdudata.infoType) {
        case RDP::INFOTYPE_LOGON:
        {
            LOG(LOG_INFO, "process save session info : Logon");
            RDP::LogonInfoVersion1_Recv liv1(ssipdudata.payload);

            process_logon_info(reinterpret_cast<char *>(liv1.Domain),
                reinterpret_cast<char *>(liv1.UserName));
        }
        break;
        case RDP::INFOTYPE_LOGON_LONG:
        {
            LOG(LOG_INFO, "process save session info : Logon long");
            RDP::LogonInfoVersion2_Recv liv2(ssipdudata.payload);

            process_logon_info(reinterpret_cast<char *>(liv2.Domain),
                reinterpret_cast<char *>(liv2.UserName));
        }
        break;
        case RDP::INFOTYPE_LOGON_PLAINNOTIFY:
        {
            LOG(LOG_INFO, "process save session info : Logon plainnotify");
            RDP::PlainNotify_Recv pn(ssipdudata.payload);
        }
        break;
        case RDP::INFOTYPE_LOGON_EXTENDED_INFO:
        {
            LOG(LOG_INFO, "process save session info : Logon extended info");
            RDP::LogonInfoExtended_Recv lie(ssipdudata.payload);

            RDP::LogonInfoField_Recv lif(lie.payload);

            if (lie.FieldsPresent & RDP::LOGON_EX_AUTORECONNECTCOOKIE) {
                LOG(LOG_INFO, "process save session info : Auto-reconnect cookie");

                RDP::ServerAutoReconnectPacket_Recv sarp(lif.payload);
            }
            if (lie.FieldsPresent & RDP::LOGON_EX_LOGONERRORS) {
                LOG(LOG_INFO, "process save session info : Logon Errors Info");

                RDP::LogonErrorsInfo_Recv lei(lif.payload);
            }
        }
        break;
        }

        stream.p = stream.end;
    }

    TODO("CGR: this can probably be unified with process_confirm_active in front")
    void process_server_caps(Stream & stream, uint16_t len) {
        if (this->verbose & 32){
            LOG(LOG_INFO, "mod_rdp::process_server_caps");
        }

        struct autoclose_file {
            FILE * file;

            ~autoclose_file()
            {
                if (this->file) {
                    fclose(this->file);
                }
            }
        };
        FILE * const output_file =
            !this->output_filename.empty()
            ? fopen(this->output_filename.c_str(), "w")
            : nullptr;
        autoclose_file autoclose{output_file};

        unsigned expected = 4; /* numberCapabilities(2) + pad2Octets(2) */
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated Demand active PDU data, need=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        uint16_t ncapsets = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (uint16_t n = 0; n < ncapsets; n++) {
            expected = 4; /* capabilitySetType(2) + lengthCapability(2) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated Demand active PDU data, need=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_MCS_PDU_TRUNCATED);
            }

            uint16_t capset_type = stream.in_uint16_le();
            uint16_t capset_length = stream.in_uint16_le();

            expected = capset_length - 4 /* capabilitySetType(2) + lengthCapability(2) */;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated Demand active PDU data, need=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_MCS_PDU_TRUNCATED);
            }

            uint8_t * next = stream.p + expected;
            switch (capset_type) {
            case CAPSTYPE_GENERAL:
                {
                    GeneralCaps general_caps;
                    general_caps.recv(stream, capset_length);
                    if (this->verbose & 1) {
                        general_caps.log("Received from server");
                    }
                    if (output_file) {
                        general_caps.dump(output_file);
                    }
                }
                break;
            case CAPSTYPE_BITMAP:
                {
                    BitmapCaps bitmap_caps;
                    bitmap_caps.recv(stream, capset_length);
                    if (this->verbose & 1) {
                        bitmap_caps.log("Received from server");
                    }
                    if (output_file) {
                        bitmap_caps.dump(output_file);
                    }
                    this->bpp = bitmap_caps.preferredBitsPerPixel;
                    this->front_width = bitmap_caps.desktopWidth;
                    this->front_height = bitmap_caps.desktopHeight;
                }
                break;
            case CAPSTYPE_ORDER:
                {
                    OrderCaps order_caps;
                    order_caps.recv(stream, capset_length);
                    if (this->verbose & 1) {
                        order_caps.log("Received from server");
                    }
                    if (output_file) {
                        order_caps.dump(output_file);
                    }
                }
                break;
            case CAPSTYPE_INPUT:
                {
                    InputCaps input_caps;
                    input_caps.recv(stream, capset_length);
                    if (this->verbose & 1) {
                        input_caps.log("Received from server");
                    }

                    this->enable_fastpath_client_input_event =
                        (this->enable_fastpath && ((input_caps.inputFlags & (INPUT_FLAG_FASTPATH_INPUT | INPUT_FLAG_FASTPATH_INPUT2)) != 0));
                }
                break;
            default:
                break;
            }
            stream.p = next;
        }

        if (this->verbose & 32){
            LOG(LOG_INFO, "mod_rdp::process_server_caps done");
        }
    }   // process_server_caps

    void send_control(int action) throw(Error) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_control");
        }

        BStream stream(256);

        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(0); /* control id */
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_control done");
        }
    }

    /* Send persistent bitmap cache enumeration PDU's
       Not implemented yet because it should be implemented
       before in process_data case. The problem is that
       we don't save the bitmap key list attached with rdp_bmpcache2 capability
       message so we can't develop this function yet */

    void send_persistent_key_list_pdu(BStream & pdu_data_stream) throw(Error) {
        BStream persistent_key_list_stream(65535);

        ShareData sdata(persistent_key_list_stream);
        sdata.emit_begin(PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST, this->share_id, RDP::STREAM_MED);

        // Payload
        persistent_key_list_stream.out_copy_bytes(pdu_data_stream);
        persistent_key_list_stream.mark_end();

        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, persistent_key_list_stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(persistent_key_list_stream);
        target_stream.mark_end();

        this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    }

    void send_persistent_key_list_regular() throw(Error) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_regular");
        }

        uint16_t totalEntriesCache[BmpCache::MAXIMUM_NUMBER_OF_CACHES] = { 0, 0, 0, 0, 0 };

        for (uint8_t cache_id = 0; cache_id < this->orders.bmp_cache->number_of_cache; cache_id++) {
            const BmpCache::cache_ & cache = this->orders.bmp_cache->get_cache(cache_id);
            if (cache.persistent()) {
                uint16_t idx = 0;
                while (idx < cache.size() && cache[idx]) {
                    ++idx;
                }
                totalEntriesCache[cache_id] = idx;
            }
        }
        //LOG(LOG_INFO, "totalEntriesCache0=%u totalEntriesCache1=%u totalEntriesCache2=%u totalEntriesCache3=%u totalEntriesCache4=%u",
        //    totalEntriesCache[0], totalEntriesCache[1], totalEntriesCache[2], totalEntriesCache[3], totalEntriesCache[4]);

        uint16_t total_number_of_entries = totalEntriesCache[0] + totalEntriesCache[1] + totalEntriesCache[2] +
                                           totalEntriesCache[3] + totalEntriesCache[4];
        if (total_number_of_entries > 0) {
            RDP::PersistentKeyListPDUData pklpdu;
            pklpdu.bBitMask |= RDP::PERSIST_FIRST_PDU;

            uint16_t number_of_entries     = 0;
            uint8_t  pdu_number_of_entries = 0;
            for (uint8_t cache_id = 0; cache_id < this->orders.bmp_cache->number_of_cache; cache_id++) {
                const BmpCache::cache_ & cache = this->orders.bmp_cache->get_cache(cache_id);

                if (!cache.persistent()) {
                    continue;
                }

                const uint16_t entries_max = totalEntriesCache[cache_id];
                for (uint16_t cache_index = 0; cache_index < entries_max; cache_index++) {
                    pklpdu.entries[pdu_number_of_entries].Key1 = cache[cache_index].sig.sig_32[0];
                    pklpdu.entries[pdu_number_of_entries].Key2 = cache[cache_index].sig.sig_32[1];

                    pklpdu.numEntriesCache[cache_id]++;
                    number_of_entries++;
                    pdu_number_of_entries++;

                    if ((pdu_number_of_entries == RDP::PersistentKeyListPDUData::MAXIMUM_ENCAPSULATED_BITMAP_KEYS) ||
                        (number_of_entries == total_number_of_entries))
                    {
                        if (number_of_entries == total_number_of_entries) {
                            pklpdu.bBitMask |= RDP::PERSIST_LAST_PDU;
                        }

                        pklpdu.totalEntriesCache[0] = totalEntriesCache[0];
                        pklpdu.totalEntriesCache[1] = totalEntriesCache[1];
                        pklpdu.totalEntriesCache[2] = totalEntriesCache[2];
                        pklpdu.totalEntriesCache[3] = totalEntriesCache[3];
                        pklpdu.totalEntriesCache[4] = totalEntriesCache[4];

                        //pklpdu.log(LOG_INFO, "Send to server");

                        BStream pdu_data_stream(65535);
                        pklpdu.emit(pdu_data_stream);
                        pdu_data_stream.mark_end();

                        this->send_persistent_key_list_pdu(pdu_data_stream);

                        pklpdu.reset();

                        pdu_number_of_entries = 0;
                    }
                }
            }
        }

        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_regular done");
        }
    }   // send_persistent_key_list_regular

    void send_persistent_key_list_transparent() throw(Error) {
        if (!this->persistent_key_list_transport) {
            return;
        }

        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_transparent");
        }

        BStream pdu_data_stream(65535);

        bool bContinue = true;
        while (bContinue) {
            try
            {
                pdu_data_stream.reset();

                this->persistent_key_list_transport->recv(&pdu_data_stream.end, 2/*pdu_size(2)*/);

                uint16_t pdu_size = pdu_data_stream.in_uint16_le();


                pdu_data_stream.reset();

                this->persistent_key_list_transport->recv(&pdu_data_stream.end, pdu_size);

                if (this->verbose & 1) {
                    SubStream stream(pdu_data_stream);
                    RDP::PersistentKeyListPDUData pklpdu;
                    pklpdu.receive(stream);
                    pklpdu.log(LOG_INFO, "Send to server");
                }

                this->send_persistent_key_list_pdu(pdu_data_stream);
            }
            catch (Error & e)
            {
                if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
                    LOG(LOG_ERR, "mod_rdp::send_persistent_key_list_transparent: error=%u", e.id);
                    throw;
                }

                bContinue = false;
            }
        }

        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_transparent done");
        }
    }

    void send_persistent_key_list() throw(Error) {
        if (this->enable_transparent_mode) {
            this->send_persistent_key_list_transparent();
        }
        else {
            this->send_persistent_key_list_regular();
        }
    }

    TODO("CGR: duplicated code in front")
    void send_synchronise() throw(Error) {
        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_synchronise");
        }
        BStream stream(65536);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_SYNCHRONIZE, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(1); /* type */
        stream.out_uint16_le(1002);
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_synchronise done");
        }
    }

    void send_fonts(int seq) throw(Error) {
        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_fonts");
        }
        BStream stream(65536);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_FONTLIST, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(0); /* number of fonts */
        stream.out_uint16_le(0); /* pad? */
        stream.out_uint16_le(seq); /* unknown */
        stream.out_uint16_le(0x32); /* entry size */
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_fonts done");
        }
    }

public:

    void send_input_slowpath(int time, int message_type, int device_flags, int param1, int param2) throw(Error) {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::send_input_slowpath");
        }
        BStream stream(65536);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_INPUT, this->share_id, RDP::STREAM_HI);

        // Payload
        stream.out_uint16_le(1); /* number of events */
        stream.out_uint16_le(0);
        stream.out_uint32_le(time);
        stream.out_uint16_le(message_type);
        stream.out_uint16_le(device_flags);
        stream.out_uint16_le(param1);
        stream.out_uint16_le(param2);
        stream.mark_end();

        // Packet trailer
        sdata.emit_end();

        BStream sctrl_header(256);
        ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, stream.size());

        HStream target_stream(1024, 65536);
        target_stream.out_copy_bytes(sctrl_header);
        target_stream.out_copy_bytes(stream);
        target_stream.mark_end();

        this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);

        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::send_input_slowpath done");
        }
    }

    void send_input_fastpath(int time, int message_type, int device_flags, int param1, int param2) throw(Error) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::send_input_fastpath");
        }

        BStream fastpath_header(256);
        HStream stream(256, 512);

        switch (message_type) {
        case RDP_INPUT_SCANCODE:
            FastPath::KeyboardEvent_Send(stream, (uint16_t)device_flags, param1);
            break;

        case RDP_INPUT_SYNCHRONIZE:
            FastPath::SynchronizeEvent_Send(stream, param1);
            break;

        case RDP_INPUT_MOUSE:
            FastPath::MouseEvent_Send(stream, (uint16_t)device_flags, param1, param2);
            break;

        default:
            LOG(LOG_WARNING, "unsupported fast-path input message type 0x%x", message_type);
            throw Error(ERR_RDP_FASTPATH);
        }

        FastPath::ClientInputEventPDU_Send out_cie(fastpath_header, stream, 1, this->encrypt, this->encryptionLevel, this->encryptionMethod);

        this->nego.trans.send(fastpath_header, stream);

        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::send_input_fastpath done");
        }
    }

    void send_input(int time, int message_type, int device_flags, int param1, int param2) throw(Error) {
        if (this->enable_fastpath_client_input_event == false) {
            this->send_input_slowpath(time, message_type, device_flags, param1, param2);
        }
        else {
            this->send_input_fastpath(time, message_type, device_flags, param1, param2);
        }
    }

    virtual void rdp_input_invalidate(const Rect & r) {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate");
        }
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            if (!r.isempty()){
                RDP::RefreshRectPDU rrpdu(this->share_id,
                                          this->userid,
                                          this->encryptionLevel,
                                          this->encrypt);

                rrpdu.addInclusiveRect(r.x, r.y, r.x + r.cx - 1, r.y + r.cy - 1);

                rrpdu.emit(this->nego.trans);
            }
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate done");
        }
    }

    virtual void rdp_input_invalidate2(const DArray<Rect> & vr) {
        LOG(LOG_INFO, " ===================> mod_rdp::rdp_input_invalidate 2 <=====================");
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate");
        }
        if ((UP_AND_RUNNING == this->connection_finalization_state)
            && (vr.size() > 0)) {
            RDP::RefreshRectPDU rrpdu(this->share_id,
                                      this->userid,
                                      this->encryptionLevel,
                                      this->encrypt);
            for (size_t i = 0; i < vr.size() ; i++){
                if (!vr[i].isempty()){
                    rrpdu.addInclusiveRect(vr[i].x, vr[i].y, vr[i].x + vr[i].cx - 1, vr[i].y + vr[i].cy - 1);
                }
            }
            rrpdu.emit(this->nego.trans);
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate done");
        }

    };

    // 2.2.9.1.2.1.7 Fast-Path Color Pointer Update (TS_FP_COLORPOINTERATTRIBUTE)
    // =========================================================================

    // updateHeader (1 byte): An 8-bit, unsigned integer. The format of this field is
    // the same as the updateHeader byte field specified in the Fast-Path Update
    // (section 2.2.9.1.2.1) structure. The updateCode bitfield (4 bits in size) MUST
    // be set to FASTPATH_UPDATETYPE_COLOR (9).

    // compressionFlags (1 byte): An 8-bit, unsigned integer. The format of this optional
    // field (as well as the possible values) is the same as the compressionFlags field
    // specified in the Fast-Path Update structure.

    // size (2 bytes): A 16-bit, unsigned integer. The format of this field (as well as
    // the possible values) is the same as the size field specified in the Fast-Path
    // Update structure.

    // colorPointerUpdateData (variable): Color pointer data. Both slow-path and
    // fast-path utilize the same data format, a Color Pointer Update (section
    // 2.2.9.1.1.4.4) structure, to represent this information.

    // 2.2.9.1.1.4.4 Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
    // =============================================================

    // The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
    // color pointer, as specified in [T128] section 8.14.3. This pointer update
    // is used for both monochrome and color pointers in RDP.

    //    cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
    // entry in the pointer cache in which to store the pointer image. The number
    // of cache entries is specified using the Pointer Capability Set (section 2.2.7.1.5).

    //    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1 ) structure containing
    // the x-coordinates and y-coordinates of the pointer hotspot.

    //    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer
    // in pixels. The maximum allowed pointer width is 96 pixels if the client
    // indicated support for large pointers by setting the LARGE_POINTER_FLAG (0x00000001)
    // in the Large Pointer Capability Set (section 2.2.7.2.7). If the LARGE_POINTER_FLAG
    // was not set, the maximum allowed pointer width is 32 pixels.

    //    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
    // in pixels. The maximum allowed pointer height is 96 pixels if the client
    // indicated support for large pointers by setting the LARGE_POINTER_FLAG (0x00000001)
    // in the Large Pointer Capability Set (section 2.2.7.2.7). If the LARGE_POINTER_FLAG
    // was not set, the maximum allowed pointer height is 32 pixels.

    //    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
    // andMaskData field.

    //    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
    // xorMaskData field.

    //    xorMaskData (variable): A variable-length array of bytes. Contains the 24-bpp,
    // bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte boundary for
    // each encoded scan-line. For example, if a 3x3 pixel cursor is being sent, then each
    // scan-line will consume 10 bytes (3 pixels per scan-line multiplied by 3 bytes per pixel,
    // rounded up to the next even number of bytes).

    //    andMaskData (variable): A variable-length array of bytes. Contains the 1-bpp, bottom-up
    // AND mask scan-line data. The AND mask is padded to a 2-byte boundary for each encoded scan-line.
    // For example, if a 7x7 pixel cursor is being sent, then each scan-line will consume 2 bytes
    // (7 pixels per scan-line multiplied by 1 bpp, rounded up to the next even number of bytes).

    //    pad (1 byte): An optional 8-bit, unsigned integer. Padding. Values in this field MUST be ignored.

    void process_color_pointer_pdu(Stream & stream) throw(Error) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu");
        }
        unsigned pointer_cache_idx = stream.in_uint16_le();
        if (pointer_cache_idx >= (sizeof(this->cursors) / sizeof(this->cursors[0]))) {
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_CACHE_NOT_OK);
        }

        struct Pointer & cursor = this->cursors[pointer_cache_idx];

        memset(&cursor, 0, sizeof(struct Pointer));
        cursor.bpp = 24;
        cursor.x      = stream.in_uint16_le();
        cursor.y      = stream.in_uint16_le();
        cursor.width  = stream.in_uint16_le();
        cursor.height = stream.in_uint16_le();
        unsigned mlen  = stream.in_uint16_le(); /* mask length */
        unsigned dlen  = stream.in_uint16_le(); /* data length */

        if ((mlen > sizeof(cursor.mask)) || (dlen > sizeof(cursor.data))) {
            LOG(LOG_WARNING, "mod_rdp::Bad length for color pointer mask_len=%u data_len=%u", mlen, dlen);
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }
        TODO("this is modifiying cursor in place: we should not do that.");
        memcpy(cursor.data, stream.in_uint8p(dlen), dlen);
        memcpy(cursor.mask, stream.in_uint8p(mlen), mlen);

        this->front.server_set_pointer(cursor);
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu done");
        }
    }

    // 2.2.9.1.1.4.6 Cached Pointer Update (TS_CACHEDPOINTERATTRIBUTE)
    // ---------------------------------------------------------------

    // The TS_CACHEDPOINTERATTRIBUTE structure is used to instruct the
    // client to change the current pointer shape to one already present
    // in the pointer cache.

    // cacheIndex (2 bytes): A 16-bit, unsigned integer. A zero-based
    // cache entry containing the cache index of the cached pointer to
    // which the client's pointer MUST be changed. The pointer data MUST
    // have already been cached using either the Color Pointer Update
    // (section 2.2.9.1.1.4.4) or New Pointer Update (section 2.2.9.1.1.4.5).

    void process_cached_pointer_pdu(Stream & stream)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu");
        }

        TODO("Add check that the idx transmitted is actually an used pointer")
        uint16_t pointer_idx = stream.in_uint16_le();
        if (pointer_idx >= (sizeof(this->cursors) / sizeof(Pointer))) {
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu pointer cache idx overflow (%d)", pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        struct Pointer & cursor = this->cursors[pointer_idx];
        if (cursor.is_valid()) {
            this->front.server_set_pointer(cursor);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::process_cached_pointer_pdu: incalid cache cell index, use system default. index=%u", pointer_idx);
            Pointer cursor(Pointer::POINTER_NORMAL);
            this->front.server_set_pointer(cursor);
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu done");
        }
    }

    // 2.2.9.1.1.4.3 System Pointer Update (TS_SYSTEMPOINTERATTRIBUTE)
    // ---------------------------------------------------------------

    // systemPointerType (4 bytes): A 32-bit, unsigned integer. The type of system pointer.

    // +---------------------------+-----------------------------+
    // |      Value                |      Meaning                |
    // +---------------------------+-----------------------------+
    // | SYSPTR_NULL    0x00000000 | The hidden pointer.         |
    // +---------------------------+-----------------------------+
    // | SYSPTR_DEFAULT 0x00007F00 | The default system pointer. |
    // +---------------------------+-----------------------------+

    void process_system_pointer_pdu(Stream & stream)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu");
        }
        int system_pointer_type = stream.in_uint32_le();
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                struct Pointer cursor;
                memset(cursor.mask, 0xff, sizeof(cursor.mask));
                this->front.server_set_pointer(cursor);
            }
            break;
        default:
            {
                Pointer cursor(Pointer::POINTER_NORMAL);
                this->front.server_set_pointer(cursor);
            }
            break;
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu done");
        }
    }

    void to_regular_mask(const uint8_t * indata, unsigned mlen, uint8_t bpp, uint8_t * mask, size_t mask_size) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_mask");
        }

        TODO("check code below: why do we revert mask and pointer when pointer is 1 BPP and not with other color depth ?"
             " Looks fishy, a mask and pointer should always be encoded in the same way, not depending on color depth"
             "difficult to see for symmetrical pointers... check documentation");
        TODO("it may be more efficient to revert cursor after creating it instead of doing it on the fly")
        switch (bpp) {
        case 1 :
        {
            for (unsigned x = 0; x < mlen ; x++) {
                BGRColor px = indata[x];
                // incoming new pointer mask is upside down, revert it
                mask[128 - 4 - (x & 0x7C) + (x & 3)] = px;
            }
        }
        break;
        default:
            memcpy(mask, indata, mlen);
        break;
        }

        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_mask");
        }
    }

    void to_regular_pointer(const uint8_t * indata, unsigned dlen, uint8_t bpp, uint8_t * data, size_t target_data_len) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_pointer");
        }
        switch (bpp) {
        case 1 :
        {
            for (unsigned x = 0; x < dlen ; x ++) {
                BGRColor px = indata[x];
                // target cursor will receive 8 bits input at once
                for (unsigned b = 0 ; b < 8 ; b++) {
                    // incoming new pointer is upside down, revert it
                    uint8_t * bstart = &(data[24 * (128 - 4 - (x & 0xFFFC) + (x & 3))]);
                    // emit all individual bits
                    ::out_bytes_le(bstart,      3, (px & 0x80) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart +  3, 3, (px & 0x40) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart +  6, 3, (px & 0x20) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart +  9, 3, (px & 0x10) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart + 12, 3, (px &    8) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart + 15, 3, (px &    4) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart + 18, 3, (px &    2) ? 0xFFFFFF : 0);
                    ::out_bytes_le(bstart + 21, 3, (px &    1) ? 0xFFFFFF : 0);
                }
            }
        }
        break;
        case 4 :
        {
            for (unsigned i = 0; i < dlen ; i++) {
                BGRColor px = indata[i];
                // target cursor will receive 8 bits input at once
                ::out_bytes_le(&(data[6 * i]),     3, color_decode((px >> 4) & 0xF, bpp, this->orders.global_palette));
                ::out_bytes_le(&(data[6 * i + 3]), 3, color_decode(px        & 0xF, bpp, this->orders.global_palette));
            }
        }
        break;
        case 32: case 24: case 16: case 15: case 8:
        {
            uint8_t BPP = nbbytes(bpp);
            for (unsigned i = 0; i + BPP <= dlen; i += BPP) {
                BGRColor px = in_uint32_from_nb_bytes_le(BPP, indata + i);
                ::out_bytes_le(&(data[(i/BPP)*3]), 3, color_decode(px, bpp, this->orders.global_palette));
            }
        }
        break;
        default:
            LOG(LOG_ERR, "Mouse pointer : color depth not supported %d, forcing green mouse (running in the grass ?)", bpp);
            for (size_t x = 0 ; x < 1024 ; x++) {
                ::out_bytes_le(data + x *3, 3, GREEN);
            }
            break;
        }

        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_pointer");
        }
    }

    // 2.2.9.1.1.4.5 New Pointer Update (TS_POINTERATTRIBUTE)
    // ------------------------------------------------------

    // The TS_POINTERATTRIBUTE structure is used to send pointer data at an arbitrary
    // color depth. Support for the New Pointer Update is advertised in the Pointer
    // Capability Set (section 2.2.7.1.5).


    // xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in bits-per-pixel
    // of the XOR mask contained in the colorPtrAttr field.

    // colorPtrAttr (variable): Encapsulated Color Pointer Update (section 2.2.9.1.1.4.4)
    //  structure which contains information about the pointer. The Color Pointer Update
    //  fields are all used, as specified in section 2.2.9.1.1.4.4; however color XOR data
    //  is presented in the color depth described in the xorBpp field (for 8 bpp, each byte
    //  contains one palette index; for 4 bpp, there are two palette indices per byte).


    void process_new_pointer_pdu(Stream & stream) throw(Error) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu");
        }

        unsigned data_bpp  = stream.in_uint16_le(); /* data bpp */
        unsigned pointer_idx = stream.in_uint16_le();

        if (pointer_idx >= (sizeof(this->cursors) / sizeof(Pointer))) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu pointer cache idx overflow (%d)", pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }

        Pointer & cursor = this->cursors[pointer_idx];
        memset(&cursor, 0, sizeof(struct Pointer));
        cursor.bpp    = 24;
        cursor.x      = stream.in_uint16_le();
        cursor.y      = stream.in_uint16_le();
        cursor.width  = stream.in_uint16_le();
        cursor.height = stream.in_uint16_le();
        uint16_t mlen  = stream.in_uint16_le(); /* mask length */
        uint16_t dlen  = stream.in_uint16_le(); /* data length */

        if (cursor.width > Pointer::MAX_WIDTH){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu pointer width overflow (%d)", cursor.width);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        if (cursor.height > Pointer::MAX_HEIGHT){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu pointer height overflow (%d)", cursor.height);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }

        if ((unsigned)cursor.x >= cursor.width){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu hotspot x out of pointer (%d >= %d)", cursor.x, cursor.width);
            cursor.x = 0;
        }

        if ((unsigned)cursor.y >= cursor.height){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu hotspot y out of pointer (%d >= %d)", cursor.y, cursor.height);
            cursor.y = 0;
        }

        if (!stream.in_check_rem(dlen)){
            LOG(LOG_ERR, "Not enough data for cursor pixels (need=%u remain=%u)", dlen, stream.in_remain());
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
        }
        if (!stream.in_check_rem(mlen + dlen)){
            LOG(LOG_ERR, "Not enough data for cursor mask (need=%u remain=%u)", mlen, stream.in_remain() - dlen);
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
        }

        size_t out_data_len = 3 * (
            (bpp == 1) ? (cursor.width * cursor.height) / 8 :
            (bpp == 4) ? (cursor.width * cursor.height) / 2 :
            (dlen / nbbytes(data_bpp)));

        if ((mlen > sizeof(cursor.mask)) ||
            (out_data_len > sizeof(cursor.data))) {
            LOG(LOG_WARNING,
                "mod_rdp::Bad length for color pointer mask_len=%u "
                    "data_len=%u Width = %u Height = %u bpp = %u out_data_len = %u nbbytes=%u",
                (unsigned)mlen, (unsigned)dlen, cursor.width, cursor.height,
                data_bpp, out_data_len, nbbytes(data_bpp));
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
        }

        if (data_bpp == 1) {
            uint8_t data_data[32*32/8];
            uint8_t mask_data[32*32/8];
            stream.in_copy_bytes(data_data, dlen);
            stream.in_copy_bytes(mask_data, mlen);

            for (unsigned i = 0 ; i < mlen; i++) {
                uint8_t new_mask_data = (mask_data[i] & (data_data[i] ^ 0xFF));
                uint8_t new_data_data = (data_data[i] ^ mask_data[i] ^ new_mask_data);
                data_data[i]    = new_data_data;
                mask_data[i]    = new_mask_data;
            }

            TODO("move that into cursor")
            this->to_regular_pointer(data_data, dlen, 1, cursor.data, sizeof(cursor.data));
            this->to_regular_mask(mask_data, mlen, 1, cursor.mask, sizeof(cursor.mask));
        }
        else {
            TODO("move that into cursor")
            this->to_regular_pointer(stream.p, dlen, data_bpp, cursor.data, sizeof(cursor.data));
            stream.in_skip_bytes(dlen);
            this->to_regular_mask(stream.p, mlen, data_bpp, cursor.mask, sizeof(cursor.mask));
            stream.in_skip_bytes(mlen);
        }

        this->front.server_set_pointer(cursor);
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu done");
        }
    }   // process_new_pointer_pdu

    void process_bitmap_updates(Stream & stream, bool fast_path) {
        if (this->verbose & 64){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates");
        }

        this->recv_bmp_update++;

        if (fast_path) {
            stream.in_skip_bytes(2); // updateType(2)
        }

        // RDP-BCGR: 2.2.9.1.1.3.1.2 Bitmap Update (TS_UPDATE_BITMAP)
        // ----------------------------------------------------------
        // The TS_UPDATE_BITMAP structure contains one or more rectangular
        // clippings taken from the server-side screen frame buffer (see [T128]
        // section 8.17).

        // shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
        // containing information about the packet. The type subfield of the
        // pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
        // MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share
        // Data Header MUST be set to PDUTYPE2_UPDATE (2).

        // bitmapData (variable): The actual bitmap update data, as specified in
        // section 2.2.9.1.1.3.1.2.1.

        // 2.2.9.1.1.3.1.2.1 Bitmap Update Data (TS_UPDATE_BITMAP_DATA)
        // ------------------------------------------------------------
        // The TS_UPDATE_BITMAP_DATA structure encapsulates the bitmap data that
        // defines a Bitmap Update (section 2.2.9.1.1.3.1.2).

        // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update
        // type. This field MUST be set to UPDATETYPE_BITMAP (0x0001).

        // numberRectangles (2 bytes): A 16-bit, unsigned integer.
        // The number of screen rectangles present in the rectangles field.
        size_t numberRectangles = stream.in_uint16_le();
        if (this->verbose & 64){
            LOG(LOG_INFO, "/* ---------------- Sending %d rectangles ----------------- */", numberRectangles);
        }

        for (size_t i = 0; i < numberRectangles; i++) {

            // rectangles (variable): Variable-length array of TS_BITMAP_DATA
            // (section 2.2.9.1.1.3.1.2.2) structures, each of which contains a
            // rectangular clipping taken from the server-side screen frame buffer.
            // The number of screen clippings in the array is specified by the
            // numberRectangles field.

            // 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
            // ----------------------------------------------

            // The TS_BITMAP_DATA structure wraps the bitmap data bytestream
            // for a screen area rectangle containing a clipping taken from
            // the server-side screen frame buffer.

            // A 16-bit, unsigned integer. Left bound of the rectangle.

            // A 16-bit, unsigned integer. Top bound of the rectangle.

            // A 16-bit, unsigned integer. Right bound of the rectangle.

            // A 16-bit, unsigned integer. Bottom bound of the rectangle.

            // A 16-bit, unsigned integer. The width of the rectangle.

            // A 16-bit, unsigned integer. The height of the rectangle.

            // A 16-bit, unsigned integer. The color depth of the rectangle
            // data in bits-per-pixel.

            // CGR: As far as I understand we should have
            // align4(right-left) == width and bottom-top == height
            // maybe put some assertion to check it's true
            // LOG(LOG_ERR, "left=%u top=%u right=%u bottom=%u width=%u height=%u bpp=%u", left, top, right, bottom, width, height, bpp);

            // A 16-bit, unsigned integer. The flags describing the format
            // of the bitmap data in the bitmapDataStream field.

            // +-----------------------------------+---------------------------+
            // | 0x0001 BITMAP_COMPRESSION         | Indicates that the bitmap |
            // |                                   | data is compressed. This  |
            // |                                   | implies that the          |
            // |                                   | bitmapComprHdr field is   |
            // |                                   | present if the NO_BITMAP_C|
            // |                                   |OMPRESSION_HDR (0x0400)    |
            // |                                   | flag is not set.          |
            // +-----------------------------------+---------------------------+
            // | 0x0400 NO_BITMAP_COMPRESSION_HDR  | Indicates that the        |
            // |                                   | bitmapComprHdr field is   |
            // |                                   | not present(removed for   |
            // |                                   | bandwidth efficiency to   |
            // |                                   | save 8 bytes).            |
            // +-----------------------------------+---------------------------+

            RDPBitmapData bmpdata;

            bmpdata.receive(stream);

            Rect boundary( bmpdata.dest_left
                           , bmpdata.dest_top
                           , bmpdata.dest_right - bmpdata.dest_left + 1
                           , bmpdata.dest_bottom - bmpdata.dest_top + 1
                           );

            // BITMAP_COMPRESSION 0x0001
            // Indicates that the bitmap data is compressed. This implies
            // that the bitmapComprHdr field is present if the
            // NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not set.

            if (this->verbose & 64) {
                LOG( LOG_INFO
                     , "/* Rect [%d] bpp=%d width=%d height=%d b(%d, %d, %d, %d) */"
                     , i
                     , bmpdata.bits_per_pixel
                     , bmpdata.width
                     , bmpdata.height
                     , boundary.x
                     , boundary.y
                     , boundary.cx
                     , boundary.cy
                     );
            }

            // bitmapComprHdr (8 bytes): Optional Compressed Data Header
            // structure (see Compressed Data Header (TS_CD_HEADER)
            // (section 2.2.9.1.1.3.1.2.3)) specifying the bitmap data
            // in the bitmapDataStream. This field MUST be present if
            // the BITMAP_COMPRESSION (0x0001) flag is present in the
            // Flags field, but the NO_BITMAP_COMPRESSION_HDR (0x0400)
            // flag is not.

            if (bmpdata.flags & BITMAP_COMPRESSION) {
                if ((bmpdata.width <= 0) || (bmpdata.height <= 0)) {
                    LOG( LOG_WARNING
                         , "Unexpected bitmap size: width=%d height=%d size=%u left=%u, top=%u, right=%u, bottom=%u"
                         , bmpdata.width
                         , bmpdata.height
                         , bmpdata.cb_comp_main_body_size
                         , bmpdata.dest_left
                         , bmpdata.dest_top
                         , bmpdata.dest_right
                         , bmpdata.dest_bottom
                         );
                }
            }

            TODO("CGR: check which sanity checks should be done");
                //            if (bufsize != bitmap.bmp_size){
                //                LOG(LOG_WARNING, "Unexpected bufsize in bitmap received [%u != %u] width=%u height=%u bpp=%u",
                //                    bufsize, bitmap.bmp_size, width, height, bpp);
                //            }
                const uint8_t * data = stream.in_uint8p(bmpdata.bitmap_size());
            Bitmap bitmap( this->bpp
                           , bmpdata.bits_per_pixel
                           , &this->orders.global_palette
                           , bmpdata.width
                           , bmpdata.height
                           , data
                           , bmpdata.bitmap_size()
                           , (bmpdata.flags & BITMAP_COMPRESSION)
                           );

            if (   bmpdata.cb_scan_width
                   && ((bmpdata.cb_scan_width - bitmap.line_size()) >= nbbytes(bitmap.bpp()))) {
                LOG( LOG_WARNING
                     , "Bad line size: line_size=%u width=%u height=%u bpp=%u"
                     , bmpdata.cb_scan_width
                     , bmpdata.width
                     , bmpdata.height
                     , bmpdata.bits_per_pixel
                     );
            }

            if (   bmpdata.cb_uncompressed_size
                   && (bmpdata.cb_uncompressed_size != bitmap.bmp_size())) {
                LOG( LOG_WARNING
                     , "final_size should be size of decompressed bitmap [%u != %u] width=%u height=%u bpp=%u"
                     , bmpdata.cb_uncompressed_size
                     , bitmap.bmp_size()
                     , bmpdata.width
                     , bmpdata.height
                     , bmpdata.bits_per_pixel
                     );
            }

            this->gd->draw(bmpdata, data, bmpdata.bitmap_size(), bitmap);
        }
        if (this->verbose & 64){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates done");
        }
    }   // process_bitmap_updates

    void send_client_info_pdu(int userid, const char * password) {
        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu");
        }
        HStream stream(1024, 2048);

        InfoPacket infoPacket( this->use_rdp5
                               , this->domain
                               , this->username
                               , password
                               , this->program
                               , this->directory
                               , this->performanceFlags
                               , this->clientAddr
                               );

        if (this->rdp_compression) {
            infoPacket.flags |= INFO_COMPRESSION;
            infoPacket.flags &= ~CompressionTypeMask;
            infoPacket.flags |= ((this->rdp_compression - 1) << 9);
        }

        if (this->remote_program) {
            infoPacket.flags |= INFO_RAIL;
        }

        if (this->verbose & 1) {
            infoPacket.log("Sending to server: ", this->password_printing_mode);
        }

        infoPacket.emit(stream);
        stream.mark_end();

        if (this->verbose & 1) {
            infoPacket.log("Preparing sec header ", this->password_printing_mode);
        }
        BStream sec_header(256);

        SEC::Sec_Send sec(sec_header, stream, SEC::SEC_INFO_PKT, this->encrypt, this->encryptionLevel);
        stream.copy_to_head(sec_header.get_data(), sec_header.size());

        if (this->verbose & 1) {
            infoPacket.log("Send data request", this->password_printing_mode);
        }
        this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, stream);

        if (this->open_session_timeout) {
            this->open_session_timeout_checker.restart_timeout(
                time(nullptr), this->open_session_timeout);
            this->event.set(1000000);
        }

        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu done");
        }
    }

    virtual void begin_update() {
        this->front.begin_update();
    }

    virtual void end_update() {
        this->front.end_update();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip,
                      const Bitmap & bmp) {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip,
                      const Bitmap & bmp) {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip,
                      const GlyphCache * gly_cache) {
        this->front.draw(cmd, clip, gly_cache);
    }

    virtual void draw(const RDPPolygonSC& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPPolygonCB& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }


    virtual void draw(const RDPPolyline& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB& cmd, const Rect & clip) {
        this->front.draw(cmd, clip);
    }

    virtual void server_set_pointer(const Pointer & cursor) {
        this->front.server_set_pointer(cursor);
    }

    virtual void draw(const RDPColCache & cmd) {
        this->front.draw(cmd);
    }

    virtual void draw(const RDP::FrameMarker & order) {
        this->front.draw(order);
    }

    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
                      size_t size, const Bitmap & bmp) {
        this->front.draw(bitmap_data, data, size, bmp);
    }

    virtual void draw(const RDPBrushCache & cmd) {
        this->front.draw(cmd);
    }

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) {
        this->front.draw(order);
    }

    virtual void draw(const RDP::RAIL::WindowIcon & order) {
        this->front.draw(order);
    }

    virtual void draw(const RDP::RAIL::CachedIcon & order) {
        this->front.draw(order);
    }

    virtual void draw(const RDP::RAIL::DeletedWindow & order) {
        this->front.draw(order);
    }

    virtual bool is_up_and_running() {
        return (UP_AND_RUNNING == this->connection_finalization_state);
    }

    virtual void disconnect() {
        if (this->is_up_and_running()) {
            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::disconnect()");
            }
            // this->send_shutdown_request();
            // this->draw_event(time(nullptr));
            this->send_disconnect_ultimatum();
        }
    }

    //void send_shutdown_request() {
    //    LOG(LOG_INFO, "SEND SHUTDOWN REQUEST PDU");
    //
    //    BStream stream(65536);
    //    ShareData sdata(stream);
    //    sdata.emit_begin(PDUTYPE2_SHUTDOWN_REQUEST, this->share_id,
    //                     RDP::STREAM_MED);
    //    sdata.emit_end();
    //    BStream sctrl_header(256);
    //    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU,
    //                      this->userid + GCC::MCS_USERCHANNEL_BASE,
    //                      stream.size());
    //    HStream target_stream(1024, 65536);
    //    target_stream.out_copy_bytes(sctrl_header);
    //    target_stream.out_copy_bytes(stream);
    //    target_stream.mark_end();
    //
    //    this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    //}

    void send_disconnect_ultimatum() {
        if (this->verbose & 1){
            LOG(LOG_INFO, "SEND MCS DISCONNECT PROVIDER ULTIMATUM PDU");
        }
        BStream x224_header(256);
        HStream mcs_data(256, 512);
        MCS::DisconnectProviderUltimatum_Send(mcs_data, 3, MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header,  mcs_data.size());
        this->nego.trans.send(x224_header, mcs_data);
    }

    //void send_flow_response_pdu(uint8_t flow_id, uint8_t flow_number) {
    //    LOG(LOG_INFO, "SEND FLOW RESPONSE PDU n° %u", flow_number);
    //    BStream flowpdu(256);
    //    FlowPDU_Send(flowpdu, FLOW_RESPONSE_PDU, flow_id, flow_number,
    //                 this->userid + GCC::MCS_USERCHANNEL_BASE);
    //    HStream target_stream(1024, 65536);
    //    target_stream.out_copy_bytes(flowpdu);
    //    target_stream.mark_end();
    //    this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    //}

    void process_auth_event(const CHANNELS::ChannelDef & auth_channel,
            Stream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        std::string auth_channel_message(char_ptr_cast(stream.p), stream.in_remain());

        LOG(LOG_INFO, "Auth channel data=\"%s\"", auth_channel_message.c_str());

        this->auth_channel_flags  = flags;
        this->auth_channel_chanid = auth_channel.chanid;

        if (this->acl) {
            this->acl->set_auth_channel_target(auth_channel_message.c_str());
        }
    }

    void process_wab_agent_event(const CHANNELS::ChannelDef & wab_agent_channel,
            Stream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        uint16_t message_length = stream.in_uint16_le();
        REDASSERT(message_length == stream.in_remain());
        (void)message_length; // disable -Wunused-variable if REDASSERT is disable
        std::string wab_agent_channel_message(char_ptr_cast(stream.p), stream.in_remain());

        while (wab_agent_channel_message.back() == '\0') wab_agent_channel_message.pop_back();

        LOG(LOG_INFO, "WAB agent channel data=\"%s\"", wab_agent_channel_message.c_str());

        const char * request_get_startup_application = "Request=Get startup application";

        if (!wab_agent_channel_message.compare(request_get_startup_application)) {
            //LOG(LOG_INFO, "<<<Get startup application>>>");
            BStream out_s(32768);

            const size_t message_length_offset = out_s.get_offset();
            out_s.out_clear_bytes(sizeof(uint16_t));

            out_s.out_string("StartupApplication=");
            if (this->real_alternate_shell.empty()) {
                out_s.out_string("[Windows Explorer]");
            }
            else {
                if (!this->real_working_dir.empty()) {
                    out_s.out_string(this->real_working_dir.c_str());
                }
                out_s.out_uint8('|');
                out_s.out_string(this->real_alternate_shell.c_str());
            }
            out_s.out_clear_bytes(1);   // Null character

            out_s.set_out_uint16_le(
                out_s.get_offset() - message_length_offset - sizeof(uint16_t),
                message_length_offset);

            out_s.mark_end();
            //hexdump(out_s.get_data(), out_s.size());

            this->send_to_channel(
                wab_agent_channel, out_s, out_s.size(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
            );
        }
    }

    void process_cliprdr_event(
            const CHANNELS::ChannelDef & cliprdr_channel, Stream & stream,
            uint32_t length, uint32_t flags, size_t chunk_size) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp server clipboard PDU");
        }

        const uint16_t msgType = stream.in_uint16_le();
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp server clipboard PDU: msgType=%d", msgType);
        }

        bool cencel_pdu = false;

        if (msgType == RDPECLIP::CB_FORMAT_LIST) {
            if (!this->authorization_channels.cliprdr_up_is_authorized() &&
                !this->authorization_channels.cliprdr_down_is_authorized()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp clipboard is fully disabled (s)");
                }

                // Build and send the CB_FORMAT_LIST_RESPONSE (with status = OK)
                // 03 00 01 00 00 00 00 00
                BStream out_s(256);
                const bool response_ok = true;
                RDPECLIP::FormatListResponsePDU(response_ok).emit(out_s);

                this->send_to_channel(
                    cliprdr_channel, out_s, out_s.size(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
                );

                cencel_pdu = true;
            }
        }
        else if (msgType == RDPECLIP::CB_FORMAT_DATA_REQUEST) {
            if (!this->authorization_channels.cliprdr_up_is_authorized()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp clipboard up is unavailable");
                }

                BStream out_s(256);
                RDPECLIP::FormatDataResponsePDU(false).emit(out_s, static_cast<uint8_t *>(nullptr), 0);

                this->send_to_channel(
                    cliprdr_channel, out_s, out_s.size(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
                );

                cencel_pdu = true;
            }
        }
        else if (msgType == RDPECLIP::CB_FILECONTENTS_REQUEST) {
            if (!this->authorization_channels.cliprdr_file_is_authorized()) {
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp requesting the contents of client file is denied");
                }

                BStream out_s(256);
                const bool response_ok = false;
                RDPECLIP::FileContentsResponse(response_ok).emit(out_s);

                this->send_to_channel(
                    cliprdr_channel, out_s, out_s.size(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
                );

                cencel_pdu = true;
            }
        }

        if (!cencel_pdu) {
            this->update_total_clipboard_data(msgType, length);
            stream.p -= 2;  // msgType(2)
            this->send_to_front_channel(
                cliprdr_channel.name, stream.p, length, chunk_size, flags
            );
        }
    }   // process_cliprdr_event

    void process_rail_event(const CHANNELS::ChannelDef & rail_channel,
            Stream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::process_rail_event: Server RAIL PDU.");
        }

        const auto saved_stream_p = stream.p;

        const uint16_t orderType   = stream.in_uint16_le();
        const uint16_t orderLength = stream.in_uint16_le();

        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::process_rail_event: orderType=%u orderLength=%u.",
                orderType, orderLength);
        }

        stream.p = saved_stream_p;

        this->send_to_front_channel(
            rail_channel.name, stream.p, length, chunk_size, flags
        );
    }

    void process_rdpdr_event(const CHANNELS::ChannelDef & rdpdr_channel,
            Stream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp::process_rdpdr_event: Server DR PDU.");
        }

        const auto saved_stream_p = stream.p;

        rdpdr::SharedHeader sh_r;

        sh_r.receive(stream);

        this->update_total_rdpdr_data(sh_r.packet_id, length);

        switch (sh_r.packet_id) {
            case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
            {
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Server Announce Request");
                }

                rdpdr::ServerAnnounceRequest server_announce_request;

                server_announce_request.receive(stream);

                if (this->verbose) {
                    server_announce_request.log(LOG_INFO);
                }

                if (this->proxy_managed_rdpdr_channel) {
                    REDASSERT(this->file_system_drive_manager.HasManagedDrive());

                    {
                        BStream out_stream(65535);

                        rdpdr::SharedHeader sr_e(rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
                        sr_e.emit(out_stream);

                        rdpdr::ClientAnnounceReply client_announce_reply(
                                0x0001, // VersionMajor, MUST be set to 0x0001.
                                0x0006, // Windows XP SP3.
                                // [MS-RDPEFS] - 3.2.5.1.3 Sending a Client Announce Reply Message.
                                ((server_announce_request.VersionMinor() >= 12) ?
                                 ::getpid() :
                                 server_announce_request.ClientId())
                            );
                        if (this->verbose) {
                            LOG(LOG_INFO, "mod_rdp::process_rdpdr_event");
                            client_announce_reply.log(LOG_INFO);
                        }
                        client_announce_reply.emit(out_stream);

                        out_stream.mark_end();

                        this->send_to_channel(rdpdr_channel, out_stream, out_stream.size(),
                                                CHANNELS::CHANNEL_FLAG_FIRST
                                              | CHANNELS::CHANNEL_FLAG_LAST
                                             );
                    }

                    {
                        BStream out_stream(65535);

                        rdpdr::SharedHeader sr_e(rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
                        sr_e.emit(out_stream);

                        rdpdr::ClientNameRequest client_name_nequest(
                            this->client_name);
                        if (this->verbose) {
                            LOG(LOG_INFO, "mod_rdp::process_rdpdr_event");
                            client_name_nequest.log(LOG_INFO);
                        }
                        client_name_nequest.emit(out_stream);

                        out_stream.mark_end();

                        this->send_to_channel(rdpdr_channel, out_stream, out_stream.size(),
                                                CHANNELS::CHANNEL_FLAG_FIRST
                                              | CHANNELS::CHANNEL_FLAG_LAST
                                             );
                    }
                }
            }
            break;

            case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Server Client ID Confirm");
                }

                if (this->proxy_managed_rdpdr_channel) {
                    REDASSERT(this->file_system_drive_manager.HasManagedDrive());

                    {
                        BStream out_stream(65535);

                        const rdpdr::SharedHeader sr_e(rdpdr::Component::RDPDR_CTYP_CORE,
                            rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY);
                        sr_e.emit(out_stream);

                        out_stream.out_uint16_le(5);    // 5 capabilities.
                        out_stream.out_clear_bytes(2);  // Padding(2)

                        // General capability set
                        const uint32_t general_capability_version =
                            rdpdr::GENERAL_CAPABILITY_VERSION_02;
                        out_stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
                        out_stream.out_uint16_le(
                                rdpdr::GeneralCapabilitySet::size(general_capability_version) +
                                8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
                            );
                        out_stream.out_uint32_le(general_capability_version);

                        rdpdr::GeneralCapabilitySet general_capability_set(
                                0x2,        // osType
                                0x50001,    // osVersion
                                0x1,        // protocolMajorVersion
                                0xC,        // protocolMinorVersion - RDP Client 6.0 and 6.1
                                0xFFFF,     // ioCode1
                                0x0,        // ioCode2
                                0x7,        // extendedPDU -
                                            //     RDPDR_DEVICE_REMOVE_PDUS(1) |
                                            //     RDPDR_CLIENT_DISPLAY_NAME_PDU(2) |
                                            //     RDPDR_USER_LOGGEDON_PDU(4)
                                0x0,        // extraFlags1
                                0x0,        // extraFlags2
                                0           // SpecialTypeDeviceCap
                            );
                        if (this->verbose) {
                            LOG(LOG_INFO, "mod_rdp::process_rdpdr_event");
                            general_capability_set.log(LOG_INFO);
                        }
                        general_capability_set.emit(out_stream, general_capability_version);

                        // Print capability set
                        out_stream.out_uint16_le(rdpdr::CAP_PRINTER_TYPE);
                        out_stream.out_uint16_le(
                                8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
                            );
                        out_stream.out_uint32_le(rdpdr::PRINT_CAPABILITY_VERSION_01);

                        // Port capability set
                        out_stream.out_uint16_le(rdpdr::CAP_PORT_TYPE);
                        out_stream.out_uint16_le(
                                8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
                            );
                        out_stream.out_uint32_le(rdpdr::PORT_CAPABILITY_VERSION_01);

                        // Drive capability set
                        out_stream.out_uint16_le(rdpdr::CAP_DRIVE_TYPE);
                        out_stream.out_uint16_le(
                                8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
                            );
                        out_stream.out_uint32_le(rdpdr::DRIVE_CAPABILITY_VERSION_01);

                        // Smart card capability set
                        out_stream.out_uint16_le(rdpdr::CAP_SMARTCARD_TYPE);
                        out_stream.out_uint16_le(
                                8   // CapabilityType(2) + CapabilityLength(2) + Version(4)
                            );
                        out_stream.out_uint32_le(rdpdr::DRIVE_CAPABILITY_VERSION_01);

                        out_stream.mark_end();

                        this->send_to_channel(rdpdr_channel, out_stream, out_stream.size(),
                                                CHANNELS::CHANNEL_FLAG_FIRST
                                              | CHANNELS::CHANNEL_FLAG_LAST
                                             );
                    }
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Server Device Announce Response");

                    rdpdr::ServerDeviceAnnounceResponse server_device_announce_response;

                    server_device_announce_response.receive(stream);
                    server_device_announce_response.log(LOG_INFO);
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
            {
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Device I/O Request");
                }

                rdpdr::DeviceIORequest device_io_request;

                device_io_request.receive(stream);
                if (this->verbose) {
                    device_io_request.log(LOG_INFO);
                }

                if (!this->file_system_drive_manager.IsManagedDrive(
                        device_io_request.DeviceId())) {
                    uint32_t extra_data = 0;

                    switch (device_io_request.MajorFunction()) {
                        case rdpdr::IRP_MJ_CREATE:
                        {
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_rdp::process_rdpdr_event: Device Create Request");
                            }

                            rdpdr::DeviceCreateRequest device_create_request;

                            device_create_request.receive(stream);

                            if (this->verbose) {
                                device_create_request.log(LOG_INFO);
                            }

                                  bool     access_ok     = true;
                            const uint32_t DesiredAccess = device_create_request.DesiredAccess();

                            if (!this->authorization_channels.rdpdr_drive_read_is_authorized() &&
                                smb2::read_access_is_required(DesiredAccess, /*strict_check = */false) &&
                                !(device_create_request.CreateOptions() & smb2::FILE_DIRECTORY_FILE) &&
                                ::strcmp(device_create_request.Path(), "/")) {
                                access_ok = false;
                            }
                            if (!this->authorization_channels.rdpdr_drive_write_is_authorized() &&
                                smb2::write_access_is_required(DesiredAccess)) {
                                access_ok = false;
                            }

                            if (!access_ok) {
                                BStream out_stream(65535);

                                const rdpdr::SharedHeader sh_e(rdpdr::Component::RDPDR_CTYP_CORE,
                                    rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);

                                sh_e.emit(out_stream);

                                const rdpdr::DeviceIOResponse device_io_response(
                                        device_io_request.DeviceId(),
                                        device_io_request.CompletionId(),
                                        0xC0000022  // STATUS_ACCESS_DENIED
                                    );

                                if (this->verbose) {
                                    device_io_response.log(LOG_INFO);
                                }
                                device_io_response.emit(out_stream);

                                const rdpdr::DeviceCreateResponse device_create_response(
                                    static_cast<uint32_t>(-1), 0);
                                if (this->verbose) {
                                    device_create_response.log(LOG_INFO);
                                }
                                device_create_response.emit(out_stream);

                                out_stream.mark_end();

                                this->send_to_channel(rdpdr_channel, out_stream,
                                                      out_stream.size(),
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                      | CHANNELS::CHANNEL_FLAG_LAST
                                    );

                                return;
                            }
                        }
                        break;

                        case rdpdr::IRP_MJ_CLOSE:
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_rdp::process_rdpdr_event: Device Close Request");
                            }
                        break;

                        case rdpdr::IRP_MJ_DEVICE_CONTROL:
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_rdp::process_rdpdr_event: Device control request");

                                rdpdr::DeviceControlRequest device_control_request;

                                device_control_request.receive(stream);
                                device_control_request.log(LOG_INFO);
                            }
                        break;

                        case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_rdp::process_rdpdr_event: Query volume information request");

                                rdpdr::ServerDriveQueryVolumeInformationRequest
                                    server_drive_query_volume_information_request;

                                server_drive_query_volume_information_request.receive(stream);
                                server_drive_query_volume_information_request.log(LOG_INFO);

                                extra_data =
                                    server_drive_query_volume_information_request.FsInformationClass();
                            }
                        break;

                        case rdpdr::IRP_MJ_QUERY_INFORMATION:
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_rdp::process_rdpdr_event: "
                                        "Server Drive Query Information Request");

                                rdpdr::ServerDriveQueryInformationRequest
                                    server_drive_query_information_request;

                                server_drive_query_information_request.receive(stream);
                                server_drive_query_information_request.log(LOG_INFO);

                                extra_data =
                                    server_drive_query_information_request.FsInformationClass();
                            }
                        break;

                        case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                            if (device_io_request.MinorFunction() ==
                                rdpdr::IRP_MN_QUERY_DIRECTORY) {
                                if (this->verbose) {
                                    LOG(LOG_INFO,
                                        "mod_rdp::process_rdpdr_event: "
                                            "Server Drive Query Directory Request");

                                    rdpdr::ServerDriveQueryDirectoryRequest
                                        server_drive_query_directory_request;

                                    server_drive_query_directory_request.receive(stream);
                                    server_drive_query_directory_request.log(LOG_INFO);

                                    extra_data =
                                        server_drive_query_directory_request.FsInformationClass();
                                }
                            }
                        break;

                        default:
                            if (this->verbose) {
                                LOG(LOG_INFO,
                                    "mod_rdp::process_rdpdr_event: "
                                        "undecoded Device I/O Request - MajorFunction=0x%X MinorFunction=0x%X",
                                    device_io_request.MajorFunction(),
                                    device_io_request.MinorFunction());
                            }
                        break;
                    }

                    if (this->verbose) {
                        this->device_io_requests.push_back(std::make_tuple(
                            device_io_request.DeviceId(),
                            device_io_request.CompletionId(),
                            device_io_request.MajorFunction(),
                            extra_data
                            ));
                    }
                }
                else {
                    if (device_io_request.MajorFunction() == rdpdr::IRP_MJ_READ) {
                        const auto saved_mj_read_stream_p = stream.p;

                        rdpdr::DeviceReadRequest device_read_request;
                        device_read_request.receive(stream);

                        stream.p = saved_mj_read_stream_p;

                        this->adjust_chunked_virtual_channel_data_stream_size(
                                20 +                            // DeviceIoReply(16) + Length(4)
                                device_read_request.Length()
                            );
                    }

                    this->chunked_virtual_channel_data_stream.reset();

                    uint32_t out_flags = 0;

                    this->file_system_drive_manager.ProcessDeviceIORequest(
                        device_io_request, stream, this->chunked_virtual_channel_data_stream,
                        out_flags, this->verbose);
                    if (this->chunked_virtual_channel_data_stream.size()) {
                        this->send_to_channel(rdpdr_channel,
                                              this->chunked_virtual_channel_data_stream,
                                              this->chunked_virtual_channel_data_stream.size(),
                                              out_flags);
                    }

                    this->chunked_virtual_channel_data_stream.reset();

                    return;
                }
            }
            break;

            case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Server Core Capability Request");
                }
            break;

            case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Server User Logged On");
                }

                this->client_device_list_announce_timeout_checker.restart_timeout(
                    TimeVal(), this->client_device_list_announce_timeout);
                REDASSERT(!this->client_device_list_announce_timer_enabled);
                this->client_device_list_announce_timer_enabled = true;
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Client Device List Announce timer is enabled.");
                }

                this->event.object_and_time = true;
                this->event.set(this->client_device_list_announce_timeout);
            break;

            case rdpdr::PacketId::PAKID_PRN_USING_XPS:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: Server Printer Set XPS Mode");
                }
            break;

            default:
                if (this->verbose) {
                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: undecoded PDU - Component=0x%X PacketId=0x%X",
                        static_cast<uint16_t>(sh_r.component), static_cast<uint16_t>(sh_r.packet_id));
                }
            break;
        }

        if (!this->proxy_managed_rdpdr_channel) {
            stream.p = saved_stream_p;

            this->send_to_front_channel(
                rdpdr_channel.name, stream.p, length, chunk_size, flags
            );
        }
    }
};

#endif
