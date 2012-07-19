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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Front object (server), used to communicate with RDP client

*/

#if !defined(__FRONT_FRONT_HPP__)
#define __FRONT_FRONT_HPP__

#include "log.hpp"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <deque>
#include <algorithm>

#include "stream.hpp"
#include "constants.hpp"
#include "ssl_calls.hpp"
#include "altoco.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "capture.hpp"
#include "font.hpp"
#include "bitmap.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "RDP/caches/fontcache.hpp"
#include "RDP/caches/pointercache.hpp"
#include "RDP/caches/brushcache.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "altoco.hpp"
#include "transport.hpp"

#include "RDP/GraphicUpdatePDU.hpp"
#include "RDP/capabilities.hpp"

#include "front_api.hpp"
#include "genrandom.hpp"


class Front : public FrontAPI {
public:
    Capture * capture;
    GraphicsUpdatePDU * orders;
    Keymap2 keymap;
    ChannelDefArray channel_list;
    int up_and_running;
    int share_id;
    struct ClientInfo client_info;
    uint32_t packet_number;
    Transport * trans;
    uint16_t userid;
    uint8_t pub_mod[512];
    uint8_t pri_exp[512];
    uint8_t server_random[32];
    CryptContext encrypt, decrypt;

    int order_level;
    Inifile * ini;
    uint32_t verbose;

    struct Font font;
    BrushCache brush_cache;
    PointerCache pointer_cache;
    GlyphCache glyph_cache;

    bool palette_sent;
    bool palette_memblt_sent[6];
    BGRPalette palette332;
    BGRPalette mod_palette;
    uint8_t mod_bpp;
    BGRPalette memblt_mod_palette;
    bool mod_palette_setted;

    enum {
        CONNECTION_INITIATION,
        WAITING_FOR_LOGON_INFO,
        WAITING_FOR_ANSWER_TO_LICENCE,
        ACTIVATE_AND_PROCESS_DATA,
    } state;

    Random * gen;

    Front(Transport * trans, Random * gen, Inifile * ini)
        : FrontAPI(ini->globals.notimestamp, ini->globals.nomouse)
        , capture(NULL)
        , orders(NULL)
        , up_and_running(0)
        , share_id(65538)
        , client_info(ini->globals.crypt_level, ini->globals.channel_code, ini->globals.bitmap_compression, ini->globals.bitmap_cache)
        , packet_number(1)
        , trans(trans)
        , userid(0)
        , order_level(0)
        , ini(ini)
        , verbose(this->ini?this->ini->globals.debug.front:0)
        , font(SHARE_PATH "/" DEFAULT_FONT_NAME)
        , brush_cache()
        , pointer_cache()
        , glyph_cache()
        , state(CONNECTION_INITIATION)
        , gen(gen)
    {
        init_palette332(this->palette332);
        this->mod_palette_setted = false;
        this->palette_sent = false;
        for (size_t i = 0; i < 6 ; i++){
            this->palette_memblt_sent[i] = false;
        }

        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        switch (this->client_info.crypt_level) {
        case 1:
        case 2:
            this->decrypt.rc4_key_size = 1; /* 40 bits */
            this->encrypt.rc4_key_size = 1; /* 40 bits */
            this->decrypt.rc4_key_len = 8; /* 8 = 40 bit */
            this->encrypt.rc4_key_len = 8; /* 8 = 40 bit */
        break;
        default:
        case 3:
            this->decrypt.rc4_key_size = 2; /* 128 bits */
            this->encrypt.rc4_key_size = 2; /* 128 bits */
            this->decrypt.rc4_key_len = 16; /* 16 = 128 bit */
            this->encrypt.rc4_key_len = 16; /* 16 = 128 bit */
        break;
        }
    }

    ~Front(){
    }

    void init_mod()
    {
    }

    int server_resize(int width, int height, int bpp)
    {
        uint32_t res = 0;
        this->mod_bpp = bpp;
        if (bpp == 8){
            this->mod_palette_setted = false;
            this->palette_sent = false;
            for (size_t i = 0; i < 6 ; i++){
                this->palette_memblt_sent[i] = false;
            }
        }

        if (this->client_info.width != width
        || this->client_info.height != height) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                // resizing needed but not available
                res = -1;
            }
            else {
                LOG(LOG_INFO, "Resizing client to : %d x %d x %d", width, height, this->client_info.bpp);

                this->client_info.width = width;
                this->client_info.height = height;

                // send buffered orders
                this->orders->flush();

                // clear all pending orders, caches data, and so on and
                // start a send_deactive, send_deman_active process with
                // the new resolution setting
                /* shut down the rdp client */
                this->up_and_running = 0;
                this->send_deactive();
                /* this should do the actual resizing */
                this->send_demand_active();

                state = ACTIVATE_AND_PROCESS_DATA;
                res = 1;
            }
        }

        // resizing not necessary
        return res;
    }

    void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
    {
        int cache_idx = 0;
        switch (this->pointer_cache.add_pointer(data, mask, x, y, cache_idx)){
        case POINTER_TO_SEND:
            this->send_pointer(cache_idx, data, mask, x, y);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cache_idx);
        break;
        }
    }

    void text_metrics(const char * text, int & width, int & height){
        height = 0;
        width = 0;
        if (text) {
            size_t len = mbstowcs(0, text, 0);
            wchar_t wstr[len + 2];
            mbstowcs(wstr, text, len + 1);
            for (size_t index = 0; index < len; index++) {
                FontChar *font_item = this->font.font_items[wstr[index]];
                width += font_item->incby;
                height = std::max(height, font_item->height);
            }
        }
    }


    TODO(" implementation of the server_draw_text function below is quite broken (a small subset of possibilities is implemented  especially for data). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)")
    void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        setlocale(LC_CTYPE, "fr_FR.UTF-8");
        this->send_global_palette();

        // add text to glyph cache
        TODO(" use mbsrtowcs instead")
        int len = mbstowcs(0, text, 0);
        wchar_t* wstr = new wchar_t[len + 2];
        mbstowcs(wstr, text, len + 1);
        int total_width = 0;
        int total_height = 0;
        uint8_t *data = new uint8_t[len * 4];
        memset(data, 0, len * 4);
        int f = 0;
        int c = 0;
        int distance_from_previous_fragment = 0;
        for (int index = 0; index < len; index++) {
            FontChar* font_item = this->font.font_items[wstr[index]];

            if (!font_item){
                font_item = this->font.font_items['?'];
            }
            TODO(" avoid passing parameters by reference to get results")
            switch (this->glyph_cache.add_glyph(font_item, f, c))
            {
                case GlyphCache::GLYPH_ADDED_TO_CACHE:
                {
                    RDPGlyphCache cmd(f, 1, c,
                        font_item->offset,
                        font_item->baseline,
                        font_item->width,
                        font_item->height,
                        font_item->data);
                    this->draw(cmd);
                }
                break;
                default:
                break;
            }
            data[index * 2] = c;
            data[index * 2 + 1] = distance_from_previous_fragment;
            distance_from_previous_fragment = font_item->incby;
            total_width += font_item->incby;
            total_height = std::max(total_height, font_item->height);
        }

        const Rect bk(x, y, total_width + 1, total_height);

         RDPGlyphIndex glyphindex(
            f, // cache_id
            0x03, // fl_accel
            0x0, // ui_charinc
            1, // f_op_redundant,
            bgcolor, // bgcolor
            fgcolor, // fgcolor
            bk, // bk
            bk, // op
            // brush
            RDPBrush(0, 0, 3, 0xaa,
                (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55"),
//            this->brush,
            x,  // glyph_x
            y + total_height, // glyph_y
            len * 2, // data_len in bytes
            data // data
        );

        this->draw(glyphindex, clip);

        delete [] wstr;
        delete [] data;
    }

    void start_capture(int width, int height, bool flag, char * path,
                const char * codec_id, const char * quality,
                const char * user, const char * ip_source, const char * target_user, const char * target_device)
    {
        if (flag){
            this->stop_capture();
            char buffer[256];
            snprintf(buffer, 256, "type='OCR title bar' username='%s' client_ip='%s' ressource='%s' account='%s'", user, ip_source, target_device, target_user);
            buffer[255] = 0;
            this->capture = new Capture(width, height, path, codec_id, quality);
            this->capture->set_prefix(buffer, strlen(buffer));
        }
    }


    void periodic_snapshot(bool pointer_is_displayed)
    {
        if (this->capture){
            this->capture->snapshot(this->mouse_x, this->mouse_y,
                    pointer_is_displayed|this->nomouse, this->notimestamp);
        }
    }


    void stop_capture()
    {
        if (this->capture){
            delete this->capture;
            this->capture = 0;
        }
    }


    virtual void reset(){
        if (this->verbose){
            LOG(LOG_INFO, "Front::reset()");
            LOG(LOG_INFO, "Front::reset::use_bitmap_comp=%u", this->client_info.use_bitmap_comp);
            LOG(LOG_INFO, "Front::reset::use_compact_packets=%u", this->client_info.use_compact_packets);
            LOG(LOG_INFO, "Front::reset::bitmap_cache_version=%u", this->client_info.bitmap_cache_version);

        }

        // reset outgoing orders and reset caches
        delete this->orders;
        this->orders = new GraphicsUpdatePDU(trans,
                        this->userid,
                        this->share_id,
                        this->client_info.crypt_level,
                        this->encrypt,
                        this->ini,
                        this->client_info.bpp,
                        this->client_info.cache1_entries,
                        this->client_info.cache1_size,
                        this->client_info.cache2_entries,
                        this->client_info.cache2_size,
                        this->client_info.cache3_entries,
                        this->client_info.cache3_size,
                        this->client_info.bitmap_cache_version,
                        this->client_info.use_bitmap_comp,
                        this->client_info.use_compact_packets);

        this->pointer_cache.reset(this->client_info);
        this->brush_cache.reset(this->client_info);
        this->glyph_cache.reset(this->client_info);
    }

    void init_pointers()
    {
        pointer_item pointer0(POINTER_CURSOR0);
        this->pointer_cache.add_pointer_static(&pointer0, 0);
        this->send_pointer(0,
                         pointer0.data,
                         pointer0.mask,
                         pointer0.x,
                         pointer0.y);

        pointer_item pointer1(POINTER_CURSOR1);
        this->pointer_cache.add_pointer_static(&pointer1, 1);
        this->send_pointer(1,
                 pointer1.data,
                 pointer1.mask,
                 pointer1.x,
                 pointer1.y);
    }

    virtual void begin_update()
    {
        if (this->verbose & 8){
            LOG(LOG_INFO, "Front::begin_update()");
        }
        this->order_level++;
    }

    virtual void end_update()
    {
        if (this->verbose & 8){
            LOG(LOG_INFO, "Front::end_update()");
        }
        this->order_level--;
        if (this->order_level == 0){
            this->orders->flush();
        }
    }

    void disconnect() throw (Error)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::disconnect()");
        }
        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_DisconnectProviderUltimatum, 0, 0);
        mcs.emit_end();

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }

    void set_console_session(bool b)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::set_console_session(%u)", b);
        }
        this->client_info.console_session = b;
    }

    virtual const ChannelDefArray & get_channel_list(void) const
    {
        return this->channel_list;
    }

    virtual void send_to_channel(
        const ChannelDef & channel,
        uint8_t* data,
        size_t length,
        size_t chunk_size,
        int flags)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::send_to_channel(channel, data=%p, length=%u, chunk_size=%u, flags=%x)", data, length, chunk_size, flags);
        }

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, channel.chanid);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);

        stream.out_uint32_le(length);
        if (channel.flags & ChannelDef::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= ChannelDef::CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        stream.out_copy_bytes(data, chunk_size);

        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);

        if (this->verbose){
            LOG(LOG_INFO, "Front::send_to_channel done");
        }
    }

    // Global palette cf [MS-RDPCGR] 2.2.9.1.1.3.1.1.1 Palette Update Data
    // -------------------------------------------------------------------

    // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update type.
    // This field MUST be set to UPDATETYPE_PALETTE (0x0002).

    // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding.
    // Values in this field are ignored.

    // numberColors (4 bytes): A 32-bit, unsigned integer.
    // The number of RGB triplets in the paletteData field.
    // This field MUST be set to NUM_8BPP_PAL_ENTRIES (256).

    void send_global_palette() throw (Error)
    {
        if (!this->palette_sent && (this->client_info.bpp == 8)){

            const BGRPalette & palette = (this->mod_bpp == 8)?this->memblt_mod_palette:this->palette332;

            if (this->verbose > 4){
                LOG(LOG_INFO, "Front::send_global_palette()");
            }
            BStream stream(65536);
            Mcs mcs(stream);
            mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
            Sec sec(stream, this->encrypt);
            sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
            ShareControl sctrl(stream);
            sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
            ShareData sdata(stream);
            sdata.emit_begin(PDUTYPE2_UPDATE, this->share_id, RDP::STREAM_MED);

            // Payload
            stream.out_uint16_le(RDP_UPDATE_PALETTE);
            stream.out_uint16_le(0);
            stream.out_uint32_le(256); /* # of colors */
            for (int i = 0; i < 256; i++) {
                int color = palette[i];
                uint8_t r = color >> 16;
                uint8_t g = color >> 8;
                uint8_t b = color;
                stream.out_uint8(b);
                stream.out_uint8(g);
                stream.out_uint8(r);
            }

            // Packet trailer
            sdata.emit_end();
            sctrl.emit_end();
            sec.emit_end();
            mcs.emit_end();
            stream.end = stream.p;    

            BStream x224_header(256);
            X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

            trans->send(x224_header.data, x224_header.end - x224_header.data);
            trans->send(stream.data, stream.end - stream.data);

            this->palette_sent = true;
        }

    }

//    2.2.9.1.1.4     Server Pointer Update PDU (TS_POINTER_PDU)
//    ----------------------------------------------------------
//    The Pointer Update PDU is sent from server to client and is used to convey
//    pointer information, including pointers' bitmap images, use of system or
//    hidden pointers, use of cached cursors and position updates.

//    tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//    x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//      section 13.7.

//    mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU which
//      encapsulates an MCS Send Data Indication structure, as specified in
//      [T125] (the ASN.1 structure definitions are given in [T125] section 7,
//      parts 7 and 10). The userData field of the MCS Send Data Indication
//      contains a Security Header and the Pointer Update PDU data.

//    securityHeader (variable): Optional security header. If the Encryption
//      Level (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater
//      than ENCRYPTION_LEVEL_NONE (0) and the Encryption Method
//      (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater than
//      ENCRYPTION_METHOD_NONE (0) then this field will contain one of the
//      following headers:

//      - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//        selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_LOW (1).

//      - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//        Level selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2) or ENCRYPTION_LEVEL_HIGH (3).

//      - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Level
//        selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_FIPS (4).

//      If the Encryption Level (sections 5.3.2 and 2.2.1.4.3) selected by the
//      server is ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections
//      5.3.2 and 2.2.1.4.3) selected by the server is ENCRYPTION_METHOD_NONE
//      (0), then this header is not included in the PDU.

//    shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//      containing information about the packet. The type subfield of the
//      pduType field of the Share Control Header (section 2.2.8.1.1.1.1) MUST
//      be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//      Header MUST be set to PDUTYPE2_POINTER (27).

//    messageType (2 bytes): A 16-bit, unsigned integer. Type of pointer update.

//    +--------------------------------+---------------------------------------+
//    | 0x0001 TS_PTRMSGTYPE_SYSTEM    | Indicates a System Pointer Update     |
//    |                                | (section 2.2.9.1.1.4.3).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0003 TS_PTRMSGTYPE_POSITION  | Indicates a Pointer Position Update   |
//    |                                | (section 2.2.9.1.1.4.2).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0006 TS_PTRMSGTYPE_COLOR     | Indicates a Color Pointer Update      |
//    |                                | (section 2.2.9.1.1.4.4).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0007 TS_PTRMSGTYPE_CACHED    | Indicates a Cached Pointer Update     |
//    |                                | (section 2.2.9.1.1.4.6).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0008 TS_PTRMSGTYPE_POINTER   | Indicates a New Pointer Update        |
//    |                                | (section 2.2.9.1.1.4.5).              |
//    +--------------------------------+---------------------------------------+


//    2.2.9.1.1.4.2     Pointer Position Update (TS_POINTERPOSATTRIBUTE)
//    -------------------------------------------------------------------
//    The TS_POINTERPOSATTRIBUTE structure is used to indicate that the client
//    pointer should be moved to the specified position relative to the top-left
//    corner of the server's desktop (see [T128] section 8.14.4).

//    position (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing
//     the new x-coordinates and y-coordinates of the pointer.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.



//    2.2.9.1.1.4.3     System Pointer Update (TS_SYSTEMPOINTERATTRIBUTE)
//    -------------------------------------------------------------------
//    The TS_SYSTEMPOINTERATTRIBUTE structure is used to hide the pointer or to
//    set its shape to that of the operating system default (see [T128] section
//    8.14.1).

//    systemPointerType (4 bytes): A 32-bit, unsigned integer.
//    The type of system pointer.
//    +---------------------------|------------------------------+
//    | 0x00000000 SYSPTR_NULL    | The hidden pointer.          |
//    +---------------------------|------------------------------+
//    | 0x00007F00 SYSPTR_DEFAULT | The default system pointer.  |
//    +---------------------------|------------------------------+


//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.

    virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::send_pointer(cache_idx=%u x=%u y=%u)", cache_idx, x, y);
        }

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_POINTER, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(RDP_POINTER_COLOR);
        stream.out_uint16_le(0); /* pad */

//    cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
//      entry in the pointer cache in which to store the pointer image. The
//      number of cache entries is negotiated using the Pointer Capability Set
//      (section 2.2.7.1.5).

        stream.out_uint16_le(cache_idx);

//    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
//      x-coordinates and y-coordinates of the pointer hotspot.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(32);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(32);

//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        stream.out_uint16_le(128);

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(32*32*3);

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).
        stream.out_copy_bytes(data, 32*32*3);

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).
        stream.out_copy_bytes(mask, 128); /* mask */

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.

        // Packet trailer
        sdata.emit_end();
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);

        if (this->verbose){
            LOG(LOG_INFO, "Front::send_pointer done");
        }
    }

//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).

//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in
//      bits-per-pixel of the XOR mask contained in the colorPtrAttr field.

//    colorPtrAttr (variable): Encapsulated Color Pointer Update (section
//      2.2.9.1.1.4.4) structure which contains information about the pointer.
//      The Color Pointer Update fields are all used, as specified in section
//      2.2.9.1.1.4.4; however, the XOR mask data alignment packing is slightly
//      different. For monochrome (1 bpp) pointers the XOR data is always padded
//      to a 4-byte boundary per scan line, while color pointer XOR data is
//      still packed on a 2-byte boundary. Color XOR data is presented in the
///     color depth described in the xorBpp field (for 8 bpp, each byte contains
//      one palette index; for 4 bpp, there are two palette indices per byte).

//    2.2.9.1.1.4.6    Cached Pointer Update (TS_CACHEDPOINTERATTRIBUTE)
//    ------------------------------------------------------------------
//    The TS_CACHEDPOINTERATTRIBUTE structure is used to instruct the client to
//    change the current pointer shape to one already present in the pointer
//    cache.

//    cacheIndex (2 bytes): A 16-bit, unsigned integer. A zero-based cache entry
//      containing the cache index of the cached pointer to which the client's
//      pointer should be changed. The pointer data should have already been
//      cached using either the Color Pointer Update (section 2.2.9.1.1.4.4) or
//      New Pointer Update (section 2.2.9.1.1.4.5).

    virtual void set_pointer(int cache_idx) throw (Error)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Front::set_pointer(cache_idx=%u)", cache_idx);
        }
        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_POINTER, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(RDP_POINTER_CACHED);
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(cache_idx);

        // Packet trailer
        sdata.emit_end();
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);

        if (this->verbose){
            LOG(LOG_INFO, "Front::set_pointer done");
        }

    }

    void incoming(Callback & cb) throw (Error)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "Front::incoming()--------------------------");
        }

        switch (this->state){
        case CONNECTION_INITIATION:
        if (this->verbose){
            LOG(LOG_INFO, "Front::incoming:CONNECTION_INITIATION");
        }
        {
            // Connection Initiation
            // ---------------------

            // The client initiates the connection by sending the server an X.224 Connection
            //  Request PDU (class 0). The server responds with an X.224 Connection Confirm
            // PDU (class 0). From this point, all subsequent data sent between client and
            // server is wrapped in an X.224 Data Protocol Data Unit (PDU).

            // Client                                                     Server
            //    |------------X224 Connection Request PDU----------------> |
            //    | <----------X224 Connection Confirm PDU----------------- |

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::receiving x224 request PDU");
            }

            {
                BStream stream(65536);
                X224::RecvFactory fac_x224(*this->trans, stream);
                X224::CR_TPDU_Recv x224(*this->trans, stream, fac_x224.length);
                if (x224.header_size != (size_t)(stream.end - stream.data)){
                    LOG(LOG_ERR, "Front::incoming::connection request : all data should have been consumed,"
                                 " %d bytes remains", stream.end - stream.data - x224.header_size);
                }
            }

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::sending x224 connection confirm PDU");
            }
            {
                BStream stream(256);
                X224::CC_TPDU_Send x224(stream, 0, 0, 0);
                this->trans->send(stream.data, stream.end - stream.data);
            }
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

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::Basic Settings Exchange");
                LOG(LOG_INFO, "Front::incoming::channel_list : %u", this->channel_list.size());
            }

            mcs_recv_connect_initial(this->trans, &this->client_info, this->channel_list);

            mcs_send_connect_response(
                this->trans,
                &this->client_info,
                this->channel_list,
                this->server_random,
                this->encrypt.rc4_key_size,
                this->pub_mod,
                this->pri_exp,
                this->gen);

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

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::Channel Connection");
                LOG(LOG_INFO, "Front::incoming::mcs_recv_erect_domain_and_attach_user_request_pdu : user_id=%u", this->userid);
            }
            mcs_recv_erect_domain_and_attach_user_request_pdu(this->trans, this->userid);

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::mcs_send_attach_user_confirm_pdu : user_id=%u", this->userid);
            }
            mcs_send_attach_user_confirm_pdu(this->trans, this->userid);

            {
                uint16_t tmp_userid;
                uint16_t tmp_chanid;
                mcs_recv_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::mcs_recv_channel_join_request (G): user_id=%u chanid=%u", tmp_userid, tmp_chanid);
                }
                if (tmp_userid != this->userid){
                    LOG(LOG_INFO, "MCS error bad userid, expecting %u got %u", this->userid, tmp_userid);
                    throw Error(ERR_MCS_BAD_USERID);
                }
                if (tmp_chanid != this->userid + MCS_USERCHANNEL_BASE){
                    LOG(LOG_INFO, "MCS error bad chanid expecting %u got %u", this->userid + MCS_USERCHANNEL_BASE, tmp_chanid);
                    throw Error(ERR_MCS_BAD_CHANID);
                }
                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::mcs_channel_join_confirm_pdu (G): user_id=%u chanid=%u", this->userid, tmp_chanid);
                }
                mcs_send_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
            }

            {
                uint16_t tmp_userid;
                uint16_t tmp_chanid;
                mcs_recv_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::mcs_recv_channel_join_request (IO): user_id=%u chanid=%u", tmp_userid, tmp_chanid);
                }
                if (tmp_userid != this->userid){
                    LOG(LOG_INFO, "MCS error bad userid, expecting %u got %u", this->userid, tmp_userid);
                    throw Error(ERR_MCS_BAD_USERID);
                }
                if (tmp_chanid != MCS_GLOBAL_CHANNEL){
                    LOG(LOG_INFO, "MCS error bad chanid expecting %u got %u", MCS_GLOBAL_CHANNEL, tmp_chanid);
                    throw Error(ERR_MCS_BAD_CHANID);
                }
                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::mcs_channel_join_confirm_pdu (IO): user_id=%u chanid=%u", this->userid, tmp_chanid);
                }
                mcs_send_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
            }

            for (size_t i = 0 ; i < this->channel_list.size() ; i++){
                    uint16_t tmp_userid;
                    uint16_t tmp_chanid;
                    mcs_recv_channel_join_request_pdu(this->trans, tmp_userid, tmp_chanid);
                    if (tmp_userid != this->userid){
                        LOG(LOG_INFO, "MCS error bad userid, expecting %u got %u", this->userid, tmp_userid);
                        throw Error(ERR_MCS_BAD_USERID);
                    }
                    if (tmp_chanid != this->channel_list[i].chanid){
                        LOG(LOG_INFO, "MCS error bad chanid expecting %u got %u", this->channel_list[i].chanid, tmp_chanid);
                        throw Error(ERR_MCS_BAD_CHANID);
                    }
                    if (this->verbose){
                        LOG(LOG_INFO, "Front::incoming::mcs_channel_join_confirm_pdu : user_id=%u chanid=%u", this->userid, tmp_chanid);
                    }
                    mcs_send_channel_join_confirm_pdu(this->trans, this->userid, tmp_chanid);
                    this->channel_list.set_chanid(i, tmp_chanid);
            }

            if (this->verbose){
                LOG(LOG_INFO, "Front::incoming::RDP Security Commencement");
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

            recv_security_exchange_PDU(this->trans, this->decrypt, this->encrypt, this->server_random, this->pub_mod, this->pri_exp);

            this->state = WAITING_FOR_LOGON_INFO;
        }
        break;

        case WAITING_FOR_LOGON_INFO:
        // Secure Settings Exchange
        // ------------------------

        // Secure Settings Exchange: Secure client data (such as the username,
        // password and auto-reconnect cookie) is sent to the server using the Client
        // Info PDU.

        // Client                                                     Server
        //    |------ Client Info PDU      ---------------------------> |

        if (this->verbose){
            LOG(LOG_INFO, "Front::incoming::Secure Settings Exchange");
        }
        {
            BStream stream(65536);
            X224::RecvFactory fx224(*this->trans, stream);
            X224::DT_TPDU_Recv x224(*this->trans, stream, fx224.length);
            SubStream payload(stream, x224.header_size);

            Mcs mcs(payload);
            mcs.recv_begin();
            if ((mcs.opcode >> 2) != MCSPDU_SendDataRequest) {
                TODO("We should make a special case for MCSPDU_DisconnectProviderUltimatum, as this one is a demand to end connection");
                // mcs.opcode >> 2) == MCSPDU_DisconnectProviderUltimatum
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            if (this->verbose >= 256){
                this->decrypt.dump();
            }
            Sec sec(payload, this->decrypt);
            sec.recv_begin(true);

            if (!sec.flags & SEC_INFO_PKT) {
                throw Error(ERR_SEC_EXPECTED_LOGON_INFO);
            }

            /* this is the first test that the decrypt is working */
            this->client_info.process_logon_info(payload, (uint16_t)(payload.end - payload.p));
            sec.recv_end();
            mcs.recv_end();

            TODO("check all data are consumed as expected")
            if (payload.end != payload.p){
                LOG(LOG_ERR, "Front::incoming::process_logon all data should have been consumed");
            }

            this->keymap.init_layout(this->client_info.keylayout);

            if (this->client_info.is_mce) {
                LOG(LOG_INFO, "Front::incoming::licencing client_info.is_mce");
                LOG(LOG_INFO, "Front::incoming::licencing send_media_lic_response");
                send_media_lic_response(this->trans, this->userid);

                // proceed with capabilities exchange

                // Capabilities Exchange
                // ---------------------

                // Capabilities Negotiation: The server sends the set of capabilities it
                // supports to the client in a Demand Active PDU. The client responds with its
                // capabilities by sending a Confirm Active PDU.

                // Client                                                     Server
                //    | <------- Demand Active PDU ---------------------------- |
                //    |--------- Confirm Active PDU --------------------------> |

                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::send_demand_active");
                }
                this->send_demand_active();

                this->state = ACTIVATE_AND_PROCESS_DATA;
            }
            else {
                LOG(LOG_INFO, "Front::incoming::licencing not client_info.is_mce");
                LOG(LOG_INFO, "Front::incoming::licencing send_lic_initial");

                send_lic_initial(this->trans, this->userid);

                LOG(LOG_INFO, "Front::incoming::waiting for answer to lic_initial");
                this->state = WAITING_FOR_ANSWER_TO_LICENCE;
            }
        }
        break;

        case WAITING_FOR_ANSWER_TO_LICENCE:
        if (this->verbose){
            LOG(LOG_INFO, "Front::incoming::WAITING_FOR_ANSWER_TO_LICENCE");
        }
        {
            BStream stream(65536);
            X224::RecvFactory fx224(*this->trans, stream);
            X224::DT_TPDU_Recv x224(*this->trans, stream, fx224.length);

            SubStream payload(stream, x224.header_size);
            Mcs mcs(payload);
            mcs.recv_begin();
            if ((mcs.opcode >> 2) != MCSPDU_SendDataRequest) {
                TODO("We should make a special case for MCSPDU_DisconnectProviderUltimatum, as this one is a demand to end connection");
                // mcs.opcode >> 2) == MCSPDU_DisconnectProviderUltimatum
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            if (this->verbose >= 256){
                this->decrypt.dump();
            }

            Sec sec(mcs.payload, this->decrypt);
            sec.recv_begin(true);

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

            // Disconnect Provider Ultimatum datagram

            if (sec.flags & SEC_LICENSE_PKT) {
                uint8_t tag = sec.payload.in_uint8();
                uint8_t version = sec.payload.in_uint8();
                uint16_t length = sec.payload.in_uint16_le();
                LOG(LOG_INFO, "Front::WAITING_FOR_ANSWER_TO_LICENCE sec_flags=%x %u %u %u", sec.flags, tag, version, length);

                switch (tag) {
                case LICENSE_REQUEST:
                    LOG(LOG_INFO, "Front::LICENSE_REQUEST");
                    LOG(LOG_INFO, "Front::incoming::licencing send_lic_response");
                    send_lic_response(this->trans, this->userid);
                    break;
                case LICENSE_INFO:
                    LOG(LOG_INFO, "Front::LICENSE_INFO");
                    break;
                case PLATFORM_CHALLENGE:
                    LOG(LOG_INFO, "Front::PLATFORM_CHALLENGE");
                    break;
                case NEW_LICENSE:
                    LOG(LOG_INFO, "Front::NEW_LICENSE");
                    break;
                case UPGRADE_LICENSE:
                    LOG(LOG_INFO, "Front::UPGRADE_LICENSE");
                    break;
                case ERROR_ALERT:
                    LOG(LOG_INFO, "Front::ERROR_ALERT");
                    break;
                case NEW_LICENSE_REQUEST:
                    LOG(LOG_INFO, "Front::NEW_LICENSE_REQUEST");
                    LOG(LOG_INFO, "Front::incoming::licencing send_lic_response");
                    send_lic_response(this->trans, this->userid);
                    break;
                case PLATFORM_CHALLENGE_RESPONSE:
                    LOG(LOG_INFO, "Front::PLATFORM_CHALLENGE_RESPONSE");
                    break;
                default:
                    LOG(LOG_INFO, "Front::LICENCE_TAG_UNKNOWN %u", tag);
                    break;
                }
                // licence received, proceed with capabilities exchange

                // Capabilities Exchange
                // ---------------------

                // Capabilities Negotiation: The server sends the set of capabilities it
                // supports to the client in a Demand Active PDU. The client responds with its
                // capabilities by sending a Confirm Active PDU.

                // Client                                                     Server
                //    | <------- Demand Active PDU ---------------------------- |
                //    |--------- Confirm Active PDU --------------------------> |

                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::send_demand_active");
                }
                this->send_demand_active();

                this->state = ACTIVATE_AND_PROCESS_DATA;
            }
            else {
                if (this->verbose){
                    LOG(LOG_INFO, "non licence packet: still waiting for licence");
                }
                ShareControl sctrl(payload);
                sctrl.recv_begin();

                switch (sctrl.pdu_type1) {
                case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
                    if (this->verbose){
                        LOG(LOG_INFO, "unexpected DEMANDACTIVE PDU while in licence negociation");
                    }
                    break;
                case PDUTYPE_CONFIRMACTIVEPDU:
                    if (this->verbose){
                        LOG(LOG_INFO, "Unexpected CONFIRMACTIVE PDU");
                    }
                    {
                        uint32_t share_id = payload.in_uint32_le();
                        uint16_t originatorId = payload.in_uint16_le();
                        this->process_confirm_active(payload);
                    }

                    break;
                case PDUTYPE_DATAPDU: /* 7 */
                    if (this->verbose & 4){
                        LOG(LOG_INFO, "unexpected DATA PDU while in licence negociation");
                    }
                    // at this point licence negociation is still ongoing
                    // most data packets should not be received
                    // actually even input is dubious,
                    // but rdesktop actually sends input data
                    this->process_data(payload, cb);
                    break;
                case PDUTYPE_DEACTIVATEALLPDU:
                    if (this->verbose){
                        LOG(LOG_INFO, "unexpected DEACTIVATEALL PDU while in licence negociation");
                    }
                    break;
                case PDUTYPE_SERVER_REDIR_PKT:
                    if (this->verbose){
                        LOG(LOG_INFO, "unsupported SERVER_REDIR_PKT while in licence negociation");
                    }
                    break;
                default:
                    LOG(LOG_WARNING, "unknown PDU type received while in licence negociation (%d)\n", sctrl.pdu_type1);
                    break;
                }
                sctrl.recv_end();
            }
            sec.payload.p = sec.payload.end;
            sec.recv_end();
            mcs.payload.p = mcs.payload.end;
            mcs.recv_end();
            if (payload.p != payload.end){
                LOG(LOG_ERR, "All data should have been consumed, %u bytes remaining", payload.end - payload.p);
            }
        }
        break;

        case ACTIVATE_AND_PROCESS_DATA:
        if (this->verbose & 4){
            LOG(LOG_INFO, "Front::incoming::ACTIVATE_AND_PROCESS_DATA");
        }
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
        {
            ChannelDefArray & channel_list = this->channel_list;

            BStream stream(65536);
            X224::RecvFactory fx224(*this->trans, stream);
            TODO("We shall put a specific case when we get Disconnect Request")
            if (fx224.type == X224::DR_TPDU){
                TODO("What is the clean way to actually disconnect ?")
                X224::DR_TPDU_Recv x224(*this->trans, stream, fx224.length);
                LOG(LOG_INFO, "Front::Received Disconnect Request from RDP client");
                throw Error(ERR_X224_EXPECTED_DATA_PDU);
            }
            else if (fx224.type != X224::DT_TPDU){
                LOG(LOG_INFO, "Front::Unexpected non data PDU (got %u)", fx224.type);
                throw Error(ERR_X224_EXPECTED_DATA_PDU);
            }

            X224::DT_TPDU_Recv x224(*this->trans, stream, fx224.length);

            SubStream payload(stream, x224.header_size);
            Mcs mcs(payload);
            mcs.recv_begin();

            // Disconnect Provider Ultimatum datagram
            if ((mcs.opcode >> 2) == MCSPDU_DisconnectProviderUltimatum) {
                throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
            }

            if ((mcs.opcode >> 2) != MCSPDU_SendDataRequest) {
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            Sec sec(payload, this->decrypt);
            sec.recv_begin(true);

            if (this->verbose & 4){
                LOG(LOG_INFO, "Front::incoming::sec_flags=%x", sec.flags);
            }

            if (sec.flags & 0x0400){ /* SEC_REDIRECT_ENCRYPT */
                if (this->verbose){
                    LOG(LOG_INFO, "Front::incoming::SEC_REDIRECT_ENCRYPT not supported");
                    throw Error(ERR_X224_EXPECTED_DATA_PDU);
                }
//                /* Check for a redirect packet, starts with 00 04 */
//                if (stream.p[0] == 0 && stream.p[1] == 4){
//                /* for some reason the PDU and the length seem to be swapped.
//                   This isn't good, but we're going to do a byte for byte
//                   swap.  So the first four value appear as: 00 04 XX YY,
//                   where XX YY is the little endian length. We're going to
//                   use 04 00 as the PDU type, so after our swap this will look
//                   like: XX YY 04 00 */

//                    uint8_t swapbyte1 = stream.p[0];
//                    stream.p[0] = stream.p[2];
//                    stream.p[2] = swapbyte1;

//                    uint8_t swapbyte2 = stream.p[1];
//                    stream.p[1] = stream.p[3];
//                    stream.p[3] = swapbyte2;

//                    uint8_t swapbyte3 = stream.p[2];
//                    stream.p[2] = stream.p[3];
//                    stream.p[3] = swapbyte3;
//                }
            }

            if (mcs.chan_id != MCS_GLOBAL_CHANNEL) {
                size_t num_channel_src = channel_list.size();
                for (size_t index = 0; index < channel_list.size(); index++){
                    if (channel_list[index].chanid == mcs.chan_id){
                        num_channel_src = index;
                        break;
                    }
                }

                if (num_channel_src >= channel_list.size()) {
                    LOG(LOG_ERR, "Front::incoming::Unknown Channel");
                    throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                }

                const ChannelDef & channel = channel_list[num_channel_src];

                int length = payload.in_uint32_le();
                int flags = payload.in_uint32_le();

                size_t chunk_size = payload.end - payload.p;

                if (this->up_and_running){
                    cb.send_to_mod_channel(channel.name, payload.p, length, chunk_size, flags);
                }
                payload.p += chunk_size;
            }
            else {
                while (payload.p < payload.end) {
                    ShareControl sctrl(payload);
                    sctrl.recv_begin();

                    switch (sctrl.pdu_type1) {
                    case PDUTYPE_DEMANDACTIVEPDU:
                        if (this->verbose){
                            LOG(LOG_INFO, "Front received DEMANDACTIVEPDU");
                        }
                        break;
                    case PDUTYPE_CONFIRMACTIVEPDU:
                        if (this->verbose){
                            LOG(LOG_INFO, "Front received CONFIRMACTIVEPDU");
                        }
                        {
                            uint32_t share_id = payload.in_uint32_le();
                            uint16_t originatorId = payload.in_uint16_le();
                            this->process_confirm_active(payload);
                        }
                        // reset caches, etc.
                        this->reset();
                        // resizing done
                        BGRPalette palette;
                        init_palette332(palette);
                        this->color_cache(palette, 0);
                        this->init_pointers();
                        break;
                    case PDUTYPE_DATAPDU: /* 7 */
                        if (this->verbose & 4){
                            LOG(LOG_INFO, "Front received DATAPDU");
                        }
                        // this is rdp_process_data that will set up_and_running to 1
                        // when fonts have been received
                        // we will not exit this loop until we are in this state.
                        this->process_data(payload, cb);
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        if (this->verbose){
                            LOG(LOG_INFO, "Front received DEACTIVATEALLPDU (unsupported)");
                        }
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        if (this->verbose){
                            LOG(LOG_INFO, "Front received SERVER_REDIR_PKT (unsupported)");
                        }
                        break;
                    default:
                        LOG(LOG_WARNING, "Front received unknown PDU type in session_data (%d)\n", sctrl.pdu_type1);
                        break;
                    }
                    sctrl.recv_end();
                }
            }
            sec.recv_end();
            mcs.recv_end();
            TODO("check all data have been consumed")
        }
        break;
        }
    }

    /*****************************************************************************/
    void send_data_update_sync() throw (Error)
    {
        if (this->verbose){
            LOG(LOG_INFO, "send_data_update_sync");
        }
        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_UPDATE, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
        stream.out_clear_bytes(2);

        // Packet trailer
        sdata.emit_end();
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }



    /*****************************************************************************/
    void send_demand_active() throw (Error)
    {
        LOG(LOG_INFO, "Front::send_demand_active");

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DEMANDACTIVEPDU, this->userid + MCS_USERCHANNEL_BASE);

        // Payload
        size_t caps_count = 0;
        stream.out_uint32_le(this->share_id);
        stream.out_uint16_le(4); /* 4 chars for RDP\0 */

        /* 2 bytes size after num caps, set later */
        uint8_t * caps_size_ptr = stream.p;
        stream.out_clear_bytes(2);
        stream.out_copy_bytes("RDP", 4);

        /* 4 byte num caps, set later */
        uint8_t * caps_count_ptr = stream.p;
        stream.out_clear_bytes(4);

        uint8_t * caps_ptr = stream.p;

        GeneralCaps general_caps;
        general_caps.log("Sending to client");
        general_caps.emit(stream);
        caps_count++;

        BitmapCaps bitmap_caps;
        bitmap_caps.preferredBitsPerPixel = this->client_info.bpp;
        bitmap_caps.desktopWidth = this->client_info.width;
        bitmap_caps.desktopHeight = this->client_info.height;
        bitmap_caps.log("Sending to client");
        bitmap_caps.emit(stream);
        caps_count++;

        FontCaps font_caps;
        font_caps.log("Sending to client");
        font_caps.emit(stream);
        caps_count++;

        OrderCaps order_caps;
        order_caps.pad4octetsA = 0x40420f00;
        order_caps.numberFonts = 0x2f;
        order_caps.orderFlags = 0x22;
        order_caps.orderSupport[TS_NEG_DSTBLT_INDEX] = 1;
        order_caps.orderSupport[TS_NEG_PATBLT_INDEX] = 1;
        order_caps.orderSupport[TS_NEG_SCRBLT_INDEX] = 1;
        order_caps.orderSupport[TS_NEG_MEMBLT_INDEX] = 1;
        order_caps.orderSupport[TS_NEG_LINETO_INDEX] = 1;
        order_caps.orderSupport[UnusedIndex3] = 1;
        order_caps.textFlags = 0x06a1;
        order_caps.pad4octetsB = 0x0f4240;
        order_caps.desktopSaveSize = 0x0f4240;
        order_caps.pad2octetsC = 1;
        order_caps.log("Sending to client");
        order_caps.emit(stream);
        caps_count++;

        ColorCacheCaps colorcache_caps;
        colorcache_caps.log("Sending to client");
        colorcache_caps.emit(stream);
        caps_count++;

        PointerCaps pointer_caps;
        pointer_caps.colorPointerCacheSize = 0x19;
        pointer_caps.pointerCacheSize = 0x19;
        pointer_caps.log("Sending to client");
        pointer_caps.emit(stream);
         caps_count++;

        ShareCaps share_caps;
        share_caps.nodeId = this->userid + MCS_USERCHANNEL_BASE;
        share_caps.pad2octets = 0xb5e2; /* 0x73e1 */
        share_caps.log("Sending to client");
        share_caps.emit(stream);
        caps_count++;

        InputCaps input_caps;
        input_caps.inputFlags = 1;
        input_caps.keyboardLayout = 0;
        input_caps.keyboardType = 0;
        input_caps.keyboardSubType = 0;
        input_caps.keyboardFunctionKey = 0;
        input_caps.log("Sending to client");
        input_caps.emit(stream);
        caps_count++;

        TODO("Check if this padding is necessary and if so how it should actually be computed. Padding is usually here for memory alignment purpose but this one looks strange")
        stream.out_clear_bytes(4); /* pad */

        size_t caps_size = stream.p - caps_ptr;
        TODO("change this using set_out_uint16_le")
        caps_size_ptr[0] = caps_size;
        caps_size_ptr[1] = caps_size >> 8;

        TODO("change this using set_out_uint32_le")
        caps_count_ptr[0] = caps_count;
        caps_count_ptr[1] = caps_count >> 8;
        caps_count_ptr[2] = caps_count >> 16;
        caps_count_ptr[3] = caps_count >> 24;

        // Packet trailer
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }


    /* store the number of client cursor cache in client_info */
    void capset_pointercache(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_pointercache");
    }


    void process_confirm_active(Stream & stream)
    {
        LOG(LOG_INFO, "process_confirm_active");
        uint16_t lengthSourceDescriptor = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        uint16_t lengthCombinedCapabilities = stream.in_uint16_le();
        stream.in_skip_bytes(lengthSourceDescriptor);

        LOG(LOG_INFO, "lengthSourceDescriptor = %u", lengthSourceDescriptor);
        LOG(LOG_INFO, "lengthCombinedCapabilities = %u", lengthCombinedCapabilities);


        uint8_t * start = stream.p;
        uint8_t* theoricCapabilitiesEnd = start + lengthCombinedCapabilities;
        uint8_t* actualCapabilitiesEnd = stream.end;

        int numberCapabilities = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (int n = 0; n < numberCapabilities; n++) {
            LOG(LOG_INFO, "capability %u", n);
            if (stream.p + 4 > theoricCapabilitiesEnd) {
                LOG(LOG_ERR, "Incomplete capabilities received (bad length): expected length=%d need=%d available=%d", 
                    lengthCombinedCapabilities, 
                    stream.p-start, 
                    stream.end-stream.p);
            }
            if (stream.p + 4 > actualCapabilitiesEnd) {
                LOG(LOG_ERR, "Incomplete capabilities received (need more data): expected length=%d need=%d available=%d", 
                    lengthCombinedCapabilities, 
                    stream.p-start, 
                    stream.end-stream.p);
                return;
            }

            uint16_t capset_type = stream.in_uint16_le();
            uint16_t capset_length = stream.in_uint16_le();
            uint8_t * next = (stream.p + capset_length) - 4;

            switch (capset_type) {
            case CAPSTYPE_GENERAL: {
                    GeneralCaps general;
                    general.recv(stream, capset_length);
                    general.log("Receiving from client");
                    this->client_info.use_compact_packets = (general.extraflags & NO_BITMAP_COMPRESSION_HDR)?1:0;
                }
                break;
            case CAPSTYPE_BITMAP: {
                    BitmapCaps bitmap_caps;
                    bitmap_caps.recv(stream, capset_length);
                    bitmap_caps.log("Receiving from client");
                    this->client_info.bpp = (bitmap_caps.preferredBitsPerPixel >= 24)?24:bitmap_caps.preferredBitsPerPixel;
                    this->client_info.width = bitmap_caps.desktopWidth;
                    this->client_info.height = bitmap_caps.desktopHeight;
                }
                break;
            case CAPSTYPE_ORDER: { /* 3 */
                    OrderCaps order_caps;
                    order_caps.log("Receiving from client");
                    order_caps.recv(stream, capset_length);
                }
                break;
            case CAPSTYPE_BITMAPCACHE: {
                    BmpCacheCaps bmpcache_caps;
                    bmpcache_caps.recv(stream, capset_length);
                    bmpcache_caps.log("Receiving from client");
                    this->client_info.cache1_entries = bmpcache_caps.cache0Entries;
                    this->client_info.cache1_size = bmpcache_caps.cache0MaximumCellSize;
                    this->client_info.cache2_entries = bmpcache_caps.cache1Entries;
                    this->client_info.cache2_size = bmpcache_caps.cache1MaximumCellSize;
                    this->client_info.cache3_entries = bmpcache_caps.cache2Entries;
                    this->client_info.cache3_size = bmpcache_caps.cache2MaximumCellSize;
                }
                break;
            case CAPSTYPE_CONTROL: /* 5 */
                break;
            case CAPSTYPE_ACTIVATION: /* 7 */
                break;
            case CAPSTYPE_POINTER: {  /* 8 */
                    stream.in_skip_bytes(2); /* color pointer */
                    int i = stream.in_uint16_le();
                    this->client_info.pointer_cache_entries = std::min(i, 32);
                }
                break;
            case CAPSTYPE_SHARE: /* 9 */
                break;
            case CAPSTYPE_COLORCACHE: /* 10 */
                break;
            case CAPSTYPE_SOUND:
                break;
            case CAPSTYPE_INPUT: /* 13 */
                break;
            case CAPSTYPE_FONT: /* 14 */
                break;
            case CAPSTYPE_BRUSH: { /* 15 */
                    BrushCacheCaps brushcache_caps;
                    brushcache_caps.log("Receiving from client");
                    brushcache_caps.recv(stream, capset_length);
                    this->client_info.brush_cache_code = brushcache_caps.brushSupportLevel;
                }
                break;
            case CAPSTYPE_GLYPHCACHE: /* 16 */
                break;
            case CAPSTYPE_OFFSCREENCACHE: /* 17 */
                break;
            case CAPSTYPE_BITMAPCACHE_HOSTSUPPORT: /* 18 */
                break;
            case CAPSTYPE_BITMAPCACHE_REV2: {
//                    BmpCache2Caps bmpcache2_caps;
//                    bmpcache2_caps.recv(stream, capset_length);
//                    bmpcache2_caps.log("Receiving from client");

                    LOG(LOG_INFO, "capset_bmpcache2");
                    this->client_info.bitmap_cache_version = 2;
                    int Bpp = nbbytes(this->client_info.bpp);
                    this->client_info.bitmap_cache_persist_enable = stream.in_uint16_le();
                    stream.in_skip_bytes(2); /* number of caches in set, 3 */
                    this->client_info.cache1_entries = stream.in_uint32_le();
                    this->client_info.cache1_size = 256 * Bpp;
                    this->client_info.cache2_entries = stream.in_uint32_le();
                    this->client_info.cache2_size = 1024 * Bpp;
                    this->client_info.cache3_entries = (stream.in_uint32_le() & 0x7fffffff);
                    this->client_info.cache3_size = 4096 * Bpp;
                }
                break;
            case CAPSTYPE_VIRTUALCHANNEL: /* 20 */
                break;
            case CAPSTYPE_DRAWNINEGRIDCACHE: /* 21 */
                break;
            case CAPSTYPE_DRAWGDIPLUS: /* 22 */
                break;
            case CAPSTYPE_RAIL: /* 23 */
                break;
            case CAPSTYPE_WINDOW: /* 24 */
                break;
            case CAPSETTYPE_COMPDESK: { /* 25 */
                    CompDeskCaps compdesk_caps;
                    compdesk_caps.log("Receiving from client");
                    compdesk_caps.recv(stream, capset_length);
                }
                break;
            case CAPSETTYPE_MULTIFRAGMENTUPDATE: /* 26 */
                break;
            case CAPSETTYPE_LARGE_POINTER: /* 27 */
                break;
            case CAPSETTYPE_SURFACE_COMMANDS: /* 28 */
                break;
            case CAPSETTYPE_BITMAP_CODECS: /* 29 */
                break;
            case CAPSETTYPE_FRAME_ACKNOWLEDGE: /* 30 */
                break;
            default:
                break;
            }
            if (stream.p > next){
                LOG(LOG_ERR, "read out of bound detected");
            }
            stream.p = next;
        }
    }

// 2.2.1.19 Server Synchronize PDU
// ===============================

// The Server Synchronize PDU is an RDP Connection Sequence PDU sent from server
// to client during the Connection Finalization phase (see section 1.3.1.1). It
// is sent after receiving the Confirm Active PDU (section 2.2.1.13.2).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU which
//   encapsulates an MCS Send Data Indication structure, as specified in [T125]
//   (the ASN.1 structure definitions are given in section 7, parts 7 and 10 of
//   [T125]). The userData field of the MCS Send Data Indication contains a
//   Security Header and the Synchronize PDU Data (section 2.2.1.14.1).

// securityHeader (variable): Optional security header. If the Encryption Level
//   (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater than
//   ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections 5.3.2 and
//   2.2.1.4.3) selected by the server is greater than ENCRYPTION_METHOD_NONE
//   (0) then this field will contain one of the following headers:

//   - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//     selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//     ENCRYPTION_LEVEL_LOW (1).

//  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Level
//    selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//    ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2), or ENCRYPTION_LEVEL_HIGH (3).

//  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Level
//    selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//    ENCRYPTION_LEVEL_FIPS (4).

// If the Encryption Level (sections 5.3.2 and 2.2.1.4.3) selected by the server
// is ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections 5.3.2 and
// 2.2.1.4.3) selected by the server is ENCRYPTION_METHOD_NONE (0), then this
// header is not included in the PDU.

// synchronizePduData (22 bytes): The contents of the Synchronize PDU as
// described in section 2.2.1.14.1.

// 2.2.1.14.1 Synchronize PDU Data (TS_SYNCHRONIZE_PDU)
// ====================================================
// The TS_SYNCHRONIZE_PDU structure is a standard T.128 Synchronize PDU (see
// [T128] section 8.6.1).

// shareDataHeader (18 bytes): Share Control Header (section 2.2.8.1.1.1.1)
//   containing information about the packet. The type subfield of the pduType
//   field of the Share Control Header MUST be set to PDUTYPE_DATAPDU (7). The
//   pduType2 field of the Share Data Header MUST be set to PDUTYPE2_SYNCHRONIZE
//   (31).

// messageType (2 bytes): A 16-bit, unsigned integer. The message type. This
//   field MUST be set to SYNCMSGTYPE_SYNC (1).

// targetUser (2 bytes): A 16-bit, unsigned integer. The MCS channel ID of the
//   target user.

    TODO(" duplicated code in mod/rdp")
    void send_synchronize()
    {
        LOG(LOG_INFO, "send_synchronize");

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_SYNCHRONIZE, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(1); /* messageType */
        stream.out_uint16_le(1002); /* control id */

        // Packet trailer
        sdata.emit_end();
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }

// 2.2.1.15.1 Control PDU Data (TS_CONTROL_PDU)
// ============================================

// The TS_CONTROL_PDU structure is a standard T.128 Synchronize PDU (see [T128]
// section 8.12).

// shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//   containing information about the packet. The type subfield of the pduType
//   field of the Share Control Header (section 2.2.8.1.1.1.1) MUST be set to
//   PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header MUST be set
//   to PDUTYPE2_CONTROL (20).

// action (2 bytes): A 16-bit, unsigned integer. The action code.
// 0x0001 CTRLACTION_REQUEST_CONTROL Request control
// 0x0002 CTRLACTION_GRANTED_CONTROL Granted control
// 0x0003 CTRLACTION_DETACH Detach
// 0x0004 CTRLACTION_COOPERATE Cooperate

// grantId (2 bytes): A 16-bit, unsigned integer. The grant identifier.

// controlId (4 bytes): A 32-bit, unsigned integer. The control identifier.

    void send_control(int action)
    {
        LOG(LOG_INFO, "send_control action=%u", action);

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(1002); /* control id */

        // Packet trailer
        sdata.emit_end();
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }



    /*****************************************************************************/
    void send_fontmap() throw (Error)
    {
        LOG(LOG_INFO, "send_fontmap");

    static uint8_t g_fontmap[172] = { 0xff, 0x02, 0xb6, 0x00, 0x28, 0x00, 0x00, 0x00,
                                0x27, 0x00, 0x27, 0x00, 0x03, 0x00, 0x04, 0x00,
                                0x00, 0x00, 0x26, 0x00, 0x01, 0x00, 0x1e, 0x00,
                                0x02, 0x00, 0x1f, 0x00, 0x03, 0x00, 0x1d, 0x00,
                                0x04, 0x00, 0x27, 0x00, 0x05, 0x00, 0x0b, 0x00,
                                0x06, 0x00, 0x28, 0x00, 0x08, 0x00, 0x21, 0x00,
                                0x09, 0x00, 0x20, 0x00, 0x0a, 0x00, 0x22, 0x00,
                                0x0b, 0x00, 0x25, 0x00, 0x0c, 0x00, 0x24, 0x00,
                                0x0d, 0x00, 0x23, 0x00, 0x0e, 0x00, 0x19, 0x00,
                                0x0f, 0x00, 0x16, 0x00, 0x10, 0x00, 0x15, 0x00,
                                0x11, 0x00, 0x1c, 0x00, 0x12, 0x00, 0x1b, 0x00,
                                0x13, 0x00, 0x1a, 0x00, 0x14, 0x00, 0x17, 0x00,
                                0x15, 0x00, 0x18, 0x00, 0x16, 0x00, 0x0e, 0x00,
                                0x18, 0x00, 0x0c, 0x00, 0x19, 0x00, 0x0d, 0x00,
                                0x1a, 0x00, 0x12, 0x00, 0x1b, 0x00, 0x14, 0x00,
                                0x1f, 0x00, 0x13, 0x00, 0x20, 0x00, 0x00, 0x00,
                                0x21, 0x00, 0x0a, 0x00, 0x22, 0x00, 0x06, 0x00,
                                0x23, 0x00, 0x07, 0x00, 0x24, 0x00, 0x08, 0x00,
                                0x25, 0x00, 0x09, 0x00, 0x26, 0x00, 0x04, 0x00,
                                0x27, 0x00, 0x03, 0x00, 0x28, 0x00, 0x02, 0x00,
                                0x29, 0x00, 0x01, 0x00, 0x2a, 0x00, 0x05, 0x00,
                                0x2b, 0x00, 0x2a, 0x00
                              };

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
        ShareData sdata(stream);
        sdata.emit_begin(PDUTYPE2_FONTMAP, this->share_id, RDP::STREAM_MED);

        // Payload
        stream.out_copy_bytes((char*)g_fontmap, 172);

        // Packet trailer
        sdata.emit_end();
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }

    /* PDUTYPE_DATAPDU */
    void process_data(Stream & stream, Callback & cb) throw (Error)
    {
        if (this->verbose & 4){
            LOG(LOG_INFO, "Front::process_data(...)");
        }
        ShareData sdata_in(stream);
        sdata_in.recv_begin();
        if (this->verbose > 0x80){
            LOG(LOG_INFO, "sdata_in.pdutype2=%u"
                          " sdata_in.len=%u"
                          " sdata_in.compressedLen=%u"
                          " remains=%u",
                (unsigned)sdata_in.pdutype2,
                (unsigned)sdata_in.len,
                (unsigned)sdata_in.compressedLen,
                (unsigned)(stream.end - stream.p)
            );
        }

        switch (sdata_in.pdutype2) {
        case PDUTYPE2_UPDATE:  // Update PDU (section 2.2.9.1.1.3)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_UPDATE");
            }
        break;
        case PDUTYPE2_CONTROL: // 20(0x14) Control PDU (section 2.2.1.15.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_CONTROL");
            }
            {
                int action = stream.in_uint16_le();
                stream.in_skip_bytes(2); /* user id */
                stream.in_skip_bytes(4); /* control id */
                switch (action){
                    case RDP_CTL_REQUEST_CONTROL:
                        this->send_control(RDP_CTL_GRANT_CONTROL);
                    break;
                    case RDP_CTL_COOPERATE:
                        this->send_control(RDP_CTL_COOPERATE);
                    break;
                    default:
                        LOG(LOG_WARNING, "process DATA_PDU_CONTROL unknown action (%d)\n", action);
                }
            }
            break;
        case PDUTYPE2_POINTER: // Pointer Update PDU (section 2.2.9.1.1.4)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_POINTER");
            }
        break;
        case PDUTYPE2_INPUT:   // 28(0x1c) Input PDU (section 2.2.8.1.1.3)
            {
                int num_events = stream.in_uint16_le();

                if (this->verbose & 2){
                    LOG(LOG_INFO, "PDUTYPE2_INPUT num_events=%u", num_events);
                }

                stream.in_skip_bytes(2); /* pad */
                for (int index = 0; index < num_events; index++) {
                    int time = stream.in_uint32_le();
                    uint16_t msg_type = stream.in_uint16_le();
                    uint16_t device_flags = stream.in_uint16_le();
                    int16_t param1 = stream.in_sint16_le();
                    int16_t param2 = stream.in_sint16_le();

                    TODO(" we should always call send_input with original data  if the other side is rdp it will merely transmit it to the other end without change. If the other side is some internal module it will be it's own responsibility to decode it")
                    TODO(" with the scheme above  any kind of keymap management is only necessary for internal modules or if we convert mapping. But only the back-end module really knows what the target mapping should be.")
                    switch (msg_type) {
                    case RDP_INPUT_SYNCHRONIZE:
                        if (this->verbose & 2){
                            LOG(LOG_INFO, "RDP_INPUT_SYNCHRONIZE");
                        }
                        /* happens when client gets focus and sends key modifier info */
                        this->keymap.synchronize(param1);
                        if (this->up_and_running){
                            cb.rdp_input_synchronize(time, device_flags, param1, param2);
                        }
                        break;
                    case RDP_INPUT_SCANCODE:
                        {
                            if (this->verbose & 2){
                                LOG(LOG_INFO, "RDP_INPUT_SCANCODE time=%u flags=%04x param1=%04x param2=%04x",
                                    time, device_flags, param1, param2
                                );
                            }
                            this->keymap.event(device_flags, param1);
                            if (this->up_and_running){
                                cb.rdp_input_scancode(param1, param2, device_flags, time, &this->keymap);
                            }
                        }
                        break;
                    case RDP_INPUT_MOUSE:
                        if (this->verbose & 6){
                            LOG(LOG_INFO, "RDP_INPUT_MOUSE(device_flags=%u, param1=%u, param2=%u)", device_flags, param1, param2);
                        }
                        this->mouse_x = param1;
                        this->mouse_y = param2;
                        if (this->up_and_running){
                            cb.rdp_input_mouse(device_flags, param1, param2, &this->keymap);
                        }
                        break;
                    default:
                        LOG(LOG_INFO, "unsupported PDUTYPE2_INPUT msg %u", msg_type);
                        break;
                    }
                }
            }
        break;
        case PDUTYPE2_SYNCHRONIZE:  // Synchronize PDU (section 2.2.1.14.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");
            }
            {
                uint16_t messageType = stream.in_uint16_le();
                uint16_t controlId = stream.in_uint16_le();
                if (this->verbose){
                    LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE"
                                  " messageType=%u controlId=%u",
                                  (unsigned)messageType,
                                  (unsigned)controlId);
                }
                this->send_synchronize();
            }
        break;
        case PDUTYPE2_REFRESH_RECT: // Refresh Rect PDU (section 2.2.11.2.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT");
            }
            {
                /* int op = */ stream.in_uint32_le();
                int left = stream.in_uint16_le();
                int top = stream.in_uint16_le();
                int right = stream.in_uint16_le();
                int bottom = stream.in_uint16_le();
                int cx = (right - left) + 1;
                int cy = (bottom - top) + 1;
                if (this->verbose){
                    LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT"
                        " left=%u top=%u right=%u bottom=%u cx=%u cy=%u",
                        left, top, right, bottom, cx, cy);
                }
                if (this->up_and_running){
                    cb.rdp_input_invalidate(Rect(left, top, cx, cy));
                }
            }
        break;
        case PDUTYPE2_PLAY_SOUND:   // Play Sound PDU (section 2.2.9.1.1.5.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_PLAY_SOUND");
            }
        break;
        case PDUTYPE2_SUPPRESS_OUTPUT:  // Suppress Output PDU (section 2.2.11.3.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SUPPRESS_OUTPUT");
            }
            // PDUTYPE2_SUPPRESS_OUTPUT comes when minimizing a full screen
            // mstsc.exe 2600. I think this is saying the client no longer wants
            // screen updates and it will issue a PDUTYPE2_REFRESH_RECT above
            // to catch up so minimized apps don't take bandwidth
            break;

        break;
        case PDUTYPE2_SHUTDOWN_REQUEST: // Shutdown Request PDU (section 2.2.2.2.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_REQUEST");
            }
            {
                // when this message comes, send a PDUTYPE2_SHUTDOWN_DENIED back
                // so the client is sure the connection is alive and it can ask
                // if user really wants to disconnect */

                BStream stream(65536);
                Mcs mcs(stream);
                mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
                Sec sec(stream, this->encrypt);
                sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
                ShareControl sctrl(stream);
                sctrl.emit_begin(PDUTYPE_DATAPDU, this->userid + MCS_USERCHANNEL_BASE);
                ShareData sdata_out(stream);
                sdata_out.emit_begin(PDUTYPE2_SHUTDOWN_DENIED, this->share_id, RDP::STREAM_MED);

                // Packet trailer
                sdata_out.emit_end();
                sctrl.emit_end();
                sec.emit_end();
                mcs.emit_end();
                stream.end = stream.p;    

                BStream x224_header(256);
                X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

                trans->send(x224_header.data, x224_header.end - x224_header.data);
                trans->send(stream.data, stream.end - stream.data);
            }
        break;
        case PDUTYPE2_SHUTDOWN_DENIED:  // Shutdown Request Denied PDU (section 2.2.2.3.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED");
            }
        break;
        case PDUTYPE2_SAVE_SESSION_INFO: // Save Session Info PDU (section 2.2.10.1.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SAVE_SESSION_INFO");
            }
        break;
        case PDUTYPE2_FONTLIST: // 39(0x27) Font List PDU (section 2.2.1.18.1)
        {
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_FONTLIST");
            }
        // 2.2.1.18.1 Font List PDU Data (TS_FONT_LIST_PDU)
        // ================================================
        // The TS_FONT_LIST_PDU structure contains the contents of the Font
        // List PDU, which is a Share Data Header (section 2.2.8.1.1.1.2) and
        // four fields.

        // shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
        // containing information about the packet. The type subfield of the
        // pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
        // MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share
        // Data Header MUST be set to PDUTYPE2_FONTLIST (39).

        // numberFonts (2 bytes): A 16-bit, unsigned integer. The number of
        // fonts. This field SHOULD be set to 0.

        // totalNumFonts (2 bytes): A 16-bit, unsigned integer. The total number
        // of fonts. This field SHOULD be set to 0.

        // listFlags (2 bytes): A 16-bit, unsigned integer. The sequence flags.
        // This field SHOULD be set to 0x0003, which is the logical OR'ed value
        // of FONTLIST_FIRST (0x0001) and FONTLIST_LAST (0x0002).

        // entrySize (2 bytes): A 16-bit, unsigned integer. The entry size. This
        // field SHOULD be set to 0x0032 (50 bytes).

            stream.in_uint16_le(); /* numberFont -> 0*/
            stream.in_uint16_le(); /* totalNumFonts -> 0 */
            int seq = stream.in_uint16_le();
            stream.in_uint16_le(); /* entrySize -> 50 */

            /* 419 client sends Seq 1, then 2 */
            /* 2600 clients sends only Seq 3 */
            /* after second font message, we are up and running */
            if (seq == 2 || seq == 3)
            {
                this->send_fontmap();
                this->send_data_update_sync();

                if (this->client_info.bpp == 8){
                    BGRPalette palette;
                    init_palette332(palette);
                    this->color_cache(palette, 0);
                }
                this->init_pointers();

                if (this->verbose){
                    LOG(LOG_INFO, "--------------> UP AND RUNNING <----------------");
                }
                this->up_and_running = 1;
            }
        }
        break;
        case PDUTYPE2_FONTMAP:  // Font Map PDU (section 2.2.1.22.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_FONTMAP");
            }
        break;
        case PDUTYPE2_SET_KEYBOARD_INDICATORS: // Set Keyboard Indicators PDU (section 2.2.8.2.1.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SET_KEYBOARD_INDICATORS");
            }
        break;
        case PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST: // Persistent Key List PDU (section 2.2.1.17.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST");
            }
            stream.in_skip_bytes(sdata_in.len);
        break;
        case PDUTYPE2_BITMAPCACHE_ERROR_PDU: // Bitmap Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_BITMAPCACHE_ERROR_PDU");
            }
        break;
        case PDUTYPE2_SET_KEYBOARD_IME_STATUS: // Set Keyboard IME Status PDU (section 2.2.8.2.2.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SET_KEYBOARD_IME_STATUS");
            }
        break;
        case PDUTYPE2_OFFSCRCACHE_ERROR_PDU: // Offscreen Bitmap Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.2)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_OFFSCRCACHE_ERROR_PDU");
            }
        break;
        case PDUTYPE2_SET_ERROR_INFO_PDU: // Set Error Info PDU (section 2.2.5.1.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");
            }
        break;
        case PDUTYPE2_DRAWNINEGRID_ERROR_PDU: // DrawNineGrid Cache Error PDU (see [MS-RDPEGDI] section 2.2.2.3.3)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_DRAWNINEGRID_ERROR_PDU");
            }
        break;
        case PDUTYPE2_DRAWGDIPLUS_ERROR_PDU: // GDI+ Error PDU (see [MS-RDPEGDI] section 2.2.2.3.4)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_DRAWGDIPLUS_ERROR_PDU");
            }
        break;
        case PDUTYPE2_ARC_STATUS_PDU: // Auto-Reconnect Status PDU (section 2.2.4.1.1)
            if (this->verbose){
                LOG(LOG_INFO, "PDUTYPE2_ARC_STATUS_PDU");
            }
        break;

        default:
            LOG(LOG_WARNING, "unsupported PDUTYPE in process_data %d\n", sdata_in.pdutype2);
            break;
        }

        sdata_in.recv_end();

        if (this->verbose & 4){
            LOG(LOG_INFO, "process_data done");
        }
    }

    void send_deactive() throw (Error)
    {
        LOG(LOG_INFO, "send_deactive");

        BStream stream(65536);
        Mcs mcs(stream);
        mcs.emit_begin(MCSPDU_SendDataIndication, this->userid, MCS_GLOBAL_CHANNEL);
        Sec sec(stream, this->encrypt);
        sec.emit_begin(this->client_info.crypt_level?SEC_ENCRYPT:0);
        ShareControl sctrl(stream);
        sctrl.emit_begin(PDUTYPE_DEACTIVATEALLPDU, this->userid + MCS_USERCHANNEL_BASE);

        // Packet trailer
        sctrl.emit_end();
        sec.emit_end();
        mcs.emit_end();
        stream.end = stream.p;    

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }


    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPOpaqueRect new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor color24 = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
                new_cmd.color = color_encode(color24, this->client_info.bpp);
            }
            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPOpaqueRect new_cmd24 = cmd;
                new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->orders->draw(cmd, clip);

            if (this->capture){
                this->capture->draw(cmd, clip);
            }
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->orders->draw(cmd, clip);
            if (this->capture){ this->capture->draw(cmd, clip); }
        }
    }


    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){
            this->send_global_palette();

            const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
            const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);

            RDPPatBlt new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                new_cmd.back_color= color_encode(back_color24, this->client_info.bpp);
                new_cmd.fore_color= color_encode(fore_color24, this->client_info.bpp);
                // this may change the brush add send it to to remote cache
            }
            this->cache_brush(new_cmd.brush);
            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPPatBlt new_cmd24 = cmd;
                new_cmd24.back_color = back_color24;
                new_cmd24.fore_color = fore_color24;
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw_vnc(const Rect & rect, const uint8_t bpp, const BGRPalette & palette332, const uint8_t * raw, uint32_t need_size)
    {
        const uint16_t TILE_CX = 32;
        const uint16_t TILE_CY = 32;

        for (int y = 0; y < rect.cy ; y += TILE_CY) {
            int cy = std::min(TILE_CY, (uint16_t)(rect.cy - y));

            for (int x = 0; x < rect.cx ; x += TILE_CX) {
                int cx = std::min(TILE_CX, (uint16_t)(rect.cx - x));

                const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                const Rect src_tile(x, y, cx, cy);

                const Bitmap tiled_bmp(raw, rect.cx, rect.cy, bpp, src_tile);
                const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                this->orders->draw(cmd2, dst_tile, tiled_bmp);
                if (this->capture){
                    this->capture->draw(cmd2, dst_tile, tiled_bmp);
                }
            }
        }

    }

    void draw_tile(const Rect & dst_tile, const Rect & src_tile, const RDPMemBlt & cmd, const Bitmap & bitmap, const Rect & clip)
    {
//        LOG(LOG_INFO, "front::draw:draw_tile((%u, %u, %u, %u) (%u, %u, %u, %u)",
//             dst_tile.x, dst_tile.y, dst_tile.cx, dst_tile.cy,
//             src_tile.x, src_tile.y, src_tile.cx, src_tile.cy);
        // No need to resize bitmap

//        if (dst_tile.x + dst_tile.cx == this->client_info.width
//        && dst_tile.y + dst_tile.cy == this->client_info.height){
//            return;
//        }


        if (src_tile == Rect(0, 0, bitmap.cx, bitmap.cy)){
            const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
            this->orders->draw(cmd2, clip, bitmap);
            if (this->capture){
                this->capture->draw(cmd2, clip, bitmap);
            }
        }
        else {
            const Bitmap tiled_bmp(bitmap, src_tile);
            const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
            this->orders->draw(cmd2, clip, tiled_bmp);
            if (this->capture){
                this->capture->draw(cmd2, clip, tiled_bmp);
            }
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap)
    {
        if (bitmap.cx < cmd.srcx || bitmap.cy < cmd.srcy){
            return;
        }

        this->send_global_palette();

        const uint8_t palette_id = 0;
        if (this->client_info.bpp == 8){
            if (!this->palette_memblt_sent[palette_id]) {
                this->color_cache(bitmap.original_palette, palette_id);
                this->palette_memblt_sent[palette_id] = true;
            }
        }

        // if not we have to split it
        const uint16_t TILE_CX = 32;
        const uint16_t TILE_CY = 32;

        const uint16_t dst_x = cmd.rect.x;
        const uint16_t dst_y = cmd.rect.y;
        // clip dst as it can be larger than source bitmap
        const uint16_t dst_cx = std::min<uint16_t>(bitmap.cx - cmd.srcx, cmd.rect.cx);
        const uint16_t dst_cy = std::min<uint16_t>(bitmap.cy - cmd.srcy, cmd.rect.cy);

        // check if target bitmap can be fully stored inside one front cache entry
        // if so no need to tile it.
        uint32_t front_bitmap_size = ::nbbytes(this->client_info.bpp) * align4(dst_cx) * dst_cy;
        // even if cache seems to be large enough, cache entries cant be used
        // for values whose width is larger or equal to 256 after alignment
        // hence, we check for this case. There does not seem to exist any
        // similar restriction on cy actual reason of this is unclear
        // (I don't even know if it's related to redemption code or client code).
//        LOG(LOG_INFO, "cache1=%u cache2=%u cache3=%u bmp_size==%u",
//            this->client_info.cache1_size,
//            this->client_info.cache2_size,
//            this->client_info.cache3_size,
//            front_bitmap_size);
        if (front_bitmap_size <= this->client_info.cache3_size
            && align4(dst_cx) < 128 && dst_cy < 128){
            // clip dst as it can be larger than source bitmap
            const Rect dst_tile(dst_x, dst_y, dst_cx, dst_cy);
            const Rect src_tile(cmd.srcx, cmd.srcy, dst_cx, dst_cy);
            this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip);
        }
        else {
            for (int y = 0; y < dst_cy ; y += TILE_CY) {
                int cy = std::min(TILE_CY, (uint16_t)(dst_cy - y));

                for (int x = 0; x < dst_cx ; x += TILE_CX) {
                    int cx = std::min(TILE_CX, (uint16_t)(dst_cx - x));

                    const Rect dst_tile(dst_x + x, dst_y + y, cx, cy);
                    const Rect src_tile(cmd.srcx + x, cmd.srcy + y, cx, cy);
                    this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip);
                }
            }
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        const uint16_t minx = std::min(cmd.startx, cmd.endx);
        const uint16_t miny = std::min(cmd.starty, cmd.endy);
        const Rect rect(minx, miny,
                        std::max(cmd.startx, cmd.endx)-minx+1,
                        std::max(cmd.starty, cmd.endy)-miny+1);

        if (!clip.isempty() && !clip.intersect(rect).isempty()){

            RDPLineTo new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd.back_color = color_encode(back_color24, this->client_info.bpp);
                const BGRColor pen_color24 = color_decode_opaquerect(cmd.pen.color, this->mod_bpp, this->mod_palette);
                new_cmd.pen.color = color_encode(pen_color24, this->client_info.bpp);
            }

            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPLineTo new_cmd24 = cmd;
                new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd24.pen.color = color_decode_opaquerect(cmd.pen.color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        if (!clip.isempty() && !clip.intersect(cmd.bk).isempty()){
            this->send_global_palette();

            RDPGlyphIndex new_cmd = cmd;
            if (this->client_info.bpp != this->mod_bpp){
                const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                new_cmd.back_color = color_encode(back_color24, this->client_info.bpp);
                new_cmd.fore_color = color_encode(fore_color24, this->client_info.bpp);
            }

            // this may change the brush and send it to to remote cache
            this->cache_brush(new_cmd.brush);

            this->orders->draw(new_cmd, clip);

            if (this->capture){
                RDPGlyphIndex new_cmd24 = cmd;
                new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
                new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                this->capture->draw(new_cmd24, clip);
            }
        }
    }

    void draw(const RDPGlyphCache & cmd)
    {
        this->orders->draw(cmd);
    }

    void flush(){}

    void cache_brush(RDPBrush & brush)
    {
        if ((brush.style == 3) && (this->client_info.brush_cache_code == 1)) {
            uint8_t pattern[8];
            pattern[0] = brush.hatch;
            memcpy(pattern+1, brush.extra, 7);
            int cache_idx = 0;
            if (BRUSH_TO_SEND == this->brush_cache.add_brush(pattern, cache_idx)){
                RDPBrushCache cmd(cache_idx, 1, 8, 8, 0x81,
                    sizeof(this->brush_cache.brush_items[cache_idx].pattern),
                    this->brush_cache.brush_items[cache_idx].pattern);
                this->orders->draw(cmd);
            }
            brush.hatch = cache_idx;
            brush.style = 0x81;
        }
    }

    void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
    {
        RDPColCache cmd(cacheIndex, palette);
        this->orders->draw(cmd);
    }

    void set_mod_palette(const BGRPalette & palette)
    {
        this->mod_palette_setted = true;
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
            this->memblt_mod_palette[i] = RGBtoBGR(palette[i]);
        }
        this->palette_sent = false;
    }
};

#endif
