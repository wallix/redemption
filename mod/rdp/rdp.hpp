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
#include <string.h>
#include <assert.h>
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
#include "channel_list.hpp"
#include "colors.hpp"

#include "RDP/x224.hpp"
#include "RDP/sec.hpp"
#include "RDP/lic.hpp"

//#include "RDP/orders/RDPOrdersNames.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

struct rdp_cursor {
    int x;
    int y;
    int width;
    int height;
    uint8_t mask[(32 * 32) / 8];
    uint8_t data[(32 * 32) * 3];
    rdp_cursor() {
        this->x = 0;
        this->y = 0;
        this->width = 0;
        this->height = 0;
        memset(this->mask, 0, (32 * 32) / 8);
        memset(this->data, 0, (32 * 32) * 3);
    }
};

/* orders */
struct rdp_orders {
    // State
    RDPOrderCommon common;
    RDPMemBlt memblt;
    RDPOpaqueRect opaquerect;
    RDPScrBlt scrblt;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyph_index;

    BGRPalette cache_colormap[6];
    BGRPalette global_palette;
    BGRPalette memblt_palette;

    TODO(" this cache_bitmap here looks strange. At least it's size should ne negotiated. And why is it not managed by the other cache management code ? This probably hide some kind of problem. See when working on cache secondary order primitives.")
    Bitmap * cache_bitmap[3][10000];

    rdp_orders() :
        common(RDP::PATBLT, Rect(0, 0, 1, 1)),
        memblt(0, Rect(), 0, 0, 0, 0),
        opaquerect(Rect(), 0),
        scrblt(Rect(), 0, 0, 0),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyph_index(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"")
    {
        memset(this->cache_bitmap, 0, sizeof(this->cache_bitmap));
        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
        memset(this->memblt_palette, 0, sizeof(this->memblt_palette));
    }


    ~rdp_orders(){
    }

    void rdp_orders_process_bmpcache(int bpp, Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
//        LOG(LOG_INFO, "rdp_orders_process_bmpcache");
        RDPBmpCache bmp(bpp);
        bmp.receive(stream, control, header, this->global_palette);
        
        TODO(" add cache_id  cache_idx range check  and also size check based on cache size by type and uncompressed bitmap size")
        if (this->cache_bitmap[bmp.id][bmp.idx]) {
            delete this->cache_bitmap[bmp.id][bmp.idx];
        }
        this->cache_bitmap[bmp.id][bmp.idx] = bmp.bmp;
//        LOG(LOG_ERR, "rdp_orders_process_bmpcache bitmap id=%u idx=%u cx=%u cy=%u bpp=%u bmp_size=%u", bmp.id, bmp.idx, bmp.bmp->cx, bmp.bmp->cy, bmp.bmp->bmp_size(bpp), bpp);
    }

    void rdp_orders_process_fontcache(Stream & stream, int flags, client_mod * mod)
    {
//        LOG(LOG_INFO, "rdp_orders_process_fontcache");
        int font = stream.in_uint8();
        int nglyphs = stream.in_uint8();
        for (int i = 0; i < nglyphs; i++) {
            int character = stream.in_uint16_le();
            int offset = stream.in_uint16_le();
            int baseline = stream.in_uint16_le();
            int width = stream.in_uint16_le();
            int height = stream.in_uint16_le();
            int datasize = (height * nbbytes(width) + 3) & ~3;
            const uint8_t *data = stream.in_uint8p(datasize);

            mod->server_add_char(font, character, offset, baseline, width, height, data);
        }
    }


    void process_colormap(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header, client_mod * mod)
    {
        RDPColCache colormap;
        colormap.receive(stream, control, header);
        memcpy(this->cache_colormap[colormap.cacheIndex], &colormap.palette, sizeof(BGRPalette));
        mod->gd.color_cache(colormap.palette, colormap.cacheIndex);
    }

    /*****************************************************************************/
    int process_orders(int bpp, Stream & stream, int num_orders, client_mod * mod)
    {
//        RDPGraphicDevice & consumer = mod->gd;
        using namespace RDP;
        int processed = 0;
        while (processed < num_orders) {
            uint8_t control = stream.in_uint8();

            if (!control & STANDARD){
                /* error, this should always be set */
                LOG(LOG_ERR, "Non standard order detected : protocol error");
                break;
            }
            if (control & SECONDARY) {
                using namespace RDP;

                RDPSecondaryOrderHeader header(stream);
//                LOG(LOG_INFO, "secondary order=%d\n", header.type);
                uint8_t *next_order = stream.p + header.length + 7;
                switch (header.type) {
                case TS_CACHE_BITMAP_COMPRESSED:
                case TS_CACHE_BITMAP_UNCOMPRESSED:
                    this->rdp_orders_process_bmpcache(bpp, stream, control, header);
                    break;
                case TS_CACHE_COLOR_TABLE:
                    this->process_colormap(stream, control, header, mod);
                    break;
                case TS_CACHE_GLYPH:
                    this->rdp_orders_process_fontcache(stream, header.flags, mod);
                    break;
                case TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)", header.type);
                  break;
                case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)", header.type);
                  break;
                case TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)", header.type);
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                stream.p = next_order;
            }
            else {
                RDPPrimaryOrderHeader header = this->common.receive(stream, control);
                const Rect & cmd_clip = ((control & BOUNDS)?this->common.clip:mod->gd.get_front_rect());
//                LOG(LOG_INFO, "/* order=%d ordername=%s */\n", this->common.order, ordernames[this->common.order]);
                switch (this->common.order) {
                case GLYPHINDEX:
                    this->glyph_index.receive(stream, header);
                    mod->gd.draw(this->glyph_index, cmd_clip);
                    break;
                case DESTBLT:
                    this->destblt.receive(stream, header);
                    mod->gd.draw(this->destblt, cmd_clip);
                    break;
                case PATBLT:
                    this->patblt.receive(stream, header);
                    mod->gd.draw(this->patblt, cmd_clip);
                    break;
                case SCREENBLT:
                    this->scrblt.receive(stream, header);
                    mod->gd.draw(this->scrblt, cmd_clip);
                    break;
                case LINE:
                    this->lineto.receive(stream, header);
                    mod->gd.draw(this->lineto, cmd_clip);
                    break;
                case RECT:
                    this->opaquerect.receive(stream, header);
                    mod->gd.draw(this->opaquerect, cmd_clip);
                    break;
                case MEMBLT:
                    this->memblt.receive(stream, header);
                    {
                        assert((this->memblt.cache_id >> 4) < 6);
                        struct Bitmap* bitmap = this->cache_bitmap[this->memblt.cache_id & 0xF][this->memblt.cache_idx];
                        if (bitmap) {
                            mod->gd.mem_blt(
                                this->memblt,
                                *bitmap,
                                this->cache_colormap[this->memblt.cache_id >> 4],
                                cmd_clip);
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
//                if (header.control & BOUNDS) {
//                    mod->gd.server_reset_clip();
//                }
            }
            processed++;
        }
        return 0;
    }
};


struct mod_rdp : public client_mod {

    /* mod data */
    int up_and_running;
    Stream in_stream;
    Transport *trans;
    ChannelList mod_channel_list;

    bool dev_redirection_enable;
    struct ModContext & context;
    wait_obj & event;
    int use_rdp5;
    int keylayout;
    struct RdpLicence lic_layer;

    rdp_orders orders;
    int share_id;
    int bitmap_compression;
    int version;
    int userid;

    char hostname[16];
    char username[128];
    char password[256];
    char domain[256];
    char program[256];
    char directory[256];
    bool console_session;
    int bpp;

    uint32_t server_public_key_len;
    uint8_t client_crypt_random[512];
    CryptContext encrypt, decrypt;


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
    struct rdp_cursor cursors[32];


    mod_rdp(Transport * trans, wait_obj & event,
            struct ModContext & context, struct Front & front,
            const char * hostname, int keylayout,
            bool clipboard_enable, bool dev_redirection_enable)
            :
                client_mod(front),
                    in_stream(65536),
                    trans(trans),
                    context(context),
                    event(event),
                    use_rdp5(0),
                    keylayout(keylayout),
                    lic_layer(hostname),
                    userid(0),
                    bpp(bpp),
                    connection_finalization_state(EARLY),
                    state(MOD_RDP_CONNECTING)
    {

        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);
        this->server_public_key_len = 0;

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        this->decrypt.rc4_key_size = 2; /* 128 bits */
        this->encrypt.rc4_key_size = 2; /* 128 bits */
        this->decrypt.rc4_key_len = 16; /* 16 = 128 bit */
        this->encrypt.rc4_key_len = 16; /* 16 = 128 bit */

        TODO(" and if hostname is really larger  what happens ? We should at least emit a warning log")
        strncpy(this->hostname, hostname, 15);
        this->hostname[15] = 0;
        TODO(" and if username is really larger  what happens ? We should at least emit a warning log")
        strncpy(this->username, context.get(STRAUTHID_TARGET_USER), 127);
        this->username[127] = 0;

        LOG(LOG_INFO, "Remote RDP Server login:%s host:%s\n", this->username, this->hostname);
        this->share_id = 0;
        this->bitmap_compression = 1;
        this->console_session = this->gd.get_client_info().console_session;

        memset(this->password, 0, 256);
        strcpy(this->password, context.get(STRAUTHID_TARGET_PASSWORD));

        memset(this->domain, 0, 256);
        memset(this->program, 0, 256);
        memset(this->directory, 0, 256);

        this->keylayout = keylayout;
        LOG(LOG_INFO, "Server key layout is %x\n", this->keylayout);


        this->up_and_running = 0;
        /* clipboard allow us to deactivate copy/paste sequence from server
        to client communication. This is allowed by default */
        this->clipboard_enable = clipboard_enable;
        this->dev_redirection_enable = dev_redirection_enable;

        while (!this->up_and_running){
            BackEvent_t res = this->draw_event();
            if (res != BACK_EVENT_NONE){
                LOG(LOG_INFO, "Creation of new mod 'RDP' failed\n");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }

    virtual ~mod_rdp() {
        delete this->trans;
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long time, const Keymap * keymap, const key_info* ki){
        if (this->up_and_running) {
//            LOG(LOG_INFO, "Direct parameter transmission \n");
            this->send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        if (this->up_and_running) {
            this->send_input(0, RDP_INPUT_SYNCHRONIZE, device_flags, param1, 0);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap)
    {
        if (this->up_and_running) {
            TODO(" is decoding and reencoding really necessary  a simple pass-through from front to back-end should be enough")
            if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
                this->send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, x, y);
                this->gd.front.mouse_x = x;
                this->gd.front.mouse_y = y;
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
                uint8_t * data,
                size_t length,
                size_t chunk_size,
                uint32_t flags)
    {
        const McsChannelItem * mod_channel = this->mod_channel_list.get(front_channel_name);
        // send it if module has a matching channel, if no matching channel is found just forget it
        if (mod_channel){
            this->send_to_channel(*mod_channel, data, length, chunk_size, flags);
        }

    }
    
    void send_to_channel(
                const McsChannelItem & channel,
                uint8_t * data,
                size_t length,
                size_t chunk_size,
                uint32_t flags)
    {
        Stream stream(65536);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, this->userid, channel.chanid);

        TODO(" merge with Front::send_to_channel  the only difference now is the crypt level  that is set to 2 here and is as client_info says on front side")
        SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);
        
        stream.out_uint32_le(length);
        stream.out_uint32_le(flags);
        if (channel.flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_copy_bytes(data, chunk_size);
        sec_out.end();
        sdrq_out.end();
        tpdu.end();
        tpdu.send(this->trans);
    }

    virtual BackEvent_t draw_event(void)
    {
        try{

        int width = this->gd.get_front_width();
        int height = this->gd.get_front_height();
        int rdp_bpp = this->gd.get_front_bpp();
        bool console_session = this->gd.get_client_info().console_session;
        char * hostname = this->hostname;


        int & userid = this->userid;

        switch (this->state){
        case MOD_RDP_CONNECTING:
            LOG(LOG_INFO, "Connection Initiation");
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
            LOG(LOG_INFO, "x224 connection request PDU sent");
            this->state = MOD_RDP_CONNECTION_INITIATION;
        break;

        case MOD_RDP_CONNECTION_INITIATION:
            LOG(LOG_INFO, "Basic Settings Exchange");
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
                    this->trans, this->gd.front.get_channel_list(), width, height, rdp_bpp, keylayout, console_session, hostname);

            this->state = MOD_RDP_BASIC_SETTINGS_EXCHANGE;
        break;

        case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
            LOG(LOG_INFO, "Channel Connection");
            recv_mcs_connect_response_pdu_with_gcc_conference_create_response(
                    this->trans, this->mod_channel_list, this->gd.front.get_channel_list(),
                    this->encrypt,
                    this->decrypt,
                    this->server_public_key_len,
                    this->client_crypt_random,
                    this->gd.get_client_info().crypt_level,
                    this->use_rdp5);

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
        LOG(LOG_INFO, "RDP Security Commencement");
        {
            recv_mcs_attach_user_confirm_pdu(this->trans, this->userid);
            LOG(LOG_INFO, "send mcs channel join request and recv confirm pdu");
            send_mcs_channel_join_request_and_recv_confirm_pdu(this->trans, this->userid, this->mod_channel_list);

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

            LOG(LOG_INFO, "security exchange_PDU");
            send_security_exchange_PDU(trans,
                this->userid,
                this->server_public_key_len,
                this->client_crypt_random);

            // Secure Settings Exchange
            // ------------------------

            // Secure Settings Exchange: Secure client data (such as the username,
            // password and auto-reconnect cookie) is sent to the server using the Client
            // Info PDU.

            // Client                                                     Server
            //    |------ Client Info PDU      ---------------------------> |

            LOG(LOG_INFO, "Secure Settings Exchange");
            int rdp5_performanceflags = this->gd.get_client_info().rdp5_performanceflags;
            rdp5_performanceflags = RDP5_NO_WALLPAPER;

            this->send_client_info_pdu(
                                this->trans,
                                this->userid,
                                context.get(STRAUTHID_TARGET_PASSWORD),
                                rdp5_performanceflags,
                                this->use_rdp5);

            this->state = MOD_RDP_GET_LICENSE;
        }
        break;

        case MOD_RDP_GET_LICENSE:
        LOG(LOG_INFO, "Licensing");
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
            Transport * trans = this->trans;
            const char * hostname = this->hostname;
            const char * username = this->username;
            const int userid = this->userid;
            int & licence_issued = this->lic_layer.licence_issued;
            int res = 0;
            Stream stream(65536);
            // read tpktHeader (4 bytes = 3 0 len)
            // TPDU class 0    (3 bytes = LI F0 PDU_DT)
            X224In in_tpdu(trans, stream);
            McsIn mcs_in(stream);
            if ((mcs_in.opcode >> 2) != MCS_SDIN) {
                throw Error(ERR_MCS_RECV_ID_NOT_MCS_SDIN);
            }
            SecIn sec(stream, this->decrypt);

            if (sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
                uint8_t tag = stream.in_uint8();
                stream.skip_uint8(3); /* version, length */
                switch (tag) {
                case LICENCE_TAG_DEMAND:
                    LOG(LOG_INFO, "LICENCE_TAG_DEMAND");
                    this->lic_layer.rdp_lic_process_demand(trans, stream, hostname, username, userid, licence_issued);
                    break;
                case LICENCE_TAG_AUTHREQ:
                    LOG(LOG_INFO, "LICENCE_TAG_AUTHREQ");
                    this->lic_layer.rdp_lic_process_authreq(trans, stream, hostname, userid, licence_issued);
                    break;
                case LICENCE_TAG_ISSUE:
                    LOG(LOG_INFO, "LICENCE_TAG_ISSUE");
                    res = this->lic_layer.rdp_lic_process_issue(stream, hostname, licence_issued);
                    break;
                case LICENCE_TAG_REISSUE:
                    LOG(LOG_INFO, "LICENCE_TAG_REISSUE");
                    break;
                case LICENCE_TAG_RESULT:
                    LOG(LOG_INFO, "LICENCE_TAG_RESULT");
                    res = 1;
                    break;
                default:
                    break;
                    /* todo unimpl("licence tag 0x%x\n", tag); */
                }
            }
            else {
                LOG(LOG_INFO, "ERR_SEC_EXPECTED_LICENCE_NEGOTIATION_PDU");
                throw Error(ERR_SEC_EXPECTED_LICENCE_NEGOTIATION_PDU);
            }
            TODO(" we haven't actually read all the actual data available  hence we can't check end. Implement full decoding and activate it.")
    //        in_tpdu.end();
            if (res){
                this->state = MOD_RDP_CONNECTED;
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
            // client and server after the connection has been finalized includes
            // connection management information and virtual channel messages (exchanged
            // between client-side plug-ins and server-side applications).

        case MOD_RDP_CONNECTED:
        {
            Stream stream(65536);
            // read tpktHeader (4 bytes = 3 0 len)
            // TPDU class 0    (3 bytes = LI F0 PDU_DT)
            X224In(this->trans, stream);
            McsIn mcs_in(stream);
            if ((mcs_in.opcode >> 2) != MCS_SDIN) {
                LOG(LOG_ERR, "Error: MCS_SDIN TPDU expected");
                throw Error(ERR_MCS_RECV_ID_NOT_MCS_SDIN);
            }
            SecIn sec(stream, this->decrypt);
            if (sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
                LOG(LOG_ERR, "Error: unexpected licence negotiation sec packet");
                throw Error(ERR_SEC_UNEXPECTED_LICENCE_NEGOTIATION_PDU);
            }

            if (sec.flags & 0x0400){ /* SEC_REDIRECT_ENCRYPT */
                LOG(LOG_INFO, "sec redirect encrypt");
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

            if (mcs_in.chan_id != MCS_GLOBAL_CHANNEL){
                size_t num_channel_src = this->mod_channel_list.size();
                for (size_t index = 0; index < num_channel_src; index++){
                    const McsChannelItem & mod_channel_item = this->mod_channel_list[index];
                    if (mcs_in.chan_id == mod_channel_item.chanid){
                        num_channel_src = index;
                        break;
                    }
                }

                if (num_channel_src >= this->mod_channel_list.size()) {
                    LOG(LOG_ERR, "mod::rdp::MOD_RDP_CONNECTED::Unknown Channel");
                    throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                }

                const McsChannelItem & mod_channel = this->mod_channel_list[num_channel_src];

                uint32_t length = stream.in_uint32_le();
                int flags = stream.in_uint32_le();
                size_t chunk_size = stream.end - stream.p;
               
                this->send_to_front_channel(mod_channel.name, stream.p, length, chunk_size, flags);
                
                stream.p = stream.end;
            }
            else {
                uint8_t * next_packet = stream.p;
                while (next_packet < stream.end) {
                    stream.p = next_packet;
                    int len = stream.in_uint16_le();
                    if (len == 0x8000) {
                        next_packet += 8;
                        LOG(LOG_INFO, "Packet len == 0x8000");
                        continue;
                    }

                    uint16_t pdu_type = stream.in_uint16_le();
                    stream.skip_uint8(2);
                    next_packet += len;

                    switch (pdu_type & 0xF) {
                    case PDUTYPE_DATAPDU:
                        switch (this->connection_finalization_state){
                        case EARLY:
                            LOG(LOG_INFO, "EARLY");
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
                            LOG(LOG_INFO, "process demand active ok\n");
                            this->gd.mod_bpp = this->bpp;
                            this->up_and_running = 1;
                            this->connection_finalization_state = UP_AND_RUNNING;
                        break;
                        case UP_AND_RUNNING:
                        {
                            ShareDataIn share_data_in(stream);
//                            LOG(LOG_INFO, "Up and running");
                            switch (share_data_in.pdutype2) {
                            case PDUTYPE2_UPDATE:
                            {
    // MS-RDPBCGR: 1.3.6
    // -----------------
    // The most fundamental output that a server can send to a connected client
    // is bitmap images of the remote session using the Update Bitmap PDU. This
    // allows the client to render the working space and enables a user to
    // interact with the session running on the server. The global palette
    // information for a session is sent to the client in the Update Palette PDU.

                                int update_type = stream.in_uint16_le();
                                this->gd.server_begin_update();
                                switch (update_type) {
                                case RDP_UPDATE_ORDERS:
                                    {
                                        stream.skip_uint8(2); /* pad */
                                        int count = stream.in_uint16_le();
                                        stream.skip_uint8(2); /* pad */
                                        this->orders.process_orders(this->bpp, stream, count, this);
                                    }
                                    break;
                                case RDP_UPDATE_BITMAP:
                                    this->process_bitmap_updates(stream, this);
                                    break;
                                case RDP_UPDATE_PALETTE:
                                    this->process_palette(stream, this);
                                    break;
                                case RDP_UPDATE_SYNCHRONIZE:
                                    break;
                                default:
                                    break;
                                }
                                this->gd.server_end_update();
                            }
                            break;
                            case PDUTYPE2_CONTROL:
                            break;
                            case PDUTYPE2_SYNCHRONIZE:
                            break;
                            case PDUTYPE2_POINTER:
                                this->process_pointer_pdu(stream, this);
                            break;
                            case PDUTYPE2_PLAY_SOUND:
                            break;
                            case PDUTYPE2_SAVE_SESSION_INFO:
                                LOG(LOG_INFO, "DATA PDU LOGON\n");
                            break;
                            case PDUTYPE2_SET_ERROR_INFO_PDU:
                                LOG(LOG_INFO, "DATA PDU DISCONNECT\n");
                                this->process_disconnect_pdu(stream);
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

                            this->share_id = stream.in_uint32_le();
                            len_src_descriptor = stream.in_uint16_le();
                            len_combined_caps = stream.in_uint16_le();
                            stream.skip_uint8(len_src_descriptor);
                            this->process_server_caps(stream, len_combined_caps, this->use_rdp5);
                            TODO(" we should be able to pack all the following sends to the same X224 TPDU  instead of creating a different one for each send")
                            LOG(LOG_INFO, "Sending confirm active PDU");
                            this->send_confirm_active(mod, this->use_rdp5);
                            LOG(LOG_INFO, "Sending synchronize");
                            this->send_synchronise();
                            LOG(LOG_INFO, "Sending control cooperate");
                            this->send_control(RDP_CTL_COOPERATE);
                            LOG(LOG_INFO, "Sending request control");
                            this->send_control(RDP_CTL_REQUEST_CONTROL);
                            LOG(LOG_INFO, "Sending input synchronize");
                            this->send_input(0, RDP_INPUT_SYNCHRONIZE, 0, 0, 0);
                            LOG(LOG_INFO, "Sending font List");
                            /* Including RDP 5.0 capabilities */
                            if (this->use_rdp5 != 0){
                                LOG(LOG_INFO, "use rdp5");
                                this->enum_bmpcache2();
                                this->send_fonts(3);
                            }
                            else{
                                LOG(LOG_INFO, "not using rdp5");
                                this->send_fonts(1);
                                this->send_fonts(2);
                            }
                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                        }
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        LOG(LOG_INFO, "Deactivate All PDU");
                        this->up_and_running = 0;
                        break;
                    TODO(" this PDUTYPE is undocumented and seems to mean the same as type 10")
                    case RDP_PDU_REDIRECT:
                        break;
                    case 0:
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        }
        }
        catch(Error e){
            try {
                Stream stream(11);
                X224Out tpdu(X224Packet::DR_TPDU, stream);
                tpdu.end();
                tpdu.send(this->trans);
            }
            catch(Error e){
                return (e.id == ERR_SOCKET_CLOSED)?BACK_EVENT_2:BACK_EVENT_1;
            };
            return BACK_EVENT_1;
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



        void out_general_caps(Stream & stream, int use_rdp5)
        {
            stream.out_uint16_le(RDP_CAPSET_GENERAL);
            const uint16_t offset_len = stream.p - stream.data;
            stream.out_uint16_le(0);
            stream.out_uint16_le(1); /* OS major type */
            stream.out_uint16_le(3); /* OS minor type */
            stream.out_uint16_le(0x200); /* Protocol version */
            stream.out_uint16_le(0); /* Pad */
            stream.out_uint16_le(0); /* Compression types */
            stream.out_uint16_le(use_rdp5 ? 0x40d : 0);
            stream.out_uint16_le(0); /* Update capability */
            stream.out_uint16_le(0); /* Remote unshare capability */
            stream.out_uint16_le(0); /* Compression level */
            stream.out_uint16_le(0); /* Pad */
            stream.set_out_uint16_le(RDP_CAPLEN_GENERAL, offset_len);
        }

        void out_bitmap_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending bitmap caps to server\n");
            stream.out_uint16_le(RDP_CAPSET_BITMAP);
            stream.out_uint16_le(RDP_CAPLEN_BITMAP);
            stream.out_uint16_le(this->bpp); /* Preferred bpp */
            stream.out_uint16_le(1); /* Receive 1 BPP */
            stream.out_uint16_le(1); /* Receive 4 BPP */
            stream.out_uint16_le(1); /* Receive 8 BPP */
            stream.out_uint16_le(800); /* Desktop width */
            stream.out_uint16_le(600); /* Desktop height */
            stream.out_uint16_le(0); /* Pad */
            stream.out_uint16_le(1); /* Allow resize */
            stream.out_uint16_le(this->bitmap_compression); /* Support compression */
            stream.out_uint16_le(0); /* Unknown */
            stream.out_uint16_le(1); /* Unknown */
            stream.out_uint16_le(0); /* Pad */
        }



        void out_order_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending order caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_ORDER);
            stream.out_uint16_le(RDP_CAPLEN_ORDER);
            stream.out_clear_bytes(20); /* Terminal desc, pad */
            stream.out_uint16_le(1); /* Cache X granularity */
            stream.out_uint16_le(20); /* Cache Y granularity */
            stream.out_uint16_le(0); /* Pad */
            stream.out_uint16_le(1); /* Max order level */
            stream.out_uint16_le(0x147); /* Number of fonts */
            stream.out_uint16_le(0x2a); /* Capability flags */

            char order_caps[32];

            memset(order_caps, 0, 32);
            TODO(" use symbolic constants for order numerotation")
            order_caps[RDP::DESTBLT] = 1; /* dest blt */
            order_caps[RDP::PATBLT] = 1; /* pat blt */
            order_caps[RDP::SCREENBLT] = 1; /* screen blt */
            order_caps[3] = 1; /* memblt */
            order_caps[4] = 0; /* todo triblt */
            order_caps[8] = 1; /* line */
            order_caps[9] = 1; /* line */
            order_caps[10] = 1; /* rect */
            order_caps[11] = 0; /* todo desksave */
            order_caps[RDP::MEMBLT] = 1; /* memblt another above */
            order_caps[RDP::TRIBLT] = 0; /* triblt another above */
            order_caps[20] = 0; /* todo polygon */
            order_caps[21] = 0; /* todo polygon2 */
            order_caps[RDP::POLYLINE] = 0; /* todo polyline */
            order_caps[25] = 0; /* todo ellipse */
            order_caps[26] = 0; /* todo ellipse2 */
            order_caps[RDP::GLYPHINDEX] = 1; /* text2 */
            stream.out_copy_bytes(order_caps, 32); /* Orders supported */

            stream.out_uint16_le(0x6a1); /* Text capability flags */
            stream.out_clear_bytes(6); /* Pad */
            stream.out_uint32_le(0 * 0x38400); /* Desktop cache size, for desktop_save */
            stream.out_uint32_le(0); /* Unknown */
            stream.out_uint32_le(0x4e4); /* Unknown */
        }

        void out_bmpcache_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending bmpcache caps to server\n");
            TODO(" see details for bmpcache caps")
            stream.out_uint16_le(RDP_CAPSET_BMPCACHE);
            stream.out_uint16_le(RDP_CAPLEN_BMPCACHE);
            int Bpp = nbbytes(this->bpp);
            stream.out_clear_bytes(24); /* unused */
            stream.out_uint16_le(0x258); /* entries */
            stream.out_uint16_le(0x100 * Bpp); /* max cell size */
            stream.out_uint16_le(0x12c); /* entries */
            stream.out_uint16_le(0x400 * Bpp); /* max cell size */
            stream.out_uint16_le(0x106); /* entries */
            stream.out_uint16_le(0x1000 * Bpp); /* max cell size */
        }


        /* Output control capability set */
        void out_control_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending control caps to server\n");
            stream.out_uint16_le(RDP_CAPSET_CONTROL);
            stream.out_uint16_le(RDP_CAPLEN_CONTROL);
            stream.out_uint16_le(0); /* Control capabilities */
            stream.out_uint16_le(0); /* Remote detach */
            stream.out_uint16_le(2); /* Control interest */
            stream.out_uint16_le(2); /* Detach interest */
        }


        void out_activate_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending Activate caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_ACTIVATE);
            stream.out_uint16_le(RDP_CAPLEN_ACTIVATE);
            stream.out_uint16_le(0); /* Help key */
            stream.out_uint16_le(0); /* Help index key */
            stream.out_uint16_le(0); /* Extended help key */
            stream.out_uint16_le(0); /* Window activate */
        }

// 2.2.7.1.5 Pointer Capability Set (TS_POINTER_CAPABILITYSET)

// The TS_POINTER_CAPABILITYSET structure advertises pointer cache sizes and 
// flags and is based on the capability set specified in [T128] section 8.2.11. 
// This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the 
// capability set. This field MUST be set to CAPSTYPE_POINTER (8).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes 
// of the capability data, including the size of the capabilitySetType and 
// lengthCapability fields.

// colorPointerFlag (2 bytes): A 16-bit, unsigned integer. Indicates support for
// color pointers. Since RDP supports monochrome cursors by using Color Pointer
// Updates and New Pointer Updates (sections 2.2.9.1.1.4.4 and 2.2.9.1.1.4.5 
// respectively), the value of this field is ignored and is always assumed to be
// TRUE (at a minimum the Color Pointer Update MUST be supported by an RDP 
// client).

// +---------------+-----------------------------------------+
// |   Value       |            Meaning                      |
// +---------------+-----------------------------------------+
// | 0x0000 FALSE  | Monochrome mouse cursors are supported. |
// +---------------+-----------------------------------------+
// | 0x0001 TRUE   | Color mouse cursors are supported.      |
// +---------------+-----------------------------------------+

// colorPointerCacheSize (2 bytes): A 16-bit, unsigned integer. The number of 
// available slots in the 24 bpp color pointer cache used to store data received
// in the Color Pointer Update (section 2.2.9.1.1.4.4).

// pointerCacheSize (2 bytes): A 16-bit, unsigned integer. The number of 
// available slots in the pointer cache used to store pointer data of arbitrary 
// bit depth received in the New Pointer Update (section 2.2.9.1.1.4.5). 

// If the value contained in this field is zero or the Pointer Capability Set 
// sent from the client does not include this field, the server will not use 
// the New Pointer Update.


        void out_pointer_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending Pointer caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_POINTER);
            stream.out_uint16_le(8); // total length of caps
            stream.out_uint16_le(1); /* colorPointerFlag */
            stream.out_uint16_le(20); /* colorPointerCacheSize */
        }

        void out_share_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending share caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_SHARE);
            stream.out_uint16_le(RDP_CAPLEN_SHARE);
            stream.out_uint16_le(0); /* userid */
            stream.out_uint16_le(0); /* pad */
        }

        void out_colcache_caps(Stream & stream)
        {
            LOG(LOG_INFO, "Sending colcache caps to server\n");

            stream.out_uint16_le(RDP_CAPSET_COLCACHE);
            stream.out_uint16_le(RDP_CAPLEN_COLCACHE);
            stream.out_uint16_le(6); /* cache size */
            stream.out_uint16_le(0); /* pad */
        }

        void out_sound_caps(Stream & stream)
        {
            const char caps_sound[] = { 0x01, 0x00, 0x00, 0x00 };
            this->out_unknown_caps(stream, 0x0c, 0x08, caps_sound);
        }


        void out_input_caps(Stream & stream)
        {
            const char caps_input[] = {
            0x01, 0x00, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
            };
            this->out_unknown_caps(stream, 0x0d, 0x58, caps_input); /* international? */   // RDP_CAPLEN_0x0D 88
        }

        void out_font_caps(Stream & stream)
        {
            const char caps_font[] = { 0x01, 0x00, 0x00, 0x00 };
            this->out_unknown_caps(stream, 0x0e, 0x08, caps_font);   // RDP_CAPLEN_0x0E 8
        }

// 2.2.7.1.8 Glyph Cache Capability Set (TS_GLYPHCACHE_CAPABILITYSET)
// ==================================================================

// The TS_GLYPHCACHE_CAPABILITYSET structure advertises the glyph support level
// and associated cache sizes. This capability is only sent from client to 
// server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the 
// capability set. This field MUST be set to CAPSTYPE_GLYPHCACHE (16).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes 
// of the capability data, including the size of the capabilitySetType and 
// lengthCapability fields.

// GlyphCache (40 bytes): An array of 10 TS_CACHE_DEFINITION structures. An 
// ordered specification of the layout of each of the glyph caches with IDs 0 
// through to 9 ([MS-RDPEGDI] section 3.1.1.1.2).

// FragCache (4 bytes): Fragment cache data. The maximum number of entries 
// allowed in the cache is 256, and the largest allowed maximum size of an 
// element is 256 bytes.

// GlyphSupportLevel (2 bytes): A 16-bit, unsigned integer. The level of glyph 
// support.

// +-------------------------------+-------------------------------------------+
// |         Value                 |                    Meaning                |
// +-------------------------------+-------------------------------------------+
// | 0x0000 GLYPH_SUPPORT_NONE     | The client does not support glyph caching.|
// |                               | All text output will be sent to the client|
// |                               | as expensive Bitmap Updates (see sections |
// |                               | 2.2.9.1.1.3.1.2 and 2.2.9.1.2.1.2).       |
// +-------------------------------+-------------------------------------------+
// | 0x0001 GLYPH_SUPPORT_PARTIAL  | Indicates support for Revision 1 Cache    |
// |                               | Glyph Secondary Drawing Orders (see       |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.2.5).      |
// +-------------------------------+-------------------------------------------+
// | 0x0002 GLYPH_SUPPORT_FULL     | Indicates support for Revision 1 Cache    |
// |                               | Glyph Secondary Drawing Orders (see       |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.2.5).      |
// +-------------------------------+-------------------------------------------+
// | 0x0003 GLYPH_SUPPORT_ENCODE   | Indicates support for Revision 2 Cache    |
// |                               | Glyph Secondary Drawing Orders (see       |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.2.6).      |
// +-------------------------------+-------------------------------------------+

//If the GlyphSupportLevel is greater than GLYPH_SUPPORT_NONE (0), the client 
//  MUST support the GlyphIndex Primary Drawing Order (see [MS-RDPEGDI] section 
//  2.2.2.2.1.1.2.13) or the FastIndex Primary Drawing Order (see [MS-RDPEGDI] 
//  section 2.2.2.2.1.1.2.14). If the FastIndex Primary Drawing Order is not 
//  supported, then support for the GlyphIndex Primary Drawing Order is assumed 
//  by the server (order support is specified in the Order Capability Set, as
//  described in section 2.2.7.1.3).

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this   
//   field MUST be ignored.

        void out_glyphcache_caps(Stream & stream)
        {
            stream.out_uint16_le(RDP_CAPSET_GLYPHCACHE);
            uint16_t offset_length = stream.p - stream.data;
            stream.out_uint16_le(0);
            uint16_t length = stream.p - stream.data;
            static const char glyphcache[] = {
            0xFE, 0x00, 0x04, 0x00, 0xFE, 0x00, 0x04, 0x00,
            0xFE, 0x00, 0x08, 0x00, 0xFE, 0x00, 0x08, 0x00,
            0xFE, 0x00, 0x10, 0x00, 0xFE, 0x00, 0x20, 0x00,
            0xFE, 0x00, 0x40, 0x00, 0xFE, 0x00, 0x80, 0x00,
            0xFE, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x08};
            stream.out_copy_bytes(glyphcache, 40);
            stream.out_uint32_le(0x01000100);
            stream.out_uint16_le(0x0000);
            stream.out_uint16_le(0);
            length = stream.p - stream.data - length;
            stream.set_out_uint16_le(length, offset_length);
            
        }

        // 2.2.1.13.1.1 Demand Active PDU Data (TS_DEMAND_ACTIVE_PDU)
        // ==========================================================
        // The TS_DEMAND_ACTIVE_PDU structure is a standard T.128 Demand Active PDU (see [T128] section 8.4.1).

        // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1) containing information about the packet. The type subfield of the pduType field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).

        // shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the packet (see [T128] section 8.4.2 for more information regarding share IDs).

        // lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes of the sourceDescriptor field.

        // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined size in bytes of the numberCapabilities, pad2Octets, and capabilitySets fields.
        
        // sourceDescriptor (variable): A variable-length array of bytes containing a source descriptor (see [T128] section 8.4.1 for more information regarding source descriptors).
        
        // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets included in the Demand Active PDU.
        
        // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

        // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number of capability sets is specified by the numberCapabilities field.

        // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.


        void send_confirm_active(client_mod * mod, int use_rdp5) throw(Error)
        {
            LOG(LOG_INFO, "Sending confirm active to server\n");

            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);

        // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1) containing information about the packet. The type subfield of the pduType field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).

            ShareControlOut rdp_control_out(stream, PDUTYPE_CONFIRMACTIVEPDU, this->userid + MCS_USERCHANNEL_BASE);

        // shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the packet (see [T128] section 8.4.2 for more information regarding share IDs).

            stream.out_uint32_le(this->share_id);
//            stream.out_uint16_le(1002); /* userid */
            stream.out_uint16_le(1002); /* userid : this parameter seems not to be documented ? */

        // lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes of the sourceDescriptor field.
            stream.out_uint16_le(5);

        // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined size in bytes of the numberCapabilities, pad2Octets, and capabilitySets fields.
        
            uint16_t offset_caplen = stream.p - stream.data; 
            stream.out_uint16_le(0); // caplen
            
        // sourceDescriptor (variable): A variable-length array of bytes containing a source descriptor (see [T128] section 8.4.1 for more information regarding source descriptors).
            stream.out_copy_bytes("MSTSC", 5);

        // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets included in the Demand Active PDU.
            uint16_t offset_capscount = stream.p - stream.data; 
            uint16_t capscount = 0;
            stream.out_uint16_le(0); /* num_caps */

        // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.
            stream.out_clear_bytes(2); /* pad */

        // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number of capability sets is specified by the numberCapabilities field.
            uint16_t total_caplen = stream.p - stream.data;

            capscount++; this->out_general_caps(stream, use_rdp5);
            capscount++; this->out_bitmap_caps(stream);
            capscount++; this->out_order_caps(stream);
            capscount++; this->out_bmpcache_caps(stream);

            if(use_rdp5){
                capscount++;
                this->out_bmpcache2_caps(stream, mod->gd.get_client_info());
            }
            capscount++; this->out_colcache_caps(stream);
            capscount++; this->out_activate_caps(stream);
            capscount++; this->out_control_caps(stream);
            capscount++; this->out_pointer_caps(stream);
            capscount++; this->out_share_caps(stream);
            capscount++; this->out_input_caps(stream);
            capscount++; this->out_sound_caps(stream);
            capscount++; this->out_font_caps(stream);
            capscount++; this->out_glyphcache_caps(stream);

            total_caplen = stream.p - stream.data - total_caplen;

            // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.
            stream.out_uint32_le(0);

//            stream.set_out_uint16_le(stream.p - stream.data - offset_caplen - 47, offset_caplen); // caplen
//            stream.set_out_uint16_le(caplen, offset_caplen); // caplen
            stream.set_out_uint16_le(total_caplen + 4, offset_caplen); // caplen
            LOG(LOG_INFO, "total_caplen = %u, caplen=%u computed caplen=%u offset_here = %u offset_caplen=%u", total_caplen, 388, stream.p - stream.data - offset_caplen, stream.p - stream.data, offset_caplen);
            stream.set_out_uint16_le(capscount, offset_capscount); // caplen

            rdp_control_out.end();
            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->trans);

            LOG(LOG_INFO, "Waiting for answer to confirm active\n");
        }


        void out_unknown_caps(Stream & stream, int id, int length, const char * caps)
        {
//            LOG(LOG_INFO, "Sending unknown caps to server\n");
            stream.out_uint16_le(id);
            stream.out_uint16_le(length);
            stream.out_copy_bytes(caps, length - 4);
        }


        void process_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
        {
//            LOG(LOG_INFO, "Process pointer PDU\n");

            int message_type = stream.in_uint16_le();
            stream.skip_uint8(2); /* pad */
            switch (message_type) {
            case RDP_POINTER_MOVE:
            {
                TODO(" implement RDP_POINTER_MOVE")
                /* int x = */ stream.in_uint16_le();
                /* int y = */ stream.in_uint16_le();
            }
            break;
            case RDP_POINTER_COLOR:
//                LOG(LOG_INFO, "Process pointer color\n");
                this->process_color_pointer_pdu(stream, mod);
//                LOG(LOG_INFO, "Process pointer color done\n");
                break;
            case RDP_POINTER_CACHED:
//                LOG(LOG_INFO, "Process pointer cached\n");
                this->process_cached_pointer_pdu(stream, mod);
//                LOG(LOG_INFO, "Process pointer cached done\n");
                break;
            case RDP_POINTER_SYSTEM:
//                LOG(LOG_INFO, "Process pointer system\n");
                this->process_system_pointer_pdu(stream, mod);
//                LOG(LOG_INFO, "Process pointer system done\n");
                break;
            default:
                break;
            }
//            LOG(LOG_INFO, "Process pointer PDU done\n");
        }

        void process_palette(Stream & stream, client_mod * mod)
        {
//            LOG(LOG_INFO, "Process palette\n");

            stream.skip_uint8(2); /* pad */
            uint16_t numberColors = stream.in_uint32_le();
            assert(numberColors == 256);
            for (int i = 0; i < 256; i++) {
                uint8_t r = stream.in_uint8();
                uint8_t g = stream.in_uint8();
                uint8_t b = stream.in_uint8();
//                uint32_t color = stream.in_bytes_le(3);
                this->orders.global_palette[i] = (r << 16)|(g << 8)|b;
                this->orders.memblt_palette[i] = (b << 16)|(g << 8)|r;
            }
            mod->gd.set_mod_palette(this->orders.global_palette);
        }

// 2.2.5.1.1 Set Error Info PDU Data (TS_SET_ERROR_INFO_PDU)
// =========================================================
// The TS_SET_ERROR_INFO_PDU structure contains the contents of the Set Error Info PDU, which is a
// Share Data Header (section 2.2.8.1.1.1.2) with an error value field.

// shareDataHeader (18 bytes): Share Data Header containing information about the packet.
// The type subfield of the pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
// MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header MUST
// be set to PDUTYPE2_SET_ERROR_INFO_PDU (47), and the pduSource field MUST be set to 0.

// errorInfo (4 bytes): A 32-bit, unsigned integer. Error code.
// Protocol-independent codes:

// 0x00000001 ERRINFO_RPC_INITIATED_DISCONNECT The disconnection was initiated by an administrative tool on the server in another session.

// 0x00000002 ERRINFO_RPC_INITIATED_LOGOFF The disconnection was due to a forced logoff initiated by an administrative tool on the server in another session.

// 0x00000003 ERRINFO_IDLE_TIMEOUT The idle session limit timer on the server has elapsed.

// 0x00000004 ERRINFO_LOGON_TIMEOUT The active session limit timer on the server has elapsed.

// 0x00000005 ERRINFO_DISCONNECTED_BY_OTHERCONNECTION Another user connected to the server, forcing the disconnection of the current connection.

// 0x00000006 ERRINFO_OUT_OF_MEMORY The server ran out of available memory resources.

// 0x00000007 ERRINFO_SERVER_DENIED_CONNECTION The server denied the connection.
//
// 0x00000009 ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES The user cannot connect to the server due to insufficient access privileges.

// 0x0000000A ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED The server does not accept saved user credentials and requires that the user enter their credentials for each connection.
//
// 0x0000000B ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER The disconnection was initiated by an administrative tool on the server running in the user's session.

// Protocol-independent licensing codes:

// 0x00000100 ERRINFO_LICENSE_INTERNAL An internal error has occurred in the Terminal Services licensing component.

// 0x00000101 ERRINFO_LICENSE_NO_LICENSE_SERVER A Remote Desktop License Server ([MS-RDPELE] section 1.1) could not be found to provide a license.

// 0x00000102 ERRINFO_LICENSE_NO_LICENSE There are no Client Access Licenses ([MS-RDPELE] section 1.1) available for the target remote computer.

// 0x00000103 ERRINFO_LICENSE_BAD_CLIENT_MSG The remote computer received an invalid licensing message from the client.

// 0x00000104 ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE The Client Access License ([MS-RDPELE] section 1.1) stored by the client has been modified.

// 0x00000105 ERRINFO_LICENSE_BAD_CLIENT_LICENSE The Client Access License ([MS-RDPELE] section 1.1) stored by the client is in an invalid format

// 0x00000106 ERRINFO_LICENSE_CANT_FINISH_PROTOCOL Network problems have caused the licensing protocol ([MS-RDPELE] section 1.3.3) to be terminated.

// 0x00000107 ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL The client prematurely ended the licensing protocol ([MS-RDPELE] section 1.3.3).

// 0x00000108 ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION A licensing message ([MS-RDPELE] sections 2.2 and 5.1) was incorrectly encrypted.

// 0x00000109 ERRINFO_LICENSE_CANT_UPGRADE_LICENSE The Client Access License ([MS-RDPELE] section 1.1) stored by the client could not be upgraded or renewed.

// 0x0000010A ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS The remote computer is not licensed to accept remote connections

// RDP specific codes:

// 0x000010C9 ERRINFO_UNKNOWNPDUTYPE2 Unknown pduType2 field in a received Share Data Header (section 2.2.8.1.1.1.2).

// 0x000010CA ERRINFO_UNKNOWNPDUTYPE Unknown pduType field in a received Share Control Header (section 2.2.8.1.1.1.1).

// 0x000010CB ERRINFO_DATAPDUSEQUENCE An out-of-sequence Slow-Path Data PDU (section 2.2.8.1.1.1.1) has been received.

// 0x000010CD ERRINFO_CONTROLPDUSEQUENCE An out-of-sequence Slow-Path Non-Data PDU (section 2.2.8.1.1.1.1) has been received.

// 0x000010CE ERRINFO_INVALIDCONTROLPDUACTION A Control PDU (sections 2.2.1.15 and 2.2.1.16) has been received with an invalid action field.

// 0x000010CF ERRINFO_INVALIDINPUTPDUTYPE (a) A Slow-Path Input Event (section 2.2.8.1.1.3.1.1) has been received with an invalid messageType field. (b) A Fast-Path Input Event (section 2.2.8.1.2.2) has been received with an invalid eventCode field.

// 0x000010D0 ERRINFO_INVALIDINPUTPDUMOUSE (a) A Slow-Path Mouse Event (section 2.2.8.1.1.3.1.1.3) or Extended Mouse Event (section 2.2.8.1.1.3.1.1.4) has been received with an invalid pointerFlags field. (b) A Fast-Path Mouse Event (section 2.2.8.1.2.2.3) or Fast-Path Extended Mouse Event (section 2.2.8.1.2.2.4) has been received with an invalid pointerFlags field.

// 0x000010D1 ERRINFO_INVALIDREFRESHRECTPDU An invalid Refresh Rect PDU (section 2.2.11.2) has been received.

// 0x000010D2 ERRINFO_CREATEUSERDATAFAILED The server failed to construct the GCC Conference Create Response user data (section 2.2.1.4).

// 0x000010D3 ERRINFO_CONNECTFAILED Processing during the Channel Connection phase of the RDP Connection Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence phases) has failed.

// 0x000010D4 ERRINFO_CONFIRMACTIVEWRONGSHAREID A Confirm Active PDU (section 2.2.1.13.2) was received from the client with an invalid shareId field.

// 0x000010D5 ERRINFO_CONFIRMACTIVEWRONGORIGINATOR A Confirm Active PDU (section 2.2.1.13.2) was received from the client with an invalid originatorId field.

// 0x000010DA ERRINFO_PERSISTENTKEYPDUBADLENGTH There is not enough data to process a Persistent Key List PDU (section 2.2.1.17).

// 0x000010DB ERRINFO_PERSISTENTKEYPDUILLEGALFIRST A Persistent Key List PDU (section 2.2.1.17) marked as PERSIST_PDU_FIRST (0x01) was received after the reception of a prior Persistent Key List PDU also marked as PERSIST_PDU_FIRST.

// 0x000010DC ERRINFO_PERSISTENTKEYPDUTOOMANYTOTALKEYS A Persistent Key List PDU (section 2.2.1.17) was received which specified a total number of bitmap cache entries larger than 262144.

// 0x000010DD ERRINFO_PERSISTENTKEYPDUTOOMANYCACHEKEYS A Persistent Key List PDU (section 2.2.1.17) was received which specified an invalid total number of keys for a bitmap cache (the number of entries that can be stored within each bitmap cache is specified in the Revision 1 or 2 Bitmap Cache Capability Set (section 2.2.7.1.4) that is sent from client to server).

// 0x000010DE ERRINFO_INPUTPDUBADLENGTH There is not enough data to process Input Event PDU Data (section 2.2.8.1.1.3. 2.2.8.1.2).

// 0x000010DF ERRINFO_BITMAPCACHEERRORPDUBADLENGTH There is not enough data to process the shareDataHeader, NumInfoBlocks, Pad1, and Pad2 fields of the Bitmap Cache Error PDU Data ([MS-RDPEGDI] section 2.2.2.3.1.1).

// 0x000010E0  ERRINFO_SECURITYDATATOOSHORT (a) The dataSignature field of the Fast-Path Input Event PDU (section 2.2.8.1.2) does not contain enough data. (b) The fipsInformation and dataSignature fields of the Fast-Path Input Event PDU (section 2.2.8.1.2) do not contain enough data.

// 0x000010E1 ERRINFO_VCHANNELDATATOOSHORT (a) There is not enough data in the Client Network Data (section 2.2.1.3.4) to read the virtual channel configuration data. (b) There is not enough data to read a complete Channel PDU Header (section 2.2.6.1.1).

// 0x000010E2 ERRINFO_SHAREDATATOOSHORT (a) There is not enough data to process Control PDU Data (section 2.2.1.15.1). (b) There is not enough data to read a complete Share Control Header (section 2.2.8.1.1.1.1). (c) There is not enough data to read a complete Share Data Header (section 2.2.8.1.1.1.2) of a Slow-Path Data PDU (section 2.2.8.1.1.1.1). (d) There is not enough data to process Font List PDU Data (section 2.2.1.18.1).

// 0x000010E3 ERRINFO_BADSUPRESSOUTPUTPDU (a) There is not enough data to process Suppress Output PDU Data (section 2.2.11.3.1). (b) The allowDisplayUpdates field of the Suppress Output PDU Data (section 2.2.11.3.1) is invalid.

// 0x000010E5 ERRINFO_CONFIRMACTIVEPDUTOOSHORT (a) There is not enough data to read the shareControlHeader, shareId, originatorId, lengthSourceDescriptor, and lengthCombinedCapabilities fields of the Confirm Active PDU Data (section 2.2.1.13.2.1). (b) There is not enough data to read the sourceDescriptor, numberCapabilities, pad2Octets, and capabilitySets fields of the Confirm Active PDU Data (section 2.2.1.13.2.1).

// 0x000010E7 ERRINFO_CAPABILITYSETTOOSMALL There is not enough data to read the capabilitySetType and the lengthCapability fields in a received Capability Set (section 2.2.1.13.1.1.1).

// 0x000010E8 ERRINFO_CAPABILITYSETTOOLARGE A Capability Set (section 2.2.1.13.1.1.1) has been received with a lengthCapability field that contains a value greater than the total length of the data received.

// 0x000010E9 ERRINFO_NOCURSORCACHE (a) Both the colorPointerCacheSize and pointerCacheSize fields in the Pointer Capability Set (section 2.2.7.1.5) are set to zero. (b) The pointerCacheSize field in the Pointer Capability Set (section 2.2.7.1.5) is not present, and the colorPointerCacheSize field is set to zero.

// 0x000010EA ERRINFO_BADCAPABILITIES The capabilities received from the client in the Confirm Active PDU (section 2.2.1.13.2) were not accepted by the server.

// 0x000010EC ERRINFO_VIRTUALCHANNELDECOMPRESSIONERR An error occurred while using the bulk compressor (section 3.1.8 and [MS- RDPEGDI] section 3.1.8) to decompress a Virtual Channel PDU (section 2.2.6.1)

// 0x000010ED ERRINFO_INVALIDVCCOMPRESSIONTYPE An invalid bulk compression package was specified in the flags field of the Channel PDU Header (section 2.2.6.1.1).

// 0x000010EF ERRINFO_INVALIDCHANNELID An invalid MCS channel ID was specified in the mcsPdu field of the Virtual Channel PDU (section 2.2.6.1).

// 0x000010F0 ERRINFO_VCHANNELSTOOMANY The client requested more than the maximum allowed 31 static virtual channels in the Client Network Data (section 2.2.1.3.4).

// 0x000010F3 ERRINFO_REMOTEAPPSNOTENABLED The INFO_RAIL flag (0x00008000) MUST be set in the flags field of the Info Packet (section 2.2.1.11.1.1) as the session on the  remote server can only host remote applications.

// 0x000010F4 ERRINFO_CACHECAPNOTSET The client sent a Persistent Key List PDU (section 2.2.1.17) without including the prerequisite Revision 2 Bitmap Cache Capability Set (section 2.2.7.1.4.2) in the Confirm Active PDU (section 2.2.1.13.2).

// 0x000010F5 ERRINFO_BITMAPCACHEERRORPDUBADLENGTH2 The NumInfoBlocks field in the Bitmap Cache Error PDU Data is inconsistent with the amount of data in the Info field ([MS-RDPEGDI] section 2.2.2.3.1.1).

// 0x000010F6 ERRINFO_OFFSCRCACHEERRORPDUBADLENGTH There is not enough data to process an Offscreen Bitmap Cache Error PDU ([MS-RDPEGDI] section 2.2.2.3.2).

// 0x000010F7 ERRINFO_DNGCACHEERRORPDUBADLENGTH There is not enough data to process a DrawNineGrid Cache Error PDU ([MS-RDPEGDI] section 2.2.2.3.3).

// 0x000010F8 ERRINFO_GDIPLUSPDUBADLENGTH There is not enough data to process a GDI+ Error PDU ([MS-RDPEGDI] section 2.2.2.3.4).

// 0x00001111 ERRINFO_SECURITYDATATOOSHORT2 There is not enough data to read a Basic Security Header (section 2.2.8.1.1.2.1).

// 0x00001112 ERRINFO_SECURITYDATATOOSHORT3 There is not enough data to read a Non- FIPS Security Header (section 2.2.8.1.1.2.2) or FIPS Security Header (section 2.2.8.1.1.2.3).

// 0x00001113 ERRINFO_SECURITYDATATOOSHORT4 There is not enough data to read the basicSecurityHeader and length fields of the Security Exchange PDU Data (section 2.2.1.10.1).

// 0x00001114 ERRINFO_SECURITYDATATOOSHORT5 There is not enough data to read the CodePage, flags, cbDomain, cbUserName, cbPassword, cbAlternateShell, cbWorkingDir, Domain, UserName, Password, AlternateShell, and WorkingDir fields in the Info Packet (section 2.2.1.11.1.1).

// 0x00001115 ERRINFO_SECURITYDATATOOSHORT6 There is not enough data to read the CodePage, flags, cbDomain, cbUserName, cbPassword, cbAlternateShell, and cbWorkingDir fields in the Info Packet (section 2.2.1.11.1.1).

// 0x00001116 ERRINFO_SECURITYDATATOOSHORT7 There is not enough data to read the clientAddressFamily and cbClientAddress fields in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001117 ERRINFO_SECURITYDATATOOSHORT8 There is not enough data to read the clientAddress field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001118 ERRINFO_SECURITYDATATOOSHORT9 There is not enough data to read the cbClientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001119 ERRINFO_SECURITYDATATOOSHORT10 There is not enough data to read the clientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x0000111A ERRINFO_SECURITYDATATOOSHORT11 There is not enough data to read the clientTimeZone field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x0000111B ERRINFO_SECURITYDATATOOSHORT12 There is not enough data to read the clientSessionId field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x0000111C ERRINFO_SECURITYDATATOOSHORT13 There is not enough data to read the performanceFlags field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x0000111D ERRINFO_SECURITYDATATOOSHORT14 There is not enough data to read the cbAutoReconnectLen field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x0000111E ERRINFO_SECURITYDATATOOSHORT15 There is not enough data to read the autoReconnectCookie field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x0000111F ERRINFO_SECURITYDATATOOSHORT16 The cbAutoReconnectLen field in the Extended Info Packet (section 2.2.1.11.1.1.1) contains a value which is larger than the maximum allowed length of 128 bytes.

// 0x00001120 ERRINFO_SECURITYDATATOOSHORT17 There is not enough data to read the clientAddressFamily and cbClientAddress fields in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001121 ERRINFO_SECURITYDATATOOSHORT18 There is not enough data to read the clientAddress field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001122 ERRINFO_SECURITYDATATOOSHORT19 There is not enough data to read the cbClientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001123 ERRINFO_SECURITYDATATOOSHORT20 There is not enough data to read the clientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001124 ERRINFO_SECURITYDATATOOSHORT21 There is not enough data to read the clientTimeZone field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001125 ERRINFO_SECURITYDATATOOSHORT22 There is not enough data to read the clientSessionId field in the Extended Info Packet (section 2.2.1.11.1.1.1).

// 0x00001126 ERRINFO_SECURITYDATATOOSHORT23 There is not enough data to read the Client Info PDU Data (section 2.2.1.11.1).

// 0x00001129 ERRINFO_BADMONITORDATA The monitorCount field in the Client Monitor Data (section 2.2.1.3.6) is invalid.

// 0x0000112A ERRINFO_VCDECOMPRESSEDREASSEMBLEFAILED The server-side decompression buffer is invalid, or the size of the decompressed VC data exceeds the chunking size specified in the Virtual Channel Capability Set (section 2.2.7.1.10).

// 0x0000112B ERRINFO_VCDATATOOLONG The size of a received Virtual Channel PDU (section 2.2.6.1) exceeds the chunking size specified in the Virtual Channel Capability Set (section 2.2.7.1.10).

// 0x0000112C ERRINFO_RESERVED Reserved for future use.

// 0x0000112D ERRINFO_GRAPHICSMODENOTSUPPORTED The graphics mode requested by the client is not supported by the server.

// 0x0000112E ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED The server-side graphics subsystem failed  to reset.

// 0x00001191 ERRINFO_UPDATESESSIONKEYFAILED An attempt to update the session keys while using Standard RDP Security mechanisms (section 5.3.7) failed.

// 0x00001192 ERRINFO_DECRYPTFAILED (a) Decryption using Standard RDP Security mechanisms (section 5.3.6) failed. (b) Session key creation using Standard RDP Security mechanisms (section 5.3.5) failed.

// 0x00001193 ERRINFO_ENCRYPTFAILED Encryption using Standard RDP Security mechanisms (section 5.3.6) failed.

// 0x00001194 ERRINFO_ENCPKGMISMATCH Failed to find a usable Encryption Method (section 5.3.2) in the encryptionMethods field of the Client Security Data (section 2.2.1.4.3).

// 0x00001195 ERRINFO_DECRYPTFAILED2 2.2.5.2 Encryption using Standard RDP Security mechanisms (section 5.3.6) failed. Unencrypted data was encountered in a protocol stream which is meant to be encrypted with Standard RDP Security mechanisms (section 5.3.6).

        void process_disconnect_pdu(Stream & stream)
        {
            uint32_t errorInfo = stream.in_uint32_le();
            LOG(LOG_INFO, "process disconnect pdu : code = %8x\n", errorInfo);
        }

        void process_general_caps(Stream & stream, int & use_rdp5)
        {
            stream.skip_uint8(10);
            /* Receiving rdp_5 extra flags supported for RDP 5.0 and later versions*/
            int extraflags = stream.in_uint16_le();
            TODO(" strange: causality seems inverted")
            if (extraflags == 0){
                use_rdp5 = 0;
            }
            LOG(LOG_INFO, "process general caps %d", extraflags);
        }

// 2.2.7.1.2    Bitmap Capability Set (TS_BITMAP_CAPABILITYSET)
// ============================================================

//  The TS_BITMAP_CAPABILITYSET structure is used to advertise bitmap-oriented
//    characteristics and is based on the capability set specified in [T128]
// section 8.2.4. This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//   capability set. This field MUST be set to CAPSTYPE_BITMAP (2).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//   of the capability data, including the size of the capabilitySetType and
//   lengthCapability fields.

// preferredBitsPerPixel (2 bytes): A 16-bit, unsigned integer. Color depth of
//   the remote session. In RDP 4.0 and 5.0, this field MUST be set to 8 (even
//   for a 16-color session).

// receive1BitPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   the client can receive 1 bpp. This field is ignored and SHOULD be set to
//   TRUE (0x0001).

// receive4BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   the client can receive 4 bpp. This field is ignored and SHOULD be set to
//   TRUE (0x0001).

// receive8BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//    the client can receive 8 bpp. This field is ignored and SHOULD be set to
//    TRUE (0x0001).

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The width of the desktop
//   in the remote session.

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The height of the
//   desktop in the remote session.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//   field are ignored.

// desktopResizeFlag (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   desktop resizing is supported.
//   0x0000 FALSE  Desktop resizing is not supported.
//   0x0001 TRUE   Desktop resizing is supported.
//   If a desktop resize occurs, the server will deactivate the session (see
//   section 1.3.1.3), and on session reactivation will specify the new desktop
//   size in the desktopWidth and desktopHeight fields in the Bitmap Capability
//   Set, along with a value of TRUE for the desktopResizeFlag field. The client
//   should check these sizes and, if different from the previous desktop size,
//   resize any windows to support this size.

// bitmapCompressionFlag (2 bytes): A 16-bit, unsigned integer. Indicates
//   whether the client supports bitmap compression. RDP requires bitmap
//   compression and hence this field MUST be set to TRUE (0x0001). If it is not
//   set to TRUE, the server MUST NOT continue with the connection.

// highColorFlags (1 byte): An 8-bit, unsigned integer. Client support for
//   16 bpp color modes. This field is ignored and SHOULD be set to 0.

// drawingFlags (1 byte): An 8-bit, unsigned integer. Flags describing support
//   for 32 bpp bitmaps.
// 0x02 DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY Indicates support for lossy compression of 32 bpp bitmaps by reducing color-fidelity on a per-pixel basis.
// 0x04 DRAW_ALLOW_COLOR_SUBSAMPLING      Indicates support for chroma subsampling when compressing 32 bpp bitmaps.
// 0x08 DRAW_ALLOW_SKIP_ALPHA             Indicates that the client supports the removal of the alpha-channel when compressing 32 bpp bitmaps. In this case the alpha is assumed to be 0xFF, meaning the bitmap is opaque.
// Compression of 32 bpp bitmaps is specified in [MS-RDPEGDI] section 3.1.9.

// multipleRectangleSupport (2 bytes): A 16-bit, unsigned integer. Indicates
//   whether the client supports the use of multiple bitmap rectangles. RDP
//   requires the use of multiple bitmap rectangles and hence this field MUST be
//   set to TRUE (0x0001). If it is not set to TRUE, the server MUST NOT
//   continue with the connection.

// pad2octetsB (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field are ignored.

        /* Process a bitmap capability set */
        void process_bitmap_caps(Stream & stream)
        {
            this->bpp = stream.in_uint16_le();
            stream.skip_uint8(6);
            int width = stream.in_uint16_le();
            int height = stream.in_uint16_le();
            /* todo, call reset if needed and use width and height */
            LOG(LOG_INFO, "Server bitmap caps (%dx%dx%d) [bpp=%d] ok\n", width, height, bpp, this->bpp);
        }


        void process_server_caps(Stream & stream, int len, int use_rdp5)
        {
            int n;
            int ncapsets;
            int capset_type;
            int capset_length;
            uint8_t* next;
            uint8_t* start;

            start = stream.p;
            ncapsets = stream.in_uint16_le();
            stream.skip_uint8(2); /* pad */
            for (n = 0; n < ncapsets; n++) {
                if (stream.p > start + len) {
                    return;
                }
                capset_type = stream.in_uint16_le();
                capset_length = stream.in_uint16_le();
                next = (stream.p + capset_length) - 4;
                switch (capset_type) {
                case RDP_CAPSET_GENERAL:
                    this->process_general_caps(stream, use_rdp5);
                    break;
                case RDP_CAPSET_BITMAP:
                    this->process_bitmap_caps(stream);
                    break;
                default:
                    break;
                }
                stream.p = next;
            }
        }


        void send_control(int action) throw (Error)
        {
            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);
            ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
            ShareDataOut rdp_data_out(stream, PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

            stream.out_uint16_le(action);
            stream.out_uint16_le(0); /* userid */
            stream.out_uint32_le(0); /* control id */

            rdp_data_out.end();
            rdp_control_out.end();
            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->trans);
        }

        TODO(" duplicated code in front")
        void send_synchronise() throw (Error)
        {
            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);
            ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
            ShareDataOut rdp_data_out(stream, PDUTYPE2_SYNCHRONIZE, this->share_id, RDP::STREAM_MED);

            stream.out_uint16_le(1); /* type */
            stream.out_uint16_le(1002);

            rdp_data_out.end();
            rdp_control_out.end();
            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->trans);
        }

        void send_fonts(int seq) throw(Error)
        {
            Stream stream(65536);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);
            ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
            ShareDataOut rdp_data_out(stream, PDUTYPE2_FONTLIST, this->share_id, RDP::STREAM_MED);

            stream.out_uint16_le(0); /* number of fonts */
            stream.out_uint16_le(0); /* pad? */
            stream.out_uint16_le(seq); /* unknown */
            stream.out_uint16_le(0x32); /* entry size */

            rdp_data_out.end();
            rdp_control_out.end();
            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->trans);
        }

    #define RDP5_FLAG 0x0030

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
//            LOG(LOG_INFO, "send_input\n");

            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            McsOut sdrq_out(stream, MCS_SDRQ, this->userid, MCS_GLOBAL_CHANNEL);
            SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);
            ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
            ShareDataOut rdp_data_out(stream, PDUTYPE2_INPUT, this->share_id, RDP::STREAM_HI);

            stream.out_uint16_le(1); /* number of events */
            stream.out_uint16_le(0);
            stream.out_uint32_le(time);
            stream.out_uint16_le(message_type);
            stream.out_uint16_le(device_flags);
            stream.out_uint16_le(param1);
            stream.out_uint16_le(param2);

            rdp_data_out.end();
            rdp_control_out.end();
            sec_out.end();
            sdrq_out.end();
            tpdu.end();
            tpdu.send(this->trans);
        }

        virtual void rdp_input_invalidate(const Rect & r)
        {
            if (this->up_and_running) {
//                LOG(LOG_INFO, "rdp_input_invalidate");
                if (!r.isempty()){
                    Stream stream(8192);
                    X224Out tpdu(X224Packet::DT_TPDU, stream);
                    McsOut sdrq_out(stream, MCS_SDRQ, this->userid, MCS_GLOBAL_CHANNEL);
                    SecOut sec_out(stream, 2, SEC_ENCRYPT, this->encrypt);
                    ShareControlOut rdp_control_out(stream, PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
                    ShareDataOut rdp_data_out(stream, PDUTYPE2_REFRESH_RECT, this->share_id, RDP::STREAM_MED);

                    stream.out_uint32_le(1);
                    stream.out_uint16_le(r.x);
                    stream.out_uint16_le(r.y);
                    TODO(" check this -1 (difference between rect and clip)")
                    stream.out_uint16_le(r.cx - 1);
                    stream.out_uint16_le(r.cy - 1);

                    rdp_data_out.end();
                    rdp_control_out.end();
                    sec_out.end();
                    sdrq_out.end();
                    tpdu.end();
                    tpdu.send(this->trans);
                }
            }
        }

    void process_color_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
    {
//        LOG(LOG_INFO, "/* process_color_pointer_pdu */\n");
        unsigned cache_idx;
        unsigned dlen;
        unsigned mlen;
        struct rdp_cursor* cursor;

        cache_idx = stream.in_uint16_le();
        if (cache_idx >= (sizeof(this->cursors) / sizeof(cursor))) {
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_CACHE_NOT_OK);
        }
        cursor = this->cursors + cache_idx;
        cursor->x = stream.in_uint16_le();
        cursor->y = stream.in_uint16_le();
        cursor->width = stream.in_uint16_le();
        cursor->height = stream.in_uint16_le();
        mlen = stream.in_uint16_le(); /* mask length */
        dlen = stream.in_uint16_le(); /* data length */
        if ((mlen > sizeof(cursor->mask)) || (dlen > sizeof(cursor->data))) {
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }
        memcpy( cursor->data, stream.in_uint8p( dlen),  dlen);
        memcpy( cursor->mask, stream.in_uint8p( mlen),  mlen);
        mod->gd.server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
    }

    void process_cached_pointer_pdu(Stream & stream, client_mod * mod)
    {
//        LOG(LOG_INFO, "/* process_cached_pointer_pdu */\n");

        int cache_idx = stream.in_uint16_le();
        if (cache_idx < 0){
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_LESS_0);
        }
        if (cache_idx >= (int)(sizeof(this->cursors) / sizeof(rdp_cursor))) {
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        struct rdp_cursor* cursor = this->cursors + cache_idx;
        mod->gd.server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
    }

    void process_system_pointer_pdu(Stream & stream, client_mod * mod)
    {
//        LOG(LOG_INFO, "/* process_system_pointer_pdu */\n");
        int system_pointer_type = stream.in_uint16_le();
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                struct rdp_cursor cursor;
                memset(cursor.mask, 0xff, sizeof(cursor.mask));
                TODO(" we should pass in a cursor to set_pointer instead of individual fields")
                mod->gd.server_set_pointer(cursor.x, cursor.y, cursor.data, cursor.mask);
                mod->set_pointer_display();
            }
            break;
        default:
            break;
        }
    }

    void process_bitmap_updates(Stream & stream, client_mod * mod)
    {
        mod->gd.server_begin_update();
//        LOG(LOG_INFO, "/* process_bitmap_updates */\n");
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
//        LOG(LOG_INFO, "/* ---------------- Sending %d rectangles ----------------- */\n", numberRectangles);
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
            uint8_t bpp = stream.in_uint16_le();

//            LOG(LOG_ERR, "left=%u top=%u right=%u bottom=%u width=%u height=%u bpp=%u", left, top, right, bottom, width, height, bpp);

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
            uint16_t bufsize = stream.in_uint16_le();

            Rect boundary(left, top, right - left + 1, bottom - top + 1);

            // BITMAP_COMPRESSION 0x0001
            // Indicates that the bitmap data is compressed. This implies
            // that the bitmapComprHdr field is present if the
            // NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not set.


//            LOG(LOG_INFO, "/* Rect [%d] bpp=%d width=%d height=%d b(%d, %d, %d, %d) */", i, bpp, width, height, boundary.x, boundary.y, boundary.cx, boundary.cy);

            if (flags & 0x0001){
                uint16_t size = bufsize;
                uint16_t line_size = row_size(width, bpp);
                uint16_t final_size = line_size * height;
                if (!(flags & 0x400)) {
                // bitmapComprHdr (8 bytes): Optional Compressed Data Header
                // structure (see Compressed Data Header (TS_CD_HEADER)
                // (section 2.2.9.1.1.3.1.2.3)) specifying the bitmap data
                // in the bitmapDataStream. This field MUST be present if
                // the BITMAP_COMPRESSION (0x0001) flag is present in the
                // Flags field, but the NO_BITMAP_COMPRESSION_HDR (0x0400)
                // flag is not.
                    // bitmapComprHdr
                    stream.skip_uint8(2); /* pad */
                    size = stream.in_uint16_le();
                    line_size = stream.in_uint16_le();
                    final_size = stream.in_uint16_le();
                }

                const uint8_t * data = stream.in_uint8p(size);
                if (width <= 0 || height <= 0){
                    LOG(LOG_ERR, "unexpected bitmap size : width=%u height=%u size=%u left=%u, top=%u, right=%u, bottom=%u", width, height, size, left, top, right, bottom);
                }

//                Bitmap bitmap(bpp, &this->orders.cache_colormap[0], width, height, data, size, true);
                Bitmap bitmap(bpp, &this->orders.global_palette, width, height, data, size, true);

                if (line_size != bitmap.line_size(bpp)){
                    LOG(LOG_WARNING, "Unexpected line_size in bitmap received [%u != %u] width=%u height=%u bpp=%u", 
                        line_size, bitmap.line_size(bpp), width, height, bpp);
                }
                if (line_size != bitmap.line_size(bpp)){
                    LOG(LOG_WARNING, "Unexpected final_size in bitmap received [%u != %u] width=%u height=%u bpp=%u", 
                        final_size, bitmap.bmp_size(bpp), width, height, bpp);
                }

                mod->gd.bitmap_update(bitmap, boundary, 0, 0, boundary);
            }
            else {
                const uint8_t * data = stream.in_uint8p(bufsize);
//                Bitmap bitmap(bpp, &this->orders.cache_colormap[0], width, height, data, bufsize);
                Bitmap bitmap(bpp, &this->orders.global_palette, width, height, data, bufsize);

                if (bufsize != bitmap.bmp_size(bpp)){
                    LOG(LOG_WARNING, "Unexpected bufsize in bitmap received [%u != %u] width=%u height=%u bpp=%u", 
                        bufsize, bitmap.bmp_size(bpp), width, height, bpp);
                }

                mod->gd.bitmap_update(bitmap, boundary, 0, 0, boundary);
            }
        }
        mod->gd.server_end_update();
    }

    void out_bmpcache2_caps(Stream & stream, const ClientInfo & client_info)
    {
        stream.out_uint16_le(RDP_CAPSET_BMPCACHE2);
        stream.out_uint16_le(RDP_CAPLEN_BMPCACHE2);

        /* version */
        stream.out_uint16_le(client_info.bitmap_cache_persist_enable ? 2 : 0);
        stream.out_uint16_be(3);	/* number of caches in this set */

        /* Sending bitmap capabilities version 2 */
        TODO(" no need any more to set a limit at 2000  use real figures")
        stream.out_uint32_le(std::min(client_info.cache1_entries, (uint32_t)2000));
        stream.out_uint32_le(std::min(client_info.cache2_entries, (uint32_t)2000));
        stream.out_uint32_le(std::min(client_info.cache3_entries, (uint32_t)2000));
 
        stream.out_clear_bytes(20);	/* other bitmap caches not used */
    }


    void send_client_info_pdu(Transport * trans, int userid, const char * password, int rdp5_performanceflags, int & use_rdp5)
    {

        int flags = RDP_LOGON_NORMAL | ((strlen(password) > 0)?RDP_LOGON_AUTO:0);

//            LOG(LOG_INFO, "send login info to server\n");
        time_t t = time(NULL);
        time_t tzone;

        // The WAB does not send it's IP to server. Is it what we want ?
        const char * ip_source = "\0\0\0\0";

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out2(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);
        SecOut sec_out(stream, 2, SEC_LOGON_INFO | SEC_ENCRYPT, this->encrypt);

        if(!use_rdp5){
            LOG(LOG_INFO, "send login info (RDP4-style) %s:%s\n",this->domain, this->username);

            stream.out_uint32_le(0);
            stream.out_uint32_le(flags);
            stream.out_uint16_le(2 * strlen(this->domain));
            stream.out_uint16_le(2 * strlen(this->username));
            stream.out_uint16_le(2 * strlen(password));
            stream.out_uint16_le(2 * strlen(this->program));
            stream.out_uint16_le(2 * strlen(this->directory));
            stream.out_unistr(this->domain);
            stream.out_unistr(this->username);
            stream.out_unistr(password);
            stream.out_unistr(this->program);
            stream.out_unistr(this->directory);
        }
        else {
            LOG(LOG_INFO, "send login info (RDP5-style) %x %s:%s\n",flags,
                this->domain,
                this->username);

            flags |= RDP_LOGON_BLOB;
            stream.out_uint32_le(0);
            stream.out_uint32_le(flags);
            stream.out_uint16_le(2 * strlen(this->domain));
            stream.out_uint16_le(2 * strlen(this->username));
            if (flags & RDP_LOGON_AUTO){
                stream.out_uint16_le(2 * strlen(password));
            }
            if (flags & RDP_LOGON_BLOB && ! (flags & RDP_LOGON_AUTO)){
                stream.out_uint16_le(0);
            }
            stream.out_uint16_le(2 * strlen(this->program));
            stream.out_uint16_le(2 * strlen(this->directory));
            if ( 0 < (2 * strlen(this->domain))){
                stream.out_unistr(this->domain);
            }
            else {
                stream.out_uint16_le(0);
            }
            stream.out_unistr(this->username);
            if (flags & RDP_LOGON_AUTO){
                stream.out_unistr(password);
            }
            else{
                stream.out_uint16_le(0);
            }
            if (0 < 2 * strlen(this->program)){
                stream.out_unistr(this->program);
            }
            else {
                stream.out_uint16_le(0);
            }
            if (2 * strlen(this->directory) < 0){
                stream.out_unistr(this->directory);
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
            use_rdp5 = 0;
        }

        sec_out.end();
        sdrq_out2.end();
        tpdu.end();
        tpdu.send(trans);

        LOG(LOG_INFO, "send login info ok\n");
    }

};

#endif
