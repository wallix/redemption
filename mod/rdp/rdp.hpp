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

#if !defined(__MOD_RDP_HPP__)
#define __MOD_RDP_HPP__

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

#include "RDP/x224.hpp"
#include "RDP/nego.hpp"
#include "RDP/mcs.hpp"
#include "RDP/lic.hpp"
#include "RDP/logon.hpp"
#include "channel_list.hpp"
#include "RDP/gcc.hpp"
#include "RDP/sec.hpp"
#include "colors.hpp"
#include "RDP/capabilities.hpp"
#include "RDP/connection.hpp"

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


#include "genrandom.hpp"

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

    TODO(" this cache_bitmap here looks strange. At least it's size should be negotiated. And why is it not managed by the other cache management code ? This probably hide some kind of problem. See when working on cache secondary order primitives.")
    const Bitmap * cache_bitmap[3][10000];

    uint32_t verbose;

    rdp_orders(uint32_t verbose) :
        common(RDP::PATBLT, Rect(0, 0, 1, 1)),
        memblt(0, Rect(), 0, 0, 0, 0),
        opaquerect(Rect(), 0),
        scrblt(Rect(), 0, 0, 0),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyph_index(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        verbose(verbose)
    {
        memset(this->cache_bitmap, 0, sizeof(this->cache_bitmap));
        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
        memset(this->memblt_palette, 0, sizeof(this->memblt_palette));
    }

    void reset()
    {
        this->common = RDPOrderCommon(RDP::PATBLT, Rect(0, 0, 1, 1));
        this->memblt = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        this->opaquerect = RDPOpaqueRect(Rect(), 0);
        this->scrblt = RDPScrBlt(Rect(), 0, 0, 0);
        this->destblt = RDPDestBlt(Rect(), 0);
        this->patblt = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        this->lineto = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        this->glyph_index = RDPGlyphIndex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"");
        memset(this->cache_bitmap, 0, sizeof(this->cache_bitmap));
        memset(this->cache_colormap, 0, sizeof(this->cache_colormap));
        memset(this->global_palette, 0, sizeof(this->global_palette));
        memset(this->memblt_palette, 0, sizeof(this->memblt_palette));
    }

    ~rdp_orders(){
    }

    void rdp_orders_process_bmpcache(uint8_t bpp, Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "rdp_orders_process_bmpcache bpp=%u", bpp);
        }
        RDPBmpCache bmp;
        bmp.receive(stream, control, header, this->global_palette);

        TODO("add cache_id, cache_idx range check, and also size check based on cache size by type and uncompressed bitmap size")
        if (this->cache_bitmap[bmp.id][bmp.idx]) {
            delete this->cache_bitmap[bmp.id][bmp.idx];
        }
        this->cache_bitmap[bmp.id][bmp.idx] = bmp.bmp;
        if (this->verbose & 64){
            LOG(LOG_ERR, "rdp_orders_process_bmpcache bitmap id=%u idx=%u cx=%u cy=%u bmp_size=%u original_bpp=%u bpp=%u", bmp.id, bmp.idx, bmp.bmp->cx, bmp.bmp->cy, bmp.bmp->bmp_size, bmp.bmp->original_bpp, bpp);
        }
    }

    void rdp_orders_process_fontcache(Stream & stream, int flags, client_mod * mod)
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "rdp_orders_process_fontcache");
        }
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
        if (this->verbose & 64){
            LOG(LOG_INFO, "rdp_orders_process_fontcache done");
        }
    }


    void process_colormap(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header, client_mod * mod)
    {
        if (this->verbose & 64){
            LOG(LOG_INFO, "process_colormap");
        }
        RDPColCache colormap;
        colormap.receive(stream, control, header);
        memcpy(this->cache_colormap[colormap.cacheIndex], &colormap.palette, sizeof(BGRPalette));
        mod->front.color_cache(colormap.palette, colormap.cacheIndex);
        if (this->verbose & 64){
            LOG(LOG_INFO, "process_colormap done");
        }
    }

    /*****************************************************************************/
    int process_orders(uint8_t bpp, Stream & stream, int num_orders, client_mod * mod)
    {
//        if (this->verbose & 64){
//            LOG(LOG_INFO, "process_orders bpp=%u", bpp);
//        }
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
//                LOG(LOG_INFO, "secondary order=%d", header.type);
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
                const Rect & cmd_clip = ((control & BOUNDS)
                                      ? this->common.clip
                                    : Rect(0, 0, mod->front_width, mod->front_height));
//                LOG(LOG_INFO, "/* order=%d ordername=%s */", this->common.order, ordernames[this->common.order]);
                switch (this->common.order) {
                case GLYPHINDEX:
                    this->glyph_index.receive(stream, header);
                    mod->front.draw(this->glyph_index, cmd_clip);
                    break;
                case DESTBLT:
                    this->destblt.receive(stream, header);
                    mod->front.draw(this->destblt, cmd_clip);
                    break;
                case PATBLT:
                    this->patblt.receive(stream, header);
                    mod->front.draw(this->patblt, cmd_clip);
                    break;
                case SCREENBLT:
                    this->scrblt.receive(stream, header);
                    mod->front.draw(this->scrblt, cmd_clip);
                    break;
                case LINE:
                    this->lineto.receive(stream, header);
                    mod->front.draw(this->lineto, cmd_clip);
                    break;
                case RECT:
                    this->opaquerect.receive(stream, header);
                    mod->front.draw(this->opaquerect, cmd_clip);
                    break;
                case MEMBLT:
                    this->memblt.receive(stream, header);
                    {
                        if ((this->memblt.cache_id >> 8) >= 6){
                            LOG(LOG_INFO, "colormap out of range in memblt:%x", (this->memblt.cache_id >> 8));
                            this->memblt.log(LOG_INFO, cmd_clip);
                            assert(false);
                        }
                        const Bitmap* bitmap = this->cache_bitmap[this->memblt.cache_id & 0x3][this->memblt.cache_idx];
                        TODO("check if bitmap has the right palette...")
                        TODO("8 bits palettes should probabily be transmitted to front, not stored in bitmaps")
                        if (bitmap) {
                            mod->front.draw(this->memblt, cmd_clip, *bitmap);
                        }
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->common.order);
                    break;
                }
            }
            processed++;
        }
//        if (this->verbose & 64){
//            LOG(LOG_INFO, "process_orders done");
//        }
        return 0;
    }
};


struct mod_rdp : public client_mod {

    /* mod data */
    BStream in_stream;
    ChannelDefArray mod_channel_list;

    bool dev_redirection_enable;
    int use_rdp5;
    int keylayout;
    struct RdpLicence lic_layer;

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

    RdpNego nego;

    mod_rdp(Transport * trans,
            const char * target_user,
            const char * target_password,
            struct FrontAPI & front,
            const char * hostname,
            const bool tls,
            const ClientInfo & info,
            Random * gen,
            uint32_t verbose = 0)
            :
                client_mod(front, info.width, info.height),
                    in_stream(65536),
                    use_rdp5(1),
                    keylayout(info.keylayout),
                    lic_layer(hostname),
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
                    nego(tls, trans, target_user)
    {
        LOG(LOG_INFO, "Creation of new mod 'RDP'");
        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        this->decrypt.encryptionMethod = 2; /* 128 bits */
        this->encrypt.encryptionMethod = 2; /* 128 bits */
        this->decrypt.rc4_key_len = 16; /* 16 = 128 bit */
        this->encrypt.rc4_key_len = 16; /* 16 = 128 bit */

        TODO(" and if hostname is really larger  what happens ? We should at least emit a warning log")
        strncpy(this->hostname, hostname, 15);
        this->hostname[15] = 0;
        TODO(" and if username is really larger  what happens ? We should at least emit a warning log")
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
            TODO(" is decoding and reencoding really necessary  a simple pass-through from front to back-end should be enough")
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
                uint8_t * data,
                size_t length,
                size_t chunk_size,
                uint32_t flags)
    {
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::send_to_mod_channel");
        }
        const ChannelDef * mod_channel = this->mod_channel_list.get(front_channel_name);
        // send it if module has a matching channel, if no matching channel is found just forget it
        if (mod_channel){
            this->send_to_channel(*mod_channel, data, length, chunk_size, flags);
        }
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::send_to_mod_channel done");
        }
    }

    void send_to_channel(
                const ChannelDef & channel,
                uint8_t * data,
                size_t length,
                size_t chunk_size,
                uint32_t flags)
    {
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::send_to_channel");
        }

        BStream stream(65536);

        stream.out_uint32_le(length);
        stream.out_uint32_le(flags);
        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= ChannelDef::CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_copy_bytes(data, chunk_size);
        stream.mark_end();    

        BStream x224_header(256);
        BStream mcs_header(256);
        BStream sec_header(256);
        SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channel.chanid, 1, 3, 
                                      sec_header.size() + stream.size() , MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

        this->nego.trans->send(x224_header.data, x224_header.size());
        this->nego.trans->send(mcs_header.data, mcs_header.size());
        this->nego.trans->send(sec_header.data, sec_header.size());
        this->nego.trans->send(stream.data, stream.size());


        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::send_to_channel done");
        }
    }

    void send_data_request(uint16_t channelId, Stream & stream)
    {
        BStream x224_header(256);
        BStream mcs_header(256);

        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, channelId, 1, 3, stream.size(), MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans->send(x224_header.data, x224_header.size());
        this->nego.trans->send(mcs_header.data, mcs_header.size());
        this->nego.trans->send(stream.data, stream.size());
    }

    virtual BackEvent_t draw_event(void)
    {
        static uint32_t count = 0;
        LOG(LOG_INFO, "============================== mod_rdp::DRAW_EVENT %u =================================", count++);

        try{

        char * hostname = this->hostname;

        switch (this->state){
        case MOD_RDP_NEGO:
            LOG(LOG_INFO, "draw_event::MOD_RDP_NEGO");
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
                    TODO("values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?")

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

                    this->nego.trans->send(x224_header.data, x224_header.size());
                    this->nego.trans->send(mcs_header.data, mcs_header.size());
                    this->nego.trans->send(gcc_header.data, gcc_header.size());
                    this->nego.trans->send(stream.data, stream.size());

                    this->state = MOD_RDP_BASIC_SETTINGS_EXCHANGE;
                }
                break;
            }
        break;

        case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
            LOG(LOG_INFO, "draw_event::MOD_RDP_BASIC_SETTINGS_EXCHANGE");
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::Basic Settings Exchange");
            }
            {
                BStream x224_data(65536);
                X224::RecvFactory f(*this->nego.trans, x224_data);
                X224::DT_TPDU_Recv x224(*this->nego.trans, x224_data);

                SubStream & mcs_data = x224.payload;
                MCS::CONNECT_RESPONSE_PDU_Recv mcs(mcs_data, MCS::BER_ENCODING);

                GCC::Create_Response_Recv gcc_cr(mcs.payload);

                while (gcc_cr.payload.check_rem(4)) {

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
                        LOG(LOG_INFO, "Receiving SC_Security from server");

                        GCC::UserData::SCSecurity sc_sec1;
                        sc_sec1.recv(f.payload);

                        this->encryptionLevel = sc_sec1.encryptionLevel;
                        this->encryptionMethod = sc_sec1.encryptionMethod; 
                        if (sc_sec1.encryptionLevel == 0 
                        &&  sc_sec1.encryptionMethod == 0) { /* no encryption */
                            LOG(LOG_INFO, "No encryption");
                        }
                        else {
                            ssllib ssl;

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
                            }
                            else {

                                // ---------------------------------------------------------------------
                                LOG(LOG_DEBUG, "We're going for the RDP5-style encryption");

                                // structure of certificate chain is : certcount [certlen cert]*

                                uint32_t certcount = sc_sec1.x509.certCount;
                                LOG(LOG_DEBUG, "Certcount = %u", certcount);

                                if (certcount < 2){
                                    LOG(LOG_DEBUG, "Server didn't send enough X509 certificates");
                                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK);
                                }

//                                uint32_t cacert_len = sc_sec1.x509.cert[certcount - 2].len;
//                                LOG(LOG_DEBUG, "CA Certificate length is %d", cacert_len);
//                                X509 *cacert =  sc_sec1.x509.cert[certcount - 2].cert;

                                uint32_t cert_len = sc_sec1.x509.cert[certcount - 1].len;
                                LOG(LOG_DEBUG, "CA Certificate length is %d", cert_len);
                                X509 *cert =  sc_sec1.x509.cert[certcount - 1].cert;

                                /* Matching certificates */
                                TODO("Currently, we don't use the CA Certificate, we should"
                                     "*) Verify the server certificate (server_cert) with the CA certificate."
                                     "*) Store the CA Certificate with the hostname of the server we are connecting"
                                     " to as key, and compare it when we connect the next time, in order to prevent"
                                     " MITM-attacks.")
                                // ---------------------------------------------------------------------

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
                                    printf("Failed to extract public key from certificate\n");
                                    throw Error(ERR_GCC);
                                }

                                RSA * server_public_key = RSAPublicKey_dup((RSA *) epk->pkey.ptr);
                                EVP_PKEY_free(epk);
                                this->server_public_key_len = RSA_size(server_public_key);

                                LOG(LOG_DEBUG, "Server public key length=%u", (unsigned)this->server_public_key_len);

                                if (NULL == server_public_key){
                                    LOG(LOG_DEBUG, "Didn't parse X509 correctly");
                                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);
                                }

                                LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d",
                                    this->server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);

                                if ((this->server_public_key_len < SEC_MODULUS_SIZE) ||
                                    (this->server_public_key_len > SEC_MAX_MODULUS_SIZE)){
                                    LOG(LOG_WARNING, "Bad server public key size (%u bits)", this->server_public_key_len * 8);
                                    RSA_free(server_public_key);
                                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
                                }

                                if ((BN_num_bytes(server_public_key->e) > SEC_EXPONENT_SIZE) 
                                ||  (BN_num_bytes(server_public_key->n) > SEC_MAX_MODULUS_SIZE)){
                                    LOG(LOG_WARNING, "Problem extracting RSA exponent, modulus");
                                    RSA_free(server_public_key);
                                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK);
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
                        for (size_t index = 0; index < sc_net.channelCount; index++){
                            this->mod_channel_list.push_back(this->front.get_channel_list()[index]);
                        }

//                        for (uint32_t index = 0; index < sc_net.channelCount; index++) {
//                            ChannelDef def = this->front.get_channel_list()[index];
//                            def.chanid = sc_net.channelDefArray[index].id;
//                            this->mod_channel_list.push_back(def);
//                        }
                        sc_net.log("Received from server");
                    }
                    break;
                    default:
                        LOG(LOG_WARNING, "unsupported response tag 0x%x", f.tag);
                        break;
                    }
                }
                if (gcc_cr.payload.check_rem(1)) {
                    LOG(LOG_ERR, "recv connect response parsing gcc data : short header");
                    throw Error(ERR_MCS_DATA_SHORT_HEADER);
                }

            }

            if (this->verbose){
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

            LOG(LOG_INFO, "Send MCS::ErectDomainRequest");
            {
                BStream x224_header(256);
                BStream mcs_data(256);

                MCS::ErectDomainRequest_Send mcs(mcs_data, 0, 0, MCS::PER_ENCODING);
                X224::DT_TPDU_Send(x224_header, mcs_data.size());
                this->nego.trans->send(x224_header.data, x224_header.size());
                this->nego.trans->send(mcs_data.data, mcs_data.size());
            }
            LOG(LOG_INFO, "Send MCS::AttachUserRequest");
            {
                BStream x224_header(256);
                BStream mcs_data(256);

                MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);
                size_t mcs_length = mcs_data.size();
                X224::DT_TPDU_Send(x224_header, mcs_length);
                size_t x224_length = x224_header.size();

                this->nego.trans->send(x224_header.data, x224_length);
                this->nego.trans->send(mcs_data.data, mcs_length);
            }
            this->state = MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER;
        break;

        case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
        LOG(LOG_INFO, "draw_event::MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER");
        if (this->verbose){
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
                TODO(" the array size below is arbitrary  it should be checked to avoid buffer overflow")

                size_t num_channels = this->mod_channel_list.size();
                uint16_t channels_id[100];
                channels_id[0] = this->userid + MCS_USERCHANNEL_BASE;
                channels_id[1] = MCS_GLOBAL_CHANNEL;
                for (size_t index = 0; index < num_channels; index++){
                    channels_id[index+2] = this->mod_channel_list[index].chanid;
                }

                for (size_t index = 0; index < num_channels+2; index++){
                    BStream x224_header(256);
                    BStream mcs_cjrq_data(256);
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
                    TODO("We should check channel confirmation worked, for now we just do like server said OK to everything... and that may not be the case, some channels may be closed for instance. We should also check requested chanid are some confirm may come out of order"); 
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
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::RDP Security Commencement");
            }

            if (this->encryptionLevel){
                BStream stream(this->server_public_key_len + 32);
                SEC::SecExchangePacket_Send mcs(stream, client_crypt_random, this->server_public_key_len);
                this->send_data_request(MCS_GLOBAL_CHANNEL, stream);
            }

            // Secure Settings Exchange
            // ------------------------

            // Secure Settings Exchange: Secure client data (such as the username,
            // password and auto-reconnect cookie) is sent to the server using the Client
            // Info PDU.

            // Client                                                     Server
            //    |------ Client Info PDU      ---------------------------> |

            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::Secure Settings Exchange");
            }

            this->send_client_info_pdu(this->userid, this->password);

            this->state = MOD_RDP_GET_LICENSE;
        }
        break;

        case MOD_RDP_GET_LICENSE:
        LOG(LOG_INFO, "draw_event::MOD_RDP_GET_LICENSE");
        if (this->verbose){
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
            const char * hostname = this->hostname;
            const char * username = this->username;
            int & license_issued = this->lic_layer.license_issued;
            int res = 0;
            // read tpktHeader (4 bytes = 3 0 len)
            // TPDU class 0    (3 bytes = LI F0 PDU_DT)

            BStream stream(65536);
            X224::RecvFactory f(*this->nego.trans, stream);
            X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
            SubStream & mcs_data = x224.payload;
            MCS::SendDataIndication_Recv mcs(mcs_data, MCS::PER_ENCODING);
            SubStream & payload = mcs.payload;

            SEC::Sec_Recv sec(mcs.payload, true, this->decrypt, this->encryptionLevel, this->encryptionMethod);

            if (sec.flags & SEC::SEC_LICENSE_PKT) {

                // 2.2.1.12.1 Valid Client License Data (LICENSE_VALID_CLIENT_DATA)
                // ================================================================


                // validClientLicenseData (variable): The actual contents of the
                // License Error (Valid Client) PDU, as specified in section 2.2.1.12.1.


                // preamble (4 bytes): Licensing Preamble (section 2.2.1.12.1.1) structure containing header
                // information. The bMsgType field of the preamble structure MUST be set to ERROR_ALERT (0xFF).


                // 2.2.1.12.1.1 Licensing Preamble (LICENSE_PREAMBLE)
                // --------------------------------------------------

                // Note: Some of the information in this section is subject to
                // change because it applies to a preliminary implementation of the
                // protocol or structure. For information about specific differences
                // between versions, see the behavior notes that are provided in the
                // Product Behavior appendix.

                // The LICENSE_PREAMBLE structure precedes every licensing packet
                // sent on the wire.

                // bMsgType (1 byte): An 8-bit, unsigned integer. A type of the
                // licensing packet. For more details about the different licensing
                // packets, see [MS-RDPELE] section 2.2.2.

                // Sent by server:
                // 0x01 LICENSE_REQUEST Indicates a License Request PDU ([MS-RDPELE] section 2.2.2.1).
                // 0x02 PLATFORM_CHALLENGE Indicates a Platform Challenge PDU ([MS-RDPELE] section 2.2.2.4).
                // 0x03 NEW_LICENSE Indicates a New License PDU ([MS-RDPELE] section 2.2.2.7).
                // 0x04 UPGRADE_LICENSE Indicates an Upgrade License PDU ([MS-RDPELE] section 2.2.2.6).

                // Sent by client:
                // 0x12 LICENSE_INFO Indicates a License Information PDU ([MS-RDPELE] section 2.2.2.3).
                // 0x13 NEW_LICENSE_REQUEST Indicates a New License Request PDU ([MS-RDPELE] section 2.2.2.2).
                // 0x15 PLATFORM_CHALLENGE_RESPONSE Indicates a Platform Challenge Response PDU ([MS-RDPELE] section 2.2.2.5).

                // Sent by either client or server:
                // 0xFF ERROR_ALERT Indicates a Licensing Error Message PDU (section 2.2.1.12.1.3).

                // flags (1 byte): An 8-bit unsigned integer. License preamble flags.

                // +-----------------------------------+------------------------------------------------------+
                // | 0x0F LicenseProtocolVersionMask   | The license protocol version. See the discussion     |
                // |                                   | which follows this table for more information.       |
                // +-----------------------------------+------------------------------------------------------+
                // | 0x80 EXTENDED_ERROR_MSG_SUPPORTED | Indicates that extended error information using the  |
                // |                                   | License Error Message (section 2.2.1.12.1.3) is      |
                // |                                   | supported.                                           |
                // +-----------------------------------+------------------------------------------------------+

                // The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version. The following are possible version values.
                // +--------------------------+------------------------------------------------+
                // | 0x2 PREAMBLE_VERSION_2_0 | RDP 4.0                                        |
                // +--------------------------+------------------------------------------------+
                // | 0x3 PREAMBLE_VERSION_3_0 | RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1, and 8.0 |
                // +--------------------------+------------------------------------------------+


                // wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in
                // bytes of the licensing packet (including the size of the preamble).
                // --------------------------------------------------

                // validClientMessage (variable): A Licensing Error Message (section
                // 2.2.1.12.1.3) structure. The dwErrorCode field of the error message
                // structure MUST be set to STATUS_VALID_CLIENT (0x00000007) and the
                // dwStateTransition field MUST be set to ST_NO_TRANSITION (0x00000002).
                // The bbErrorInfo field MUST contain an empty binary large object
                // (BLOB) of type BB_ERROR_BLOB (0x0004).


                // 2.2.1.12.1.3 Licensing Error Message (LICENSE_ERROR_MESSAGE)
                // ============================================================

                // The LICENSE_ERROR_MESSAGE structure is used to indicate that an
                //  error occurred during the licensing protocol. Alternatively,
                // it is also used to notify the peer of important status information.

                // dwErrorCode (4 bytes): A 32-bit, unsigned integer. The error or
                // status code.

                // Sent by client:
                // ERR_INVALID_SERVER_CERTIFICATE 0x00000001
                // ERR_NO_LICENSE 0x00000002

                // Sent by server
                // ERR_INVALID_SCOPE 0x00000004
                // ERR_NO_LICENSE_SERVER 0x00000006
                // STATUS_VALID_CLIENT 0x00000007
                // ERR_INVALID_CLIENT 0x00000008
                // ERR_INVALID_PRODUCTID 0x0000000B
                // ERR_INVALID_MESSAGE_LEN 0x0000000C

                // Sent by client and server:
                // ERR_INVALID_MAC 0x00000003

                // dwStateTransition (4 bytes): A 32-bit, unsigned integer. The
                // licensing state to transition into upon receipt of this message.
                // For more details about how this field is used, see [MS-RDPELE]
                // section 3.1.5.2.

                // ST_TOTAL_ABORT 0x00000001
                // ST_NO_TRANSITION 0x00000002
                // ST_RESEND_LAST_MESSAGE 0x00000003
                // ST_RESET_PHASE_TO_START 0x00000004

                // bbErrorInfo (variable): A LICENSE_BINARY_BLOB (section
                // 2.2.1.12.1.2) structure which MUST contain a BLOB of type
                // BB_ERROR_BLOB (0x0004) that includes information relevant to
                // the error code specified in dwErrorCode.

                SubStream & payload = sec.payload;
                uint8_t tag = payload.in_uint8();
                uint8_t version = payload.in_uint8();
                uint16_t length = payload.in_uint16_le();
                switch (tag) {
                case LIC::LICENSE_REQUEST:
                    if (this->verbose){
                        LOG(LOG_INFO, "Rdp:: License Request");
                    }
                    {
                        /* Retrieve the server random from the incoming packet */
                        const uint8_t * server_random = payload.in_uint8p(SEC_RANDOM_SIZE);

                        uint8_t null_data[SEC_MODULUS_SIZE];

                        /* We currently use null client keys. This is a bit naughty but, hey,
                           the security of licence negotiation isn't exactly paramount. */
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
                            sha1.update(pad, i + 1);
                            sha1.update(pre_master_secret, 48);
                            sha1.update(client_random, 32);
                            sha1.update(server_random, 32);
                            sha1.final(shasig);

                            SslMd5 md5;
                            md5.update(pre_master_secret, 48);
                            md5.update(shasig, 20);
                            md5.final(&master_secret[i * 16]);
                        }

                        for (int i = 0; i < 3; i++) {
                            uint8_t shasig[20];
                            uint8_t pad[4];
                            memset(pad, 'A' + i, i + 1);

                            SslSha1 sha1;
                            sha1.update(pad, i + 1);
                            sha1.update(master_secret, 48);
                            sha1.update(server_random, 32);
                            sha1.update(client_random, 32);
                            sha1.final(shasig);

                            SslMd5 md5;
                            md5.update(master_secret, 48);
                            md5.update(shasig, 20);
                            md5.final(&key_block[i * 16]);
                        }

                        /* Store first 16 bytes of session key as MAC secret */
                        memcpy(this->lic_layer.license_sign_key, key_block, 16);

                        // Generate RC4 key from next 16 bytes
                        // 16-byte transformation used to generate export keys (6.2.2).
                        SslMd5 md5;
                        md5.update(key_block + 16, 16);
                        md5.update(client_random, 32);
                        md5.update(server_random, 32);
                        md5.final(this->lic_layer.license_key);

                        BStream stream(65535);

                        if (this->lic_layer.license_size > 0) {
                            uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                            buf_out_uint32(hwid, 2);
                            memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                            ssllib ssl;
                            /* Generate a signature for the HWID buffer */
                            uint8_t signature[LIC::LICENSE_SIGNATURE_SIZE];
                            ssl.sign(signature, 16, this->lic_layer.license_sign_key, 16, hwid, sizeof(hwid));
                            /* Now encrypt the HWID */

                            RC4_KEY crypt_key;
                            ssl.rc4_set_key(crypt_key, this->lic_layer.license_key, 16);
                            ssl.rc4_crypt(crypt_key, hwid, hwid, sizeof(hwid));

                            uint8_t null_data[SEC_MODULUS_SIZE];
                            memset(null_data, 0, sizeof(null_data));

                            int length = 16 + SEC_RANDOM_SIZE + SEC_MODULUS_SIZE + SEC_PADDING_SIZE +
                                     this->lic_layer.license_size + LIC::LICENSE_HWID_SIZE + LIC::LICENSE_SIGNATURE_SIZE;

                            stream.out_uint8(LIC::LICENSE_INFO);
                            stream.out_uint8(this->use_rdp5?3:2); /* version */
                            stream.out_uint16_le(length);
                            stream.out_uint32_le(1);
                            stream.out_uint16_le(0);
                            stream.out_uint16_le(0x0201);
                            stream.out_copy_bytes(null_data, SEC_RANDOM_SIZE); // client_random
                            stream.out_uint16_le(0);
                            stream.out_uint16_le(SEC_MODULUS_SIZE + SEC_PADDING_SIZE);
                            stream.out_copy_bytes(null_data, SEC_MODULUS_SIZE); // rsa_data
                            stream.out_clear_bytes( SEC_PADDING_SIZE);
                            stream.out_uint16_le(1);
                            stream.out_uint16_le(this->lic_layer.license_size);
                            stream.out_copy_bytes(this->lic_layer.license_data, this->lic_layer.license_size);
                            stream.out_uint16_le(1);
                            stream.out_uint16_le(LIC::LICENSE_HWID_SIZE);
                            stream.out_copy_bytes(hwid, LIC::LICENSE_HWID_SIZE);
                            stream.out_copy_bytes(signature, LIC::LICENSE_SIGNATURE_SIZE);
                            stream.mark_end();
                        }
                        else {
                            uint8_t null_data[SEC_MODULUS_SIZE];
                            memset(null_data, 0, sizeof(null_data));

                            LOG(LOG_INFO, "rdp_lic_send_request");

                            int userlen = strlen(username) + 1;
                            int hostlen = strlen(hostname) + 1;
                            int length = 128 + userlen + hostlen;

                            stream.out_uint8(LIC::NEW_LICENSE_REQUEST);
                            stream.out_uint8(this->use_rdp5?3:2);
                            stream.out_uint16_le(length);

                            // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
                            // indicates the key exchange algorithm chosen by the client. It MUST be set
                            // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
                            // exchange with a 512-bit asymmetric key.<9>

                            stream.out_uint32_le(KEY_EXCHANGE_ALG_RSA);

                            // PlatformId (4 bytes): A 32-bit unsigned integer. This field is composed
                            // of two identifiers: the operating system identifier and the independent
                            // software vendor (ISV) identifier. The platform ID is composed of the
                            // logical OR of these two values.

                            // The most significant byte of the PlatformId field contains the operating
                            // system version of the client.<10>

                            // The second most significant byte of the PlatformId field identifies the
                            // ISV that provided the client image.<11>

                            // The remaining two bytes in the PlatformId field are used by the ISV to
                            // identify the build number of the operating system.<12>

                            stream.out_uint32_le(0);

                            // ClientRandom (32 bytes): A 32-byte random number generated by the client
                            // using a cryptographically secure pseudo-random number generator. The
                            // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
                            // the data in the EncryptedPreMasterSecret field, are used to generate
                            // licensing encryption keys (see section 5.1.3). These keys are used to
                            // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

                            stream.out_copy_bytes(null_data, SEC_RANDOM_SIZE); // client_random

                            // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
                            // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
                            // This BLOB contains an encrypted 48-byte random number. For instructions
                            // on how to encrypt this random number, see section 5.1.2.1.

                            // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
                            // --------------------------------------------------------
                            // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
                            // length binary licensing data.

                            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
                            // the binary information. If wBlobLen is set to 0, then the contents
                            // of this field SHOULD be ignored.

                            // +------------------------------------+-------------------------------------+
                            // | 0x0001 BB_DATA_BLOB                | Used by License Information PDU and |
                            // |                                    | Platform Challenge Response PDU     |
                            // |                                    | ([MS-RDPELE] sections 2.2.2.3 and   |
                            // |                                    | 2.2.2.5).                           |
                            // +------------------------------------+-------------------------------------+
                            // | 0x0002 BB_RANDOM_BLOB              | Used by License Information PDU and |
                            // |                                    | New License Request PDU ([MS-RDPELE]|
                            // |                                    | sections 2.2.2.3 and 2.2.2.2).      |
                            // +------------------------------------+-------------------------------------+
                            // | 0x0003 BB_CERTIFICATE_BLOB         | Used by License Request PDU         |
                            // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
                            // +------------------------------------+-------------------------------------+
                            // | 0x0004 BB_ERROR_BLOB               | Used by License Error PDU (section  |
                            // |                                    | 2.2.1.12).                          |
                            // +------------------------------------+-------------------------------------+
                            // | 0x0009 BB_ENCRYPTED_DATA_BLOB      | Used by Platform Challenge Response |
                            // |                                    | PDU and Upgrade License PDU         |
                            // |                                    | ([MS-RDPELE] sections 2.2.2.5 and   |
                            // |                                    | 2.2.2.6).                           |
                            // +------------------------------------+-------------------------------------+
                            // | 0x000D BB_KEY_EXCHG_ALG_BLOB       | Used by License Request PDU         |
                            // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
                            // +------------------------------------+-------------------------------------+
                            // | 0x000E BB_SCOPE_BLOB               | Used by License Request PDU         |
                            // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
                            // +------------------------------------+-------------------------------------+
                            // | 0x000F BB_CLIENT_USER_NAME_BLOB    | Used by New License Request PDU     |
                            // |                                    | ([MS-RDPELE] section 2.2.2.2).      |
                            // +------------------------------------+-------------------------------------+
                            // | 0x0010 BB_CLIENT_MACHINE_NAME_BLOB | Used by New License Request PDU     |
                            // |                                    | ([MS-RDPELE] section 2.2.2.2).      |
                            // +------------------------------------+-------------------------------------+

                            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the 
                            // binary information in the blobData field. If wBlobLen is set to 0, then the
                            // blobData field is not included in the Licensing Binary BLOB structure and the
                            // contents of the wBlobType field SHOULD be ignored.

                            // blobData (variable): Variable-length binary data. The size of this data in 
                            // bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field
                            // is not included in the Licensing Binary BLOB structure.

                            stream.out_uint16_le(BB_RANDOM_BLOB);
                            stream.out_uint16_le((SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
                            stream.out_copy_bytes(null_data, SEC_MODULUS_SIZE); // rsa_data
                            stream.out_clear_bytes(SEC_PADDING_SIZE);

                            // ClientUserName (variable): A Licensing Binary BLOB structure (see
                            // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_USER_NAME_BLOB
                            // (0x000F). This BLOB contains the client user name string in
                            // null-terminated ANSI character set format and is used along with the
                            // ClientMachineName BLOB to keep track of licenses issued to clients.

                            stream.out_uint16_le(LIC::LICENSE_TAG_USER);
                            stream.out_uint16_le(userlen);
                            stream.out_copy_bytes(username, userlen);

                            // ClientMachineName (variable): A Licensing Binary BLOB structure (see
                            // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_MACHINE_NAME_BLOB
                            // (0x0010). This BLOB contains the client machine name string in
                            // null-terminated ANSI character set format and is used along with the
                            // ClientUserName BLOB to keep track of licenses issued to clients.

                            stream.out_uint16_le(LIC::LICENSE_TAG_HOST);
                            stream.out_uint16_le(hostlen);
                            stream.out_copy_bytes(hostname, hostlen);
                            stream.mark_end();
                        }

                        BStream x224_header(256);
                        BStream mcs_header(256);
                        BStream sec_header(256);

                        SEC::Sec_Send sec(sec_header, stream, SEC::SEC_LICENSE_PKT, this->encrypt, 0, 0);
                        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                                      sec_header.size() + stream.size() , MCS::PER_ENCODING);
                        X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

                        this->nego.trans->send(x224_header.data, x224_header.size());
                        this->nego.trans->send(mcs_header.data, mcs_header.size());
                        this->nego.trans->send(sec_header.data, sec_header.size());
                        this->nego.trans->send(stream.data, stream.size());
                    }
                    break;
                case LIC::PLATFORM_CHALLENGE:
                    if (this->verbose){
                        LOG(LOG_INFO, "Rdp::Platform Challenge");
                    }
                    {
                        BStream stream(65535);

                        ssllib ssl;

                        const uint8_t* in_token;
                        uint8_t out_token[LIC::LICENSE_TOKEN_SIZE];
                        uint8_t decrypt_token[LIC::LICENSE_TOKEN_SIZE];
                        uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                        uint8_t crypt_hwid[LIC::LICENSE_HWID_SIZE];
                        uint8_t out_sig[LIC::LICENSE_SIGNATURE_SIZE];

                        in_token = 0;
                        /* Parse incoming packet and save the encrypted token */
                        payload.in_skip_bytes(6); /* unknown: f8 3d 15 00 04 f6 */

                        int tokenlen = payload.in_uint16_le();
                        if (tokenlen != LIC::LICENSE_TOKEN_SIZE) {
                            LOG(LOG_ERR, "token len = %d, expected %d", tokenlen, LIC::LICENSE_TOKEN_SIZE);
                        }
                        else{
                            in_token = payload.in_uint8p(tokenlen);
                            payload.in_uint8p(LIC::LICENSE_SIGNATURE_SIZE); // in_sig
                            payload.check_end();
                        }

                        memcpy(out_token, in_token, LIC::LICENSE_TOKEN_SIZE);
                        /* Decrypt the token. It should read TEST in Unicode. */
                        RC4_KEY crypt_key;
                        ssl.rc4_set_key(crypt_key, this->lic_layer.license_key, 16);
                        memcpy(decrypt_token, in_token, LIC::LICENSE_TOKEN_SIZE);
                        ssl.rc4_crypt(crypt_key, decrypt_token, decrypt_token, LIC::LICENSE_TOKEN_SIZE);

                        hexdump((const char*)decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                        /* Generate a signature for a buffer of token and HWID */
                        buf_out_uint32(hwid, 2);
                        memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);
                //        memcpy(hwid, "\x00\x00\x00\x00\x73\x19\x46\x88\x47\xd7\xb1\xae\xe4\x0d\xbf\x5d\xd9\x63\xc9\x99", LIC::LICENSE_HWID_SIZE);
                        hexdump((const char*)hwid, LIC::LICENSE_HWID_SIZE);

                        uint8_t sealed_buffer[LIC::LICENSE_TOKEN_SIZE + LIC::LICENSE_HWID_SIZE];
                        memcpy(sealed_buffer, decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                        memcpy(sealed_buffer + LIC::LICENSE_TOKEN_SIZE, hwid, LIC::LICENSE_HWID_SIZE);
                        ssl.sign(out_sig, 16, this->lic_layer.license_sign_key, 16, sealed_buffer, sizeof(sealed_buffer));

                        /* Now encrypt the HWID */
                        ssl.rc4_set_key(crypt_key, this->lic_layer.license_key, 16);
                        memcpy(crypt_hwid, hwid, LIC::LICENSE_HWID_SIZE);
                        ssl.rc4_crypt(crypt_key, crypt_hwid, crypt_hwid, LIC::LICENSE_HWID_SIZE);

                        int length = 58;

                        stream.out_uint8(LIC::PLATFORM_CHALLENGE_RESPONSE);
                        stream.out_uint8(this->use_rdp5?3:2); /* version */
                        stream.out_uint16_le(length);

                        // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
                        // the binary information. If wBlobLen is set to 0, then the contents
                        // of this field SHOULD be ignored.
                        stream.out_uint16_le(BB_DATA_BLOB);

                        // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of
                        // the binary information in the blobData field. If wBlobLen is set to 0,
                        // then the blobData field is not included in the Licensing Binary BLOB
                        // structure and the contents of the wBlobType field SHOULD be ignored.
                        stream.out_uint16_le(LIC::LICENSE_TOKEN_SIZE);
                        stream.out_copy_bytes(out_token, LIC::LICENSE_TOKEN_SIZE);

                        // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
                        // the binary information. If wBlobLen is set to 0, then the contents
                        // of this field SHOULD be ignored.
                        stream.out_uint16_le(BB_DATA_BLOB);

                        // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of
                        // the binary information in the blobData field. If wBlobLen is set to 0,
                        // then the blobData field is not included in the Licensing Binary BLOB
                        // structure and the contents of the wBlobType field SHOULD be ignored.
                        stream.out_uint16_le(LIC::LICENSE_HWID_SIZE);
                        stream.out_copy_bytes(crypt_hwid, LIC::LICENSE_HWID_SIZE);

                        stream.out_copy_bytes(out_sig, LIC::LICENSE_SIGNATURE_SIZE);
                        stream.mark_end();

                        BStream x224_header(256);
                        BStream mcs_header(256);
                        BStream sec_header(256);
                        SEC::Sec_Send sec(sec_header, stream, SEC::SEC_LICENSE_PKT, this->encrypt, 0, 0);
                        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                                      sec_header.size() + stream.size() , MCS::PER_ENCODING);
                        X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

                        this->nego.trans->send(x224_header.data, x224_header.size());
                        this->nego.trans->send(mcs_header.data, mcs_header.size());
                        this->nego.trans->send(sec_header.data, sec_header.size());
                        this->nego.trans->send(stream.data, stream.size());
                    }
                    break;
                case LIC::NEW_LICENSE:
                {
                    if (this->verbose){
                        LOG(LOG_INFO, "Rdp::New License");
                    }

                    payload.in_skip_bytes(2); /* 3d 45 - unknown */
                    int length = payload.in_uint16_le();
                    ssllib ssl;
                    RC4_KEY crypt_key;
                    ssl.rc4_set_key(crypt_key, this->lic_layer.license_key, 16);
                    ssl.rc4_crypt(crypt_key, payload.p, payload.p, length);
                    int check = payload.in_uint16_le();
                    license_issued = 1;

                    payload.in_skip_bytes(2); /* pad */

                    length = payload.in_uint32_le();
                    payload.in_skip_bytes(length);

                    length = payload.in_uint32_le();
                    payload.in_skip_bytes(length);

                    length = payload.in_uint32_le();
                    payload.in_skip_bytes(length);

                    length = payload.in_uint32_le();

                    TODO("Save licence to keep a local copy of the licence of a remote server thus avoiding to ask it every time we connect. Not obvious files is the best choice to do that")
                    res = 1;

                    LOG(LOG_INFO, "Temporary exit to ensure that this suspicious LICENSE_issue code is really called... after some testing it looks like this code is indeed never called");
                    exit(0);
                }
                break;
                case LIC::UPGRADE_LICENSE:
                    if (this->verbose){
                        LOG(LOG_INFO, "Rdp::Upgrade License");
                    }
                    break;
                case LIC::ERROR_ALERT:
                    if (this->verbose){
                        LOG(LOG_INFO, "Rdp::Get licence status");
                    }
                TODO("This should be moved to RDP/lic.hpp (and probably the switch should also move there)")
                {
                    uint32_t dwErrorCode = payload.in_uint32_le();
                    uint32_t dwStateTransition = payload.in_uint32_le();
                    uint32_t bbErrorInfo = payload.in_uint32_le();
                    if (this->verbose){
                        LOG(LOG_INFO, "%u %u dwErrorCode=%u dwStateTransition=%u bbErrorInfo=%u",
                            version, length, dwErrorCode, dwStateTransition, bbErrorInfo);
                    }
                    res = 1;
                }
                break;
                default:
                    LOG(LOG_WARNING, "Error: unexpected license tag %x", tag);
                    break;
                }
            }
            else {
                LOG(LOG_INFO, "ERR_SEC_EXPECTED_LICENSE_NEGOTIATION_PDU");
                throw Error(ERR_SEC_EXPECTED_LICENSE_NEGOTIATION_PDU);
            }
            TODO("check if moving end is still necessary all data should have been consumed")
            if (payload.p != payload.end){
                LOG(LOG_ERR, "all data should have been consumed %s:%u ", __FILE__, __LINE__);
            }
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
            LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED bpp=%u", this->bpp);
            // read tpktHeader (4 bytes = 3 0 len)
            // TPDU class 0    (3 bytes = LI F0 PDU_DT)

            BStream stream(65536);
            X224::RecvFactory f(*this->nego.trans, stream);
            X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
            SubStream & mcs_data = x224.payload;
            MCS::SendDataIndication_Recv mcs(mcs_data, MCS::PER_ENCODING);
            SEC::Sec_Recv sec(mcs.payload, false, this->decrypt, this->encryptionLevel, this->encryptionMethod);
            SubStream & payload = sec.payload;
            if (mcs.channelId != MCS_GLOBAL_CHANNEL){
//                LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:Channel");
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

                uint32_t length = payload.in_uint32_le();
                int flags = payload.in_uint32_le();
                size_t chunk_size = payload.end - payload.p;

                this->send_to_front_channel(mod_channel.name, payload.p, length, chunk_size, flags);

                payload.p = payload.end;
            }
            else {

                uint8_t * next_packet = payload.p;
                while (next_packet < payload.end) {
                    payload.p = next_packet;
                    ShareControl sctrl(payload);
                    sctrl.recv_begin();
                    if (this->verbose){
//                        LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:len = %u pdu_type = %u",
//                            sctrl.len, sctrl.pdu_type1);
                    }
                    next_packet += sctrl.len;
                    switch (sctrl.pdu_type1) {
                    case PDUTYPE_DATAPDU:
                        switch (this->connection_finalization_state){
                        case EARLY:
                            LOG(LOG_WARNING, "Rdp::finalization is early");
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
                            LOG(LOG_INFO, "process demand active ok");
                            this->connection_finalization_state = UP_AND_RUNNING;
                        break;
                        case UP_AND_RUNNING:
                        {
//                            LOG(LOG_INFO, "Up and running bpp=%u", this->bpp);
                            ShareData sdata(payload);
                            sdata.recv_begin();
//                            LOG(LOG_INFO, "Up and running");
                            switch (sdata.pdutype2) {
                            case PDUTYPE2_UPDATE:
                            {
                                // MS-RDPBCGR: 1.3.6
                                // -----------------
                                // The most fundamental output that a server can send to a connected client
                                // is bitmap images of the remote session using the Update Bitmap PDU. This
                                // allows the client to render the working space and enables a user to
                                // interact with the session running on the server. The global palette
                                // information for a session is sent to the client in the Update Palette PDU.

                                int update_type = payload.in_uint16_le();
                                LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:update_type = %u", update_type);
                                switch (update_type) {
                                case RDP_UPDATE_ORDERS:
                                    {
                                        LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_ORDERS");
                                        payload.in_skip_bytes(2); /* pad */
                                        int count = payload.in_uint16_le();
                                        payload.in_skip_bytes(2); /* pad */
                                        this->front.begin_update();
                                        this->orders.process_orders(this->bpp, payload, count, this);
                                        this->front.end_update();
                                    }
                                    break;
                                case RDP_UPDATE_BITMAP:
                                    LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_BITMAP");
                                    this->front.begin_update();
                                    this->process_bitmap_updates(payload, this);
                                    this->front.end_update();
                                    break;
                                case RDP_UPDATE_PALETTE:
                                    LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_PALETTE");
                                    this->front.begin_update();
                                    this->process_palette(payload, this);
                                    this->front.end_update();
                                    break;
                                case RDP_UPDATE_SYNCHRONIZE:
                                    LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_SYNCHRONIZE");
                                    TODO("Replace moving end pointer by actual parsing of update synchronize");
                                    break;
                                default:
                                    LOG(LOG_INFO, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_UNKNOWN");
                                    break;
                                }
                            }
                            sdata.recv_end();
                            break;
                            case PDUTYPE2_CONTROL:
                                LOG(LOG_INFO, "mod_rdp::PDUTYPE2_CONTROL");
                            break;
                            case PDUTYPE2_SYNCHRONIZE:
                                LOG(LOG_INFO, "mod_rdp::PDUTYPE2_SYNCHRONIZE");
                            break;
                            case PDUTYPE2_POINTER:
                                LOG(LOG_INFO, "mod_rdp::PDUTYPE2_POINTER");
                                this->process_pointer_pdu(payload, this);
                            break;
                            case PDUTYPE2_PLAY_SOUND:
                                LOG(LOG_INFO, "mod_rdp::PDUTYPE2_PLAY_SOUND");
                            break;
                            case PDUTYPE2_SAVE_SESSION_INFO:
                                LOG(LOG_INFO, "DATA PDU LOGON");
                            break;
                            case PDUTYPE2_SET_ERROR_INFO_PDU:
                                LOG(LOG_INFO, "DATA PDU DISCONNECT");
                                this->process_disconnect_pdu(payload);
                            break;
                            default:
                                LOG(LOG_INFO, "mod_rdp::unknown PDUTYPE2");
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
                            this->share_id = payload.in_uint32_le();
                            uint16_t lengthSourceDescriptor = payload.in_uint16_le();
                            uint16_t lengthCombinedCapabilities = payload.in_uint16_le();
                            payload.in_skip_bytes(lengthSourceDescriptor);
                            this->process_server_caps(payload, lengthCombinedCapabilities);
//                            uint32_t sessionId = payload.in_uint32_le();
                            TODO(" we should be able to pack all the following sends to the same X224 TPDU  instead of creating a different one for each send")
                            LOG(LOG_INFO, "Sending confirm active PDU");
                            this->send_confirm_active(mod);
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
                            LOG(LOG_INFO, "Resizing to %ux%ux%u",
                                this->front_width, this->front_height, this->bpp);
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
                        LOG(LOG_INFO, "Deactivate All PDU");
                        TODO("Check we are indeed expecting Synchronize... dubious")
                        this->connection_finalization_state = WAITING_SYNCHRONIZE;
                    break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        LOG(LOG_INFO, "PDUTYPE_SERVER_REDIR_PKT");
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
            LOG(LOG_DEBUG, "mod_rdp::draw_event::Exception!!!Closing connection (status=%u)", e.id);
            BStream stream(256);
            X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
            try {
                this->nego.trans->send(stream.data, stream.size());
                LOG(LOG_DEBUG, "Connection closed (status=0)");
            }
            catch(Error e){
                LOG(LOG_DEBUG, "Connection Already closed (status=%u)", e.id);
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


        void send_confirm_active(client_mod * mod) throw(Error)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_confirm_active");
            }

            BStream stream(65536);

            // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1) 
            // containing information about the packet. The type subfield of the pduType
            // field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_CONFIRMACTIVEPDU, this->userid + MCS_USERCHANNEL_BASE);

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
            // capability sets included in the Demand Active PDU.
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
            pointer_caps.len = 8;
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

            TODO("Check caplen here")
            total_caplen = stream.get_offset() - total_caplen;

            // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.
            stream.out_uint32_le(0);
            stream.mark_end();    

//            stream.set_out_uint16_le(stream.get_offset() - (offset_caplen + 47), offset_caplen); // caplen
//            stream.set_out_uint16_le(caplen, offset_caplen); // caplen
            stream.set_out_uint16_le(total_caplen + 4, offset_caplen); // caplen
//            LOG(LOG_INFO, "total_caplen = %u, caplen=%u computed caplen=%u offset_here = %u offset_caplen=%u", total_caplen, 388, stream.get_offset() - offset_caplen, stream.get_offset(), offset_caplen);
            stream.set_out_uint16_le(capscount, offset_capscount); // caplen

            // Packet trailer
            sctrl.emit_end();

            BStream x224_header(256);
            BStream mcs_header(256);
            BStream sec_header(256);
            SEC::Sec_Send sec(sec_header, stream, 0, this->encrypt, this->encryptionLevel, this->encryptionMethod);
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header.data, x224_header.size());
            this->nego.trans->send(mcs_header.data, mcs_header.size());
            this->nego.trans->send(sec_header.data, sec_header.size());
            this->nego.trans->send(stream.data, stream.size());

            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_confirm_active done");
                LOG(LOG_INFO, "Waiting for answer to confirm active");
            }
        }


        void out_unknown_caps(Stream & stream, int id, int length, const char * caps)
        {
//            LOG(LOG_INFO, "Sending unknown caps to server");
            stream.out_uint16_le(id);
            stream.out_uint16_le(length);
            stream.out_copy_bytes(caps, length - 4);
        }


        void process_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::process_pointer_pdu");
            }

            int message_type = stream.in_uint16_le();
            stream.in_skip_bytes(2); /* pad */
            switch (message_type) {
            case RDP_POINTER_MOVE:
            {
                TODO(" implement RDP_POINTER_MOVE")
                /* int x = */ stream.in_uint16_le();
                /* int y = */ stream.in_uint16_le();
            }
            break;
            case RDP_POINTER_COLOR:
                if (this->verbose){
                    LOG(LOG_INFO, "Process pointer color");
                }
                this->process_color_pointer_pdu(stream, mod);
                if (this->verbose){
                    LOG(LOG_INFO, "Process pointer color done");
                }
                break;
            case RDP_POINTER_CACHED:
                if (this->verbose){
                    LOG(LOG_INFO, "Process pointer cached");
                }
                this->process_cached_pointer_pdu(stream, mod);
                if (this->verbose){
                    LOG(LOG_INFO, "Process pointer cached done");
                }
                break;
            case RDP_POINTER_SYSTEM:
                if (this->verbose){
                    LOG(LOG_INFO, "Process pointer system");
                }
                this->process_system_pointer_pdu(stream, mod);
                if (this->verbose){
                    LOG(LOG_INFO, "Process pointer system done");
                }
                break;
            default:
                break;
            }
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::process_pointer_pdu done");
            }
        }

        void process_palette(Stream & stream, client_mod * mod)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::process_palette");
            }

            stream.in_skip_bytes(2); /* pad */
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
            if (this->verbose){
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
            LOG(LOG_INFO, "process disconnect pdu : code = %8x", errorInfo);
        }


        TODO("this can probably be unified with process_confirm_active in front");
        void process_server_caps(Stream & stream, uint16_t len)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::process_server_caps");
            }
            uint8_t* start = stream.p;
            int ncapsets = stream.in_uint16_le();
            stream.in_skip_bytes(2); /* pad */
            for (int n = 0; n < ncapsets; n++) {
                if (stream.p + 4 > start + len) {
                    return;
                }
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
                    order_caps.log("Received from server");
                    order_caps.recv(stream, capset_length);
                    break;
                }
                default:
                    break;
                }
                stream.p = next;
            }
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::process_server_caps done");
            }
        }


        void send_control(int action) throw (Error)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_control");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
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
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header.data, x224_header.size());
            this->nego.trans->send(mcs_header.data, mcs_header.size());
            this->nego.trans->send(sec_header.data, sec_header.size());
            this->nego.trans->send(stream.data, stream.size());

            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_control done");
            }
        }

        TODO(" duplicated code in front")
        void send_synchronise() throw (Error)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_synchronise");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
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
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header.data, x224_header.size());
            this->nego.trans->send(mcs_header.data, mcs_header.size());
            this->nego.trans->send(sec_header.data, sec_header.size());
            this->nego.trans->send(stream.data, stream.size());

            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_synchronise done");
            }
        }

        void send_fonts(int seq) throw(Error)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::send_fonts");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
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
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header.data, x224_header.size());
            this->nego.trans->send(mcs_header.data, mcs_header.size());
            this->nego.trans->send(sec_header.data, sec_header.size());
            this->nego.trans->send(stream.data, stream.size());

            if (this->verbose){
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
            if (this->verbose > 10){
                LOG(LOG_INFO, "mod_rdp::send_input");
            }
            BStream stream(65536);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
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
            MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                          sec_header.size() + stream.size() , MCS::PER_ENCODING);
            X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

            this->nego.trans->send(x224_header.data, x224_header.size());
            this->nego.trans->send(mcs_header.data, mcs_header.size());
            this->nego.trans->send(sec_header.data, sec_header.size());
            this->nego.trans->send(stream.data, stream.size());

            if (this->verbose > 10){
                LOG(LOG_INFO, "mod_rdp::send_input done");
            }
        }

        virtual void rdp_input_invalidate(const Rect & r)
        {
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate");
            }
            if (UP_AND_RUNNING == this->connection_finalization_state) {
//                LOG(LOG_INFO, "rdp_input_invalidate");
                if (!r.isempty()){
                    BStream stream(65536);
                    ShareControl sctrl(stream);
                    sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
                    ShareData sdata(stream);
                    sdata.emit_begin(PDUTYPE2_REFRESH_RECT, this->share_id, RDP::STREAM_MED);

                   // Payload
                    stream.out_uint32_le(1);
                    stream.out_uint16_le(r.x);
                    stream.out_uint16_le(r.y);
                    TODO(" check this -1 (difference between rect and clip)")
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
                    MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                                  sec_header.size() + stream.size() , MCS::PER_ENCODING);
                    X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

                    this->nego.trans->send(x224_header.data, x224_header.size());
                    this->nego.trans->send(mcs_header.data, mcs_header.size());
                    this->nego.trans->send(sec_header.data, sec_header.size());
                    this->nego.trans->send(stream.data, stream.size());
                }
            }
            if (this->verbose){
                LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate done");
            }
        }

    void process_color_pointer_pdu(Stream & stream, client_mod * mod) throw(Error)
    {
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu");
        }
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
        mod->front.server_set_pointer(cursor->x, cursor->y, cursor->data, cursor->mask);
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu done");
        }
    }

    void process_cached_pointer_pdu(Stream & stream, client_mod * mod)
    {
        if (this->verbose){
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
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu done");
        }
    }

    void process_system_pointer_pdu(Stream & stream, client_mod * mod)
    {
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu");
        }
        int system_pointer_type = stream.in_uint16_le();
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                struct rdp_cursor cursor;
                memset(cursor.mask, 0xff, sizeof(cursor.mask));
                TODO(" we should pass in a cursor to set_pointer instead of individual fields")
                mod->front.server_set_pointer(cursor.x, cursor.y, cursor.data, cursor.mask);
                mod->set_pointer_display();
            }
            break;
        default:
            break;
        }
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu done");
        }
    }

    void process_bitmap_updates(Stream & stream, client_mod * mod)
    {
        if (this->verbose > 10){
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
        LOG(LOG_INFO, "/* ---------------- Sending %d rectangles ----------------- */", numberRectangles);
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

            LOG(LOG_INFO, "/* Rect [%d] bpp=%d width=%d height=%d b(%d, %d, %d, %d) */", i, bpp, width, height, boundary.x, boundary.y, boundary.cx, boundary.cy);

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
                    LOG(LOG_ERR, "unexpected bitmap size : width=%u height=%u size=%u left=%u, top=%u, right=%u, bottom=%u", width, height, size, left, top, right, bottom);
                }
                compressed = true;
            }

            TODO("check which sanity checks should be done")
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
        if (this->verbose > 10){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates done");
        }
    }

    void send_client_info_pdu(int userid, const char * password)
    {
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu");
        }
        BStream stream(1024);
        InfoPacket infoPacket;
        infoPacket.rdp5_support = this->use_rdp5;
        infoPacket.cbDomain = 2 * strlen(this->domain);
        memcpy(infoPacket.Domain, this->domain, infoPacket.cbDomain);
        infoPacket.cbUserName = 2 * strlen(this->username);
        memcpy(infoPacket.UserName, this->username, infoPacket.cbUserName);
        infoPacket.cbPassword = 2 * strlen(password);
        memcpy(infoPacket.Password, password, infoPacket.cbPassword);
        infoPacket.cbAlternateShell = 2 * strlen(this->program);
        memcpy(infoPacket.AlternateShell, this->program, infoPacket.cbAlternateShell);
        infoPacket.cbWorkingDir = 2 * strlen(this->directory);
        memcpy(infoPacket.WorkingDir, this->directory, infoPacket.cbWorkingDir);
        infoPacket.extendedInfoPacket.performanceFlags = PERF_DISABLE_WALLPAPER | this->nego.tls * ( PERF_DISABLE_FULLWINDOWDRAG
                                                                                                   | PERF_DISABLE_MENUANIMATIONS );
        infoPacket.log("Sending to server: ");
        infoPacket.emit( stream );
        stream.mark_end();        

        BStream x224_header(256);
        BStream mcs_header(256);
        BStream sec_header(256);
        SEC::Sec_Send sec(sec_header, stream, SEC::SEC_INFO_PKT, this->encrypt, this->encryptionLevel, this->encryptionMethod);
        MCS::SendDataRequest_Send mcs(mcs_header, this->userid, MCS_GLOBAL_CHANNEL, 1, 3, 
                                      sec_header.size() + stream.size() , MCS::PER_ENCODING);
        X224::DT_TPDU_Send(x224_header, mcs_header.size() + sec_header.size() + stream.size());

        this->nego.trans->send(x224_header.data, x224_header.size());
        this->nego.trans->send(mcs_header.data, mcs_header.size());
        this->nego.trans->send(sec_header.data, sec_header.size());
        this->nego.trans->send(stream.data, stream.size());

        LOG(LOG_INFO, "send login info ok");
        if (this->verbose){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu done");
        }
    }

};

#endif
