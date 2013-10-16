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
#define _REDEMPTION_MOD_RDP_RDP_HPP__

#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <math.h>

#include "rdp/rdp_orders.hpp"

/* include "ther h files */
#include "stream.hpp"
#include "ssl_calls.hpp"
#include "mod_api.hpp"
#include "auth_api.hpp"

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
#include "RDP/bitmapupdate.hpp"
#include "RDP/clipboard.hpp"
#include "RDP/fastpath.hpp"
#include "RDP/protocol.hpp"
#include "RDP/RefreshRectPDU.hpp"
#include "RDP/SaveSessionInfoPDU.hpp"
#include "RDP/pointer.hpp"

#include "genrandom.hpp"

struct mod_rdp : public mod_api {
    FrontAPI & front;

    CHANNELS::ChannelDefArray mod_channel_list;

    int  use_rdp5;

    int  keylayout;

    uint8_t   lic_layer_license_key[16];
    uint8_t   lic_layer_license_sign_key[16];
    uint8_t * lic_layer_license_data;
    size_t    lic_layer_license_size;

    rdp_orders orders;

    int      share_id;
    uint16_t userid;

    int bitmap_compression;

    int version;

    char hostname[16];
    char username[128];
    char password[256];
    char domain[256];
    char program[512];
    char directory[512];

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
    const int brush_cache_code;
    const uint8_t front_bpp;
    const uint32_t performanceFlags;
    Random * gen;
    uint32_t verbose;

    char auth_channel[8];
    int  auth_channel_flags;
    int  auth_channel_chanid;
    int  auth_channel_state;    // 0 means unused, 1 means session running

    auth_api * acl;

    RdpNego nego;

    char clientAddr[512];

    bool enable_bitmap_update;
    bool enable_clipboard;                   // true clipboard available, false clipboard unavailable
    bool enable_fastpath;                    // choice of programmer
    bool enable_fastpath_client_input_event; // choice of programmer + capability of server
    bool enable_fastpath_server_update;      // = choice of programmer
    bool enable_mem3blt;
    bool enable_new_pointer;
    bool enable_rdp_bulk_compression;
    bool enable_transparent_mode;

    size_t recv_bmp_update;

    rdp_mppc_unified_dec mppc_dec;

    redemption::string * error_message;

    bool     disconnect_on_logon_user_change;
    uint32_t open_session_timeout;

    Timeout  open_session_timeout_checker;

    redemption::string output_filename;

    redemption::string end_session_reason;
    redemption::string end_session_message;

    unsigned certificate_change_action;

    mod_rdp( Transport * trans
           , const char * target_user
           , const char * target_password
           , const char * client_address
           , struct FrontAPI & front
           , const bool enable_tls
           , const ClientInfo & info
           , Random * gen
           , int key_flags
           , auth_api * acl
           , const char * auth_channel
           , const char * alternate_shell
           , const char * shell_working_directory
           , bool enable_clipboard
           , bool enable_fastpath    // If true, fast-path must be supported
           , bool enable_mem3blt
           , bool enable_bitmap_update
           , uint32_t verbose = 0
           , bool enable_new_pointer = false
           , bool enable_rdp_bulk_compression = false
           , redemption::string * error_message = NULL
           , bool disconnect_on_logon_user_change = false
           , uint32_t open_session_timeout = 0
           , unsigned certificate_change_action = 0
           , bool enable_transparent_mode = false
           , const char * output_filename = ""
           )
        : mod_api(info.width, info.height)
        , front(front)
        , use_rdp5(1)
        , keylayout(info.keylayout)
        , orders(0)
        , share_id(0)
        , userid(0)
        , bitmap_compression(1)
        , version(0)
        , bpp(0)
        , encryptionLevel(0)
        , key_flags(key_flags)
        , server_public_key_len(0)
        , connection_finalization_state(EARLY)
        , state(MOD_RDP_NEGO)
        , console_session(info.console_session)
        , brush_cache_code(info.brush_cache_code)
        , front_bpp(info.bpp)
        , performanceFlags(info.rdp5_performanceflags)
        , gen(gen)
        , verbose(verbose)
        , auth_channel_flags(0)
        , auth_channel_chanid(0)
        , auth_channel_state(0) // 0 means unused
        , acl(acl)
        , nego(enable_tls, trans, target_user)
        , enable_bitmap_update(enable_bitmap_update)
        , enable_clipboard(enable_clipboard)
        , enable_fastpath(enable_fastpath)
        , enable_fastpath_client_input_event(false)
        , enable_fastpath_server_update(enable_fastpath)
        , enable_mem3blt(enable_mem3blt)
        , enable_new_pointer(enable_new_pointer)
        , enable_rdp_bulk_compression(enable_rdp_bulk_compression)
        , enable_transparent_mode(enable_transparent_mode)
        , recv_bmp_update(0)
        , error_message(error_message)
        , disconnect_on_logon_user_change(disconnect_on_logon_user_change)
        , open_session_timeout(open_session_timeout)
        , open_session_timeout_checker(0)
        , output_filename(output_filename)
        , certificate_change_action(certificate_change_action)
    {
        if (this->verbose & 1)
        {
            if (!enable_transparent_mode)
            {
                LOG(LOG_INFO, "Creation of new mod 'RDP'");
            }
            else
            {
                LOG(LOG_INFO, "Creation of new mod 'RDP Transparent'");

                if (this->output_filename.is_empty())
                {
                    LOG(LOG_INFO, "Use transparent capabilities.");
                }
                else
                {
                    LOG(LOG_INFO, "Use proxy default capabilities.");
                }
            }
        }

        this->event.object_and_time = (this->open_session_timeout > 0);

        memset(this->auth_channel, 0, sizeof(this->auth_channel));
        strncpy(this->auth_channel, auth_channel, sizeof(this->auth_channel) - 1);

        memset(this->clientAddr, 0, sizeof(this->clientAddr));
        strncpy(this->clientAddr, client_address, sizeof(this->clientAddr) - 1);

        this->lic_layer_license_data = 0;
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

        TODO("CGR: and if hostname is really larger  what happens ? We should at least emit a warning log");
        if (::strlen(info.hostname) >= sizeof(this->hostname)) {
            LOG(LOG_INFO, "mod_rdp: hostname too long! %u > %u", ::strlen(info.hostname), sizeof(this->hostname));
        }
        strncpy(this->hostname, info.hostname, 15);
        this->hostname[15] = 0;

        TODO("CGR: and if username is really larger  what happens ? We should at least emit a warning log");
        if (::strlen(target_user) >= sizeof(this->username)) {
            LOG(LOG_INFO, "mod_rdp: username too long! %u > %u", ::strlen(target_user), sizeof(this->username));
        }
        strncpy(this->username, target_user, 127);
        this->username[127] = 0;

        LOG(LOG_INFO, "Remote RDP Server login:%s host:%s", this->username, this->hostname);

        strncpy(this->password, target_password, sizeof(this->password) - 1);
        this->password[sizeof(this->password) - 1] = 0;

        memset(this->domain, 0, sizeof(this->domain));

        strncpy(this->program, alternate_shell, sizeof(this->program) - 1);
        this->program[sizeof(this->program) - 1] = 0;
        strncpy(this->directory, shell_working_directory, sizeof(this->directory) - 1);
        this->directory[sizeof(this->directory) - 1] = 0;

        LOG(LOG_INFO, "Server key layout is %x", this->keylayout);

        while (UP_AND_RUNNING != this->connection_finalization_state){
            this->draw_event(time(NULL));
            if (this->event.signal != BACK_EVENT_NONE){
                LOG(LOG_INFO, "Creation of new mod 'RDP' failed");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }

        if (this->acl)
        {
            this->acl->report("CONNECTION_SUCCESSFUL", "Ok.");
        }

        this->end_session_reason.copy_c_str("OPEN_SESSION_FAILED");
        this->end_session_message.copy_c_str("Open RDP session cancelled.");
    }

    virtual ~mod_rdp()
    {
        if (this->acl && !this->end_session_reason.is_empty() &&
            !this->end_session_message.is_empty()) {
            this->acl->report(this->end_session_reason.c_str(),
                this->end_session_message.c_str());
        }

        if (this->lic_layer_license_data) {
            free(this->lic_layer_license_data);
        }

        if (this->verbose) {
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp cache count  = %llu",
                this->orders.recv_bmp_cache_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv order count      = %llu",
                this->orders.recv_order_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp update count = %llu",
                this->recv_bmp_update);
        }
    }

    virtual void rdp_input_scancode( long param1, long param2, long device_flags, long time
                                     , Keymap2 * keymap) {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
//            LOG(LOG_INFO, "Direct parameter transmission");

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
        const CHANNELS::ChannelDef * front_channel =
            this->front.get_channel_list().get_by_name(mod_channel_name);
        if (front_channel) {
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
    }

    virtual void send_to_mod_channel( const char * const front_channel_name
                                    , Stream & chunk
                                    , size_t length
                                    , uint32_t flags) {
        if (this->verbose & 16) {
            LOG(LOG_INFO, "mod_rdp::send_to_mod_channel");
            LOG(LOG_INFO, "sending to channel %s", front_channel_name);
        }

        // Clipboard is unavailable and is a Clipboard PDU
        if (!this->enable_clipboard && !::strcmp(front_channel_name, CLIPBOARD_VIRTUAL_CHANNEL_NAME)) {
            if (this->verbose) {
                LOG(LOG_INFO, "mod_rdp clipboard PDU");
            }

            if (!chunk.in_check_rem(2)) {
                LOG(LOG_INFO, "mod_vnc::send_to_mod_channel truncated msgType, need=2 remains=%u",
                    chunk.in_remain());
                throw Error(ERR_VNC);
            }

            uint16_t msgType = chunk.in_uint16_le();

            if (msgType == RDPECLIP::CB_FORMAT_LIST) {
                if (this->verbose) {
                    LOG(LOG_INFO, "mod_rdp clipboard is unavailable");
                }

                bool response_ok = false;

                // Build and send the CB_FORMAT_LIST_RESPONSE (with status = FAILED)
                // 03 00 02 00 00 00 00 00
                RDPECLIP::FormatListResponsePDU format_list_response_pdu(response_ok);
                BStream                         out_s(256);

                format_list_response_pdu.emit(out_s);

                size_t length     = out_s.size();
                size_t chunk_size = length;

                this->send_to_front_channel( (char *)CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                             , out_s.get_data()
                                             , length
                                             , chunk_size
                                             , CHANNELS::CHANNEL_FLAG_FIRST
                                             | CHANNELS::CHANNEL_FLAG_LAST
                                             );

                return;
            }
        }

        const CHANNELS::ChannelDef * mod_channel =
            this->mod_channel_list.get_by_name(front_channel_name);
        // send it if module has a matching channel, if no matching channel is found just forget it
        if (mod_channel) {
            if (this->verbose & 16) {
                int index = this->mod_channel_list.get_index_by_name(front_channel_name);
                mod_channel->log(index);
            }
            this->send_to_channel(*mod_channel, chunk, length, flags);
        }

        if (this->verbose & 16) {
            LOG(LOG_INFO, "mod_rdp::send_to_mod_channel done");
        }
    }

    // Method used by session to transmit sesman answer for auth_channel
    virtual void send_auth_channel_data(const char * data) {
        if (strncmp("Error:", data, 6)) {
            this->auth_channel_state = 1; // session started
        }

        CHANNELS::VirtualChannelPDU virtual_channel_pdu;
        StaticStream                chunk(data, ::strlen(data));

        virtual_channel_pdu.send_to_server( *this->nego.trans, this->encrypt, this->encryptionLevel
                            , this->userid, this->auth_channel_chanid, chunk.size()
                            , this->auth_channel_flags, chunk);
    }

    void send_to_channel( const CHANNELS::ChannelDef & channel, Stream & chunk, size_t length
                          , uint32_t flags) {
        if (this->verbose & 16) {
            LOG( LOG_INFO, "mod_rdp::send_to_channel length=%u chunk_size=%u", static_cast<unsigned>(length)
                 , (unsigned)chunk.size());
            channel.log(-1);
        }

        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        }

        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        virtual_channel_pdu.send_to_server( *this->nego.trans, this->encrypt, this->encryptionLevel
                                            , this->userid, channel.chanid, length, flags, chunk);

        if (this->verbose & 16) {
            LOG(LOG_INFO, "mod_rdp::send_to_channel done");
        }
    }

    void send_data_request(uint16_t channelId, HStream & stream)
    {
        BStream x224_header(256);
        BStream mcs_header(256);

        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channelId, 1,
                                      3, stream.size(), MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans->send(x224_header, mcs_header, stream);
    }

    void send_data_request_ex(uint16_t channelId, HStream & stream)
    {
        BStream x224_header(256);
        BStream mcs_header(256);
        BStream sec_header(256);

        SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt,
                          this->encryptionLevel);
        stream.copy_to_head(sec_header);

        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channelId, 1,
                                      3, stream.size(), MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans->send(x224_header, mcs_header, stream);
    }

    virtual void draw_event(time_t now)
    {
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
                            cs_core.highColorDepth = this->front_bpp;
                            cs_core.keyboardLayout = this->keylayout;
                            uint16_t hostlen = strlen(hostname);
                            uint16_t maxhostlen = std::min((uint16_t)15, hostlen);
                            for (size_t i = 0; i < maxhostlen ; i++){
                                cs_core.clientName[i] = hostname[i];
                            }
                            bzero(&(cs_core.clientName[hostlen]), 16-hostlen);
                            if (this->nego.tls){
                                cs_core.serverSelectedProtocol = 1;
                            }
                            if (this->verbose) {
                                cs_core.log("Sending to Server");
                            }
                            if (this->nego.tls){
                            }
                            cs_core.emit(stream);

                            // ------------------------------------------------------------
                            GCC::UserData::CSCluster cs_cluster;
                            TODO("CGR: values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?");

                                if (!this->nego.tls){
                                    if (this->console_session){
                                        cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID | (3 << 2) ; // REDIRECTION V4
                                    }
                                    else {
                                        cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED            | (2 << 2) ; // REDIRECTION V3
                                    }
                                }
                                else {
                                    cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED * ((3 << 2)|1);  // REDIRECTION V4
                                    if (this->console_session){
                                        cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID ;
                                    }
                                }
                            if (this->verbose) {
                                cs_cluster.log("Sending to server");
                            }
                            cs_cluster.emit(stream);
                            // ------------------------------------------------------------

                            GCC::UserData::CSSecurity cs_security;
                            if (this->verbose) {
                                cs_security.log("Sending to server");
                            }
                            cs_security.emit(stream);
                            // ------------------------------------------------------------

                            const CHANNELS::ChannelDefArray & channel_list = this->front.get_channel_list();
                            size_t num_channels = channel_list.size();
                            if ((num_channels > 0) || this->auth_channel[0]) {
                                /* Here we need to put channel information in order to redirect channel data
                                   from client to server passing through the "proxy" */
                                GCC::UserData::CSNet cs_net;
                                cs_net.channelCount = num_channels;
                                for (size_t index = 0; index < num_channels; index++){
                                    const CHANNELS::ChannelDef & channel_item = channel_list[index];
                                    memcpy(cs_net.channelDefArray[index].name, channel_list[index].name, 8);
                                    cs_net.channelDefArray[index].options = channel_item.flags;
                                    CHANNELS::ChannelDef def;
                                    memcpy(def.name, cs_net.channelDefArray[index].name, 8);
                                    def.flags = channel_item.flags;
                                    if (this->verbose & 16){
                                        def.log(index);
                                    }
                                    this->mod_channel_list.push_back(def);
                                }

                                // Inject a new channel for auth_channel virtual channel (wablauncher)
                                if (this->auth_channel[0]) {
                                    memcpy(cs_net.channelDefArray[num_channels].name, this->auth_channel, 8);
                                    cs_net.channelDefArray[num_channels].options =
                                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED/*
                                        | GCC::UserData::CSNet::CHANNEL_OPTION_ENCRYPT_RDP
                                        | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                                        | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL*/;
                                    cs_net.channelCount++;
                                    CHANNELS::ChannelDef def;
                                    memcpy(def.name, this->auth_channel, 8);
                                    def.flags = cs_net.channelDefArray[num_channels].options;
                                    if (this->verbose & 16){
                                        def.log(num_channels);
                                    }
                                    this->mod_channel_list.push_back(def);
                                }

                                if (this->verbose) {
                                    cs_net.log("Sending to server");
                                }
                                cs_net.emit(stream);
                            }
                            // ------------------------------------------------------------

                            BStream gcc_header(65536);
                            GCC::Create_Request_Send(gcc_header, stream.size());

                            BStream mcs_header(65536);
                            MCS::CONNECT_INITIAL_Send mcs(mcs_header, gcc_header.size() + stream.size(), MCS::BER_ENCODING);

                            BStream x224_header(256);
                            X224::DT_TPDU_Send(x224_header, mcs_header.size() + gcc_header.size() + stream.size());

                            this->nego.trans->send(x224_header, mcs_header, gcc_header, stream);

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
                        BStream x224_data(65536);
                        X224::RecvFactory f(*this->nego.trans, x224_data);
                        X224::DT_TPDU_Recv x224(*this->nego.trans, x224_data);

                        SubStream & mcs_data = x224.payload;
                        MCS::CONNECT_RESPONSE_PDU_Recv mcs(mcs_data, MCS::BER_ENCODING);

                        GCC::Create_Response_Recv gcc_cr(mcs.payload);

                        while (gcc_cr.payload.in_check_rem(4)) {

                            GCC::UserData::RecvFactory f(gcc_cr.payload);
                            switch (f.tag) {
                            case SC_CORE:
                                {
                                    GCC::UserData::SCCore sc_core;
                                    sc_core.recv(f.payload);
                                    if (this->verbose) {
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
                                    if (this->verbose) {
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
                                            if (NULL == epk){
                                                LOG(LOG_WARNING, "Failed to extract public key from certificate\n");
                                                throw Error(ERR_SEC);
                                            }

                                            RSA * server_public_key = RSAPublicKey_dup((RSA *) epk->pkey.ptr);
                                            EVP_PKEY_free(epk);
                                            this->server_public_key_len = RSA_size(server_public_key);

                                            if (NULL == server_public_key){
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
                                        this->gen->random(client_random, SEC_RANDOM_SIZE);

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
                                    sc_net.recv(f.payload);

                                    /* We assume that the channel_id array is confirmed in the same order
                                       that it has been sent. If there are any channels not confirmed, they're
                                       going to be the last channels on the array sent in MCS Connect Initial */
                                    if (this->verbose & 16){
                                        LOG(LOG_INFO, "server_channels_count=%u sent_channels_count=%u",
                                            sc_net.channelCount,
                                            mod_channel_list.channelCount);
                                    }
                                    for (uint32_t index = 0; index < sc_net.channelCount; index++) {
                                        if (this->verbose & 16){
                                            this->mod_channel_list.items[index].log(index);
                                        }
                                        this->mod_channel_list.set_chanid(index, sc_net.channelDefArray[index].id);
                                    }
                                    if (this->verbose) {
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
                        HStream mcs_data(256, 512);

                        MCS::ErectDomainRequest_Send mcs(mcs_data, 0, 0, MCS::PER_ENCODING);
                        X224::DT_TPDU_Send(x224_header, mcs_data.size());
                        this->nego.trans->send(x224_header, mcs_data);
                    }
                    if (this->verbose & 1){
                        LOG(LOG_INFO, "Send MCS::AttachUserRequest");
                    }
                    {
                        BStream x224_header(256);
                        HStream mcs_data(256, 512);

                        MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);

                        X224::DT_TPDU_Send(x224_header, mcs_data.size());
                        this->nego.trans->send(x224_header, mcs_data);
                    }
                    this->state = MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER;
                    break;

                case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
                    if (this->verbose & 1){
                        LOG(LOG_INFO, "mod_rdp::Channel Connection Attach User");
                    }
                    {
                        {
                            BStream stream(65536);
                            X224::RecvFactory f(*this->nego.trans, stream);
                            X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
                            SubStream & payload = x224.payload;

                            MCS::AttachUserConfirm_Recv mcs(payload, MCS::PER_ENCODING);
                            if (mcs.initiator_flag){
                                this->userid = mcs.initiator;
                            }
                        }

                        {
                            size_t num_channels = this->mod_channel_list.size();
                            uint16_t channels_id[CHANNELS::MAX_STATIC_VIRTUAL_CHANNELS];
                            channels_id[0] = this->userid + GCC::MCS_USERCHANNEL_BASE;
                            channels_id[1] = GCC::MCS_GLOBAL_CHANNEL;
                            for (size_t index = 0; index < num_channels; index++){
                                channels_id[index+2] = this->mod_channel_list[index].chanid;
                            }

                            for (size_t index = 0; index < num_channels+2; index++){
                                BStream x224_header(256);
                                HStream mcs_cjrq_data(256, 512);
                                if (this->verbose & 16){
                                    LOG(LOG_INFO, "cjrq[%u] = %u", index, channels_id[index]);
                                }
                                MCS::ChannelJoinRequest_Send(mcs_cjrq_data, this->userid, channels_id[index], MCS::PER_ENCODING);
                                X224::DT_TPDU_Send(x224_header, mcs_cjrq_data.size());
                                this->nego.trans->send(x224_header, mcs_cjrq_data);

                                BStream x224_data(256);
                                X224::RecvFactory f(*this->nego.trans, x224_data);
                                X224::DT_TPDU_Recv x224(*this->nego.trans, x224_data);
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
                        const char * username = this->username;
                        // read tpktHeader (4 bytes = 3 0 len)
                        // TPDU class 0    (3 bytes = LI F0 PDU_DT)

                        BStream stream(65536);
                        X224::RecvFactory f(*this->nego.trans, stream);
                        X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
                        SubStream & mcs_data = x224.payload;
                        MCS::SendDataIndication_Recv mcs(mcs_data, MCS::PER_ENCODING);

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

                                        ssllib ssl;
                                        /* Generate a signature for the HWID buffer */
                                        uint8_t signature[LIC::LICENSE_SIGNATURE_SIZE];

                                        FixedSizeStream sig(signature, sizeof(signature));
                                        FixedSizeStream key(this->lic_layer_license_sign_key, sizeof(this->lic_layer_license_sign_key));
                                        FixedSizeStream data(hwid, sizeof(hwid));

                                        ssl.sign(sig, key, data);
                                        /* Now encrypt the HWID */

                                        SslRC4 rc4;
                                        rc4.set_key(FixedSizeStream(this->lic_layer_license_key, 16));

                                        FixedSizeStream hwid_stream(hwid, sizeof(hwid));
                                        rc4.crypt(hwid_stream);

                                        LIC::ClientLicenseInfo_Send(lic_data, this->use_rdp5?3:2,
                                                                    this->lic_layer_license_size, this->lic_layer_license_data, hwid, signature);
                                    }
                                    else {
                                        LIC::NewLicenseRequest_Send(lic_data, this->use_rdp5?3:2, username, hostname);
                                    }

                                    SEC::Sec_Send sec(sec_header, lic_data,
                                        SEC::SEC_LICENSE_PKT, this->encrypt, 0);
                                    lic_data.copy_to_head(sec_header);

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
                                    rc4_decrypt_token.set_key(FixedSizeStream(this->lic_layer_license_key, 16));
                                    FixedSizeStream decrypt_token_stream(decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                                    rc4_decrypt_token.crypt(decrypt_token_stream);

                                    /* Generate a signature for a buffer of token and HWID */
                                    buf_out_uint32(hwid, 2);
                                    memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                                    uint8_t sealed_buffer[LIC::LICENSE_TOKEN_SIZE + LIC::LICENSE_HWID_SIZE];
                                    memcpy(sealed_buffer, decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                                    memcpy(sealed_buffer + LIC::LICENSE_TOKEN_SIZE, hwid, LIC::LICENSE_HWID_SIZE);

                                    ssllib ssl;

                                    FixedSizeStream sig(out_sig, sizeof(out_sig));
                                    FixedSizeStream key(this->lic_layer_license_sign_key, sizeof(this->lic_layer_license_sign_key));
                                    FixedSizeStream data(sealed_buffer, sizeof(sealed_buffer));

                                    ssl.sign(sig, key, data);

                                    /* Now encrypt the HWID */
                                    memcpy(crypt_hwid, hwid, LIC::LICENSE_HWID_SIZE);
                                    SslRC4 rc4_hwid;
                                    rc4_hwid.set_key(FixedSizeStream(this->lic_layer_license_key, 16));
                                    FixedSizeStream crypt_hwid_stream(crypt_hwid, LIC::LICENSE_HWID_SIZE);
                                    rc4_hwid.crypt(crypt_hwid_stream);

                                    BStream sec_header(256);
                                    HStream lic_data(1024, 65535);

                                    LIC::ClientPlatformChallengeResponse_Send(lic_data, this->use_rdp5?3:2, out_token, crypt_hwid, out_sig);
                                    SEC::Sec_Send sec(sec_header, lic_data,
                                        SEC::SEC_LICENSE_PKT, this->encrypt, 0);
                                    lic_data.copy_to_head(sec_header);
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
                            LOG(LOG_ERR, "Failed to get expected license negotiation PDU");
                            hexdump(x224.payload.get_data(), x224.payload.size());
                            //                throw Error(ERR_SEC);
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

                        BStream stream(65536);

                        // Detect fast-path PDU
                        X224::RecvFactory f( *this->nego.trans
                                           , stream
                                           , true               /* Support Fast-Path. */
                                           );

                        if (f.fast_path) {
                            FastPath::ServerUpdatePDU_Recv su(*this->nego.trans, stream, this->decrypt);
                            while (su.payload.in_remain()) {
                                FastPath::Update_Recv upd(su.payload, &this->mppc_dec);

                                switch (upd.updateCode) {
                                case FastPath::FASTPATH_UPDATETYPE_ORDERS:
                                    this->front.begin_update();
                                    this->orders.process_orders(this->bpp, upd.payload, true, this);
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
                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_SYNCHRONIZE, not yet supported"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PTR_NULL:
                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PTR_NULL, not yet supported"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PTR_DEFAULT:
                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PTR_DEFAULT, not yet supported"); }
                                    break;

                                case FastPath::FASTPATH_UPDATETYPE_PTR_POSITION:
                                    if (this->verbose & 8) { LOG(LOG_INFO, "FASTPATH_UPDATETYPE_PTR_POSITION, not yet supported"); }
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
                                    break;
                                }
                            }
                            break;
                        }

                        X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
                        SubStream & mcs_data = x224.payload;
                        MCS::SendDataIndication_Recv mcs(mcs_data, MCS::PER_ENCODING);

                        if (mcs.type == MCS::MCSPDU_DisconnectProviderUltimatum){
                            LOG(LOG_ERR, "mod_rdp: got MCS DisconnectProviderUltimatum");
                            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
                        }

                        SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);

                        if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL){
                            if (this->verbose & 16){
                                LOG(LOG_INFO, "received channel data on mcs.chanid=%u", mcs.channelId);
                            }

                            int num_channel_src =
                                this->mod_channel_list.get_index_by_id(mcs.channelId);
                            if (num_channel_src == -1) {
                                LOG(LOG_WARNING, "mod::rdp::MOD_RDP_CONNECTED::Unknown Channel id=%d", mcs.channelId);
                                throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                            }

                            const CHANNELS::ChannelDef & mod_channel = this->mod_channel_list[num_channel_src];
                            if (this->verbose & 16){
                                mod_channel.log(num_channel_src);
                            }

                            uint32_t length = sec.payload.in_uint32_le();
                            int flags = sec.payload.in_uint32_le();
                            size_t chunk_size = sec.payload.in_remain();

                            // If channel name is our virtual channel, then don't send data to front
                            if (this->auth_channel[0] /*&& this->acl */&& !strcmp(mod_channel.name, this->auth_channel)){
                                const char * auth_channel_message = (const char *)sec.payload.p;
                                if (this->auth_channel_state == 0) {
                                    this->auth_channel_flags = flags;
                                    this->auth_channel_chanid = mod_channel.chanid;
                                    if (strncmp("target:", auth_channel_message, 7)){
                                        LOG(LOG_ERR, "Invalid request (%s)", auth_channel_message);
                                        this->send_auth_channel_data("Error: Invalid request");
                                    }
                                    else if (this->program[0]) {
    //                                    LOG(LOG_ERR, "WabLauncher send program (%s)", this->program);
                                        this->send_auth_channel_data(this->program);
                                    }
                                    else if (this->acl) {
                                        // Ask sesman for requested target
                                        this->acl->set_auth_channel_target(auth_channel_message + 7);
                                    }
                                }
                                else if (this->auth_channel_state == 1){
                                    if (strncmp("result:", auth_channel_message, 7)){
                                        LOG(LOG_ERR, "Invalid result (%s)", auth_channel_message);
                                        auth_channel_message = "result:Session interrupted";
                                    }
                                    this->auth_channel_state = 0;
                                    if (this->acl) {
                                        this->acl->set_auth_channel_result(auth_channel_message + 7);
                                    }
                                }
                            }
                            else if (!this->enable_clipboard && !strcmp(mod_channel.name, CLIPBOARD_VIRTUAL_CHANNEL_NAME)) {
                                // Clipboard is unavailable and is a Clipboard PDU

                                TODO("RZ: Don't reject clipboard update, this can block rdesktop.");

                                    if (this->verbose) {
                                        LOG(LOG_INFO, "mod_rdp clipboard PDU");
                                    }

                                uint16_t msgType = sec.payload.in_uint16_le();

                                if (msgType == RDPECLIP::CB_FORMAT_LIST) {
                                    if (this->verbose) {
                                        LOG(LOG_INFO, "mod_rdp clipboard is unavailable");
                                    }

                                    bool response_ok = true;

                                    // Build and send the CB_FORMAT_LIST_RESPONSE (with status = FAILED)
                                    // 03 00 02 00 00 00 00 00
                                    RDPECLIP::FormatListResponsePDU format_list_response_pdu(response_ok);
                                    BStream                         out_s(256);

                                    format_list_response_pdu.emit(out_s);

                                    const CHANNELS::ChannelDef * mod_channel =
                                        this->mod_channel_list.get_by_name(
                                            CLIPBOARD_VIRTUAL_CHANNEL_NAME);

                                    if (mod_channel) {
                                        this->send_to_channel( *mod_channel
                                                             , out_s
                                                             , out_s.size()
                                                             ,   CHANNELS::CHANNEL_FLAG_FIRST
                                                               | CHANNELS::CHANNEL_FLAG_LAST
                                                             );
                                    }
                                }
                            }
                            else {
                                this->send_to_front_channel(mod_channel.name, sec.payload.p, length, chunk_size, flags);
                            }
                            sec.payload.p = sec.payload.end;
                        }
                        else {
                            uint8_t * next_packet = sec.payload.p;
                            while (next_packet < sec.payload.end) {
                                sec.payload.p = next_packet;

                                uint8_t * current_packet = next_packet;

                                ShareControl_Recv sctrl(sec.payload);
                                next_packet += sctrl.totalLength;

                                if (this->verbose & 128) {
                                    LOG(LOG_WARNING, "LOOPING on PDUs: %u", (unsigned)sctrl.totalLength);
                                }

                                switch (sctrl.pdu_type1) {
                                case PDUTYPE_DATAPDU:
                                    if (this->verbose & 128) {
                                        LOG(LOG_WARNING, "PDUTYPE_DATAPDU");
                                    }
                                    switch (this->connection_finalization_state){
                                    case EARLY:
                                        LOG(LOG_WARNING, "Rdp::finalization is early");
                                        throw Error(ERR_SEC);
                                        break;
                                    case WAITING_SYNCHRONIZE:
                                        if (this->verbose & 1){
                                            LOG(LOG_WARNING, "WAITING_SYNCHRONIZE");
                                        }
                                        //                            this->check_data_pdu(PDUTYPE2_SYNCHRONIZE);
                                        this->connection_finalization_state = WAITING_CTL_COOPERATE;
                                        break;
                                    case WAITING_CTL_COOPERATE:
                                        if (this->verbose & 1){
                                            LOG(LOG_WARNING, "WAITING_CTL_COOPERATE");
                                        }
                                        //                            this->check_data_pdu(PDUTYPE2_CONTROL);
                                        this->connection_finalization_state = WAITING_GRANT_CONTROL_COOPERATE;
                                        break;
                                    case WAITING_GRANT_CONTROL_COOPERATE:
                                        if (this->verbose & 1){
                                            LOG(LOG_WARNING, "WAITING_GRANT_CONTROL_COOPERATE");
                                        }
                                        //                            this->check_data_pdu(PDUTYPE2_CONTROL);
                                        this->connection_finalization_state = WAITING_FONT_MAP;
                                        break;
                                    case WAITING_FONT_MAP:
                                        if (this->verbose & 1){
                                            LOG(LOG_WARNING, "PDUTYPE2_FONTMAP");
                                        }
                                        //                            this->check_data_pdu(PDUTYPE2_FONTMAP);
                                        this->connection_finalization_state = UP_AND_RUNNING;

                                        // Synchronize sent to indicate server the state of sticky keys (x-locks)
                                        // Must be sent at this point of the protocol (sent before, it xwould be ignored or replaced)
                                        rdp_input_synchronize(0, 0, (this->key_flags & 0x07), 0);
                                        break;
                                    case UP_AND_RUNNING:
                                        if (this->enable_transparent_mode)
                                        {
                                            sec.payload.p = current_packet;

                                            HStream copy_stream(1024, 65535);

                                            copy_stream.out_copy_bytes(sec.payload.p, sec.payload.in_remain());
                                            copy_stream.mark_end();

                                            this->front.send_data_indication_ex(mcs.channelId, copy_stream);

                                            next_packet = sec.payload.end;

                                            break;
                                        }

                                        {
                                            ShareData sdata(sctrl.payload);
                                            sdata.recv_begin(&this->mppc_dec);
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
                                                        if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_ORDERS");}
                                                        this->front.begin_update();
                                                        this->orders.process_orders(this->bpp, sdata.payload, false, this);
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
                                            default:
                                                LOG(LOG_WARNING, "PDUTYPE2 unsupported tag=%u", sdata.pdutype2);
                                                TODO("CGR: Data should actually be consumed");
                                                    sdata.payload.p = sdata.payload.end;
                                                break;
                                            }
                                            sdata.recv_end();
                                        }
                                        break;
                                    }
                                    break;
                                case PDUTYPE_DEMANDACTIVEPDU:
                                    {
                                        if (this->verbose & 128){
                                             LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");
                                        }

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

                                        this->send_input(0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);

                                        /* Including RDP 5.0 capabilities */
                                        if (this->use_rdp5){
                                            LOG(LOG_INFO, "use rdp5");
                                            this->enum_bmpcache2();
                                            this->send_fonts(3);
                                        }
                                        else{
                                            LOG(LOG_INFO, "not using rdp5");
                                            this->send_fonts(1);
                                            this->send_fonts(2);
                                        }
                                        LOG(LOG_INFO, "Resizing to %ux%ux%u", this->front_width, this->front_height, this->bpp);
                                        if (-1 == this->front.server_resize(this->front_width, this->front_height, this->bpp)){
                                            LOG(LOG_WARNING, "Resize not available on older clients,"
                                                " change client resolution to match server resolution");
                                            throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
                                        }
                                        this->orders.reset();
                                        this->connection_finalization_state = WAITING_SYNCHRONIZE;
                                    }
                                    break;
                                case PDUTYPE_DEACTIVATEALLPDU:
                                    if (this->verbose & 128){ LOG(LOG_INFO, "PDUTYPE_DEACTIVATEALLPDU"); }
                                    LOG(LOG_INFO, "Deactivate All PDU");
                                    TODO("CGR: Data should actually be consumed");
                                        TODO("CGR: Check we are indeed expecting Synchronize... dubious");
                                        this->connection_finalization_state = WAITING_SYNCHRONIZE;
                                    break;
                                case PDUTYPE_SERVER_REDIR_PKT:
                                    if (this->verbose & 128){ LOG(LOG_INFO, "PDUTYPE_SERVER_REDIR_PKT"); }
                                    break;
                                default:
                                    LOG(LOG_INFO, "unknown PDU %u", sctrl.pdu_type1);
                                    break;
                                }
                                TODO("check sctrl.payload is completely consumed");
                            }
                        }
                    }
                }
            }
            catch(Error e){
                if (this->acl)
                {
                    char message[128];
                    snprintf(message, sizeof(message), "Code=%d", e.id);
                    this->acl->report("SESSION_EXCEPTION", message);

                    this->end_session_reason.empty();
                    this->end_session_message.empty();
                }

                BStream stream(256);
                X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
                try {
                    this->nego.trans->send(stream);
                    LOG(LOG_INFO, "Connection to server closed");
                }
                catch(Error e){
                    LOG(LOG_INFO, "Connection to server Already closed", e.id);
                };
                this->event.signal = BACK_EVENT_NEXT;

                if (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED)
                {
                    throw;
                }
            }
        }

        if (this->open_session_timeout) {
            switch(this->open_session_timeout_checker.check(now)) {
            case Timeout::TIMEOUT_REACHED:
                if (this->error_message) {
                    this->error_message->copy_c_str(
                        "Logon timer expired!");
                }
                LOG(LOG_ERR,
                    "Logon timer expired on %s. The session will be disconnected.",
                    this->hostname);
                if (this->acl)
                {
                    this->acl->report("CONNECTION_FAILED",
                        "Logon timer expired.");
                }

                this->event.signal = BACK_EVENT_NEXT;
                this->event.set();
            break;
            case Timeout::TIMEOUT_NOT_REACHED:
                this->event.set(1000000);
            break;
            case Timeout::TIMEOUT_INACTIVE:
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


    void send_confirm_active(mod_api * mod) throw(Error)
    {
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
        if (this->verbose) {
            general_caps.log("Sending to server");
        }
        confirm_active_pdu.emit_capability_set(general_caps);

        BitmapCaps bitmap_caps;
        bitmap_caps.preferredBitsPerPixel = this->bpp;
        bitmap_caps.desktopWidth          = this->front_width;
        bitmap_caps.desktopHeight         = this->front_height;
        bitmap_caps.bitmapCompressionFlag = this->bitmap_compression;
        if (this->verbose) {
            bitmap_caps.log("Sending bitmap caps to server");
        }
        confirm_active_pdu.emit_capability_set(bitmap_caps);

        OrderCaps order_caps;
        order_caps.numberFonts                                   = 0x147;
        order_caps.orderFlags                                    = 0x2a;
        order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_PATBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]            = (this->enable_mem3blt ? 1 : 0);
        order_caps.orderSupport[TS_NEG_LINETO_INDEX]             = 1;
        order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = 1;
        order_caps.orderSupport[UnusedIndex3]                    = 1;
        order_caps.orderSupport[UnusedIndex5]                    = 1;
        order_caps.orderSupport[TS_NEG_INDEX_INDEX]              = 1;
        order_caps.textFlags                                     = 0x06a1;
        order_caps.textANSICodePage                              = 0x4e4; // Windows-1252 codepage is passed (latin-1)
        if (this->verbose) {
            order_caps.log("Sending order caps to server");
        }
        confirm_active_pdu.emit_capability_set(order_caps);

        BmpCacheCaps bmp_cache_caps;
        bmp_cache_caps.cache0Entries         = 0x258;
        bmp_cache_caps.cache0MaximumCellSize = nbbytes(this->bpp) * 0x100;
        bmp_cache_caps.cache1Entries         = 0x12c;
        bmp_cache_caps.cache1MaximumCellSize = nbbytes(this->bpp) * 0x400;
        bmp_cache_caps.cache2Entries         = 0x106;
        bmp_cache_caps.cache2MaximumCellSize = nbbytes(this->bpp) * 0x1000;
        if (this->verbose) {
            bmp_cache_caps.log("Sending bmp cache caps to server");
        }
        confirm_active_pdu.emit_capability_set(bmp_cache_caps);

        //            if(this->use_rdp5){
        //                BmpCache2Caps bmpcache2_caps;
        //                bmpcache2_caps.numCellCaches = 3;
        //                bmpcache2_caps.bitmapCache0CellInfo = 2000;
        //                bmpcache2_caps.bitmapCache1CellInfo = 2000;
        //                bmpcache2_caps.bitmapCache2CellInfo = 2000;
        //                confirm_active_pdu.emit_capability_set(bmpcache2_caps);
        //            }

        ColorCacheCaps colorcache_caps;
        if (this->verbose) {
            colorcache_caps.log("Sending colorcache caps to server");
        }
        confirm_active_pdu.emit_capability_set(colorcache_caps);

        ActivationCaps activation_caps;
        if (this->verbose) {
            activation_caps.log("Sending activation caps to server");
        }
        confirm_active_pdu.emit_capability_set(activation_caps);

        ControlCaps control_caps;
        if (this->verbose) {
            control_caps.log("Sending control caps to server");
        }
        confirm_active_pdu.emit_capability_set(control_caps);

        PointerCaps pointer_caps;
        pointer_caps.len                       = 10;
        if (this->enable_new_pointer == false) {
            pointer_caps.pointerCacheSize      = 0;
            pointer_caps.colorPointerCacheSize = 20;
            pointer_caps.len                   = 8;
        }
        if (this->verbose) {
            pointer_caps.log("Sending pointer caps to server");
        }
        confirm_active_pdu.emit_capability_set(pointer_caps);

        ShareCaps share_caps;
        if (this->verbose) {
            share_caps.log("Sending share caps to server");
        }
        confirm_active_pdu.emit_capability_set(share_caps);

        InputCaps input_caps;
        if (this->verbose) {
            input_caps.log("Sending input caps to server");
        }
        confirm_active_pdu.emit_capability_set(input_caps);

        SoundCaps sound_caps;
        if (this->verbose) {
            sound_caps.log("Sending sound caps to server");
        }
        confirm_active_pdu.emit_capability_set(sound_caps);

        FontCaps font_caps;
        if (this->verbose) {
            font_caps.log("Sending font caps to server");
        }
        confirm_active_pdu.emit_capability_set(font_caps);

        GlyphSupportCaps glyphsupport_caps;
        if (this->verbose) {
            glyphsupport_caps.log("Sending glyphsupport caps to server");
        }
        confirm_active_pdu.emit_capability_set(glyphsupport_caps);

        //            BrushCacheCaps brushcache_caps;
        //            brushcache_caps.log("Sending brushcache caps to server");
        //            confirm_active_pdu.emit_capability_set(BrushCacheCaps);

        //            CompDeskCaps compdesk_caps;
        //            compdesk_caps.log("Sending compdesk caps to server");
        //            confirm_active_pdu.emit_capability_set(CompDeskCaps);

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
    }

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
            this->process_color_pointer_pdu(stream);
            if (this->verbose & 4){
                LOG(LOG_INFO, "Process pointer color done");
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
                LOG(LOG_WARNING, "mod::rdp::RDP Pointer move not yet supported");
                TODO("CGR: implement RDP_POINTER_MOVE");
                    /* int x = */ stream.in_uint16_le();
                /* int y = */ stream.in_uint16_le();
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
        ERRINFO_DECRYPTFAILED2                    = 0x00001195,
    };

    void process_disconnect_pdu(Stream & stream)
    {
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
    }

    void process_logon_info(const char * domain, const char * username) {
        char domain_username_format_0[2048];
        char domain_username_format_1[2048];

        snprintf(domain_username_format_0, sizeof(domain_username_format_0),
            "%s@%s", username, domain);
        snprintf(domain_username_format_1, sizeof(domain_username_format_0),
            "%s\\%s", domain, username);
//        LOG(LOG_INFO,
//            "Domain username format 0=(%s) Domain username format 1=(%s)",
//            domain_username_format_0, domain_username_format_0);

        if (this->disconnect_on_logon_user_change &&
            ((strcasecmp(domain, this->domain) || strcasecmp(username, this->username)) &&
             (this->domain[0] ||
              (strcasecmp(domain_username_format_0, this->username) &&
               strcasecmp(domain_username_format_1, this->username) &&
               strcasecmp(username, this->username))))) {
            if (this->error_message) {
                this->error_message->copy_c_str(
                    "Unauthorized logon user change detected!");
            }
            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s\\%s) -> (%s\\%s). "
                    "The session will be disconnected.",
                this->hostname, this->domain, this->username, domain, username);
            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s\\%s). "
                    "The session will be disconnected.",
                this->hostname, domain, username);

            this->end_session_reason.copy_c_str("OPEN_SESSION_FAILED");
            this->end_session_message.copy_c_str(
                "Unauthorized logon user change detected.");

            throw Error(ERR_RDP_LOGON_USER_CHANGED);
        }

        if (this->acl)
        {
            this->acl->report("OPEN_SESSION_SUCCESSFUL", "Ok.");
        }
        this->end_session_reason.copy_c_str("CLOSE_SESSION_SUCCESSFUL");
        this->end_session_message.copy_c_str("OK.");

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

    TODO("CGR: this can probably be unified with process_confirm_active in front");
    void process_server_caps(Stream & stream, uint16_t len)
    {
        if (this->verbose & 32){
            LOG(LOG_INFO, "mod_rdp::process_server_caps");
        }

        FILE * output_file = 0;

        if (!this->output_filename.is_empty())
        {
            output_file = fopen(this->output_filename.c_str(), "w");
        }

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
                    if (this->verbose) {
                        general_caps.log("Received from server");
                    }
                    if (output_file)
                    {
                        general_caps.dump(output_file);
                    }
                }
                break;
            case CAPSTYPE_BITMAP:
                {
                    BitmapCaps bitmap_caps;
                    bitmap_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        bitmap_caps.log("Received from server");
                    }
                    if (output_file)
                    {
                        bitmap_caps.dump(output_file);
                    }
                    this->bpp = bitmap_caps.preferredBitsPerPixel;
                    this->front_width = bitmap_caps.desktopWidth;
                    this->front_height = bitmap_caps.desktopHeight;

                    this->orders.create_cache_bitmap(this->bpp, 0x258, nbbytes(this->bpp) * 0x100,
                        0x12c, nbbytes(this->bpp) * 0x400, 0x106, nbbytes(this->bpp) * 0x1000);
                }
                break;
            case CAPSTYPE_ORDER:
                {
                    OrderCaps order_caps;
                    order_caps.recv(stream, capset_length);
                    if (this->verbose) {
                        order_caps.log("Received from server");
                    }
                    if (output_file)
                    {
                        order_caps.dump(output_file);
                    }
                }
                break;
            case CAPSTYPE_INPUT:
                {
                    InputCaps input_caps;
                    input_caps.recv(stream, capset_length);
                    if (this->verbose) {
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

        if (output_file)
        {
            fclose(output_file);
        }

        if (this->verbose & 32){
            LOG(LOG_INFO, "mod_rdp::process_server_caps done");
        }
    }

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

    TODO("CGR: duplicated code in front");
    void send_synchronise() throw (Error)
    {
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

    void send_fonts(int seq) throw(Error)
    {
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
    /* Send persistent bitmap cache enumeration PDU's
       Not implemented yet because it should be implemented
       before in process_data case. The problem is that
       we don't save the bitmap key list attached with rdp_bmpcache2 capability
       message so we can't develop this function yet */

    void enum_bmpcache2() {}

    void send_input_slowpath(int time, int message_type, int device_flags, int param1, int param2) throw(Error)
    {
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
            {
                FastPath::KeyboardEvent_Send ke(stream, (uint16_t)device_flags, param1);
            }
            break;

        case RDP_INPUT_SYNCHRONIZE:
            {
                FastPath::SynchronizeEvent_Send se(stream, param1);
            }
            break;

        case RDP_INPUT_MOUSE:
            {
                FastPath::MouseEvent_Send me(stream, (uint16_t)device_flags, param1, param2);
            }
            break;

        default:
            LOG(LOG_WARNING, "unsupported fast-path input message type 0x%x", message_type);
            throw Error(ERR_RDP_FASTPATH);
            break;
        }

        FastPath::ClientInputEventPDU_Send out_cie(fastpath_header, stream, 1, this->encrypt, this->encryptionLevel, this->encryptionMethod);

        this->nego.trans->send(fastpath_header, stream);

        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::send_input_fastpath done");
        }
    }

    void send_input(int time, int message_type, int device_flags, int param1, int param2) throw(Error) {
        if (this->enable_fastpath_client_input_event == false) {
            send_input_slowpath(time, message_type, device_flags, param1, param2);
        }
        else {
            send_input_fastpath(time, message_type, device_flags, param1, param2);
        }
    }

    virtual void rdp_input_invalidate(const Rect & r)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate");
        }
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            if (!r.isempty()){
                RDP::RefreshRectPDU rrpdu(this->share_id,
                                          this->userid,
                                          this->encryptionLevel,
                                          this->encrypt);

                rrpdu.addInclusiveRect(r.x, r.y, r.cx - 1, r.cy - 1);

                rrpdu.emit(*this->nego.trans);
            }
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate done");
        }
    }

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
            LOG(LOG_WARNING,
                "mod_rdp::Bad length for color pointer mask_len=%u data_len=%u",
                (unsigned)mlen, static_cast<unsigned>(dlen));
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

    void process_cached_pointer_pdu(Stream & stream)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu");
        }

        int pointer_idx = stream.in_uint16_le();
        if (pointer_idx < 0){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu negative pointer cache idx (%d)", pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_LESS_0);
        }
        if (pointer_idx >= (int)(sizeof(this->cursors) / sizeof(Pointer))) {
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu pointer cache idx overflow (%d)", pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        struct Pointer & cursor = this->cursors[pointer_idx];
        this->front.server_set_pointer(cursor);
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu done");
        }
    }

    void process_system_pointer_pdu(Stream & stream)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu");
        }
        int system_pointer_type = stream.in_uint16_le();
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                struct Pointer cursor;
                memset(cursor.mask, 0xff, sizeof(cursor.mask));
                this->front.server_set_pointer(cursor);
                this->set_pointer_display();
            }
            break;
        default:
            break;
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu done");
        }
    }

    void to_regular_mask(Stream & stream, unsigned mlen, uint8_t bpp,
            uint8_t * mask, size_t mask_size) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_mask");
        }
        TODO("CGR: we should ensure we have data enough to create mask");
        uint8_t * end = stream.p + mlen;
        switch (bpp) {
        case 1 :
        {
            for (unsigned x = 0; x < mlen ; x++) {
                BGRColor px = stream.in_uint8();
                // incoming new pointer mask is upside down, revert it
                mask[128 - 4 - (x & 0xFFFC) + (x & 3)] = px;
            }
        }
        break;
        default:
            for (unsigned x = 0; x < mlen ; x++) {
                BGRColor px = stream.in_uint8();
                mask[x] = px;
            }
//            stream.in_copy_bytes(mask, mlen);
        break;
        }

        stream.p = end;
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_mask");
        }
    }

    void to_regular_pointer(Stream & stream, unsigned dlen, uint8_t bpp, uint8_t * data, size_t target_data_len) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_pointer");
        }
        uint8_t * end = stream.p + dlen;
        TODO("CGR: we should ensure we have data enough to create pointer");
        switch (bpp) {
        case 1 :
        {
            for (unsigned x = 0; x < dlen ; x ++) {
                BGRColor px = stream.in_uint8();
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
            for (unsigned i=0; i < dlen ; i++) {
                BGRColor px = stream.in_uint8();
                // target cursor will receive 8 bits input at once
                ::out_bytes_le(&(data[6 * i]),     3, color_decode((px >> 4) & 0xF, bpp, this->orders.global_palette));
                ::out_bytes_le(&(data[6 * i + 3]), 3, color_decode(px        & 0xF, bpp, this->orders.global_palette));
            }
        }
        break;
        case 32: case 24: case 16: case 15: case 8:
        {
            uint8_t BPP = nbbytes(bpp);
            for (unsigned i=0; i + BPP <= dlen; i += BPP) {
                BGRColor px = stream.in_bytes_le(BPP);
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

        stream.p = end;
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::to_regular_pointer");
        }
    }

    void process_new_pointer_pdu(Stream & stream) throw(Error) {
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu");
        }

        unsigned data_bpp  = stream.in_uint16_le(); /* data bpp */
        unsigned pointer_idx = stream.in_uint16_le();
        
        if (pointer_idx < 0){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu negative pointer cache idx (%d)", pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_LESS_0);
        }
        if (pointer_idx >= (int)(sizeof(this->cursors) / sizeof(Pointer))) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu pointer cache idx overflow (%d)", pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }

        Pointer & cursor = this->cursors[pointer_idx];
        memset(&cursor, 0, sizeof(struct Pointer));
        cursor.bpp    = data_bpp;
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
            uint8_t copy_data_data[32*32*4];
            uint8_t copy_mask_data[32*32/8];
            stream.in_copy_bytes(copy_data_data, dlen);
            stream.in_copy_bytes(copy_mask_data, mlen);

            unsigned   i;
            uint8_t  * mask_data;
            uint8_t  * data_data;
            uint8_t    new_mask_data;

            for (i = 0, data_data = copy_data_data, mask_data = copy_mask_data; i < mlen;
                 i++, data_data++, mask_data++) {
                new_mask_data = (*mask_data & (*data_data ^ 0xFF));
                *data_data    = (*data_data ^ *mask_data ^ new_mask_data);
                *mask_data    = new_mask_data;
            }

            FixedSizeStream data_stream(copy_data_data, sizeof(copy_data_data));
            FixedSizeStream mask_stream(copy_mask_data, sizeof(copy_mask_data));

            to_regular_pointer(data_stream,
                dlen, data_bpp, cursor.data, sizeof(cursor.data));
            to_regular_mask(mask_stream,
                mlen, data_bpp, cursor.mask, sizeof(cursor.mask));
                cursor.bpp = 24;
        }
        else {
            to_regular_pointer(stream, dlen, data_bpp, cursor.data, sizeof(cursor.data));
            to_regular_mask(stream, mlen, data_bpp, cursor.mask, sizeof(cursor.mask));
            cursor.bpp = 24;
        }

        this->front.server_set_pointer(cursor);
        if (this->verbose & 4) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu done");
        }
    }

    void process_bitmap_updates(Stream & stream, bool fast_path)
    {
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
            Bitmap bitmap( bmpdata.bits_per_pixel
                           , &this->orders.global_palette
                           , bmpdata.width
                           , bmpdata.height
                           , data
                           , bmpdata.bitmap_size()
                           , (bmpdata.flags & BITMAP_COMPRESSION)
                           );

            if (   bmpdata.cb_scan_width
                   && ((bmpdata.cb_scan_width - bitmap.line_size) >= nbbytes(bitmap.original_bpp))) {
                LOG( LOG_WARNING
                     , "Bad line size: line_size=%u width=%u height=%u bpp=%u"
                     , bmpdata.cb_scan_width
                     , bmpdata.width
                     , bmpdata.height
                     , bmpdata.bits_per_pixel
                     );
            }

            if (   bmpdata.cb_uncompressed_size
                   && (bmpdata.cb_uncompressed_size != bitmap.bmp_size)) {
                LOG( LOG_WARNING
                     , "final_size should be size of decompressed bitmap [%u != %u] width=%u height=%u bpp=%u"
                     , bmpdata.cb_uncompressed_size
                     , bitmap.bmp_size
                     , bmpdata.width
                     , bmpdata.height
                     , bmpdata.bits_per_pixel
                     );
            }

            TODO("this is to protect rdesktop different color depth works with mstsc and xfreerdp");
            if (!this->enable_bitmap_update
               || (bmpdata.bits_per_pixel != this->front_bpp)
               || ((bmpdata.bits_per_pixel == 8) && (this->front_bpp != 8))) {
                this->front.draw(RDPMemBlt(0, boundary, 0xCC, 0, 0, 0), boundary, bitmap);
            }
            else {
                this->front.draw(bmpdata, data, bmpdata.bitmap_size(), bitmap);
            }
        }
        if (this->verbose & 64){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates done");
        }
    }

    void send_client_info_pdu(int userid, const char * password)
    {
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

        if (this->enable_rdp_bulk_compression) {
            infoPacket.flags |= INFO_COMPRESSION;
            infoPacket.flags &= ~CompressionTypeMask;
            infoPacket.flags |= (PACKET_COMPR_TYPE_64K << 9);
        }

        if (this->verbose) {
            infoPacket.log("Sending to server: ");
        }
        infoPacket.emit(stream);
        stream.mark_end();

        BStream sec_header(256);

        SEC::Sec_Send sec(sec_header, stream, SEC::SEC_INFO_PKT,
            this->encrypt, this->encryptionLevel);
        stream.copy_to_head(sec_header);

        this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, stream);

        if (this->open_session_timeout) {
            this->open_session_timeout_checker.restart_timeout(
                time(NULL), this->open_session_timeout);
            this->event.set(1000000);
        }

        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu done");
        }
    }

    virtual void begin_update()
    {
        this->front.begin_update();
    }

    virtual void end_update()
    {
        this->front.begin_update();
    }

    virtual void draw(const RDPGlyphCache & cmd)
    {
        this->front.draw(cmd);
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        this->front.draw(cmd, clip, gly_cache);
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        this->front.server_draw_text(x, y, text, fgcolor, bgcolor, clip);
    }

    virtual void text_metrics(const char * text, int & width, int & height)
    {
        this->front.text_metrics(text, width, height);
    }

    virtual void draw(const RDPColCache & cmd)
    {
        this->front.draw(cmd);
    }
};

#endif
