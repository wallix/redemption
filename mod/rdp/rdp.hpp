/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages
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

#include"rdp/rdp_orders.hpp"
#include"rdp/rdp_cursor.hpp"

/* include "ther h files */
#include"stream.hpp"
#include"ssl_calls.hpp"
#include"constants.hpp"
#include"client_mod.hpp"

#include"RDP/x224.hpp"
#include"RDP/nego.hpp"
#include"RDP/mcs.hpp"
#include"RDP/lic.hpp"
#include"RDP/logon.hpp"
#include"channel_list.hpp"
#include"RDP/gcc.hpp"
#include"RDP/sec.hpp"
#include"colors.hpp"
#include"RDP/capabilities.hpp"

#include"../acl/authentifier.hpp"

#include"genrandom.hpp"



struct mod_rdp : public client_mod {

    /* mod data */
    BStream in_stream;
    ChannelDefArray mod_channel_list;

    bool dev_redirection_enable;
    int use_rdp5;
    int keylayout;

    uint8_t lic_layer_license_key[16];
    uint8_t lic_layer_license_sign_key[16];
    int lic_layer_license_issued;
    uint8_t * lic_layer_license_data;
    size_t lic_layer_license_size;

    rdp_orders orders;
    int share_id;
    int bitmap_compression;
    int version;
    uint16_t userid;

    char hostname[16];
    char username[128];
    char password[256];
    char domain[256];
    char program[256];
    char directory[256];
    uint8_t bpp;

    int encryptionLevel;
    int encryptionMethod;
    int key_flags;
    uint32_t server_public_key_len;
    uint8_t client_crypt_random[512];
    CryptContext encrypt, decrypt;

    enum {
        MOD_RDP_NEGO,
        MOD_RDP_BASIC_SETTINGS_EXCHANGE,
        MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER,
        MOD_RDP_GET_LICENSE,
        MOD_RDP_WAITING_DEMAND_ACTIVE_PDU,
        MOD_RDP_CONNECTED,
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
    struct rdp_cursor cursors[32];
    const bool console_session;
    const int brush_cache_code;
    const uint8_t front_bpp;
    Random * gen;
    uint32_t verbose;

    SessionManager *sesman;
    char auth_channel[8];
    int auth_channel_flags;
    int auth_channel_chanid;
    int auth_channel_state; // 0 means unused, 1 means session running

    RdpNego nego;

    char clientAddr[512];
    uint16_t cbClientAddr;

    bool enable_new_pointer;

    bool opt_clipboard;  // true clipboard available, false clipboard unavailable
    uint32_t performanceFlags;

    mod_rdp(Transport * trans,
            const char * target_user,
            const char * target_password,
            const char * clientIP,
            struct FrontAPI & front,
            const char * hostname,
            const bool tls,
            const ClientInfo & info,
            Random * gen,
            int key_flags,
            SessionManager * sesman,
            const char * auth_channel,
            bool clipboard,
            uint32_t verbose = 0,
            bool enable_new_pointer = false)
            :
                client_mod(front, info.width, info.height),
                    in_stream(65536),
                    use_rdp5(1),
                    keylayout(info.keylayout),
                    orders(0),
                    share_id(0),
                    bitmap_compression(1),
                    version(0),
                    userid(0),
                    bpp(bpp),
                    encryptionLevel(0),
                    server_public_key_len(0),
                    connection_finalization_state(EARLY),
                    state(MOD_RDP_NEGO),
                    console_session(info.console_session),
                    brush_cache_code(info.brush_cache_code),
                    front_bpp(info.bpp),
                    gen(gen),
                    verbose(verbose),
                    sesman(sesman),
                    auth_channel_flags(0),
                    auth_channel_chanid(0),
                    auth_channel_state(0), // 0 means unused
                    nego(tls, trans, target_user),
                    enable_new_pointer(enable_new_pointer),
                    opt_clipboard(clipboard),
                    performanceFlags(info.rdp5_performanceflags)
    {
        if (this->verbose & 1){
            LOG(LOG_INFO, "Creation of new mod 'RDP'");
        }

        memset(this->auth_channel, 0, 8);
        strncpy(this->auth_channel, auth_channel, 8);
        this->cbClientAddr = strlen(clientIP)+1;
        memcpy(this->clientAddr, clientIP, this->cbClientAddr);

        this->key_flags = key_flags;
        this->lic_layer_license_issued = 0;

        this->lic_layer_license_size = 0;
        memset(this->lic_layer_license_key, 0, 16);
        memset(this->lic_layer_license_sign_key, 0, 16);
        TODO("CGR: license loading should be done before creating protocol layers")
        struct stat st;
        char path[256];
        sprintf(path, LICENSE_PATH "/license.%s", hostname);
        int fd = open(path, O_RDONLY);
        if (fd != -1 && fstat(fd, &st) != 0){
            this->lic_layer_license_data = (uint8_t *)malloc(this->lic_layer_license_size);
            if (this->lic_layer_license_data){
                size_t lic_size = read(fd, this->lic_layer_license_data, this->lic_layer_license_size);
                if (lic_size != this->lic_layer_license_size){
                    LOG(LOG_ERR, "license file truncated : expected %u, got %u", this->lic_layer_license_size, lic_size);
                }
            }
            close(fd);
        }

        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        this->decrypt.encryptionMethod = 2; /* 128 bits */
        this->encrypt.encryptionMethod = 2; /* 128 bits */

        TODO("CGR: and if hostname is really larger  what happens ? We should at least emit a warning log")
        strncpy(this->hostname, hostname, 15);
        this->hostname[15] = 0;
        TODO("CGR: and if username is really larger  what happens ? We should at least emit a warning log")
        strncpy(this->username, target_user, 127);
        this->username[127] = 0;

        LOG(LOG_INFO, "Remote RDP Server login:%s host:%s", this->username, this->hostname);

        memset(this->password, 0, 256);
        strcpy(this->password, target_password);

        memset(this->domain, 0, 256);
        memset(this->program, 0, 256);
        memset(this->directory, 0, 256);

        LOG(LOG_INFO, "Server key layout is %x", this->keylayout);

        while (UP_AND_RUNNING != this->connection_finalization_state){
            BackEvent_t res = this->draw_event();
            if (res != BACK_EVENT_NONE){
                LOG(LOG_INFO, "Creation of new mod 'RDP' failed");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }

    virtual ~mod_rdp() {
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long time, Keymap2 * keymap){
        if (UP_AND_RUNNING == this->connection_finalization_state) {
//            LOG(LOG_INFO, "Direct parameter transmission ");
            this->send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_SYNCHRONIZE, device_flags, param1, 0);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            TODO("CGR: is decoding and reencoding really necessary  a simple pass-through from front to back-end should be enough")
            if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, x, y);
            }
            if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1 | (device_flags & MOUSE_FLAG_DOWN), x, y);
            }
            if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2 | (device_flags & MOUSE_FLAG_DOWN), x, y);
            }
            if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON3 | (device_flags & MOUSE_FLAG_DOWN), x, y);
            }
            if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_DOWN, x, y);
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4, x, y);
            }
            if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5 | MOUSE_FLAG_DOWN, x, y);
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5, x, y);
            }
        }
    }

    virtual void send_to_mod_channel(
                const char * const front_channel_name,
                Stream & chunk,
                size_t length,
                uint32_t flags)
    {
        if (this->verbose & 16){
            LOG(LOG_INFO, "mod_rdp::send_to_mod_channel");
            LOG(LOG_INFO, "sending to channel %s", front_channel_name); 
        }

        // Clipboard is unavailable and is a Clipboard PDU
        if (!this->opt_clipboard && !::strcmp(front_channel_name, CLIPBOARD_VIRTUAL_CHANNEL_NAME)){
            if ( this->verbose ){
                LOG( LOG_INFO, "mod_rdp clipboard PDU" );
            }

            if (!chunk.in_check_rem(2)){
                LOG(LOG_INFO, "mod_vnc::send_to_vnc truncated msgType, need=2 remains=%u",
                    chunk.in_remain());
                throw Error(ERR_VNC);
            }

            uint16_t msgType = chunk.in_uint16_le();

            if (msgType == ChannelDef::CB_FORMAT_LIST){
                if ( this->verbose ){
                    LOG( LOG_INFO, "mod_rdp clipboard is unavailable" );
                }

                TODO("RZ: duplicate code")

                // Build and send the CB_FORMAT_LIST_RESPONSE (with status = FAILED)
                // 03 00 02 00 00 00 00 00

                BStream out_s(256);
                out_s.out_uint16_le(ChannelDef::CB_FORMAT_LIST_RESPONSE);   //  - MSG Type 2 bytes
                out_s.out_uint16_le(ChannelDef::CB_RESPONSE_FAIL);          //  - MSG flags 2 bytes
                out_s.out_uint32_le(0);                                     //  - remaining datalen of message
                out_s.mark_end();

                size_t length = out_s.size();

                this->send_to_front_channel( (char *) CLIPBOARD_VIRTUAL_CHANNEL_NAME
                                           , out_s.data
                                           , length
                                           , out_s.size()
                                           , ChannelDef::CHANNEL_FLAG_FIRST | ChannelDef::CHANNEL_FLAG_LAST
                                           );

                return;
            }
        }

        const ChannelDef * mod_channel = this->mod_channel_list.get(front_channel_name);
        // send it if module has a matching channel, if no matching channel is found just forget it
        if (mod_channel){
            if (this->verbose & 16){
                int index = this->mod_channel_list.get_index(front_channel_name);
                mod_channel->log(index);
            }
            this->send_to_channel(*mod_channel, chunk, length, flags);
        }

        if (this->verbose & 16){
            LOG(LOG_INFO, "mod_rdp::send_to_mod_channel done");
        }
    }

    // Method used by session to transmit sesman answer for auth_channel
    virtual void send_auth_channel_data(const char * data) {
        if (!this->sesman){ return; }
        BStream stream(65536);
        BStream x224_header(256);
        BStream mcs_header(256);
        BStream sec_header(256);

        if (strncmp("Error:", data, 6)) {
            this->auth_channel_state = 1; // session started
        }
        stream.out_uint32_le(strlen(data));
        stream.out_uint32_le(this->auth_channel_flags);
        stream.out_copy_bytes(data, strlen(data));
        stream.mark_end();

        SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, 0);
        MCS::SendDataIndication_Send mcs(mcs_header, userid, 
            this->auth_channel_chanid, 1, 3, sec_header.size() + stream.size(), MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header,  mcs_header.size() + sec_header.size() + stream.size());

        this->nego.trans->send(x224_header);
        this->nego.trans->send(mcs_header);
        this->nego.trans->send(sec_header);
        this->nego.trans->send(stream);
    }

    void send_to_channel(
                const ChannelDef & channel,
                Stream & chunk,
                size_t length,
                uint32_t flags)
    {
        if (this->verbose & 16){
            LOG(LOG_INFO, "mod_rdp::send_to_channel length=%u chunk_size=%u", (unsigned)length, (unsigned)chunk.size());
            channel.log(-1);
        }

        BStream stream(65536);

        stream.out_uint32_le(length);
        stream.out_uint32_le(flags);
        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= ChannelDef::CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_copy_bytes(chunk.data, chunk.size());
        stream.mark_end();

        BStream x224_header(256);
        BStream mcs_header(256);
        BStream sec_header(256);
        SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channel.chanid, 1, 3,
                                      sec_header.size() + stream.size() , MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

        this->nego.trans->send(x224_header);
        this->nego.trans->send(mcs_header);
        this->nego.trans->send(sec_header);
        this->nego.trans->send(stream);

        if (this->verbose & 16){
            LOG(LOG_INFO, "mod_rdp::send_to_channel done");
        }
    }

    void send_data_request(uint16_t channelId, Stream & stream)
    {
        BStream x224_header(256);
        BStream mcs_header(256);

        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channelId, 1, 3, stream.size(), MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans->send(x224_header);
        this->nego.trans->send(mcs_header);
        this->nego.trans->send(stream);
    }

    virtual BackEvent_t draw_event(void)
    {
        try{
        char * hostname = this->hostname;

        switch (this->state){
        case MOD_RDP_NEGO:
            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::Early TLS Security Exchange");
            }
            switch (this->nego.state){
                default:
                    this->nego.server_event();
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

                    BStream stream(65536);
                    // ------------------------------------------------------------
                    GCC::UserData::CSCore cs_core;
                    cs_core.version = this->use_rdp5?0x00080004:0x00080001;
                    cs_core.desktopWidth = this->front_width;
                    cs_core.desktopHeight = this->front_height;
                    cs_core.highColorDepth = this->front_bpp;
                    cs_core.keyboardLayout = keylayout;
                    uint16_t hostlen = strlen(hostname);
                    uint16_t maxhostlen = std::min((uint16_t)15, hostlen);
                    for (size_t i = 0; i < maxhostlen ; i++){
                        cs_core.clientName[i] = hostname[i];
                    }
                    bzero(&(cs_core.clientName[hostlen]), 16-hostlen);
                    if (this->nego.tls){
                        cs_core.serverSelectedProtocol = 1;
                    }
                    cs_core.log("Sending to Server");
                    if (this->nego.tls){
                    }
                    cs_core.emit(stream);

                    // ------------------------------------------------------------
                    GCC::UserData::CSCluster cs_cluster;
                    TODO("CGR: values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?")

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
                    cs_cluster.log("Sending to server");
                    cs_cluster.emit(stream);
                    // ------------------------------------------------------------

                    GCC::UserData::CSSecurity cs_security;
                    cs_security.log("Sending to server");
                    cs_security.emit(stream);
                    // ------------------------------------------------------------

                    const ChannelDefArray & channel_list = this->front.get_channel_list();
                    size_t num_channels = channel_list.size();
                    if (num_channels > 0) {
                        /* Here we need to put channel information in order to redirect channel data
                           from client to server passing through the "proxy" */
                        GCC::UserData::CSNet cs_net;
                        cs_net.channelCount = num_channels;
                        for (size_t index = 0; index < num_channels; index++){
                            const ChannelDef & channel_item = channel_list[index];
                            memcpy(cs_net.channelDefArray[index].name, channel_list[index].name, 8);
                            cs_net.channelDefArray[index].options = channel_item.flags;
                            ChannelDef def;
                            memcpy(def.name, cs_net.channelDefArray[index].name, 8);
                            def.flags = channel_item.flags;
                            if (this->verbose & 16){
                                def.log(index);
                            }
                            this->mod_channel_list.push_back(def);
                        }
                        // Inject a new channel for auth_channel virtual channel
                        if (this->auth_channel[0] && this->sesman){
                            memcpy(cs_net.channelDefArray[num_channels].name, this->auth_channel, 8);
                            TODO("CGR: We should figure out what value options should actually have, not just get any channel option and copy it")
                            cs_net.channelDefArray[num_channels].options = cs_net.channelDefArray[num_channels-1].options;
                            cs_net.channelCount++;
                            ChannelDef def;
                            memcpy(def.name, this->auth_channel, 8);
                            def.flags = cs_net.channelDefArray[num_channels].options;
                            if (this->verbose & 16){
                                def.log(num_channels);
                            }
                            this->mod_channel_list.push_back(def);
                        }
                        
                        cs_net.log("Sending to server");
                        cs_net.emit(stream);
                    }
                    // ------------------------------------------------------------

                    BStream gcc_header(65536);
                    GCC::Create_Request_Send(gcc_header, stream.size());

                    BStream mcs_header(65536);
                    MCS::CONNECT_INITIAL_Send mcs(mcs_header, gcc_header.size() + stream.size(), MCS::BER_ENCODING);

                    BStream x224_header(256);
                    X224::DT_TPDU_Send(x224_header, mcs_header.size() + gcc_header.size() + stream.size());

                    this->nego.trans->send(x224_header);
                    this->nego.trans->send(mcs_header);
                    this->nego.trans->send(gcc_header);
                    this->nego.trans->send(stream);

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
                        sc_core.log("Received from server");
                        if (0x0080001 == sc_core.version){ // can't use rdp5
                            this->use_rdp5 = 0;
                        }
                    }
                    break;
                    case SC_SECURITY:
                    {
                        GCC::UserData::SCSecurity sc_sec1;
                        sc_sec1.recv(f.payload);
                        sc_sec1.log("Received from server");

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

                            ssl.rsa_encrypt(client_crypt_random, client_random,
                                    SEC_RANDOM_SIZE, this->server_public_key_len, modulus, exponent);
                            uint8_t key_block[48];
                            ssl.rdp_sec_generate_keyblock(key_block, client_random, serverRandom);
                            memcpy(encrypt.sign_key, key_block, 16);
                            if (sc_sec1.encryptionMethod == 1){
                                ssl.sec_make_40bit(encrypt.sign_key);
                            }
                            this->decrypt.generate_key(&key_block[16], client_random, serverRandom, sc_sec1.encryptionMethod);
                            this->encrypt.generate_key(&key_block[32], client_random, serverRandom, sc_sec1.encryptionMethod);
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
                        sc_net.log("Received from server");
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
                BStream mcs_data(256);

                MCS::ErectDomainRequest_Send mcs(mcs_data, 0, 0, MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_data.size());
                this->nego.trans->send(x224_header);
                this->nego.trans->send(mcs_data);
            }
            if (this->verbose & 1){
                LOG(LOG_INFO, "Send MCS::AttachUserRequest");
            }
            {
                BStream x224_header(256);
                BStream mcs_data(256);

                MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);
                size_t mcs_length = mcs_data.size();
                X224::DT_TPDU_Send(x224_header, mcs_length);

                this->nego.trans->send(x224_header);
                this->nego.trans->send(mcs_data);
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
                TODO("CGR: the array size below is arbitrary  it should be checked to avoid buffer overflow")

                size_t num_channels = this->mod_channel_list.size();
                uint16_t channels_id[100];
                channels_id[0] = this->userid + GCC::MCS_USERCHANNEL_BASE;
                channels_id[1] = GCC::MCS_GLOBAL_CHANNEL;
                for (size_t index = 0; index < num_channels; index++){
                    channels_id[index+2] = this->mod_channel_list[index].chanid;
                }

                for (size_t index = 0; index < num_channels+2; index++){
                    BStream x224_header(256);
                    BStream mcs_cjrq_data(256);
                    if (this->verbose & 16){
                        LOG(LOG_INFO, "cjrq[%u] = %u", index, channels_id[index]);
                    }
                    MCS::ChannelJoinRequest_Send(mcs_cjrq_data, this->userid, channels_id[index], MCS::PER_ENCODING);
                    size_t mcs_length = mcs_cjrq_data.size();
                    X224::DT_TPDU_Send(x224_header, mcs_length);
                    size_t x224_header_length = x224_header.size();
                    this->nego.trans->send(x224_header.data, x224_header_length);
                    this->nego.trans->send(mcs_cjrq_data.data, mcs_length);

                    BStream x224_data(256);
                    X224::RecvFactory f(*this->nego.trans, x224_data);
                    X224::DT_TPDU_Recv x224(*this->nego.trans, x224_data);
                    SubStream & mcs_cjcf_data = x224.payload;
                    MCS::ChannelJoinConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
                    TODO("CGR: We should check channel confirmation worked, for now we just do like server said OK to everything... and that may not be the case, some channels may be closed for instance. We should also check requested chanid are some confirm may come out of order");
                    TODO("If mcs.result is negative channel is not confirmed and should be removed from mod_channel list")
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
            // header is include" with the data if encryption is in force (the Client Info
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
                    LOG(LOG_INFO, "mod_rdp::SecExchangePacket keylen=%u", this->server_public_key_len);
                }
                BStream stream(this->server_public_key_len + 32);
                SEC::SecExchangePacket_Send mcs(stream, client_crypt_random, this->server_public_key_len);
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

            SEC::Sec_Recv sec(mcs.payload, true, this->decrypt, this->encryptionLevel, this->encryptionMethod);

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

                            /* We currently use null client keys. This is a bit naughty but, hey,
                               the security of license negotiation isn't exactly paramount. */
                            memset(null_data, 0, sizeof(null_data));
                            uint8_t* client_random = null_data;
                            uint8_t* pre_master_secret = null_data;
                            uint8_t master_secret[48];
                            uint8_t key_block[48];

                            /* Generate master secret and then key material */
                            for (int i = 0; i < 3; i++) {
                                uint8_t shasig[20];
                                uint8_t pad[4];

                                memset(pad, 'A' + i, i + 1);

                                SslSha1 sha1;
                                sha1.update(FixedSizeStream(pad, i + 1));
                                sha1.update(FixedSizeStream(pre_master_secret, 48));
                                sha1.update(FixedSizeStream(client_random, 32));
                                sha1.update(FixedSizeStream(lic.server_random, 32));
                                sha1.final(shasig);

                                SslMd5 md5;
                                md5.update(FixedSizeStream(pre_master_secret, 48));
                                md5.update(FixedSizeStream(shasig, sizeof(shasig)));
                                md5.final(&master_secret[i * 16]);
                            }

                            for (int i = 0; i < 3; i++) {
                                uint8_t shasig[20];
                                uint8_t pad[4];
                                memset(pad, 'A' + i, i + 1);

                                SslSha1 sha1;
                                sha1.update(FixedSizeStream(pad, i + 1));
                                sha1.update(FixedSizeStream(master_secret, 48));
                                sha1.update(FixedSizeStream(lic.server_random, 32));
                                sha1.update(FixedSizeStream(client_random, 32));
                                sha1.final(shasig);

                                SslMd5 md5;
                                md5.update(FixedSizeStream(master_secret, 48));
                                md5.update(FixedSizeStream(shasig, sizeof(shasig)));
                                md5.final(&key_block[i * 16]);
                            }

                            /* Store first 16 bytes of session key as MAC secret */
                            memcpy(this->lic_layer_license_sign_key, key_block, 16);

                            // Generate RC4 key from next 16 bytes
                            // 16-byte transformation used to generate export keys (6.2.2).
                            SslMd5 md5;
                            md5.update(FixedSizeStream(key_block + 16, 16));
                            md5.update(FixedSizeStream(client_random, 32));
                            md5.update(FixedSizeStream(lic.server_random, 32));
                            md5.final(this->lic_layer_license_key);

                            BStream x224_header(256);
                            BStream mcs_header(256);
                            BStream sec_header(256);
                            BStream lic_data(65535);

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

                            SEC::Sec_Send sec(sec_header, lic_data, SEC::SEC_LICENSE_PKT, this->encrypt, 0, 0);
                            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                                          sec_header.size() + lic_data.size() , MCS::PER_ENCODING);
                            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + lic_data.size());

                            this->nego.trans->send(x224_header);
                            this->nego.trans->send(mcs_header);
                            this->nego.trans->send(sec_header);
                            this->nego.trans->send(lic_data);
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

                            BStream x224_header(256);
                            BStream mcs_header(256);
                            BStream sec_header(256);
                            BStream lic_data(65535);

                            LIC::ClientPlatformChallengeResponse_Send(lic_data, this->use_rdp5?3:2, out_token, crypt_hwid, out_sig);
                            SEC::Sec_Send sec(sec_header, lic_data, SEC::SEC_LICENSE_PKT, this->encrypt, 0, 0);
                            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                                          sec_header.size() + lic_data.size() , MCS::PER_ENCODING);
                            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + lic_data.size());

                            this->nego.trans->send(x224_header);
                            this->nego.trans->send(mcs_header);
                            this->nego.trans->send(sec_header);

                            this->nego.trans->send(lic_data);
                        }
                        break;
                    case LIC::NEW_LICENSE:
                    {
                        if (this->verbose & 2){
                            LOG(LOG_INFO, "Rdp::New License");
                        }

                        LIC::NewLicense_Recv lic(sec.payload, this->lic_layer_license_key);

                        TODO("CGR: Save license to keep a local copy of the license of a remote server thus avoiding to ask it every time we connect. Not obvious files is the best choice to do that")
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
                TODO("CGR: check if moving end is still necessary all data should have been consumed")
                if (sec.payload.p != sec.payload.end){
                    LOG(LOG_ERR, "all data should have been consumed %s:%u tag = %x", __FILE__, __LINE__, flic.tag);
                    throw Error(ERR_SEC);
                }
            }
            else {
                LOG(LOG_ERR, "Failed to get expected license negotiation PDU");
                hexdump(x224.payload.data, x224.payload.size());
//                throw Error(ERR_SEC);
                this->state = MOD_RDP_CONNECTED;
                sec.payload.p = sec.payload.end;
                hexdump(sec.payload.data, sec.payload.size());
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
            // client and server after the connection has been finalized include"
            // connection management information and virtual channel messages (exchanged
            // between client-side plug-ins and server-side applications).

        case MOD_RDP_CONNECTED:
        {
            // read tpktHeader (4 bytes = 3 0 len)
            // TPDU class 0    (3 bytes = LI F0 PDU_DT)

            BStream stream(65536);
            X224::RecvFactory f(*this->nego.trans, stream);
            X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
            SubStream & mcs_data = x224.payload;
            MCS::SendDataIndication_Recv mcs(mcs_data, MCS::PER_ENCODING);
            SEC::Sec_Recv sec(mcs.payload, false, this->decrypt, this->encryptionLevel, this->encryptionMethod);

            if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL){
                if (this->verbose & 16){
                    LOG(LOG_INFO, "received channel data on mcs.chanid=%u", mcs.channelId);
                }
                TODO("CGR: move this to channel_list.hpp")
                size_t num_channel_src = this->mod_channel_list.size();
                for (size_t index = 0; index < num_channel_src; index++){
                    const ChannelDef & mod_channel_item = this->mod_channel_list[index];
                    if (mcs.channelId == mod_channel_item.chanid){
                        num_channel_src = index;
                        break;
                    }
                }
                if (num_channel_src >= this->mod_channel_list.size()) {
                    LOG(LOG_WARNING, "mod::rdp::MOD_RDP_CONNECTED::Unknown Channel");
                    throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                }

                const ChannelDef & mod_channel = this->mod_channel_list[num_channel_src];
                if (this->verbose & 16){
                    mod_channel.log(num_channel_src);
                }

                uint32_t length = sec.payload.in_uint32_le();
                int flags = sec.payload.in_uint32_le();
                size_t chunk_size = sec.payload.in_remain();

                // If channel name is our virtual channel, then don't send data to front
                if (this->auth_channel[0] && this->sesman && !strcmp(mod_channel.name, this->auth_channel)){
                    const char * auth_channel_message = (const char *)sec.payload.p;
                    if (this->auth_channel_state == 0) {
                        this->auth_channel_flags = flags;
                        this->auth_channel_chanid = mod_channel.chanid;
                        if (strncmp("target:", auth_channel_message, 7)){
                            LOG(LOG_ERR, "Invalid request (%s)", auth_channel_message);
                            this->send_auth_channel_data("Error: Invalid request");
                        } else {
                            // Ask sesman for requested target
                            this->sesman->ask_auth_channel_target(auth_channel_message + 7);
                        }
                    }
                    else if (this->auth_channel_state == 1){
                        if (strncmp("result:", auth_channel_message, 7)){
                            LOG(LOG_ERR, "Invalid result (%s)", auth_channel_message);
                            auth_channel_message = "result:Session interrupted";
                        }
                        this->auth_channel_state = 0;
                        this->sesman->set_auth_channel_result(auth_channel_message + 7);
                    }
                }
                else if (!this->opt_clipboard && !strcmp(mod_channel.name, CLIPBOARD_VIRTUAL_CHANNEL_NAME)){
                    // Clipboard is unavailable and is a Clipboard PDU

                    TODO("RZ: Don't reject clipboard update, this can block rdesktop.")

                    if ( this->verbose ){
                        LOG( LOG_INFO, "mod_rdp clipboard PDU" );
                    }

                    uint16_t msgType = sec.payload.in_uint16_le();

                    if (msgType == ChannelDef::CB_FORMAT_LIST){
                        if ( this->verbose ){
                            LOG( LOG_INFO, "mod_rdp clipboard is unavailable" );
                        }

                        TODO("RZ: duplicate code")

                        // Build and send the CB_FORMAT_LIST_RESPONSE (with status = FAILED)
                        // 03 00 02 00 00 00 00 00

                        BStream out_s(256);
                        out_s.out_uint16_le(ChannelDef::CB_FORMAT_LIST_RESPONSE);   //  - MSG Type 2 bytes
                        out_s.out_uint16_le(ChannelDef::CB_RESPONSE_FAIL);          //  - MSG flags 2 bytes
                        out_s.out_uint32_le(0);                                     //  - remaining datalen of message
                        out_s.mark_end();

                        const ChannelDef * mod_channel = this->mod_channel_list.get(CLIPBOARD_VIRTUAL_CHANNEL_NAME);

                        if (mod_channel){
                            this->send_to_channel(*mod_channel,
                                                  out_s,
                                                  out_s.size(),
                                                  ChannelDef::CHANNEL_FLAG_FIRST | ChannelDef::CHANNEL_FLAG_LAST);
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
                    ShareControl sctrl(sec.payload);
                    sctrl.recv_begin();
                    next_packet += sctrl.len;

                    if (this->verbose & 128){
                        LOG(LOG_WARNING, "LOOPING on PDUs: %u", (unsigned)sctrl.len);
                    }


                    switch (sctrl.pdu_type1) {
                    case PDUTYPE_DATAPDU:
                        if (this->verbose & 128){
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
                            TODO("CGR: Data should actually be consumed")
                            sctrl.payload.p = sctrl.payload.end;
                            this->connection_finalization_state = WAITING_CTL_COOPERATE;
                        break;
                        case WAITING_CTL_COOPERATE:
                            if (this->verbose & 1){
                                LOG(LOG_WARNING, "WAITING_CTL_COOPERATE");
                            }
                            TODO("CGR: Data should actually be consumed")
                            sctrl.payload.p = sctrl.payload.end;
//                            this->check_data_pdu(PDUTYPE2_CONTROL);
                            this->connection_finalization_state = WAITING_GRANT_CONTROL_COOPERATE;
                        break;
                        case WAITING_GRANT_CONTROL_COOPERATE:
                            if (this->verbose & 1){
                                LOG(LOG_WARNING, "WAITING_GRANT_CONTROL_COOPERATE");
                            }
                            TODO("CGR: Data should actually be consumed")
                            sctrl.payload.p = sctrl.payload.end;
//                            this->check_data_pdu(PDUTYPE2_CONTROL);
                            this->connection_finalization_state = WAITING_FONT_MAP;
                        break;
                        case WAITING_FONT_MAP:
                            if (this->verbose & 1){
                                LOG(LOG_WARNING, "PDUTYPE2_FONTMAP");
                            }
                            TODO("CGR: Data should actually be consumed")
                            sctrl.payload.p = sctrl.payload.end;
//                            this->check_data_pdu(PDUTYPE2_FONTMAP);
                            this->connection_finalization_state = UP_AND_RUNNING;

                            // Synchronize sent to indicate server the state of sticky keys (x-locks)
                            // Must be sent at this point of the protocol (sent before, it xwould be ignored or replaced)
                            rdp_input_synchronize(0, 0, (this->key_flags & 0x07), 0);

                        break;
                        case UP_AND_RUNNING:
                        {
                            ShareData sdata(sctrl.payload);
                            sdata.recv_begin();
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

                                int update_type = sdata.payload.in_uint16_le();
                                switch (update_type) {
                                case RDP_UPDATE_ORDERS:
                                    if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_ORDERS");}
                                    {
                                        sdata.payload.in_skip_bytes(2); /* pad */
                                        int count = sdata.payload.in_uint16_le();
                                        sdata.payload.in_skip_bytes(2); /* pad */
                                        this->front.begin_update();
                                        this->orders.process_orders(this->bpp, sdata.payload, count, this);
                                        this->front.end_update();
                                    }
                                    break;
                                case RDP_UPDATE_BITMAP:
                                    if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_BITMAP");}
                                    this->front.begin_update();
                                    this->process_bitmap_updates(sdata.payload, this);
                                    this->front.end_update();
                                    break;
                                case RDP_UPDATE_PALETTE:
                                    if (this->verbose & 8){ LOG(LOG_INFO, "RDP_UPDATE_PALETTE");}
                                    this->front.begin_update();
                                    this->process_palette(sdata.payload, this);
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
                                TODO("CGR: Data should actually be consumed")
                                sdata.payload.p = sdata.payload.end;
                            break;
                            case PDUTYPE2_SYNCHRONIZE:
                                if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");}
                                TODO("CGR: Data should actually be consumed")
                                sdata.payload.p = sdata.payload.end;
                            break;
                            case PDUTYPE2_POINTER:
                                if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_POINTER");}
                                this->process_pointer_pdu(sdata.payload, this);
                                TODO("CGR: Data should actually be consumed")
                                sdata.payload.p = sdata.payload.end;
                            break;
                            case PDUTYPE2_PLAY_SOUND:
                                if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_PLAY_SOUND");}
                                TODO("CGR: Data should actually be consumed")
                                sdata.payload.p = sdata.payload.end;
                            break;
                            case PDUTYPE2_SAVE_SESSION_INFO:
                                if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SAVE_SESSION_INFO");}
                                TODO("CGR: Data should actually be consumed")
                                sdata.payload.p = sdata.payload.end;
                            break;
                            case PDUTYPE2_SET_ERROR_INFO_PDU:
                                if (this->verbose & 8){ LOG(LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");}
                                this->process_disconnect_pdu(sdata.payload);
                            break;
                            default:
                                LOG(LOG_WARNING, "PDUTYPE2 unsupported tag=%u", sdata.pdutype2);
                                TODO("CGR: Data should actually be consumed")
                                sdata.payload.p = sdata.payload.end;
                            break;
                            }
                            sdata.recv_end();
                            sctrl.payload.p = sdata.payload.end;
                        }
                        break;
                    }
                    break;
                    case PDUTYPE_DEMANDACTIVEPDU:
                        {
                            if (this->verbose & 128){ LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU"); }
                            client_mod * mod = this;
                            this->share_id = sctrl.payload.in_uint32_le();
                            uint16_t lengthSourceDescriptor = sctrl.payload.in_uint16_le();
                            uint16_t lengthCombinedCapabilities = sctrl.payload.in_uint16_le();
                            sctrl.payload.in_skip_bytes(lengthSourceDescriptor);
                            this->process_server_caps(sctrl.payload, lengthCombinedCapabilities);
                            uint32_t sessionId = sctrl.payload.in_uint32_le();

                            this->send_confirm_active(mod);
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
                        TODO("CGR: Data should actually be consumed")
                        sctrl.payload.p = sctrl.payload.end;
                        sctrl.recv_end();
                        TODO("CGR: Check we are indeed expecting Synchronize... dubious")
                        this->connection_finalization_state = WAITING_SYNCHRONIZE;
                    break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        if (this->verbose & 128){ LOG(LOG_INFO, "PDUTYPE_SERVER_REDIR_PKT"); }
                    break;
                    default:
                        LOG(LOG_INFO, "unknown PDU %u", sctrl.pdu_type1);
                        break;
                    }
                    sctrl.recv_end();
                }
            }
        }
        }
        }
        catch(Error e){
            BStream stream(256);
            X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
            try {
                this->nego.trans->send(stream);
                LOG(LOG_INFO, "Connection to server closed");
            }
            catch(Error e){
                LOG(LOG_INFO, "Connection to server Already closed", e.id);
            };
            return BACK_EVENT_NEXT;
        }
        return BACK_EVENT_NONE;
    }


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

        // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets include" in the Demand Active PDU.

        // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

        // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number of capability sets is specified by the numberCapabilities field.

        // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.


        void send_confirm_active(client_mod * mod) throw(Error)
        {
            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::send_confirm_active");
            }

            BStream stream(65536);

            // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
            // containing information about the packet. The type subfield of the pduType
            // field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_CONFIRMACTIVEPDU, this->userid + GCC::MCS_USERCHANNEL_BASE);

            // shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for
            // the packet (see [T128] section 8.4.2 for more information regarding share IDs).

            // Payload
            stream.out_uint32_le(this->share_id);
            stream.out_uint16_le(1002);

            // lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes
            // of the sourceDescriptor field.
            stream.out_uint16_le(5);

            // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined
            // size in bytes of the numberCapabilities, pad2Octets, and capabilitySets fields.
            uint16_t offset_caplen = stream.get_offset();
            stream.out_uint16_le(0); // caplen

            // sourceDescriptor (variable): A variable-length array of bytes containing a
            // source descriptor (see [T128] section 8.4.1 for more information regarding
            // source descriptors).
            stream.out_copy_bytes("MSTSC", 5);

            // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of
            // capability sets include" in the Demand Active PDU.
            uint16_t offset_capscount = stream.get_offset();
            uint16_t capscount = 0;
            stream.out_uint16_le(0); /* num_caps */

            // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in
            // this field MUST be ignored.
            stream.out_clear_bytes(2); /* pad */

            // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1)
            // structures. The number of capability sets is specified by the numberCapabilities field.
            uint16_t total_caplen = stream.get_offset();

            GeneralCaps general_caps;
            general_caps.extraflags = this->use_rdp5 ? NO_BITMAP_COMPRESSION_HDR|AUTORECONNECT_SUPPORTED|LONG_CREDENTIALS_SUPPORTED:0;
            general_caps.log("Sending to server");
            general_caps.emit(stream);
            stream.mark_end();
            capscount++;

            BitmapCaps bitmap_caps;
            bitmap_caps.preferredBitsPerPixel = this->bpp;
            bitmap_caps.desktopWidth = this->front_width;
            bitmap_caps.desktopHeight = this->front_height;
            bitmap_caps.bitmapCompressionFlag = this->bitmap_compression;
            bitmap_caps.log("Sending bitmap caps to server");
            bitmap_caps.emit(stream);
            stream.mark_end();
            capscount++;

            OrderCaps order_caps;
            order_caps.numberFonts = 0x147;
            order_caps.orderFlags = 0x2a;
            order_caps.orderSupport[TS_NEG_DSTBLT_INDEX] = 1;
            order_caps.orderSupport[TS_NEG_PATBLT_INDEX] = 1;
            order_caps.orderSupport[TS_NEG_SCRBLT_INDEX] = 1;
            order_caps.orderSupport[TS_NEG_MEMBLT_INDEX] = 1;
            order_caps.orderSupport[TS_NEG_LINETO_INDEX] = 1;
            order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = 1;
            order_caps.orderSupport[UnusedIndex3] = 1;
            order_caps.orderSupport[UnusedIndex5] = 1;
            order_caps.orderSupport[TS_NEG_INDEX_INDEX] = 1;
            order_caps.textFlags = 0x06a1;
            order_caps.textANSICodePage = 0x4e4; // Windows-1252 code"page is passed (latin-1)
            order_caps.log("Sending order caps to server");
            order_caps.emit(stream);
            stream.mark_end();
            capscount++;

            BmpCacheCaps bmpcache_caps;
            bmpcache_caps.cache0Entries = 0x258;
            bmpcache_caps.cache0MaximumCellSize = nbbytes(this->bpp) * 0x100;
            bmpcache_caps.cache1Entries = 0x12c;
            bmpcache_caps.cache1MaximumCellSize = nbbytes(this->bpp) * 0x400;
            bmpcache_caps.cache2Entries = 0x106;
            bmpcache_caps.cache2MaximumCellSize = nbbytes(this->bpp) * 0x1000;
            bmpcache_caps.log("Sending bmpcache caps to server");
            bmpcache_caps.emit(stream);
            stream.mark_end();
            capscount++;

//            if(this->use_rdp5){
//                BmpCache2Caps bmpcache2_caps;
//                bmpcache2_caps.numCellCaches = 3;
//                bmpcache2_caps.bitmapCache0CellInfo = 2000;
//                bmpcache2_caps.bitmapCache1CellInfo = 2000;
//                bmpcache2_caps.bitmapCache2CellInfo = 2000;
//                bmpcache2_caps.emit(stream);
//                stream.mark_end();
//                capscount++;
//            }

            ColorCacheCaps colorcache_caps;
            colorcache_caps.log("Sending colorcache caps to server");
            colorcache_caps.emit(stream);
            stream.mark_end();
            capscount++;

            ActivationCaps activation_caps;
            activation_caps.log("Sending activation caps to server");
            activation_caps.emit(stream);
            stream.mark_end();
            capscount++;

            ControlCaps control_caps;
            control_caps.log("Sending control caps to server");
            control_caps.emit(stream);
            stream.mark_end();
            capscount++;

            PointerCaps pointer_caps;
            pointer_caps.len = 10;
            if (this->enable_new_pointer == false) {
                pointer_caps.pointerCacheSize = 0;
                pointer_caps.colorPointerCacheSize = 20;
                pointer_caps.len = 8;
            }
            pointer_caps.log("Sending pointer caps to server");
            pointer_caps.emit(stream);
            stream.mark_end();
            capscount++;

            ShareCaps share_caps;
            share_caps.log("Sending share caps to server");
            share_caps.emit(stream);
            stream.mark_end();
            capscount++;

            InputCaps input_caps;
            input_caps.log("Sending input caps to server");
            input_caps.emit(stream);
            stream.mark_end();
            capscount++;

            SoundCaps sound_caps;
            sound_caps.log("Sending sound caps to server");
            sound_caps.emit(stream);
            stream.mark_end();
            capscount++;

            FontCaps font_caps;
            font_caps.log("Sending font caps to server");
            font_caps.emit(stream);
            stream.mark_end();
            capscount++;

            GlyphSupportCaps glyphsupport_caps;
            glyphsupport_caps.log("Sending glyphsupport caps to server");
            glyphsupport_caps.emit(stream);
            stream.mark_end();
            capscount++;

//            BrushCacheCaps brushcache_caps;
//            brushcache_caps.log("Sending brushcache caps to server");
//            brushcache_caps.emit(stream);
//            stream.mark_end();
//            capscount++;

//            CompDeskCaps compdesk_caps;
//            compdesk_caps.log("Sending compdesk caps to server");
//            compdesk_caps.emit(stream);
//            stream.mark_end();
//            capscount++;

            TODO("CGR: Check caplen here")
            total_caplen = stream.get_offset() - total_caplen;

            // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.
            stream.out_uint32_le(0);
            stream.mark_end();

            stream.set_out_uint16_le(total_caplen + 4, offset_caplen); // caplen
            stream.set_out_uint16_le(capscount, offset_capscount); // caplen

            // Packet trailer
            sctrl.emit_end();

            BStream x224_header(256);
            BStream mcs_header(256);
            BStream sec_header(256);
            SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header);
            this->nego.trans->send(mcs_header);
            this->nego.trans->send(sec_header);
            this->nego.trans->send(stream);

            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::send_confirm_active done");
                LOG(LOG_INFO, "Waiting for answer to confirm active");
            }
        }

        void process_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
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
                this->process_cached_pointer_pdu(stream, mod);
                if (this->verbose & 4){
                    LOG(LOG_INFO, "Process pointer cached done");
                }
                break;
            case RDP_POINTER_COLOR:
                if (this->verbose & 4){
                    LOG(LOG_INFO, "Process pointer color");
                }
                this->process_system_pointer_pdu(stream, mod);
                if (this->verbose & 4){
                    LOG(LOG_INFO, "Process pointer system done");
                }
                break;
                this->process_color_pointer_pdu(stream, mod);
                if (this->verbose & 4){
                    LOG(LOG_INFO, "Process pointer color done");
                }
                break;
            case RDP_POINTER_NEW:
                if (this->verbose & 4){
                    LOG(LOG_INFO, "Process pointer new");
                }
                if (enable_new_pointer) {
                    this->process_new_pointer_pdu(stream, mod); // Pointer with arbitrary color depth
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
                TODO("CGR: implement RDP_POINTER_MOVE")
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

        void process_palette(Stream & stream, client_mod * mod)
        {
            if (this->verbose & 4){
                LOG(LOG_INFO, "mod_rdp::process_palette");
            }

            stream.in_skip_bytes(2); /* pad */
            TODO("CGR: check that : why am I reading 32 bits into 16 bits ?")
            uint16_t numberColors = stream.in_uint32_le();
            assert(numberColors == 256);
            for (int i = 0; i < numberColors; i++) {
                uint8_t r = stream.in_uint8();
                uint8_t g = stream.in_uint8();
                uint8_t b = stream.in_uint8();
//                uint32_t color = stream.in_bytes_le(3);
                this->orders.global_palette[i] = (r << 16)|(g << 8)|b;
                this->orders.memblt_palette[i] = (b << 16)|(g << 8)|r;
            }
            mod->front.set_mod_palette(this->orders.global_palette);
            if (this->verbose & 4){
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
// |                                          | the Extended Info Packet       |
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
// | 0x0000112C ERRINFO_RESERVED              | Reserved for future use.       |
// +------------------------------------------+--------------------------------+
// | 0x0000112D                               | The graphics mode requested by |
// | ERRINFO_GRAPHICSMODENOTSUPPORTED         | the client is not supported by |
// |                                          | the server.                    |
// +------------------------------------------+--------------------------------+
// | 0x0000112E                               | The server-side graphics       |
// | ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED     | subsystem failed to reset.     |
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
            ERRINFO_RESERVED                          = 0x0000112C,
            ERRINFO_GRAPHICSMODENOTSUPPORTED          = 0x0000112D,
            ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED      = 0x0000112E,
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
            case ERRINFO_RESERVED:
                LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "RESERVED");
            break;
            case ERRINFO_GRAPHICSMODENOTSUPPORTED:
                LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "GRAPHICSMODENOTSUPPORTED");
            break;
            case ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED:
                LOG(LOG_INFO, "process disconnect pdu : code = %8x error=%s", errorInfo, "GRAPHICSSUBSYSTEMRESETFAILED");
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


        TODO("CGR: this can probably be unified with process_confirm_active in front");
        void process_server_caps(Stream & stream, uint16_t len)
        {
            if (this->verbose & 32){
                LOG(LOG_INFO, "mod_rdp::process_server_caps");
            }
            uint16_t ncapsets = stream.in_uint16_le();
            stream.in_skip_bytes(2); /* pad */
            for (uint16_t n = 0; n < ncapsets; n++) {
                uint16_t capset_type = stream.in_uint16_le();
                uint16_t capset_length = stream.in_uint16_le();
                uint8_t * next = (stream.p + capset_length) - 4;
                switch (capset_type) {
                case CAPSTYPE_GENERAL:
                {
                    GeneralCaps general_caps;
                    general_caps.recv(stream, capset_length);
                    general_caps.log("Received from server");
                }
                break;
                case CAPSTYPE_BITMAP:
                {
                    BitmapCaps bitmap_caps;
                    bitmap_caps.recv(stream, capset_length);
                    bitmap_caps.log("Received from server");
                    this->bpp = bitmap_caps.preferredBitsPerPixel;
                    this->front_width = bitmap_caps.desktopWidth;
                    this->front_height = bitmap_caps.desktopHeight;
                }
                break;
                case CAPSTYPE_ORDER:
                {
                    OrderCaps order_caps;
                    order_caps.recv(stream, capset_length);
                    order_caps.log("Received from server");
                    break;
                }
                default:
                    break;
                }
                stream.p = next;
            }
            if (this->verbose & 32){
                LOG(LOG_INFO, "mod_rdp::process_server_caps done");
            }
        }


        void send_control(int action) throw (Error)
        {
            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::send_control");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE);
            ShareData sdata(stream);
            sdata.emit_begin(PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

            // Payload
            stream.out_uint16_le(action);
            stream.out_uint16_le(0); /* userid */
            stream.out_uint32_le(0); /* control id */
            stream.mark_end();

            // Packet trailer
            sdata.emit_end();
            sctrl.emit_end();

            BStream x224_header(256);
            BStream mcs_header(256);
            BStream sec_header(256);
            SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header);
            this->nego.trans->send(mcs_header);
            this->nego.trans->send(sec_header);
            this->nego.trans->send(stream);

            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::send_control done");
            }
        }

        TODO("CGR: duplicated code in front")
        void send_synchronise() throw (Error)
        {
            if (this->verbose & 1){
                LOG(LOG_INFO, "mod_rdp::send_synchronise");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE);
            ShareData sdata(stream);
            sdata.emit_begin(PDUTYPE2_SYNCHRONIZE, this->share_id, RDP::STREAM_MED);

            // Payload
            stream.out_uint16_le(1); /* type */
            stream.out_uint16_le(1002);
            stream.mark_end();

            // Packet trailer
            sdata.emit_end();
            sctrl.emit_end();

            BStream x224_header(256);
            BStream mcs_header(256);
            BStream sec_header(256);
            SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header);
            this->nego.trans->send(mcs_header);
            this->nego.trans->send(sec_header);
            this->nego.trans->send(stream);

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
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE);
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
            sctrl.emit_end();

            BStream x224_header(256);
            BStream mcs_header(256);
            BStream sec_header(256);
            SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header);
            this->nego.trans->send(mcs_header);
            this->nego.trans->send(sec_header);
            this->nego.trans->send(stream);

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

        void enum_bmpcache2()
        {

        }

        void send_input(int time, int message_type,
                        int device_flags, int param1, int param2) throw(Error)
        {
            if (this->verbose & 4){
                LOG(LOG_INFO, "mod_rdp::send_input");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE);
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
            sctrl.emit_end();

            BStream x224_header(256);
            BStream mcs_header(256);
            BStream sec_header(256);
            SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header);
            this->nego.trans->send(mcs_header);
            this->nego.trans->send(sec_header);
            this->nego.trans->send(stream);

            if (this->verbose & 4){
                LOG(LOG_INFO, "mod_rdp::send_input done");
            }
        }

        virtual void rdp_input_invalidate(const Rect & r)
        {
            if (this->verbose & 4){
                LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate");
            }
            if (UP_AND_RUNNING == this->connection_finalization_state) {
                if (!r.isempty()){
                    BStream stream(65536);
                    ShareControl sctrl(stream);
                    sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE);
                    ShareData sdata(stream);
                    sdata.emit_begin(PDUTYPE2_REFRESH_RECT, this->share_id, RDP::STREAM_MED);

                   // Payload
                    stream.out_uint32_le(1);
                    stream.out_uint16_le(r.x);
                    stream.out_uint16_le(r.y);
                    TODO("CGR: check this -1 (difference between rect and clip)")
                    stream.out_uint16_le(r.cx - 1);
                    stream.out_uint16_le(r.cy - 1);
                    stream.mark_end();

                    // Packet trailer
                    sdata.emit_end();
                    sctrl.emit_end();

                    BStream x224_header(256);
                    BStream mcs_header(256);
                    BStream sec_header(256);
                    SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
                    MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                                  sec_header.size() + stream.size() , MCS::PER_ENCODING);
                    X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

                    this->nego.trans->send(x224_header);
                    this->nego.trans->send(mcs_header);
                    this->nego.trans->send(sec_header);
                    this->nego.trans->send(stream);
                }
            }
            if (this->verbose & 4){
                LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate done");
            }
        }

    void process_color_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu");
        }
        unsigned cache_idx = stream.in_uint16_le();
        if (cache_idx >= (sizeof(this->cursors) / sizeof(this->cursors[0]))) {
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_CACHE_NOT_OK);
        }
        struct rdp_cursor* cursor = this->cursors + cache_idx;
        
        TODO("CGR: move that to rdp_cursor")
        
        cursor->x = stream.in_uint16_le();
        cursor->y = stream.in_uint16_le();
        cursor->width = stream.in_uint16_le();
        cursor->height = stream.in_uint16_le();
        unsigned mlen = stream.in_uint16_le(); /* mask length */
        unsigned dlen = stream.in_uint16_le(); /* data length */

        if ((mlen > sizeof(cursor->mask)) || (dlen > sizeof(cursor->data))) {
            LOG(LOG_WARNING, "mod_rdp::Bad length for color pointer mask_len=%u data_len=%u",
                (unsigned)mlen, (unsigned)dlen);
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }
        memcpy(cursor->data, stream.in_uint8p(dlen), dlen);
        memcpy(cursor->mask, stream.in_uint8p(mlen), mlen);

        mod->front.server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu done");
        }
    }

    void process_cached_pointer_pdu(Stream & stream, client_mod * mod)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu");
        }

        int cache_idx = stream.in_uint16_le();
        if (cache_idx < 0){
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_LESS_0);
        }
        if (cache_idx >= (int)(sizeof(this->cursors) / sizeof(rdp_cursor))) {
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        struct rdp_cursor* cursor = this->cursors + cache_idx;
        mod->front.server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu done");
        }
    }

    void process_system_pointer_pdu(Stream & stream, client_mod * mod)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu");
        }
        int system_pointer_type = stream.in_uint16_le();
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                struct rdp_cursor cursor;
                memset(cursor.mask, 0xff, sizeof(cursor.mask));
                TODO("CGR: we should pass in a cursor to set_pointer instead of individual fields")
                mod->front.server_set_pointer(cursor.x, cursor.y, cursor.data, cursor.mask);
                mod->set_pointer_display();
            }
            break;
        default:
            break;
        }
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu done");
        }
    }

    void to_regular_mask(Stream & stream, unsigned mlen, uint8_t bpp, uint8_t * mask, size_t mask_size)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::to_regular_mask");
        }
        TODO("CGR: we should ensure we have data enough to create mask")
        uint8_t * end = stream.p + mlen;
        switch (bpp) {
        case 1 : {
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
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::to_regular_mask");
        }
    }

    void to_regular_pointer(Stream & stream, unsigned dlen, uint8_t bpp, uint8_t * data, size_t target_data_len)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::to_regular_pointer");
        }
        uint8_t * end = stream.p + dlen;
        TODO("CGR: we should ensure we have data enough to create pointer")
        switch (bpp) {
        case 1 : 
        {
            for (unsigned x = 0; x < dlen ; x ++) {
                BGRColor px = stream.in_uint8();
                // target cursor will receive 8 bits input at once
                for (unsigned b = 0 ; b < 8 ; b++){
                    // incoming new pointer is upside down, revert it
                    uint8_t * bstart = &(data[24 * (128 - 4 - (x & 0xFFFC) + (x & 3))]);
                    // emit all individual bits
                    ::out_bytes_le(bstart ,      3, (px & 0x80)?0xFFFFFF:0);
                    ::out_bytes_le(bstart +  3,  3, (px & 0x40)?0xFFFFFF:0);
                    ::out_bytes_le(bstart +  6,  3, (px & 0x20)?0xFFFFFF:0);
                    ::out_bytes_le(bstart +  9 , 3, (px & 0x10)?0xFFFFFF:0);
                    ::out_bytes_le(bstart + 12 , 3, (px &    8)?0xFFFFFF:0);
                    ::out_bytes_le(bstart + 15 , 3, (px &    4)?0xFFFFFF:0);
                    ::out_bytes_le(bstart + 18 , 3, (px &    2)?0xFFFFFF:0);
                    ::out_bytes_le(bstart + 21 , 3, (px &    1)?0xFFFFFF:0);
                }
            }
        }
        break;
        case 4 : 
        {
            for (unsigned i=0; i < dlen ; i++) {
                BGRColor px = stream.in_uint8();
                // target cursor will receive 8 bits input at once
                ::out_bytes_le(&(data[6 *i]),     3, color_decode((px >> 4) & 0xF, bpp, this->orders.global_palette));
                ::out_bytes_le(&(data[6 *i + 3]), 3, color_decode(px        & 0xF, bpp, this->orders.global_palette));
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
            for (size_t x = 0 ; x < 1024 ; x++){
                ::out_bytes_le(data + x *3, 3, GREEN);
            }
            break;
        }

        stream.p = end;
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::to_regular_pointer");
        }
    }


    void process_new_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu");
        }
        
        unsigned data_bpp = stream.in_uint16_le(); /* data bpp */
        unsigned cache_idx = stream.in_uint16_le();
        if (cache_idx >= (sizeof(this->cursors) / sizeof(this->cursors[0]))) {
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_CACHE_NOT_OK);
        }

        struct rdp_cursor* cursor = this->cursors + cache_idx;
        cursor->x = stream.in_uint16_le();
        cursor->y = stream.in_uint16_le();
        cursor->width = stream.in_uint16_le();
        cursor->height = stream.in_uint16_le();
        unsigned mlen = stream.in_uint16_le(); /* mask length */
        unsigned dlen = stream.in_uint16_le(); /* data length */

        size_t out_data_len = (bpp == 1) ? (cursor->width * cursor->height) / 8 :
                              (bpp == 4) ? (cursor->width * cursor->height) / 2 :
                                           (dlen * 3) / nbbytes(data_bpp) ;

        if ((mlen > sizeof(cursor->mask)) || (out_data_len > sizeof(cursor->data))) {
            LOG(LOG_WARNING, "mod_rdp::Bad length for color pointer mask_len=%u data_len=%u Width = %u Height = %u bpp = %u out_data_len = %u nbbytes=%u",
                (unsigned)mlen, (unsigned)dlen, cursor->width, cursor->height, data_bpp, out_data_len, nbbytes(data_bpp));
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
        }

        to_regular_pointer(stream, dlen, data_bpp, cursor->data, sizeof(cursor->data)); 
        to_regular_mask(stream, mlen, data_bpp, cursor->mask, sizeof(cursor->mask)); 

        mod->front.server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
        if (this->verbose & 4){
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu done");
        }
    }

    void process_bitmap_updates(Stream & stream, client_mod * mod)
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates");
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
            const uint16_t left = stream.in_uint16_le();

            // A 16-bit, unsigned integer. Top bound of the rectangle.
            const uint16_t top = stream.in_uint16_le();

            // A 16-bit, unsigned integer. Right bound of the rectangle.
            const uint16_t right = stream.in_uint16_le();

            // A 16-bit, unsigned integer. Bottom bound of the rectangle.
            const uint16_t bottom = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The width of the rectangle.
            const uint16_t width = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The height of the rectangle.
            const uint16_t height = stream.in_uint16_le();

            // A 16-bit, unsigned integer. The color depth of the rectangle
            // data in bits-per-pixel.
            uint8_t bpp = (uint8_t)stream.in_uint16_le();

            // CGR: As far as I understand we should have
            // align4(right-left) == width and bottom-top == height
            // maybe put some assertion to check it's true
            // LOG(LOG_ERR, "left=%u top=%u right=%u bottom=%u width=%u height=%u bpp=%u", left, top, right, bottom, width, height, bpp);

            assert(bpp == 24 || bpp == 16 || bpp == 8 || bpp == 15);

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

            int flags = stream.in_uint16_le();
            uint16_t size = stream.in_uint16_le();

            Rect boundary(left, top, right - left + 1, bottom - top + 1);

            // BITMAP_COMPRESSION 0x0001
            // Indicates that the bitmap data is compressed. This implies
            // that the bitmapComprHdr field is present if the
            // NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not set.

            if (this->verbose & 64){
                LOG(LOG_INFO, "/* Rect [%d] bpp=%d width=%d height=%d b(%d, %d, %d, %d) */",
                    i, bpp, width, height, boundary.x, boundary.y, boundary.cx, boundary.cy);
            }

            bool compressed = false;
            uint16_t line_size = 0;
            uint16_t final_size = 0;
            if (flags & 0x0001){
                if (!(flags & 0x400)) {
                // bitmapComprHdr (8 bytes): Optional Compressed Data Header
                // structure (see Compressed Data Header (TS_CD_HEADER)
                // (section 2.2.9.1.1.3.1.2.3)) specifying the bitmap data
                // in the bitmapDataStream. This field MUST be present if
                // the BITMAP_COMPRESSION (0x0001) flag is present in the
                // Flags field, but the NO_BITMAP_COMPRESSION_HDR (0x0400)
                // flag is not.
                    // bitmapComprHdr
                    stream.in_skip_bytes(2); /* pad */
                    size = stream.in_uint16_le();
                    line_size = stream.in_uint16_le();
                    final_size = stream.in_uint16_le();
                }

                if (width <= 0 || height <= 0){
                    LOG(LOG_WARNING, "Unexpected bitmap size : width=%d height=%d size=%u left=%u, top=%u, right=%u, bottom=%u",
                        width, height, size, left, top, right, bottom);
                }
                compressed = true;
            }

            TODO("CGR: check which sanity checks should be done")
//            if (bufsize != bitmap.bmp_size){
//                LOG(LOG_WARNING, "Unexpected bufsize in bitmap received [%u != %u] width=%u height=%u bpp=%u",
//                    bufsize, bitmap.bmp_size, width, height, bpp);
//            }
            const uint8_t * data = stream.in_uint8p(size);
            Bitmap bitmap(bpp, &this->orders.global_palette, width, height, data, size, compressed);
            if (line_size && (line_size - bitmap.line_size)>= nbbytes(bitmap.original_bpp)){
                LOG(LOG_WARNING, "Bad line size: line_size=%u width=%u height=%u bpp=%u",
                   line_size, width, height, bpp);
            }

            if (final_size && final_size != bitmap.bmp_size){
                LOG(LOG_WARNING, "final_size should be size of decompressed bitmap [%u != %u] width=%u height=%u bpp=%u",
                    final_size, bitmap.bmp_size, width, height, bpp);
            }
            mod->front.draw(RDPMemBlt(0, boundary, 0xCC, 0, 0, 0), boundary, bitmap);
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
        BStream stream(1024);

/*
        uint32_t perfFlags = (this->performanceFlags ? this->performanceFlags :
            ( PERF_DISABLE_WALLPAPER | this->nego.tls * ( PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS ) ) );
*/
        InfoPacket infoPacket( this->use_rdp5
                             , this->domain
                             , this->username
                             , password
                             , this->program
                             , this->directory
//                             , perfFlags
                             , this->performanceFlags
                             , this->clientAddr
                             );

        infoPacket.log("Sending to server: ");
        infoPacket.emit(stream);
        stream.mark_end();

        BStream x224_header(256);
        BStream mcs_header(256);
        BStream sec_header(256);

        SEC::Sec_Send sec(sec_header, stream, SEC::SEC_INFO_PKT, this->encrypt, this->encryptionLevel, this->encryptionMethod);
        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, GCC::MCS_GLOBAL_CHANNEL, 1, 3,
                                      sec_header.size() + stream.size() , MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

        this->nego.trans->send(x224_header);
        this->nego.trans->send(mcs_header);
        this->nego.trans->send(sec_header);
        this->nego.trans->send(stream);

        if (this->verbose & 1){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu done");
        }
    }

};

#endif
