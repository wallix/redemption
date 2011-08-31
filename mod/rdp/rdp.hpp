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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rdp module main header file

*/

#if !defined(__CLIENT_RDP_HPP__)
#define __CLIENT_RDP_HPP__

#include <unistd.h>

#include <netdb.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdlib.h>

/* include other h files */
#include "stream.hpp"
#include "ssl_calls.hpp"
#include "constants.hpp"
#include "client_mod.hpp"
#include "log.hpp"

#include "rdp_rdp.hpp"

struct mod_rdp : public client_mod {

    /* mod data */
    struct rdp_rdp rdp_layer;
    int up_and_running;
    Stream in_stream;
    Transport *trans;
    struct vector<mcs_channel_item*> front_channel_list;
    bool dev_redirection_enable;
    struct ModContext & context;
    wait_obj & event;
    int use_rdp5;
    int keylayout;

    enum {
        MOD_RDP_CONNECTING,
        MOD_RDP_CONNECTED,
    };

    int state;

    mod_rdp(Transport * trans, wait_obj & event,
            int (& keys)[256], int & key_flags, Keymap * &keymap,
            struct ModContext & context, struct Front & front,
            vector<mcs_channel_item*> channel_list,
            const char * hostname, int keylayout,
            bool clipboard_enable, bool dev_redirection_enable)
            :
                client_mod(keys, key_flags, keymap, front),
                  rdp_layer(this, trans,
                    context.get(STRAUTHID_TARGET_USER),
                    context.get(STRAUTHID_TARGET_PASSWORD),
                    hostname, channel_list,
                    this->get_client_info().rdp5_performanceflags,
                    this->get_front_width(),
                    this->get_front_height(),
                    this->get_front_bpp(),
                    keylayout,
                    this->get_client_info().console_session),
                    in_stream(8192),
                    trans(trans),
                    context(context),
                    event(event),
                    use_rdp5(0),
                    keylayout(keylayout),
                    state(MOD_RDP_CONNECTING)
        {
        // copy channel list from client.
        // It will be changed after negotiation with server
        // to hold only channels actually supported.
        this->front_channel_list = channel_list;
        this->up_and_running = 0;
        /* clipboard allow us to deactivate copy/paste sequence from server
        to client communication. This is allowed by default */
        this->clipboard_enable = clipboard_enable;
        this->dev_redirection_enable = dev_redirection_enable;
        this->mod_signal();
    }

    virtual ~mod_rdp() {
        delete this->trans;
    }

    virtual void scancode(long param1, long param2, long device_flags, long time, int & key_flags, Keymap & keymap, int keys[]){
        long p1 = param1 % 128;
        int msg = WM_KEYUP;
        keys[p1] = 1 | device_flags;
        if ((device_flags & KBD_FLAG_UP) == 0) { /* 0x8000 */
            /* key down */
            msg = WM_KEYDOWN;
            switch (p1) {
            case 58:
                key_flags ^= 4;
                break; /* caps lock */
            case 69:
                key_flags ^= 2;
                break; /* num lock */
            case 70:
                key_flags ^= 1;
                break; /* scroll lock */
            default:
                ;
            }
        }
        if (this->up_and_running) {
//            LOG(LOG_INFO, "Direct parameter transmission \n");
            Stream stream = Stream(8192 * 2);
//            LOG(LOG_INFO, "resend input: time=%lu device_flags=%lu param1=%lu param2=%lu\n", time, device_flags, param1, param2);
            this->rdp_layer.send_input(stream, time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
        }
        if (msg == WM_KEYUP){
            keys[param1] = 0;
        }
    }

    #warning most of code below should move to rdp_rdp
    virtual int mod_event(int msg, long param1, long param2, long param3, long param4)
    {
        try{
            if (!this->up_and_running) {
                LOG(LOG_INFO, "Not up and running\n");
                return 0;
            }
            Stream stream = Stream(8192 * 2);
            switch (msg) {
            case WM_KEYDOWN:
            case WM_KEYUP:
                #warning bypassed by call to scancode, need some code cleanup here, we would probably be better of with less key decoding.
                assert(false);
                // this->rdp_layer.send_input(&stream, 0, RDP_INPUT_SCANCODE, param4, param3, 0);
                break;
            #warning find out what is this message and define symbolic constant
            case 17:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_SYNCHRONIZE, param4, param3, 0);
                break;
            case WM_MOUSEMOVE:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, param1, param2);
                break;
            case WM_LBUTTONUP:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1, param1, param2);
                break;
            case WM_LBUTTONDOWN:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_RBUTTONUP:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2, param1, param2);
                break;
            case WM_RBUTTONDOWN:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_BUTTON3UP:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON3, param1, param2);
                break;
            case WM_BUTTON3DOWN:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON3 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_BUTTON4UP:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4, param1, param2);
                break;
            case WM_BUTTON4DOWN:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_BUTTON5UP:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5, param1, param2);
                break;
            case WM_BUTTON5DOWN:
                this->rdp_layer.send_input(stream, 0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_INVALIDATE:
                this->rdp_layer.send_invalidate(stream, (param1 >> 16) & 0xffff, param1 & 0xffff,(param2 >> 16) & 0xffff, param2 & 0xffff);
                break;
            case WM_CHANNELDATA:
//                LOG(LOG_INFO, "rdp::mod_event::WM_CHANNEL_DATA");
                this->rdp_layer.send_redirect_pdu(param1, param2, param3, param4, this->front_channel_list);
                break;
            default:
                break;
            }
        }
        catch(Error){
            return 0;
        }
        return 0;
    }

    #warning most of code below should move to rdp_rdp
    virtual int mod_signal(void)
    {
        switch (this->state){
        case MOD_RDP_CONNECTING:
        {
        LOG(LOG_INFO, "keylayout sent to server is %x\n", keylayout);

        int width = this->get_front_width();
        int height = this->get_front_height();
        int rdp_bpp = this->get_front_bpp();
        bool console_session = this->get_client_info().console_session;
        char * hostname = this->rdp_layer.hostname;
        int & userid = this->rdp_layer.userid;

        // Connection Initiation
        // ---------------------

        // The client initiates the connection by sending the server an X.224 Connection
        //  Request PDU (class 0). The server responds with an X.224 Connection Confirm
        // PDU (class 0). From this point, all subsequent data sent between client and
        // server is wrapped in an X.224 Data Protocol Data Unit (PDU).

        // Client                                                     Server
        //    |------------X224 Connection Request PDU----------------> |
        //    | <----------X224 Connection Confirm PDU----------------- |

        this->x224_connection_request_pdu(trans);
        this->x224_connection_confirm_pdu(this->trans);

        try{

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

            this->rdp_layer.sec_layer.mcs_connect_initial_pdu_with_gcc_conference_create_request(
                    this->trans, this->channel_list, width, height, rdp_bpp, keylayout, console_session, hostname);

            this->rdp_layer.sec_layer.mcs_connect_response_pdu_with_gcc_conference_create_response(
                    this->trans, this->channel_list, this->use_rdp5);


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

            Stream edrq_stream(8192);
            X224Out edrq_tpdu(X224Packet::DT_TPDU, edrq_stream);
            edrq_stream.out_uint8((MCS_EDRQ << 2));
            edrq_stream.out_uint16_be(0x100); /* height */
            edrq_stream.out_uint16_be(0x100); /* interval */
            edrq_tpdu.end();
            edrq_tpdu.send(this->trans);

            // -----------------------------------------------
            Stream aurq_stream(8192);
            X224Out aurq_tpdu(X224Packet::DT_TPDU, aurq_stream);
            aurq_stream.out_uint8((MCS_AURQ << 2));
            aurq_tpdu.end();
            aurq_tpdu.send(this->trans);

            // -----------------------------------------------
            Stream aucf_stream(8192);
            X224In aucf_tpdu(this->trans, aucf_stream);
            int opcode = aucf_stream.in_uint8();
            if ((opcode >> 2) != MCS_AUCF) {
                throw Error(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK);
            }
            int res = aucf_stream.in_uint8();
            if (res != 0) {
                throw Error(ERR_MCS_RECV_AUCF_RES_NOT_0);
            }
            if (opcode & 2) {
                userid = aucf_stream.in_uint16_be();
            }
            aucf_tpdu.end();

            // 2.2.1.8 Client MCS Channel Join Request PDU
            // -------------------------------------------
            // The MCS Channel Join Request PDU is an RDP Connection Sequence PDU sent
            // from client to server during the Channel Connection phase (see section
            // 1.3.1.1). It is sent after receiving the MCS Attach User Confirm PDU
            // (section 2.2.1.7). The client uses the MCS Channel Join Request PDU to
            // join the user channel obtained from the Attach User Confirm PDU, the
            // I/O channel and all of the static virtual channels obtained from the
            // Server Network Data structure (section 2.2.1.4.4).

            // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

            // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
            //                     section 13.7.

            // mcsCJrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an
            //                    MCS Channel Join Request structure as specified in
            //                    [T125] sections 10.19 and I.3 (the ASN.1 structure
            //                    definitions are given in [T125] section 7, parts 6
            //                    and 10).

            // ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
            // {
            //     initiator UserId
            //     channelId ChannelId
            //               -- may be zero
            // }

            #warning the array size below is arbitrary, it should be checked to avoid buffer overflow
            uint16_t channels[100];

            size_t num_channels = this->channel_list.size();
            channels[0] = userid + 1001;
            channels[1] = MCS_GLOBAL_CHANNEL;
            for (size_t index = 2; index < num_channels+2; index++){
                const mcs_channel_item* channel_item = this->channel_list[index-2];
                channels[index] = channel_item->chanid;
            }

            for (size_t index = 0; index < num_channels+2; index++){
                // -----------------------------------------------
                Stream cjrq_stream(8192);
                X224Out cjrq_tpdu(X224Packet::DT_TPDU, cjrq_stream);
                cjrq_stream.out_uint8((MCS_CJRQ << 2));
                cjrq_stream.out_uint16_be(userid);
                cjrq_stream.out_uint16_be(channels[index]);
                cjrq_tpdu.end();
                cjrq_tpdu.send(this->trans);
                // -----------------------------------------------
                Stream cjcf_stream(8192);
                X224In cjcf_tpdu(this->trans, cjcf_stream);
                int opcode = cjcf_stream.in_uint8();
                if ((opcode >> 2) != MCS_CJCF) {
                    throw Error(ERR_MCS_RECV_CJCF_OPCODE_NOT_CJCF);
                }
                if (0 != cjcf_stream.in_uint8()) {
                    throw Error(ERR_MCS_RECV_CJCF_EMPTY);
                }
                cjcf_stream.skip_uint8(4); /* mcs_userid, req_chanid */
                if (opcode & 2) {
                    cjcf_stream.skip_uint8(2); /* join_chanid */
                }
                cjcf_tpdu.end();
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
            // header is included with the data if encryption is in force (the Client Info
            // and licensing PDUs are an exception in that they always have a security
            // header). The Security Header follows the X.224 and MCS Headers and indicates
            // whether the attached data is encrypted.

            // Even if encryption is in force server-to-client traffic may not always be
            // encrypted, while client-to-server traffic will always be encrypted by
            // Microsoft RDP implementations (encryption of licensing PDUs is optional,
            // however).

            // Client                                                     Server
            //    |------Security Exchange PDU ---------------------------> |

            LOG(LOG_INFO, "Iso Layer : setting encryption\n");
            /* Send the client random to the server */
            //      if (this->encryption)
            Stream sdrq_stream(8192);
            X224Out sdrq_tpdu(X224Packet::DT_TPDU, sdrq_stream);
            McsOut sdrq_out(sdrq_stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

            sdrq_stream.out_uint32_le(SEC_CLIENT_RANDOM);
            sdrq_stream.out_uint32_le(rdp_layer.sec_layer.server_public_key_len + SEC_PADDING_SIZE);
            LOG(LOG_INFO, "Server public key is %d bytes long", rdp_layer.sec_layer.server_public_key_len);
            sdrq_stream.out_copy_bytes(rdp_layer.sec_layer.client_crypt_random, rdp_layer.sec_layer.server_public_key_len);
            sdrq_stream.out_clear_bytes(SEC_PADDING_SIZE);

            sdrq_out.end();
            sdrq_tpdu.end();
            sdrq_tpdu.send(this->trans);
        }
        catch(...){
            Stream stream(11);
            X224Out tpdu(X224Packet::DR_TPDU, stream);
            tpdu.end();
            tpdu.send(this->trans);
            throw;
        }

        int flags = RDP_LOGON_NORMAL;

        const char * password = context.get(STRAUTHID_TARGET_PASSWORD);
        if (strlen(password) > 0) {
            flags |= RDP_LOGON_AUTO;
        }

// Secure Settings Exchange
// ------------------------

// Secure Settings Exchange: Secure client data (such as the username,
// password and auto-reconnect cookie) is sent to the server using the Client
// Info PDU.

// Client                                                     Server
//    |------ Client Info PDU      ---------------------------> |

            int rdp5_performanceflags = this->get_client_info().rdp5_performanceflags;

//            LOG(LOG_INFO, "send login info to server\n");
            time_t t = time(NULL);
            time_t tzone;

            rdp5_performanceflags = RDP5_NO_WALLPAPER;

            // The WAB does not send it's IP to server. Is it what we want ?
            const char * ip_source = "\0\0\0\0";

            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->rdp_layer.userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_LOGON_INFO | SEC_ENCRYPT, this->rdp_layer.sec_layer.encrypt);

            if(!this->use_rdp5){
                LOG(LOG_INFO, "send login info (RDP4-style) %s:%s\n",this->rdp_layer.domain, this->rdp_layer.username);

                stream.out_uint32_le(0);
                stream.out_uint32_le(flags);
                stream.out_uint16_le(2 * strlen(this->rdp_layer.domain));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.username));
                stream.out_uint16_le(2 * strlen(password));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.program));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.directory));
                stream.out_unistr(this->rdp_layer.domain);
                stream.out_unistr(this->rdp_layer.username);
                stream.out_unistr(password);
                stream.out_unistr(this->rdp_layer.program);
                stream.out_unistr(this->rdp_layer.directory);
            }
            else {
                LOG(LOG_INFO, "send login info (RDP5-style) %x %s:%s\n",flags,
                    this->rdp_layer.domain,
                    this->rdp_layer.username);

                flags |= RDP_LOGON_BLOB;
                stream.out_uint32_le(0);
                stream.out_uint32_le(flags);
                stream.out_uint16_le(2 * strlen(this->rdp_layer.domain));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.username));
                if (flags & RDP_LOGON_AUTO){
                    stream.out_uint16_le(2 * strlen(password));
                }
                if (flags & RDP_LOGON_BLOB && ! (flags & RDP_LOGON_AUTO)){
                    stream.out_uint16_le(0);
                }
                stream.out_uint16_le(2 * strlen(this->rdp_layer.program));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.directory));
                if ( 0 < (2 * strlen(this->rdp_layer.domain))){
                    stream.out_unistr(this->rdp_layer.domain);
                }
                else {
                    stream.out_uint16_le(0);
                }
                stream.out_unistr(this->rdp_layer.username);
                if (flags & RDP_LOGON_AUTO){
                    stream.out_unistr(password);
                }
                else{
                    stream.out_uint16_le(0);
                }
                if (0 < 2 * strlen(this->rdp_layer.program)){
                    stream.out_unistr(this->rdp_layer.program);
                }
                else {
                    stream.out_uint16_le(0);
                }
                if (2 * strlen(this->rdp_layer.directory) < 0){
                    stream.out_unistr(this->rdp_layer.directory);
                }
                else{
                    stream.out_uint16_le(0);
                }
                stream.out_uint16_le(2);
                stream.out_uint16_le(2 * strlen(ip_source) + 2);
                stream.out_unistr(ip_source);
                stream.out_uint16_le(2 * strlen("C:\\WINNT\\System32\\mstscax.dll") + 2);
                stream.out_unistr("C:\\WINNT\\System32\\mstscax.dll");

                tzone = (mktime(gmtime(&t)) - mktime(localtime(&t))) / 60;
                stream.out_uint32_le(tzone);

                stream.out_unistr("GTB, normaltid");
                stream.out_clear_bytes(62 - 2 * strlen("GTB, normaltid"));

                stream.out_uint32_le(0x0a0000);
                stream.out_uint32_le(0x050000);
                stream.out_uint32_le(3);
                stream.out_uint32_le(0);
                stream.out_uint32_le(0);

                stream.out_unistr("GTB, sommartid");
                stream.out_clear_bytes(62 - 2 * strlen("GTB, sommartid"));

                stream.out_uint32_le(0x30000);
                stream.out_uint32_le(0x050000);
                stream.out_uint32_le(2);
                stream.out_uint32_le(0);
                stream.out_uint32_le(0xffffffc4);
                stream.out_uint32_le(0xfffffffe);
                stream.out_uint32_le(rdp5_performanceflags);
                stream.out_uint16_le(0);
                this->use_rdp5 = 0;
            }

            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->trans);

            LOG(LOG_INFO, "send login info ok\n");
        }
        this->state = MOD_RDP_CONNECTED;
        break;

        case MOD_RDP_CONNECTED:
        {
            int type;
            int cont;

            this->in_stream.init(8192 * 2);
            try{
                cont = 1;
                while (cont) {
                    type = this->rdp_layer.recv(this->in_stream, this);
                    switch (type) {
                    case PDUTYPE_DATAPDU:
                        this->rdp_layer.process_data_pdu(this->in_stream, this);
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU:
                        {
                            client_mod * mod = this;
                            LOG(LOG_INFO, "process demand active\n");

                            int type;
                            int len_src_descriptor;
                            int len_combined_caps;

                            this->rdp_layer.share_id = this->in_stream.in_uint32_le();
                            len_src_descriptor = this->in_stream.in_uint16_le();
                            len_combined_caps = this->in_stream.in_uint16_le();
                            this->in_stream.skip_uint8(len_src_descriptor);
                            this->rdp_layer.process_server_caps(this->in_stream, len_combined_caps, this->use_rdp5);
                            this->rdp_layer.send_confirm_active(this->in_stream, mod, this->use_rdp5);
                            this->rdp_layer.send_synchronise(this->in_stream);
                            this->rdp_layer.send_control(this->in_stream, RDP_CTL_COOPERATE);
                            this->rdp_layer.send_control(this->in_stream, RDP_CTL_REQUEST_CONTROL);
                            type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_PDU_SYNCHRONIZE */
                            type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_CTL_COOPERATE */
                            type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_CTL_GRANT_CONTROL */
                            this->rdp_layer.send_input(this->in_stream, 0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);
                            /* Including RDP 5.0 capabilities */
                            if (this->use_rdp5 != 0){
                                this->rdp_layer.enum_bmpcache2();
                                this->rdp_layer.send_fonts(this->in_stream, 3);
                            }
                            else{
                                this->rdp_layer.send_fonts(this->in_stream, 1);
                                this->rdp_layer.send_fonts(this->in_stream, 2);
                            }
                            type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_PDU_UNKNOWN 0x28 (Fonts?) */
                            this->rdp_layer.orders.rdp_orders_reset_state();
                            LOG(LOG_INFO, "process demand active ok, reset state [bpp=%d]\n", this->rdp_layer.bpp);
                        }
                        this->mod_bpp = this->rdp_layer.bpp;
                        this->up_and_running = 1;
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        this->up_and_running = 0;
                        break;
                    #warning this PDUTYPE is undocumented and seems to mean the same as type 10
                    case RDP_PDU_REDIRECT:
                        break;
                    case 0:
                        break;
                    default:
                        break;
                    }
                    cont = this->in_stream.next_packet < this->in_stream.end;
                }
            }
            catch(Error e){
                return (e.id == ERR_SOCKET_CLOSED)?2:1;
            }
            catch(...){
                #warning this exception happen, check why (it shouldnt, some error not of Error type is generated)
                return 1;
            }
        }
        }
        return 0;
    }

    // 2.2.1.1 Client X.224 Connection Request PDU
    // ===========================================

    // The X.224 Connection Request PDU is an RDP Connection Sequence PDU sent from
    // client to server during the Connection Initiation phase (see section 1.3.1.1).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Crq (7 bytes): An X.224 Class 0 Connection Request transport protocol
    // data unit (TPDU), as specified in [X224] section 13.3.

    // routingToken (variable): An optional and variable-length routing token
    // (used for load balancing) terminated by a carriage-return (CR) and line-feed
    // (LF) ANSI sequence. For more information about Terminal Server load balancing
    // and the routing token format, see [MSFT-SDLBTS]. The length of the routing
    // token and CR+LF sequence is included in the X.224 Connection Request Length
    // Indicator field. If this field is present, then the cookie field MUST NOT be
    //  present.

    //cookie (variable): An optional and variable-length ANSI text string terminated
    // by a carriage-return (CR) and line-feed (LF) ANSI sequence. This text string
    // MUST be "Cookie: mstshash=IDENTIFIER", where IDENTIFIER is an ANSI string
    //(an example cookie string is shown in section 4.1.1). The length of the entire
    // cookie string and CR+LF sequence is included in the X.224 Connection Request
    // Length Indicator field. This field MUST NOT be present if the routingToken
    // field is present.

    // rdpNegData (8 bytes): An optional RDP Negotiation Request (section 2.2.1.1.1)
    // structure. The length of this negotiation structure is included in the X.224
    // Connection Request Length Indicator field.

    void x224_connection_request_pdu(Transport * trans)
    {
        Stream out;
        X224Out crtpdu(X224Packet::CR_TPDU, out);
        crtpdu.end();
        crtpdu.send(trans);
    }

    // 2.2.1.2 Server X.224 Connection Confirm PDU
    // ===========================================

    // The X.224 Connection Confirm PDU is an RDP Connection Sequence PDU sent from
    // server to client during the Connection Initiation phase (see section
    // 1.3.1.1). It is sent as a response to the X.224 Connection Request PDU
    // (section 2.2.1.1).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Ccf (7 bytes): An X.224 Class 0 Connection Confirm TPDU, as specified in
    // [X224] section 13.4.

    // rdpNegData (8 bytes): Optional RDP Negotiation Response (section 2.2.1.2.1)
    // structure or an optional RDP Negotiation Failure (section 2.2.1.2.2)
    // structure. The length of the negotiation structure is included in the X.224
    // Connection Confirm Length Indicator field.

    void x224_connection_confirm_pdu(Transport * trans)
    {
        Stream in;
        X224In cctpdu(trans, in);
        if (cctpdu.tpkt.version != 3){
            throw Error(ERR_T123_EXPECTED_TPKT_VERSION_3);
        }
        if (cctpdu.tpdu_hdr.code != X224Packet::CC_TPDU){
            throw Error(ERR_X224_EXPECTED_CONNECTION_CONFIRM);
        }
    }
};

#endif
