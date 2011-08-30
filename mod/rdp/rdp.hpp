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
    Transport *t;
    struct vector<mcs_channel_item*> front_channel_list;
    bool dev_redirection_enable;

    mod_rdp(Transport * t,
            int (& keys)[256], int & key_flags, Keymap * &keymap,
            struct ModContext & context, struct Front & front,
            vector<mcs_channel_item*> channel_list,
            const char * hostname, int keylayout,
            bool clipboard_enable, bool dev_redirection_enable)
            : client_mod(keys, key_flags, keymap, front),
              rdp_layer(this, t,
                context.get(STRAUTHID_TARGET_USER),
                context.get(STRAUTHID_TARGET_PASSWORD),
                hostname, channel_list,
                this->get_client_info().rdp5_performanceflags,
                this->get_front_width(),
                this->get_front_height(),
                this->get_front_bpp(),
                keylayout,
                this->get_client_info().console_session),
                in_stream(8192)

    {
        #warning if some error occurs while connecting we should manage disconnection from t
        this->t = t;
        // copy channel list from client.
        // It will be changed after negotiation with server
        // to hold only channels actually supported.
        this->front_channel_list = channel_list;
        this->up_and_running = 0;
        /* clipboard allow us to deactivate copy/paste sequence from server
        to client communication. This is allowed by default */
        this->clipboard_enable = clipboard_enable;
        this->dev_redirection_enable = dev_redirection_enable;

        LOG(LOG_INFO, "keylayout sent to server is %x\n", keylayout);

        const char * password = context.get(STRAUTHID_TARGET_PASSWORD);

        {

        this->rdp_layer.sec_layer.rdp_sec_connect2(t, channel_list, this->get_front_width(), this->get_front_height(), this->get_front_bpp(), keylayout, this->get_client_info().console_session, this->rdp_layer.use_rdp5, this->rdp_layer.hostname);

        int flags = RDP_LOGON_NORMAL;

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

            int sec_flags = SEC_LOGON_INFO | SEC_ENCRYPT;

            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->rdp_layer.sec_layer.userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_LOGON_INFO | SEC_ENCRYPT, this->rdp_layer.sec_layer.encrypt);

            if(!this->rdp_layer.use_rdp5){
                LOG(LOG_INFO, "send login info (RDP4-style) %s:%s\n",this->rdp_layer.domain, this->rdp_layer.username);

                stream.out_uint32_le(0);
                stream.out_uint32_le(flags);
                stream.out_uint16_le(2 * strlen(this->rdp_layer.domain));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.username));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.password));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.program));
                stream.out_uint16_le(2 * strlen(this->rdp_layer.directory));
                stream.out_unistr(this->rdp_layer.domain);
                stream.out_unistr(this->rdp_layer.username);
                stream.out_unistr(this->rdp_layer.password);
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
                    stream.out_uint16_le(2 * strlen(this->rdp_layer.password));
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
                    stream.out_unistr(this->rdp_layer.password);
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
                this->rdp_layer.use_rdp5 = 0;
            }

            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->rdp_layer.trans);

            LOG(LOG_INFO, "send login info ok\n");
        }
    }

    virtual ~mod_rdp() {
        delete this->t;
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
                        this->rdp_layer.process_server_caps(this->in_stream, len_combined_caps);
                        this->rdp_layer.send_confirm_active(this->in_stream, mod);
                        this->rdp_layer.send_synchronise(this->in_stream);
                        this->rdp_layer.send_control(this->in_stream, RDP_CTL_COOPERATE);
                        this->rdp_layer.send_control(this->in_stream, RDP_CTL_REQUEST_CONTROL);
                        type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_PDU_SYNCHRONIZE */
                        type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_CTL_COOPERATE */
                        type = this->rdp_layer.recv(this->in_stream, mod); /* RDP_CTL_GRANT_CONTROL */
                        this->rdp_layer.send_input(this->in_stream, 0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);
                        /* Including RDP 5.0 capabilities */
                        if (this->rdp_layer.use_rdp5 != 0){
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
        return 0;
    }

};

#endif
