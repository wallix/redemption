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
    ChannelList mod_channel_list;
    bool dev_redirection_enable;
    struct ModContext & context;
    wait_obj & event;
    int use_rdp5;
    int keylayout;

    enum {
        MOD_RDP_CONNECTING,
        MOD_RDP_CONNECTION_INITIATION,
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

    mod_rdp(Transport * trans, wait_obj & event,
            int (& keys)[256], int & key_flags, Keymap * &keymap,
            struct ModContext & context, struct Front & front,
            const char * hostname, int keylayout,
            bool clipboard_enable, bool dev_redirection_enable)
            :
                client_mod(keys, key_flags, keymap, front),
                  rdp_layer(this, trans,
                    context.get(STRAUTHID_TARGET_USER),
                    context.get(STRAUTHID_TARGET_PASSWORD),
                    hostname,
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
                    connection_finalization_state(EARLY),
                    state(MOD_RDP_CONNECTING)
        {
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
            this->rdp_layer.send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
        }
        if (msg == WM_KEYUP){
            keys[param1] = 0;
        }
    }

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
                // this->rdp_layer.send_input(0, RDP_INPUT_SCANCODE, param4, param3, 0);
                break;
            #warning find out what is this message and define symbolic constant
            case 17:
                this->rdp_layer.send_input(0, RDP_INPUT_SYNCHRONIZE, param4, param3, 0);
                break;
            case WM_MOUSEMOVE:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, param1, param2);
                break;
            case WM_LBUTTONUP:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1, param1, param2);
                break;
            case WM_LBUTTONDOWN:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_RBUTTONUP:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2, param1, param2);
                break;
            case WM_RBUTTONDOWN:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_BUTTON3UP:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON3, param1, param2);
                break;
            case WM_BUTTON3DOWN:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON3 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_BUTTON4UP:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4, param1, param2);
                break;
            case WM_BUTTON4DOWN:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_BUTTON5UP:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5, param1, param2);
                break;
            case WM_BUTTON5DOWN:
                this->rdp_layer.send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5 | MOUSE_FLAG_DOWN, param1, param2);
                break;
            case WM_INVALIDATE:
                this->rdp_layer.send_invalidate(stream, (param1 >> 16) & 0xffff, param1 & 0xffff,(param2 >> 16) & 0xffff, param2 & 0xffff);
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

    virtual void send_to_mod_channel(const McsChannelItem & front_channel, uint8_t * data, size_t size, size_t length, uint32_t flags)
    {
        LOG(LOG_INFO, "---------------------------------> send_to_mod_channel(front_channel(%u,%x,%s), data=%p, size=%u, length=%u, flags=%x)",
            front_channel.chanid, front_channel.flags, front_channel.name, data, size, length, flags);
        size_t index = mod_channel_list.size();
        for (size_t i = 0; i < mod_channel_list.size(); i++){
            if (strcmp(front_channel.name, mod_channel_list[i].name) == 0){
                index = i;
            }
            break;
        }
        if (index < mod_channel_list.size()){
            const McsChannelItem & mod_channel = mod_channel_list[index];
            LOG(LOG_INFO, "sent to %u", mod_channel.chanid);
            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->rdp_layer.userid, mod_channel.chanid);
            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->rdp_layer.sec_layer.encrypt);
            stream.out_uint32_le(length);
            stream.out_uint32_le(flags);
            memcpy(stream.p, data, size);
            stream.p+= size;
            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->rdp_layer.trans);
        }
    }

//    void send_redirect_pdu(long param1, long param2, long param3, int param4, const ChannelList & mod_channel_list, const ChannelList & front_channel_list) throw(Error)
//    {
//        LOG(LOG_INFO, "send_redirect_pdu\n");
//        char* name = 0;
//        /* We need to verify this in order to right process the stream passed */
//        int chan_id = (int)(param1 & 0xffff) + MCS_GLOBAL_CHANNEL + 1;
//        int flags = (int)((param1 >> 16) & 0xffff);
//        int size = param2;
//        char * data = (char*)param3;
//        int total_data_length = param4;
//        /* We need to recover the name of the channel linked with this
//        channel_id in order to match it with the same channel on the
//        first channel_list created by the RDP client at initialization
//        process */

//        int mod_chan_id = 0;
//        size_t num_channels_src = front_channel_list.size();
//        for (size_t index = 0; index < num_channels_src; index++){
//            const McsChannelItem & front_channel_item = front_channel_list[index];
//            if (chan_id == front_channel_item.chanid){ // found channel

//                size_t num_channels_dst = mod_channel_list.size();
//                for (size_t index = 0; index < num_channels_dst; index++){
//                    const McsChannelItem & mod_channel_item = mod_channel_list[index];
//                    if (strcmp(front_channel_item.name, mod_channel_item.name) == 0){
//                        mod_chan_id = mod_channel_item.chanid;
//                    }
//                    break;
//                }
//            }
//        }
//        if (mod_chan_id){
//            LOG(LOG_INFO, "sent to %u", mod_chan_id);
//            Stream stream(8192);
//            X224Out tpdu(X224Packet::DT_TPDU, stream);
//            McsOut sdrq_out(stream, MCS_SDRQ, this->rdp_layer.userid, mod_chan_id);
//            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->rdp_layer.sec_layer.encrypt);
//            stream.out_uint32_le(total_data_length);
//            stream.out_uint32_le(flags);
//            memcpy(stream.p, data, size);
//            stream.p+= size;

//            /* in send_redirect_pdu, sending data from stream.p throughout channel channel_item->name */
//            //g_hexdump(stream.p, size + 8);
//            /* We need to call send_data but with another code because we need to build an
//            virtual_channel packet and not an MCS_GLOBAL_CHANNEL packet */
//            sec_out.end();
//            sdrq_out.end();
//            tpdu.end();
//            tpdu.send(this->rdp_layer.trans);
//        }
//    }


    #warning most of code below should move to rdp_rdp
    virtual int mod_signal(void)
    {
        try{

        int width = this->get_front_width();
        int height = this->get_front_height();
        int rdp_bpp = this->get_front_bpp();
        bool console_session = this->get_client_info().console_session;
        char * hostname = this->rdp_layer.hostname;


        int & userid = this->rdp_layer.userid;

        switch (this->state){
        case MOD_RDP_CONNECTING:
            LOG(LOG_INFO, "MOD_RDP_CONNECTING");
            // Connection Initiation
            // ---------------------

            // The client initiates the connection by sending the server an X.224 Connection
            //  Request PDU (class 0). The server responds with an X.224 Connection Confirm
            // PDU (class 0). From this point, all subsequent data sent between client and
            // server is wrapped in an X.224 Data Protocol Data Unit (PDU).

            // Client                                                     Server
            //    |------------X224 Connection Request PDU----------------> |
            //    | <----------X224 Connection Confirm PDU----------------- |

            this->send_x224_connection_request_pdu(trans);
            this->state = MOD_RDP_CONNECTION_INITIATION;
        break;

        case MOD_RDP_CONNECTION_INITIATION:
            LOG(LOG_INFO, "MOD_RDP_CONNECTION_INITIATION");
            this->recv_x224_connection_confirm_pdu(this->trans);

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

            send_mcs_connect_initial_pdu_with_gcc_conference_create_request(
                    this->trans, this->front.get_channel_list(), width, height, rdp_bpp, keylayout, console_session, hostname);

            this->state = MOD_RDP_BASIC_SETTINGS_EXCHANGE;
        break;

        case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
            LOG(LOG_INFO, "MOD_RDP_BASIC_SETTINGS_EXCHANGE");
            this->mcs_connect_response_pdu_with_gcc_conference_create_response(
                    this->trans, this->mod_channel_list, this->use_rdp5);

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

            send_mcs_erect_domain_and_attach_user_request_pdu(this->trans);

            this->state = MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER;
        break;

        case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
        LOG(LOG_INFO, "MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER");
        {
            recv_mcs_attach_user_confirm_pdu(this->trans, this->rdp_layer.userid);
            LOG(LOG_INFO, "send_mcs_channel_join_request_and_recv_confirm_pdu");
            send_mcs_channel_join_request_and_recv_confirm_pdu(this->trans, this->rdp_layer.userid, this->mod_channel_list);

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

            LOG(LOG_INFO, "sec_layer.security_exchange_PDU");
            this->rdp_layer.sec_layer.send_security_exchange_PDU(trans, this->rdp_layer.userid);

            // Secure Settings Exchange
            // ------------------------

            // Secure Settings Exchange: Secure client data (such as the username,
            // password and auto-reconnect cookie) is sent to the server using the Client
            // Info PDU.

            // Client                                                     Server
            //    |------ Client Info PDU      ---------------------------> |

            LOG(LOG_INFO, "send_client_info_pdu");
            int rdp5_performanceflags = this->get_client_info().rdp5_performanceflags;
            rdp5_performanceflags = RDP5_NO_WALLPAPER;

            this->rdp_layer.send_client_info_pdu(
                                this->trans,
                                this->rdp_layer.userid,
                                context.get(STRAUTHID_TARGET_PASSWORD),
                                rdp5_performanceflags,
                                this->use_rdp5);

            this->state = MOD_RDP_GET_LICENSE;
        }
        break;

        case MOD_RDP_GET_LICENSE:
        LOG(LOG_INFO, "MOD_RDP_GET_LICENSE");
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
            //    | <------ Licence Error PDU Valid Client ---------------- |

            if (this->rdp_layer.sec_layer.rdp_lic_process(this->trans, this->rdp_layer.hostname, this->rdp_layer.username, this->rdp_layer.userid)){
                this->state = MOD_RDP_CONNECTED;
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
            // client and server after the connection has been finalized includes
            // connection management information and virtual channel messages (exchanged
            // between client-side plug-ins and server-side applications).

        case MOD_RDP_CONNECTED:
        {
            int pdu_type;

            Stream stream(65536);
            try{
                uint8_t * next_packet = 0;
                uint32_t sec_flags = 0;
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)
                X224In(this->trans, stream);
                McsIn mcs_in(stream);
                if ((mcs_in.opcode >> 2) != MCS_SDIN) {
                    LOG(LOG_INFO, "Error: MCS_SDIN TPDU expected");
                    throw Error(ERR_MCS_RECV_ID_NOT_MCS_SDIN);
                }
                SecIn sec(stream, this->rdp_layer.sec_layer.decrypt);
                if (sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
                    LOG(LOG_INFO, "Error: unexpected licence negotiation sec packet");
                    throw Error(ERR_SEC_UNEXPECTED_LICENCE_NEGOTIATION_PDU);
                }

                if (sec.flags & 0x0400){ /* SEC_REDIRECT_ENCRYPT */
                    /* Check for a redirect packet, starts with 00 04 */
                    if (stream.p[0] == 0 && stream.p[1] == 4){
                    /* for some reason the PDU and the length seem to be swapped.
                       This isn't good, but we're going to do a byte for byte
                       swap.  So the first four value appear as: 00 04 XX YY,
                       where XX YY is the little endian length. We're going to
                       use 04 00 as the PDU type, so after our swap this will look
                       like: XX YY 04 00 */

                        uint8_t swapbyte1 = stream.p[0];
                        stream.p[0] = stream.p[2];
                        stream.p[2] = swapbyte1;

                        uint8_t swapbyte2 = stream.p[1];
                        stream.p[1] = stream.p[3];
                        stream.p[3] = swapbyte2;

                        uint8_t swapbyte3 = stream.p[2];
                        stream.p[2] = stream.p[3];
                        stream.p[3] = swapbyte3;
                    }
                }

                next_packet = stream.p;
                if (mcs_in.chan_id != MCS_GLOBAL_CHANNEL){
                    this->recv_virtual_channel(stream, mcs_in.chan_id);
                    next_packet = stream.end;
                }

                while (next_packet < stream.end) {
                        stream.p = next_packet;
                    {
                        int len = stream.in_uint16_le();
                        if (len == 0x8000) {
                            next_packet += 8;
                            pdu_type = 0;
                        }
                        else {
                            pdu_type = stream.in_uint16_le();
                            stream.skip_uint8(2);
                            next_packet += len;
                        }
                    }
                    switch (pdu_type & 0xF) {
                    case PDUTYPE_DATAPDU:
                        switch (this->connection_finalization_state){
                        case EARLY:
                        break;
                        case WAITING_SYNCHRONIZE:
                            LOG(LOG_INFO, "Receiving Synchronize");
//                            this->check_data_pdu(PDUTYPE2_SYNCHRONIZE);
                            this->connection_finalization_state = WAITING_CTL_COOPERATE;
                        break;
                        case WAITING_CTL_COOPERATE:
//                            this->check_data_pdu(PDUTYPE2_CONTROL);
                            LOG(LOG_INFO, "Receiving Control Cooperate");
                            this->connection_finalization_state = WAITING_GRANT_CONTROL_COOPERATE;
                        break;
                        case WAITING_GRANT_CONTROL_COOPERATE:
//                            this->check_data_pdu(PDUTYPE2_CONTROL);
                            LOG(LOG_INFO, "Receiving Granted Control");
                            this->connection_finalization_state = WAITING_FONT_MAP;
                        break;
                        case WAITING_FONT_MAP:
                            LOG(LOG_INFO, "Receiving Font Map");
//                            this->check_data_pdu(PDUTYPE2_FONTMAP);
                            this->rdp_layer.orders.rdp_orders_reset_state();
                            LOG(LOG_INFO, "process demand active ok, reset state [bpp=%d]\n", this->rdp_layer.bpp);
                            this->mod_bpp = this->rdp_layer.bpp;
                            this->up_and_running = 1;
                            this->connection_finalization_state = UP_AND_RUNNING;
                        break;
                        case UP_AND_RUNNING:
                        {
                            uint32_t shareid = stream.in_uint32_le();
                            uint8_t pad1 = stream.in_uint8();
                            uint8_t streamid = stream.in_uint8();
                            uint16_t len = stream.in_uint16_le();
                            uint8_t pdutype2 = stream.in_uint8();
                            uint8_t compressedType = stream.in_uint8();
                            uint8_t compressedLen = stream.in_uint16_le();
                            switch (pdutype2) {
                            case PDUTYPE2_UPDATE:
                                this->rdp_layer.process_update_pdu(stream, this);
                                break;
                            case PDUTYPE2_CONTROL:
                                break;
                            case PDUTYPE2_SYNCHRONIZE:
                                break;
                            case PDUTYPE2_POINTER:
                                this->rdp_layer.process_pointer_pdu(stream, this);
                                break;
                            case PDUTYPE2_PLAY_SOUND:
                                break;
                            case PDUTYPE2_SAVE_SESSION_INFO:
                //                LOG(LOG_INFO, "DATA PDU LOGON\n");
                                break;
                            case PDUTYPE2_SET_ERROR_INFO_PDU:
                //                LOG(LOG_INFO, "DATA PDU DISCONNECT\n");
                                this->rdp_layer.process_disconnect_pdu(stream);
                                break;
                            default:
                                break;
                            }
                        }
                        break;
                        }
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU:
                        LOG(LOG_INFO, "Received demand active PDU");
                        {
                            client_mod * mod = this;
                            int len_src_descriptor;
                            int len_combined_caps;

                            this->rdp_layer.share_id = stream.in_uint32_le();
                            len_src_descriptor = stream.in_uint16_le();
                            len_combined_caps = stream.in_uint16_le();
                            stream.skip_uint8(len_src_descriptor);
                            this->rdp_layer.process_server_caps(stream, len_combined_caps, this->use_rdp5);
                            #warning we should be able to pack all the following sends to the same X224 TPDU, instead of creating a different one for each send
                            LOG(LOG_INFO, "Sending confirm active PDU");
                            this->rdp_layer.send_confirm_active(mod, this->use_rdp5);
                            LOG(LOG_INFO, "Sending synchronize");
                            this->rdp_layer.send_synchronise();
                            LOG(LOG_INFO, "Sending control cooperate");
                            this->rdp_layer.send_control(RDP_CTL_COOPERATE);
                            LOG(LOG_INFO, "Sending request control");
                            this->rdp_layer.send_control(RDP_CTL_REQUEST_CONTROL);
                            LOG(LOG_INFO, "Sending input synchronize");
                            this->rdp_layer.send_input(0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);
                            LOG(LOG_INFO, "Sending font List");
                            /* Including RDP 5.0 capabilities */
                            if (this->use_rdp5 != 0){
                                this->rdp_layer.enum_bmpcache2();
                                this->rdp_layer.send_fonts(stream, 3);
                            }
                            else{
                                this->rdp_layer.send_fonts(stream, 1);
                                this->rdp_layer.send_fonts(stream, 2);
                            }
                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                        }
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
        }
        catch(...){
            Stream stream(11);
            X224Out tpdu(X224Packet::DR_TPDU, stream);
            tpdu.end();
            tpdu.send(this->trans);
            throw;
        }
        return 0;
    }


    // redirect_pdu
    void recv_virtual_channel(Stream & stream, int chan)
    {
      uint32_t length = stream.in_uint32_le();
      int channel_flags = stream.in_uint32_le();

      LOG(LOG_INFO, "recv virtual channel %u length=%u flags=%x", chan, length, channel_flags);

        /* We need to recover the name of the channel linked with this
         channel_id in order to match it with the same channel on the
         first channel_list created by the RDP client at initialization
         process*/

        int num_channels_src = (int) this->mod_channel_list.size();
        for (int index = 0; index < num_channels_src; index++){
            const McsChannelItem & mod_channel_item = this->mod_channel_list[index];
            if (chan == mod_channel_item.chanid){
                /* Here, we're going to search the correct channel in order to send
                information throughout this channel to RDP client*/
                #warning move that code to server_send_to_channel_mod
                LOG(LOG_INFO, "found back channel chanid=%u flags=%x [channel_flags=%x] name=%s", chan, channel_flags, mod_channel_item.flags, mod_channel_item.name);
                int num_channels_dst = (int) this->front.get_channel_list().size();
                for (int front_index = 0; front_index < num_channels_dst; front_index++){
                    const McsChannelItem & front_channel_item = this->front.get_channel_list()[front_index];
                    if (strcmp(mod_channel_item.name, front_channel_item.name) == 0){
                        LOG(LOG_INFO, "found front channel chanid=%u flags=%x [channel_flags=%x] name=%s", front_channel_item.chanid, channel_flags, front_channel_item.flags, front_channel_item.name);
                        int size = (int)(stream.end - stream.p);

                        /* TODO: create new function in order to activate / deactivate copy-paste
                        sequence from server to client */

//                        if(this->rdp_layer.sec_layer.clipboard_check(name, this->clipboard_enable) == 1){
//                            /* Clipboard deactivation required */
//                        }
                        this->server_send_to_channel_mod(front_channel_item, stream.p, length, length, channel_flags);
                        break;
                    }
                }
                break;
            }
        }
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

    void send_x224_connection_request_pdu(Transport * trans)
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

    void recv_x224_connection_confirm_pdu(Transport * trans)
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

// 2.2.1.4  Server MCS Connect Response PDU with GCC Conference Create Response
// ----------------------------------------------------------------------------

// From [MSRDPCGR]

// The MCS Connect Response PDU is an RDP Connection Sequence PDU sent from
// server to client during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent as a response to the MCS Connect Initial PDU (section
// 2.2.1.3). The MCS Connect Response PDU encapsulates a GCC Conference Create
// Response, which encapsulates concatenated blocks of settings data.

// A basic high-level overview of the nested structure for the Server MCS
// Connect Response PDU is illustrated in section 1.3.1.1, in the figure
// specifying MCS Connect Response PDU. Note that the order of the settings
// data blocks is allowed to vary from that shown in the previously mentioned
// figure and the message syntax layout that follows. This is possible because
// each data block is identified by a User Data Header structure (section
// 2.2.1.4.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
// section 13.7.

// mcsCrsp (variable): Variable-length BER-encoded MCS Connect Response
//   structure (using definite-length encoding) as described in [T125]
//   (the ASN.1 structure definition is detailed in [T125] section 7, part 2).
//   The userData field of the MCS Connect Response encapsulates the GCC
//   Conference Create Response data (contained in the gccCCrsp and subsequent
//   fields).

// gccCCrsp (variable): Variable-length PER-encoded GCC Connect Data structure
//   which encapsulates a Connect GCC PDU that contains a GCC Conference Create
//   Response structure as described in [T124] (the ASN.1 structure definitions
//   are specified in [T124] section 8.7) appended as user data to the MCS
//   Connect Response (using the format specified in [T124] sections 9.5 and
//   9.6). The userData field of the GCC Conference Create Response contains
//   one user data set consisting of concatenated server data blocks.

// serverCoreData (12 bytes): Server Core Data structure (section 2.2.1.4.2).

// serverSecurityData (variable): Variable-length Server Security Data structure
//   (section 2.2.1.4.3).

// serverNetworkData (variable): Variable-length Server Network Data structure
//   (section 2.2.1.4.4).


// 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
// ------------------------------------------

//The TS_UD_CS_CORE data block contains core client connection-related
// information.

//header (4 bytes): GCC user data block header, as specified in section
//                  2.2.1.3.1. The User Data Header type field MUST be set to
//                  CS_CORE (0xC001).

// version (4 bytes): A 32-bit, unsigned integer. Client version number for the
//                    RDP. The major version number is stored in the high 2
//                    bytes, while the minor version number is stored in the
//                    low 2 bytes.
// +------------+------------------------------------+
// |   Value    |    Meaning                         |
// +------------+------------------------------------+
// | 0x00080001 | RDP 4.0 clients                    |
// +------------+------------------------------------+
// | 0x00080004 | RDP 5.0, 5.1, 5.2, and 6.0 clients |
// +------------+------------------------------------+

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                         width in pixels (up to a maximum value of 4096
//                         pixels).

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                          height in pixels (up to a maximum value of 2048
//                          pixels).

// colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color depth.
//                       Values in this field MUST be ignored if the
//                       postBeta2ColorDepth field is present.
// +--------------------------+-------------------------+
// |     Value                |        Meaning          |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+

// SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access sequence.
//                        This field SHOULD be set to RNS_UD_SAS_DEL (0xAA03).

// keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout (active
//                           input locale identifier). For a list of possible
//                           input locales, see [MSDN-MUI].

// clientBuild (4 bytes): A 32-bit, unsigned integer. The build number of the
//                        client.

// clientName (32 bytes): Name of the client computer. This field contains up to
//                        15 Unicode characters plus a null terminator.

// keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
// +-------+--------------------------------------------+
// | Value |              Meaning                       |
// +-------+--------------------------------------------+
// |   1   | IBM PC/XT or compatible (83-key) keyboard  |
// +-------+--------------------------------------------+
// |   2   | Olivetti "ICO" (102-key) keyboard          |
// +-------+--------------------------------------------+
// |   3   | IBM PC/AT (84-key) and similar keyboards   |
// +-------+--------------------------------------------+
// |   4   | IBM enhanced (101- or 102-key) keyboard    |
// +-------+--------------------------------------------+
// |   5   | Nokia 1050 and similar keyboards           |
// +-------+--------------------------------------------+
// |   6   | Nokia 9140 and similar keyboards           |
// +-------+--------------------------------------------+
// |   7   | Japanese keyboard                          |
// +-------+--------------------------------------------+

// keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard subtype
//                            (an original equipment manufacturer-dependent
//                            value).

// keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number of
//                                function keys on the keyboard.

// imeFileName (64 bytes): A 64-byte field. The Input Method Editor (IME) file
//                         name associated with the input locale. This field
//                         contains up to 31 Unicode characters plus a null
//                         terminator.

// postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
//                                color depth. Values in this field MUST be
//                                ignored if the highColorDepth field is
//                                present.
// +--------------------------+-------------------------+
// |      Value               |         Meaning         |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientProductId (2 bytes): A 16-bit, unsigned integer. The client product ID.
//                            This field SHOULD be initialized to 1. If this
//                            field is present, then all of the preceding fields
//                            MUST also be present. If this field is not
//                            present, then none of the subsequent fields MUST
//                            be present.

// serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number. This field
//                         SHOULD be initialized to 0. If this field is present,
//                         then all of the preceding fields MUST also be
//                         present. If this field is not present, then none of
//                         the subsequent fields MUST be present.

// highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
//                           depth.
// +-------+-------------------------------------------------------------------+
// | Value |                      Meaning                                      |
// +-------+-------------------------------------------------------------------+
// |     4 |   4 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |     8 |   8 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |    15 |  15-bit 555 RGB mask                                              |
// |       |  (5 bits for red, 5 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    16 |  16-bit 565 RGB mask                                              |
// |       |  (5 bits for red, 6 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    24 |  24-bit RGB mask                                                  |
// |       |  (8 bits for red, 8 bits for green, and 8 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies the
//                                 high color depths that the client is capable
//                                 of supporting.
// +-----------------------------+---------------------------------------------+
// |          Flag               |                Meaning                      |
// +-----------------------------+---------------------------------------------+
// | 0x0001 RNS_UD_24BPP_SUPPORT | 24-bit RGB mask                             |
// |                             | (8 bits for red, 8 bits for green,          |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0002 RNS_UD_16BPP_SUPPORT | 16-bit 565 RGB mask                         |
// |                             | (5 bits for red, 6 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0004 RNS_UD_15BPP_SUPPORT | 15-bit 555 RGB mask                         |
// |                             | (5 bits for red, 5 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0008 RNS_UD_32BPP_SUPPORT | 32-bit RGB mask                             |
// |                             | (8 bits for the alpha channel,              |
// |                             | 8 bits for red, 8 bits for green,           |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// earlyCapabilityFlags (2 bytes): A 16-bit, unsigned integer. It specifies
// capabilities early in the connection sequence.
// +---------------------------------------------+-----------------------------|
// |                Flag                         |              Meaning        |
// +---------------------------------------------+-----------------------------|
// | 0x0001 RNS_UD_CS_SUPPORT_ERRINFO_PDU        | Indicates that the client   |
// |                                             | supports the Set Error Info |
// |                                             | PDU (section 2.2.5.1).      |
// +---------------------------------------------+-----------------------------|
// | 0x0002 RNS_UD_CS_WANT_32BPP_SESSION         | Indicates that the client is|
// |                                             | requesting a session color  |
// |                                             | depth of 32 bpp. This flag  |
// |                                             | is necessary because the    |
// |                                             | highColorDepth field does   |
// |                                             | not support a value of 32.  |
// |                                             | If this flag is set, the    |
// |                                             | highColorDepth field SHOULD |
// |                                             | be set to 24 to provide an  |
// |                                             | acceptable fallback for the |
// |                                             | scenario where the server   |
// |                                             | does not support 32 bpp     |
// |                                             | color.                      |
// +---------------------------------------------+-----------------------------|
// | 0x0004 RNS_UD_CS_SUPPORT_STATUSINFO_PDU     | Indicates that the client   |
// |                                             | supports the Server Status  |
// |                                             | Info PDU (section 2.2.5.2). |
// +---------------------------------------------+-----------------------------|
// | 0x0008 RNS_UD_CS_STRONG_ASYMMETRIC_KEYS     | Indicates that the client   |
// |                                             | supports asymmetric keys    |
// |                                             | larger than 512 bits for use|
// |                                             | with the Server Certificate |
// |                                             | (section 2.2.1.4.3.1) sent  |
// |                                             | in the Server Security Data |
// |                                             | block (section 2.2.1.4.3).  |
// +---------------------------------------------+-----------------------------|
// | 0x0020 RNS_UD_CS_RESERVED1                  | Reserved for future use.    |
// |                                             | This flag is ignored by the |
// |                                             | server.                     |
// +---------------------------------------------+-----------------------------+
// | 0x0040 RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU | Indicates that the client   |
// |                                             | supports the Monitor Layout |
// |                                             | PDU (section 2.2.12.1).     |
// +---------------------------------------------+-----------------------------|
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientDigProductId (64 bytes): Contains a value that uniquely identifies the
//                                client. If this field is present, then all of
//                                the preceding fields MUST also be present. If
//                                this field is not present, then none of the
//                                subsequent fields MUST be present.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding to align the
//   serverSelectedProtocol field on the correct byte boundary.
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer. It contains the value returned
//   by the server in the selectedProtocol field of the RDP Negotiation Response structure
//   (section 2.2.1.2.1). In the event that an RDP Negotiation Response structure was not sent,
//   this field MUST be initialized to PROTOCOL_RDP (0). If this field is present, then all of the
//   preceding fields MUST also be present.



    void mcs_connect_response_pdu_with_gcc_conference_create_response(
                                Transport * trans,
                                ChannelList & channel_list,
                                int & use_rdp5)
    {
        Stream cr_stream(8192);
        X224In(trans, cr_stream);
        if (cr_stream.in_uint16_be() != BER_TAG_MCS_CONNECT_RESPONSE) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = cr_stream.in_ber_len();

        if (cr_stream.in_uint8() != BER_TAG_RESULT) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = cr_stream.in_ber_len();

        int res = cr_stream.in_uint8();

        if (res != 0) {
            throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
        }
        if (cr_stream.in_uint8() != BER_TAG_INTEGER) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = cr_stream.in_ber_len();
        cr_stream.skip_uint8(len); /* connect id */

        if (cr_stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = cr_stream.in_ber_len();
        cr_stream.skip_uint8(len);

        if (cr_stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = cr_stream.in_ber_len();

        cr_stream.skip_uint8(21); /* header (T.124 ConferenceCreateResponse) */
        len = cr_stream.in_uint8();

        if (len & 0x80) {
            len = cr_stream.in_uint8();
        }
        while (cr_stream.p < cr_stream.end) {
            uint16_t tag = cr_stream.in_uint16_le();
            uint16_t length = cr_stream.in_uint16_le();
            if (length <= 4) {
                return;
            }
            uint8_t *next_tag = (cr_stream.p + length) - 4;
            switch (tag) {
            case SEC_TAG_SRV_INFO:
            {
                uint16_t rdp_version = cr_stream.in_uint16_le();
                LOG(LOG_DEBUG, "Server RDP version is %d\n", rdp_version);
                if (1 == rdp_version){ // can't use rdp5
                    use_rdp5 = 0;
                    #warning why caring of server_depth here ? Quite strange
                    //        this->server_depth = 8;
                }
            }
            break;
            case SEC_TAG_SRV_CRYPT:
                this->rdp_layer.sec_layer.rdp_sec_process_crypt_info(cr_stream, this->rdp_layer.sec_layer.server_public_key_len, this->rdp_layer.sec_layer.crypt_level);
            break;
            case SEC_TAG_SRV_CHANNELS:
                // map front channels to mod channels
                    #warning we should have mod_rdp and front channel lists
                 process_srv_channels(cr_stream, this->front.get_channel_list(), channel_list);
                break;
            default:
                LOG(LOG_WARNING, "response tag 0x%x\n", tag);
                break;
            }
            cr_stream.p = next_tag;
        }
    }

};

#endif
