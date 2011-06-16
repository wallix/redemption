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
    struct ClientInfo *client_info;
    bool dev_redirection_enable;

    mod_rdp(Transport * t,
            int (& keys)[256], int & key_flags, Keymap * &keymap,
            struct ModContext & context, struct Front & front,
            ClientInfo *client_info, vector<mcs_channel_item*> channel_list,
            const char * hostname, int keylayout,
            bool clipboard_enable, bool dev_redirection_enable)
            : client_mod(keys, key_flags, keymap, front),
              rdp_layer(this, t,
                context.get(STRAUTHID_TARGET_USER),
                context.get(STRAUTHID_TARGET_PASSWORD),
                hostname, channel_list,
                client_info->rdp5_performanceflags,
                this->get_front_width(),
                this->get_front_height(),
                this->get_front_bpp(),
                keylayout,
                client_info->console_session),
                in_stream(8192)

    {
        const char * password = context.get(STRAUTHID_TARGET_PASSWORD);

//        LOG(LOG_INFO, "mod_rdp connect\n");
        this->t = 0;
        try {
            this->t = t;
            this->client_info = client_info;
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

            int flags = RDP_LOGON_NORMAL;

            if (strlen(password) > 0) {
                flags |= RDP_LOGON_AUTO;
            }

            this->rdp_layer.send_login_info(flags, this->client_info->rdp5_performanceflags);
        } catch (...) {
        #warning this->t is not allocated here, it shouldn't be desallocated here
            if (this->t){
                delete this->t;
            }
            throw;
        };
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
//        LOG(LOG_INFO, "rdp::mod_event");

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
            LOG(LOG_WARNING, "rdp::mod_event(...), catched Error exception\n");
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
                type = 0;
                this->rdp_layer.recv(this->in_stream, &type, this);
                switch (type) {
                case RDP_PDU_DATA:
                    this->rdp_layer.process_data_pdu(this->in_stream, this);
                    break;
                case RDP_PDU_DEMAND_ACTIVE:
                    this->rdp_layer.process_demand_active(this->in_stream, this);
                    this->mod_bpp = this->rdp_layer.bpp;
                    this->up_and_running = 1;
                    break;
                case RDP_PDU_DEACTIVATE:
                    this->up_and_running = 0;
                    break;
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
            LOG(LOG_WARNING, "Catched regular exception %s\n", strerror(errno));
            return 1;
        }
        return 0;
    }

};

#endif
